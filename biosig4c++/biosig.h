/*
% $Id: biosig.h,v 1.140 2009/04/09 15:06:55 schloegl Exp $
% Copyright (C) 2005,2006,2007,2008,2009,2010 Alois Schloegl <a.schloegl@ieee.org>
% This file is part of the "BioSig for C/C++" repository 
% (biosig4c++) at http://biosig.sf.net/ 


    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */


/* test whether HDR.CHANNEL[].{bi,bi8} can be replaced, reduction of header size by about 3% 
   currently this is not working, because FAMOS seems to need it. 	
//#define NO_BI 
*/


/* External API definitions */

/****************************************************************************/
/**                                                                        **/
/**                 DEFINITIONS, TYPEDEFS AND MACROS                       **/
/**                                                                        **/
/****************************************************************************/

#ifndef __BIOSIG_EXT_H__
#define __BIOSIG_EXT_H__

#ifdef _VCPP_DEF
#define __BYTE_ORDER  __LITTLE_ENDIAN
typedef unsigned __int64	uint64_t;
typedef __int64			int64_t;
typedef unsigned long		uint32_t;
typedef long			int32_t;
typedef unsigned short		uint16_t;
typedef short			int16_t;
typedef unsigned char		uint8_t;
typedef char			int8_t;

#else
    #include <inttypes.h>
#endif

#ifdef WITH_HDF5
    #include <hdf5.h>
#endif 
#ifdef WITH_NIFTI
    #include <nifti1.h>
#endif 


#ifdef __cplusplus
    #define EXTERN_C extern "C"
#else
    #define EXTERN_C
#endif

/* 
	Including ZLIB enables reading gzipped files (they are decompressed on-the-fly)  
	The output files can be zipped, too. 
 */

#ifdef WITH_ZLIB
    #ifdef __MINGW32__
	#include "win32/zlib/include/zlib.h"
    #else
	#include <zlib.h>
    #endif 
#endif 

#ifdef WITH_CHOLMOD
    #include <suitesparse/cholmod.h>
#endif
#ifdef WITH_GSL
    #include <gsl/gsl_matrix_double.h>
#endif
#include <stdio.h>
#include <time.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	biosig_data_type    data type of  internal data format 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef double biosig_data_type;


/****************************************************************************/
/**                                                                        **/
/**                CONSTANTS and Global variables                          **/
/**                                                                        **/
/****************************************************************************/


/* for error handling */ 
enum B4C_ERROR {
	B4C_NO_ERROR=0,
	B4C_FORMAT_UNKNOWN,
	B4C_FORMAT_UNSUPPORTED,
	B4C_CANNOT_OPEN_FILE,
	B4C_CANNOT_WRITE_FILE,
	B4C_INSUFFICIENT_MEMORY,
	B4C_ENDIAN_PROBLEM,
	B4C_CRC_ERROR,
	B4C_DATATYPE_UNSUPPORTED,
	B4C_SCLOSE_FAILED,
	B4C_DECOMPRESSION_FAILED,
	B4C_MEMORY_ALLOCATION_FAILED,
	B4C_RAWDATA_COLLAPSING_FAILED,
	B4C_REREF_FAILED,
	B4C_INCOMPLETE_FILE,
	B4C_UNSPECIFIC_ERROR,
};


	/* list of file formats */
enum FileFormat {
	noFile, unknown, 
	ABF, ACQ, ACR_NEMA, AIFC, AIFF, AINF, alpha, AU, ASF, ATES, ATF, AVI,
	BCI2000, BDF, BIN, BKR, BLSC, BMP, BNI, BSCS, 
	BrainVision, BrainVisionVAmp, BrainVisionMarker, BZ2, 
	CDF, CFWB, CNT, CTF, DICOM, DEMG, 
	EBS, EDF, EEG1100, EEProbe, EEProbe2, EEProbeAvr, EGI, EGIS, ELF, EMBLA, ET_MEG, ETG4000, EVENT, EXIF, 
	FAMOS, FEF, FITS, FLAC, GDF, GDF1,
	GIF, GTF, GZIP, HDF, HL7aECG, ISHNE, JPEG, Lexicor, 
	Matlab, MFER, MIDI, MIT, MM, MSI, 
	native, NetCDF, NEX1, NIFTI, OGG, OpenXDF,
	PBMA, PBMN, PDF, PDP, Persyst, PGMA, PGMB, PLEXON, PNG, PNM, POLY5, PPMA, PPMB, PS, 
	RIFF, SCP_ECG, SIGIF, Sigma, SMA, SND, SVG, SXI,    
	TIFF, TMS32, TMSiLOG, TRC, UNIPRO, VRML, VTK, 
	WAV, WinEEG, WMF, XML, XPM,
	Z, ZIP, ZIP2,
	ASCII_IBI, ASCII, 
};


EXTERN_C int   B4C_ERRNUM;
EXTERN_C const char *B4C_ERRMSG;

#define BIOSIG_VERSION_MAJOR 0
#define BIOSIG_VERSION_MINOR 94
#define BIOSIG_VERSION_STEPPING 0
#define BIOSIG_VERSION (BIOSIG_VERSION_MAJOR+0.01*BIOSIG_VERSION_MINOR)

EXTERN_C int   VERBOSE_LEVEL; 	// used for debugging
//#define VERBOSE_LEVEL 0	// turn off debugging information 


/****************************************************************************/
/**                                                                        **/
/**                 DEFINITIONS, TYPEDEFS AND MACROS                       **/
/**                                                                        **/
/****************************************************************************/



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	Conversion of time formats between Unix and GDF format.

	The default time format in BIOSIG uses a 64-bit fixed point format with 
	reference date 01-Jan-0000 00h00m00s (value=0). 
	One unit indicates the 2^(-32) part of 1 day (ca 20 us). Accordingly, 
	the higher 32 bits count the number of days, the lower 32 bits describe 
	the fraction of a day.  01-Jan-1970 is the day 719529. 

      	time_t t0; 
      	t0 = time(NULL); 
      	T0 = (double)t0/86400.0;	// convert seconds in days since 1970-Jan-01
      	floor(T0) + 719529;		// number of days since 01-Jan-0000
      	floor(ldexp(T0-floor(T0),32));  // fraction x/2^32; one day is 2^32 
      	
      	The following macros define the conversions between the unix time and the 
      	GDF format. 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef int64_t 		gdf_time; /* gdf time is represented in 64 bits */

#define t_time2gdf_time(t)	((gdf_time)floor(ldexp(((double)(t))/86400.0 + 719529.0, 32)))
#define	ntp_time2gdf_time(t)	((gdf_time)ldexp(ldexp(((double)(t)),-32)/86400 + 719529.0 - 70,32))
#define	gdf_time2ntp_time(t)	((int64_t)ldexp((ldexp(((double)(t)),-32) - 719529.0 + 70) * 86400,32))

#ifdef __cplusplus
EXTERN_C {
#endif 
gdf_time   tm_time2gdf_time(struct tm *t);
struct tm *gdf_time2tm_time(gdf_time t);
#ifdef __cplusplus
}
#endif 

typedef int64_t 		nrec_t;	/* type for number of records */

/****************************************************************************/
/**                                                                        **/
/**                     TYPEDEFS AND STRUCTURES                            **/
/**                                                                        **/
/****************************************************************************/

/*
	This structure defines the header for each channel (variable header) 
 */
#define MAX_LENGTH_LABEL 	40	// TMS: 40 
#define MAX_LENGTH_TRANSDUCER 	80
#define MAX_LENGTH_PHYSDIM 	20
#define MAX_LENGTH_PID	 	80      // length of Patient ID: MFER<65, GDF<67, EDF/BDF<81, etc. 
#define MAX_LENGTH_RID		80	// length of Recording ID: EDF,GDF,BDF<80, HL7 ?  	
#define MAX_LENGTH_NAME 	132	// max length of personal name: MFER<=128, EBS<=33*4
#define MAX_LENGTH_MANUF 	128	// max length of manufacturer field: MFER<128
#define MAX_LENGTH_TECHNICIAN 	128	// max length of manufacturer field: SCP<41 

#define ATT_ALI __attribute__ ((aligned (8)))	/* Matlab v7.3+ requires 8 byte alignment*/

typedef struct CHANNEL_STRUCT {
	double 		PhysMin ATT_ALI;	/* physical minimum */
	double 		PhysMax ATT_ALI;	/* physical maximum */
	double 		DigMin 	ATT_ALI;	/* digital minimum */
	double	 	DigMax 	ATT_ALI;	/* digital maximum */
	double		Cal 	ATT_ALI;	/* gain factor */ 
	double		Off 	ATT_ALI;	/* bias */ 

	char		OnOff	ATT_ALI; 		
	char		Label[MAX_LENGTH_LABEL+1] ATT_ALI; 	/* Label of channel */
	uint16_t	LeadIdCode ATT_ALI;	/* Lead identification code */ 
	char 		Transducer[MAX_LENGTH_TRANSDUCER+1] ATT_ALI;	/* transducer e.g. EEG: Ag-AgCl electrodes */
	char 		PhysDim[MAX_LENGTH_PHYSDIM+1] ATT_ALI  	__attribute__ ((deprecated));	/* physical dimension */
			/*PhysDim is now obsolete - use function PhysDim(PhysDimCode,PhysDimText) instead */
	uint16_t	PhysDimCode ATT_ALI;	/* code for physical dimension */
	/* char* 	PreFilt;	// pre-filtering */

	float 		LowPass		ATT_ALI;	/* lowpass filter */
	float 		HighPass	ATT_ALI;	/* high pass */
	float 		Notch		ATT_ALI;	/* notch filter */
	float 		XYZ[3]		ATT_ALI;	/* sensor position */
//	float 		Orientation[3]	__attribute__ ((deprecated));	// sensor direction
//	float 		Area		__attribute__ ((deprecated));	// area of sensor (e.g. for MEG)

        /* context specific channel information */
	float 		Impedance	ATT_ALI;   	/* Electrode Impedance in Ohm, defined only if PhysDim = _Volt */
	float 		fZ        	ATT_ALI;   	/* ICG probe frequency, defined only if PhysDim = _Ohm */
	/* end of context specific channel information */

	uint16_t 	GDFTYP 		ATT_ALI;	/* data type */
	uint32_t 	SPR 		ATT_ALI;	/* samples per record (block) */
	uint32_t	bi 		ATT_ALI __attribute__ ((deprecated));	/* start byte (byte index) of channel within data block */
	uint32_t	bi8 		ATT_ALI __attribute__ ((deprecated));	/* start bit  (bit index) of channel within data block */
	uint8_t*	bufptr		ATT_ALI;	/* pointer to buffer: NRec<=1 and bi,bi8 not used */
} CHANNEL_TYPE	ATT_ALI;


/*
	This structure defines the general (fixed) header  
*/
typedef struct {
	
	enum FileFormat TYPE 	ATT_ALI; 	/* type of file format */
	float 		VERSION ATT_ALI;	/* GDF version number */ 
	const char* 	FileName ATT_ALI;
	
	struct {
		size_t 			size[2] ATT_ALI; /* size {rows, columns} of data block	 */
		biosig_data_type* 	block ATT_ALI; 	 /* data block */
	} data ATT_ALI;

	uint32_t 	HeadLen ATT_ALI;	/* length of header in bytes */
	uint16_t 	NS 	ATT_ALI;	/* number of channels */
	uint32_t 	SPR 	ATT_ALI;	/* samples per block (when different sampling rates are used, this is the LCM(CHANNEL[..].SPR) */
	nrec_t  	NRec 	ATT_ALI;	/* number of records/blocks -1 indicates length is unknown. */	
	uint32_t 	Dur[2] 	ATT_ALI __attribute__ ((deprecated));	/* Duration of each block in seconds expressed in the fraction Dur[0]/Dur[1]  */
	double 		SampleRate ATT_ALI;	/* Sampling rate */
	uint8_t 	IPaddr[16] ATT_ALI; 	/* IP address of recording device (if applicable) */
	uint32_t  	LOC[4] 	ATT_ALI;	/* location of recording according to RFC1876 */
	gdf_time 	T0 	ATT_ALI; 	/* starttime of recording */
	int16_t 	tzmin 	ATT_ALI;	/* time zone (minutes of difference to UTC */

#ifdef CHOLMOD_H
	cholmod_sparse  *Calib ATT_ALI;                  /* re-referencing matrix */
#else 	
        void            *Calib ATT_ALI;                  /* re-referencing matrix */
#endif 	
	CHANNEL_TYPE 	*rerefCHANNEL ATT_ALI;  

	/* Patient specific information */
	struct {
		char		Name[MAX_LENGTH_NAME+1]; /* because for privacy protection it is by default not supported, support is turned on with FLAG.ANONYMOUS */
//		char*		Name;	// because for privacy protection it is by default not supported, support is turned on with FLAG.ANONYMOUS
		char		Id[MAX_LENGTH_PID+1];	/* patient identification, identification code as used in hospital  */
		uint8_t		Weight;		/* weight in kilograms [kg] 0:unkown, 255: overflow  */
		uint8_t		Height;		/* height in centimeter [cm] 0:unkown, 255: overflow  */
		//		BMI;		// the body-mass index = weight[kg]/height[m]^2
		gdf_time 	Birthday; 	/* Birthday of Patient */
		// 		Age;		// the age is HDR.T0 - HDR.Patient.Birthday, even if T0 and Birthday are not known
		uint16_t	Headsize[3]; 	/* circumference, nasion-inion, left-right mastoid in millimeter;  */
		/* Patient classification */
		int	 	Sex;		/* 0:Unknown, 1: Male, 2: Female */
		int		Handedness;	/* 0:Unknown, 1: Right, 2: Left, 3: Equal */
		int		Smoking;	/* 0:Unknown, 1: NO, 2: YES */
		int		AlcoholAbuse;	/* 0:Unknown, 1: NO, 2: YES */
		int		DrugAbuse;	/* 0:Unknown, 1: NO, 2: YES */
		int		Medication;	/* 0:Unknown, 1: NO, 2: YES */
		struct {
			int 	Visual;		/* 0:Unknown, 1: NO, 2: YES, 3: Corrected */
			int 	Heart;		/* 0:Unknown, 1: NO, 2: YES, 3: Pacemaker */
		} Impairment;
		
	} Patient ATT_ALI; 

	struct {
		char		Recording[MAX_LENGTH_RID+1]; 	/* HL7, EDF, GDF, BDF replaces HDR.AS.RID */
		char 		Technician[MAX_LENGTH_TECHNICIAN+1];
		char* 		Hospital;	/* recording institution */
		uint64_t 	Equipment; 	/* identifies this software */
		struct {
			/* see 
				SCP: section1, tag14, 
				MFER: tag23:  "Manufacturer^model^version number^serial number"
			*/	
			char	_field[MAX_LENGTH_MANUF+1];	/* buffer */
			const char*	Name;  
			const char*	Model;
			const char*	Version;
			const char*	SerialNumber;
		} Manufacturer;  
	} ID ATT_ALI;

	/* position of electrodes; see also HDR.CHANNEL[k].XYZ */
	struct {
		float		REF[3];	/* XYZ position of reference electrode */
		float		GND[3];	/* XYZ position of ground electrode */
	} ELEC ATT_ALI;

	/* EVENTTABLE */
	struct {
		double  	SampleRate ATT_ALI;	/* for converting POS and DUR into seconds  */
		uint32_t  	N ATT_ALI;	/* number of events */
		uint16_t 	*TYP ATT_ALI;	/* defined at http://cvs.sourceforge.net/viewcvs.py/biosig/biosig/t200/eventcodes.txt?view=markup */
		uint32_t 	*POS ATT_ALI;	/* starting position [in samples] using a 0-based indexing */
		uint32_t 	*DUR ATT_ALI;	/* duration [in samples] */
		uint16_t 	*CHN ATT_ALI;	/* channel number; 0: all channels  */
		char		**CodeDesc ATT_ALI;	/* describtion of "free text"/"user specific" events (encoded with TYP=0..255 */
		uint16_t	LenCodeDesc ATT_ALI;	/* length of CodeDesc Table */
	} EVENT ATT_ALI; 

	struct {	/* flags */
		char		OVERFLOWDETECTION; 	/* overflow & saturation detection 0: OFF, !=0 ON */
		char		UCAL; 		/* UnCalibration  0: scaling  !=0: NO scaling - raw data return  */
		char		ANONYMOUS; 	/* 1: anonymous mode, no personal names are processed */ 
		char		ROW_BASED_CHANNELS;     /* 0: column-based data [default]; 1: row-based data */
		char		TARGETSEGMENT; /* in multi-segment files (like Nihon-Khoden, EEG1100), it is used to select a segment */ 
	} FLAG ATT_ALI; 

	CHANNEL_TYPE 	*CHANNEL ATT_ALI;  
		// moving CHANNEL after the next struct (HDR.FILE) gives problems at AMD64 MEX-file. 
		// perhaps some alignment problem. 
	
	struct {	/* File specific data  */
#ifdef ZLIB_H
		gzFile		gzFID;
#endif
#ifdef _BZLIB_H
//		BZFILE*		bzFID;
#endif
		FILE* 		FID;		/* file handle  */
		size_t 		POS;		/* current reading/writing position [in blocks] */
		//size_t 	POS2;		// current reading/writing position [in samples] */
		int		Des;		/* file descriptor */
		uint8_t		OPEN; 		/* 0: closed, 1:read, 2: write */
		uint8_t		LittleEndian;   /* 1 if file is LittleEndian data format and 0 for big endian data format*/  
		uint8_t		COMPRESSION;    /* 0: no compression 9: best compression */
		int		DES;		/* descriptor for streams */
	} FILE ATT_ALI; 

	/*	internal variables (not public)  */
	struct {
//		char 		PID[MAX_LENGTH_PID+1];	// use HDR.Patient.Id instead
//		char* 		RID;		// recording identification
//		uint32_t 	spb __attribute__ ((deprecated)); // total samples per block
		uint32_t 	bpb;  		/* total bytes per block */
		uint32_t 	bpb8;  		/* total bits per block */

		uint8_t*	Header; 
		uint8_t*	rawEventData;
		uint8_t*	rawdata; 	/* raw data block */
		char		flag_collapsed_rawdata; /*0 if rawdata contain obsolete channels, too. 	*/
		nrec_t		first;		/* first block loaded in buffer - this is equivalent to hdr->FILE.POS */
		nrec_t		length;		/* number of block(s) loaded in buffer */
		uint8_t*	auxBUF;  	/* auxillary buffer - used for storing EVENT.CodeDesc, MIT FMT infor, alpha:rawdata header */
		char*		bci2000;
	} AS ATT_ALI;
	
	void *aECG;
	
} HDRTYPE;


/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/

#ifdef __cplusplus
EXTERN_C {
#endif 

HDRTYPE* constructHDR(const unsigned NS, const unsigned N_EVENT);
/* 	allocates memory initializes header HDR of type HDRTYPE 
	with NS channels an N_EVENT event elements
 --------------------------------------------------------------- */


void 	 destructHDR(HDRTYPE* hdr);
/* 	destroys the header *hdr and frees allocated memory
 --------------------------------------------------------------- */

HDRTYPE* getfiletype(HDRTYPE* hdr);
/* 	identify file format from header information 
	input:
		hdr->AS.Header contains header of hdr->HeadLen bytes 
		hdr->TYPE must	CHANNEL_TYPE 	*CHANNEL ATT_ALI;  
 be unknown, otherwise no FileFormat evaluation is performed
	output:
		hdr->TYPE	file format
		hdr->VERSION	is defined for some selected formats e.g. ACQ, EDF, BDF, GDF
 --------------------------------------------------------------- */

HDRTYPE* sopen(const char* FileName, const char* MODE, HDRTYPE* hdr);
/*	FileName: name of file 
	Mode: "r" is reading mode, requires FileName
	Mode: "w" is writing mode, hdr contains the header information
		If the number of records is not known, set hdr->NRec=-1 and 
		sclose will fill in the correct number. 
	hdr should be generated with constructHDR, and the necessary fields 
	must be defined. In read-mode, hdr can be NULL; however, 
	hdr->FLAG... can be used to turn off spurious warnings. In write-mode, 
	the whole header information must be defined.    
	After calling sopen, the file header is read or written, and 
	the position pointer points to the beginning of the data section
 --------------------------------------------------------------- */

int 	sclose(HDRTYPE* hdr);
/* 	closes the file corresponding to hdr
	file handles are closed, the position pointer becomes meaningless
	Note: hdr is not destroyed; use destructHDR() to free the memory of hdr
	if hdr was opened in writing mode, the event table is added to the file 
	and if hdr->NRec=-1, the number of records is obtained from the 
	    position pointer and written into the header,
 --------------------------------------------------------------- */

size_t	sread(biosig_data_type* DATA, size_t START, size_t LEN, HDRTYPE* hdr);
/*	LEN data segments are read from file associated with hdr, starting from 
	segment START. The data is copied into DATA; if DATA == NULL, a 
	sufficient amount of memory is allocated, and the pointer to the data 
	is available in hdr->data.block. 

	In total, LEN*hdr->SPR*NS samples are read and stored in 
	data type of biosig_data_type (currently double). 
	NS is the number of channels with non-zero hdr->CHANNEL[].OnOff. 
	The number of successfully read data blocks is returned.

 	A pointer to the data block is also available from hdr->data.block, 
	the number of columns and rows is available from 
	hdr->data.size[0] and hdr->data.size[1] respectively. 
 
	The following flags will influence the result.
 	hdr->FLAG.UCAL = 0 	scales the data to its physical values
 	hdr->FLAG.UCAL = 1 	does not apply the scaling factors

 	hdr->FLAG.OVERFLOWDETECTION = 0 does not apply overflow detection 
 	hdr->FLAG.OVERFLOWDETECTION = 1: replaces all values that exceed 
		the dynamic range (defined by Phys/Dig/Min/Max)

	hdr->FLAG.ROW_BASED_CHANNELS = 0 each channel is in one column 	
	hdr->FLAG.ROW_BASED_CHANNELS = 1 each channel is in one row 	
 --------------------------------------------------------------- */

#ifdef __GSL_MATRIX_DOUBLE_H__
size_t	gsl_sread(gsl_matrix* DATA, size_t START, size_t LEN, HDRTYPE* hdr);
/*	same as sread but return data is of type gsl_matrix
 --------------------------------------------------------------- */
#endif 

size_t  swrite(const biosig_data_type *DATA, size_t NELEM, HDRTYPE* hdr);
/*	DATA contains the next NELEM data segment(s) for writing. 
 *	hdr contains the definition of the header information and was generated by sopen 
 *	the number of successfully written segments is returned;	
 --------------------------------------------------------------- */


int	seof(HDRTYPE* hdr);
/*	returns 1 if end of file is reached. 
 --------------------------------------------------------------- */


void	srewind(HDRTYPE* hdr);
/*	postions file pointer to the beginning
 *
 *	Currently, this function is meaning less because sread requires always the start value
 --------------------------------------------------------------- */


int 	sseek(HDRTYPE* hdr, long int offset, int whence);
/*	positions file pointer 
 *
 *	Currently, this function is meaning less because sread requires always the start value
 --------------------------------------------------------------- */


long int stell(HDRTYPE* hdr);
/*	returns position of file point in segments
 --------------------------------------------------------------- */


int 	serror();
/*	handles errors; it reports whether an error has occured. 
 *	if yes, an error message is displayed, and the error status is reset. 
 * 	the return value is 0 if no error has occured, otherwise the error code 
 *	is returned.
 --------------------------------------------------------------- */


int 	sflush_gdf_event_table(HDRTYPE* hdr);
/*	writes the event table of file hdr. hdr must define a file in GDF format  
 *  	and can be opened as read or write. 
 *	In case of success, the return value is 0. 
 --------------------------------------------------------------- */

int 	cachingWholeFile(HDRTYPE* hdr);
/*	caching: load data of whole file into buffer                      
 *		 this will speed up data access, especially in interactive mode 
 --------------------------------------------------------------- */


int	hdr2ascii(HDRTYPE* hdr,FILE *fid, int VERBOSITY);
/*	writes the header information is ascii format the stream defined by fid 
 *	Typically fid is stdout. VERBOSITY defines how detailed the information is. 
 *	VERBOSITY=0 or 1 report just some basic information,
 *	VERBOSITY=2 reports als the channel information 
 *	VERBOSITY=3 provides in addition the event table. 
 --------------------------------------------------------------- */

int RerefCHANNEL(HDRTYPE *hdr, void *ReRef, char rrtype);
/* rerefCHAN 
        defines rereferencing of channels, 
        hdr->Calib defines the rereferencing matrix 
        hdr->rerefCHANNEL is defined. 
        hdr->rerefCHANNEL[.].Label is  by some heuristics from hdr->CHANNEL
                either the maximum scaling factor  
        if ReRef is NULL, rereferencing is turned off (hdr->Calib and 
        hdr->rerefCHANNEL are reset to NULL). 
        if rrtype==1, Reref is a filename pointing to a MatrixMarket file 
        if rrtype==2, Reref must be a pointer to a cholmod sparse matrix (cholmod_sparse*)
        In case of an error (mismatch of dimensions), a non-zero is returned,
        and serror() is set.     

        rr is a pointer to a rereferencing matrix
        rrtype determines the type of pointer 
        rrtype=0: no rereferencing, RR is ignored (NULL) 
               1: pointer to MarketMatrix file (char*)
               2: pointer to a sparse cholmod matrix  (cholmod_sparse*)
 ------------------------------------------------------------------------*/

const char* GetFileTypeString(enum FileFormat FMT);
/*	returns a string with file format
 --------------------------------------------------------------- */


HDRTYPE* sload(const char* FileName, size_t CHANLIST[], biosig_data_type** DATA);
/*	FileName: name of file 
	returns a HDR struct containing the header information 
	CHANLIST[0] contains the length of the channel list 
	CHANLIST[1..CHANLIST[0]] contains the selected channels 
	and DATA points to a matrix with HDR.SPR*HDR.NRec samples and HDR.NS channels
 --------------------------------------------------------------- */


uint16_t PhysDimCode(char* PhysDim0);
/* Encodes  Physical Dimension as 16bit integer according to 
   ISO/IEEE 11073-10101:2004 Vital Signs Units of Measurement
 --------------------------------------------------------------- */

char* PhysDim(uint16_t PhysDimCode, char *PhysDimText);
/* converts HDR.CHANNEL[k].PhysDimCode into a readable Physical Dimension
   the memory for PhysDim must be preallocated, its maximum length is 
   defined by (MAX_LENGTH_PHYSDIM+1)  
 --------------------------------------------------------------- */
double PhysDimScale(uint16_t PhysDimCode);
/* returns scaling factor of physical dimension 
	e.g. 0.001 for milli, 1000 for kilo etc. 
 --------------------------------------------------------------- */


void sort_eventtable(HDRTYPE *hdr);
/* sort event table with respect to hdr->EVENT.POS    
  --------------------------------------------------------------*/

void convert2to4_eventtable(HDRTYPE *hdr);
/* converts event table from {TYP,POS} to [TYP,POS,CHN,DUR} format   
  -------------------------------------------------------------- */

void convert4to2_eventtable(HDRTYPE *hdr);
/* converts event table from [TYP,POS,CHN,DUR} to {TYP,POS} format 
	all CHN[k] must be 0
  -------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif 


/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/

#endif	/* __BIOSIG_EXT_H__ */
