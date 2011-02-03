/*

    $Id$
    Copyright (C) 2005,2006,2007,2008,2009,2010,2011 Alois Schloegl <a.schloegl@ieee.org>
    This file is part of the "BioSig for C/C++" repository
    (biosig4c++) at http://biosig.sf.net/


    BioSig is free software; you can redistribute it and/or
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


/* TODO: ensure that hdr->CHANNEL[.].TOffset gets initialized after very alloc() */

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <locale.h>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "biosig-dev.h"
#include "biosig-network.h"

#ifdef _WIN32
  extern int getlogin_r(char* name, size_t namesize);
  #include <winsock2.h>
  #define FILESEP '\\'
#else
  #include <netdb.h>
  #include <unistd.h>
  #define FILESEP '/'
#endif

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

int errnum;
int B4C_STATUS  = 0;
int B4C_ERRNUM  = 0;
const char *B4C_ERRMSG;

#ifndef VERBOSE_LEVEL
int VERBOSE_LEVEL = 0;
#endif

#define HARDCODED_PHYSDIMTABLE

#ifdef __cplusplus
extern "C" {
#endif

int sopen_SCP_read     (HDRTYPE* hdr);
int sopen_SCP_write    (HDRTYPE* hdr);
int sopen_HL7aECG_read (HDRTYPE* hdr);
void sopen_HL7aECG_write(HDRTYPE* hdr);
int sopen_alpha_read   (HDRTYPE* hdr);
int sopen_FAMOS_read   (HDRTYPE* hdr);
int sclose_HL7aECG_write(HDRTYPE* hdr);
int sopen_trc_read   (HDRTYPE* hdr);
int sopen_unipro_read   (HDRTYPE* hdr);
int sopen_eeprobe(HDRTYPE* hdr);
int sopen_fef_read(HDRTYPE* hdr);
int sclose_fef_read(HDRTYPE* hdr);
int sopen_zzztest(HDRTYPE* hdr);
#ifdef WITH_DICOM
int sopen_dicom_read(HDRTYPE* hdr);
#endif
#ifdef __cplusplus
}
#endif



const int16_t GDFTYP_BITS[] = {
	8, 8, 8,16,16,32,32,64,64,32,64, 0, 0, 0, 0, 0,   /* 0  */
	32,64,128,0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 16 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 32 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 48 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 64 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	16,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 128: EEG1100 coder,  */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 8, 0,10, 0,12, 0, 0, 0,16,    /* 256 - 271*/
	0, 0, 0, 0, 0, 0, 0,24, 0, 0, 0, 0, 0, 0, 0,32,    /* 255+24 = bit24, 3 byte */
	0, 0, 0, 0, 0, 0, 0,40, 0, 0, 0, 0, 0, 0, 0,48,
	0, 0, 0, 0, 0, 0, 0,56, 0, 0, 0, 0, 0, 0, 0,64,
	0, 0, 0, 0, 0, 0, 0,72, 0, 0, 0, 0, 0, 0, 0,80,
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
	0, 0, 0, 0, 0, 0, 0, 8, 0,10, 0,12, 0, 0, 0,16,    /* 512 - 527*/
	0, 0, 0, 0, 0, 0, 0,24, 0, 0, 0, 0, 0, 0, 0,32,
	0, 0, 0, 0, 0, 0, 0,40, 0, 0, 0, 0, 0, 0, 0,48,
	0, 0, 0, 0, 0, 0, 0,56, 0, 0, 0, 0, 0, 0, 0,64,
	0, 0, 0, 0, 0, 0, 0,72, 0, 0, 0, 0, 0, 0, 0,80 };

char *gdftyp_string[] = {
	"char","int8","uint8","int16","uint16","int32","uint32","int64","uint64",
	"","","","","","","","float32","float64","float128"
	};


const char *LEAD_ID_TABLE[] = { "unspecified",
	"I","II","V1","V2","V3","V4","V5","V6",
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


/*
        This information was obtained from here:
        http://www.physionet.org/physiotools/wfdb/lib/ecgcodes.h
*/
const char *MIT_EVENT_DESC[] = {
        "normal beat",
        "left bundle branch block beat",
        "right bundle branch block beat",
        "aberrated atrial premature beat",
        "premature ventricular contraction",
        "fusion of ventricular and normal beat",
        "nodal (junctional) premature beat",
        "atrial premature contraction",
        "premature or ectopic supraventricular beat",
        "ventricular escape beat",
        "nodal (junctional) escape beat",
        "paced beat",
        "unclassifiable beat",
        "signal quality change",
        "condition 15",
        "isolated QRS-like artifact",
        "condition 17",
        "ST change",
        "T-wave change",
        "systole",
        "diastole",
        "comment annotation",
        "measurement annotation",
        "P-wave peak",
        "left or right bundle branch block",
        "non-conducted pacer spike",
        "T-wave peak",
        "rhythm change",
        "U-wave peak",
        "learning",
        "ventricular flutter wave",
        "start of ventricular flutter/fibrillation",
        "end of ventricular flutter/fibrillation",
        "atrial escape beat",
        "supraventricular escape beat",
        "link to external data (aux contains URL)",
        "non-conducted P-wave (blocked APB)",
        "fusion of paced and normal beat",
        "PQ junction (beginning of QRS)",
        "J point (end of QRS)",
        "R-on-T premature ventricular contraction",
        "condition 42",
        "condition 43",
        "condition 44",
        "condition 45",
        "condition 46",
        "condition 47",
        "condition 48",
        "not-QRS (not a getann/putann code)",        // code = 0 is mapped to 49(ACMAX)
        ""};

/* --------------------------------------------------- *
 *	Global Event Code Table                        *
 * --------------------------------------------------- */
#ifndef HARDCODED_PHYSDIMTABLE
static char GLOBAL_PHYSDIMIDX_ISLOADED = 0;
#else
#define GLOBAL_PHYSDIMIDX_ISLOADED 1
#endif
static char GLOBAL_EVENTCODES_ISLOADED = 0;
struct global_t {
	uint16_t LenCodeDesc;
	uint16_t *CodeIndex;
	char 	 **CodeDesc;
	char  	 *EventCodesTextBuffer;

#ifndef HARDCODED_PHYSDIMTABLE
	uint16_t LenPhysDimCode;
	uint16_t *PhysDimCode;
	char 	 **PhysDim;
	char	 *PhysUnitsTextBuffer;
#endif
} Global;

#ifdef HARDCODED_EVENTTABLE
struct etd_t {
        uint16_t typ;
        char*   desc;
} ETD[];


struct etd_t ETD[] = { 
#include "eventcodes.i"
	0, NULL 
};

#endif


/****************************************************************************/
/**                                                                        **/
/**                      INTERNAL FUNCTIONS                                **/
/**                                                                        **/
/****************************************************************************/

// greatest common divisor
uint32_t gcd(uint32_t A, uint32_t B)
{	size_t t;
	if (A<B) {t=B; B=A; A=t;};
	while (B>0) {
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
		fprintf(stderr,"Error: HDR.SPR=LCM(%u,%u) overflows and does not fit into uint32.\n",A,B);
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
	} b1;
	b1.f32 = x;
	b1.u32 = bswap_32(b1.u32);
	return(b1.f32);
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
	for (char k=0; k<4; k++) {
		o<<=8;
		o += *(i+k);
	}
	return(*(float*)(&o));
}
double bef64p(uint8_t* i) {
	// decode big endian double pointer
	uint64_t o=0;
	for (char k=0; k<8; k++) {
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
		fprintf(stdout,"swap=%i %i %i \nlen=%i %2x%2x%2x%2x%2x%2x%2x%2x\n",(int)FLAG_SWAP, __BYTE_ORDER, __LITTLE_ENDIAN, len, buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);

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
		for (k=len; k < sizeof(iType); buf[k++]=0) {};
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

	if (num==ceil(num))
		sprintf(buf,"%d",(int)num);
	else
		sprintf(buf,"%f",num);

	f1 = atof(buf);
	buf[8] = 0; 	// truncate
	f2 = atof(buf);

	return (fabs(f1-f2) > (fabs(f1)+fabs(f2)) * 1e-6);
}

int is_nihonkohden_signature(char *str) {
  return (!(
	strncmp(str, "EEG-1100A V01.00", 16) &&
	strncmp(str, "EEG-1100B V01.00", 16) &&
	strncmp(str, "EEG-1100C V01.00", 16) &&
	strncmp(str, "QI-403A   V01.00", 16) &&
  	strncmp(str, "QI-403A   V02.00", 16) &&
  	strncmp(str, "EEG-2100  V01.00", 16) &&
  	strncmp(str, "EEG-2100  V02.00", 16) &&
  	strncmp(str, "DAE-2100D V01.30", 16) &&
  	strncmp(str, "DAE-2100D V02.00", 16)
  ));
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

#ifdef HARDCODED_PHYSDIMTABLE
const struct PhysDimIdx
	{
		const uint16_t	idx;
		const char*	PhysDimDesc;
	} _physdim[] = {
	{ 0 ,  "?" },
	{ 512 ,  "-" },
	{ 544 ,  "%" },
	{ 576 ,  "ppht" },
	{ 608 ,  "ppm" },
	{ 640 ,  "ppmd" },
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
	{ 6048 ,  "\xB0\x43" },	//degree C
	{ 4416 ,  "\xB0\x46" }, //degree F
	{ 4448 ,  "K W-1" },
	{ 4480 ,  "cd" },
	{ 4512 ,  "osmole" },
	{ 4544 ,  "mol" },
	{ 4544 ,  "Mol" },
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
	{ 6016 ,  "dyne s m-2 cm-5" },
	{ 6176 ,  "mmHg %-1" },
	{ 6208 ,  "Pa %-1" },
	{ 6432 ,  "B" },
	{ 6624 ,  "m s-2"},             // acceleration
	{ 6656 ,  "rad s-2"},           // angular acceleration
	{65148 ,  "\xb0/m" },
	{65184 ,  "m/m" },
	{65216 ,  "km/h" },
	{65248 ,  "g s-2" },
	{65280 ,  "g s-3" },
	{65312 ,  "mHg s-1" },
	{65344 ,  "mol l-1 mm"}, 	// "light path length","milli(Mol/Liter)*millimeter"
	{65376 ,  "r.p.m"}, 		// "rotations per minute"
	{65408 ,  "B"}, 		/* obsolete, use 6432 instead */
	{65440 ,  "dyne s m2 cm-5" },
	{65472 ,  "l m-2" },
	{65504 ,  "T" },
	{0xffff,  "end-of-table" },
} ;
#endif

/*
	compare first n characters of two strings, ignore case
 */
int strncmpi(const char* str1, const char* str2, size_t n)
{
	unsigned int k=0;
	int r=0;
	while (!r && str1[k] && str2[k] && (k<n)) {
		r = tolower(str1[k]) - tolower(str2[k]);
		k++;
	}
	return(r);
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

/*
	compare strings, accept bit7=1
 */
int strcmp8(const char* str1, const char* str2)
{
	unsigned int k=0;
	int r;
	r = str1[k] - str2[k];
	while (!r && str1[k] && str2[k]) {
		k++;
		r = str1[k] - str2[k];
	}
	return(r);
}

/*
	compare uint32_t
*/
int u32cmp(const void *a,const void *b)
{
	return(*(uint32_t*)a - *(uint32_t*)b);
}


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

#ifndef HARDCODED_PHYSDIMTABLE
void LoadGlobalPhysDimCodeTable();	// this functions is defined below
#endif

char* PhysDim(uint16_t PhysDimCode, char *PhysDim)
{
	// converting PhysDimCode -> PhysDim

#ifndef HARDCODED_PHYSDIMTABLE
	LoadGlobalPhysDimCodeTable();	// make sure Global Code table is loaded
#endif
	strcpy(PhysDim,PhysDimFactor[PhysDimCode & 0x001F]);

	PhysDimCode &= ~0x001F;
	uint16_t k;
#ifdef HARDCODED_PHYSDIMTABLE
	for (k=0; _physdim[k].idx<0xffff; k++)
	if (PhysDimCode == _physdim[k].idx) {
		strncat(PhysDim, _physdim[k].PhysDimDesc, MAX_LENGTH_PHYSDIM);
#else
	for (k=0; Global.PhysDimCode[k]<0xffff; k++)
	if (PhysDimCode == Global.PhysDimCode[k]) {
		strncat(PhysDim, Global.PhysDim[k], MAX_LENGTH_PHYSDIM);
#endif
		PhysDim[MAX_LENGTH_PHYSDIM]=0;
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

#ifndef HARDCODED_PHYSDIMTABLE
	LoadGlobalPhysDimCodeTable();	// make sure Global Code table is loaded
#endif
	uint16_t k1, k2;
	char s[80];
	char *s1;

	// greedy search - check all codes 0..65535
	for (k1=0; k1<33; k1++)
	if (!strncmp(PhysDimFactor[k1],PhysDim0,strlen(PhysDimFactor[k1])) && (PhysDimScale(k1)>0.0))
	{ 	// exclude if beginning of PhysDim0 differs from PhysDimFactor and if NaN
		strcpy(s, PhysDimFactor[k1]);
		s1 = s+strlen(s);
#ifdef HARDCODED_PHYSDIMTABLE
		for (k2=0; _physdim[k2].idx < 0xffff; k2++) {
			strcpy(s1, _physdim[k2].PhysDimDesc);
#else
		for (k2=0; Global.PhysDimCode[k2] < 0xffff; k2++) {
			strcpy(s1, Global.PhysDim[k2]);
#endif
			if (!strcmp8(PhysDim0, s)) {
		 		if (k1==32) k1 = 19;		// hack for "Âµ" = "u"
#ifdef HARDCODED_PHYSDIMTABLE
				return(_physdim[k2].idx+k1);
#else
				return(Global.PhysDimCode[k2]+k1);
#endif
			}
		}
	}
	return(0);
}

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

gdf_time tm_time2gdf_time(struct tm *t){
	/* based Octave's datevec.m
	it referes Peter Baum's algorithm at http://vsg.cape.com/~pbaum/date/date0.htm
	but the link is not working anymore as of 2008-12-03.

	Other links to Peter Baum's algorithm are
	http://www.rexswain.com/b2mmddyy.rex
	http://www.dpwr.net/forums/index.php?s=ecfa72e38be61327403126e23aeea7e5&showtopic=4309
	*/

	int Y,M,s; //h,m,
	double D;
	gdf_time o;

	D = t->tm_mday;
	M = t->tm_mon+1;
	Y = t->tm_year+1900;

	// Set start of year to March by moving Jan. and Feb. to previous year.
  	// Correct for months > 12 by moving to subsequent years.
  	Y += fix ((M-14.0)/12);

  	const int monthstart[] = {306, 337, 0, 31, 61, 92, 122, 153, 184, 214, 245, 275};
	// Lookup number of days since start of the current year.
  	D += monthstart[t->tm_mon % 12] + 60;

	// Add number of days to the start of the current year. Correct
  	// for leap year every 4 years except centuries not divisible by 400.
  	D += 365*Y + floor (Y/4) - floor (Y/100) + floor (Y/400);

  	// Add fraction representing current second of the day.
  	s = t->tm_hour*3600 + t->tm_min*60 + t->tm_sec;

	// s -= timezone;
  	o = (((int64_t)D) << 32) + (((int64_t)s) << 32)/86400;

	return(o);
}

struct tm *gdf_time2tm_time(gdf_time t) {
	/* based Octave's datevec.m
	it referes Peter Baum's algorithm at http://vsg.cape.com/~pbaum/date/date0.htm
	but the link is not working anymore as of 2008-12-03.

	Other links to Peter Baum's algorithm are
	http://www.rexswain.com/b2mmddyy.rex
	http://www.dpwr.net/forums/index.php?s=ecfa72e38be61327403126e23aeea7e5&showtopic=4309
	*/

	static struct tm tt;	// allocate memory for t3;
	struct tm *t3 = &tt;
	int32_t rd = (int32_t)floor(ldexp((double)t,-32)); // days since 0001-01-01


	/* derived from datenum.m from Octave 3.0.0 */

	// Move day 0 from midnight -0001-12-31 to midnight 0000-3-1
	double z = floor (rd) - 60;
	// Calculate number of centuries; K1 = 0.25 is to avoid rounding problems.
	double a = floor ((z - 0.25) / 36524.25);
	// Days within century; K2 = 0.25 is to avoid rounding problems.
	double b = z - 0.25 + a - floor (a / 4);
	// Calculate the year (year starts on March 1).
	int y = (int)floor (b / 365.25);
    	// Calculate day in year.
	double c = fix (b - floor (365.25 * y)) + 1;
	// Calculate month in year.
	double m = fix ((5 * c + 456) / 153);
	double d = c - fix ((153 * m - 457) / 5);

	// Move to Jan 1 as start of year.
	if (m>12) {y++; m-=12;}
	t3->tm_year = y-1900;
	t3->tm_mon  = (int)m-1;
	t3->tm_mday = (int)d;

	double s = ldexp((t & 0x00000000ffffffff)*86400,-32); // seconds of the day
	// s += timezone;

	t3->tm_hour = (int)(floor (s / 3600));
	s = s - 3600 * t3->tm_hour;
	t3->tm_min = (int)(floor (s / 60));
	t3->tm_sec = (int)(s) - 60 * t3->tm_min;
	//t3->tm_gmtoff = 3600;

    	return(t3);
}


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
	hdr->FILE.FID = fopen(hdr->FileName, mode);
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
	return(gzprintf(hdr->FILE.gzFID, format, arg));
	else
#endif
	return(fprintf(hdr->FILE.FID, format, arg));
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

int ifsetpos(HDRTYPE* hdr, size_t *pos) {
#if __sparc__ || __APPLE__ || __MINGW32__
	fpos_t p = *pos;
#else
	fpos_t p;
	p.__pos = *pos;
#endif

#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION) {
		gzseek(hdr->FILE.gzFID,*pos,SEEK_SET);
		size_t pos1 = *pos;
		*pos = gztell(hdr->FILE.gzFID);
		return(*pos - pos1);
	}
	else
#endif
	{
	int c= fsetpos(hdr->FILE.FID,&p);
#if __sparc__ || __APPLE__ || __MINGW32__
	*pos = p;
#else
	*pos = p.__pos;
#endif
	return(c);
	}
}

int ifgetpos(HDRTYPE* hdr, size_t *pos) {
#ifdef ZLIB_H
	if (hdr->FILE.COMPRESSION) {
		z_off_t p = gztell(hdr->FILE.gzFID);
		if (p<0) return(-1);
		else {
			*pos = p;
			return(0);
		}
	} else
#endif
	{
		fpos_t p;
		int c = fgetpos(hdr->FILE.FID, &p);
#if __sparc__ || __APPLE__ || __MINGW32__
		*pos = p;
#else
		*pos = p.__pos;	// ugly hack but working
#endif
		return(c);
	}
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
	sort event table according to EVENT.POS
  ------------------------------------------------------------------------*/
typedef struct {
	uint32_t POS;
	uint32_t DUR;
	uint16_t TYP;
	uint16_t CHN;
}  entry_t;

int compare_eventpos(const void *e1, const void *e2) {
	return(((entry_t*)(e1))->POS - ((entry_t*)(e2))->POS);
}

void sort_eventtable(HDRTYPE *hdr) {
	size_t k;
	entry_t *entry = (entry_t*) calloc(hdr->EVENT.N,sizeof(entry_t));
	if ((hdr->EVENT.DUR != NULL) && (hdr->EVENT.CHN != NULL))
	for (k=0; k<hdr->EVENT.N;k++) {
		entry[k].TYP = hdr->EVENT.TYP[k];
		entry[k].POS = hdr->EVENT.POS[k];
		entry[k].CHN = hdr->EVENT.CHN[k];
		entry[k].DUR = hdr->EVENT.DUR[k];
	}
	else
	for (k=0; k<hdr->EVENT.N;k++) {
		entry[k].TYP = hdr->EVENT.TYP[k];
		entry[k].POS = hdr->EVENT.POS[k];
	}

	qsort(entry,hdr->EVENT.N,sizeof(entry_t),&compare_eventpos);

	if ((hdr->EVENT.DUR != NULL) && (hdr->EVENT.CHN != NULL))
	for (k=0; k<hdr->EVENT.N;k++) {
		hdr->EVENT.TYP[k] = entry[k].TYP;
		hdr->EVENT.POS[k] = entry[k].POS;
		hdr->EVENT.CHN[k] = entry[k].CHN;
		hdr->EVENT.DUR[k] = entry[k].DUR;
	}
	else
	for (k=0; k<hdr->EVENT.N;k++) {
		hdr->EVENT.TYP[k] = entry[k].TYP;
		hdr->EVENT.POS[k] = entry[k].POS;
	}
	free(entry);
}

/*------------------------------------------------------------------------
	converts event table from {TYP,POS} to [TYP,POS,CHN,DUR} format
  ------------------------------------------------------------------------*/
void convert2to4_eventtable(HDRTYPE *hdr) {
	size_t k1,k2,N=hdr->EVENT.N;

	sort_eventtable(hdr);

	if (hdr->EVENT.DUR == NULL)
		hdr->EVENT.DUR = (typeof(hdr->EVENT.DUR)) calloc(N,sizeof(*hdr->EVENT.DUR));
	if (hdr->EVENT.CHN == NULL)
		hdr->EVENT.CHN = (typeof(hdr->EVENT.CHN)) calloc(N,sizeof(*hdr->EVENT.CHN));

	for (k1=0; k1<N; k1++) {
		typeof(*hdr->EVENT.TYP) typ =  hdr->EVENT.TYP[k1];
		if ((typ < 0x8000) && (typ>0)  && !hdr->EVENT.DUR[k1])
		for (k2 = k1+1; k2<N; k2++) {
			if ((typ|0x8000) == hdr->EVENT.TYP[k2]) {
				hdr->EVENT.DUR[k1] = hdr->EVENT.POS[k2] - hdr->EVENT.POS[k1];
				hdr->EVENT.TYP[k2] = 0;
				break;
			}
		}
	}
	for (k1=0,k2=0; k1<N; k1++) {
		if (k2!=k1) {
			hdr->EVENT.TYP[k2]=hdr->EVENT.TYP[k1];
			hdr->EVENT.POS[k2]=hdr->EVENT.POS[k1];
			hdr->EVENT.DUR[k2]=hdr->EVENT.DUR[k1];
			hdr->EVENT.CHN[k2]=hdr->EVENT.CHN[k1];
		}
		if (hdr->EVENT.TYP[k1]) k2++;
	}
	hdr->EVENT.N = k2;
}
/*------------------------------------------------------------------------
	converts event table from [TYP,POS,CHN,DUR} to {TYP,POS} format
  ------------------------------------------------------------------------*/
void convert4to2_eventtable(HDRTYPE *hdr) {
	if ((hdr->EVENT.DUR == NULL) || (hdr->EVENT.CHN == NULL)) return;

	size_t k1,k2,N = hdr->EVENT.N;
	for (k1=0; k1<N; k1++)
		if (hdr->EVENT.CHN[k1]) return;

	hdr->EVENT.TYP = (typeof(hdr->EVENT.TYP)) realloc(hdr->EVENT.TYP,2*N*sizeof(*hdr->EVENT.TYP));
	hdr->EVENT.POS = (typeof(hdr->EVENT.POS)) realloc(hdr->EVENT.POS,2*N*sizeof(*hdr->EVENT.POS));

	for (k1=0,k2=N; k1<N; k1++)
		if (hdr->EVENT.DUR[k1]) {
			hdr->EVENT.TYP[k2] = hdr->EVENT.TYP[k1] | 0x8000;
			hdr->EVENT.POS[k2] = hdr->EVENT.POS[k1] + hdr->EVENT.DUR[k1];
			k2++;
		}
	hdr->EVENT.N = k2;

	free(hdr->EVENT.CHN); hdr->EVENT.CHN=NULL;
	free(hdr->EVENT.DUR); hdr->EVENT.DUR=NULL;
	sort_eventtable(hdr);
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


fprintf(stdout,"write_gdf_eventtable is obsolete - use hdrEVT2rawEVT instead;\n");

	ifseek(hdr, hdr->HeadLen + hdr->AS.bpb*hdr->NRec, SEEK_SET);
	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"WriteEventTable: %p %p %p %p\t",hdr->EVENT.TYP,hdr->EVENT.POS,hdr->EVENT.DUR,hdr->EVENT.CHN);
	flag = (hdr->EVENT.DUR != NULL) && (hdr->EVENT.CHN != NULL);
	if (flag)   // any DUR or CHN is larger than 0
		for (k32u=0, flag=0; (k32u<hdr->EVENT.N) && !flag; k32u++)
			flag |= hdr->EVENT.CHN[k32u] || hdr->EVENT.DUR[k32u];

	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"flag=%d.\n",flag);

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
	if (flag) {
		for (k32u=0; k32u<hdr->EVENT.N; k32u++) {
			hdr->EVENT.DUR[k32u] = l_endian_u32(hdr->EVENT.DUR[k32u]);
			hdr->EVENT.CHN[k32u] = l_endian_u16(hdr->EVENT.CHN[k32u]);
		}
		ifwrite(hdr->EVENT.CHN, sizeof(*hdr->EVENT.CHN), hdr->EVENT.N,hdr);
		ifwrite(hdr->EVENT.DUR, sizeof(*hdr->EVENT.DUR), hdr->EVENT.N,hdr);
	}
}

/*------------------------------------------------------------------------
	Load Table of Event Codes
  ------------------------------------------------------------------------*/
void FreeGlobalEventCodeTable() {
//	if (VERBOSE_LEVEL>8) fprintf(stdout,"FreeGlobalEventTable(start)%i\n",GLOBAL_EVENTCODES_ISLOADED);

	Global.LenCodeDesc = 0;
	if (Global.EventCodesTextBuffer) free(Global.EventCodesTextBuffer);
	Global.EventCodesTextBuffer = NULL;
	if (Global.CodeDesc) free(Global.CodeDesc);
	Global.CodeDesc = NULL;
	if (Global.CodeIndex) free(Global.CodeIndex);
	Global.CodeIndex = NULL;

	GLOBAL_EVENTCODES_ISLOADED = 0;
//	if (VERBOSE_LEVEL>8) fprintf(stdout,"FreeGlobalEventTable(end)%i\n",GLOBAL_EVENTCODES_ISLOADED);
}

void LoadGlobalEventCodeTable()
{

	if (VERBOSE_LEVEL>8) fprintf(stdout,"LoadGlobalEventTable(start)%i\n",GLOBAL_EVENTCODES_ISLOADED);

	if (GLOBAL_EVENTCODES_ISLOADED) return; 	// table is already loaded

	Global.CodeDesc = NULL;
	Global.CodeIndex = NULL;
	Global.EventCodesTextBuffer = NULL;
	Global.LenCodeDesc = 0;

	if (VERBOSE_LEVEL>8) fprintf(stdout,"LoadGlobalEventTable(101)%i %i\n",GLOBAL_EVENTCODES_ISLOADED,Global.LenCodeDesc);

#ifdef HARDCODED_EVENTTABLE

	size_t N = 0;
	while (ETD[N].desc != NULL) N++;
	N += 256; 	// make sure there is enough space for free text events 
	Global.CodeIndex = (uint16_t*)realloc(Global.CodeIndex, N*sizeof(uint16_t));
	Global.CodeDesc  = (char**)realloc(Global.CodeDesc, N*sizeof(char*));

	while (ETD[Global.LenCodeDesc].desc != NULL) {
		Global.CodeDesc[Global.LenCodeDesc]  = ETD[Global.LenCodeDesc].desc;
		Global.CodeIndex[Global.LenCodeDesc] = ETD[Global.LenCodeDesc].typ;
		Global.LenCodeDesc++;
	}

#else
	HDRTYPE HDR;
	HDR.FileName = "eventcodes.txt";
	HDR.FILE.COMPRESSION = 0;
	ifopen(&HDR,"r");

	if (VERBOSE_LEVEL>8) fprintf(stdout,"LoadGlobalEventTable(102) OPEN=%i %i\n",HDR.FILE.OPEN,Global.LenCodeDesc);

	if (!HDR.FILE.OPEN) {
		fprintf(stdout,"table of event codes not found\n");
		return;	// event code table not available
	}

	if (VERBOSE_LEVEL>8) fprintf(stdout,"LoadGlobalEventTable(102) OPEN=%i %i\n",HDR.FILE.OPEN,Global.LenCodeDesc);

	atexit(&FreeGlobalEventCodeTable);	// make sure memory is freed

	if (VERBOSE_LEVEL>8) fprintf(stdout,"LoadGlobalEventTable(102) OPEN=%i %i\n",HDR.FILE.OPEN,Global.LenCodeDesc);

	size_t count = 0;
	if (Global.EventCodesTextBuffer==NULL)
	while (!ifeof(&HDR)) {
		const size_t bufsiz = 8092;
		Global.EventCodesTextBuffer = (char*)realloc(Global.EventCodesTextBuffer,count+bufsiz+1);
		count  += ifread(Global.EventCodesTextBuffer+count,1,bufsiz,&HDR);
	}
	ifclose(&HDR);
	Global.EventCodesTextBuffer[count]=0;	// terminating /0 character

	if (VERBOSE_LEVEL>8) fprintf(stdout,"LoadGlobalEventTable(103) count=%i\n",count);

	size_t N = 0;
	char *line = strtok(Global.EventCodesTextBuffer,"\x0a\x0d");
	while ((line != NULL) && (strlen(line)>0)) {
//		if (VERBOSE_LEVEL>8) fprintf(stdout,"%i <%s>\n",Global.LenCodeDesc, line);

		if (line[0]=='0') {
			size_t k;
			int i;
			for (k=6; isspace(line[k]); k++) {};
			char *t = line+k;
			sscanf(line,"%x",&i);
			i &= 0xffff;

			if (N<=Global.LenCodeDesc) {
				N += 256;
				Global.CodeIndex = (uint16_t*)realloc(Global.CodeIndex,N*sizeof(uint16_t));
				Global.CodeDesc = (char**)realloc(Global.CodeDesc,N*sizeof(char*));

				if (VERBOSE_LEVEL>8) fprintf(stdout,"++++++++ %i %i %i %i\n",k,i,N,Global.LenCodeDesc);
			}

			for (k=0; (k<Global.LenCodeDesc) && (Global.CodeIndex[k]!=i); k++) {};

			if (k<Global.LenCodeDesc) {
				fprintf(stdout,"Warning: Event Type %x is already defined (error in eventcodes.txt)\n",i);
			}
			else {
				Global.CodeDesc[Global.LenCodeDesc]  = t;
				Global.CodeIndex[Global.LenCodeDesc] = i;
				Global.LenCodeDesc++;
			}
//			if (VERBOSE_LEVEL>8) fprintf(stdout,"%x <%s>\n",i,t);
		}
		line = strtok(NULL,"\x0a\x0d");
	}

#endif 

	GLOBAL_EVENTCODES_ISLOADED = 1;
	if (VERBOSE_LEVEL>7) fprintf(stdout,"LoadGlobalEventTable(end)%i\n",Global.LenCodeDesc);
}

#ifndef HARDCODED_PHYSDIMTABLE
void FreeGlobalPhysDimCodeTable()
{
	Global.LenPhysDimCode = 0;
	if (Global.PhysDimCode) free(Global.PhysDimCode);
	Global.PhysDimCode = NULL;
	if (Global.PhysDim) free(Global.PhysDim);
	Global.PhysDim = NULL;
	if (Global.PhysUnitsTextBuffer) free(Global.PhysUnitsTextBuffer);
	Global.PhysUnitsTextBuffer = NULL;

	GLOBAL_PHYSDIMIDX_ISLOADED = 0;
}

void LoadGlobalPhysDimCodeTable()
{

	if (GLOBAL_PHYSDIMIDX_ISLOADED) return; 	// table is already loaded

	Global.LenPhysDimCode = 0;
	Global.PhysDimCode = NULL;
	Global.PhysDim = NULL;
	Global.PhysUnitsTextBuffer = NULL;

	if (VERBOSE_LEVEL>8) fprintf(stdout,"LoadGlobalPhysDimCodeTable(101)%i %i\n",GLOBAL_PHYSDIMIDX_ISLOADED,Global.LenCodeDesc);

	HDRTYPE HDR;
	HDR.FileName = "units.csv";
	ifopen(&HDR,"r");
	if (!HDR.FILE.OPEN) return;	// event code table not available

	if (VERBOSE_LEVEL>8) fprintf(stdout,"LoadGlobalEventTable(102) OPEN=%i %i\n",HDR.FILE.OPEN,Global.LenCodeDesc);

	atexit(&FreeGlobalPhysDimCodeTable);	// make sure memory is freed
	size_t count = 0;
	if (Global.PhysUnitsTextBuffer==NULL)
	while (!ifeof(&HDR)) {
		size_t bufsiz = 8092;
		Global.PhysUnitsTextBuffer = (char*)realloc(Global.PhysUnitsTextBuffer,(count+bufsiz+25));
		count  += ifread(Global.PhysUnitsTextBuffer+count,1,bufsiz,&HDR);
	}
	ifclose(&HDR);
	strcpy(Global.PhysUnitsTextBuffer+count,"\n65535,\"end-of-table\"\n");	// add terminating entry

	size_t N = 0;
	char *line = strtok(Global.PhysUnitsTextBuffer,"\x0a\x0d");
	while ((line != NULL) && (strlen(line)>0)) {
		if (VERBOSE_LEVEL>8) fprintf(stdout,"%i <%s>\n",Global.LenPhysDimCode, line);

		if (isdigit(line[0])) {
			int i;
			char *tok;
			size_t k;
			for (k=0; isdigit(line[k]); k++);
			if ((line[k]==',') && (line[k+1]=='\"') ) {
				line[k] = 0;
				line[k+1] = 0;
				i = atoi(line);
				tok = line+k+2;
				*(strchr(tok,'\"'))=0;
				line = strchr(tok,'[');
				if (line)
				do {
					line[0] = 0;
					--line;
				} while (isspace(line[0]));
			}
			else
				fprintf(stdout,"Warning: unable to decode line <%s>  (error in units.csv)\n",line);

			i &= 0xffff;

			if (N<=Global.LenPhysDimCode) {
				N += 256;
				Global.PhysDimCode = (uint16_t*)realloc(Global.PhysDimCode,N*sizeof(uint16_t));
				Global.PhysDim = (char**)realloc(Global.PhysDim,N*sizeof(char*));

				if (VERBOSE_LEVEL>8) fprintf(stdout,"++++++++ %i %i %i %i\n",k,i,N,Global.LenPhysDimCode);
			}

			for (k=0; (k<Global.LenPhysDimCode) || (Global.PhysDimCode[k]==i); k++);
			if (Global.PhysDimCode[k]==i) {
				fprintf(stdout,"Warning: PhysDimCode %u is already defined (error in units.csv)\n",i);
			}
			else {
				Global.PhysDim[Global.LenPhysDimCode]  = tok;
				Global.PhysDimCode[Global.LenPhysDimCode] = i;
				Global.LenPhysDimCode++;
			}
			if (VERBOSE_LEVEL>8) fprintf(stdout,"%x <%s>\n",i,tok);
		}
		line = strtok(NULL,"\x0a\x0d");
	}
	GLOBAL_PHYSDIMIDX_ISLOADED = 1;
	if (VERBOSE_LEVEL>7) fprintf(stdout,"LoadGlobalEventTable(end)%i\n",Global.LenPhysDimCode);

}
#endif

/*------------------------------------------------------------------------
	adds free text annotation to event table
	the EVENT.TYP is identified from the table EVENT.CodeDesc
	if annotations is not listed in CodeDesc, it is added to CodeDesc
	The table is limited to 256 entries, because the table EventCodes
	allows only codes 0-255 as user specific entry.
  ------------------------------------------------------------------------*/
void FreeTextEvent(HDRTYPE* hdr,size_t N_EVENT, char* annotation) {
	/* free text annotations encoded as user specific events (codes 1-255) */

	size_t k;
//	static int LengthCodeDesc = 0;
	if (hdr->EVENT.CodeDesc == NULL) {
		hdr->EVENT.CodeDesc = (typeof(hdr->EVENT.CodeDesc)) realloc(hdr->EVENT.CodeDesc,257*sizeof(*hdr->EVENT.CodeDesc));
		hdr->EVENT.CodeDesc[0] = "";	// typ==0, is always empty
		hdr->EVENT.LenCodeDesc = 1;
	}

	// First, compare text with any global event description
	if (!GLOBAL_EVENTCODES_ISLOADED) LoadGlobalEventCodeTable();
	for (k=0; k<Global.LenCodeDesc; k++) {
		if (Global.CodeIndex[k]>255) {
			// compare description only up last non-space character before '#'
			int len = strcspn(Global.CodeDesc[k],"#");
			while ((len>=0) && isspace(Global.CodeDesc[k][len-1])) len--;
			if (!strncmp(Global.CodeDesc[k],annotation,len)) {
				// annotation is already a globally defined event
				hdr->EVENT.TYP[N_EVENT] = Global.CodeIndex[k];
				return;
			}
		}
	}

	// Second, compare text with user-defined event description
	int flag=1;
	for (k=0; (k < hdr->EVENT.LenCodeDesc) && flag; k++) {
		if (!strncmp(hdr->EVENT.CodeDesc[k], annotation, strlen(annotation))) {
			hdr->EVENT.TYP[N_EVENT] = k;
			flag = 0;
		}
	}

	// Third, add event description if needed
	if (flag) {
		hdr->EVENT.TYP[N_EVENT] = hdr->EVENT.LenCodeDesc;
		hdr->EVENT.CodeDesc[hdr->EVENT.LenCodeDesc] = annotation;
		hdr->EVENT.LenCodeDesc++;
	}
	if (hdr->EVENT.LenCodeDesc > 255) {
		B4C_ERRNUM = B4C_INSUFFICIENT_MEMORY;
		B4C_ERRMSG = "Maximum number of user-defined events (256) exceeded";
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
	uint8_t	LittleEndian = (EndianTest.testbyte[0]==0x1d);

	if  ((LittleEndian) && (__BYTE_ORDER == __BIG_ENDIAN))	{
		B4C_ERRNUM = B4C_ENDIAN_PROBLEM;
		B4C_ERRMSG = "Panic: mixed results on Endianity test.";
		exit(-1);
	}
	if  ((!LittleEndian) && (__BYTE_ORDER == __LITTLE_ENDIAN))	{
		B4C_ERRNUM = B4C_ENDIAN_PROBLEM;
		B4C_ERRMSG = "Panic: mixed results on Endianity test.";
		exit(-1);
	}

	hdr->FILE.OPEN = 0;
	hdr->FILE.FID = 0;
    	hdr->FILE.POS = 0;
	hdr->FILE.Des = 0;
	hdr->FILE.COMPRESSION = 0;
#ifdef ZLIB_H
	hdr->FILE.gzFID = 0;
#endif

	hdr->AS.Header = NULL;
	hdr->AS.rawEventData = NULL;
	hdr->AS.auxBUF = NULL;
	hdr->AS.bpb    = 0;

      	hdr->TYPE = noFile;
      	hdr->VERSION = 2.0;
      	hdr->AS.rawdata = NULL; //(uint8_t*) malloc(0);
	hdr->AS.flag_collapsed_rawdata = 0;	// is rawdata not collapsed
	hdr->AS.first = 0;
	hdr->AS.length  = 0;  // no data loaded
	hdr->Calib = NULL;
        hdr->rerefCHANNEL = NULL;

      	hdr->NRec = 0;
      	hdr->NS = NS;
	hdr->SampleRate = 4321.5;
      	hdr->Patient.Id[0]=0;
      	strcpy(hdr->ID.Recording,"00000000");
	hdr->data.size[0] = 0; 	// rows
	hdr->data.size[1] = 0;  // columns
	hdr->data.block = (biosig_data_type*)malloc(0);
      	hdr->T0 = (gdf_time)0;
      	hdr->tzmin = 0;
      	hdr->ID.Equipment = *(uint64_t*) & "b4c_0.95";
      	hdr->ID.Manufacturer._field[0]    = 0;
      	hdr->ID.Manufacturer.Name         = NULL;
      	hdr->ID.Manufacturer.Model        = NULL;
      	hdr->ID.Manufacturer.Version      = NULL;
      	hdr->ID.Manufacturer.SerialNumber = NULL;
	hdr->ID.Technician[0] 	= 0;
	hdr->ID.Hospital 	= "\x00";
	memset(hdr->IPaddr, 0, 16);

      	// set default technician name to local IP address
	getlogin_r(hdr->ID.Technician, MAX_LENGTH_TECHNICIAN);
	//hdr->ID.Technician[MAX_LENGTH_TECHNICIAN]=0;

      	// set default IP address to local IP address
	char localhostname[HOST_NAME_MAX+1];

#ifndef WITHOUT_NETWORK
#ifdef _WIN32
	WSADATA wsadata;
	WSAStartup(MAKEWORD(1,1), &wsadata);
#endif
	if (!gethostname(localhostname,HOST_NAME_MAX+1)) {
		// TODO: replace gethostbyname by getaddrinfo (for IPv6)
		struct hostent *host = gethostbyname(localhostname);
		memcpy(hdr->IPaddr, host->h_addr, host->h_length);
	}
#ifdef _WIN32
	WSACleanup();
#endif
#endif // not WITHOUT_NETWORK

	hdr->Patient.Name[0] 	= 0;
	//hdr->Patient.Id[0] 	= 0;
	hdr->Patient.Birthday 	= (gdf_time)0;        // Unknown;
      	hdr->Patient.Medication = 0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.DrugAbuse 	= 0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.AlcoholAbuse=0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.Smoking 	= 0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.Sex 	= 0;	// 0:Unknown, 1: Male, 2: Female
      	hdr->Patient.Handedness = 0;	// 0:Unknown, 1: Right, 2: Left, 3: Equal
      	hdr->Patient.Impairment.Visual = 0;	// 0:Unknown, 1: NO, 2: YES, 3: Corrected
      	hdr->Patient.Impairment.Heart  = 0;	// 0:Unknown, 1: NO, 2: YES, 3: Pacemaker
      	hdr->Patient.Weight 	= 0;	// 0:Unknown
      	hdr->Patient.Height 	= 0;	// 0:Unknown

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
	hdr->FLAG.ANONYMOUS = 1; 	// <>0: no personal names are processed
	hdr->FLAG.TARGETSEGMENT = 1;   // read 1st segment
	hdr->FLAG.CNT32 = 0; 		// assume 16-bit CNT format

       	// define variable header
	hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	size_t BitsPerBlock = 0;
	for (k=0;k<hdr->NS;k++)	{
		CHANNEL_TYPE *hc = hdr->CHANNEL+k;
	      	hc->Label[0]  = 0;
	      	hc->LeadIdCode= 0;
	      	strcpy(hc->Transducer, "EEG: Ag-AgCl electrodes");
	      	hc->PhysDimCode = 19+4256; // uV
	      	hc->PhysMax   = +100;
	      	hc->PhysMin   = -100;
	      	hc->DigMax    = +2047;
	      	hc->DigMin    = -2048;
	      	hc->Cal	      = NaN;
	      	hc->Off	      = 0.0;
	      	hc->TOffset   = 0.0;
	      	hc->GDFTYP    = 3;	// int16
	      	hc->SPR       = 1;	// one sample per block
	      	hc->bi 	      = 2*k;
	      	hc->bi8	      = BitsPerBlock;
		uint32_t nbits 	= GDFTYP_BITS[hc->GDFTYP]*hc->SPR;
		BitsPerBlock   += nbits;
	      	hc->OnOff     = 1;
	      	hc->HighPass  = 0.16;
	      	hc->LowPass   = 70.0;
	      	hc->Notch     = 50;
	      	hc->Impedance = INF;
	      	hc->fZ        = NaN;
	      	hc->XYZ[0] 	= 0.0;
	      	hc->XYZ[1] 	= 0.0;
	      	hc->XYZ[2] 	= 0.0;
	}

	// define EVENT structure
	hdr->EVENT.N = N_EVENT;
	hdr->EVENT.SampleRate = 0;
	hdr->EVENT.CodeDesc = NULL;
	hdr->EVENT.LenCodeDesc = 0;
	if (hdr->EVENT.N) {
		hdr->EVENT.POS = (uint32_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.POS));
		hdr->EVENT.TYP = (uint16_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.TYP));
		hdr->EVENT.DUR = (uint32_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.DUR));
		hdr->EVENT.CHN = (uint16_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.CHN));
	} else {
		hdr->EVENT.POS = NULL;
		hdr->EVENT.TYP = NULL;
		hdr->EVENT.DUR = NULL;
		hdr->EVENT.CHN = NULL;
	}

	// initialize specialized fields
	hdr->aECG = NULL;
	hdr->AS.bci2000 = NULL;

	return(hdr);
}

/* just for debugging
void debug_showptr(HDRTYPE* hdr) {
	fprintf(stdout,"=========================\n");
	fprintf(stdout,"&AS.Header=%p\n",hdr->AS.Header);
	fprintf(stdout,"&AS.auxBUF=%p\n",hdr->AS.auxBUF);
	fprintf(stdout,"&aECG=%p\n",hdr->aECG);
	fprintf(stdout,"&AS.bci2000=%p\n",hdr->AS.bci2000);
	fprintf(stdout,"&AS.rawEventData=%p\n",hdr->AS.rawEventData);
	fprintf(stdout,"&AS.rawData=%p\n",hdr->AS.rawdata);
	fprintf(stdout,"&data.block=%p\n",hdr->data.block);
	fprintf(stdout,"&CHANNEL=%p\n",hdr->CHANNEL);
	fprintf(stdout,"&EVENT.POS=%p\n",hdr->EVENT.POS);
	fprintf(stdout,"&EVENT.TYP=%p\n",hdr->EVENT.TYP);
	fprintf(stdout,"&EVENT.DUR=%p\n",hdr->EVENT.DUR);
	fprintf(stdout,"&EVENT.CHN=%p\n",hdr->EVENT.CHN);
	fprintf(stdout,"&EVENT.CodeDesc=%p\n",hdr->EVENT.CodeDesc);
	fprintf(stdout,"&FileName=%p %s\n",&hdr->FileName,hdr->FileName);
	fprintf(stdout,"&Hospital=%p\n",hdr->ID.Hospital);
}
*/

void destructHDR(HDRTYPE* hdr) {

	if (hdr==NULL) return;

	sclose(hdr);

	if (VERBOSE_LEVEL>7) fprintf(stdout,"destructHDR(%s): free HDR.aECG\n",hdr->FileName);
    	if (hdr->aECG != NULL) {
		if (((aECG_TYPE*)hdr->aECG)->Section8.NumberOfStatements>0)
			free(((aECG_TYPE*)hdr->aECG)->Section8.Statements);
		if (((aECG_TYPE*)hdr->aECG)->Section11.NumberOfStatements>0)
			free(((aECG_TYPE*)hdr->aECG)->Section11.Statements);
    		free(hdr->aECG);
    	}

    	if (hdr->AS.bci2000 != NULL) free(hdr->AS.bci2000);

	if (VERBOSE_LEVEL>7)  fprintf(stdout,"destructHDR: free HDR.AS.rawdata @%p\n",hdr->AS.rawdata);

	if ((hdr->AS.rawdata != NULL) && (hdr->TYPE != SCP_ECG))
	{	// for SCP: hdr->AS.rawdata is part of hdr->AS.Header
        	free(hdr->AS.rawdata);
        }

	if (VERBOSE_LEVEL>7)  fprintf(stdout,"destructHDR: free HDR.data.block @%p\n",hdr->data.block);

    	if (hdr->data.block != NULL) free(hdr->data.block);
       	hdr->data.size[0]=0;
       	hdr->data.size[1]=0;

	if (VERBOSE_LEVEL>7)  fprintf(stdout,"destructHDR: free HDR.CHANNEL[] @%p %p\n",hdr->CHANNEL,hdr->rerefCHANNEL);

    	if (hdr->CHANNEL != NULL) free(hdr->CHANNEL);
    	hdr->CHANNEL = NULL;

	if (VERBOSE_LEVEL>7)  fprintf(stdout,"destructHDR: free HDR.AS.Header\n");

    	if (hdr->AS.rawEventData != NULL) free(hdr->AS.rawEventData);
    	hdr->AS.rawEventData = NULL;
    	if (hdr->AS.Header != NULL) free(hdr->AS.Header);
	hdr->AS.Header = NULL;

	if (VERBOSE_LEVEL>7)  fprintf(stdout,"destructHDR: free Event Table %p %p %p %p \n",hdr->EVENT.TYP,hdr->EVENT.POS,hdr->EVENT.DUR,hdr->EVENT.CHN);

    	if (hdr->EVENT.POS != NULL)  free(hdr->EVENT.POS);
    	if (hdr->EVENT.TYP != NULL)  free(hdr->EVENT.TYP);
    	if (hdr->EVENT.DUR != NULL)  free(hdr->EVENT.DUR);
    	if (hdr->EVENT.CHN != NULL)  free(hdr->EVENT.CHN);
    	if (hdr->EVENT.CodeDesc != NULL) free(hdr->EVENT.CodeDesc);

	if (VERBOSE_LEVEL>7)  fprintf(stdout,"destructHDR: free HDR.AS.auxBUF\n");

    	if (hdr->AS.auxBUF != NULL) free(hdr->AS.auxBUF);

	if (VERBOSE_LEVEL>7)  fprintf(stdout,"destructHDR: free HDR.rerefCHANNEL\n");

#ifdef CHOLMOD_H
        cholmod_common c ;
        cholmod_start (&c) ; /* start CHOLMOD */
        //if (hdr->Calib) cholmod_print_sparse(hdr->Calib,"destructHDR hdr->Calib",&c);
	if (VERBOSE_LEVEL>7)  fprintf(stdout,"destructHDR: free hdr->Calib\n");
	if (hdr->Calib) cholmod_free_sparse(&hdr->Calib,&c);
        cholmod_finish (&c) ;
	if (VERBOSE_LEVEL>7)  fprintf(stdout,"destructHDR: free hdr->rerefCHANNEL %p\n",hdr->rerefCHANNEL);
	if (hdr->rerefCHANNEL) free(hdr->rerefCHANNEL);
	hdr->rerefCHANNEL = NULL;
#endif

	if (VERBOSE_LEVEL>7)  fprintf(stdout,"destructHDR: free HDR\n");

	if (hdr != NULL) free(hdr);
	return;
}



/****************************************************************************/
/**                     GETFILETYPE                                        **/
/****************************************************************************/
HDRTYPE* getfiletype(HDRTYPE* hdr)
/*
	input:
		hdr->AS.Header1 contains first block of hdr->HeadLen bytes
		hdr->TYPE must be unknown, otherwise no FileFormat evaluation is performed
	output:
		hdr->TYPE	file format
		hdr->VERSION	is defined for some selected formats e.g. ACQ, EDF, BDF, GDF
 */
{
	// ToDo: use LEN to detect buffer overflow

    	hdr->TYPE = unknown;

	if (VERBOSE_LEVEL>7) fprintf(stdout,"[GETFILETYPE 101]!\n");

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
	const uint8_t MAGIC_NUMBER_UNIPRO[]   = {40,0,4,1,44,1,102,2,146,3,44,0,190,3};
	const char* MAGIC_NUMBER_BRAINVISION  = "Brain Vision Data Exchange Header File";
	const char* MAGIC_NUMBER_BRAINVISION1 = "Brain Vision V-Amp Data Header File Version";
	const char* MAGIC_NUMBER_BRAINVISIONMARKER = "Brain Vision Data Exchange Marker File, Version";

    	/******** read 1st (fixed)  header  *******/
  	uint32_t U32 = leu32p(hdr->AS.Header+2);
	uint32_t MAGIC_EN1064_Section0Length  = leu32p(hdr->AS.Header+10);

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

	if (VERBOSE_LEVEL>7) fprintf(stdout,"[GETFILETYPE 200] %i %i!\n",leu16p(hdr->AS.Header),leu16p(hdr->AS.Header+154));

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
    	else if (strstr(Header1,"ALPHA-TRACE-MEDICAL"))
	    	hdr->TYPE = alpha;
    	else if (!memcmp(Header1,"ATES MEDICA SOFT. EEG for Windows",33))
	    	hdr->TYPE = ATES;
    	else if (!memcmp(Header1,"ATF\x09",4))
    	        hdr->TYPE = ATF;

    	else if (!memcmp(Header1,"HeaderLen=",10)) {
	    	hdr->TYPE = BCI2000;
	    	hdr->VERSION = 1.0;
	}
    	else if (!memcmp(Header1,"BCI2000V",8)) {
	    	hdr->TYPE = BCI2000;
	    	hdr->VERSION = 1.1;
	}
    	else if (!memcmp(Header1+1,"BIOSEMI",7) && (hdr->AS.Header[0]==0xff) && (hdr->HeadLen > 255)) {
    		hdr->TYPE = BDF;
    		hdr->VERSION = -1;
    	}
    	else if (!memcmp(Header1,"#BIOSIG ASCII",13))
	    	hdr->TYPE = ASCII;
    	else if (!memcmp(Header1,"#BIOSIG BINARY",14))
	    	hdr->TYPE = BIN;
    	else if ((leu16p(hdr->AS.Header)==207) && (leu16p(hdr->AS.Header+154)==0))
	    	hdr->TYPE = BKR;
    	else if (!memcmp(Header1+34,"BLSC",4))
	    	hdr->TYPE = BLSC;
    	else if (!memcmp(Header1,"bscs://",7))
	    	hdr->TYPE = BSCS;
    	else if ((beu16p(Header1)==0x0311) && (beu32p(Header1+4)==0x0809B002)
    		 && (leu16p(Header1+2) > 240) && (leu16p(Header1+2) < 250)  		// v2.40 - v2.50
    		 || !memcmp(Header1+307, "E\x00\x00\x00\x00\x00\x00\x00DAT", 11)
    		)
	    	hdr->TYPE = BLSC;
    	else if (!memcmp(Header1,"FileFormat = BNI-1-BALTIMORE",28))
	    	hdr->TYPE = BNI;
        else if (!memcmp(Header1,MAGIC_NUMBER_BRAINVISION,38) || ((leu32p(hdr->AS.Header)==0x42bfbbef) && !memcmp(Header1+3, MAGIC_NUMBER_BRAINVISION,38)))
                hdr->TYPE = BrainVision;
        else if (!memcmp(Header1,MAGIC_NUMBER_BRAINVISION1,38))
                hdr->TYPE = BrainVisionVAmp;
        else if (!memcmp(Header1,MAGIC_NUMBER_BRAINVISIONMARKER,38))
                hdr->TYPE = BrainVisionMarker;
    	else if (!memcmp(Header1,"BZh91",5))
	    	hdr->TYPE = BZ2;
    	else if (!memcmp(Header1,"CDF",3))
	    	hdr->TYPE = CDF;
    	else if (!memcmp(Header1,"CEDFILE",7))
	    	hdr->TYPE = CFS;
    	else if (!memcmp(Header1,"CFWB\1\0\0\0",8))
	    	hdr->TYPE = CFWB;
    	else if (!memcmp(Header1,"Version 3.0",11))
	    	hdr->TYPE = CNT;

    	else if (!memcmp(Header1,"MEG4",4))
	    	hdr->TYPE = CTF;
    	else if (!memcmp(Header1,"CTF_MRI_FORMAT VER 2.2",22))
	    	hdr->TYPE = CTF;
    	else if (!memcmp(Header1,"PATH OF DATASET:",16))
	    	hdr->TYPE = CTF;

    	else if (!memcmp(Header1,"DEMG",4))
	    	hdr->TYPE = DEMG;
    	else if (!memcmp(Header1+128,"DICM\x02\x00\x00\x00",8))
	    	hdr->TYPE = DICOM;
    	else if (!memcmp(Header1, MAGIC_NUMBER_DICOM,18))
	    	hdr->TYPE = DICOM;
    	else if (!memcmp(Header1+12, MAGIC_NUMBER_DICOM,18))
	    	hdr->TYPE = DICOM;
    	else if (!memcmp(Header1+12, MAGIC_NUMBER_DICOM,8))
	    	hdr->TYPE = DICOM;

    	else if (!memcmp(Header1,"EBS\x94\x0a\x13\x1a\x0d",8))
	    	hdr->TYPE = EBS;

    	else if (!memcmp(Header1,"0       ",8) && (hdr->HeadLen > 255)) {
	    	hdr->TYPE = EDF;
	    	hdr->VERSION = 0;
	}

	/* Nihon Kohden */
	else if (is_nihonkohden_signature((char*)Header1) && is_nihonkohden_signature((char*)(Header1+0x81)))
	    	hdr->TYPE = EEG1100;

    	else if (!memcmp(Header1, "RIFF",4) && !memcmp(Header1+8, "CNT ",4))
	    	hdr->TYPE = EEProbe;
    	else if (!memcmp(Header1, "EEP V2.0",8))
	    	hdr->TYPE = EEProbe;
    	else if (!memcmp(Header1, "\x26\x00\x10\x00",4))	// AVR
	    	hdr->TYPE = EEProbe;

    	else if (( bei32p(hdr->AS.Header) == 0x01020304) &&
    		 ((beu16p(hdr->AS.Header+4) == 0xffff) || (beu16p(hdr->AS.Header+4) == 3)) )
    	{
	    	hdr->TYPE = EGIS;
	    	hdr->FILE.LittleEndian = 0;
    	}
    	else if (( lei32p(hdr->AS.Header) == 0x01020304) &&
    		 ((leu16p(hdr->AS.Header+4) == 0xffff) || (leu16p(hdr->AS.Header+4) == 3)) )
	{
	    	hdr->TYPE = EGIS;
	    	hdr->FILE.LittleEndian = 1;
    	}
    	else if ((beu32p(hdr->AS.Header) > 1) && (beu32p(hdr->AS.Header) < 8)) {
	    	hdr->TYPE = EGI;
	    	hdr->VERSION = hdr->AS.Header[3];
    	}
    	else if (*(uint32_t*)(Header1) == b_endian_u32(0x7f454c46))
	    	hdr->TYPE = ELF;
    	else if (!memcmp(Header1,"Embla data file",15))
	    	hdr->TYPE = EMBLA;
    	else if (!memcmp(Header1,"[Header]",8))
	    	hdr->TYPE = ET_MEG;
    	else if (!memcmp(Header1,"Header\r\nFile Version'",20))
	    	hdr->TYPE = ETG4000;
    	else if (!memcmp(Header1,"|CF,",4))
	    	hdr->TYPE = FAMOS;

    	else if (!memcmp(Header1,MAGIC_NUMBER_FEF1,8) || !memcmp(Header1,MAGIC_NUMBER_FEF2,8)) {
	    	hdr->TYPE = FEF;
		char tmp[9];
		strncpy(tmp,(char*)hdr->AS.Header+8,8);
		tmp[8]=0;
		hdr->VERSION = (float)atol(tmp);
    	}
    	else if (!memcmp(Header1,"fLaC",4))
	    	hdr->TYPE = FLAC;
    	else if (!memcmp(Header1,"GDF",3) && (hdr->HeadLen > 255)) {
	    	hdr->TYPE = GDF;
	    	char tmp[6];
      	    	strncpy(tmp,(char*)hdr->AS.Header+3,5); tmp[5]=0;
	    	hdr->VERSION 	= strtod(tmp,NULL);
	}
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
	else if (!memcmp(Header1,"DATA\0\0\0\0",8)) {
		hdr->TYPE = HEKA;
		hdr->VERSION = 0;
	}
	else if (!memcmp(Header1,"DAT1\0\0\0\0",8)) {
		hdr->TYPE = HEKA;
		hdr->VERSION = 1;
	}
	else if (!memcmp(Header1,"DAT2\0\0\0\0",8)) {
		hdr->TYPE = HEKA;
		hdr->VERSION = 2;
	}
    	else if (!memcmp(Header1,"IGOR",4))
	    	hdr->TYPE = ITX;
    	else if (!memcmp(Header1,"ISHNE1.0",8))
	    	hdr->TYPE = ISHNE;
    	else if (!memcmp(Header1,"@  MFER ",8))
	    	hdr->TYPE = MFER;
    	else if (!memcmp(Header1,"@ MFR ",6))
	    	hdr->TYPE = MFER;
    	else if (!memcmp(Header1,"%%MatrixMarket",14))
	    	hdr->TYPE = MM;
/*    	else if (!memcmp(Header1,"MThd\000\000\000\001\000",9))
	    	hdr->TYPE = MIDI;
*/

    	else if (!memcmp(Header1,"\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3E\x00\x03\x00\xFE\xFF\x09\x00\x06",0x21))
	    	hdr->TYPE = MSI;

    	else if ( (Header1[344]=='n') && (Header1[347]=='\0') && \
    		  ((Header1[345]=='i') || (Header1[345]=='+') ) && \
    		   (Header1[346]>'0') && (Header1[346]<='9') ) {
	    	hdr->TYPE = NIFTI;
	    	hdr->VERSION = Header1[346]-'0';
		}
    	else if (!memcmp(Header1+344,"ni1",4) || !memcmp(Header1+344,"n+1",4) )
	    	hdr->TYPE = NIFTI;
    	else if (!memcmp(Header1,"NEX1",4))
	    	hdr->TYPE = NEX1;
    	else if (!memcmp(Header1,"Neuron",6))
	    	hdr->TYPE = NEURON;
    	else if (!memcmp(Header1,"SXDF",4))
	    	hdr->TYPE = OpenXDF;
    	else if (!memcmp(Header1,"PLEX",4))
	    	hdr->TYPE = PLEXON;
    	else if (!memcmp(Header1,"\x55\xAA\x00\xb0",2)) {
	    	hdr->TYPE = RDF;	// UCSD ERPSS aquisition system
	    	hdr->FILE.LittleEndian = 1;
	}
    	else if (!memcmp(Header1,"\xAA\x55\xb0\x00",2)) {
	    	hdr->TYPE = RDF;	// UCSD ERPSS aquisition system
	    	hdr->FILE.LittleEndian = 0;
	}
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
/*
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
*/
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

	else if (!memcmp(Header1,"HEADER RECORD*******LIBRARY HEADER RECORD!!!!!!!000000000000000000000000000000",78))
		hdr->TYPE = SASXPT;	// SAS Transport file format (XPORT)
	else if (!memcmp(Header1,"$FL2@(#) SPSS DATA FILE",8)) {
		hdr->TYPE = SPSS;	// SPSS file format
		switch (*(uint32_t*)(Header1+64)) {
		case 0x00000002:
		case 0x00000003:
		    	hdr->FILE.LittleEndian = 1;
		    	break;
		case 0x02000000:
		case 0x03000000:
		    	hdr->FILE.LittleEndian = 0;
		    	break;
		}
	}
	else if ((Header1[0]==0x71 || Header1[0]==0x72) && (Header1[1]==1 || Header1[1]==2) && Header1[2]==1  && Header1[3]==0 )
		hdr->TYPE = STATA;
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
	else if (!memcmp(Header1,"FileId=TMSi PortiLab sample log file\x0a\x0dVersion=",35))
		hdr->TYPE = TMSiLOG;
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
	else if (!memcmp(hdr->AS.Header+4,MAGIC_NUMBER_UNIPRO,14))
		hdr->TYPE = UNIPRO;
	else if (!memcmp(Header1,"# vtk DataFile Version ",23)) {
		hdr->TYPE = VTK;
		char tmp[4];
		strncpy(tmp,(char*)Header1+23,3);
		hdr->VERSION = strtod(tmp,NULL);
	}
	else if (!strncmp(Header1,"Serial number",13))
		hdr->TYPE = ASCII_IBI;
	else if (!memcmp(Header1,MAGIC_NUMBER_Z,3))
		hdr->TYPE = Z;
	else if (!strncmp(Header1,"PK\003\004",4))
		hdr->TYPE = ZIP;
	else if (!strncmp(Header1,"PK\005\006",4))
		hdr->TYPE = ZIP;
	else if (!strncmp(Header1,"!<arch>\n",8))
		hdr->TYPE = MSVCLIB;
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
	else if (!memcmp(hdr->AS.Header,"V3.0            ",16) && !memcmp(hdr->AS.Header+32,"[PatInfo]",9)) {
		hdr->TYPE = Sigma;
		hdr->VERSION = 3.0;
	}
	else if ((hdr->HeadLen > 175) && (hdr->AS.Header[175] < 5))
	{	hdr->TYPE = TRC; // Micromed *.TRC format
		hdr->FILE.LittleEndian = 1;
    	}
	else if (!memcmp(hdr->AS.Header,"\x4c\x00\x00\x00\x01\x14\x02\x00\x00\x00\x00\x00\xC0\x00\x00\x00\x00\x00\x46",20))
	{	hdr->TYPE = MS_LNK; // Microsoft *.LNK format
		hdr->FILE.LittleEndian = 1;
 	}

	if (VERBOSE_LEVEL>8) fprintf(stdout,"[228] %i %s %s \n",hdr->TYPE,GetFileTypeString(hdr->TYPE),hdr->FileName);

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

	case alpha: 	{ FileType = "alpha"; break; }
	case ABF: 	{ FileType = "ABF"; break; }
	case ACQ: 	{ FileType = "ACQ"; break; }
	case ACR_NEMA: 	{ FileType = "ACR_NEMA"; break; }
	case AINF: 	{ FileType = "AINF"; break; }
	case AIFC: 	{ FileType = "AIFC"; break; }
	case AIFF: 	{ FileType = "AIFF"; break; }
	case ARFF: 	{ FileType = "ARFF"; break; }
	case ASCII: 	{ FileType = "ASCII"; break; }
	case ATES: 	{ FileType = "ATES"; break; }
	case ATF: 	{ FileType = "ATF"; break; }
	case AU: 	{ FileType = "AU"; break; }

	case BCI2000: 	{ FileType = "BCI2000"; break; }
	case BDF: 	{ FileType = "BDF"; break; }
	case BKR: 	{ FileType = "BKR"; break; }
	case BLSC: 	{ FileType = "BLSC"; break; }
	case BMP: 	{ FileType = "BMP"; break; }
	case BNI: 	{ FileType = "BNI-1-Baltimore/Nicolet"; break; }
	case BrainVision:
	case BrainVisionVAmp:
		 	{ FileType = "BrainVision"; break; }
	case BZ2: 	{ FileType = "BZ2"; break; }

	case CDF: 	{ FileType = "CDF"; break; }
	case CFS: 	{ FileType = "CFS"; break; }
	case CFWB: 	{ FileType = "CFWB"; break; }
	case CNT: 	{ FileType = "CNT"; break; }
	case CTF: 	{ FileType = "CTF"; break; }
	case DEMG: 	{ FileType = "DEMG"; break; }
	case DICOM: 	{ FileType = "DICOM"; break; }

	case EBS: 	{ FileType = "EBS"; break; }
	case EDF: 	{ FileType = "EDF"; break; }
	case EEG1100: 	{ FileType = "EEG1100"; break; }
	case EEProbe: 	{ FileType = "EEProbe"; break; }
	case EGI: 	{ FileType = "EGI"; break; }
	case EGIS: 	{ FileType = "EGIS"; break; }
	case ELF: 	{ FileType = "ELF"; break; }
	case EMBLA: 	{ FileType = "EMBLA"; break; }
	case ET_MEG: 	{ FileType = "ET-MEG"; break; }
	case ETG4000: 	{ FileType = "ETG4000"; break; }
	case EVENT: 	{ FileType = "EVENT"; break; }
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
	case HEKA: 	{ FileType = "HEKA"; break; }
	case HL7aECG: 	{ FileType = "HL7aECG"; break; }
	case ITX: 	{ FileType = "ITX"; break; }
	case ISHNE: 	{ FileType = "ISHNE"; break; }
	case JPEG: 	{ FileType = "JPEG"; break; }

	case Matlab: 	{ FileType = "MAT"; break; }
	case MFER: 	{ FileType = "MFER"; break; }
	case MIDI: 	{ FileType = "MIDI"; break; }
	case MIT: 	{ FileType = "MIT"; break; }
	case MM: 	{ FileType = "MatrixMarket"; break; }
	case MSI: 	{ FileType = "MSI"; break; }
	case MS_LNK:    { FileType = ".LNK"; break; }
 	case MSVCLIB: 	{ FileType = "MS VC++ Library"; break; }

	case native: 	{ FileType = "native"; break; }
	case NetCDF: 	{ FileType = "NetCDF"; break; }
	case NEX1: 	{ FileType = "NEX1"; break; }
	case NIFTI: 	{ FileType = "NIFTI"; break; }
	case NEURON: 	{ FileType = "NEURON"; break; }
	case OGG: 	{ FileType = "OGG"; break; }
	case PDP: 	{ FileType = "PDP"; break; }

	case RDF: 	{ FileType = "RDF"; break; }		// UCSD ERPSS aquisition system
	case RIFF: 	{ FileType = "RIFF"; break; }

	case SASXPT: 	{ FileType = "SAS_XPORT"; break; }
	case SCP_ECG: 	{ FileType = "SCP"; break; }
	case SIGIF: 	{ FileType = "SIGIF"; break; }
	case Sigma: 	{ FileType = "Sigma"; break; }		// Sigma PLpro
	case SMA: 	{ FileType = "SMA"; break; }
	case SND: 	{ FileType = "SND"; break; }
	case SPSS: 	{ FileType = "SPSS"; break; }
	case STATA: 	{ FileType = "STATA"; break; }
	case SVG: 	{ FileType = "SVG"; break; }

	case TIFF: 	{ FileType = "TIFF"; break; }
	case TMS32: 	{ FileType = "TMS32"; break; }		// Poly5+TMS32
	case TMSiLOG: 	{ FileType = "TMSiLOG"; break; }
	case TRC: 	{ FileType = "TRC"; break; }
	case UNIPRO: 	{ FileType = "UNIPRO"; break; }
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
/**                     struct2gdfbin                                      **/
/****************************************************************************/
void struct2gdfbin(HDRTYPE *hdr)
{
	size_t k;
	char tmp[81];
	uint32_t Dur[2];

	// NS number of channels selected for writing
     	typeof(hdr->NS)  NS = 0;
	for (k=0; k<hdr->NS; k++) {
		CHANNEL_TYPE *hc = hdr->CHANNEL+k;
		if (hc->OnOff) NS++;
		hc->Cal = (hc->PhysMax-hc->PhysMin)/(hc->DigMax-hc->DigMin);
		hc->Off =  hc->PhysMin-hc->Cal*hc->DigMin;
	}

 	    	hdr->HeadLen = (NS+1)*256;

	     	if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw 101 %i \n", hdr->HeadLen);

		/* experimental code: writing header 3, in Tag-Length-Value from
			currently only tag=1 is used for storing user-specific events (i.e. free text annotations
		 */
	     	uint32_t TagNLen[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	     	uint8_t tag=1;
	     	if (hdr->EVENT.LenCodeDesc > 1) {	// first entry is always empty - no need to save tag1
	     		for (k=0; k<hdr->EVENT.LenCodeDesc; k++)
		     		TagNLen[tag] += strlen(hdr->EVENT.CodeDesc[k])+1;
	     		TagNLen[tag] += 1; 			// acounts for terminating \0
	     		hdr->HeadLen += 4+TagNLen[tag];
	     	}
	     	if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw101 %i %i %i\n",tag, hdr->HeadLen,TagNLen[1]);
	     	tag = 2;
	     	if (hdr->AS.bci2000 != NULL) {
	     		TagNLen[tag] = strlen(hdr->AS.bci2000)+1;
	     		hdr->HeadLen += 4+TagNLen[tag];
	     	}
	     	if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw101 %i %i %i\n",tag, hdr->HeadLen,TagNLen[1]);
	     	tag = 3;
	     	if ((hdr->ID.Manufacturer.Name != NULL) || (hdr->ID.Manufacturer.Model != NULL) || (hdr->ID.Manufacturer.Version != NULL) || (hdr->ID.Manufacturer.SerialNumber != NULL)) {
	     		if (hdr->ID.Manufacturer.Name == NULL) hdr->ID.Manufacturer.Name="";
	     		if (hdr->ID.Manufacturer.Model == NULL) hdr->ID.Manufacturer.Model="";
	     		if (hdr->ID.Manufacturer.Version == NULL) hdr->ID.Manufacturer.Version="";
	     		if (hdr->ID.Manufacturer.SerialNumber == NULL) hdr->ID.Manufacturer.SerialNumber="";

	     		TagNLen[tag] = strlen(hdr->ID.Manufacturer.Name)+strlen(hdr->ID.Manufacturer.Model)+strlen(hdr->ID.Manufacturer.Version)+strlen(hdr->ID.Manufacturer.SerialNumber)+4;
	     		hdr->HeadLen += 4+TagNLen[tag];
	     	}
	     	if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw101 %i %i %i\n",tag, hdr->HeadLen,TagNLen[1]);
	     	tag = 4;
/* OBSOLETE
	     	char FLAG_SENSOR_ORIENTATION = 0;
	     	for (k=0; k<hdr->NS; k++) {
	     		FLAG_SENSOR_ORIENTATION |= hdr->CHANNEL[k].Orientation[0] != (float)0.0;
	     		FLAG_SENSOR_ORIENTATION |= hdr->CHANNEL[k].Orientation[1] != (float)0.0;
	     		FLAG_SENSOR_ORIENTATION |= hdr->CHANNEL[k].Orientation[2] != (float)0.0;
	     		FLAG_SENSOR_ORIENTATION |= hdr->CHANNEL[k].Area != (float)0.0;
	     	}
	     	if (FLAG_SENSOR_ORIENTATION)
	     		TagNLen[tag] = hdr->NS*sizeof(float)*4;
     		hdr->HeadLen += 4+TagNLen[tag];
*/
	     	if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw101 %i %i %i\n",tag, hdr->HeadLen,TagNLen[1]);
	     	tag = 5;
	     	for (k=0; k<16; k++) {
	     		if (hdr->IPaddr[k]) {
		     		if (k<4) TagNLen[tag] = 4;
		     		else 	 TagNLen[tag] = 16;
		     	}
	     		hdr->HeadLen += 4+TagNLen[tag];
	     	}
	     	if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw101 %i %i %i\n",tag, hdr->HeadLen,TagNLen[1]);
	     	tag = 6;
     		TagNLen[tag] = strlen(hdr->ID.Technician);
	     	if (TagNLen[tag]) {
	     		TagNLen[tag]++;
	     		hdr->HeadLen += 4+TagNLen[tag];
	     	}
	     	if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw101 %i %i %i\n",tag, hdr->HeadLen,TagNLen[1]);
	     	tag = 7;
		if (hdr->ID.Hospital!=NULL) {
	     		TagNLen[tag] = strlen(hdr->ID.Hospital);
		     	if (TagNLen[tag]) {
	     			TagNLen[tag]++;
	     			hdr->HeadLen += 4+TagNLen[tag];
	     		}
	     	}

	     	if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw101 %i %i %i\n",tag, hdr->HeadLen,TagNLen[1]);
	     	/* end */

		if (hdr->TYPE==GDF) {
			hdr->VERSION = 2.21;
			if (hdr->HeadLen & 0x00ff)	// in case of GDF v2, make HeadLen a multiple of 256.
			hdr->HeadLen = (hdr->HeadLen & 0xff00) + 256;
		}
		else if (hdr->TYPE==GDF1) {
			hdr->VERSION = 1.25;
			hdr->TYPE = GDF;
		}
		else
	     	if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw101 %i %i\n",hdr->HeadLen,TagNLen[1]);

	    	hdr->AS.Header = (uint8_t*) calloc(hdr->HeadLen,1);
	     	sprintf((char*)hdr->AS.Header,"GDF %4.2f",hdr->VERSION);
	    	uint8_t* Header2 = hdr->AS.Header+256;

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
	     		Header1[87] = (hdr->Patient.Sex%4) + ((hdr->Patient.Handedness%4)<<2) + ((hdr->Patient.Impairment.Visual%4)<<4) + ((hdr->Patient.Impairment.Heart%4)<<6);
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
    			struct tm *t = gdf_time2tm_time(hdr->T0);

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

	/* FIXME: this part should make the records as small as possible
		size_t DIV = 1, div;
		for (k=0; k<hdr->NS; k++) {
			div = hdr->SPR/hdr->CHANNEL[k].SPR;
			if (div>DIV) DIV=div;
		}
		for (k=0; k<hdr->NS; k++) {
			hdr->CHANNEL[k].SPR = (hdr->CHANNEL[k].SPR*DIV)/hdr->SPR;
		}
		hdr->NRec *= hdr->SPR/DIV;
		hdr->SPR  = DIV;
	*/

		double fDur = hdr->SPR/hdr->SampleRate;
		if (hdr->VERSION < 2.21) {
			/* Duration is expressed as an fraction of integers */
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

			*(uint32_t*) (Header1+244) = l_endian_u32(Dur[0]);
			*(uint32_t*) (Header1+248) = l_endian_u32(Dur[1]);
		}
		else
			*(double*) (Header1+244) = l_endian_f64(fDur);

		*(uint16_t*) (Header1+252) = l_endian_u16(NS);

	     	/* define HDR.Header2
	     	this requires checking the arguments in the fields of the struct HDR.CHANNEL
	     	and filling in the bytes in HDR.Header2.
	     	*/
		typeof(k) k2=0;
		for (k=0; k<hdr->NS; k++)
		if (hdr->CHANNEL[k].OnOff)
		{
			const char *tmpstr;
			if (hdr->CHANNEL[k].LeadIdCode)
				tmpstr = LEAD_ID_TABLE[hdr->CHANNEL[k].LeadIdCode];
			else
				tmpstr = hdr->CHANNEL[k].Label;

		     	len = strlen(tmpstr);
		     	memcpy(Header2+16*k2,tmpstr,min(len,16));

		     	len = strlen(hdr->CHANNEL[k].Transducer);
		     	memcpy(Header2+80*k2 + 16*NS, hdr->CHANNEL[k].Transducer, min(len,80));
		     	PhysDim(hdr->CHANNEL[k].PhysDimCode, tmp);
		     	len = strlen(tmp);
		     	if (hdr->VERSION < 1.9)
		     		memcpy(Header2+ 8*k2 + 96*NS, tmp, min(8,len));
		     	else {
		     		memcpy(Header2+ 6*k2 + 96*NS, tmp, min(6,len));
		     		*(uint16_t*)(Header2+ 2*k2 + 102*NS) = l_endian_u16(hdr->CHANNEL[k].PhysDimCode);
			};

		     	*(double*)(Header2 + 8*k2 + 104*NS)   = l_endian_f64(hdr->CHANNEL[k].PhysMin);
		     	*(double*)(Header2 + 8*k2 + 112*NS)   = l_endian_f64(hdr->CHANNEL[k].PhysMax);
		     	if (hdr->VERSION < 1.9) {
			     	*(int64_t*)(Header2 + 8*k2 + 120*NS)   = l_endian_i64((int64_t)hdr->CHANNEL[k].DigMin);
			     	*(int64_t*)(Header2 + 8*k2 + 128*NS)   = l_endian_i64((int64_t)hdr->CHANNEL[k].DigMax);
			     	// FIXME // memcpy(Header2 + 80*k + 136*hdr->NS,hdr->CHANNEL[k].PreFilt,max(80,strlen(hdr->CHANNEL[k].PreFilt)));
			}
			else {
			     	*(double*)(Header2 + 8*k2 + 120*NS) = l_endian_f64(hdr->CHANNEL[k].DigMin);
			     	*(double*)(Header2 + 8*k2 + 128*NS) = l_endian_f64(hdr->CHANNEL[k].DigMax);
			     	*(float*) (Header2 + 4*k2 + 204*NS) = l_endian_f32(hdr->CHANNEL[k].LowPass);
			     	*(float*) (Header2 + 4*k2 + 208*NS) = l_endian_f32(hdr->CHANNEL[k].HighPass);
			     	*(float*) (Header2 + 4*k2 + 212*NS) = l_endian_f32(hdr->CHANNEL[k].Notch);

				*(float*) (Header2 + 4*k2 + 224*NS) = l_endian_f32(hdr->CHANNEL[k].XYZ[0]);
				*(float*) (Header2 + 4*k2 + 228*NS) = l_endian_f32(hdr->CHANNEL[k].XYZ[1]);
				*(float*) (Header2 + 4*k2 + 232*NS) = l_endian_f32(hdr->CHANNEL[k].XYZ[2]);

        		     	if (hdr->VERSION < (float)2.19)
       	     				Header2[k2+236*NS] = (uint8_t)ceil(log10(min(39e8,hdr->CHANNEL[k].Impedance))/log10(2.0)*8.0-0.5);

        		     	else switch (hdr->CHANNEL[k].PhysDimCode & 0xFFE0) {
        		     	        // context-specific header 2 area
        		     	        case 4256:
        	     				*(float*)(Header2+236*NS+20*k2) = (float)hdr->CHANNEL[k].Impedance;
        	     				break;
        		     	        case 4288:
        	     				*(float*)(Header2+236*NS+20*k2) = (float)hdr->CHANNEL[k].fZ;
        	     				break;
        	     			// default:        // reserved area
                                }
		     	}
		     	*(uint32_t*) (Header2 + 4*k2 + 216*NS) = l_endian_u32(hdr->CHANNEL[k].SPR);
		     	*(uint32_t*) (Header2 + 4*k2 + 220*NS) = l_endian_u32(hdr->CHANNEL[k].GDFTYP);
		     	k2++;
		}

		if (errno==34) errno = 0; // reset numerical overflow error

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"GDFw 444 %i %s\n", errno, strerror(errno));

	     	/* define Header3
	     		currently writing of free text annotations is supported

	     	*/
	    	Header2 = hdr->AS.Header+(NS+1)*256;
	    	tag = 1;
	     	if (TagNLen[tag]>0) {
	     		*(uint32_t*)(Header2) = l_endian_u32(tag + (TagNLen[tag]<<8)); // Tag=1 & Length of Tag 1
	     		size_t pos = 4;
	     		for (k=0; k<hdr->EVENT.LenCodeDesc; k++) {
	     			strcpy((char*)(Header2+pos),hdr->EVENT.CodeDesc[k]);
		     		pos += strlen(hdr->EVENT.CodeDesc[k])+1;
		     	}
		     	Header2[pos]=0; 	// terminating NULL
	     		Header2 += pos+1;
	     	}

		if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw tag2\n");

	     	tag = 2;
	     	if (TagNLen[tag]>0) {
	     		*(uint32_t*)(Header2) = l_endian_u32(tag + (TagNLen[tag]<<8)); // Tag=2 & Length of Tag 2
     			strcpy((char*)(Header2+4),hdr->AS.bci2000);
			Header2 += 4+TagNLen[tag];
	     	}
	     	tag = 3;

		if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw tag3 %i %i\n",tag, TagNLen[tag]);

	     	if (TagNLen[tag]>0) {
	     		*(uint32_t*)(Header2) = l_endian_u32(tag + (TagNLen[tag]<<8)); // Tag=3 & Length of Tag 3
			if (VERBOSE_LEVEL>8) fprintf(stdout,"SOPEN(GDF)w: tag=%i,len=%i\n",tag,TagNLen[tag]);
	     		memset(Header2+4,0,TagNLen[tag]);
	     		size_t len = 0;

     			strcpy((char*)(Header2+4), hdr->ID.Manufacturer.Name);
		     	if (hdr->ID.Manufacturer.Name != NULL)
		     		len += strlen(hdr->ID.Manufacturer.Name);

     			strcpy((char*)(Header2+5+len), hdr->ID.Manufacturer.Model);
		     	if (hdr->ID.Manufacturer.Model != NULL)
		     		len += strlen(hdr->ID.Manufacturer.Model);

     			strcpy((char*)(Header2+6+len), hdr->ID.Manufacturer.Version);
		     	if (hdr->ID.Manufacturer.Version != NULL)
		     		len += strlen(hdr->ID.Manufacturer.Version);

     			strcpy((char*)(Header2+7+len), hdr->ID.Manufacturer.SerialNumber);
		     	if (hdr->ID.Manufacturer.SerialNumber != NULL)
		     		len += strlen(hdr->ID.Manufacturer.SerialNumber);
			Header2 += 4+TagNLen[tag];

	     	}

		if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw tag4\n");

/*
	     	tag = 4;
	     	if (TagNLen[tag]>0) {

	     		*(uint32_t*)(Header2) = l_endian_u32(tag + (TagNLen[tag]<<8)); // Tag=4 & Length of Tag 4
	     		Header2 += 4;
	     		for (k=0; k<hdr->NS; k++) {
	     			*(float*)(Header2 + 4*k)             = l_endian_f32(hdr->CHANNEL[k].Orientation[0]);
	     			*(float*)(Header2 + 4*k + 4*hdr->NS) = l_endian_f32(hdr->CHANNEL[k].Orientation[1]);
	     			*(float*)(Header2 + 4*k + 8*hdr->NS) = l_endian_f32(hdr->CHANNEL[k].Orientation[2]);
	     			*(float*)(Header2 + 4*k +12*hdr->NS) = l_endian_f32(hdr->CHANNEL[k].Area);
	     		}
     			Header2 += 4*sizeof(float)*hdr->NS;
	     	}
*/

		if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw tag5\n");

	     	tag = 5;
	     	if (TagNLen[tag]>0) {
	     		*(uint32_t*)(Header2) = l_endian_u32(tag + (TagNLen[tag]<<8)); // Tag=5 & Length of Tag 5
     			memcpy(Header2+4,hdr->IPaddr,TagNLen[tag]);
			Header2 += 4+TagNLen[tag];
	     	}

		if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw tag6\n");

	     	tag = 6;
	     	if (TagNLen[tag]>0) {
	     		*(uint32_t*)(Header2) = l_endian_u32(tag + (TagNLen[tag]<<8)); // Tag=6 & Length of Tag 6
     			strcpy((char*)(Header2+4),hdr->ID.Technician);
			Header2 += 4+TagNLen[tag];
	     	}

		if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw tag7\n");

	     	tag = 7;
	     	if (TagNLen[tag]>0) {
	     		*(uint32_t*)(Header2) = l_endian_u32(tag + (TagNLen[tag]<<8)); // Tag=7 & Length of Tag 7
     			strcpy((char*)(Header2+4),hdr->ID.Hospital);
			Header2 += 4+TagNLen[tag];
	     	}

		if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFw [339] %p %p\n", Header1,Header2);
}

/****************************************************************************
       gdfbin2struct
	converts flat file into hdr structure
 ****************************************************************************/
int gdfbin2struct(HDRTYPE *hdr)
{
    	unsigned int 	k;
    	char 		tmp[81];
//    	double 		Dur;
//	char*		ptr_str;
	struct tm 	tm_time;
//	time_t		tt;
	uint32_t Dur[2];

      	    	strncpy(tmp,(char*)hdr->AS.Header+3,5); tmp[5]=0;
	    	hdr->VERSION 	= strtod(tmp,NULL);
	    	hdr->NRec 	= lei64p(hdr->AS.Header+236);
	    	Dur[0]  	= leu32p(hdr->AS.Header+244);
	    	Dur[1]  	= leu32p(hdr->AS.Header+248);
	    	hdr->NS   	= leu16p(hdr->AS.Header+252);

	    	if (hdr->VERSION > 1.90) {
		    	hdr->HeadLen 	= leu16p(hdr->AS.Header+184)<<8;
		    	int len = min(66,MAX_LENGTH_PID);
	    		strncpy(hdr->Patient.Id,(const char*)hdr->AS.Header+8,len);
	    		hdr->Patient.Id[len]=0;
	    		len = min(64,MAX_LENGTH_RID);
	    		strncpy(hdr->ID.Recording,(const char*)hdr->AS.Header+88,len);
	    		hdr->ID.Recording[len]=0;
	    		strtok(hdr->Patient.Id," ");
	    		char *tmpptr = strtok(NULL," ");
	    		if ((!hdr->FLAG.ANONYMOUS) && (tmpptr != NULL)) {
				// strncpy(hdr->Patient.Name,tmpptr,Header1+8-tmpptr);
		    		strncpy(hdr->Patient.Name,tmpptr,MAX_LENGTH_NAME);
		    	}

			if (VERBOSE_LEVEL>8) fprintf(stdout,"[202] FMT=%s Ver=%4.2f\n",GetFileTypeString(hdr->TYPE),hdr->VERSION);

	    		hdr->Patient.Smoking      =  Header1[84]%4;
	    		hdr->Patient.AlcoholAbuse = (Header1[84]>>2)%4;
	    		hdr->Patient.DrugAbuse    = (Header1[84]>>4)%4;
	    		hdr->Patient.Medication   = (Header1[84]>>6)%4;
	    		hdr->Patient.Weight       =  Header1[85];
	    		hdr->Patient.Height       =  Header1[86];
	    		hdr->Patient.Sex       	  =  Header1[87]%4;
	    		hdr->Patient.Handedness   = (Header1[87]>>2)%4;
	    		hdr->Patient.Impairment.Visual = (Header1[87]>>4)%4;
	    		hdr->Patient.Impairment.Heart  = (Header1[87]>>6)%4;

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

			if (VERBOSE_LEVEL>8) fprintf(stdout,"[203] FMT=%s Ver=%4.2f\n",GetFileTypeString(hdr->TYPE),hdr->VERSION);

			hdr->T0 		= lei64p(hdr->AS.Header+168);
			hdr->Patient.Birthday 	= lei64p(hdr->AS.Header+176);
			// memcpy(&hdr->T0, Header1+168,8);
			// memcpy(&hdr->Patient.Birthday, Header1+176, 8);

			hdr->ID.Equipment 	= lei64p(hdr->AS.Header+192);
			if (hdr->VERSION < (float)2.10) memcpy(hdr->IPaddr, Header1+200,4);
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
		    		if (fid != NULL) {
			    		fprintf(fid,"\nb4c %f \n%c %i %c.%c%c%c%c%c%c%c\n",hdr->VERSION,169,2007,65,83,99,104,108,246,103,108);
			    		fclose(fid);
			    	}
		    	}

			if (VERBOSE_LEVEL>8) fprintf(stdout,"[209] FMT=%s Ver=%4.2f\n",GetFileTypeString(hdr->TYPE),hdr->VERSION);

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

			hdr->T0 = tm_time2gdf_time(&tm_time);
		    	hdr->HeadLen 	= leu64p(hdr->AS.Header+184);
	    	}
	    	else {
    			B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	    		B4C_ERRMSG = "Error SOPEN(GDF); invalid version number.";
	    		return(B4C_ERRNUM);
	    	}

		if (hdr->HeadLen < (256u * (hdr->NS + 1u))) {
			B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
			B4C_ERRMSG = "(GDF) Length of Header is too small";
			return(B4C_ERRNUM);
		}

	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	    	uint8_t *Header2 = hdr->AS.Header+256;

		hdr->AS.bpb=0;
		size_t bpb8 = 0;
		for (k=0; k<hdr->NS; k++)	{
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;

			if (VERBOSE_LEVEL>8) fprintf(stdout,"[GDF 212] #=%i/%i\n",k,hdr->NS);

			strncpy(hc->Label,(char*)Header2 + 16*k,min(16,MAX_LENGTH_LABEL));
			strncpy(hc->Transducer,(char*)Header2 + 16*hdr->NS + 80*k,min(MAX_LENGTH_TRANSDUCER,80));

			hc->PhysMin = lef64p(Header2+ 8*k + 104*hdr->NS);
			hc->PhysMax = lef64p(Header2+ 8*k + 112*hdr->NS);

			hc->SPR     = leu32p(Header2+ 4*k + 216*hdr->NS);
			hc->GDFTYP  = leu16p(Header2+ 4*k + 220*hdr->NS);
			hc->OnOff   = 1;
			hc->bi      = bpb8>>3;
			hc->bi8     = bpb8;
			uint32_t nbits = (GDFTYP_BITS[hc->GDFTYP]*hc->SPR);
			bpb8 += nbits;

			if (hdr->VERSION < 1.90) {
				char p[9];
				strncpy(p, (char*)Header2 + 8*k + 96*hdr->NS,8);
				p[8] = 0; // remove trailing blanks
				int k1;
				for (k1=7; (k1>0) && isspace(p[k1]); p[k1--] = 0) {};

				hc->PhysDimCode = PhysDimCode(p);

				hc->DigMin   = (double) lei64p(Header2 + 8*k + 120*hdr->NS);
				hc->DigMax   = (double) lei64p(Header2 + 8*k + 128*hdr->NS);

				char *PreFilt  = (char*)(Header2+ 68*k + 136*hdr->NS);
				hc->LowPass  = NaN;
				hc->HighPass = NaN;
				hc->Notch    = NaN;
				hc->TOffset = NaN;
				float lf,hf;
				if (sscanf(PreFilt,"%f - %f Hz",&lf,&hf)==2) {
					hc->LowPass  = hf;
					hc->HighPass = lf;
				}
			}
			else {
				hc->PhysDimCode = leu16p(Header2+ 2*k + 102*hdr->NS);
				// PhysDim(hc->PhysDimCode,hc->PhysDim);

				hc->DigMin   = lef64p(Header2+ 8*k + 120*hdr->NS);
				hc->DigMax   = lef64p(Header2+ 8*k + 128*hdr->NS);

				hc->LowPass  = lef32p(Header2+ 4*k + 204*hdr->NS);
				hc->HighPass = lef32p(Header2+ 4*k + 208*hdr->NS);
				hc->Notch    = lef32p(Header2+ 4*k + 212*hdr->NS);
				hc->XYZ[0]   = lef32p(Header2+ 4*k + 224*hdr->NS);
				hc->XYZ[1]   = lef32p(Header2+ 4*k + 228*hdr->NS);
				hc->XYZ[2]   = lef32p(Header2+ 4*k + 232*hdr->NS);
				// memcpy(&hc->XYZ,Header2 + 4*k + 224*hdr->NS,12);
				hc->Impedance= ldexp(1.0, (uint8_t)Header2[k + 236*hdr->NS]/8);
				hc->TOffset  = NaN;

        		     	if (hdr->VERSION < (float)2.19)
        				hc->Impedance = ldexp(1.0, (uint8_t)Header2[k + 236*hdr->NS]/8);
        		     	else switch(hdr->CHANNEL[k].PhysDimCode & 0xFFE0) {
        		     	        // context-specific header 2 area
        		     	        case 4256:
                				hc->Impedance = *(float*)(Header2+236*hdr->NS+20*k);
        	     				break;
        		     	        case 4288:
        		     	                hc->fZ = *(float*)(Header2+236*hdr->NS+20*k);
        	     				break;
        	     			// default:        // reserved area
                                }
			}
			hc->Cal = (hc->PhysMax-hc->PhysMin)/(hc->DigMax-hc->DigMin);
			hc->Off =  hc->PhysMin-hc->Cal*hc->DigMin;
		}
		hdr->AS.bpb = bpb8>>3;
		if (bpb8 & 0x07) {		// each block must use whole number of bytes
			hdr->AS.bpb++;
			hdr->AS.bpb8 = hdr->AS.bpb<<3;
		}

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[213] FMT=%s Ver=%4.2f\n",GetFileTypeString(hdr->TYPE),hdr->VERSION);

		for (k=0, hdr->SPR=1; k<hdr->NS;k++) {

			if (VERBOSE_LEVEL>8) fprintf(stdout,"[GDF 214] #=%i\n",k);

			if (hdr->CHANNEL[k].SPR)
				hdr->SPR = lcm(hdr->SPR,hdr->CHANNEL[k].SPR);

			if (GDFTYP_BITS[hdr->CHANNEL[k].GDFTYP]==0) {
				B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
				B4C_ERRMSG = "GDF: Invalid or unsupported GDFTYP";
				return(B4C_ERRNUM);
			}
		}

		if (hdr->VERSION < 2.21)
			hdr->SampleRate = ((double)(hdr->SPR))*Dur[1]/Dur[0];
		else
			hdr->SampleRate = ((double)(hdr->SPR))/lef64p(Header1+244);

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[219] FMT=%s Ver=%4.2f\n",GetFileTypeString(hdr->TYPE),hdr->VERSION);

		/* read GDF Header 3 - experimental */
		if ((hdr->HeadLen > 256u*(hdr->NS+1u)) && (hdr->VERSION>=(float)2.10)) {
		    	uint8_t *Header2 = hdr->AS.Header + 256*(hdr->NS+1);
		    	uint8_t tag = 0xff;
		    	size_t pos=0,len=0;
	    		tag = (uint8_t)Header2[0];

			if (VERBOSE_LEVEL>8) fprintf(stdout,"[220] GDFr3: %i %i Tag=%i\n",hdr->HeadLen,hdr->NS,tag);

		    	while ((pos < (hdr->HeadLen-256*(hdr->NS+1)-4)) && (tag>0)) {
		    		len = leu32p(Header2+pos)>>8;

    				if (VERBOSE_LEVEL>8) fprintf(stdout,"GDFr3: Tag=%i Len=%i pos=%i\n",tag,len,pos);

		    		if (0) {}
		    		else if (tag==1) {
		    			// user-specific events i.e. free text annotations
if (VERBOSE_LEVEL>6) fprintf(stdout,"user-specific events defined\n");
					hdr->AS.auxBUF = (uint8_t*) realloc(hdr->AS.auxBUF,len);
					memcpy(hdr->AS.auxBUF, Header2+pos+4, len);
					hdr->EVENT.CodeDesc = (typeof(hdr->EVENT.CodeDesc)) realloc(hdr->EVENT.CodeDesc,257*sizeof(*hdr->EVENT.CodeDesc));
					hdr->EVENT.CodeDesc[0] = "";	// typ==0, is always empty
					hdr->EVENT.LenCodeDesc = 1;
					k = 1;
					while (hdr->AS.auxBUF[k]) {
						hdr->EVENT.CodeDesc[hdr->EVENT.LenCodeDesc++] = (char*)(hdr->AS.auxBUF+k);
						k += strlen((char*)(hdr->AS.auxBUF+k))+1;
					}
		    		}
		    		else if (tag==2) {
		    			/* BCI 2000 information */
		    			hdr->AS.bci2000 = (char*) realloc(hdr->AS.bci2000,len+1);
		    			memcpy(hdr->AS.bci2000,Header2+pos+4,len);
		    			hdr->AS.bci2000[len]=0;
		    		}
		    		else if (tag==3) {
		    			/* manufacture information */
		    			if (len > MAX_LENGTH_MANUF) {
		    				fprintf(stderr,"Warning: length of Manufacturer information (%i) exceeds length of %i bytes\n", len, MAX_LENGTH_MANUF);
		    				len = MAX_LENGTH_MANUF;
		    			}
		    			memcpy(hdr->ID.Manufacturer._field,Header2+pos+4,len);
		    			hdr->ID.Manufacturer._field[MAX_LENGTH_MANUF]=0;
		    			hdr->ID.Manufacturer.Name = hdr->ID.Manufacturer._field;
		    			hdr->ID.Manufacturer.Model= hdr->ID.Manufacturer.Name+strlen(hdr->ID.Manufacturer.Name)+1;
		    			hdr->ID.Manufacturer.Version = hdr->ID.Manufacturer.Model+strlen(hdr->ID.Manufacturer.Model)+1;
		    			hdr->ID.Manufacturer.SerialNumber = hdr->ID.Manufacturer.Version+strlen(hdr->ID.Manufacturer.Version)+1;
		    		}
		    		else if (0) {
		    			// (tag==4) {
		    			/* sensor orientation */
/*		    			// OBSOLETE
		    			for (k=0; k<hdr->NS; k++) {
		    				hdr->CHANNEL[k].Orientation[0] = lef32p(Header2+pos+4+4*k);
		    				hdr->CHANNEL[k].Orientation[1] = lef32p(Header2+pos+4+4*k+hdr->NS*4);
		    				hdr->CHANNEL[k].Orientation[2] = lef32p(Header2+pos+4+4*k+hdr->NS*8);
		    				// if (len >= 12*hdr->NS)
			    				hdr->CHANNEL[k].Area   = lef32p(Header2+pos+4+4*k+hdr->NS*12);
						if (VERBOSE_LEVEL>8)
							fprintf(stdout,"GDF tag=4 #%i pos=%i/%i: %f\n",k,pos,len,hdr->CHANNEL[k].Area);
		    			}
*/		    		}
		    		else if (tag==5) {
		    			/* IP address  */
		    			memcpy(hdr->IPaddr,Header2+pos+4,len);
		    		}
		    		else if (tag==6) {
		    			/* Technician  */
		    			memcpy(hdr->ID.Technician,Header2+pos+4,min(len,MAX_LENGTH_TECHNICIAN));
		    			hdr->ID.Technician[min(len,MAX_LENGTH_TECHNICIAN)]=0;
		    		}
		    		else if (tag==7) {
		    			// recording institution
		    			hdr->ID.Hospital = (char*)(Header2+pos+4);
		    		}

		    		/* further tags may include
		    		- Manufacturer: SCP, MFER, GDF1
		    		- Orientation of MEG channels
		    		- Study ID
		    		- BCI: session, run
		    		*/

		    		pos+= 4+len;
		    		tag = (uint8_t)Header2[pos];
		    	}
		}

		// if (VERBOSE_LEVEL>8) fprintf(stdout,"[GDF 217] #=%li\n",iftell(hdr));
		return(B4C_ERRNUM);
}

/*********************************************************************************
	hdrEVT2rawEVT(HDRTYPE *hdr)
	converts structure HDR.EVENT into raw event data (hdr->AS.rawEventData)
 *********************************************************************************/
size_t hdrEVT2rawEVT(HDRTYPE *hdr) {

	size_t k32u;
	char flag = (hdr->EVENT.DUR != NULL) && (hdr->EVENT.CHN != NULL);
	if (flag)   // any DUR or CHN is larger than 0
		for (k32u=0, flag=0; (k32u<hdr->EVENT.N) && !flag; k32u++)
			flag |= hdr->EVENT.CHN[k32u] || hdr->EVENT.DUR[k32u];

	int sze = flag ? 12 : 6;
	size_t len = 8+hdr->EVENT.N*sze;
	hdr->AS.rawEventData = (uint8_t*) realloc(hdr->AS.rawEventData,len);
	uint8_t *buf = hdr->AS.rawEventData;

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
	uint8_t *buf1=hdr->AS.rawEventData+8;
	uint8_t *buf2=hdr->AS.rawEventData+8+hdr->EVENT.N*4;
	for (k32u=0; k32u<hdr->EVENT.N; k32u++) {
		*(uint32_t*)(buf1+k32u*4) = l_endian_u32(hdr->EVENT.POS[k32u]+1); // convert from 0-based (biosig4c++) to 1-based (GDF) indexing
		*(uint16_t*)(buf2+k32u*2) = l_endian_u16(hdr->EVENT.TYP[k32u]);
	}
	if (flag) {
		buf1 = hdr->AS.rawEventData+8+hdr->EVENT.N*6;
		buf2 = hdr->AS.rawEventData+8+hdr->EVENT.N*8;
		for (k32u=0; k32u<hdr->EVENT.N; k32u++) {
			*(uint16_t*)(buf1+k32u*2) = l_endian_u16(hdr->EVENT.CHN[k32u]);
			*(uint32_t*)(buf2+k32u*4) = l_endian_u32(hdr->EVENT.DUR[k32u]);
		}
	}
	return(len);
}

/*********************************************************************************
	rawEVT2hdrEVT(HDRTYPE *hdr)
	converts raw event data (hdr->AS.rawEventData) into structure HDR.EVENT
 *********************************************************************************/
void rawEVT2hdrEVT(HDRTYPE *hdr) {
	// TODO: avoid additional copying
	size_t k;
			uint8_t *buf = hdr->AS.rawEventData;
			if (buf==NULL) {
				hdr->EVENT.N = 0;
				return;
			}

			if (hdr->VERSION < 1.94) {
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
//			int sze = (buf[0]>1) ? 12 : 6;

	 		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, hdr->EVENT.N*sizeof(*hdr->EVENT.POS) );
			hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP) );

			uint8_t *buf1 = hdr->AS.rawEventData+8;
			uint8_t *buf2 = hdr->AS.rawEventData+8+4*hdr->EVENT.N;
			for (k=0; k < hdr->EVENT.N; k++) {
				hdr->EVENT.POS[k] = leu32p(buf1 + k*4)-1;  // convert from 1-based (GDF) to 0-based (biosig4c++) indexing
				hdr->EVENT.TYP[k] = leu16p(buf2 + k*2);
			}
			if (buf[0]>1) {
				hdr->EVENT.DUR = (uint32_t*) realloc(hdr->EVENT.DUR,hdr->EVENT.N*sizeof(*hdr->EVENT.DUR));
				hdr->EVENT.CHN = (uint16_t*) realloc(hdr->EVENT.CHN,hdr->EVENT.N*sizeof(*hdr->EVENT.CHN));

				buf1 = hdr->AS.rawEventData+8+6*hdr->EVENT.N;
				buf2 = hdr->AS.rawEventData+8+8*hdr->EVENT.N;
				for (k=0; k < hdr->EVENT.N; k++) {
					hdr->EVENT.CHN[k] = leu16p(buf1 + k*2);
					hdr->EVENT.DUR[k] = leu32p(buf2 + k*4);
				}
			}
			else {
				hdr->EVENT.DUR = NULL;
				hdr->EVENT.CHN = NULL;
			}
}

int NumberOfChannels(HDRTYPE *hdr)
{
        int k,NS;
        for (k=0, NS=0; k<hdr->NS; k++)
                if (hdr->CHANNEL[k].OnOff) NS++;

#ifdef CHOLMOD_H
        if (hdr->Calib == NULL)
                return (NS);

        if (NS==hdr->Calib->nrow)
                return (hdr->Calib->ncol);
#endif
        return(hdr->NS);
}

int RerefCHANNEL(HDRTYPE *hdr, void *arg2, char Mode)
{
#ifndef CHOLMOD_H
                if (!arg2 || !Mode) return(0); // do nothing

                B4C_ERRNUM = B4C_REREF_FAILED;
                B4C_ERRMSG = "Error RerefCHANNEL: cholmod library is missing";
                return(1);
#else
                if (arg2==NULL) Mode = 0; // do nothing

                cholmod_sparse *ReRef=NULL;
		uint16_t flag,NS;
                size_t i,j,k;
                long r;

                cholmod_common c ;
                cholmod_start (&c) ; /* start CHOLMOD */

                switch (Mode) {
                case 1: {
                        HDRTYPE *RR = sopen((char*)arg2,"r",NULL);
                        ReRef       = RR->Calib;
                        RR->Calib   = NULL; // do not destroy ReRef
                        destructHDR(RR);
                        break;
                        }
                case 2: ReRef = (cholmod_sparse*) arg2;
                        break;
                }

                if ((ReRef==NULL) || !Mode) {
                        // reset rereferencing
        		if (hdr->Calib != NULL)
                                cholmod_free_sparse(&hdr->Calib,&c);

                        hdr->Calib = ReRef;
        		free(hdr->rerefCHANNEL);
        		hdr->rerefCHANNEL = NULL;
                        cholmod_finish (&c) ;
        	        return(0);
                }
                cholmod_sparse *A = ReRef;

                // check dimensions
                for (k=0, NS=0; k<hdr->NS; k++)
                        if (hdr->CHANNEL[k].OnOff) NS++;
                if (NS - A->nrow) {
                        B4C_ERRNUM = B4C_REREF_FAILED;
                        B4C_ERRMSG = "Error REREF_CHAN: size of data does not fit ReRef-matrix";
                        cholmod_finish (&c) ;
                        return(1);
                }

                // allocate memory
		if (hdr->Calib != NULL)
                        cholmod_free_sparse(&hdr->Calib,&c);

		if (VERBOSE_LEVEL>6) {
			c.print = 5;
			cholmod_print_sparse(ReRef,"HDR.Calib", &c);
		}
                cholmod_finish (&c) ;        // stop cholmod

                hdr->Calib = ReRef;
		CHANNEL_TYPE *NEWCHANNEL = (CHANNEL_TYPE*) realloc(hdr->rerefCHANNEL, A->ncol*sizeof(CHANNEL_TYPE));
		hdr->rerefCHANNEL = NEWCHANNEL;
                hdr->FLAG.ROW_BASED_CHANNELS = 1;

                // check each component
       		for (i=0; i<A->ncol; i++)         // i .. column index
       		{
			flag = 0;
			int mix = -1, oix = -1, pix = -1;
			double m  = 0.0;
			double v;
			for (j = *(((int*)A->p)+i); j < *(((int*)A->p)+i+1); j++) {

				v = *(((double*)A->x)+j);
				r = *(((int*)A->i)+j);        // r .. row index

				if (v>m) {
					m = v;
					mix = r;
				}
				if (v==1.0) {
					if (oix<0)
						oix = r;
					else
						fprintf(stderr,"Warning: ambiguous channel information (in new #%i, more than one scaling factor of 1.0 is used.) \n",i,j);
				}
				if (v) {
					if (pix == -1) {
                				memcpy(NEWCHANNEL+i, hdr->CHANNEL+r, sizeof(CHANNEL_TYPE));
						pix = 0;
					}
					else {
					        if (NEWCHANNEL[i].PhysDimCode != hdr->CHANNEL[r].PhysDimCode)
					                NEWCHANNEL[i].PhysDimCode = 0;
					        if (NEWCHANNEL[i].LowPass != hdr->CHANNEL[r].LowPass)
					                NEWCHANNEL[i].LowPass = NaN;
					        if (NEWCHANNEL[i].HighPass != hdr->CHANNEL[r].HighPass)
					                NEWCHANNEL[i].HighPass = NaN;
					        if (NEWCHANNEL[i].Notch != hdr->CHANNEL[r].Notch)
					                NEWCHANNEL[i].Notch = NaN;

					        if (NEWCHANNEL[i].SPR != hdr->CHANNEL[r].SPR)
					                NEWCHANNEL[i].SPR = lcm(NEWCHANNEL[i].SPR, hdr->CHANNEL[r].SPR);
					        if (NEWCHANNEL[i].GDFTYP != hdr->CHANNEL[r].GDFTYP)
					                NEWCHANNEL[i].GDFTYP = max(NEWCHANNEL[i].GDFTYP, hdr->CHANNEL[r].GDFTYP);

				                NEWCHANNEL[i].Impedance += fabs(v)*NEWCHANNEL[r].Impedance;
				                NEWCHANNEL[i].GDFTYP = 16;
					}
				}
				if (r >= hdr->NS) {
					flag = 1;
					fprintf(stderr,"Error: index (%i) in channel (%i) exceeds number of channels (%i)\n",r,i,hdr->NS);
				}
			}

			if (oix>-1) r=oix;        // use the info from channel with a scaling of 1.0 ;
			else if (mix>-1) r=mix;   // use the info from channel with the largest scale;
			else r = -1;

			if (!flag && (r<hdr->NS) && (r>=0)) {
			        // if successful
			        memcpy(NEWCHANNEL[i].Label, hdr->CHANNEL[r].Label, MAX_LENGTH_LABEL);
				NEWCHANNEL[i].GDFTYP = 16; // float
                        }
			else {
			        sprintf(NEWCHANNEL[i].Label,"component #%i",i);
			}
                }
		return(0);
#endif
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

    	unsigned int 	k,k2;
//    	uint32_t	k32u;
    	size_t	 	count;
    	char 		tmp[81];
//    	double 		Dur;
	char*		ptr_str;
	struct tm 	tm_time;
//	time_t		tt;
	uint16_t	BCI2000_StatusVectorLength=0;	// specific for BCI2000 format


	if (hdr==NULL)
		hdr = constructHDR(0,0);	// initializes fields that may stay undefined during SOPEN

	hdr->FileName = FileName;

	setlocale(LC_NUMERIC,"C");

// hdr->FLAG.SWAP = (__BYTE_ORDER == __BIG_ENDIAN); 	// default: most data formats are little endian
hdr->FILE.LittleEndian = 1;

if (!strncmp(MODE,"r",1))
{
	if (VERBOSE_LEVEL>8)
		fprintf(stdout,"[201] %s\n",hdr->FileName);

	size_t k,name=0,ext=0;
	for (k=0; hdr->FileName[k]; k++) {
		if (hdr->FileName[k]==FILESEP) name = k+1;
		if (hdr->FileName[k]=='.')     ext  = k+1;
	}

	const char *FileExt  = hdr->FileName+ext;
	const char *FileName = hdr->FileName+name;

	/* AINF */
	if (!strcmp(FileExt, "ainf")) {
		if (VERBOSE_LEVEL>8) fprintf(stdout,"getfiletype ainf1 %s %i\n",hdr->FileName,ext);
		char* AINF_RAW_FILENAME = (char*)calloc(strlen(hdr->FileName)+5,sizeof(char));
		strncpy(AINF_RAW_FILENAME, hdr->FileName,ext);
		strcpy(AINF_RAW_FILENAME+ext, "raw");
		FILE* fid1=fopen(AINF_RAW_FILENAME,"rb");
		if (fid1) {
			fclose(fid1);
			hdr->TYPE = AINF;
		}
		free(AINF_RAW_FILENAME);
	}
	else if (!strcmp(FileExt, "raw")) {
		char* AINF_RAW_FILENAME = (char*)calloc(strlen(hdr->FileName)+5,sizeof(char));
		strncpy(AINF_RAW_FILENAME, hdr->FileName,ext);
		strcpy(AINF_RAW_FILENAME+ext, "ainf");
		FILE* fid1=fopen(AINF_RAW_FILENAME,"r");
		if (fid1) {
			fclose(fid1);
			hdr->TYPE = AINF;
		}
		free(AINF_RAW_FILENAME);
	}

#ifndef WITHOUT_NETWORK
	if (!memcmp(hdr->FileName,"bscs://",7)) {
		uint64_t ID;
    		char *hostname = (char*)hdr->FileName+7;
    		char *t = strrchr(hostname,'/');
    		if (t==NULL) {
			B4C_ERRNUM = B4C_CANNOT_OPEN_FILE;
    			B4C_ERRMSG = "SOPEN-NETWORK: file identifier not specifed";
    			return(hdr);
    		}
    		*t=0;
		cat64(t+1, &ID);
		int sd,s;
		sd = bscs_connect(hostname);
		if (sd<0) {
			B4C_ERRNUM = B4C_CANNOT_OPEN_FILE;
			B4C_ERRMSG = "could not connect to server";
			return(hdr);
		}
  		hdr->FILE.Des = sd;
		s  = bscs_open(sd, &ID);
  		s  = bscs_requ_hdr(sd,hdr);
  		s  = bscs_requ_evt(sd,hdr);
  		hdr->FILE.OPEN = 1;
  		return(hdr);
    	}
#endif

        hdr->FILE.COMPRESSION = 0;
	hdr   = ifopen(hdr,"rb");
	if (!hdr->FILE.OPEN) {
		B4C_ERRNUM = B4C_CANNOT_OPEN_FILE;
    		B4C_ERRMSG = "Error SOPEN(READ); Cannot open file.";
    		return(hdr);
	}
    	hdr->AS.Header = (uint8_t*)malloc(353);
	count = ifread(Header1,1,352,hdr);
	hdr->AS.Header[count]=0;

	const uint8_t MAGIC_NUMBER_GZIP[] = {31,139,8};
	if (!memcmp(Header1,MAGIC_NUMBER_GZIP,3)) {
#ifdef ZLIB_H
		ifclose(hdr);

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[221] \n");

	        hdr->FILE.COMPRESSION = 1;
//		hdr->FILE.gzFID = gzdopen(hdr->FILE.FID,"rb"); // FIXME
        	hdr= ifopen(hdr,"rb");
	    	if (!hdr->FILE.OPEN) {
    			B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
			B4C_ERRMSG = "Error SOPEN(GZREAD); Cannot open file.";
	    		return(hdr);
	    	}
		count = ifread(Header1,1,352,hdr);
        	hdr->AS.Header[352]=0;
#else
		B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	    	B4C_ERRMSG = "Error SOPEN(READ); *.gz file not supported because not linked with zlib.";
#endif
    	}
	hdr->HeadLen = count;
	getfiletype(hdr);
//		fprintf(stdout,"[201] FMT=%s Ver=%4.2f\n",GetFileTypeString(hdr->TYPE),hdr->VERSION);

	if (hdr->TYPE != unknown)
		;
    	else if (!memcmp(Header1,FileName,strspn(FileName,".")) && (!strcmp(FileExt,"HEA") || !strcmp(FileExt,"hea") ))
	    	hdr->TYPE = MIT;

    	if (hdr->TYPE == unknown) {
    		B4C_ERRNUM = B4C_FORMAT_UNKNOWN;
    		B4C_ERRMSG = "ERROR BIOSIG4C++ SOPEN(read): Dataformat not known.\n";
    		ifclose(hdr);
		return(hdr);
	}

	if (VERBOSE_LEVEL>8)
		fprintf(stdout,"[201] FMT=%s Ver=%4.2f\n",GetFileTypeString(hdr->TYPE),hdr->VERSION);

    	count = iftell(hdr);
    	hdr->AS.first  =  0;
    	hdr->AS.length =  0;
	hdr->AS.bpb    = -1; 	// errorneous value: ensures that hdr->AS.bpb will be defined

#ifndef WITHOUT_NETWORK
	if (!memcmp(hdr->AS.Header,"bscs://",7)) {
		hdr->AS.Header[count]=0;

		uint64_t ID;
    		char *hostname = Header1+7;
		Header1[6]=0;
    		char *t = strrchr(hostname,'/');
    		if (t==NULL) {
			B4C_ERRNUM = B4C_CANNOT_OPEN_FILE;
    			B4C_ERRMSG = "SOPEN-NETWORK: file identifier not specifed";
    			return(hdr);
    		}
    		t[0]=0;
		cat64(t+1, &ID);
		int sd,s;
		sd = bscs_connect(hostname);
		if (sd<0) {
    			fprintf(stderr,"could not connect to %s\n",hostname);
			B4C_ERRNUM = B4C_CANNOT_OPEN_FILE;
			B4C_ERRMSG = "could not connect to server";
			return(hdr);
		}
  		hdr->FILE.Des = sd;
		s  = bscs_open(sd, &ID);
  		s  = bscs_requ_hdr(sd,hdr);
  		s  = bscs_requ_evt(sd,hdr);
  		hdr->FILE.OPEN = 1;
  		return(hdr);
    	}
    	else
#endif

	if (hdr->TYPE == GDF) {

	    	if (hdr->VERSION > 1.90)
		    	hdr->HeadLen = leu16p(hdr->AS.Header+184)<<8;
		else
		    	hdr->HeadLen = leu64p(hdr->AS.Header+184);

	    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,hdr->HeadLen);
                if (count < hdr->HeadLen)
		    	count += ifread(hdr->AS.Header+count, 1, hdr->HeadLen-count, hdr);

                if (count < hdr->HeadLen) {
			if (VERBOSE_LEVEL>7) fprintf(stdout,"ambigous GDF header size: %i %i\n",count,hdr->HeadLen);
                        B4C_ERRNUM = B4C_INCOMPLETE_FILE;
                        B4C_ERRMSG = "reading GDF header failed";
                        return(hdr);
		}

		gdfbin2struct(hdr);

		hdr->EVENT.N   = 0;
		hdr->EVENT.POS = NULL;
		hdr->EVENT.TYP = NULL;
		hdr->EVENT.DUR = NULL;
		hdr->EVENT.CHN = NULL;

		struct stat FileBuf;
		stat(hdr->FileName,&FileBuf);

		if (hdr->NRec < 0) {
			hdr->NRec = (FileBuf.st_size - hdr->HeadLen)/hdr->AS.bpb;
			if (hdr->AS.rawEventData!=NULL) {
				free(hdr->AS.rawEventData);
				hdr->AS.rawEventData=NULL;
			}
		}
		else if (FileBuf.st_size > hdr->HeadLen + hdr->AS.bpb*hdr->NRec + 8)
		{
			ifseek(hdr, hdr->HeadLen + hdr->AS.bpb*hdr->NRec, SEEK_SET);
			// READ EVENTTABLE
			hdr->AS.rawEventData = (uint8_t*)realloc(hdr->AS.rawEventData,8);
			int c = ifread(hdr->AS.rawEventData, sizeof(uint8_t), 8, hdr);
    			uint8_t *buf = hdr->AS.rawEventData;

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
			else {
				hdr->EVENT.N = buf[1] + (buf[2] + buf[3]*256)*256;
				hdr->EVENT.SampleRate = lef32p(buf + 4);
			}
			int sze = (buf[0]>1) ? 12 : 6;
			hdr->AS.rawEventData = (uint8_t*)realloc(hdr->AS.rawEventData,8+hdr->EVENT.N*sze);
			c = ifread(hdr->AS.rawEventData+8, sze, hdr->EVENT.N, hdr);
			ifseek(hdr, hdr->HeadLen, SEEK_SET);
			if (c < hdr->EVENT.N) {
                                B4C_ERRNUM = B4C_INCOMPLETE_FILE;
                                B4C_ERRMSG = "reading GDF eventtable failed";
                                return(hdr);
			}
			rawEVT2hdrEVT(hdr);
		}
		else
			hdr->EVENT.N = 0;

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[228] FMT=%s Ver=%4.2f\n",GetFileTypeString(hdr->TYPE),hdr->VERSION);

    	}
    	else if ((hdr->TYPE == EDF) || (hdr->TYPE == BDF))	{
                if (count < 256) {
                        B4C_ERRNUM = B4C_INCOMPLETE_FILE;
                        B4C_ERRMSG = "reading BDF/EDF fixed header failed";
                        return(hdr);
                }

		size_t	EventChannel = 0;
    		strncpy(hdr->Patient.Id,Header1+8,min(MAX_LENGTH_PID,80));
    		memcpy(hdr->ID.Recording,Header1+88,min(80,MAX_LENGTH_RID));
		hdr->ID.Recording[MAX_LENGTH_RID]=0;

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[EDF 211] #=%li\nT0=<%16s>",iftell(hdr),Header1+168);

		// TODO: sanity check of T0
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
	    	if (hdr->HeadLen != ((hdr->NS+1u)<<8)) {
	    		B4C_ERRNUM = B4C_UNSPECIFIC_ERROR;
	    		B4C_ERRMSG = "EDF/BDF corrupted: HDR.NS and HDR.HeadLen do not fit";
	    		if (VERBOSE_LEVEL>8)
	    		fprintf(stdout,"HeadLen=%i,%i\n",hdr->HeadLen ,(hdr->NS+1)<<8);
	    	};

	    	hdr->NRec	= atoi(strncpy(tmp,Header1+236,8));
	    	//Dur		= atof(strncpy(tmp,Header1+244,8));

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[EDF 211b] #=%li\nT0=%s\n",iftell(hdr),asctime(&tm_time));

		if (!strncmp(Header1+192,"EDF+",4)) {
			char ListOfMonth[12][4] = {"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[EDF 211c+] <%s>\n",hdr->Patient.Id);

	    		strtok(hdr->Patient.Id," ");
	    		ptr_str = strtok(NULL," ");
			if (ptr_str!=NULL) {
				// define Id, Sex, Birthday, Name
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[EDF 211c+] <%p>\n",ptr_str);

	    		hdr->Patient.Sex = (ptr_str[0]=='f')*2 + (ptr_str[0]=='F')*2 + (ptr_str[0]=='M') + (ptr_str[0]=='m');
	    		ptr_str = strtok(NULL," ");	// startdate
	    		char *tmpptr = strtok(NULL," ");
	    		if ((!hdr->FLAG.ANONYMOUS) && (tmpptr != NULL)) {
		    		strcpy(hdr->Patient.Name,tmpptr);
		    	}

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[EDF 211c] #=%li\n",iftell(hdr));

			if (strlen(ptr_str)==11) {
				struct tm t1;
		    		t1.tm_mday = atoi(strtok(ptr_str,"-"));
		    		strcpy(tmp,strtok(NULL,"-"));
		    		for (k=0; k<strlen(tmp); ++k) tmp[k]= toupper(tmp[k]);	// convert to uppper case
		    		t1.tm_year = atoi(strtok(NULL,"- ")) - 1900;
		    		t1.tm_mon  = -1;
		    		for (k=0; k<12; k++)
		    			if (!strcmp(tmp,ListOfMonth[k])) t1.tm_mon = k;
		    		t1.tm_sec  = 0;
		    		t1.tm_min  = 0;
		    		t1.tm_hour = 12;
		    		t1.tm_isdst= -1;
		    		hdr->Patient.Birthday = tm_time2gdf_time(&t1);
		    	}}

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[EDF 211d] <%s>\n",hdr->ID.Recording);

			if (!strncmp(Header1+88,"Startdate ",10)) {
				size_t pos = strcspn(Header1+88+10," ")+10;
				strncpy(hdr->ID.Recording, Header1+88+pos+1, 80-pos);
				hdr->ID.Recording[80-pos-1] = 0;
				if (strtok(hdr->ID.Recording," ")!=NULL) {
					strncpy(hdr->ID.Technician, strtok(NULL," "),MAX_LENGTH_TECHNICIAN);
					hdr->ID.Manufacturer.Name  = strtok(NULL," ");
				}

				Header1[167]=0;
		    		strtok(Header1+88," ");
	    			ptr_str = strtok(NULL," ");
				// check EDF+ Startdate against T0
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[EDF 211e-] <%s>\n",ptr_str);
				/* TODO:
					fix "Startdate X ..."

				*/
				if (strcmp(ptr_str,"X")) {
					int d,m,y;
					d = atoi(strtok(ptr_str,"-"));
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[EDF 211e] <%s>\n",ptr_str);
					ptr_str = strtok(NULL,"-");
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[EDF 211f] <%s>\n",ptr_str);
	    				strcpy(tmp,ptr_str);
		    			for (k=0; k<strlen(tmp); ++k) tmp[k]=toupper(tmp[k]);	// convert to uppper case
		    			for (m=0; m<12; m++) if (!strcmp(tmp,ListOfMonth[m])) break;
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[EDF 211g] <%s>\n",tmp);
	    				y = atoi(strtok(NULL,"-")) - 1900;
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[EDF 211h] <%i>\n",tm_time.tm_year);

		    			if ((tm_time.tm_mday == d) && (tm_time.tm_mon == m)) {
		    				tm_time.tm_year = y;
				    		tm_time.tm_isdst= -1;
				    	}
					else {
	    					fprintf(stderr,"Error SOPEN(EDF+): recording dates do not match %i/%i <> %i/%i\n",d,m,tm_time.tm_mday,tm_time.tm_mon);
	    				}
		    		}
			}

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[EDF 211z] #=%li\n",iftell(hdr));

		}
		hdr->T0 = tm_time2gdf_time(&tm_time);
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[EDF 212] #=%li\n",iftell(hdr));

		if (hdr->NS==0) return(hdr);

	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	    	hdr->AS.Header = (uint8_t*) realloc(Header1,hdr->HeadLen);
	    	char *Header2 = (char*)hdr->AS.Header+256;
	    	count  += ifread(hdr->AS.Header+count, 1, hdr->HeadLen-count, hdr);

                if (count < hdr->HeadLen) {
                        B4C_ERRNUM = B4C_INCOMPLETE_FILE;
                        B4C_ERRMSG = "reading BDF/EDF variable header failed";
                        return(hdr);
                }

		char p[9];
		hdr->AS.bpb = 0;
		size_t BitsPerBlock = 0;
		for (k=0, hdr->SPR = 1; k<hdr->NS; k++)	{
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"[EDF 213] #%i/%i\n",k,hdr->NS);

			strncpy(hc->Label,Header2 + 16*k,min(MAX_LENGTH_LABEL,16));
			int k1;
			for (k1=strlen(hc->Label)-1; isspace(hc->Label[k1]) && k1; k1--)
				hc->Label[k1] = 0;	// deblank

			strncpy(hc->Transducer, Header2+80*k+16*hdr->NS, min(80,MAX_LENGTH_TRANSDUCER));
			for (k1=strlen(hc->Transducer)-1; isspace(hc->Transducer[k1]) && k1; k1--)
				hc->Transducer[k1]='\0'; 	// deblank

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"[EDF 214] #%i/%i\n",k,hdr->NS);

			// PhysDim -> PhysDimCode
			memcpy(p,Header2 + 8*k + 96*hdr->NS,8);

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"[EDF 215-] #%i/%i\n",k,hdr->NS);

			p[8] = 0; // remove trailing blanks
			for (k1=7; (k1>0) && isspace(p[k1]); p[k1--]=0) {};

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"[EDF 215] #%i/%i\n",k,hdr->NS);

			hc->PhysDimCode = PhysDimCode(p);
			tmp[8] = 0;

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"[EDF 215a] #%i/%i\n",k,hdr->NS);

			hc->PhysMin = atof(strncpy(tmp,Header2 + 8*k + 104*hdr->NS,8));
			hc->PhysMax = atof(strncpy(tmp,Header2 + 8*k + 112*hdr->NS,8));
			hc->DigMin  = atof(strncpy(tmp,Header2 + 8*k + 120*hdr->NS,8));
			hc->DigMax  = atof(strncpy(tmp,Header2 + 8*k + 128*hdr->NS,8));

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"[EDF 215b] #%i/%i\n",k,hdr->NS);

			hc->Cal     = (hc->PhysMax - hc->PhysMin) / (hc->DigMax-hc->DigMin);
			hc->Off     =  hc->PhysMin - hc->Cal*hc->DigMin;

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"[EDF 215c] #%i/%i\n",k,hdr->NS);

			hc->SPR     	= atol(strncpy(tmp, Header2 + 8*k + 216*hdr->NS,8));
			hc->GDFTYP  	= ((hdr->TYPE != BDF) ? 3 : 255+24);
			hc->OnOff   	= 1;
			hdr->SPR 	= lcm(hdr->SPR, hc->SPR);
			hc->bi 		= hdr->AS.bpb;
			hc->bi8     	= BitsPerBlock;
			uint32_t nbits 	= GDFTYP_BITS[hc->GDFTYP]*hc->SPR;
			BitsPerBlock   += nbits;
			uint32_t nbytes = nbits>>3;
			hdr->AS.bpb 	+= nbytes;

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"[EDF 216] #%i/%i/%i/%i/%i/%i\n",k,hdr->NS,nbytes,hdr->AS.bpb,hc->SPR,hdr->SPR);

			hc->LowPass = NaN;
			hc->HighPass = NaN;
			hc->Notch = NaN;
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
					hc->HighPass = lf * PhysDimScale(pdc);
				pdc = PhysDimCode(s2);
				if ((pdc & 0xffe0) == 2496)	// Hz
					hc->LowPass  = hf * PhysDimScale(pdc);
			}
			else {
				d = sscanf(PreFilt,"HP: %s LP: %f %s ",s1,&hf,s2);
				if (d==3) {
					if (!strncmp(s1,"DC",2))
						hc->HighPass = 0;
					pdc = PhysDimCode(s2);
					if ((pdc & 0xffe0) == 2496)	// Hz
						hc->LowPass  = hf * PhysDimScale(pdc);
				}
			}

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"[EDF 218] #%i/%i/%i\n",k,hdr->NS,hdr->SPR);

			if ((hdr->TYPE==EDF) && !strncmp(Header1+192,"EDF+",4) && !strcmp(hc->Label,"EDF Annotations")) {
				hc->OnOff = 0;
				EventChannel = k+1;
			}
			else if ((hdr->TYPE==BDF) && !strcmp(hc->Label,"Status")) {
				hc->OnOff = 1;
				EventChannel = k+1;
			}

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"[EDF 219] #%i/%i/%i\n",k,hdr->NS,hdr->SPR);

		}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// EDF does not support automated overflow and saturation detection
	    	double Dur	= atof(strncpy(tmp,Header1+244,8));
		hdr->SampleRate = hdr->SPR/Dur;

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[EDF 220] #=%li\n",iftell(hdr));

		if (hdr->NRec <= 0) {
        		struct stat FileBuf;
        		stat(hdr->FileName,&FileBuf);
			hdr->NRec = (FileBuf.st_size - hdr->HeadLen)/hdr->AS.bpb;
		}

		if (EventChannel) {
			/* read Annotation and Status channel and extract event information */
			CHANNEL_TYPE *hc = hdr->CHANNEL+EventChannel-1;

			size_t sz   	= GDFTYP_BITS[hc->GDFTYP]>>3;
			size_t len 	= hc->SPR * hdr->NRec * sz;
			uint8_t *Marker = (uint8_t*)malloc(len + 1);
			size_t skip 	= hdr->AS.bpb - hc->SPR * sz;
			ifseek(hdr, hdr->HeadLen + hc->bi, SEEK_SET);
			nrec_t k3;
			for (k3=0; k3<hdr->NRec; k3++) {
			    	ifread(Marker+k3*hc->SPR * sz, 1, hc->SPR * sz, hdr);
				ifseek(hdr, skip, SEEK_CUR);
			}
			size_t N_EVENT  = 0;
			hdr->EVENT.SampleRate = hdr->SampleRate;

			if (hdr->TYPE==EDF) {
			/* convert EDF+ annotation channel into event table */

				char *p0,*p1,*p2;
				N_EVENT=0;
				Marker[len]=255; // stop marker;
				double Onset,Duration;
				char FLAG_NONZERO_DURATION = 0;
				p1 = (char*)Marker;

				for (k=0; k<len; ) {

					while ((Marker[k] == 0) && (k<len)) ++k; // search for start of annotation
					if (k>=len) break;

					if (N_EVENT+1 >= hdr->EVENT.N) {
						hdr->EVENT.N  += 2560;
						hdr->EVENT.POS = (uint32_t*)realloc(hdr->EVENT.POS, hdr->EVENT.N*sizeof(*hdr->EVENT.POS));
						hdr->EVENT.DUR = (uint32_t*)realloc(hdr->EVENT.DUR, hdr->EVENT.N*sizeof(*hdr->EVENT.DUR));
						hdr->EVENT.TYP = (uint16_t*)realloc(hdr->EVENT.TYP, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP));
						hdr->EVENT.CHN = (uint16_t*)realloc(hdr->EVENT.CHN, hdr->EVENT.N*sizeof(*hdr->EVENT.CHN));
					}

					Onset = strtod(p1+k, &p2);
					p0 = p2;
					if (*p2==21) {
						Duration = strtod(p2+1,&p0);
						FLAG_NONZERO_DURATION = 1;
					}
					else if (*p2==20)
						Duration = 0;
					else {  /* sanity check */
						fprintf(stdout,"Warning EDF+: corrupted annotation channel - decoding of event #%i <%s> failed.\n",N_EVENT+1,p1+k);
						break; // do not decode any further events.
					}

					p0[strlen(p0)-1] = 0;	// remove last ascii(20)
					hdr->EVENT.POS[N_EVENT] = (uint32_t)round(Onset * hdr->EVENT.SampleRate);	// 0-based indexing
					hdr->EVENT.DUR[N_EVENT] = (uint32_t)round(Duration * hdr->EVENT.SampleRate);
					hdr->EVENT.CHN[N_EVENT] = 0;
					hdr->EVENT.TYP[N_EVENT] = min(255,strlen(p0+1));	// this is a hack, maps all codes into "user specific events"

					/* conversion from free text annotations to biosig event codes */
					if (!strcmp(p0+1,"QRS")) hdr->EVENT.TYP[N_EVENT] = 0x0501;
					else {
						FreeTextEvent(hdr,N_EVENT,p0+1);
					}

					// if (VERBOSE_LEVEL>8)
					//	fprintf(stdout,"%i,EDF+: %i\t%i\t%03i:\t%f\t%f\t%s\t%s\n",sizeof(char**),len,k,N_EVENT,Onset,Duration,p2+1,p0+1);

					N_EVENT++;
					while ((Marker[k] > 0) && (k<len)) k++;	// search for end of annotation
				}
				hdr->EVENT.N = N_EVENT;
				if (!FLAG_NONZERO_DURATION){
					free(hdr->EVENT.DUR);
					hdr->EVENT.DUR = NULL;
					free(hdr->EVENT.CHN);
					hdr->EVENT.CHN = NULL;
				}
				hdr->AS.auxBUF = Marker;	// contains EVENT.CodeDesc strings
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
				for (k=1; k<len/3; k++) {
					d1 = ((uint32_t)Marker[3*k+2]<<16) + ((uint32_t)Marker[3*k+1]<<8) + (uint32_t)Marker[3*k];
					if ((d1 & 0x010000) != (d0 & 0x010000)) ++N_EVENT;
					if ((d1 & 0x00ffff) != (d0 & 0x00ffff)) ++N_EVENT;
					d0 = d1;
				}

				hdr->EVENT.N = N_EVENT+1;
				hdr->EVENT.SampleRate = hdr->SampleRate;
				hdr->EVENT.POS = (uint32_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.POS));
				hdr->EVENT.TYP = (uint16_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.TYP));
				hdr->EVENT.DUR = NULL;
				hdr->EVENT.CHN = NULL;
				d0 = ((uint32_t)Marker[2]<<16) + ((uint32_t)Marker[1]<<8) + (uint32_t)Marker[0];
				hdr->EVENT.POS[0] = 0;        // 0-based indexing
				hdr->EVENT.TYP[0] = d0 & 0x00ffff;
				for (N_EVENT=1, k=1; k<len/3; k++) {

					d1 = ((uint32_t)Marker[3*k+2]<<16) + ((uint32_t)Marker[3*k+1]<<8) + (uint32_t)Marker[3*k];
					if ((d1 & 0x010000) != (d0 & 0x010000)) {
						hdr->EVENT.POS[N_EVENT] = k;        // 0-based indexing
						hdr->EVENT.TYP[N_EVENT] = 0x7ffe;
						++N_EVENT;
					}

					if ((d1 & 0x00ffff) != (d0 & 0x00ffff)) {
						hdr->EVENT.POS[N_EVENT] = k;        // 0-based indexing
						uint16_t d2 = d1 & 0x00ffff;
						if (!d2) d2 = (uint16_t)(d0 & 0x00ffff) | 0x8000;
						hdr->EVENT.TYP[N_EVENT] = d2;
						++N_EVENT;
						if (d2==0x7ffe)
							fprintf(stdout,"Warning: BDF file %s uses ambigous code 0x7ffe; For details see file eventcodes.txt. \n",hdr->FileName);
					}
					d0 = d1;
				}

				free(Marker);
			}

		}	/* End reading EDF/BDF Status channel */

		ifseek(hdr, hdr->HeadLen, SEEK_SET);
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
			int k1;
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

					hdr->AS.bpb = 0;
					for (k=0;k<hdr->NS;k++)	{
						CHANNEL_TYPE *hc = hdr->CHANNEL+k;
						// initialize fields
					      	hc->Label[0]  = 0;
					      	strcpy(hc->Transducer, "EEG: Ag-AgCl electrodes");
					      	hc->PhysDimCode = 19+4256; // uV
					      	hc->PhysMax   = +100;
					      	hc->PhysMin   = -100;
					      	hc->DigMax    = +2047;
					      	hc->DigMin    = -2048;
					      	hc->GDFTYP    = 3;	// int16
					      	hc->SPR       = leu32p(b+k*BlockSize+20);
					      	hc->OnOff     = 1;
					      	hc->Notch     = 50;
					      	hc->Impedance = INF;
					      	hc->fZ        = NaN;
					      	hc->bi 	  = hdr->AS.bpb;
					      	hdr->AS.bpb += (GDFTYP_BITS[hc->GDFTYP]*hc->SPR)>>3;
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
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;

	    		uint8_t* Header2 = hdr->AS.Header+POS;
			hc->Transducer[0] = '\0';
			CHAN = leu16p(Header2+4);
			strncpy(hc->Label,(char*)Header2+6,min(MAX_LENGTH_LABEL,40));
			strncpy(tmp,(char*)Header2+68,20); tmp[20]=0;
			if (!strcmp(tmp,"Volts"))
				hc->PhysDimCode = 4256;
			else
				hc->PhysDimCode = PhysDimCode(tmp);

			hc->Off     = lef64p(Header2+52);
			hc->Cal     = lef64p(Header2+60);

			hc->OnOff   = 1;
			hc->SPR     = 1;
			if (hdr->VERSION >= 38.0) {
				hc->SPR = leu16p(Header2+250);  // used here as Divider
			}
			hdr->SPR = lcm(hdr->SPR, hc->SPR);

			ACQ_NoSamples[k] = leu32p(Header2+88);
			size_t tmp64 = leu32p(Header2+88) * hc->SPR;
			if (minBufLenXVarDiv > tmp64) minBufLenXVarDiv = tmp64;

			POS += leu32p((uint8_t*)Header2);
		}

		/// foreign data section - skip
		POS += leu16p(hdr->AS.Header+POS);

		size_t DataLen=0;
		for (k=0, hdr->AS.bpb=0; k<hdr->NS; k++)	{
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
			if (hdr->VERSION>=38.0)
				hc->SPR = hdr->SPR/hc->SPR;  // convert DIVIDER into SPR

			switch ((leu16p(hdr->AS.Header+POS+2)))	{
			case 1:
				hc->GDFTYP = 17;  // double
				DataLen += ACQ_NoSamples[k]<<3;
				hc->DigMax =  1e9;
				hc->DigMin = -1e9;
				break;
			case 2:
				hc->GDFTYP = 3;   // int
				DataLen += ACQ_NoSamples[k]<<1;
				hc->DigMax =  32767;
				hc->DigMin = -32678;
				break;
			default:
				B4C_ERRNUM = B4C_UNSPECIFIC_ERROR;
				B4C_ERRMSG = "SOPEN(ACQ-READ): invalid channel type.";
			};
			hc->PhysMax = hc->DigMax * hc->Cal + hc->Off;
			hc->PhysMin = hc->DigMin * hc->Cal + hc->Off;
					      	hc->bi 	  = hdr->AS.bpb;
		      	hdr->AS.bpb += (GDFTYP_BITS[hc->GDFTYP]*hc->SPR)>>3;
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

		hdr->SampleRate = atof(strtok(t1+7," "));
		hdr->SPR = 1;
		hdr->NS = 0;
		hdr->AS.bpb = 4;
		while (t) {
			int chno1=-1, chno2=-1;
			double f1,f2;
			char label[MAX_LENGTH_LABEL+1];

			sscanf(t,"%d %s %d %lf %lf",&chno1,label,&chno2,&f1,&f2);

			k = hdr->NS++;
			hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
			sprintf(hc->Label,"%s %03i",label,chno2);

			hc->LeadIdCode = 0;
			hc->SPR    = 1;
			hc->Cal    = f1*f2;
			hc->Off    = 0.0;
			hc->OnOff  = 1;
			hc->GDFTYP = 3;
			hc->DigMax =  32767;
			hc->DigMin = -32678;
			hc->PhysMax= hc->DigMax * hc->Cal + hc->Off;
			hc->PhysMin= hc->DigMin * hc->Cal + hc->Off;
			hc->bi  = hdr->AS.bpb;
			hdr->AS.bpb += 2;

			if (strcmp(label,"MEG")==0)
				hc->PhysDimCode = 1446; // "T/m"
			else
				hc->PhysDimCode = 4256; // "V"

		 	t = strtok(NULL,"\x0a\x0d");
		}

		/* open data file */
		strcpy(ext,"raw");
		struct stat FileBuf;
		stat(hdr->FileName,&FileBuf);

		hdr = ifopen(hdr,"rb");
		hdr->NRec = FileBuf.st_size/hdr->AS.bpb;
		hdr->HeadLen   = 0;
		// hdr->FLAG.SWAP = (__BYTE_ORDER == __LITTLE_ENDIAN);  	// AINF is big endian
		hdr->FILE.LittleEndian = 0;
		/* restore input file name, and free temporary file name  */
		hdr->FileName = filename;
		free(tmpfile);
	}

    	else if (hdr->TYPE==alpha) {
		ifclose(hdr); 	// close already opened file (typically its .../alpha.alp)
		sopen_alpha_read(hdr);
	}

    	else if ((hdr->TYPE==ASCII) || (hdr->TYPE==BIN)) {
		while (!ifeof(hdr)) {
			size_t bufsiz = 65536;
		    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,count+bufsiz+1);
		    	count  += ifread(hdr->AS.Header+count,1,bufsiz,hdr);
		}
		hdr->AS.Header[count]=0;
		hdr->HeadLen = count;
		ifclose(hdr);

		hdr->NS   = 0;
		hdr->NRec = 1;
		hdr->SPR  = 1;
		hdr->AS.bpb = 0;
		double Fs = 1.0;
		size_t N  = 0;
		char status = 0;
		char *val   = NULL;
		const char sep[] = " =\x09";
		double duration = 0;
		size_t lengthRawData = 0;
		uint8_t FLAG_NUMBER_OF_FIELDS_READ;	// used to trigger consolidation of channel info
		CHANNEL_TYPE *cp = NULL;
		char *datfile = NULL;
		uint16_t gdftyp = 0;

		char *line  = strtok((char*)hdr->AS.Header,"\x0a\x0d");
		while (line!=NULL) {

			if (VERBOSE_LEVEL>7)
				fprintf(stdout,"ASCII read line [%i]: <%s>\n",status,line);

			if (!strncmp(line,"[Header 1]",10))
				status = 1;
			else if (!strncmp(line,"[Header 2]",10)) {
				status = 2;
				hdr->NS = 0;
				FLAG_NUMBER_OF_FIELDS_READ=0;
			}
			else if (!strncmp(line,"[EVENT TABLE]",13)) {
				status = 3;
				hdr->EVENT.SampleRate = hdr->SampleRate;
				N = 0;
			}

			val = strchr(line,'=');
			if ((val != NULL) && (status<3)) {
				val += strspn(val,sep);
				size_t c;
				c = strspn(val,"#");
				if (c) val[c] = 0; // remove comments
				c = strcspn(line,sep);
				if (c) line[c] = 0; // deblank
				FLAG_NUMBER_OF_FIELDS_READ++;
			}
			if (VERBOSE_LEVEL>8) fprintf(stdout,"BIN <%s>=<%s> \n",line,val);

			if (status==1) {
				if (!strcmp(line,"Duration"))
					duration = atof(val);
				//else if (!strncmp(line,"NumberOfChannels"))
				else if (!strcmp(line,"Patient.Id"))
					strncpy(hdr->Patient.Id,val,MAX_LENGTH_NAME);
				else if (!strcmp(line,"Patient.Birthday")) {
					struct tm t;
					sscanf(val,"%04i-%02i-%02i %02i:%02i:%02i",&t.tm_year,&t.tm_mon,&t.tm_mday,&t.tm_hour,&t.tm_min,&t.tm_sec);
					t.tm_year -=1900;
					t.tm_mon--;
					t.tm_isdst = -1;
					hdr->Patient.Birthday = tm_time2gdf_time(&t);
				}
				else if (!strcmp(line,"Patient.Weight"))
					hdr->Patient.Weight = atoi(val);
				else if (!strcmp(line,"Patient.Height"))
					hdr->Patient.Height = atoi(val);
				else if (!strcmp(line,"Patient.Gender"))
					hdr->Patient.Sex = atoi(val);
				else if (!strcmp(line,"Patient.Handedness"))
					hdr->Patient.Handedness = atoi(val);
				else if (!strcmp(line,"Patient.Smoking"))
					hdr->Patient.Smoking = atoi(val);
				else if (!strcmp(line,"Patient.AlcoholAbuse"))
					hdr->Patient.AlcoholAbuse = atoi(val);
				else if (!strcmp(line,"Patient.DrugAbuse"))
					hdr->Patient.DrugAbuse = atoi(val);
				else if (!strcmp(line,"Patient.Medication"))
					hdr->Patient.Medication = atoi(val);
				else if (!strcmp(line,"Recording.ID"))
					strncpy(hdr->ID.Recording,val,MAX_LENGTH_RID);
				else if (!strcmp(line,"Recording.Time")) {
					struct tm t;
					sscanf(val,"%04i-%02i-%02i %02i:%02i:%02i",&t.tm_year,&t.tm_mon,&t.tm_mday,&t.tm_hour,&t.tm_min,&t.tm_sec);
					t.tm_year -= 1900;
					t.tm_mon--;
					t.tm_isdst = -1;
					hdr->T0 = tm_time2gdf_time(&t);
				}
				else if (!strcmp(line,"Recording.IPaddress")) {
					/* ###FIXME: IPv6 are currently not supported.
					 	gethostbyaddr will become obsolete,
					 	use getaddrinfo instead
					*/
#ifndef WITHOUT_NETWORK
#ifdef _WIN32
					WSADATA wsadata;
					WSAStartup(MAKEWORD(1,1), &wsadata);
#endif
					struct hostent *host = gethostbyaddr(val,strlen(val),AF_INET);
					// TODO: replace gethostbyaddr: getaddrinfo, getnameinfo()
					if (host!=NULL)
						memcpy(hdr->IPaddr, host->h_addr, host->h_length);
#ifdef _WIN32
					WSACleanup();
#endif
#endif // not WITHOUT_NETWORK
				}
				else if (!strcmp(line,"Recording.Technician"))
					strncpy(hdr->ID.Technician,val,MAX_LENGTH_TECHNICIAN);
				else if (!strcmp(line,"Manufacturer.Name"))
					hdr->ID.Manufacturer.Name = val;
				else if (!strcmp(line,"Manufacturer.Model"))
					hdr->ID.Manufacturer.Model = val;
				else if (!strcmp(line,"Manufacturer.Version"))
					hdr->ID.Manufacturer.Version = val;
				else if (!strcmp(line,"Manufacturer.SerialNumber"))
					hdr->ID.Manufacturer.SerialNumber = val;
			}

			else if (status==2) {
				if (!strcmp(line,"Filename")) {

					// add next channel
					++hdr->NS;
					hdr->CHANNEL = (CHANNEL_TYPE*)realloc(hdr->CHANNEL, hdr->NS*sizeof(CHANNEL_TYPE));
					cp = hdr->CHANNEL+hdr->NS-1;
					cp->bi = hdr->AS.bpb;
					cp->PhysDimCode = 0;
					cp->HighPass = NaN;
					cp->LowPass  = NaN;
					cp->Notch    = NaN;
					cp->Impedance= NaN;
					cp->fZ       = NaN;
					cp->LeadIdCode = 0;
					datfile      = val;

					FLAG_NUMBER_OF_FIELDS_READ = 1;

				}

				else if (!strcmp(line,"Label"))
					strncpy(cp->Label,val,MAX_LENGTH_LABEL);

				else if (!strcmp(line,"GDFTYP")) {
					if      (!strcmp(val,"int8"))	gdftyp = 1;
					else if (!strcmp(val,"uint8"))	gdftyp = 2;
					else if (!strcmp(val,"int16"))	gdftyp = 3;
					else if (!strcmp(val,"uint16"))	gdftyp = 4;
					else if (!strcmp(val,"int32"))	gdftyp = 5;
					else if (!strcmp(val,"uint32"))	gdftyp = 6;
					else if (!strcmp(val,"int64"))	gdftyp = 7;
					else if (!strcmp(val,"uint64"))	gdftyp = 8;
					else if (!strcmp(val,"float32"))	gdftyp = 16;
					else if (!strcmp(val,"float64"))	gdftyp = 17;
					else if (!strcmp(val,"float128"))	gdftyp = 18;
					else if (!strcmp(val,"ascii"))	gdftyp = 0xfffe;
					else 				gdftyp = atoi(val);

				}
				else if (!strcmp(line,"PhysicalUnits"))
					cp->PhysDimCode = PhysDimCode(val);
				else if (!strcmp(line,"PhysDimCode")) {
					// If PhysicalUnits and PhysDimCode conflict, PhysicalUnits gets the preference
					if (!cp->PhysDimCode)
						cp->PhysDimCode = atoi(val);
				}
				else if (!strcmp(line,"Transducer"))
					strncpy(cp->Transducer,val,MAX_LENGTH_TRANSDUCER);

				else if (!strcmp(line,"SamplingRate"))
					Fs = atof(val);

				else if (!strcmp(line,"NumberOfSamples")) {
					cp->SPR = atol(val);
					if (cp->SPR>0) hdr->SPR = lcm(hdr->SPR,cp->SPR);

					if ((gdftyp>0) && (gdftyp<256)) {
						cp->GDFTYP = gdftyp;

						FILE *fid = fopen(datfile,"rb");
						if (fid != NULL) {
							size_t bufsiz = cp->SPR*GDFTYP_BITS[cp->GDFTYP]>>3;
							hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata,lengthRawData+bufsiz+1);
							count = fread(hdr->AS.rawdata+lengthRawData,1,bufsiz+1,fid);
							if (count != bufsiz)
								fprintf(stderr,"Warning SOPEN(BIN) #%i: mismatch between sample number and file size (%i,%i)\n",hdr->NS-1,count,bufsiz);
							lengthRawData += bufsiz;
							fclose(fid);
						}
						else if (cp->SPR > 0) {
							cp->SPR = 0;
							fprintf(stderr,"Warning SOPEN(BIN) #%i: data file (%s) not found\n",datfile);
						}
					}
					else if (gdftyp==0xfffe) {
						cp->GDFTYP = 17;	// double

						struct stat FileBuf;
						stat(datfile, &FileBuf);

						FILE *fid = fopen(datfile,"rb");
						if (fid != NULL) {
							char *buf = (char*)malloc(FileBuf.st_size+1);
							count = fread(buf, 1, FileBuf.st_size, fid);
							fclose(fid);
							buf[count] = 0;

							size_t sz = GDFTYP_BITS[cp->GDFTYP]>>3;
							const size_t bufsiz = cp->SPR * sz;
							hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata, lengthRawData+bufsiz);

							char *bufbak  = buf; 	// backup copy
							char **endptr = &bufbak;
							for (k = 0; k < cp->SPR; k++) {
								double d = strtod(*endptr,endptr);
								*(double*)(hdr->AS.rawdata+lengthRawData+sz*k) = d;
							}
							lengthRawData += bufsiz;
							free(buf);
						}
						else if (cp->SPR > 0) {
							cp->SPR = 0;
							fprintf(stderr,"Warning SOPEN(BIN) #%i: data file (%s) not found\n",datfile);
						}
					}
					else {
						B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
						B4C_ERRMSG = "ASCII/BIN: data type unsupported";
					}
					hdr->AS.bpb  = lengthRawData;
				}
				else if (!strcmp(line,"HighPassFilter"))
					cp->HighPass = atof(val);
				else if (!strcmp(line,"LowPassFilter"))
					cp->LowPass = atof(val);
				else if (!strcmp(line,"NotchFilter"))
					cp->Notch = atof(val);
				else if (!strcmp(line,"DigMax"))
					cp->DigMax = atof(val);
				else if (!strcmp(line,"DigMin"))
					cp->DigMin = atof(val);
				else if (!strcmp(line,"PhysMax"))
					cp->PhysMax = atof(val);
				else if (!strcmp(line,"PhysMin"))
					cp->PhysMin = atof(val);
				else if (!strcmp(line,"Impedance"))
					cp->Impedance = atof(val);
				else if (!strcmp(line,"freqZ"))
					cp->fZ = atof(val);
				else if (!strncmp(line,"Position",8)) {
					sscanf(val,"%f \t%f \t%f",cp->XYZ,cp->XYZ+1,cp->XYZ+2);

					// consolidate previous channel
					if (((GDFTYP_BITS[cp->GDFTYP]*cp->SPR >> 3) != (hdr->AS.bpb-cp->bi)) && (hdr->TYPE==BIN)) {
						fprintf(stdout,"Warning SOPEN(BIN): problems with channel %i - filesize %i does not fit header info %i\n",k+1, hdr->AS.bpb-hdr->CHANNEL[k].bi,GDFTYP_BITS[hdr->CHANNEL[k].GDFTYP]*hdr->CHANNEL[k].SPR >> 3);
					}

					hdr->SampleRate = hdr->SPR/duration;
					cp->LeadIdCode = 0;
					cp->OnOff = 1;
					cp->Cal = (cp->PhysMax - cp->PhysMin) / (cp->DigMax - cp->DigMin);
					cp->Off =  cp->PhysMin - cp->Cal*cp->DigMin;
				}
			}

			else if (status==3) {
				if (!strncmp(line,"0x",2)) {

					if (hdr->EVENT.N+1 >= N) {
						N += 4096;
				 		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, N*sizeof(*hdr->EVENT.POS) );
						hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, N*sizeof(*hdr->EVENT.TYP) );
						hdr->EVENT.DUR = (uint32_t*) realloc(hdr->EVENT.DUR, N*sizeof(*hdr->EVENT.DUR));
						hdr->EVENT.CHN = (uint16_t*) realloc(hdr->EVENT.CHN, N*sizeof(*hdr->EVENT.CHN));
					}

					val = line+2;
					int i;
					sscanf(val,"%04x",&i);
					if (i>0xffff)
						fprintf(stdout,"Warning: Type %i of event %i does not fit in 16bit\n",i,hdr->EVENT.N);
					else
						hdr->EVENT.TYP[hdr->EVENT.N] = (typeof(hdr->EVENT.TYP[0]))i;

					double d;
					val = strchr(val,'\t')+1;
					sscanf(val,"%lf",&d);

					hdr->EVENT.POS[hdr->EVENT.N] = (typeof(*hdr->EVENT.POS))round(d*hdr->EVENT.SampleRate);  // 0-based indexing

					val = strchr(val,'\t')+1;
					if (val[0]!='\t') {
						sscanf(val,"%lf",&d);
						hdr->EVENT.DUR[hdr->EVENT.N] = (typeof(*hdr->EVENT.POS))round(d*hdr->EVENT.SampleRate);
					}
					else
						hdr->EVENT.DUR[hdr->EVENT.N] = 0;

					val = strchr(val,'\t')+1;
					if (val[0]!='\t') {
						sscanf(val,"%d",&i);
						if (i>0xffff)
							fprintf(stdout,"Warning: channel number %i of event %i does not fit in 16bit\n",i,hdr->EVENT.N);
						else
							hdr->EVENT.CHN[hdr->EVENT.N] = i;
					}
					else
						hdr->EVENT.CHN[hdr->EVENT.N] = 0;

					val = strchr(val,'\t')+1;
					if ((hdr->EVENT.TYP[hdr->EVENT.N]==0x7fff) && (hdr->EVENT.CHN[hdr->EVENT.N]>0) && (!hdr->CHANNEL[hdr->EVENT.CHN[hdr->EVENT.N]-1].SPR)) {
						sscanf(val,"%d",&hdr->EVENT.DUR[hdr->EVENT.N]);
					}
					++hdr->EVENT.N;
				}
			}
			line = strtok(NULL,"\x0a\x0d");
		}
		hdr->AS.length = hdr->NRec;
    	}

	else if (hdr->TYPE==BCI2000) {
		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"[201] start reading BCI2000 data!\n");

		char *ptr, *t1;

		/* decode header length */
		hdr->HeadLen = 0;
		ptr = strstr((char*)hdr->AS.Header,"HeaderLen=");
		if (ptr==NULL)
			B4C_ERRNUM = B4C_FORMAT_UNKNOWN;
		else {
			/* read whole header */
			hdr->HeadLen = (typeof(hdr->HeadLen)) strtod(ptr+10,&ptr);
			if (count <= hdr->HeadLen) {
				hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, hdr->HeadLen+1);
				count   += ifread(hdr->AS.Header+count,1,hdr->HeadLen-count,hdr);
			}
			else
				ifseek(hdr,hdr->HeadLen,SEEK_SET);
		}
		hdr->AS.Header[hdr->HeadLen]=0;
		hdr->AS.bci2000 = (char*)realloc(hdr->AS.bci2000, hdr->HeadLen+1);
		memcpy(hdr->AS.bci2000, hdr->AS.Header, hdr->HeadLen+1);

		/* decode number of channels */
		t1  = strtok((char*)hdr->AS.Header,"\x0a\x0d");
		ptr = strstr(t1,"SourceCh=");
		if (ptr==NULL)
			B4C_ERRNUM = B4C_FORMAT_UNKNOWN;
		else
			hdr->NS = (typeof(hdr->NS)) strtod(ptr+9,&ptr);

		/* decode length of state vector */
		ptr = strstr(t1,"StatevectorLen=");
		if (ptr==NULL)
			B4C_ERRNUM = B4C_FORMAT_UNKNOWN;
		else
		    	BCI2000_StatusVectorLength = (size_t) strtod(ptr+15,&ptr);

		/* decode data format */
		ptr = strstr(ptr,"DataFormat=");
		uint16_t gdftyp=3;
		if (ptr == NULL) gdftyp = 3;
		else if (!strncmp(ptr+12,"int16",3))	gdftyp = 3;
		else if (!strncmp(ptr+12,"int32",5))	gdftyp = 5;
		else if (!strncmp(ptr+12,"float32",5))	gdftyp = 16;
		else if (!strncmp(ptr+12,"int24",5))	gdftyp = 255+24;
		else if (!strncmp(ptr+12,"uint16",3))	gdftyp = 4;
		else if (!strncmp(ptr+12,"uint32",5))	gdftyp = 6;
		else if (!strncmp(ptr+12,"uint24",5))	gdftyp = 511+24;
		else if (!strncmp(ptr+12,"float64",6))	gdftyp = 17;
		else B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;

		if (B4C_ERRNUM) {
			/* return if any error has occured. */
			B4C_ERRMSG = "ERROR 1234 SOPEN(BCI2000): invalid file format .\n";
			return(hdr);
		}

		if (hdr->FLAG.OVERFLOWDETECTION) {
			fprintf(stderr,"WARNING: Automated overflowdetection not supported in BCI2000 file %s\n",hdr->FileName);
			hdr->FLAG.OVERFLOWDETECTION = 0;
		}

		hdr->SPR = 1;
		double gain=0.0, offset=0.0, digmin=0.0, digmax=0.0;
		size_t tc_len=0,tc_pos=0, rs_len=0,rs_pos=0, fb_len=0,fb_pos=0;
		char TargetOrientation=0;

		hdr->AS.bpb = 0;
		hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
			sprintf(hc->Label,"#%03i",k+1);
			hc->Cal    = gain;
			hc->Off    = offset;
			hc->PhysDimCode = 4275; // uV
			hc->LeadIdCode = 0;
			hc->OnOff  = 1;
			hc->SPR    = 1;
			hc->GDFTYP = gdftyp;
			hc->bi     = hdr->AS.bpb;
			hdr->AS.bpb    += (GDFTYP_BITS[hc->GDFTYP] * hc->SPR)>>3;
		}
		if (hdr->TYPE==BCI2000)
			hdr->AS.bpb += BCI2000_StatusVectorLength;

		int status = 0;
		ptr = strtok(NULL,"\x0a\x0d");
		while (ptr != NULL) {

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"[203] %i:  %s !\n",status,ptr);

			if (!strncmp(ptr,"[ State Vector Definition ]",26))
				status = 1;
			else if (!strncmp(ptr,"[ Parameter Definition ]",24))
				status = 2;
			else if (!strncmp(ptr,"[ ",2))
				status = 3;

			else if (status==1) {
				int  i[4];
				char item[20];
				sscanf(ptr,"%s %i %i %i %i",item,i,i+1,i+2,i+3);
				if (!strcmp(item,"TargetCode")) {
					tc_pos = i[2]*8 + i[3];
					tc_len = i[0];
				}
				else if (!strcmp(item,"ResultCode")) {
					rs_pos = i[2]*8 + i[3];
					rs_len = i[0];
				}
				else if (!strcmp(item,"Feedback")) {
					fb_pos = i[2]*8 + i[3];
					fb_len = i[0];
				}
			}

			else if (status==2) {
				t1 = strstr(ptr,"ChannelNames=");
				if (t1 != NULL) {
		    			unsigned NS = (unsigned)strtod(t1+13,&ptr);
		    			for (k=0; k<NS; k++) {
		    				while (isspace(ptr[0])) ++ptr;
		    				int k1=0;
		    				while (!isspace(ptr[k1])) ++k1;
		    				ptr += k1;
		    				if (k1>MAX_LENGTH_LABEL) k1=MAX_LENGTH_LABEL;
		    				strncpy(hdr->CHANNEL[k].Label,ptr-k1,k1);
						hdr->CHANNEL[k].Label[k1]=0; 	// terminating 0
		    			}
		    		}

				t1 = strstr(ptr,"SamplingRate=");
				if (t1 != NULL)	hdr->SampleRate = strtod(t1+14,&ptr);

				t1 = strstr(ptr,"SourceChGain=");
				if (t1 != NULL) {
		    			unsigned NS = (unsigned) strtod(t1+13,&ptr);
		    			for (k=0; k<NS; k++) hdr->CHANNEL[k].Cal = strtod(ptr,&ptr);
		    			for (; k<hdr->NS; k++) hdr->CHANNEL[k].Cal = hdr->CHANNEL[k-1].Cal;
		    		}
				t1 = strstr(ptr,"SourceChOffset=");
				if (t1 != NULL) {
		    			unsigned NS = (unsigned) strtod(t1+15,&ptr);
		    			for (k=0; k<NS; k++) hdr->CHANNEL[k].Off = strtod(ptr,&ptr);
		    			for (; k<hdr->NS; k++) hdr->CHANNEL[k].Off = hdr->CHANNEL[k-1].Off;
		    		}
				t1 = strstr(ptr,"SourceMin=");
				if (t1 != NULL)	digmin = strtod(t1+10,&ptr);

				t1 = strstr(ptr,"SourceMax=");
				if (t1 != NULL) digmax = strtod(t1+10,&ptr);

				t1 = strstr(ptr,"StorageTime=");
				if (t1 != NULL) {
					char *t2 = strstr(t1,"%20");
					while (t2!=NULL) {
						memset(t2,' ',3);
						t2 = strstr(t1,"%20");
					}
					int c=sscanf(t1+12,"%03s %03s %2u %2u:%2u:%2u %4u",tmp+10,tmp,&tm_time.tm_mday,&tm_time.tm_hour,&tm_time.tm_min,&tm_time.tm_sec,&tm_time.tm_year);
					if (c==7) {
						tm_time.tm_isdst = -1;
						tm_time.tm_year -= 1900;
						if      (!strcmp(tmp,"Jan")) tm_time.tm_mon = 0;
						else if (!strcmp(tmp,"Feb")) tm_time.tm_mon = 1;
						else if (!strcmp(tmp,"Mar")) tm_time.tm_mon = 2;
						else if (!strcmp(tmp,"Apr")) tm_time.tm_mon = 3;
						else if (!strcmp(tmp,"May")) tm_time.tm_mon = 4;
						else if (!strcmp(tmp,"Jun")) tm_time.tm_mon = 5;
						else if (!strcmp(tmp,"Jul")) tm_time.tm_mon = 6;
						else if (!strcmp(tmp,"Aug")) tm_time.tm_mon = 7;
						else if (!strcmp(tmp,"Sep")) tm_time.tm_mon = 8;
						else if (!strcmp(tmp,"Oct")) tm_time.tm_mon = 9;
						else if (!strcmp(tmp,"Nov")) tm_time.tm_mon = 10;
						else if (!strcmp(tmp,"Dec")) tm_time.tm_mon = 11;
						hdr->T0 = tm_time2gdf_time(&tm_time);
					}
				}
				t1 = strstr(ptr,"TargetOrientation=");
				if (t1 != NULL)	TargetOrientation = (char) strtod(t1+18, &ptr);

			// else if (status==3);

			}
			ptr = strtok(NULL,"\x0a\x0d");
		}

		for (k=0; k<hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
			hc->DigMax = digmax;
			hc->DigMin = digmin;
			hc->PhysMax= hc->DigMax * hc->Cal + hc->Off;
			hc->PhysMin= hc->DigMin * hc->Cal + hc->Off;
		}
		hdr->AS.bpb = (hdr->NS * (GDFTYP_BITS[gdftyp]>>3) + BCI2000_StatusVectorLength);

		/* decode state vector into event table */
		hdr->EVENT.SampleRate = hdr->SampleRate;
	        size_t skip = hdr->NS * (GDFTYP_BITS[gdftyp]>>3);
	        size_t N = 0;
	        count 	 = 0;
	        uint8_t *StatusVector = (uint8_t*) malloc(BCI2000_StatusVectorLength*2);
		uint32_t b0=0,b1=0,b2=0,b3,b4=0,b5;
	        while (!ifeof(hdr)) {
		        ifseek(hdr, skip, SEEK_CUR);
			ifread(StatusVector + BCI2000_StatusVectorLength*(count & 1), 1, BCI2000_StatusVectorLength, hdr);
			if (memcmp(StatusVector, StatusVector+BCI2000_StatusVectorLength, BCI2000_StatusVectorLength)) {
				if (N+4 >= hdr->EVENT.N) {
					hdr->EVENT.N  += 1024;
					hdr->EVENT.POS = (uint32_t*)realloc(hdr->EVENT.POS, hdr->EVENT.N*sizeof(*hdr->EVENT.POS));
					hdr->EVENT.TYP = (uint16_t*)realloc(hdr->EVENT.TYP, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP));
				}

				/*
					event codes according to
					http://www.bci2000.org/wiki/index.php/User_Reference:GDFFileWriter
					http://biosig.cvs.sourceforge.net/biosig/biosig/doc/eventcodes.txt?view=markup
				*/

				/* decode ResultCode */
				b3 = *(uint32_t*)(StatusVector + BCI2000_StatusVectorLength*(count & 1) + (rs_pos>>3));
				b3 = (b3 >> (rs_pos & 7)) & ((1<<rs_len)-1);
				if (b3 != b2) {
					if (b3>b2) hdr->EVENT.TYP[N] = ( b3==b1 ? 0x0381 : 0x0382);
					else 	   hdr->EVENT.TYP[N] = ( b2==b0 ? 0x8381 : 0x8382);
					hdr->EVENT.POS[N] = count;        // 0-based indexing
					N++;
					b2 = b3;
				}

				/* decode TargetCode */
				b1 = *(uint32_t*)(StatusVector + BCI2000_StatusVectorLength*(count & 1) + (tc_pos>>3));
				b1 = (b1 >> (tc_pos & 7)) & ((1<<tc_len)-1);
				if (b1 != b0) {
					if (TargetOrientation==1) {	// vertical
						switch ((int)b1-(int)b0) {
						case  1: hdr->EVENT.TYP[N] = 0x030c; break;
						case  2: hdr->EVENT.TYP[N] = 0x0306; break;
						case -1: hdr->EVENT.TYP[N] = 0x830c; break;
						case -2: hdr->EVENT.TYP[N] = 0x8306; break;
						default:
							if (b1>b0) hdr->EVENT.TYP[N] = 0x0300 + b1 - b0;
							else       hdr->EVENT.TYP[N] = 0x8300 + b0 - b1;
						}
					}
					else {
						if (b1>b0) hdr->EVENT.TYP[N] = 0x0300 + b1 - b0;
						else       hdr->EVENT.TYP[N] = 0x8300 + b0 - b1;
					}

					hdr->EVENT.POS[N] = count;        // 0-based indexing
					N++;
					b0 = b1;
				}

				/* decode Feedback */
				b5 = *(uint32_t*)(StatusVector + BCI2000_StatusVectorLength*(count & 1) + (fb_pos>>3));
				b5 = (b5 >> (fb_pos & 7)) & ((1<<fb_len)-1);
				if (b5 > b4)
					hdr->EVENT.TYP[N] = 0x030d;
				else if (b5 < b4)
					hdr->EVENT.TYP[N] = 0x830d;
				if (b5 != b4) {
					hdr->EVENT.POS[N] = count;        // 0-based indexing
					N++;
					b4 = b5;
				}
			}
			count++;
		}
		hdr->EVENT.N = N;
		free(StatusVector);
		hdr->NRec = (iftell(hdr) - hdr->HeadLen) / hdr->AS.bpb;
	        ifseek(hdr, hdr->HeadLen, SEEK_SET);

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[209] header finished!\n");
	}

	else if (hdr->TYPE==BKR) {

		if (VERBOSE_LEVEL>8) fprintf(stdout,"libbiosig/sopen (BKR)\n");

	    	hdr->HeadLen 	 = 1024;
	    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, hdr->HeadLen);
	    	count   += ifread(hdr->AS.Header+count,1,hdr->HeadLen-count,hdr);
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
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
			sprintf(hc->Label,"# %02i",k);
			hc->Transducer[0] = '\0';
		    	hc->GDFTYP 	= 3;
		    	hc->SPR 	= 1; // *(int32_t*)(Header1+56);
		    	hc->LowPass	= lef32p(hdr->AS.Header+22);
		    	hc->HighPass = lef32p(hdr->AS.Header+26);
		    	hc->Notch	= -1.0;  // unknown
		    	hc->PhysMax	= (double)leu16p(hdr->AS.Header+14);
		    	hc->DigMax	= (double)leu16p(hdr->AS.Header+16);
		    	hc->PhysMin	= -hc->PhysMax;
		    	hc->DigMin	= -hc->DigMax;
		    	hc->Cal	 	= hc->PhysMax/hc->DigMax;
		    	hc->Off	 	= 0.0;
			hc->OnOff    	= 1;
		    	hc->PhysDimCode = 4275; // uV
		    	hc->LeadIdCode  = 0;
		    	hc->bi      	= k*2;
		}
		hdr->AS.bpb = hdr->NS*2;
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// BKR does not support automated overflow and saturation detection
	}

	else if (hdr->TYPE==BLSC) {
		hdr->HeadLen = hdr->AS.Header[1]<<7;
		if (count<hdr->HeadLen) {
		    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, hdr->HeadLen);
		    	count   += ifread(hdr->AS.Header+count,1,hdr->HeadLen-count,hdr);
		}

		hdr->VERSION  = leu16p(hdr->AS.Header+2)/100.0;
		hdr->SampleRate = 128;
		hdr->SPR = 1;
		hdr->NS  = hdr->AS.Header[346];

		const uint32_t GAIN[] = {
			0,50000,75000,100000,150000,200000,250000,300000,  //0-7
			0,5000,7500,10000,15000,20000,25000,30000,  //8-15
			0,500,750,1000,1500,2000,2500,3000,  //16-23
			10,50,75,100,150,200,250,300  //24-31
			};

	    	hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
			hc->Label[0] 	= 0;
			hc->Transducer[0] = '\0';
		    	hc->GDFTYP 	= 2;
		    	hc->SPR 	= hdr->SPR; // *(int32_t*)(Header1+56);
		    	hc->LowPass	= -1.0;
		    	hc->HighPass 	= -1.0;
    			hc->Notch	= -1.0;  // unknown
		    	hc->DigMax	= 255;
		    	hc->DigMin	= 0;

#define SENS  	leu16p(hdr->AS.Header+467)
#define CALUV 	leu16p(hdr->AS.Header+469)
#define CV 	hdr->AS.Header[425+k]
#define DC 	hdr->AS.Header[446+k]
#define gain 	GAIN[hdr->AS.Header[602+k]]

if (VERBOSE_LEVEL>8)
	fprintf(stdout,"#%i sens=%i caluv=%i cv=%i dc=%i Gain=%i\n",k,SENS,CALUV,CV,DC,gain);

			double cal, off;
		    	if (hdr->AS.Header[5]==0) {
		    		// external amplifier
				cal = 0.2*CALUV*SENS/CV;
				off = -DC*cal;
		    	}
		    	else {
		    		// internal amplifier
			    	cal = 4e6/(CV*gain);
				off = -(128+(DC-128)*gain/3e5)*cal;
			}

#undef SENS
#undef CALUV
#undef CV
#undef DC
#undef gain

		    	hc->Cal	 = cal;
		    	hc->Off	 = off;
		    	hc->PhysMax	 = hc->DigMax * cal + off;
		    	hc->PhysMin	 = hc->DigMin * cal + off;
			hc->OnOff    = 1;
		    	hc->PhysDimCode = 4275; // uV
    			hc->LeadIdCode  = 0;
    			hc->bi 	= k*hc->SPR*GDFTYP_BITS[2]>>3;
		}
		hdr->AS.bpb     = hdr->NS*hdr->SPR*GDFTYP_BITS[2]>>3;

		struct stat FileBuf;
		stat(hdr->FileName,&FileBuf);
		hdr->NRec = FileBuf.st_size/hdr->NS;
	        ifseek(hdr, hdr->HeadLen, SEEK_SET);

	}

	else if (hdr->TYPE==BNI) {
		// BNI-1-Baltimore/Nicolet
		char *line = strtok((char*)hdr->AS.Header,"\x0a\x0d");
		fprintf(stdout,"Warning SOPEN: BNI not implemented - experimental code!\n");
		double cal=0,age;
		char *Label=NULL;
		struct tm t;
		while (line != NULL) {
			size_t c1 = strcspn(line," =");
			size_t c2 = strspn(line+c1," =");
			char *val = line+c1+c2;
			if (!strncmp(line,"PatientId",9))
				strncpy(hdr->Patient.Id,val,MAX_LENGTH_PID);
			else if (!strncmpi(line,"Sex",3))
				hdr->Patient.Sex = 1*(toupper(val[0])=='M')+2*(toupper(val[0])=='F');
			else if (!strncmpi(line,"medication",11))
				hdr->Patient.Medication = val==NULL ? 1 : 2;
			else if (!strncmpi(line,"diagnosis",10)) {
			}
			else if (!strncmpi(line,"MontageRaw",9))
				Label = val;
			else if (!strncmpi(line,"Age",3))
				age = atol(val);
			else if (!strncmpi(line,"Date",c1))
				sscanf(val,"%02i/%02i/%02i",&t.tm_mon,&t.tm_mday,&t.tm_year);
			else if (!strncmpi(line,"Time",c1))
				sscanf(val,"%02i:%02i:%02i",&t.tm_hour,&t.tm_min,&t.tm_sec);
			else if (!strncmpi(line,"Rate",c1))
				hdr->SampleRate = atol(val);
			else if (!strncmpi(line,"NchanFile",9))
				hdr->NS = atol(val);
			else if (!strncmpi(line,"UvPerBit",c1))
				cal = atof(val);
			else if (!strncmpi(line,"[Events]",c1)) {
				// not implemented yet
			}
			else
				fprintf(stdout,"SOPEN(BNI): unknown field %s=%s\n",line,val);

			line = strtok(NULL,"\x0a\x0d");
		}
		hdr->T0 = tm_time2gdf_time(&t);
	    	hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
			if (!k) strncpy(hc->Label, strtok(Label,","),MAX_LENGTH_LABEL);
			else 	strncpy(hc->Label, strtok(NULL,","),MAX_LENGTH_LABEL);

			hc->Transducer[0] = '\0';
		    	hc->GDFTYP 	= 0xffff;	// unknown - triggers error status
		    	hc->SPR 	= 1; //
		    	hc->LowPass	= -1.0;
		    	hc->HighPass 	= -1.0;
    			hc->Notch	= -1.0;  // unknown
		    	hc->DigMax	= 32767;
		    	hc->DigMin	= -32768;

		    	hc->Cal	 	= cal;
		    	hc->Off	 	= 0.0;
		    	hc->PhysMax	= hc->DigMax * cal;
		    	hc->PhysMin	= hc->DigMin * cal;
			hc->OnOff    	= 1;
		    	hc->PhysDimCode = 4275; // uV
    			hc->LeadIdCode  = 0;
    			//hc->bi 	= k*GDFTYP_BITS[hc->GDFTYP]>>3;
		}
	}

	else if (hdr->TYPE==BrainVisionMarker) {

		while (!ifeof(hdr)) {
			size_t bufsiz = 4096;
		    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,count+bufsiz+1);
		    	count  += ifread(hdr->AS.Header+count,1,bufsiz,hdr);
		}
		hdr->AS.Header[count]=0;
		hdr->HeadLen = count;
		ifclose(hdr);

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"SOPEN(BV): marker file read.\n");

		int seq = 0;
		/* decode marker file */


		char *t,*t1="    ";;
		t  = Header1;
		t += strcspn(Header1,"\x0A\x0D");
		t += strspn(t,"\x0A\x0D");
		//char *t1 = strtok(Header1,"\x0A\x0D");
		// skip first line
		size_t N_EVENT=0;
		hdr->EVENT.N=0;
		do {
			t1 = t;
			t += strcspn(t,"\x0A\x0D");
			t += strspn(t,"\x0A\x0D");
			t[-1]=0;

			if (VERBOSE_LEVEL>8) fprintf(stdout,"%i <%s>\n",seq,t1);

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

			if (VERBOSE_LEVEL>8) fprintf(stdout,"  %i %i %i %i %i %i \n",p1,p2,p3,p4,p5,p6);

				t1[p1]=0;
				t1[p2]=0;
				t1[p3]=0;
				t1[p4]=0;
				t1[p5]=0;

				if (hdr->EVENT.N <= N_EVENT) {
					hdr->EVENT.N  += 256;
			 		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, hdr->EVENT.N*sizeof(*hdr->EVENT.POS));
					hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP));
					hdr->EVENT.DUR = (uint32_t*) realloc(hdr->EVENT.DUR, hdr->EVENT.N*sizeof(*hdr->EVENT.DUR));
					hdr->EVENT.CHN = (uint16_t*) realloc(hdr->EVENT.CHN, hdr->EVENT.N*sizeof(*hdr->EVENT.CHN));
				}
				hdr->EVENT.TYP[N_EVENT] = atol(t1+p2+2);
				hdr->EVENT.POS[N_EVENT] = atol(t1+p3+1)-1;        // 0-based indexing
				hdr->EVENT.DUR[N_EVENT] = atol(t1+p4+1);
				hdr->EVENT.CHN[N_EVENT] = atol(t1+p5+1);
				if (!strncmp(t1+p1+1,"New Segment",11)) {
					hdr->EVENT.TYP[N_EVENT] = 0x7ffe;

					char* t2 = t1+p6+1;
					t2[14]=0;	tm_time.tm_sec  = atoi(t2+12);
					t2[12]=0;	tm_time.tm_min  = atoi(t2+10);
					t2[10]=0;	tm_time.tm_hour = atoi(t2+8);
					t2[8] =0;	tm_time.tm_mday = atoi(t2+6);
					t2[6] =0;	tm_time.tm_mon  = atoi(t2+4)-1;
					t2[4] =0;	tm_time.tm_year = atoi(t2)-1900;
					hdr->T0 = tm_time2gdf_time(&tm_time);
				}
				else {
					if (VERBOSE_LEVEL>8) fprintf(stdout,"#%02i <%s>\n",N_EVENT,t1+p2+1);
					FreeTextEvent(hdr,N_EVENT,t1+p2+1);
				}

				++N_EVENT;
			}
		}
		while (strlen(t1)>0);

		// free(vmrk);
		hdr->AS.auxBUF = hdr->AS.Header;
		hdr->AS.Header = NULL;
		hdr->EVENT.N   = N_EVENT;
		hdr->TYPE      = EVENT;

	}

	else if ((hdr->TYPE==BrainVision) || (hdr->TYPE==BrainVisionVAmp)) {
		/* open and read header file */
		// ifclose(hdr);
		const char *filename = hdr->FileName; // keep input file name
		char* tmpfile = (char*)calloc(strlen(hdr->FileName)+5,1);
		strcpy(tmpfile,hdr->FileName);
		hdr->FileName = tmpfile;
		char* ext = strrchr((char*)hdr->FileName,'.')+1;

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

		/* decode header information */
		hdr->FLAG.OVERFLOWDETECTION = 0;
		seq = 0;
		uint16_t gdftyp=3;
		char FLAG_ASCII = 0;
		hdr->FILE.LittleEndian = 1;	// default little endian
		double physmax=1e6,physmin=-1e6,digmax=1e6,digmin=-1e6,cal=1.0,off=0.0;
		enum o_t{VEC,MUL} orientation = MUL;
		char DECIMALSYMBOL='.';
		int  SKIPLINES=0, SKIPCOLUMNS=0;
		size_t npts=0;

		char *t;
		size_t pos;
		// skip first line with <CR><LF>
		const char EOL[] = "\r\n";
		pos  = strcspn(Header1,EOL);
		pos += strspn(Header1+pos,EOL);
		while (pos < hdr->HeadLen) {
			t    = Header1+pos;	// start of line
			pos += strcspn(t,EOL);
			Header1[pos] = 0;	// line terminator
			pos += strspn(Header1+pos+1,EOL)+1; // skip <CR><LF>

			if (VERBOSE_LEVEL>8) fprintf(stdout,"[212]: %i pos=%i <%s>, ERR=%i\n",seq,pos,t,B4C_ERRNUM);

			if (!strncmp(t,";",1)) 	// comments
				;
			else if (!strncmp(t,"[Common Infos]",14))
				seq = 1;
			else if (!strncmp(t,"[Binary Infos]",14))
				seq = 2;
			else if (!strncmp(t,"[ASCII Infos]",13)) {
				seq = 2;
				FLAG_ASCII = 1;
				gdftyp = 17;

//				B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
//				B4C_ERRMSG = "Error SOPEN(BrainVision): ASCII-format not supported (yet).";
			}
			else if (!strncmp(t,"[Channel Infos]",14)) {
				seq = 3;

				/* open data file */
				if (FLAG_ASCII) hdr = ifopen(hdr,"rt");
				else 	        hdr = ifopen(hdr,"rb");

				hdr->AS.bpb = (hdr->NS*GDFTYP_BITS[gdftyp])>>3;
				if (hdr->TYPE==BrainVisionVAmp) hdr->AS.bpb += 4;
				if (!npts) {
					struct stat FileBuf;
					stat(hdr->FileName,&FileBuf);
					npts = FileBuf.st_size/hdr->AS.bpb;
		        	}

				/* restore input file name, and free temporary file name  */
				hdr->FileName = filename;
				free(tmpfile);

				if (orientation == VEC) {
					hdr->SPR = npts;
					hdr->NRec= 1;
					hdr->AS.bpb*= hdr->SPR;
				} else {
					hdr->SPR = 1;
					hdr->NRec= npts;
				}

			    	hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
				for (k=0; k<hdr->NS; k++) {
					CHANNEL_TYPE *hc = hdr->CHANNEL+k;
					hc->Label[0] = 0;
					hc->Transducer[0] = '\0';
				    	hc->GDFTYP 	= gdftyp;
				    	hc->SPR 	= hdr->SPR; // *(int32_t*)(Header1+56);
				    	hc->LowPass	= -1.0;
				    	hc->HighPass	= -1.0;
		    			hc->Notch	= -1.0;  // unknown
				    	hc->PhysMax	= physmax;
				    	hc->DigMax	= digmax;
				    	hc->PhysMin	= physmin;
				    	hc->DigMin	= digmin;
				    	hc->Impedance	= NaN;
				    	hc->Cal		= cal;
				    	hc->Off		= off;
					hc->OnOff   	= 1;
				    	hc->PhysDimCode = 4275; // uV
		    			hc->LeadIdCode  = 0;
					hc->bi8         = k*hdr->SPR*GDFTYP_BITS[gdftyp];
					hc->bi          = hc->bi8>>3;
				}

				if (VERBOSE_LEVEL>7) fprintf(stdout,"BVA210 seq=%i,pos=%i,%i <%s> bpb=%i\n",seq,pos,hdr->HeadLen,t,hdr->AS.bpb);
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

				else if (!strncmp(t,"MarkerFile=",11)) {

					char* mrkfile = (char*)calloc(strlen(hdr->FileName)+strlen(t),1);

					if (strrchr(hdr->FileName,FILESEP)) {
						strcpy(mrkfile,hdr->FileName);
						strcpy(strrchr(mrkfile,FILESEP)+1,t+11);
					} else
						strcpy(mrkfile,t+11);

					if (VERBOSE_LEVEL>8)
						fprintf(stdout,"SOPEN marker file <%s>.\n",mrkfile);

					HDRTYPE *hdr2 = sopen(mrkfile,"r",NULL);

					hdr->T0 = hdr2->T0;
					memcpy(&hdr->EVENT,&hdr2->EVENT,sizeof(hdr2->EVENT));
					hdr->AS.auxBUF = hdr2->AS.auxBUF;  // contains the free text annotation
					// do not de-allocate event table when hdr2 is deconstructed
					memset(&hdr2->EVENT,0,sizeof(hdr2->EVENT));
					hdr2->AS.auxBUF = NULL;
					sclose(hdr2);
					destructHDR(hdr2);
					free(mrkfile);
					B4C_ERRNUM = 0; // reset error status - missing or incorrect marker file is not critical
				}
				else if (!strncmp(t,"DataFormat=BINARY",11))
					;
				else if (!strncmp(t,"DataFormat=ASCII",16)) {
					FLAG_ASCII = 1;
					gdftyp     = 17;
//					B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
//					B4C_ERRMSG = "Error SOPEN(BrainVision): ASCII-format not supported (yet).";
				}
				else if (!strncmp(t,"DataOrientation=VECTORIZED",25))
					orientation = VEC;
				else if (!strncmp(t,"DataOrientation=MULTIPLEXED",26))
					orientation = MUL;
				else if (!strncmp(t,"DataType=TIMEDOMAIN",19))
					;
				else if (!strncmp(t,"DataType=",9)) {
					B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
					B4C_ERRMSG = "Error SOPEN(BrainVision): DataType is not TIMEDOMAIN";
				}
				else if (!strncmp(t,"NumberOfChannels=",17)) {
					hdr->NS = atoi(t+17);
				}
				else if (!strncmp(t,"DataPoints=",11)) {
					npts = atol(t+11);
				}
				else if (!strncmp(t,"SamplingInterval=",17)) {
					hdr->SampleRate = 1e6/atof(t+17);
					hdr->EVENT.SampleRate = hdr->SampleRate;
				}
			}
			else if (seq==2) {
				if      (!strncmp(t,"BinaryFormat=IEEE_FLOAT_32",26)) {
					gdftyp = 16;
					digmax =  physmax/cal;
					digmin =  physmin/cal;
				}
				else if (!strncmp(t,"BinaryFormat=INT_16",19)) {
					gdftyp =  3;
					digmax =  32767;
					digmin = -32768;
					hdr->FLAG.OVERFLOWDETECTION = 1;
				}
				else if (!strncmp(t,"BinaryFormat=UINT_16",20)) {
					gdftyp = 4;
					digmax = 65535;
					digmin = 0;
					hdr->FLAG.OVERFLOWDETECTION = 1;
				}
				else if (!strncmp(t,"BinaryFormat",12)) {
					B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
					B4C_ERRMSG = "Error SOPEN(BrainVision): BinaryFormat=<unknown>";
				}
				else if (!strncmp(t,"UseBigEndianOrder=NO",20)) {
					// hdr->FLAG.SWAP = (__BYTE_ORDER == __BIG_ENDIAN);
					hdr->FILE.LittleEndian = 1;
				}
				else if (!strncmp(t,"UseBigEndianOrder=YES",21)) {
					// hdr->FLAG.SWAP = (__BYTE_ORDER == __LITTLE_ENDIAN);
					hdr->FILE.LittleEndian = 0;
				}
				else if (!strncmp(t,"DecimalSymbol=",14)) {
					DECIMALSYMBOL = t[14];
				}
				else if (!strncmp(t,"SkipLines=",10)) {
					SKIPLINES = atoi(t+10);
				}
				else if (!strncmp(t,"SkipColumns=",12)) {
					SKIPCOLUMNS = atoi(t+12);
				}
				else if (0) {
					B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
					B4C_ERRMSG = "Error SOPEN(BrainVision): BinaryFormat=<unknown>";
					return(hdr);
				}
			}
			else if (seq==3) {
				if (VERBOSE_LEVEL==9)
					fprintf(stdout,"BVA: seq=%i,line=<%s>,ERR=%i\n",seq,t,B4C_ERRNUM );

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
						double tmp = atof(ptr);
						if (tmp) hdr->CHANNEL[n].Cal = tmp;
						hdr->CHANNEL[n].PhysMax = hdr->CHANNEL[n].DigMax * hdr->CHANNEL[n].Cal ;
						hdr->CHANNEL[n].PhysMin = hdr->CHANNEL[n].DigMin * hdr->CHANNEL[n].Cal ;
					}

					if (VERBOSE_LEVEL==9)
						fprintf(stdout,"Ch%02i=%s,,%s(%f)\n",n,hdr->CHANNEL[n].Label,ptr,hdr->CHANNEL[n].Cal );
				}
			}
			else if (seq==4) {
			}
			else if (seq==5) {
			}
			else if (seq==6) {
			}

			// t = strtok(NULL,"\x0a\x0d");	// extract next line
		}
		hdr->HeadLen  = 0;
	    	if (FLAG_ASCII) {
	    		count = 0;
			size_t bufsiz  = hdr->NS*hdr->SPR*hdr->NRec*16;
			while (!ifeof(hdr)) {
			    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,count+bufsiz+1);
		    		count += ifread(hdr->AS.Header+count,1,bufsiz,hdr);
			}
			ifclose(hdr);
			hdr->AS.Header[count]=0;	// terminating null character

			size_t pos=0;
			if (DECIMALSYMBOL != '.')
				do {
					if (hdr->AS.Header[pos]==DECIMALSYMBOL)
						hdr->AS.Header[pos] = '.';
				} while (hdr->AS.Header[++pos]);

			pos = 0;
	    		while (SKIPLINES>0) {
				while (!iscntrl(hdr->AS.Header[pos])) pos++; 	// skip line
				while ( iscntrl(hdr->AS.Header[pos])) pos++;	// skip line feed and carriage return
				SKIPLINES--;
	    		}

			hdr->AS.rawdata = (uint8_t*)malloc(hdr->NS*npts*sizeof(double));
			char* POS=(char*)(hdr->AS.Header+pos);
			for (k=0; k < hdr->NS*npts; k++) {
		    		if (((orientation==MUL) && !(k%hdr->NS)) ||
		    		    ((orientation==VEC) && !(k%npts))) {
			    		int sc = SKIPCOLUMNS;
					while (sc--) strtod(POS,&POS);
	    			}
		    		*(double*)(hdr->AS.rawdata+k*sizeof(double)) = strtod(POS,&POS);
	    		}
	    		hdr->TYPE = native;
			hdr->AS.length  = hdr->NRec;
	    	}
	}

	else if (hdr->TYPE==CFS) {

		// DO NOT USE THESE STRUCTS UNLESS YOU ARE SURE THERE ARE NO ALIGNMENT ERRORS
		struct CFSGeneralHeader_t {
			char 	Marker[8];
			char 	Filename[14];
			uint32_t Filesize;
			char 	Time[8];
			char 	Date[8];
			uint16_t NChannels;
			uint16_t NFileVars;
			uint16_t NDataSectionVars;
			uint16_t SizeHeader;
			uint16_t SizeData;
			uint32_t LastDataHeaderSectionOffset;
			uint16_t NDataSections;
			uint16_t DiskBlockSizeRounding;
			char 	Comment[73];
			uint32_t PointerTableOffset;
			char	reserved[40];
		} *CFSGeneralHeader=NULL;

		struct CFSChannelDev_t {
			char 	Channelname[22];
			char 	YYnits[10];
			char 	XUnits[10];
			uint8_t datatype;
			uint8_t datakind;
			uint16_t ByteSpaceBetweenElements;
			uint16_t NextChannel;
		} *CFSChannelDev=NULL;

		fprintf(stdout,"Warning: support for CFS is very experimental\n");

#define H1LEN (8+14+4+8+8+2+2+2+2+2+4+2+2+74+4+40)

		while (!ifeof(hdr)) {
			hdr->AS.Header = (uint8_t*) realloc(hdr->AS.Header,count*2+1);
			count += ifread(hdr->AS.Header+count,1,count,hdr);
		}
		hdr->AS.Header[count] = 0;
		hdr->FLAG.OVERFLOWDETECTION = 0;

		uint8_t k;
		hdr->NS = leu16p(hdr->AS.Header+42);
		/* General Header */
		uint32_t filesize = leu32p(hdr->AS.Header+22);
		hdr->NS    = leu16p(hdr->AS.Header+42);	// 6  number of channels
		uint8_t  n = leu16p(hdr->AS.Header+44);	// 7  number of file variables
		uint16_t d = leu16p(hdr->AS.Header+46);	// 8  number of data section variables
		uint16_t FileHeaderSize = leu16p(hdr->AS.Header+48);	// 9  byte size of file header
		uint16_t DataHeaderSize = leu16p(hdr->AS.Header+50);	// 10 byte size of data section header
		uint32_t LastDataSectionHeaderOffset = leu32p(hdr->AS.Header+52);	// 11 last data section header offset
		uint16_t NumberOfDataSections = leu16p(hdr->AS.Header+56);	// 12 last data section header offset

		if (NumberOfDataSections) {
			hdr->EVENT.TYP = (typeof(hdr->EVENT.TYP)) realloc(hdr->EVENT.TYP, (hdr->EVENT.N + NumberOfDataSections - 1) * sizeof(*hdr->EVENT.TYP));
			hdr->EVENT.POS = (typeof(hdr->EVENT.POS)) realloc(hdr->EVENT.POS, (hdr->EVENT.N + NumberOfDataSections - 1) * sizeof(*hdr->EVENT.POS));
			hdr->EVENT.CHN = (typeof(hdr->EVENT.CHN)) realloc(hdr->EVENT.CHN, (hdr->EVENT.N + NumberOfDataSections - 1) * sizeof(*hdr->EVENT.CHN));
			hdr->EVENT.DUR = (typeof(hdr->EVENT.DUR)) realloc(hdr->EVENT.DUR, (hdr->EVENT.N + NumberOfDataSections - 1) * sizeof(*hdr->EVENT.DUR));
		}


if (VERBOSE_LEVEL>7) fprintf(stdout,"CFS 131 - %d,%d,%d,0x%x,0x%x,0x%x,%d,0x%x\n",hdr->NS,n,d,FileHeaderSize,DataHeaderSize,LastDataSectionHeaderOffset,NumberOfDataSections,leu32p(hdr->AS.Header+0x86));

		/* channel information */
		hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS, sizeof(CHANNEL_TYPE));
#define H2LEN (22+10+10+1+1+2+2)
 		char* H2 = (char*)(hdr->AS.Header + H1LEN);
		double xPhysDimScale[100];		// CFS is limited to 99 channels
		for (k = 0; k < hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL + k;
			/*
				1 offset because CFS uses pascal type strings (first byte contains string length)
				in addition, the strings are \0 terminated.
			*/
			hc->OnOff = 1;

			char len = min(21, MAX_LENGTH_LABEL);
			strncpy(hc->Label, H2 + 1 + k*H2LEN, len);
			len = strlen(hc->Label);
			while (isspace(hc->Label[len])) len--;		// remove trailing blanks
			hc->Label[len+1] = 0;

			hc->PhysDimCode  = PhysDimCode (H2 + 22 + 1 + k*H2LEN);
			xPhysDimScale[k] = PhysDimScale(PhysDimCode(H2 + 32 + 1 + k*H2LEN));

			uint8_t gdftyp   = H2[42 + k*H2LEN];
			hc->GDFTYP = gdftyp < 5 ? gdftyp+1 : gdftyp+11;
			if (H2[43 + k * H2LEN]) {
				B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
				B4C_ERRMSG = "(CFS)Subsidiary or Matrix data not supported";
			}
			hc->LowPass  = NaN;
			hc->HighPass = NaN;
			hc->Notch    = NaN;
if (VERBOSE_LEVEL>7) fprintf(stdout,"Channel #%i: [%s](%i/%i) <%s>/<%s> ByteSpace%i,Next#%i\n",k+1, H2 + 1 + k*H2LEN, gdftyp, H2[43], H2 + 23 + k*H2LEN, H2 + 33 + k*H2LEN, leu16p(H2+44+k*H2LEN), leu16p(H2+46+k*H2LEN));
		}

		size_t datapos = H1LEN + H2LEN*hdr->NS;

		/* file variable information */
		// n*36 bytes
if (VERBOSE_LEVEL>7) fprintf(stdout,"\n******* file variable information *********\n");
		for (k = 0; k < n; k++) {
			int i; double f;
			size_t pos = datapos + k*36;
			uint16_t typ = leu16p(hdr->AS.Header+pos+22);
			uint16_t off = leu16p(hdr->AS.Header+pos+34);

//fprintf(stdout,"\n%3i @0x%6x: <%s>  %i  [%s] %i ",k, pos, hdr->AS.Header+pos+1,typ,hdr->AS.Header+pos+25,off);
			size_t p3 = H1LEN + H2LEN*hdr->NS + (n+d)*36 + off + 42;

			switch (typ) {
			case 0:
			case 1: i = hdr->AS.Header[p3]; break;
			case 2: i = lei16p(hdr->AS.Header+p3); break;
			case 3: i = leu16p(hdr->AS.Header+p3); break;
			case 4: i = lei32p(hdr->AS.Header+p3); break;
			case 5: f = lef32p(hdr->AS.Header+p3); break;
			case 6: f = lef64p(hdr->AS.Header+p3); break;
			}
if (VERBOSE_LEVEL>7) 	{
			if (typ<5) fprintf(stdout," *0x%x = [%d]",p3,i);
			else if (typ<7) fprintf(stdout," *0x%x = [%g]",p3,f);
			else if (typ==7) fprintf(stdout," *0x%x = <%s>",p3,hdr->AS.Header+p3);
			}
		}

if (VERBOSE_LEVEL>7) fprintf(stdout,"\n******* DS variable information *********\n");
		datapos = LastDataSectionHeaderOffset; //H1LEN + H2LEN*hdr->NS + n*36;
		// reverse order of data sections
		uint32_t *DATAPOS = (uint32_t*)malloc(sizeof(uint32_t)*NumberOfDataSections);

		uint8_t m;
		for (m = NumberOfDataSections; 0 < m; ) {
			DATAPOS[--m] = datapos;
			datapos = leu32p(hdr->AS.Header + datapos);
		}

//		void *VarChanInfoPos = hdr->AS.Header + datapos + 30;  // unused
		char flag_ChanInfoChanged = 0;
		hdr->NRec = NumberOfDataSections;
		size_t SPR = 0, SZ = 0;
		for (m = 0; m < NumberOfDataSections; m++) {
			datapos = DATAPOS[m];
			if (!leu32p(hdr->AS.Header+datapos+8)) continue; 	// empty segment

//			flag_ChanInfoChanged |= memcmp(VarChanInfoPos, hdr->AS.Header + datapos + 30, 24*hdr->NS);

if (VERBOSE_LEVEL>7) fprintf(stdout,"\n******* DATA SECTION --%03i-- %i *********\n",m,flag_ChanInfoChanged);
if (VERBOSE_LEVEL>7) fprintf(stdout,"\n[DS#%3i] 0x%x 0x%x [0x%x 0x%x szChanData=%i] 0x02%x\n", m, FileHeaderSize, datapos, leu32p(hdr->AS.Header+datapos), leu32p(hdr->AS.Header+datapos+4), leu32p(hdr->AS.Header+datapos+8), leu16p(hdr->AS.Header+datapos+12));

			uint32_t sz    = 0;
			uint32_t bpb   = 0, spb = 0, spr = 0;
			hdr->AS.first  = 0;
			hdr->AS.length = 0;
			for (k = 0; k < hdr->NS; k++) {
				uint8_t *pos = hdr->AS.Header + datapos + 30 + 24 * k;

				CHANNEL_TYPE *hc = hdr->CHANNEL + k;

				uint32_t p  = leu32p(pos);
				hc->SPR     = leu32p(pos+4);
				hc->Cal     = lef32p(pos+8);
				hc->Off     = lef32p(pos+12);
				double Xcal = lef32p(pos+16);
				//double Xoff = lef32p(pos+20);// unused
				hc->OnOff   = 1;
				hc->bi      = bpb;

if (VERBOSE_LEVEL>7) fprintf(stdout,"CFS 409: %i #%i: SPR=%i=%i=%i  x%f+-%f %i\n",m,k,spr,SPR,hc->SPR,hc->Cal,hc->Off,p);

				double Fs = 1.0 / (xPhysDimScale[k] * Xcal);
				if (!m && !k) {
					hdr->SampleRate = Fs;
				}
				else if (fabs(hdr->SampleRate - Fs) > 1e-3) {
					B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
					B4C_ERRMSG = "CED/CFS: different sampling rates are not supported";
				}

				spr  = hc->SPR;
				spb += hc->SPR;
				sz  += hc->SPR * GDFTYP_BITS[hc->GDFTYP] >> 3;
				bpb += GDFTYP_BITS[hc->GDFTYP]>>3;	// per single sample
				hdr->AS.length += hc->SPR;
			}

			if (NumberOfDataSections > 1) {
				// hack: copy data into a single block, only if more than one section
				hdr->AS.rawdata = (uint8_t*)realloc(hdr->AS.rawdata, (hdr->NS * SPR + spb) * sizeof(double));

/*
				if (VERBOSE_LEVEL>7)
				 	fprintf(stdout,"CFS 411: @%p %i @%p\n",hdr->AS.rawdata, (hdr->NS * SPR + spb), srcaddr);
*/
				hdr->AS.first = 0;
				for (k = 0; k < hdr->NS; k++) {
					CHANNEL_TYPE *hc = hdr->CHANNEL + k;

					uint32_t memoffset = leu32p(hdr->AS.Header + datapos + 30 + 24 * k);
					uint8_t *srcaddr = hdr->AS.Header+leu32p(hdr->AS.Header+datapos + 4) + memoffset;

				if (VERBOSE_LEVEL>7)
				 	fprintf(stdout,"CFS 412 #%i %i: @%p %i\n", k, hc->SPR, srcaddr, leu32p(hdr->AS.Header+datapos + 4) + leu32p(hdr->AS.Header + datapos + 30 + 24 * k));

					int16_t szz = (GDFTYP_BITS[hc->GDFTYP]>>3);
					int k2;
					for (k2 = 0; k2 < hc->SPR; k2++) {
						uint8_t *ptr = srcaddr + k2*szz;
						double val;

						switch (hc->GDFTYP) {
						// reorder for performance reasons - more frequent gdftyp's come first
						case 3:  val = lei16p(ptr); break;
						case 4:  val = leu16p(ptr); break;
						case 16: val = lef32p(ptr); break;
						case 17: val = lef64p(ptr); break;
						case 0:  val = *(   char*) ptr; break;
						case 1:  val = *( int8_t*) ptr; break;
						case 2:  val = *(uint8_t*) ptr; break;
						case 5:  val = lei32p(ptr); break;
						case 6:  val = leu32p(ptr); break;
						case 7:  val = lei64p(ptr); break;
						case 8:  val = leu64p(ptr); break;
						default:
							B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
							B4C_ERRMSG = "CED/CFS: invalid data type";
						}

if (VERBOSE_LEVEL>8)
 	fprintf(stdout,"CFS read: %2i #%2i:%5i [%i]: %f -> %f  @%p\n",m,k,k2,bpb,SPR,val,val*hc->Cal + hc->Off, hdr->AS.rawdata + ((SPR + k2) * hdr->NS + k) * sizeof(double));

						*(double*) (hdr->AS.rawdata + k * sizeof(double) + (SPR + k2) * hdr->NS * sizeof(double)) = val * hc->Cal + hc->Off;
					}
//					srcaddr += hdr->CHANNEL[k].SPR * GDFTYP_BITS[hdr->CHANNEL[k].GDFTYP] >> 3;
				}
			}
			else {
				hdr->AS.rawdata = (uint8_t*)realloc(hdr->AS.rawdata,sz);
				memcpy(hdr->AS.rawdata, hdr->AS.Header + leu32p(hdr->AS.Header+datapos + 4), leu32p(hdr->AS.Header+datapos + 8));
				hdr->AS.bpb = sz;
			}

			if (m>0) {
				hdr->EVENT.TYP[hdr->EVENT.N] = 0x7ffe;
				hdr->EVENT.POS[hdr->EVENT.N] = SPR;
				hdr->EVENT.CHN[hdr->EVENT.N] = 0;
				hdr->EVENT.DUR[hdr->EVENT.N] = 0;
				hdr->EVENT.N++;
			}

			SPR += spr;
			SZ  += sz;
if (VERBOSE_LEVEL>7) fprintf(stdout,"CFS 414: SPR=%i,%i,%i NRec=%i, @%p\n",spr,SPR,hdr->SPR,hdr->NRec, hdr->AS.rawdata);

			// for (k = 0; k < d; k++) {
			for (k = 0; k < 0; k++) {
			// read data variables of each block - this currently broken.
				//size_t pos = leu16p(hdr->AS.Header + datapos + 30 + hdr->NS * 24 + k * 36 + 34);
				size_t pos = datapos + 30 + hdr->NS * 24;
				int i; double f;
				uint16_t typ = leu16p(hdr->AS.Header + pos + 22 + k*36) ;
				uint16_t off = leu16p(hdr->AS.Header + pos + 34 + k*36);
				uint32_t p3  = pos + off;

if (VERBOSE_LEVEL>7) fprintf(stdout,"\n[DS#%3i/%3i] @0x%6x+0x%3x: <%s>  %i  [%s] :", m, k, pos, off, hdr->AS.Header+pos+off+1, typ, hdr->AS.Header+pos+off+25);

				switch (typ) {
				case 0:
				case 1: i = hdr->AS.Header[p3];        break;
				case 2: i = lei16p(hdr->AS.Header+p3); break;
				case 3: i = leu16p(hdr->AS.Header+p3); break;
				case 4: i = lei32p(hdr->AS.Header+p3); break;
				case 5: f = lef32p(hdr->AS.Header+p3); break;
				case 6: f = lef64p(hdr->AS.Header+p3); break;
				}
if (VERBOSE_LEVEL>7) {
				if (typ<5) fprintf(stdout," *0x%x = %d",p3,i);
				else if (typ<7) fprintf(stdout," *0x%x = %g", p3,f);
				else if (typ==7) fprintf(stdout," *0x%x = <%s>",p3,hdr->AS.Header+p3);
}
			}
			datapos = leu32p(hdr->AS.Header + datapos);
		}
		free(DATAPOS);

if (VERBOSE_LEVEL>7) fprintf(stdout,"CFS 419: SPR=%i=%i NRec=%i  @%p\n",SPR,hdr->SPR,hdr->NRec, hdr->AS.rawdata);

		hdr->AS.first = 0;
		hdr->EVENT.SampleRate = hdr->SampleRate;
		if (NumberOfDataSections<=1) {
			// hack: copy data into a single block, only if more than one section
			hdr->FLAG.UCAL = 0;
			hdr->SPR  = SPR;
			hdr->NRec = 1;
			hdr->AS.length = 1;
		}
		else  {
			hdr->FLAG.UCAL = 1;
			hdr->SPR       = 1;
			hdr->NRec      = SPR;
			hdr->AS.bpb    = hdr->NS * sizeof(double);
			hdr->AS.length = SPR;
			for (k = 0; k < hdr->NS; k++) {
				CHANNEL_TYPE *hc = hdr->CHANNEL + k;
				hc->GDFTYP  = 17;	// double
				hc->bi      = sizeof(double)*k;
				hc->SPR     = hdr->SPR;
				hc->Cal     = 1.0;
				hc->Off     = 0.0;
			}
		}

if (VERBOSE_LEVEL>7) fprintf(stdout,"CFS 429: SPR=%i=%i NRec=%i\n",SPR,hdr->SPR,hdr->NRec);
		datapos   = FileHeaderSize;  //+DataHeaderSize;

		if (flag_ChanInfoChanged) {
			B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
			B4C_ERRMSG = "CED/CFS: varying channel information not supported";
		}

		for (k = 0; k < hdr->NS; k++) {
			switch (hdr->CHANNEL[k].GDFTYP) {
			case 0:
			case 1:
				hdr->CHANNEL[k].DigMax  =  127;
				hdr->CHANNEL[k].DigMin  = -128;
				break;
			case 2:
				hdr->CHANNEL[k].DigMax  =  255;
				hdr->CHANNEL[k].DigMin  =  0;
				break;
			case 3:
				hdr->CHANNEL[k].DigMax  = (int16_t)0x7fff;
				hdr->CHANNEL[k].DigMin  = (int16_t)0x8000;
				break;
			case 4:
				hdr->CHANNEL[k].DigMax  = 0xffff;
				hdr->CHANNEL[k].DigMin  = 0;
				break;
			case 16:
			case 17:
				hdr->CHANNEL[k].DigMax  =  1e9;
				hdr->CHANNEL[k].DigMin  = -1e9;
				break;
			}
			hdr->CHANNEL[k].PhysMax = hdr->CHANNEL[k].DigMax * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
			hdr->CHANNEL[k].PhysMin = hdr->CHANNEL[k].DigMin * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
		}
		hdr->FLAG.UCAL = 0;

#undef H1LEN
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
	    	// = *(double*)(Header1+44);	// pre-trigger time
	    	hdr->NS   	= leu32p(hdr->AS.Header+52);
	    	hdr->NRec	= leu32p(hdr->AS.Header+56);
#define CFWB_FLAG_TIME_CHANNEL  (*(int32_t*)(Header1+60))	// TimeChannel
	    	//  	= *(int32_t*)(Header1+64);	// DataFormat

	    	hdr->HeadLen = 68 + hdr->NS*96;
	    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,hdr->HeadLen);
	    	if (count<=hdr->HeadLen)
			count += ifread(hdr->AS.Header+count, 1, hdr->HeadLen-count, hdr);
		else
	    		ifseek(hdr, hdr->HeadLen, SEEK_SET);

		uint16_t gdftyp = leu32p(hdr->AS.Header+64);
		hdr->AS.bpb = (CFWB_FLAG_TIME_CHANNEL ? GDFTYP_BITS[CFWB_GDFTYP[gdftyp-1]]>>3 : 0);

	    	hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS; k++)	{
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
		    	uint8_t* Header2 = hdr->AS.Header+68+k*96;
			hc->Transducer[0] = '\0';
		    	hc->GDFTYP 	= CFWB_GDFTYP[gdftyp-1];
		    	hc->SPR 	= 1; // *(int32_t*)(Header1+56);
		    	strncpy(hc->Label, (char*)Header2, min(32,MAX_LENGTH_LABEL));
		    	char p[MAX_LENGTH_PHYSDIM+1];
		    	strncpy(p, (char*)Header2+32, min(16,MAX_LENGTH_PHYSDIM));
		    	p[MAX_LENGTH_PHYSDIM] = 0;
		    	hc->PhysDimCode = PhysDimCode(p);
		    	hc->LeadIdCode  = 0;
		    	hc->Cal	= lef64p(Header2+64);
		    	hc->Off	= lef64p(Header2+72);
		    	hc->PhysMax	= lef64p(Header2+80);
		    	hc->PhysMin	= lef64p(Header2+88);
		    	hc->DigMax	= (hc->PhysMax - hc->Off) / hc->Cal;
		    	hc->DigMin	= (hc->PhysMin - hc->Off) / hc->Cal;
			hc->OnOff    	= 1;
			hc->bi    	= hdr->AS.bpb;
			hdr->AS.bpb += GDFTYP_BITS[hc->GDFTYP]>>3;
		}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// CFWB does not support automated overflow and saturation detection
	}

	else if (hdr->TYPE==CNT) {

		if (VERBOSE_LEVEL>7)
			fprintf(stdout,"SOPEN: Neuroscan format \n");

		// TODO: fix handling of AVG and EEG files
	    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, 900);
	    	hdr->VERSION = atof((char*)hdr->AS.Header + 8);
	    	count  += ifread(hdr->AS.Header+count, 1, 900-count, hdr);

		uint16_t gdftyp = 0;
	    	uint8_t minor_revision = hdr->AS.Header[804];
	    	size_t eventtablepos = leu32p(hdr->AS.Header+886);
	    	size_t nextfilepos = leu32p(hdr->AS.Header+12);


		/* make base of filename */
		int i=0, j=0;
		while (i<strlen(hdr->FileName)) {
			if ((hdr->FileName[i]=='/') || (hdr->FileName[i]=='\\')) { j=i+1; }
			i++;
		}
		/* skip the extension '.cnt' of filename base and copy to Patient.Id */
		strncpy(hdr->Patient.Id, hdr->FileName+j, min(MAX_LENGTH_PID,strlen(hdr->FileName)-j-4));
		hdr->Patient.Id[MAX_LENGTH_PID] = 0;

	    	ptr_str = (char*)hdr->AS.Header+136;
    		hdr->Patient.Sex = (ptr_str[0]=='f')*2 + (ptr_str[0]=='F')*2 + (ptr_str[0]=='M') + (ptr_str[0]=='m');
	    	ptr_str = (char*)hdr->AS.Header+137;
	    	hdr->Patient.Handedness = (ptr_str[0]=='r')*2 + (ptr_str[0]=='R')*2 + (ptr_str[0]=='L') + (ptr_str[0]=='l');
	    	ptr_str = (char*)hdr->AS.Header+225;
	    	tm_time.tm_sec  = atoi(strncpy(tmp,ptr_str+16,2));
    		tm_time.tm_min  = atoi(strncpy(tmp,ptr_str+13,2));
    		tm_time.tm_hour = atoi(strncpy(tmp,ptr_str+10,2));
    		tm_time.tm_mday = atoi(strncpy(tmp,ptr_str,2));
    		tm_time.tm_mon  = atoi(strncpy(tmp,ptr_str+3,2))-1;
    		tm_time.tm_year = atoi(strncpy(tmp,ptr_str+6,2));

	    	if (tm_time.tm_year<=80)    	tm_time.tm_year += 100;
		hdr->T0 = tm_time2gdf_time(&tm_time);

		hdr->NS  = leu16p(hdr->AS.Header+370);
	    	hdr->HeadLen = 900+hdr->NS*75;
		hdr->SampleRate = leu16p(hdr->AS.Header+376);
		hdr->AS.bpb = hdr->NS*2;

		if (hdr->AS.Header[20]==1) {
			// Neuroscan EEG
			hdr->NRec = leu16p(hdr->AS.Header+362);
			hdr->SPR  = leu16p(hdr->AS.Header+368);
	    		hdr->AS.bpb = 2*hdr->NS*hdr->SPR+1+2+2+4+2+2;
	    		size_t bpb4 = 4*hdr->NS*hdr->SPR+1+2+2+4+2+2;
			struct stat FileBuf;

			if (VERBOSE_LEVEL>7)
				fprintf(stdout,"SOPEN: Neuroscan format: minor rev=%i\n", minor_revision);

		    	switch (minor_revision) {
		    	case 9:
		    		// TODO: FIXME
    				fprintf(stderr,"Warning biosig/sopen (CNT/EEG): minor revision %i is experimental\n",minor_revision);
		    		gdftyp = 3;
		    		hdr->FILE.LittleEndian = 0;
				stat(hdr->FileName,&FileBuf);
				if (hdr->NRec <= 0) {
					hdr->NRec = (min(FileBuf.st_size,nextfilepos) - hdr->HeadLen)/hdr->AS.bpb;
				}
		    		break;

		    	case 12:
		    		gdftyp = 3;
		    		eventtablepos = hdr->HeadLen + hdr->NRec*hdr->AS.bpb;
		    		break;

		    	default:
	    			if (minor_revision != 16)
	    				fprintf(stderr,"Warning biosig/sopen (CNT/EEG): minor revision %i not tested\n",minor_revision);

				if (VERBOSE_LEVEL>7)
		    			fprintf(stdout,"biosig/sopen (CNT/EEG):  %i %i %i %i %i %i \n", hdr->NRec, hdr->SPR, hdr->NS, eventtablepos, (hdr->AS.bpb * hdr->NRec + hdr->HeadLen), (bpb4 * hdr->NRec + hdr->HeadLen));

	    			if ((hdr->AS.bpb * hdr->NRec + hdr->HeadLen) == eventtablepos)
	    				gdftyp = 3;
	    			else if ((bpb4 * hdr->NRec + hdr->HeadLen) == eventtablepos) {
	    				hdr->AS.bpb = bpb4;
	    				gdftyp = 5;
	    			}
	    			else {
	    				B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	    				B4C_ERRMSG = "CNT/EEG: type of format not supported";
			    		return(hdr);
	    			}
		    	}
		}
		else {
			// Neuroscan CNT
			hdr->SPR    = 1;
			eventtablepos = leu32p(hdr->AS.Header+886);
	    		gdftyp      = hdr->FLAG.CNT32 ? 5 : 3;
		    	hdr->AS.bpb = hdr->NS*GDFTYP_BITS[gdftyp]/8;
			hdr->NRec   = (eventtablepos-hdr->HeadLen) / hdr->AS.bpb;

			if (VERBOSE_LEVEL>7)
	    			fprintf(stdout,"biosig/sopen (CNT):  %i %i %i %i %i \n", hdr->NRec, hdr->SPR, hdr->NS, eventtablepos, (hdr->AS.bpb * hdr->NRec + hdr->HeadLen) );
		}

	    	hdr->AS.Header = (uint8_t*) realloc(Header1,hdr->HeadLen);
	    	count  += ifread(Header1+900, 1, hdr->NS*75, hdr);

	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	    	size_t bi = 0;
		for (k=0; k<hdr->NS; k++)	{
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
		    	uint8_t* Header2 = hdr->AS.Header+900+k*75;
			hc->Transducer[0] = '\0';
		    	hc->GDFTYP 	= gdftyp;
		    	hc->SPR 	= hdr->SPR; // *(int32_t*)(Header1+56);
		    	const size_t len = min(10, MAX_LENGTH_LABEL);
		    	strncpy(hc->Label, (char*)Header2, len);
		    	hc->Label[len]  = 0;
		    	hc->LeadIdCode  = 0;
			hc->PhysDimCode = 4256+19;  // uV
		    	hc->Cal		= lef32p(Header2+59);
		    	hc->Cal    	*= lef32p(Header2+71)/204.8;
		    	hc->Off		= lef32p(Header2+47) * hc->Cal;
		    	hc->HighPass	= CNT_SETTINGS_HIGHPASS[(uint8_t)Header2[64]];
		    	hc->LowPass	= CNT_SETTINGS_LOWPASS[(uint8_t)Header2[65]];
		    	hc->Notch	= CNT_SETTINGS_NOTCH[(uint8_t)Header1[682]];
			hc->OnOff       = 1;

			if (hdr->FLAG.CNT32) {
			  	hc->DigMax	=  (double)(0x007fffff);
			    	hc->DigMin	= -(double)(int32_t)(0xff800000);
			}
			else {
			    	hc->DigMax	=  (double)32767;
			    	hc->DigMin	= -(double)32768;
			}
		    	hc->PhysMax	= hc->DigMax * hc->Cal + hc->Off;
		    	hc->PhysMin	= hc->DigMin * hc->Cal + hc->Off;
			hc->bi    	= bi;
			bi 		+= hdr->SPR * GDFTYP_BITS[hc->GDFTYP]>>3;
		}

	    	if ((eventtablepos < nextfilepos) && !ifseek(hdr, eventtablepos, SEEK_SET)) {
		    	/* read event table */
			hdr->EVENT.SampleRate = hdr->SampleRate;
			ifread(tmp, 9, 1, hdr);
			int8_t   TeegType   = tmp[0];
			uint32_t TeegSize   = leu32p(tmp+1);
			// uint32_t TeegOffset = leu32p(tmp+5); // not used

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
				hdr->EVENT.TYP[k] = leu16p(buf+k*fieldsize);	// stimulus type
				uint8_t tmp8 = buf[k*fieldsize+3];
				if (tmp8>0) {
					if (hdr->EVENT.TYP[k]>0)
						fprintf(stdout,"Warning SOPEN(CNT) event %i: both, stimulus and response, codes (%i/%i) are non-zero. response code is ignored.\n",k+1,hdr->EVENT.TYP[k],tmp8);
					else
						hdr->EVENT.TYP[k] |= tmp8 | 0x80;	// response type
				}
				hdr->EVENT.POS[k] = leu32p(buf+4+k*fieldsize);        // 0-based indexing
				if (TeegType != 3)
					hdr->EVENT.POS[k] = (hdr->EVENT.POS[k] - hdr->HeadLen) / hdr->AS.bpb;
			}
			free(buf);
	    	}
	    	ifseek(hdr, hdr->HeadLen, SEEK_SET);
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// automated overflow and saturation detection not supported
	}

	else if (hdr->TYPE==CTF) {

		if (VERBOSE_LEVEL>8) fprintf(stdout,"CTF[101]: %s\n",hdr->FileName);

		hdr->AS.Header  = (uint8_t*)realloc(hdr->AS.Header,1844);
		hdr->HeadLen    = 1844;
		const char *f0  = hdr->FileName;
		char *f1 	= (char*)malloc(strlen(f0)+6);
		strcpy(f1,f0);
		strcpy(strrchr(f1,'.')+1,"res4");

		if (VERBOSE_LEVEL>8) fprintf(stdout,"CTF[102]: %s\n\t%s\n",f0,f1);

		if (strcmp(strrchr(hdr->FileName,'.'),".res4")) {
			if (VERBOSE_LEVEL>8) fprintf(stdout,"CTF[103]:\n");
			ifclose(hdr);
			hdr->FileName = f1;
			hdr = ifopen(hdr,"rb");
			count = 0;
		}

		count += ifread(hdr->AS.Header+count,1,hdr->HeadLen-count,hdr);

		if (VERBOSE_LEVEL>8) fprintf(stdout,"CTF[104]: %i %s\n\t%s\n",count,f0,f1);

		struct tm t;
		sscanf((char*)(hdr->AS.Header+778),"%d:%d:%d",&t.tm_hour,&t.tm_min,&t.tm_sec);
		sscanf((char*)(hdr->AS.Header+778+255),"%d/%d/%d",&t.tm_mday,&t.tm_mon,&t.tm_year);
		--t.tm_mon;
		hdr->T0 = tm_time2gdf_time(&t);

		hdr->SPR 	= bei32p(hdr->AS.Header+1288);
		hdr->NS  	= bei16p(hdr->AS.Header+1292);
		hdr->SampleRate = bef64p(hdr->AS.Header+1296);
		// double Dur	= bef64p(hdr->AS.Header+1304);
		hdr->NRec	= bei16p(hdr->AS.Header+1312);
		strncpy(hdr->Patient.Id,(char*)(hdr->AS.Header+1712),min(MAX_LENGTH_PID,32));
		int32_t CTF_RunSize  = bei32p(hdr->AS.Header+1836);
		//int32_t CTF_RunSize2 = bei32p(hdr->AS.Header+1844);

		hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,1844+CTF_RunSize+2);
		count += ifread(hdr->AS.Header+count,1,CTF_RunSize+2,hdr);
		int16_t CTF_NumberOfFilters = bei16p(hdr->AS.Header+1844+CTF_RunSize);
		hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,count+CTF_NumberOfFilters*26+hdr->NS*(32+48+1280));
		count += ifread(hdr->AS.Header+count,1,CTF_NumberOfFilters*26+hdr->NS*(32+48+1280),hdr);
		ifclose(hdr);

		size_t pos = 1846+CTF_RunSize+CTF_NumberOfFilters*26;

	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	    	hdr->AS.bpb = 0;
		for (k=0; k<hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;

			strncpy(hc->Label,(const char*)(hdr->AS.Header+pos+k*32),min(32,MAX_LENGTH_LABEL));
			hc->Label[min(MAX_LENGTH_LABEL,32)]=0;

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"CTF[107]: #%i\t%x\t%s\n",k,pos+k*32,hc->Label);

			int16_t index = bei16p(hdr->AS.Header+pos+hdr->NS*32+k*(48+1280)); // index
			hc->Cal = 1.0/bef64p(hdr->AS.Header+pos+hdr->NS*32+k*(48+1280)+16);
			switch (index) {
			case 0:
			case 1:
			case 9:
				hc->Cal /= bef64p(hdr->AS.Header+pos+hdr->NS*32+k*(48+1280)+8);
			}

		    	hc->GDFTYP 	= 5;
		    	hc->SPR 	= hdr->SPR;
		    	hc->LeadIdCode  = 0;
		    	hc->Off	= 0.0;
			hc->OnOff   = 1;

			hc->PhysDimCode = 0;
		    	hc->DigMax	= ldexp( 1.0,31);
		    	hc->DigMin	= ldexp(-1.0,31);
		    	hc->PhysMax	= hc->DigMax * hc->Cal + hc->Off;
		    	hc->PhysMin	= hc->DigMin * hc->Cal + hc->Off;

			hc->bi    = hdr->AS.bpb;
			hdr->AS.bpb += hdr->SPR*GDFTYP_BITS[hc->GDFTYP]>>3;
		}

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"CTF[109] %s: \n",hdr->FileName);

		/********** read marker file **********/
		char *f2 = (char*)malloc(strlen(f0)+16);
		strcpy(f2,f0);
		strcpy(strrchr(f2,FILESEP)+1,"MarkerFile.mrk");
		hdr->EVENT.SampleRate = hdr->SampleRate;
		hdr->EVENT.N = 0;

		hdr->FileName = f2;
       		hdr = ifopen(hdr,"rb");
	    	if (hdr->FILE.OPEN) {
			size_t bufsiz = 4096;
			count = 0;
	    		char *vmrk=NULL;
			while (!ifeof(hdr)) {
			    	vmrk   = (char*)realloc(vmrk,count+bufsiz+1);
			    	count += ifread(vmrk+count,1,bufsiz,hdr);
			}
		    	vmrk[count] = 0;	// add terminating \0 character
			ifclose(hdr);

			char *t1, *t2;
			float u1,u2;
			t1 = strstr(vmrk,"TRIAL NUMBER");
			t2 = strtok(t1,"\x0a\x0d");
			size_t N = 0;
			t2 = strtok(NULL,"\x0a\x0d");
			while (t2 != NULL) {
				sscanf(t2,"%f %f",&u1,&u2);

				if (N+1 >= hdr->EVENT.N) {
					hdr->EVENT.N  += 256;
			 		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, hdr->EVENT.N*sizeof(*hdr->EVENT.POS));
					hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP));
			 		hdr->EVENT.DUR = (uint32_t*) realloc(hdr->EVENT.DUR, hdr->EVENT.N*sizeof(*hdr->EVENT.POS));
					hdr->EVENT.CHN = (uint16_t*) realloc(hdr->EVENT.CHN, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP));
				}
				hdr->EVENT.TYP[N] = 1;
				hdr->EVENT.POS[N] = (uint32_t)(u1*hdr->SPR+u2*hdr->SampleRate);
				hdr->EVENT.DUR[N] = 0;
				hdr->EVENT.CHN[N] = 0;
				N++;

				t2 = strtok(NULL,"\x0a\x0d");
			}
			hdr->EVENT.N = N;
			free(vmrk);
		}
		free(f2);
		/********** end reading event/marker file **********/


		strcpy(strrchr(f1,'.')+1,"meg4");
		hdr->FileName = f1;
		hdr = ifopen(hdr,"rb");
	    	hdr->HeadLen  = 8;
		hdr->HeadLen  = ifread(hdr->AS.Header,1,8,hdr);
		// hdr->FLAG.SWAP= (__BYTE_ORDER == __LITTLE_ENDIAN);
		hdr->FILE.LittleEndian = 0;

		hdr->FileName = f0;
		free(f1);

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
	    	hdr->AS.bpb = 0;
		for (k=0; k < hdr->NS; k++) {
			CHANNEL_TYPE* hc = hdr->CHANNEL+k;
			hc->GDFTYP   = gdftyp;
			hc->SPR      = 1;
			hc->Cal      = Cal;
			hc->Off      = Off;
			hc->OnOff    = 1;
			hc->Transducer[0] = '\0';
			hc->LowPass  = 450;
			hc->HighPass = 20;
			hc->PhysMax  = PhysMax;
			hc->PhysMin  = PhysMin;
			hc->DigMax   = DigMax;
			hc->DigMin   = DigMin;
		    	hc->LeadIdCode  = 0;
			hc->bi    = hdr->AS.bpb;
			hdr->AS.bpb += GDFTYP_BITS[gdftyp]>>3;
		}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// automated overflow and saturation detection not supported
	    	hdr->HeadLen = 19;
	    	ifseek(hdr, 19, SEEK_SET);
	}

	else if (hdr->TYPE==EBS) {

		fprintf(stdout,"Warning SOPEN(EBS): support for EBS format is experimental\n");

		/**  Fixed Header (32 bytes)  **/
		uint32_t EncodingID = beu32p(Header1+8);
		hdr->NS  = beu32p(Header1+12);
		hdr->SPR = beu64p(Header1+16);
		uint64_t datalen = beu64p(Header1+24);

		enum encoding {
			TIB_16 = 0x00000000,
			CIB_16 = 0x00000001,
			TIL_16 = 0x00000002,
			CIL_16 = 0x00000003,
			TI_16D = 0x00000010,
			CI_16D = 0x00000011
		};

		hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
		size_t pos = 32;
		uint32_t tag, len;
                /**  Variable Header  **/
                tag = beu32p(hdr->AS.Header+pos);
                while (tag) {
	                len = beu32p(hdr->AS.Header+pos+4)<<2;
	                pos += 8;
	                if (count < pos+len+8) {
	                	hdr->AS.Header = (uint8_t*) realloc(hdr->AS.Header,count*2);
	                	count += ifread(hdr->AS.Header+count, 1, count, hdr);
	                }
			if (VERBOSE_LEVEL>8)
		                fprintf(stdout,"%6i %6i tag=%08x len=%5i: |%c%c%c%c| %s\n", pos,count,tag, len, Header1[0x015f], Header1[0x0160], Header1[0x0161], Header1[0x0162], hdr->AS.Header+pos);

        		/* Appendix A */
        		switch (tag) {
        		case 0x00000002: break;
        		case 0x00000004:
        			strncpy(hdr->Patient.Name,Header1+pos,MAX_LENGTH_NAME);
        			break;
        		case 0x00000006:
        			strncpy(hdr->Patient.Id,Header1+pos,MAX_LENGTH_PID);
        			break;
        		case 0x00000008: {
        			struct tm t;
        			t.tm_mday = (Header1[pos+6]-'0')*10 + (Header1[pos+7]-'0');
        			Header1[pos+6] = 0;
        			t.tm_mon  = atoi(Header1+pos+4) + 1;
        			Header1[pos+4] = 0;
        			t.tm_year = atoi(Header1+pos) - 1900;
				t.tm_hour = 0;
				t.tm_min = 0;
				t.tm_sec = 0;
				hdr->Patient.Birthday = tm_time2gdf_time(&t);
        			break;
        			}
        		case 0x0000000a:
        			hdr->Patient.Sex = bei32p(Header1+pos);
        			break;
        		case 0x00000010:
        			hdr->SampleRate = atof(Header1+pos);
        			break;
        		case 0x00000012:
        			hdr->ID.Hospital = Header1+pos;
        			break;

        		case 0x00000003: // units
        			{
        			int k;
				char* ptr = Header1+pos;
        			for (k=0; k < hdr->NS; k++) {
					CHANNEL_TYPE *hc = hdr->CHANNEL + k;
       					hc->Cal = strtod(ptr, &ptr);
       					int c = 0;
       					char physdim[MAX_LENGTH_PHYSDIM+1];
        				while (bei32p(ptr)) {

						if (VERBOSE_LEVEL>8)
							fprintf(stdout,"0x03: [%i %i] %c\n",k,c,*ptr);

        					if (*ptr && (c<=MAX_LENGTH_PHYSDIM)) {
        						physdim[c++] = *ptr;
        					}
        					ptr++;
        				}
        				ptr += 4;
					physdim[c] = 0;
					hc->PhysDimCode = PhysDimCode(physdim);
        			}
        			}
        			break;

        		case 0x00000005:
        			{
        			int k;
				char* ptr = Header1+pos;
        			for (k=0; k < hdr->NS; k++) {
					CHANNEL_TYPE *hc = hdr->CHANNEL + k;
					int c = 0;
        				while (beu32p(ptr)) {
						if (VERBOSE_LEVEL>8)
							fprintf(stdout,"0x05: [%i %i] |%c%c%c%c%c%c%c%c|\n",k,c,ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7]);

						if ((*ptr) && (c<=MAX_LENGTH_LABEL)) {
        						hc->Label[c++] = *ptr;
        					}
        					ptr++;
        				}
					if (VERBOSE_LEVEL>8)
						fprintf(stdout,"0x05: %08x\n",beu32p(ptr));
					hc->Label[c] = 0;
        				ptr += 4;
        				while (bei32p(ptr)) ptr++;
        				ptr += 4;
				}
				}
				break;

        		case 0x0000000b: // recording time
        			if (Header1[pos+8]=='T') {
	        			struct tm t;
					t.tm_sec = atoi(Header1+pos+13);
        				Header1[pos+13] = 0;
					t.tm_min = atoi(Header1+pos+11);
        				Header1[pos+11] = 0;
					t.tm_hour = atoi(Header1+pos+9);
        				Header1[pos+8] = 0;
        				t.tm_mday = atoi(Header1+pos+6);
        				Header1[pos+6] = 0;
        				t.tm_mon  = atoi(Header1+pos+4) + 1;
	        			Header1[pos+4] = 0;
        				t.tm_year = atoi(Header1+pos) - 1900;
					hdr->T0 = tm_time2gdf_time(&t);
					if (VERBOSE_LEVEL>8)
		       				fprintf(stdout,"<%s>, T0 = %s\n",Header1+pos,asctime(&t));
        			}
				if (VERBOSE_LEVEL>8)
	       				fprintf(stdout,"<%s>\n",Header1+pos);
        			break;
        		case 0x0000000f: // filter
        			{
        			int k;
				char* ptr = Header1+pos;
        			for (k=0; k < hdr->NS; k++) {
					CHANNEL_TYPE *hc = hdr->CHANNEL + k;
        				switch (beu32p(ptr)) {
        				case 1: // lowpass
        					hc->LowPass  = strtod(ptr+4, &ptr);
        					break;
        				case 2: // high pass
        					hc->HighPass = strtod(ptr+4, &ptr);
        					break;
        				otherwise:
						fprintf(stderr,"Warning SOPEN (EBS): unknown filter\n");
        				}
        				while (bei32p(ptr) != -1) ptr++;
        				ptr += 4;
        			}
        			}
        			break;
        		}

			pos += len;
	                tag = beu32p(hdr->AS.Header+pos);
                }
                hdr->HeadLen = pos;
                ifseek(hdr,pos,SEEK_SET);
		hdr->AS.first = 0;
		hdr->AS.length = 0;
		hdr->ID.Hospital = "";

		if ((bei64p(Header1+24)==-1) && (bei64p(Header1+24)==-1)) {
			/* if data length is not present */
			struct stat FileBuf;
			stat(hdr->FileName,&FileBuf);
			datalen = (FileBuf.st_size - hdr->HeadLen);
		}
		else 	datalen <<= 2;

                /**  Encoded Signal Data (4*d bytes)  **/
                size_t spr = datalen/(2*hdr->NS);
                switch (EncodingID) {
                case TIB_16:
                	hdr->SPR = 1;
                	hdr->NRec = spr;
                	hdr->FILE.LittleEndian = 0;
                	break;
                case CIB_16:
                	hdr->SPR = spr;
                	hdr->NRec = 1;
                	hdr->FILE.LittleEndian = 0;
                	break;
                case TIL_16:
                	hdr->SPR = 1;
                	hdr->NRec = spr;
                	hdr->FILE.LittleEndian = 1;
                	break;
                case CIL_16:
                	hdr->SPR = spr;
                	hdr->NRec = 1;
                	hdr->FILE.LittleEndian = 1;
                	break;
                case TI_16D:
                case CI_16D:
                default:
                	B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
                	B4C_ERRMSG = "EBS: unsupported Encoding";
                	return(hdr);
                }

		typeof(hdr->NS) k;
		for (k = 0; k < hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL + k;
			hc->GDFTYP = 3; 	// int16
			hc->SPR    = hdr->SPR; 	// int16
			hc->bi     = k*2;
			hc->Off    = 0.0;
			hc->OnOff  = 1;
			hc->DigMax = (double)32767;
			hc->DigMin = (double)-32768;
			hc->PhysMax = hc->DigMax*hc->Cal;
			hc->PhysMin = hc->DigMin*hc->Cal;
			hc->Transducer[0] = 0;
			hc->Notch = NaN;
			hc->Impedance = INF;
		      	hc->fZ        = NaN;
			hc->XYZ[0] = 0.0;
			hc->XYZ[1] = 0.0;
			hc->XYZ[2] = 0.0;
		}
		hdr->AS.bpb = hdr->SPR*hdr->NS*2;

                /**  Optional Second Variable Header  **/

	}

	else if (hdr->TYPE==EEG1100) {
		// the information of this format is derived from nk2edf-0.43beta-src of Teunis van Beelen

		char *fn = (char*)malloc((strlen(hdr->FileName)+5)*sizeof(char));
		strcpy(fn,hdr->FileName);
		uint8_t *LOG=NULL;

		/* read .pnt */
			if (strrchr(fn,FILESEP))
				strcpy(strrchr(fn,FILESEP)+1, (char*)(hdr->AS.Header + 32));
			else
				strcpy(fn, (char*)(hdr->AS.Header + 32));

			FILE *fid = fopen(fn,"rb");
			if (fid != NULL) {
				count = 0;

			 	while (!feof(fid)) {
					LOG = (uint8_t*) realloc(LOG,count+1025);
					count += fread(LOG+count,1,1024,fid);
			 	}
				fclose(fid);

				LOG[count] = 0;
				// Name: @0x062e

				if (!hdr->FLAG.ANONYMOUS) {
					strncpy(hdr->Patient.Name, (char*)(LOG+0x62e), MAX_LENGTH_PID);
					hdr->Patient.Name[MAX_LENGTH_NAME] = 0;
				}

				// Id: @0x0604
				strncpy(hdr->Patient.Id, (char*)(LOG+0x604), MAX_LENGTH_PID);
				hdr->Patient.Id[MAX_LENGTH_PID] = 0;

				// Gender: @0x064a
				hdr->Patient.Sex = (toupper(LOG[0x064a])=='M') + 2*(toupper(LOG[0x064a])=='F') + 2*(toupper(LOG[0x064a])=='W');

				// Birthday: @0x0660
				sscanf((char*)(LOG+0x0660),"%04u/%02u/%02u",&tm_time.tm_year,&tm_time.tm_mon,&tm_time.tm_mday);
				tm_time.tm_hour  = 12;
				tm_time.tm_min   = 0;
				tm_time.tm_sec   = 0;
				tm_time.tm_year -= 1900;
				tm_time.tm_mon--;
				tm_time.tm_isdst = -1;
				hdr->Patient.Birthday = tm_time2gdf_time(&tm_time);

			}


		size_t n1,n2,pos1,pos2;
		n1 = hdr->AS.Header[145];
		if ((n1*20+0x92) > count) {
			hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,n1*20+0x92+1);
			count += ifread(hdr->AS.Header+count, 1, n1*20+0x92-count,hdr);
		}
		// Start date: @0x0040
		sscanf((char*)(hdr->AS.Header+0x40),"%04u%02u%02u%02u%02u%02u",&tm_time.tm_year,&tm_time.tm_mon,&tm_time.tm_mday,&tm_time.tm_hour,&tm_time.tm_min,&tm_time.tm_sec);
		tm_time.tm_year -= 1900;
		tm_time.tm_mon--;	// Jan=0, Feb=1, ...
		tm_time.tm_isdst = -1;
		//hdr->T0 = tm_time2gdf_time(&tm_time);

		int TARGET_SEGMENT = hdr->FLAG.TARGETSEGMENT;
		int numSegments = 0;

		size_t Total_NRec = 0;
		uint8_t *h2 = (uint8_t*)malloc(22);
		uint8_t *h3 = (uint8_t*)malloc(40);
		for (k=0; k<n1; k++) {
			pos1 = leu32p(hdr->AS.Header+146+k*20);
			ifseek(hdr, pos1, SEEK_SET);
			ifread(h2, 1, 22, hdr);
			n2 = h2[17];
			if (n2>1) {
				h2 = (uint8_t*)realloc(h2,2+n2*20);
				ifread(h2+22, 1, 2+n2*20-22, hdr);
			}

			hdr->EVENT.TYP = (typeof(hdr->EVENT.TYP)) realloc(hdr->EVENT.TYP,(hdr->EVENT.N+n2)*sizeof(*hdr->EVENT.TYP));
			hdr->EVENT.POS = (typeof(hdr->EVENT.POS)) realloc(hdr->EVENT.POS,(hdr->EVENT.N+n2)*sizeof(*hdr->EVENT.POS));
			hdr->EVENT.CHN = (typeof(hdr->EVENT.CHN)) realloc(hdr->EVENT.CHN,(hdr->EVENT.N+n2)*sizeof(*hdr->EVENT.CHN));
			hdr->EVENT.DUR = (typeof(hdr->EVENT.DUR)) realloc(hdr->EVENT.DUR,(hdr->EVENT.N+n2)*sizeof(*hdr->EVENT.DUR));

//			if (n2>1) fprintf(stdout,"EEG1100: more than 1 segment [%i,%i] not supported.\n",n1,n2);
			for (k2=0; k2<n2; k2++) {
				pos2 = leu32p(h2 + 18 + k2*20);

				ifseek(hdr, pos2, SEEK_SET);
				size_t pos3 = ifread(h3, 1, 40, hdr);

				// fprintf(stdout,"@%i: <%s>\n",pos3,(char*)h3+1);
				if (!strncmp((char*)h3+1,"TIME",4)) {
					sscanf((char*)(h3+5),"%02u%02u%02u",&tm_time.tm_hour,&tm_time.tm_min,&tm_time.tm_sec);
				}

				typeof(hdr->NS) NS = h3[38];
				typeof(hdr->SampleRate) SampleRate = leu16p(h3+26) & 0x3fff;
				nrec_t NRec = (nrec_t)(leu32p(h3+28) * SampleRate * 0.1);
				size_t HeadLen = pos2 + 39 + 10*NS;

				hdr->EVENT.TYP[hdr->EVENT.N] = 0x7ffe;
				hdr->EVENT.POS[hdr->EVENT.N] = Total_NRec;
				hdr->EVENT.DUR[hdr->EVENT.N] = NRec;
				hdr->EVENT.CHN[hdr->EVENT.N] = 0;
				Total_NRec += NRec;
				hdr->EVENT.N++;
				numSegments++;

				--TARGET_SEGMENT;	// decrease target segment counter
				if (TARGET_SEGMENT != 0) {
					continue;
				}

				hdr->T0 = tm_time2gdf_time(&tm_time);
				hdr->NS = NS;
				hdr->SampleRate = SampleRate;
				hdr->EVENT.SampleRate = SampleRate;
				hdr->NRec = NRec;
				hdr->HeadLen = HeadLen;
				hdr->SPR = 1;
				int16_t gdftyp = 128; // Nihon-Kohden int16 format
				hdr->AS.bpb = ((hdr->NS*GDFTYP_BITS[gdftyp])>>3)+2;

				// fprintf(stdout,"NK k=%i <%s> k2=%i <%s>\n",k,h2+1,k2,h3+1);
				// fprintf(stdout,"[%i %i]:pos=%u (%x) length=%Li(%Lx).\n",k,k2,pos2,pos2,hdr->NRec*(hdr->NS+1)*2,hdr->NRec*(hdr->NS+1)*2);

				h3 = (uint8_t*)realloc(h3,32 + hdr->NS*10);
				pos3 += ifread(h3+pos3, 1, 32+hdr->NS*10 - pos3, hdr);

				hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS,sizeof(CHANNEL_TYPE));
				int k3;
				for (k3=0; k3<hdr->NS; k3++) {
					CHANNEL_TYPE *hc = hdr->CHANNEL+k3;
					uint8_t u8 = h3[39+k3*10];
					switch (u8) {
					case 0: strcpy(hc->Label,"Fp1"); break;
					case 1: strcpy(hc->Label,"Fp2"); break;
					case 2: strcpy(hc->Label,"F3"); break;
					case 3: strcpy(hc->Label,"F4"); break;
					case 4: strcpy(hc->Label,"C3"); break;
					case 5: strcpy(hc->Label,"C4"); break;
					case 6: strcpy(hc->Label,"P3"); break;
					case 7: strcpy(hc->Label,"P4"); break;
					case 8: strcpy(hc->Label,"O1"); break;
					case 9: strcpy(hc->Label,"O2"); break;
					case 10: strcpy(hc->Label,"F7"); break;
					case 11: strcpy(hc->Label,"F8"); break;
					case 12: strcpy(hc->Label,"T3"); break;
					case 13: strcpy(hc->Label,"T4"); break;
					case 14: strcpy(hc->Label,"T5"); break;
					case 15: strcpy(hc->Label,"T6"); break;
					case 16: strcpy(hc->Label,"Fz"); break;
					case 17: strcpy(hc->Label,"Cz"); break;
					case 18: strcpy(hc->Label,"Pz"); break;
					case 19: strcpy(hc->Label,"E"); break;
					case 20: strcpy(hc->Label,"PG1"); break;
					case 21: strcpy(hc->Label,"PG2"); break;
					case 22: strcpy(hc->Label,"A1"); break;
					case 23: strcpy(hc->Label,"A2"); break;
					case 24: strcpy(hc->Label,"T1"); break;
					case 25: strcpy(hc->Label,"T2"); break;

					case 74: strcpy(hc->Label,"BN1"); break;
					case 75: strcpy(hc->Label,"BN2"); break;
					case 76: strcpy(hc->Label,"Mark1"); break;
					case 77: strcpy(hc->Label,"Mark2"); break;

					case 100: strcpy(hc->Label,"X12/BP1"); break;
					case 101: strcpy(hc->Label,"X13/BP2"); break;
					case 102: strcpy(hc->Label,"X14/BP3"); break;
					case 103: strcpy(hc->Label,"X15/BP4"); break;

					case 254: strcpy(hc->Label,"-"); break;
					case 255: strcpy(hc->Label,"Z"); break;
					default:
						if 	((25<u8)&&(u8<=36)) sprintf(hc->Label,"X%u",u8-25);
						else if ((36<u8)&&(u8<=41)) strcpy(hc->Label,"-");
						else if ((41<u8)&&(u8<=73)) sprintf(hc->Label,"DC%02u",u8-41);
						else if ((77<u8)&&(u8<=99)) strcpy(hc->Label,"-");
						else if	((103<u8)&&(u8<=254)) sprintf(hc->Label,"X%u",u8-88);
					}

					if ((41<u8) && (u8<=73)) {
						hc->PhysDimCode = 4274;	// mV
						hc->PhysMin = -12002.9;
						hc->PhysMax =  12002.56;
					} else {
						hc->PhysDimCode = 4275;    // uV
						hc->PhysMin = -3200.0;
						hc->PhysMax =  3200.0*((1<<15)-1)/(1<<15);
					}
				    	hc->GDFTYP = 128;	// Nihon-Kohden int16 format
					hc->DigMax =  32767.0;
					hc->DigMin = -32768.0;

					hc->Cal   	= (hc->PhysMax - hc->PhysMin) / (hc->DigMax - hc->DigMin);
					hc->Off   	=  hc->PhysMin - hc->Cal * hc->DigMin;
					hc->SPR    = 1;
				    	hc->LeadIdCode = 0;
					hc->OnOff  = 1;
					hc->Transducer[0] = 0;
					hc->bi = (k3*GDFTYP_BITS[gdftyp])>>3;

					// hc->LowPass  = 0.1;
					// hc->HighPass = 100;
					// hdr->CHANNEL[k3].Notch    = 0;
				}
			}
		}
		free(h2);
		free(h3);
		ifseek(hdr, hdr->HeadLen, SEEK_SET);
		if ((numSegments>1) && (hdr->FLAG.TARGETSEGMENT==1))
			fprintf(stdout,"File %s has more than one (%i) segment; use TARGET_SEGMENT argument to select other segments.\n",hdr->FileName,numSegments);


		/* read .log */
		char *c = strrchr(fn,'.');
		if (c != NULL) {
			strcpy(c+1,"log");
			FILE *fid = fopen(fn,"rb");
			if (fid == NULL) {
				strcpy(c+1,"LOG");
				fid = fopen(fn,"rb");
			}
			if (fid != NULL) {

				count = 0;
			 	while (!feof(fid)) {
					LOG = (uint8_t*) realloc(LOG,count+11521);
					count += fread(LOG+count,1,11520,fid);
			 	}
				fclose(fid);
				LOG[count]=0;

				for (k=0; k<LOG[145]; k++) {
					uint32_t lba = leu32p(LOG+146+k*20);
					uint32_t N = LOG[lba+18];
					hdr->EVENT.TYP = (typeof(hdr->EVENT.TYP)) realloc(hdr->EVENT.TYP,(hdr->EVENT.N+N)*sizeof(*hdr->EVENT.TYP));
					hdr->EVENT.POS = (typeof(hdr->EVENT.POS)) realloc(hdr->EVENT.POS,(hdr->EVENT.N+N)*sizeof(*hdr->EVENT.POS));
					hdr->EVENT.CHN = (typeof(hdr->EVENT.CHN)) realloc(hdr->EVENT.CHN,(hdr->EVENT.N+N)*sizeof(*hdr->EVENT.CHN));
					hdr->EVENT.DUR = (typeof(hdr->EVENT.DUR)) realloc(hdr->EVENT.DUR,(hdr->EVENT.N+N)*sizeof(*hdr->EVENT.DUR));
					size_t k1;
					for (k1=0; k1<N; k1++) {
						FreeTextEvent(hdr,hdr->EVENT.N,(char*)(LOG+lba+20+k1*45));
						// fprintf(stdout,"%s %s\n",(char*)(LOG+lba+20+k1*45),(char*)(LOG+lba+40+k1*45));
						sscanf((char*)(LOG+lba+46+k1*45),"(%02u%02u%02u%02u%02u%02u)",&tm_time.tm_year,&tm_time.tm_mon,&tm_time.tm_mday,&tm_time.tm_hour,&tm_time.tm_min,&tm_time.tm_sec);
						tm_time.tm_year += tm_time.tm_year<20 ? 100:0;
						tm_time.tm_mon--;	// Jan=0, Feb=1, ...
						gdf_time t0 = tm_time2gdf_time(&tm_time);

						if (t0 >= hdr->T0)
						{
							hdr->EVENT.POS[hdr->EVENT.N] = (uint32_t)(ldexp((t0 - hdr->T0)*86400*hdr->SampleRate,-32));        // 0-based indexing
							//hdr->EVENT.POS[hdr->EVENT.N] = (uint32_t)(atoi(strtok((char*)(LOG+lba+40+k1*45),"("))*hdr->SampleRate);
							hdr->EVENT.DUR[hdr->EVENT.N] = 0;
							hdr->EVENT.CHN[hdr->EVENT.N] = 0;
							hdr->EVENT.N++;
						}
					}
				}
			}
		}
		hdr->AS.auxBUF = LOG;
		free(fn);
	}

	else if (hdr->TYPE==EEProbe) {
		sopen_eeprobe(hdr);
	}

	else if (hdr->TYPE==EGI) {

		fprintf(stdout,"Reading EGI is under construction\n");

		uint16_t NEC = 0;	// specific for EGI format
		uint16_t gdftyp = 3;

		// BigEndian
		hdr->FILE.LittleEndian = 0;
		hdr->VERSION	= beu32p(hdr->AS.Header);
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
		uint16_t  Gain  = beu16p(Header1+24);
		uint16_t  Bits  = beu16p(hdr->AS.Header+26);
		uint16_t PhysMax= beu16p(hdr->AS.Header+28);
		size_t POS;
		if (hdr->AS.Header[3] & 0x01)
		{ 	// Version 3,5,7
			POS = 32;
			for (k=0; k < beu16p(hdr->AS.Header+30); k++) {
				char tmp[256];
				int  len = hdr->AS.Header[POS];
				strncpy(tmp,Header1+POS,len);
				tmp[len]=0;
				if (VERBOSE_LEVEL>7)
					fprintf(stdout,"EGI categorie %i: <%s>\n",k,tmp);

				POS += *(hdr->AS.Header+POS);	// skip EGI categories
				if (POS > count-8) {
					hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,2*count);
					count += ifread(hdr->AS.Header,1,count,hdr);
				}
			}

			hdr->NRec= beu16p(hdr->AS.Header+POS);
			hdr->SPR = beu32p(hdr->AS.Header+POS+2);
			NEC = beu16p(hdr->AS.Header+POS+6);	// EGI.N
			POS += 8;
		}
		else
		{ 	// Version 2,4,6
			hdr->NRec = beu32p(hdr->AS.Header+30);
			NEC = beu16p(hdr->AS.Header+34);	// EGI.N
			hdr->SPR  = 1;
			/* see also end-of-sopen
			hdr->AS.spb = hdr->SPR+NEC;
			hdr->AS.bpb = (hdr->NS + NEC)*GDFTYP_BITS[hdr->CHANNEL[0].GDFTYP]>>3;
			*/
			POS = 36;
		}

		/* read event code description */
		hdr->AS.auxBUF = (uint8_t*) realloc(hdr->AS.auxBUF,5*NEC);
		hdr->EVENT.CodeDesc = (typeof(hdr->EVENT.CodeDesc)) realloc(hdr->EVENT.CodeDesc,257*sizeof(*hdr->EVENT.CodeDesc));
		hdr->EVENT.CodeDesc[0] = "";	// typ==0, is always empty
		hdr->EVENT.LenCodeDesc = NEC+1;
		for (k=0; k < NEC; k++) {
			memcpy(hdr->AS.auxBUF+5*k,Header1+POS,4);
			hdr->AS.auxBUF[5*k+4]=0;
			hdr->EVENT.CodeDesc[k+1] = (char*)hdr->AS.auxBUF+5*k;
			POS += 4;
		}
		hdr->HeadLen = POS;
		ifseek(hdr,hdr->HeadLen,SEEK_SET);

		hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS,sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
			hc->GDFTYP = gdftyp;
			hc->PhysDimCode = 4275;  // "uV"
			hc->LeadIdCode  = 0;
			sprintf(hc->Label,"# %03i",k);
			hc->Cal	= PhysMax/ldexp(1,Bits);
			hc->Off	= 0;
			hc->SPR	= hdr->SPR;
			hc->bi	= k*hdr->SPR*GDFTYP_BITS[gdftyp]>>3;

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"SOPEN(EGI): #%i %i %f\n",k,Bits, PhysMax);

			if (Bits && PhysMax) {
				hc->PhysMax = PhysMax;
				hc->PhysMin = -PhysMax;
				hc->DigMax  = ldexp(1,Bits);
				hc->DigMin  = ldexp(-1,Bits);
			}
			else {
/*			hc->PhysMax = PhysMax;
			hc->PhysMin = -PhysMax;
			hc->DigMax  = ldexp(1,Bits);
			hc->DigMin  = ldexp(-1,Bits);
*/			hc->Cal     = 1.0;
			hc->OnOff   = 1;
			}
		}
		hdr->AS.bpb = (hdr->NS*hdr->SPR + NEC) * GDFTYP_BITS[gdftyp]>>3;
		if (hdr->AS.Header[3] & 0x01)	// triggered
			hdr->AS.bpb += 6;

		size_t N = 0;
		if (NEC > 0) {
			/* read event information */

			size_t sz	= GDFTYP_BITS[gdftyp]>>3;
			size_t len	= hdr->SPR * hdr->NRec * NEC * sz;

			uint8_t *buf 	= (uint8_t*)calloc(NEC, sz);
			uint8_t *buf8 	= (uint8_t*)calloc(NEC*2, 1);
			size_t *ix 	= (size_t*)calloc(NEC, sizeof(size_t));

			size_t skip	= hdr->AS.bpb - NEC * sz;
			ifseek(hdr, hdr->HeadLen + skip, SEEK_SET);
			int k1;
			nrec_t k;
			for (k=0; (k < hdr->NRec*hdr->SPR) && !ifeof(hdr); k++) {
				ifread(buf, sz,   NEC, hdr);
				ifseek(hdr, skip, SEEK_CUR);

				int off0, off1;
				if (k & 0x01)
				{ 	off0 = 0; off1=NEC; }
				else
				{ 	off0 = NEC; off1=0; }

				memset(buf8+off1,0,NEC);	// reset
				for (k1=0; k1 < NEC * sz; k1++)
					if (buf[k1]) buf8[ off1 + k1/sz ] = 1;

				for (k1=0; k1 < NEC ; k1++) {
					if (buf8[off1 + k1] && !buf8[off0 + k1]) {
						/* rising edge */
						ix[k1] = k;
					}
					else if (!buf8[off1 + k1] && buf8[off0 + k1]) {
						/* falling edge */
						if (N <= (hdr->EVENT.N + NEC*2)) {
							N += (hdr->EVENT.N+NEC)*2;	// allocate memory for this and the terminating line.
							hdr->EVENT.TYP = (typeof(hdr->EVENT.TYP)) realloc(hdr->EVENT.TYP, N*sizeof(*hdr->EVENT.TYP));
							hdr->EVENT.POS = (typeof(hdr->EVENT.POS)) realloc(hdr->EVENT.POS, N*sizeof(*hdr->EVENT.POS));
							hdr->EVENT.CHN = (typeof(hdr->EVENT.CHN)) realloc(hdr->EVENT.CHN, N*sizeof(*hdr->EVENT.CHN));
							hdr->EVENT.DUR = (typeof(hdr->EVENT.DUR)) realloc(hdr->EVENT.DUR, N*sizeof(*hdr->EVENT.DUR));
						}
						hdr->EVENT.TYP[hdr->EVENT.N] = k1+1;
						hdr->EVENT.POS[hdr->EVENT.N] = ix[k1];        // 0-based indexing
						hdr->EVENT.CHN[hdr->EVENT.N] = 0;
						hdr->EVENT.DUR[hdr->EVENT.N] = k-ix[k1];
						hdr->EVENT.N++;
						ix[k1] = 0;
					}
				}
			}

			for (k1 = 0; k1 < NEC; k1++)
			if (ix[k1]) {
				/* end of data */
				hdr->EVENT.TYP[hdr->EVENT.N] = k1+1;
				hdr->EVENT.POS[hdr->EVENT.N] = ix[k1];        // 0-based indexing
				hdr->EVENT.CHN[hdr->EVENT.N] = 0;
				hdr->EVENT.DUR[hdr->EVENT.N] = k-ix[k1];
				hdr->EVENT.N++;
				ix[k1] = 0;
			}

			hdr->EVENT.SampleRate = hdr->SampleRate;
			free(buf);
			free(buf8);
			free(ix);
		}
		ifseek(hdr,hdr->HeadLen,SEEK_SET);

		/* TODO: check EGI format */
	}


#ifdef WITH_EGIS
	else if (hdr->TYPE==EGIS) {
		fprintf(stdout,"Reading EGIS is under construction\n");

#if __BYTE_ORDER == __BIG_ENDIAN
		char FLAG_SWAP = hdr->FLAG.LittleEndian;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
		char FLAG_SWAP = hdr->FLAG.LittleEndian;
#endif
		hdr->VERSION = *(int16_t*) mfer_swap8b(hdr->AS.Header+4, sizeof(int16_t), char FLAG_SWAP);
		hdr->HeadLen = *(uint16_t*) mfer_swap8b(hdr->AS.Header+6, sizeof(uint16_t), char FLAG_SWAP);
		//hdr->HeadLen = *(uint32_t*) mfer_swap8b(hdr->AS.Header+8, sizeof(uint32_t), char FLAG_SWAP);

		/* read file */
		hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,hdr->HeadLen+1);
		count  += ifread(hdr->AS.Header+count,1,hdr->HeadLen-count,hdr);
		hdr->AS.Header[count]=0;

	}
#endif

#ifdef WITH_EMBLA
	else if (hdr->TYPE==EMBLA) {
		strncpy(hdr->CHANNEL[k].Label,buf+0x116,MAX_LENGTH_LABEL);
	}
#endif

    	else if (hdr->TYPE==ET_MEG) {
		B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
		B4C_ERRMSG = "FLT/ET-MEG format not supported";
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

		hdr->Patient.Birthday = hdr->T0 - (uint64_t)ldexp(age,32);
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
		    	hc->PhysDimCode = 65362;	//mmol l-1 mm
	    		hc->bi   = k*GDFTYP_BITS[gdftyp]>>3;

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
    		hdr->AS.bpb = hdr->NS*GDFTYP_BITS[gdftyp]>>3;

		/* decode data section */
		// hdr->FLAG.SWAP = 0;
		hdr->FILE.LittleEndian = (__BYTE_ORDER == __LITTLE_ENDIAN);

		uint32_t pos;
		int Mark=0,hh,mm,ss,ds,BodyMovement,RemovalMark,PreScan;
		size_t NEV = 16;
		hdr->EVENT.N = 0;
		hdr->EVENT.SampleRate = hdr->SampleRate;
		hdr->EVENT.DUR = NULL;
		hdr->EVENT.CHN = NULL;

		pos = atol(strtok(NULL,dlm));
		while (pos) {
			hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata, ((hdr->NRec+1) * hdr->NS * GDFTYP_BITS[gdftyp])>>3);
			for (k=0; k < hdr->NS; k++) {
			if (gdftyp==16)
				*(float*)(hdr->AS.rawdata  + ((hdr->NRec*hdr->NS+k)*GDFTYP_BITS[gdftyp]>>3)) = (float)atof(strtok(NULL,dlm));
			else if (gdftyp==17)
				*(double*)(hdr->AS.rawdata + ((hdr->NRec*hdr->NS+k)*GDFTYP_BITS[gdftyp]>>3)) = atof(strtok(NULL,dlm));
			}
			++hdr->NRec;

			Mark = atoi(strtok(NULL,dlm));
			if (Mark) {
                                if (hdr->EVENT.N+1 >= NEV) {
                                        NEV<<=1;        // double allocated memory
        		 		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, NEV*sizeof(*hdr->EVENT.POS) );
        				hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, NEV*sizeof(*hdr->EVENT.TYP) );
        			}
				hdr->EVENT.POS[hdr->EVENT.N] = pos;         // 0-based indexing
				hdr->EVENT.TYP[hdr->EVENT.N] = Mark;
				if (FLAG_StimType_STIM && !(hdr->EVENT.N & 0x01))
					hdr->EVENT.TYP[hdr->EVENT.N] = Mark | 0x8000;
				++hdr->EVENT.N;
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
			hdr->EVENT.POS[hdr->EVENT.N-1] = pos;         // 0-based indexing
			hdr->EVENT.TYP[hdr->EVENT.N-1] = Mark | 0x8000;
		}
		hdr->AS.length  = hdr->NRec;
	}

#ifdef WITH_FAMOS
    	else if (hdr->TYPE==FAMOS) {
    		hdr->HeadLen=count;
		sopen_FAMOS_read(hdr);
	}
#endif

    	else if (hdr->TYPE==FEF) {
#ifdef WITH_FEF
		size_t bufsiz = 1l<<24;
		while (!ifeof(hdr)) {
		    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,count+bufsiz+1);
		    	count  += ifread(hdr->AS.Header+count,1,bufsiz,hdr);
		}
		hdr->AS.Header[count]=0;
		hdr->HeadLen = count;

    		tmp[8] = 0;
    		memcpy(tmp, hdr->AS.Header+8, 8);
    		hdr->VERSION = atol(tmp)/100.0;
    		memcpy(tmp, hdr->AS.Header+24, 8);
    		hdr->FILE.LittleEndian = !atol(tmp);
    		ifseek(hdr,32,SEEK_SET);

		if (VERBOSE_LEVEL>7) fprintf(stdout,"ASN1 [401] %i\n",count);
		sopen_fef_read(hdr);
		if (VERBOSE_LEVEL>7) fprintf(stdout,"ASN1 [491]\n");
#else
		B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
		B4C_ERRMSG = "VITAL/FEF Format not supported\n";
		return(hdr);

#endif
	}

    	else if (hdr->TYPE==HDF) {
#ifdef _HDF5_H
#endif
		ifclose(hdr);
		B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
		B4C_ERRMSG = "Format HDF not supported\n";
		return(hdr);
	}

    	else if (hdr->TYPE==HEKA) {
    		// HEKA PatchMaster file format
		hdr->HeadLen = count;

    		sopen_zzztest(hdr);

	}

    	else if (hdr->TYPE==ITX) {
#define IGOR_MAXLENLINE 400

    		char line[IGOR_MAXLENLINE+1];
    		char flagData = 0;
    		char flagSupported = 1;

	        if (VERBOSE_LEVEL>7)
                        fprintf(stdout,"[701] start reading %s,v%4.2f format \n",GetFileTypeString(hdr->TYPE),hdr->VERSION);

		typeof(hdr->SPR) SPR = 0, spr = 0;
		typeof(hdr->NS)  ns  = 0, NS  = 0;
    		size_t DIV = 1;
		int chanNo=0, PrevChanNo=0, sweepNo=0, PrevSweepNo=0;
    		hdr->SPR = 0;
    		hdr->NRec= 0;

		/*
			checks the structure of the file and extracts formating information
		*/

    		ifseek(hdr,0,SEEK_SET);
    		while (!ifeof(hdr)) {
			ifgets(line, IGOR_MAXLENLINE, hdr);

			if (!strncmp(line,"BEGIN",5)) {
	    			flagData = 1;
	    			spr = 0;
	    			hdr->CHANNEL[ns].bi = SPR*sizeof(double);
	    		}
	    		else if (!strncmp(line,"END",3)) {
	    			flagData = 0;
                                if ((SPR!=0) && (SPR != spr))
					flagSupported = 0;
				else
					SPR = spr;

				PrevChanNo  = chanNo;
				PrevSweepNo = sweepNo;
				if (ns==0) hdr->SPR += SPR;
	    		}

	    		else if (!strncmp(line,"X SetScale/P x,",15)) {
	    			strtok(line,",");
	    			strtok(NULL,",");
	    			double dur = atof(strtok(NULL,","));
	    			char *p = strchr(line,'"');
	    			if (p != NULL) {
	    				p++;
	    				char *p2 = strchr(p,'"');
	    				if (p2 != NULL) *p2=0;
	    				dur *= PhysDimScale(PhysDimCode(p));
	    			}

	    			double div = spr / (hdr->SampleRate * dur);
	    			if (ns==0) {
	    				hdr->SampleRate = 1.0 / dur;
				}
	    			else if (hdr->SampleRate == 1.0 / dur)
	    				;
	    			else if (div == floor(div)) {
	    				hdr->SampleRate *= div;
	    			}
	    		}

	    		else if (!strncmp(line,"X SetScale y,",13)) {
	    			char *p = strchr(line,'"');
	    			if (p!=NULL) {
	    				p++;
	    				char *p2 = strchr(p,'"');
	    				if (p2!=NULL) *p2=0;
					if (hdr->CHANNEL[ns].PhysDimCode == 0)
		    				hdr->CHANNEL[ns].PhysDimCode = PhysDimCode(p);
					else if (hdr->CHANNEL[ns].PhysDimCode != PhysDimCode(p))
						flagSupported = 0;	// physical units do not match
	    			}
	    		}
	    		else if (!strncmp(line,"WAVES",5)) {
				char *p;
				p = strrchr(line,'_'); chanNo  = atoi(p+1); ns = chanNo - 1; p[0] = 0;
				p = strrchr(line,'_'); sweepNo = atoi(p+1); if (sweepNo > hdr->NRec) hdr->NRec = sweepNo; p[0] = 0;

				flagSupported &= (ns==0) || (chanNo == PrevChanNo+1); 	// reset flag if channel number does not increment by one.
				flagSupported &= (ns==0 && sweepNo==PrevSweepNo+1) || (sweepNo == PrevSweepNo); 	// reset flag if sweep number does not increment by one when chanNo==0.

				if (ns >= hdr->NS) {
					hdr->NS = ns+1;
					hdr->CHANNEL = (CHANNEL_TYPE*)realloc(hdr->CHANNEL, hdr->NS * sizeof(CHANNEL_TYPE));
				}
				CHANNEL_TYPE* hc = hdr->CHANNEL + ns;
	    			strncpy(hc->Label, line+6, MAX_LENGTH_LABEL);

 				hc->OnOff    = 1;
        			hc->GDFTYP   = 17;
        			hc->DigMax   = (double)(int16_t)(0x7fff);
        			hc->DigMin   = (double)(int16_t)(0x8000);
				hc->LeadIdCode = 0;

				hc->Cal      = 1.0;
				hc->Off      = 0.0;
				hc->Transducer[0] = '\0';
				hc->LowPass  = NaN;
				hc->HighPass = NaN;
				hc->PhysMax  = hc->Cal * hc->DigMax;
				hc->PhysMin  = hc->Cal * hc->DigMin;
				hc->PhysDimCode = 0;

				// decode channel number and sweep number
                if (VERBOSE_LEVEL>7)
                        fprintf(stdout,"[776]<%s>#%i: %i/%i\n",line,(int)ns,(int)spr,(int)hdr->SPR);

	    		}
	    		else if (flagData)
	    			spr++;
	    	}

                if (VERBOSE_LEVEL>7)
                        fprintf(stdout,"[751] scaning %s,v%4.2f format \n",GetFileTypeString(hdr->TYPE),hdr->VERSION);

		if (!flagSupported) {
			B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
			B4C_ERRMSG = "This ITX format is not supported. Possible reasons: not generated by Heka-Patchmaster, corrupted, physical units do not match between sweeos, or do not fullfil some other requirements\n";
			return(hdr);
		}

                if (VERBOSE_LEVEL>7)
                        fprintf(stdout,"[781] [%i,%i,%i] = %i\n",(int)hdr->NS,(int)hdr->SPR,(int)hdr->NRec,(int)hdr->NRec*hdr->SPR*hdr->NS);

		hdr->EVENT.N = hdr->NRec - 1;
		hdr->EVENT.SampleRate = hdr->SampleRate;
		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS,hdr->EVENT.N*sizeof(*hdr->EVENT.POS));
		hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP,hdr->EVENT.N*sizeof(*hdr->EVENT.TYP));

		hdr->NRec = hdr->SPR;
		hdr->SPR  = 1;
		hdr->AS.first  = 0;
		hdr->AS.length = hdr->NRec;
		hdr->AS.bpb = sizeof(double)*hdr->NS;
	    	for (ns=0; ns<hdr->NS; ns++) {
			hdr->CHANNEL[ns].SPR = hdr->SPR;
			hdr->CHANNEL[ns].bi  = sizeof(double)*ns;
		}

       		double *data = (double*)realloc(hdr->AS.rawdata,hdr->NRec*hdr->SPR*hdr->NS*sizeof(double));
		hdr->FILE.LittleEndian = (__BYTE_ORDER == __LITTLE_ENDIAN);   // no swapping
		hdr->AS.rawdata = (uint8_t*) data;

		/*
			reads and converts data into biosig structure
		*/

	    	spr = 0;SPR = 0;
    		ifseek(hdr, 0, SEEK_SET);
    		while (!ifeof(hdr)) {
	    		ifgets(line, IGOR_MAXLENLINE, hdr);
	    		if (!strncmp(line,"BEGIN",5)) {
	    			flagData = 1;
				spr = 0;
	    		}
	    		else if (!strncmp(line,"END",3)) {
	    			flagData = 0;
				if (chanNo+1 == hdr->NS) SPR += spr;
	    		}
	    		else if (!strncmp(line,"X SetScale y,",13)) {
	    			//ns++;
	    		}
	    		else if (!strncmp(line,"WAVES",5)) {
				// decode channel number and sweep number
				char *p;
				p = strrchr(line,'_'); chanNo  = atoi(p+1)-1; p[0] = 0;
				p = strrchr(line,'_'); sweepNo = atoi(p+1)-1; p[0] = 0;
				spr = 0;
				if (sweepNo > 0 && chanNo==0) {
					hdr->EVENT.POS[sweepNo-1] = SPR;
					hdr->EVENT.TYP[sweepNo-1] = 0x7ffe;
				}
			}
	    		else if (flagData) {
				double val = atof(line);
				data[hdr->NS*(SPR + spr) + chanNo] = val;
				spr++;
	    		}
	    	}

                if (VERBOSE_LEVEL>7)
			fprintf(stdout,"[791] reading %s,v%4.2f format finished \n",GetFileTypeString(hdr->TYPE),hdr->VERSION);

		hdr->SPR   = 1;
		hdr->NRec *= hdr->SPR;
		hdr->AS.first  = 0;
		hdr->AS.length = hdr->NRec;
		hdr->AS.bpb = sizeof(double)*hdr->NS;
#undef IGOR_MAXLENLINE
	}

    	else if (hdr->TYPE==ISHNE) {
                   // unknown, generic, X,Y,Z, I-VF, V1-V6, ES, AS, AI
    	        uint16_t Table1[] = {0,0,16,17,18,1,2,87,88,89,90,3,4,5,6,7,8,131,132,133};
    	        size_t len;
    	        struct tm  t;
		size_t bufsiz = 522;
		hdr->HeadLen = 522 + lei32p(hdr->AS.Header+10);
                if (count < hdr->HeadLen) {
		    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,hdr->HeadLen);
		    	count  += ifread(hdr->AS.Header+count,1,hdr->HeadLen-count,hdr);
		}
		hdr->HeadLen = count;

		int offsetVarHdr = lei32p(hdr->AS.Header+18);
		int offsetData = lei32p(hdr->AS.Header+22);
		hdr->VERSION = (float)lei16p(hdr->AS.Header+26);

                if (!hdr->FLAG.ANONYMOUS) {
                        len = max(80,MAX_LENGTH_NAME);
	               	strncpy(hdr->Patient.Name, (char*)(hdr->AS.Header+28), len);
	        	hdr->Patient.Name[len] = 0;
	        }
                len = max(20, MAX_LENGTH_PID);
		strncpy(hdr->Patient.Id, (char*)(hdr->AS.Header+108), len);
		hdr->Patient.Id[len] = 0;

                hdr->Patient.Sex = lei16p(hdr->AS.Header+128);
                // Race = lei16p(hdr->AS.Header+128);

                t.tm_mday  = lei16p(hdr->AS.Header+132);
                t.tm_mon   = lei16p(hdr->AS.Header+134)-1;
                t.tm_year  = lei16p(hdr->AS.Header+136)-1900;
                t.tm_hour  = 12;
                t.tm_min   = 0;
                t.tm_sec   = 0;
                hdr->Patient.Birthday = tm_time2gdf_time(&t);

                t.tm_mday  = lei16p(hdr->AS.Header + 138);
                t.tm_mon   = lei16p(hdr->AS.Header + 140) - 1;
                t.tm_year  = lei16p(hdr->AS.Header + 142) - 1900;
                //t.tm_hour  = lei16p(hdr->AS.Header + 144);
                //t.tm_min   = lei16p(hdr->AS.Header + 146);
                //t.tm_sec   = lei16p(hdr->AS.Header + 148);
                t.tm_hour  = lei16p(hdr->AS.Header + 150);
                t.tm_min   = lei16p(hdr->AS.Header + 152);
                t.tm_sec   = lei16p(hdr->AS.Header + 154);
                hdr->T0    = tm_time2gdf_time(&t);

                hdr->NS    = lei16p(hdr->AS.Header + 156);
		hdr->SPR   = 1;
		hdr->NRec  = lei32p(hdr->AS.Header+14)/hdr->NS;
                hdr->AS.bpb = hdr->NS*2;
                hdr->SampleRate = lei16p(hdr->AS.Header + 272);
                hdr->Patient.Impairment.Heart = lei16p(hdr->AS.Header+230) ? 3 : 0;    // Pacemaker
		hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS, sizeof(CHANNEL_TYPE));
		for (k=0; k < hdr->NS; k++) {
			CHANNEL_TYPE* hc = hdr->CHANNEL+k;
			hc->OnOff    = 1;
			if (hdr->VERSION == 1) {
        			hc->GDFTYP   = 3;        //int16 - 2th complement
        			hc->DigMax   = (double)(int16_t)(0x7fff);
        			hc->DigMin   = (double)(int16_t)(0x8000);
        		}
        		else {
        			hc->GDFTYP   = 4;        //uint16
        			hc->DigMax   = (double)(uint16_t)(0xffff);
        			hc->DigMin   = (double)(uint16_t)(0x0000);
        		}
			hc->SPR      = 1;
			hc->Cal      = lei16p(hdr->AS.Header + 206 + 2*k);
			hc->Off      = 0.0;
			hc->Transducer[0] = '\0';
			hc->LowPass  = NaN;
			hc->HighPass = NaN;
			hc->PhysMax  = hc->Cal * hc->DigMax;
			hc->PhysMin  = hc->Cal * hc->DigMin;
		    	hc->LeadIdCode  = Table1[lei16p(hdr->AS.Header + 158 + 2*k)];
		    	hc->PhysDimCode = 4276;	        // nV

	    		hc->bi       = k*2;
	    		strcpy(hc->Label, LEAD_ID_TABLE[hc->LeadIdCode]);
		}
    		ifseek(hdr,offsetData,SEEK_SET);
    		hdr->FILE.POS = 0;
	}

    	else if (hdr->TYPE==MFER) {
		// ISO/TS 11073-92001:2007(E), Table 5, p.9
    		/* ###  FIXME: some units are not encoded */
    		const uint16_t MFER_PhysDimCodeTable[30] = {
    			4256, 3872, 3840, 3904,65330,	// Volt, mmHg, Pa, mmH2O, mmHg/s
    			3808, 3776,  544, 6048, 2528,	// dyne, N, %, °C, 1/min
    			4264, 4288, 4160,65376, 4032,	// 1/s, Ohm, A, rpm, W
    			6448, 1731, 3968, 6016, 1600,	// dB, kg, J, dyne s m-2 cm-5, l
    			3040, 3072, 4480,    0,    0,	// l/s, l/min, cd
    			   0,    0,    0,    0,    0,	//
		};

		hdr->FLAG.OVERFLOWDETECTION = 0; 	// MFER does not support automated overflow and saturation detection

	    	uint8_t buf[128];
		uint8_t gdftyp = 3; 	// default: int16
		uint8_t UnitCode=0;
		double Cal = 1.0, Off = 0.0;
		char SWAP = ( __BYTE_ORDER == __LITTLE_ENDIAN);   // default of MFER is BigEndian
		hdr->FILE.LittleEndian = 0;
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
				hdr->FILE.LittleEndian = buf[0];
#if (__BYTE_ORDER == __BIG_ENDIAN)
				SWAP = hdr->FILE.LittleEndian;
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
				SWAP = !hdr->FILE.LittleEndian;
#endif
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
				hdr->SPR = *(int64_t*) mfer_swap8b(buf, len, SWAP);
			}
			else if (tag==5)     //0x05: number of channels
			{
				if (len>4) fprintf(stderr,"Warning MFER tag5 incorrect length %i>4\n",len);
				curPos += ifread(buf,1,len,hdr);
				hdr->NS = *(int64_t*) mfer_swap8b(buf, len, SWAP);
				hdr->CHANNEL = (CHANNEL_TYPE*)realloc(hdr->CHANNEL, hdr->NS*sizeof(CHANNEL_TYPE));
				for (k=0; k<hdr->NS; k++) {
					hdr->CHANNEL[k].SPR = 0;
					hdr->CHANNEL[k].PhysDimCode = 0;
					hdr->CHANNEL[k].Cal = 1.0;
				}
			}
			else if (tag==6) 	// 0x06 "number of sequences"
			{
				// NRec
				if (len>4) fprintf(stderr,"Warning MFER tag6 incorrect length %i>4\n",len);
				curPos += ifread(buf,1,len,hdr);
				hdr->NRec = *(int64_t*) mfer_swap8b(buf, len, SWAP);
			}
			else if (tag==8) {
				// Type of Waveform
				uint8_t TypeOfWaveForm8[2];
				uint16_t TypeOfWaveForm;
				if (len>2) fprintf(stderr,"Warning MFER tag8 incorrect length %i>2\n",len);
				curPos += ifread(&TypeOfWaveForm8,1,len,hdr);
				if (len==1)
					TypeOfWaveForm = TypeOfWaveForm8[0];
				else if (SWAP)
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
			else if (tag==11)    //0x0B
			{
				// Fs
				if (len>6) fprintf(stderr,"Warning MFER tag11 incorrect length %i>6\n",len);
				double  fval;
				curPos += ifread(buf,1,len,hdr);
				fval = *(int64_t*) mfer_swap8b(buf+2, len-2, SWAP);

				hdr->SampleRate = fval*pow(10.0, (int8_t)buf[1]);
				if (buf[0]==1)  // s
					hdr->SampleRate = 1.0/hdr->SampleRate;
			}
			else if (tag==12)    //0x0C
			{
				// sampling resolution
				if (len>6) fprintf(stderr,"Warning MFER tag12 incorrect length %i>6\n",len);
				val32   = 0;
				int8_t  v8;
				curPos += ifread(&UnitCode,1,1,hdr);
				curPos += ifread(&v8,1,1,hdr);
				curPos += ifread(buf,1,len-2,hdr);
				Cal = *(int64_t*) mfer_swap8b(buf, len-2, SWAP);
				Cal *= pow(10.0,v8);
				if (!MFER_PhysDimCodeTable[UnitCode])
					fprintf(stderr,"Warning MFER: unsupported physical unit (code=%i)\n", UnitCode);
			}
			else if (tag==13) {
				if (len>8) fprintf(stderr,"Warning MFER tag13 incorrect length %i>8\n",len);
				curPos += ifread(&buf,1,len,hdr);
				if      (gdftyp == 1) Off = ( int8_t)buf[0];
				else if (gdftyp == 2) Off = (uint8_t)buf[0];
				else if (SWAP) {
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
				for (k=0; isprint(hdr->ID.Manufacturer._field[k]) && (k<MAX_LENGTH_MANUF); k++) ;
				hdr->ID.Manufacturer._field[k]    = 0;
				hdr->ID.Manufacturer.Name         = strtok(hdr->ID.Manufacturer._field,"^");
				hdr->ID.Manufacturer.Model        = strtok(NULL,"^");
				hdr->ID.Manufacturer.Version      = strtok(NULL,"^");
				hdr->ID.Manufacturer.SerialNumber = strtok(NULL,"^");
		     		if (hdr->ID.Manufacturer.Name == NULL) hdr->ID.Manufacturer.Name="\0";
		     		if (hdr->ID.Manufacturer.Model == NULL) hdr->ID.Manufacturer.Model="\0";
	     			if (hdr->ID.Manufacturer.Version == NULL) hdr->ID.Manufacturer.Version="\0";
	     			if (hdr->ID.Manufacturer.SerialNumber == NULL) hdr->ID.Manufacturer.SerialNumber="\0";
			}
			else if (tag==30)     //0x1e: waveform data
			{
				// data block
				hdr->AS.rawdata = (uint8_t*)realloc(hdr->AS.rawdata,len);
				hdr->HeadLen    = curPos;
				curPos += ifread(hdr->AS.rawdata,1,len,hdr);
				hdr->AS.first = 0;
				hdr->AS.length= hdr->NRec;
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
						hdr->CHANNEL[chan].SPR = *(int64_t*) mfer_swap8b(buf, len2, SWAP);
					}
					else if (tag2==9) {	//leadname
						if (len2==2)
							hdr->CHANNEL[chan].LeadIdCode = 0;
						else if (len2==1)
							hdr->CHANNEL[chan].LeadIdCode = buf[0];
						else if (len2<=32)
							strncpy(hdr->CHANNEL[chan].Label,(char*)buf,len2);
						else
							fprintf(stderr,"Warning MFER tag63-9 incorrect length %i>32\n",len2);
					}
					else if (tag2==11) {	// sampling resolution
						if (len2>6) fprintf(stderr,"Warning MFER tag63-11 incorrect length %i>6\n",len2);
						double  fval;
						fval = *(int64_t*) mfer_swap8b(buf+2, len2-2, SWAP);

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
						double cal = *(int64_t*) mfer_swap8b(buf+2, len2-2, SWAP);
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
			else if (tag==64)     //0x40
			{
				// preamble
				curPos += ifread(tmp,1,len,hdr);
				if (VERBOSE_LEVEL>7) {
					fprintf(stdout,"Preamble: pos=%i|",curPos);
					for (k=0; k<len; k++) fprintf(stdout,"%c",tmp[k]);
					fprintf(stdout,"|\n");
				}
			}
			else if (tag==65)     //0x41: patient event
			{
				// event table
				curPos += ifread(buf,1,len,hdr);
				if (len>2) {
					hdr->EVENT.N++;

					if (VERBOSE_LEVEL>8)
						fprintf(stdout,"MFER-event: N=%i\n",hdr->EVENT.N);

					hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS,hdr->EVENT.N*sizeof(*hdr->EVENT.POS));
					hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP,hdr->EVENT.N*sizeof(*hdr->EVENT.TYP));
					hdr->EVENT.DUR = (uint32_t*) realloc(hdr->EVENT.DUR,hdr->EVENT.N*sizeof(*hdr->EVENT.DUR));
					hdr->EVENT.CHN = (uint16_t*) realloc(hdr->EVENT.CHN,hdr->EVENT.N*sizeof(*hdr->EVENT.CHN));

					hdr->EVENT.CHN[hdr->EVENT.N] = 0;
					hdr->EVENT.DUR[hdr->EVENT.N] = 0;
					if (SWAP) {
						hdr->EVENT.TYP[hdr->EVENT.N] = bswap_16(*(uint16_t*)(buf));
						hdr->EVENT.POS[hdr->EVENT.N] = bswap_32(*(uint32_t*)(buf+2));   // 0-based indexing
						if (len>6)
							hdr->EVENT.DUR[hdr->EVENT.N] = bswap_32(*(uint32_t*)(buf+6));
					}
					else {
						hdr->EVENT.TYP[hdr->EVENT.N] = *(uint16_t*)buf;
						hdr->EVENT.POS[hdr->EVENT.N] = *(uint32_t*)(buf+2);   // 0-based indexing
						if (len>6)
							hdr->EVENT.DUR[hdr->EVENT.N] = *(uint32_t*)(buf+6);
					}
				}
			}
			else if (tag==66)     //0x42: NIPB, SpO2(value)
			{
			}

			else if (tag==129)   //0x81
			{
				if (!hdr->FLAG.ANONYMOUS)
					curPos += ifread(hdr->Patient.Name,1,len,hdr);
				else 	{
					ifseek(hdr,len,SEEK_CUR);
					curPos += len;
				}
			}

			else if (tag==130)    //0x82
			{
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

			else if (tag==131)    //0x83
			{
				// Patient Age
				if (len!=7) fprintf(stderr,"Warning MFER tag131 incorrect length %i!=7\n",len);
				curPos += ifread(buf,1,len,hdr);
				tm_time.tm_year = *(uint16_t*)(buf+3);
				if (SWAP) tm_time.tm_year = bswap_16(tm_time.tm_year);
		    		tm_time.tm_year-= 1900;
		    		tm_time.tm_mon  = buf[5]-1;
		    		tm_time.tm_mday = buf[6];
		    		tm_time.tm_hour = 12;
		    		tm_time.tm_min  = 0;
		    		tm_time.tm_sec  = 0;
				hdr->Patient.Birthday  = tm_time2gdf_time(&tm_time);
				//hdr->Patient.Age = buf[0] + cswap_u16(*(uint16_t*)(buf+1))/365.25;
			}
			else if (tag==132)    //0x84
			{
				// Patient Sex
				if (len!=1) fprintf(stderr,"Warning MFER tag132 incorrect length %i!=1\n",len);
				curPos += ifread(&hdr->Patient.Sex,1,len,hdr);
			}
			else if (tag==133)    //0x85
			{
				curPos += ifread(buf,1,len,hdr);
				tm_time.tm_year = *(uint16_t*)buf;
				if (SWAP) tm_time.tm_year = bswap_16(tm_time.tm_year);
				tm_time.tm_year-= 1900;
		    		tm_time.tm_mon  = buf[2] - 1;
		    		tm_time.tm_mday = buf[3];
		    		tm_time.tm_hour = buf[4];
		    		tm_time.tm_min  = buf[5];
		    		tm_time.tm_sec  = buf[6];

				hdr->T0  = tm_time2gdf_time(&tm_time);
				// add milli- and micro-seconds
				if (SWAP)
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
	 	hdr->AS.bpb = 0;
	 	for (k=0; k<hdr->NS; k++) {

	 		if (VERBOSE_LEVEL>8)
	 			fprintf(stdout,"sopen(MFER): #%i\n",k);

			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
	 		if (!hc->PhysDimCode) hc->PhysDimCode = MFER_PhysDimCodeTable[UnitCode];
	 		if (hc->Cal==1.0) hc->Cal = Cal;
	 		hc->Off = Off * hc->Cal;
	 		if (!hc->SPR) hc->SPR = hdr->SPR;
	 		hc->GDFTYP = gdftyp;
	 		if (gdftyp<16)
	 			if (gdftyp & 0x01) {
		 			hc->DigMax = ldexp( 1.0,GDFTYP_BITS[gdftyp]-1) - 1.0;
		 			hc->DigMin = ldexp(-1.0,GDFTYP_BITS[gdftyp]-1);
	 			}
	 			else {
	 				hc->DigMax = ldexp( 1.0,GDFTYP_BITS[gdftyp]);
		 			hc->DigMin = 0.0;
	 			}
	 		else {
	 			hc->DigMax =  INF;
		 		hc->DigMin = -INF;
	 		}
	 		hc->PhysMax = hc->DigMax * hc->Cal + hc->Off;
	 		hc->PhysMin = hc->DigMin * hc->Cal + hc->Off;
	    		hc->OnOff   = 1;
	    		hc->bi      = hdr->AS.bpb;
	    		hdr->AS.bpb += hdr->SPR*GDFTYP_BITS[gdftyp]>>3;
	 	}

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"[MFER] -V=%i NE=%i\n",VERBOSE_LEVEL,hdr->EVENT.N);
//	 	hdr2ascii(hdr,stdout,4);
	}

	else if (hdr->TYPE==MIT) {
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[MIT 111]: %i \n",VERBOSE_LEVEL);

    		int bufsiz = 1024;
	    	while (!ifeof(hdr)) {
			hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,count+bufsiz);
		    	count += ifread(hdr->AS.Header+count, 1, bufsiz, hdr);
	    	}
	    	ifclose(hdr);

		/* MIT: decode header information */
		if (VERBOSE_LEVEL>8)
		    	fprintf(stdout,"[MIT 112]: %s\n",(char*)hdr->AS.Header);

	    	hdr->SampleRate = 250.0;
	    	hdr->NRec = 0;
	    	hdr->SPR  = 1;
	    	size_t NumberOfSegments = 1;
		char *ptr = (char*)hdr->AS.Header;
	    	char *line;
	    	do line = strtok((char*)hdr->AS.Header,"\x0d\x0a"); while ((line != NULL) && (line[0]=='#'));

		ptr = strpbrk(line,"\x09\x0a\x0d\x20"); 	// skip 1st field
		ptr[0] = 0;
		if (VERBOSE_LEVEL>8)
		    	fprintf(stdout,"[MIT 113]:<%s>\n",ptr);

		if (strchr(line,'/') != NULL) {
			NumberOfSegments = atol(strchr(line,'/')+1);
			B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
			B4C_ERRMSG = "MIT/HEA/PhysioBank: multi-segment records are not supported\n";
		}
		hdr->NS = (typeof(hdr->NS))strtod(ptr+1,&ptr);		// number of channels

		if (VERBOSE_LEVEL>8)
		    	fprintf(stdout,"[MIT 121]: NS=%i\n",hdr->NS);

	    	if ((ptr != NULL) && strlen(ptr)) {
			if (VERBOSE_LEVEL>8)
			    	fprintf(stdout,"123: <%s>\n",ptr);
			hdr->SampleRate = strtod(ptr,&ptr);
			if (ptr[0]=='/') {
				double CounterFrequency = strtod(ptr+1,&ptr);
			}
			if (ptr[0]=='(') {
				double BaseCounterValue = strtod(ptr+1,&ptr);
				ptr++; // skip ")"
			}
	    	}
	    	if ((ptr != NULL) && strlen(ptr)) {
			hdr->NRec = (nrec_t)strtod(ptr,&ptr);
		}
	    	if ((ptr != NULL) && strlen(ptr)) {
			struct tm t;
			sscanf(ptr," %u:%u:%u %u/%u/%u",&t.tm_hour,&t.tm_min,&t.tm_sec,&t.tm_mday,&t.tm_mon,&t.tm_year);
			t.tm_mon--;
			t.tm_year -= 1900;
			t.tm_isdst = -1;
			hdr->T0 = tm_time2gdf_time(&t);
		}


		int fmt=0,FMT=0;
		size_t MUL=1;
		char **DatFiles = (char**)calloc(hdr->NS, sizeof(char*));
		size_t *ByteOffset = (size_t*)calloc(hdr->NS, sizeof(size_t));
		size_t nDatFiles = 0;
		uint16_t gdftyp,NUM=1,DEN=1;
		hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS, sizeof(CHANNEL_TYPE));
		hdr->AS.bpb8 = 0;
		hdr->AS.bpb=0;
		for (k=0; k < hdr->NS; k++) {

			double skew=0;
			double byteoffset=0;
			double ADCgain=200;
			double baseline=0;
			double ADCresolution=12;
			double ADCzero=0;
			double InitialValue=0;
			double BlockSize=0;

			CHANNEL_TYPE* hc = hdr->CHANNEL+k;
		    	do line = strtok(NULL,"\x0d\x0a"); while (line[0]=='#'); // read next line

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"[MIT 111] #%i <%s>\n",k, line);

			for (ptr=line; !isspace(ptr[0]); ptr++) {}; 	// skip 1st field
			ptr[0]=0;
			if (k==0)
				DatFiles[nDatFiles++]=line;
			else if (strcmp(DatFiles[nDatFiles-1],line))
				DatFiles[nDatFiles++]=line;

			fmt = (typeof(fmt))strtod(ptr+1,&ptr);
			if (k==0) FMT = fmt;
			else if (FMT != fmt) {
				B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
				B4C_ERRMSG = "MIT/HEA/PhysioBank: different formats within a single data set is not supported\n";
			}

			size_t DIV=1;
			if (ptr[0]=='x') {
				DIV = (size_t)strtod(ptr+1,&ptr);
				hdr->CHANNEL[k].SPR *= DIV;
				MUL = lcm(MUL,DIV);
			}
			hdr->CHANNEL[k].SPR = DIV;

			if (ptr[0]==':') skew = strtod(ptr+1,&ptr);
			if (ptr[0]=='+') ByteOffset[k] = (size_t)strtod(ptr+1,&ptr);

			if (ptr != NULL) ADCgain = strtod(ptr+1,&ptr);
			if (ptr[0] == '(') {
				baseline = strtod(ptr+1,&ptr);
				ptr++;
			}
			hc->PhysDimCode = 4274; // mV
			if (ptr[0] == '/') {
				char  *PhysUnits = ++ptr;
				while (!isspace(ptr[0])) ++ptr;
				ptr[0] = 0;
				hc->PhysDimCode = PhysDimCode(PhysUnits);
			}

			if (ptr != NULL) ADCresolution = strtod(ptr+1,&ptr);
			if (ptr != NULL) ADCzero = strtod(ptr+1,&ptr);
			if (ptr != NULL) InitialValue = strtod(ptr+1,&ptr);
			else InitialValue = ADCzero;

			double checksum;
			if (ptr != NULL) checksum = strtod(ptr+1,&ptr);
			if (ptr != NULL) BlockSize = strtod(ptr+1,&ptr);
			while (isspace(ptr[0])) ++ptr;

			strncpy(hdr->CHANNEL[k].Label,ptr,MAX_LENGTH_LABEL);

			hc->Cal      = 1/ADCgain;
			hc->Off      = -ADCzero*hc->Cal;
			hc->OnOff    = 1;
			hc->Transducer[0] = '\0';
			hc->LowPass  = -1;
			hc->HighPass = -1;
			// hdr->FLAG.SWAP = (__BYTE_ORDER == __BIG_ENDIAN);
			hdr->FILE.LittleEndian = 1;
			switch (fmt) {
			case 8:
				gdftyp = 1;
				hc->DigMax =  127.0;
				hc->DigMin = -128.0;
				B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
				B4C_ERRMSG = "MIT/HEA/PhysioBank format 8(diff) not supported.\n";
				break;
			case 80:
				gdftyp = 2; 	// uint8;
				hc->Off= -128*hc->Cal;
				hc->DigMax = 255.0;
				hc->DigMin = 0.0;
				break;
			case 16:
			 	gdftyp = 3;
				NUM = 2; DEN = 1;
				hc->DigMax = ldexp( 1.0,15)-1.0;
				hc->DigMin = ldexp(-1.0,15);
				break;
			case 61:
				gdftyp = 3;
				// hdr->FLAG.SWAP = !(__BYTE_ORDER == __BIG_ENDIAN);
				hdr->FILE.LittleEndian = 0;
				NUM = 2; DEN = 1;
				hc->DigMax =  ldexp( 1.0,15)-1.0;
				hc->DigMin =  ldexp(-1.0,15);
				break;
			case 160:
			 	gdftyp = 4; 	// uint16;
				hc->Off= ldexp(-1.0,15)*hc->Cal;
				NUM = 2; DEN = 1;
				hc->DigMax = ldexp(1.0,16)-1.0;
				hc->DigMin = 0.0;
				break;
			case 212:
			 	gdftyp = 255+12;
				NUM = 3; DEN = 2;
				hc->DigMax =  ldexp( 1.0,11)-1.0;
				hc->DigMin =  ldexp(-1.0,11);
				break;
			case 310:
			case 311:
				gdftyp = 255+10;
				NUM = 4; DEN = 3;
				hc->DigMax = ldexp( 1.0,9)-1.0;
				hc->DigMin = ldexp(-1.0,9);
				B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
				B4C_ERRMSG = "MIT/HEA/PhysioBank format 310/311 not supported.\n";
				break;
			default:
				gdftyp = 0xffff;
				B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
				B4C_ERRMSG = "MIT/HEA/PhysioBank: unknown format.\n";
			}

			hc->GDFTYP   = gdftyp;
		    	hc->LeadIdCode  = 0;
	 		hc->PhysMax = hc->DigMax * hc->Cal + hc->Off;
	 		hc->PhysMin = hc->DigMin * hc->Cal + hc->Off;
	 		hc->bi8 = hdr->AS.bpb8;
			hdr->AS.bpb8 += (hdr->SPR*NUM<<3)/DEN;
	 		hc->bi = hdr->AS.bpb;
			hdr->AS.bpb += hdr->AS.bpb8>>3;

			if (VERBOSE_LEVEL>8)
			    	fprintf(stdout,"[MIT 150] #%i: FMT=%i bi8=%i\n",k+1,fmt,hc->bi8);

		}
		hdr->SampleRate *= MUL;
		hdr->SPR 	*= MUL;

		if (!hdr->NRec)
			hdr->NRec = (hdr->HeadLen + count)/hdr->AS.bpb;

		/* read age, sex etc. */
		line = strtok(NULL,"\x0d\x0a");
		if (line != NULL) {
			char *t1;
			double age=0.0;
			for (k=0; k<strlen(line); k++) line[k]=toupper(line[k]);
			t1 = strstr(line,"AGE:");
			if (t1 != NULL) age = strtod(t1+4,&ptr);
			t1 = strstr(line,"AGE>:");
			if (t1 != NULL) age = strtod(t1+5,&ptr);
			if (age>0.0)
				hdr->Patient.Birthday = hdr->T0 - (uint64_t)ldexp(age*365.25,32);

			t1 = strstr(line,"SEX:");
			if (t1 != NULL) t1 += 4;
			else {
				t1 = strstr(line,"SEX>:");
				if (t1 != NULL) t1 += 5;
			}
			if (t1 != NULL) {
			        while (isspace(t1[0])) t1++;
			        hdr->Patient.Sex = (t1[0]=='M') + 2* (t1[0]=='F');
			}
		}

		if (VERBOSE_LEVEL>8)
		    	fprintf(stdout,"[MIT 177] #%i: (%i) %s FMT=%i+%i\n",k+1,nDatFiles,DatFiles[0],fmt,ByteOffset[0]);

		/* MIT: read ATR annotation file */
		uint16_t *Marker=NULL;
		count = 0;

		const char *f0 = hdr->FileName;
		char *f1 = (char*) malloc(strlen(hdr->FileName)+5);
		strcpy(f1,hdr->FileName);
		strcpy(strrchr(f1,'.')+1,"atr");
		hdr->FileName = f1;
		hdr   = ifopen(hdr,"r");
		if (!hdr->FILE.OPEN) {
			// if no *.atr file, try *.qrs
			strcpy(strrchr(f1,'.')+1,"qrs");
			hdr   = ifopen(hdr,"r");
		}
		if (!hdr->FILE.OPEN) {
			// *.ecg
			strcpy(strrchr(f1,'.')+1,"ecg");
			hdr   = ifopen(hdr,"r");
		}
		if (hdr->FILE.OPEN) {
		    	while (!ifeof(hdr)) {
				Marker = (uint16_t*)realloc(Marker,(count+bufsiz)*2);
			    	count += ifread(Marker+count, 2, bufsiz, hdr);
		    	}
		    	ifclose(hdr);

                        /* define user specified events according to http://www.physionet.org/physiotools/wfdb/lib/ecgcodes.h */
        		hdr->EVENT.CodeDesc = (typeof(hdr->EVENT.CodeDesc)) realloc(hdr->EVENT.CodeDesc,257*sizeof(*hdr->EVENT.CodeDesc));
        		for (k=0; strlen(MIT_EVENT_DESC[k])>0; k++) {
                                //hdr->EVENT.CodeDesc[k+1] = MIT_EVENT_DESC[k];
                                hdr->EVENT.CodeDesc[k+1] = (char*)MIT_EVENT_DESC[k];   // hack to satisfy MinGW (gcc version 4.2.1-sjlj)
        		        if (VERBOSE_LEVEL>7) fprintf(stdout,"[MIT 182] %i %s %s\n",k,MIT_EVENT_DESC[k],hdr->EVENT.CodeDesc[k]);
                        }
        		hdr->EVENT.LenCodeDesc = k+1;

			if (VERBOSE_LEVEL>7) fprintf(stdout,"[MIT 183] %s %i\n",f1,count);

			/* decode ATR annotation information */
			size_t N = count;
			hdr->EVENT.TYP = (typeof(hdr->EVENT.TYP)) realloc(hdr->EVENT.TYP,N*sizeof(*hdr->EVENT.TYP));
			hdr->EVENT.POS = (typeof(hdr->EVENT.POS)) realloc(hdr->EVENT.POS,N*sizeof(*hdr->EVENT.POS));
			hdr->EVENT.CHN = (typeof(hdr->EVENT.CHN)) realloc(hdr->EVENT.CHN,N*sizeof(*hdr->EVENT.CHN));

			hdr->EVENT.N   = 0;
			hdr->EVENT.SampleRate = hdr->SampleRate;
			uint16_t chn   = 0;
			size_t pos     = 0;
			char flag_chn  = 0;
			for (k=0; (k<N) && Marker[k]; k++) {
				uint16_t a16 = leu16p(Marker+k);
				uint16_t A   = a16 >> 10;
				uint16_t len = a16 & 0x03ff;

//				if (VERBOSE_LEVEL>8) fprintf(stdout,"[MIT 183] k=%i/%i N=%i A=%i l=%i\n", k, N, hdr->EVENT.N, a16>>10, len);

				switch (A) {
				case 59:	// SKIP
					pos += (((uint32_t)leu16p(Marker+k+1))<<16) + leu16p(Marker+k+2);
					k   += 2;
					break;
				case 60:	// NUM
				case 61:	// SUB
					break;
				case 62: 	// CHN
					chn = len;
					flag_chn = flag_chn || chn;
					break;
				case 63: 	// AUX
					k += (len+1)/2;
					break;
				default:
					pos += len;
					// code = 0 is mapped to 49(ACMAX), see MIT_EVENT_DESC and http://www.physionet.org/physiotools/wfdb/lib/ecgcodes.h
					hdr->EVENT.TYP[hdr->EVENT.N] = (A==0 ? 49 : A);
					hdr->EVENT.POS[hdr->EVENT.N] = pos-1;   // convert to 0-based indexing
					hdr->EVENT.CHN[hdr->EVENT.N] = chn;
					++hdr->EVENT.N;
				}
			}
			if (flag_chn)
				hdr->EVENT.DUR = (typeof(hdr->EVENT.DUR)) realloc(hdr->EVENT.DUR,N*sizeof(*hdr->EVENT.DUR));
			else {
				free(hdr->EVENT.CHN);
				hdr->EVENT.CHN = NULL;
			}
			free(Marker);
		}
		free(f1);
		hdr->FileName = f0;

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[MIT 185] \n");

		/* MIT: open data file */
		if (nDatFiles == 1) {
			const char *f0 = hdr->FileName;
			char *f1 = (char*) malloc(strlen(hdr->FileName)+strlen(DatFiles[0])+2);
			strcpy(f1,hdr->FileName);
			hdr->FileName = f1;
			char *ptr = (char*) strrchr(hdr->FileName,FILESEP);
			if (ptr != NULL)
				strcpy(ptr+1,DatFiles[0]);
			else
				strcpy(f1,DatFiles[0]);

			hdr->HeadLen = ByteOffset[0];
			hdr = ifopen(hdr,"rb");
			ifseek(hdr, hdr->HeadLen, SEEK_SET);

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[MIT 186] %s\n",hdr->FileName);

			count  = 0;
			bufsiz = 1<<20;
		    	while (!ifeof(hdr)) {
				hdr->AS.rawdata = (uint8_t*)realloc(hdr->AS.rawdata,(count+bufsiz));
			    	count += ifread(hdr->AS.rawdata+count, 1, bufsiz, hdr);
		    	}
		    	ifclose(hdr);

			free(f1);
			hdr->FileName = f0;

			if (!hdr->NRec) {
				hdr->NRec = count/(hdr->AS.bpb);
			}
		}

		if (VERBOSE_LEVEL>8)
		    	fprintf(stdout,"[MIT 198] #%i: (%i) %s FMT=%i\n",k+1,nDatFiles,DatFiles[0],fmt);

		free(DatFiles);
		free(ByteOffset);

		if (VERBOSE_LEVEL>8)
		    	fprintf(stdout,"[MIT 199] #%i: (%i) %s FMT=%i\n",k+1,nDatFiles,DatFiles[0],fmt);

		if (nDatFiles != 1) {
			B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
			B4C_ERRMSG = "MIT/HEA/PhysioBank: multiply data files within a single data set is not supported\n";
			return(hdr);
		}
		hdr->AS.length  = hdr->NRec;

	} /* END OF MIT FORMAT */

#ifdef CHOLMOD_H
	else if ((hdr->TYPE==MM) && (!hdr->FILE.COMPRESSION)) {

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[MM 001] %i,%i\n",count,hdr->FILE.COMPRESSION);

		ifseek(hdr,0,SEEK_SET);
                cholmod_common c ;
                cholmod_start (&c) ; /* start CHOLMOD */
                c.print = 5;
                hdr->Calib = cholmod_read_sparse (hdr->FILE.FID, &c); /* read in a matrix */

                if (VERBOSE_LEVEL>8)
                        cholmod_print_sparse (hdr->Calib, "Calib", &c); /* print the matrix */
                cholmod_finish (&c) ; /* finish CHOLMOD */

		ifclose(hdr);
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[MM 003] %i\n",count);
		return(hdr);
	} /* END OF MatrixMarket */
#endif

	else if (hdr->TYPE==NEURON) {
		hdr->HeadLen = count;

	if (VERBOSE_LEVEL>7) fprintf(stdout,"NEURON: start\n");

		size_t count;
		while (!ifeof(hdr)) {
			count = max(1<<20, hdr->HeadLen*2);
			hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, count);
			hdr->HeadLen  += ifread(hdr->AS.Header + hdr->HeadLen, 1, count - hdr->HeadLen - 1, hdr);
		}
		hdr->AS.Header[hdr->HeadLen] = 0;
		hdr->NS  = 1;
		hdr->SPR = 1;
		hdr->CHANNEL = (CHANNEL_TYPE*)realloc(hdr->CHANNEL, hdr->NS * sizeof(CHANNEL_TYPE));
		hdr->CHANNEL[0].GDFTYP   = 17;
		hdr->CHANNEL[0].Cal      = 1.0;
		hdr->CHANNEL[0].Off      = 0.0;
		hdr->CHANNEL[0].PhysMin  = -1e9;
		hdr->CHANNEL[0].PhysMax  = +1e9;
		hdr->CHANNEL[0].DigMin   = -1e9;
		hdr->CHANNEL[0].DigMax   = +1e9;
		hdr->AS.bpb = sizeof(double);
		hdr->CHANNEL[0].bi       = 0;
		hdr->CHANNEL[0].bi8      = 0;
		hdr->CHANNEL[0].LeadIdCode = 0;
		hdr->CHANNEL[0].SPR      = hdr->SPR;
		hdr->CHANNEL[0].LowPass  = NAN;
		hdr->CHANNEL[0].HighPass = NAN;
		hdr->CHANNEL[0].Notch    = NAN;

	if (VERBOSE_LEVEL>7) fprintf(stdout,"NEURON 202: \n");

		char *t = strtok( (char*)hdr->AS.Header, "\x0A\x0D");
		char status = 0;
		size_t spr  = 0;

		while (t != NULL) {

	if (VERBOSE_LEVEL>8) fprintf(stdout,"NEURON 301: <%s>\n", t);

			if (status==0) {
				if (!strncmp(t,"Header:", 7))
					status = 1;
			}
			else if (status==1) {
	if (VERBOSE_LEVEL>7) fprintf(stdout,"NEURON 311: <%s>\n",t);
				char *val = t+strlen(t);
				while (isspace(*(--val))) {}; val[1]=0;	// remove trailing blanks
				val = strchr(t,':');			// find right value 
				val[0] = 0;
				while (isspace(*(++val))) {};

				if (!strncmp(t,"Data", 7))
					status=2;

				else if (!strcmp(t,"SampleInt"))
					hdr->SampleRate = 1.0 / atof(val);

				else if (!strcmp(t,"Points")) {
					hdr->NRec = atoi(val);
					hdr->AS.rawdata = (uint8_t*)realloc(hdr->AS.rawdata, sizeof(double) * hdr->NRec);
				}
				else if (!strcmp(t,"XUnit")) {
					uint16_t xunits = PhysDimCode(val);
					double   scale = PhysDimScale(xunits);
					if ((xunits & 0xffe0)==2176) hdr->SampleRate /= scale;
					else fprintf(stdout, "Error NEURON: invalid XUnits <%s>\n", val);
				}

				else if (!strcmp(t,"YUnit")) {
	if (VERBOSE_LEVEL>7) fprintf(stdout,"NEURON 321: Yunits:<%s>\n",val);
					hdr->CHANNEL[0].PhysDimCode = PhysDimCode(val);
				}
				else if (!strcmp(t,"Method")) {
					strncpy(hdr->CHANNEL[0].Label, val, MAX_LENGTH_LABEL);
				}
			}
			else if (status==2) {
				*(double*)(hdr->AS.rawdata + spr*sizeof(double)) = atof(t);
			}
			t = strtok(NULL, "\x0A\x0D");
		}
		free(hdr->AS.Header); 
		hdr->AS.Header = NULL;
	}


	else if (hdr->TYPE==NIFTI) {
	    	count += ifread(hdr->AS.Header+count, 1, 352-count, hdr);
	    	// nifti_1_header *NIFTI_HDR = (nifti_1_header*)hdr-AS.Header;
	    	char SWAP = *(int16_t*)(Header1+40) > 7;
#if   (__BYTE_ORDER == __BIG_ENDIAN)
		hdr->FILE.LittleEndian = SWAP;
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
		hdr->FILE.LittleEndian = !SWAP;
#endif
	    	if (!SWAP) {
		    	hdr->HeadLen = (size_t)*(float*)(Header1+80);
		}
		else {
			union {uint32_t u32; float f32;} u;
			u.u32 = bswap_32(*(uint32_t*)(Header1+108));
		    	hdr->HeadLen = (size_t)u.f32;
		}

		if (Header1[345]=='i') {
			ifclose(hdr);
			const char *f0 = hdr->FileName;
			char *f1 = (char*)malloc(strlen(hdr->FileName)+4);
			strcpy(f1,hdr->FileName);
			strcpy(strrchr(f1,'.') + 1, "img");
			hdr->FileName = f1;
			hdr = ifopen(hdr,"r");
			hdr->FileName = f0;
		}
		else
			ifseek(hdr,hdr->HeadLen,SEEK_SET);

#ifdef _NIFTI_HEADER_
		nifti_1_header *NIFTI_HDR = (nifti_1_header*)hdr->AS.Header;
#endif

		ifclose(hdr);
		B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
		B4C_ERRMSG = "Format NIFTI not supported\n";
		return(hdr);
	}

    	else if (hdr->TYPE==Persyst) {

		while (~ifeof(hdr)) {
			hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,count*2+1);
		    	count += ifread(hdr->AS.Header + count, 1, count, hdr);
		}
		hdr->AS.Header[count] = 0;
		ifclose(hdr);
		int status = 0;
		size_t pos=0;
		char *line = strtok(Header1,"\n\r");
		while (pos<count) {
			if (!strncmp(line,"[FileInfo]",10))
				status = 1;
			else if (!strncmp(line,"[ChannelMap]",12))
				status = 2;
			else if (!strncmp(line,"[Sheets]",8))
				status = 3;
			else if (!strncmp(line,"[Comments]",10))
				status = 4;
			else if (!strncmp(line,"[Patient]",9))
				status = 5;
			else if (!strncmp(line,"[SampleTimes]",13))
				status = 6;

			else if (status==1) {
//					filename = strchr(line,'=')+1;
					if (!strncmp(line,"File",4))
					;
//						hdr->FILE.FID = fopen(''
					else if (!strncmp(line,"File",4))
					;

				}


			line = strtok(NULL,"\n\r");
		}

		uint16_t gdftyp = 3;


    		B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
    		B4C_ERRMSG = "Format Persyst not supported,yet.";
	}

	else if (hdr->TYPE==RDF) {

		// UCSD ERPSS aquisition system

		#define RH_F_CONV   0x0001   /* converted from other format */
		#define RH_F_RCOMP  0x0002   /* raw file is compressed */
		#define RH_F_DCMAP  0x4000   /* channel mapping used during dig. */

		#define RH_CHANS       256   /* maximum # channels */
		#define RH_DESC         64   /* max # chars in description */
		#define RH_CHDESC        8   /* max # chars in channel descriptor */
		#define RH_SFILL         4   /* short filler */
		#define RH_LFILL         6   /* long filler */
		#define RH_ALOG        828   /* max # chars in ASCII log */

		typedef struct rawhdr {
			uint16_t rh_byteswab;          /* ERPSS byte swab indicator */
			uint16_t rh_magic;             /* file magic number */
			uint16_t rh_flags;             /* flags */
			uint16_t rh_nchans;            /* # channels */
			uint16_t rh_calsize;           /* (if norm, |cal|) */
			uint16_t rh_res;               /* (if norm, in pts/unit) */
			uint16_t rh_pmod;              /* # times processed */
			uint16_t rh_dvers;             /* dig. program version */
			uint16_t rh_l2recsize;         /* log 2 record size */
			uint16_t rh_recsize;           /* record size in pts */
			uint16_t rh_errdetect;         /* error detection in effect */
			uint16_t rh_chksum;            /* error detection chksum */
			uint16_t rh_tcomp;             /* log 2  time comp/exp */
			uint16_t rh_narbins;           /* (# art. rej. count slots) */
			uint16_t rh_sfill[RH_SFILL];   /* short filler (to 16 slots) */
			uint16_t rh_nrmcs[RH_CHANS];   /* (cal sizes used for norm.) */
			uint32_t rh_time;              /* creation time, secs since 1970 */
			uint32_t rh_speriod;           /* digitization sampling period */
			uint32_t rh_lfill[RH_LFILL];   /* long filler (to 8 slots) */
			char     rh_chdesc[RH_CHANS][RH_CHDESC]; /* chan descriptions */
			char     rh_chmap[RH_CHANS];   /* input chan mapping array */
			char     rh_subdesc[RH_DESC];  /* subject description */
			char     rh_expdesc[RH_DESC];  /* experimenter description */
			char     rh_ename[RH_DESC];    /* experiment name */
			char     rh_hname[RH_DESC];    /* host machine name */
			char     rh_filedesc[RH_DESC]; /* file description */
			char     rh_arbdescs[RH_DESC]; /* (art. rej. descriptions) */
			char     rh_alog[RH_ALOG];     /* ASCII log */
		} RAWHDR;

		if (count < sizeof(struct rawhdr)) {
			hdr->HeadLen = sizeof(struct rawhdr);
		    	hdr->AS.Header = (uint8_t*) realloc(hdr->AS.Header,hdr->HeadLen+1);
    			count += ifread(Header1+count, 1, hdr->HeadLen-count, hdr);
			hdr->AS.Header[hdr->HeadLen]=0;
		}

		hdr->NS = *(uint16_t*)(hdr->AS.Header+2);
		uint32_t T0 = *(uint32_t*)(hdr->AS.Header+32);	// seconds since 1970
		hdr->SampleRate = 1e6 / (*(uint32_t*)(hdr->AS.Header+36));
		strncpy(hdr->Patient.Id, (const char*)hdr->AS.Header+32+24+256*9, min(64,MAX_LENGTH_PID));
		hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS,sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL + k;
			hc->OnOff = 1;
			strncmp(hc->Label,(char*)(hdr->AS.Header+32+24+8*k),8);
		}

    		B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
    		B4C_ERRMSG = "Format RDF (UCSD ERPSS) not supported,yet.";
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
		// hdr->FLAG.SWAP = 0; 	// no swapping
		hdr->FILE.LittleEndian = (__BYTE_ORDER == __LITTLE_ENDIAN); 	// no swapping
		hdr->AS.length = hdr->NRec;
	}

	else if (hdr->TYPE==Sigma) {  /********* Sigma PLpro ************/
		hdr->HeadLen = leu32p(hdr->AS.Header+16);
	    	hdr->AS.Header = (uint8_t*) realloc(hdr->AS.Header,hdr->HeadLen+1);
    		count += ifread(Header1+count, 1, hdr->HeadLen-count, hdr);
		hdr->AS.Header[hdr->HeadLen]=0;

		struct tm t;
		char *tag, *val;
		size_t pos = leu32p(hdr->AS.Header+28);
		uint8_t len;

		int k;
		for (k=0; k<5; k++) {
#define line ((char*)(hdr->AS.Header+pos))
			len = strcspn(line,"\x0a\x0d");
			line[len] = 0;
			tag = line;
			val = strchr(line,'=');
			if (val!=NULL) {
				val[0] = 0;
				val++;
			}

		if (VERBOSE_LEVEL>7) fprintf(stdout,"%i: %s=%s\n",k,tag,val);

			if (0) {}
			//else if (!strcmp(tag,"Name")) {}
			//else if (!strcmp(tag,"Vorname")) {}
			else if (!strcmp(tag,"GebDat")) {
				sscanf(val,"%02u.%02u.%04u",&t.tm_mday,&t.tm_mon,&t.tm_year);
				t.tm_year -=1900;
				t.tm_min--;
				t.tm_hour = 12;
				t.tm_min = 0;
				t.tm_sec = 0;
				t.tm_isdst = -1;
				hdr->T0 = tm_time2gdf_time(&t);
			}
			else if (!strcmp(tag,"ID"))
				strncpy(hdr->Patient.Id,val,MAX_LENGTH_PID);

			pos += len+1;
			while ((line[0]==10) || (line[0]==13)) pos++;
		}

		if (VERBOSE_LEVEL>7) fprintf(stdout,"333 SIGMA  pos=%i, 0x%x\n", pos, pos);

		hdr->NS = leu16p(hdr->AS.Header+pos);
		hdr->SampleRate = 128;
		hdr->SPR  = 1;
		hdr->NRec = -1;		// unknown
		struct stat stbuf;
		if(!stat(hdr->FileName, &stbuf))
		if (!hdr->FILE.COMPRESSION)
			hdr->NRec = (stbuf.st_size-hdr->HeadLen)/(2*hdr->NS);
		else
			fprintf(stdout,"Compressed Sigma file (%s) is currently not supported. Uncompress file and try again.", hdr->FileName);


		if (VERBOSE_LEVEL>7) fprintf(stdout,"333 SIGMA NS=%i/0x%x, Fs=%f, SPR=%i, NRec=%i\n",hdr->NS,hdr->NS, hdr->SampleRate,hdr->SPR,hdr->NRec);

	       	// define variable header
		pos     = 148;
		hdr->FLAG.UCAL = 1;
		hdr->FLAG.OVERFLOWDETECTION = 0;
		hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS,sizeof(CHANNEL_TYPE));

		uint16_t p[] = {4,19,19,19,19+2,19,19,19,19+8,9,11};	// difference of positions of string elements within variable header
		for (k=1; k < sizeof(p)/sizeof(p[0]); k++) p[k] += p[k-1];	// relative position

		double *fs = (double*) malloc(hdr->NS * sizeof(double));
		double minFs = 1.0/0.0;
		for (k=0; k<hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
			pos = 148 + k*203;
			// ch = lei16p(hdr->AS.Header+pos);
			double val;
			hc->GDFTYP = 3;
			hc->OnOff  = 1;
			hc->SPR    = 1;
		      	hc->DigMax    = (int16_t)0x7fff;
		      	hc->DigMin    = (int16_t)0x8000;
	      		hc->PhysMax   = hc->DigMax;
		      	hc->PhysMin   = hc->DigMin;
//		      	hc->Cal       = 1.0;
	      		hc->Off       = 0.0;
		      	hc->HighPass  = NaN;
		      	hc->LowPass   = NaN;
		      	hc->Notch     = *(int16_t*)(hdr->AS.Header+pos+2) ? 1.0 : 0.0;
		      	hc->Impedance = INF;
		      	hc->fZ        = NaN;
	      		hc->XYZ[0]    = 0.0;
		      	hc->XYZ[1]    = 0.0;
		      	hc->XYZ[2]    = 0.0;

			int k1;
			for (k1 = sizeof(p)/sizeof(p[0]); k1>0; ) {
				k1--;
				len = hdr->AS.Header[pos+p[k1]];
				hdr->AS.Header[pos+p[k1]+len+1] = 0;
				val = atof((char*)(hdr->AS.Header+pos+p[k1]+1));
				switch (k1) {
				case 0: 	// Abtastrate
					fs[k] = val;
					if (hdr->SampleRate < fs[k]) hdr->SampleRate=fs[k];
					if (minFs > fs[k]) minFs=fs[k];
					break;
				case 1: 	// obere Grenzfrequenz
				      	hc->LowPass = val;
				      	break;
				case 2: 	// untere Grenzfrequenz
				      	hc->HighPass = val;
				      	break;
				case 6: 	// Electrodenimpedanz
				      	hc->Impedance = val;
				      	break;
				case 7: 	// Sensitivitaet Verstaerker
				      	hc->Cal = val;
				      	break;
				case 8: 	// Elektrodenbezeichnung
					strcpy(hc->Label, (char*)(hdr->AS.Header+pos+p[k1]+1));
					break;
				case 10: 	// Masseinheit
					hc->PhysDimCode = PhysDimCode((char*)(hdr->AS.Header+pos+p[k1]+1));
					break;
				case 11: 	//
					strcpy(hc->Transducer, (char*)(hdr->AS.Header+pos+p[k1]+1));
					break;
				case 3: 	// gerfac ?
				case 4: 	// Kalibriergroesse
				case 5: 	// Kanaldimension
				case 9: 	// Bezeichnung der Referenzelektrode
					{};
				}

			}
			hc->Off    = lei16p(hdr->AS.Header+pos+ 80) * hc->Cal;
			hc->XYZ[0] = lei32p(hdr->AS.Header+pos+158);
			hc->XYZ[1] = lei32p(hdr->AS.Header+pos+162);
			hc->XYZ[2] = lei32p(hdr->AS.Header+pos+166);
	  	}
#undef line
		hdr->SPR = (typeof(hdr->SPR))round(hdr->SampleRate/minFs);
		for (k=0,hdr->AS.bpb=0; k<hdr->NS; k++) {
			hdr->CHANNEL[k].SPR = (typeof(hdr->SPR))round(fs[k]/minFs);
		      	hdr->CHANNEL[k].bi = hdr->AS.bpb;
		      	hdr->AS.bpb += hdr->CHANNEL[k].SPR*2;
		}
		free(fs);
	}	/******* end of Sigma PLpro ********/

/*
	else if (hdr->TYPE==SMA) {
		char *delim = "=\x0a\x0d\x22";
	}
*/
	else if (hdr->TYPE==SND) {
		/* read file */
		// hdr->FLAG.SWAP  = (__BYTE_ORDER == __LITTLE_ENDIAN);
		hdr->FILE.LittleEndian = 0;
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
		hdr->AS.bpb = hdr->NS * GDFTYP_BITS[gdftyp]>>3;
		double Cal = 1;
		if ((filetype>1) && (filetype<6))
			Cal = ldexp(1,-GDFTYP_BITS[gdftyp]);

		hdr->NRec = datlen/hdr->AS.bpb;
		hdr->SPR  = 1;

		hdr->FLAG.OVERFLOWDETECTION = 0; 	// automated overflow and saturation detection not supported
		hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS, sizeof(CHANNEL_TYPE));
		double digmax = ldexp(1,GDFTYP_BITS[gdftyp]);
		for (k=0,hdr->AS.bpb=0; k < hdr->NS; k++) {
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
		    	hc->bi    = hdr->AS.bpb;
			hdr->AS.bpb  += GDFTYP_BITS[gdftyp]>>3;
		}
	}

	else if (hdr->TYPE==TMS32) {
		hdr->VERSION 	= lei16p(hdr->AS.Header+31);
		hdr->SampleRate = lei16p(hdr->AS.Header+114);
		size_t NS 	= lei16p(hdr->AS.Header+119);
		hdr->HeadLen 	= 217 + NS*136;
		if (hdr->HeadLen > count) {
			hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,hdr->HeadLen);
		    	count += ifread(hdr->AS.Header+count, 1, hdr->HeadLen-count, hdr);
		} else
			ifseek(hdr,hdr->HeadLen,SEEK_SET);

		// size_t filesize	= lei32p(hdr->AS.Header+121);
		tm_time.tm_year = lei16p(hdr->AS.Header+129)-1900;
		tm_time.tm_mon 	= lei16p(hdr->AS.Header+131)-1;
		tm_time.tm_mday = lei16p(hdr->AS.Header+133);
		tm_time.tm_hour = lei16p(hdr->AS.Header+137);
		tm_time.tm_min 	= lei16p(hdr->AS.Header+139);
		tm_time.tm_sec 	= lei16p(hdr->AS.Header+141);
		tm_time.tm_isdst= -1;
		hdr->T0 	= tm_time2gdf_time(&tm_time);
		hdr->NRec 	= lei32p(hdr->AS.Header+143);
		hdr->SPR 	= leu16p(hdr->AS.Header+147);
		//hdr->AS.bpb 	= leu16p(hdr->AS.Header+149)+86;
		hdr->AS.bpb = 86;
		hdr->FLAG.OVERFLOWDETECTION = 0;

		hdr->CHANNEL = (CHANNEL_TYPE*)calloc(NS, sizeof(CHANNEL_TYPE));
		size_t aux = 0;
		for (k=0; k < NS; k += 1) {
			CHANNEL_TYPE* hc = hdr->CHANNEL+aux;
			uint8_t StringLength = hdr->AS.Header[217+k*136];
			char *SignalName = (char*)(hdr->AS.Header+218+k*136);
			if (!strncmp(SignalName, "(Lo) ", 5)) {
				strncpy(hc->Label,SignalName+5,StringLength-5);
				hc->GDFTYP = 16;
				aux += 1;
				hc->Label[StringLength-5] = 0;
				hc->bi    = hdr->AS.bpb;
			}
			else if (!strncmp(SignalName, "(Hi) ", 5)) {
			}
			else {
				strncpy(hc->Label, SignalName, StringLength);
				hc->GDFTYP = 3;
				aux += 1;
				hc->Label[StringLength] = 0;
				hc->bi    = hdr->AS.bpb;
			}

			StringLength = hdr->AS.Header[45+217+k*136];
			strncpy(tmp, (char*)(hdr->AS.Header+46+217+k*136), StringLength);
			tmp[StringLength] = 0;
		    	hc->PhysDimCode = PhysDimCode(tmp);
			hc->PhysMin  = lef32p(hdr->AS.Header+56+217+k*136);
			hc->PhysMax  = lef32p(hdr->AS.Header+60+217+k*136);
			hc->DigMin   = lef32p(hdr->AS.Header+64+217+k*136);
			hc->DigMax   = lef32p(hdr->AS.Header+68+217+k*136);

			hc->Cal      = (hc->PhysMax-hc->PhysMin)/(hc->DigMax-hc->DigMin);
                	hc->Off      = hc->PhysMin - hc->Cal * hc->DigMin;

			hc->OnOff    = 1;
			hc->SPR      = hdr->SPR;
			hc->Transducer[0] = '\0';
			hc->LowPass  = -1;
			hc->HighPass = -1;
		    	hc->LeadIdCode  = 0;
			//hdr->AS.bpb    += 2 * hc->SPR;
			hdr->AS.bpb    += (GDFTYP_BITS[hc->GDFTYP] * hc->SPR)>>3;

			if (VERBOSE_LEVEL>7)
				fprintf(stdout,"k=%i\tLabel=%s [%s]\tNS=%i\tpos=%i\n",k,SignalName,tmp,NS,iftell(hdr));

		}
		hdr->NS = aux;
		hdr->CHANNEL = (CHANNEL_TYPE*)realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
	}

	else if (hdr->TYPE==TMSiLOG) {
		/* read header
		      docu says HeadLen = 141+275*NS, but our example has 135+277*NS;
		 */
		int bufsiz = 16384;
		while (!ifeof(hdr)) {
			hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, count+bufsiz+1);
		    	count += ifread(hdr->AS.Header+count, 1, bufsiz, hdr);
		}
	    	ifclose(hdr);
	    	hdr->AS.Header[count] = 0;

	    	hdr->NS    = 0;
	    	hdr->SPR   = 1;
	    	hdr->NRec  = 1;
	    	double duration = 0.0;
	    	uint16_t gdftyp = 0;

	    	char *line = strstr(Header1,"Signals=");
		if (line) {
			char tmp[5];
			strncpy(tmp,line+8,5);
	 	    	hdr->NS    = atoi(tmp);
		}
		if (!hdr->NS) {
			B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
			B4C_ERRMSG = "TMSiLOG: multiple data files not supported\n";
		}
		hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS, sizeof(CHANNEL_TYPE));

		char *filename = NULL;
		line = strtok(Header1,"\x0d\x0a");
		while (line) {
			char *val = strchr(line,'=');
			val[0] = 0;
			val++;

			if (!strcmp(line,"FileId")) {}
			else if (!strcmp(line,"Version"))
				hdr->VERSION = atoi(val);
			else if (!strcmp(line,"DateTime")) {
				struct tm t;
				sscanf(val,"%04d/%02d/%02d-%02d:%02d:%02d",&t.tm_year,&t.tm_mon,&t.tm_mday,&t.tm_hour,&t.tm_min,&t.tm_sec);
				t.tm_year -= 1900;
				t.tm_mon--;
				t.tm_isdst =-1;
			}
			else if (!strcmp(line,"Format")) {
				if      (!strcmp(val,"Float32")) gdftyp = 16;
				else if (!strcmp(val,"Int32  ")) gdftyp = 5;
				else if (!strcmp(val,"Int16  ")) gdftyp = 3;
				else if (!strcmp(val,"Ascii  ")) gdftyp = 0xfffe;
				else                             gdftyp = 0xffff;
			}
			else if (!strcmp(line,"Length")) {
				duration = atof(val);
			}
			else if (!strcmp(line,"Signals")) {
				hdr->NS = atoi(val);
			}
			else if (!strncmp(line,"Signal",6)) {
				char tmp[5];
				strncpy(tmp,line+6,4);
				size_t ch = atoi(tmp)-1;
				char *field = line+11;

				if (!strcmp(field,"Name"))
					strncpy(hdr->CHANNEL[ch].Label,val,MAX_LENGTH_LABEL);
				else if (!strcmp(field,"UnitName"))
					hdr->CHANNEL[ch].PhysDimCode=PhysDimCode(val);
				else if (!strcmp(field,"Resolution"))
					hdr->CHANNEL[ch].Cal=atof(val);
				else if (!strcmp(field,"StoreRate")) {
					hdr->NRec = (nrec_t)atof(val)*duration;
					hdr->CHANNEL[ch].SPR = 1;
					// hdr->CHANNEL[ch].SPR=atof(val)*duration;
					//hdr->SPR = lcm(hdr->SPR,hdr->CHANNEL[ch].SPR);
				}
				else if (!strcmp(field,"File")) {
					if (!filename)
						filename = val;
					else if (strcmp(val, filename)) {

					fprintf(stdout,"<%s><%s>",val,filename);

						B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
						B4C_ERRMSG = "TMSiLOG: multiple data files not supported\n";
					}
				}
				else if (!strcmp(field,"Index")) {}
				else
					fprintf(stdout,"TMSi Signal%04i.%s = <%s>\n",ch,field,val);
			}
			else
				fprintf(stdout,"TMSi %s = <%s>\n",line,val);

			line = strtok(NULL,"\x0d\x0a");
		}

		hdr->SampleRate = hdr->SPR*hdr->NRec/duration;
		hdr->NRec *= hdr->SPR;
		hdr->SPR   = 1;
		char *fullfilename = (char*)malloc(strlen(hdr->FileName)+strlen(filename)+1);

		if (!filename) {
			B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
			B4C_ERRMSG = "TMSiLOG: data file not specified\n";
		}
		else if (strrchr(hdr->FileName,FILESEP)) {
			strcpy(fullfilename,hdr->FileName);
			strcpy(strrchr(fullfilename,FILESEP)+1,filename);
		}
		else {
			strcpy(fullfilename,filename);
		}
		filename = NULL; // filename had a pointer to hdr->AS.Header; could be released here

		if (gdftyp < 1000) {
			FILE *fid = fopen(fullfilename,"rb");

			if (!fid) {
				B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
				B4C_ERRMSG = "TMSiLOG: data file not found\n";
			}
			else {
				int16_t h[3];
				fread(h, 2, 3, fid);
				if      (h[2]==16)  h[2] = 3;
				else if (h[2]==32)  h[2] = 5;
				else if (h[2]==288) h[2] = 16;
				else                h[2] = 0xffff;  	// this triggers the error trap

				if ((h[0] != hdr->NS) || (((double)h[1]) != hdr->SampleRate) || (h[2] != gdftyp) ) {
					B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
					B4C_ERRMSG = "TMSiLOG: Binary file corrupted\n";
				}
				else {
					size_t sz = hdr->NS*hdr->SPR*hdr->NRec*GDFTYP_BITS[gdftyp]>>3;
					hdr->AS.rawdata = (uint8_t*)realloc(hdr->AS.rawdata,sz);
					fread(hdr->AS.rawdata,sz,1,fid);
				}
				fclose(fid);
			}
		}
		else if (gdftyp==0xfffe) {
			// double Fs;
			gdftyp = 17; 	// ascii is converted to double
			FILE *fid = fopen(fullfilename,"rt");

			if (!fid) {
				B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
				B4C_ERRMSG = "TMSiLOG: data file not found\n";
			}
			else {
				size_t sz  = (hdr->NS+1)*24;
				char *line = (char*) malloc(sz);
				fgets(line,sz,fid);
				fgets(line,sz,fid);
				fgets(line,sz,fid);
				// TODO: sanity checks
				if (0) {
					B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
					B4C_ERRMSG = "TMSiLOG: Binary file corrupted\n";
				}
				else {
					// hdr->FLAG.SWAP = 0; 	// no swaping required
					hdr->FILE.LittleEndian = (__BYTE_ORDER == __LITTLE_ENDIAN);
					size_t sz = hdr->NS*hdr->SPR*hdr->NRec*GDFTYP_BITS[gdftyp]>>3;
					hdr->AS.rawdata = (uint8_t*)realloc(hdr->AS.rawdata,sz);
					for (k=0; k < hdr->SPR*hdr->NRec; k++)
					if (fgets(line,sz,fid)) {
						char *next;
						strtoul(line, &next, 10);	// skip index entry
						for (k2=0;k2<hdr->NS;k2++)
							*(double*)(hdr->AS.rawdata+(k*hdr->NS+k2)*sizeof(double)) = strtod(next,&next);
					}
					else {
						int k2;
						for (k2=0;k2<hdr->NS;k2++)
							*(double*)(hdr->AS.rawdata+(k*hdr->NS+k2)*sizeof(double)) = NaN;
					}
				}
				free(line);
				fclose(fid);
			}
		}
		free(fullfilename);
		hdr->AS.length  = hdr->NRec;

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"TMSi [149] \n");


		hdr->AS.bpb = 0;
		for (k=0; k<hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"TMSi [151] %i\n",k);

			hc->GDFTYP = gdftyp;
			if (gdftyp==3) {
				hc->DigMax = (double)(int16_t)0x7fff;
				hc->DigMin = (double)(int16_t)0x8000;
			}
			else if (gdftyp==5) {
				hc->DigMax = (double)(int32_t)0x7fffffff;
				hc->DigMin = (double)(int32_t)0x80000000;
			}
			else {
				hc->DigMax = 1.0;
				hc->DigMin = 0.0;
				hdr->FLAG.OVERFLOWDETECTION = 0;	// no overflow detection
			}
			hc->PhysMax = hc->DigMax * hc->Cal;
			hc->PhysMin = hc->DigMin * hc->Cal;
		      	hc->LeadIdCode = 0;
	      		hc->Transducer[0] = 0;
		      	hc->SPR       = 1;	// one sample per block
		      	hc->OnOff     = 1;
	      		hc->HighPass  = NaN;
		      	hc->LowPass   = NaN;
		      	hc->Notch     = NaN;
	      		hc->Impedance = INF;
		      	hc->fZ        = NaN;
		      	hc->XYZ[0] = 0.0;
		      	hc->XYZ[1] = 0.0;
		      	hc->XYZ[2] = 0.0;
		      	hc->bi  = hdr->AS.bpb;
			hdr->AS.bpb += GDFTYP_BITS[gdftyp]>>3;
		}
	}

	else if (hdr->TYPE==AIFF) {
		// hdr->FLAG.SWAP  = (__BYTE_ORDER == __LITTLE_ENDIAN);
		hdr->FILE.LittleEndian = 0;
		uint8_t *tag;
		uint32_t tagsize;
		//uint16_t gdftyp;
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
		/// TODO, FIXME
	}

	else if ((hdr->TYPE==WAV)||(hdr->TYPE==AVI)||(hdr->TYPE==RIFF)) {
		// hdr->FLAG.SWAP  = (__BYTE_ORDER == __BIG_ENDIAN);
		hdr->FILE.LittleEndian = 1;
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
					Cal 	= ldexp(1,-8*(bits/8 + ((bits%8) > 0)));
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
					hdr->AS.bpb = hdr->NS * ((bits/8) + ((bits%8)>0));
					hdr->SPR    = tagsize/hdr->AS.bpb;
				}
			}

			pos += tagsize;
			tag  = hdr->AS.Header+pos;
		/// TODO, FIXME
		}
	}

	else if (hdr->TYPE==ASCII_IBI) {

		hdr->NS   = 0;
		hdr->NRec = 0;
		hdr->SPR  = 1;
		hdr->AS.bpb = 0;
		ifseek(hdr,0,SEEK_SET);
		char line[81];
		char desc[80];
		ifgets(line,80,hdr);
		size_t N = 0;
		hdr->EVENT.N = 0;
		while (!ifeof(hdr) && strlen(line)) {

			if (isdigit(line[0])) {
				struct tm t;
				int ms,rri;

				sscanf(line,"%02u-%02u-%02u %02u:%02u:%02u %03u %s %u",&t.tm_mday,&t.tm_mon,&t.tm_year,&t.tm_hour,&t.tm_min,&t.tm_sec,&ms,desc,&rri);
				if (t.tm_year<1970) t.tm_year+=100;
				t.tm_mon--;
				t.tm_isdst = -1;

				if (N+1 >= hdr->EVENT.N) {
					hdr->EVENT.N  += 4096;
			 		hdr->EVENT.POS = (typeof(hdr->EVENT.POS)) realloc(hdr->EVENT.POS, hdr->EVENT.N*sizeof(*hdr->EVENT.POS) );
					hdr->EVENT.TYP = (typeof(hdr->EVENT.TYP)) realloc(hdr->EVENT.TYP, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP) );
				}
				if (N==0) {
					hdr->T0 = (gdf_time)(tm_time2gdf_time(&t) + ldexp((ms-rri)/(24*3600*1e3),32));
					hdr->EVENT.POS[0] = 0;
					hdr->EVENT.TYP[0] = 0x0501;
					hdr->EVENT.POS[1] = rri;
					hdr->EVENT.TYP[1] = 0x0501;
					N = 1;
				}
				else {
					hdr->EVENT.POS[N] = hdr->EVENT.POS[N-1] + rri;
				}

				if (!strcmp(desc,"IBI"))
					hdr->EVENT.TYP[N] = 0x0501;
				else
					FreeTextEvent(hdr,N,desc);

				++N;
			}
			else {
				strtok(line,":");
				char *v = strtok(NULL,":");
				if (!strncmp(line,"File version",12))
					hdr->VERSION = atof(v);
				else if (!hdr->FLAG.ANONYMOUS && !strncmp(line,"File version",12))
					strncpy(hdr->Patient.Id,v,MAX_LENGTH_PID);
			}
			ifgets(line,80,hdr);
		}
	    	ifclose(hdr);
	    	hdr->EVENT.N = N;
	    	hdr->SampleRate = 1000.0;
	    	hdr->EVENT.SampleRate = 1000.0;
	    	hdr->TYPE = EVENT;
		hdr->data.block = NULL;
		hdr->data.size[0] = 0;
		hdr->data.size[1] = 0;
	}

#ifdef WITH_DICOM
	else if (hdr->TYPE==DICOM) {
		fprintf(stdout,"DICOM support is very (!!!) experimental!\n");

		hdr->HeadLen = count;
		sopen_dicom_read(hdr);

    		return(hdr);
	}
#endif

	else if (hdr->TYPE==HL7aECG) {
		sopen_HL7aECG_read(hdr);
		if (VERBOSE_LEVEL>7)
			fprintf(stdout,"[181] #%i\n",hdr->NS);

    		if (serror()) return(hdr);
    		// hdr->FLAG.SWAP = 0;
		hdr->FILE.LittleEndian = (__BYTE_ORDER == __LITTLE_ENDIAN); // no swapping
		hdr->AS.length  = hdr->NRec;
	}

#ifdef WITH_MICROMED
    	else if (hdr->TYPE==TRC) {
    		sopen_TRC_read(hdr);
	}
#endif

	else if (hdr->TYPE==UNIPRO) {
		hdr->FILE.LittleEndian = (__BYTE_ORDER == __LITTLE_ENDIAN);
		struct tm t0;
		char tmp[5];
		memset(tmp,0,5);
		strncpy(tmp,Header1+0x9c,2);
		t0.tm_mon = atoi(tmp)-1;
		strncpy(tmp,Header1+0x9e,2);
		t0.tm_mday = atoi(tmp);
		strncpy(tmp,Header1+0xa1,2);
		t0.tm_hour = atoi(tmp);
		strncpy(tmp,Header1+0xa3,2);
		t0.tm_min = atoi(tmp);
		strncpy(tmp,Header1+0xa5,2);
		t0.tm_sec = atoi(tmp);
		strncpy(tmp,Header1+0x98,4);
		t0.tm_year = atoi(tmp)-1900;
		hdr->T0 = tm_time2gdf_time(&t0);

		memset(tmp,0,5);
		strncpy(tmp,Header1+0x85,2);
		t0.tm_mday = atoi(tmp);
		strncpy(tmp,Header1+0x83,2);
		t0.tm_mon = atoi(tmp)-1;
		strncpy(tmp,Header1+0x7f,4);
		t0.tm_year = atoi(tmp)-1900;
		hdr->Patient.Birthday = tm_time2gdf_time(&t0);

		// filesize = leu32p(hdr->AS.Header + 0x24);
		sopen_unipro_read(hdr);
		if (VERBOSE_LEVEL>7)
			fprintf(stdout,"[181] #%i\n",hdr->NS);

    		if (serror()) return(hdr);
    		// hdr->FLAG.SWAP = 0;
	}

	else {
    		B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
    		B4C_ERRMSG = "ERROR BIOSIG SOPEN(READ): data format is not supported";
    		ifclose(hdr);
    		return(hdr);
	}

	hdr->FILE.POS = 0;

	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"[189] #%i\n",hdr->NS);

	for (k=0; k<hdr->NS; k++) {
		if (VERBOSE_LEVEL>7)
			fprintf(stdout,"[190] #%i: LeadIdCode=%i\n",k,hdr->CHANNEL[k].LeadIdCode);

		// set HDR.PhysDim - this part will become obsolete
/*
		k1 = hdr->CHANNEL[k].PhysDimCode;
		if (k1>0)
			PhysDim(k1,hdr->CHANNEL[k].PhysDim);
		else
			hdr->CHANNEL[k].PhysDimCode = PhysDimCode(hdr->CHANNEL[k].PhysDim);
*/
		// set HDR.PhysDimCode
		if (hdr->CHANNEL[k].LeadIdCode == 0) {
			int k1;
			if (!strncmp(hdr->CHANNEL[k].Label, "MDC_ECG_LEAD_", 13)) {
				// MDC_ECG_LEAD_*  - ignore case  //
				for (k1=0; strcmpi(hdr->CHANNEL[k].Label+13,LEAD_ID_TABLE[k1]) && LEAD_ID_TABLE[k1][0]; k1++) {};
				if (LEAD_ID_TABLE[k1][0])
					hdr->CHANNEL[k].LeadIdCode = k1;
			}
			else {
				for (k1=0; strcmp(hdr->CHANNEL[k].Label, LEAD_ID_TABLE[k1]) && LEAD_ID_TABLE[k1][0]; k1++) {};
				if (LEAD_ID_TABLE[k1][0])
					hdr->CHANNEL[k].LeadIdCode = k1;
			}
		}

		if (hdr->CHANNEL[k].LeadIdCode)
			strcpy(hdr->CHANNEL[k].Label,LEAD_ID_TABLE[hdr->CHANNEL[k].LeadIdCode]);

	}

	/*
	convert2to4_event_table(hdr->EVENT);
	convert into canonical form if needed
	*/

}
else if (!strncmp(MODE,"w",1))	 /* --- WRITE --- */
{
	hdr->FILE.COMPRESSION = hdr->FILE.COMPRESSION || strchr(MODE,'z');
	if (!strlen(hdr->Patient.Id))
		strcpy(hdr->Patient.Id,"00000000");

#ifdef __sparc__
	fprintf(stdout,"Warning SOPEN: Alignment errors might cause Bus Error (SPARC platform).\nWe are aware of the problem but it is not fixed yet.\n");
#endif

#ifndef WITHOUT_NETWORK
    	if (!memcmp(hdr->FileName,"bscs://",7)) {
    		// network: write to server
                const char *hostname = hdr->FileName+7;
                char *tmp= (char*)strchr(hostname,'/');
                if (tmp != NULL) tmp[0]=0;   // ignore terminating slash

                uint64_t ID=0;
                int sd, s;
		sd = bscs_connect(hostname);
		if (sd<0) {
			fprintf(stdout,"could not connect to <%s>\n",hostname);
			B4C_ERRNUM = B4C_CANNOT_OPEN_FILE;
			B4C_ERRMSG = "could not connect to server";
			return(hdr);
		}
  		hdr->FILE.Des = sd;
		s  = bscs_open(sd, &ID);
  		s  = bscs_send_hdr(sd,hdr);
  		hdr->FILE.OPEN = 2;
  		fprintf(stdout,"write file to bscs://%s/%016Lx\n",hostname,ID);
  		return(hdr);
	}
#endif

	// NS number of channels selected for writing
     	typeof(hdr->NS)  NS = 0;
	for (k=0; k<hdr->NS; k++)
		if (hdr->CHANNEL[k].OnOff) NS++;

	if (VERBOSE_LEVEL>8)
		fprintf(stdout,"sopen-W ns=%i (%s)\n",NS,GetFileTypeString(hdr->TYPE));

    	if ((hdr->TYPE==ASCII) || (hdr->TYPE==BIN)) {

    		FILE *fid = fopen(hdr->FileName,"w");
		hdr->FILE.LittleEndian = 1;

    		fprintf(fid,"#BIOSIG %s\n", (hdr->TYPE==ASCII ? "ASCII" : "BINARY"));
    		fprintf(fid,"#   comments start with #\n\n");
    		fprintf(fid,"Filename\t= %s\t # (this file)\n",hdr->FileName);
    		fprintf(fid,"\n[Header 1]\n");
    		// fprintf(fid,"\n[Header 1]\nNumberOfChannels\t= %i\n",hdr->NS);
    		//fprintf(fid,"NRec\t= %i\n",hdr->NRec);
    		fprintf(fid,"Duration         \t= %f\t# in seconds\n",hdr->SPR*hdr->NRec/hdr->SampleRate);
    		struct tm *t = gdf_time2tm_time(hdr->T0);
    		fprintf(fid,"Recording.Time    \t= %04i-%02i-%02i %02i:%02i:%02i\t# YYYY-MM-DD hh:mm:ss\n",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);

    		fprintf(fid,"Patient.Id        \t= %s\n",hdr->Patient.Id);
    		t = gdf_time2tm_time(hdr->Patient.Birthday);
    		fprintf(fid,"Patient.Birthday  \t= %04i-%02i-%02i %02i:%02i:%02i\t# YYYY-MM-DD hh:mm:ss\n",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
    		fprintf(fid,"Patient.Weight    \t= %i\t# in [kg]\n",hdr->Patient.Weight);
    		fprintf(fid,"Patient.Height    \t= %i\t# in [cm]\n",hdr->Patient.Height);
    		fprintf(fid,"Patient.Gender    \t= %i\t# 0:Unknown, 1: Male, 2: Female, 9: Unspecified\n",hdr->Patient.Sex);
    		fprintf(fid,"Patient.Handedness\t= %i\t# 0:Unknown, 1: Right, 2: Left, 3: Equal\n",hdr->Patient.Handedness);
    		fprintf(fid,"Patient.Smoking   \t= %i\t# 0:Unknown, 1: NO, 2: YES\n",hdr->Patient.Sex);
    		fprintf(fid,"Patient.AlcoholAbuse\t= %i\t# 0:Unknown, 1: NO, 2: YES\n",hdr->Patient.AlcoholAbuse);
    		fprintf(fid,"Patient.DrugAbuse \t= %i\t# 0:Unknown, 1: NO, 2: YES \n",hdr->Patient.DrugAbuse);
    		fprintf(fid,"Patient.Medication\t= %i\t# 0:Unknown, 1: NO, 2: YES \n",hdr->Patient.Medication);
		fprintf(fid,"Recording.ID      \t= %s\n",hdr->ID.Recording);
		uint8_t IPv6=0;
		for (k=4; k<16; k++) IPv6 |= hdr->IPaddr[k];
		if (IPv6)
			fprintf(fid,"Recording.IPaddress \t= %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",hdr->IPaddr[0],hdr->IPaddr[1],hdr->IPaddr[2],hdr->IPaddr[3],hdr->IPaddr[4],hdr->IPaddr[5],hdr->IPaddr[6],hdr->IPaddr[7],hdr->IPaddr[8],hdr->IPaddr[9],hdr->IPaddr[10],hdr->IPaddr[11],hdr->IPaddr[12],hdr->IPaddr[13],hdr->IPaddr[14],hdr->IPaddr[15]);
		else
			fprintf(fid,"Recording.IPaddress \t= %u.%u.%u.%u\n",hdr->IPaddr[0],hdr->IPaddr[1],hdr->IPaddr[2],hdr->IPaddr[3]);
		fprintf(fid,"Recording.Technician\t= %s\n",hdr->ID.Technician);
		fprintf(fid,"Manufacturer.Name \t= %s\n",hdr->ID.Manufacturer.Name);
		fprintf(fid,"Manufacturer.Model\t= %s\n",hdr->ID.Manufacturer.Model);
		fprintf(fid,"Manufacturer.Version\t= %s\n",hdr->ID.Manufacturer.Version);
		fprintf(fid,"Manufacturer.SerialNumber\t= %s\n",hdr->ID.Manufacturer.SerialNumber);


   		fprintf(fid,"\n[Header 2]\n");
		char fn[1025];
		strcpy(fn,hdr->FileName);
		char e1 = (hdr->TYPE == ASCII ? 'a' : 's');
		if (strrchr(fn,'.')==NULL)
			strcat(fn,".");

    		for (k=0; k<hdr->NS; k++)
    		if (hdr->CHANNEL[k].OnOff) {
    			sprintf(strrchr(fn,'.'),".%c%02i",e1,k+1);
    			if (hdr->FILE.COMPRESSION) strcat(fn,"_gz");
	    		fprintf(fid,"Filename  \t= %s\n",fn);
	    		fprintf(fid,"Label     \t= %s\n",hdr->CHANNEL[k].Label);
	    		if (hdr->TYPE==ASCII)
		    		fprintf(fid,"GDFTYP    \t= ascii\n");
	    		else if (hdr->TYPE==BIN) {
	    			char *gdftyp;
	    			switch (hdr->CHANNEL[k].GDFTYP) {
	    			case 1:	gdftyp="int8"; break;
	    			case 2:	gdftyp="uint8"; break;
	    			case 3:	gdftyp="int16"; break;
	    			case 4:	gdftyp="uint16"; break;
	    			case 5:	gdftyp="int32"; break;
	    			case 6:	gdftyp="uint32"; break;
	    			case 7:	gdftyp="int64"; break;
	    			case 8:	gdftyp="uint64"; break;
	    			case 16: gdftyp="float32"; break;
	    			case 17: gdftyp="float64"; break;
	    			case 18: gdftyp="float128"; break;
	    			case 255+24: gdftyp="bit24"; break;
	    			case 511+24: gdftyp="ubit24"; break;
	    			case 255+12: gdftyp="bit12"; break;
	    			case 511+12: gdftyp="ubit12"; break;
	    			default: gdftyp = "unknown";
	    			}
		    		fprintf(fid,"GDFTYP    \t= %s\n",gdftyp);
	    		}

	    		fprintf(fid,"Transducer\t= %s\n",hdr->CHANNEL[k].Transducer);
	    		fprintf(fid,"PhysicalUnits\t= %s\n",PhysDim(hdr->CHANNEL[k].PhysDimCode,tmp));
	    		fprintf(fid,"PhysDimCode\t= %i\n",hdr->CHANNEL[k].PhysDimCode);
	    		fprintf(fid,"DigMax   \t= %f\n",hdr->CHANNEL[k].DigMax);
	    		fprintf(fid,"DigMin   \t= %f\n",hdr->CHANNEL[k].DigMin);
	    		fprintf(fid,"PhysMax  \t= %f\n",hdr->CHANNEL[k].PhysMax);
	    		fprintf(fid,"PhysMin  \t= %f\n",hdr->CHANNEL[k].PhysMin);
	    		fprintf(fid,"SamplingRate\t= %f\n",hdr->CHANNEL[k].SPR*hdr->SampleRate/hdr->SPR);
	    		fprintf(fid,"NumberOfSamples\t= %Li\n",hdr->CHANNEL[k].SPR*hdr->NRec);
	    		fprintf(fid,"HighPassFilter\t= %f\n",hdr->CHANNEL[k].HighPass);
	    		fprintf(fid,"LowPassFilter\t= %f\n",hdr->CHANNEL[k].LowPass);
	    		fprintf(fid,"NotchFilter\t= %f\n",hdr->CHANNEL[k].Notch);
	    		switch (hdr->CHANNEL[k].PhysDimCode & 0xffe0) {
	       		case 4256:        // Voltage data
	    		        fprintf(fid,"Impedance\t= %f\n",hdr->CHANNEL[k].Impedance);
	    		        break;
	    		case 4288:         // Impedance data
	    		        fprintf(fid,"freqZ\t= %f\n",hdr->CHANNEL[k].fZ);
	    		        break;
	    		}
	    		fprintf(fid,"PositionXYZ\t= %f\t%f\t%f\n",hdr->CHANNEL[k].XYZ[0],hdr->CHANNEL[k].XYZ[1],hdr->CHANNEL[k].XYZ[2]);
//	    		fprintf(fid,"OrientationXYZ\t= %f\t%f\t%f\n",hdr->CHANNEL[k].Orientation[0],hdr->CHANNEL[k].Orientation[1],hdr->CHANNEL[k].Orientation[2]);
//	    		fprintf(fid,"Area     \t= %f\n",hdr->CHANNEL[k].Area);

	    		fprintf(fid,"\n");
			hdr->CHANNEL[k].SPR *= hdr->NRec;
    		}
		hdr->SPR *= hdr->NRec;
		hdr->NRec = 1;

		fprintf(fid,"[EVENT TABLE]\n");
		fprintf(fid,"TYP\tPOS [s]\tDUR [s]\tCHN\tVAL/Desc");
		if (!GLOBAL_EVENTCODES_ISLOADED) LoadGlobalEventCodeTable();

		size_t k;
		for (k=0; k<hdr->EVENT.N; k++) {

			fprintf(fid,"\n0x%04x\t%f\t",hdr->EVENT.TYP[k],hdr->EVENT.POS[k]/hdr->EVENT.SampleRate);   // EVENT.POS uses 0-based indexing
			if (hdr->EVENT.DUR != NULL)
				fprintf(fid,"%f\t%d\t",hdr->EVENT.DUR[k]/hdr->EVENT.SampleRate,hdr->EVENT.CHN[k]);
			else
				fprintf(fid,"\t\t");

			if (hdr->EVENT.TYP[k] == 0x7fff)
				fprintf(fid,"%i\t# sparse sample ",hdr->EVENT.DUR[k]);	// value of sparse samples
			else if (hdr->EVENT.TYP[k] == 0)
				{}
			else if (hdr->EVENT.TYP[k] < hdr->EVENT.LenCodeDesc)
				fprintf(fid,"%s",hdr->EVENT.CodeDesc[hdr->EVENT.TYP[k]]);
			else if (GLOBAL_EVENTCODES_ISLOADED) {
				uint16_t k1;
				for (k1=0; (k1 < Global.LenCodeDesc) && (hdr->EVENT.TYP[k] != Global.CodeIndex[k1]); k1++) {};
				if (hdr->EVENT.TYP[k] == Global.CodeIndex[k1])
					fprintf(fid,"%s",Global.CodeDesc[k1]);
			}
		}
		fclose(fid);
    	}
	else if (hdr->TYPE==BrainVision) {

		if (VERBOSE_LEVEL>8) fprintf(stdout,"BVA-write: [210]\n");

		char* tmpfile = (char*)calloc(strlen(hdr->FileName)+6,1);
		strcpy(tmpfile,hdr->FileName);
		char* ext = strrchr(tmpfile,'.');
		if (ext != NULL) strcpy(ext+1,"vhdr");
		else 		strcat(tmpfile,".vhdr");

		if (VERBOSE_LEVEL>8) fprintf(stdout,"BVA-write: [211]\n");

    		hdr->HeadLen = 0;
    		FILE *fid = fopen(tmpfile,"wb");
    		fprintf(fid,"Brain Vision Data Exchange Header File Version 1.0\r\n");
    		fprintf(fid,"; Data created by BioSig4C++\r\n\r\n");
    		fprintf(fid,"[Common Infos]\r\n");
    		fprintf(fid,"DataFile=%s\r\n",hdr->FileName);
    		fprintf(fid,"MarkerFile=%s\r\n",strcpy(strrchr(tmpfile,'.')+1,"vhdr"));
    		fprintf(fid,"DataFormat=BINARY\r\n");
    		fprintf(fid,"; Data orientation: MULTIPLEXED=ch1,pt1, ch2,pt1 ...\r\n");
    		fprintf(fid,"DataOrientation=MULTIPLEXED\r\n");
    		hdr->NRec *= hdr->SPR;
		hdr->SPR = 1;
    		fprintf(fid,"NumberOfChannels=%i\r\n",hdr->NS);
    		fprintf(fid,"; Sampling interval in microseconds\r\n");
    		fprintf(fid,"SamplingInterval=%f\r\n\r\n",1e6/hdr->SampleRate);

		if (VERBOSE_LEVEL>8) fprintf(stdout,"BVA-write: [212]\n");

    		fprintf(fid,"[Binary Infos]\r\nBinaryFormat=");
		uint16_t gdftyp = 0;
    		for (k=0; k<hdr->NS; k++)
    			if (gdftyp < hdr->CHANNEL[k].GDFTYP)
    				gdftyp = hdr->CHANNEL[k].GDFTYP;
    		if (gdftyp<4) {
    			gdftyp = 3;
    			fprintf(fid,"INT_16");
    		}
    		else {
    			gdftyp = 16;
 	   		fprintf(fid,"IEEE_FLOAT_32");
		}

		if (VERBOSE_LEVEL>8) fprintf(stdout,"BVA-write: [214] gdftyp=%i NS=%i\n",gdftyp,hdr->NS);

		hdr->AS.bpb = hdr->NS * hdr->SPR * GDFTYP_BITS[gdftyp] >> 3;

    		fprintf(fid,"\r\n\r\n[Channel Infos]\r\n");
    		fprintf(fid,"; Each entry: Ch<Channel number>=<Name>,<Reference channel name>,\r\n");
    		fprintf(fid,"; <Resolution in \"Unit\">,<Unit>,,<Future extensions..\r\n");
    		fprintf(fid,"; Fields are delimited by commas, some fields might be omitted (empty).\r\n");
    		fprintf(fid,"; Commas in channel names are coded as \"\\1\".\r\n");
    		for (k=0; k<hdr->NS; k++) {

			if (VERBOSE_LEVEL>8) fprintf(stdout,"BVA-write: [220] %i\n",k);

			hdr->CHANNEL[k].SPR = hdr->SPR;
			hdr->CHANNEL[k].GDFTYP = gdftyp;
    			char physdim[MAX_LENGTH_PHYSDIM+1];
    			char Label[MAX_LENGTH_LABEL+1];
    			strcpy(Label,hdr->CHANNEL[k].Label);
    			size_t k1;
    			for (k1=0; Label[k1]; k1++) if (Label[k1]==',') Label[k1]=1;
	    		fprintf(fid,"Ch%d=%s,,1,%s\r\n",k+1,Label,PhysDim(hdr->CHANNEL[k].PhysDimCode,physdim));
    		}
    		fprintf(fid,"\r\n\r\n[Coordinates]\r\n");
    		// fprintf(fid,"; Each entry: Ch<Channel number>=<Radius>,<Theta>,<Phi>\n\r");
    		fprintf(fid,"; Each entry: Ch<Channel number>=<X>,<Y>,<Z>\r\n");
    		for (k=0; k<hdr->NS; k++)
	    		fprintf(fid,"Ch%i=%f,%f,%f\r\n",k+1,hdr->CHANNEL[k].XYZ[0],hdr->CHANNEL[k].XYZ[1],hdr->CHANNEL[k].XYZ[2]);

		if (VERBOSE_LEVEL>8) fprintf(stdout,"BVA-write: [222]\n");

    		fprintf(fid,"\r\n\r\n[Comment]\r\n\r\n");

		fprintf(fid,"A m p l i f i e r  S e t u p\r\n");
		fprintf(fid,"============================\r\n");
		fprintf(fid,"Number of channels: %i\r\n",hdr->NS);
		fprintf(fid,"Sampling Rate [Hz]: %f\r\n",hdr->SampleRate);
		fprintf(fid,"Sampling Interval [µS]: %f\r\n",1e6/hdr->SampleRate);
		fprintf(fid,"Channels\r\n--------\r\n");
		fprintf(fid,"#     Name      Phys. Chn.    Resolution [µV]  Low Cutoff [s]   High Cutoff [Hz]   Notch [Hz]\n\r");
    		for (k=0; k<hdr->NS; k++) {
			fprintf(fid,"\r\n%6i %13s %17i %18f",k+1,hdr->CHANNEL[k].Label,k+1,hdr->CHANNEL[k].Cal);

			if (hdr->CHANNEL[k].HighPass>0)
				fprintf(fid," %15f",1/(2*3.141592653589793238462643383279502884197169399375*hdr->CHANNEL[k].HighPass));
			else
				fprintf(fid,"\t-");

			if (hdr->CHANNEL[k].LowPass>0)
				fprintf(fid," %15f",hdr->CHANNEL[k].LowPass);
			else
				fprintf(fid,"\t-");

			if (hdr->CHANNEL[k].Notch>0)
				fprintf(fid," %f",hdr->CHANNEL[k].Notch);
			else
				fprintf(fid,"\t-");
		}

    		fprintf(fid,"\r\n\r\nImpedance [kOhm] :\r\n\r\n");
    		for (k=0; k<hdr->NS; k++)
    		if (isnan(hdr->CHANNEL[k].Impedance))
			fprintf(fid,"%s:\t\t-\r\n",hdr->CHANNEL[k].Label);
		else
			fprintf(fid,"%s:\t\t%f\r\n",hdr->CHANNEL[k].Label,hdr->CHANNEL[k].Impedance);


		fclose(fid);

		strcpy(strrchr(tmpfile,'.')+1,"vmrk");
		fid = fopen(tmpfile,"wb");
    		fprintf(fid,"Brain Vision Data Exchange Marker File, Version 1.0\r\n");
    		fprintf(fid,"; Data created by BioSig4C++\r\n\r\n");
    		fprintf(fid,"[Common Infos]\r\n");
    		fprintf(fid,"DataFile=%s\r\n\r\n",hdr->FileName);
    		fprintf(fid,"[Marker Infos]\r\n\r\n");
    		fprintf(fid,"; Each entry: Mk<Marker number>=<Type>,<Description>,<Position in data points>,\r\n");
    		fprintf(fid,"; <Size in data points>, <Channel number (0 = marker is related to all channels)>\r\n");
    		fprintf(fid,"; Fields are delimited by commas, some fields might be omitted (empty).\r\n");
    		fprintf(fid,"; Commas in type or description text are coded as \"\\1\".\r\n");
    		struct tm *T0 = gdf_time2tm_time(hdr->T0);
		uint32_t us = (hdr->T0*24*3600 - floor(hdr->T0*24*3600))*1e6;
    		fprintf(fid,"Mk1=New Segment,,1,1,0,%04u%02u%02u%02u%02u%02u%06u",T0->tm_year+1900,T0->tm_mon+1,T0->tm_mday,T0->tm_hour,T0->tm_min,T0->tm_sec,us); // 20081002150147124211

		if ((hdr->EVENT.DUR==NULL) && (hdr->EVENT.CHN==NULL))
	    		for (k=0; k<hdr->EVENT.N; k++) {
				fprintf(fid,"\r\nMk%i=,0x%04x,%u,1,0",k+2,hdr->EVENT.TYP[k],hdr->EVENT.POS[k]+1);  // convert to 1-based indexing
   			}
    		else
    			for (k=0; k<hdr->EVENT.N; k++) {
				fprintf(fid,"\r\nMk%i=,0x%04x,%u,%u,%u",k+2,hdr->EVENT.TYP[k],hdr->EVENT.POS[k]+1,hdr->EVENT.DUR[k],hdr->EVENT.CHN[k]); // convert EVENT.POS to 1-based indexing
	   		}
		fclose(fid);

		free(tmpfile);

		if (VERBOSE_LEVEL>8) fprintf(stdout,"BVA-write: [290] %s %s\n",tmpfile,hdr->FileName);
    	}

    	else if (hdr->TYPE==CFWB) {
	     	hdr->HeadLen = 68 + NS*96;
	    	hdr->AS.Header = (uint8_t*)malloc(hdr->HeadLen);
	    	uint8_t* Header2 = hdr->AS.Header+68;
		memset(Header1,0,hdr->HeadLen);
	    	memcpy(Header1,"CFWB\1\0\0\0",8);
	    	*(double*)(Header1+8) = l_endian_f64(1/hdr->SampleRate);

		struct tm *t = gdf_time2tm_time(hdr->T0);
    		*(uint32_t*)(Header1+16) = l_endian_u32(t->tm_year + 1900);
	    	*(uint32_t*)(Header1+20) = l_endian_u32(t->tm_mon + 1);
	    	*(uint32_t*)(Header1+24) = l_endian_u32(t->tm_mday);
	    	*(uint32_t*)(Header1+28) = l_endian_u32(t->tm_hour);
	    	*(uint32_t*)(Header1+32) = l_endian_u32(t->tm_min);
	    	*(double*)  (Header1+36) = l_endian_f64(t->tm_sec);
	    	*(double*)  (Header1+44) = l_endian_f64(0.0);	// pretrigger time
	    	*(uint32_t*)(Header1+52) = l_endian_u32(NS);
	    	hdr->NRec *= hdr->SPR; hdr->SPR = 1;
	    	*(uint32_t*)(Header1+56) = l_endian_u32(hdr->NRec); // number of samples
	    	*(int32_t*) (Header1+60) = l_endian_i32(0);	// 1: time channel

	    	int32_t gdftyp = 3; // 1:double, 2:float, 3: int16; see CFWB_GDFTYP too.
		for (k=0; k<hdr->NS; k++)
		if (hdr->CHANNEL[k].OnOff)
		{
			/* if int16 is not sufficient, use float or double */
			if (hdr->CHANNEL[k].GDFTYP>16)
				gdftyp = min(gdftyp,1);	// double
			else if (hdr->CHANNEL[k].GDFTYP>3)
				gdftyp = min(gdftyp,2);	// float
		}
	    	*(int32_t*)(Header1+64)	= l_endian_i32(gdftyp);	// 1: double, 2: float, 3:short

		for (k=0,k2=0; k<hdr->NS; k++)
		if (hdr->CHANNEL[k].OnOff)
		{
	    		hdr->CHANNEL[k].SPR = 1;
			hdr->CHANNEL[k].GDFTYP = CFWB_GDFTYP[gdftyp-1];
			const char *tmpstr;
			if (hdr->CHANNEL[k].LeadIdCode)
				tmpstr = LEAD_ID_TABLE[hdr->CHANNEL[k].LeadIdCode];
			else
				tmpstr = hdr->CHANNEL[k].Label;
		     	size_t len = strlen(tmpstr);
		     	memcpy(Header2+96*k2, tmpstr, min(len,32));

		     	PhysDim(hdr->CHANNEL[k].PhysDimCode, tmp);
		     	len = strlen(tmp);
		     	memcpy(Header2+96*k2+32, tmp, min(len,32));

			*(double*)(Header2+96*k2+64) = l_endian_f64(hdr->CHANNEL[k].Cal);
			*(double*)(Header2+96*k2+72) = l_endian_f64(hdr->CHANNEL[k].Off);
			*(double*)(Header2+96*k2+80) = l_endian_f64(hdr->CHANNEL[k].PhysMax);
			*(double*)(Header2+96*k2+88) = l_endian_f64(hdr->CHANNEL[k].PhysMin);
			k2++;
		}
	}

    	else if ((hdr->TYPE==GDF) || (hdr->TYPE==GDF1)) {

		struct2gdfbin(hdr);

		size_t bpb8 = 0;
		for (k=0, hdr->AS.bpb=0; k<hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
			hc->bi8 = bpb8;
			hc->bi  = bpb8>>3;
			if (hc->OnOff)
				bpb8 += (GDFTYP_BITS[hc->GDFTYP] * hc->SPR);
		}
		hdr->AS.bpb8 = bpb8;
		hdr->AS.bpb  = bpb8>>3;
		if (bpb8 & 0x07) {		// each block must use whole number of bytes
			hdr->AS.bpb++;
			hdr->AS.bpb8 = hdr->AS.bpb<<3;
		}

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"GDFw h3\n");

	}

    	else if ((hdr->TYPE==EDF) || (hdr->TYPE==BDF)) {
	     	hdr->HeadLen   = (NS+1)*256;
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

/* obsolete
		tt = gdf_time2t_time(hdr->Patient.Birthday);
		if (hdr->Patient.Birthday>1) strftime(tmp,81,"%d-%b-%Y",localtime(&tt));
*/
		struct tm *t = gdf_time2tm_time(hdr->Patient.Birthday);
		if (hdr->Patient.Birthday>1)
			strftime(tmp,81,"%02d-%b-%04Y",t);
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

		t = gdf_time2tm_time(hdr->T0);
		if (hdr->T0>1)
			strftime(tmp,81,"%02d-%b-%04Y",t);
		else strcpy(tmp,"X");
		if (!strlen(hdr->ID.Technician)) strcpy(hdr->ID.Technician,"X");
		size_t len = sprintf(cmd,"Startdate %s X %s ", tmp, hdr->ID.Technician);
	     	memcpy(Header1+88, cmd, len);
	     	memcpy(Header1+88+len, &hdr->ID.Equipment, 8);

		t = gdf_time2tm_time(hdr->T0);
		strftime(tmp,81,"%d.%m.%y%H.%M.%S",t);
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

		len = sprintf(tmp,"%i",NS);
		if (len>4) fprintf(stderr,"Warning: NS is (%s) too long (%i>4).\n",tmp,len);
	     	memcpy(Header1+252, tmp, len);

		for (k=0,k2=0; k<hdr->NS; k++)
		if (hdr->CHANNEL[k].OnOff)
		{
			const char *tmpstr;
			if (hdr->CHANNEL[k].LeadIdCode)
				tmpstr = LEAD_ID_TABLE[hdr->CHANNEL[k].LeadIdCode];
			else
				tmpstr = hdr->CHANNEL[k].Label;
		     	len = strlen(tmpstr);
			if (len>15)
			//fprintf(stderr,"Warning: Label (%s) of channel %i is to long.\n",hdr->CHANNEL[k].Label,k);
		     	fprintf(stderr,"Warning: Label of channel %i,%i is too long (%i>16).\n",k,k2, len);
		     	memcpy(Header2+16*k2,tmpstr,min(len,16));
		     	len = strlen(hdr->CHANNEL[k].Transducer);
			if (len>80) fprintf(stderr,"Warning: Transducer of channel %i,%i is too long (%i>80).\n",k,k2, len);
		     	memcpy(Header2+80*k2 + 16*NS,hdr->CHANNEL[k].Transducer,min(len,80));
		     	PhysDim(hdr->CHANNEL[k].PhysDimCode, tmp);
		     	len = strlen(tmp);
		     	if (len>8) fprintf(stderr,"Warning: Physical Dimension (%s) of channel %i is too long (%i>8).\n",tmp,k,len);
		     	memcpy(Header2 + 8*k2 + 96*NS, tmp, min(len,8));

			if (ftoa8(tmp,hdr->CHANNEL[k].PhysMin))
				fprintf(stderr,"Warning: PhysMin (%f)(%s) of channel %i does not fit into 8 bytes of EDF header.\n",hdr->CHANNEL[k].PhysMin,tmp,k);
		     	memcpy(Header2 + 8*k2 + 104*NS, tmp, strlen(tmp));
			if (ftoa8(tmp,hdr->CHANNEL[k].PhysMax))
				fprintf(stderr,"Warning: PhysMax (%f)(%s) of channel %i does not fit into 8 bytes of EDF header.\n",hdr->CHANNEL[k].PhysMax,tmp,k);
		     	memcpy(Header2 + 8*k2 + 112*NS, tmp, strlen(tmp));
			if (ftoa8(tmp,hdr->CHANNEL[k].DigMin))
				fprintf(stderr,"Warning: DigMin (%f)(%s) of channel %i does not fit into 8 bytes of EDF header.\n",hdr->CHANNEL[k].DigMin,tmp,k);
		     	memcpy(Header2 + 8*k2 + 120*NS, tmp, strlen(tmp));
			if (ftoa8(tmp,hdr->CHANNEL[k].DigMax))
				fprintf(stderr,"Warning: DigMax (%f)(%s) of channel %i does not fit into 8 bytes of EDF header.\n",hdr->CHANNEL[k].DigMax,tmp,k);
		     	memcpy(Header2 + 8*k2 + 128*NS, tmp, strlen(tmp));

			if (hdr->CHANNEL[k].Notch>0)
				len = sprintf(tmp,"HP:%fHz LP:%fHz Notch:%fHz",hdr->CHANNEL[k].HighPass,hdr->CHANNEL[k].LowPass,hdr->CHANNEL[k].Notch);
			else
				len = sprintf(tmp,"HP:%fHz LP:%fHz",hdr->CHANNEL[k].HighPass,hdr->CHANNEL[k].LowPass);
		     	memcpy(Header2+ 80*k2 + 136*NS,tmp,min(80,len));

			len = sprintf(tmp,"%i",hdr->CHANNEL[k].SPR);
			if (len>8) fprintf(stderr,"Warning: SPR (%s) of channel %i is to long (%i)>8.\n",tmp,k,len);
		     	memcpy(Header2+ 8*k2 + 216*NS,tmp,min(8,len));
		     	hdr->CHANNEL[k].GDFTYP = ( (hdr->TYPE != BDF) ? 3 : 255+24);
		     	k2++;
		}
	}

    	else if (hdr->TYPE==HL7aECG) {
   		hdr->FileName = FileName;
		sopen_HL7aECG_write(hdr);

		// hdr->FLAG.SWAP = 0;
		hdr->FILE.LittleEndian = (__BYTE_ORDER == __LITTLE_ENDIAN); // no byte-swapping
	}

    	else if (hdr->TYPE==MFER) {
    		uint8_t tag;
    		size_t  len, curPos=0;
    		hdr->FileName  = FileName;
	     	hdr->HeadLen   = 32+128+3*6+3 +80000;
	    	hdr->AS.Header = (uint8_t*)malloc(hdr->HeadLen);
		memset(Header1, ' ', hdr->HeadLen);

		hdr->FILE.LittleEndian = 0;

		fprintf(stderr,"Warning SOPEN(MFER): write support for MFER format under construction\n");
		/* FIXME & TODO:
		   known issues:
			Label
			Sampling Rate
			HeadLen
			Encoding of data block
		*/

		// tag 64: preamble
		// Header1[curPos] = 64;
		// len =32;
		curPos = 34;
		strncpy(Header1,"@  MFER                                ",curPos);
		// Header1[curPos+1] = len;
		// curPos = len+2;

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[MFER 711]:\n");

		// tag 23: Manufacturer
		tag = 23;
		Header1[curPos] = tag;
		strcpy(Header1+curPos+2,hdr->ID.Manufacturer.Name);
		strcat(Header1+curPos+2,"^");
		if (hdr->ID.Manufacturer.Model != NULL) {
			strcat(Header1+curPos+2,hdr->ID.Manufacturer.Model);
		}
		strcat(Header1+curPos+2,"^");
		if (hdr->ID.Manufacturer.Version != NULL) {
			strcat(Header1+curPos+2,hdr->ID.Manufacturer.Version);
		}
		strcat(Header1+curPos+2,"^");
		if (hdr->ID.Manufacturer.SerialNumber!=NULL) {
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

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[MFER 720-4]:\n");

		// tag 4: SPR
		tag = 4;
		len = sizeof(uint32_t);
		Header1[curPos++] = tag;
		Header1[curPos++] = len;
		*(uint32_t*)(Header1+curPos) = b_endian_u32(hdr->SPR);
		curPos += len;

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[MFER 720-5]:\n");

		// tag 5: NS
		tag = 5;
		len = sizeof(uint16_t);
		Header1[curPos++] = tag;
		Header1[curPos++] = len;
		*(uint16_t*)(Header1+curPos) = b_endian_u16(hdr->NS);
		curPos += len;

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[MFER 720-6]:\n");

		// tag 6: NRec
		tag = 6;
		len = sizeof(uint32_t);
		Header1[curPos++] = tag;
		Header1[curPos++] = len;
		*(uint32_t*)(Header1+curPos) = b_endian_u32(hdr->NRec);
		curPos += len;

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[MFER 720-8]:\n");

		// tag 8: Waveform: unidentified
		tag = 8;
		len = sizeof(uint8_t);
		Header1[curPos++] = tag;
		Header1[curPos++] = len;
		*(Header1+curPos) = 0; // unidentified
		curPos += len;

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[MFER 720-129]:\n");

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
		if (hdr->Patient.Birthday>0) {
			tag = 131;
			len = 7;
			struct tm *t = gdf_time2tm_time(hdr->Patient.Birthday);
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
		}

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[MFER 720-132]:\n");

		// tag 132: Patient Sex
		tag = 132;
		Header1[curPos]   = tag;
		Header1[curPos+1] = 1;
		Header1[curPos+2] = hdr->Patient.Sex;
		curPos += 3;

		// tag 133: Recording time
		tag = 133;
		len = 11;
		{
			struct tm *t = gdf_time2tm_time(hdr->T0);
			Header1[curPos++] = tag;
			Header1[curPos++] = len;
			*(uint16_t*)(Header1+curPos) = b_endian_u16(t->tm_year+1900);
			*(Header1+curPos+2) = (t->tm_mon+1);
			*(Header1+curPos+3) = (t->tm_mday);
			*(Header1+curPos+4) = (t->tm_hour);
			*(Header1+curPos+5) = (t->tm_min);
			*(Header1+curPos+6) = (t->tm_sec);
			*(uint16_t*)(Header1+curPos+7) = 0;
			*(uint16_t*)(Header1+curPos+9) = 0;
			curPos += len;
		}


		// tag  9: LeadId
		// tag 10: gdftyp
		// tag 11: SampleRate
		// tag 12: Cal
		// tag 13: Off
		hdr->HeadLen = curPos;
		// tag 63: channel-specific settings

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[MFER 720-63]:\n");

		tag = 63;
		size_t ch;
		for (ch=0; ch<hdr->NS; ch++) {

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[MFER 720-63 #%i/%i %i]:\n",ch,hdr->NS,hdr->CHANNEL[ch].LeadIdCode);

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
				Header1[ix++] = 2;
				*(uint16_t*)(Header1+ix) = hdr->CHANNEL[ch].LeadIdCode;
				len1 = 2;

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

#ifdef WITH_TMSiLOG
    	else if (hdr->TYPE==TMSiLOG) {
    		// ###FIXME: writing of TMSi-LOG file is experimental and not completed
    		hdr->FileName = FileName;
	    	FILE *fid = fopen(FileName,"wb");
		fprintf(fid,"FileId=TMSi PortiLab sample log file\n\rVersion=0001\n\r",NULL);
		struct tm *t = gdf_time2tm_time(hdr->T0);
		fprintf(fid,"DateTime=%04d/02d/02d-02d:02d:02d\n\r",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
		fprintf(fid,"Format=Float32\n\rLength=%f\n\rSignals=%04i\n\r",hdr->NRec*hdr->SPR/hdr->SampleRate,hdr->NS);
		const char* fn = strrchr(FileName,FILESEP);
		if (!fn) fn=FileName;
		size_t len = strcspn(fn,".");
		char* fn2 = (char*)malloc(len+1);
		strncpy(fn2,fn,len);
		fn2[len]=0;
		for (k=0; k<hdr->NS; k++) {
			fprintf(fid,"Signal%04d.Name=%s\n\r",k+1,hdr->CHANNEL[k].Label);
			char tmp[MAX_LENGTH_PHYSDIM+1];
			PhysDim(hdr->CHANNEL[k].PhysDimCode,tmp);
			fprintf(fid,"Signal%04d.UnitName=%s\n\r",k+1,tmp);
			fprintf(fid,"Signal%04d.Resolution=%f\n\r",k+1,hdr->CHANNEL[k].Cal);
			fprintf(fid,"Signal%04d.StoreRate=%f\n\r",k+1,hdr->SampleRate);
			fprintf(fid,"Signal%04d.File=%s.asc\n\r",k+1,fn2);
			fprintf(fid,"Signal%04d.Index=%04d\n\r",k+1,k+1);
		}
		fprintf(fid,"\n\r\n\r");
		fclose(fid);

		// ###FIXME: this belongs into SWRITE
		// write data file
		fn2 = (char*) realloc(fn2, strlen(FileName)+5);
		strcpy(fn2,FileName);
		strcpy(strrchr(fn2,'.'),".asc");
    		// hdr->FileName = fn2;
	    	fid = fopen(fn2,"wb");
	    	fprintf(fid,"%d\tHz\n\r\n\rN",hdr->SampleRate);
		for (k=0; k<hdr->NS; k++) {
			char tmp[MAX_LENGTH_PHYSDIM+1];
			PhysDim(hdr->CHANNEL[k].PhysDimCode,tmp);
			fprintf(fid,"\t%s(%s)",hdr->CHANNEL[k].Label,tmp);
		}
		for (k1=0; k1<hdr->SPR*hdr->NRec; k1++) {
			fprintf(fid,"\n%i",k1);
			for (k=0; k<hdr->NS; k++) {
				// TODO: Row/Column ordering
				fprintf(fid,"\t%f",hdr->data.block[]);
			}
		}

		fclose(fid);
		free(fn2);
	}
#endif  // WITH_TMSiLOG

	else {
		B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
		B4C_ERRMSG = "ERROR: Writing of format not supported\n";
		return(NULL);
	}

    	hdr->FileName = FileName;
	if ((hdr->TYPE != ASCII) && (hdr->TYPE != BIN) && (hdr->TYPE != HL7aECG) && (hdr->TYPE != TMSiLOG)){
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

	size_t bpb8 = 0;
	if (hdr->TYPE==AINF) {
		hdr->AS.bpb = 4;
		bpb8 = 32;
	}
	else
		hdr->AS.bpb = 0;

	for (k=0, hdr->SPR = 1; k<hdr->NS; k++) {
		hdr->CHANNEL[k].bi  = bpb8>>3;
		hdr->CHANNEL[k].bi8 = bpb8;
		if (hdr->CHANNEL[k].OnOff) {
			bpb8 += (GDFTYP_BITS[hdr->CHANNEL[k].GDFTYP] * hdr->CHANNEL[k].SPR);
			if (hdr->CHANNEL[k].SPR > 0)  // ignore sparse channels
				hdr->SPR = lcm(hdr->SPR, hdr->CHANNEL[k].SPR);
		}
	}
	hdr->AS.bpb8 = bpb8;
	hdr->AS.bpb  = bpb8>>3;
	if ((hdr->TYPE==GDF) && (bpb8 & 0x07)) {
		// each block must use whole number of bytes
		hdr->AS.bpb++;
		hdr->AS.bpb8 = hdr->AS.bpb<<3;
	}

}	// end of branch "write"


	if (hdr->FILE.POS != 0)
		fprintf(stdout,"Debugging Information: (Format=%d) FILE.POS=%d is not zero.\n",hdr->TYPE,hdr->FILE.POS);

	for (k=0; k<hdr->NS; k++)
	if  (GDFTYP_BITS[hdr->CHANNEL[k].GDFTYP] % 8) {

	if (hdr->TYPE==alpha)
		; // 12bit alpha is well tested
	else if  ((__BYTE_ORDER == __LITTLE_ENDIAN) && !hdr->FILE.LittleEndian)
			fprintf(stdout,"GDFTYP=%i [12bit LE/BE] not well tested\n",hdr->CHANNEL[k].GDFTYP);
	else if  ((__BYTE_ORDER == __LITTLE_ENDIAN) && hdr->FILE.LittleEndian)
			fprintf(stdout,"GDFTYP=%i [12bit LE/LE] not well tested\n",hdr->CHANNEL[k].GDFTYP);
	else if  ((__BYTE_ORDER == __BIG_ENDIAN) && hdr->FILE.LittleEndian)
			fprintf(stdout,"GDFTYP=%i [12bit BE/LE] not well tested\n",hdr->CHANNEL[k].GDFTYP);
	else if  ((__BYTE_ORDER == __BIG_ENDIAN) && !hdr->FILE.LittleEndian)
			fprintf(stdout,"GDFTYP=%i [12bit BE/BE] not well tested\n",hdr->CHANNEL[k].GDFTYP);
	}

	if (VERBOSE_LEVEL>8)
		fprintf(stdout,"sopen{return} %i %s\n", B4C_ERRNUM,GetFileTypeString(hdr->TYPE) );

	return(hdr);
}  // end of SOPEN



/****************************************************************************
	bpb8_collapsed_rawdata
	 computes the bytes per block when rawdata is collapsed
 ****************************************************************************/
size_t bpb8_collapsed_rawdata(HDRTYPE *hdr)
{
	size_t bpb8=0;
	CHANNEL_TYPE *CHptr;
	typeof(hdr->NS) k;
	for (k=0; k<hdr->NS; k++) {
		CHptr 	= hdr->CHANNEL+k;
		if (CHptr->OnOff) bpb8 += CHptr->SPR*GDFTYP_BITS[CHptr->GDFTYP];
	}
	return(bpb8);
}

/* ***************************************************************************
   collapse raw data
	this function is used to remove obsolete channels (e.g.
	status and annotation channels because the information
	as been already converted into the event table)
	that are not needed in GDF.

	re-allocates buffer (buf) to hold collapsed data
	bpb are the bytes per block.

 ****************************************************************************/

void collapse_rawdata(HDRTYPE *hdr)
{
	CHANNEL_TYPE *CHptr;
	size_t bpb;
//	char bitflag = 0;
	size_t	k1,k4,count,SZ;

	if (VERBOSE_LEVEL>8) fprintf(stdout,"collapse: started\n");

	bpb = bpb8_collapsed_rawdata(hdr);
	if (bpb == hdr->AS.bpb<<3) return; 	// no collapsing needed

	if ((bpb & 7) || (hdr->AS.bpb8 & 7)) {
		B4C_ERRNUM = B4C_RAWDATA_COLLAPSING_FAILED;
		B4C_ERRMSG = "collapse_rawdata: does not support bitfields";
	}
	bpb >>= 3;

	if (VERBOSE_LEVEL>8) fprintf(stdout,"collapse: bpb=%i/%i\n",bpb,hdr->AS.bpb);

	count = hdr->AS.length;

	uint8_t *buf = (uint8_t*) malloc(count*bpb);
	size_t bi = 0;
	for (k1=0; k1<hdr->NS; k1++) {
		CHptr 	= hdr->CHANNEL+k1;

		SZ = CHptr->SPR*GDFTYP_BITS[CHptr->GDFTYP];
		if (SZ & 7) {
			B4C_ERRNUM = B4C_RAWDATA_COLLAPSING_FAILED;
			B4C_ERRMSG = "collapse_rawdata: does not support bitfields";
		}
		SZ >>= 3;

		if (CHptr->OnOff)	/* read selected channels only */
		if (CHptr->SPR > 0) {

			if (VERBOSE_LEVEL>8) fprintf(stdout,"%i: %i %i %i %i \n",k1,bi,CHptr->bi,bpb,hdr->AS.bpb);

			for (k4 = 0; k4 < count; k4++) {
				size_t off1 = k4*hdr->AS.bpb + CHptr->bi;
				size_t off2 = k4*bpb + bi;

			if (VERBOSE_LEVEL>8) fprintf(stdout,"%i %i: %i %i \n",k1,k4,off1,off2);

				memcpy(buf + off2, hdr->AS.rawdata + off1, SZ);
			}
			bi += SZ;
		}
	}
	free(hdr->AS.rawdata);
	hdr->AS.rawdata = buf;
	hdr->AS.flag_collapsed_rawdata = 1;	// rawdata is now "collapsed"

	if (VERBOSE_LEVEL>8) fprintf(stdout,"collapse: finished\n");
}

/****************************************************************************/
/**	SREAD_RAW : segment-based                                          **/
/****************************************************************************/
size_t sread_raw(size_t start, size_t length, HDRTYPE* hdr, char flag) {
/*
 *	Reads LENGTH blocks with HDR.AS.bpb BYTES each
 * 	(and HDR.SPR samples).
 *	Rawdata is available in hdr->AS.rawdata.
 *
 *        start <0: read from current position
 *             >=0: start reading from position start
 *        length  : try to read length blocks
 *
 *	  flag!=0 : unused channels (those channels k where HDR.CHANNEL[k].OnOff==0)
 *		are collapsed
 */


	if (hdr->AS.flag_collapsed_rawdata && ! flag)
		hdr->AS.length = 0; // 	force reloading of data

	size_t	count;
	nrec_t	nelem;

	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"####SREAD-RAW########## start=%d length=%d bpb=%i\n",(int)start,(int)length, hdr->AS.bpb);

	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"sread raw 211: %d %d %d %d\n",(int)start, (int)length,  (int)hdr->NRec, (int)hdr->FILE.POS);

	if ((nrec_t)start > hdr->NRec)
		return(0);
	else if ((nrec_t)start+length < 0)
		return(0);
	else if (start < 0)
		start = hdr->FILE.POS;


	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"sread raw 216: %d %d %d %d\n",(int)start, (int)length, (int)hdr->NRec, (int)hdr->FILE.POS);

	// limit reading to end of data block
	if (hdr->NRec<0)
		nelem = length;
	else if (start>=hdr->NRec)
		nelem = 0;
	else
		nelem = min(length, hdr->NRec - start);

	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"sread raw 221: %i %i %i %i %i\n",(int)start, (int)length, (int)nelem, (int)hdr->NRec, (int)hdr->FILE.POS);

	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"sread raw 221 %i=?=%i  %i=?=%i \n", (int)start,(int)hdr->AS.first,(int)(start+nelem),(int)hdr->AS.length);


	if ((start >= hdr->AS.first) && ((start+nelem) <= (hdr->AS.first+hdr->AS.length))) {
		// Caching, no file-IO, data is already loaded into hdr->AS.rawdata
		hdr->FILE.POS = start;
		count = nelem;

		if (VERBOSE_LEVEL>7)
			fprintf(stdout,"sread-raw: 222\n");

	}
#ifndef WITHOUT_NETWORK
	else if (hdr->FILE.Des>0) {
		// network connection
		int s = bscs_requ_dat(hdr->FILE.Des,start,length,hdr);
		count = hdr->AS.length;

		if (VERBOSE_LEVEL>7)
			fprintf(stdout,"sread-raw from network: 222 count=%i\n",count);
	}
#endif
	else {
		if (VERBOSE_LEVEL>7)
			fprintf(stdout,"sread-raw: 223\n");

		// read required data block(s)
		if (ifseek(hdr, start*hdr->AS.bpb + hdr->HeadLen, SEEK_SET)<0) {
			if (VERBOSE_LEVEL>7)
				fprintf(stdout,"--%i %i %i %i \n",(int)(start*hdr->AS.bpb + hdr->HeadLen), (int)start, (int)hdr->AS.bpb, (int)hdr->HeadLen);
			return(0);
		}
		else
			hdr->FILE.POS = start;

		if (VERBOSE_LEVEL>7)
			fprintf(stdout,"sread-raw: 224 %i\n",hdr->AS.bpb);


		// allocate AS.rawdata
                if (log2(hdr->AS.bpb) + log2(nelem) + 1 >= sizeof(size_t)*8) {
                        // used to check the 2GByte limit on 32bit systems
                        B4C_ERRNUM = B4C_MEMORY_ALLOCATION_FAILED;
                        B4C_ERRMSG = "Size of rawdata buffer too large (exceeds size_t addressable space)!";
                        return(0);
                }
		hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata, hdr->AS.bpb*nelem);

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"#sread(%i %li)\n",hdr->HeadLen + hdr->FILE.POS*hdr->AS.bpb, iftell(hdr));

		// read data
		count = ifread(hdr->AS.rawdata, hdr->AS.bpb, nelem, hdr);
		hdr->AS.flag_collapsed_rawdata = 0;	// is rawdata not collapsed
//		if ((count<nelem) && ((hdr->NRec < 0) || (hdr->NRec > start+count))) hdr->NRec = start+count; // get NRec if NRec undefined, not tested yet.
		if (count<nelem) {
			fprintf(stderr,"warning: less then requested blocks read (%i/%i) from file %s - something went wrong\n",count,nelem,hdr->FileName);
			if (VERBOSE_LEVEL>7)
				fprintf(stderr,"warning: only %i instead of %i blocks read - something went wrong (bpb=%i,pos=%li)\n",count,nelem,hdr->AS.bpb,iftell(hdr));
		}
//		else    fprintf(stderr,"              %i            %i blocks read                        (bpb=%i,pos=%li)\n",count,nelem,hdr->AS.bpb,iftell(hdr));

		hdr->AS.first = start;
		hdr->AS.length= count;
	}
	// (uncollapsed) data is now in buffer hdr->AS.rawdata

	if (flag) {
		collapse_rawdata(hdr);
	}
	return(count);
}

/****************************************************************************
 	caching: load data of whole file into buffer
		 this will speed up data access, especially in interactive mode
 ****************************************************************************/
int cachingWholeFile(HDRTYPE* hdr) {

	sread_raw(0,hdr->NRec,hdr, 0);

	return((hdr->AS.first != 0) || (hdr->AS.length!=hdr->NRec));
}



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
 *      hdr->FLAG.LittleEndian controls swapping
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
 *
 * ToDo:
 *	- sample-based loading
 *
 */

	size_t			count,k1,k2,k4,k5,SZ,NS;//bi,bi8;
	uint16_t		GDFTYP;
	size_t	 		DIV;
	uint8_t			*ptr=NULL; // *buffer;
	CHANNEL_TYPE		*CHptr;
	int32_t			int32_value;
	biosig_data_type 	sample_value=NaN;
	size_t			toffset;	// time offset for rawdata
	biosig_data_type	*data1=NULL;

	if (start >= hdr->NRec) return(0);
	if ((start + length) < 0) return(0);

int V = VERBOSE_LEVEL;
//VERBOSE_LEVEL = 9;
	count = sread_raw(start, length, hdr, 0);

	if (B4C_ERRNUM) return(0);

	toffset = start - hdr->AS.first;

	// set position of file handle
	size_t POS = hdr->FILE.POS;
	hdr->FILE.POS += count;

	// count number of selected channels
	for (k1=0,NS=0; k1<hdr->NS; ++k1)
		if (hdr->CHANNEL[k1].OnOff) ++NS;

	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"SREAD: count=%i pos=[%i,%i,%i,%i], size of data = %ix%ix%ix%i = %i\n",(int)count,(int)start,(int)length,(int)POS,hdr->FILE.POS,(int)hdr->SPR, (int)count, (int)NS, sizeof(biosig_data_type), (int)(hdr->SPR * count * NS * sizeof(biosig_data_type)));

        if (log2(hdr->SPR) + log2(count) + log2(NS) + log2(sizeof(biosig_data_type)) + 1 >= sizeof(size_t)*8) {
                // used to check the 2GByte limit on 32bit systems
                B4C_ERRNUM = B4C_MEMORY_ALLOCATION_FAILED;
                B4C_ERRMSG = "Size of required data buffer too large (exceeds size_t addressable space)!";
                return(0);
        }
	// transfer RAW into BIOSIG data format
	if ((data==NULL) || hdr->Calib) {
		// local data memory required
		data1 = (biosig_data_type*) realloc(hdr->data.block, hdr->SPR * count * NS * sizeof(biosig_data_type));
		hdr->data.block = data1;
	}
	else
		data1 = data;

	char ALPHA12BIT = (hdr->TYPE==alpha) && (hdr->NS>0) && (hdr->CHANNEL[0].GDFTYP==(255+12));
	char MIT12BIT   = (hdr->TYPE==MIT  ) && (hdr->NS>0) && (hdr->CHANNEL[0].GDFTYP==(255+12));
#if (__BYTE_ORDER == __BIG_ENDIAN)
	char SWAP = hdr->FILE.LittleEndian;
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
	char SWAP = !hdr->FILE.LittleEndian;
#endif

	uint16_t MITTYP=0;
	if (hdr->TYPE==MIT) {
		MITTYP = *(uint16_t*)hdr->AS.auxBUF;
		if (VERBOSE_LEVEL>7)
			fprintf(stdout,"0x%x 0x%x \n",*(uint32_t*)hdr->AS.rawdata,*(uint32_t*)hdr->AS.rawdata);
	}

	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"sread 223 alpha12bit=%i SWAP=%i spr=%i   %p\n", ALPHA12BIT, SWAP, hdr->SPR, hdr->AS.rawdata);

	for (k1=0,k2=0; k1<hdr->NS; k1++) {
		CHptr 	= hdr->CHANNEL+k1;

	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"sread 223a #%i#%i: alpha12bit=%i SWAP=%i spr=%i   %p | bi=%i bpb=%i \n", k1,k2, ALPHA12BIT, SWAP, hdr->SPR, hdr->AS.rawdata,(int)CHptr->bi,(int)hdr->AS.bpb );

	if (CHptr->OnOff) {	/* read selected channels only */
	if (CHptr->SPR > 0) {
		DIV 	= hdr->SPR/CHptr->SPR;
		GDFTYP 	= CHptr->GDFTYP;
		SZ  	= GDFTYP_BITS[GDFTYP];
		int32_value = 0;
		uint8_t bitoff = 0;

		union {int16_t i16; uint16_t u16; uint32_t i32; float f32; uint64_t i64; double f64;} u;

		// TODO:  MIT data types
		for (k4 = 0; k4 < count; k4++)
		{  	uint8_t *ptr1;

			if (hdr->TYPE == FEF) {
				ptr1 = CHptr->bufptr;
			}
			else
				ptr1 = hdr->AS.rawdata + (k4+toffset)*hdr->AS.bpb + CHptr->bi;

		for (k5 = 0; k5 < CHptr->SPR; k5++)
		{

		// size_t off = (k4+toffset)*hdr->AS.bpb + CHptr->bi + (k5*SZ>>3);
		// ptr = hdr->AS.rawdata + off;
		ptr = ptr1 + (k5*SZ>>3);

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"SREAD 555: k_i = [%d %d %d %d ] 0x%08x[%f] @%p => ",(int)k1,(int)k2,(int)k4,(int)k5,(int)leu32p(ptr),(int)lef64p(ptr),ptr);

		switch (GDFTYP) {
		case 1:
			sample_value = (biosig_data_type)(*(int8_t*)ptr);
			break;
		case 2:
			sample_value = (biosig_data_type)(*(uint8_t*)ptr);
			break;
		case 3:
			if (hdr->TYPE==TMS32) {
				ptr = hdr->AS.rawdata + (k4+toffset)*hdr->AS.bpb + (k1+k5*hdr->NS)*(SZ>>3)+86;
#if __BYTE_ORDER == __BIG_ENDIAN
				sample_value = (biosig_data_type)(int16_t)bswap_16(*(int16_t*)ptr);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
				sample_value = (biosig_data_type)(*(int16_t*)ptr);
#endif
			}
			else if (SWAP) {
				sample_value = (biosig_data_type)(int16_t)bswap_16(*(int16_t*)ptr);
			}
			else {
				sample_value = (biosig_data_type)(*(int16_t*)ptr);
			}
			break;
		case 4:
			if (SWAP) {
				sample_value = (biosig_data_type)(uint16_t)bswap_16(*(uint16_t*)ptr);
			}
			else {
				sample_value = (biosig_data_type)(*(uint16_t*)ptr);
			}
			break;
		case 5:
			if (SWAP) {
				sample_value = (biosig_data_type)(int32_t)bswap_32(*(int32_t*)ptr);
			}
			else {
				sample_value = (biosig_data_type)(*(int32_t*)ptr);
			}
			break;
		case 6:
			if (SWAP) {
				sample_value = (biosig_data_type)(uint32_t)bswap_32(*(uint32_t*)ptr);
			}
			else {
				sample_value = (biosig_data_type)(*(uint32_t*)ptr);
			}
			break;
		case 7:
			if (SWAP) {
				sample_value = (biosig_data_type)(int64_t)bswap_64(*(int64_t*)ptr);
			}
			else {
				sample_value = (biosig_data_type)(*(int64_t*)ptr);
			}
			break;
		case 8:
			if (SWAP) {
				sample_value = (biosig_data_type)(uint64_t)bswap_64(*(uint64_t*)ptr);
			}
			else {
				sample_value = (biosig_data_type)(*(uint64_t*)ptr);
			}
			break;
		case 16:
			if (hdr->TYPE==TMS32) {
				ptr = hdr->AS.rawdata + (k4+toffset)*hdr->AS.bpb + (k1+k5*hdr->NS)*(SZ>>3)+86;
#if __BYTE_ORDER == __BIG_ENDIAN
				u.i32 = bswap_32(*(uint32_t*)(ptr));
				sample_value = (biosig_data_type)(u.f32);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
				sample_value = (biosig_data_type)(*(float*)(ptr));
#endif
			}
			else if (SWAP) {
				u.i32 = bswap_32(*(uint32_t*)(ptr));
				sample_value = (biosig_data_type)(u.f32);
			}
			else {
				sample_value = (biosig_data_type)(*(float*)(ptr));
			}
			break;

		case 17:
			if (SWAP) {
				u.i64 = bswap_64(*(uint64_t*)(ptr));
				sample_value = (biosig_data_type)(u.f64);
			}
			else {
				sample_value = (biosig_data_type)(*(double*)(ptr));
			}
			break;

		case 128:	// Nihon-Kohden little-endian int16 format
			u.u16 = leu16p(ptr) + 0x8000;
			sample_value = (biosig_data_type) (u.i16);
			break;

		case 255+12:
			if (ALPHA12BIT) {
				// get source address
				size_t off = (k4+toffset)*hdr->NS*SZ + hdr->CHANNEL[k1].bi8 + k5*SZ;
				ptr = hdr->AS.rawdata + (off>>3);

				if (off & 0x07)
					u.i16 = ptr[1] + ((ptr[0] & 0x0f)<<8);
				else
					u.i16 = (ptr[0]<<4) + (ptr[1] >> 4);

				if (u.i16 & 0x0800) u.i16 -= 0x1000;
				sample_value = (biosig_data_type)u.i16;
			}
			else if (MIT12BIT) {
				size_t off = (k4+toffset)*hdr->NS*SZ + hdr->CHANNEL[k1].bi8 + k5*SZ;
				ptr = hdr->AS.rawdata + (off>>3);
				//bitoff = k5*SZ & 0x07;
				if (off & 0x07)
					u.i16 = (((uint16_t)ptr[0] & 0xf0) << 4) + ptr[1];
				else
					//u.i16 = ((uint16_t)ptr[0]<<4) + (ptr[1] & 0x0f);
					u.i16 = leu16p(ptr) & 0x0fff;

				if (u.i16 & 0x0800) u.i16 -= 0x1000;
				sample_value = (biosig_data_type)u.i16;
			}
			else if (hdr->FILE.LittleEndian) {
				bitoff = k5*SZ & 0x07;
#if __BYTE_ORDER == __BIG_ENDIAN
				u.i16 = (leu16p(ptr) >> (4-bitoff)) & 0x0FFF;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
				u.i16 = (leu16p(ptr) >> bitoff) & 0x0FFF;
#endif
				if (u.i16 & 0x0800) u.i16 -= 0x1000;
				sample_value = (biosig_data_type)u.i16;
			}
			else {
				bitoff = k5*SZ & 0x07;
#if __BYTE_ORDER == __BIG_ENDIAN
				u.i16 = (beu16p(ptr) >> (4-bitoff)) & 0x0FFF;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
				u.i16 = (beu16p(ptr) >> (4-bitoff)) & 0x0FFF;
#endif
				if (u.i16 & 0x0800) u.i16 -= 0x1000;
				sample_value = (biosig_data_type)u.i16;
			}
			break;

		case 511+12:
			bitoff = k5*SZ & 0x07;
			if (hdr->FILE.LittleEndian) {
#if __BYTE_ORDER == __BIG_ENDIAN
				sample_value = (biosig_data_type)((leu16p(ptr) >> (4-bitoff)) & 0x0FFF);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
				sample_value = (biosig_data_type)((leu16p(ptr) >> bitoff) & 0x0FFF);
#endif
			} else {
#if __BYTE_ORDER == __BIG_ENDIAN
				sample_value = (biosig_data_type)((beu16p(ptr) >> (4-bitoff)) & 0x0FFF);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
				sample_value = (biosig_data_type)((beu16p(ptr) >> (4-bitoff)) & 0x0FFF);
#endif
			}

		case 255+24:
			if (hdr->FILE.LittleEndian) {
				int32_value = (*(uint8_t*)(ptr)) + (*(uint8_t*)(ptr+1)<<8) + (*(int8_t*)(ptr+2)*(1<<16));
				sample_value = (biosig_data_type)int32_value;
			}
			else {
				int32_value = (*(uint8_t*)(ptr+2)) + (*(uint8_t*)(ptr+1)<<8) + (*(int8_t*)(ptr)*(1<<16));
				sample_value = (biosig_data_type)int32_value;
			}
			break;

		case 511+24:
			if (hdr->FILE.LittleEndian) {
				int32_value = (*(uint8_t*)(ptr)) + (*(uint8_t*)(ptr+1)<<8) + (*(uint8_t*)(ptr+2)<<16);
				sample_value = (biosig_data_type)int32_value;
			}
			else {
				int32_value = (*(uint8_t*)(ptr+2)) + (*(uint8_t*)(ptr+1)<<8) + (*(uint8_t*)(ptr)<<16);
				sample_value = (biosig_data_type)int32_value;
			}
			break;

		default:
/*
			if (MITTYP==212)
				;
			else if (MITTYP==310)
				;
			else if (MITTYP==311)
				;
			else
*/

			B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
			B4C_ERRMSG = "Error SREAD: datatype not supported";
			exit(-1);
		}	// end switch

		// overflow and saturation detection
		if ((hdr->FLAG.OVERFLOWDETECTION) && ((sample_value <= CHptr->DigMin) || (sample_value >= CHptr->DigMax)))
			sample_value = NaN; 	// missing value
		else if (!hdr->FLAG.UCAL)	// scaling
			sample_value = sample_value * CHptr->Cal + CHptr->Off;

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"%f\n",sample_value);

		// resampling 1->DIV samples
		if (hdr->FLAG.ROW_BASED_CHANNELS) {
			size_t k3;
			for (k3=0; k3 < DIV; k3++)
				data1[k2 + (k4*hdr->SPR + k5*DIV + k3)*NS] = sample_value; // row-based channels
		} else {
			size_t k3;
			for (k3=0; k3 < DIV; k3++)
				data1[k2*count*hdr->SPR + k4*hdr->SPR + k5*DIV + k3] = sample_value; // column-based channels
		}

		}	// end for (k5 ....
		}	// end for (k4 ....

	}
	k2++;
	}}

	if (hdr->FLAG.ROW_BASED_CHANNELS) {
		hdr->data.size[0] = k2;			// rows
		hdr->data.size[1] = hdr->SPR*count;	// columns
	} else {
		hdr->data.size[0] = hdr->SPR*count;	// rows
		hdr->data.size[1] = k2;			// columns
	}

	/* read sparse samples */
	if (((hdr->TYPE==GDF) && (hdr->VERSION > 1.9)) || (hdr->TYPE==PDP)) {

		for (k1=0,k2=0; k1<hdr->NS; k1++) {
			CHptr 	= hdr->CHANNEL+k1;
			// Initialize sparse channels with NaNs
			if (CHptr->OnOff) {	/* read selected channels only */
				if (CHptr->SPR==0) {
					// sparsely sampled channels are stored in event table
					if (hdr->FLAG.ROW_BASED_CHANNELS) {
						for (k5 = 0; k5 < hdr->SPR*count; k5++)
							data1[k2 + k5*NS] = NaN;		// row-based channels
					} else {
						for (k5 = 0; k5 < hdr->SPR*count; k5++)
							data1[k2*count*hdr->SPR + k5] = NaN; 	// column-based channels
					}
				}
				k2++;
			}
		}

		double c = hdr->SPR / hdr->SampleRate * hdr->EVENT.SampleRate;
		size_t *ChanList = (size_t*)calloc(hdr->NS+1,sizeof(size_t));

		// Note: ChanList and EVENT.CHN start with index=1 (not 0)
		size_t ch = 0;
		for (k1=0; k1<hdr->NS; k1++) // list of selected channels
			ChanList[k1+1]= (hdr->CHANNEL[k1].OnOff ? ++ch : 0);

		for (k1=0; k1<hdr->EVENT.N; k1++)
		if (hdr->EVENT.TYP[k1] == 0x7fff) 	// select non-equidistant sampled value
		if (ChanList[hdr->EVENT.CHN[k1]] > 0)	// if channel is selected
		if ((hdr->EVENT.POS[k1] >= POS*c) && (hdr->EVENT.POS[k1] < hdr->FILE.POS*c)) {
			ptr = (uint8_t*)(hdr->EVENT.DUR + k1);

			k2 = ChanList[hdr->EVENT.CHN[k1]]-1;
			CHptr = hdr->CHANNEL+k2;
			DIV 	= (uint32_t)ceil(hdr->SampleRate/hdr->EVENT.SampleRate);
			GDFTYP 	= CHptr->GDFTYP;
			SZ  	= GDFTYP_BITS[GDFTYP]>>3;
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
				// assume LITTLE_ENDIAN format
				int32_value = (*(uint8_t*)(ptr)) + (*(uint8_t*)(ptr+1)<<8) + (*(int8_t*)(ptr+2)*(1<<16));
				sample_value = (biosig_data_type)int32_value;
			}
			else if (GDFTYP==511+24) {
				// assume LITTLE_ENDIAN format
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
			if (hdr->FLAG.ROW_BASED_CHANNELS) {
				size_t k3;
				for (k3=0; k3 < DIV; k3++)
					data1[k2 + (k5 + k3)*NS] = sample_value;
			} else {
				size_t k3;
				for (k3=0; k3 < DIV; k3++)
					data1[k2 * count * hdr->SPR + k5 + k3] = sample_value;
			}

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"E%02i: s(1)= %d %e %e %e\n",k1,leu32p(ptr),sample_value,(*(double*)(ptr)),(*(float*)(ptr)));

		}
		free(ChanList);
	}
	else if (hdr->TYPE==TMS32) {
		// post-processing TMS32 files: last block can contain undefined samples
		size_t spr = lei32p(hdr->AS.Header+121);
		if (hdr->FILE.POS*hdr->SPR > spr)
		for (k2=0; k2<NS; k2++) {
			for (k5 = spr - POS*hdr->SPR; k5 < hdr->SPR*count; k5++)
			if (hdr->FLAG.ROW_BASED_CHANNELS)
				data1[k2 + k5*NS] = NaN;		// row-based channels
			else
				data1[k2*count*hdr->SPR + k5] = NaN; 	// column-based channels
		}
	}

#ifdef CHOLMOD_H
	if (hdr->Calib)
        if (!hdr->FLAG.ROW_BASED_CHANNELS)
                fprintf(stderr,"Error SREAD: Re-Referencing on column-based data not supported.");
        else {
			cholmod_dense X,Y;
			X.nrow = hdr->data.size[0];
			X.ncol = hdr->data.size[1];
			X.d    = hdr->data.size[0];
			X.nzmax= hdr->data.size[1]*hdr->data.size[0];
			X.x    = data1;
                        X.xtype = CHOLMOD_REAL;
                        X.dtype = CHOLMOD_DOUBLE;

			Y.nrow = hdr->Calib->ncol;
			Y.ncol = hdr->data.size[1];
			Y.d    = Y.nrow;
			Y.nzmax= Y.nrow * Y.ncol;
			if (data)
				Y.x    = data;
			else
				Y.x    = malloc(Y.nzmax*sizeof(double));

                        Y.xtype = CHOLMOD_REAL;
                        Y.dtype = CHOLMOD_DOUBLE;

			double alpha[]={1,0},beta[]={0,0};

                        cholmod_common c ;
                        cholmod_start (&c) ; // start CHOLMOD

			cholmod_sdmult(hdr->Calib,1,alpha,beta,&X,&Y,&c);
                        cholmod_finish (&c) ; /* finish CHOLMOD */

			if (VERBOSE_LEVEL>8) fprintf(stdout,"%f -> %f\n",*(double*)X.x,*(double*)Y.x);
			free(X.x);
			if (data==NULL)
				hdr->data.block = (biosig_data_type*)Y.x;
			else
				hdr->data.block = NULL;

        		hdr->data.size[0] = Y.nrow;

	}
#endif

	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"sread - end \n");

VERBOSE_LEVEL = V;

	return(count);

}  // end of SREAD


#ifdef __GSL_MATRIX_DOUBLE_H__
/****************************************************************************/
/**	GSL_SREAD : GSL-version of sread                                   **/
/****************************************************************************/
size_t gsl_sread(gsl_matrix* m, size_t start, size_t length, HDRTYPE* hdr) {
/* 	same as sread but return data is of type gsl_matrix
*/
	// TODO: testing

        size_t count = sread(NULL, start, length, hdr);
	size_t n = hdr->data.size[0]*hdr->data.size[1];

	if (m->owner && m->block) gsl_block_free(m->block);
	m->block = gsl_block_alloc(n);
	m->block->data = hdr->data.block;

	m->size1 = hdr->data.size[1];
	m->tda   = hdr->data.size[0];
	m->size2 = hdr->data.size[0];
	m->data  = m->block->data;
	m->owner = 1;
	hdr->data.block = NULL;

	return(count);
}
#endif


/****************************************************************************/
/**                     SWRITE                                             **/
/****************************************************************************/
size_t swrite(const biosig_data_type *data, size_t nelem, HDRTYPE* hdr) {
/*
 *	writes NELEM blocks with HDR.AS.bpb BYTES each,
 */
	uint8_t		*ptr;
	size_t		count=0,k1,k2,k4,k5,DIV,SZ=0;
	int 		GDFTYP;
	CHANNEL_TYPE*	CHptr;
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

#if (__BYTE_ORDER == __BIG_ENDIAN)
	char SWAP = hdr->FILE.LittleEndian;
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
	char SWAP = !hdr->FILE.LittleEndian;
#endif
	// char SWAP = hdr->FLAG.SWAP;

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
#define MAX_INT64  ((((uint64_t)1)<<63)-1)
#define MIN_INT64  ((int64_t)((uint64_t)1)<<63)
#define MAX_UINT64 ((uint64_t)0xffffffffffffffffl)
#define MIN_UINT64 ((uint64_t)0)


	size_t bpb8 = bpb8_collapsed_rawdata(hdr);

	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"swrite sz=%i\n",hdr->NRec*bpb8>>3);

	if ((hdr->NRec*bpb8>0) && (hdr->TYPE != SCP_ECG)) {
	// memory allocation for SCP is done in SOPEN_SCP_WRITE Section 6
		ptr = (typeof(ptr))realloc(hdr->AS.rawdata, (hdr->NRec*bpb8>>3)+1);
		if (ptr==NULL) {
			B4C_ERRNUM = B4C_INSUFFICIENT_MEMORY;
			B4C_ERRMSG = "SWRITE: memory allocation failed.";
			exit(-1);
		}
		else
			hdr->AS.rawdata = (uint8_t*)ptr;
	}


	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"swrite 311: %li %i\n",hdr->NRec,hdr->NS);

	size_t bi8 = 0;
	for (k1=0,k2=0; k1<hdr->NS; k1++) {
	CHptr 	= hdr->CHANNEL+k1;

	if (CHptr->OnOff != 0) {
	if (CHptr->SPR) {

		DIV 	= hdr->SPR/CHptr->SPR;
		GDFTYP 	= CHptr->GDFTYP;
		SZ  	= GDFTYP_BITS[GDFTYP];
		iCal	= 1/CHptr->Cal;
		//iOff	= CHptr->DigMin - CHptr->PhysMin*iCal;
		iOff	= -CHptr->Off*iCal;

		size_t col = (hdr->data.size[1-hdr->FLAG.ROW_BASED_CHANNELS]<hdr->NS) ? k2 : k1;        // if collapsed data, use k2, otherwise use k1

	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"swrite 312=#%i gdftyp=%i %i %i %i %f %f %f %f %i\n",k1,GDFTYP,(int)bi8,(int)SZ,(int)CHptr->SPR,CHptr->Cal,CHptr->Off,iCal,iOff,(int)bpb8);

		for (k4 = 0; k4 < hdr->NRec; k4++) {
		for (k5 = 0; k5 < CHptr->SPR; k5++) {

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"swrite 313a #%i: [%i %i] %i %i %i %i %i\n",(int)k1,(int)hdr->data.size[0],(int)hdr->data.size[1],(int)k4,(int)k5,(int)hdr->SPR,(int)DIV,(int)nelem);

			size_t k3=0;
			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"swrite 313+: [%i %i %i %i %i] %i %i %i %i %i %i\n",(int)k1,(int)k2,(int)k3,(int)k4,(int)k5,(int)col,(int)hdr->data.size[0],(int)hdr->data.size[1],(int)hdr->SPR,nelem,(int)hdr->NRec);

        		if (hdr->FLAG.ROW_BASED_CHANNELS) {
            			for (k3=0, sample_value=0.0; k3 < DIV; k3++)
        				sample_value += data[col + (k4*hdr->SPR + k5*DIV + k3)*hdr->data.size[0]];
                        }
            		else {
            			for (k3=0, sample_value=0.0; k3 < DIV; k3++)
        				sample_value += data[col*nelem*hdr->SPR + k4*hdr->SPR + k5*DIV + k3];
                        }

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"swrite 313b: %f/%i\n",sample_value,DIV);

			sample_value /= DIV;

			if (!hdr->FLAG.UCAL)	// scaling
				sample_value = sample_value * iCal + iOff;

			// get target address
			//ptr = hdr->AS.rawdata + k4*hdr->AS.bpb + hdr->CHANNEL[k1].bi + k5*SZ;
			//ptr = hdr->AS.rawdata + (k4*bpb8 + bi8 + k5*SZ)>>3;

			//size_t off = k4*hdr->AS.bpb8 + hdr->CHANNEL[k1].bi8 + (k5*SZ);
			size_t off = k4*bpb8 + bi8 + (k5*SZ);
			ptr = hdr->AS.rawdata + (off>>3);

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"swrite 313e %i %i %li %f\n",k4,k5,off>>3,sample_value);

			// mapping of raw data type to (biosig_data_type)
			switch (GDFTYP) {
			case 3:
				if      (sample_value > MAX_INT16) val.i16 = MAX_INT16;
				else if (sample_value > MIN_INT16) val.i16 = (int16_t) sample_value;
				else     val.i16 = MIN_INT16;
				if (!SWAP)
					*(int16_t*)ptr = val.i16;
				else
					*(int16_t*)ptr = bswap_16(val.i16);
				break;

			case 4:
				if      (sample_value > MAX_UINT16) val.u16 = MAX_UINT16;
				else if (sample_value > MIN_UINT16) val.u16 = (uint16_t) sample_value;
				else     val.u16 = MIN_UINT16;
				*(uint16_t*)ptr = l_endian_u16(val.u16);
				break;

			case 16:
				*(float*)ptr  = l_endian_f32((float)sample_value);
				break;

			case 17:
				*(double*)ptr = l_endian_f64((double)sample_value);
				break;

			case 0:
				if      (sample_value > MAX_INT8) val.i8 = MAX_INT8;
				else if (sample_value > MIN_INT8) val.i8 = (int8_t) sample_value;
				else     val.i8 = MIN_INT8;
				*(int8_t*)ptr = val.i8;
				break;

			case 1:
				if      (sample_value > MAX_INT8) val.i8 = MAX_INT8;
				else if (sample_value > MIN_INT8) val.i8 = (int8_t) sample_value;
				else     val.i8 = MIN_INT8;
				*(int8_t*)ptr = val.i8;
				break;

			case 2:
				if      (sample_value > MAX_UINT8) val.u8 = MAX_UINT8;
				else if (sample_value > MIN_UINT8) val.u8 = (uint8_t) sample_value;
				else      val.u8 = MIN_UINT8;
				*(uint8_t*)ptr = val.u8;
				break;

			case 5:
				if      (sample_value > ldexp(1.0,31)-1) val.i32 = MAX_INT32;
				else if (sample_value > ldexp(-1.0,31)) val.i32 = (int32_t) sample_value;
				else     val.i32 = MIN_INT32;
				if (!SWAP)
					*(int32_t*)ptr = val.i32;
				else
					*(int32_t*)ptr = bswap_32(val.i32);
				break;

			case 6:
				if      (sample_value > ldexp(1.0,32)-1.0) val.u32 = MAX_UINT32;
				else if (sample_value > 0.0) val.u32 = (uint32_t) sample_value;
				else     val.u32 = MIN_UINT32;
				*(uint32_t*)ptr = l_endian_u32(val.u32);
				break;
			case 7:
				if      (sample_value > ldexp(1.0,63)-1.0) val.i64 = MAX_INT64;
				else if (sample_value > -ldexp(1.0,63)) val.i64 = (int64_t) sample_value;
				else     val.i64 = MIN_INT64;
				*(int64_t*)ptr = l_endian_i64(val.i64);
				break;
			case 8:
				if      (sample_value > ldexp(1.0,64)-1.0) val.u64 = (uint64_t)(-1);
				else if (sample_value > 0.0) val.u64 = (uint64_t) sample_value;
				else     val.u64 = 0;
				*(uint64_t*)ptr = l_endian_u64(val.u64);
				break;

			case 255+24:
				if      (sample_value > MAX_INT24) val.i32 = MAX_INT24;
				else if (sample_value > MIN_INT24) val.i32 = (int32_t) sample_value;
				else     val.i32 = MIN_INT24;
				*(uint8_t*)ptr = (uint8_t)(val.i32 & 0x000000ff);
				*((uint8_t*)ptr+1) = (uint8_t)((val.i32>>8) & 0x000000ff);
				*((uint8_t*)ptr+2) = (uint8_t)((val.i32>>16) & 0x000000ff);
				break;

			case 511+24:
				if      (sample_value > MAX_UINT24) val.i32 = MAX_UINT24;
				else if (sample_value > MIN_UINT24) val.i32 = (int32_t) sample_value;
				else     val.i32 = MIN_UINT24;
				*(uint8_t*)ptr     =  val.i32 & 0x000000ff;
				*((uint8_t*)ptr+1) = (uint8_t)((val.i32>>8) & 0x000000ff);
				*((uint8_t*)ptr+2) = (uint8_t)((val.i32>>16) & 0x000000ff);
				break;

			case 255+12:
			case 511+12: {
				if (GDFTYP == 255+12) {
					if      (sample_value > ((1<<11)-1)) val.i16 =  (1<<11)-1;
					else if (sample_value > -(1<<11))  val.i16 = (int16_t) sample_value;
					else     val.i16 = -(1<<11);
				}
				else if (GDFTYP == 511+12) {
					if      (sample_value > ((1<<12)-1)) val.u16 =  (1<<12)-1;
					else if (sample_value > 0)  val.u16 = (int16_t) sample_value;
					else     val.u16 = 0;
				}

				if (hdr->FILE.LittleEndian) {
					uint16_t acc = l_endian_u16(*(uint16_t*)ptr);
					if (off)
						*(uint16_t*)ptr = l_endian_u16( (acc & 0x000F) | (val.u16<<4));
//						leu16a(leu16p(ptr) & 0x000F) | (i16<<4), ptr);
					else
						*(uint16_t*)ptr = l_endian_u16( (acc & 0xF000) | (val.u16 & 0x0FFF));
//						leu16a(leu16p(ptr) & 0xF000) | i16, ptr);
				}
				else {
					uint16_t acc = b_endian_u16(*(uint16_t*)ptr);
					if (!off)
						*(uint16_t*)ptr = b_endian_u16( (acc & 0x000F) | (val.u16<<4));
//						beu16a(beu16p(ptr) & 0x000F) | (i16<<4), ptr);
					else
						*(uint16_t*)ptr = b_endian_u16( (acc & 0xF000) | (val.u16 & 0x0FFF));
//						beu16a(beu16p(ptr) & 0xF000) | i16, ptr);
				}
				break;
			}
			default:
				B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
				B4C_ERRMSG = "SWRITE: datatype not supported";
				exit(-1);
			}
		}        // end for k5
		}        // end for k4
		}        // end if SPR
		k2++;
		bi8 += SZ*CHptr->SPR;
	}       // end if OnOff
	}	// end for k1

	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"swrite 313\n");

#ifndef WITHOUT_NETWORK
	if (hdr->FILE.Des>0) {

	if (VERBOSE_LEVEL>7) fprintf(stdout,"bscs_send_dat sz=%i\n",hdr->NRec*bpb8>>3);

		int s = bscs_send_dat(hdr->FILE.Des,hdr->AS.rawdata,hdr->NRec*bpb8>>3);

	if (VERBOSE_LEVEL>7) fprintf(stdout,"bscs_send_dat succeeded %i\n",s);

	}
	else
#endif
	if ((hdr->TYPE == ASCII) || (hdr->TYPE == BIN)) {
		char fn[1025];
		HDRTYPE H1;
		H1.FILE.COMPRESSION = hdr->FILE.COMPRESSION;
		char e1 = (hdr->TYPE == ASCII ? 'a' : 's');

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"swrite ASCII/BIN\n");

		strcpy(fn,hdr->FileName);
		if (strrchr(fn,'.')==NULL)
			strcat(fn,".");

		for (k1=0; k1<hdr->NS; k1++)
    		if (hdr->CHANNEL[k1].OnOff) {
			CHptr 	= hdr->CHANNEL+k1;
    			sprintf(strrchr(fn,'.'),".%c%02i",e1,k1+1);
			if (H1.FILE.COMPRESSION)
	    			strcat(fn,"_gz");
	    		else

    			if (VERBOSE_LEVEL>7)
				fprintf(stdout,"#%i: %s\n",k1,fn);

			H1.FileName = fn;
			ifopen(&H1,"wb");

			if (hdr->TYPE == ASCII) {
				DIV = hdr->SPR/CHptr->SPR;
				size_t k2;
				for (k2=0; k2<CHptr->SPR*hdr->NRec; k2++) {
					biosig_data_type i = 0.0;
					size_t k3;
						// TODO: row channels
					if (hdr->FLAG.ROW_BASED_CHANNELS)
						for (k3=0; k3<DIV; k3++)
							i += hdr->data.block[k1+(k2*DIV+k3)*hdr->data.size[0]];
					else
						// assumes column channels
						for (k3=0; k3<DIV; k3++)
							i += hdr->data.block[hdr->SPR*hdr->NRec*k1+k2*DIV+k3];

/*
        		if (hdr->FLAG.ROW_BASED_CHANNELS) {
            			for (k3=0, sample_value=0.0; k3 < DIV; k3++)
        				sample_value += data[col + (k4*hdr->SPR + k5*DIV + k3)*hdr->data.size[0]];
                        }
            		else {
            			for (k3=0, sample_value=0.0; k3 < DIV; k3++)
        				sample_value += data[col*nelem*hdr->SPR + k4*hdr->SPR + k5*DIV + k3];
                        }
*/

#ifdef ZLIB_H
					if (H1.FILE.COMPRESSION)
						gzprintf(H1.FILE.gzFID,"%f\n",i/DIV);
					else
#endif
						fprintf(H1.FILE.FID,"%f\n",i/DIV);
				}
			}
			else if (hdr->TYPE == BIN) {
				uint32_t nbytes	= (GDFTYP_BITS[hdr->CHANNEL[k1].GDFTYP]*hdr->CHANNEL[k1].SPR)>>3;
				ifwrite(hdr->AS.rawdata+hdr->CHANNEL[k1].bi, nbytes, hdr->NRec, &H1);
			}
			ifclose(&H1);
		}
		count = hdr->NRec;
	}
	else if ((hdr->TYPE != SCP_ECG) && (hdr->TYPE != HL7aECG)) {
		// for SCP: writing to file is done in SCLOSE
		count = ifwrite((uint8_t*)(hdr->AS.rawdata), hdr->AS.bpb, hdr->NRec, hdr);
	}
	else { 	// SCP_ECG, HL7aECG#ifdef CHOLMOD_H

		count = 1;
	}

	// set position of file handle
	hdr->FILE.POS += count;

	return(count);

}  // end of SWRITE


/****************************************************************************/
/**                     SEOF                                               **/
/****************************************************************************/
int seof(HDRTYPE* hdr)
{
	return (hdr->FILE.POS >= hdr->NRec);
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

	if (VERBOSE_LEVEL>8) fprintf(stdout,"sclose(121)\n");

        if (hdr==NULL) return(0);

	size_t k;
	for (k=0; k<hdr->NS; k++) {
		// replace Nihon-Kohden code with standard code
		if (hdr->CHANNEL[k].GDFTYP==128)
			hdr->CHANNEL[k].GDFTYP=3;
	}

	if (VERBOSE_LEVEL>8) fprintf(stdout,"sclose(122) OPEN=%i %s\n",hdr->FILE.OPEN,GetFileTypeString(hdr->TYPE));

#ifdef WITH_FEF
	if (hdr->TYPE == FEF) sclose_fef_read(hdr);
#endif

#ifndef WITHOUT_NETWORK
	if (hdr->FILE.Des>0) {
		// network connection
		if (hdr->FILE.OPEN > 1) bscs_send_evt(hdr->FILE.Des,hdr);
  		int s = bscs_close(hdr->FILE.Des);
  		if (s) {
			B4C_ERRNUM = B4C_SCLOSE_FAILED;
			B4C_ERRMSG = "bscs_close failed";
  		}
  		hdr->FILE.Des = 0;
  		hdr->FILE.OPEN = 0;
		bscs_disconnect(hdr->FILE.Des);
	}
	else
#endif
	if ((hdr->FILE.OPEN>1) && ((hdr->TYPE==GDF) || (hdr->TYPE==EDF) || (hdr->TYPE==BDF)))
	{

		if (VERBOSE_LEVEL>8) fprintf(stdout,"sclose(121) nrec= %li\n",hdr->NRec);

		// WRITE HDR.NRec
		pos = (iftell(hdr)-hdr->HeadLen);
		if (hdr->NRec<0)
		{	if (pos>0) 	hdr->NRec = pos/hdr->AS.bpb;
			else		hdr->NRec = 0;
			if (hdr->TYPE==GDF) {
				*(int64_t*)tmp = l_endian_i64(hdr->NRec);
				len = sizeof(hdr->NRec);
			}
			else {
				len = sprintf(tmp,"%Ld",hdr->NRec);
				if (len>8) fprintf(stderr,"Warning: NRec is (%s) to long.\n",tmp);
			}
			/* ### FIXME : gzseek supports only forward seek */
			if (hdr->FILE.COMPRESSION>0)
				fprintf(stderr,"Warning: writing NRec in gz-file requires gzseek which may not be supported.\n");
			ifseek(hdr,236,SEEK_SET);
			ifwrite(tmp,len,1,hdr);
		}

		if (VERBOSE_LEVEL>7)
			fprintf(stdout, "888: File Type=%s ,N#of Events %i,bpb=%i\n",GetFileTypeString(hdr->TYPE),hdr->EVENT.N,hdr->AS.bpb);

		if ((hdr->TYPE==GDF) && (hdr->EVENT.N>0)) {

			size_t len = hdrEVT2rawEVT(hdr);
			ifseek(hdr, hdr->HeadLen + hdr->AS.bpb*hdr->NRec, SEEK_SET);
			ifwrite(hdr->AS.rawEventData, len, 1, hdr);

//			write_gdf_eventtable(hdr);
		}

	}
	else if ((hdr->FILE.OPEN>1) && (hdr->TYPE==SCP_ECG)) {
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
		hdr->FILE.OPEN = 0;
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

	size_t len = hdrEVT2rawEVT(hdr);
	ifseek(hdr, hdr->HeadLen + hdr->AS.bpb*hdr->NRec, SEEK_SET);
	ifwrite(hdr->AS.rawEventData, len, 1, hdr);
//	write_gdf_eventtable(hdr);

	ifseek(hdr,filepos,SEEK_SET);

	return(0);
}


/****************************************************************************/
/**                     HDR2ASCII                                          **/
/**	displaying header information                                      **/
/****************************************************************************/
int hdr2ascii(HDRTYPE* hdr, FILE *fid, int VERBOSE)
{
	CHANNEL_TYPE* 	cp;
	struct tm  	*T0;
	float		age;

	if (!GLOBAL_EVENTCODES_ISLOADED) LoadGlobalEventCodeTable();

	if (VERBOSE==7) {
		T0 = gdf_time2tm_time(hdr->T0);
		char tmp[60];
		strftime(tmp, 59, "%x %X %Z", T0);
		fprintf(fid,"\tStartOfRecording: %s\n",tmp);
		return(0);
	}

	if (VERBOSE>0) {
		/* demographic information */
		fprintf(fid,"\n===========================================\n[FIXED HEADER]\n");
//		fprintf(fid,"\nPID:\t|%s|\nPatient:\n",hdr->AS.PID);
		fprintf(fid,   "Recording:\n\tID              : %s\n",hdr->ID.Recording);
		fprintf(fid,               "\tInstitution     : %s\n",hdr->ID.Hospital);
		fprintf(fid,               "\tTechnician      : %s\t# default: localuser\n",hdr->ID.Technician);
		char tmp[60];
		strncpy(tmp,(char*)&hdr->ID.Equipment,8);
		tmp[8] = 0;
		fprintf(fid,               "\tEquipment       : %s\n",tmp);
		if (VERBOSE_LEVEL>8)
			fprintf(fid,       "\t                  %#.16Lx\n",(uint64_t)hdr->ID.Equipment);
		uint8_t k,IPv6=0;
		for (k=4; k<16; k++) IPv6 |= hdr->IPaddr[k];
		if (IPv6) fprintf(fid,     "\tIPv6 address    : %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",hdr->IPaddr[0],hdr->IPaddr[1],hdr->IPaddr[2],hdr->IPaddr[3],hdr->IPaddr[4],hdr->IPaddr[5],hdr->IPaddr[6],hdr->IPaddr[7],hdr->IPaddr[8],hdr->IPaddr[9],hdr->IPaddr[10],hdr->IPaddr[11],hdr->IPaddr[12],hdr->IPaddr[13],hdr->IPaddr[14],hdr->IPaddr[15]);
		else fprintf(fid,          "\tIPv4 address    : %u.%u.%u.%u",hdr->IPaddr[0],hdr->IPaddr[1],hdr->IPaddr[2],hdr->IPaddr[3]);

		fprintf(fid,"\t # default:local host\nManufacturer:\n\tName            : %s\n",hdr->ID.Manufacturer.Name);
		fprintf(fid,               "\tModel           : %s\n",hdr->ID.Manufacturer.Model);
		fprintf(fid,               "\tVersion         : %s\n",hdr->ID.Manufacturer.Version);
		fprintf(fid,               "\tSerialNumber    : %s\n",hdr->ID.Manufacturer.SerialNumber);
		fprintf(fid,     "Patient:\n\tID              : %s\n",hdr->Patient.Id);
		if (hdr->Patient.Name!=NULL)
			fprintf(fid,       "\tName            : %s\n",hdr->Patient.Name);

		if (hdr->Patient.Birthday>0)
			age = (hdr->T0 - hdr->Patient.Birthday)/ldexp(365.25,32);
		else
			age = NaN;

		if (hdr->Patient.Height)
			fprintf(fid,"\tHeight          : %i cm\n",hdr->Patient.Height);
		if (hdr->Patient.Height)
			fprintf(stdout,"\tWeight          : %i kg\n",hdr->Patient.Weight);

		const char *Gender[] = {"unknown","male","female","unknown"};
		const char *EyeImpairment[] = {"unknown","no","yes","corrected"};
		const char *HeartImpairment[] = {"unknown","no","yes","pacemaker"};
		fprintf(fid,"\tGender          : %s\n",Gender[hdr->Patient.Sex]);
		fprintf(fid,"\tEye Impairment  : %s\n",EyeImpairment[hdr->Patient.Impairment.Visual]);
		fprintf(fid,"\tHeart Impairment: %s\n",HeartImpairment[hdr->Patient.Impairment.Heart]);
		if (hdr->Patient.Birthday) {
			T0 = gdf_time2tm_time(hdr->Patient.Birthday);
			fprintf(fid,"\tAge             : %4.1f years\n\tBirthday        : (%.6f) %s ",age,ldexp(hdr->Patient.Birthday,-32),asctime(T0));
		}
		else
			fprintf(fid,"\tAge             : ----\n\tBirthday        : unknown\n");

		T0 = gdf_time2tm_time(hdr->T0);
		strftime(tmp, 59, "%x %X %Z", T0);
		fprintf(fid,"\tStartOfRecording: (%.6f) %s\n",ldexp(hdr->T0,-32),asctime(T0));
		if (hdr->AS.bci2000 != NULL) {
			size_t c = min(39,strcspn(hdr->AS.bci2000,"\xa\xd"));
			strncpy(tmp,hdr->AS.bci2000,c); tmp[c]=0;
			fprintf(fid,"BCI2000 [%i]\t\t: <%s...>\n",strlen(hdr->AS.bci2000),tmp);
		}
		fprintf(fid,"bpb=%i\n",hdr->AS.bpb);
		fprintf(fid,"row-based=%i\n",hdr->FLAG.ROW_BASED_CHANNELS);
		fprintf(fid,"uncalib  =%i\n",hdr->FLAG.UCAL);
		fprintf(fid,"OFdetect =%i\n",hdr->FLAG.OVERFLOWDETECTION);
	}

	if (VERBOSE>1) {
		/* display header information */
		fprintf(fid,"FileName:\t%s\nType    :\t%s\nVersion :\t%4.2f\nHeadLen :\t%i\n",hdr->FileName,GetFileTypeString(hdr->TYPE),hdr->VERSION,hdr->HeadLen);
//		fprintf(fid,"NoChannels:\t%i\nSPR:\t\t%i\nNRec:\t\t%Li\nDuration[s]:\t%u/%u\nFs:\t\t%f\n",hdr->NS,hdr->SPR,hdr->NRec,hdr->Dur[0],hdr->Dur[1],hdr->SampleRate);
		fprintf(fid,"NoChannels:\t%i\nSPR:\t\t%i\nNRec:\t\t%Li\nFs:\t\t%f\n",hdr->NS,hdr->SPR,hdr->NRec,hdr->SampleRate);
		fprintf(fid,"Events/Annotations:\t%i\nEvents/SampleRate:\t%f\n",hdr->EVENT.N,hdr->EVENT.SampleRate);
	}

	if (VERBOSE>2) {
		/* channel settings */
		fprintf(fid,"\n[CHANNEL HEADER] %p",hdr->CHANNEL);
		fprintf(fid,"\n#No  LeadId Label\tFs[Hz]\tSPR\tGDFTYP\tCal\tOff\tPhysDim PhysMax  PhysMin DigMax DigMin HighPass LowPass Notch X Y Z");
		size_t k;
#ifdef CHOLMOD_H
                int NS = hdr->NS;
                if (hdr->Calib) NS += hdr->Calib->ncol;
		for (k=0; k<NS; k++) {
		        if (k<hdr->NS)
        			cp = hdr->CHANNEL+k;
        		else
        			cp = hdr->rerefCHANNEL + k - hdr->NS;
#else
		for (k=0; k<hdr->NS; k++) {
       			cp = hdr->CHANNEL+k;
#endif

			char p[MAX_LENGTH_PHYSDIM+1];
			const char *label = cp->Label;
			if (label==NULL || strlen(label)==0) label = LEAD_ID_TABLE[cp->LeadIdCode];

			if (cp->PhysDimCode) PhysDim(cp->PhysDimCode, p); else p[0] = 0;
			fprintf(fid,"\n#%2i: %3i %i %-17s\t%5f %5i",
				k+1,cp->LeadIdCode,cp->bi8,label,cp->SPR*hdr->SampleRate/hdr->SPR,cp->SPR);

			if      (cp->GDFTYP<20)  fprintf(fid," %s  ",gdftyp_string[cp->GDFTYP]);
			else if (cp->GDFTYP>511) fprintf(fid, " bit%i  ", cp->GDFTYP-511);
			else if (cp->GDFTYP>255) fprintf(fid, " bit%i  ", cp->GDFTYP-255);

			fprintf(fid,"%e %e %s\t%g\t%g\t%5f\t%5f\t%5f\t%5f\t%5f\t%5f\t%5f\t%5f",
				cp->Cal, cp->Off, p,
				cp->PhysMax, cp->PhysMin, cp->DigMax, cp->DigMin,cp->HighPass,cp->LowPass,cp->Notch,
				cp->XYZ[0],cp->XYZ[1],cp->XYZ[2]);
			//fprintf(fid,"\t %3i", cp->SPR);
		}
	}

	if (VERBOSE>3) {
		/* channel settings */
		fprintf(fid,"\n\n[EVENT TABLE] N=%i Fs=%f",hdr->EVENT.N,hdr->EVENT.SampleRate);
		fprintf(fid,"\n#No\tTYP\tPOS\tDUR\tCHN\tVAL\tDesc");

		size_t k;
		for (k=0; k<hdr->EVENT.N; k++) {
			fprintf(fid,"\n%5i\t0x%04x\t%d",k+1,hdr->EVENT.TYP[k],hdr->EVENT.POS[k]);
			if (hdr->EVENT.DUR != NULL)
				fprintf(fid,"\t%5d\t%d",hdr->EVENT.DUR[k],hdr->EVENT.CHN[k]);

			if ((hdr->EVENT.TYP[k] == 0x7fff) && (hdr->TYPE==GDF))
				fprintf(fid,"\t[neds]");
			else if (hdr->EVENT.TYP[k] < hdr->EVENT.LenCodeDesc)
				fprintf(fid,"\t\t%s",hdr->EVENT.CodeDesc[hdr->EVENT.TYP[k]]);
			else if (GLOBAL_EVENTCODES_ISLOADED) {
				uint16_t k1;
				for (k1=0; (k1 < Global.LenCodeDesc) && (hdr->EVENT.TYP[k] != Global.CodeIndex[k1]); k1++) {};
				if (k1 < Global.LenCodeDesc)
					fprintf(fid,"\t\t%s",Global.CodeDesc[k1]);
			}
		}
	}

	if (VERBOSE>4) {
		if (hdr->aECG && (hdr->TYPE==SCP_ECG)) {
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
*/
			fprintf(stdout,"ACQ_DEV_MANUF        : %s\n",aECG->Section1.Tag14.ACQ_DEV_MANUF);
			fprintf(stdout,"Compression  HUFFMAN : %i\n",aECG->FLAG.HUFFMAN);
			fprintf(stdout,"Compression  REF-BEAT: %i\n",aECG->FLAG.REF_BEAT);
			fprintf(stdout,"Compression  BIMODAL : %i\n",aECG->FLAG.BIMODAL);
			fprintf(stdout,"Compression  DIFF    : %i\n",aECG->FLAG.DIFF);
			if ((aECG->systolicBloodPressure > 0.0) || (aECG->diastolicBloodPressure > 0.0))
				fprintf(stdout,"Blood pressure (systolic/diastolic) : %3.0f/%3.0f mmHg\n",aECG->systolicBloodPressure,aECG->diastolicBloodPressure);


			const char* StatusString;
			switch (aECG->Section8.Confirmed) {
			case 0: StatusString = "Original (not overread)"; break;
			case 1: StatusString = "Confirmed"; break;
			case 2: StatusString = "Overread (not confirmed)"; break;
			default: StatusString = "unknown"; break;
			}

			uint8_t k;
			if (aECG->Section8.NumberOfStatements>0) {
				fprintf(stdout,"\n\nReport %04i-%02i-%02i %02ih%02im%02is (Status=%s)\n",aECG->Section8.t.tm_year+1900,aECG->Section8.t.tm_mon+1,aECG->Section8.t.tm_mday,aECG->Section8.t.tm_hour,aECG->Section8.t.tm_min,aECG->Section8.t.tm_sec,StatusString);
				for (k=0; k<aECG->Section8.NumberOfStatements;k++) {
					fprintf(stdout,"%s\n",aECG->Section8.Statements[k]);
				}
			}

			if (aECG->Section11.NumberOfStatements>0) {
				fprintf(stdout,"\n\nReport %04i-%02i-%02i %02ih%02im%02is (Status=%s)\n",aECG->Section11.t.tm_year+1900,aECG->Section11.t.tm_mon+1,aECG->Section11.t.tm_mday,aECG->Section11.t.tm_hour,aECG->Section11.t.tm_min,aECG->Section11.t.tm_sec,StatusString);
				for (k=0; k<aECG->Section11.NumberOfStatements;k++) {
					fprintf(stdout,"%s\n",aECG->Section11.Statements[k]);
				}
			}

			fprintf(stdout,"\n\nSection9:\n%s\n\n",aECG->Section9.StartPtr);
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

