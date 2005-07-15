/* vim: set ts=4: */
#include <stdio.h>
#include <sys/queue.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <sys/stat.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <glib.h>

#include "draw.h"
#include "pref.h"
#include "bsv_module.h"

/* Debugging ******************************************************************/

//#define DEBUG
#undef DEBUG
#ifdef DEBUG
#define say(fun, fmt, args...)	printf("DEBUG: "fun": "fmt"\n", ##args) 
#else
#define say(fun, fmt, args...)
#endif

/* Macros *********************************************************************/

#define ABSMAX(x, y) ( ( (((x)<0)?-(x):(x)) < (((y)<0)?-(y):(y)) ) ? \
									(((y)<0)?-(y):(y)) : (((x)<0)?-(x):(x)) )


/* Defines ********************************************************************/

#define DEAD				0
#define ALIVE				1
#define RUNNING				2
#define REDRAWING			3
#define DYING				4

#define E_POINTS			8
#define DA_XGRID_WIDTH		64		/* must be a multible of E_POINTS */
#define DA_XGRID_OFF		3
#define G_REDRAW_CNT		1
#define DA_X_BWID			75
#define DA_Y_BWID			20


/* Typedefs *******************************************************************/

typedef struct d_chan {
	CIRCLEQ_ENTRY(d_chan)		entries;
	GdkPoint*					d_points;
	int							d_wrptr;
	int							d_size;
	GdkPoint					e_points[E_POINTS+1];
	int							e_wrptr;
	int							e_off;
	int							da_off;
	int							da_ydelta;
	int							g_redraw_cnt;
	unsigned long				chtype;
	unsigned long				srecnum;
	const char*					label;
	const char*					pfilt;
	const char*					pdim;
	int							mag;
	int							notch;
	gint						s_off;
	gint						s_dmaxabs;
	double						s_pmaxabs;
} d_chan_t;


/* Global Variables ***********************************************************/

static CIRCLEQ_HEAD(circleq, d_chan)	head;
static int								channum			= 0;
static GtkWidget*						drawarea		= NULL;
static GMutex*							start_mtx_ptr;
static GCond*							start_con_ptr;
static GMutex*							state_mtx_ptr;
static volatile unsigned char			state			= DEAD;
static volatile unsigned char			wait_cnt		= 0;
static bsv_data_t*						sample_ptr		= NULL;
static GdkGC*							draw_color		= NULL;
static GdkGC*							grid_color		= NULL;
static GdkGC*							erase_color		= NULL;
static char*							errormsg_ptr	= NULL;
static GThread*							drawer_ptr; 
static char*							terrormsg_ptr	= NULL;
static int								channel_offset	= 0;


/* External Variables *********************************************************/

extern GtkWidget*						bsv_main_win;


/* Private functions **********************************************************/

static void clear_circleq(void)
{
	d_chan_t*	chan_ptr;

	while(head.cqh_first != (void*)&head) {
		chan_ptr	= head.cqh_first;

		/* remove item from list */
		CIRCLEQ_REMOVE(&head, head.cqh_first, entries);

		/* dealloc item */
		free(chan_ptr->d_points);
		free(chan_ptr);
	}
}

static void init_drawarea(void)
{
	GdkColor	c;

	/* realize */
	gtk_widget_realize(drawarea);
	
	/* setup colors */
	draw_color	= gdk_gc_new(drawarea->window);

	gdk_color_parse("black", &c);
	gdk_gc_set_rgb_fg_color(draw_color, &c);

	gdk_color_parse("seashell", &c);
	gdk_gc_set_rgb_bg_color(draw_color, &c);

	gdk_gc_set_line_attributes(draw_color, 1,				/* width */
										   GDK_LINE_SOLID,	/* stile */
										   GDK_CAP_ROUND,	/* end of lines */
										   GDK_JOIN_ROUND);	/* round join */

	grid_color	= gdk_gc_new(drawarea->window);

	gdk_color_parse("sky blue", &c);
	gdk_gc_set_rgb_fg_color(grid_color, &c);

	gdk_color_parse("seashell", &c);
	gdk_gc_set_rgb_bg_color(grid_color, &c);

	gdk_gc_set_line_attributes(grid_color, 1,				/* width */
										   GDK_LINE_SOLID,	/* stile */
										   GDK_CAP_ROUND,	/* end of lines */
										   GDK_JOIN_ROUND);	/* round join */

	erase_color	= gdk_gc_new(drawarea->window);

	gdk_gc_set_rgb_fg_color(erase_color, &c);

	gdk_gc_set_rgb_bg_color(erase_color, &c);

	gdk_gc_set_line_attributes(erase_color, 1,				/* width */
											GDK_LINE_SOLID,	/* stile */
											GDK_CAP_ROUND,	/* end of lines */
											GDK_JOIN_ROUND);/* round join */

	/* setup background color */
	gtk_widget_modify_bg(drawarea, GTK_STATE_NORMAL, &c);
}

static void draw_set_xpoints(d_chan_t* c_ptr)
{
	int	i;
	int	width;

	gdk_drawable_get_size(drawarea->window, &width, NULL);

	for(i = 0; i < c_ptr->d_size; i++) {
		c_ptr->d_points[i].x	= DA_X_BWID + 
								  (((width-DA_X_BWID)*i) / c_ptr->d_size);
		c_ptr->d_points[i].y	= c_ptr->da_off;
	}
}

static void draw_set_yoffdelta(d_chan_t* c_ptr, int cnt)
{
	int		delta;
	int		height;
	int		cnum;

	gdk_drawable_get_size(drawarea->window, NULL, &height);

	cnum	= (p_get_chanperscreen() > channum) ? channum : 
												  p_get_chanperscreen();
	delta	= (height - (DA_Y_BWID*cnum)) / cnum;
	c_ptr->da_off	= (delta*cnt + DA_Y_BWID*(cnt+1) + delta/2) - 
					  (delta+DA_Y_BWID)*channel_offset;
	c_ptr->da_ydelta	= delta / 2;

	say("draw_set_yoffdelta",
		"cps:%d o:%d yd:%d d:%d",
		cnum, c_ptr->da_off, c_ptr->da_ydelta, delta);
}

static int setup_channels(void)
{
	bsv_channelinfo_t*	chaninfo_ptr;
	d_chan_t*			c_ptr;
	bsv_modinfo_t*		modinfo_ptr;

	channum			= 0;
	channel_offset	= 0;

	modinfo_ptr	= bsv_get_moduleinfo();

	/* Add channels */
	while( (chaninfo_ptr = bsv_get_channelinfo(channum)) ) {
		c_ptr	= (d_chan_t*)calloc(sizeof(struct d_chan), 1);
		if( !c_ptr ) {
			return -1;
		}

		/* d_size must be a multiple of E_POINTS */
		c_ptr->d_size	= (( (p_get_hrange()*BSV_GET_SRATEDENOM(modinfo_ptr)) / 
							 BSV_GET_SRATENOM(modinfo_ptr) ) * E_POINTS)
						  / E_POINTS;

		say("setup_channels",
			"hr:%d den:%d n:%d d_size:%d",
			p_get_hrange(), 
			BSV_GET_SRATEDENOM(modinfo_ptr), 
			BSV_GET_SRATENOM(modinfo_ptr), 
			c_ptr->d_size);
						  
		c_ptr->d_points	= (GdkPoint*)calloc(sizeof(GdkPoint)*c_ptr->d_size, 1);
		c_ptr->d_wrptr	= 0;

		c_ptr->e_wrptr	= 0;
		c_ptr->e_off	= 1;

		c_ptr->g_redraw_cnt	= 0;

		c_ptr->chtype	= BSV_CH_GET_CHTYPE(chaninfo_ptr);
		c_ptr->srecnum	= BSV_CH_GET_SRECNUM(chaninfo_ptr);
		c_ptr->label	= BSV_CH_GET_LABEL(chaninfo_ptr);
		c_ptr->pfilt	= BSV_CH_GET_PFILT(chaninfo_ptr);
		c_ptr->pdim		= BSV_CH_GET_PDIM(chaninfo_ptr);

		c_ptr->mag		= 100;
		c_ptr->notch	= 0;

		c_ptr->s_off	= (gint)(((BSV_CH_GET_PMAX(chaninfo_ptr)*
								   ((double)BSV_CH_GET_DMAX(chaninfo_ptr)-
								    (double)BSV_CH_GET_DMIN(chaninfo_ptr)+
								    1
								   )
								  )/
								  (BSV_CH_GET_PMAX(chaninfo_ptr)-
								   BSV_CH_GET_PMIN(chaninfo_ptr)
								  )
								 )-
								 1
								)-
								(gint)BSV_CH_GET_DMAX(chaninfo_ptr);
		c_ptr->s_pmaxabs= ABSMAX(BSV_CH_GET_PMAX(chaninfo_ptr),
								 BSV_CH_GET_PMIN(chaninfo_ptr));
		c_ptr->s_dmaxabs= ABSMAX(BSV_CH_GET_DMAX(chaninfo_ptr),
								 BSV_CH_GET_DMIN(chaninfo_ptr));

		/* Insert into queue */
		CIRCLEQ_INSERT_TAIL(&head, c_ptr, entries);

		/* Update number of channels */
		channum++;

		say("setup_channels", 
			"cn:%d label:%s", 
			channum, 
			BSV_CH_GET_LABEL(chaninfo_ptr));
	}

	return 0;
}

static int setup_drawarea(void)
{
	d_chan_t*		c_ptr;
	int 			val;
	GtkRange*		vscrollbar;
	GtkComboBox*	combobox;
	GtkSpinButton*	spinbut;
	GtkButton*		but;

	/* Update chansel */
	combobox	= (GtkComboBox*)lookup_widget(bsv_main_win, "chansel_combo");
	gdk_threads_enter();
	gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(combobox)));
	gdk_threads_leave();
	for(c_ptr = head.cqh_first; 
		c_ptr != (void*)&head;
		c_ptr = c_ptr->entries.cqe_next) {
		gdk_threads_enter();
		gtk_combo_box_append_text(combobox, (const gchar*)c_ptr->label);
		gdk_threads_leave();
	}
	gdk_threads_enter();
	gtk_widget_set_sensitive((GtkWidget*)combobox, TRUE);
	gdk_threads_leave();

	/* Update scrollbar */
	vscrollbar	= (GtkRange*)lookup_widget(bsv_main_win, "vscrollbar1");
	gdk_threads_enter();
	if( channum < p_get_chanperscreen() ) {
		gtk_range_set_range(vscrollbar, (gdouble)0, (gdouble)1);
	} else {
		gtk_range_set_range(vscrollbar, (gdouble)0, 
							(gdouble)(channum-p_get_chanperscreen()+1));
	}
	gtk_range_set_value(vscrollbar, (gdouble)0);
	gtk_widget_set_sensitive((GtkWidget*)vscrollbar, TRUE);
	gdk_threads_leave();

	/* Update spinbut */
	spinbut		= (GtkSpinButton*)lookup_widget(bsv_main_win, "mag_spinbut");
	gdk_threads_enter();
	gtk_spin_button_set_range(spinbut, 100, 999);
	gtk_widget_set_sensitive((GtkWidget*)spinbut, TRUE);
	gdk_threads_leave();

	/* Update notch combobox */
	combobox	= (GtkComboBox*)lookup_widget(bsv_main_win, "combo1");
	gdk_threads_enter();
	gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(combobox)));	
	gtk_combo_box_append_text(combobox, "Off");
	/* TODO Implement the notch filter
	gtk_combo_box_append_text(combobox, "On"); */
	gtk_widget_set_sensitive((GtkWidget*)combobox, TRUE);
	gdk_threads_leave();

	/* Update button */	
	but			= (GtkButton*)lookup_widget(bsv_main_win, "apply_but");
	gdk_threads_enter();
	gtk_widget_set_sensitive((GtkWidget*)but, TRUE);
	gdk_threads_leave();

	return 0;
}

static void cleanup_drawarea(void)
{
	gint			val;
	GtkRange*		vscrollbar;
	GtkComboBox*	combobox;
	GtkSpinButton*	spinbut;
	GtkButton*		but;

	/* Update chansel */
	combobox	= (GtkComboBox*)lookup_widget(bsv_main_win, "chansel_combo");
	gdk_threads_enter();
	gtk_widget_set_sensitive((GtkWidget*)combobox, FALSE);
	gdk_threads_leave();

	/* Update scrollbar */
	vscrollbar	= (GtkRange*)lookup_widget(bsv_main_win, "vscrollbar1");
	gdk_threads_enter();
	gtk_widget_set_sensitive((GtkWidget*)vscrollbar, FALSE);
	gdk_threads_leave();

	/* Update spinbut */
	spinbut		= (GtkSpinButton*)lookup_widget(bsv_main_win, "mag_spinbut");
	gdk_threads_enter();
	gtk_widget_set_sensitive((GtkWidget*)spinbut, FALSE);
	gdk_threads_leave();

	/* Update notch combobox */
	combobox	= (GtkComboBox*)lookup_widget(bsv_main_win, "combo1");
	gdk_threads_enter();
	gtk_widget_set_sensitive((GtkWidget*)combobox, FALSE);
	gdk_threads_leave();

	/* Update button */	
	but			= (GtkButton*)lookup_widget(bsv_main_win, "apply_but");
	gdk_threads_enter();
	gtk_widget_set_sensitive((GtkWidget*)but, FALSE);
	gdk_threads_leave();
}

static inline void save_old_point(d_chan_t* c_ptr)
{
	c_ptr->e_wrptr++;

	/* save point */
	c_ptr->e_points[c_ptr->e_wrptr].x	= c_ptr->d_points[c_ptr->d_wrptr].x;
	c_ptr->e_points[c_ptr->e_wrptr].y	= c_ptr->d_points[c_ptr->d_wrptr].y;

	/* wrap ewrptr if necessary */
	if( c_ptr->e_wrptr >= E_POINTS ) {
		c_ptr->e_wrptr	= 0;
	}

	say("save_old_point",
		"e_wrptr:%d",
		c_ptr->e_wrptr);
}

static inline int get_new_point(d_chan_t* c_ptr, int* o_ptr, gint* p_ptr)
{
	int 	i;
	gint	sample;
	

	/* get new point - take the first point in case srecnum > 1 */
	switch( c_ptr->chtype ) {
		case BSV_CHTYPE_INT16: {
			sample		= *( (short*)(sample_ptr->samples + *o_ptr) );

			say("get_new_point",
				"s:%d off:%d",
				sample, *o_ptr);
			
			*o_ptr		+= c_ptr->srecnum*sizeof(short);
			sample		= ((sample+c_ptr->s_off)*-1*c_ptr->mag)/100;
			*p_ptr		= (sample*c_ptr->da_ydelta) / (c_ptr->s_dmaxabs);
		} break;

		case BSV_CHTYPE_UINT16: {
			sample      = *( (unsigned short*)(sample_ptr->samples + *o_ptr) );

			say("get_new_point",
				"s:%d off:%d",
				sample, *o_ptr);
			
			*o_ptr		+= c_ptr->srecnum*sizeof(short);
			sample		= ((sample+c_ptr->s_off)*-1*c_ptr->mag)/100;
			*p_ptr		= (sample*c_ptr->da_ydelta) / (c_ptr->s_dmaxabs);
		} break;

		default:
			terrormsg_ptr= "get_new_point: Error: Chtype not implemented";
			return -1;
	}

	say("get_new_point",
		"np:%d",
		*p_ptr);

	return 0;
}

static inline void get_preprocessed_point(d_chan_t* c_ptr, gint* p_ptr)
{
	say("get_preprocessed_point",
		"hu");
}

static inline void set_new_point(d_chan_t* c_ptr, gint* p_ptr)
{
	c_ptr->d_points[c_ptr->d_wrptr].y	= *p_ptr;

	/* Truncate value */
	if( c_ptr->d_points[c_ptr->d_wrptr].y > c_ptr->da_ydelta ) {
		c_ptr->d_points[c_ptr->d_wrptr].y	= c_ptr->da_ydelta;
	}
	if( c_ptr->d_points[c_ptr->d_wrptr].y < ((-1)*c_ptr->da_ydelta) ) {
		c_ptr->d_points[c_ptr->d_wrptr].y	= ((-1)*c_ptr->da_ydelta);
	}

	/* Add offset */
	c_ptr->d_points[c_ptr->d_wrptr].y	+= c_ptr->da_off;

	/* inc d_wrptr */
	c_ptr->d_wrptr++;
	if( c_ptr->d_wrptr >= c_ptr->d_size ) {
		c_ptr->d_wrptr		= 0;
	}

	say("set_new_point",
		"huhu");
}

static inline void draw_points(d_chan_t* c_ptr)
{
	GdkDisplay*	display;

	gdk_threads_enter();

	/* Erase last points */
	gdk_draw_lines(	drawarea->window,
					erase_color,
					&c_ptr->e_points[c_ptr->e_off],
					E_POINTS+1-c_ptr->e_off);

	if( !c_ptr->g_redraw_cnt ) {
		/* erase zero line */
		gdk_draw_line(	drawarea->window,
						erase_color,
						c_ptr->e_points[c_ptr->e_off].x,
						c_ptr->da_off,
						c_ptr->e_points[E_POINTS].x,
						c_ptr->da_off);

		if( !(c_ptr->e_points[c_ptr->e_off].x%DA_XGRID_WIDTH) ) {
			/* erase xgrid */
			gdk_draw_line(	drawarea->window,
							erase_color,
							c_ptr->e_points[c_ptr->e_off].x,
							c_ptr->da_off - c_ptr->da_ydelta,
							c_ptr->e_points[c_ptr->e_off].x,
							c_ptr->da_off + c_ptr->da_ydelta);
		}
	}

	if( c_ptr->d_wrptr > E_POINTS ) {
	
		/* draw grid */
		if( !c_ptr->g_redraw_cnt ) {
			/* draw zero line */
			gdk_draw_line(	drawarea->window,
							grid_color,
							c_ptr->d_points[c_ptr->d_wrptr-E_POINTS-1].x,
							c_ptr->da_off,
							c_ptr->d_points[c_ptr->d_wrptr-1].x,
							c_ptr->da_off);

			if( !((c_ptr->d_wrptr - E_POINTS) % DA_XGRID_WIDTH) ) {
				/* draw xgrid */
				gdk_draw_line(	drawarea->window,
								grid_color,
								c_ptr->d_points[c_ptr->d_wrptr-E_POINTS-1].x,
								c_ptr->da_off - c_ptr->da_ydelta,
								c_ptr->d_points[c_ptr->d_wrptr-E_POINTS-1].x,
								c_ptr->da_off+c_ptr->da_ydelta);
			}
		}

		/* draw last points */
		gdk_draw_lines(	drawarea->window,
						draw_color,
						&c_ptr->d_points[c_ptr->d_wrptr-E_POINTS-1],
						E_POINTS+1);

		c_ptr->e_points[0].x	= c_ptr->e_points[E_POINTS].x;
		c_ptr->e_points[0].y	= c_ptr->e_points[E_POINTS].y;
		c_ptr->e_off			= 0;
	} else 
	if( c_ptr->d_wrptr == E_POINTS ) {

		/* draw grid */
		if( !c_ptr->g_redraw_cnt ) {
			gdk_draw_line(	drawarea->window,
							grid_color,
							c_ptr->d_points[c_ptr->d_wrptr-E_POINTS].x,
							c_ptr->da_off,
							c_ptr->d_points[c_ptr->d_wrptr-1].x,
							c_ptr->da_off);

			/* draw xgrid at origin */
			gdk_draw_line(	drawarea->window,
							grid_color,
							c_ptr->d_points[c_ptr->d_wrptr-E_POINTS].x,
							c_ptr->da_off-c_ptr->da_ydelta,
							c_ptr->d_points[c_ptr->d_wrptr-E_POINTS].x,
							c_ptr->da_off+c_ptr->da_ydelta);
		}

		/* draw last points */
		gdk_draw_lines(	drawarea->window,
						draw_color,
						&c_ptr->d_points[c_ptr->d_wrptr-E_POINTS],
						E_POINTS);

		c_ptr->e_points[0].x	= c_ptr->e_points[E_POINTS].x;
		c_ptr->e_points[0].y	= c_ptr->e_points[E_POINTS].y;
		c_ptr->e_off			= 0;
	} else { 

		/* draw grid */
		if( !c_ptr->g_redraw_cnt ) {
			/* draw zero line */
			gdk_draw_line(	drawarea->window,
							grid_color,
							c_ptr->d_points[c_ptr->d_size-E_POINTS-1].x,
							c_ptr->da_off,
							c_ptr->d_points[c_ptr->d_size-1].x,
							c_ptr->da_off);

			if( !((c_ptr->d_size-E_POINTS) % DA_XGRID_WIDTH) ) {
				/* draw xgrid */
				gdk_draw_line(	drawarea->window,
								grid_color,
								c_ptr->d_points[c_ptr->d_size-E_POINTS-1].x,
								c_ptr->da_off-c_ptr->da_ydelta,
								c_ptr->d_points[c_ptr->d_size-E_POINTS-1].x,
								c_ptr->da_off+c_ptr->da_ydelta);
			}

			c_ptr->g_redraw_cnt	= G_REDRAW_CNT;
		} else {
			c_ptr->g_redraw_cnt--;
		}

		/* draw last points */
		gdk_draw_lines(	drawarea->window,
						draw_color,
						&c_ptr->d_points[c_ptr->d_size-E_POINTS-1],
						E_POINTS+1);

		c_ptr->e_off	= 1;
	}

	/* Flush */
	display	= gdk_display_get_default();
	XFlush(GDK_DISPLAY_XDISPLAY(display));
	
	gdk_threads_leave();
}

static int draw_samples(void)
{
	d_chan_t*	c_ptr;
	gint		npoint;
	int			chcnt;
	int			offset;		/* offset in sample_ptr array */

	for(c_ptr = head.cqh_first, chcnt = 0, offset = 0; 
		c_ptr != (void*)&head; 
		c_ptr = c_ptr->entries.cqe_next, chcnt++) {
		
		say("draw_samples",
			"chcnt:%d off:%d coff:%d",
			chcnt, offset, channel_offset);
	
		save_old_point(c_ptr);

		if( get_new_point(c_ptr, &offset, &npoint) ) {
			return -1;
		}

		get_preprocessed_point(c_ptr, &npoint);

		set_new_point(c_ptr, &npoint);

		if( !c_ptr->e_wrptr &&
			chcnt >= channel_offset &&
			chcnt < (channel_offset + p_get_chanperscreen()) ) {
			draw_points(c_ptr);
		}
	}

	return 0;
}

static void draw_grid(void)
{
	d_chan_t*		c_ptr;
	int				chcnt;
	int				width;
	GdkDisplay*		display;
	PangoLayout*	layout;
	char			str[150];

	gdk_threads_enter();
	gdk_window_clear(drawarea->window);

	gdk_drawable_get_size(drawarea->window, &width, NULL);

	for(c_ptr = head.cqh_first, chcnt = 0; 
		c_ptr != (void*)&head;
		c_ptr = c_ptr->entries.cqe_next, chcnt++) {
		if( chcnt >= channel_offset &&
			chcnt < (channel_offset + p_get_chanperscreen()) ) {

			/* High line */
			gdk_draw_line(	drawarea->window,
							grid_color,
							0,
							c_ptr->da_off-c_ptr->da_ydelta-1,
							width,
							c_ptr->da_off-c_ptr->da_ydelta-1);

			/* Low line */
			gdk_draw_line(	drawarea->window,
							grid_color,
							0,
							c_ptr->da_off+c_ptr->da_ydelta+1,
							width,
							c_ptr->da_off+c_ptr->da_ydelta+1);

			/* Voltage scaling */
			snprintf(str, 25, "%+2.1e", (c_ptr->s_pmaxabs*100)/
										(double)c_ptr->mag);
			layout	= gtk_widget_create_pango_layout(drawarea, str);
			gdk_draw_layout(drawarea->window, 
							draw_color, 
							0, 
							c_ptr->da_off-c_ptr->da_ydelta-2,
							layout);
			g_object_unref(layout);

			snprintf(str, 25, "+0.0e0");
			layout	= gtk_widget_create_pango_layout(drawarea, str);
			gdk_draw_layout(drawarea->window, 
							draw_color, 
							0, 
							c_ptr->da_off-10,
							layout);
			g_object_unref(layout);

			snprintf(str, 25, "%+2.1e", -(c_ptr->s_pmaxabs*100)/
										(double)c_ptr->mag);
			layout	= gtk_widget_create_pango_layout(drawarea, str);
			gdk_draw_layout(drawarea->window, 
							draw_color, 
							0, 
							c_ptr->da_off+c_ptr->da_ydelta-15,
							layout);
			g_object_unref(layout);

			/* Top description */
			snprintf(str, 25, "Volt. in %s", c_ptr->pdim);
			layout	= gtk_widget_create_pango_layout(drawarea, str);
			gdk_draw_layout(drawarea->window, 
							draw_color, 
							0, 
							c_ptr->da_off-c_ptr->da_ydelta-DA_Y_BWID+2,
							layout);
			g_object_unref(layout);

			/* Vertical line */
			gdk_draw_line(	drawarea->window,
							grid_color,
							DA_X_BWID,
							c_ptr->da_off-c_ptr->da_ydelta-DA_Y_BWID+1,
							DA_X_BWID,
							c_ptr->da_off-c_ptr->da_ydelta-1);

			/* Channel description */
			snprintf(str, 150, "Channel:%s Prefilt:%s Tbase:%+2.1es/div "
							  "Notch:%s", 
					 c_ptr->label, 
					 c_ptr->pfilt, 
					 ( ((double)(p_get_hrange()*DA_XGRID_WIDTH)) /
					   ((double)c_ptr->d_size) ), 
					 (c_ptr->notch)?"On":"Off");
			layout	= gtk_widget_create_pango_layout(drawarea, str);
			gdk_draw_layout(drawarea->window, 
							draw_color, 
							DA_X_BWID+2, 
							c_ptr->da_off-c_ptr->da_ydelta-DA_Y_BWID+2,
							layout);
			g_object_unref(layout);
		}
	}

	/* Flush */
	display	= gdk_display_get_default();
	XFlush(GDK_DISPLAY_XDISPLAY(display));

	gdk_threads_leave();

}

static int redraw(void)
{
	d_chan_t*		c_ptr;
	int				chcnt;

	say("redraw",
		"redraw");

	for(c_ptr = head.cqh_first, chcnt = 0; 
		c_ptr != (void*)&head;
		c_ptr = c_ptr->entries.cqe_next, chcnt++) {
		draw_set_yoffdelta(c_ptr, chcnt);
		draw_set_xpoints(c_ptr);
		c_ptr->g_redraw_cnt	= 0;
	}

	draw_grid();

	return 0;
}

static void* draw_data(void* param)
{
	/* First redraw */
	g_mutex_lock(state_mtx_ptr);
	if( state == ALIVE ) {
		state	= REDRAWING;
	} else {
		g_mutex_unlock(state_mtx_ptr);
		goto exit;
	}
	g_mutex_unlock(state_mtx_ptr);

	say("draw_data",
		"REDRAWING");

	while(1) {
		g_mutex_lock(state_mtx_ptr);
		if( state == RUNNING ) {
			g_mutex_unlock(state_mtx_ptr);

			/* Wait for something to draw */
			g_mutex_lock(start_mtx_ptr);

			/* Increase wait counter */
			wait_cnt++;

			g_cond_wait(start_con_ptr, start_mtx_ptr);

			/* Decrease wait counter */
			wait_cnt--;

			g_mutex_unlock(start_mtx_ptr);
		} else {
			g_mutex_unlock(state_mtx_ptr);
		}

		/* Check state */
		g_mutex_lock(state_mtx_ptr);
		switch( state) {
			case RUNNING:
				g_mutex_unlock(state_mtx_ptr);
				/* Draw */
				if( draw_samples() ) {
					goto exit;
				}
			break;
				
			case REDRAWING:
				g_mutex_unlock(state_mtx_ptr);
				/* Redraw */
				if( redraw() ) {
					goto exit;
				}

				/* New state */
				g_mutex_lock(state_mtx_ptr);
				state	= RUNNING;
				g_mutex_unlock(state_mtx_ptr);
			break;	

			default:
				g_mutex_unlock(state_mtx_ptr);
				/* DYING, aso... */
				goto exit;
		}
	}

exit:
	g_mutex_lock(state_mtx_ptr);
	state	= DEAD;
	g_mutex_unlock(state_mtx_ptr);

	say("draw_data",
		"DEAD");

	g_thread_exit(NULL);
}


/* Public functions ***********************************************************/

void d_init(void)
{
	CIRCLEQ_INIT(&head);
	state_mtx_ptr = g_mutex_new();
	start_mtx_ptr = g_mutex_new();
	start_con_ptr = g_cond_new();

	drawarea	= (GtkWidget*)lookup_widget(bsv_main_win, "drawingarea");
	init_drawarea();
}

void d_destroy(void)
{
	d_cleanup();

	g_mutex_destroy(state_mtx_ptr);
	g_mutex_destroy(start_mtx_ptr);
	g_cond_free(start_con_ptr);
}

int d_setup(void)
{
	if( setup_channels() ) {
		errormsg_ptr	= "d_setup: Error: Could not setup channels";
		return -1;
	}

	if( setup_drawarea() ) {
		errormsg_ptr	= "d_setup: Error: Could not setup drawarea";
		return -1;
	}

	g_mutex_lock(state_mtx_ptr);
	if( state != DEAD ) {
		/* set new state */
		state	= DEAD;
		g_mutex_unlock(state_mtx_ptr);

		/* Error */
		errormsg_ptr	= "d_setup: Error: Thread was running";
		return -1;
	}

	/* set new state */
	state	= ALIVE;
	g_mutex_unlock(state_mtx_ptr);

	/* start thread */
	drawer_ptr = g_thread_create(draw_data, NULL, TRUE, NULL);

	return 0;
}

void d_cleanup(void)
{
	g_mutex_lock(state_mtx_ptr);
	if( state != RUNNING && state != REDRAWING && state != ALIVE ) {
		g_mutex_unlock(state_mtx_ptr);

		/* Error */
		goto exit;
	}

	/* set new state */
	state	= DYING;
	g_mutex_unlock(state_mtx_ptr);

	g_cond_signal(start_con_ptr);
	g_thread_join(drawer_ptr);

exit:
	cleanup_drawarea();

	clear_circleq();
	terrormsg_ptr	= NULL;
}

int d_set_samples(void* s_ptr)
{
	say("d_set_samples",
		"d_set_samples");

	g_mutex_lock(state_mtx_ptr);
	switch( state ) {
		case RUNNING:
			g_mutex_unlock(state_mtx_ptr);

			/* Check if thread is waiting */
			g_mutex_lock(start_mtx_ptr);
			if( !wait_cnt ) {
				g_mutex_unlock(start_mtx_ptr);
				printf("d_set_samples: Warning: previous write "
					   "operation has not finished\n");
				return 0;
			} else {
				g_mutex_unlock(start_mtx_ptr);
			}

			/* set samples */
			sample_ptr		= (bsv_data_t*)s_ptr;
			g_cond_signal(start_con_ptr);

			say("d_set_samples",
				"s1:%d s2:%d", 
				*(short*)sample_ptr->samples, 
				*(short*)(sample_ptr->samples+2));
			return 0;
		
		case REDRAWING:
		case ALIVE:
			g_mutex_unlock(state_mtx_ptr);

			/* Ignore samples */
			return 0;
			
		default:
			g_mutex_unlock(state_mtx_ptr);

			/* Error */
			if( terrormsg_ptr ) {
				errormsg_ptr	= terrormsg_ptr;
				terrormsg_ptr	= NULL;
			} else {
				errormsg_ptr	= "d_set_samples: Error: Thread died"
								  " unexpectedly";
			}
			return -1;
	}

	/* Should never get here */
}

int d_set_offset(int off)
{
	say("d_set_offset",
		"off:%d",
		off);

	if( off != channel_offset ) {
		channel_offset	= off;
		return 0;
	} else {
		return -1;
	}
}

void d_redraw(void)
{
	say("d_redraw",
		"redraw");

	g_mutex_lock(state_mtx_ptr);
	if( state == RUNNING ) {
		/* Set new state */
		state	= REDRAWING;
		g_cond_signal(start_con_ptr);
	}
	g_mutex_unlock(state_mtx_ptr);
}

char* d_get_errormsg(void)
{
	return errormsg_ptr;
}

void d_control_load(int chansel_item)
{
	d_chan_t*		c_ptr;
	int				val;
	GtkSpinButton*	spinbut;
	GtkComboBox*	cbox;

	if( chansel_item < 0 ) {
		return;
	}

	spinbut	= (GtkSpinButton*)lookup_widget(bsv_main_win, "mag_spinbut");
	cbox	= (GtkComboBox*)lookup_widget(bsv_main_win, "combo1");

	for(c_ptr = head.cqh_first, val = 0; 
		c_ptr != (void*)&head;
		c_ptr = c_ptr->entries.cqe_next, val++) {
		if( val == chansel_item ) {
			gtk_combo_box_set_active(cbox, c_ptr->notch);
			gtk_spin_button_set_value(spinbut, (gdouble)c_ptr->mag);
		}
	}
}

void d_control_apply(int chansel_item, int mag, int notch_item)
{
	d_chan_t*		c_ptr;
	int				val;

	if( chansel_item < 0 || notch_item < 0 ) {
		return;
	}

	for(c_ptr = head.cqh_first, val = 0; 
		c_ptr != (void*)&head;
		c_ptr = c_ptr->entries.cqe_next, val++) {
		if( val == chansel_item ) {
			c_ptr->notch	= notch_item;
			c_ptr->mag		= mag;
		}
	}
}
