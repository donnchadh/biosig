/*

    $Id: biosig.c,v 1.83 2007-08-04 20:11:30 schloegl Exp $
    Copyright (C) 2005,2006,2007 Alois Schloegl <a.schloegl@ieee.org>
		    
    This function is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 


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

	reading and writing of GDF files is demonstrated 
	
	Features: 
	- reading and writing of EDF, BDF and GDF2.0pre files 
	- reading of GDF1.x, BKR, CFWB, CNT files 
	
	implemented functions: 
	- SOPEN, SREAD, SWRITE, SCLOSE, SEOF, SSEEK, STELL, SREWIND 
	
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <libxml/xmlreader.h>

#include "biosig.h"

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
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2,    /* 256 */  
	0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4,    /* 255+24 = bit24, 3 byte */ 
	0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 6, 
	0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 8, 
	0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0,10, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 384 */ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2,    /* 512 */ 
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
	, NULL };  // stop marker 



/****************************************************************************/
/**                                                                        **/
/**                      INTERNAL FUNCTIONS                                **/
/**                                                                        **/
/****************************************************************************/
 
// greatest common divisor 
size_t gcd(size_t A,size_t B) 
{	size_t t; 
	if (A<B) {t=B; B=A; A=t;}; 
	while (B) {
		t = B; 
		B = A%B;
		A = t; 
	}
	return(A);
};

// least common multiple
size_t lcm(size_t A,size_t B) 
{
	return(A*B/gcd(A,B));
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


/* physical units are defined in 
 prEN ISO 11073-10101 (Nov 2003)
 Health Informatics - Point-of-care medical device communications - Part 10101:Nomenclature
 (ISO/DIS 11073-10101:2003)
 Table A.6.1: Table of Decimal Factors

 CEN/TC251/PT40 2001	
 File Exchange Format for Vital Signs - Annex A 
 Table A.4.1: Table of Decimal Factors	const double scale[32] =
*/

char* PhysDimTable[2048];   
const struct PhysDimIdx 
	{
		uint16_t idx;
		char*	PhysDimDesc;
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
	{ 6016 ,  "dyne s m-2 cm-5" },
	{ 65440 ,  "dyne s m2 cm-5" },
	{ 65472 ,  "l m-2" },
	{ 65504 ,  "T" },
	{ 0xffff ,  "end-of-table" },
};

int InitPhysDimTable()
{
	/* 
		Initializes PhysDimTable (must not be modified by any other instance)	
	*/	

	int k; 
	for (k=0; k<2048; PhysDimTable[k++]="\0");
	for (k=0; _physdim[k].idx<0xffff; k++) {
		PhysDimTable[_physdim[k].idx>>5] = _physdim[k].PhysDimDesc;
	}	
	return(1);  
}
		
const char* PhysDimFactor[] = {
	"","da","h","k","M","G","T","P","E","Z","Y","#","#","#","#","#",
	"d","c","m","u","n","p","f","a","z","y","#","#","#","#","#","#"};

double PhysDimScale(uint16_t PhysDimCode)
{	
// converting PhysDimCode -> scaling factor

	const double scale[32] =
	{ 1e0,  1e1,  1e2,  1e3,  1e6,  1e9,   1e12,  1e15,
	  1e18, 1e21, 1e24, NaN,  NaN,  NaN,   NaN,   NaN, 
	  1e-1, 1e-2, 1e-3, 1e-6, 1e-9, 1e-12, 1e-15, 1e-18, 
	  1e-21,1e-24,NaN,  NaN,  NaN,  NaN,   NaN,   NaN }; 

	return (scale[PhysDimCode & 0x001f]); 
}

char* PhysDim(uint16_t PhysDimCode, char *PhysDim)
{	
	// converting PhysDimCode -> PhysDim

	static int INIT_FLAG = 0; 
	if (!INIT_FLAG) INIT_FLAG = InitPhysDimTable();	/* Init PhysDimTable once*/	

	strcpy(PhysDim,PhysDimFactor[PhysDimCode & 0x1f]);
	strcat(PhysDim,PhysDimTable[PhysDimCode>>5]);
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

	// greedy search - check all codes 0..65535
	for (k1=0; k1<32; k1++)
	if (PhysDimScale(k1)>0.0)  // exclude NaN
	for (k2=0; _physdim[k2].idx < 0xffff; k2++) {
		strcpy(s, PhysDimFactor[k1]);
		strcat(s, _physdim[k2].PhysDimDesc);
		if (!strncmp(PhysDim0, s, strlen(PhysDim0))) 
			return(_physdim[k2].idx+k1); 
	}
	return(0);
}


/*
	Interface for mixed use of ZLIB and STDIO 
	If ZLIB is not available, STDIO is used. 
	If ZLIB is availabe, HDR.FILE.COMPRESSION tells
	whether STDIO or ZLIB is used. 
 */
int errnum;
HDRTYPE* FOPEN(HDRTYPE* hdr, char* mode) {
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

int FCLOSE(HDRTYPE* hdr) {
	hdr->FILE.OPEN = 0; 
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
		return(gzclose(hdr->FILE.gzFID));  
	else	
#endif
	return(fclose(hdr->FILE.FID));
}

size_t FREAD(void* ptr, size_t size, size_t nmemb, HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION>0)
		return(gzread(hdr->FILE.gzFID, ptr, size * nmemb)/size);
	else	
#endif
	return(fread(ptr, size, nmemb, hdr->FILE.FID));
}

size_t FWRITE(void* ptr, size_t size, size_t nmemb, HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
	return(gzwrite(hdr->FILE.gzFID, ptr, size*nmemb)/size);
	else	
#endif
	return(fwrite(ptr, size, nmemb, hdr->FILE.FID));
}

int FPRINTF(HDRTYPE* hdr, const char *format, va_list arg) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
	return(gzprintf(hdr->FILE.gzFID,format, arg));
	else	
#endif
	return(fprintf(hdr->FILE.FID,format, arg));
}


int FPUTC(int c, HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
	return(gzputc(hdr->FILE.FID,c));
	else	
#endif
	return(fputc(c,hdr->FILE.FID));
}

int FGETC(HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
	return(gzgetc(hdr->FILE.gzFID));
	else	
#endif
	return(fgetc(hdr->FILE.FID));
}

char* FGETS(char *str, int n, HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
	return(gzgets(hdr->FILE.gzFID, str, n));
	else	
#endif
	return(fgets(str,n,hdr->FILE.FID));
}

int FSEEK(HDRTYPE* hdr, long offset, int whence) {
#ifdef ZLIB_H
	if (whence==SEEK_END)
		fprintf(stdout,"### Warning SEEK_END is not supported but used in gzseek/FSEEK\n");

	if (hdr->FILE.COMPRESSION)
	return(gzseek(hdr->FILE.gzFID,offset,whence));
	else	
#endif
	return(fseek(hdr->FILE.FID,offset,whence));
}

long FTELL(HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
	return(gztell(hdr->FILE.gzFID));
	else	
#endif
	return(ftell(hdr->FILE.FID));
}

int FGETPOS(HDRTYPE* hdr, fpos_t *pos) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION) {
		z_off_t p = gztell(hdr->FILE.gzFID);
		if (p<0) return(-1); 
		else {
			pos->__pos = p;	// ugly hack but working
			return(0);
		}	
	} else	
#endif
	return(fgetpos(hdr->FILE.FID, pos));
}

int FEOF(HDRTYPE* hdr) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION)
	return(gzeof(hdr->FILE.gzFID));
	else	
#endif
	return(feof(hdr->FILE.FID));
}

int FERROR(HDRTYPE* hdr) {
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



/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/


/****************************************************************************/
/**                     INIT HDR                                           **/
/****************************************************************************/
HDRTYPE* create_default_hdr(const unsigned NS, const unsigned N_EVENT)
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
		fprintf(stderr,"Panic: mixed results on Endianity test.\n"); 
		exit(-1); 
	}		
	if  ((!hdr->FILE.LittleEndian) && (__BYTE_ORDER == __LITTLE_ENDIAN))	{
		fprintf(stderr,"Panic: mixed results on Endianity test.\n"); 
		exit(-1); 
	}	

	hdr->FILE.OPEN = 0;
	hdr->FILE.FID = 0;

	hdr->AS.Header1 = NULL;

      	hdr->TYPE = GDF; 
      	hdr->VERSION = 1.94;
      	hdr->AS.rawdata = (uint8_t*) malloc(0);
      	hdr->NRec = 0; 
      	hdr->NS = NS;	
	hdr->SampleRate = 4321.5;
      	memset(hdr->AS.PID,32,81); 
      	hdr->AS.RID = "GRAZ"; 
	hdr->AS.bi = (uint32_t*)calloc(hdr->NS+1,sizeof(uint32_t));
	hdr->data.size[0] = 0; 	// rows 
	hdr->data.size[1] = 0;  // columns 
	hdr->data.block = (biosig_data_type*)malloc(0); 
      	hdr->T0 = t_time2gdf_time(time(NULL));
      	hdr->ID.Equipment = *(uint64_t*)&"b4c_0.45";

	hdr->Patient.Name 	= "X";
	hdr->Patient.Id 	= "\0\0";
	hdr->Patient.Birthday 	= (gdf_time)0;        // Unknown;
      	hdr->Patient.Medication = 0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.DrugAbuse 	= 0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.AlcoholAbuse= 0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.Smoking 	= 0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.Sex 	= 0;	// 0:Unknown, 1: Male, 2: Female
      	hdr->Patient.Handedness = 0;	// 0:Unknown, 1: Right, 2: Left, 3: Equal
      	hdr->Patient.Impairment.Visual = 0;	// 0:Unknown, 1: NO, 2: YES, 3: Corrected
      	hdr->Patient.Weight 	= 0;	// 0:Unknown
      	hdr->Patient.Height 	= 0;	// 0:Unknown
      	hdr->Dur[0] = 1;
      	hdr->Dur[1] = 1;
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
	hdr->FLAG.OVERFLOWDETECTION = 1; 		// overflow detection ON
	
       	// define variable header 
	hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	for (k=0;k<hdr->NS;k++)	{
	      	hdr->CHANNEL[k].Label     = "";
	      	hdr->CHANNEL[k].LeadIdCode= 0;
	      	hdr->CHANNEL[k].Transducer= "EEG: Ag-AgCl electrodes";
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
	hdr->EVENT.SampleRate = hdr->SampleRate; 
	hdr->EVENT.POS = (uint32_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.POS));
	hdr->EVENT.TYP = (uint16_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.TYP));
	hdr->EVENT.DUR = (uint32_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.DUR));
	hdr->EVENT.CHN = (uint16_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.CHN));
	
	// initialize "Annotated ECG structure"
	hdr->aECG = NULL; 
	
	return(hdr);
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

	uint8_t *Header1 = hdr->AS.Header1;
	uint32_t U32 = l_endian_u32(*(uint32_t*)(Header1+2)); 

    	if (hdr->TYPE != unknown)
    		return(hdr); 

    	else if ((U32>=30) & (U32<=42)) {
    		hdr->VERSION = (float)U32; 
    		U32 = l_endian_u32(*(uint32_t*)(Header1+6));
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
	const uint8_t MAGIC_NUMBER_ZIP[]  = {80,75,3,4};
	const uint8_t MAGIC_NUMBER_TIFF_l32[] = {73,73,42,0};
	const uint8_t MAGIC_NUMBER_TIFF_b32[] = {77,77,0,42};
	const uint8_t MAGIC_NUMBER_TIFF_l64[] = {73,73,43,0,8,0,0,0};
	const uint8_t MAGIC_NUMBER_TIFF_b64[] = {77,77,0,43,0,8,0,0};
	const uint8_t MAGIC_NUMBER_DICOM[]    = {8,0,5,0,10,0,0,0,73,83,79,95,73,82,32,49,48,48};
	
    	if (hdr->TYPE != unknown)
       		return(hdr); 
    	else if (!memcmp(Header1+20,"ACR-NEMA",8))
	    	hdr->TYPE = ACR_NEMA;
    	else if (!memcmp(Header1+1,"BIOSEMI",7) & (Header1[0]==255)) {
    		hdr->TYPE = BDF;
    		hdr->VERSION = -1; 
    	}
    	else if ((Header1[0]==207) & (!Header1[1]) & (!Header1[154]) & (!Header1[155]))
	    	hdr->TYPE = BKR;
        else if (!memcmp(Header1,"Brain Vision Data Exchange Header File",38))
                hdr->TYPE = BrainVision;
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
    	else if (!memcmp(Header1,MAGIC_NUMBER_FEF1,8) | !memcmp(Header1,MAGIC_NUMBER_FEF2,8)) {
	    	hdr->TYPE = FEF;
		char tmp[9];
		strncpy(tmp,(char*)hdr->AS.Header1+8,8);
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
	else if (!memcmp(Header1,MAGIC_NUMBER_GZIP,3))  {
		hdr->TYPE = GZIP;
		hdr->FILE.COMPRESSION = 1; 
	}	
    	else if (!memcmp(Header1,"@  MFER ",8))
	    	hdr->TYPE = MFER;
    	else if (!memcmp(Header1,"@ MFR ",6))
	    	hdr->TYPE = MFER;
    	else if (!memcmp(Header1,"MThd\0\0\0\1\0",9))
	    	hdr->TYPE = MIDI;
    	else if (!memcmp(Header1,"NEX1",4))
	    	hdr->TYPE = NEX1;
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
    	else if (!memcmp(Header1+16,"SCPECG",6)) {
	    	hdr->TYPE = SCP_ECG;
    		if (!memcmp(Header1+8,"\0\0\136\0\0\0\13\13",8)) 
		    	hdr->VERSION = 1.3;
    		else if (!memcmp(Header1+8,"\0\0",2)) 
		    	hdr->VERSION = -1.0;
		else    	
		    	hdr->VERSION = -2.0;
	}    	
    	else if (!memcmp(Header1,"\"Snap-Master Data File\"",24))
	    	hdr->TYPE = SMA;
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
		strncpy(tmp,(char*)hdr->AS.Header1+23,3);
		hdr->VERSION = atof(tmp);
	}	
	else if (!memcmp(Header1,MAGIC_NUMBER_Z,3))
		hdr->TYPE = Z;
	else if (!memcmp(Header1,MAGIC_NUMBER_ZIP,sizeof(MAGIC_NUMBER_ZIP)))
		hdr->TYPE = ZIP;
	else if (!memcmp(Header1,"<?xml version",13))
		hdr->TYPE = HL7aECG;                    
    	
	return(hdr); 
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

    	int 		k,k1;
    	uint32_t	k32u; 
    	size_t	 	count,len;
    	uint8_t 	buf[81];
    	char 		tmp[81];
    	char 		cmd[256];
    	double 		Dur; 
	char* 		Header1;
	char* 		Header2;
	char*		ptr_str;
	struct tm 	tm_time; 
	time_t		tt;
	unsigned	EventChannel = 0;

	if (hdr==NULL)
		hdr = create_default_hdr(0,0);	// initializes fields that may stay undefined during SOPEN 

if (!strncmp(MODE,"r",1))	
{
 	Header1 = (char*)malloc(257);
    	Header1[256] = 0;
	hdr->AS.Header1 = (uint8_t*)Header1; 	
	hdr->TYPE = unknown; 

	hdr->FileName = FileName; 
	// check for gzip because on-the-fly decompression might not be implemented
        hdr->FILE.COMPRESSION = 1;   	
        hdr = FOPEN(hdr,"rb");
#ifdef ZLIB_H
    	if (hdr->FILE.gzFID == NULL) 
#else
    	if (hdr->FILE.FID == NULL) 
#endif
    	{ 	
    		fprintf(stderr,"Error SOPEN(READ); Cannot open file %s\n",FileName);
    		free(hdr);    		
		return(NULL);
    	}	    
    
    	/******** read 1st (fixed)  header  *******/	
    	count   = FREAD(Header1,1,256,hdr);
	hdr  = getfiletype(hdr);
    	
    	if (hdr->TYPE == unknown) {
		fprintf(stdout,"ERROR BIOSIG SOPEN(read): Format of file %s not supported\n",hdr->FileName);
    		FCLOSE(hdr);
    		free(hdr->AS.Header1);
    		free(hdr);
		return(NULL);
	}	

	if (hdr->TYPE == GDF) {
  	    	strncpy(tmp,(char*)Header1+3,5);
	    	hdr->VERSION 	= atof(tmp);
	    	hdr->NRec 	= l_endian_i64( *( int64_t*) (Header1+236) ); 
	    	hdr->Dur[0]  	= l_endian_u32( *(uint32_t*) (Header1+244) );
	    	hdr->Dur[1]  	= l_endian_u32( *(uint32_t*) (Header1+248) ); 
	    	hdr->NS   	= l_endian_u16( *(uint16_t*) (Header1+252) ); 
	    	
	    	if (hdr->VERSION > 1.90) { 
		    	hdr->HeadLen 	= l_endian_u16( *(uint16_t*) (Header1+184) )<<8; 
	    		strncpy(hdr->AS.PID,Header1+8,66);
	    		hdr->Patient.Id = strtok(hdr->AS.PID," ");
	    		hdr->Patient.Name = strtok(NULL," ");
	    		
	    		hdr->Patient.Smoking      =  Header1[84]%4;
	    		hdr->Patient.AlcoholAbuse = (Header1[84]>>2)%4;
	    		hdr->Patient.DrugAbuse    = (Header1[84]>>4)%4;
	    		hdr->Patient.Medication   = (Header1[84]>>6)%4;
	    		hdr->Patient.Weight       =  Header1[85];
	    		hdr->Patient.Height       =  Header1[86];
	    		hdr->Patient.Sex       	  =  Header1[87]%4;
	    		hdr->Patient.Handedness   = (Header1[87]>>2)%4;
	    		hdr->Patient.Impairment.Visual = (Header1[87]>>4)%4;
	
			*(uint32_t*)(Header1+156) = l_endian_u32( *(uint32_t*) (Header1+156) );
			*(uint32_t*)(Header1+160) = l_endian_u32( *(uint32_t*) (Header1+160) );
			*(uint32_t*)(Header1+164) = l_endian_u32( *(uint32_t*) (Header1+164) );
			if (Header1[156]) {
				hdr->LOC[0] = 0x00292929;
				memcpy(&hdr->LOC[1], Header1+156, 12);
			}
			else {
				*(uint32_t*) (Header1+152) = l_endian_u32(*(uint32_t*) (Header1+152));
				memcpy(&hdr->LOC, Header1+152, 16);
			}

			hdr->T0 		= l_endian_i64( *(int64_t*) (Header1+168) );
			hdr->Patient.Birthday 	= l_endian_i64( *(int64_t*) (Header1+176) );
			// memcpy(&hdr->T0, Header1+168,8);
			// memcpy(&hdr->Patient.Birthday, Header1+176, 8);

			hdr->ID.Equipment 	= l_endian_i64( *(int64_t*) (Header1+192) );
			memcpy(&hdr->IPaddr, Header1+200,6);
			hdr->Patient.Headsize[0]= l_endian_u16( *(uint16_t*)(Header1+206) );
				hdr->Patient.Headsize[1]= l_endian_u16( *(uint16_t*)(Header1+208) );
			hdr->Patient.Headsize[2]= l_endian_u16( *(uint16_t*)(Header1+210) );

			//memcpy(&hdr->ELEC.REF, Header1+212,12);
			//memcpy(&hdr->ELEC.GND, Header1+224,12);
			hdr->ELEC.REF[0]   = l_endian_f32( *(float*)(Header1+ 212) );
			hdr->ELEC.REF[1]   = l_endian_f32( *(float*)(Header1+ 216) );
			hdr->ELEC.REF[2]   = l_endian_f32( *(float*)(Header1+ 220) );
			hdr->ELEC.GND[0]   = l_endian_f32( *(float*)(Header1+ 212) );
			hdr->ELEC.GND[1]   = l_endian_f32( *(float*)(Header1+ 216) );
			hdr->ELEC.GND[2]   = l_endian_f32( *(float*)(Header1+ 220) );
	    	}
	    	else {
	    		strncpy(hdr->AS.PID,Header1+8,80);
	    		hdr->Patient.Id = strtok(hdr->AS.PID," ");
	    		hdr->Patient.Name = strtok(NULL," ");
	    		
	    		tm_time.tm_sec  = atoi(strncpy(tmp,Header1+168+12,2)); 
	    		tm_time.tm_min  = atoi(strncpy(tmp,Header1+168+10,2)); 
	    		tm_time.tm_hour = atoi(strncpy(tmp,Header1+168+8,2)); 
	    		tm_time.tm_mday = atoi(strncpy(tmp,Header1+168+6,2)); 
	    		tm_time.tm_mon  = atoi(strncpy(tmp,Header1+168+4,2)); 
	    		tm_time.tm_year = atoi(strncpy(tmp,Header1+168,4)); 
	    		hdr->T0 = t_time2gdf_time(mktime(&tm_time)); 
		    	hdr->HeadLen 	= l_endian_u64( *(uint64_t*) (Header1+184) ); 
	    	}

	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	    	Header1 = (char*)realloc(Header1,hdr->HeadLen);
	    	Header2 = Header1+256; 
	    	count   = FREAD(Header2, 1, hdr->HeadLen-256, hdr);

		for (k=0; k<hdr->NS; k++)	{
			Header2[16*k + 15] = 0;
			hdr->CHANNEL[k].Label   = (Header2 + 16*k);
			Header2[16*hdr->NS + 80*k + 79] = 0;
			hdr->CHANNEL[k].Transducer  = (Header2 + 16*hdr->NS + 80*k);
			
			hdr->CHANNEL[k].PhysMin = l_endian_f64( *(double*) (Header2+ 8*k + 104*hdr->NS) );
			hdr->CHANNEL[k].PhysMax = l_endian_f64( *(double*) (Header2+ 8*k + 112*hdr->NS) );

			hdr->CHANNEL[k].SPR     = l_endian_u32( *(uint32_t*) (Header2+ 4*k + 216*hdr->NS) );
			hdr->CHANNEL[k].GDFTYP  = l_endian_u16( *(uint16_t*) (Header2+ 4*k + 220*hdr->NS) );
			if (hdr->VERSION < 1.90) {
				strncpy(hdr->CHANNEL[k].PhysDim, Header2 + 8*k + 96*hdr->NS,8);
				hdr->CHANNEL[k].PhysDim[8] = 0; // remove trailing blanks
				int k1;
				for (k1=7; (k1>0) & !isalnum(hdr->CHANNEL[k].PhysDim[k1]); k1--)
					hdr->CHANNEL[k].PhysDim[k1] = 0;

				hdr->CHANNEL[k].PhysDimCode = PhysDimCode(hdr->CHANNEL[k].PhysDim);
				/*
				/ ###FIXME###
				hdr->CHANNEL[k].PreFilt = (hdr->Header2+ 68*k + 136*hdr->NS);
				*/

				hdr->CHANNEL[k].DigMin   = (double) l_endian_i64( *(int64_t*)(Header2+ 8*k + 120*hdr->NS) );
				hdr->CHANNEL[k].DigMax   = (double) l_endian_i64( *(int64_t*)(Header2+ 8*k + 128*hdr->NS) );
			}	
			else {
				hdr->CHANNEL[k].PhysDimCode = l_endian_u16( *(uint16_t*)(Header2+ 2*k + 102*hdr->NS) );
				PhysDim(hdr->CHANNEL[k].PhysDimCode,hdr->CHANNEL[k].PhysDim);

				hdr->CHANNEL[k].DigMin   = l_endian_f64( *(double*)(Header2+ 8*k + 120*hdr->NS) );
				hdr->CHANNEL[k].DigMax   = l_endian_f64( *(double*)(Header2+ 8*k + 128*hdr->NS) );

				hdr->CHANNEL[k].LowPass  = l_endian_f32( *(float*) (Header2+ 4*k + 204*hdr->NS) );
				hdr->CHANNEL[k].HighPass = l_endian_f32( *(float*) (Header2+ 4*k + 208*hdr->NS) );
				hdr->CHANNEL[k].Notch    = l_endian_f32( *(float*) (Header2+ 4*k + 212*hdr->NS) );
				hdr->CHANNEL[k].XYZ[0]   = l_endian_f32( *(float*) (Header2+ 4*k + 224*hdr->NS) );
				hdr->CHANNEL[k].XYZ[1]   = l_endian_f32( *(float*) (Header2+ 4*k + 228*hdr->NS) );
				hdr->CHANNEL[k].XYZ[2]   = l_endian_f32( *(float*) (Header2+ 4*k + 232*hdr->NS) );
				//memcpy(&hdr->CHANNEL[k].XYZ,Header2 + 4*k + 224*hdr->NS,12);
				hdr->CHANNEL[k].Impedance= ldexp(1.0, (uint8_t)Header2[k + 236*hdr->NS]/8);
			}

			hdr->CHANNEL[k].Cal   	= (hdr->CHANNEL[k].PhysMax-hdr->CHANNEL[k].PhysMin)/(hdr->CHANNEL[k].DigMax-hdr->CHANNEL[k].DigMin);
			hdr->CHANNEL[k].Off   	=  hdr->CHANNEL[k].PhysMin-hdr->CHANNEL[k].Cal*hdr->CHANNEL[k].DigMin;

		}
		for (k=0, hdr->SPR=1, hdr->AS.spb=0, hdr->AS.bpb=0; k<hdr->NS;k++) {
			hdr->AS.spb += hdr->CHANNEL[k].SPR;
			hdr->AS.bpb += GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP]*hdr->CHANNEL[k].SPR;			
			hdr->SPR = lcm(hdr->SPR,hdr->CHANNEL[k].SPR);
		}	
		hdr->SampleRate = ((double)(hdr->SPR))*hdr->Dur[1]/hdr->Dur[0];

		// READ EVENTTABLE 
		int c;
		FSEEK(hdr, hdr->HeadLen + hdr->AS.bpb*hdr->NRec, SEEK_SET); 
		c = FREAD(buf, sizeof(uint8_t), 8, hdr);

		if (c<8) {
			hdr->EVENT.SampleRate = hdr->SampleRate; 
			hdr->EVENT.N = 0;
		}	
		else if (hdr->VERSION < 1.94) {
			hdr->EVENT.SampleRate = (float)buf[1] + (buf[2] + buf[3]*256.0)*256.0; 
			hdr->EVENT.N = l_endian_u32( *(uint32_t*) (buf + 4) );
		}	
		else	{
			hdr->EVENT.N = buf[1] + (buf[2] + buf[3]*256)*256; 
			hdr->EVENT.SampleRate = l_endian_f32( *(float*) (buf + 4) );
		}	

 		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, hdr->EVENT.N*sizeof(*hdr->EVENT.POS) );
		hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP) );
		FREAD(hdr->EVENT.POS, sizeof(*hdr->EVENT.POS), hdr->EVENT.N, hdr);
		FREAD(hdr->EVENT.TYP, sizeof(*hdr->EVENT.TYP), hdr->EVENT.N, hdr);

		for (k32u=0; k32u < hdr->EVENT.N; k32u++) {
			hdr->EVENT.POS[k32u] = l_endian_u32(hdr->EVENT.POS[k32u]); 
			hdr->EVENT.TYP[k32u] = l_endian_u16(hdr->EVENT.TYP[k32u]); 
		}
		if (buf[0]>1) {
			hdr->EVENT.DUR = (uint32_t*) realloc(hdr->EVENT.DUR,hdr->EVENT.N*sizeof(*hdr->EVENT.DUR));
			hdr->EVENT.CHN = (uint16_t*) realloc(hdr->EVENT.CHN,hdr->EVENT.N*sizeof(*hdr->EVENT.CHN));
			FREAD(hdr->EVENT.CHN,sizeof(*hdr->EVENT.CHN),hdr->EVENT.N,hdr);
			FREAD(hdr->EVENT.DUR,sizeof(*hdr->EVENT.DUR),hdr->EVENT.N,hdr);

			for (k32u=0; k32u<hdr->EVENT.N; k32u++) {
				hdr->EVENT.DUR[k32u] = l_endian_u32(hdr->EVENT.DUR[k32u]); 
				hdr->EVENT.CHN[k32u] = l_endian_u16(hdr->EVENT.CHN[k32u]); 
			}
		}
		else {
			hdr->EVENT.DUR = NULL;
			hdr->EVENT.CHN = NULL;
		}	
    	}
    	else if ((hdr->TYPE == EDF) | (hdr->TYPE == BDF))	{
    		strncpy(hdr->AS.PID,Header1+8,80);

	    	hdr->HeadLen 	= atoi(strncpy(tmp,Header1+184,8));
	    	hdr->NRec 	= atoi(strncpy(tmp,Header1+236,8));
	    	Dur 		= atoi(strncpy(tmp,Header1+244,8));
	    	hdr->NS		= atoi(strncpy(tmp,Header1+252,4));
		if (!Dur)
		{	
			hdr->Dur[0] = lround(Dur*1e7); 
			hdr->Dur[1] = 10000000L; 
		}
    		tm_time.tm_sec  = atoi(strncpy(tmp,Header1+168+14,2)); 
    		tm_time.tm_min  = atoi(strncpy(tmp,Header1+168+11,2)); 
    		tm_time.tm_hour = atoi(strncpy(tmp,Header1+168+8,2)); 
    		tm_time.tm_mday = atoi(strncpy(tmp,Header1+168,2)); 
    		tm_time.tm_mon  = atoi(strncpy(tmp,Header1+168+3,2)); 
    		tm_time.tm_year = atoi(strncpy(tmp,Header1+168+6,2)); 
    		tm_time.tm_year+= (tm_time.tm_year<85)*100;
		hdr->T0 = tm_time2gdf_time(&tm_time); 

		if (!strncmp(Header1+192,"EDF+",4)) {
	    		hdr->Patient.Id  = strtok(hdr->AS.PID," ");
	    		ptr_str = strtok(NULL," ");
	    		hdr->Patient.Sex = (ptr_str[0]=='f')*2 + (ptr_str[0]=='F')*2 + (ptr_str[0]=='M') + (ptr_str[0]=='m');
	    		ptr_str = strtok(NULL," ");	// birthday
	    		hdr->Patient.Name= strtok(NULL," ");

			if (strlen(ptr_str)==11) {
		    		tm_time.tm_mday = atoi(strtok(ptr_str,"-")); 
		    		strcpy(tmp,strtok(NULL,"-"));
		    		tm_time.tm_year = atoi(strtok(NULL,"-"))-1900; 
		    		tm_time.tm_mon  = !strcmp(tmp,"Feb")+!strcmp(tmp,"Mar")*2+!strcmp(tmp,"Apr")*3+!strcmp(tmp,"May")*4+!strcmp(tmp,"Jun")*5+!strcmp(tmp,"Jul")*6+!strcmp(tmp,"Aug")*7+!strcmp(tmp,"Sep")*8+!strcmp(tmp,"Oct")*9+!strcmp(tmp,"Nov")*10+!strcmp(tmp,"Dec")*11;
		    		tm_time.tm_sec  = 0; 
		    		tm_time.tm_min  = 0; 
		    		tm_time.tm_hour = 12; 
		    		hdr->Patient.Birthday = t_time2gdf_time(mktime(&tm_time));
		    	}	
		}

	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	    	Header1 = (char*) realloc(Header1,hdr->HeadLen);
	    	Header2 = Header1+256; 
	    	count   = FREAD(Header2, 1, hdr->HeadLen-256, hdr);

		for (k=0; k<hdr->NS; k++)	{
			hdr->CHANNEL[k].Label   = (Header2 + 16*k);
			hdr->CHANNEL[k].Label[15]=0;//   hack FIXME
			for (k1=14; isspace(hdr->CHANNEL[k].Label[k1]) && k1; k1--)
				hdr->CHANNEL[k].Label[k1]='\0';		// deblank

			hdr->CHANNEL[k].Transducer  = (Header2 + 80*k + 16*hdr->NS);
			hdr->CHANNEL[k].Transducer[79]=0;//   hack  FIXME
			for (k1=78; isspace(hdr->CHANNEL[k].Transducer[k1]) && k1; k1--)
				hdr->CHANNEL[k].Transducer[k1]='\0'; 	// deblank
			
			// PhysDim -> PhysDimCode 
			strncpy(hdr->CHANNEL[k].PhysDim,Header2 + 8*k + 96*hdr->NS,8);
			hdr->CHANNEL[k].PhysDim[8] = 0; // remove trailing blanks
			int k1;			
			for (k1=7; (k1>0) & !isalnum(hdr->CHANNEL[k].PhysDim[k1]); k1--)
				hdr->CHANNEL[k].PhysDim[k1] = 0;
			hdr->CHANNEL[k].PhysDimCode = PhysDimCode(hdr->CHANNEL[k].PhysDim);
			
			hdr->CHANNEL[k].PhysMin = atof(strncpy(tmp,Header2 + 8*k + 104*hdr->NS,8)); 
			hdr->CHANNEL[k].PhysMax = atof(strncpy(tmp,Header2 + 8*k + 112*hdr->NS,8)); 
			hdr->CHANNEL[k].DigMin  = atof(strncpy(tmp,Header2 + 8*k + 120*hdr->NS,8)); 
			hdr->CHANNEL[k].DigMax  = atof(strncpy(tmp,Header2 + 8*k + 128*hdr->NS,8)); 
			hdr->CHANNEL[k].Cal     = (hdr->CHANNEL[k].PhysMax-hdr->CHANNEL[k].PhysMin)/(hdr->CHANNEL[k].DigMax-hdr->CHANNEL[k].DigMin);
			hdr->CHANNEL[k].Off     =  hdr->CHANNEL[k].PhysMin-hdr->CHANNEL[k].Cal*hdr->CHANNEL[k].DigMin;

			hdr->CHANNEL[k].SPR     = atol(strncpy(tmp,Header2+  8*k + 216*hdr->NS,8));
			hdr->CHANNEL[k].GDFTYP  = ((hdr->TYPE!=BDF) ? 3 : 255+24); 
			
			//hdr->CHANNEL[k].PreFilt = (Header2+ 80*k + 136*hdr->NS);
			if ((hdr->TYPE==EDF) & strncmp(Header1+192,"EDF+",4)) {
				hdr->CHANNEL[k].OnOff = memcmp(hdr->CHANNEL[k].Label,"EDF Annotations",15);
				EventChannel = k; 
			// decode filter information into hdr->Filter.{Lowpass, Highpass, Notch}					
			}	
			else if (hdr->TYPE==BDF) {
				hdr->CHANNEL[k].OnOff = strcmp(hdr->CHANNEL[k].Label,"Status");
				EventChannel = k; 
			}	
			else	{
				hdr->CHANNEL[k].OnOff = 1;
			}
		}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// EDF does not support automated overflow and saturation detection

		for (k=0, hdr->SPR=1, hdr->AS.spb=0, hdr->AS.bpb=0; k<hdr->NS; k++) {
			hdr->AS.spb += hdr->CHANNEL[k].SPR;
			hdr->AS.bpb += GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP]*hdr->CHANNEL[k].SPR;
			if (hdr->CHANNEL[k].OnOff)
				hdr->SPR = lcm(hdr->SPR,hdr->CHANNEL[k].SPR);
		}	
		hdr->SampleRate = ((double)(hdr->SPR))*hdr->Dur[1]/hdr->Dur[0];
	}      	

	else if (hdr->TYPE==ACQ) {
		/* defined in http://biopac.com/AppNotes/app156FileFormat/FileFormat.htm */
		hdr->NS   = l_endian_i16(*(int16_t*)(Header1+10));
		hdr->SampleRate = 1000.0/l_endian_f64(*(double*)(Header1+16));
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
	    	Header1 = (char*)realloc(Header1,hdr->HeadLen);
	    	count   = FREAD(Header1+256, 1, hdr->HeadLen-256, hdr);
		uint32_t POS = hdr->HeadLen; 
	    	
		// read "foreign data section" and "per channel data types section"  
		hdr->HeadLen += l_endian_u16(*(uint16_t*)(Header1+hdr->HeadLen-4));
		hdr->HeadLen += 4*hdr->NS; 
	    	Header1 = (char*)realloc(Header1,hdr->HeadLen+8);
	    	count   = FREAD(Header1+POS, 1, hdr->HeadLen-POS, hdr);
		
		// define channel specific header information
		hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
		uint32_t* ACQ_NoSamples = (uint32_t*) calloc(hdr->NS,sizeof(uint32_t));
		uint16_t CHAN; 
    		POS = l_endian_u32(*(uint32_t*)(Header1+6));
		for (k = 0; k < hdr->NS; k++)	{
			hdr->CHANNEL[k].Transducer  = "";
			CHAN = l_endian_u16(*(uint16_t*)(Header1+POS+4));
			hdr->CHANNEL[k].Label   = (char*)(Header1+POS+6);
			hdr->CHANNEL[k].Label[39]   = 0;  
			strncpy(tmp,Header1+POS+68,20);
			hdr->CHANNEL[k].PhysDimCode = PhysDimCode(tmp);
			/* PhysDim is OBSOLETE  
			hdr->CHANNEL[k].PhysDim = (char*)(Header1+POS+68);
			hdr->CHANNEL[k].PhysDim[19] = 0;
			*/
			//strncpy(hdr->CHANNEL[k].PhysDim,(char*)(Header1+POS+68),20);
			hdr->CHANNEL[k].Off     = l_endian_f64(*(double*)(Header1+POS+52));
			hdr->CHANNEL[k].Cal     = l_endian_f64(*(double*)(Header1+POS+60));

			hdr->CHANNEL[k].SPR     = 1; 
			if (hdr->VERSION >= 38.0) {
				hdr->CHANNEL[k].SPR = l_endian_u16(*(uint16_t*)(Header1+POS+250));  // used here as Divider
			}
			hdr->SPR = lcm(hdr->SPR, hdr->CHANNEL[k].SPR);

			ACQ_NoSamples[k] = l_endian_u32(*(uint32_t*)(Header1+POS+88));

			POS += l_endian_u32(*(uint32_t*)(Header1+POS));
		}
		hdr->NRec /= hdr->SPR; 

		/// foreign data section - skip 
		POS += l_endian_u16(*(uint16_t*)(Header1+POS));
		
		size_t DataLen=0; 
		for (k=0, hdr->AS.spb=0, hdr->AS.bpb=0; k<hdr->NS; k++)	{
			if (hdr->VERSION>=38.0)
				hdr->CHANNEL[k].SPR = hdr->SPR/hdr->CHANNEL[k].SPR;  // convert DIVIDER into SPR

			switch (l_endian_u16(*(uint16_t*)(Header1+POS+2)))
			{
			case 1: 
				hdr->CHANNEL[k].GDFTYP = 17;  // double
				hdr->AS.bpb += hdr->CHANNEL[k].SPR<<3;
				DataLen += ACQ_NoSamples[k]<<3; 
				break;   
			case 2: 
				hdr->CHANNEL[k].GDFTYP = 3;   // int
				hdr->AS.bpb += hdr->CHANNEL[k].SPR<<1;
				DataLen += ACQ_NoSamples[k]<<1; 
				break;
			default:
				fprintf(stderr,"ERROR SOPEN(ACQ-READ): invalid type in channel %i.\n",k);	
			};
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
	    	size_t LengthMarkerItemSection = (l_endian_u32(*(uint32_t*)(Header1+POS)));

	    	hdr->EVENT.N = (l_endian_u32(*(uint32_t*)(Header1+POS+4)));
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
			hdr->EVENT.POS[k] = l_endian_u32(*(uint32_t*)(Header1+POS));
			POS += 12 + l_endian_u16(*(uint16_t*)(Header1+POS+10));
		} 
*/
		FSEEK(hdr, hdr->HeadLen, SEEK_SET); 
	}      	

	else if (hdr->TYPE==BKR) {
	    	Header1 = (char*) realloc(Header1,1024);
	    	count   = FREAD(Header1+256,1,1024-256,hdr);

	    	hdr->HeadLen 	 = 1024; 
		hdr->NS  	 = l_endian_u16( *(uint16_t*) (Header1+2) ); 
		hdr->SampleRate  = (double)l_endian_u16( *(uint16_t*) (Header1+4) ); 
		hdr->NRec   	 = l_endian_u32( *(uint32_t*) (Header1+6) ); 
		hdr->SPR  	 = l_endian_u32( *(uint32_t*) (Header1+10) ); 
		hdr->NRec 	*= hdr->SPR; 
		hdr->SPR  	 = 1; 
		hdr->T0 	 = 0;        // Unknown;
	    	/* extract more header information */
	    	hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS;k++)	{
			hdr->CHANNEL[k].Transducer  = "";
		    	hdr->CHANNEL[k].GDFTYP 	 = 3; 
		    	hdr->CHANNEL[k].SPR 	 = 1; // *(int32_t*)(Header1+56);
		    	*(uint32_t*)(Header1+22) = l_endian_u32(*(uint32_t*)Header1+22);
		    	*(uint32_t*)(Header1+26) = l_endian_u32(*(uint32_t*)Header1+26);
		    	hdr->CHANNEL[k].LowPass	 = *(float*)(Header1+22);
		    	hdr->CHANNEL[k].HighPass = *(float*)(Header1+26);
		    	hdr->CHANNEL[k].Notch	 = -1.0;
		    	*(uint16_t*)(Header1+14) = l_endian_u16(*(uint16_t*)Header1+14);
		    	*(uint16_t*)(Header1+16) = l_endian_u16(*(uint16_t*)Header1+16);
		    	hdr->CHANNEL[k].PhysMax	 = (double)*(uint16_t*)(Header1+14);
		    	hdr->CHANNEL[k].DigMax	 = (double)*(uint16_t*)(Header1+16);
		    	hdr->CHANNEL[k].Cal	 =  ((double)hdr->CHANNEL[k].PhysMax)/hdr->CHANNEL[k].DigMax;
		    	hdr->CHANNEL[k].Off	 = 0.0;
			hdr->CHANNEL[k].OnOff    = 1;
		}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// BKR does not support automated overflow and saturation detection
	}

	else if (hdr->TYPE==CFWB) {
	    	hdr->SampleRate = 1/l_endian_f64(*(double*)(Header1+8));
	    	tm_time.tm_year = l_endian_u32(*(uint32_t*)(Header1+16));
	    	tm_time.tm_mon  = l_endian_u32(*(uint32_t*)(Header1+20));
	    	tm_time.tm_mday = l_endian_u32(*(uint32_t*)(Header1+24));
	    	tm_time.tm_hour = l_endian_u32(*(uint32_t*)(Header1+28));
	    	tm_time.tm_min  = l_endian_u32(*(uint32_t*)(Header1+32));
	    	tm_time.tm_sec  = (int)l_endian_f64(*(double*)(Header1+36));
    		hdr->T0 	= tm_time2gdf_time(&tm_time);
	    	// = *(double*)(Header1+44);
	    	hdr->NS   	= l_endian_u32( *(uint32_t*)(Header1+52));
	    	hdr->NRec	= l_endian_u32( *(uint32_t*)(Header1+56));
	    	//  	= *(int32_t*)(Header1+60);	// TimeChannel
	    	//  	= *(int32_t*)(Header1+64);	// DataFormat

	    	hdr->HeadLen = 68 + hdr->NS*96; 
	    	Header1 = (char*)realloc(Header1,hdr->HeadLen);
	    	Header2 = Header1+68; 
	    	if (count<=hdr->HeadLen)
			count = FREAD(Header1+count, 1, hdr->HeadLen-count, hdr);
		else 	
	    		FSEEK(hdr, hdr->HeadLen, SEEK_SET);
		
		int GDFTYP = l_endian_u32(*(uint32_t*)(Header1+64));
	    	hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS; k++)	{
			hdr->CHANNEL[k].Transducer  = "";
		    	hdr->CHANNEL[k].GDFTYP 	= CFWB_GDFTYP[GDFTYP-1];
		    	hdr->CHANNEL[k].SPR 	= 1; // *(int32_t*)(Header1+56);
		    	hdr->CHANNEL[k].Label	= Header2+k*96;
		    	//hdr->CHANNEL[k].PhysDim	= Header2+k*96+32;  // OBSOLETE
		    	strncpy(hdr->CHANNEL[k].PhysDim,Header2+k*96+32,16);
			//hdr->CHANNEL[k].PhysDimCode = PhysDimCode(Header2+k*96+32);
		    	hdr->CHANNEL[k].Cal	= l_endian_f64(*(double*)(Header2+k*96+64));
		    	hdr->CHANNEL[k].Off	= l_endian_f64(*(double*)(Header2+k*96+72));
		    	hdr->CHANNEL[k].PhysMax	= l_endian_f64(*(double*)(Header2+k*96+80));
		    	hdr->CHANNEL[k].PhysMin	= l_endian_f64(*(double*)(Header2+k*96+88));
		    	hdr->CHANNEL[k].DigMax	= (hdr->CHANNEL[k].PhysMax - hdr->CHANNEL[k].Off) / hdr->CHANNEL[k].Cal; 
		    	hdr->CHANNEL[k].DigMin	= (hdr->CHANNEL[k].PhysMin - hdr->CHANNEL[k].Off) / hdr->CHANNEL[k].Cal; 
			hdr->CHANNEL[k].OnOff    = 1;
		}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// CFWB does not support automated overflow and saturation detection
	}
	else if (hdr->TYPE==CNT) {
	    	Header1 = (char*) realloc(Header1,900);
	    	hdr->VERSION = atof(Header1+8);
	    	count   = FREAD(Header1+256,1,900-256,hdr);

	    	ptr_str = Header1+136;
    		hdr->Patient.Sex = (ptr_str[0]=='f')*2 + (ptr_str[0]=='F')*2 + (ptr_str[0]=='M') + (ptr_str[0]=='m');
	    	ptr_str = Header1+137;
	    	hdr->Patient.Handedness = (ptr_str[0]=='r')*2 + (ptr_str[0]=='R')*2 + (ptr_str[0]=='L') + (ptr_str[0]=='l');
    		tm_time.tm_sec  = atoi(strncpy(tmp,Header1+225+16,2)); 
    		tm_time.tm_min  = atoi(strncpy(tmp,Header1+225+13,2)); 
    		tm_time.tm_hour = atoi(strncpy(tmp,Header1+225+10,2)); 
    		tm_time.tm_mday = atoi(strncpy(tmp,Header1+225,2)); 
    		tm_time.tm_mon  = atoi(strncpy(tmp,Header1+225+3,2)); 
    		tm_time.tm_year = atoi(strncpy(tmp,Header1+225+6,2)); 
	    	if (tm_time.tm_year<=80)    	tm_time.tm_year += 2000;
	    	else if (tm_time.tm_year<100) 	tm_time.tm_year += 1900;
		hdr->T0 = tm_time2gdf_time(&tm_time); 
		
		hdr->NS  = l_endian_u16( *(uint16_t*) (Header1+370) ); 
	    	hdr->HeadLen = 900+hdr->NS*75; 
		hdr->SampleRate = l_endian_u16( *(uint16_t*) (Header1+376) ); 
		hdr->SPR = 1; 
#define _eventtablepos (*(uint32_t*)(Header1+886))
		hdr->NRec = (_eventtablepos-hdr->HeadLen)/(hdr->NS*2);
		
	    	Header1 = (char*) realloc(Header1,hdr->HeadLen);
	    	Header2 = Header1+900; 
	    	count   = FREAD(Header2,1,hdr->NS*75,hdr);

	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS;k++)	{
			hdr->CHANNEL[k].Transducer  = "";
		    	hdr->CHANNEL[k].GDFTYP 	= 3;
		    	hdr->CHANNEL[k].SPR 	= 1; // *(int32_t*)(Header1+56);
		    	hdr->CHANNEL[k].Label	= Header2+k*75;
		    	//hdr->CHANNEL[k].PhysDim	= "uV";	// OBSOLETE
		    	//strcpy(hdr->CHANNEL[k].PhysDim,"uV");
			hdr->CHANNEL[k].PhysDimCode = 4256+19;
		    	hdr->CHANNEL[k].Cal	= l_endian_f32(*(float*)(Header2+k*75+59));
		    	hdr->CHANNEL[k].Cal    *= l_endian_f32(*(float*)(Header2+k*75+71))/204.8;
		    	hdr->CHANNEL[k].Off	= l_endian_f32(*(float*)(Header2+k*75+47)) * hdr->CHANNEL[k].Cal;
		    	hdr->CHANNEL[k].HighPass= CNT_SETTINGS_HIGHPASS[(uint8_t)Header2[64+k*75]];
		    	hdr->CHANNEL[k].LowPass	= CNT_SETTINGS_LOWPASS[(uint8_t)Header2[65+k*75]];
		    	hdr->CHANNEL[k].Notch	= CNT_SETTINGS_NOTCH[(uint8_t)Header1[682]];
			hdr->CHANNEL[k].OnOff   = 1;
		}

	    	/* extract more header information */
	    	// eventtablepos = l_endian_u32( *(uint32_t*) (Header1+886) );
	    	FSEEK(hdr, 68, SEEK_SET);
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// automated overflow and saturation detection not supported
	}
	
	else if (hdr->TYPE==SCP_ECG) {
		hdr->HeadLen 	= l_endian_u32(*(uint32_t*)(Header1+2));
		Header1         = (char*)realloc(Header1,hdr->HeadLen);
		hdr->AS.Header1 = (uint8_t*)Header1; 
	    	count += FREAD(hdr->AS.Header1+count, 1, hdr->HeadLen-count, hdr);
	    	uint16_t crc 	= CRCEvaluate(hdr->AS.Header1+2,hdr->HeadLen-2);

	    	if ( l_endian_u16(*(uint16_t*)hdr->AS.Header1) != crc)
	    		fprintf(stderr,"Warning SOPEN(SCP-READ): Bad CRC %x %x !\n",crc,*(uint16_t*)(hdr->AS.Header1));

		hdr = sopen_SCP_read(hdr);

		for (k=0; k<hdr->NS; k++) {	
			k1 = hdr->CHANNEL[k].LeadIdCode;
			if (k1>0) hdr->CHANNEL[k].Label = (char*)LEAD_ID_TABLE[k1];
		}    	
	}
	
	else if (hdr->TYPE==HL7aECG) 
	{
		hdr = sopen_HL7aECG_read(hdr);
	}
	else 
	{
		fprintf(stdout,"ERROR BIOSIG SOPEN(READ): Format %i of file %s not supported\n",hdr->TYPE,hdr->FileName);
    		FCLOSE(hdr);
    		free(hdr->AS.Header1);
    		free(hdr);
		return(NULL);
	}

	hdr->AS.Header1 = (uint8_t*)Header1; 	
	for (k=0; k<hdr->NS; k++) {	
		// set HDR.PhysDim
		k1 = hdr->CHANNEL[k].PhysDimCode;
		if (k1>0)
			PhysDim(k1,hdr->CHANNEL[k].PhysDim);
		else 
			hdr->CHANNEL[k].PhysDimCode = PhysDimCode(hdr->CHANNEL[k].PhysDim);

		// set HDR.PhysDimCode
		if (hdr->CHANNEL[k].LeadIdCode == 0)
		if (!strncmp(hdr->CHANNEL[k].Label, "MDC_ECG_LEAD_", 13)) {
			// MDC_ECG_LEAD_* 
			for (k1=0; strcmp(hdr->CHANNEL[k].Label+13, LEAD_ID_TABLE[k1]) && LEAD_ID_TABLE[k1] != NULL; k1++); 
			if (LEAD_ID_TABLE[k1] != NULL)	
				hdr->CHANNEL[k].LeadIdCode = k1;
		}
		else {
			for (k1=0; strcmp(hdr->CHANNEL[k].Label, LEAD_ID_TABLE[k1]) && LEAD_ID_TABLE[k1] != NULL; k1++); 
			if (LEAD_ID_TABLE[k1] != NULL)	
				hdr->CHANNEL[k].LeadIdCode = k1;
		}
	}    	
	
}
else if (!strncmp(MODE,"w",1))	 /* --- WRITE --- */
{
	for (k=0; k<hdr->NS; k++) {	
		// set HDR.PhysDim
		k1 = hdr->CHANNEL[k].PhysDimCode;
		if (k1>0) PhysDim(k1,hdr->CHANNEL[k].PhysDim);
	}    	

    	if (hdr->TYPE==CFWB) {	
	     	hdr->HeadLen = 68 + hdr->NS*96;
	    	Header1 = (char*) malloc(hdr->HeadLen);
	    	Header2 = Header1+68; 
		memset(Header1,0,hdr->HeadLen);
	    	memcpy(Header1,"CFWB\1\0\0\0",8);
	    	*(double*)(Header1+8) = l_endian_f64(1/hdr->SampleRate);
		
		tt = gdf_time2t_time(hdr->T0); 
		struct tm *t = localtime(&tt);
    		*(int32_t*)(Header1+16) = l_endian_u32(t->tm_year);
	    	*(int32_t*)(Header1+20) = l_endian_u32(t->tm_mon);
	    	*(int32_t*)(Header1+24) = l_endian_u32(t->tm_mday);
	    	*(int32_t*)(Header1+28) = l_endian_u32(t->tm_hour);
	    	*(int32_t*)(Header1+32) = l_endian_u32(t->tm_min);
	    	*(double*)(Header1+36)  = l_endian_f64(t->tm_sec);
	    	*(double*)(Header1+44)  = l_endian_f64(0.0);	// pretrigger time 
	    	*(int32_t*)(Header1+52) = l_endian_u32(hdr->NS);
	    	hdr->NRec *= hdr->SPR; hdr->SPR = 1;
	    	*(int32_t*)(Header1+56)	= l_endian_u32(hdr->NRec); // number of samples 
	    	*(int32_t*)(Header1+60)	= l_endian_i32(0);	// 1: time channel

	    	int GDFTYP = 3; // 1:double, 2:float, 3: int16; see CFWB_GDFTYP too. 
		for (k=0; k<hdr->NS; k++) {
			/* if int16 is not sufficient, use float or double */
			if (hdr->CHANNEL[k].GDFTYP>16)
				GDFTYP = 1;	// double 
			else if (hdr->CHANNEL[k].GDFTYP>3)
				GDFTYP = 2;	// float 
		}
	    	*(int32_t*)(Header1+64)	= l_endian_i32(GDFTYP);	// 1: double, 2: float, 3:short
		
		for (k=0; k<hdr->NS; k++) {
	    		hdr->CHANNEL[k].SPR = 1;
			hdr->CHANNEL[k].GDFTYP = CFWB_GDFTYP[GDFTYP-1];
			const char *tmpstr;
			if (hdr->CHANNEL[k].LeadIdCode)
				tmpstr = LEAD_ID_TABLE[hdr->CHANNEL[k].LeadIdCode];
			else
				tmpstr = hdr->CHANNEL[k].Label;
		     	len = strlen(tmpstr);
		     	memcpy(Header2+96*k, tmpstr, min(len,32));

		     	PhysDim(hdr->CHANNEL[k].PhysDimCode, tmp);
		     	len = strlen(tmp);
		     	memcpy(Header2+96*k+32, tmp, min(len,32));
			
			*(double*)(Header2+96*k+64) = l_endian_f64(hdr->CHANNEL[k].Cal);
			*(double*)(Header2+96*k+72) = l_endian_f64(hdr->CHANNEL[k].Off);
			*(double*)(Header2+96*k+80) = l_endian_f64(hdr->CHANNEL[k].PhysMax);
			*(double*)(Header2+96*k+88) = l_endian_f64(hdr->CHANNEL[k].PhysMin);
		}
		hdr->AS.Header1 = (uint8_t*)Header1; 
	}
    	else if (hdr->TYPE==GDF) {	
	     	hdr->HeadLen = (hdr->NS+1)*256;
	    	Header1 = (char*) malloc(hdr->HeadLen);
	    	Header2 = Header1+256; 

		memset(Header1,0,hdr->HeadLen);
		hdr->VERSION = 1.99;
	     	sprintf(Header1,"GDF %4.2f",hdr->VERSION);
	     	strncat(Header1+8, hdr->Patient.Id,   66);
	     	strncat(Header1+8, " ",   66);
	     	strncat(Header1+8, hdr->Patient.Name, 66);

	     	Header1[84] = (hdr->Patient.Smoking%4) + ((hdr->Patient.AlcoholAbuse%4)<<2) + ((hdr->Patient.DrugAbuse%4)<<4) + ((hdr->Patient.Medication%4)<<6);
	     	Header1[85] =  hdr->Patient.Weight;
	     	Header1[86] =  hdr->Patient.Height;
	     	Header1[87] = (hdr->Patient.Sex%4) + ((hdr->Patient.Handedness%4)<<2) + ((hdr->Patient.Impairment.Visual%4)<<4);

	     	len = strlen(hdr->AS.RID);
	     	memcpy(Header1+ 88,  hdr->AS.RID, min(len,80));
		memcpy(Header1+152, &hdr->LOC, 16);  
		*(uint32_t*) (Header1+152) = l_endian_u32( *(uint32_t*) (Header1+152) );
		*(uint32_t*) (Header1+156) = l_endian_u32( *(uint32_t*) (Header1+156) );
		*(uint32_t*) (Header1+160) = l_endian_u32( *(uint32_t*) (Header1+160) );
		*(uint32_t*) (Header1+164) = l_endian_u32( *(uint32_t*) (Header1+164) );
	
		//memcpy(Header1+168, &hdr->T0, 8); 
		*(uint64_t*) (Header1+168) = l_endian_u64(hdr->T0);
		//memcpy(Header1+176, &hdr->Patient.Birthday, 8); 
		*(uint64_t*) (Header1+176) = l_endian_u64(hdr->Patient.Birthday);
		// *(uint16_t*)(Header1+184) = (hdr->HeadLen>>8)+(hdr->HeadLen%256>0); 
		*(uint16_t*) (Header1+184) = l_endian_u16(hdr->HeadLen>>8); 
		
		memcpy(Header1+192, &hdr->ID.Equipment, 8);

		memcpy(Header1+200, &hdr->IPaddr, 6);
		memcpy(Header1+206, &hdr->Patient.Headsize, 6);
		*(float*) (Header1+212) = l_endian_f32(hdr->ELEC.REF[0]);
		*(float*) (Header1+216) = l_endian_f32(hdr->ELEC.REF[1]);
		*(float*) (Header1+220) = l_endian_f32(hdr->ELEC.REF[2]);
		*(float*) (Header1+224) = l_endian_f32(hdr->ELEC.GND[0]);
		*(float*) (Header1+228) = l_endian_f32(hdr->ELEC.GND[1]);
		*(float*) (Header1+232) = l_endian_f32(hdr->ELEC.GND[2]);

		//memcpy(Header1+236, &hdr->NRec, 8);
		*(uint64_t*) (Header1+236) = l_endian_u64(hdr->NRec);
		//memcpy(Header1+244, &hdr->Dur, 8); 
		*(uint32_t*) (Header1+244) = l_endian_u32(hdr->Dur[0]);
		*(uint32_t*) (Header1+248) = l_endian_u32(hdr->Dur[1]);
		//memcpy(Header1+252, &hdr->NS, 2); 
		*(uint16_t*) (Header1+252) = l_endian_u16(hdr->NS);

	     	/* define HDR.Header2 
	     	this requires checking the arguments in the fields of the struct HDR.CHANNEL
	     	and filling in the bytes in HDR.Header2. 
	     	*/
		for (k=0; k<hdr->NS; k++)
		{
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

		     	if (hdr->VERSION<1.9)
		     		memcpy(Header2+ 8*k + 96*hdr->NS, tmp, min(8,len));
		     	else {
		     		memcpy(Header2+ 6*k + 96*hdr->NS, tmp, min(6,len));
		     		*(uint16_t*)(Header2+ 2*k + 102*hdr->NS) = l_endian_u16(hdr->CHANNEL[k].PhysDimCode);
			};

		     	*(double*)(Header2 + 8*k + 104*hdr->NS)   = l_endian_f64(hdr->CHANNEL[k].PhysMin);
		     	*(double*)(Header2 + 8*k + 112*hdr->NS)   = l_endian_f64(hdr->CHANNEL[k].PhysMax);
		     	*(double*)(Header2 + 8*k + 120*hdr->NS)   = l_endian_f64(hdr->CHANNEL[k].DigMin);
		     	*(double*)(Header2 + 8*k + 128*hdr->NS)   = l_endian_f64(hdr->CHANNEL[k].DigMax);

		     	*(float*) (Header2+ 4*k + 204*hdr->NS)    = l_endian_f32(hdr->CHANNEL[k].LowPass);
		     	*(float*) (Header2+ 4*k + 208*hdr->NS)    = l_endian_f32(hdr->CHANNEL[k].HighPass);
		     	*(float*) (Header2+ 4*k + 212*hdr->NS)    = l_endian_f32(hdr->CHANNEL[k].Notch);
		     	*(uint32_t*) (Header2+ 4*k + 216*hdr->NS) = l_endian_u32(hdr->CHANNEL[k].SPR);
		     	*(uint32_t*) (Header2+ 4*k + 220*hdr->NS) = l_endian_u32(hdr->CHANNEL[k].GDFTYP);

			*(float*) (Header2+ 4*k + 224*hdr->NS)    = l_endian_f32(hdr->CHANNEL[k].XYZ[0]);
			*(float*) (Header2+ 4*k + 228*hdr->NS)    = l_endian_f32(hdr->CHANNEL[k].XYZ[1]);
			*(float*) (Header2+ 4*k + 232*hdr->NS)    = l_endian_f32(hdr->CHANNEL[k].XYZ[2]);

	     		Header2[k+236*hdr->NS] = (uint8_t)ceil(log10(min(39e8,hdr->CHANNEL[k].Impedance))/log10(2.0)*8.0-0.5);
		}
	    	hdr->AS.bi = (uint32_t*) realloc(hdr->AS.bi,(hdr->NS+1)*sizeof(int32_t));
		hdr->AS.bi[0] = 0;
		for (k=0, hdr->AS.spb=0, hdr->AS.bpb=0; k<hdr->NS;)
		{
			hdr->AS.spb += hdr->CHANNEL[k].SPR;
			hdr->AS.bpb += GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP] * hdr->CHANNEL[k].SPR;
			hdr->AS.bi[++k] = hdr->AS.bpb; 
		}	
		hdr->AS.Header1 = (uint8_t*)Header1; 
	}
    	else if ((hdr->TYPE==EDF) | (hdr->TYPE==BDF)) {	
	     	hdr->HeadLen = (hdr->NS+1)*256;
	    	Header1 = (char*) malloc(hdr->HeadLen);
	    	Header2 = Header1+256; 
		memset(Header1,' ',hdr->HeadLen);
		if (hdr->TYPE==BDF) {	
			Header1[0] = 255;
	     		memcpy(Header1+1,"BIOSEMI",7);
		}
		else {
			Header1[0] = '0';
	     	}

		tt = gdf_time2t_time(hdr->Patient.Birthday); 
		if (hdr->Patient.Birthday>1) strftime(tmp,81,"%d-%b-%Y",localtime(&tt));
		else strcpy(tmp,"X");	
		sprintf(cmd,"%s %c %s %s",hdr->Patient.Id,GENDER[hdr->Patient.Sex],tmp,hdr->Patient.Name);
	     	memcpy(Header1+8, cmd, strlen(cmd));
	     	
		tt = gdf_time2t_time(hdr->T0); 
		if (hdr->T0>1) strftime(tmp,81,"%d-%b-%Y",localtime(&tt));
		else strcpy(tmp,"X");	
		len = sprintf(cmd,"Startdate %s X X ",tmp);
	     	memcpy(Header1+88, cmd, len);
	     	memcpy(Header1+88+len, &hdr->ID.Equipment, 8);
	     	
		tt = gdf_time2t_time(hdr->T0); 
		strftime(tmp,81,"%d.%m.%y%H:%M:%S",localtime(&tt));
	     	memcpy(Header1+168, tmp, 16);

		len = sprintf(tmp,"%i",hdr->HeadLen);
		if (len>8) fprintf(stderr,"Warning: HeaderLength is (%s) to long.\n",tmp);  
	     	memcpy(Header1+184, tmp, len);
	     	memcpy(Header1+192, "EDF+C  ", 5);

		len = sprintf(tmp,"%Lu",hdr->NRec);
		if (len>8) fprintf(stderr,"Warning: NRec is (%s) to long.\n",tmp);  
	     	memcpy(Header1+236, tmp, len);

		len = sprintf(tmp,"%f",((double)hdr->Dur[0])/hdr->Dur[1]);
		if (len>8) fprintf(stderr,"Warning: Duration is %s) to long.\n",tmp);  
	     	memcpy(Header1+244, tmp, len);

		len = sprintf(tmp,"%i",hdr->NS);
		if (len>4) fprintf(stderr,"Warning: NS is %s) to long.\n",tmp);  
	     	memcpy(Header1+252, tmp, len);
	     	
		for (k=0;k<hdr->NS;k++)
		{
			const char *tmpstr;
			if (hdr->CHANNEL[k].LeadIdCode)
				tmpstr = LEAD_ID_TABLE[hdr->CHANNEL[k].LeadIdCode];
			else
				tmpstr = hdr->CHANNEL[k].Label;
		     	len = strlen(tmpstr);
			if (len>15) 
			//fprintf(stderr,"Warning: Label (%s) of channel %i is to long.\n",hdr->CHANNEL[k].Label,k);  
		     	fprintf(stderr,"Warning: Label of channel %i is to long (%i>16).\n",k, len);  
		     	memcpy(Header2+16*k,tmpstr,min(len,16));
		     	len = strlen(hdr->CHANNEL[k].Transducer);
			if (len>80) 
			//fprintf(stderr,"Warning: Transducer (%s) of channel %i is to long.\n",hdr->CHANNEL[k].Transducer,k);  
		     	fprintf(stderr,"Warning: Transducer of channel %i is to long (%i>80).\n",k, len);  
		     	memcpy(Header2+80*k + 16*hdr->NS,hdr->CHANNEL[k].Transducer,min(len,80));
		     	PhysDim(hdr->CHANNEL[k].PhysDimCode, tmp);
		     	len = strlen(tmp);
		     	if (len>8) fprintf(stderr,"Warning: Physical Dimension (%s) of channel %i is to long (%i>8).\n",tmp,k,len);  
		     	memcpy(Header2+ 8*k + 96*hdr->NS,tmp,min(len,8));
	
			len = sprintf(tmp,"%f",hdr->CHANNEL[k].PhysMin);
			if (len>8) fprintf(stderr,"Warning: PhysMin (%s) of channel %i is to long (%i>8).\n",tmp,k,len);  
		     	memcpy(Header2+ 8*k + 104*hdr->NS,tmp,min(8,len));
			len = sprintf(tmp,"%f",hdr->CHANNEL[k].PhysMax);
			if (len>8) fprintf(stderr,"Warning: PhysMax (%s) of channel %i is to long(%i>8).\n",tmp,k,len);  
		     	memcpy(Header2+ 8*k + 112*hdr->NS,tmp,min(8,len));
			len = sprintf(tmp,"%f",hdr->CHANNEL[k].DigMin);
			if (len>8) fprintf(stderr,"Warning: DigMin (%s) of channel %i is to long(%i>8).\n",tmp,k,len);  
		     	memcpy(Header2+ 8*k + 120*hdr->NS,tmp,min(8,len));
			len = sprintf(tmp,"%f",hdr->CHANNEL[k].DigMax);
			if (len>8) fprintf(stderr,"Warning: DigMax (%s) of channel %i is to long(%i>8).\n",tmp,k,len);  
		     	memcpy(Header2+ 8*k + 128*hdr->NS,tmp,min(8,len));
		     	
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
		hdr->AS.Header1 = (uint8_t*)Header1; 
	}
    	else if (hdr->TYPE==SCP_ECG) {	
    		hdr->FileName = FileName;
    		hdr = sopen_SCP_write(hdr);
	}
    	else if (hdr->TYPE==HL7aECG) {	
   		hdr->FileName = FileName;
		for (k=0; k<hdr->NS; k++) {
			hdr->CHANNEL[k].GDFTYP = 5; //int32: internal datatype 
		}
		hdr->SPR *= hdr->NRec;
		hdr->NRec = 1; 
		hdr->FILE.OPEN=2;

	}
	else {
    	 	fprintf(stderr,"ERROR: Writing of format (%c) not supported\n",hdr->TYPE);
		return(NULL); 
	}

    	hdr->FileName = FileName; 
	if(hdr->TYPE != HL7aECG){
	    	hdr = FOPEN(hdr,"wb");
		
		if (!hdr->FILE.COMPRESSION && hdr->FILE.FID == NULL ){
		     	fprintf(stderr,"ERROR: Unable to open file %s \n",FileName);
			return(NULL);
		}
#ifdef ZLIB_H	
		if (hdr->FILE.COMPRESSION && hdr->FILE.gzFID == NULL ){
		     	fprintf(stderr,"ERROR: Unable to open gzip file %s \n",FileName);
			return(NULL);
		}
#endif
		if(hdr->TYPE != SCP_ECG){
			FWRITE(hdr->AS.Header1, sizeof(char), hdr->HeadLen, hdr);
		}	

		hdr->FILE.OPEN = 2;
		hdr->FILE.POS  = 0;
	}

}	// end of else 

	// internal variables
	hdr->AS.bi = (uint32_t*) realloc(hdr->AS.bi,(hdr->NS+1)*sizeof(uint32_t));
	hdr->AS.bi[0] = 0;
	for (k=0, hdr->SPR = 1, hdr->AS.spb=0, hdr->AS.bpb=0; k<hdr->NS; k++) {
		hdr->AS.spb += hdr->CHANNEL[k].SPR;
		hdr->AS.bpb += GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP]*hdr->CHANNEL[k].SPR;			
		hdr->AS.bi[k+1] = hdr->AS.bpb; 
		if (hdr->CHANNEL[k].SPR>0)  // ignore sparse channels
			hdr->SPR = lcm(hdr->SPR, hdr->CHANNEL[k].SPR);
	}	
	return(hdr);
}  // end of SOPEN 


/****************************************************************************/
/**	SREAD                                                              **/
/****************************************************************************/
size_t sread(HDRTYPE* hdr, size_t start, size_t length) {
/* 
 *	reads LENGTH blocks with HDR.AS.bpb BYTES each, 
 *	rawdata is available in hdr->AS.rawdata
 *      output data is available in hdr->data.block
 *	size of output data is availabe in hdr->data.size
 *
 *	channel selection is controlled by hdr->CHANNEL[k}.OnOff
 * 
 *        start <0: read from current position
 *             >=0: start reading from position start
 *        length  : try to read length blocks
 *
 */

	size_t			count,k1,k2,k3,k4,k5,DIV,SZ,nelem; 
	int 			GDFTYP;
	uint8_t*		ptr;
	CHANNEL_TYPE*		CHptr;
	int32_t			int32_value;
	biosig_data_type 	sample_value; 
	

	if (hdr->TYPE != SCP_ECG && hdr->TYPE != HL7aECG) {	
		// check reading segment 
		if (start >= 0) {
			if (start > hdr->NRec) 
				return(0);
			else if (FSEEK(hdr, start*hdr->AS.bpb + hdr->HeadLen, SEEK_SET)<0)
				return(0);
			hdr->FILE.POS = start; 	
		}

		// allocate AS.rawdata 	
		hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata, (hdr->AS.bpb)*length);

		// limit reading to end of data block
		nelem = max(min(length, hdr->NRec - hdr->FILE.POS),0);

		// read data	
		count = FREAD(hdr->AS.rawdata, hdr->AS.bpb, nelem, hdr);
		if (count<nelem)
			fprintf(stderr,"warning: only %i instead of %i blocks read - something went wrong\n",count,nelem); 
	}
	else 	{  // SCP_ECG & HL7aECG format 
		// hdr->AS.rawdata was defined in SOPEN	
		count = hdr->NRec;
	}
	
	// set position of file handle 
	hdr->FILE.POS += count;

	// transfer RAW into BIOSIG data format 
	hdr->data.block   = (biosig_data_type*) realloc(hdr->data.block, (hdr->SPR) * count * (hdr->NS) * sizeof(biosig_data_type));

	for (k1=0,k2=0; k1<hdr->NS; k1++) {
		CHptr 	= hdr->CHANNEL+k1;
	//(CHptr->OnOff != 0) 
	if (hdr->CHANNEL[k1].SPR==0)
	{	// sparsly sampled channels are stored in event table
		for (k5 = 0; k5 < hdr->SPR*hdr->NRec; k5++)
			hdr->data.block[k2*count*hdr->SPR + k5] = NaN;
	}
	else 
	{
		DIV 	= hdr->SPR/CHptr->SPR; 
		GDFTYP 	= CHptr->GDFTYP;
		SZ  	= GDFTYP_BYTE[GDFTYP];
		int32_value = 0; 

		for (k4 = 0; k4 < count; k4++)
		for (k5 = 0; k5 < CHptr->SPR; k5++) {

			// get source address 	
			ptr = hdr->AS.rawdata + k4*hdr->AS.bpb + hdr->AS.bi[k1] + k5*SZ;
			
			// mapping of raw data type to (biosig_data_type)
			if (0); 
			else if ((GDFTYP==3) && (hdr->TYPE==HL7aECG))
				/* no endian conversion needed */
				sample_value = (biosig_data_type)(int16_t)(*(int16_t*)ptr); 
			else if (GDFTYP==3)
				sample_value = (biosig_data_type)l_endian_i16(*(int16_t*)ptr); 
			else if (GDFTYP==4)
				sample_value = (biosig_data_type)l_endian_u16(*(uint16_t*)ptr); 
			else if (GDFTYP==16) 
				sample_value = (biosig_data_type)(l_endian_f32(*(float*)(ptr)));
			else if (GDFTYP==17) 
				sample_value = (biosig_data_type)(l_endian_f64(*(double*)(ptr))); 
			else if (GDFTYP==0)
				sample_value = (biosig_data_type)(*(char*)ptr); 
			else if (GDFTYP==1)
				sample_value = (biosig_data_type)(*(int8_t*)ptr); 
			else if (GDFTYP==2)
				sample_value = (biosig_data_type)(*(uint8_t*)ptr); 
			else if ((GDFTYP==5) && (hdr->TYPE==HL7aECG))
				sample_value = (biosig_data_type)(int32_t)(*(int32_t*)ptr); 
			else if (GDFTYP==5)
				sample_value = (biosig_data_type)l_endian_i32(*(int32_t*)ptr); 
			else if (GDFTYP==6)
				sample_value = (biosig_data_type)l_endian_u32(*(uint32_t*)ptr); 
			else if (GDFTYP==7)
				sample_value = (biosig_data_type)l_endian_i64(*(int64_t*)ptr); 
			else if (GDFTYP==8)
				sample_value = (biosig_data_type)l_endian_u64(*(uint64_t*)ptr); 
			else if (GDFTYP==255+24) {
				int32_value = (*(uint8_t*)(ptr)) + (*(uint8_t*)(ptr+1)<<8) + (*(int8_t*)(ptr+2)*(1<<16)); 
				sample_value = (biosig_data_type)int32_value; 
			}	
			else if (GDFTYP==511+24) {
				int32_value = (*(uint8_t*)(ptr)) + (*(uint8_t*)(ptr+1)<<8) + (*(uint8_t*)(ptr+2)<<16); 
				sample_value = (biosig_data_type)int32_value; 
			}	
			else {
				fprintf(stderr,"Error SREAD: datatype %i not supported\n",GDFTYP);
				exit(-1);
			}

			// overflow and saturation detection 
			if ((hdr->FLAG.OVERFLOWDETECTION) && ((sample_value<=hdr->CHANNEL[k1].DigMin) || (sample_value>=hdr->CHANNEL[k1].DigMax)))
				sample_value = NaN; 	// missing value 
			else if (!hdr->FLAG.UCAL)	// scaling 
				sample_value = sample_value * CHptr->Cal + CHptr->Off;
			// resampling 1->DIV samples
			for (k3=0; k3 < DIV; k3++) 
				hdr->data.block[k2*count*hdr->SPR + k4*CHptr->SPR + k5 + k3] = sample_value; 
		}
		k2++;
	}}
	hdr->data.size[0] = hdr->SPR*count;	// rows	
	hdr->data.size[1] = k2;			// columns 

	return(count);

}  // end of SREAD 



/****************************************************************************/
/**	SREAD2                                                             **/
/****************************************************************************/
size_t sread2(biosig_data_type** channels_dest, size_t start, size_t length, HDRTYPE* hdr) {
/* 
 *      no memory allocation is done 
 *      data is moved into channel_dest
 *	size of output data is availabe in hdr->data.size
 *
 *	channel selection is controlled by hdr->CHANNEL[k}.OnOff
 * 
 *      start <0: read from current position
 *           >=0: start reading from position start
 *      length  : try to read length blocks
 *
 */

	size_t			count,k1,k2,k3,k4,k5,DIV,SZ,nelem; 
	int 			GDFTYP;
	uint8_t*		ptr;
	CHANNEL_TYPE*		CHptr;
	int32_t			int32_value;
	biosig_data_type 	sample_value; 
	

	// check reading segment 
	if (start >= 0) {
		if (start > hdr->NRec)
			return(0);
		else if (FSEEK(hdr, start*hdr->AS.bpb + hdr->HeadLen, SEEK_SET))
			return(0);
		hdr->FILE.POS = start; 	
	}

	if (hdr->TYPE != SCP_ECG) {	
		// allocate AS.rawdata 	
		hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata, (hdr->AS.bpb)*length);

		// limit reading to end of data block
		nelem = max(min(length, hdr->NRec - hdr->FILE.POS),0);

		// read data
		count = FREAD(hdr->AS.rawdata, hdr->AS.bpb, nelem, hdr);
		if (count<nelem)
			fprintf(stderr,"warning: only %i instead of %i blocks read - something went wrong\n",count,nelem); 
	}
	else 	{  // SCP format 
		// hdr->AS.rawdata was defined in SOPEN	
		count = hdr->NRec;
	}
	

	// set position of file handle 
	hdr->FILE.POS += count;

	// transfer RAW into BIOSIG data format 
	// hdr->data.block   = realloc(hdr->data.block, (hdr->SPR) * count * (hdr->NS) * sizeof(biosig_data_type));

	for (k1=0,k2=0; k1<hdr->NS; k1++) {
	CHptr 	= hdr->CHANNEL+k1;
	if (CHptr->OnOff != 0) {
		DIV 	= hdr->SPR/CHptr->SPR; 
		GDFTYP 	= CHptr->GDFTYP;
		SZ  	= GDFTYP_BYTE[GDFTYP];
		int32_value = 0; 

		for (k4 = 0; k4 < count; k4++)
		for (k5 = 0; k5 < CHptr->SPR; k5++) {

			// get source address 	
			ptr = hdr->AS.rawdata + k4*hdr->AS.bpb + hdr->AS.bi[k1] + k5*SZ;
			
			// mapping of raw data type to (biosig_data_type)
			if (0); 
			else if (GDFTYP==3)
				sample_value = (biosig_data_type)l_endian_i16(*(int16_t*)ptr); 
			else if (GDFTYP==4)
				sample_value = (biosig_data_type)l_endian_u16(*(uint16_t*)ptr); 
			else if (GDFTYP==16) 
				sample_value = (biosig_data_type)(l_endian_f32(*(float*)(ptr)));
			else if (GDFTYP==17) 
				sample_value = (biosig_data_type)(l_endian_f64(*(double*)(ptr))); 
			else if (GDFTYP==0)
				sample_value = (biosig_data_type)(*(char*)ptr); 
			else if (GDFTYP==1)
				sample_value = (biosig_data_type)(*(int8_t*)ptr); 
			else if (GDFTYP==2)
				sample_value = (biosig_data_type)(*(uint8_t*)ptr); 
			else if (GDFTYP==5)
				sample_value = (biosig_data_type)l_endian_i32(*(int32_t*)ptr); 
			else if (GDFTYP==6)
				sample_value = (biosig_data_type)l_endian_u32(*(uint32_t*)ptr); 
			else if (GDFTYP==7)
				sample_value = (biosig_data_type)l_endian_i64(*(int64_t*)ptr); 
			else if (GDFTYP==8)
				sample_value = (biosig_data_type)l_endian_u64(*(uint64_t*)ptr); 
			else if (GDFTYP==255+24) {
				int32_value = (*ptr) + (*(ptr+1)<<8) + (*(ptr+2)*(1<<16)); 
				sample_value = (biosig_data_type)int32_value; 
			}	
			else if (GDFTYP==511+24) {
				int32_value = (*ptr) + (*(ptr+1)<<8) + (*(ptr+2)<<16); 
				sample_value = (biosig_data_type)int32_value; 
			}	
			else {
				fprintf(stderr,"Error SREAD: datatype %i not supported\n",GDFTYP);
				exit(-1);
			}

			// overflow and saturation detection 
			if ((hdr->FLAG.OVERFLOWDETECTION) && ((sample_value<=hdr->CHANNEL[k1].DigMin) || (sample_value>=hdr->CHANNEL[k1].DigMax)))
				sample_value = NaN; 	// missing value 

			else if (!(hdr->FLAG.UCAL))	// scaling 
				sample_value = sample_value * CHptr->Cal + CHptr->Off;

			// resampling 1->DIV samples
			for (k3=0; k3 < DIV; k3++) 
				*(channels_dest[k2] + k4*CHptr->SPR + k5 + k3) = sample_value; 
				//hdr->data.block[k2*count*hdr->SPR + k4*CHptr->SPR + k5 + k3] = sample_value; 
		}
		k2++;
	}}
	hdr->data.size[0] = hdr->SPR*count;	// rows	
	hdr->data.size[1] = k2;			// columns 

	return(count);

}  // end of SREAD2 


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


	if (hdr->TYPE != SCP_ECG)  // memory allocation for SCP is done in SOPEN_SCP_WRITE Section 6	
	{
		ptr = realloc(hdr->AS.rawdata, hdr->AS.bpb*hdr->NRec);
		if (ptr==NULL) {
			fprintf(stderr,"memory allocation failed .\n");
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

			// get source address 	
			ptr = hdr->AS.rawdata + k4*hdr->AS.bpb + hdr->AS.bi[k1] + k5*SZ;

			// mapping of raw data type to (biosig_data_type)
			if (0); 

			else if (GDFTYP==3) {
				if      (sample_value > MAX_INT16) val.i16 = MAX_INT16;
				else if (sample_value < MIN_INT16) val.i16 = MIN_INT16;
				else     val.i16 = (int16_t) sample_value;
				if (hdr->TYPE==HL7aECG)
					*(int16_t*)ptr = val.i16; 
				else	
					*(int16_t*)ptr = l_endian_i16(val.i16); 
			}

			else if (GDFTYP==4) {
				if      (sample_value > MAX_UINT16) val.u16 = MAX_UINT16;
				else if (sample_value < MIN_UINT16) val.u16 = MIN_UINT16;
				else     val.u16 = (uint16_t) sample_value;
				*(uint16_t*)ptr = l_endian_u16(val.u16); 
			}

			else if (GDFTYP==16) 
				*(float*)ptr  = l_endian_f32((float)sample_value); 

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
				if (hdr->TYPE==HL7aECG)
					*(int32_t*)ptr = val.i32; 
				else	
					*(int32_t*)ptr = l_endian_i16(val.i32); 
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
				fprintf(stderr,"Error SWRITE: datatype %i not supported\n", GDFTYP);
				exit(-1);
			}
		}
		}
	}
	}	

	if (hdr->TYPE != SCP_ECG  && hdr->TYPE != HL7aECG) {
		// for SCP: writing to file is done in SCLOSE	
		count = FWRITE((uint8_t*)(hdr->AS.rawdata), hdr->AS.bpb, hdr->NRec, hdr);
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
	size_t pos=0; 
	
	if    	(whence < 0) 
		pos = offset * hdr->AS.bpb; 
	else if (whence == 0) 
		pos = (hdr->FILE.POS + offset) * hdr->AS.bpb;
	else if (whence > 0) 
		pos = (hdr->NRec + offset) * hdr->AS.bpb;
	
	if ((pos < 0) | (pos > hdr->NRec * hdr->AS.bpb))
		return(-1);
	else if (FSEEK(hdr, pos + hdr->HeadLen, SEEK_SET))
		return(-1);

	hdr->FILE.POS = pos / (hdr->AS.bpb); 	
	return(0);
	
}  // end of SSEEK


/****************************************************************************/
/**                     STELL                                              **/
/****************************************************************************/
long int stell(HDRTYPE* hdr)
{
	size_t pos; 
	
	pos = FTELL(hdr);	
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
	uint32_t	k32u; 
	uint8_t 	buf[88]; 
	char tmp[88]; 
	char flag; 
	
//	if ((hdr->NRec<0) & (hdr->FILE.OPEN>1))
	if ((hdr->FILE.OPEN>1) & ((hdr->TYPE==GDF) | (hdr->TYPE==EDF) | (hdr->TYPE==BDF)))
	{
		// WRITE HDR.NRec 
		pos = (FTELL(hdr)-hdr->HeadLen); 
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
			FSEEK(hdr,236,SEEK_SET); 
			FWRITE(tmp,len,1,hdr);
		}
	
		// WRITE EVENTTABLE 
		if ((hdr->TYPE==GDF) & (hdr->EVENT.N>0)) {
			FSEEK(hdr, hdr->HeadLen + hdr->AS.bpb*hdr->NRec, SEEK_SET); 
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
			FWRITE(buf, 8, 1, hdr);
			FWRITE(hdr->EVENT.POS, sizeof(*hdr->EVENT.POS), hdr->EVENT.N, hdr);
			FWRITE(hdr->EVENT.TYP, sizeof(*hdr->EVENT.TYP), hdr->EVENT.N, hdr);
			if (buf[0]>1) {
				for (k32u=0; k32u<hdr->EVENT.N; k32u++) {
					hdr->EVENT.DUR[k32u] = l_endian_u32(hdr->EVENT.DUR[k32u]); 
					hdr->EVENT.CHN[k32u] = l_endian_u16(hdr->EVENT.	CHN[k32u]); 
				}
				FWRITE(hdr->EVENT.CHN, sizeof(*hdr->EVENT.CHN), hdr->EVENT.N,hdr);
				FWRITE(hdr->EVENT.DUR, sizeof(*hdr->EVENT.DUR), hdr->EVENT.N,hdr);
			}	
		}
	}		
	else if ((hdr->FILE.OPEN>1) & (hdr->TYPE==SCP_ECG))
	{
		uint16_t 	crc; 
		uint8_t*	ptr; 	// pointer to memory mapping of the file layout

		if (hdr->aECG->Section5.Length>0) {
			// compute CRC for Section 5
			uint16_t crc = CRCEvaluate(hdr->AS.Header1 + hdr->aECG->Section5.StartPtr+2,hdr->aECG->Section5.Length-2); // compute CRC
			*(uint16_t*)(hdr->AS.Header1 + hdr->aECG->Section5.StartPtr) = l_endian_u16(crc);
		}	
		if (hdr->aECG->Section6.Length>0) {
			// compute CRC for Section 6
			uint16_t crc = CRCEvaluate(hdr->AS.Header1 + hdr->aECG->Section6.StartPtr+2,hdr->aECG->Section6.Length-2); // compute CRC
			*(uint16_t*)(hdr->AS.Header1 + hdr->aECG->Section6.StartPtr) = l_endian_u16(crc);
		}	
		// compute crc and len and write to preamble 
		ptr = hdr->AS.Header1; 
		*(uint32_t*)(ptr+2) = l_endian_u32(hdr->HeadLen); 
		crc = CRCEvaluate(ptr+2,hdr->HeadLen-2); 
		*(int16_t*)ptr      = l_endian_u16(crc);
		FWRITE(hdr->AS.Header1, sizeof(char), hdr->HeadLen, hdr);
	}		
	else if ((hdr->FILE.OPEN>1) && (hdr->TYPE==HL7aECG))
	{
		hdr = sclose_HL7aECG_write(hdr);
		hdr->FILE.OPEN = 0; 
	}

	if (hdr->FILE.OPEN > 0) {
		fprintf(stdout,"attempt to close file\n");
		int status = FCLOSE(hdr);
		if (status) fprintf(stderr,"biosig.c:sclose Error closing file\n");
    		hdr->FILE.FID = 0;
    	}	

    	if (hdr->aECG != NULL)	
        	free(hdr->aECG);

    	if ((hdr->AS.rawdata != NULL) && (hdr->TYPE != SCP_ECG)) 
    	{	// for SCP: hdr->AS.rawdata is part of hdr.AS.Header1 
        	free(hdr->AS.rawdata);
        }	

    	if (hdr->data.block != NULL) {	
        	free(hdr->data.block);
        	hdr->data.size[0]=0;
        	hdr->data.size[1]=0;
        }	
// fprintf(stdout,"sclose: 05\n");
    	if (hdr->CHANNEL != NULL)	
        	free(hdr->CHANNEL);
// fprintf(stdout,"sclose: 06\n");
    	if (hdr->AS.bi != NULL)	
        	free(hdr->AS.bi);
// fprintf(stdout,"sclose: 07\n");
    	if (hdr->AS.Header1 != NULL)	
        	free(hdr->AS.Header1);

// fprintf(stdout,"sclose: 08\n");
    	if (hdr->EVENT.POS != NULL)	
        	free(hdr->EVENT.POS);
    	if (hdr->EVENT.TYP != NULL)	
        	free(hdr->EVENT.TYP);
    	if (hdr->EVENT.DUR != NULL)	
        	free(hdr->EVENT.DUR);
    	if (hdr->EVENT.CHN != NULL)	
        	free(hdr->EVENT.CHN);
// fprintf(stdout,"sclose: 09\n");
        	
        hdr->EVENT.N   = 0; 
	hdr->FILE.OPEN = 0; 	     	
// fprintf(stdout,"sclose: 10\n");

    	return(0);
}


/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/

// big-endian platforms
