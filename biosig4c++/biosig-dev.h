/*
%
% $Id: biosig-dev.h,v 1.17 2009/04/08 12:49:54 schloegl Exp $
% Copyright (C) 2005,2006,2007,2008,2009 Alois Schloegl <a.schloegl@ieee.org>
% This file is part of the "BioSig for C/C++" repository 
% (biosig4c++) at http://biosig.sf.net/ 


    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.


 */

/* 
	Internal definitions (external API is defined in biosig.h) 
*/

/****************************************************************************/
/**                                                                        **/
/**                CONSTANTS and Global variables                          **/
/**                                                                        **/
/****************************************************************************/



/****************************************************************************/
/**                                                                        **/
/**                 DEFINITIONS, TYPEDEFS AND MACROS                       **/
/**                                                                        **/
/****************************************************************************/
#ifndef __BIOSIG_INTERNAL_H__
#define __BIOSIG_INTERNAL_H__

#include "biosig.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#if  	__APPLE__
#define __BIG_ENDIAN  	 4321
#define __LITTLE_ENDIAN  1234
#if	__LITTLE_ENDIAN__
#define __BYTE_ORDER 	__LITTLE_ENDIAN
#else
#define __BYTE_ORDER 	__BIG_ENDIAN
#endif
#endif

#if	__sparc__
#define __BIG_ENDIAN  	 4321
#define __LITTLE_ENDIAN  1234
#define __BYTE_ORDER 	__BIG_ENDIAN
#endif 

#if	__MINGW32__
#define __BIG_ENDIAN  	 4321
#define __LITTLE_ENDIAN  1234
#define __BYTE_ORDER 	__LITTLE_ENDIAN
#endif 

#ifdef __linux__
/* use byteswap macros from the host system, hopefully optimized ones ;-) */
#include <byteswap.h>
#endif 

#ifdef	__WIN32__
#define FILESEP '\\'
#else
#define FILESEP '/'
#endif


#ifndef _BYTESWAP_H
/* define our own version - needed for Max OS X*/
#define bswap_16(x)   \
	((((x) & 0xff00) >> 8) | (((x) & 0x00ff) << 8))

#define bswap_32(x)   \
	 ((((x) & 0xff000000) >> 24) \
        | (((x) & 0x00ff0000) >> 8)  \
	| (((x) & 0x0000ff00) << 8)  \
	| (((x) & 0x000000ff) << 24))

#define bswap_64(x) \
      	 ((((x) & 0xff00000000000000ull) >> 56)	\
      	| (((x) & 0x00ff000000000000ull) >> 40)	\
      	| (((x) & 0x0000ff0000000000ull) >> 24)	\
      	| (((x) & 0x000000ff00000000ull) >> 8)	\
      	| (((x) & 0x00000000ff000000ull) << 8)	\
      	| (((x) & 0x0000000000ff0000ull) << 24)	\
      	| (((x) & 0x000000000000ff00ull) << 40)	\
      	| (((x) & 0x00000000000000ffull) << 56))
#endif  /* _BYTESWAP_H */

#ifdef __cplusplus
EXTERN_C {
#endif 

#if __BYTE_ORDER == __BIG_ENDIAN
#define l_endian_u16(x) ((uint16_t)bswap_16((uint16_t)(x)))
#define l_endian_u32(x) ((uint32_t)bswap_32((uint32_t)(x)))
#define l_endian_u64(x) ((uint64_t)bswap_64((uint64_t)(x)))
#define l_endian_i16(x) ((int16_t)bswap_16((int16_t)(x)))
#define l_endian_i32(x) ((int32_t)bswap_32((int32_t)(x)))
#define l_endian_i64(x) ((int64_t)bswap_64((int64_t)(x)))
float   l_endian_f32(float x); 
double  l_endian_f64(double x); 

#define b_endian_u16(x) ((uint16_t)(x))
#define b_endian_u32(x) ((uint32_t)(x))
#define b_endian_u64(x) ((uint64_t)(x))
#define b_endian_i16(x) ((int16_t)(x))
#define b_endian_i32(x) ((int32_t)(x))
#define b_endian_i64(x) ((int64_t)(x))
#define b_endian_f32(x) ((float)(x))
#define b_endian_f64(x) ((double)(x))

#elif __BYTE_ORDER==__LITTLE_ENDIAN
#define l_endian_u16(x) ((uint16_t)(x))
#define l_endian_u32(x) ((uint32_t)(x))
#define l_endian_u64(x) ((uint64_t)(x))
#define l_endian_i16(x) ((int16_t)(x))
#define l_endian_i32(x) ((int32_t)(x))
#define l_endian_i64(x) ((int64_t)(x))
#define l_endian_f32(x) ((float)(x))
#define l_endian_f64(x) ((double)(x))

#define b_endian_u16(x) ((uint16_t)bswap_16((uint16_t)(x)))
#define b_endian_u32(x) ((uint32_t)bswap_32((uint32_t)(x)))
#define b_endian_u64(x) ((uint64_t)bswap_64((uint64_t)(x)))
#define b_endian_i16(x) ((int16_t)bswap_16((int16_t)(x)))
#define b_endian_i32(x) ((int32_t)bswap_32((int32_t)(x)))
#define b_endian_i64(x) ((int64_t)bswap_64((int64_t)(x)))
float   b_endian_f32(float x); 
double  b_endian_f64(double x); 

#endif /* __BYTE_ORDER */


#ifndef __sparc__
// if misaligned data words can be handled 
#define leu16p(i) l_endian_u16(*(uint16_t*)(i))
#define lei16p(i) l_endian_i16(*( int16_t*)(i))
#define leu32p(i) l_endian_u32(*(uint32_t*)(i))
#define lei32p(i) l_endian_i32(*( int32_t*)(i))
#define leu64p(i) l_endian_u64(*(uint64_t*)(i))
#define lei64p(i) l_endian_i64(*( int64_t*)(i))
#define lef32p(i) l_endian_f32(*(float*)(i))
#define lef64p(i) l_endian_f64(*(double*)(i))

#define beu16p(i) b_endian_u16(*(uint16_t*)(i))
#define bei16p(i) b_endian_i16(*( int16_t*)(i))
#define beu32p(i) b_endian_u32(*(uint32_t*)(i))
#define bei32p(i) b_endian_i32(*( int32_t*)(i))
#define beu64p(i) b_endian_u64(*(uint64_t*)(i))
#define bei64p(i) b_endian_i64(*( int64_t*)(i))
#define bef32p(i) b_endian_f32(*(float*)(i))
#define bef64p(i) b_endian_f64(*(double*)(i))

#define leu16a(i,r) (*(uint16_t*)r = l_endian_u16(i))
#define lei16a(i,r) (*( int16_t*)r = l_endian_i16(i))
#define leu32a(i,r) (*(uint32_t*)r = l_endian_u32(i))
#define lei32a(i,r) (*( int32_t*)r = l_endian_i32(i))
#define leu64a(i,r) (*(uint64_t*)r = l_endian_u64(i))
#define lei64a(i,r) (*( int64_t*)r = l_endian_i64(i))
#define lef32a(i,r) (*(uint32_t*)r = l_endian_f32(i))
#define lef64a(i,r) (*(uint64_t*)r = l_endian_f64(i))

#define beu16a(i,r) (*(uint16_t*)r = b_endian_u16(i))
#define bei16a(i,r) (*( int16_t*)r = b_endian_i16(i))
#define beu32a(i,r) (*(uint32_t*)r = b_endian_u32(i))
#define bei32a(i,r) (*( int32_t*)r = b_endian_i32(i))
#define beu64a(i,r) (*(uint64_t*)r = b_endian_u64(i))
#define bei64a(i,r) (*( int64_t*)r = b_endian_i64(i))
#define bef32a(i,r) (*(uint32_t*)r = b_endian_f32(i))
#define bef64a(i,r) (*(uint64_t*)r = b_endian_f64(i))

#else
/*    SPARC: missing alignment must be explicitly handled     */ 
uint16_t leu16p(uint8_t* i);
int16_t  lei16p(uint8_t* i);
uint32_t leu32p(uint8_t* i);
int32_t  lei32p(uint8_t* i);
uint64_t leu64p(uint8_t* i);
int64_t  lei64p(uint8_t* i);
float    lef32p(uint8_t* i);
double   lef64p(uint8_t* i);

uint16_t beu16p(uint8_t* i);
int16_t  bei16p(uint8_t* i);
uint32_t beu32p(uint8_t* i);
int32_t  bei32p(uint8_t* i);
uint64_t beu64p(uint8_t* i);
int64_t  bei64p(uint8_t* i);
float    bef32p(uint8_t* i);
double   bef64p(uint8_t* i);

void leu16a(uint16_t i, uint8_t* r);
void lei16a( int16_t i, uint8_t* r);
void leu32a(uint32_t i, uint8_t* r);
void lei32a( int32_t i, uint8_t* r);
void leu64a(uint64_t i, uint8_t* r);
void lei64a( int64_t i, uint8_t* r);
void lef32a(   float i, uint8_t* r);
void lef64a(  double i, uint8_t* r);

void beu16a(uint16_t i, uint8_t* r);
void bei16a( int16_t i, uint8_t* r);
void beu32a(uint32_t i, uint8_t* r);
void bei32a( int32_t i, uint8_t* r);
void beu64a(uint64_t i, uint8_t* r);
void bei64a( int64_t i, uint8_t* r);
void bef32a(   float i, uint8_t* r);
void bef64a(  double i, uint8_t* r);

#endif
#ifdef __cplusplus
}
#endif 


#define NaN (0.0/0.0)	/* used for encoding of missing values */ 
#define INF (1.0/0.0)	/* positive infinity */

//const double NaN = (0.0/0.0);
//const double INF = (1.0/0.0);

#define min(a,b)	(((a) < (b)) ? (a) : (b))
#define max(a,b)	(((a) > (b)) ? (a) : (b))
#define fix(m)     	(m<0 ? ceil(m) : floor(m))	

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	global constants and variables 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef VERBOSE_LEVEL
EXTERN_C int   VERBOSE_LEVEL; 	// used for debugging
#endif 




/****************************************************************************/
/**                                                                        **/
/**                     TYPEDEFS AND STRUCTURES                            **/
/**                                                                        **/
/****************************************************************************/


/*
	This structure defines the fields used for "VitalFEF" 
 */
typedef struct {
	void *pduType;
	void *SAS;
} ASN1_t;

/*
	This structure defines the fields used for "Annotated ECG" 
 */
typedef struct {
	char*		test;		/* test field for annotated ECG */
	
	float		diastolicBloodPressure;		/* in mmHg */
	float		systolicBloodPressure;		/* in mmHg */
	char*		MedicationDrugs;
	char*		ReferringPhysician;
	char*		LatestConfirmingPhysician;
	char*		Diagnosis;
	uint8_t		EmergencyLevel; /* 0: routine 1-10: increased emergency level */

	float		HeartRate;	
	float		P_wave[2]; 	/* start and end  */
	float		QRS_wave[2]; 	/* start and end  */
	float		T_wave[2]; 	/* start and end  */
	float		P_QRS_T_axes[3];

	/***** SCP only fields *****/
	struct {	
		uint8_t	HUFFMAN;
		uint8_t	REF_BEAT;
		uint8_t	DIFF;
		uint8_t	BIMODAL;
	} FLAG;
        struct {
		//uint8_t tag14[41],tag15[41];
	        struct {
			uint16_t INST_NUMBER;		/* tag 14, byte 1-2  */
			uint16_t DEPT_NUMBER;		/* tag 14, byte 3-4  */
			uint16_t DEVICE_ID;		/* tag 14, byte 5-6  */
			uint8_t  DEVICE_TYPE;		/* tag 14, byte 7: 0: Cart, 1: System (or Host)  */
			uint8_t MANUF_CODE;		/* tag 14, byte 8 (MANUF_CODE has to be 255) */
			char*   MOD_DESC;		/* tag 14, byte 9 (MOD_DESC has to be "Cart1") */
			uint8_t VERSION;		/* tag 14, byte 15 (VERSION has to be 20) */
			uint8_t PROT_COMP_LEVEL;	/* tag 14, byte 16 (PROT_COMP_LEVEL has to be 0xA0 => level II) */
			uint8_t LANG_SUPP_CODE;		/* tag 14, byte 17 (LANG_SUPP_CODE has to be 0x00 => Ascii only, latin and 1-byte code) */
			uint8_t ECG_CAP_DEV;		/* tag 14, byte 18 (ECG_CAP_DEV has to be 0xD0 => Acquire, (No Analysis), Print and Store) */
			uint8_t MAINS_FREQ;		/* tag 14, byte 19 (MAINS_FREQ has to be 0: unspecified, 1: 50 Hz, 2: 60Hz) */
			char 	reserved[22]; 		/* char[35-19] reserved; */			
			char* 	ANAL_PROG_REV_NUM;
			char* 	SERIAL_NUMBER_ACQ_DEV;
			char* 	ACQ_DEV_SYS_SW_ID;
			char* 	ACQ_DEV_SCP_SW; 	/* tag 14, byte 38 (SCP_IMPL_SW has to be "OpenECG XML-SCP 1.00") */
			char* 	ACQ_DEV_MANUF;		/* tag 14, byte 38 (ACQ_DEV_MANUF has to be "Manufacturer") */
        	} Tag14, Tag15; 
        } Section1;
        struct {
        	size_t   StartPtr;
        	size_t	 Length;
        } Section5;
        struct {
        	size_t   StartPtr;
        	size_t	 Length;
        } Section6;
        struct {
        	char	 Confirmed; // 0: original report (not overread); 1:Confirmed report; 2: Overread report (not confirmed)
		struct tm t; 
		uint8_t	 NumberOfStatements;
		char 	 **Statements;
        } Section8;
        struct {
        	char*    StartPtr;
        	size_t	 Length;
        } Section9;
        struct {
        	size_t   StartPtr;
        	size_t	 Length;
        } Section10;
        struct {
        	char	 Confirmed; // 0: original report (not overread); 1:Confirmed report; 2: Overread report (not confirmed)
		struct tm t; 
		uint8_t	 NumberOfStatements;
		char 	 **Statements;
        } Section11;
} aECG_TYPE;

/****************************************************************************/
/**                                                                        **/
/**                     INTERNAL FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/

/*
        file access wrapper: use ZLIB (if available) or STDIO
 */ 	 
#ifdef __cplusplus
EXTERN_C {
#endif 

HDRTYPE* 	ifopen(HDRTYPE* hdr, const char* mode );
int 		ifclose(HDRTYPE* hdr);
int             ifeof(HDRTYPE* hdr);
int 		ifflush(HDRTYPE* hdr);
size_t 		ifread(void* buf, size_t size, size_t nmemb, HDRTYPE* hdr);
size_t 		ifwrite(void* buf, size_t size, size_t nmemb, HDRTYPE* hdr);
int             ifprintf(HDRTYPE* hdr, const char *format, va_list arg);
int             ifputc(int c, HDRTYPE* hdr);
int 		ifgetc(HDRTYPE* hdr);
char*           ifgets(char *str, int n, HDRTYPE* hdr);
int             ifseek(HDRTYPE* hdr, long offset, int whence );
long            iftell(HDRTYPE* hdr);
int 		ifgetpos(HDRTYPE* hdr, size_t *pos);
int             iferror(HDRTYPE* hdr);


/*
	various utility functions 
*/

uint32_t gcd(uint32_t A, uint32_t B);
uint32_t lcm(uint32_t A, uint32_t B);

EXTERN_C const int16_t GDFTYP_BITS[];
EXTERN_C const char *LEAD_ID_TABLE[];

uint16_t CRCEvaluate(uint8_t* datablock, uint32_t datalength);
int16_t CRCCheck(uint8_t* datablock, uint32_t datalength);

#ifndef _WIN32
int strcmpi(const char* str1, const char* str2);
#endif
int strncmpi(const char* str1, const char* str2, size_t n);
int u32cmp(const void *a, const void *b); 

//double strtod (const char *nptr, char **endptr); // no locale dependency 

/*
	some important functions used internally, 
	the interface for these functios is a bit clumsy and is 
	(therefore) not 
*/

void struct2gdfbin(HDRTYPE *hdr);
int gdfbin2struct(HDRTYPE *hdr);
/* struct2gdfbin and gdfbin2struct
	convert between the streamed header information (as in a GDF file or 
	on a network connection) and the header structure HDRTYPE 
	Specifically, the fixed header, the variable hadder and the optional 
	header information (header 1,2 and 3). This incluedes the 
	description of the user-specified events (TYP=1..255), but not the 
	event table itself. 
 ------------------------------------------------------------------------*/

size_t hdrEVT2rawEVT(HDRTYPE *hdr);
void rawEVT2hdrEVT(HDRTYPE *hdr);
/* rawEVT2hdrEVT and hdrEVT2rawEVT
	convert between streamed event table and the structure
	HDRTYPE.EVENT.
 ------------------------------------------------------------------------*/

void FreeGlobalEventCodeTable();
void FreeTextEvent(HDRTYPE* hdr,size_t N_EVENT, char* annotation);
/*------------------------------------------------------------------------
	adds free text annotation to event table    
	the EVENT.TYP is identified from the table EVENT.CodeDesc
	if annotations is not listed in CodeDesc, it is added to CodeDesc
	The table is limited to 256 entries, because the table EventCodes
	allows only codes 0-255 as user specific entry.
  ------------------------------------------------------------------------*/

size_t	sread_raw(size_t START, size_t LEN, HDRTYPE* hdr, char flag);
/* sread_raw: 
	LEN data segments are read from file associated with hdr, starting from 
	segment START. A sufficient amount of memory is (re-)allocated in 
	hdr->AS.rawdata and the data is copied into  hdr->AS.rawdata
	Typically, LEN*hdr->AS.bpb bytes are read and stored in its native format.
	No Overflowdetection or calibration is applied. 
	
	The number of successfully read data blocks is returned, this can be smaller 
	than LEN at the end of the file. The data can be "cached", this means 
	that more than the requested number of blocks is available in hdr->AS.rawdata. 
	hdr->AS.first and hdr->AS.length contain the number of the first 
	block and the number of blocks, respectively.  
 --------------------------------------------------------------- */

size_t bpb8_collapsed_rawdata(HDRTYPE *hdr);
/* bpb8_collapsed_rawdata
	computes the bits per block when rawdata is collapsed
--------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif 

/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/

#endif	/* BIOSIG_INTERNAL_H */
