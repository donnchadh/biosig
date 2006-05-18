/*
%
% $Id: biosig.h,v 1.32 2006-05-18 07:54:57 schloegl Exp $
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



#ifndef __BIOSIG_H__
#define __BIOSIG_H__

#ifdef VS_DEF
#include <math.h>
#include <time.h>
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
#include <byteswap.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
//#include <libxml/tree.h>

#endif


	// list of file formats 
enum FileFormat {unknown, ACQ, BKR, BDF, CFWB, CNT, DEMG, EDF, EVENT, FLAC, GDF, MFER, NEX1, PLEXON, SCP_ECG, HL7aECG, XML}; 


#define NaN (0.0/0.0)	// used for encoding of missing values 
#define INF (1.0/0.0)	// positive infinity

#define min(a,b)	(((a) < (b)) ? (a) : (b))
#define max(a,b)	(((a) > (b)) ? (a) : (b))


#if __BYTE_ORDER == __BIG_ENDIAN

#define l_endian_u16(x) bswap_16((uint16_t)(x))
#define l_endian_u32(x) bswap_32((uint32_t)(x))
#define l_endian_u64(x) bswap_64((uint64_t)(x))

#define l_endian_i16(x) bswap_16((int16_t)(x))
#define l_endian_i32(x) bswap_32((int32_t)(x))
#define l_endian_i64(x) bswap_64((int64_t)(x))

double  l_endian_f32(double x); 
double  l_endian_f64(double x); 

#elif __BYTE_ORDER == __LITTLE_ENDIAN

#define l_endian_u16(x) (x)
#define l_endian_u32(x) (x)
#define l_endian_u64(x) (x)

#define l_endian_i16(x) (x)
#define l_endian_i32(x) (x)
#define l_endian_i64(x) (x)
	
#define l_endian_f32(x) (x)
#define l_endian_f64(x) (x)

#endif 


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	biosig_data_type    data type of  internal data format 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef double	 		biosig_data_type;


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
typedef int64_t 		gdf_time; // gdf time is represented in 64 bits
#define t_time2gdf_time(t)	((gdf_time)floor(ldexp(((double)(t))/86400.0 + 719529.0, 32)))
#define gdf_time2t_time(t)	((time_t)((ldexp(((double)(t)),-32) - 719529) * 86400))
#define tm_time2gdf_time(t) 	t_time2gdf_time(mktime(t))
//#define gdf_time2tm_time(t)	localtime(gdf_time2t_time(t))
#define	ntp_time2gdf_time(t)	((gdf_time)ldexp(ldexp(((double)(t)),-32)/86400 + 719529.0 - 70,32))
#define	gdf_time2ntp_time(t)	((int64_t)ldexp((ldexp(((double)(t)),-32) - 719529.0 + 70) * 86400,32))


/****************************************************************************/
/**                                                                        **/
/**                     TYPEDEFS AND STRUCTURES                            **/
/**                                                                        **/
/****************************************************************************/

/*
	This structure defines the header for each channel (variable header) 
 */
typedef struct {
	char		OnOff; 		// 
	char* 		Label;		// Label of channel 
	uint16_t	LeadIdCode;	// Lead identification code 
	char* 		Transducer;	// transducer e.g. EEG: Ag-AgCl electrodes
	char* 		PhysDim;	// physical dimension
	uint16_t	PhysDimCode;	// code for physical dimension
	//char* 	PreFilt;	// pre-filtering

	float 		LowPass;	// lowpass filter
	float 		HighPass;	// high pass
	float 		Notch;		// notch filter
	float 		XYZ[3];		// electrode position
	float 		Impedance;   	// in Ohm
	
	double 		PhysMin;	// physical minimum
	double 		PhysMax;	// physical maximum
	double 		DigMin;		// digital minimum
	double	 	DigMax;		// digital maximum

	uint16_t 	GDFTYP;		// data type
	uint32_t 	SPR;		// samples per record (block)
	
	double		Cal;		// gain factor 
	double		Off;		// bias 
} CHANNEL_TYPE;


/*
	This structure defines the fields used for "Annotated ECG" 
 */
typedef struct {
	char*		test;		// test field for annotated ECG
	
	// Eugenio suggested to include these fields 
	float		diastolicBloodPressure;				
	float		systolicBloodPressure;	
	char*		MedicationDrugs;
	char*		ReferringPhysician;
	char*		Diagnosis;

	float		HeartRate;	
	float		P_wave[2]; 	// start and end 
	float		QRS_wave[2]; 	// start and end 
	float		T_wave[2]; 	// start and end 
	float		P_QRS_T_axes[3];
} aECG_TYPE;

/*
	This structure defines the general (fixed) header  
*/
typedef struct {
	enum FileFormat TYPE; 		// type of file format
	float 		VERSION;	// GDF version number 
	const char* 	FileName;
	
	struct {
		size_t 			size[2]; // size {rows, columns} of data block	
		biosig_data_type* 	block; 	 // data block
	} data;

	uint32_t 	HeadLen;	// length of header in bytes
	uint16_t 	NS;		// number of channels
	uint32_t 	SPR;		// samples per block (when different sampling rates are used, this is the LCM(CHANNEL[..].SPR)
	uint64_t  	NRec;		// number of records/blocks -1 indicates length is unknown.	
	uint32_t 	Dur[2];		// Duration of each block in seconds expressed in the fraction Dur[0]/Dur[1] 
	double 		SampleRate;	// Sampling rate
	uint8_t 	IPaddr[6]; 	// IP address of recording device (if applicable)	
	uint32_t  	LOC[4];		// location of recording according to RFC1876
	gdf_time 	T0; 		// starttime of recording

	// Patient specific information 
	struct {
		char*		Name;		// not recommended because of privacy protection 
		char*		Id;		// identification code as used in hospital 
		uint8_t		Weight;		// weight in kilograms [kg] 0:unkown, 255: overflow 
		uint8_t		Height;		// height in centimeter [cm] 0:unkown, 255: overflow 
		gdf_time 	Birthday; 	// Birthday of Patient
		uint16_t	Headsize[3]; 	// circumference, nasion-inion, left-right mastoid in millimeter; 
		// Patient classification // 
		int	 	Sex;		// 0:Unknown, 1: Male, 2: Female 	
		int		Handedness;	// 0:Unknown, 1: Right, 2: Left, 3: Equal
		int		Smoking;	// 0:Unknown, 1: NO, 2: YES
		int		AlcoholAbuse;	// 0:Unknown, 1: NO, 2: YES
		int		DrugAbuse;	// 0:Unknown, 1: NO, 2: YES
		int		Medication;	// 0:Unknown, 1: NO, 2: YES
		struct {
			int 	Visual;		// 0:Unknown, 1: NO, 2: YES, 3: Corrected
		} Impairment;
	} Patient; 
	
	struct {
		char* 		Technician; 	
		char* 		Hospital; 	
		uint64_t 	Equipment; 	// identfies this software
	} ID; 

	// position of electrodes; see also HDR.CHANNEL[k].XYZ
	struct {
		float		REF[3];	// XYZ position of reference electrode
		float		GND[3];	// XYZ position of ground electrode
	} ELEC;

	//	EVENTTABLE 
	struct {
		double  	SampleRate;	// for converting POS and DUR into seconds 
		uint32_t  	N;	// number of events
		uint16_t 	*TYP;	// defined at http://cvs.sourceforge.net/viewcvs.py/biosig/biosig/t200/eventcodes.txt?view=markup
		uint32_t 	*POS;	// starting position [in samples]
		uint32_t 	*DUR;	// duration [in samples]
		uint16_t 	*CHN;	// channel number; 0: all channels 
	} EVENT; 

	struct {	// flags
		char		OVERFLOWDETECTION; 	// overflow & saturation detection 0: OFF, !=0 ON
		char		UCAL; 		// UnCalibration  0: scaling  !=0: NO scaling - raw data return 
	} FLAG; 

	struct {	// File specific data 
		FILE* 		FID;		// file handle 
		size_t 		POS;		// current reading/writing position [in blocks]
		uint8_t		OPEN; 		// 0: closed, 1:read, 2: write
		uint8_t		LittleEndian; 	// 
	} FILE; 

	//	internal variables (not public) 
	struct {
		char 		PID[81];	// patient identification
		char* 		RID;		// recording identification 
		uint32_t 	spb;		// total samples per block
		uint32_t 	bpb;  		// total bytes per block
		uint32_t 	*bi;
		uint8_t*	Header1; 
		uint8_t*	rawdata; 	// raw data block 
	} AS; 	
	CHANNEL_TYPE *CHANNEL;  
	aECG_TYPE *aECG;
} HDRTYPE;


/****************************************************************************/
/**                                                                        **/
/**                     INTERNAL FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/

/*
	These functions are for the converter between SCP to HL7aECG
 */ 	 

HDRTYPE* sopen_SCP_read     (char* Header, HDRTYPE* hdr);
HDRTYPE* sopen_SCP_write    (HDRTYPE* hdr);
HDRTYPE* sopen_HL7aECG_read (HDRTYPE* hdr);
HDRTYPE* sopen_HL7aECG_write(HDRTYPE* hdr);

int16_t	Label_to_LeadIDCode(char* Label);

/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/


HDRTYPE* create_default_hdr(const unsigned NS, const unsigned N_EVENT);
HDRTYPE* sopen(const char* FileName, const char* MODE, HDRTYPE* hdr);
int 	sclose(HDRTYPE* hdr);
size_t 	sread(HDRTYPE* hdr, size_t start, size_t length);
size_t 	sread2(biosig_data_type** channels_dest, size_t start, size_t length, HDRTYPE* hdr); 

size_t	swrite(const void *ptr, size_t nelem, HDRTYPE* hdr);
int	seof(HDRTYPE* hdr);
void	srewind(HDRTYPE* hdr);
int 	sseek(HDRTYPE* hdr, long int offset, int whence);
long int stell(HDRTYPE* hdr);

/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/

#endif	/* BIOSIG_H */
