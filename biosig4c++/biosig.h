/*
%
% $Id: biosig.h,v 1.8 2005-09-21 08:08:23 schloegl Exp $
% Copyright (C) 2000,2005 Alois Schloegl <a.schloegl@ieee.org>
% This file is part of the "BioSig for C/C++" repository 
% (biosig4c++) at http://biosig.sf.net/ 
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
/**                 DEFINITIONS, TYPEDEFS AND MACROS                       **/
/**                                                                        **/
/****************************************************************************/

#include <inttypes.h>

#ifndef BIOSIG_H
#define BIOSIG_H

	// list of file formats 
enum FileFormat {ACQ, BKR, BDF, CNT, DEMG, EDF, EVENT, FLAC, GDF, MFER, NEX1, PLEXON}; 

enum HANDEDNESS {Unknown=0, Right=1, Left=2, Equal=3}; 
enum GENDER  	{male=1,  female=2};
enum SCALE 	{No=1,    Yes=2,	Corrected=3};

const int GDFTYP_BYTE[] = {1, 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 0, 0, 0, 0, 0, 4, 8, 16};

#define min(a,b)                        (((a) < (b)) ? (a) : (b))
#define max(a,b)                        (((a) > (b)) ? (a) : (b))


/*
	Conversion of time formats between Unix and GDF format.

	The default time format in BIOSIG uses a 64-bit fixed point format with 
	reference date 01-Jan-0000 00h00m00s (value=0). 
	One unit indicates the 2^(-32) part of 1 day (ca 20 us). Accordingly, 
	the higher 32 bits count the number of days, the lower 32 bits describe 
	the fraction of a day.  01-Jan-1970 is the day 719529. 

      	time_t t0; 
      	t0 = time(NULL); 
      	T0 = (double)t0/86400.0;	// convert seconds it days since 1970-Jan-01
      	floor(T0) + 719529;		// number of days since 01-Jan-0000
      	floor(ldexp(T0-floor(T0),32));  // fraction x/2^32; one day is 2^32 
      	
      	The following macros define the conversions between the unix time and the 
      	GDF format. 
*/
typedef int64_t 		time_gdf; // gdf time is represented in 64 bits
#define time_t2time_gdf(t)	((time_gdf)floor(ldexp((t)/86400.0 + 719529, 32)))
#define time_gdf2time_t(t)	((time_t)((ldexp((t),-32) - 719529) * 86400))


/****************************************************************************/
/**                                                                        **/
/**                     TYPEDEFS AND STRUCTURES                            **/
/**                                                                        **/
/****************************************************************************/

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
	int64_t 	DigMin;		// digital minimum
	int64_t 	DigMax;		// digital maximum

	uint16_t 	GDFTYP;		// data type
	uint32_t 	SPR;		// samples per record (block)
} CHANNEL_TYPE;


/*
	This structure defines the general (fixed) header  
*/
typedef struct {
	enum FileFormat TYPE; 	// type of file format
	float 	VERSION;	// GDF version number 
	char* 	FileName;
	void*  	buffer; 	// data returned by sread

	uint32_t 	NS;		// number of channels
	uint32_t 	SampleRate;	// Sampling rate
	uint32_t 	SPR;		// samples per block (when different sampling rates are used, this is the LCM(CHANNEL[..].SPR)
	uint8_t 	IPaddr[6]; 	// IP address of recording device (if applicable)	
	uint32_t  LOC[4];		// location of recording according to RFC1876
	time_gdf T0; 		// starttime of recording
	uint32_t 	HeadLen;	// length of header in bytes
	int32_t  	NRec;		// number of records/blocks -1 indicates length is unknown.	
	uint32_t 	Dur[2];	// Duration of each block in seconds expressed in the fraction Dur[0]/Dur[1] 

	// Patient specific information 
	struct {
		char*		Name;		// not recommended because of privacy protection 
		char*		Id;		// identification code as used in hospital 
		uint8_t		Weight;		// weight in kilograms [kg] 0:unkown, 255: overflow 
		uint8_t		Height;		// height in centimeter [cm] 0:unkown, 255: overflow 
		time_gdf 	Birthday; 	// Birthday of Patient
		uint16_t	Headsize[3]; 	// circumference, nasion-inion, left-right mastoid in millimeter; 
		enum GENDER 	Sex; 	
		enum HANDEDNESS Handedness;	
		// Patient classification // 
		enum SCALE 	Smoking;
		enum SCALE 	AlcoholAbuse;
		enum SCALE 	DrugAbuse;
		enum SCALE 	Medication;
		struct {
			enum SCALE Visual;
		} Impairment;
	} Patient; 
	struct {
		char* Technician; 	
		char* Hospital; 	
		uint64_t Equipment; 	// identfies this software
	} ID; 

	// position of electrodes; see also HDR.CHANNEL[k].XYZ
	struct {
		float	REF[3];	// XYZ position of reference electrode
		float	GND[3];	// XYZ position of ground electrode
	} ELEC;

	//	EVENTTABLE 
	struct {
		uint32_t 	SampleRate;	// for converting POS and DUR into seconds 
		uint32_t 	N;	// number of events
		uint16_t *TYP;	// defined at http://cvs.sourceforge.net/viewcvs.py/biosig/biosig/t200/eventcodes.txt?view=markup
		uint32_t *POS;	// starting position [in samples]
		uint32_t *DUR;	// duration [in samples]
		uint16_t *CHN;	// channel number; 0: all channels 
	} EVENT; 

	struct {	// File specific data 
		FILE* 		FID;		// file handle 
		uint32_t 	POS;		// current reading/writing position in samples 
		uint8_t		OPEN; 		// 0: closed, 1:read, 2: write
		uint8_t		LittleEndian; 	// 
	} FILE; 

	//	internal variables (not public) 
	struct {
		char 	PID[81];	// patient identification
		char* 	RID;		// recording identification 
		uint32_t 	spb;		// total samples per block
		uint32_t 	bpb;  		// total bytes per block
		uint32_t 	*bi;
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
size_t 	sread(HDRTYPE* hdr, size_t nelem);
size_t	swrite(const void *ptr, size_t nelem, HDRTYPE* hdr);
int	seof(HDRTYPE HDR);
int	srewind(HDRTYPE* hdr);
int 	sseek(HDRTYPE* hdr, uint32_t offset, int whence);
int32_t   stell(HDRTYPE HDR);


/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/

#endif	/* BIOSIG_H */
