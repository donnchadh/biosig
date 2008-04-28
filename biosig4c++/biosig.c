/*

    $Id: biosig.c,v 1.179 2008-04-28 11:02:16 schloegl Exp $
    Copyright (C) 2005,2006,2007,2008 Alois Schloegl <a.schloegl@ieee.org>
    This file is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 


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

/*

	Library function for reading and writing of varios biosignal data formats.
	It provides also one reference implementation for reading and writing of the 
	GDF data format [1].
	
	Features: 
	- reading and writing of EDF, BDF, GDF1, GDF2, CWFB, HL7aECG, SCP files 
	- reading of ACQ, AINF, BKR, BrainVision, CNT, DEMG, EGI, ETG4000, MFER files 
	
	implemented functions: 
	- SOPEN, SREAD, SWRITE, SCLOSE, SEOF, SSEEK, STELL, SREWIND 
	

	References: 
	[1] GDF - A general data format for biomedical signals. 
		available online http://arxiv.org/abs/cs.DB/0608052	

	
*/

#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "biosig-dev.h"

const int16_t GDFTYP_BYTE[] = {
	1, 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 0, 0, 0, 0, 0,   /* 0  */ 
	4, 8,16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 16 */ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 32 */ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 48 */ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 64 */ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 128 */ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2,    /* 256 - 271*/  
	0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4,    /* 255+24 = bit24, 3 byte */ 
	0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 6, 
	0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 8, 
	0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0,10, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 384 - 399*/ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2,    /* 512 - 527*/ 
	0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4, 
	0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 6, 
	0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 8, 
	0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0,10};


const char *LEAD_ID_TABLE[] = { "unspecified",
	"I","II","V1","V2","V3","V4","V5","V6",
	"V7","V2R","V1","V2","V3","V4","V5","V6",
	"V7","V2R","V3R","V4R","V5R","V6R","V7R","X",
	"Y","Z","CC5","CM5","LA","RA","LL","fI",
	"fE","fC","fA","fM","fF","fH","dI",
	"dII","dV1","dV2","dV3","dV4","dV5",
	"dV6","dV7","dV2R","dV3R","dV4R","dV5R",
	"dV6R","dV7R","dX","dY","dZ","dCC5","dCM5",
	"dLA","dRA","dLL","dfI","dfE","dfC","dfA",
	"dfM","dfF","dfH","III","aVR","aVL","aVF",
	"aVRneg","V8","V9","V8R","V9R","D","A","J",
	"Defib","Extern","A1","A2","A3","A4","dV8",
	"dV9","dV8R","dV9R","dD","dA","dJ","Chest",
	"V","VR","VL","VF","MCL","MCL1","MCL2","MCL3",
	"MCL4","MCL5","MCL6","CC","CC1","CC2","CC3",
	"CC4","CC6","CC7","CM","CM1","CM2","CM3","CM4",
	"CM6","dIII","daVR","daVL","daVF","daVRneg","dChest",
	"dV","dVR","dVL","dVF","CM7","CH5","CS5","CB5","CR5",
	"ML","AB1","AB2","AB3","AB4","ES","AS","AI","S",
	"dDefib","dExtern","dA1","dA2","dA3","dA4","dMCL1",
	"dMCL2","dMCL3","dMCL4","dMCL5","dMCL6","RL","CV5RL",
	"CV6LL","CV6LU","V10","dMCL","dCC","dCC1","dCC2",
	"dCC3","dCC4","dCC6","dCC7","dCM","dCM1","dCM2",
	"dCM3","dCM4","dCM6","dCM7","dCH5","dCS5","dCB5",
	"dCR5","dML","dAB1","dAB2","dAB3","dAB4","dES",
	"dAS","dAI","dS","dRL","dCV5RL","dCV6LL","dCV6LU","dV10"
/*   EEG Leads - non consecutive index 
	,"NZ","FPZ","AFZ","FZ","FCZ","CZ","CPZ","PZ",
	"POZ","OZ","IZ","FP1","FP2","F1","F2","F3","F4",
	"F5","F6","F7","F8","F9","F10","FC1","FC2","FC3",
	"FC4","FC5","FC6","FT7","FT8","FT9","FT10","C1",
	"C2","C3","C4","C5","C6","CP1","CP2","CP3","CP4",
	"CP5","CP6","P1","P2","P3","P4","P5","P6","P9",
	"P10","O1","O2","AF3","AF4","AF7","AF8","PO3",
	"PO4","PO7","PO8","T3","T7","T4","T8","T5","P7",
	"T6","P8","T9","T10","TP7","TP8","TP9","TP10",
	"A1","A2","T1","T2","PG1","PG2","SP1","SP2",
	"E0","EL1","EL2","EL3","EL4","EL5","EL6","EL7",
	"ER1","ER2","ER3","ER4","ER5","ER6","ER7","ELL",
	"ERL","ELA","ELB","ERA","ERB"
*/
	, "\0\0" };  // stop marker 



/****************************************************************************/
/**                                                                        **/
/**                      INTERNAL FUNCTIONS                                **/
/**                                                                        **/
/****************************************************************************/
 
// greatest common divisor 
uint32_t gcd(uint32_t A, uint32_t B)
{	size_t t; 
	if (A<B) {t=B; B=A; A=t;}; 
	while (B) {
		t = B; 
		B = A%B;
		A = t; 
	}
	return(A);
};

// least common multiple - used for obtaining the common HDR.SPR
uint32_t lcm(uint32_t A, uint32_t B)
{	
	// return(A*(B/gcd(A,B)) with overflow detection 
	uint64_t A64 = A;
	A64 *= B/gcd(A,B);
	if (A64 > 0x00000000ffffffffllu) {
		fprintf(stderr,"Error: HDR.SPR=LCM(%i,%i) overflows and does not fit into uint32.\n",A,B);
		exit(-4);
	}
	return((uint32_t)A64);
};

#if __BYTE_ORDER == __BIG_ENDIAN
float l_endian_f32(float x) 
#elif __BYTE_ORDER == __LITTLE_ENDIAN
float b_endian_f32(float x) 
#endif
{
	union {
		float f32;
		uint32_t u32;
	} b1,b2; 
	b1.f32 = x; 
	b2 = b1; 
	b2.u32 = bswap_32(b1.u32);
	return(b2.f32);
}

#if __BYTE_ORDER == __BIG_ENDIAN
double l_endian_f64(double x) 
#elif __BYTE_ORDER == __LITTLE_ENDIAN
double b_endian_f64(double x) 
#endif
{
	union {
		double f64;
		uint32_t u32[2];
	} b1,b2; 
	b1.f64 = x; 
	b2 = b1; 
	b2.u32[0] = bswap_32(b1.u32[1]);
	b2.u32[1] = bswap_32(b1.u32[0]);
	return(b2.f64);
}

#ifdef __sparc__
/*    SPARC: missing alignment must be explicitly handled     */ 
uint16_t leu16p(uint8_t* i) {
	// decode little endian uint16 pointer 
	return ((*i) + ((uint16_t)*(i+1) << 8));
}
int16_t lei16p(uint8_t* i) {
	// decode little endian int16 pointer 
	uint16_t o = ((*i) + ((uint16_t)*(i+1) << 8));
	return(*(int16_t*)(&o)); 
}
uint32_t leu32p(uint8_t* i) {
	// decode little endian uint32 pointer 
	uint32_t o=0;
	for (char k=0; k<4; k++)
		o += ((uint32_t)*(i+k))<<(k*8);
	return(o); 
}
int32_t lei32p(uint8_t* i) {
	// decode little endian int32 pointer 
	uint32_t o=0;
	for (char k=0; k<4; k++)
		o += ((uint32_t)*(i+k))<<(k*8);
	return(*(int32_t*)(&o)); 
}
uint64_t leu64p(uint8_t* i) {
	// decode little endian uint64 pointer 
	uint64_t o=0;
	for (char k=0; k<8; k++)
		o += ((uint64_t)*(i+k))<<(k*8);
	return(o); 
}
int64_t lei64p(uint8_t* i) {
	// decode little endian int64 pointer 
	uint64_t o=0;
	for (char k=0; k<8; k++)
		o += ((uint64_t)*(i+k))<<(k*8);
	return(*(int64_t*)(&o)); 
}
float lef32p(uint8_t* i) {
	// decode little endian float pointer 
	uint32_t o;
	for (char k=0, o=0; k<4; k++)
		o += ((uint32_t)*(i+k))<<(k*8);
	return(*(float*)(&o)); 
}
double lef64p(uint8_t* i) {
	// decode little endian double pointer 
	uint64_t o=0;
	for (char k=0; k<8; k++)
		o += ((uint64_t)*(i+k))<<(k*8);
	return(*(double*)(&o)); 
}

uint16_t beu16p(uint8_t* i) {
	// decode big endian uint16 pointer 
	return (((uint16_t)*i<<8) + (*(i+1)));
}
int16_t bei16p(uint8_t* i) {
	// decode big endian int16 pointer 
	uint16_t o = (((uint16_t)*i << 8) + (*(i+1)));
	return(*(int16_t*)(&o)); 
}
uint32_t beu32p(uint8_t* i) {
	// decode big endian uint32 pointer 
	uint32_t o=0;
	for (char k=0; k<4; k++) {
		o<<=8;
		o += *(i+k);
	}	
	return(o); 
}
int32_t bei32p(uint8_t* i) {
	// decode big endian int32 pointer 
	uint32_t o=0;
	for (char k=0; k<4; k++) {
		o<<=8;
		o += *(i+k);
	}	
	return(*(int32_t*)(&o)); 
}
uint64_t beu64p(uint8_t* i) {
	// decode big endian uint64 pointer 
	uint64_t o=0;
	for (char k=0; k<8; k++) {
		o<<=8;
		o += *(i+k);
	}	
	return(o); 
}
int64_t bei64p(uint8_t* i) {
	// decode big endian int64 pointer 
	uint64_t o=0;
	for (char k=0; k<8; k++){
		o<<=8;
		o += *(i+k);
	}	
	return(*(int64_t*)(&o)); 
}
float bef32p(uint8_t* i) {
	// decode big endian float pointer 
	uint32_t o=0;
	for (char k=0; k<4; k++){
		o<<=8;
		o += *(i+k);
	}	
	return(*(float*)(&o)); 
}
double bef64p(uint8_t* i) {
	// decode big endian double pointer 
	uint64_t o=0;
	for (char k=0; k<8; k++){
		o<<=8;
		o += *(i+k);
	}	
	return(*(double*)(&o)); 
}

void leu16a(uint16_t i, uint8_t* r) {
	i = l_endian_u16(i);
	memcpy(&i,r,sizeof(i));
}
void lei16a( int16_t i, uint8_t* r) {
	i = l_endian_i16(i);
	memcpy(&i,r,sizeof(i));
}
void leu32a(uint32_t i, uint8_t* r) {
	i = l_endian_u32(i);
	memcpy(&i,r,sizeof(i));
}
void lei32a( int32_t i, uint8_t* r) {
	i = l_endian_i32(i);
	memcpy(&i,r,sizeof(i));
}
void leu64a(uint64_t i, uint8_t* r) {
	i = l_endian_u64(i);
	memcpy(&i,r,sizeof(i));
}
void lei64a( int64_t i, uint8_t* r) {
	i = l_endian_i64(i);
	memcpy(&i,r,sizeof(i));
}
void lef32a( float i, uint8_t* r) {
	uint32_t i32 = l_endian_u32(*(uint32_t*)(&i));
	memcpy(&i32,r,sizeof(i32));
}
void lef64a(  double i, uint8_t* r) {
	uint64_t i64 = l_endian_u64(*(uint64_t*)(&i));
	memcpy(&i64,r,sizeof(i64));
}

void beu16a(uint16_t i, uint8_t* r) {
	i = b_endian_u16(i);
	memcpy(&i,r,sizeof(i));
};
void bei16a( int16_t i, uint8_t* r) {
	i = b_endian_i16(i);
	memcpy(&i,r,sizeof(i));
}
void beu32a(uint32_t i, uint8_t* r) {
	i = b_endian_u32(i);
	memcpy(&i,r,sizeof(i));
}
void bei32a( int32_t i, uint8_t* r) {
	i = b_endian_i32(i);
	memcpy(&i,r,sizeof(i));
}
void beu64a(uint64_t i, uint8_t* r) {
	i = b_endian_u64(i);
	memcpy(&i,r,sizeof(i));
}
void bei64a( int64_t i, uint8_t* r) {
	i = b_endian_i64(i);
	memcpy(&i,r,sizeof(i));
}
void bef32a(   float i, uint8_t* r) {
	uint32_t i32 = b_endian_u32(*(uint32_t*)(&i));
	memcpy(&i32,r,sizeof(i32));
}
void bef64a(  double i, uint8_t* r) {
	uint64_t i64 = b_endian_u64(*(uint64_t*)(&i));
	memcpy(&i64,r,sizeof(i64));
}

#endif


void* mfer_swap8b(uint8_t *buf, int8_t len, char FLAG_SWAP) 
{	
	if (VERBOSE_LEVEL==9) 
		fprintf(stdout,"swap=%i %i %i \nlen=%i %2x%2x%2x%2x%2x%2x%2x%2x\n",FLAG_SWAP, __BYTE_ORDER, __LITTLE_ENDIAN, len, buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]); 
	
	typedef uint64_t iType; 
#if __BYTE_ORDER == __BIG_ENDIAN
        if (FLAG_SWAP) {
                for (unsigned k=len; k < sizeof(iType); buf[k++]=0);
                *(iType*)buf = bswap_64(*(iType*)buf); 
        }
        else
                *(iType*)buf >>= (sizeof(iType)-len)*8;

#elif __BYTE_ORDER == __LITTLE_ENDIAN
        if (FLAG_SWAP)
                *(iType*)buf = bswap_64(*(iType*)buf) >> (sizeof(iType)-len)*8; 
        else {
        	unsigned k;
		for (k=len; k < sizeof(iType); buf[k++]=0);
	}	

#endif

	if (VERBOSE_LEVEL==9)	
		fprintf(stdout,"%2x%2x%2x%2x%2x%2x%2x%2x %Li %f\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],*(uint64_t*)buf,*(double*)buf); 

	return(buf); 
}


/* --------------------------------
 * float to ascii[8] conversion 
 * -------------------------------- */
int ftoa8(char* buf, double num)
{
	// used for converting scaling factors Dig/Phys/Min/Max into EDF header
	// Important note: buf may need more than len+1 bytes. make sure there is enough memory allocated.   
	double f1,f2;

	sprintf(buf,"%f",num);

	f1 = atof(buf); 
	buf[8] = 0; 	// truncate 
	f2 = atof(buf); 

	return (fabs((f1-f2)/(f1+f2)) > 1e-6); 
}



/* physical units are defined in 
 prEN ISO 11073-10101 (Nov 2003)
 Health Informatics - Point-of-care medical device communications - Part 10101:Nomenclature
 (ISO/DIS 11073-10101:2003)
 Table A.6.1: Table of Decimal Factors

 CEN/TC251/PT40 2001	
 File Exchange Format for Vital Signs - Annex A 
 Table A.4.1: Table of Decimal Factors	const double scale[32] =
*/

const struct PhysDimIdx 
	{
		const uint16_t	idx;
		const char*	PhysDimDesc;
	} _physdim[] = 	{
	{ 0 ,  "?" },
	{ 512 ,  "-" },
	{ 544 ,  "%" },
	{ 576 ,  "ppht" },
	{ 608 ,  "ppm" },
	{ 640 ,  "" },
	{ 672 ,  "ppb" },
	{ 704 ,  "ppt" },
	{ 736 ,  "degree" },
	{ 768 ,  "rad" },
	{ 800 ,  "g g-1" },
	{ 832 ,  "g kg-1" },
	{ 864 ,  "mol mol-1" },
	{ 896 ,  "l l-1" },
	{ 928 ,  "m m-3 " },
	{ 960 ,  "m cm-3" },
	{ 6240 ,  "vol %" },
	{ 992 ,  "pH" },
	{ 1024 ,  "drop" },
	{ 1056 ,  "rbc" },
	{ 1088 ,  "beat" },
	{ 1120 ,  "breath" },
	{ 1152 ,  "cell" },
	{ 1184 ,  "cough" },
	{ 1216 ,  "sigh" },
	{ 1248 ,  "%PCV" },
	{ 1280 ,  "m" },
	{ 1312 ,  "yd" },
	{ 1344 ,  "ft" },
	{ 1376 ,  "in" },
	{ 1408 ,  "lm-2" },
	{ 1440 ,  "m-1" },
	{ 1472 ,  "m2" },
	{ 1504 ,  "in2" },
	{ 1536 ,  "m-2" },
	{ 1568 ,  "m3" },
	{ 1600 ,  "l" },
	{ 1632 ,  "l breath-1" },
	{ 6112 ,  "l beat-1" },
	{ 1664 ,  "m-3" },
	{ 1696 ,  "l-1" },
	{ 1728 ,  "g" },
	{ 1760 ,  "lb" },
	{ 1792 ,  "oz" },
	{ 1824 ,  "g-1" },
	{ 1856 ,  "g m" },
	{ 1888 ,  "g m m-2 " },
	{ 1920 ,  "kg m2" },
	{ 1952 ,  "kg m-2" },
	{ 1984 ,  "g m-3 " },
	{ 2016 ,  "g cm-3" },
	{ 2048 ,  "g l-1" },
	{ 2080 ,  "g cl-3" },
	{ 2112 ,  "g dl-3" },
	{ 2144 ,  "g ml-3" },
	{ 2176 ,  "s" },
	{ 2208 ,  "min" },
	{ 2240 ,  "h" },
	{ 2272 ,  "d" },
	{ 2304 ,  "weeks" },
	{ 2336 ,  "mth" },
	{ 2368 ,  "y" },
	{ 2400 ,  "TOD" },
	{ 2432 ,  "DATE" },
	{ 2464 ,  "s-1" },
	{ 2496 ,  "Hz" },
	{ 2528 ,  "min-1" },
	{ 2560 ,  "h-1" },
	{ 2592 ,  "d-1" },
	{ 2624 ,  "week-1" },
	{ 2656 ,  "mth-1" },
	{ 2688 ,  "y-1" },
	{ 2720 ,  "bpm" },
	{ 2752 ,  "puls min-1" },
	{ 2784 ,  "resp min-1" },
	{ 2816 ,  "m s-1" },
	{ 2848 ,  "l min-1 m-2" },
	{ 2880 ,  "m2 s-1" },
	{ 2912 ,  "m3 s-1" },
	{ 2944 ,  "m3 min-1" },
	{ 2976 ,  "m3 h-1" },
	{ 3008 ,  "m3 d-1" },
	{ 3040 ,  "l s-1" },
	{ 3072 ,  "l min-1" },
	{ 3104 ,  "l h-1" },
	{ 3136 ,  "l d-1" },
	{ 3168 ,  "l kg-1" },
	{ 3200 ,  "m3 d-1" },
	{ 3232 ,  "m Pa-1s-1" },
	{ 3264 ,  "l min-1 mmHG-1" },
	{ 3296 ,  "g s-1" },
	{ 3328 ,  "g m-1" },
	{ 3360 ,  "g h-1" },
	{ 3392 ,  "g d-1" },
	{ 3424 ,  "g kg-1 s-1" },
	{ 3456 ,  "g kg-1 m-1" },
	{ 3488 ,  "g kg-1 h-1" },
	{ 3520 ,  "g kg-1 d-1" },
	{ 3552 ,  "g l-1 s-1" },
	{ 3584 ,  "g l-1 m-1" },
	{ 3616 ,  "g l-1 h-1" },
	{ 3648 ,  "g l-1 d-1" },
	{ 3680 ,  "g m-1 s-1" },
	{ 3712 ,  "gm s-1" },
	{ 3744 ,  "Ns" },
	{ 3776 ,  "N" },
	{ 3808 ,  "dyn" },
	{ 3840 ,  "Pa" },
	{ 3872 ,  "mmHg" },
	{ 3904 ,  "cm H2O" },
	{ 3936 ,  "bar" },
	{ 3968 ,  "J" },
	{ 4000 ,  "eV" },
	{ 4032 ,  "W" },
	{ 4064 ,  "Pa s m-3" },
	{ 4096 ,  "Pa s l-1" },
	{ 4128 ,  "dyn s cm-5" },
	{ 5888 ,  "l (cmH2O)-1" },
	{ 6272 ,  "l (mmHg)-1" },
	{ 6304 ,  "l Pa-1" },
	{ 6144 ,  "cmH2O l-1" },
	{ 6336 ,  "mmHg l-1" },
	{ 6368 ,  "Pa l-1" },
	{ 4160 ,  "A" },
	{ 4192 ,  "C" },
	{ 6080 ,  "Ah" },
	{ 4224 ,  "A m-1" },
	{ 4256 ,  "V" },
	{ 4288 ,  "Ohm" },
	{ 4320 ,  "Wm" },
	{ 4352 ,  "F" },
	{ 4384 ,  "K" },
	{ 6048 ,  "\xB0\x43" },	//°C
	{ 4416 ,  "\xB0\x46" }, //°F
	{ 4448 ,  "K W-1" },
	{ 4480 ,  "cd" },
	{ 4512 ,  "osmole" },
	{ 4544 ,  "mol" },
	{ 4576 ,  "eq" },
	{ 4608 ,  "osmol l-1" },
	{ 4640 ,  "mol cm-3" },
	{ 4672 ,  "mol m-3" },
	{ 4704 ,  "mol l-1" },
	{ 4736 ,  "mol ml-1" },
	{ 4768 ,  "eq cm-3" },
	{ 4800 ,  "eq m-3" },
	{ 4832 ,  "eq l-1" },
	{ 4864 ,  "eq ml-1" },
	{ 4896 ,  "osmol  kg-1" },
	{ 4928 ,  "mol kg-1" },
	{ 4960 ,  "mol s-1" },
	{ 4992 ,  "mol min-1" },
	{ 5024 ,  "mol h-1" },
	{ 5056 ,  "mol d-1" },
	{ 5088 ,  "eq s-1" },
	{ 5120 ,  "eq min-1" },
	{ 5152 ,  "eq h-1" },
	{ 5184 ,  "eq d-1" },
	{ 5216 ,  "mol kg-1 s-1" },
	{ 5248 ,  "mol kg-1 min-1" },
	{ 5280 ,  "mol kg-1 h-1" },
	{ 5312 ,  "mol kg-1 d-1" },
	{ 5344 ,  "eq kg-1 s-1" },
	{ 5376 ,  "eq kg-1 min-1" },
	{ 5408 ,  "eq kg-1 h-1" },
	{ 5440 ,  "eq kg-1 d-1" },
	{ 5472 ,  "i.u." },
	{ 5504 ,  "i.u. cm-3" },
	{ 5536 ,  "i.u. m-3" },
	{ 5568 ,  "i.u. l-1" },
	{ 5600 ,  "i.u. ml-1" },
	{ 5632 ,  "i.u. s-1" },
	{ 5664 ,  "i.u. min-1" },
	{ 5696 ,  "i.u. h-1" },
	{ 5728 ,  "i.u. d-1" },
	{ 5760 ,  "i.u. kg-1 s-1" },
	{ 5792 ,  "i.u. kg-1 min-1" },
	{ 5824 ,  "i.u. kg-1 h-1" },
	{ 5856 ,  "i.u. kg-1 d-1" },
	{ 5920 ,  "cmH2O l-1s-1" },
	{ 5952 ,  "l2s-1" },
	{ 5984 ,  "cmH2O %-1" },
	{ 6176 ,  "mmHg %-1" },
	{ 6208 ,  "Pa %-1" },
	{ 6432 ,  "dB" },
	{ 6016 ,  "dyne s m-2 cm-5" },
	{65344 ,  "mol l-1 mm"}, 	// "light path length","milli(Mol/Liter)*millimeter"	
	{65376 ,  "r.p.m"}, 		// "rotations per minute"
	{65408 ,  "B"}, 		// "Bel", "relative power decibel"	
	{65440 ,  "dyne s m2 cm-5" },
	{65472 ,  "l m-2" },
	{65504 ,  "T" },
	{0xffff ,  "end-of-table" },
};
		
const char* PhysDimFactor[] = {
	"","da","h","k","M","G","T","P",	//  0..7
	"E","Z","Y","#","#","#","#","#",	//  8..15
	"d","c","m","u","n","p","f","a",	// 16..23
	"z","y","#","#","#","#","#","#",	// 24..31
	"\xB5"	//hack for "µ" = "u"		// 32
	};

double PhysDimScale(uint16_t PhysDimCode)
{	
// converting PhysDimCode -> scaling factor

	const double scale[] =
	{ 1e+0, 1e+1, 1e+2, 1e+3, 1e+6, 1e+9,  1e+12, 1e+15,	//  0..7 
	  1e+18,1e+21,1e+24,NaN,  NaN,  NaN,   NaN,   NaN, 	//  8..15
	  1e-1, 1e-2, 1e-3, 1e-6, 1e-9, 1e-12, 1e-15, 1e-18, 	// 16..23
	  1e-21,1e-24,NaN,  NaN,  NaN,  NaN,   NaN,   NaN,	// 24..31
	  1e-6	// hack for "µ" = "u" 				// 32
	  }; 

	return (scale[PhysDimCode & 0x001f]); 
}

char* PhysDim(uint16_t PhysDimCode, char *PhysDim)
{	
	// converting PhysDimCode -> PhysDim

	strcpy(PhysDim,PhysDimFactor[PhysDimCode & 0x001F]);

	PhysDimCode &= ~0x001F; 
	uint16_t k;
	for (k=0; _physdim[k].idx<0xffff; k++)
	if (PhysDimCode == _physdim[k].idx) {
		strcat(PhysDim,_physdim[k].PhysDimDesc);
		break;
	}
	return(PhysDim);
}

uint16_t PhysDimCode(char* PhysDim0)
{	
// converting PhysDim -> PhysDimCode
	/* converts Physical dimension into 16 bit code */
	if (PhysDim0==NULL) return(0);
	if (!strlen(PhysDim0)) return(0);
	
	uint16_t k1, k2;
	char s[80];
	char *s1;

	// greedy search - check all codes 0..65535
	for (k1=0; k1<33; k1++)
	if (!strncmp(PhysDimFactor[k1],PhysDim0,strlen(PhysDimFactor[k1])) && PhysDimScale(k1)>0.0) 
	{ 	// exclude if beginning of PhysDim0 differs from PhysDimFactor and if NaN 
		strcpy(s, PhysDimFactor[k1]);
		s1 = s+strlen(s);
		for (k2=0; _physdim[k2].idx < 0xffff; k2++) {
			strcpy(s1, _physdim[k2].PhysDimDesc);
			if (!strcmp(PhysDim0, s)) {
		 		if (k1==32) k1 = 19;		// hack for "Âµ" = "u"
				return(_physdim[k2].idx+k1);
			}	 
		}
	}
	return(0);
}

/*
	compare strings, ignore case  
 */
int strcmpi(const char* str1, const char* str2)
{	
	unsigned int k=0;
	int r;
	r = tolower(str1[k]) - tolower(str2[k]); 
	while (!r && str1[k] && str2[k]) {
		k++; 
		r = tolower(str1[k]) - tolower(str2[k]);
	}	
	return(r); 	 	
}

int errnum;
int B4C_STATUS  = 0;
int B4C_ERRNUM  = 0;
const char *B4C_ERRMSG;
int VERBOSE_LEVEL = -1; 

/*
	Interface for mixed use of ZLIB and STDIO 
	If ZLIB is not available, STDIO is used. 
	If ZLIB is availabe, HDR.FILE.COMPRESSION tells
	whether STDIO or ZLIB is used. 
 */
HDRTYPE* ifopen(HDRTYPE* hdr, const char* mode) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION) 
	{
	hdr->FILE.gzFID = gzopen(hdr->FileName, mode);
	hdr->FILE.OPEN = (hdr->FILE.gzFID != NULL); 
	} else	
#endif
	{ 
	hdr->FILE.FID = fopen(hdr->FileName,mode);
	hdr->FILE.OPEN = (hdr->FILE.FID != NULL); 
	} 
	return(hdr);
}

int ifclose(HDRTYPE* hdr) {
	hdr->FILE.OPEN = 0; 
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
		return(gzclose(hdr->FILE.gzFID));  
	else	
#endif
	return(fclose(hdr->FILE.FID));
}

int ifflush(HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
		return(gzflush(hdr->FILE.gzFID,Z_FINISH));  
	else	
#endif
	return(fflush(hdr->FILE.FID));
}

size_t ifread(void* ptr, size_t size, size_t nmemb, HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION>0)
		return(gzread(hdr->FILE.gzFID, ptr, size * nmemb)/size);
	else	
#endif
	return(fread(ptr, size, nmemb, hdr->FILE.FID));
}

size_t ifwrite(void* ptr, size_t size, size_t nmemb, HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
	return(gzwrite(hdr->FILE.gzFID, ptr, size*nmemb)/size);
	else	
#endif
	return(fwrite(ptr, size, nmemb, hdr->FILE.FID));
}

int ifprintf(HDRTYPE* hdr, const char *format, va_list arg) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
	return(gzprintf(hdr->FILE.gzFID,format, arg));
	else	
#endif
	return(fprintf(hdr->FILE.FID,format, arg));
}


int ifputc(int c, HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
	return(gzputc(hdr->FILE.FID,c));
	else	
#endif
	return(fputc(c,hdr->FILE.FID));
}

int ifgetc(HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
	return(gzgetc(hdr->FILE.gzFID));
	else	
#endif
	return(fgetc(hdr->FILE.FID));
}

char* ifgets(char *str, int n, HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
	return(gzgets(hdr->FILE.gzFID, str, n));
	else	
#endif
	return(fgets(str,n,hdr->FILE.FID));
}

int ifseek(HDRTYPE* hdr, long offset, int whence) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION) {
	if (whence==SEEK_END)
		fprintf(stdout,"Warning SEEK_END is not supported but used in gzseek/ifseek.\nThis can cause undefined behaviour.\n");
	return(gzseek(hdr->FILE.gzFID,offset,whence));
	} else	
#endif
	return(fseek(hdr->FILE.FID,offset,whence));
}

long int iftell(HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
	return(gztell(hdr->FILE.gzFID));
	else	
#endif
	return(ftell(hdr->FILE.FID));
}

int ifgetpos(HDRTYPE* hdr, fpos_t *pos) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION) {
		z_off_t p = gztell(hdr->FILE.gzFID);
		if (p<0) return(-1); 
		else {
#if __sparc__ || __APPLE__
			*pos = p;
#else
			pos->__pos = p;	// ugly hack but working
#endif
			return(0);
		}	
	} else	
#endif
	return(fgetpos(hdr->FILE.FID, pos));
}

int ifeof(HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
	return(gzeof(hdr->FILE.gzFID));
	else	
#endif
	return(feof(hdr->FILE.FID));
}

int iferror(HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION) {
		const char *tmp = gzerror(hdr->FILE.gzFID,&errnum);
		fprintf(stderr,"GZERROR: %i %s \n",errnum, tmp);
		return(errnum);
	}
	else	
#endif
	return(ferror(hdr->FILE.FID));
}


/*------------------------------------------------------------------------
	write GDF event table   
	utility function for SCLOSE and SFLUSH_GDF_EVENT_TABLE                                           
  ------------------------------------------------------------------------*/
void write_gdf_eventtable(HDRTYPE *hdr) 
{
	uint32_t	k32u; 
	uint8_t 	buf[88]; 
	char flag; 

	ifseek(hdr, hdr->HeadLen + hdr->AS.bpb*hdr->NRec, SEEK_SET); 
	flag = (hdr->EVENT.DUR != NULL) & (hdr->EVENT.CHN != NULL); 
	if (flag)   // any DUR or CHN is larger than 0 
		for (k32u=0, flag=0; (k32u<hdr->EVENT.N) & !flag; k32u++)
			flag |= hdr->EVENT.CHN[k32u] | hdr->EVENT.DUR[k32u];

	buf[0] = (flag ? 3 : 1);
	if (hdr->VERSION < 1.94) {
		k32u   = lround(hdr->EVENT.SampleRate); 
		buf[1] =  k32u      & 0x000000FF;
		buf[2] = (k32u>>8 ) & 0x000000FF;
		buf[3] = (k32u>>16) & 0x000000FF;
		*(uint32_t*)(buf+4) = l_endian_u32(hdr->EVENT.N);
	}
	else {
		k32u   = hdr->EVENT.N; 
		buf[1] =  k32u      & 0x000000FF;
		buf[2] = (k32u>>8 ) & 0x000000FF;
		buf[3] = (k32u>>16) & 0x000000FF;
		*(float*)(buf+4) = l_endian_f32(hdr->EVENT.SampleRate);
	};
	for (k32u=0; k32u<hdr->EVENT.N; k32u++) {
		hdr->EVENT.POS[k32u] = l_endian_u32(hdr->EVENT.POS[k32u]); 
		hdr->EVENT.TYP[k32u] = l_endian_u16(hdr->EVENT.TYP[k32u]); 
	}
	ifwrite(buf, 8, 1, hdr);
	ifwrite(hdr->EVENT.POS, sizeof(*hdr->EVENT.POS), hdr->EVENT.N, hdr);
	ifwrite(hdr->EVENT.TYP, sizeof(*hdr->EVENT.TYP), hdr->EVENT.N, hdr);
	if (buf[0]>1) {
		for (k32u=0; k32u<hdr->EVENT.N; k32u++) {
			hdr->EVENT.DUR[k32u] = l_endian_u32(hdr->EVENT.DUR[k32u]); 
			hdr->EVENT.CHN[k32u] = l_endian_u16(hdr->EVENT.CHN[k32u]); 
		}
		ifwrite(hdr->EVENT.CHN, sizeof(*hdr->EVENT.CHN), hdr->EVENT.N,hdr);
		ifwrite(hdr->EVENT.DUR, sizeof(*hdr->EVENT.DUR), hdr->EVENT.N,hdr);
	}	
}



/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/


/****************************************************************************/
/**                     INIT HDR                                           **/
/****************************************************************************/
#define Header1 ((char*)hdr->AS.Header) 

HDRTYPE* constructHDR(const unsigned NS, const unsigned N_EVENT)
{
/*
	HDR is initialized, memory is allocated for 
	NS channels and N_EVENT number of events. 

	The purpose is to set the define all parameters at an initial step. 
	No parameters must remain undefined.
 */
	HDRTYPE* hdr = (HDRTYPE*)malloc(sizeof(HDRTYPE));

	union {
		uint32_t testword;
		uint8_t  testbyte[sizeof(uint32_t)];
	} EndianTest; 
    	int k,k1;

	EndianTest.testword = 0x4a3b2c1d; 
	hdr->FILE.LittleEndian = (EndianTest.testbyte[0]==0x1d); 
	
	if  ((hdr->FILE.LittleEndian) && (__BYTE_ORDER == __BIG_ENDIAN))	{
		B4C_ERRNUM = B4C_ENDIAN_PROBLEM;
		B4C_ERRMSG = "Panic: mixed results on Endianity test.";
		exit(-1); 
	}		
	if  ((!hdr->FILE.LittleEndian) && (__BYTE_ORDER == __LITTLE_ENDIAN))	{
		B4C_ERRNUM = B4C_ENDIAN_PROBLEM;
		B4C_ERRMSG = "Panic: mixed results on Endianity test.";
		exit(-1); 
	}	

	hdr->FILE.OPEN = 0;
	hdr->FILE.FID = 0;

	hdr->AS.Header = NULL;

      	hdr->TYPE = noFile; 
      	hdr->VERSION = 2.0;
      	hdr->AS.rawdata = (uint8_t*) malloc(0);
      	hdr->NRec = 0; 
      	hdr->NS = NS;	
	hdr->SampleRate = 4321.5;
      	hdr->Patient.Id[0]=0;
      	strcpy(hdr->ID.Recording,"00000000"); 
	hdr->AS.bi = (uint32_t*)calloc(hdr->NS+1,sizeof(uint32_t));
	hdr->data.size[0] = 0; 	// rows 
	hdr->data.size[1] = 0;  // columns 
	hdr->data.block = (biosig_data_type*)malloc(0); 
      	hdr->T0 = t_time2gdf_time(time(NULL));
      	hdr->tzmin = 0; 
      	hdr->ID.Equipment = *(uint64_t*)&"b4c_0.62";
      	hdr->ID.Manufacturer._field[0]    = 0;
      	hdr->ID.Manufacturer.Name         = " ";
      	hdr->ID.Manufacturer.Model        = " ";
      	hdr->ID.Manufacturer.Version      = " ";
      	hdr->ID.Manufacturer.SerialNumber = " ";

	hdr->Patient.Name[0] 	= 0; 
	//hdr->Patient.Name 	= NULL; 
	//hdr->Patient.Id[0] 	= 0; 
	hdr->Patient.Birthday 	= (gdf_time)0;        // Unknown;
      	hdr->Patient.Medication = 0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.DrugAbuse 	= 0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.AlcoholAbuse=0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.Smoking 	= 0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.Sex 	= 0;	// 0:Unknown, 1: Male, 2: Female
      	hdr->Patient.Handedness = 0;	// 0:Unknown, 1: Right, 2: Left, 3: Equal
      	hdr->Patient.Impairment.Visual = 0;	// 0:Unknown, 1: NO, 2: YES, 3: Corrected
      	hdr->Patient.Weight 	= 0;	// 0:Unknown
      	hdr->Patient.Height 	= 0;	// 0:Unknown
	memset(&hdr->IPaddr,0,6);
      	for (k1=0; k1<3; k1++) {
      		hdr->Patient.Headsize[k1] = 0;        // Unknown;
      		hdr->ELEC.REF[k1] = 0.0;
      		hdr->ELEC.GND[k1] = 0.0;
      	}
	hdr->LOC[0] = 0x00292929; 		
	hdr->LOC[1] = 48*3600000+(1<<31); 	// latitude
	hdr->LOC[2] = 15*3600000+(1<<31); 	// longitude 
	hdr->LOC[3] = 35000; 	 	//altitude in centimeter above sea level

	hdr->FLAG.UCAL = 0; 		// un-calibration OFF (auto-scaling ON) 
	hdr->FLAG.OVERFLOWDETECTION = 1; 	// overflow detection ON
	hdr->FLAG.ANONYMOUS = 0; 	// 1: no personal names are processed 
	
       	// define variable header 
	hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	for (k=0;k<hdr->NS;k++)	{
	      	hdr->CHANNEL[k].Label[0]  = 0;
	      	hdr->CHANNEL[k].LeadIdCode= 0;
	      	strcpy(hdr->CHANNEL[k].Transducer, "EEG: Ag-AgCl electrodes");
	      	hdr->CHANNEL[k].PhysDimCode = 19+4256; // uV
	      	hdr->CHANNEL[k].PhysMax   = +100;
	      	hdr->CHANNEL[k].PhysMin   = -100;
	      	hdr->CHANNEL[k].DigMax    = +2047;
	      	hdr->CHANNEL[k].DigMin    = -2048;
	      	hdr->CHANNEL[k].GDFTYP    = 3;	// int16
	      	hdr->CHANNEL[k].SPR       = 1;	// one sample per block
	      	hdr->CHANNEL[k].OnOff     = 1;
	      	hdr->CHANNEL[k].HighPass  = 0.16;
	      	hdr->CHANNEL[k].LowPass   = 70.0;
	      	hdr->CHANNEL[k].Notch     = 50;
	      	hdr->CHANNEL[k].Impedance = INF;
	      	for (k1=0; k1<3; hdr->CHANNEL[k].XYZ[k1++] = 0.0);
	}      	

	// define EVENT structure
	hdr->EVENT.N = N_EVENT; 
	hdr->EVENT.SampleRate = 0;  
	hdr->EVENT.POS = (uint32_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.POS));
	hdr->EVENT.TYP = (uint16_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.TYP));
	hdr->EVENT.DUR = (uint32_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.DUR));
	hdr->EVENT.CHN = (uint16_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.CHN));
	
	// initialize "Annotated ECG structure"
	hdr->aECG = NULL; 
	
	return(hdr);
}


void destructHDR(HDRTYPE* hdr) {

	if (VERBOSE_LEVEL>8)  fprintf(stdout,"destructHDR: free HDR.aECG\n");

    	if (hdr->aECG != NULL)	
        	free(hdr->aECG);

	if (VERBOSE_LEVEL>8)  fprintf(stdout,"destructHDR: free HDR.AS.rawdata\n");

    	if ((hdr->AS.rawdata != NULL) && (hdr->TYPE != SCP_ECG)) 
    	{	// for SCP: hdr->AS.rawdata is part of hdr.AS.Header1 
        	free(hdr->AS.rawdata);
        }	

	if (VERBOSE_LEVEL>8)  fprintf(stdout,"destructHDR: free HDR.data.block\n");

    	if (hdr->data.block != NULL) {	
        	free(hdr->data.block);
       	}
       	hdr->data.size[0]=0;
       	hdr->data.size[1]=0;

	if (VERBOSE_LEVEL>8)  fprintf(stdout,"destructHDR: free HDR.CHANNEL[]\n");

    	if (hdr->CHANNEL != NULL)	
        	free(hdr->CHANNEL);

	if (VERBOSE_LEVEL>8)  fprintf(stdout,"destructHDR: free HDR.AS.bi\n");

    	if (hdr->AS.bi != NULL)	
        	free(hdr->AS.bi);

	if (VERBOSE_LEVEL>8)  fprintf(stdout,"destructHDR: free HDR.AS.Header\n");

    	if (hdr->AS.Header != NULL)	
        	free(hdr->AS.Header);

	if (VERBOSE_LEVEL>8)  fprintf(stdout,"destructHDR: free Event Table %p %p %p %p \n",hdr->EVENT.TYP,hdr->EVENT.POS,hdr->EVENT.DUR,hdr->EVENT.CHN);

    	if (hdr->EVENT.POS != NULL)	
        	free(hdr->EVENT.POS);
    	if (hdr->EVENT.TYP != NULL)	
        	free(hdr->EVENT.TYP);
    	if (hdr->EVENT.DUR != NULL)	
        	free(hdr->EVENT.DUR);
    	if (hdr->EVENT.CHN != NULL)	
        	free(hdr->EVENT.CHN);

	if (VERBOSE_LEVEL>8)  fprintf(stdout,"destructHDR: 09\n");
        	
        hdr->EVENT.N   = 0; 
	hdr->FILE.OPEN = 0; 	     	

	if (VERBOSE_LEVEL>8)  fprintf(stdout,"destructHDR: free HDR\n");

	free(hdr);
	return; 
}



/****************************************************************************/
/**                     GETFILETYPE                                        **/
/****************************************************************************/
HDRTYPE* getfiletype(HDRTYPE* hdr)
/*
	input:
		hdr->AS.Header1 contains first block up to 256 bytes 
		hdr->TYPE must be unknown, otherwise no FileFormat evaluation is performed
	output:
		hdr->TYPE	file format
		hdr->VERSION	is defined for some selected formats e.g. ACQ, EDF, BDF, GDF
 */
{

    	hdr->TYPE = unknown; 
		
	uint32_t U32 = leu32p(hdr->AS.Header+2); 

    	if ((U32>=30) & (U32<=42)) {
    		hdr->VERSION = (float)U32; 
    		U32 = leu32p(hdr->AS.Header+6);
    		if      ((hdr->VERSION <34.0) & (U32 == 150)) hdr->TYPE = ACQ;  
    		else if ((hdr->VERSION <35.0) & (U32 == 164)) hdr->TYPE = ACQ;  
    		else if ((hdr->VERSION <36.0) & (U32 == 326)) hdr->TYPE = ACQ;  
    		else if ((hdr->VERSION <37.0) & (U32 == 886)) hdr->TYPE = ACQ;  
    		else if ((hdr->VERSION <38.0) & (U32 ==1894)) hdr->TYPE = ACQ;  
    		else if ((hdr->VERSION <41.0) & (U32 ==1896)) hdr->TYPE = ACQ;  
    		else if ((hdr->VERSION>=41.0) & (U32 ==1944)) hdr->TYPE = ACQ;
	    	if (hdr->TYPE == ACQ) {
    			hdr->HeadLen = U32; // length of fixed header  
    			return(hdr);
    		}
    	}

 	const uint8_t MAGIC_NUMBER_FEF1[] = {67,69,78,13,10,0x1a,4,0x84};
	const uint8_t MAGIC_NUMBER_FEF2[] = {67,69,78,0x13,0x10,0x1a,4,0x84};
	const uint8_t MAGIC_NUMBER_GZIP[] = {31,139,8};
	const uint8_t MAGIC_NUMBER_Z[]    = {31,157,144};
	// const uint8_t MAGIC_NUMBER_ZIP[]  = {80,75,3,4};
	const uint8_t MAGIC_NUMBER_TIFF_l32[] = {73,73,42,0};
	const uint8_t MAGIC_NUMBER_TIFF_b32[] = {77,77,0,42};
	const uint8_t MAGIC_NUMBER_TIFF_l64[] = {73,73,43,0,8,0,0,0};
	const uint8_t MAGIC_NUMBER_TIFF_b64[] = {77,77,0,43,0,8,0,0};
	const uint8_t MAGIC_NUMBER_DICOM[]    = {8,0,5,0,10,0,0,0,73,83,79,95,73,82,32,49,48,48};
	uint32_t MAGIC_EN1064_Section0Length  = leu32p(hdr->AS.Header+10);
	
	/* AINF */
	int k;
	for (k = strlen(hdr->FileName); --k>0; ) {
		if (hdr->FileName[k] == '.') {
			if (!strcmp(hdr->FileName+k+1, "ainf")) {
				char* AINF_RAW_FILENAME = (char*)calloc(strlen(hdr->FileName)+5,sizeof(char));
				strncpy(AINF_RAW_FILENAME, hdr->FileName,k+1);
				strcpy(AINF_RAW_FILENAME+k+1, "raw");
				FILE* fid1=fopen(AINF_RAW_FILENAME,"r");
				if (fid1) {
					fclose(fid1);
					hdr->TYPE = AINF; 
				}	
				free(AINF_RAW_FILENAME);
			}
			else if (!strcmp(hdr->FileName+k+1, "raw") && !(*(uint32_t*)hdr->AS.Header)) {
				char* AINF_RAW_FILENAME = (char*)calloc(strlen(hdr->FileName)+5,sizeof(char));
				strncpy(AINF_RAW_FILENAME, hdr->FileName,k+1);
				strcpy(AINF_RAW_FILENAME+k+1, "ainf");
				FILE* fid1=fopen(AINF_RAW_FILENAME,"r");
				if (fid1) {
					fclose(fid1);
					hdr->TYPE = AINF; 
				}	
				free(AINF_RAW_FILENAME);
			}
			k = 0;  // stop loop 
		}
	}


    	if (hdr->TYPE != unknown)
      		return(hdr); 
	else if (beu32p(hdr->AS.Header) == 0x41424620) {
    	// else if (!memcmp(Header1,"ABF \x66\x66\xE6\x3F",4)) { // ABF v1.8
	    	hdr->TYPE = ABF;
    		hdr->VERSION = lef32p(hdr->AS.Header+4);
    	}	
	//else if (!memcmp(Header1,"ABF2",4)) {
	else if (beu32p(hdr->AS.Header) == 0x41424632) {
	    	hdr->TYPE = ABF;
    		hdr->VERSION = beu32p(hdr->AS.Header+4);
    	}	
    	else if (!memcmp(Header1+20,"ACR-NEMA",8))
	    	hdr->TYPE = ACR_NEMA;
    	else if (!memcmp(Header1,"ATES MEDICA SOFT. EEG for Windows",33))
	    	hdr->TYPE = ATES;
    	else if (!memcmp(Header1,"ATF\x09",4))
    	        	hdr->TYPE = ATF;

    	else if (!memcmp(Header1,"HeaderLen=",10)) {
	    	hdr->TYPE = BCI2000;
	    	hdr->VERSION = 1;
	}    	
    	else if (!memcmp(Header1,"BCI2000",7)) {
	    	hdr->TYPE = BCI2000;
	    	hdr->VERSION = 1.1;
	}    	
    	else if (!memcmp(Header1+1,"BIOSEMI",7) && (hdr->AS.Header[0]==0xff)) {
    		hdr->TYPE = BDF;
    		hdr->VERSION = -1; 
    	}
    	else if ((leu16p(hdr->AS.Header)==207) && (leu16p(hdr->AS.Header+154)==0))
	    	hdr->TYPE = BKR;
    	else if (!memcmp(Header1+34,"BLSC",4))
	    	hdr->TYPE = BLSC;
        else if (!memcmp(Header1,"Brain Vision Data Exchange Header File",38))
                hdr->TYPE = BrainVision;
    	else if (!memcmp(Header1,"BZh91",5))
	    	hdr->TYPE = BZ2;
    	else if (!memcmp(Header1,"CDF",3))
	    	hdr->TYPE = CDF; 
    	else if (!memcmp(Header1,"CFWB\1\0\0\0",8))
	    	hdr->TYPE = CFWB;
    	else if (!memcmp(Header1,"Version 3.0",11))
	    	hdr->TYPE = CNT;

    	else if (!memcmp(Header1,"DEMG",4))
	    	hdr->TYPE = DEMG;
    	else if (!memcmp(Header1+128,"DICM",4))
	    	hdr->TYPE = DICOM;
    	else if (!memcmp(Header1, MAGIC_NUMBER_DICOM,18))
	    	hdr->TYPE = DICOM;
    	else if (!memcmp(Header1+12, MAGIC_NUMBER_DICOM,18))
	    	hdr->TYPE = DICOM;
    	else if (!memcmp(Header1+12, MAGIC_NUMBER_DICOM,8))
	    	hdr->TYPE = DICOM;

    	else if (!memcmp(Header1,"0       ",8)) {
	    	hdr->TYPE = EDF;
	    	hdr->VERSION = 0; 
	}
    	else if ((beu32p(hdr->AS.Header) > 1) && (beu32p(hdr->AS.Header) < 8)) {
	    	hdr->TYPE = EGI;
	    	hdr->VERSION = hdr->AS.Header[3];
    	}
    	else if (*(uint32_t*)(Header1) == b_endian_u32(0x7f454c46))
	    	hdr->TYPE = ELF;
    	else if (!memcmp(Header1,"Header\r\nFile Version'",20))
	    	hdr->TYPE = ETG4000;

    	else if (!memcmp(Header1,MAGIC_NUMBER_FEF1,8) | !memcmp(Header1,MAGIC_NUMBER_FEF2,8)) {
	    	hdr->TYPE = FEF;
		char tmp[9];
		strncpy(tmp,(char*)hdr->AS.Header+8,8);
		hdr->VERSION = atof(tmp);
    	}
    	else if (!memcmp(Header1,"fLaC",4))
	    	hdr->TYPE = FLAC;
    	else if (!memcmp(Header1,"GDF",3))
	    	hdr->TYPE = GDF; 
    	else if (!memcmp(Header1,"GIF87a",6))
	    	hdr->TYPE = GIF; 
    	else if (!memcmp(Header1,"GIF89a",6))
	    	hdr->TYPE = GIF; 
    	else if (!memcmp(Header1,"GALILEO EEG TRACE FILE",22))
	    	hdr->TYPE = GTF; 
	else if (!memcmp(Header1,MAGIC_NUMBER_GZIP,3))  {
		hdr->TYPE = GZIP;
//		hdr->FILE.COMPRESSION = 1; 
	}	
    	else if (!memcmp(Header1,"\x89HDF",4))
	    	hdr->TYPE = HDF; 
    	else if (!memcmp(Header1,"@  MFER ",8))
	    	hdr->TYPE = MFER;
    	else if (!memcmp(Header1,"@ MFR ",6))
	    	hdr->TYPE = MFER;
/*    	else if (!memcmp(Header1,"MThd\000\000\000\001\000",9))
	    	hdr->TYPE = MIDI;
*/
    	else if (!memcmp(Header1,"NEX1",4))
	    	hdr->TYPE = NEX1;
    	else if (!memcmp(Header1,"OggS",4))
	    	hdr->TYPE = OGG;
    	else if (!memcmp(Header1,"PLEX",4))
	    	hdr->TYPE = PLEXON;
    	else if (!memcmp(Header1,"RIFF",4)) {
	    	hdr->TYPE = RIFF;
	    	if (!memcmp(Header1+8,"WAVE",4))
	    		hdr->TYPE = WAV;
	    	if (!memcmp(Header1+8,"AIF",3))
	    		hdr->TYPE = AIFF;
	    	if (!memcmp(Header1+8,"AVI ",4))
	    		hdr->TYPE = AVI;
	}    	
	
	// general SCP 
	else if (  ( MAGIC_EN1064_Section0Length    >  120)   
		&& ( MAGIC_EN1064_Section0Length    <  250)   
		&& ((MAGIC_EN1064_Section0Length%10)== 6)   
		&& (*(uint16_t*)(hdr->AS.Header+ 8) == 0x0000)
		&& (leu32p(hdr->AS.Header+10) == leu32p(hdr->AS.Header+24))
		&& (  (!memcmp(hdr->AS.Header+16,"SCPECG\0\0",8)) 
		   || (*(uint64_t*)(hdr->AS.Header+16) == 0)
		   )
		&& (leu32p(hdr->AS.Header+28) == (uint32_t)0x00000007)
		&& (leu16p(hdr->AS.Header+32) == (uint16_t)0x0001)
		) {
	    	hdr->TYPE = SCP_ECG;
	    	hdr->VERSION = *(hdr->AS.Header+14)/10.0;
	}    	
	// special SCP files - header is strange, files can be decoded 	
	else if (  (leu32p(hdr->AS.Header+10) == 136)
		&& (*(uint16_t*)(hdr->AS.Header+ 8) == 0x0000)
		&& (  (!memcmp(hdr->AS.Header+14,"\x0A\x01\x25\x01\x99\x01\xE7\x49\0\0",10)) 
		   || (!memcmp(hdr->AS.Header+14,"\x0A\x00\x90\x80\0\0\x78\x80\0\0",10)) 
		   || (!memcmp(hdr->AS.Header+14,"\x0A\xCD\xCD\xCD\xCD\xCD\xCD\xCD\0\0",10))
		   ) 
		&& (leu32p(hdr->AS.Header+24) == 136)
		&& (leu32p(hdr->AS.Header+28) == 0x0007)
		&& (leu16p(hdr->AS.Header+32) == 0x0001)
		)  {
	    	hdr->TYPE = SCP_ECG;
	    	hdr->VERSION = -2;
	}    	
	else if (  (leu32p(hdr->AS.Header+10)       == 136)
		&& (*(uint16_t*)(hdr->AS.Header+ 8) == 0x0000)
		&& (*(uint8_t*) (hdr->AS.Header+14) == 0x0A)
		&& (*(uint8_t*) (hdr->AS.Header+15) == 0x0B)
		&& (*(uint32_t*)(hdr->AS.Header+16) == 0)
		&& (*(uint32_t*)(hdr->AS.Header+20) == 0)
		&& (*(uint32_t*)(hdr->AS.Header+24) == 0)
		&& (*(uint32_t*)(hdr->AS.Header+28) == 0)
		&& (leu16p(hdr->AS.Header+32)       == 0x0001)
		) {
	    	hdr->TYPE = SCP_ECG;
	    	hdr->VERSION = -3;
	}    	

	/*
	// special SCP files - header is strange, files cannot be decoded 
	else if (  (leu32p(hdr->AS.Header+10) == 136)
		&& (*(uint16_t*)(hdr->AS.Header+ 8) == 0x0000)
		&& (leu16p(hdr->AS.Header+14) == 0x0b0b)
		&& (!memcmp(hdr->AS.Header+16,"x06SCPECG",7)) 
		)  {  
	    	hdr->TYPE = SCP_ECG;
	    	hdr->VERSION = -1;
	}    	
	else if (  (leu32p(hdr->AS.Header+10) == 136)
		&& (*(uint16_t*)(hdr->AS.Header+ 8) == 0x0000)
		&& (leu16p(hdr->AS.Header+14) == 0x0d0d)
		&& (!memcmp(hdr->AS.Header+16,"SCPEGC\0\0",8)) 
		&& (leu32p(hdr->AS.Header+24) == 136)
		&& (leu32p(hdr->AS.Header+28) == 0x0007)
		&& (leu16p(hdr->AS.Header+32) == 0x0001)
		)  {
	    	hdr->TYPE = SCP_ECG;
	    	hdr->VERSION = -4;
	}    	
	*/

	else if (!memcmp(Header1,"IAvSFo",6))
		hdr->TYPE = SIGIF;
	else if (!memcmp(Header1,"\"Snap-Master Data File\"",24))
	    	hdr->TYPE = SMA;
	else if (!memcmp(Header1,".snd",5))
		hdr->TYPE = SND;                    
	else if (!memcmp(Header1,".snd",5))
		hdr->TYPE = SND;                    

	else if (!memcmp(Header1,"POLY SAMPLE FILEversion ",24) && !memcmp(Header1+28, "\x0d\x0a\x1a",3))
		hdr->TYPE = TMS32;
	else if (!memcmp(Header1,MAGIC_NUMBER_TIFF_l32,4))
		hdr->TYPE = TIFF;
	else if (!memcmp(Header1,MAGIC_NUMBER_TIFF_b32,4))
		hdr->TYPE = TIFF;
	else if (!memcmp(Header1,MAGIC_NUMBER_TIFF_l64,8))
		hdr->TYPE = TIFF;
	else if (!memcmp(Header1,MAGIC_NUMBER_TIFF_b64,8))
		hdr->TYPE = TIFF;
	else if (!memcmp(Header1,"#VRML",5))
		hdr->TYPE = VRML;                    
	else if (!memcmp(Header1,"# vtk DataFile Version ",23)) {
		hdr->TYPE = VTK;
		char tmp[4];
		strncpy(tmp,(char*)Header1+23,3);
		hdr->VERSION = atof(tmp);
	}	
	else if (!memcmp(Header1,MAGIC_NUMBER_Z,3))
		hdr->TYPE = Z;
	else if (!strncmp(Header1,"PK\003\004",4))
		hdr->TYPE = ZIP;
	else if (!strncmp(Header1,"PK\005\006",4))
		hdr->TYPE = ZIP;
	else if (!strncmp(Header1,"ZIP2",4))
		hdr->TYPE = ZIP2;
	else if (!memcmp(Header1,"<?xml version",13))
		hdr->TYPE = HL7aECG;
	else if ( (leu32p(hdr->AS.Header) & 0x00FFFFFFL) == 0x00BFBBEFL  
		&& !memcmp(Header1+3,"<?xml version",13))
		hdr->TYPE = HL7aECG;	// UTF8
	else if (leu16p(hdr->AS.Header)==0xFFFE) 
	{	hdr->TYPE = XML; // UTF16 BigEndian 
		hdr->FILE.LittleEndian = 0;
    	}
	else if (leu16p(hdr->AS.Header)==0xFEFF) 
	{	hdr->TYPE = XML; // UTF16 LittleEndian 
		hdr->FILE.LittleEndian = 1;
    	}
	return(hdr); 
}



/* ------------------------------------------
 *   	returns string of file type 	
 * ------------------------------------------- */
const char* GetFileTypeString(enum FileFormat FMT) {
	const char *FileType;
		
	switch (FMT) {
	case noFile: 	{ FileType = NULL; break; }
	case unknown: 	{ FileType = "unknown"; break; }
	
	case ABF: 	{ FileType = "ABF"; break; }
	case ACQ: 	{ FileType = "ACQ"; break; }
	case ACR_NEMA: 	{ FileType = "ACR_NEMA"; break; }
	case AINF: 	{ FileType = "AINF"; break; }
	case AIFC: 	{ FileType = "AIFC"; break; }
	case AIFF: 	{ FileType = "AIFF"; break; }
	case ATES: 	{ FileType = "ATES"; break; }
	case ATF: 	{ FileType = "ATF"; break; }
	case AU: 	{ FileType = "AU"; break; }

	case BCI2000: 	{ FileType = "BCI2000"; break; }
	case BDF: 	{ FileType = "BDF"; break; }
	case BKR: 	{ FileType = "BKR"; break; }
	case BLSC: 	{ FileType = "BLSC"; break; }
	case BMP: 	{ FileType = "BMP"; break; }
	case BrainVision: 	{ FileType = "BrainVision"; break; }
	case BZ2: 	{ FileType = "BZ2"; break; }

	case CDF: 	{ FileType = "CDF"; break; }
	case DEMG: 	{ FileType = "DEMG"; break; }
	case CFWB: 	{ FileType = "CFWB"; break; }
	case CNT: 	{ FileType = "CNT"; break; }
	case DICOM: 	{ FileType = "DICOM"; break; }

	case EDF: 	{ FileType = "EDF"; break; }
	case EEProbe: 	{ FileType = "EEProbe"; break; }
	case EGI: 	{ FileType = "EGI"; break; }
	case ELF: 	{ FileType = "ELF"; break; }
	case ETG4000: 	{ FileType = "ETG4000"; break; }
	case EXIF: 	{ FileType = "EXIF"; break; }

	case FAMOS: 	{ FileType = "FAMOS"; break; }
	case FEF: 	{ FileType = "FEF"; break; }
	case FITS: 	{ FileType = "FITS"; break; }
	case FLAC: 	{ FileType = "FLAC"; break; }

	case GDF: 	{ FileType = "GDF"; break; }
	case GIF: 	{ FileType = "GIF"; break; }
	case GTF: 	{ FileType = "GTF"; break; }
	case GZIP: 	{ FileType = "GZIP"; break; }
	case HDF: 	{ FileType = "HDF"; break; }
	case HL7aECG: 	{ FileType = "HL7aECG"; break; }
	case JPEG: 	{ FileType = "JPEG"; break; }

	case Matlab: 	{ FileType = "MAT"; break; }
	case MFER: 	{ FileType = "MFER"; break; }
	case MIDI: 	{ FileType = "MIDI"; break; }
	case NetCDF: 	{ FileType = "NetCDF"; break; }
	case NEX1: 	{ FileType = "NEX1"; break; }
	case OGG: 	{ FileType = "OGG"; break; }

	case RIFF: 	{ FileType = "RIFF"; break; }
	case SCP_ECG: 	{ FileType = "SCP"; break; }
	case SIGIF: 	{ FileType = "SIGIF"; break; }
	case SMA: 	{ FileType = "SMA"; break; }
	case SND: 	{ FileType = "SND"; break; }
	case SVG: 	{ FileType = "SVG"; break; }

	case TIFF: 	{ FileType = "TIFF"; break; }
	case TMS32: 	{ FileType = "TMS32"; break; }
	case VRML: 	{ FileType = "VRML"; break; }
	case VTK: 	{ FileType = "VTK"; break; }
	case WAV: 	{ FileType = "WAV"; break; }

	case WMF: 	{ FileType = "WMF"; break; }
	case XML: 	{ FileType = "XML"; break; }
	case ZIP: 	{ FileType = "ZIP"; break; }
	case ZIP2: 	{ FileType = "ZIP2"; break; }
	case Z: 	{ FileType = "Z"; break; }
	default: 	  FileType = "unknown";
	}
	return(FileType); 
}


/****************************************************************************/
/**                     SOPEN                                              **/
/****************************************************************************/
HDRTYPE* sopen(const char* FileName, const char* MODE, HDRTYPE* hdr)
/*
	MODE="r" 
		reads file and returns HDR 
	MODE="w" 
		writes HDR into file 
 */
{
    	
    	const char*	GENDER = "XMFX";  
    	const uint16_t	CFWB_GDFTYP[] = {17,16,3};  
	const float	CNT_SETTINGS_NOTCH[] = {0.0, 50.0, 60.0}; 
	const float	CNT_SETTINGS_LOWPASS[] = {30, 40, 50, 70, 100, 200, 500, 1000, 1500, 2000, 2500, 3000};
	const float	CNT_SETTINGS_HIGHPASS[] = {NaN, 0, .05, .1, .15, .3, 1, 5, 10, 30, 100, 150, 300};

    	unsigned int 	k,k1;
    	uint32_t	k32u; 
    	size_t	 	count;
    	char 		tmp[81];
//    	double 		Dur; 
	char*		ptr_str;
	struct tm 	tm_time; 
	time_t		tt;
	uint16_t	EGI_LENGTH_CODETABLE=0;	// specific for EGI format 

	if (hdr==NULL)
		hdr = constructHDR(0,0);	// initializes fields that may stay undefined during SOPEN 


hdr->FLAG.SWAP = (__BYTE_ORDER == __BIG_ENDIAN); 	// default: most data formats are little endian 
if (!strncmp(MODE,"r",1))	
{
 	hdr->AS.Header = (uint8_t*)malloc(257);
    	Header1[256] = 0;
	hdr->TYPE = noFile; 

	hdr->FileName = FileName; 
        hdr->FILE.COMPRESSION = 0;   	
        hdr = ifopen(hdr,"rb");
    	if (!hdr->FILE.OPEN) { 	
    		B4C_ERRNUM = B4C_CANNOT_OPEN_FILE;
    		B4C_ERRMSG = "Error SOPEN(READ); Cannot open file.";		
    		return(hdr); 
    	}	    
    
    	/******** read 1st (fixed)  header  *******/	
    	count = ifread(Header1,1,256,hdr);
	hdr   = getfiletype(hdr);
    	
	if (VERBOSE_LEVEL==9) 
		fprintf(stdout,"File Format %i\n",hdr->TYPE); 

    	if (hdr->TYPE == GZIP) {
#ifdef ZLIB_H
    		ifclose(hdr); 
	        hdr->FILE.COMPRESSION = 1;   	
	        // hdr->FILE.gzFID = gzdopen(hdr->FILE.FID,"rb"); // FIXME
        	hdr= ifopen(hdr,"rb");
	    	if (!hdr->FILE.OPEN) { 	
    			B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	    		B4C_ERRMSG = "Error SOPEN(GZREAD); Cannot open file.";		
	    		return(hdr);
    		}	    
	    	count = ifread(Header1,1,256,hdr);
		hdr   = getfiletype(hdr);
#else 
		B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
    		B4C_ERRMSG = "Error SOPEN(READ); *.gz file not supported because not linked with zlib.";
#endif
    	}
    	if (hdr->TYPE == unknown) {
    		B4C_ERRNUM = B4C_FORMAT_UNKNOWN;
    		B4C_ERRMSG = "ERROR BIOSIG SOPEN(read): Dataformat Format not known.\n";
    		ifclose(hdr);
		return(hdr);
	}	

	if (VERBOSE_LEVEL>8) fprintf(stdout,"[201] GDF=%i %i Ver=%4.2f\n",GDF,hdr->TYPE,hdr->VERSION);

	if (hdr->TYPE == GDF) {
		uint32_t Dur[2];
      	    	strncpy(tmp,(char*)hdr->AS.Header+3,5); tmp[5]=0;
	    	hdr->VERSION 	= atof(tmp);
	    	hdr->NRec 	= lei64p(hdr->AS.Header+236); 
	    	Dur[0]  	= leu32p(hdr->AS.Header+244);
	    	Dur[1]  	= leu32p(hdr->AS.Header+248); 
	    	hdr->NS   	= leu16p(hdr->AS.Header+252); 
	    	
	    	if (hdr->VERSION > 1.90) { 
		    	hdr->HeadLen 	= leu16p(hdr->AS.Header+184)<<8; 
	    		strncpy(hdr->Patient.Id,(const char*)hdr->AS.Header+8,min(66,MAX_LENGTH_PID));
	    		strncpy(hdr->ID.Recording,(const char*)hdr->AS.Header+88,min(80,MAX_LENGTH_RID));
	    		strtok(hdr->Patient.Id," ");
	    		char *tmpptr = strtok(NULL," ");
	    		if ((!hdr->FLAG.ANONYMOUS) && (tmpptr != NULL)) {
				// strncpy(hdr->Patient.Name,tmpptr,Header1+8-tmpptr);
		    		strncpy(hdr->Patient.Name,tmpptr,MAX_LENGTH_NAME);
		    	}	
		    		
	    		hdr->Patient.Smoking      =  Header1[84]%4;
	    		hdr->Patient.AlcoholAbuse = (Header1[84]>>2)%4;
	    		hdr->Patient.DrugAbuse    = (Header1[84]>>4)%4;
	    		hdr->Patient.Medication   = (Header1[84]>>6)%4;
	    		hdr->Patient.Weight       =  Header1[85];
	    		hdr->Patient.Height       =  Header1[86];
	    		hdr->Patient.Sex       	  =  Header1[87]%4;
	    		hdr->Patient.Handedness   = (Header1[87]>>2)%4;
	    		hdr->Patient.Impairment.Visual = (Header1[87]>>4)%4;
	
			*(uint32_t*)(hdr->AS.Header+156) = leu32p(hdr->AS.Header+156);
			*(uint32_t*)(hdr->AS.Header+160) = leu32p(hdr->AS.Header+160);
			*(uint32_t*)(hdr->AS.Header+164) = leu32p(hdr->AS.Header+164);
			if (hdr->AS.Header[156]) {
				hdr->LOC[0] = 0x00292929;
				memcpy(&hdr->LOC[1], hdr->AS.Header+156, 12);
			}
			else {
				*(uint32_t*) (hdr->AS.Header+152) = leu32p(hdr->AS.Header+152);
				memcpy(&hdr->LOC, hdr->AS.Header+152, 16);
			}

			hdr->T0 		= lei64p(hdr->AS.Header+168);
			hdr->Patient.Birthday 	= lei64p(hdr->AS.Header+176);
			// memcpy(&hdr->T0, Header1+168,8);
			// memcpy(&hdr->Patient.Birthday, Header1+176, 8);

			hdr->ID.Equipment 	= lei64p(hdr->AS.Header+192);
			memcpy(&hdr->IPaddr, Header1+200,6);
			hdr->Patient.Headsize[0]= leu16p(hdr->AS.Header+206);
			hdr->Patient.Headsize[1]= leu16p(hdr->AS.Header+208);
			hdr->Patient.Headsize[2]= leu16p(hdr->AS.Header+210);

			//memcpy(&hdr->ELEC.REF, Header1+212,12);
			//memcpy(&hdr->ELEC.GND, Header1+224,12);
			hdr->ELEC.REF[0]   = lef32p(hdr->AS.Header+212);
			hdr->ELEC.REF[1]   = lef32p(hdr->AS.Header+216);
			hdr->ELEC.REF[2]   = lef32p(hdr->AS.Header+220);
			hdr->ELEC.GND[0]   = lef32p(hdr->AS.Header+212);
			hdr->ELEC.GND[1]   = lef32p(hdr->AS.Header+216);
			hdr->ELEC.GND[2]   = lef32p(hdr->AS.Header+220);
		    	if (hdr->VERSION > 100000.0) {
		    		fprintf(stdout,"%e \nb4c %c %i %c. %c%c%c%c%c%c%c\n",hdr->VERSION,169,2007,65,83,99,104,108,246,103,108);
		    		FILE *fid = fopen("/tmp/b4c_tmp","wb");
		    		if (fid != NULL)	{
			    		fprintf(fid,"\nb4c %f \n%c %i %c.%c%c%c%c%c%c%c\n",hdr->VERSION,169,2007,65,83,99,104,108,246,103,108);
			    		fclose(fid);
			    	}
		    	}
	    	}
	    	else if (hdr->VERSION > 0.0) {
	    		strncpy(hdr->Patient.Id,Header1+8,min(80,MAX_LENGTH_PID));
	    		hdr->Patient.Id[min(80,MAX_LENGTH_PID)] = 0;
			strncpy(hdr->ID.Recording,(const char*)Header1+88,min(80,MAX_LENGTH_RID));
	    		hdr->ID.Recording[min(80,MAX_LENGTH_RID)] = 0;
	    		strtok(hdr->Patient.Id," ");
	    		char *tmpptr = strtok(NULL," ");
	    		if ((!hdr->FLAG.ANONYMOUS) && (tmpptr != NULL)) {
				// strncpy(hdr->Patient.Name,tmpptr,Header1+8-tmpptr);
		    		strncpy(hdr->Patient.Name,tmpptr,MAX_LENGTH_NAME);
		    	}	

			memset(tmp,0,5);
			strncpy(tmp,Header1+168+12,2); 	    		 
	    		tm_time.tm_sec  = atoi(tmp); 
			strncpy(tmp,Header1+168+10,2);	    		 
	    		tm_time.tm_min  = atoi(tmp); 
			strncpy(tmp,Header1+168+ 8,2);	    		 
	    		tm_time.tm_hour = atoi(tmp); 
			strncpy(tmp,Header1+168+ 6,2);	    		 
	    		tm_time.tm_mday = atoi(tmp); 
			strncpy(tmp,Header1+168+ 4,2);	    		 
	    		tm_time.tm_mon  = atoi(tmp)-1;
			strncpy(tmp,Header1+168   ,4);	    		 
	    		tm_time.tm_year = atoi(tmp)-1900; 
	    		tm_time.tm_isdst= -1; 

			hdr->T0 = t_time2gdf_time(mktime(&tm_time)); 
		    	hdr->HeadLen 	= leu64p(hdr->AS.Header+184); 
	    	}

	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	    	hdr->AS.Header = (uint8_t*)realloc(Header1,hdr->HeadLen);
//	    	Header1 = (char*)hdr->AS.Header1; 
	    	uint8_t *Header2 = hdr->AS.Header+256; 
	    	count   += ifread(Header2, 1, hdr->HeadLen-256, hdr);

		for (k=0; k<hdr->NS; k++)	{

			if (VERBOSE_LEVEL>8) fprintf(stdout,"[GDF 211] #=%i/%i\n",k,hdr->NS);

			strncpy(hdr->CHANNEL[k].Label,(char*)Header2 + 16*k,min(16,MAX_LENGTH_LABEL));
			strncpy(hdr->CHANNEL[k].Transducer,(char*)Header2 + 16*hdr->NS + 80*k,min(MAX_LENGTH_TRANSDUCER,80));
			
			hdr->CHANNEL[k].PhysMin = lef64p(Header2+ 8*k + 104*hdr->NS);
			hdr->CHANNEL[k].PhysMax = lef64p(Header2+ 8*k + 112*hdr->NS);

			hdr->CHANNEL[k].SPR     = leu32p(Header2+ 4*k + 216*hdr->NS);
			hdr->CHANNEL[k].GDFTYP  = leu16p(Header2+ 4*k + 220*hdr->NS);
			hdr->CHANNEL[k].OnOff   = 1;
			if (hdr->VERSION < 1.90) {
				strncpy(hdr->CHANNEL[k].PhysDim, (char*)Header2 + 8*k + 96*hdr->NS,8);
				hdr->CHANNEL[k].PhysDim[8] = 0; // remove trailing blanks
				int k1;
				for (k1=7; (k1>0) & !isalnum(hdr->CHANNEL[k].PhysDim[k1]); k1--)
					hdr->CHANNEL[k].PhysDim[k1] = 0;

				hdr->CHANNEL[k].PhysDimCode = PhysDimCode(hdr->CHANNEL[k].PhysDim);

				hdr->CHANNEL[k].DigMin   = (double) lei64p(Header2 + 8*k + 120*hdr->NS);
				hdr->CHANNEL[k].DigMax   = (double) lei64p(Header2 + 8*k + 128*hdr->NS);

				char *PreFilt  = (char*)(Header2+ 68*k + 136*hdr->NS);
				hdr->CHANNEL[k].LowPass  = NaN;
				hdr->CHANNEL[k].HighPass = NaN;
				hdr->CHANNEL[k].Notch    = NaN;
				float lf,hf;
				if (sscanf(PreFilt,"%f - %f Hz",&lf,&hf)==2) {
					hdr->CHANNEL[k].LowPass  = hf;
					hdr->CHANNEL[k].HighPass = lf;
				}
			}	
			else {
				hdr->CHANNEL[k].PhysDimCode = leu16p(Header2+ 2*k + 102*hdr->NS);
				PhysDim(hdr->CHANNEL[k].PhysDimCode,hdr->CHANNEL[k].PhysDim);

				hdr->CHANNEL[k].DigMin   = lef64p(Header2+ 8*k + 120*hdr->NS);
				hdr->CHANNEL[k].DigMax   = lef64p(Header2+ 8*k + 128*hdr->NS);

				hdr->CHANNEL[k].LowPass  = lef32p(Header2+ 4*k + 204*hdr->NS);
				hdr->CHANNEL[k].HighPass = lef32p(Header2+ 4*k + 208*hdr->NS);
				hdr->CHANNEL[k].Notch    = lef32p(Header2+ 4*k + 212*hdr->NS);
				hdr->CHANNEL[k].XYZ[0]   = lef32p(Header2+ 4*k + 224*hdr->NS);
				hdr->CHANNEL[k].XYZ[1]   = lef32p(Header2+ 4*k + 228*hdr->NS);
				hdr->CHANNEL[k].XYZ[2]   = lef32p(Header2+ 4*k + 232*hdr->NS);
				//memcpy(&hdr->CHANNEL[k].XYZ,Header2 + 4*k + 224*hdr->NS,12);
				hdr->CHANNEL[k].Impedance= ldexp(1.0, (uint8_t)Header2[k + 236*hdr->NS]/8);
			}

			hdr->CHANNEL[k].Cal   	= (hdr->CHANNEL[k].PhysMax-hdr->CHANNEL[k].PhysMin)/(hdr->CHANNEL[k].DigMax-hdr->CHANNEL[k].DigMin);
			hdr->CHANNEL[k].Off   	=  hdr->CHANNEL[k].PhysMin-hdr->CHANNEL[k].Cal*hdr->CHANNEL[k].DigMin;

		}
		for (k=0, hdr->SPR=1, hdr->AS.spb=0, hdr->AS.bpb=0; k<hdr->NS;k++) {

			if (VERBOSE_LEVEL>8) fprintf(stdout,"[GDF 212] #=%i\n",k);

			hdr->AS.spb += hdr->CHANNEL[k].SPR;
			hdr->AS.bpb += GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP]*hdr->CHANNEL[k].SPR;
			if (hdr->CHANNEL[k].SPR)
				hdr->SPR = lcm(hdr->SPR,hdr->CHANNEL[k].SPR);

			if (GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP]==0) {
				B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
				B4C_ERRMSG = "GDF: Invalid or unsupported GDFTYP";
				return(hdr);
			}
		}	
		hdr->SampleRate = ((double)(hdr->SPR))*Dur[1]/Dur[0];

		// READ EVENTTABLE 
		int c;
    		uint8_t buf[8];
		ifseek(hdr, hdr->HeadLen + hdr->AS.bpb*hdr->NRec, SEEK_SET); 
		c = ifread(buf, sizeof(uint8_t), 8, hdr);

		if (c<8) {
			hdr->EVENT.SampleRate = hdr->SampleRate; 
			hdr->EVENT.N = 0;
		}	
		else if (hdr->VERSION < 1.94) {
			if (buf[1] | buf[2] | buf[3])
				hdr->EVENT.SampleRate = buf[1] + (buf[2] + buf[3]*256.0)*256.0; 
			else {
				fprintf(stdout,"Warning GDF v1: SampleRate in Eventtable is not set in %s !!!\n",hdr->FileName);
				hdr->EVENT.SampleRate = hdr->SampleRate; 
			}	
			hdr->EVENT.N = leu32p(buf + 4);
		}	
		else	{
			hdr->EVENT.N = buf[1] + (buf[2] + buf[3]*256)*256; 
			hdr->EVENT.SampleRate = lef32p(buf + 4);
		}	

 		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, hdr->EVENT.N*sizeof(*hdr->EVENT.POS) );
		hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP) );
		ifread(hdr->EVENT.POS, sizeof(*hdr->EVENT.POS), hdr->EVENT.N, hdr);
		ifread(hdr->EVENT.TYP, sizeof(*hdr->EVENT.TYP), hdr->EVENT.N, hdr);

		for (k32u=0; k32u < hdr->EVENT.N; k32u++) {
			hdr->EVENT.POS[k32u] = l_endian_u32(hdr->EVENT.POS[k32u]); 
			hdr->EVENT.TYP[k32u] = l_endian_u16(hdr->EVENT.TYP[k32u]); 
		}
		if (buf[0]>1) {
			hdr->EVENT.DUR = (uint32_t*) realloc(hdr->EVENT.DUR,hdr->EVENT.N*sizeof(*hdr->EVENT.DUR));
			hdr->EVENT.CHN = (uint16_t*) realloc(hdr->EVENT.CHN,hdr->EVENT.N*sizeof(*hdr->EVENT.CHN));
			ifread(hdr->EVENT.CHN,sizeof(*hdr->EVENT.CHN),hdr->EVENT.N,hdr);
			ifread(hdr->EVENT.DUR,sizeof(*hdr->EVENT.DUR),hdr->EVENT.N,hdr);

			for (k32u=0; k32u<hdr->EVENT.N; k32u++) {
				hdr->EVENT.DUR[k32u] = l_endian_u32(hdr->EVENT.DUR[k32u]); 
				hdr->EVENT.CHN[k32u] = l_endian_u16(hdr->EVENT.CHN[k32u]); 
			}
		}
		else {
			hdr->EVENT.DUR = NULL;
			hdr->EVENT.CHN = NULL;
		}	
		ifseek(hdr, hdr->HeadLen, SEEK_SET); 
	    	hdr->FILE.POS = 0; 

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[GDF 217] #=%li\n",iftell(hdr));
    	}

    	else if ((hdr->TYPE == EDF) | (hdr->TYPE == BDF))	{
		size_t	EventChannel = 0;
    		strncpy(hdr->Patient.Id,Header1+8,min(MAX_LENGTH_PID,80));
    		memcpy(hdr->ID.Recording,Header1+88,min(80,MAX_LENGTH_RID));
		hdr->ID.Recording[MAX_LENGTH_RID]=0;
		
		memset(tmp,0,9); 	
		strncpy(tmp,Header1+168+14,2); 
    		tm_time.tm_sec  = atoi(tmp); 
    		strncpy(tmp,Header1+168+11,2);
    		tm_time.tm_min  = atoi(tmp);
    		strncpy(tmp,Header1+168+8,2); 
    		tm_time.tm_hour = atoi(tmp); 
    		strncpy(tmp,Header1+168,2); 
    		tm_time.tm_mday = atoi(tmp);
    		strncpy(tmp,Header1+168+3,2); 
    		tm_time.tm_mon  = atoi(tmp)-1;
    		strncpy(tmp,Header1+168+6,2); 
    		tm_time.tm_year = atoi(tmp); 
    		tm_time.tm_year+= (tm_time.tm_year < 70 ? 100 : 0);
    		
		hdr->EVENT.N 	= 0;
	    	hdr->NS		= atoi(strncpy(tmp,Header1+252,4));
	    	hdr->HeadLen	= atoi(strncpy(tmp,Header1+184,8));
	    	if (hdr->HeadLen != ((hdr->NS+1)<<8)) {
	    		B4C_ERRNUM = B4C_UNSPECIFIC_ERROR;
	    		B4C_ERRMSG = "EDF/BDF corrupted: HDR.NS and HDR.HeadLen do not fit";
	    	};	

	    	hdr->NRec	= atoi(strncpy(tmp,Header1+236,8));
	    	//Dur		= atof(strncpy(tmp,Header1+244,8));

		if (!strncmp(Header1+192,"EDF+",4)) {
	    		strtok(hdr->Patient.Id," ");
	    		ptr_str = strtok(NULL," ");
	    		hdr->Patient.Sex = (ptr_str[0]=='f')*2 + (ptr_str[0]=='F')*2 + (ptr_str[0]=='M') + (ptr_str[0]=='m');
	    		ptr_str = strtok(NULL," ");	// startdate
	    		char *tmpptr = strtok(NULL," ");
	    		if ((!hdr->FLAG.ANONYMOUS) && (tmpptr != NULL)) {
		    		strcpy(hdr->Patient.Name,tmpptr);
		    	}	
			if (strlen(ptr_str)==11) {
				struct tm t1;
		    		t1.tm_mday = atoi(strtok(ptr_str,"-")); 
		    		strcpy(tmp,strtok(NULL,"-"));
		    		for (k=0; k<strlen(tmp); ++k); tmp[k]= toupper(tmp[k]);	// convert to uppper case 
		    		t1.tm_year = atoi(strtok(NULL,"- ")) - 1900; 
		    		t1.tm_mon  = !strcmp(tmp,"FEB")+!strcmp(tmp,"MAR")*2+!strcmp(tmp,"APR")*3+!strcmp(tmp,"MAY")*4+!strcmp(tmp,"JUN")*5+!strcmp(tmp,"JUL")*6+!strcmp(tmp,"AUG")*7+!strcmp(tmp,"SEP")*8+!strcmp(tmp,"OCT")*9+!strcmp(tmp,"NOV")*10+!strcmp(tmp,"DEC")*11;
		    		t1.tm_sec  = 0;
		    		t1.tm_min  = 0;
		    		t1.tm_hour = 12;
		    		t1.tm_isdst= -1;
		    		hdr->Patient.Birthday = t_time2gdf_time(mktime(&t1));
		    	}

	    		strtok(hdr->ID.Recording," ");
	    		ptr_str = strtok(NULL," ");
	    		tm_time.tm_mday = atoi(strtok(ptr_str,"-")); 
	    		strcpy(tmp,strtok(NULL,"-"));
	    		for (k=0;k<strlen(tmp); ++k); tmp[k]= toupper(tmp[k]);	// convert to uppper case 
	    		tm_time.tm_year = atoi(strtok(NULL,"-")) - 1900; 
	    		tm_time.tm_mon  = !strcmp(tmp,"FEB")+!strcmp(tmp,"MAR")*2+!strcmp(tmp,"APR")*3+!strcmp(tmp,"MAY")*4+!strcmp(tmp,"JUN")*5+!strcmp(tmp,"JUL")*6+!strcmp(tmp,"AUG")*7+!strcmp(tmp,"SEP")*8+!strcmp(tmp,"OCT")*9+!strcmp(tmp,"NOV")*10+!strcmp(tmp,"DEC")*11;
	    		tm_time.tm_sec  = 0;
	    		tm_time.tm_min  = 0;
	    		tm_time.tm_hour = 12;
	    		tm_time.tm_isdst= -1;
		}   
		hdr->T0 = tm_time2gdf_time(&tm_time); 

	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	    	hdr->AS.Header = (uint8_t*) realloc(Header1,hdr->HeadLen);
	    	char *Header2 = (char*)hdr->AS.Header+256; 
	    	count  += ifread(Header2, 1, hdr->HeadLen-256, hdr);

		for (k=0, hdr->SPR = 1; k<hdr->NS; k++)	{
			strncpy(hdr->CHANNEL[k].Label,Header2 + 16*k,min(MAX_LENGTH_LABEL,16));
			for (k1=strlen(hdr->CHANNEL[k].Label)-1; isspace(hdr->CHANNEL[k].Label[k1]) && k1; k1--)
				hdr->CHANNEL[k].Label[k1] = 0;	// deblank

			strncpy(hdr->CHANNEL[k].Transducer, Header2+80*k+16*hdr->NS, min(80,MAX_LENGTH_TRANSDUCER));
			for (k1=strlen(hdr->CHANNEL[k].Transducer)-1; isspace(hdr->CHANNEL[k].Transducer[k1]) && k1; k1--)
				hdr->CHANNEL[k].Transducer[k1]='\0'; 	// deblank
			
			// PhysDim -> PhysDimCode 
			strncpy(hdr->CHANNEL[k].PhysDim,Header2 + 8*k + 96*hdr->NS,8);
			hdr->CHANNEL[k].PhysDim[8] = 0; // remove trailing blanks
			int k1;			
			for (k1=7; (k1>0) & !isalnum(hdr->CHANNEL[k].PhysDim[k1]); k1--)
				hdr->CHANNEL[k].PhysDim[k1] = 0;
			hdr->CHANNEL[k].PhysDimCode = PhysDimCode(hdr->CHANNEL[k].PhysDim);
			tmp[8] = 0;
			hdr->CHANNEL[k].PhysMin = atof(strncpy(tmp,Header2 + 8*k + 104*hdr->NS,8)); 
			hdr->CHANNEL[k].PhysMax = atof(strncpy(tmp,Header2 + 8*k + 112*hdr->NS,8)); 
			hdr->CHANNEL[k].DigMin  = atof(strncpy(tmp,Header2 + 8*k + 120*hdr->NS,8)); 
			hdr->CHANNEL[k].DigMax  = atof(strncpy(tmp,Header2 + 8*k + 128*hdr->NS,8)); 
			hdr->CHANNEL[k].Cal     = (hdr->CHANNEL[k].PhysMax-hdr->CHANNEL[k].PhysMin)/(hdr->CHANNEL[k].DigMax-hdr->CHANNEL[k].DigMin);
			hdr->CHANNEL[k].Off     =  hdr->CHANNEL[k].PhysMin-hdr->CHANNEL[k].Cal*hdr->CHANNEL[k].DigMin;

			hdr->CHANNEL[k].SPR     = atol(strncpy(tmp,Header2 + 8*k + 216*hdr->NS,8));
			hdr->CHANNEL[k].GDFTYP  = ((hdr->TYPE!=BDF) ? 3 : 255+24); 
			hdr->CHANNEL[k].OnOff   = 1;
			hdr->SPR 	 	= lcm(hdr->SPR,hdr->CHANNEL[k].SPR);
			
			hdr->CHANNEL[k].LowPass = NaN;
			hdr->CHANNEL[k].HighPass = NaN;
			hdr->CHANNEL[k].Notch = NaN;
			// decode filter information into hdr->Filter.{Lowpass, Highpass, Notch}					
			float lf,hf;
			char *PreFilt = (Header2+ 80*k + 136*hdr->NS);
			char s1[40],s2[40];
			PreFilt[79] = 0;
			uint16_t d, pdc;
			d = sscanf(PreFilt,"HP: %f %s LP:%f %s ",&lf,s1,&hf,s2);
			if (d==4) {
				pdc = PhysDimCode(s1);
				if ((pdc & 0xffe0) == 2496)	// Hz 
					hdr->CHANNEL[k].HighPass = lf * PhysDimScale(pdc);
				pdc = PhysDimCode(s2);
				if ((pdc & 0xffe0) == 2496)	// Hz 
					hdr->CHANNEL[k].LowPass  = hf * PhysDimScale(pdc);
			}
			else {
				d = sscanf(PreFilt,"HP: %s LP: %f %s ",s1,&hf,s2);
				if (d==3) {
					if (!strncmp(s1,"DC",2)) 
						hdr->CHANNEL[k].HighPass = 0;
					pdc = PhysDimCode(s2);
					if ((pdc & 0xffe0) == 2496)	// Hz
						hdr->CHANNEL[k].LowPass  = hf * PhysDimScale(pdc);
				}
			}
				
			if ((hdr->TYPE==EDF) && !strncmp(Header1+192,"EDF+",4) && !strcmp(hdr->CHANNEL[k].Label,"EDF Annotations")) {
				hdr->CHANNEL[k].OnOff = 0;
				EventChannel = k; 
			}	
			else if ((hdr->TYPE==BDF) && !strcmp(hdr->CHANNEL[k].Label,"Status")) {
				hdr->CHANNEL[k].OnOff = 0;
				EventChannel = k; 
			}
		}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// EDF does not support automated overflow and saturation detection
	    	double Dur	= atof(strncpy(tmp,Header1+244,8));
		hdr->SampleRate = hdr->SPR/Dur;

		if (EventChannel) {
			/* read Annotation and Status channel and extract event information */		
			hdr->AS.bi  = (uint32_t*) realloc(hdr->AS.bi,(hdr->NS+1)*sizeof(uint32_t));
			hdr->AS.bi[0] = 0;
			for (k=0, hdr->AS.spb=0, hdr->AS.bpb=0; k<hdr->NS;k++) {
				hdr->AS.spb 	+= hdr->CHANNEL[k].SPR;
				hdr->AS.bpb 	+= GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP]*hdr->CHANNEL[k].SPR;			
				hdr->AS.bi[k+1]  = hdr->AS.bpb;
			}	

			size_t sz   	= GDFTYP_BYTE[hdr->CHANNEL[EventChannel].GDFTYP];
			uint8_t *Marker = (uint8_t*)malloc(hdr->CHANNEL[EventChannel].SPR * hdr->NRec * sz);
			size_t skip 	= hdr->AS.bpb - hdr->CHANNEL[EventChannel].SPR * sz;
			ifseek(hdr, hdr->HeadLen + hdr->AS.bi[EventChannel], SEEK_SET);
			for (k=0; k<hdr->NS; k++) {
			    	ifread(Marker+k*hdr->CHANNEL[EventChannel].SPR * sz, 1, hdr->CHANNEL[EventChannel].SPR * sz, hdr);
				ifseek(hdr, skip, SEEK_CUR);
			}
			size_t len = hdr->SPR*hdr->NRec;		
			size_t N_EVENT  = 0;
			if (hdr->TYPE==EDF) {
			/* convert EDF annotation channel into event table */
				
				for (k=1; k<len; k++) {
				}

				hdr->EVENT.N = N_EVENT;
				hdr->EVENT.SampleRate = hdr->SampleRate; 
				hdr->EVENT.POS = (uint32_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.POS));
				hdr->EVENT.TYP = (uint16_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.TYP));
				hdr->EVENT.DUR = (uint32_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.DUR));
				hdr->EVENT.CHN = (uint16_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.CHN));

				for (k=1; k<len; k++) {
				}
/*
                        N  = 0; 
			[s,t] = strtok(HDR.EDF.ANNONS,0);
    			while ~isempty(s)
    				N  = N + 1; 
    				ix = find(s==20);
    				[s1,s2] = strtok(s(1:ix(1)-1),21);
    				onset(N,1) = str2double(s1);
   				tmp = str2double(s2(2:end));
   				if  ~isempty(tmp)
   					dur(N,1) = tmp; 	
   				else 
   					dur(N,1) = 0; 	
   				end;
    				TeegType{N} = char(s(ix(1)+1:end-1));
				[s,t] = strtok(t,0);
    			end;		
                        HDR.EVENT.TYP = ones(N,1);
                        HDR.EVENT.POS = round(onset * HDR.SampleRate);
                        HDR.EVENT.DUR = dur * HDR.SampleRate;
                        HDR.EVENT.CHN = zeros(N,1); 
*/
			}
			else if (hdr->TYPE==BDF) {
				/* convert BDF status channel into event table*/
				uint32_t d1, d0 = ((uint32_t)Marker[2]<<16) + ((uint32_t)Marker[1]<<8) + (uint32_t)Marker[0];
				for (k=1; k<len; k++) {
					d1 = ((uint32_t)Marker[3*k1+2]<<16) + ((uint32_t)Marker[3*k1+1]<<8) + (uint32_t)Marker[3*k1];
					if ((d1 & 0x010000) != (d0 & 0x010000)) ++N_EVENT;
					if ((d1 & 0x00ffff) != (d0 & 0x00ffff)) ++N_EVENT;
					d0 = d1;
				}	
				hdr->EVENT.N = N_EVENT;
				hdr->EVENT.SampleRate = hdr->SampleRate; 
				hdr->EVENT.POS = (uint32_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.POS));
				hdr->EVENT.TYP = (uint16_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.TYP));
				hdr->EVENT.DUR = NULL;
				hdr->EVENT.CHN = NULL;
				d0 = ((uint32_t)Marker[2]<<16) + ((uint32_t)Marker[1]<<8) + (uint32_t)Marker[0];
				for (N_EVENT=0, k=1; k<len; k++) {
					d1 = ((uint32_t)Marker[3*k1+2]<<16) + ((uint32_t)Marker[3*k1+1]<<8) + (uint32_t)Marker[3*k1];
					if ((d1 & 0x010000) > (d0 & 0x010000)) {
						hdr->EVENT.POS[N_EVENT] = k;
						hdr->EVENT.TYP[N_EVENT] = 0x7ffe;
						++N_EVENT;
					}
					else if ((d1 & 0x010000) < (d0 & 0x010000)) {
						hdr->EVENT.POS[N_EVENT] = k;
						hdr->EVENT.TYP[N_EVENT] = 0x7ffe;
						++N_EVENT;
					}
					if ((d1 & 0x00ffff) > (d0 & 0x00ffff)) {
						hdr->EVENT.POS[N_EVENT] = k;
						hdr->EVENT.TYP[N_EVENT] = d1 & 0x00ff;
						++N_EVENT;
					}	
					else if ((d1 & 0x00ffff) < (d0 & 0x00ffff)) {
						hdr->EVENT.POS[N_EVENT] = k;
						hdr->EVENT.TYP[N_EVENT] = (d0 & 0x00ff) | 0x8000;
						++N_EVENT;
					}	
					d0 = d1;
				}	
			}

			free(Marker);
			ifseek(hdr, hdr->HeadLen, SEEK_SET);
		}	/* End reading EDF/BDF Status channel */

		ifseek(hdr, hdr->HeadLen, SEEK_SET); 
	    	hdr->FILE.POS = 0; 
	}      	

	else if (hdr->TYPE==ABF) {
		fprintf(stdout,"Warning ABF v%4.2f: implementation is not complete!\n",hdr->VERSION);
		if (hdr->VERSION < 2.0) {  // ABF v1.x
			
		}
		else {	// ABF 2.0+ 
			hdr->HeadLen = leu32p(hdr->AS.Header+8);
		    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,hdr->HeadLen);
	    		count   += ifread(Header1+count, 1, hdr->HeadLen-count, hdr);

			//uint16_t gdftyp = 3; 
			float fADCRange;
			float fDACRange;
			long  lADCResolution;
			long  lDACResolution;
			uint8_t* b = NULL; 
			for (k1=0; k1<18; ++k1) {
				size_t BlockIndex  = leu32p(hdr->AS.Header + k1*16 + 19*4);
				size_t BlockSize   = leu32p(hdr->AS.Header + k1*16 + 19*4+4);
				uint64_t numBlocks = leu64p(hdr->AS.Header + k1*16 + 19*4+8);

				if (VERBOSE_LEVEL>8)
					fprintf(stdout,"ABF %02i: %04i %04i %08Li\n",k1,BlockIndex,BlockSize,numBlocks);

				ifseek(hdr, BlockIndex*512, SEEK_SET);
				b  = (uint8_t*)realloc(b,numBlocks*BlockSize);
				ifread(b,numBlocks,BlockSize,hdr);
				
				if 	(BlockIndex==1) {
					hdr->SampleRate = 1.0 / lef32p(b+k*BlockSize+2);
					hdr->NRec       = leu32p(b+k*BlockSize+36);
   					fADCRange 	= lef32p(b+k*BlockSize+108);
					fDACRange 	= lef32p(b+k*BlockSize+112);
					lADCResolution	= leu32p(b+k*BlockSize+116);
					lDACResolution	= leu32p(b+k*BlockSize+120);
				}
				else if (BlockIndex==2) {
					hdr->NS = numBlocks;
					hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS,sizeof(CHANNEL_TYPE));

					for (k=0;k<hdr->NS;k++)	{
						// initialize fields
					      	hdr->CHANNEL[k].Label[0]  = 0;
					      	strcpy(hdr->CHANNEL[k].Transducer, "EEG: Ag-AgCl electrodes");
					      	hdr->CHANNEL[k].PhysDimCode = 19+4256; // uV
					      	hdr->CHANNEL[k].PhysMax   = +100;
					      	hdr->CHANNEL[k].PhysMin   = -100;
					      	hdr->CHANNEL[k].DigMax    = +2047;
					      	hdr->CHANNEL[k].DigMin    = -2048;
					      	hdr->CHANNEL[k].GDFTYP    = 3;	// int16
					      	hdr->CHANNEL[k].SPR       = leu32p(b+k*BlockSize+20);
					      	hdr->CHANNEL[k].OnOff     = 1;
					      	hdr->CHANNEL[k].Notch     = 50;
					      	hdr->CHANNEL[k].Impedance = INF;
					}      	
					
					uint16_t ch; 
					for (k=0;k<hdr->NS;k++)	{
						ch = leu16p(b+k*BlockSize);
					      	hdr->CHANNEL[ch].LeadIdCode= 0;
					      	hdr->CHANNEL[ch].OnOff     = 1;
						hdr->CHANNEL[ch].Cal 	 = lef32p(b+k*BlockSize+48);
						hdr->CHANNEL[ch].Off 	 = lef32p(b+k*BlockSize+52);
						hdr->CHANNEL[ch].LowPass = lef32p(b+k*BlockSize+56);
						hdr->CHANNEL[ch].HighPass= lef32p(b+k*BlockSize+60);
					}	
				}
				else if (BlockIndex==11) {
//					fprintf(stdout,"%i: %s\n",c,buf); 
				}
			}
			free(b);
	    	}	
	    	hdr->FILE.POS = 0; 
	}      	

	else if (hdr->TYPE==ACQ) {
		/* defined in http://biopac.com/AppNotes/app156FileFormat/FileFormat.htm */
		hdr->NS   = lei16p(hdr->AS.Header+10);
		hdr->SampleRate = 1000.0/lef64p(hdr->AS.Header+16);
		hdr->NRec = 1; 
		hdr->SPR  = 1;

		// add "per channel data section" 
		if (hdr->VERSION<38.0)		// Version 3.0+
			hdr->HeadLen += hdr->NS*122;
		else if (hdr->VERSION<39.0)	// Version 3.7.0+
			hdr->HeadLen += hdr->NS*252;
		else 				// Version 3.7.3+
			hdr->HeadLen += hdr->NS*254;

		hdr->HeadLen += 4;	
		// read header up to nLenght and nID of foreign data section 
	    	hdr->AS.Header = (uint8_t*) realloc(hdr->AS.Header,hdr->HeadLen);
	    	count  += ifread(Header1+count, 1, hdr->HeadLen-count, hdr);
		uint32_t POS = hdr->HeadLen; 
	    	
		// read "foreign data section" and "per channel data types section"  
		hdr->HeadLen += leu16p(hdr->AS.Header+hdr->HeadLen-4);
		hdr->HeadLen += 4*hdr->NS; 
	    	hdr->AS.Header = (uint8_t*)realloc(Header1,hdr->HeadLen+8);
	    	count  += ifread(Header1+POS, 1, hdr->HeadLen-POS, hdr);
		
		// define channel specific header information
		hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
		uint32_t* ACQ_NoSamples = (uint32_t*) calloc(hdr->NS,sizeof(uint32_t));
		uint16_t CHAN; 
    		POS = leu32p(hdr->AS.Header+6);
    		size_t minBufLenXVarDiv = -1;	// maximum integer value
		for (k = 0; k < hdr->NS; k++)	{

	    		uint8_t* Header2 = hdr->AS.Header+POS;
			hdr->CHANNEL[k].Transducer[0] = '\0';
			CHAN = leu16p(Header2+4);
			strncpy(hdr->CHANNEL[k].Label,(char*)Header2+6,min(MAX_LENGTH_LABEL,40));
			strncpy(tmp,(char*)Header2+68,20);
			hdr->CHANNEL[k].PhysDimCode = PhysDimCode(tmp);
			/* PhysDim is OBSOLETE  
			hdr->CHANNEL[k].PhysDim = (char*)(Header1+POS+68);
			hdr->CHANNEL[k].PhysDim[19] = 0;
			*/
			//strncpy(hdr->CHANNEL[k].PhysDim,(char*)(Header1+POS+68),20);
			hdr->CHANNEL[k].Off     = lef64p(Header2+52);
			hdr->CHANNEL[k].Cal     = lef64p(Header2+60);

			hdr->CHANNEL[k].SPR     = 1; 
			if (hdr->VERSION >= 38.0) {
				hdr->CHANNEL[k].SPR = leu16p(Header2+250);  // used here as Divider
			}
			hdr->SPR = lcm(hdr->SPR, hdr->CHANNEL[k].SPR);

			ACQ_NoSamples[k] = leu32p(Header2+88);
			size_t tmp64 = leu32p(Header2+88) * hdr->CHANNEL[k].SPR; 
			if (minBufLenXVarDiv > tmp64) minBufLenXVarDiv = tmp64;
			
			POS += leu32p((uint8_t*)Header2);
		}

		/// foreign data section - skip 
		POS += leu16p(hdr->AS.Header+POS);
		
		size_t DataLen=0; 
		for (k=0, hdr->AS.spb=0, hdr->AS.bpb=0; k<hdr->NS; k++)	{
			if (hdr->VERSION>=38.0)
				hdr->CHANNEL[k].SPR = hdr->SPR/hdr->CHANNEL[k].SPR;  // convert DIVIDER into SPR

			switch ((leu16p(hdr->AS.Header+POS+2)))
			{
			case 1: 
				hdr->CHANNEL[k].GDFTYP = 17;  // double
				hdr->AS.bpb += hdr->CHANNEL[k].SPR<<3;
				DataLen += ACQ_NoSamples[k]<<3; 
				hdr->CHANNEL[k].DigMax =  1e9;
				hdr->CHANNEL[k].DigMin = -1e9;
				break;   
			case 2: 
				hdr->CHANNEL[k].GDFTYP = 3;   // int
				hdr->AS.bpb += hdr->CHANNEL[k].SPR<<1;
				DataLen += ACQ_NoSamples[k]<<1; 
				hdr->CHANNEL[k].DigMax =  32767;
				hdr->CHANNEL[k].DigMin = -32678;
				break;
			default:
				B4C_ERRNUM = B4C_UNSPECIFIC_ERROR;
				B4C_ERRMSG = "SOPEN(ACQ-READ): invalid channel type.";	
			};
			hdr->CHANNEL[k].PhysMax = hdr->CHANNEL[k].DigMax * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
			hdr->CHANNEL[k].PhysMin = hdr->CHANNEL[k].DigMin * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
			hdr->AS.spb += hdr->CHANNEL[k].SPR;
			POS +=4; 
		}
		free(ACQ_NoSamples);
		
/*  ### FIXME ### 
	reading Marker section
		
	    	POS     = hdr->HeadLen; 
#ifdef ZLIB_H
		gzseek(hdr->FILE.FID, hdr->HeadLen+DataLen, SEEK_SET); // start of markers header section
	    	count   = gzread(hdr->FILE.FID, Header1+POS, 8);
#else
		fseek(hdr->FILE.FID, hdr->HeadLen+DataLen, SEEK_SET); // start of markers header section
	    	count   = fread(Header1+POS, 1, 8, hdr->FILE.FID);
#endif
	    	size_t LengthMarkerItemSection = (leu32p(Header1+POS));

	    	hdr->EVENT.N = (leu32p(Header1+POS+4));
	    	Header1 = (char*)realloc(Header1,hdr->HeadLen+8+LengthMarkerItemSection);
	    	POS    += 8; 
#ifdef ZLIB_H
	    	count   = gzread(hdr->FILE.FID, Header1+POS, LengthMarkerItemSection);
#else
	    	count   = fread(Header1+POS, 1, LengthMarkerItemSection, hdr->FILE.FID);
#endif		 
		hdr->EVENT.TYP = (uint16_t*)calloc(hdr->EVENT.N,2); 
		hdr->EVENT.POS = (uint32_t*)calloc(hdr->EVENT.N,4);
		for (k=0; k<hdr->EVENT.N; k++)
		{
fprintf(stdout,"ACQ EVENT: %i POS: %i\n",k,POS);		
			hdr->EVENT.POS[k] = leu32p(Header1+POS);
			POS += 12 + leu16p(Header1+POS+10);
		} 
*/
		ifseek(hdr, hdr->HeadLen, SEEK_SET); 
	    	hdr->FILE.POS = 0; 
	}      	

	else if (hdr->TYPE==AINF) {
		ifclose(hdr);
		const char *filename = hdr->FileName; // keep input file name 
		char* tmpfile = (char*)calloc(strlen(hdr->FileName)+5,1);		
		strcpy(tmpfile,hdr->FileName);
		char* ext = strrchr(tmpfile,'.')+1; 

		/* open and read header file */ 
		strcpy(ext,"ainf");		
		hdr->FileName = tmpfile; 
		hdr = ifopen(hdr,"rb"); 
		count = 0; 
		while (!ifeof(hdr)) {
			size_t bufsiz = 4096;
		    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,count+bufsiz+1);
		    	count  += ifread(hdr->AS.Header+count,1,bufsiz,hdr);
		}
		hdr->AS.Header[count]=0;
		hdr->HeadLen = count; 
		ifclose(hdr);

		char *t1= NULL;
		char *t = strtok((char*)hdr->AS.Header,"\xA\xD");
		while ((t) && !strncmp(t,"#",1)) {
			char* p;
			if ((p = strstr(t,"sfreq ="))) t1 = p;
			t = strtok(NULL,"\xA\xD");
		}

		hdr->NS = 0; 
		while (t) {
			int chno1=-1, chno2=-1; 
			double f1,f2;
			char label[MAX_LENGTH_LABEL+1];

			sscanf(t,"%d %s %d %lf %lf",&chno1,label,&chno2,&f1,&f2);
			
			k = hdr->NS++;
			hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
			sprintf(hdr->CHANNEL[k].Label,"%s %03i",label,chno2);

			hdr->CHANNEL[k].LeadIdCode = 0;
			hdr->CHANNEL[k].SPR    = 1;
			hdr->CHANNEL[k].Cal    = f1*f2; 
			hdr->CHANNEL[k].Off    = 0.0; 
			hdr->CHANNEL[k].GDFTYP = 3;
			hdr->CHANNEL[k].DigMax =  32767;
			hdr->CHANNEL[k].DigMin = -32678;
			hdr->CHANNEL[k].PhysMax= hdr->CHANNEL[k].DigMax * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
			hdr->CHANNEL[k].PhysMin= hdr->CHANNEL[k].DigMin * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
			
			if (strcmp(label,"MEG")==0) 
				hdr->CHANNEL[k].PhysDimCode = 1446; // "T/m" 
			else 	
				hdr->CHANNEL[k].PhysDimCode = 4256; // "V" 

		 	t = strtok(NULL,"\x0a\x0d");	
		}
		hdr->AS.bpb = 2*hdr->NS + 4;	
		hdr->SampleRate = atof(strtok(t1+7," "));
		hdr->SPR = 1; 

		/* open data file */ 
		strcpy(ext,"raw");		
		hdr = ifopen(hdr,"rb"); 
	        ifseek(hdr,0,SEEK_END);	// TODO: replace SEEK_END
		hdr->NRec = iftell(hdr)/hdr->AS.bpb;
	        ifseek(hdr,0,SEEK_SET);
		hdr->HeadLen = 0;
	    	hdr->FILE.POS = 0; 
		/* restore input file name, and free temporary file name  */
		hdr->FileName = filename;
		free(tmpfile);
	}      	

	else if (hdr->TYPE==BKR) {
	    	hdr->HeadLen 	 = 1024; 
	    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, hdr->HeadLen);
	    	count   += ifread(hdr->AS.Header+count,1,1024-count,hdr);
		hdr->NS  	 = leu16p(hdr->AS.Header+2); 
		hdr->NRec   	 = leu32p(hdr->AS.Header+6); 
		hdr->SPR  	 = leu32p(hdr->AS.Header+10); 
		hdr->NRec 	*= hdr->SPR; 
		hdr->SPR  	 = 1; 
		hdr->T0 	 = 0;        // Unknown;
		hdr->SampleRate	 = leu16p(hdr->AS.Header+4);

	    	/* extract more header information */
	    	hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS; k++) {
			sprintf(hdr->CHANNEL[k].Label,"# %02i",k);
			hdr->CHANNEL[k].Transducer[0] = '\0';
		    	hdr->CHANNEL[k].GDFTYP 	 = 3; 
		    	hdr->CHANNEL[k].SPR 	 = 1; // *(int32_t*)(Header1+56);
		    	hdr->CHANNEL[k].LowPass	 = lef32p(hdr->AS.Header+22);
		    	hdr->CHANNEL[k].HighPass = lef32p(hdr->AS.Header+26);
		    	hdr->CHANNEL[k].Notch	 = -1.0;  // unknown 
		    	hdr->CHANNEL[k].PhysMax	 = (double)leu16p(hdr->AS.Header+14);
		    	hdr->CHANNEL[k].DigMax	 = (double)leu16p(hdr->AS.Header+16);
		    	hdr->CHANNEL[k].PhysMin	 = -hdr->CHANNEL[k].PhysMax;
		    	hdr->CHANNEL[k].DigMin	 = -hdr->CHANNEL[k].DigMax;
		    	hdr->CHANNEL[k].Cal	 = hdr->CHANNEL[k].PhysMax/hdr->CHANNEL[k].DigMax;
		    	hdr->CHANNEL[k].Off	 = 0.0;
			hdr->CHANNEL[k].OnOff    = 1;
		    	hdr->CHANNEL[k].PhysDimCode = 4275; // uV
		    	hdr->CHANNEL[k].LeadIdCode  = 0;
		}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// BKR does not support automated overflow and saturation detection
	    	hdr->FILE.POS = 0; 
	}

	else if (hdr->TYPE==BrainVision) {
		/* open and read header file */ 
		// ifclose(hdr);
		const char *filename = hdr->FileName; // keep input file name 
		char* tmpfile = (char*)calloc(strlen(hdr->FileName)+5,1);		
		strcpy(tmpfile,hdr->FileName);
		hdr->FileName = tmpfile;
		char* ext = strrchr(hdr->FileName,'.')+1; 

		while (!ifeof(hdr)) {
			size_t bufsiz = 4096;
		    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,count+bufsiz+1);
		    	count  += ifread(hdr->AS.Header+count,1,bufsiz,hdr);
		}
		hdr->AS.Header[count]=0;
		hdr->HeadLen = count; 
		ifclose(hdr);

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"SOPEN(BV): header file read.\n"); 

		int seq = 0;
		/* read marker file */
		strcpy(ext,"vmrk");		
       		hdr = ifopen(hdr,"r");
		size_t bufsiz = 4096;
		count = 0;
	    	char* vmrk = (char*)malloc(bufsiz+1);
		while (!ifeof(hdr)) {
		    	vmrk = (char*)realloc(vmrk,count+bufsiz+1);
		    	count  += ifread(vmrk+count,1,bufsiz,hdr);
		}
	    	vmrk[count] = 0;	// add terminating \0 character
		ifclose(hdr);

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"SOPEN(BV): marker file read.\n"); 
			
		/* decode marker file */
		size_t pos = 0;

		char *t1 = strtok(vmrk+pos,"\x0A\x0D");	// skip first line 
		t1 = strtok(NULL,"\x0A\x0D");		
		size_t N_EVENT=0,n=100;
 		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, n*sizeof(*hdr->EVENT.POS));
		hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, n*sizeof(*hdr->EVENT.TYP));
		hdr->EVENT.DUR = (uint32_t*) realloc(hdr->EVENT.DUR, n*sizeof(*hdr->EVENT.DUR));
		hdr->EVENT.CHN = (uint16_t*) realloc(hdr->EVENT.CHN, n*sizeof(*hdr->EVENT.CHN));
		while (t1 != NULL) {
			if (!strncmp(t1,";",1))
				; 	
			else if (!strncmp(t1,"[Common Infos]",14))
				seq = 1; 	
			else if (!strncmp(t1,"[Marker Infos]",14))
				seq = 2; 

			else if (seq==1) 
				;
			else if ((seq==2) && !strncmp(t1,"Mk",2)) {
				int p1 = strcspn(t1,"="); 
				int p2 = p1 + 1 + strcspn(t1+p1+1,","); 
				int p3 = p2 + 1 + strcspn(t1+p2+1,","); 
				int p4 = p3 + 1 + strcspn(t1+p3+1,","); 
				int p5 = p4 + 1 + strcspn(t1+p4+1,","); 
				int p6 = p5 + 1 + strcspn(t1+p5+1,",");

				t1[p1]=0;				
				t1[p2]=0;				
				t1[p3]=0;				
				t1[p4]=0;				
				t1[p5]=0;				

				if (n <= N_EVENT) {
					n += 256;
			 		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, n*sizeof(*hdr->EVENT.POS));
					hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, n*sizeof(*hdr->EVENT.TYP));
					hdr->EVENT.DUR = (uint32_t*) realloc(hdr->EVENT.DUR, n*sizeof(*hdr->EVENT.DUR));
					hdr->EVENT.CHN = (uint16_t*) realloc(hdr->EVENT.CHN, n*sizeof(*hdr->EVENT.CHN));
				}
				hdr->EVENT.TYP[N_EVENT] = atol(t1+p2+2);
				hdr->EVENT.POS[N_EVENT] = atol(t1+p3+1);
				hdr->EVENT.DUR[N_EVENT] = atol(t1+p4+1);
				hdr->EVENT.CHN[N_EVENT] = atol(t1+p5+1);
				if (!strncmp(t1+p1+1,"New Segment",11)) {
					hdr->EVENT.TYP[N_EVENT] = 0x7ffe;
					
					char* t2 = t1+p6+1;
					t2[14]=0;
					tm_time.tm_sec  = atoi(t2+12);
					t2[12]=0;
					tm_time.tm_min  = atoi(t2+10);
					t2[10]=0;
					tm_time.tm_hour = atoi(t2+8);
					t2[8]=0;
					tm_time.tm_mday = atoi(t2+6);
					t2[6]=0;
					tm_time.tm_mon  = atoi(t2+4)-1;
					t2[4]=0;
					tm_time.tm_year = atoi(t2)-1900;
					
					hdr->T0 = t_time2gdf_time(mktime(&tm_time));
				}
				++N_EVENT;
			}		
			t1 = strtok(NULL,"\x0A\x0D");		
		}
		hdr->EVENT.N = N_EVENT;
		free(vmrk);

		/* decode header information */
		hdr->FLAG.OVERFLOWDETECTION = 0;
		seq = 0;
		uint16_t gdftyp=3; 
		double physmax=1e6,physmin=-1e6,digmax=1e6,digmin=-1e6,cal=1.0,off=0.0;
		enum o_t{VEC,MUL} orientation = MUL;
		size_t npts=0;

		char *t = strtok((char*)hdr->AS.Header,"\xA\xD");
		t = strtok(NULL,"\xA\xD");
		while (t) {

			if (VERBOSE_LEVEL>8) fprintf(stdout,"[212]: %i <%s>\n",seq,t);

			if (!strncmp(t,";",1)) 	// comments
				;
			else if (!strncmp(t,"[Common Infos]",14))
				seq = 1; 
			else if (!strncmp(t,"[Binary Infos]",14))
				seq = 2; 
			else if (!strncmp(t,"[ASCII Infos]",13))
				seq = 2; 
			else if (!strncmp(t,"[Channel Infos]",14)) {
				seq = 3; 

				/* open data file */ 
				hdr = ifopen(hdr,"rb"); 
	        		ifseek(hdr,0,SEEK_END);		// fix SEEK_END
				size_t FileSize = iftell(hdr);
			        ifseek(hdr,0,SEEK_SET);
				hdr->HeadLen = 0;
				/* restore input file name, and free temporary file name  */
				hdr->FileName = filename;
				free(tmpfile);

				if (!npts) npts = FileSize/hdr->AS.bpb;
				if (orientation == VEC) {
					hdr->SPR = npts;
					hdr->NRec= 1;
				} else {
					hdr->SPR = 1;
					hdr->NRec= npts;
				}
			    	hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
				for (k=0; k<hdr->NS; k++) {
					hdr->CHANNEL[k].Label[0] = 0;
					hdr->CHANNEL[k].Transducer[0] = '\0';
				    	hdr->CHANNEL[k].GDFTYP 	 = gdftyp; 
				    	hdr->CHANNEL[k].SPR 	 = hdr->SPR; // *(int32_t*)(Header1+56);
				    	hdr->CHANNEL[k].LowPass	 = -1.0;
				    	hdr->CHANNEL[k].HighPass = -1.0;
		    			hdr->CHANNEL[k].Notch	 = -1.0;  // unknown 
				    	hdr->CHANNEL[k].PhysMax	 = physmax;
				    	hdr->CHANNEL[k].DigMax	 = digmax;
				    	hdr->CHANNEL[k].PhysMin	 = physmin;		
				    	hdr->CHANNEL[k].DigMin	 = digmin;
				    	hdr->CHANNEL[k].Cal	 = cal;
				    	hdr->CHANNEL[k].Off	 = off;
					hdr->CHANNEL[k].OnOff    = 1;
				    	hdr->CHANNEL[k].PhysDimCode = 4275; // uV
		    			hdr->CHANNEL[k].LeadIdCode  = 0;
					}
				}	
			//else if (!strncmp(t,"[Common Infos]",14))
			//	seq = 4; 
			else if (!strncmp(t,"[Coordinates]",13))
				seq = 5; 
			else if (!strncmp(t,"[Comment]",9))
				seq = 6; 
			else if (!strncmp(t,"[",1))
				seq = 9; 
			

			else if (seq==1) {
				if      (!strncmp(t,"DataFile=",9)) 
					strcpy(ext,strrchr(t,'.')+1);		
				else if (!strncmp(t,"MarkerFile=",11))
					;
				else if (!strncmp(t,"DataFormat=BINARY",11))
					;
				else if (!strncmp(t,"DataOrientation=VECTORIZED",25))
					orientation = VEC;
				else if (!strncmp(t,"DataOrientation=MULTIPLEXED",26))
					orientation = MUL;
				else if (!strncmp(t,"DataType=TIMEDOMAIN",19))
					;
				else if (!strncmp(t,"NumberOfChannels=",17)) {
					hdr->NS = atoi(t+17);
				}	
				else if (!strncmp(t,"DataPoints=",11)) {
					npts = atof(t+11);
				}	
				else if (!strncmp(t,"SamplingInterval=",17)) {
					hdr->SampleRate = 1e6/atof(t+17);
					hdr->EVENT.SampleRate = hdr->SampleRate;
				}	
			}
			else if (seq==2) {
				if      (!strncmp(t,"BinaryFormat=IEEE_FLOAT_32",26)) {
					gdftyp = 16;
					hdr->AS.bpb = 4*hdr->NS;
				}	
				else if (!strncmp(t,"BinaryFormat=INT_16",19)) {
					gdftyp =  3;
					digmax =  32767;
					digmin = -32768;
					hdr->AS.bpb = 2*hdr->NS;
					hdr->FLAG.OVERFLOWDETECTION = 1;
				}	
				else if (!strncmp(t,"UseBigEndianOrder=NO",20))
					hdr->FLAG.SWAP = (__BYTE_ORDER == __BIG_ENDIAN); 
				else if (!strncmp(t,"UseBigEndianOrder=YES",21))
					hdr->FLAG.SWAP = (__BYTE_ORDER == __LITTLE_ENDIAN); 
				else if (0){
					B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED; 
					B4C_ERRMSG = "Error SOPEN(BrainVision): BinaryFormat=<unknown>";
					return(hdr);
				}	
			}
			else if (seq==3) {
				if (!strncmp(t,"Ch",2)) {
					char* ptr;

					if (VERBOSE_LEVEL==9) fprintf(stdout,"%s\n",t);			

					int n = strtoul(t+2, &ptr, 10)-1;
					if ((n < 0) || (n >= hdr->NS)) {
						B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED; 
						B4C_ERRMSG = "Error SOPEN(BrainVision): invalid channel number";
						ifclose(hdr);
						return(hdr);
					}
					size_t len = min(strcspn(ptr+1,","),MAX_LENGTH_LABEL);
					strncpy(hdr->CHANNEL[n].Label,ptr+1,len);
					hdr->CHANNEL[n].Label[len]=0;
					ptr += len+2;
					ptr += strcspn(ptr,",")+1;
					if (strlen(ptr)>0) {
						hdr->CHANNEL[n].Cal = atof(ptr);
						hdr->CHANNEL[n].PhysMax = hdr->CHANNEL[n].DigMax * hdr->CHANNEL[n].Cal ;
						hdr->CHANNEL[n].PhysMin = hdr->CHANNEL[n].DigMin * hdr->CHANNEL[n].Cal ;
					}	

					if (VERBOSE_LEVEL==9) 
						fprintf(stdout,"Ch%02i=%s,,%s(%f)\n",n,hdr->CHANNEL[n-1].Label,ptr,hdr->CHANNEL[n-1].Cal );			
				}	
			}
			else if (seq==4) {
			}
			else if (seq==5) {
			}
			else if (seq==6) {
			}
					
			t = strtok(NULL,"\x0a\x0d");	// extract next line
		}
	    	hdr->FILE.POS = 0; 
	}

	else if (hdr->TYPE==CFWB) {
	    	hdr->SampleRate = 1.0/lef64p(hdr->AS.Header+8);
		hdr->SPR    	= 1; 
	    	tm_time.tm_year = lei32p(hdr->AS.Header+16) - 1900;
	    	tm_time.tm_mon  = lei32p(hdr->AS.Header+20) - 1;
	    	tm_time.tm_mday = lei32p(hdr->AS.Header+24);
	    	tm_time.tm_hour = lei32p(hdr->AS.Header+28);
	    	tm_time.tm_min  = lei32p(hdr->AS.Header+32);
	    	tm_time.tm_sec  = (int)lef64p(hdr->AS.Header+36);
		tm_time.tm_isdst = -1; 
		
    		hdr->T0 	= tm_time2gdf_time(&tm_time);
	    	// = *(double*)(Header1+44);
	    	hdr->NS   	= leu32p(hdr->AS.Header+52);
	    	hdr->NRec	= leu32p(hdr->AS.Header+56);
	    	//  	= *(int32_t*)(Header1+60);	// TimeChannel
	    	//  	= *(int32_t*)(Header1+64);	// DataFormat

	    	hdr->HeadLen = 68 + hdr->NS*96; 
	    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,hdr->HeadLen);
	    	if (count<=hdr->HeadLen)
			count += ifread(hdr->AS.Header+count, 1, hdr->HeadLen-count, hdr);
		else 	
	    		ifseek(hdr, hdr->HeadLen, SEEK_SET);
		
		uint16_t gdftyp = leu32p(hdr->AS.Header+64);
	    	hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS; k++)	{
		    	uint8_t* Header2 = hdr->AS.Header+68+k*96; 
			hdr->CHANNEL[k].Transducer[0] = '\0';
		    	hdr->CHANNEL[k].GDFTYP 	= CFWB_GDFTYP[gdftyp-1];
		    	hdr->CHANNEL[k].SPR 	= 1; // *(int32_t*)(Header1+56);
		    	strncpy(hdr->CHANNEL[k].Label, (char*)Header2, min(32,MAX_LENGTH_LABEL));
		    	strncpy(hdr->CHANNEL[k].PhysDim, (char*)Header2+32, 16);
		    	hdr->CHANNEL[k].LeadIdCode  = 0;
		    	hdr->CHANNEL[k].Cal	= lef64p(Header2+64);
		    	hdr->CHANNEL[k].Off	= lef64p(Header2+72);
		    	hdr->CHANNEL[k].PhysMax	= lef64p(Header2+80);
		    	hdr->CHANNEL[k].PhysMin	= lef64p(Header2+88);
		    	hdr->CHANNEL[k].DigMax	= (hdr->CHANNEL[k].PhysMax - hdr->CHANNEL[k].Off) / hdr->CHANNEL[k].Cal; 
		    	hdr->CHANNEL[k].DigMin	= (hdr->CHANNEL[k].PhysMin - hdr->CHANNEL[k].Off) / hdr->CHANNEL[k].Cal; 
			hdr->CHANNEL[k].OnOff    = 1;
		}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// CFWB does not support automated overflow and saturation detection
	    	hdr->FILE.POS = 0; 
	}

	else if (hdr->TYPE==CNT) {
	    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,900);
	    	hdr->VERSION = atof((char*)hdr->AS.Header+8);
	    	count  += ifread(hdr->AS.Header+count,1,900-count,hdr);

	    	ptr_str = (char*)hdr->AS.Header+136;
    		hdr->Patient.Sex = (ptr_str[0]=='f')*2 + (ptr_str[0]=='F')*2 + (ptr_str[0]=='M') + (ptr_str[0]=='m');
	    	ptr_str = (char*)hdr->AS.Header+137;
	    	hdr->Patient.Handedness = (ptr_str[0]=='r')*2 + (ptr_str[0]=='R')*2 + (ptr_str[0]=='L') + (ptr_str[0]=='l');
	    	ptr_str = (char*)hdr->AS.Header+255;
	    	tm_time.tm_sec  = atoi(strncpy(tmp,ptr_str+16,2)); 
    		tm_time.tm_min  = atoi(strncpy(tmp,ptr_str+13,2)); 
    		tm_time.tm_hour = atoi(strncpy(tmp,ptr_str+10,2)); 
    		tm_time.tm_mday = atoi(strncpy(tmp,ptr_str,2)); 
    		tm_time.tm_mon  = atoi(strncpy(tmp,ptr_str+3,2)-1); 
    		tm_time.tm_year = atoi(strncpy(tmp,ptr_str+6,2)); 

	    	if (tm_time.tm_year<=80)    	tm_time.tm_year += 2000;
	    	else if (tm_time.tm_year<100) 	tm_time.tm_year += 1900;
		hdr->T0 = tm_time2gdf_time(&tm_time); 
		
		hdr->NS  = leu16p(hdr->AS.Header+370); 
	    	hdr->HeadLen = 900+hdr->NS*75; 
		hdr->SPR    = 1; 
		hdr->SampleRate = leu16p(hdr->AS.Header+376); 
		hdr->AS.bpb = hdr->NS*2;
	    	size_t eventtablepos = leu32p(hdr->AS.Header+886);
		hdr->NRec   = (eventtablepos-hdr->HeadLen) / hdr->AS.bpb;
		
	    	hdr->AS.Header = (uint8_t*) realloc(Header1,hdr->HeadLen);
	    	count  += ifread(Header1+900,1,hdr->NS*75,hdr);

	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS;k++)	{
		    	uint8_t* Header2 = hdr->AS.Header+900+k*75; 
			hdr->CHANNEL[k].Transducer[0] = '\0';
		    	hdr->CHANNEL[k].GDFTYP 	= 3;
		    	hdr->CHANNEL[k].SPR 	= 1; // *(int32_t*)(Header1+56);
		    	strncpy(hdr->CHANNEL[k].Label,(char*)Header2,min(10,MAX_LENGTH_LABEL));
		    	hdr->CHANNEL[k].LeadIdCode  = 0;
			hdr->CHANNEL[k].PhysDimCode = 4256+19;  // uV
		    	hdr->CHANNEL[k].Cal	= lef32p(Header2+59);
		    	hdr->CHANNEL[k].Cal    *= lef32p(Header2+71)/204.8;
		    	hdr->CHANNEL[k].Off	= lef32p(Header2+47) * hdr->CHANNEL[k].Cal;
		    	hdr->CHANNEL[k].HighPass= CNT_SETTINGS_HIGHPASS[(uint8_t)Header2[64]];
		    	hdr->CHANNEL[k].LowPass	= CNT_SETTINGS_LOWPASS[(uint8_t)Header2[65]];
		    	hdr->CHANNEL[k].Notch	= CNT_SETTINGS_NOTCH[(uint8_t)Header1[682]];
			hdr->CHANNEL[k].OnOff   = 1;

		    	hdr->CHANNEL[k].DigMax	=  (double)32767;
		    	hdr->CHANNEL[k].DigMin	= -(double)32768;
		    	hdr->CHANNEL[k].PhysMax	= hdr->CHANNEL[k].DigMax * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
		    	hdr->CHANNEL[k].PhysMin	= hdr->CHANNEL[k].DigMin * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
		}

	    	/* read event table */
	    	int status = ifseek(hdr, eventtablepos, SEEK_SET);

	    	if (!status) {
			hdr->EVENT.SampleRate = hdr->SampleRate; 
			ifread(tmp,9,1,hdr);	    	
			int8_t   TeegType   = tmp[0]; 
			uint32_t TeegSize   = leu32p(tmp+1);
			uint32_t TeegOffset = leu32p(tmp+5);

			int fieldsize;
			switch (TeegType) {
			case 2: 
			case 3:  fieldsize = 19; break; 
			default: fieldsize = 8; 
			} 
			
			uint8_t* buf = (uint8_t*)malloc(TeegSize);
			count = ifread(buf, 1, TeegSize, hdr);
			hdr->EVENT.N   = count/fieldsize;
			hdr->EVENT.POS = (uint32_t*) calloc(hdr->EVENT.N, sizeof(hdr->EVENT.POS));
			hdr->EVENT.TYP = (uint16_t*) calloc(hdr->EVENT.N, sizeof(hdr->EVENT.TYP));
			hdr->EVENT.DUR = NULL;
			hdr->EVENT.CHN = NULL;

			for  (k = 0; k < hdr->EVENT.N; k++) {
				hdr->EVENT.TYP[k] = leu16p(buf+k*fieldsize);
				hdr->EVENT.POS[k] = leu32p(buf+4+k*fieldsize);
				if (TeegType!=3)
					hdr->EVENT.POS[k] = (hdr->EVENT.POS[k] - hdr->HeadLen) / hdr->AS.bpb;
			}
			free(buf);
		    	ifseek(hdr, hdr->HeadLen, SEEK_SET);
	    	}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// automated overflow and saturation detection not supported
	    	hdr->FILE.POS = 0; 
	}

	else if (hdr->TYPE==DEMG) {
	    	hdr->VERSION 	= leu16p(hdr->AS.Header+4);
	    	hdr->NS		= leu16p(hdr->AS.Header+6);
	    	hdr->SPR	= 1;
	    	hdr->SampleRate = leu32p(hdr->AS.Header+8);
	    	hdr->NRec	= leu32p(hdr->AS.Header+12);
	    	
		uint16_t gdftyp = 16;
		uint8_t  bits    = hdr->AS.Header[16];	
		double   PhysMin = (double)(int8_t)hdr->AS.Header[17];	
		double   PhysMax = (double)(int8_t)hdr->AS.Header[18];	
		double	 Cal = 1.0; 
		double	 Off = 0.0; 
		
		if (hdr->VERSION==1) {
			gdftyp = 16;	// float32
			Cal = 1.0;
			Off = 0.0;
		}
		else if (hdr->VERSION==2) {
			gdftyp = 4;	// uint16
			Cal = (PhysMax-PhysMin)/((1<<bits) - 1.0);
			Off = (double)PhysMin;
		}	
		double DigMax = (PhysMax-Off)/Cal;
		double DigMin = (PhysMin-Off)/Cal;
	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
		for (k=0; k < hdr->NS; k++) {
			CHANNEL_TYPE* hc = hdr->CHANNEL+k;
			hc->GDFTYP   = gdftyp;
			hc->SPR      = 1; 
			hc->Cal      = Cal; 
			hc->Off      = Off;
			hc->Transducer[0] = '\0';
			hc->LowPass  = 450;
			hc->HighPass = 20;
			hc->PhysMax  = PhysMax;
			hc->PhysMin  = PhysMin;
			hc->DigMax   = DigMax;
			hc->DigMin   = DigMin;
		    	hc->LeadIdCode  = 0;
		}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// automated overflow and saturation detection not supported
	    	hdr->HeadLen = 19; 
	    	ifseek(hdr, 19, SEEK_SET);
	    	hdr->FILE.POS = 0; 
	}

	else if (hdr->TYPE==EGI) {
	    	uint16_t gdftyp=3;
		// BigEndian 
		hdr->FLAG.SWAP = (__BYTE_ORDER == __LITTLE_ENDIAN); 	// default: most data formats are little endian 
	    	hdr->VERSION  	= beu32p(hdr->AS.Header);
    		if      (hdr->VERSION==2 || hdr->VERSION==3)	gdftyp = 3;	// int32
    		else if (hdr->VERSION==4 || hdr->VERSION==5)	gdftyp = 16;	// float
    		else if (hdr->VERSION==6 || hdr->VERSION==7)	gdftyp = 17;	// double
    		
	    	tm_time.tm_year = beu16p(hdr->AS.Header+4) - 1900;
	    	tm_time.tm_mon  = beu16p(hdr->AS.Header+6) - 1;
	    	tm_time.tm_mday = beu16p(hdr->AS.Header+8);
	    	tm_time.tm_hour = beu16p(hdr->AS.Header+10);
	    	tm_time.tm_min  = beu16p(hdr->AS.Header+12);
	    	tm_time.tm_sec  = beu16p(hdr->AS.Header+14);
	    	// tm_time.tm_sec  = beu32p(Header1+16)/1000; // not supported by tm_time

	    	hdr->T0 = tm_time2gdf_time(&tm_time);
	    	hdr->SampleRate = beu16p(hdr->AS.Header+20);
	    	hdr->NS         = beu16p(hdr->AS.Header+22);
	    	// uint16_t  Gain  = beu16p(Header1+24);
	    	uint16_t  Bits  = beu16p(hdr->AS.Header+26);
	    	uint16_t PhysMax= beu16p(hdr->AS.Header+28);
	    	size_t POS; 
	    	if (hdr->AS.Header[3] & 0x01)
	    	{ 	// Version 3,5,7
	    		POS = 32; 
	    		for (k=0; k < beu16p(hdr->AS.Header+30); k++)
	    			POS += *(hdr->AS.Header+POS);	// skip EGI categories 

			if (POS > count-8) {
				hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,POS+8);
				count += ifread(hdr->AS.Header,1,POS+8-count,hdr);
			}

	    		hdr->NRec= beu16p(hdr->AS.Header+POS);
	    		hdr->SPR = beu32p(hdr->AS.Header+POS+2);
	    		EGI_LENGTH_CODETABLE = beu16p(hdr->AS.Header+POS+6);	// EGI.N
	    		POS += 8; 
	    	}
	    	else
	    	{ 	// Version 2,4,6
	    		hdr->NRec = beu32p(hdr->AS.Header+30);
	    		EGI_LENGTH_CODETABLE = beu16p(hdr->AS.Header+34);	// EGI.N
	    		hdr->SPR  = 1;
	    		/* see also end-of-sopen  
	    		hdr->AS.spb = hdr->SPR+EGI_LENGTH_CODETABLE ;
		    	hdr->AS.bpb = (hdr->NS + EGI_LENGTH_CODETABLE)*GDFTYP_BYTE[hdr->CHANNEL[0].GDFTYP];
		    	*/    
		    	POS = 36; 
	    	}
		POS += 4*EGI_LENGTH_CODETABLE;	    	// skip event codes
		hdr->HeadLen = POS; 
		ifseek(hdr,hdr->HeadLen,SEEK_SET);
		hdr->FILE.POS  = 0;

		hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	    	for (k=0; k<hdr->NS; k++) {
    			hdr->CHANNEL[k].GDFTYP = gdftyp;
	    		hdr->CHANNEL[k].PhysDimCode = 4275;  // "uV"
		    	hdr->CHANNEL[k].LeadIdCode  = 0;
	    		sprintf(hdr->CHANNEL[k].Label,"# %03i",k);  
	    		hdr->CHANNEL[k].Cal     = PhysMax/ldexp(1,Bits);
	    		hdr->CHANNEL[k].Off     = 0;
	    		hdr->CHANNEL[k].SPR     = hdr->SPR;
		    	if (Bits && PhysMax) {
		    		hdr->CHANNEL[k].PhysMax = PhysMax;
		    		hdr->CHANNEL[k].PhysMin = -PhysMax;
	    			hdr->CHANNEL[k].DigMax  = ldexp(1,Bits);
	    			hdr->CHANNEL[k].DigMin  = ldexp(-1,Bits);
	    		} 
	    		else {	
/*	    		hdr->CHANNEL[k].PhysMax = PhysMax;
	    		hdr->CHANNEL[k].PhysMin = -PhysMax;
	    		hdr->CHANNEL[k].DigMax  = ldexp(1,Bits);
	    		hdr->CHANNEL[k].DigMin  = ldexp(-1,Bits);
*/	    		hdr->CHANNEL[k].Cal     = 1.0;
	    		}
	    	}  
	}
	
	else if (hdr->TYPE==ETG4000) {
		/* read file */ 
		while (!ifeof(hdr)) {
			size_t bufsiz = 1<<16;
		    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,count+bufsiz+1);
		    	count  += ifread(hdr->AS.Header+count,1,bufsiz,hdr);
		}
		hdr->AS.Header[count]=0;
		hdr->HeadLen = count; 
		ifclose(hdr);

		if (VERBOSE_LEVEL==9) 
			fprintf(stdout,"Size of File %s is %i\n",hdr->FileName,count); 
		
		/* decode header section */
		char dlm[2];
		dlm[0] = (char)hdr->AS.Header[20];
		dlm[1] = 0;
		hdr->VERSION = -1; 
		char FLAG_StimType_STIM = 0;
		char *t = strtok((char*)hdr->AS.Header,"\xA\xD");
		char *ag=NULL, *dg=NULL, *label;
		double lpf=-1.0,hpf=-1.0,age=0.0;
		while (strncmp(t,"Probe",5)) {
			if (VERBOSE_LEVEL==9) 
				fprintf(stderr,"-> %s\n",t);
				 
			if (!strncmp(t,"File Version",12))
				hdr->VERSION = atof(strpbrk(t,dlm)+1);
			else if (!strncmp(t,"Name",4))
				strncpy(hdr->Patient.Id,strpbrk(t,dlm)+1,MAX_LENGTH_PID);
			else if (!strncmp(t,"Sex",3))
				hdr->Patient.Sex = (toupper(*strpbrk(t,dlm)+1)=='F')<<1 + (toupper(*strpbrk(t,dlm)+1)=='M');
			else if (!strncmp(t,"Age",3)) {
				char *tmp1 = strpbrk(t,dlm)+1;
				size_t c = strcspn(tmp1,"0123456789");
				char buf[20];
				age = atof(strncpy(buf,tmp1,19));
				if (tmp1[c]=='y') 
					age *= 365.25;
				else if (tmp1[c]=='m') 
					age *= 30;
			}
			else if (!strncmp(t,"Date",4)) {
				sscanf(strpbrk(t,dlm)+1,"%d/%d/%d %d:%d",&(tm_time.tm_year),&(tm_time.tm_mon),&(tm_time.tm_mday),&(tm_time.tm_hour),&(tm_time.tm_min));
				tm_time.tm_sec = 0;
				tm_time.tm_year -= 1900;
				tm_time.tm_mon -= 1;
				//fprintf(stdout,"%s\n",asctime(&tm_time));
				hdr->T0 = tm_time2gdf_time(&tm_time);
			}
			else if (!strncmp(t,"HPF[Hz]",7))
				hpf = atof(strpbrk(t,dlm)+1);
			else if (!strncmp(t,"LPF[Hz]",7))
				lpf = atof(strpbrk(t,dlm)+1);
			else if (!strncmp(t,"Analog Gain",11))
				ag = strpbrk(t,dlm);
			else if (!strncmp(t,"Digital Gain",12))
				dg = strpbrk(t,dlm)+1;
			else if (!strncmp(t,"Sampling Period[s]",18))
				hdr->SampleRate = 1.0/atof(strpbrk(t,dlm)+1);
			else if (!strncmp(t,"StimType",8))
				FLAG_StimType_STIM = !strncmp(t+9,"STIM",4);

			t = strtok(NULL,"\xA\xD");
		}
		if (VERBOSE_LEVEL==9) 
			fprintf(stderr,"\nNS=%i\n-> %s\n",hdr->NS,t);

		hdr->Patient.Birthday = hdr->T0 - ldexp(age,32);
		hdr->NS = 0; 
		while (ag != NULL) {
			++hdr->NS; 
			ag = strpbrk(ag+1,dlm);
		}	
		hdr->NS >>= 1; 

		if (VERBOSE_LEVEL==9) 
			fprintf(stderr,"\n-V=%i NS=%i\n-> %s\n",VERBOSE_LEVEL,hdr->NS,t);

	    	label = strpbrk(t,dlm) + 1; 
	    	//uint16_t gdftyp = 16;			// use float32 as internal buffer
	    	uint16_t gdftyp = 17;			// use float64 as internal buffer
		double DigMax = 1.0, DigMin = -1.0;
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// automated overflow and saturation detection not supported
		hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS, sizeof(CHANNEL_TYPE));
		for (k=0; k < hdr->NS; k++) {
			CHANNEL_TYPE* hc = hdr->CHANNEL+k;
			hc->OnOff    = 1;
			hc->GDFTYP   = gdftyp;
			hc->SPR      = 1; 
			hc->Cal      = 1.0; 
			hc->Off      = 0.0;
			hc->Transducer[0] = '\0';
			hc->LowPass  = lpf;
			hc->HighPass = hpf;
			hc->PhysMax  = DigMax;
			hc->PhysMin  = DigMin;
			hc->DigMax   = DigMax;
			hc->DigMin   = DigMin;
		    	hc->LeadIdCode  = 0;
		    	hc->PhysDimCode = 65362;	//mol l-1 mm
		    	size_t c     = strcspn(label,dlm);
		    	size_t c1    = min(c,MAX_LENGTH_LABEL);
		    	strncpy(hc->Label, label, c1);
		    	hc->Label[c1]= 0;
		    	label += c+1;

			if (VERBOSE_LEVEL>8) 
				fprintf(stderr,"-> Label #%02i: len(%i) %s\n",k,c1,hc->Label);
		}
		hdr->SPR    = 1;
		hdr->NRec   = 0;

		/* decode data section */
		hdr->FLAG.SWAP = 0; 

		uint32_t pos;
		int Mark,hh,mm,ss,ds,BodyMovement,RemovalMark,PreScan;
		hdr->EVENT.N = 0;
		hdr->EVENT.SampleRate = hdr->SampleRate;
		hdr->EVENT.DUR = NULL;
		hdr->EVENT.CHN = NULL;
 
		pos = atol(strtok(NULL,dlm));
		while (pos) {
			hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata, (hdr->NRec+1) * hdr->NS * GDFTYP_BYTE[gdftyp]);
			for (k=0; k < hdr->NS; k++) {
			if (gdftyp==16)	
				*(float*)(hdr->AS.rawdata + (hdr->NRec*hdr->NS+k)*GDFTYP_BYTE[gdftyp]) = (float)atof(strtok(NULL,dlm));
			else if (gdftyp==17)
				*(double*)(hdr->AS.rawdata + (hdr->NRec*hdr->NS+k)*GDFTYP_BYTE[gdftyp]) = atof(strtok(NULL,dlm));
			}
			++hdr->NRec;

			Mark = atoi(strtok(NULL,dlm));
			if (Mark) {
				++hdr->EVENT.N;
		 		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, hdr->EVENT.N*sizeof(*hdr->EVENT.POS) );
				hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP) );
				hdr->EVENT.POS[hdr->EVENT.N-1] = pos; 
				hdr->EVENT.TYP[hdr->EVENT.N-1] = Mark; 
				if (FLAG_StimType_STIM && !(hdr->EVENT.N & 0x01))
					hdr->EVENT.TYP[hdr->EVENT.N-1] = Mark | 0x8000; 
			}
			sscanf(strtok(NULL,dlm),"%d:%d:%d.%d",&hh,&mm,&ss,&ds);
			BodyMovement 	= atoi(strtok(NULL,dlm));
			RemovalMark 	= atoi(strtok(NULL,dlm));
			PreScan 	= atoi(strtok(NULL,"\xA\xD"));

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"%d: %d %02d:%02d:%02d.%02d %d %d %d\n",pos,Mark,hh,mm,ss,ds,BodyMovement,RemovalMark,PreScan);

			pos = atol(strtok(NULL,dlm));
		};

		if (FLAG_StimType_STIM && (hdr->EVENT.N & 0x01)) {
			/* if needed, add End-Of-Event marker */
			++hdr->EVENT.N;
	 		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, hdr->EVENT.N*sizeof(*hdr->EVENT.POS) );
			hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP) );
			hdr->EVENT.POS[hdr->EVENT.N-1] = pos; 
			hdr->EVENT.TYP[hdr->EVENT.N-1] = Mark | 0x8000; 
		}
	    	hdr->FILE.POS = 0; 
	}

    	else if (hdr->TYPE==MFER) {	
		// MFER101E-2003, Table 5, p.18-19
    		/* ###  FIXME: some units are not encoded */  	
    		const uint16_t MFER_PhysDimCodeTable[30] = {
    			4256, 3872, 3840, 3904,    0,	// Volt, mmHg, Pa, mmH2O, mmHg/S
    			3808, 3776,  544, 6048, 2528,	// dyne, N, %, °C, 1/min 
    			4264, 4288, 4160,65376, 4032,	// 1/s, Ohm, A, rpm, W
    			6432, 1731, 3968, 6016,    0,	// dB, kg, J, dyne s m-2 cm-5, l
    			3040, 3072, 4480,    0,    0,	// L/s, L/min, cd
    			   0,    0,    0,    0,    0,	// 
		};

		hdr->FLAG.OVERFLOWDETECTION = 0; 	// MFER does not support automated overflow and saturation detection

	    	uint8_t buf[128];
		uint8_t gdftyp = 3; 	// default: int16
		uint8_t UnitCode=0; 
		double Cal = 1.0, Off = 0.0; 
		hdr->FLAG.SWAP = ( __BYTE_ORDER == __LITTLE_ENDIAN);   // default of MFER is BigEndian
		/* TAG */ 
		uint8_t tag = hdr->AS.Header[0];
    		ifseek(hdr,1,SEEK_SET);
    		int curPos = 1; 
		while (!ifeof(hdr)) {
			uint32_t len, val32=0;
			int32_t  chan=-1; 
			uint8_t tmplen;

			if (tag==255)  
				break;
			else if (tag==63) {
				/* CONTEXT */ 
				curPos += ifread(buf,1,1,hdr);
				chan = buf[0] & 0x7f;
				while (buf[0] & 0x80) {
					curPos += ifread(buf,1,1,hdr);
					chan    = (chan<<7) + (buf[0] & 0x7f);
				}
			}
			
			/* LENGTH */ 
			curPos += ifread(&tmplen,1,1,hdr);
			char FlagInfiniteLength = 0; 
			if ((tag==63) && (tmplen==0x80)) {
				FlagInfiniteLength = -1; //Infinite Length
				len = 0; 
			}	
			else if (tmplen & 0x80) {
				tmplen &= 0x7f;
				curPos += ifread(&buf,1,tmplen,hdr);
				len = 0; 
				k = 0; 
				while (k<tmplen)
					len = (len<<8) + buf[k++];
			}
			else
				len = tmplen;
			
			if (VERBOSE_LEVEL==9) 
				fprintf(stdout,"MFER: tag=%3i chan=%2i len=%i %3i curPos=%i %li\n",tag,chan,tmplen,len,curPos,iftell(hdr));

			/* VALUE */ 
			if (tag==0) {
				if (len!=1) fprintf(stderr,"Warning MFER tag0 incorrect length %i!=1\n",len); 
				curPos += ifread(buf,1,len,hdr);
			}	
			else if (tag==1) {
				// Endianity 
				if (len!=1) fprintf(stderr,"Warning MFER tag1 incorrect length %i!=1\n",len); 
					ifseek(hdr,len-1,SEEK_CUR); 
				curPos += ifread(buf,1,1,hdr);
				if      ( __BYTE_ORDER == __LITTLE_ENDIAN)
					hdr->FLAG.SWAP =  !buf[0];
				else if ( __BYTE_ORDER == __BIG_ENDIAN)
					hdr->FLAG.SWAP =   buf[0];
			}
			else if (tag==2) {
				// Version
				uint8_t v[3];
				if (len!=3) fprintf(stderr,"Warning MFER tag2 incorrect length %i!=3\n",len); 
				curPos += ifread(&v,1,3,hdr);
				hdr->VERSION = v[0] + (v[1]<10 ? v[1]/10.0 : (v[1]<100 ? v[1]/100.0 : v[1]/1000.0)); 
				}	
			else if (tag==3) {
				// character code 
				char v[17];
				if (len>16) fprintf(stderr,"Warning MFER tag2 incorrect length %i>16\n",len); 
				curPos += ifread(&v,1,len,hdr);
				v[len]  = 0; 
			}	
			else if (tag==4) {
				// SPR
				if (len>4) fprintf(stderr,"Warning MFER tag4 incorrect length %i>4\n",len); 
				curPos += ifread(buf,1,len,hdr);
				hdr->SPR = *(int64_t*) mfer_swap8b(buf, len, hdr->FLAG.SWAP); 
			}	
			else if (tag==5) {
				// NS
				if (len>4) fprintf(stderr,"Warning MFER tag5 incorrect length %i>4\n",len); 
				curPos += ifread(buf,1,len,hdr);
				hdr->NS = *(int64_t*) mfer_swap8b(buf, len, hdr->FLAG.SWAP); 
				hdr->CHANNEL = (CHANNEL_TYPE*)realloc(hdr->CHANNEL, hdr->NS*sizeof(CHANNEL_TYPE));
				for (k=0; k<hdr->NS; k++) {
					hdr->CHANNEL[k].SPR = 0; 
					hdr->CHANNEL[k].PhysDimCode = 0; 
					hdr->CHANNEL[k].Cal = 1.0; 
				}
			}	
			else if (tag==6) {
				// NRec
				if (len>4) fprintf(stderr,"Warning MFER tag6 incorrect length %i>4\n",len); 
				curPos += ifread(buf,1,len,hdr);
				hdr->NRec = *(int64_t*) mfer_swap8b(buf, len, hdr->FLAG.SWAP); 
			}	
			else if (tag==8) {
				// Type of Waveform
				uint8_t TypeOfWaveForm8[2];
				uint16_t TypeOfWaveForm;
				if (len>2) fprintf(stderr,"Warning MFER tag8 incorrect length %i>2\n",len); 
				curPos += ifread(&TypeOfWaveForm8,1,len,hdr);
				if (len==1) 
					TypeOfWaveForm = TypeOfWaveForm8[0];
				else if (hdr->FLAG.SWAP)
					TypeOfWaveForm = bswap_16(*(uint16_t*)TypeOfWaveForm8);
				else 
					TypeOfWaveForm =          *(uint16_t*)TypeOfWaveForm8;
			}
			else if (tag==10) {
				// GDFTYP
				if (len!=1) fprintf(stderr,"warning MFER tag10 incorrect length %i!=1\n",len); 
				curPos += ifread(&gdftyp,1,1,hdr);
				if 	(gdftyp==0)	gdftyp=3; // int16
				else if (gdftyp==1)	gdftyp=4; // uint16
				else if (gdftyp==2)	gdftyp=5; // int32
				else if (gdftyp==3)	gdftyp=2; // uint8
				else if (gdftyp==4)	gdftyp=4; // bit16
				else if (gdftyp==5)	gdftyp=1; // int8
				else if (gdftyp==6)	gdftyp=6; // uint32
				else if (gdftyp==7)	gdftyp=16; // float32
				else if (gdftyp==8)	gdftyp=17; // float64
				else if (gdftyp==9)	//gdftyp=2; // 8 bit AHA compression
					fprintf(stdout,"Warning: MFER compressed format not supported\n");   
				else			gdftyp=3;
			}	
			else if (tag==11) {
				// Fs
				if (len>6) fprintf(stderr,"Warning MFER tag11 incorrect length %i>6\n",len); 
				double  fval; 
				curPos += ifread(buf,1,len,hdr);
				fval = *(int64_t*) mfer_swap8b(buf+2, len-2, hdr->FLAG.SWAP); 
				
				hdr->SampleRate = fval*pow(10.0, (int8_t)buf[1]);
				if (buf[0]==1)  // s
					hdr->SampleRate = 1.0/hdr->SampleRate;
			}	
			else if (tag==12) {
				// sampling resolution 
				if (len>6) fprintf(stderr,"Warning MFER tag12 incorrect length %i>6\n",len); 
				val32   = 0;
				int8_t  v8; 
				curPos += ifread(&UnitCode,1,1,hdr);
				curPos += ifread(&v8,1,1,hdr);
				curPos += ifread(buf,1,len-2,hdr);
				Cal = *(int64_t*) mfer_swap8b(buf, len-2, hdr->FLAG.SWAP); 
				Cal *= pow(10.0,v8); 
				if (!MFER_PhysDimCodeTable[UnitCode]) 
					fprintf(stderr,"Warning MFER: unsupported physical unit (code=%i)\n", UnitCode); 
			}	
			else if (tag==13) {
				if (len>8) fprintf(stderr,"Warning MFER tag13 incorrect length %i>8\n",len); 
				curPos += ifread(&buf,1,len,hdr);
				if      (gdftyp == 1) Off = ( int8_t)buf[0];
				else if (gdftyp == 2) Off = (uint8_t)buf[0];
				else if (hdr->FLAG.SWAP) {
					if      (gdftyp == 3) Off = ( int16_t)bswap_16(*( int16_t*)buf);
					else if (gdftyp == 4) Off = (uint16_t)bswap_16(*(uint16_t*)buf);
					else if (gdftyp == 5) Off = ( int32_t)bswap_32(*( int32_t*)buf);
					else if (gdftyp == 6) Off = (uint32_t)bswap_32(*(uint32_t*)buf);
					else if (gdftyp == 7) Off = ( int64_t)bswap_64(*( int64_t*)buf);
					else if (gdftyp == 8) Off = (uint64_t)bswap_64(*(uint64_t*)buf);
					else if (gdftyp ==16) { 
						*(uint32_t*)buf = bswap_32(*(uint32_t*)buf);
						Off = *(float*)buf;
					}
					else if (gdftyp ==17) { 
						*(uint64_t*)buf = bswap_64(*(uint64_t*)buf);
						Off = *(double*)buf;
					}
				}	
				else {
					if      (gdftyp == 3) Off = *( int16_t*)buf;
					else if (gdftyp == 4) Off = *(uint16_t*)buf;
					else if (gdftyp == 5) Off = *( int32_t*)buf;
					else if (gdftyp == 6) Off = *(uint32_t*)buf;
					else if (gdftyp == 7) Off = *( int64_t*)buf;
					else if (gdftyp == 8) Off = *(uint64_t*)buf;
					else if (gdftyp ==16) Off = *(float*)buf;
					else if (gdftyp ==17) Off = *(double*)buf;
				}	
			}
			else if (tag==23) {
				// manufacturer information: "Manufacturer^model^version number^serial number"
				ifread(hdr->ID.Manufacturer._field,1,min(MAX_LENGTH_MANUF,len),hdr);
				if (len>MAX_LENGTH_MANUF) {
					fprintf(stderr,"Warning MFER tag23 incorrect length %i>128\n",len); 
					ifseek(hdr,len-MAX_LENGTH_MANUF,SEEK_CUR);
				}	
				curPos += len;
				for (k=0; isprint(hdr->ID.Manufacturer._field[k]) && (k<MAX_LENGTH_MANUF); k++);
				hdr->ID.Manufacturer._field[k]    = 0; 
				hdr->ID.Manufacturer.Name         = strtok(hdr->ID.Manufacturer._field,"^");  
				hdr->ID.Manufacturer.Model        = strtok(NULL,"^");  
				hdr->ID.Manufacturer.Version      = strtok(NULL,"^");  
				hdr->ID.Manufacturer.SerialNumber = strtok(NULL,"^");  
			}
			else if (tag==30) {
				// data block 
				hdr->AS.rawdata = (uint8_t*)realloc(hdr->AS.rawdata,len); 
				hdr->HeadLen    = curPos;
				curPos += ifread(hdr->AS.rawdata,1,len,hdr);
			}
			else if (tag==63) {
				uint8_t tag2=255, len2=255; 

				count = 0; 
				while ((count<len) && !(FlagInfiniteLength && len2==0 && tag2==0)){ 
					curPos += ifread(&tag2,1,1,hdr);
					curPos += ifread(&len2,1,1,hdr);
					if (VERBOSE_LEVEL==9)
						fprintf(stdout,"MFER: tag=%3i chan=%2i len=%-4i tag2=%3i len2=%3i curPos=%i %li count=%4i\n",tag,chan,len,tag2,len2,curPos,iftell(hdr),count);
				
					if (FlagInfiniteLength && len2==0 && tag2==0) break; 

					count  += (2+len2);
					curPos += ifread(&buf,1,len2,hdr);
					if (tag2==4) {
						// SPR
						if (len2>4) fprintf(stderr,"Warning MFER tag63-4 incorrect length %i>4\n",len2); 
						hdr->CHANNEL[chan].SPR = *(int64_t*) mfer_swap8b(buf, len2, hdr->FLAG.SWAP); 
					}	
					else if (tag2==9) {	//leadname 
						if (len2==1)
							hdr->CHANNEL[chan].LeadIdCode = buf[0]; 
						else if (len2==2)
							hdr->CHANNEL[chan].LeadIdCode = 0; 
						else if (len2<=32)
							strncpy(hdr->CHANNEL[chan].Label,(char*)buf,len2); 
						else
							fprintf(stderr,"Warning MFER tag63-9 incorrect length %i>32\n",len2); 
					}
					else if (tag2==11) {	// sampling resolution 
						if (len2>6) fprintf(stderr,"Warning MFER tag63-11 incorrect length %i>6\n",len2); 
						double  fval; 
						fval = *(int64_t*) mfer_swap8b(buf+2, len2-2, hdr->FLAG.SWAP); 
						
						fval *= pow(10.0, buf[1]);
						if (buf[0]==1)  // s
							fval = 1.0/fval;

						hdr->CHANNEL[chan].SPR = lround(hdr->SPR * fval / hdr->SampleRate);
					}
					else if (tag2==12) {	// sensitivity 
						if (len2>6) 
							fprintf(stderr,"Warning MFER tag63-12 incorrect length %i>6\n", len2); 
						if (!MFER_PhysDimCodeTable[UnitCode]) 
							fprintf(stderr,"Warning MFER: unsupported physical unit (code=%i)\n", UnitCode); 

						hdr->CHANNEL[chan].PhysDimCode = MFER_PhysDimCodeTable[UnitCode];
						double cal = *(int64_t*) mfer_swap8b(buf+2, len2-2, hdr->FLAG.SWAP); 
						hdr->CHANNEL[chan].Cal = cal * pow(10.0,(int8_t)buf[1]); 
					}
/*					else if (tag2==0x0c)	// block length
					;
*/
					else {
						ifseek(hdr,len2,SEEK_CUR); 
						curPos += len2;
						if (VERBOSE_LEVEL==9)
							fprintf(stdout,"tag=63-%i (len=%i) not supported\n",tag,len); 
					}
					
				}
			}	
			else if (tag==64) {
				// preamble  
				fprintf(stdout,"Preamble: pos=%i|",curPos); 
				curPos += ifread(tmp,1,len,hdr);
				for (k=0; k<len; k++)
					fprintf(stdout,"%c",tmp[k]); 
				fprintf(stdout,"|\n"); 
			}	
			else if (tag==65) {
				// event table  
				curPos += ifread(buf,1,len,hdr);
				if (len>2) {
					hdr->EVENT.N++;
					hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS,hdr->EVENT.N*sizeof(*hdr->EVENT.POS));
					hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP,hdr->EVENT.N*sizeof(*hdr->EVENT.TYP));
					hdr->EVENT.DUR = (uint32_t*) realloc(hdr->EVENT.DUR,hdr->EVENT.N*sizeof(*hdr->EVENT.DUR));
					hdr->EVENT.CHN = (uint16_t*) realloc(hdr->EVENT.CHN,hdr->EVENT.N*sizeof(*hdr->EVENT.CHN));

					hdr->EVENT.CHN[hdr->EVENT.N] = 0;
					hdr->EVENT.DUR[hdr->EVENT.N] = 0;
					if (hdr->FLAG.SWAP) {
						hdr->EVENT.TYP[hdr->EVENT.N] = bswap_16(*(uint16_t*)(buf));
						hdr->EVENT.POS[hdr->EVENT.N] = bswap_32(*(uint32_t*)(buf+2));
						if (len>6)
							hdr->EVENT.DUR[hdr->EVENT.N] = bswap_32(*(uint32_t*)(buf+6));
					}
					else {
						hdr->EVENT.TYP[hdr->EVENT.N] = *(uint16_t*)buf;
						hdr->EVENT.POS[hdr->EVENT.N] = *(uint32_t*)(buf+2);
						if (len>6)
							hdr->EVENT.DUR[hdr->EVENT.N] = *(uint32_t*)(buf+6);
					}		
				}		
			}

			else if (tag==129) {
				if (!hdr->FLAG.ANONYMOUS)
					curPos += ifread(hdr->Patient.Name,1,len,hdr);
				else 	{
					ifseek(hdr,len,SEEK_CUR); 
					curPos += len; 
				}		
			}	

			else if (tag==130) {
				// Patient Id 
				if (len>64) fprintf(stderr,"Warning MFER tag131 incorrect length %i>64\n",len); 
				if (len>MAX_LENGTH_PID) {
					ifread(hdr->Patient.Id,1,MAX_LENGTH_PID,hdr);
					ifseek(hdr,MAX_LENGTH_PID-len,SEEK_CUR);
					curPos += len;
				}	
				else
					curPos += ifread(hdr->Patient.Id,1,len,hdr);
			}	

			else if (tag==131) {
				// Patient Age 
				if (len!=7) fprintf(stderr,"Warning MFER tag131 incorrect length %i!=7\n",len); 
				curPos += ifread(buf,1,len,hdr);
				tm_time.tm_year = *(uint16_t*)(buf+3);
				if (hdr->FLAG.SWAP) tm_time.tm_year = bswap_16(tm_time.tm_year);
		    		tm_time.tm_year-= 1900;
		    		tm_time.tm_mon  = buf[5]-1; 
		    		tm_time.tm_mday = buf[6]; 
		    		tm_time.tm_hour = 12; 
		    		tm_time.tm_min  = 0; 
		    		tm_time.tm_sec  = 0; 
				hdr->Patient.Birthday  = t_time2gdf_time(mktime(&tm_time)); 
				//hdr->Patient.Age = buf[0] + cswap_u16(*(uint16_t*)(buf+1))/365.25;
			}	
			else if (tag==132) {
				// Patient Sex 
				if (len!=1) fprintf(stderr,"Warning MFER tag132 incorrect length %i!=1\n",len); 
				curPos += ifread(&hdr->Patient.Sex,1,len,hdr);
			}	
			else if (tag==133) {
				curPos += ifread(buf,1,len,hdr);
				tm_time.tm_year = *(uint16_t*)buf;
				if (hdr->FLAG.SWAP) tm_time.tm_year = bswap_16(tm_time.tm_year);
				tm_time.tm_year-= 1900;
		    		tm_time.tm_mon  = buf[2] - 1; 
		    		tm_time.tm_mday = buf[3]; 
		    		tm_time.tm_hour = buf[4]; 
		    		tm_time.tm_min  = buf[5]; 
		    		tm_time.tm_sec  = buf[6]; 
				
				hdr->T0  = t_time2gdf_time(mktime(&tm_time)); 
				// add milli- and micro-seconds
				if (hdr->FLAG.SWAP) 
					hdr->T0 += (uint64_t) ( (bswap_16(*(uint16_t*)(buf+7)) * 1e+3 + bswap_16(*(uint16_t*)(buf+9))) * ldexp(1.0,32) / (24*3600e6) );
				else
					hdr->T0 += (uint64_t) ( (        (*(uint16_t*)(buf+7)) * 1e+3 +         (*(uint16_t*)(buf+9))) * ldexp(1.0,32) / (24*3600e6) );
			}
			else	{ 	
		    		curPos += len; 
		    		ifseek(hdr,len,SEEK_CUR);
				if (VERBOSE_LEVEL==9)
					fprintf(stdout,"tag=%i (len=%i) not supported\n",tag,len); 
		    	}	

		    	if (curPos != iftell(hdr))
		    		fprintf(stdout,"positions differ %i %li \n",curPos,iftell(hdr));
				
			/* TAG */ 
			int sz=ifread(&tag,1,1,hdr);
			curPos += sz;
	 	}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// overflow detection OFF - not supported
	 	for (k=0; k<hdr->NS; k++) {
	 		if (!hdr->CHANNEL[k].PhysDimCode) hdr->CHANNEL[k].PhysDimCode = MFER_PhysDimCodeTable[UnitCode]; 
	 		if (hdr->CHANNEL[k].Cal==1.0) hdr->CHANNEL[k].Cal = Cal;
	 		hdr->CHANNEL[k].Off = Off * hdr->CHANNEL[k].Cal;
	 		if (!hdr->CHANNEL[k].SPR) hdr->CHANNEL[k].SPR = hdr->SPR; 
	 		hdr->CHANNEL[k].GDFTYP = gdftyp; 
	 		if (gdftyp<16)
	 			if (gdftyp & 0x01) {
		 			hdr->CHANNEL[k].DigMax = ldexp( 1.0,GDFTYP_BYTE[gdftyp]*8-1) - 1.0; 
		 			hdr->CHANNEL[k].DigMin = ldexp(-1.0,GDFTYP_BYTE[gdftyp]*8-1); 
	 			}	
	 			else {	
	 				hdr->CHANNEL[k].DigMax = ldexp( 1.0,GDFTYP_BYTE[gdftyp]*8); 
		 			hdr->CHANNEL[k].DigMin = 0.0; 
	 			}	
	 		else {	
	 			hdr->CHANNEL[k].DigMax =  INF; 
		 		hdr->CHANNEL[k].DigMin = -INF; 
	 		}
	 		hdr->CHANNEL[k].PhysMax = hdr->CHANNEL[k].DigMax * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off; 
	 		hdr->CHANNEL[k].PhysMin = hdr->CHANNEL[k].DigMin * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off; 
	 	}	
	    	hdr->FILE.POS = 0; 
	}

	else if (hdr->TYPE==SCP_ECG) {
		hdr->HeadLen   = leu32p(hdr->AS.Header+2);
		hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,hdr->HeadLen);
	    	count += ifread(hdr->AS.Header+count, 1, hdr->HeadLen-count, hdr);
	    	uint16_t crc   = CRCEvaluate(hdr->AS.Header+2,hdr->HeadLen-2);

	    	if ( leu16p(hdr->AS.Header) != crc) {
	    		B4C_ERRNUM = B4C_CRC_ERROR;
	    		B4C_ERRMSG = "Warning SOPEN(SCP-READ): Bad CRC!";  
		}

		sopen_SCP_read(hdr);
		serror();
		hdr->FLAG.SWAP = 0; 	// no swapping
	    	hdr->FILE.POS = 0; 

	}
	
	else if (hdr->TYPE==SND) {
		/* read file */ 
		hdr->FLAG.SWAP  = (__BYTE_ORDER == __LITTLE_ENDIAN);
		hdr->HeadLen  	= beu32p(hdr->AS.Header+4);
		size_t datlen 	= beu32p(hdr->AS.Header+8);
		uint32_t filetype = beu32p(hdr->AS.Header+12);
		hdr->SampleRate	= (double)beu32p(hdr->AS.Header+16);
		hdr->NS	      	= beu32p(hdr->AS.Header+20);
		if (count<hdr->HeadLen) {
		    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,hdr->HeadLen);
		    	count  += ifread(hdr->AS.Header+count,1,hdr->HeadLen-count,hdr);
		}
		else {
		    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,hdr->HeadLen);
			ifseek(hdr,hdr->HeadLen,SEEK_SET);
		}
	    	const uint16_t	SND_GDFTYP[] = {0,2,1,3,255+24,5,16,17,0,0,0,2,4,511+25,6};
		uint16_t gdftyp = SND_GDFTYP[filetype];
		hdr->AS.bpb = hdr->NS * GDFTYP_BYTE[gdftyp];	
		double Cal = 1;
		if ((filetype>1) && (filetype<6))
			Cal = ldexp(1,-8*GDFTYP_BYTE[gdftyp]);

		hdr->NRec = datlen/hdr->AS.bpb;
		hdr->SPR  = 1;
		
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// automated overflow and saturation detection not supported
		hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS, sizeof(CHANNEL_TYPE));
		double digmax = ldexp(1,8*GDFTYP_BYTE[gdftyp]);
		for (k=0; k < hdr->NS; k++) {
			CHANNEL_TYPE* hc = hdr->CHANNEL+k;
			hc->OnOff    = 1;
			hc->GDFTYP   = gdftyp;
			hc->SPR      = 1; 
			hc->Cal      = Cal; 
			hc->Off      = 0.0;
			hc->Transducer[0] = '\0';
			hc->LowPass  = -1;
			hc->HighPass = -1;
			hc->PhysMax  =  1.0;
			hc->PhysMin  = -1.0;
			hc->DigMax   =  digmax;
			hc->DigMin   = -digmax;
		    	hc->LeadIdCode  = 0;
		    	hc->PhysDimCode = 0;
		    	hc->Label[0] = 0;
		}
	}
	
	else if (hdr->TYPE==AIFF) {
		hdr->FLAG.SWAP  = (__BYTE_ORDER == __LITTLE_ENDIAN);
		uint8_t  *tag;
		uint32_t tagsize;
		uint16_t gdftyp;
		size_t pos; 
		tagsize  = beu32p(hdr->AS.Header+4);
		tagsize += tagsize & 0x0001;
		pos 	 = 12;
		tag	 = hdr->AS.Header+pos;
		while (1) {
			tagsize  = beu32p(hdr->AS.Header+pos+4);
			tagsize += tagsize & 0x0001;
			if (!strncmp((char*)tag,"COMM",4)) {
			}
			else if (!strncmp((char*)tag,"DATA",4)) {
			}
		
			pos += tagsize;
			tag  = hdr->AS.Header+pos;
		}
	}
	
	else if ((hdr->TYPE==WAV)||(hdr->TYPE==AVI)||(hdr->TYPE==RIFF)) {
		hdr->FLAG.SWAP  = (__BYTE_ORDER == __BIG_ENDIAN);
		uint8_t *tag;
		uint32_t tagsize;
		uint16_t gdftyp;
		uint16_t format=0, bits = 0;
		double Cal=1.0, Off=0.0; 
		size_t pos; 
		tagsize  = leu32p(hdr->AS.Header+4);
		tagsize += tagsize & 0x0001;
		pos 	 = 12;
		tag	 = hdr->AS.Header+pos;
		while (1) {
			tagsize  = leu32p(hdr->AS.Header+pos+4);
			tagsize += tagsize & 0x0001;
			if (!strncmp((char*)tag,"fmt ",4)) {
				format	 	= leu16p(hdr->AS.Header+pos+4);
				hdr->NS 	= leu16p(hdr->AS.Header+pos+4+2);
				hdr->SampleRate = (double)leu32p(hdr->AS.Header+pos+4+4);
				if (format==1) {
					bits 	= leu16p(hdr->AS.Header+pos+4+14);
					Cal 	= ldexp(1,-8*ceil(bits/8.0));
					if 	(bits <= 8) {
						gdftyp = 2;
						Off = 0.5;
					}	 
					else if (bits <= 16)
						gdftyp = 3; 
					else if (bits <= 24)
						gdftyp = 255+24; 
					else if (bits <= 32)
						gdftyp = 5; 
				}
				else
					fprintf(stdout,"Warning (WAV): format not supported.\n");
			}
			else if (!strncmp((char*)tag,"data",4)) {
				if (format==1) {
					hdr->AS.bpb = hdr->NS * ceil(bits/8.0);
					hdr->SPR    = tagsize/hdr->AS.bpb; 
				}
			}
		
			pos += tagsize;
			tag  = hdr->AS.Header+pos;
		}
	}	

	else if (hdr->TYPE==HL7aECG) {
		sopen_HL7aECG_read(hdr);
    		if (serror()) return(hdr);
    		hdr->FLAG.SWAP = 0; 
	}

	else 
	{
    		B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
    		B4C_ERRMSG = "ERROR BIOSIG SOPEN(READ): data format is not supported";		
    		ifclose(hdr);
    		return(hdr);
	}

	hdr->FILE.POS = 0; 

/* this will become obsolote because HDR.Dur is depreciated */
	if (hdr->SPR*hdr->Dur[1] != hdr->SampleRate*hdr->Dur[0]) {
		// set duration if it was not properly set 
		double Dur = hdr->SPR/hdr->SampleRate; 
		double dtmp1, dtmp2;
		dtmp2 = modf(Dur, &dtmp1); 
		// approximate real with rational number 
		if (fabs(dtmp2) < DBL_EPSILON) {
			hdr->Dur[0] = lround(Dur); 
			hdr->Dur[1] = 1; 
		}
		else {
			hdr->Dur[1] = lround(1.0 / dtmp2 ); 
			hdr->Dur[0] = lround(1.0 + dtmp1 * hdr->Dur[1]); 
		}		
	}

	for (k=0; k<hdr->NS; k++) {	

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[GDF 219] #=%i\n",k);

		// set HDR.PhysDim
		k1 = hdr->CHANNEL[k].PhysDimCode;
		if (k1>0)
			PhysDim(k1,hdr->CHANNEL[k].PhysDim);
		else 
			hdr->CHANNEL[k].PhysDimCode = PhysDimCode(hdr->CHANNEL[k].PhysDim);

		// set HDR.PhysDimCode
		if (hdr->CHANNEL[k].LeadIdCode == 0) {
		if (!strncmp(hdr->CHANNEL[k].Label, "MDC_ECG_LEAD_", 13)) {
			// MDC_ECG_LEAD_*  - ignore case  //
			for (k1=0; strcmpi(hdr->CHANNEL[k].Label+13,LEAD_ID_TABLE[k1]) && LEAD_ID_TABLE[k1][0]; k1++);
			if (LEAD_ID_TABLE[k1][0])	
				hdr->CHANNEL[k].LeadIdCode = k1;
		}
		else {
			for (k1=0; strcmp(hdr->CHANNEL[k].Label, LEAD_ID_TABLE[k1]) && LEAD_ID_TABLE[k1][0]; k1++); 
			if (LEAD_ID_TABLE[k1][0])	
				hdr->CHANNEL[k].LeadIdCode = k1;
		}}

		if (hdr->CHANNEL[k].LeadIdCode)
			strcpy(hdr->CHANNEL[k].Label,LEAD_ID_TABLE[hdr->CHANNEL[k].LeadIdCode]);

	}
}
else if (!strncmp(MODE,"w",1))	 /* --- WRITE --- */
{
	for (k=0; k<hdr->NS; k++) {	
		// set HDR.PhysDim
		k1 = hdr->CHANNEL[k].PhysDimCode;
		if (k1>0) PhysDim(k1,hdr->CHANNEL[k].PhysDim);
	}
	if (!strlen(hdr->Patient.Id))
		strcpy(hdr->Patient.Id,"00000000"); 

#ifdef __sparc__
	fprintf(stdout,"Warning SOPEN: Alignment errors might cause Bus Error (SPARC platform).\nWe are aware of the problem but it is not fixed yet.\n");
#endif

    	if (hdr->TYPE==CFWB) {	
	     	hdr->HeadLen = 68 + hdr->NS*96;
	    	hdr->AS.Header = (uint8_t*)malloc(hdr->HeadLen);
	    	uint8_t* Header2 = hdr->AS.Header+68; 
		memset(Header1,0,hdr->HeadLen);
	    	memcpy(Header1,"CFWB\1\0\0\0",8);
	    	*(double*)(Header1+8) = l_endian_f64(1/hdr->SampleRate);
		
		tt = gdf_time2t_time(hdr->T0); 
		struct tm *t = gmtime(&tt);
    		*(uint32_t*)(Header1+16) = l_endian_u32(t->tm_year + 1900);
	    	*(uint32_t*)(Header1+20) = l_endian_u32(t->tm_mon + 1);
	    	*(uint32_t*)(Header1+24) = l_endian_u32(t->tm_mday);
	    	*(uint32_t*)(Header1+28) = l_endian_u32(t->tm_hour);
	    	*(uint32_t*)(Header1+32) = l_endian_u32(t->tm_min);
	    	*(double*)  (Header1+36) = l_endian_f64(t->tm_sec);
	    	*(double*)  (Header1+44) = l_endian_f64(0.0);	// pretrigger time 
	    	*(uint32_t*)(Header1+52) = l_endian_u32(hdr->NS);
	    	hdr->NRec *= hdr->SPR; hdr->SPR = 1;
	    	*(uint32_t*)(Header1+56) = l_endian_u32(hdr->NRec); // number of samples 
	    	*(int32_t*) (Header1+60) = l_endian_i32(0);	// 1: time channel

	    	int32_t gdftyp = 3; // 1:double, 2:float, 3: int16; see CFWB_GDFTYP too. 
		for (k=0; k<hdr->NS; k++) {
			/* if int16 is not sufficient, use float or double */
			if (hdr->CHANNEL[k].GDFTYP>16)
				gdftyp = 1;	// double 
			else if (hdr->CHANNEL[k].GDFTYP>3)
				gdftyp = 2;	// float 
		}
	    	*(int32_t*)(Header1+64)	= l_endian_i32(gdftyp);	// 1: double, 2: float, 3:short
		
		for (k=0; k<hdr->NS; k++) {
	    		hdr->CHANNEL[k].SPR = 1;
			hdr->CHANNEL[k].GDFTYP = CFWB_GDFTYP[gdftyp-1];
			const char *tmpstr;
			if (hdr->CHANNEL[k].LeadIdCode)
				tmpstr = LEAD_ID_TABLE[hdr->CHANNEL[k].LeadIdCode];
			else
				tmpstr = hdr->CHANNEL[k].Label;
		     	size_t len = strlen(tmpstr);
		     	memcpy(Header2+96*k, tmpstr, min(len,32));

		     	PhysDim(hdr->CHANNEL[k].PhysDimCode, tmp);
		     	len = strlen(tmp);
		     	memcpy(Header2+96*k+32, tmp, min(len,32));
			
			*(double*)(Header2+96*k+64) = l_endian_f64(hdr->CHANNEL[k].Cal);
			*(double*)(Header2+96*k+72) = l_endian_f64(hdr->CHANNEL[k].Off);
			*(double*)(Header2+96*k+80) = l_endian_f64(hdr->CHANNEL[k].PhysMax);
			*(double*)(Header2+96*k+88) = l_endian_f64(hdr->CHANNEL[k].PhysMin);
		}
	}

    	else if ((hdr->TYPE==GDF) || (hdr->TYPE==GDF1)) {	
		uint32_t Dur[2];
	     	hdr->HeadLen = (hdr->NS+1)*256;
	    	hdr->AS.Header = (uint8_t*) malloc(hdr->HeadLen);
	    	uint8_t* Header2 = hdr->AS.Header+256; 

		memset(Header1,0,hdr->HeadLen);
		hdr->VERSION = 1.99;
		if (hdr->TYPE==GDF1) hdr->VERSION = 1.25;
	     	sprintf(Header1,"GDF %4.2f",hdr->VERSION);
	     	
		uint16_t maxlen=66; 
		if (hdr->VERSION<1.90) maxlen=80;
		if (strlen(hdr->Patient.Id) > 0) {
			for (k=0; hdr->Patient.Id[k]; k++)
				if (isspace(hdr->Patient.Id[k]))
					hdr->Patient.Id[k] = '_';
					
	     		strncat(Header1+8, hdr->Patient.Id, maxlen);
		} 
		else     	
		     	strncat(Header1+8, "X", maxlen);

	     	strncat(Header1+8, " ", maxlen);
		if (!hdr->FLAG.ANONYMOUS && (hdr->Patient.Name!=NULL)) 
		     	strncat(Header1+8, hdr->Patient.Name, maxlen);
		else     	
		     	strncat(Header1+8, "X", maxlen);

		if (hdr->VERSION>1.90) { 
	     		Header1[84] = (hdr->Patient.Smoking%4) + ((hdr->Patient.AlcoholAbuse%4)<<2) + ((hdr->Patient.DrugAbuse%4)<<4) + ((hdr->Patient.Medication%4)<<6);
	     		Header1[85] =  hdr->Patient.Weight;
	     		Header1[86] =  hdr->Patient.Height;
	     		Header1[87] = (hdr->Patient.Sex%4) + ((hdr->Patient.Handedness%4)<<2) + ((hdr->Patient.Impairment.Visual%4)<<4);
		}
		
	     	size_t len = strlen(hdr->ID.Recording);
	     	memcpy(Header1+ 88,  hdr->ID.Recording, min(len,80));
		if (hdr->VERSION>1.90) { 
			memcpy(Header1+152, &hdr->LOC, 16);  
			*(uint32_t*) (Header1+152) = l_endian_u32( *(uint32_t*) (Header1+152) );
			*(uint32_t*) (Header1+156) = l_endian_u32( *(uint32_t*) (Header1+156) );
			*(uint32_t*) (Header1+160) = l_endian_u32( *(uint32_t*) (Header1+160) );
			*(uint32_t*) (Header1+164) = l_endian_u32( *(uint32_t*) (Header1+164) );
		}
		
		if (hdr->VERSION<1.90) { 
			tt = gdf_time2t_time(hdr->T0); 
			struct tm *t = gmtime(&tt);
			sprintf(tmp,"%04i%02i%02i%02i%02i%02i00",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
			memcpy(Header1+168,tmp,max(strlen(tmp),16));
			*(uint32_t*) (Header1+184) = l_endian_u32(hdr->HeadLen);

			memcpy(Header1+192, &hdr->ID.Equipment, 8);
			// FIXME: 200: LabId, 208 TechId, 216, Serial No // 
		}
		else {
			//memcpy(Header1+168, &hdr->T0, 8); 
			*(uint64_t*) (Header1+168) = l_endian_u64(hdr->T0);
			//memcpy(Header1+176, &hdr->Patient.Birthday, 8); 
			*(uint64_t*) (Header1+176) = l_endian_u64(hdr->Patient.Birthday);
			// *(uint16_t*)(Header1+184) = (hdr->HeadLen>>8)+(hdr->HeadLen%256>0); 
			*(uint32_t*) (Header1+184) = l_endian_u32(hdr->HeadLen>>8);
		
			memcpy(Header1+192, &hdr->ID.Equipment, 8);
			memcpy(Header1+200, &hdr->IPaddr, 6);
			memcpy(Header1+206, &hdr->Patient.Headsize, 6);
			*(float*) (Header1+212) = l_endian_f32(hdr->ELEC.REF[0]);
			*(float*) (Header1+216) = l_endian_f32(hdr->ELEC.REF[1]);
			*(float*) (Header1+220) = l_endian_f32(hdr->ELEC.REF[2]);
			*(float*) (Header1+224) = l_endian_f32(hdr->ELEC.GND[0]);
			*(float*) (Header1+228) = l_endian_f32(hdr->ELEC.GND[1]);
			*(float*) (Header1+232) = l_endian_f32(hdr->ELEC.GND[2]);
		}
		
		*(uint64_t*) (Header1+236) = l_endian_u64(hdr->NRec);

		/* Duration is expressed as an fraction of integers */ 
		double fDur = hdr->SPR/hdr->SampleRate;
		double dtmp1, dtmp2;
		dtmp2 = modf(fDur, &dtmp1);
		// approximate real with rational number 
		if (fabs(dtmp2) < DBL_EPSILON) {
			Dur[0] = lround(fDur); 
			Dur[1] = 1; 
		}
		else {   
			Dur[1] = lround(1.0 / dtmp2 ); 
			Dur[0] = lround(1.0 + dtmp1 * Dur[1]); 
		}		
		fprintf(stdout,"\n SOPEN(GDF write): %i/%f to %i/%i\n",hdr->SPR,hdr->SampleRate,Dur[0],Dur[1]);
		*(uint32_t*) (Header1+244) = l_endian_u32(Dur[0]);
		*(uint32_t*) (Header1+248) = l_endian_u32(Dur[1]);
		*(uint16_t*) (Header1+252) = l_endian_u16(hdr->NS);

	     	/* define HDR.Header2 
	     	this requires checking the arguments in the fields of the struct HDR.CHANNEL
	     	and filling in the bytes in HDR.Header2. 
	     	*/
		for (k=0; k<hdr->NS; k++) {
			const char *tmpstr;
			if (hdr->CHANNEL[k].LeadIdCode)
				tmpstr = LEAD_ID_TABLE[hdr->CHANNEL[k].LeadIdCode];
			else
				tmpstr = hdr->CHANNEL[k].Label;
		     	len = strlen(tmpstr);
		     	memcpy(Header2+16*k,tmpstr,min(len,16));

		     	len = strlen(hdr->CHANNEL[k].Transducer);
		     	memcpy(Header2+80*k + 16*hdr->NS, hdr->CHANNEL[k].Transducer, min(len,80));
		     	PhysDim(hdr->CHANNEL[k].PhysDimCode, tmp);
		     	len = strlen(tmp);
		     	if (hdr->VERSION < 1.9)
		     		memcpy(Header2+ 8*k + 96*hdr->NS, tmp, min(8,len));
		     	else {
		     		memcpy(Header2+ 6*k + 96*hdr->NS, tmp, min(6,len));
		     		*(uint16_t*)(Header2+ 2*k + 102*hdr->NS) = l_endian_u16(hdr->CHANNEL[k].PhysDimCode);
			};

		     	*(double*)(Header2 + 8*k + 104*hdr->NS)   = l_endian_f64(hdr->CHANNEL[k].PhysMin);
		     	*(double*)(Header2 + 8*k + 112*hdr->NS)   = l_endian_f64(hdr->CHANNEL[k].PhysMax);
		     	if (hdr->VERSION < 1.9) {
			     	*(int64_t*)(Header2 + 8*k + 120*hdr->NS)   = l_endian_i64((int64_t)hdr->CHANNEL[k].DigMin);
			     	*(int64_t*)(Header2 + 8*k + 128*hdr->NS)   = l_endian_i64((int64_t)hdr->CHANNEL[k].DigMax);
			     	// FIXME // memcpy(Header2 + 80*k + 136*hdr->NS,hdr->CHANNEL[k].PreFilt,max(80,strlen(hdr->CHANNEL[k].PreFilt)));
			}     	
			else {
			     	*(double*)(Header2 + 8*k + 120*hdr->NS) = l_endian_f64(hdr->CHANNEL[k].DigMin);
			     	*(double*)(Header2 + 8*k + 128*hdr->NS) = l_endian_f64(hdr->CHANNEL[k].DigMax);
			     	*(float*) (Header2 + 4*k + 204*hdr->NS) = l_endian_f32(hdr->CHANNEL[k].LowPass);
			     	*(float*) (Header2 + 4*k + 208*hdr->NS) = l_endian_f32(hdr->CHANNEL[k].HighPass);
			     	*(float*) (Header2 + 4*k + 212*hdr->NS) = l_endian_f32(hdr->CHANNEL[k].Notch);

				*(float*) (Header2 + 4*k + 224*hdr->NS) = l_endian_f32(hdr->CHANNEL[k].XYZ[0]);
				*(float*) (Header2 + 4*k + 228*hdr->NS) = l_endian_f32(hdr->CHANNEL[k].XYZ[1]);
				*(float*) (Header2 + 4*k + 232*hdr->NS) = l_endian_f32(hdr->CHANNEL[k].XYZ[2]);

	     			Header2[k+236*hdr->NS] = (uint8_t)ceil(log10(min(39e8,hdr->CHANNEL[k].Impedance))/log10(2.0)*8.0-0.5);
		     	}
		     	*(uint32_t*) (Header2 + 4*k + 216*hdr->NS) = l_endian_u32(hdr->CHANNEL[k].SPR);
		     	*(uint32_t*) (Header2 + 4*k + 220*hdr->NS) = l_endian_u32(hdr->CHANNEL[k].GDFTYP);
		}
	    	hdr->AS.bi = (uint32_t*) realloc(hdr->AS.bi,(hdr->NS+1)*sizeof(int32_t));
		hdr->AS.bi[0] = 0;
		for (k=0, hdr->AS.spb=0, hdr->AS.bpb=0; k<hdr->NS;) {
			hdr->AS.spb += hdr->CHANNEL[k].SPR;
			hdr->AS.bpb += GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP] * hdr->CHANNEL[k].SPR;
			hdr->AS.bi[++k] = hdr->AS.bpb;
		}	
//		hdr->AS.Header1 = (uint8_t*)Header1; 
	}

    	else if ((hdr->TYPE==EDF) | (hdr->TYPE==BDF)) {	
	     	hdr->HeadLen   = (hdr->NS+1)*256;
	    	hdr->AS.Header = (uint8_t*)malloc(hdr->HeadLen);
	    	char* Header2  = (char*)hdr->AS.Header+256; 
		memset(Header1,' ',hdr->HeadLen);
		if (hdr->TYPE==BDF) {	
			Header1[0] = 255;
	     		memcpy(Header1+1,"BIOSEMI",7);
		}
		else {
			Header1[0] = '0';
	     	}

		tt = gdf_time2t_time(hdr->Patient.Birthday); 
		if (hdr->Patient.Birthday>1) strftime(tmp,81,"%d-%b-%Y",gmtime(&tt));
		else strcpy(tmp,"X");	
		
		if (strlen(hdr->Patient.Id) > 0) 
			for (k=0; hdr->Patient.Id[k]; k++)
				if (isspace(hdr->Patient.Id[k]))
					hdr->Patient.Id[k] = '_';

	    	char cmd[256];
		if (!hdr->FLAG.ANONYMOUS)
			sprintf(cmd,"%s %c %s %s",hdr->Patient.Id,GENDER[hdr->Patient.Sex],tmp,hdr->Patient.Name);
		else	
			sprintf(cmd,"%s %c %s X",hdr->Patient.Id,GENDER[hdr->Patient.Sex],tmp);
			
	     	memcpy(Header1+8, cmd, strlen(cmd));
	     	
		tt = gdf_time2t_time(hdr->T0); 
		if (hdr->T0>1) strftime(tmp,81,"%d-%b-%Y",gmtime(&tt));
		else strcpy(tmp,"X");	
		size_t len = sprintf(cmd,"Startdate %s X X ",tmp);
	     	memcpy(Header1+88, cmd, len);
	     	memcpy(Header1+88+len, &hdr->ID.Equipment, 8);
	     
		tt = gdf_time2t_time(hdr->T0); 
		strftime(tmp,81,"%d.%m.%y%H.%M.%S",gmtime(&tt));
	     	memcpy(Header1+168, tmp, 16);

		len = sprintf(tmp,"%i",hdr->HeadLen);
		if (len>8) fprintf(stderr,"Warning: HeaderLength is (%s) too long (%i>8).\n",tmp,len);  
	     	memcpy(Header1+184, tmp, len);
	     	memcpy(Header1+192, "EDF+C  ", 5);

		len = sprintf(tmp,"%Lu",hdr->NRec);
		if (len>8) fprintf(stderr,"Warning: NRec is (%s) too long (%i>8).\n",tmp,len);  
	     	memcpy(Header1+236, tmp, len);

		len = sprintf(tmp,"%f",hdr->SPR/hdr->SampleRate);
		if (len>8) fprintf(stderr,"Warning: Duration is (%s) too long (%i>8).\n",tmp,len);  
	     	memcpy(Header1+244, tmp, len);

		len = sprintf(tmp,"%i",hdr->NS);
		if (len>4) fprintf(stderr,"Warning: NS is (%s) too long (%i>4).\n",tmp,len);  
	     	memcpy(Header1+252, tmp, len);
	     	
		for (k=0;k<hdr->NS;k++) {
			const char *tmpstr;
			if (hdr->CHANNEL[k].LeadIdCode)
				tmpstr = LEAD_ID_TABLE[hdr->CHANNEL[k].LeadIdCode];
			else
				tmpstr = hdr->CHANNEL[k].Label;
		     	len = strlen(tmpstr);
			if (len>15)
			//fprintf(stderr,"Warning: Label (%s) of channel %i is to long.\n",hdr->CHANNEL[k].Label,k);
		     	fprintf(stderr,"Warning: Label of channel %i is too long (%i>16).\n",k, len);
		     	memcpy(Header2+16*k,tmpstr,min(len,16));
		     	len = strlen(hdr->CHANNEL[k].Transducer);
			if (len>80) fprintf(stderr,"Warning: Transducer of channel %i is too long (%i>80).\n",k, len);  
		     	memcpy(Header2+80*k + 16*hdr->NS,hdr->CHANNEL[k].Transducer,min(len,80));
		     	PhysDim(hdr->CHANNEL[k].PhysDimCode, tmp);
		     	len = strlen(tmp);
		     	if (len>8) fprintf(stderr,"Warning: Physical Dimension (%s) of channel %i is too long (%i>8).\n",tmp,k,len);
		     	memcpy(Header2 + 8*k + 96*hdr->NS, tmp, min(len,8));

			if (ftoa8(tmp,hdr->CHANNEL[k].PhysMin))
				fprintf(stderr,"Warning: PhysMin (%f) of channel %i does not fit into 8 bytes of EDF header.\n",hdr->CHANNEL[k].PhysMin,k);
		     	memcpy(Header2 + 8*k + 104*hdr->NS, tmp, strlen(tmp));
			if (ftoa8(tmp,hdr->CHANNEL[k].PhysMax))
				fprintf(stderr,"Warning: PhysMax (%f) of channel %i does not fit into 8 bytes of EDF header.\n",hdr->CHANNEL[k].PhysMax,k);
		     	memcpy(Header2 + 8*k + 112*hdr->NS, tmp, strlen(tmp));
			if (ftoa8(tmp,hdr->CHANNEL[k].DigMin))
				fprintf(stderr,"Warning: DigMin (%f) of channel %i does not fit into 8 bytes of EDF header.\n",hdr->CHANNEL[k].DigMin,k);
		     	memcpy(Header2 + 8*k + 120*hdr->NS, tmp, strlen(tmp));
			if (ftoa8(tmp,hdr->CHANNEL[k].DigMax))
				fprintf(stderr,"Warning: DigMax (%f) of channel %i does not fit into 8 bytes of EDF header.\n",hdr->CHANNEL[k].DigMax,k);
		     	memcpy(Header2 + 8*k + 128*hdr->NS, tmp, strlen(tmp));
		     	
			if (hdr->CHANNEL[k].Notch>0)		     	
				len = sprintf(tmp,"HP:%fHz LP:%fHz Notch:%fHz",hdr->CHANNEL[k].HighPass,hdr->CHANNEL[k].LowPass,hdr->CHANNEL[k].Notch);
			else
				len = sprintf(tmp,"HP:%fHz LP:%fHz",hdr->CHANNEL[k].HighPass,hdr->CHANNEL[k].LowPass);
		     	memcpy(Header2+ 80*k + 136*hdr->NS,tmp,min(80,len));
		     	
			len = sprintf(tmp,"%i",hdr->CHANNEL[k].SPR);
			if (len>8) fprintf(stderr,"Warning: SPR (%s) of channel %i is to long (%i)>8.\n",tmp,k,len);  
		     	memcpy(Header2+ 8*k + 216*hdr->NS,tmp,min(8,len));
		     	hdr->CHANNEL[k].GDFTYP = ( (hdr->TYPE != BDF) ? 3 : 255+24);
		}
	}

    	else if (hdr->TYPE==HL7aECG) {	
   		hdr->FileName = FileName;
		for (k=0; k<hdr->NS; k++) {
			hdr->CHANNEL[k].GDFTYP = 5; //int32: internal datatype 
		}
		hdr->SPR *= hdr->NRec;
		hdr->NRec = 1; 
		hdr->FILE.OPEN = 2;
    		hdr->FLAG.SWAP = 0; 
	}

    	else if (hdr->TYPE==MFER) {	
    		uint8_t tag; 
    		size_t  len, curPos=0; 
    		hdr->FileName  = FileName;
	     	hdr->HeadLen   = 32+128+3*6+3 +80000;
	    	hdr->AS.Header = (uint8_t*)malloc(hdr->HeadLen);
		memset(Header1, ' ', hdr->HeadLen);

		hdr->FLAG.SWAP = ( __BYTE_ORDER == __LITTLE_ENDIAN);   // default of MFER is BigEndian
		
		fprintf(stderr,"Warning SOPEN(MFER): write support for MFER format under construction\n"); 
		// tag 64: preamble 
		// Header1[curPos] = 64; 
		// len =32;
		curPos = 34; 
		strncpy(Header1,"@  MFER                                ",curPos);
		// Header1[curPos+1] = len; 
		// curPos = len+2; 

		// tag 23: Manufacturer 
		tag = 23;
		Header1[curPos] = tag; 
		strcpy(Header1+curPos+2,hdr->ID.Manufacturer.Name);
		if (hdr->ID.Manufacturer.Model != NULL) {
			strcat(Header1+curPos+2,"^");
			strcat(Header1+curPos+2,hdr->ID.Manufacturer.Model);
		}	
		if (hdr->ID.Manufacturer.Version != NULL) {
			strcat(Header1+curPos+2,"^");
			strcat(Header1+curPos+2,hdr->ID.Manufacturer.Version);
		}	
		if (hdr->ID.Manufacturer.SerialNumber!=NULL) {
			strcat(Header1+curPos+2,"^");
			strcat(Header1+curPos+2,hdr->ID.Manufacturer.SerialNumber);
		}	
		len = strlen(Header1+curPos+2); 
		Header1[curPos] = tag; 
		if (len<128) {
			Header1[curPos+1] = len;
			curPos += len+2;
		} else if (len <= 0xffff) {
			Header1[curPos+1] = sizeof(uint16_t);
			*(uint16_t*)(Header1+curPos+2) = b_endian_u16(len);
			curPos += len+1+1+2;
		} else 	 
			fprintf(stderr,"Warning MFER(W) Tag23 (manufacturer) to long len=%i>128\n",len); 	

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"Write MFER: tag=%i,len%i,curPos=%i\n",tag,len,curPos); 	

		// tag 1: Endianity 
		// use default BigEndianity		
		 
		// tag 4: SPR 
		tag = 4;
		len = sizeof(uint32_t);
		Header1[curPos++] = tag; 
		Header1[curPos++] = len; 
		*(uint32_t*)(Header1+curPos) = b_endian_u32(hdr->SPR); 
		curPos += len; 

		// tag 5: NS 
		tag = 5;
		len = sizeof(uint16_t);
		Header1[curPos++] = tag; 
		Header1[curPos++] = len; 
		*(uint16_t*)(Header1+curPos) = b_endian_u16(hdr->NS); 
		curPos += len; 

		// tag 6: NRec 
		tag = 6;
		len = sizeof(uint32_t);
		Header1[curPos++] = tag; 
		Header1[curPos++] = len; 
		*(uint32_t*)(Header1+curPos) = b_endian_u32(hdr->NRec); 
		curPos += len; 

		// tag 8: Waveform: unidentified 
		tag = 8;
		len = sizeof(uint8_t);
		Header1[curPos++] = tag; 
		Header1[curPos++] = len; 
		*(Header1+curPos) = 0; // unidentified 
		curPos += len; 

		// tag 129: Patient Name  
		if (!hdr->FLAG.ANONYMOUS) {
			tag = 129;
			len = strlen(hdr->Patient.Name);
			Header1[curPos++] = tag; 
			Header1[curPos++] = len; 
			strcpy(Header1+curPos,hdr->Patient.Name); 
			curPos += len;
		}

		// tag 130: Patient Id  
		tag = 130;
		len = strlen(hdr->Patient.Id);
		Header1[curPos++] = tag; 
		Header1[curPos++] = len; 
		strcpy(Header1+curPos,hdr->Patient.Id); 
		curPos += len; 

		// tag 131: Patient Age  
		tag = 131;
		len = 7;
		tt = gdf_time2t_time(hdr->T0); 
		struct tm *t = gmtime(&tt);
		Header1[curPos++] = tag; 
		Header1[curPos++] = len; 
		*(Header1+curPos) = (uint8_t)((hdr->T0 - hdr->Patient.Birthday)/365.25); 
		double tmpf64 = (hdr->T0 - hdr->Patient.Birthday); 
		tmpf64 -= 365.25*floor(tmpf64/365.25);
		*(uint16_t*)(Header1+curPos+1) = b_endian_u16((uint16_t)tmpf64); 
		*(uint16_t*)(Header1+curPos+3) = b_endian_u16(t->tm_year+1900); 
		*(Header1+curPos+5) = (t->tm_mon+1); 
		*(Header1+curPos+6) = (t->tm_mday); 
		curPos += len; 

		// tag 132: Patient Sex  
		tag = 132;
		Header1[curPos]   = tag; 
		Header1[curPos+1] = 1; 
		Header1[curPos+2] = hdr->Patient.Sex; 
		curPos += 3; 


		// tag  9: LeadId
		// tag 10: gdftyp
		// tag 11: SampleRate
		// tag 12: Cal
		// tag 13: Off
		hdr->HeadLen = curPos; 		
		// tag 63: channel-specific settings
		
		tag = 63;
		size_t ch; 
		for (ch=0; ch<hdr->NS; k++) {
		 	// FIXME: this is broken 
			len = 0; 
			Header1[curPos++] = tag; 
			if (ch<128)
				Header1[curPos++] = ch;
			else {
				Header1[curPos++] = (ch >> 7) | 0x80;
				Header1[curPos++] = (ch && 0x7f);
			}
			// tag1  9: LeadId
			size_t ix = curPos; 
			size_t len1 = 0; 
			Header1[ix++] = 9;
			if (hdr->CHANNEL[ch].LeadIdCode>0) {
				Header1[ix++] = 1;
				Header1[ix++] = hdr->CHANNEL[ch].LeadIdCode;
			} else {	
				len1 = strlen(hdr->CHANNEL[ch].Label);
				Header1[ix++] = len1;
				strcpy(Header1+ ix, hdr->CHANNEL[ch].Label);
			}
			// tag1 10: gdftyp
			// tag1 11: SampleRate
			// tag1 12: Cal
			// tag1 13: Off

			len += len1+ix-curPos;
			*(Header1+curPos) = len;  
			curPos += len+curPos; 
		} 
		// tag 30: data
	}

    	else if (hdr->TYPE==SCP_ECG) {	
    		hdr->FileName = FileName;
    		sopen_SCP_write(hdr);
    		if (serror()) return(hdr);
	}

	else {
		B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
		B4C_ERRMSG = "ERROR: Writing of format not supported\n"; 
		return(NULL); 
	}

    	hdr->FileName = FileName; 
	if(hdr->TYPE != HL7aECG){
	    	hdr = ifopen(hdr,"wb");
		
		if (!hdr->FILE.COMPRESSION && (hdr->FILE.FID == NULL) ){
			B4C_ERRNUM = B4C_CANNOT_WRITE_FILE;
			B4C_ERRMSG = "ERROR: Unable to open file for writing.\n"; 
			return(NULL);
		}
#ifdef ZLIB_H	
		else if (hdr->FILE.COMPRESSION && (hdr->FILE.gzFID == NULL) ){
			B4C_ERRNUM = B4C_CANNOT_WRITE_FILE;
			B4C_ERRMSG = "ERROR: Unable to open file for writing.\n"; 
			return(NULL);
		}
#endif
		if(hdr->TYPE != SCP_ECG){
			ifwrite(Header1, sizeof(char), hdr->HeadLen, hdr);
		}	

		hdr->FILE.OPEN = 2;
		hdr->FILE.POS  = 0;
	}

}	// end of else 

	// internal variables
	if (VERBOSE_LEVEL>8) fprintf(stderr,"-4> #info: @%p\n",&(hdr->CHANNEL));

	hdr->AS.bi  = (uint32_t*) realloc(hdr->AS.bi,(hdr->NS+1)*sizeof(uint32_t));
	hdr->AS.bpb = (hdr->TYPE==AINF ? 4 : 0); 
	hdr->AS.bi[0] = hdr->AS.bpb;
	for (k=0, hdr->SPR = 1, hdr->AS.spb=0; k<hdr->NS; k++) {
		hdr->AS.spb += hdr->CHANNEL[k].SPR;
		hdr->AS.bpb += GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP] * hdr->CHANNEL[k].SPR;			
		hdr->AS.bi[k+1] = hdr->AS.bpb; 
		if (hdr->CHANNEL[k].SPR > 0)  // ignore sparse channels
			hdr->SPR = lcm(hdr->SPR, hdr->CHANNEL[k].SPR);

		if (VERBOSE_LEVEL>8) fprintf(stderr,"-4> Label #%02i: @%p %s\n",k,&(hdr->CHANNEL[k].Label),hdr->CHANNEL[k].Label);
	}
	if (hdr->TYPE==EGI) {
		hdr->AS.bpb += EGI_LENGTH_CODETABLE * GDFTYP_BYTE[hdr->CHANNEL[0].GDFTYP];
		if (hdr->AS.Header[3] & 0x01)	// triggered  
			hdr->AS.bpb += 6;
	}

	if (hdr->FILE.POS != 0)	
		fprintf(stdout,"Debugging Information: (Format=%d) FILE.POS=%d is not zero.\n",hdr->TYPE,hdr->FILE.POS);

		
	return(hdr);
}  // end of SOPEN 


#define ROW_BASED_CHANNELS_NO

/****************************************************************************/
/**	SREAD : segment-based                                              **/
/****************************************************************************/
size_t sread(biosig_data_type* data, size_t start, size_t length, HDRTYPE* hdr) {
/* 
 *	Reads LENGTH blocks with HDR.AS.bpb BYTES each 
 * 	(and HDR.SPR samples). 
 *	Rawdata is available in hdr->AS.rawdata.
 *      Output data is available in hdr->data.block. 
 *      If the request can be completed, hdr->data.block contains 
 *	LENGTH*HDR.SPR samples and HDR.NS channels. 
 *	The size of the output data is availabe in hdr->data.size.
 *
 *      hdr->FLAG.SWAP controls swapping 
 *
 *      hdr->CHANNEL[k].OnOff 	controls whether channel k is loaded or not 
 *
 *	data is a pointer to a memory array to write the data. 
 *	if data is NULL, memory is allocated and the pointer is returned 
 *	in hdr->data.block. 
 *
 *	channel selection is controlled by hdr->CHANNEL[k].OnOff
 * 
 *        start <0: read from current position
 *             >=0: start reading from position start
 *        length  : try to read length blocks
 *
 */

	size_t			count,k1,k2,k3,k4,k5,DIV,SZ,nelem,NS; 
	int 			GDFTYP;
	uint8_t*		ptr;
	CHANNEL_TYPE*		CHptr;
	int32_t			int32_value;
	biosig_data_type 	sample_value; 
	size_t			toffset = 0;	// time offset for rawdata

	switch (hdr->TYPE) {
	case ETG4000: toffset = start;	
	case HL7aECG: 		
	case SCP_ECG: {
		// hdr->AS.rawdata was defined in SOPEN	
		hdr->FILE.POS = start; 	
		count = max(min(length, hdr->NRec - hdr->FILE.POS),0);
		break; 
	}		
	default: {
		// check reading segment 
		// if ((start < 0) || (start > hdr->NRec)) 
		if (start > hdr->NRec) 
			return(0);
		else if (ifseek(hdr, start*hdr->AS.bpb + hdr->HeadLen, SEEK_SET)<0)
			return(0);
		hdr->FILE.POS = start; 	
		
		// allocate AS.rawdata 	
		hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata, (hdr->AS.bpb)*length);

		// limit reading to end of data block
		nelem = max(min(length, hdr->NRec - hdr->FILE.POS),0);

		// read data
		count = ifread(hdr->AS.rawdata, hdr->AS.bpb, nelem, hdr);
		if (count<nelem)
			fprintf(stderr,"warning: only %i instead of %i blocks read - something went wrong\n",count,nelem); 
		}
	}
	
	// set position of file handle 
	size_t POS = hdr->FILE.POS;
	hdr->FILE.POS += count;

	// count number of selected channels 
	for (k1=0,NS=0; k1<hdr->NS; ++k1)
		if (hdr->CHANNEL[k1].OnOff) ++NS; 

	if (VERBOSE_LEVEL>8) fprintf(stdout,"SREAD: pos=[%i,%i,%i,%i], size of data = %ix%ix%ix%i = %i\n",start,length,POS,hdr->FILE.POS,hdr->SPR, count, NS, sizeof(biosig_data_type), hdr->SPR * count * NS * sizeof(biosig_data_type));

	// transfer RAW into BIOSIG data format 
	if (data==NULL)
		data = (biosig_data_type*) malloc(hdr->SPR * count * NS * sizeof(biosig_data_type));
		
	hdr->data.block = data; 
//	hdr->data.block = (biosig_data_type*) realloc(hdr->data.block, (hdr->SPR) * count * NS * sizeof(biosig_data_type));

	for (k1=0,k2=0; k1<hdr->NS; k1++) {
		CHptr 	= hdr->CHANNEL+k1;

	if (CHptr->OnOff) {	/* read selected channels only */ 
	if (CHptr->SPR==0) {	
		// sparsely sampled channels are stored in event table
		for (k5 = 0; k5 < hdr->SPR*count; k5++)
#ifdef ROW_BASED_CHANNELS
			hdr->data.block[k2 + k5*NS] = NaN;		// row-based channels 
#else
			hdr->data.block[k2*count*hdr->SPR + k5] = NaN; 	// column-based channels 
#endif
	}
	else {
		DIV 	= hdr->SPR/CHptr->SPR; 
		GDFTYP 	= CHptr->GDFTYP;
		SZ  	= GDFTYP_BYTE[GDFTYP];
		int32_value = 0; 

		for (k4 = 0; k4 < count; k4++)
		for (k5 = 0; k5 < CHptr->SPR; k5++) 
		{
		if (hdr->FLAG.SWAP)		
		{

			// get source address 	
			ptr = hdr->AS.rawdata + (k4+toffset)*hdr->AS.bpb + hdr->AS.bi[k1] + k5*SZ;
			
			// mapping of raw data type to (biosig_data_type)
			if (0); 
			else if (GDFTYP==3) 
				sample_value = (biosig_data_type)(int16_t)bswap_16(*(int16_t*)ptr); 
			else if (GDFTYP==4)
				sample_value = (biosig_data_type)bswap_16(*(uint16_t*)ptr); 
			else if (GDFTYP==16) {
				union {uint32_t i32; float f32;} u; 
				u.i32 = bswap_32(*(uint32_t*)(ptr));
				sample_value = (biosig_data_type)(u.f32);
			}	
			else if (GDFTYP==17) {
				union {uint64_t i64; double f64;} u; 
				u.i64 = bswap_64(*(uint64_t*)(ptr));
				sample_value = (biosig_data_type)(u.f64);
			}	
			else if (GDFTYP==0)
				sample_value = (biosig_data_type)(*(char*)ptr); 
			else if (GDFTYP==1)
				sample_value = (biosig_data_type)(*(int8_t*)ptr); 
			else if (GDFTYP==2)
				sample_value = (biosig_data_type)(*(uint8_t*)ptr); 
			else if (GDFTYP==5)
				sample_value = (biosig_data_type)bswap_32(*(int32_t*)ptr); 
			else if (GDFTYP==6)
				sample_value = (biosig_data_type)bswap_32(*(uint32_t*)ptr); 
			else if (GDFTYP==7)
				sample_value = (biosig_data_type)bswap_64(*(int64_t*)ptr); 
			else if (GDFTYP==8)
				sample_value = (biosig_data_type)bswap_64(*(uint64_t*)ptr); 
			else if (GDFTYP==255+24) {
				// assume LITTLE_ENDIAN platform
				int32_value = (*(uint8_t*)(ptr)) + (*(uint8_t*)(ptr+1)<<8) + (*(int8_t*)(ptr+2)*(1<<16)); 
				sample_value = (biosig_data_type)int32_value; 
			}	
			else if (GDFTYP==511+24) {
				// assume LITTLE_ENDIAN platform
				int32_value = (*(uint8_t*)(ptr)) + (*(uint8_t*)(ptr+1)<<8) + (*(uint8_t*)(ptr+2)<<16); 
				sample_value = (biosig_data_type)int32_value; 
			}	
			else {
				B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
				B4C_ERRMSG = "Error SREAD: datatype not supported";
				exit(-1);
			}

		} else {

			// get source address 	
			ptr = hdr->AS.rawdata + (k4+toffset)*hdr->AS.bpb + hdr->AS.bi[k1] + k5*SZ;
			
			// mapping of raw data type to (biosig_data_type)
			if (0); 
			else if (GDFTYP==3) 
				sample_value = (biosig_data_type)(*(int16_t*)ptr); 
			else if (GDFTYP==4)
				sample_value = (biosig_data_type)(*(uint16_t*)ptr); 
			else if (GDFTYP==16) 
				sample_value = (biosig_data_type)(*(float*)(ptr));
			else if (GDFTYP==17) 
				sample_value = (biosig_data_type)(*(double*)(ptr)); 
			else if (GDFTYP==0)
				sample_value = (biosig_data_type)(*(char*)ptr); 
			else if (GDFTYP==1)
				sample_value = (biosig_data_type)(*(int8_t*)ptr); 
			else if (GDFTYP==2)
				sample_value = (biosig_data_type)(*(uint8_t*)ptr); 
			else if (GDFTYP==5)
				sample_value = (biosig_data_type)(*(int32_t*)ptr); 
			else if (GDFTYP==6)
				sample_value = (biosig_data_type)(*(uint32_t*)ptr); 
			else if (GDFTYP==7)
				sample_value = (biosig_data_type)(*(int64_t*)ptr); 
			else if (GDFTYP==8)
				sample_value = (biosig_data_type)(*(uint64_t*)ptr); 
			else if (GDFTYP==255+24) {
				// assume LITTLE_ENDIAN platform
				int32_value = (*(uint8_t*)(ptr)) + (*(uint8_t*)(ptr+1)<<8) + (*(int8_t*)(ptr+2)*(1<<16)); 
				sample_value = (biosig_data_type)int32_value; 
			}	
			else if (GDFTYP==511+24) {
				// assume LITTLE_ENDIAN platform
				int32_value = (*(uint8_t*)(ptr)) + (*(uint8_t*)(ptr+1)<<8) + (*(uint8_t*)(ptr+2)<<16); 
				sample_value = (biosig_data_type)int32_value; 
			}	
			else {
				B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
				B4C_ERRMSG = "Error SREAD: datatype not supported";
				exit(-1);
			}
		}

		// overflow and saturation detection 
		if ((hdr->FLAG.OVERFLOWDETECTION) && ((sample_value <= CHptr->DigMin) || (sample_value >= CHptr->DigMax)))
			sample_value = NaN; 	// missing value 
		else if (!hdr->FLAG.UCAL)	// scaling 
			sample_value = sample_value * CHptr->Cal + CHptr->Off;

		// resampling 1->DIV samples
		for (k3=0; k3 < DIV; k3++) 
#ifdef ROW_BASED_CHANNELS
			hdr->data.block[k2 + (k4*hdr->SPR + k5*DIV + k3)*NS] = sample_value; // row-based channels 
#else
			hdr->data.block[k2*count*hdr->SPR + k4*hdr->SPR + k5*DIV + k3] = sample_value; // column-based channels 
#endif

//		if ((VERBOSE_LEVEL>7) && (k4==0) && (k5==0)) 
//		fprintf(stdout,":s(1)=%f, NS=%d,[%d,%d,%d,%d SZ=%i, bpb=%i] %e %e %e\n",*(double*)hdr->AS.rawdata,NS,k1,k2,k4,k5,SZ,hdr->AS.bpb,sample_value,(*(double*)(ptr)),(*(float*)(ptr)));
		}

	}	
	k2++;
	}}

#ifdef ROW_BASED_CHANNELS
	hdr->data.size[0] = k2;			// rows	
	hdr->data.size[1] = hdr->SPR*count;	// columns 
#else
	hdr->data.size[0] = hdr->SPR*count;	// rows	
	hdr->data.size[1] = k2;			// columns 
#endif

	/* read sparse samples */	
	if ((hdr->TYPE==GDF) && (hdr->VERSION > 1.9)) {
		double c = hdr->SPR / hdr->SampleRate * hdr->EVENT.SampleRate;
		size_t *ChanList = (size_t*)calloc(hdr->NS+1,sizeof(size_t));
		// Note: ChanList and EVENT.CHN start with index=1 (not 0)
		size_t ch = 1;
		for (k1=0; k1<hdr->NS; k1++) // list of selected channels 
			ChanList[k1+1]= (hdr->CHANNEL[k1].OnOff ? ch++ : 0);
		
		for (k1=0; k1<hdr->EVENT.N; k1++) 
		if (hdr->EVENT.TYP[k1] == 0x7fff) 	// select non-equidistant sampled value
		if (ChanList[hdr->EVENT.CHN[k1]] > 0)	// if channel is selected
		if ((hdr->EVENT.POS[k1] >= POS*c) && (hdr->EVENT.POS[k1] < hdr->FILE.POS*c)) {
			ptr = (uint8_t*)(hdr->EVENT.DUR + k1);

			k2 = ChanList[hdr->EVENT.CHN[k1]]-1;
			CHptr = hdr->CHANNEL+k2;
			DIV 	= ceil(hdr->SampleRate/hdr->EVENT.SampleRate); 
			GDFTYP 	= CHptr->GDFTYP;
			SZ  	= GDFTYP_BYTE[GDFTYP];
			int32_value = 0; 
			
			if (0); 
			else if (GDFTYP==3) 
				sample_value = (biosig_data_type)lei16p(ptr); 
			else if (GDFTYP==4)
				sample_value = (biosig_data_type)leu16p(ptr); 
			else if (GDFTYP==16) 
				sample_value = (biosig_data_type)lef32p(ptr);
/*			else if (GDFTYP==17) 
				sample_value = (biosig_data_type)lef64p(ptr); 
*/			else if (GDFTYP==0)
				sample_value = (biosig_data_type)(*(char*)ptr); 
			else if (GDFTYP==1)
				sample_value = (biosig_data_type)(*(int8_t*)ptr); 
			else if (GDFTYP==2)
				sample_value = (biosig_data_type)(*(uint8_t*)ptr); 
			else if (GDFTYP==5)
				sample_value = (biosig_data_type)lei32p(ptr); 
			else if (GDFTYP==6)
				sample_value = (biosig_data_type)leu32p(ptr); 
/*			else if (GDFTYP==7)
				sample_value = (biosig_data_type)(*(int64_t*)ptr); 
			else if (GDFTYP==8)
				sample_value = (biosig_data_type)(*(uint64_t*)ptr); 
*/			else if (GDFTYP==255+24) {
				// assume LITTLE_ENDIAN platform
				int32_value = (*(uint8_t*)(ptr)) + (*(uint8_t*)(ptr+1)<<8) + (*(int8_t*)(ptr+2)*(1<<16)); 
				sample_value = (biosig_data_type)int32_value; 
			}	
			else if (GDFTYP==511+24) {
				// assume LITTLE_ENDIAN platform
				int32_value = (*(uint8_t*)(ptr)) + (*(uint8_t*)(ptr+1)<<8) + (*(uint8_t*)(ptr+2)<<16); 
				sample_value = (biosig_data_type)int32_value; 
			}	
			else {
				B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
				B4C_ERRMSG = "Error SREAD: datatype not supported";
				exit(-1);
			}

			// overflow and saturation detection 
			if ((hdr->FLAG.OVERFLOWDETECTION) && ((sample_value<=CHptr->DigMin) || (sample_value>=CHptr->DigMax)))
				sample_value = NaN; 	// missing value 
			else if (!hdr->FLAG.UCAL)	// scaling 
				sample_value = sample_value * CHptr->Cal + CHptr->Off;
			// resampling 1->DIV samples
			k5  = (hdr->EVENT.POS[k1]/c - POS)*hdr->SPR;
			for (k3=0; k3 < DIV; k3++) 
#ifdef ROW_BASED_CHANNELS
				hdr->data.block[k2 + (k5 + k3)*NS] = sample_value; 
#else
				hdr->data.block[k2 * count * hdr->SPR + k5 + k3] = sample_value; 
#endif 

		if (VERBOSE_LEVEL>7) 
			fprintf(stdout,"E%02i: s(1)= %d %e %e %e\n",k1,leu32p(ptr),sample_value,(*(double*)(ptr)),(*(float*)(ptr)));

		}
		free(ChanList);
	}

	return(count);

}  // end of SREAD 


/****************************************************************************/
/**	SREAD : sample-based
	this is still experiemental                                        **/
/****************************************************************************/
size_t sread2(biosig_data_type* data, size_t start, size_t length, HDRTYPE* hdr) {

	size_t count,k1,NS;

	// count number of selected channels 
	for (k1=0,NS=0; k1<hdr->NS; ++k1)
		if (hdr->CHANNEL[k1].OnOff) ++NS; 

	size_t s = floor(start/hdr->SPR);
	size_t e = ceil ((start+length)/hdr->SPR);
	count    = sread2(data, s, s-e, hdr);
	/* in order to avoid memory leaks need fixing */
	data     = data + NS*sizeof(biosig_data_type)*(start-s*hdr->SPR);
	return(count);
}


/****************************************************************************/
/**                     SWRITE                                             **/
/****************************************************************************/
size_t swrite(const biosig_data_type *data, size_t nelem, HDRTYPE* hdr) {
/* 
 *	writes NELEM blocks with HDR.AS.bpb BYTES each, 
 */
	void*			ptr;
	size_t			count,k1,k2,k3,k4,k5,DIV,SZ; 
	int 			GDFTYP;
	CHANNEL_TYPE*		CHptr;
	biosig_data_type 	sample_value, iCal, iOff; 
	union {	
		int8_t i8;
		uint8_t u8;
		int16_t i16;
		uint16_t u16;
		int32_t i32;
		uint32_t u32;
		int64_t i64;
		uint64_t u64;
	} val;

	// write data 

#define MAX_INT8   ((int8_t)0x7f)
#define MIN_INT8   ((int8_t)0x80)
#define MAX_UINT8  ((uint8_t)0xff)
#define MIN_UINT8  ((uint8_t)0)
#define MAX_INT16  ((int16_t)0x7fff)
#define MIN_INT16  ((int16_t)0x8000)
#define MAX_UINT16 ((uint16_t)0xffff)
#define MIN_UINT16 ((uint16_t)0)
#define MAX_INT24  ((int32_t)0x007fffff)
#define MIN_INT24  ((int32_t)0xff800000)
#define MAX_UINT24 ((uint32_t)0x00ffffff)
#define MIN_UINT24 ((uint32_t)0)
#define MAX_INT32  ((int32_t)0x7fffffff)
#define MIN_INT32  ((int32_t)0x80000000)
#define MAX_UINT32 ((uint32_t)0xffffffff)
#define MIN_UINT32 ((uint32_t)0)
#define MAX_INT64  ((uint64_t)(ldexp(1.0,63)-1.0))
#define MIN_INT64  ((int64_t)(-ldexp(1.0,63)))
#define MAX_UINT64 ((uint64_t)0xffffffffffffffff)
#define MIN_UINT64 ((uint64_t)0)

	size_t sz = hdr->AS.bpb*hdr->NRec;
	if ((sz>0) && (hdr->TYPE != SCP_ECG)) { 
	// memory allocation for SCP is done in SOPEN_SCP_WRITE Section 6	
		ptr = realloc(hdr->AS.rawdata, sz);
		if (ptr==NULL) {
			B4C_ERRNUM = B4C_INSUFFICIENT_MEMORY;
			B4C_ERRMSG = "SWRITE: memory allocation failed.";
			exit(-1);
		}	
		else 	 
			hdr->AS.rawdata = (uint8_t*)ptr; 
	}

	for (k1=0, k2=0; k1<hdr->NS; k1++) {
	CHptr 	= hdr->CHANNEL+k1;
	
	if (CHptr->SPR > 0) { /////CHptr->OnOff != 0) {
		DIV 	= hdr->SPR/CHptr->SPR; 
		GDFTYP 	= CHptr->GDFTYP;
		SZ  	= GDFTYP_BYTE[GDFTYP];
		iCal	= 1/CHptr->Cal;
		//iOff	= CHptr->DigMin - CHptr->PhysMin*iCal;
		iOff	= -CHptr->Off*iCal;

		for (k4 = 0; k4 < hdr->NRec; k4++) {
		for (k5 = 0; k5 < CHptr->SPR; k5++) {
    			for (k3=0, sample_value=0; k3 < DIV; k3++) 
				sample_value += data[k1*nelem*hdr->SPR + k4*CHptr->SPR + k5 + k3]; 

			sample_value /= DIV;

			if (!hdr->FLAG.UCAL)	// scaling 
				sample_value = sample_value * iCal + iOff;

			// get target address 	
			ptr = hdr->AS.rawdata + k4*hdr->AS.bpb + hdr->AS.bi[k1] + k5*SZ;

			// mapping of raw data type to (biosig_data_type)
			if (0); 

			else if (GDFTYP==3) {
				if      (sample_value > MAX_INT16) val.i16 = MAX_INT16;
				else if (sample_value < MIN_INT16) val.i16 = MIN_INT16;
				else     val.i16 = (int16_t) sample_value;
				if (!hdr->FLAG.SWAP)
					*(int16_t*)ptr = val.i16; 
				else	
					*(int16_t*)ptr = bswap_16(val.i16); 
			}

			else if (GDFTYP==4) {
				if      (sample_value > MAX_UINT16) val.u16 = MAX_UINT16;
				else if (sample_value < MIN_UINT16) val.u16 = MIN_UINT16;
				else     val.u16 = (uint16_t) sample_value;
				*(uint16_t*)ptr = l_endian_u16(val.u16); 
			}

			else if (GDFTYP==16) {
				*(float*)ptr  = l_endian_f32((float)sample_value);
			}
			else if (GDFTYP==17) 
				*(double*)ptr = l_endian_f64((double)sample_value);

			else if (GDFTYP==0) {
				if      (sample_value > MAX_INT8) val.i8 = MAX_INT8;
				else if (sample_value < MIN_INT8) val.i8 = MIN_INT8;
				else     val.i8 = (int8_t) sample_value;
				*(int8_t*)ptr = val.i8; 
			}
			else if (GDFTYP==1) {
				if      (sample_value > MAX_INT8) val.i8 = MAX_INT8;
				else if (sample_value < MIN_INT8) val.i8 = MIN_INT8;
				else     val.i8 = (int8_t) sample_value;
				*(int8_t*)ptr = val.i8; 
			}
			else if (GDFTYP==2) {
				if      (sample_value > MAX_UINT8) val.u8 = MAX_UINT8;
				else if (sample_value < MIN_UINT8) val.u8 = MIN_UINT8;
				else     val.u8 = (uint8_t) sample_value;
				*(uint8_t*)ptr = val.u8; 
			}
			else if (GDFTYP==5) {
				if      (sample_value > ldexp(1.0,31)-1) val.i32 = MAX_INT32;
				else if (sample_value < ldexp(-1.0,31)) val.i32 = MIN_INT32;
				else     val.i32 = (int32_t) sample_value;
				if (!hdr->FLAG.SWAP)
					*(int32_t*)ptr = val.i32; 
				else	
					*(int32_t*)ptr = bswap_32(val.i32); 
			}
			else if (GDFTYP==6) {
				if      (sample_value > ldexp(1.0,32)-1.0) val.u32 = MAX_UINT32;
				else if (sample_value < 0.0) val.u32 = MIN_UINT32;
				else     val.u32 = (uint32_t) sample_value;
				*(uint32_t*)ptr = l_endian_u32(val.u32); 
			}

			else if (GDFTYP==7) {
				if      (sample_value > ldexp(1.0,63)-1.0) 
					val.i64 = MAX_INT64;
				else if (sample_value < -ldexp(1.0,63)) 
					val.i64 = MIN_INT64;
				else     
					val.i64 = (int64_t) sample_value;
				*(int64_t*)ptr = l_endian_i64(val.i64); 
			}

			else if (GDFTYP==8) {
				if      (sample_value > ldexp(1.0,64)-1.0) val.u64 = (uint64_t)(-1);
				else if (sample_value < 0.0) val.u64 = 0;
				else     val.u64 = (uint64_t) sample_value;
				*(uint64_t*)ptr = l_endian_u64(val.u64); 
			}

			else if (GDFTYP==255+24) {
				if      (sample_value > MAX_INT24) val.i32 = MAX_INT24;
				else if (sample_value < MIN_INT24) val.i32 = MIN_INT24;
				else     val.i32 = (int32_t) sample_value;
				*(uint8_t*)ptr = (uint8_t)(val.i32 & 0x000000ff); 
				*((uint8_t*)ptr+1) = (uint8_t)((val.i32>>8) & 0x000000ff); 
				*((uint8_t*)ptr+2) = (uint8_t)((val.i32>>16) & 0x000000ff); 
			}	

			else if (GDFTYP==511+24) {
				if      (sample_value > MAX_UINT24) val.i32 = MAX_UINT24;
				else if (sample_value < MIN_UINT24) val.i32 = MIN_UINT24;
				else     val.i32 = (int32_t) sample_value;
				*(uint8_t*)ptr     =  val.i32 & 0x000000ff; 
				*((uint8_t*)ptr+1) = (uint8_t)((val.i32>>8) & 0x000000ff); 
				*((uint8_t*)ptr+2) = (uint8_t)((val.i32>>16) & 0x000000ff); 
			}

			else {
				B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
				B4C_ERRMSG = "SWRITE: datatype not supported";
				exit(-1);
			}
		}
		}
	}
	}	

	if (hdr->TYPE != SCP_ECG  && hdr->TYPE != HL7aECG) {
		// for SCP: writing to file is done in SCLOSE	
		count = ifwrite((uint8_t*)(hdr->AS.rawdata), hdr->AS.bpb, hdr->NRec, hdr);
	}	
	else { 
		count = 1; 	
	}
	
	// set position of file handle 
	(hdr->FILE.POS) += count; 

	return(count);

}  // end of SWRITE 


/****************************************************************************/
/**                     SEOF                                               **/
/****************************************************************************/
int seof(HDRTYPE* hdr)
{
	return(hdr->FILE.POS >= hdr->NRec);
}


/****************************************************************************/
/**                     SREWIND                                            **/
/****************************************************************************/
void srewind(HDRTYPE* hdr)
{
	sseek(hdr,0,SEEK_SET);
	return;
}


/****************************************************************************/
/**                     SSEEK                                              **/
/****************************************************************************/
int sseek(HDRTYPE* hdr, long int offset, int whence)
{
	int64_t pos=0; 
	
	if    	(whence < 0) 
		pos = offset * hdr->AS.bpb; 
	else if (whence == 0) 
		pos = (hdr->FILE.POS + offset) * hdr->AS.bpb;
	else if (whence > 0) 
		pos = (hdr->NRec + offset) * hdr->AS.bpb;
	
	if ((pos < 0) | (pos > hdr->NRec * hdr->AS.bpb))
		return(-1);
	else if (ifseek(hdr, pos + hdr->HeadLen, SEEK_SET))
		return(-1);

	hdr->FILE.POS = pos / (hdr->AS.bpb); 	
	return(0);
	
}  // end of SSEEK


/****************************************************************************/
/**                     STELL                                              **/
/****************************************************************************/
long int stell(HDRTYPE* hdr)
{
	long int pos = iftell(hdr);	

	if (pos<0)
		return(-1);
	else if (pos != (hdr->FILE.POS * hdr->AS.bpb + hdr->HeadLen))
		return(-1);
	else 
		return(hdr->FILE.POS);
	
}  // end of STELL


/****************************************************************************/
/**                     SCLOSE                                             **/
/****************************************************************************/
int sclose(HDRTYPE* hdr)
{
	int32_t 	pos, len; 
	char tmp[88]; 
	
	if ((hdr->FILE.OPEN>1) & ((hdr->TYPE==GDF) | (hdr->TYPE==EDF) | (hdr->TYPE==BDF)))
	{
		// WRITE HDR.NRec 
		pos = (iftell(hdr)-hdr->HeadLen); 
		if (hdr->NRec<0)
		{	if (pos>0) 	hdr->NRec = pos/hdr->AS.bpb;
			else		hdr->NRec = 0; 	
			if (hdr->TYPE==GDF) {
				*(uint64_t*)tmp = l_endian_u64(hdr->NRec);
				len = sizeof(hdr->NRec);
			}
			else {
				len = sprintf(tmp,"%Lu",hdr->NRec);
				if (len>8) fprintf(stderr,"Warning: NRec is (%s) to long.\n",tmp);  
			}
			/* ### FIXME : gzseek supports only forward seek */
			if (hdr->FILE.COMPRESSION>0)
				fprintf(stderr,"Warning: writing NRec in gz-file requires gzseek which may not be supported.\n");
			ifseek(hdr,236,SEEK_SET); 
			ifwrite(tmp,len,1,hdr);
		}
	
		if ((hdr->TYPE==GDF) & (hdr->EVENT.N>0))
			write_gdf_eventtable(hdr);

	}		
	else if ((hdr->FILE.OPEN>1) & (hdr->TYPE==SCP_ECG))
	{
		uint16_t 	crc; 
		uint8_t*	ptr; 	// pointer to memory mapping of the file layout

		aECG_TYPE* aECG = (aECG_TYPE*)hdr->aECG;
		if (aECG->Section5.Length>0) {
			// compute CRC for Section 5
			uint16_t crc = CRCEvaluate(hdr->AS.Header + aECG->Section5.StartPtr+2,aECG->Section5.Length-2); // compute CRC
			*(uint16_t*)(hdr->AS.Header + aECG->Section5.StartPtr) = l_endian_u16(crc);
		}	
		if (aECG->Section6.Length>0) {
			// compute CRC for Section 6
			uint16_t crc = CRCEvaluate(hdr->AS.Header + aECG->Section6.StartPtr+2,aECG->Section6.Length-2); // compute CRC
			*(uint16_t*)(hdr->AS.Header + aECG->Section6.StartPtr) = l_endian_u16(crc);
		}	
		// compute crc and len and write to preamble 
		ptr = hdr->AS.Header; 
		*(uint32_t*)(ptr+2) = l_endian_u32(hdr->HeadLen); 
		crc = CRCEvaluate(ptr+2,hdr->HeadLen-2); 
		*(int16_t*)ptr      = l_endian_u16(crc);
		ifwrite(hdr->AS.Header, sizeof(char), hdr->HeadLen, hdr);
	}		
	else if ((hdr->FILE.OPEN>1) && (hdr->TYPE==HL7aECG))
	{
		sclose_HL7aECG_write(hdr);
		hdr->FILE.OPEN = 0; 
	}

	if (hdr->FILE.OPEN > 0) {
		int status = ifclose(hdr);
		if (status) iferror(hdr);
    	}	

    	return(0);
}


/****************************************************************************/
/**                     SERROR                                             **/
/****************************************************************************/
int serror() {
	int status = B4C_ERRNUM; 
	if (status) {
		fprintf(stderr,"ERROR %i: %s\n",B4C_ERRNUM,B4C_ERRMSG);
		B4C_ERRNUM = B4C_NO_ERROR;
	}
	return(status);
}



/****************************************************************************/
/*        Write / Update Event Table in GDF file                            */
/*                                                                          */
/* returns 0 in case of success                                             */
/* returns -1 in case of failure                                            */
/****************************************************************************/
int sflush_gdf_event_table(HDRTYPE* hdr)
{
	if ((hdr->TYPE!=GDF) || hdr->FILE.COMPRESSION)
		return(-1);
	
	long int filepos = iftell(hdr); 
	ifclose(hdr);
	hdr = ifopen(hdr,"rb+");
	if (!hdr->FILE.OPEN) {
		/* file cannot be opened in write mode */ 
		hdr = ifopen(hdr,"rb");
		return(-1);
	}

	write_gdf_eventtable(hdr);  

	ifseek(hdr,filepos,SEEK_SET); 
		
	return(0);		
}



/****************************************************************************/
/**                     HDR2ASCII                                          **/
/**	displaying header information                                      **/ 
/****************************************************************************/
int hdr2ascii(HDRTYPE* hdr, FILE *fid, int VERBOSE_LEVEL)
{
	CHANNEL_TYPE* 	cp; 
	time_t  	T0;

	if (VERBOSE_LEVEL==7) {
		T0 = gdf_time2t_time(hdr->T0);
		char tmp[60];
		strftime(tmp, 59, "%x %X %Z", gmtime(&T0));
		fprintf(fid,"\tStartOfRecording: %s\n",tmp); 
		return(0);
	}
		
	if (VERBOSE_LEVEL>1) {
		/* display header information */
		fprintf(fid,"FileName:\t%s\nType    :\t%s\nVersion :\t%4.2f\nHeadLen :\t%i\n",hdr->FileName,GetFileTypeString(hdr->TYPE),hdr->VERSION,hdr->HeadLen);
//		fprintf(fid,"NoChannels:\t%i\nSPR:\t\t%i\nNRec:\t\t%Li\nDuration[s]:\t%u/%u\nFs:\t\t%f\n",hdr->NS,hdr->SPR,hdr->NRec,hdr->Dur[0],hdr->Dur[1],hdr->SampleRate);
		fprintf(fid,"NoChannels:\t%i\nSPR:\t\t%i\nNRec:\t\t%Li\nFs:\t\t%f\n",hdr->NS,hdr->SPR,hdr->NRec,hdr->SampleRate);
		fprintf(fid,"Events/Annotations:\t%i\nEvents/SampleRate:\t%f\n",hdr->EVENT.N,hdr->EVENT.SampleRate); 
	}
		
	if (VERBOSE_LEVEL>0) {
		/* demographic information */
		fprintf(fid,"\n[FIXED HEADER]\n");
//		fprintf(fid,"\nPID:\t|%s|\nPatient:\n",hdr->AS.PID);
		fprintf(fid,"Recording:\n\tID              : %s\n",hdr->ID.Recording);
		fprintf(fid,"Manufacturer:\n\tName            : %s\n",hdr->ID.Manufacturer.Name);
		fprintf(fid,"\tModel           : %s\n",hdr->ID.Manufacturer.Model);
		fprintf(fid,"\tVersion         : %s\n",hdr->ID.Manufacturer.Version);
		fprintf(fid,"\tSerialNumber    : %s\n",hdr->ID.Manufacturer.SerialNumber);
		fprintf(fid,"Patient:\n\tID              : %s\n",hdr->Patient.Id); 
		if (hdr->Patient.Name!=NULL)
			fprintf(fid,"\tName            : %s\n",hdr->Patient.Name); 
		float age = (hdr->T0 - hdr->Patient.Birthday)/ldexp(365.25,32); 
		if (hdr->Patient.Height)
			fprintf(fid,"\tHeight          : %i cm\n",hdr->Patient.Height); 
		if (hdr->Patient.Height)
			fprintf(stdout,"\tWeight          : %i kg\n",hdr->Patient.Weight); 
			
		fprintf(fid,"\tGender          : "); 
		if (hdr->Patient.Sex==1)
			fprintf(fid,"male\n"); 
		else if (hdr->Patient.Sex==2)
			fprintf(fid,"female\n"); 
		else 
			fprintf(fid,"unknown\n"); 
		if (hdr->Patient.Birthday) {
			T0 = gdf_time2t_time(hdr->Patient.Birthday);
			fprintf(fid,"\tAge             : %4.1f years\n\tBirthday        : (%.6f) %s ",age,ldexp(hdr->Patient.Birthday,-32),asctime(gmtime(&T0)));
		}
		else
			fprintf(fid,"\tAge             : ----\n\tBirthday        : unknown\n");
			 
		T0 = gdf_time2t_time(hdr->T0);
		char tmp[60];
		strftime(tmp, 59, "%x %X %Z", localtime(&T0));
		fprintf(fid,"\tStartOfRecording: (%.6f) %s\n",ldexp(hdr->T0,-32),tmp); 
	}
		
	if (VERBOSE_LEVEL>2) {
		/* channel settings */ 
		fprintf(fid,"\n[CHANNEL HEADER]");
		fprintf(fid,"\n#No  LeadId Label\tFs[Hz]\tGDFTYP\tCal\tOff\tPhysDim PhysMax  PhysMin DigMax DigMin Filter");
		size_t k;
		for (k=0; k<hdr->NS; k++) {
			cp = hdr->CHANNEL+k; 
			fprintf(fid,"\n#%2i: %3i %i %7s\t%5.1f %2i  %5f %5f %s\t%5f\t%5f\t%5f\t%5f\t%5f\t%5f",
				k+1,cp->LeadIdCode,cp->OnOff,cp->Label,cp->SPR * hdr->SampleRate/hdr->SPR,
				cp->GDFTYP, cp->Cal, cp->Off, cp->PhysDim, 
				cp->PhysMax, cp->PhysMin, cp->DigMax, cp->DigMin,cp->HighPass,cp->LowPass);
			//fprintf(fid,"\t %3i", cp->SPR);
		}
	}
		
	if (VERBOSE_LEVEL>3) {
		/* channel settings */ 
		fprintf(fid,"\n\n[EVENT TABLE] N=%i Fs=%f",hdr->EVENT.N,hdr->EVENT.SampleRate);
		fprintf(fid,"\n#No\tTYP\tPOS\tDUR\tCHN\tVAL");
		size_t k;
		for (k=0; k<hdr->EVENT.N; k++) {
			fprintf(fid,"\n%5i\t0x%04x\t%d",k+1,hdr->EVENT.TYP[k],hdr->EVENT.POS[k]);
			if (hdr->EVENT.DUR != NULL)		
				fprintf(fid,"\t%5d\t%d",hdr->EVENT.DUR[k],hdr->EVENT.CHN[k]);
			if ((hdr->EVENT.TYP[k] == 0x7fff) && (hdr->TYPE==GDF))
				fprintf(fid,"\t[neds]");
		}
	}
		
	if (VERBOSE_LEVEL>4) {
		if (hdr->aECG) {
			aECG_TYPE* aECG = (aECG_TYPE*)hdr->aECG;
			fprintf(stdout,"\nInstitution Number: %i\n",aECG->Section1.Tag14.INST_NUMBER);
			fprintf(stdout,"DepartmentNumber : %i\n",aECG->Section1.Tag14.DEPT_NUMBER);
			fprintf(stdout,"Device Id        : %i\n",aECG->Section1.Tag14.DEVICE_ID);
			fprintf(stdout,"Device Type      : %i\n",aECG->Section1.Tag14.DEVICE_TYPE);
			fprintf(stdout,"Manufacture code : %i\n",aECG->Section1.Tag14.MANUF_CODE);
			fprintf(stdout,"MOD_DESC         : %s\n",aECG->Section1.Tag14.MOD_DESC); 
			fprintf(stdout,"Version          : %i\n",aECG->Section1.Tag14.VERSION);
			fprintf(stdout,"ProtCompLevel    : %i\n",aECG->Section1.Tag14.PROT_COMP_LEVEL);
			fprintf(stdout,"LangSuppCode     : %i\n",aECG->Section1.Tag14.LANG_SUPP_CODE);
			fprintf(stdout,"ECG_CAP_DEV      : %i\n",aECG->Section1.Tag14.ECG_CAP_DEV);
			fprintf(stdout,"Mains Frequency  : %i\n",aECG->Section1.Tag14.MAINS_FREQ);
/*
			fprintf(stdout,"ANAL_PROG_REV_NUM    : %s\n",aECG->Section1.Tag14.ANAL_PROG_REV_NUM);
			fprintf(stdout,"SERIAL_NUMBER_ACQ_DEV: %s\n",aECG->Section1.Tag14.SERIAL_NUMBER_ACQ_DEV);
			fprintf(stdout,"ACQ_DEV_SYS_SW_ID    : %i\n",aECG->Section1.Tag14.ACQ_DEV_SYS_SW_ID);
			fprintf(stdout,"ACQ_DEV_SCP_SW       : %i\n",aECG->Section1.Tag14.ACQ_DEV_SCP_SW);
			fprintf(stdout,"ACQ_DEV_MANUF        : %i\n",aECG->Section1.Tag14.ACQ_DEV_MANUF);
*/
			fprintf(stdout,"Compression  HUFFMAN : %i\n",aECG->FLAG.HUFFMAN);
			fprintf(stdout,"Compression  REF-BEAT: %i\n",aECG->FLAG.REF_BEAT);		
			fprintf(stdout,"Compression  BIMODAL : %i\n",aECG->FLAG.BIMODAL);		
			fprintf(stdout,"Compression  DIFF    : %i",aECG->FLAG.DIFF);		
		}
	}
	fprintf(fid,"\n\n");
	return(0);
} 	/* end of HDR2ASCII */


/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/

