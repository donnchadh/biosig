/*
---------------------------------------------------------------------------
Copyright (C) 2005-2006  Franco Chiarugi
Developed at the Foundation for Research and Technology - Hellas, Heraklion, Crete

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
---------------------------------------------------------------------------
*/
//////////////////////////////////////////////////////////////////////
//
// XMLECG_Parser.h: interface for the cXML_Parser class.
// ------- Draft, just an example -------
//
//////////////////////////////////////////////////////////////////////

#if !defined XMLECG_PARSER
#define XMLECG_PARSER

#ifndef VS_DEF
#include "biosig.h"
#else
#include "biosig_vs.h"
#endif

// SCP Lead definition
#define SCP_LT_I			1
#define SCP_LT_II			2
#define SCP_LT_V1			3
#define SCP_LT_V2			4
#define SCP_LT_V3			5
#define SCP_LT_V4			6
#define SCP_LT_V5			7
#define SCP_LT_V6			8
#define SCP_LT_V7			9
#define SCP_LT_V2R			10
#define SCP_LT_V3R			11
#define SCP_LT_V4R			12
#define SCP_LT_V5R			13
#define SCP_LT_V6R			14
#define SCP_LT_V7R			15
#define SCP_LT_X			16
#define SCP_LT_Y			17
#define SCP_LT_Z			18
#define SCP_LT_CC5			19
#define SCP_LT_CM5			20
#define SCP_LT_LA			21
#define SCP_LT_RA			22
#define SCP_LT_LL			23
#define SCP_LT_I1			24
#define SCP_LT_E			25
#define SCP_LT_C			26
#define SCP_LT_A			27
#define SCP_LT_M			28
#define SCP_LT_F			29
#define SCP_LT_H			30
#define SCP_LT_I_cal		31
#define SCP_LT_II_cal		32
#define SCP_LT_V1_cal		33
#define SCP_LT_V2_cal		34
#define SCP_LT_V3_cal		35
#define SCP_LT_V4_cal		36
#define SCP_LT_V5_cal		37
#define SCP_LT_V6_cal		38
#define SCP_LT_V7_cal		39
#define SCP_LT_V2R_cal		40
#define SCP_LT_V3R_cal		41
#define SCP_LT_V4R_cal		42
#define SCP_LT_V5R_cal		43
#define SCP_LT_V6R_cal		44
#define SCP_LT_V7R_cal		45
#define SCP_LT_X_cal		46
#define SCP_LT_Y_cal		47
#define SCP_LT_Z_cal		48
#define SCP_LT_CC5_cal		49
#define SCP_LT_CM5_cal		50
#define SCP_LT_LA_cal		51
#define SCP_LT_RA_cal		52
#define SCP_LT_LL_cal		53
#define SCP_LT_I1_cal		54
#define SCP_LT_E_cal		55
#define SCP_LT_C_cal		56
#define SCP_LT_A_cal		57
#define SCP_LT_M_cal		58
#define SCP_LT_F_cal		59
#define SCP_LT_H_cal		60
#define SCP_LT_III			61
#define SCP_LT_aVR			62
#define SCP_LT_aVL			63
#define SCP_LT_aVF			64
#define SCP_LT_maVR			65
#define SCP_LT_V8			66
#define SCP_LT_V9			67
#define SCP_LT_V8R			68
#define SCP_LT_V9R			69
#define SCP_LT_NehbD		70
#define SCP_LT_NehbA		71
#define SCP_LT_NehbJ		72
#define SCP_LT_DL			73
#define SCP_LT_EPL			74
#define SCP_LT_A1			75
#define SCP_LT_A2			76
#define SCP_LT_A3			77
#define SCP_LT_A4			78
#define SCP_LT_V8_cal		79
#define SCP_LT_V9_cal		80
#define SCP_LT_V8R_cal		81
#define SCP_LT_V9R_cal		82
#define SCP_LT_NehbD_cal	83
#define SCP_LT_NehbA_cal	84
#define SCP_LT_NehbJ_cal	85

// XML aECG lead codes (10102) (calculated leads are not defined in XML aECG)
#define XML_CONFIG			0
#define XML_I				1
#define XML_II				2
#define XML_V1				3
#define XML_V2				4
#define XML_V3				5
#define XML_V4				6
#define XML_V5				7
#define XML_V6				8
#define XML_V7				9
#define XML_V2R				10
#define XML_V3R				11
#define XML_V4R				12
#define XML_V5R				13
#define XML_V6R				14
#define XML_V7R				15
#define XML_X				16
#define XML_Y				17
#define XML_Z				18
#define XML_CC5				19
#define XML_CM5				20
#define XML_LA				21
#define XML_RA				22
#define XML_LL				23
#define XML_fI				24
#define XML_fE				25
#define XML_fC				26
#define XML_fA				27
#define XML_fM				28
#define XML_fF				29
#define XML_fH				30
#define XML_III				61
#define XML_AVR				62
#define XML_AVL				63
#define XML_AVF				64
#define XML_AVRneg			65
#define XML_V8				66
#define XML_V9				67
#define XML_V8R				68
#define XML_V9R				69
#define XML_D				70
#define XML_A				71
#define XML_J				72
#define XML_DEFIB			73
#define XML_EXTERN			74
#define XML_A1				75
#define XML_A2				76
#define XML_A3				77
#define XML_A4				78
// From here on the codes are not defined till SCP 2.1, but only in the amendment 2.2
#define XML_C				86
#define XML_V				87
#define XML_VR				88
#define XML_VL				89
#define XML_VF				90
#define XML_MCL				91
#define XML_MCL1			92
#define XML_MCL2			93
#define XML_MCL3			94
#define XML_MCL4			95
#define XML_MCL5			96
#define XML_MCL6			97
#define XML_CC				98
#define XML_CC1				99
#define XML_CC2				100
#define XML_CC3				101
#define XML_CC4				102
#define XML_CC6				103
#define XML_CC7				104
#define XML_CM				105
#define XML_CM1				106
#define XML_CM2				107
#define XML_CM3				108
#define XML_CM4				109
#define XML_CM6				110
#define XML_CM7				121
#define XML_CH5				122
#define XML_CS5				123
#define XML_CB5				124
#define XML_CR5				125
#define XML_ML				126
#define XML_AB1				127
#define XML_AB2				128
#define XML_AB3				129
#define XML_AB4				130
#define XML_ES				131
#define XML_AS				132
#define XML_AI				133
#define XML_S				134
#define XML_RL				147

#define MAXLEADS			12		    /* Max number of lead in the measurements */
#define MAX_COMPLEXES		64          /* Max number of complexes */

typedef struct {
    int16_t RefBeat_fidptr;	// in sample points (from the beginning of the ref. beat)
	int16_t Num_Cpxs;			// #
    int16_t QRS_type[MAX_COMPLEXES];
    int32_t QRS_start_refbeat_sub[MAX_COMPLEXES]; // in sample points (from the beginning of the rhythm)
    int32_t QRS_fidptr[MAX_COMPLEXES];			// in sample points (from the beginning of the rhythm)
    int32_t QRS_end_refbeat_sub[MAX_COMPLEXES];	// in sample points (from the beginning of the rhythm)
	int16_t P_onset;			// in msec (from the beginning of the ref. beat)
	int16_t P_end;			// in msec (from the beginning of the ref. beat)
	int16_t QRS_onset;		// in msec (from the beginning of the ref. beat)
	int16_t QRS_end;			// in msec (from the beginning of the ref. beat)
	int16_t T_end;			// in msec (from the beginning of the ref. beat)
	int16_t Pspikes_nr;		// #
	int16_t HR;				// in bpm
	int16_t RR_median;		// in msec
	int16_t PQ_intv;			// in msec
	int16_t QT_intv;			// in msec
	int16_t QTc_intv;			// in msec
	int16_t P_dur;			// in msec
	int16_t QRS_dur;			// in msec
	int16_t T_dur;			// in msec
	int16_t P_axis;			// in degree
	int16_t QRS_axis;			// in degree
	int16_t STT_axis;			// in degree
} GLOBMEAS;

typedef struct {
	int16_t P_dur;
	int16_t PR_int;
	int16_t QRS_dur;
	int16_t QT_int;
	int16_t Q_dur;
	int16_t R_dur;
	int16_t S_dur;
	int16_t R1_dur;
	int16_t S1_dur;
	int16_t Q_amp;
	int16_t R_amp;
	int16_t S_amp;
	int16_t R1_amp;
	int16_t S1_amp;
	int16_t J_amp;
	int16_t pp_amp;
	int16_t pm_amp;
	int16_t tp_amp;
	int16_t tm_amp;
	int16_t ST_slope;
	int16_t P_morpho;
	int16_t T_morpho;
	int16_t I_segment;
	int16_t K_segment;
	int16_t intr_defl;
	int16_t quality_code;
	int16_t ST_amp_J20;
	int16_t ST_amp_J60;
	int16_t ST_amp_J80;
	int16_t ST_amp_JRRdiv16;
	int16_t ST_amp_JRRdiv8;
} LEADMEAS;

class cXML_Parser
{
public:
	cXML_Parser();
	virtual ~cXML_Parser(void);
	int16_t Initialize();
	int16_t Close();
	int16_t ParseECGInfo(int8_t*);

	HDRTYPE XMLaECGParsedData;
};

#endif
