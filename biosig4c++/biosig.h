/*
%
% $Id: biosig.h,v 1.4 2005-09-09 17:18:34 schloegl Exp $
% Author: Alois Schloegl <a.schloegl@ieee.org>
% Copyright (C) 2000,2005 A.Schloegl
% 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

/* 

	Headers for the BioSig4C library
	
	Currently, the GDF-Header is defined. 
   
*/

/****************************************************************************/
/**                                                                        **/
/**                     DEFINITIONS AND MACROS                             **/
/**                                                                        **/
/****************************************************************************/

enum FileFormat {unknown, BKR, BDF, CNT, DEMG, EDF, FLAC, GDF, MFER, NEX1, PLEXON}; // list of file formats 
const int GDFTYP_BYTE[] = {1, 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 0, 0, 0, 0, 0, 4, 8, 16};

#define min(a,b)                        (((a) < (b)) ? (a) : (b))
#define max(a,b)                        (((a) > (b)) ? (a) : (b))
#define convert_timet2timegdf(t)	((uint64)floor(ldexp((t)/86400.0 + 719529,32)))
#define convert_timegdf2timet(t)	((time_t) ( (ldexp((t),-32) - 719529) * 86400)))

/****************************************************************************/
/**                                                                        **/
/**                     TYPEDEFS AND STRUCTURES                            **/
/**                                                                        **/
/****************************************************************************/

typedef char 			int8; 
typedef unsigned char 		uint8; 
typedef short 			int16; 
typedef unsigned short 		uint16; 
typedef long 			int32; 
typedef unsigned long 		uint32;
typedef long long 		int64; 
typedef unsigned long long 	uint64; 


/*
	This structure defines the header for each channel (variable header) 
*/
typedef struct {
	char* 	Label;		// Label of channel 
	char* 	Transducer;	// transducer e.g. EEG: Ag-AgCl electrodes
	char* 	PhysDim;	// physical dimension
	//char* PreFilt;	// pre-filtering

	float 	LowPass;	// lowpass filter
	float 	HighPass;	// high pass
	float 	Notch;		// notch filter
	float 	XYZ[3];		// electrode position
	float 	Impedance;   	// in Ohm
	
	double 	PhysMin;	// physical minimum
	double 	PhysMax;	// physical maximum
	int64 	DigMin;		// digital minimum
	int64 	DigMax;		// digital maximum

	uint16 	GDFTYP;		// data type
	uint32 	SPR;		// samples per record (block)
} CHANNEL_TYPE ;


typedef struct {
	enum FileFormat TYPE; 	// type of file format
	float 	VERSION;	// GDF version number 
	char* 	FileName;
	char* 	PID;			// patient identification
	char* 	RID;			// recording identification 

	uint32 	NS;		// number of channels
	uint32 	SampleRate;	// Sampling rate
	uint32 	SPR;		// samples per block (when different sampling rates are used, this is the LCM(CHANNEL[..].SPR)
	uint8 	IPaddr[6]; 	// IP address of recording device (if applicable)	
	uint16 	Headsize[3]; 	// circumference, nasion-inion, left-right mastoid in millimeter; 
	uint32  LOC[4];		// location of recording according to RFC1876
	uint64 	T0; 		// recording time
		// T0[0]*2^32/(24*3600) are the seconds of the day since midnight
		// T0[1] contains the number of days since 01-Jan-0000; 01-Jan-1970 is day 719529
	uint32 	HeadLen;	// length of header in bytes
	int32  	NRec;		// number of records/blocks -1 indicates length is unknown.	
	uint32 	Dur[2];	// Duration of each block in seconds expressed in the fraction Dur[0]/Dur[1] 

	// position of electrodes; see also HDR.CHANNEL[k].XYZ
	struct {
		float	REF[3];	// XYZ position of reference electrode
		float	GND[3];	// XYZ position of ground electrode
	} ELEC;
	
	// Patient specific information 
	struct {
		char Smoking;		// 0: unknown, 1: no, 2: yes
		char AlcoholAbuse;	// 0: unknown, 1: no, 2: yes
		char DrugAbuse;		// 0: unknown, 1: no, 2: yes
		char Medication;	// 0: unknown, 1: no, 2: yes
		uint8 Height;		// height in centimeter [cm]
		uint8 Weight;		// weight in kilogramms [kg]
		char Gender; 		// 0: unknown, 1: male, 2: female
		char Handedness;	// 0: unknown, 1: right, 2: left, 3: both equal
		struct {
			char Visual; // 0: unknown, 1: no, 2: yes, 3: yes and corrected
		} Impairment;
		//char VisualImpairment;  // 0: unknown, 1: no, 2: yes, 3: yes and corrected
		uint64 Birthday; 	// Birthday of Patient	// Birthday uses the same format as T0.
	} Patient; 

	//	EVENTTABLE 
	struct {
		uint32 	SampleRate;
		uint32 	N;
		uint16 *TYP;
		uint32 *POS;
		uint32 *DUR;
		uint16 *CHN;
	} EVENT; 

	struct {	// File specific data 
		FILE* 	FID;		// file handle 
		uint32 	POS;		// current reading/writing position in samples 
		uint8 	OPEN; 		// 0: closed, 1:read, 2: write
	} FILE; 

	//	internal variables (not public) 
	struct {
		uint32 spb;	// total samples per block
		uint32 bpb;  // total bytes per block
		uint32 *bi;
	} AS; 	
	CHANNEL_TYPE *CHANNEL;  
} HDRTYPE;


/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/


HDRTYPE init_default_hdr(HDRTYPE HDR, const unsigned NS, const unsigned N_EVENT);
HDRTYPE sopen(const char* FileName, HDRTYPE HDR, const char* MODE);
HDRTYPE sclose(HDRTYPE HDR);
//HDRTYPE swrite(const void *ptr, size_t size, size_t nelem, HDRTYPE HDR);
//HDRTYPE sread (const void *ptr, size_t size, size_t nelem, HDRTYPE HDR);


/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/
