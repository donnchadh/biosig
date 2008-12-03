/*

    $Id: sopen_scp_read.c,v 1.63 2008-12-03 11:18:29 schloegl Exp $
    Copyright (C) 2005,2006,2007 Alois Schloegl <a.schloegl@ieee.org>

    This file is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 


    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.


 */


// #define WITHOUT_SCP_DECODE    // use SCP-DECODE if needed, Bimodal, reference beat  

/*
	the experimental version needs a few more thinks: 
	- Bimodal and RefBeat decoding do not work yet

	- validation and testing  
*/


#include <stdio.h>             // system includes
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#include "../biosig-dev.h"

#include "structures.h"
static const U_int_S _NUM_SECTION=12U;	//consider first 11 sections of SCP
static bool add_filter=true;             // additional filtering gives better shape, but use with care

#ifndef WITHOUT_SCP_DECODE
int scp_decode(HDRTYPE*, pointer_section*, DATA_DECODE&, DATA_RECORD&, DATA_INFO&, bool&);
#endif

// Huffman Tables         	
uint16_t NHT; 	/* number of Huffman tables */
struct table_t {
	uint8_t PrefixLength;
       	uint8_t CodeLength;
       	uint8_t TableModeSwitch;
       	int16_t BaseValue;
       	uint32_t BaseCode;
}; 
struct huffman_t {
      	uint16_t NCT; 	/* number of Code structures in Table #1 */
       	table_t *Table; 
} *Huffman;

struct htree_t {
	htree_t* child0;
	htree_t* child1;
	uint16_t idxTable;
} **HTrees;

table_t DefaultTable[19] = {
	{ 1,  1, 1, 0, 0 },
	{ 3,  3, 1, 1, 1 },
	{ 3,  3, 1,-1, 5 },
	{ 4,  4, 1, 2, 3 },
	{ 4,  4, 1,-2, 11},
	{ 5,  5, 1, 3, 7 },
	{ 5,  5, 1,-3, 23},
	{ 6,  6, 1, 4, 15},
	{ 6,  6, 1,-4, 47},
	{ 7,  7, 1, 5, 31},
	{ 7,  7, 1,-5, 95},
	{ 8,  8, 1, 6, 63},
	{ 8,  8, 1,-6, 191},
	{ 9,  9, 1, 7, 127},
	{ 9,  9, 1,-7, 383},
	{10, 10, 1, 8, 255},
	{10, 10, 1,-8, 767},
	{18, 10, 1, 0, 511},
	{26, 10, 1, 0, 1023}
};

/*
	This structure defines the fields used for "Annotated ECG" 
 */
struct en1064_t {
	char*		test;		/* test field for annotated ECG */
	
	float		diastolicBloodPressure;				
	float		systolicBloodPressure;	
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
		uint8_t	DIFF;// OBSOLETE
		uint8_t	BIMODAL;// OBSOLETE
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
        } Section2;
        struct {
        	uint8_t NS, flags;
        	struct { 
        		uint32_t start;
        		uint32_t end;
//        		uint8_t  id;
        	} *lead;
        } Section3;
        struct {
        	uint16_t len_ms, fiducial_sample, N;
       		uint32_t SPR;
        	struct { 
        		uint16_t btyp;
        		uint32_t SB;
        		uint32_t fcM;
        		uint32_t SE;
        		uint32_t QB;
        		uint32_t QE;
        	} *beat;
        } Section4;
        struct {
        	size_t   StartPtr;
        	size_t	 Length;
        	uint16_t AVM, dT_us;
        	uint8_t  DIFF; //diff: see FLAG 
        	uint16_t *inlen;
        	int32_t  *datablock;
        } Section5;
        struct {
        	size_t   StartPtr;
        	size_t	 Length;
        	uint16_t AVM, dT_us;
        	uint8_t  DIFF, BIMODAL; //diff, bimodal: see FLAG 
        	uint16_t *inlen;
        	int32_t  *datablock;
        } Section6;
} en1064;

/* new node in Huffman tree */
htree_t* newNode() {
	htree_t* T  = (htree_t*) malloc(sizeof(htree_t));
	T->child0   = NULL;
	T->child1   = NULL;
	T->idxTable = 0; 
	return(T);
}

/* check Huffman tree */
int checkTree(htree_t *T) {
	int v,v1,v2,v3; 
	
	v1 = (T->child0 == NULL) && (T->child0 == NULL) && (T->idxTable > 0);
	v2 = (T->idxTable == 0) && (T->child0 != NULL) && checkTree(T->child0);
	v3 = (T->idxTable == 0) && (T->child1 != NULL) && checkTree(T->child1);
	v = v1 || v2 || v3;
	if (!v) fprintf(stderr,"Warning: Invalid Node in Huffman Tree: %i %p %p\n",T->idxTable,T->child0,T->child1);
	return(v);
}

/* convert Huffman Table into a Huffman tree */
htree_t* makeTree(huffman_t HT) {
	uint16_t k1,k2;
	htree_t* T = newNode();
	htree_t* node;
	for (k1=0; k1<HT.NCT; k1++) {
		node = T; 
		uint32_t bc = HT.Table[k1].BaseCode;
		for (k2=0; k2<HT.Table[k1].CodeLength; k2++, bc>>=1) {
			if (bc & 0x00000001) {
				if (node->child1==NULL) node->child1 = newNode();
				node = node->child1;
			}
			else {
				if (node->child0==NULL) node->child0 = newNode();
				node = node->child0;
			}
		}
		node->idxTable = k1+1;
	}
	return(T); 
}

/* get rid of Huffman tree */
void freeTree(htree_t* T) {
	if (T->child0 != NULL) freeTree(T->child0);
	if (T->child1 != NULL) freeTree(T->child1);
	free(T);
}

int DecodeHuffman(htree_t *HTrees[], huffman_t *HuffmanTables, uint8_t* indata, size_t inlen, int32_t* outdata, size_t outlen) {
	uint16_t ActualTable = 0; 
	htree_t *node; 
	size_t k1, k2, i;
	uint32_t acc; 
	int8_t dlen,k3,r;

	k1=0, k2=0;
	node = HTrees[ActualTable];
	r = 0; i = 0;
	while ((k1 < inlen*8) && (k2 < outlen)) {
		r = k1 % 8; 
		i = k1 / 8;

		if (!node->idxTable) {
			if (indata[i] & (1<<(7-r))) {
				if (node->child1 != NULL)
					node = node->child1;
				else {
					B4C_ERRMSG = "Empty node in Huffman table! Do not know what to do !\n";
					B4C_ERRNUM = B4C_DECOMPRESSION_FAILED;					
					return(-1);
				}	
			}	
			else {
				if (node->child0 != NULL)
					node = node->child0;
				else {
					B4C_ERRMSG = "Empty node in Huffman table! Do not know what to do !\n";
					B4C_ERRNUM = B4C_DECOMPRESSION_FAILED;					
					return(-1);
				}	
			}	
			++k1;
		}

		r = k1 % 8; 
		i = k1 / 8;

		if (node->idxTable) {
			// leaf of tree reached
			table_t TableEntry = HuffmanTables[ActualTable].Table[node->idxTable - 1]; 
			dlen = TableEntry.PrefixLength - TableEntry.CodeLength;
			if (!TableEntry.TableModeSwitch)
				// switch Huffman Code 
				ActualTable = TableEntry.BaseValue;
			else if (dlen) {
				// no compression
				acc = 0;  //(uint32_t)(indata[i]%(1<<r)); 
				for (k3=0; k3*8-r < dlen; k3++)
					acc = (acc<<8)+(uint32_t)indata[i+k3];
				
				outdata[k2] = (acc >> (k3*8 - r - dlen)) & ((1L << dlen) - 1L) ; 
				if (outdata[k2] >= (1<<dlen-1))
					outdata[k2] -= 1<<dlen;
				k1 += dlen; 
				++k2;
			}
			else {
				// lookup Huffman Table 
				outdata[k2++] = TableEntry.BaseValue; 
			}
			// reset node to root
			node = HTrees[ActualTable];
		}
	}
	return(0);
};

void deallocEN1064(en1064_t en1064) {	
	/* free allocated memory */ 
	if (en1064.FLAG.HUFFMAN) {
		for (size_t k1=0; k1<en1064.FLAG.HUFFMAN; k1++) {
		 	if (NHT!=19999) free(Huffman[k1].Table);
		 	freeTree(HTrees[k1]); 
		} 
		free(Huffman);
		free(HTrees); 
	}	

	if (en1064.Section3.lead != NULL) 	free(en1064.Section3.lead);
	if (en1064.Section4.beat != NULL) 	free(en1064.Section4.beat);
	if (en1064.Section5.inlen != NULL) 	free(en1064.Section5.inlen);
	if (en1064.Section5.datablock != NULL) 	free(en1064.Section5.datablock);
	if (en1064.Section6.inlen != NULL) 	free(en1064.Section6.inlen);
//	if (en1064.Section6.datablock != NULL) 	free(en1064.Section6.datablock);
}

int sopen_SCP_read(HDRTYPE* hdr) {	
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: 
		char* Header	// contains the file content
		
	Output: 
		HDRTYPE *hdr	// defines the HDR structure accoring to "biosig.h"
*/	

	uint8_t*	ptr; 	// pointer to memory mapping of the file layout
	uint8_t*	PtrCurSect;	// point to current section 
	uint8_t*	Ptr2datablock=NULL; 	// pointer to data block 
	int32_t* 	data;		// point to rawdata
	uint16_t	curSect=0; 	// current section
	uint32_t 	len; 
	uint16_t 	crc; 
	uint32_t	i,k1,k2; 
	size_t		curSectPos;
	size_t 		sectionStart; 
	int 		NSections = 12;
	uint8_t		tag;
	float 		HighPass=0, LowPass=1.0/0.0, Notch=-1; 	// filter settings
	uint16_t	Cal5=0, Cal6=0, Cal0;	// scaling coefficients 
	uint16_t 	dT_us = 1000; 	// sampling interval in microseconds

	/* 
	   Try direct conversion SCP->HDR to internal data structure
		+ whole data is loaded once, then no further File I/O is needed. 
		- currently Huffman and Bimodal compression is not supported. 
	*/	

	aECG_TYPE* aECG;
	if (hdr->aECG == NULL) {
		hdr->aECG = malloc(sizeof(aECG_TYPE));
		aECG = (aECG_TYPE*)hdr->aECG;
		aECG->diastolicBloodPressure=0.0;				 
		aECG->systolicBloodPressure=0.0;
		aECG->MedicationDrugs = "\x0";
		aECG->ReferringPhysician= "\x0";
		
		aECG->LatestConfirmingPhysician="\x00";
		aECG->Diagnosis="\x00";
		aECG->EmergencyLevel=0;
	}
	else 
		aECG = (aECG_TYPE*)hdr->aECG;

	
	aECG->Section1.Tag14.VERSION = 0; // acquiring.protocol_revision_number 
	aECG->Section1.Tag15.VERSION = 0; // analyzing.protocol_revision_number
	aECG->FLAG.HUFFMAN   = 0; 
	aECG->FLAG.DIFF      = 0; 
	aECG->FLAG.REF_BEAT  = 0; 
	aECG->FLAG.BIMODAL   = 0;
	aECG->Section8.NumberOfStatements = 0; 
	aECG->Section8.Statements = NULL; 
	aECG->Section11.NumberOfStatements = 0; 
	aECG->Section11.Statements = NULL; 

	en1064.FLAG.HUFFMAN  = 0; 
	en1064.FLAG.DIFF     = 0; 
	en1064.FLAG.REF_BEAT = 0; 
	en1064.FLAG.BIMODAL  = 0;
	en1064.Section4.len_ms	 = 0;
	
#ifndef WITHOUT_SCP_DECODE
	struct pointer_section section[_NUM_SECTION];
	struct DATA_DECODE decode;
	struct DATA_RECORD record;
	struct DATA_INFO textual;
	bool   AS_DECODE = 0; 

	decode.length_BdR0 = NULL; 	
	decode.samples_BdR0= NULL;
	decode.length_Res  = NULL;
	decode.samples_Res = NULL;
	decode.t_Huffman=NULL;
	decode.flag_Huffman=NULL;
	decode.data_lead=NULL;
	decode.data_protected=NULL;
	decode.data_subtraction=NULL;
	decode.length_BdR0=NULL;
	decode.samples_BdR0=NULL;
	decode.Median=NULL;
	decode.length_Res=NULL;
	decode.samples_Res=NULL;
	decode.Residual=NULL;
	decode.Reconstructed=NULL;

	//variables inizialization
	decode.flag_lead.number=0;
	decode.flag_lead.subtraction=0;
	decode.flag_lead.all_simultaneously=0;
	decode.flag_lead.number_simultaneously=0;

	decode.flag_BdR0.length=0;
	decode.flag_BdR0.fcM=0;
	decode.flag_BdR0.AVM=0;
	decode.flag_BdR0.STM=0;
	decode.flag_BdR0.number_samples=0;
	decode.flag_BdR0.encoding=0;

	decode.flag_Res.AVM=0;
	decode.flag_Res.STM=0;
	decode.flag_Res.number=0;
	decode.flag_Res.number_samples=0;
	decode.flag_Res.encoding=0;
	decode.flag_Res.bimodal=0;
	decode.flag_Res.decimation_factor=0;
#endif 
	
	ptr = hdr->AS.Header; 
	hdr->NRec = 0;

	sectionStart = 6;
	PtrCurSect = ptr+sectionStart;	

	/**** SECTION 0 ****/
	len = leu32p(PtrCurSect+4); 
	NSections = min((len-16)/10,_NUM_SECTION);

	section[0].ID	  = 0; 	
	section[0].length = len; 	
	section[0].index  = 6+16;
	for (int K=1; K<_NUM_SECTION; K++) {
		section[K].ID	  = K; 	
		section[K].length = 0; 	
		section[K].index  = 0;
	}

	for (int K=1; K<NSections; K++)	{
		// this is needed because fields are not always sorted
		curSect = leu32p(ptr+6+16+K*10); 
		if (curSect < NSections) {
			section[curSect].ID 	= curSect; 	
			section[curSect].length = leu32p(ptr+6+16+K*10+2); 	
			section[curSect].index  = leu32p(ptr+6+16+K*10+6)-1;
		}
	}

	for (int K=1; K<NSections; K++)	{

	if (VERBOSE_LEVEL>8)
		fprintf(stdout,"%s(%i) ",hdr->FileName,K);
		
		curSect           = section[K].ID;
		len		  = section[K].length;
		sectionStart 	  = section[K].index;

	if (VERBOSE_LEVEL>8)
		fprintf(stdout,"SCP Section %i %i len=%i secStart=%i HeaderLength=%i\n",K,curSect,len,sectionStart,hdr->HeadLen);

	if (len==0) continue;	 /***** empty section *****/
		PtrCurSect = ptr+sectionStart;
		crc 	   = leu16p(PtrCurSect);
		uint16_t tmpcrc = CRCEvaluate((uint8_t*)(PtrCurSect+2),len-2); 
		if ((crc != 0xffff) && (crc != tmpcrc))
			fprintf(stderr,"Warning SOPEN(SCP-READ): faulty CRC in section %i: crc=%x, %x\n" ,curSect,crc,tmpcrc);
		if (curSect != leu16p(PtrCurSect+2))
			fprintf(stderr,"Warning SOPEN(SCP-READ): Current Section No does not match field in sections (%i %i)\n",curSect,leu16p(PtrCurSect+2)); 
		if (len != leu32p(PtrCurSect+4))
			fprintf(stderr,"Warning SOPEN(SCP-READ): length field in pointer section (%i) does not match length field in sections (%i %i)\n",K,len,leu32p(PtrCurSect+4)); 

		uint8_t versionSection  = *(ptr+sectionStart+8);
		uint8_t versionProtocol = *(ptr+sectionStart+9);

		curSectPos = 16;

		/**** SECTION 0 ****/
		if (curSect==0)  
		{
		}		

		/**** SECTION 1 ****/
		else if (curSect==1)  
		{
			struct tm t0,t1;
			t0.tm_year = 0;
			t0.tm_mon  = 0;
			t0.tm_mday = 0;
			t0.tm_hour = 0;
			t0.tm_min  = 0;
			t0.tm_sec  = 0;
			t0.tm_isdst= -1; // daylight savings time - unknown 
			hdr->T0    = 0;
			hdr->Patient.Birthday = 0;
			uint32_t len1;
 
 			while (*(PtrCurSect+curSectPos) < 255) {
				tag = *(PtrCurSect+curSectPos);
				len1 = leu16p(PtrCurSect+curSectPos+1);
				if (VERBOSE_LEVEL>8)
					fprintf(stdout,"SCP(r): Section 1 Tag %i Len %i\n",tag,len1); 

				curSectPos += 3;
				if (curSectPos+len1 > len) {
					fprintf(stdout,"Warning SCP(read): section 1 corrupted (exceeds file length)\n");
			break;
				} 	 
				if (tag==0) {
					if (!hdr->FLAG.ANONYMOUS)
						strncpy(hdr->Patient.Name, (char*)(PtrCurSect+curSectPos),min(len1,MAX_LENGTH_NAME));
				}		
				else if (tag==1) {
//					hdr->Patient.FirstName = (char*)(PtrCurSect+curSectPos);
				}
				else if (tag==2) {
					if (len1>MAX_LENGTH_PID) {
						fprintf(stdout,"Warning SCP(read): length of Patient Id (section1 tag2) exceeds %i>%i\n",len1,MAX_LENGTH_PID); 
					}	
					strncpy(hdr->Patient.Id,(char*)(PtrCurSect+curSectPos),min(len1,MAX_LENGTH_PID)); 

					if (!strcmp(hdr->Patient.Id,"UNKNOWN")) 
						hdr->Patient.Id[0] = 0;
				}
				else if (tag==3) {
				}
				else if (tag==4) {
				}
				else if (tag==5) {
					t1.tm_year = leu16p(PtrCurSect+curSectPos)-1900;
					t1.tm_mon  = *(PtrCurSect+curSectPos+2)-1;
					t1.tm_mday = *(PtrCurSect+curSectPos+3);
					t1.tm_hour = 12; 
					t1.tm_min  =  0; 
					t1.tm_sec  =  0; 
					t1.tm_isdst= -1; // daylight saving time: unknown
//					t1.tm_gmtoff  =  0; 
					hdr->Patient.Birthday = tm_time2gdf_time(&t1);
				}
				else if (tag==6) {
					hdr->Patient.Height = leu16p(PtrCurSect+curSectPos);
				}
				else if (tag==7) {
					hdr->Patient.Weight = leu16p(PtrCurSect+curSectPos);
				}
				else if (tag==8) {
					hdr->Patient.Sex = *(PtrCurSect+curSectPos);
				}
				else if (tag==9) {
				}
				else if (tag==10) {
				}
				else if (tag==11) {
					aECG->systolicBloodPressure  = leu16p(PtrCurSect+curSectPos);
				}
				else if (tag==12) {
 					aECG->diastolicBloodPressure = leu16p(PtrCurSect+curSectPos);
				}
				else if (tag==13) {
					aECG->Diagnosis = (char*)(PtrCurSect+curSectPos);
				}
				else if (tag==14) {
					if (len1>80)
						fprintf(stderr,"Warning SCP(r): length of tag14 %i>40\n",len1);
					memcpy(hdr->ID.Manufacturer._field,(char*)PtrCurSect+curSectPos,min(len1,MAX_LENGTH_MANUF)); 
					hdr->ID.Manufacturer._field[min(len1,MAX_LENGTH_MANUF)] = 0;
					hdr->ID.Manufacturer.Model = hdr->ID.Manufacturer._field+8;  
					hdr->ID.Manufacturer.Version = hdr->ID.Manufacturer._field+36;  
					int tmp = strlen(hdr->ID.Manufacturer.Version)+1;
					hdr->ID.Manufacturer.SerialNumber = hdr->ID.Manufacturer.Version+tmp;
					tmp += strlen(hdr->ID.Manufacturer.Version+tmp)+1;	// skip SW ID
					tmp += strlen(hdr->ID.Manufacturer.Version+tmp)+1;	// skip SW
					tmp += strlen(hdr->ID.Manufacturer.Version+tmp)+1;	// skip SW
					hdr->ID.Manufacturer.Name = hdr->ID.Manufacturer.Version+tmp;
					
					/* might become obsolete */					
					//memcpy(hdr->aECG->Section1.tag14,PtrCurSect+curSectPos,40);
					//hdr->VERSION = *(PtrCurSect+curSectPos+14)/10.0;	// tag 14, byte 15
					aECG->Section1.Tag14.INST_NUMBER = leu16p(PtrCurSect+curSectPos);
					aECG->Section1.Tag14.DEPT_NUMBER = leu16p(PtrCurSect+curSectPos+2);
					aECG->Section1.Tag14.DEVICE_ID   = leu16p(PtrCurSect+curSectPos+4);
					aECG->Section1.Tag14.DEVICE_TYPE = *(PtrCurSect+curSectPos+ 6);
					aECG->Section1.Tag14.MANUF_CODE  = *(PtrCurSect+curSectPos+ 7);	// tag 14, byte 7 (MANUF_CODE has to be 255)

					const char *MANUFACTURER[] = {
						"unknown","Burdick","Cambridge",
						"Compumed","Datamed","Fukuda","Hewlett-Packard",
						"Marquette Electronics","Mortara Instruments",
						"Nihon Kohden","Okin","Quinton","Siemens","Spacelabs",
						"Telemed","Hellige","ESA-OTE","Schiller",
						"Picker-Schwarzer","et medical devices",
						"Zwönitz",NULL}; 
											
					if (!strlen(hdr->ID.Manufacturer.Name)) {
						if (aECG->Section1.Tag14.MANUF_CODE < 21)
							hdr->ID.Manufacturer.Name = MANUFACTURER[aECG->Section1.Tag14.MANUF_CODE];
						else 
							fprintf(stderr,"Warning SOPEN(SCP): unknown manufacturer code\n");
					}		 
											

					aECG->Section1.Tag14.MOD_DESC    = (char*)(PtrCurSect+curSectPos+8); 
					aECG->Section1.Tag14.VERSION     = *(PtrCurSect+curSectPos+14);
					aECG->Section1.Tag14.PROT_COMP_LEVEL = *(PtrCurSect+curSectPos+15); 	// tag 14, byte 15 (PROT_COMP_LEVEL has to be 0xA0 => level II)
					aECG->Section1.Tag14.LANG_SUPP_CODE  = *(PtrCurSect+curSectPos+16);	// tag 14, byte 16 (LANG_SUPP_CODE has to be 0x00 => Ascii only, latin and 1-byte code)
					aECG->Section1.Tag14.ECG_CAP_DEV     = *(PtrCurSect+curSectPos+17);	// tag 14, byte 17 (ECG_CAP_DEV has to be 0xD0 => Acquire, (No Analysis), Print and Store)
					aECG->Section1.Tag14.MAINS_FREQ      = *(PtrCurSect+curSectPos+18);	// tag 14, byte 18 (MAINS_FREQ has to be 0: unspecified, 1: 50 Hz, 2: 60Hz)

					aECG->Section1.Tag14.ANAL_PROG_REV_NUM = (char*)(PtrCurSect+curSectPos+36);
					tmp = strlen((char*)(PtrCurSect+curSectPos+36));					
					aECG->Section1.Tag14.SERIAL_NUMBER_ACQ_DEV = (char*)(PtrCurSect+curSectPos+36+tmp+1);
					tmp += strlen((char*)(PtrCurSect+curSectPos+36+tmp+1));					
					aECG->Section1.Tag14.ACQ_DEV_SYS_SW_ID = (char*)(PtrCurSect+curSectPos+36+tmp+1);
					tmp += strlen((char*)(PtrCurSect+curSectPos+36+tmp+1));					
					aECG->Section1.Tag14.ACQ_DEV_SCP_SW = (char*)(PtrCurSect+curSectPos+36+tmp+1); 	// tag 14, byte 38 (SCP_IMPL_SW has to be "OpenECG XML-SCP 1.00")
					tmp += strlen((char*)(PtrCurSect+curSectPos+36+tmp+1)); 
					aECG->Section1.Tag14.ACQ_DEV_MANUF  = (char*)(PtrCurSect+curSectPos+36+tmp+1);	// tag 14, byte 38 (ACQ_DEV_MANUF has to be "Manufacturer")
					
					
				}
				else if (tag==15) {
					//memcpy(hdr->aECG->Section1.tag15,PtrCurSect+curSectPos,40);
					aECG->Section1.Tag15.VERSION     = *(PtrCurSect+curSectPos+14);
				}
				else if (tag==16) {
					hdr->ID.Hospital = (char*)(PtrCurSect+curSectPos+14);
				}
				else if (tag==17) {
				}
				else if (tag==18) {
				}
				else if (tag==19) {
				}
				else if (tag==20) {
					aECG->ReferringPhysician = (char*)(PtrCurSect+curSectPos);
				}
				else if (tag==21) {
					aECG->MedicationDrugs = (char*)(PtrCurSect+curSectPos);
				}
				else if (tag==22) {
					strncpy(hdr->ID.Technician,(char*)(PtrCurSect+curSectPos),MAX_LENGTH_TECHNICIAN);
				}
				else if (tag==23) {
				}
				else if (tag==24) {
					aECG->EmergencyLevel = *(PtrCurSect+curSectPos);
				}
				else if (tag==25) {
					t0.tm_year = leu16p(PtrCurSect+curSectPos)-1900;
					t0.tm_mon  = (*(PtrCurSect+curSectPos+2)) - 1;
					t0.tm_mday = *(PtrCurSect+curSectPos+3);
				}
				else if (tag==26) {
					t0.tm_hour = *(PtrCurSect+curSectPos);
					t0.tm_min  = *(PtrCurSect+curSectPos+1);
					t0.tm_sec  = *(PtrCurSect+curSectPos+2);
				}
				else if (tag==27) {
					HighPass   = leu16p(PtrCurSect+curSectPos)/100.0;
				}
				else if (tag==28) {
					LowPass    = leu16p(PtrCurSect+curSectPos);
				}
				else if (tag==29) {
					uint8_t bitmap = *(PtrCurSect+curSectPos);
					if (bitmap==0)
						Notch = NaN;	// undefined 
					else if ((bitmap & 0x03)==0)
						Notch = -1;	// notch off
					else if (bitmap & 0x01)
						Notch = 60.0; 	// notch 60Hz
					else if (bitmap & 0x02)
						Notch = 50.0; 	// notch 50Hz
				}
				else if (tag==30) {
				}
 				else if (tag==31) {
				}
				else if (tag==32) {
				}
				else if (tag==33) {
				}
				else if (tag==34) {
					int16_t tzmin = lei16p(PtrCurSect+curSectPos);
					if (tzmin != 0x7fff)
						if (abs(tzmin)<=780) 
							t0.tm_min += tzmin;
						else 
							fprintf(stderr,"Warning SOPEN(SCP-READ): invalid time zone (Section 1, Tag34)\n");
					//fprintf(stdout,"SOPEN(SCP-READ): tzmin = %i %x \n",tzmin,tzmin); 
				}
				else {
				}
				curSectPos += len1;
			}
//			t0.tm_gmtoff = 60*tzminutes;
//			t0.tm_isdst = -1;
			hdr->T0     = tm_time2gdf_time(&t0);
		}

		/**** SECTION 2 ****/
		else if (curSect==2)  {
			aECG->FLAG.HUFFMAN = 1; 
			en1064.FLAG.HUFFMAN = 1; 

			NHT = leu16p(PtrCurSect+curSectPos);
			curSectPos += 2;
			if (NHT==19999) {
				en1064.FLAG.HUFFMAN = 1; 
				Huffman = (huffman_t*)malloc(sizeof(huffman_t));
				HTrees  = (htree_t**)malloc(sizeof(htree_t*));
				Huffman[0].NCT   = 19;
				Huffman[0].Table = DefaultTable;
				HTrees [0] = makeTree(Huffman[0]);
				k2 = 0; 
				if (VERBOSE_LEVEL==9)
					for (k1=0; k1<Huffman[k2].NCT; k1++) 
					fprintf(stdout,"%3i: %2i %2i %1i %3i %6u \n",k1,Huffman[k2].Table[k1].PrefixLength,Huffman[k2].Table[k1].CodeLength,Huffman[k2].Table[k1].TableModeSwitch,Huffman[k2].Table[k1].BaseValue,Huffman[k2].Table[k1].BaseCode); 
				if (!checkTree(HTrees[0])) // ### OPTIONAL, not needed ### 
					fprintf(stderr,"Warning: invalid Huffman Tree\n");
			}
			else {
				en1064.FLAG.HUFFMAN = NHT; 
				Huffman = (huffman_t*)malloc(NHT*sizeof(huffman_t));
				for (k2=0; k2<NHT; k2++) {
					Huffman[k2].NCT   = leu16p(PtrCurSect+curSectPos);
					curSectPos += 2;
					Huffman[k2].Table = (typeof(Huffman[k2].Table))malloc(Huffman[k2].NCT * sizeof(*Huffman[k2].Table));
					HTrees      = (htree_t**)malloc(Huffman[k2].NCT*sizeof(htree_t*));
					for (k1=0; k1<Huffman[k2].NCT; k1++) {
						Huffman[k2].Table[k1].PrefixLength = *(PtrCurSect+curSectPos);  
						Huffman[k2].Table[k1].CodeLength = *(PtrCurSect+curSectPos+1);  
						Huffman[k2].Table[k1].TableModeSwitch = *(PtrCurSect+curSectPos+2);  
						Huffman[k2].Table[k1].BaseValue  = lei16p(PtrCurSect+curSectPos+3);  
						Huffman[k2].Table[k1].BaseCode   = leu32p(PtrCurSect+curSectPos+5);
						curSectPos += 9;
						if (VERBOSE_LEVEL==9)
							fprintf(stdout,"%3i %3i: %2i %2i %1i %3i %6u \n",k2,k1,Huffman[k2].Table[k1].PrefixLength,Huffman[k2].Table[k1].CodeLength,Huffman[k2].Table[k1].TableModeSwitch,Huffman[k2].Table[k1].BaseValue,Huffman[k2].Table[k1].BaseCode);
					}
					HTrees[k2] = makeTree(Huffman[k2]); 
					if (!checkTree(HTrees[k2])) {
						B4C_ERRMSG = "Warning: invalid Huffman Tree\n";
						B4C_ERRNUM = B4C_DECOMPRESSION_FAILED; 
						// AS_DECODE = 2; // forced use of SCP-DECODE 
					}
				}
			}
		}

		/**** SECTION 3 ****/
		else if (curSect==3)  
		{
			hdr->NS = *(PtrCurSect+curSectPos);
			aECG->FLAG.REF_BEAT = (*(PtrCurSect+curSectPos+1) & 0x01);
			en1064.FLAG.REF_BEAT = (*(PtrCurSect+curSectPos+1) & 0x01);
			en1064.Section3.flags = *(PtrCurSect+curSectPos+1);
			if (en1064.FLAG.REF_BEAT && !section[4].length)
				fprintf(stderr,"Warning (SCP): Reference Beat but no Section 4\n");
			if (!(en1064.Section3.flags & 0x04) || ((en1064.Section3.flags>>3) != hdr->NS)) 
				fprintf(stderr,"Warning (SCP): channels are not simultaneously recorded! %x %i\n",en1064.Section3.flags,hdr->NS);

			curSectPos += 2;
			hdr->CHANNEL = (CHANNEL_TYPE *) calloc(hdr->NS, sizeof(CHANNEL_TYPE));
			en1064.Section3.lead = (typeof(en1064.Section3.lead))malloc(hdr->NS*sizeof(*en1064.Section3.lead));

			uint32_t startindex0; 
			startindex0 = leu32p(PtrCurSect+curSectPos);
			for (i = 0, hdr->SPR=1; i < hdr->NS; i++) {
				en1064.Section3.lead[i].start = leu32p(PtrCurSect+curSectPos);
				en1064.Section3.lead[i].end   = leu32p(PtrCurSect+curSectPos+4);

				hdr->CHANNEL[i].SPR 	= en1064.Section3.lead[i].end - en1064.Section3.lead[i].start + 1;
				hdr->SPR 		= lcm(hdr->SPR,hdr->CHANNEL[i].SPR);
				hdr->CHANNEL[i].LeadIdCode =  *(PtrCurSect+curSectPos+8);
				hdr->CHANNEL[i].Label[0]= 0;
				hdr->CHANNEL[i].LowPass = LowPass; 
				hdr->CHANNEL[i].HighPass= HighPass; 
				hdr->CHANNEL[i].Notch 	= Notch; 
				curSectPos += 9;

				if (en1064.Section3.lead[i].start != startindex0)
					fprintf(stderr,"Warning SCP(read): starting sample %i of #%i differ to %x in #1\n",en1064.Section3.lead[i].start,*(PtrCurSect+curSectPos+8),startindex0);
			}
		}
		/**** SECTION 4 ****/
		else if (curSect==4)  {
			en1064.Section4.len_ms	= leu16p(PtrCurSect+curSectPos);
			en1064.Section4.fiducial_sample	= leu16p(PtrCurSect+curSectPos+2);
			en1064.Section4.N	= leu16p(PtrCurSect+curSectPos+4);
			en1064.Section4.SPR	= hdr->SPR/4;

			en1064.Section4.beat	= (typeof(en1064.Section4.beat))malloc(en1064.Section4.N*sizeof(*en1064.Section4.beat));

			curSectPos += 6;
			for (i=0; i < en1064.Section4.N; i++) {
				en1064.Section4.beat[i].btyp = leu16p(PtrCurSect+curSectPos);
				en1064.Section4.beat[i].SB   = leu32p(PtrCurSect+curSectPos+2);
				en1064.Section4.beat[i].fcM  = leu32p(PtrCurSect+curSectPos+6);
				en1064.Section4.beat[i].SE   = leu32p(PtrCurSect+curSectPos+10);
				curSectPos += 14;
			}	
			for (i=0; i < en1064.Section4.N; i++) {
				en1064.Section4.beat[i].QB   = leu32p(PtrCurSect+curSectPos);
				en1064.Section4.beat[i].QE   = leu32p(PtrCurSect+curSectPos+4);
				curSectPos += 8;
				en1064.Section4.SPR += en1064.Section4.beat[i].QE-en1064.Section4.beat[i].QB-1;
			}	
			if (en1064.Section4.len_ms==0) {
				en1064.FLAG.REF_BEAT = 0;
			}	
		}

		/**** SECTION 5 ****/
		else if (curSect==5)  {
			Cal5 			= leu16p(PtrCurSect+curSectPos);
			en1064.Section5.AVM	= leu16p(PtrCurSect+curSectPos);
			en1064.Section5.dT_us	= leu16p(PtrCurSect+curSectPos+2);
			en1064.Section5.DIFF 	= *(PtrCurSect+curSectPos+4);
			en1064.Section5.Length  = (1000L * en1064.Section4.len_ms) / en1064.Section5.dT_us; // hdr->SPR;
			en1064.Section5.inlen	= (typeof(en1064.Section5.inlen))malloc(hdr->NS*sizeof(*en1064.Section5.inlen));
			for (i=0; i < hdr->NS; i++) {
				en1064.Section5.inlen[i] = leu16p(PtrCurSect+curSectPos+6+2*i);
				if (!section[4].length && (en1064.Section5.Length<en1064.Section5.inlen[i]))
					en1064.Section5.Length = en1064.Section5.inlen[i];
			}
			if (!section[4].length && en1064.FLAG.HUFFMAN) {
				 en1064.Section5.Length *= 5; // decompressed data might need more space 
				 fprintf(stderr,"Warning SCPOPEN: Section 4 not defined - size of Sec5 can be only guessed (%i allocated)\n",en1064.Section5.Length);
			}

			if (en1064.FLAG.REF_BEAT) {
				en1064.Section5.datablock = (int32_t*)malloc(4 * hdr->NS * en1064.Section5.Length); 

				Ptr2datablock           = (PtrCurSect+curSectPos+6+2*hdr->NS);
				for (i=0; i < hdr->NS; i++) {
					en1064.Section5.inlen[i] = leu16p(PtrCurSect+curSectPos+6+2*i);
					if (en1064.FLAG.HUFFMAN) {
						if (B4C_ERRNUM) {
							deallocEN1064(en1064);
							return(-1);
						}
						DecodeHuffman(HTrees, Huffman, Ptr2datablock, en1064.Section5.inlen[i], en1064.Section5.datablock + en1064.Section5.Length*i, en1064.Section5.Length);
					}
					else {
						for (k1=0; k1<en1064.Section5.Length; k1++)
							en1064.Section5.datablock[i*en1064.Section5.Length+k1] = lei16p(Ptr2datablock + 2*(i*en1064.Section5.Length + k1));
					}
					Ptr2datablock += en1064.Section5.inlen[i];
				}	
				size_t ix;
				data = en1064.Section5.datablock;
				if (en1064.Section5.DIFF==1)
					for (k1 = 0; k1 < hdr->NS; k1++)
					for (ix = k1*en1064.Section5.Length+1; ix < (k1+1)*en1064.Section5.Length; ix++)
						data[ix] += data[ix-1];

				else if (en1064.Section5.DIFF==2)
					for (k1 = 0; k1 < hdr->NS; k1++)
					for (ix = k1*en1064.Section5.Length+2; ix < (k1+1)*en1064.Section5.Length; ix++)
						data[ix] += 2*data[ix-1] - data[ix-2];
			}
		}

		/**** SECTION 6 ****/
		else if (curSect==6)  {
			hdr->NRec = 1;

			en1064.Section6.inlen	= (typeof(en1064.Section6.inlen))malloc(hdr->NS*sizeof(*en1064.Section6.inlen));
			uint16_t gdftyp 	= 5;	// int32: internal raw data type   
			hdr->AS.rawdata = (uint8_t*)malloc(4 * hdr->NS * hdr->SPR * hdr->NRec); 
			data = (int32_t*)hdr->AS.rawdata;

			en1064.Section6.AVM	= leu16p(PtrCurSect+curSectPos);
			en1064.Section6.dT_us	= leu16p(PtrCurSect+curSectPos+2);
			hdr->SampleRate 	= 1e6/en1064.Section6.dT_us;
			en1064.Section6.DIFF 	= *(PtrCurSect+curSectPos+4);
			en1064.FLAG.DIFF 	= *(PtrCurSect+curSectPos+4);
			en1064.Section6.BIMODAL = *(PtrCurSect+curSectPos+5);
			en1064.FLAG.BIMODAL     = *(PtrCurSect+curSectPos+5);

			Cal6 			= leu16p(PtrCurSect+curSectPos);
			en1064.Section6.dT_us	= leu16p(PtrCurSect+curSectPos+2);
			aECG->FLAG.DIFF 	= *(PtrCurSect+curSectPos+4);
			aECG->FLAG.BIMODAL = *(PtrCurSect+curSectPos+5);

			if ((section[5].length>4) &&  en1064.Section5.dT_us) 
				dT_us = en1064.Section5.dT_us;
			else 	
				dT_us = en1064.Section6.dT_us;
			hdr->SampleRate 	= 1e6/dT_us; 

			typeof(hdr->SPR) SPR  = ( en1064.FLAG.BIMODAL ? en1064.Section4.SPR : hdr->SPR);  

			if      (Cal5==0 && Cal6 >0) Cal0 = Cal6;
			else if (Cal5 >0 && Cal6==0) Cal0 = Cal5;
			else if (Cal5 >0 && Cal6 >0) Cal0 = gcd(Cal5,Cal6);
			uint16_t cal5 = Cal5/Cal0; 
			uint16_t cal6 = Cal6/Cal0; 

			Ptr2datablock = (PtrCurSect+curSectPos + 6 + hdr->NS*2);   // pointer for huffman decoder
			len = 0;  
			size_t ix; 			
			for (i=0; i < hdr->NS; i++) {
				if (VERBOSE_LEVEL>8)
					fprintf(stdout,"sec6-%i\n",i);
				
				hdr->CHANNEL[i].SPR 	    = hdr->SPR;
				hdr->CHANNEL[i].PhysDimCode = 4275; // PhysDimCode("uV") physical unit "uV"
				hdr->CHANNEL[i].Cal 	    = Cal0 * 1e-3;
				hdr->CHANNEL[i].Off         = 0;
				hdr->CHANNEL[i].OnOff       = 1;    // 1: ON 0:OFF
				hdr->CHANNEL[i].Transducer[0] = 0;
				hdr->CHANNEL[i].GDFTYP      = gdftyp;  

				// ### these values should represent the true saturation values ### //
				hdr->CHANNEL[i].DigMax      = ldexp(1.0,20)-1;
				hdr->CHANNEL[i].DigMin      = ldexp(-1.0,20);
				hdr->CHANNEL[i].PhysMax     = hdr->CHANNEL[i].DigMax * hdr->CHANNEL[i].Cal;
				hdr->CHANNEL[i].PhysMin     = hdr->CHANNEL[i].DigMin * hdr->CHANNEL[i].Cal;
				
#ifndef WITHOUT_SCP_DECODE
//				if (AS_DECODE > 0) continue; 
#endif

				en1064.Section6.inlen[i]    = leu16p(PtrCurSect+curSectPos+6+2*i);
				if (en1064.FLAG.HUFFMAN) {
					if (B4C_ERRNUM) {
						deallocEN1064(en1064);
						return(-1);
					}
					DecodeHuffman(HTrees, Huffman, Ptr2datablock, en1064.Section6.inlen[i], data + i*hdr->SPR, hdr->SPR);
				}
				else {
					for (k1=0, ix = i*hdr->SPR; k1 < SPR; k1++)
						data[ix+k1] = lei16p(Ptr2datablock + 2*k1);
				}
				len += en1064.Section6.inlen[i];
				Ptr2datablock += en1064.Section6.inlen[i]; 

				if (aECG->FLAG.DIFF==1) {
					for (ix = i*hdr->SPR+1; ix < i*hdr->SPR + SPR; ix++)
						data[ix] += data[ix-1];
				}		
				else if (aECG->FLAG.DIFF==2) {
					for (ix = i*hdr->SPR+2; ix < i*hdr->SPR + SPR; ix++)
						data[ix] += 2*data[ix-1] - data[ix-2];
				}
#ifndef WITHOUT_SCP_DECODE
				if (aECG->FLAG.BIMODAL || en1064.FLAG.REF_BEAT) { 	
//				if (aECG->FLAG.BIMODAL) {
//				if (aECG->FLAG.REF_BEAT {
					/*	this is experimental work
						Bimodal and RefBeat decompression are under development. 
						"continue" ignores code below  
						AS_DECODE=1 will call later SCP-DECODE instead  
					*/ 	
					AS_DECODE = 1; continue; 
				}
#endif

				if (aECG->FLAG.BIMODAL) {
					// ### FIXME ### 
					ix = i*hdr->SPR;		// memory offset
					k1 = en1064.Section4.SPR-1; 	// SPR of decimated data 
					k2 = hdr->SPR;			// SPR of sample data
					uint32_t k3 = en1064.Section4.N-1; // # of protected zones 
					uint8_t  k4 = 4;		// decimation factor 
					do {
						--k2;
						data[ix + k2] = data[ix + k1];
						if (k2 > en1064.Section4.beat[k3].QE) { // outside protected zone 
							if (--k4==0) {k4=4; --k1; };
						}
						else {	// inside protected zone 
							--k1;
							if (k2<en1064.Section4.beat[k3].QB) {--k3; k4=4;};
						}
					} while (k2 && (k1>=0));
				}
			
				if (en1064.FLAG.REF_BEAT) {
					/* Add reference beats */
					// ### FIXME ### 
					for (k1 = 0; k1 < en1064.Section4.N; k1++) {
						if (en1064.Section4.beat[k1].btyp == 0)
						for (ix = 0; ix < en1064.Section5.Length; ix++) {
							int32_t ix1 = en1064.Section4.beat[k1].SB - en1064.Section4.beat[k1].fcM + ix;
							int32_t ix2 = i*hdr->SPR + ix1;
							if ((en1064.Section4.beat[k1].btyp==0) && (ix1 >= 0) && (ix1 < hdr->SPR))
								data[ix2] = data[ix2] * cal6 + en1064.Section5.datablock[i*en1064.Section5.Length+ix] * cal5;
						}
					}
				}
			}

			en1064.Section6.datablock = data; 

			curSectPos += 6 + 2*hdr->NS + len;

			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"end sec6\n");

		}

		/**** SECTION 7 ****/
		else if (curSect==7)  {
			uint16_t N_QRS = *(uint8_t*)(PtrCurSect+curSectPos)-1;
			uint8_t  N_PaceMaker = *(uint8_t*)(PtrCurSect+curSectPos+1);
			uint16_t RRI = leu16p(PtrCurSect+curSectPos+2);
			uint16_t PPI = leu16p(PtrCurSect+curSectPos+4);
			curSectPos += 6;
			size_t curSectPos0 = curSectPos; // backup of pointer 
			
			// skip data on QRS measurements 
			/*
			// ### FIXME ### 
			It seems that the P,QRS, and T wave events can not be reconstructed 
			because they refer to the reference beat and not to the overall signal data.  	
			Maybe Section 4 information need to be used. However, EN1064 does not mention this.
			
			hdr->EVENT.POS = (uint32_t*)realloc(hdr->EVENT.POS, (hdr->EVENT.N+5*N_QRS+N_PaceMaker)*sizeof(*hdr->EVENT.POS));
			hdr->EVENT.TYP = (uint16_t*)realloc(hdr->EVENT.TYP, (hdr->EVENT.N+5*N_QRS+N_PaceMaker)*sizeof(*hdr->EVENT.TYP));
			hdr->EVENT.DUR = (uint32_t*)realloc(hdr->EVENT.DUR, (hdr->EVENT.N+5*N_QRS+N_PaceMaker)*sizeof(*hdr->EVENT.DUR));
			hdr->EVENT.CHN = (uint16_t*)realloc(hdr->EVENT.CHN, (hdr->EVENT.N+5*N_QRS+N_PaceMaker)*sizeof(*hdr->EVENT.CHN));
			for (i=0; i < 5*N_QRS; i++) {
				hdr->EVENT.DUR[hdr->EVENT.N+i] = 0;
				hdr->EVENT.CHN[hdr->EVENT.N+i] = 0;
			}
			for (i=0; i < 5*N_QRS; i+=5) {
				uint8_t typ = *(PtrCurSect+curSectPos+i);
				hdr->EVENT.TYP[hdr->EVENT.N]   = 0x0502;
				hdr->EVENT.TYP[hdr->EVENT.N+1] = 0x8502;
				hdr->EVENT.TYP[hdr->EVENT.N+2] = 0x0503;
				hdr->EVENT.TYP[hdr->EVENT.N+3] = 0x8503;
				hdr->EVENT.TYP[hdr->EVENT.N+4] = 0x8506;
				hdr->EVENT.POS[hdr->EVENT.N]   = leu16p(PtrCurSect+curSectPos0);
				hdr->EVENT.POS[hdr->EVENT.N+1] = leu16p(PtrCurSect+curSectPos0+2);
				hdr->EVENT.POS[hdr->EVENT.N+2] = leu16p(PtrCurSect+curSectPos0+4);
				hdr->EVENT.POS[hdr->EVENT.N+3] = leu16p(PtrCurSect+curSectPos0+6);
				hdr->EVENT.POS[hdr->EVENT.N+4] = leu16p(PtrCurSect+curSectPos0+8);
				hdr->EVENT.N+= 5;
				curSectPos0 += 16;
			}
			*/
			curSectPos += N_QRS*16;
				// pace maker information is stored in sparse sampling channel
			if (N_PaceMaker>0) {
				hdr->EVENT.POS = (uint32_t*)realloc(hdr->EVENT.POS, (hdr->EVENT.N+N_PaceMaker)*sizeof(*hdr->EVENT.POS));
				hdr->EVENT.TYP = (uint16_t*)realloc(hdr->EVENT.TYP, (hdr->EVENT.N+N_PaceMaker)*sizeof(*hdr->EVENT.TYP));
				hdr->EVENT.DUR = (uint32_t*)realloc(hdr->EVENT.DUR, (hdr->EVENT.N+N_PaceMaker)*sizeof(*hdr->EVENT.DUR));
				hdr->EVENT.CHN = (uint16_t*)realloc(hdr->EVENT.CHN, (hdr->EVENT.N+N_PaceMaker)*sizeof(*hdr->EVENT.CHN));
				/* add pacemaker channel */
				hdr->CHANNEL = (CHANNEL_TYPE *) realloc(hdr->CHANNEL,(++hdr->NS)*sizeof(CHANNEL_TYPE));
				i = hdr->NS;
				hdr->CHANNEL[i].SPR 	    = 0;    // sparse event channel 
				hdr->CHANNEL[i].PhysDimCode = 4275; // PhysDimCode("uV") physical unit "uV"
				hdr->CHANNEL[i].Cal 	    = 1;
				hdr->CHANNEL[i].Off         = 0;
				hdr->CHANNEL[i].OnOff       = 1;    // 1: ON 0:OFF
				strcpy(hdr->CHANNEL[i].Transducer,"Pacemaker");
				hdr->CHANNEL[i].GDFTYP      = 3;  

				// ### these values should represent the true saturation values ###//
				hdr->CHANNEL[i].DigMax      = ldexp(1.0,15)-1;
				hdr->CHANNEL[i].DigMin      = ldexp(-1.0,15);
				hdr->CHANNEL[i].PhysMax     = hdr->CHANNEL[i].DigMax * hdr->CHANNEL[i].Cal;
				hdr->CHANNEL[i].PhysMin     = hdr->CHANNEL[i].DigMin * hdr->CHANNEL[i].Cal;
			}
			// skip pacemaker spike measurements  
			for (i=0; i < N_PaceMaker; i++) {
				++hdr->EVENT.N;
				hdr->EVENT.TYP[hdr->EVENT.N] = 0x7fff;
				hdr->EVENT.CHN[hdr->EVENT.N] = hdr->NS;
				hdr->EVENT.POS[hdr->EVENT.N] = (uint32_t)(leu16p(PtrCurSect+curSectPos)*hdr->SampleRate*1e-3);
				hdr->EVENT.DUR[hdr->EVENT.N] = leu16p(PtrCurSect+curSectPos+2);
				curSectPos += 4;
			}
			// skip pacemaker spike information section  
			curSectPos += N_PaceMaker*6;
			
			// QRS type information 
			N_QRS = leu16p(PtrCurSect+curSectPos);
			curSectPos += 2;

		}

		/**** SECTION 8 ****/
		else if (curSect==8)  {
			aECG->Section8.Confirmed = *(char*)(PtrCurSect+curSectPos);
			aECG->Section8.t.tm_year = leu16p(PtrCurSect+curSectPos+1)-1900;
			aECG->Section8.t.tm_mon  = *(uint8_t*)(PtrCurSect+curSectPos+3)-1;
			aECG->Section8.t.tm_mday = *(uint8_t*)(PtrCurSect+curSectPos+4);
			aECG->Section8.t.tm_hour = *(uint8_t*)(PtrCurSect+curSectPos+5);
			aECG->Section8.t.tm_min  = *(uint8_t*)(PtrCurSect+curSectPos+6);
			aECG->Section8.t.tm_sec  = *(uint8_t*)(PtrCurSect+curSectPos+7);
			aECG->Section8.NumberOfStatements = *(uint8_t*)(PtrCurSect+curSectPos+8);
			aECG->Section8.Statements= (char**)malloc(aECG->Section8.NumberOfStatements*sizeof(char*));
			curSectPos += 9;
			for (uint8_t k=0; k<aECG->Section8.NumberOfStatements;k++) {
				aECG->Section8.Statements[k] = (char*)(PtrCurSect+curSectPos+3);
				curSectPos += 3+leu16p(PtrCurSect+curSectPos+1);
			}
		}

		/**** SECTION 9 ****/
		else if (curSect==9)  {
			aECG->Section9.StartPtr = (char*)(PtrCurSect+curSectPos);
			aECG->Section9.Length   = len;
		}

		/**** SECTION 10 ****/
		else if (curSect==10)  {
		}

		/**** SECTION 11 ****/
		else if (curSect==11)  {
			aECG->Section11.Confirmed = *(char*)(PtrCurSect+curSectPos);
			aECG->Section11.t.tm_year = leu16p(PtrCurSect+curSectPos+1)-1900;
			aECG->Section11.t.tm_mon  = *(uint8_t*)(PtrCurSect+curSectPos+3)-1;
			aECG->Section11.t.tm_mday = *(uint8_t*)(PtrCurSect+curSectPos+4);
			aECG->Section11.t.tm_hour = *(uint8_t*)(PtrCurSect+curSectPos+5);
			aECG->Section11.t.tm_min  = *(uint8_t*)(PtrCurSect+curSectPos+6);
			aECG->Section11.t.tm_sec  = *(uint8_t*)(PtrCurSect+curSectPos+7);
			aECG->Section11.NumberOfStatements = *(uint8_t*)(PtrCurSect+curSectPos+8);
			aECG->Section11.Statements= (char**)malloc(aECG->Section11.NumberOfStatements*sizeof(char*));
			curSectPos += 9;
			for (uint8_t k=0; k<aECG->Section11.NumberOfStatements;k++) {
				aECG->Section11.Statements[k] = (char*)(PtrCurSect+curSectPos+4);
				curSectPos += 3+leu16p(PtrCurSect+curSectPos+1);
			}
		}

		/**** SECTION 9 ****/
		else if (curSect==9)  {
		}

		/**** SECTION 10 ****/
		else if (curSect==10)  {
		}

		/**** SECTION 11 ****/
		else if (curSect==11)  {
		}
		else {
		}
	}	

	/* free allocated memory */ 
	deallocEN1064(en1064);	


#ifndef WITHOUT_SCP_DECODE
   	if (AS_DECODE==0) return(0); 
    		
/*
---------------------------------------------------------------------------
Copyright (C) 2006  Eugenio Cervesato.
Developed at the Associazione per la Ricerca in Cardiologia - Pordenone - Italy,

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

	/* Fall back method: 





		+ implements Huffman, reference beat and Bimodal compression. 
		- uses piece-wise file access
		- defines intermediate data structure
	*/	

	fprintf(stdout, "\nUse SCP_DECODE (Huffman=%i RefBeat=%i Bimodal=%i)\n", aECG->FLAG.HUFFMAN, aECG->FLAG.REF_BEAT, aECG->FLAG.BIMODAL);

	textual.des.acquiring.protocol_revision_number = aECG->Section1.Tag14.VERSION;
	textual.des.analyzing.protocol_revision_number = aECG->Section1.Tag15.VERSION; 

	decode.flag_Res.bimodal = (aECG->Section1.Tag14.VERSION > 10 ? aECG->FLAG.BIMODAL : 0);  
	decode.Reconstructed    = (int32_t*) hdr->AS.rawdata; 

	if (scp_decode(hdr, section, decode, record, textual, add_filter)) {
		if (Cal0>1)
			for (i=0; i < hdr->NS * hdr->SPR * hdr->NRec; ++i)
				data[i] /= Cal0;
	}
	else { 
		B4C_ERRNUM = B4C_CANNOT_OPEN_FILE;
		B4C_ERRMSG = "SCP-DECODE can not read file"; 
		return(0);
	}
	
	 // end of fall back method 
	return(1);
#endif 
};
