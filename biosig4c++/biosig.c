/*

    $Id: biosig.c,v 1.61 2007-06-22 21:02:29 schloegl Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//#include <libxml/xmlreader.h>


#include "biosig.h"
//#include "zlib.h"


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
	};


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

double l_endian_f64(double x) 
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
#endif


/* physical units are defined in 
 prEN ISO 11073-10101 (Nov 2003)
 Health Informatics - Point-of-care medical device communications - Part 10101:Nomenclature
 (ISO/DIS 11073-10101:2003)
 Table A.6.1: Table of Decimal Factors

 CEN/TC251/PT40 2001	
 File Exchange Format for Vital Signs - Annex A 
 Table A.4.1: Table of Decimal Factors	const double scale[32] =
*/

char* PhysDimTable[4096];   
void InitPhysDimTable()
{
	/* 
		Initializes PhysDimTable (must not be modified by any other instance)	
	*/	

	/* 	needs to run only once	*/	
	static int FLAG = 0; 
	if (FLAG) return; 	


	/* load table from file units.csv */
# define SIZE_OF_UNITS_FILE (11082)
	FILE* fid; 
	long int TableLen=SIZE_OF_UNITS_FILE;
	char Table[SIZE_OF_UNITS_FILE+1];
	char *line, *tok1, *tok2;
	int k; 	

	for (k=0; k<4096; PhysDimTable[k++]="\0");
	fid = fopen("units.csv","r");
	if (fid==NULL)
		fprintf(stderr,"Error: units.csv file not found.\n");	

	// get file size
	fseek(fid,0,SEEK_END);
	TableLen = ftell(fid);
	if (TableLen>SIZE_OF_UNITS_FILE)
		fprintf(stderr,"Warning: allocated TABLE_SIZE too small (%li %i).\n",TableLen,SIZE_OF_UNITS_FILE);
	
	// read file
	fseek(fid,0,SEEK_SET);
	TableLen = fread(Table, sizeof(char), min(TableLen,SIZE_OF_UNITS_FILE), fid);
	Table[TableLen]=0;
	if (TableLen!=SIZE_OF_UNITS_FILE)
		fprintf(stderr,"Warning: units.csv was modified.\n");
	fclose(fid);

	// parse file units.csv
	line = strtok(Table,"\n\r");	// read 1st line 
	while (line!=NULL)
	{
		if (line[0]==0) ;
		else if (line[0]=='#') ;
		else if (line[0]==',') ;
		else {
			tok1= line;
			for (k=0; line[k]!=','; k++); // find comma ,
			line[k]=0; 
			while (line[k]!='\"') k++;	// find 1st double quote "
			tok2 = line+k+1;
			for (k++; line[k]!='\"'; k++);	// find 2nd double quote "
			line[k]=0; 
			PhysDimTable[atoi(tok1)>>5] = tok2;
		}	
		line = strtok(NULL,"\n\r");	// read next line
	}
	FLAG = 1; 
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

char* PhysDim(uint16_t PhysDimCode, char* PhysDim)
{	
// converting PhysDimCode -> PhysDim
	PhysDim = strcpy(PhysDim,PhysDimFactor[PhysDimCode & 0x1f]);
	PhysDim = strcat(PhysDim,PhysDimTable[PhysDimCode>>5]);
	return(PhysDim);

}

uint16_t PhysDimCode(char* PhysDim0)
{	
// converting PhysDim -> PhysDimCode
	/* converts Physical dimension into 16 bit code */
	if (PhysDim0==NULL) return(0);
	if (!strlen(PhysDim0)) return(0);
	
	uint16_t Code, k1, k2;
	char s[80];
	
	// greedy search - check all codes 0..65535
	for (k1=0; k1<32;   k1++)
	if (PhysDimScale(k1)>0.0)  // exclude NaN
	for (k2=0; k2<4096; k2++)
	{
		Code = k2<<5+k1;
		if (strcmp(PhysDim0, PhysDim(Code,s))) return(Code); 
	}
	return(0);
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
      	hdr->ID.Equipment = *(uint64_t*)&"b4c_0.35";

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
	      	hdr->CHANNEL[k].PhysDim   = "uV";	// ### OBSOLETE ###
	      	hdr->CHANNEL[k].PhysDimCode = 19+4256; // uV
	      	hdr->CHANNEL[k].PhysMax   = +100;
	      	hdr->CHANNEL[k].PhysMin   = -100;
	      	hdr->CHANNEL[k].DigMax    = +2047;
	      	hdr->CHANNEL[k].DigMin    = -2048;
	      	hdr->CHANNEL[k].GDFTYP    = 3;	// int16 	
	      	hdr->CHANNEL[k].SPR       = 1;	// one sample per block
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

	InitPhysDimTable();
	if (hdr==NULL)
	hdr = create_default_hdr(0,0);	// initializes fields that may stay undefined during SOPEN 

if (!strcmp(MODE,"r"))	
{
#undef ZLIB_H   

#ifdef ZLIB_H
	hdr->FILE.FID = fzopen(FileName,"rb");
#else
	hdr->FILE.FID = fopen(FileName,"rb");
#endif

	hdr->FileName = FileName; 
    	if (hdr->FILE.FID == NULL) 
    	{ 	
    		fprintf(stderr,"Error SOPEN(READ); Cannot open file %s\n",FileName);
    		free(hdr);    		
		return(NULL);
    	}	    
    
    	/******** read 1st (fixed)  header  *******/	
 	Header1 = (char*)malloc(257);
#ifdef ZLIB_H
    	count   = gzread(hdr->FILE.FID,Header1,256);
#else
    	count   = fread(Header1,1,256,hdr->FILE.FID);
    	Header1[256]=0;
#endif
	
	hdr->TYPE = unknown; 

    	if (hdr->TYPE != unknown); 
    	else if (l_endian_u32((*(uint32_t*)(Header1+2)>=30)) & l_endian_u32((*(uint32_t*)(Header1+2))<=42)) {
    		hdr->VERSION    = l_endian_u32(*(uint32_t*)(Header1+2)); 
    		uint32_t offset = l_endian_u32(*(uint32_t*)(Header1+6));
    		hdr->HeadLen = offset; // length of fixed header  
    		if      ((hdr->VERSION <34.0) & (offset== 150)) hdr->TYPE = ACQ;  
    		else if ((hdr->VERSION <35.0) & (offset== 164)) hdr->TYPE = ACQ;  
    		else if ((hdr->VERSION <36.0) & (offset== 326)) hdr->TYPE = ACQ;  
    		else if ((hdr->VERSION <37.0) & (offset== 886)) hdr->TYPE = ACQ;  
    		else if ((hdr->VERSION <38.0) & (offset==1894)) hdr->TYPE = ACQ;  
    		else if ((hdr->VERSION <41.0) & (offset==1896)) hdr->TYPE = ACQ;  
    		else if ((hdr->VERSION>=41.0) & (offset==1944)) hdr->TYPE = ACQ;
    	}	

    	if (hdr->TYPE != unknown); 
    	else if (!memcmp(Header1+1,"BIOSEMI",7)) {
    		hdr->TYPE = BDF;
    		hdr->VERSION = -1; 
    	}	
    	else if ((Header1[0]==(char)207) & (!Header1[1]) & (!Header1[154]) & (!Header1[155]))
	    	hdr->TYPE = BKR;
    	else if (!memcmp(Header1,"CFWB\1\0\0\0",8))
	    	hdr->TYPE = CFWB;
    	else if (!memcmp(Header1,"Version 3.0",11))
	    	hdr->TYPE = CNT;
    	else if (!memcmp(Header1,"DEMG",4))
	    	hdr->TYPE = DEMG;
    	else if (!memcmp(Header1,"0       ",8)) {
	    	hdr->TYPE = EDF;
	    	hdr->VERSION = 0; 
	}    	
    	else if (!memcmp(Header1,"fLaC",4))
	    	hdr->TYPE = FLAC;
    	else if (!memcmp(Header1,"GDF",3))
	    	hdr->TYPE = GDF; 
    	else if (!memcmp(Header1,"@  MFER ",8))
	    	hdr->TYPE = MFER;
    	else if (!memcmp(Header1,"@ MFR ",6))
	    	hdr->TYPE = MFER;
    	else if (!memcmp(Header1,"NEX1",4))
	    	hdr->TYPE = NEX1;
    	else if (!memcmp(Header1,"PLEX",4))
	    	hdr->TYPE = PLEXON;
    	else if (!memcmp(Header1+16,"SCPECG",6))
	    	hdr->TYPE = SCP_ECG;
	else if (!memcmp(Header1,"<?xml version",13))   //Elias
		hdr->TYPE = HL7aECG;                       //Elias
    	else {
		fclose(hdr->FILE.FID); 
		free(Header1);


fprintf(stdout,"SOPEN(READ); File %s is of TYPE %i %s\n",FileName,hdr->TYPE,(char*)(Header1+16));

#ifdef __XML_XMLREADER_H__
		LIBXML_TEST_VERSION
    		reader = xmlReaderForFile(hdr->FileName, NULL, XML_PARSE_DTDATTR | XML_PARSE_NOENT); 
    			// XML_PARSE_DTDVALID cannot be used in aECG 

		if (reader != NULL) {
		        ret = xmlTextReaderRead(reader);
        		while (ret == 1) {
//            			processNode(reader);
            			ret = xmlTextReaderRead(reader);
        		}
        		if (ret != 0) {
            			fprintf(stderr, "%s : failed to parse\n", hdr->FileName);
        		}	
        	/*
		 * Once the document has been fully parsed check the validation results
		 */
			if (xmlTextReaderIsValid(reader) != 1) {
	    			fprintf(stderr, "Document %s does not validate\n", hdr->FileName);
			}
		        xmlFreeTextReader(reader);
			hdr->TYPE = XML; 
		}        
		else
#endif
		{
			hdr->TYPE = unknown; 
	//		sclose(hdr); 
	//		free(hdr); 
		}	
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
	    	count   = fread(Header2, 1, hdr->HeadLen-256, hdr->FILE.FID);
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
				strncpy(tmp, Header2 + 96*hdr->NS + 8*k, 8);
				hdr->CHANNEL[k].PhysDimCode = PhysDimCode(tmp);
				/*
				hdr->CHANNEL[k].PhysDim = (Header2 + 96*hdr->NS + 8*k);  // ### OBSOLETE ###
				/ ###FIXME###
				hdr->CHANNEL[k].PreFilt = (hdr->Header2+ 68*k + 136*hdr->NS);
				*/

				hdr->CHANNEL[k].DigMin   = (double) l_endian_i64( *(int64_t*)(Header2+ 8*k + 120*hdr->NS) );
				hdr->CHANNEL[k].DigMax   = (double) l_endian_i64( *(int64_t*)(Header2+ 8*k + 128*hdr->NS) );
			}	
			else {
				hdr->CHANNEL[k].PhysDimCode = l_endian_u16( *(uint16_t*)(Header2+ 2*k + 102*hdr->NS) );
				// ###FIXME### 
				// hdr->CHANNEL[k].PhysDim  = PhysDim(hdr->CHANNEL[k].PhysDimCode,hdr->CHANNEL[k].PhysDim);

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
		fseek(hdr->FILE.FID, hdr->HeadLen + hdr->AS.bpb*hdr->NRec, SEEK_SET); 
		fread(buf, sizeof(uint8_t), 8, hdr->FILE.FID);

		if (hdr->VERSION < 1.94) {
			hdr->EVENT.SampleRate = (float)buf[1] + (buf[2] + buf[3]*256.0)*256.0; 
			hdr->EVENT.N = l_endian_u32( *(uint32_t*) (buf + 4) );
		}	
		else	{
			hdr->EVENT.N = buf[1] + (buf[2] + buf[3]*256)*256; 
			hdr->EVENT.SampleRate = l_endian_f32( *(float*) (buf + 4) );
		}	
		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, hdr->EVENT.N*sizeof(*hdr->EVENT.POS) );
		hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP) );
		fread(hdr->EVENT.POS, sizeof(*hdr->EVENT.POS), hdr->EVENT.N, hdr->FILE.FID);
		fread(hdr->EVENT.TYP, sizeof(*hdr->EVENT.TYP), hdr->EVENT.N, hdr->FILE.FID);
		for (k32u=0; k32u < hdr->EVENT.N; k32u++) {
			hdr->EVENT.POS[k32u] = l_endian_u32(hdr->EVENT.POS[k32u]); 
			hdr->EVENT.TYP[k32u] = l_endian_u16(hdr->EVENT.TYP[k32u]); 
		}
		if (tmp[0]>1) {
			hdr->EVENT.DUR = (uint32_t*) realloc(hdr->EVENT.DUR,hdr->EVENT.N*sizeof(*hdr->EVENT.DUR));
			hdr->EVENT.CHN = (uint16_t*) realloc(hdr->EVENT.CHN,hdr->EVENT.N*sizeof(*hdr->EVENT.CHN));
			fread(hdr->EVENT.CHN,sizeof(*hdr->EVENT.CHN),hdr->EVENT.N,hdr->FILE.FID);
			fread(hdr->EVENT.DUR,sizeof(*hdr->EVENT.DUR),hdr->EVENT.N,hdr->FILE.FID);
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
	    	count   = fread(Header2, 1, hdr->HeadLen-256, hdr->FILE.FID);
		for (k=0; k<hdr->NS; k++)	{
			hdr->CHANNEL[k].Label   = (Header2 + 16*k);
			hdr->CHANNEL[k].Label[15]=0;//   hack
			hdr->CHANNEL[k].Transducer  = (Header2 + 80*k + 16*hdr->NS);
			hdr->CHANNEL[k].Transducer[79]=0;//   hack
			
			/* OBSOLETE 
			hdr->CHANNEL[k].PhysDim = (Header2 + 8*k + 96*hdr->NS);
			hdr->CHANNEL[k].PhysDim[7]=0; //hack
			*/
			// PhysDim -> PhysDimCode belongs here 
			strncpy(tmp,Header2 + 8*k + 96*hdr->NS,8);
			hdr->CHANNEL[k].PhysDimCode = PhysDimCode(tmp);
			
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
	    	count   = fread(Header1+256, 1, hdr->HeadLen-256, hdr->FILE.FID);
		uint32_t POS = hdr->HeadLen; 
	    	
		// read "foreign data section" and "per channel data types section"  
		hdr->HeadLen += l_endian_u16(*(uint16_t*)(Header1+hdr->HeadLen-4));
		hdr->HeadLen += 4*hdr->NS; 
	    	Header1 = (char*)realloc(Header1,hdr->HeadLen+8);
	    	count   = fread(Header1+POS, 1, hdr->HeadLen-POS, hdr->FILE.FID);
		
		// define channel specific header information
		hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
		uint32_t* ACQ_NoSamples = (uint32_t*) calloc(hdr->NS,sizeof(uint32_t));
		uint16_t CHAN; 
    		POS = l_endian_u32(*(uint32_t*)(Header1+6));
		for (k = 0; k < hdr->NS; k++)	{
			CHAN = l_endian_u16(*(uint16_t*)(Header1+POS+4));
			hdr->CHANNEL[k].Label   = (char*)(Header1+POS+6);
			hdr->CHANNEL[k].Label[39]   = 0;  
			strncpy(tmp,Header1+POS+68,20);
			hdr->CHANNEL[k].PhysDimCode = PhysDimCode(tmp);
			// PhysDim is OBSOLETE  
			hdr->CHANNEL[k].PhysDim = (char*)(Header1+POS+68);
			hdr->CHANNEL[k].PhysDim[19] = 0;
			
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
		
		fseek(hdr->FILE.FID,hdr->HeadLen+DataLen,-1); // start of markers header section
	    	POS     = hdr->HeadLen; 
	    	count   = fread(Header1+POS, 1, 8, hdr->FILE.FID);
	    	size_t LengthMarkerItemSection = (l_endian_u32(*(uint32_t*)(Header1+POS)));

	    	hdr->EVENT.N = (l_endian_u32(*(uint32_t*)(Header1+POS+4)));
	    	Header1 = (char*)realloc(Header1,hdr->HeadLen+8+LengthMarkerItemSection);
	    	POS    += 8; 
	    	count   = fread(Header1+POS, 1, LengthMarkerItemSection, hdr->FILE.FID);
		 
		hdr->EVENT.TYP = (uint16_t*)calloc(hdr->EVENT.N,2); 
		hdr->EVENT.POS = (uint32_t*)calloc(hdr->EVENT.N,4);
		for (k=0; k<hdr->EVENT.N; k++)
		{
fprintf(stdout,"ACQ EVENT: %i POS: %i\n",k,POS);		
			hdr->EVENT.POS[k] = l_endian_u32(*(uint32_t*)(Header1+POS));
			POS += 12 + l_endian_u16(*(uint16_t*)(Header1+POS+10));
		} 
*/
		fseek(hdr->FILE.FID,hdr->HeadLen,-1); 
	}      	

	else if (hdr->TYPE==BKR) {
	    	Header1 = (char*) realloc(Header1,1024);
	    	count   = fread(Header1+256,1,1024-256,hdr->FILE.FID);
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
	    	*(uint64_t*)(Header1+8) = l_endian_u64(*(uint64_t*)Header1+8);
	    	hdr->SampleRate = *(double*) (Header1+8);
	    	tm_time.tm_year = l_endian_u32( *(int32_t*)(Header1+16) );
	    	tm_time.tm_mon  = l_endian_u32( *(int32_t*)(Header1+20) );
	    	tm_time.tm_mday = l_endian_u32( *(int32_t*)(Header1+24) );
	    	tm_time.tm_hour = l_endian_u32( *(int32_t*)(Header1+28) );
	    	tm_time.tm_min  = l_endian_u32( *(int32_t*)(Header1+32) );
	    	*(uint64_t*)(Header1+36) = l_endian_u64(*(uint64_t*)(Header1+36));
	    	tm_time.tm_sec  = (int)*(double*) (Header1+36);
    		hdr->T0 	= tm_time2gdf_time(&tm_time);
	    	// = *(double*)(Header1+44);
	    	hdr->NS   	= l_endian_u32( *(int32_t*)(Header1+52) );
	    	hdr->NRec	= l_endian_u32( *(int32_t*)(Header1+56) );
	    	//  	= *(int32_t*)(Header1+60);	// TimeChannel
	    	//  	= *(int32_t*)(Header1+64);	// DataFormat

	    	Header1 = (char*) realloc(Header1,68+96*hdr->NS);
	    	Header2 = Header1+96; 
	    	hdr->HeadLen = 68+96*hdr->NS; 
	    	fseek(hdr->FILE.FID,68,SEEK_SET);
		count   = fread(Header2,1,96*hdr->NS,hdr->FILE.FID);
	    	
	    	hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS; k++)	{
		    	hdr->CHANNEL[k].GDFTYP 	= CFWB_GDFTYP[l_endian_u32(*(uint32_t*)(Header1+64))-1];
		    	hdr->CHANNEL[k].SPR 	= 1; // *(int32_t*)(Header1+56);
		    	hdr->CHANNEL[k].Label	= Header2+k*96;
		    	hdr->CHANNEL[k].PhysDim	= Header2+k*96+32;  // OBSOLETE
			hdr->CHANNEL[k].PhysDimCode = PhysDimCode(Header2+k*96+32);
		    	hdr->CHANNEL[k].Cal	= l_endian_f64(*(double*)(Header2+k*96+64));
		    	hdr->CHANNEL[k].Off	= l_endian_f64(*(double*)(Header2+k*96+72));
		    	hdr->CHANNEL[k].PhysMax	= l_endian_f64(*(double*)(Header2+k*96+80));
		    	hdr->CHANNEL[k].PhysMin	= l_endian_f64(*(double*)(Header2+k*96+88));
			hdr->CHANNEL[k].OnOff    = 1;
		}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// CFWB does not support automated overflow and saturation detection
	}
	else if (hdr->TYPE==CNT) {
	    	Header1 = (char*) realloc(Header1,900);
	    	hdr->VERSION = atof(Header1+8);
	    	count   = fread(Header1+256,1,900-256,hdr->FILE.FID);
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
	    	count   = fread(Header2,1,hdr->NS*75,hdr->FILE.FID);

	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS;k++)	{
		    	hdr->CHANNEL[k].GDFTYP 	= 3;
		    	hdr->CHANNEL[k].SPR 	= 1; // *(int32_t*)(Header1+56);
		    	hdr->CHANNEL[k].Label	= Header2+k*75;
		    	hdr->CHANNEL[k].PhysDim	= "uV";	// OBSOLETE
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
	    	fseek(hdr->FILE.FID,_eventtablepos,SEEK_SET);
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// automated overflow and saturation detection not supported
	}
	
	else if (hdr->TYPE==SCP_ECG) {
		hdr->AS.Header1 = (uint8_t*)Header1; 
		hdr->HeadLen 	= l_endian_u32(*(uint32_t*)(Header1+2));
		hdr->AS.Header1 = (uint8_t*)realloc(hdr->AS.Header1,hdr->HeadLen);
	    	count 	+= fread(hdr->AS.Header1+count, 1, hdr->HeadLen-count, hdr->FILE.FID);
	    	uint16_t crc 	= CRCEvaluate(hdr->AS.Header1+2,hdr->HeadLen-2);

	    	if ( l_endian_u16(*(uint16_t*)hdr->AS.Header1) != crc)
	    	{
	    		fprintf(stderr,"Warning SOPEN(SCP-READ): Bad CRC %x %x !\n",crc,*(uint16_t*)(hdr->AS.Header1));
	    	}
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
	
	else // if (hdr->TYPE==unknown) 
	{
		fprintf(stdout,"unknown file format %s \n",hdr->FileName);
	}
	hdr->AS.Header1 = (uint8_t*)Header1; 
	
}
else { // WRITE

    	if (hdr->TYPE==GDF) {	
	     	hdr->HeadLen = (hdr->NS+1)*256;
	    	Header1 = (char*) malloc(hdr->HeadLen);
	    	Header2 = Header1+256; 

		memset(Header1,0,hdr->HeadLen);
		hdr->VERSION = 1.94;
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
		for (k=0;k<hdr->NS;k++)
		{
		     	len = strlen(hdr->CHANNEL[k].Label);
		     	memcpy(Header2+16*k,hdr->CHANNEL[k].Label,min(len,16));
		     	len = strlen(hdr->CHANNEL[k].Transducer);

		     	memcpy(Header2+80*k + 16*hdr->NS, hdr->CHANNEL[k].Transducer, min(len,80));
		     	PhysDim(hdr->CHANNEL[k].PhysDimCode, tmp);
		     	len = strlen(tmp);
		     	if (hdr->VERSION<1.9)
		     		memcpy(Header2+ 8*k + 96*hdr->NS, tmp, min(len,8));
		     	else {
		     		memcpy(Header2+ 6*k + 96*hdr->NS, tmp, min(len,6));
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
		if (hdr->TYPE==BDF) {	
			Header1[0] = 255;
	     		memcpy(Header1+1,"BIOSEMI",7);
		}
		else {
			memset(Header1,' ',hdr->HeadLen);
	     		memcpy(Header1,"0       ",8);
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
		     	len = strlen(hdr->CHANNEL[k].Label);
			if (len>16) fprintf(stderr,"Warning: Label (%s) of channel %i is to long.\n",hdr->CHANNEL[k].Label,k);  
		     	memcpy(Header2+16*k,hdr->CHANNEL[k].Label,min(len,16));
		     	len = strlen(hdr->CHANNEL[k].Transducer);
			if (len>80) fprintf(stderr,"Warning: Transducer (%s) of channel %i is to long.\n",hdr->CHANNEL[k].Transducer,k);  
		     	memcpy(Header2+80*k + 16*hdr->NS,hdr->CHANNEL[k].Transducer,min(len,80));
		     	PhysDim(hdr->CHANNEL[k].PhysDimCode, tmp);
		     	len = strlen(tmp);
fprintf(stdout,"#%02i: PhysDim=%s\n",k,tmp);		     	
			if (len>8) fprintf(stderr,"Warning: Physical Dimension (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 96*hdr->NS,tmp,min(len,8));
	
			len = sprintf(tmp,"%f",hdr->CHANNEL[k].PhysMin);
			if (len>8) fprintf(stderr,"Warning: PhysMin (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 104*hdr->NS,tmp,min(8,len));
			len = sprintf(tmp,"%f",hdr->CHANNEL[k].PhysMax);
			if (len>8) fprintf(stderr,"Warning: PhysMax (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 112*hdr->NS,tmp,min(8,len));
			len = sprintf(tmp,"%f",hdr->CHANNEL[k].DigMin);
			if (len>8) fprintf(stderr,"Warning: DigMin (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 120*hdr->NS,tmp,min(8,len));
			len = sprintf(tmp,"%f",hdr->CHANNEL[k].DigMax);
			if (len>8) fprintf(stderr,"Warning: DigMax (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 128*hdr->NS,tmp,min(8,len));
		     	
			if (hdr->CHANNEL[k].Notch>0)		     	
				len = sprintf(tmp,"HP:%fHz LP:%fHz Notch:%fHz",hdr->CHANNEL[k].HighPass,hdr->CHANNEL[k].LowPass,hdr->CHANNEL[k].Notch);
			else
				len = sprintf(tmp,"HP:%fHz LP:%fHz",hdr->CHANNEL[k].HighPass,hdr->CHANNEL[k].LowPass);
		     	memcpy(Header2+ 80*k + 136*hdr->NS,tmp,min(80,len));
		     	
			len = sprintf(tmp,"%i",hdr->CHANNEL[k].SPR);
			if (len>8) fprintf(stderr,"Warning: SPR (%s) of channel %i is to long.\n",tmp,k);  
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
    		hdr = sopen_HL7aECG_write(hdr);
	}
	else {
    	 	fprintf(stderr,"ERROR: Writing of format (%c) not supported\n",hdr->TYPE);
		return(NULL); 
	}

	if(hdr->TYPE != HL7aECG){
	    	hdr->FILE.FID = fopen(FileName,"wb");
	    	hdr->FileName = FileName; 
		if (hdr->FILE.FID == NULL){
		     	fprintf(stderr,"ERROR: Unable to open file %s \n",FileName);
		return(NULL);
		}
		fwrite(hdr->AS.Header1,sizeof(char),hdr->HeadLen,hdr->FILE.FID);
		hdr->FILE.OPEN = 2;
		hdr->FILE.POS  = 0;
	}

}	// end of else 

	// internal variables
	hdr->AS.bi = (uint32_t*) realloc(hdr->AS.bi,(hdr->NS+1)*sizeof(uint32_t));
	hdr->AS.bi[0] = 0;
	for (k=0, hdr->SPR = 1, hdr->AS.spb=0, hdr->AS.bpb=0; k<hdr->NS;k++) {
		hdr->AS.spb += hdr->CHANNEL[k].SPR;
		hdr->AS.bpb += GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP]*hdr->CHANNEL[k].SPR;			
		hdr->SPR = lcm(hdr->SPR,hdr->CHANNEL[k].SPR);
		hdr->AS.bi[k+1] = hdr->AS.bpb; 
//fprintf(stdout,"-: %i %i %i\n",hdr->AS.bi[k],hdr->CHANNEL[k].GDFTYP,GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP]);		
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
	

	// check reading segment 
	if (start >= 0) {
		if (start > hdr->NRec)
			return(0);
		else if (fseek(hdr->FILE.FID, start*hdr->AS.bpb + hdr->HeadLen, SEEK_SET))
			return(0);
		hdr->FILE.POS = start; 	
	}

	if (hdr->TYPE != SCP_ECG && hdr->TYPE != HL7aECG) {	
		// allocate AS.rawdata 	
		hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata, (hdr->AS.bpb)*length);

		// limit reading to end of data block
		nelem = max(min(length, hdr->NRec - hdr->FILE.POS),0);

		// read data	
		count = fread(hdr->AS.rawdata, hdr->AS.bpb, nelem, hdr->FILE.FID);
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
	hdr->data.block   = (biosig_data_type*) realloc(hdr->data.block, (hdr->SPR) * count * (hdr->NS) * sizeof(biosig_data_type));

	for (k1=0,k2=0; k1<hdr->NS; k1++) {
		CHptr 	= hdr->CHANNEL+k1;
	if (1) //(CHptr->OnOff != 0) 
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
		else if (fseek(hdr->FILE.FID, start*hdr->AS.bpb + hdr->HeadLen, SEEK_SET))
			return(0);
		hdr->FILE.POS = start; 	
	}

	if (hdr->TYPE != SCP_ECG) {	
		// allocate AS.rawdata 	
		hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata, (hdr->AS.bpb)*length);

		// limit reading to end of data block
		nelem = max(min(length, hdr->NRec - hdr->FILE.POS),0);

		// read data	
		count = fread(hdr->AS.rawdata, hdr->AS.bpb, nelem, hdr->FILE.FID);
		if (count<nelem)
			fprintf(stderr,"warning: only %i instead of %i blocks read - something went wrong\n",count,nelem); 
	}
	else 	{  // SCP format 
		// hdr->AS.rawdata was defined in SOPEN	
		count = hdr->NRec;
	}
	
	// allocate AS.rawdata 	
	hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata, (hdr->AS.bpb)*length);

	// limit reading to end of data block
	nelem = max(min(length, hdr->NRec - hdr->FILE.POS),0);

	// read data	
	count = fread(hdr->AS.rawdata, hdr->AS.bpb, nelem, hdr->FILE.FID);

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
size_t swrite(const void *ptr, size_t nelem, HDRTYPE* hdr) {
/* 
 *	writes NELEM blocks with HDR.AS.bpb BYTES each, 
 */
	size_t			count,k1,k2,k3,k4,k5,DIV,SZ; 
	int 			GDFTYP;
	CHANNEL_TYPE*		CHptr;
	biosig_data_type 	sample_value; 
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


//	fwrite(hdr->data.block, sizeof(biosig_data_type),hdr->NRec*hdr->SPR*hdr->NS, hdr->FILE.FID);

	// write data 

#ifdef FALSE
	count = fwrite((uint8_t*)ptr, hdr->AS.bpb, nelem, hdr->FILE.FID);

#else

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
#define MAX_INT64  (ldexp(1.0,63)-1.0)
#define MIN_INT64  (-ldexp(1.0,63))
#define MAX_UINT64 ((uint64_t)0xffffffffffffffff)
#define MIN_UINT64 ((uint64_t)0)

	hdr->AS.rawdata = (uint8_t*)realloc(hdr->AS.rawdata, hdr->AS.bpb*hdr->NRec);

	for (k1=0, k2=0; k1<hdr->NS; k1++) {

	CHptr 	= hdr->CHANNEL+k1;
	if (CHptr->OnOff != 0) {
		DIV 	= hdr->SPR/CHptr->SPR; 
		GDFTYP 	= CHptr->GDFTYP;
		SZ  	= GDFTYP_BYTE[GDFTYP];

		for (k4 = 0; k4 < hdr->NRec; k4++)
		{
//fprintf(stdout,"\n=4: %i %i %i %i %i %i %i %i\n",k1,k2,k4,hdr->AS.bpb,hdr->AS.bi[k1],hdr->AS.bi[k1+1],SZ,GDFTYP);

		for (k5 = 0; k5 < CHptr->SPR; k5++) {

//fprintf(stdout,"\n=5: %i %i %i 	\n",k1,k5,k4);

			for (k3=0, sample_value=0; k3 < DIV; k3++) 
				sample_value += hdr->data.block[k1*count*hdr->SPR + k4*CHptr->SPR + k5 + k3]; 

			sample_value /= DIV;
			 	
//fprintf(stdout,"\n=6: %i %i %i %i\n",k1,k5,k4,sample_value);

			if (!hdr->FLAG.UCAL)	// scaling 
				sample_value = (sample_value - CHptr->Off) / CHptr->Cal;

			// get source address 	
			ptr = hdr->AS.rawdata + k4*hdr->AS.bpb + hdr->AS.bi[k1] + k5*SZ;
			
//fprintf(stdout,"=3: %6i %i %i %i %i %i %i %i %i %i\n",k4*hdr->AS.bpb + hdr->AS.bi[k1] + k5*SZ,k1,k2,k3,k4,k5,hdr->NS,hdr->AS.bpb,k1*count*hdr->SPR + k4*CHptr->SPR + k5 + k3,k4*hdr->AS.bpb + hdr->AS.bi[k1] + k5*SZ);

			// mapping of raw data type to (biosig_data_type)
			if (0); 

			else if (GDFTYP==3) {
				if      (sample_value > MAX_INT16) val.i16 = MAX_INT16;
				else if (sample_value < MIN_INT16) val.i16 = MIN_INT16;
				else     val.i16 = (int16_t) sample_value;
				*(int16_t*)ptr = l_endian_i16(val.i16); 
//	fprintf(stdout,"5+: %i %i\n",*(int16_t*)ptr,*((int16_t*)ptr+1));				
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
				*(int32_t*)ptr = l_endian_i32(val.i32); 

//	fprintf(stdout,"5+: %i %i\n",*(int32_t*)ptr,*((int32_t*)ptr+1));				
			}
			else if (GDFTYP==6) {
				if      (sample_value > ldexp(1.0,32)-1.0) val.u32 = MAX_UINT32;
				else if (sample_value < 0.0) val.u32 = MIN_UINT32;
				else     val.u32 = (uint32_t) sample_value;
				*(uint32_t*)ptr = l_endian_u32(val.u32); 
			}

			else if (GDFTYP==7) {
				if      (sample_value > ldexp(1.0,63)-1.0) val.i64 = MAX_INT64;
				else if (sample_value < -ldexp(1.0,63)) val.i64 = MIN_INT64;
				else     val.i64 = (int64_t) sample_value;
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

fprintf(stdout,"=: %i %i %i \n",hdr->AS.bpb, hdr->NRec, GDFTYP);
for (int k=0;k<hdr->NS;k++)
fprintf(stdout," %5i %4i %4i %4i %4i \n",k,*(hdr->AS.rawdata+k*5000),*(hdr->AS.rawdata+k*5001),*(hdr->AS.rawdata+k*5002),*(hdr->AS.rawdata+k*5003));

	count = fwrite((uint8_t*)(hdr->AS.rawdata), hdr->AS.bpb, hdr->NRec, hdr->FILE.FID);

#endif

fprintf(stdout,"===222--------\n");
	
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
	long int pos=0; 
	
	if    	(whence < 0) 
		pos = offset * hdr->AS.bpb; 
	else if (whence == 0) 
		pos = (hdr->FILE.POS + offset) * hdr->AS.bpb;
	else if (whence > 0) 
		pos = (hdr->NRec + offset) * hdr->AS.bpb;
	
	if ((pos < 0) | (pos > hdr->NRec * hdr->AS.bpb))
		return(-1);
	else if (fseek(hdr->FILE.FID, pos + hdr->HeadLen, SEEK_SET))
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
	
	pos = ftell(hdr->FILE.FID);	
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
	
	if ((hdr->NRec<0) & (hdr->FILE.OPEN>1))
	if ((hdr->TYPE==GDF) | (hdr->TYPE==EDF) | (hdr->TYPE==BDF))
	{
		// WRITE HDR.NRec 
		pos = (ftell(hdr->FILE.FID)-hdr->HeadLen); 
		if (hdr->NRec<0)
		{	if (pos>0) 	hdr->NRec = pos/hdr->AS.bpb;
			else		hdr->NRec = 0; 	
			fseek(hdr->FILE.FID,236,SEEK_SET); 
			if (hdr->TYPE==GDF) {
				*(uint64_t*)tmp = l_endian_u64(hdr->NRec);
				fwrite(tmp,sizeof(hdr->NRec),1,hdr->FILE.FID);
			}	
			else {
				len = sprintf(tmp,"%Lu",hdr->NRec);
				if (len>8) fprintf(stderr,"Warning: NRec is (%s) to long.\n",tmp);  
				fwrite(tmp,len,1,hdr->FILE.FID);
			}	
		};	
	
		// WRITE EVENTTABLE 
		if ((hdr->TYPE==GDF) & (hdr->EVENT.N>0)) {
			fseek(hdr->FILE.FID, hdr->HeadLen + hdr->AS.bpb*hdr->NRec, SEEK_SET); 
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
			
			fwrite(buf, 8, 1, hdr->FILE.FID);
			for (k32u=0; k32u<hdr->EVENT.N; k32u++) {
				hdr->EVENT.POS[k32u] = l_endian_u32(hdr->EVENT.POS[k32u]); 
				hdr->EVENT.TYP[k32u] = l_endian_u16(hdr->EVENT.TYP[k32u]); 
			}
			fwrite(hdr->EVENT.POS, sizeof(*hdr->EVENT.POS), hdr->EVENT.N, hdr->FILE.FID);
			fwrite(hdr->EVENT.TYP, sizeof(*hdr->EVENT.TYP), hdr->EVENT.N, hdr->FILE.FID);
			if (tmp[0]>1) {
				for (k32u=0; k32u<hdr->EVENT.N; k32u++) {
					hdr->EVENT.DUR[k32u] = l_endian_u32(hdr->EVENT.DUR[k32u]); 
					hdr->EVENT.CHN[k32u] = l_endian_u16(hdr->EVENT.CHN[k32u]); 
				}
				fwrite(hdr->EVENT.CHN,sizeof(*hdr->EVENT.CHN),hdr->EVENT.N,hdr->FILE.FID);
				fwrite(hdr->EVENT.DUR,sizeof(*hdr->EVENT.DUR),hdr->EVENT.N,hdr->FILE.FID);
			}	
		}
	}		
// fprintf(stdout,"sclose: 01\n");
	if (hdr->TYPE != XML) {
		fclose(hdr->FILE.FID);
    		hdr->FILE.FID = 0;
    	}	

// fprintf(stdout,"sclose: 02\n");
    	if (hdr->aECG != NULL)	
        	free(hdr->aECG);
// fprintf(stdout,"sclose: 03\n");
    	if (hdr->AS.rawdata != NULL)	
        	free(hdr->AS.rawdata);
// fprintf(stdout,"sclose: 04\n");

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
        	
        hdr->EVENT.N   = 0; 
	hdr->FILE.OPEN = 0; 	     	

    	return(0);
}


/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/

// big-endian platforms
