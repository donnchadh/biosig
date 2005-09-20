/*

    $Id: biosig.c,v 1.7 2005-09-20 20:25:58 schloegl Exp $
    Copyright (C) 2000,2005 Alois Schloegl <a.schloegl@ieee.org>
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
	- reading a GDF2.0pre file (fixed and variable header, data and eventtable)  
	- writing a GDF2.0pre file (fixed and variable header, data and eventtable)  
	- reading fixed header of EDF, BDF and GDF1.x files 
	
	implemented functions: 
	- SOPEN, SREAD, SWRITE, SCLOSE, SEOF, SSEEK, STELL, SREWIND 
	
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "biosig.h"
//#include "zlib.h"

/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/


/****************************************************************************/
/**                     INIT HDR                                           **/
/****************************************************************************/
HDRTYPE init_default_hdr(HDRTYPE HDR, const unsigned NS, const unsigned N_EVENT)
{
/*
	HDR is initialized, memory is allocated for 
	NS channels and N_EVENT number of events. 

	The purpose is to set the define all parameters at an initial step. 
	No parameters must remain undefined.
 */
	union {
		uint32_t testword;
		uint8  testbyte[sizeof(uint32_t)];
	} EndianTest; 
    	int k,k1;

	EndianTest.testword = 0x4a3b2c1d; 
	HDR.FILE.LittleEndian = (EndianTest.testbyte[0]==0x1d); 
	
	if  (!HDR.FILE.LittleEndian) {
		fprintf(stderr,"error: only little endian platforms are supported, yet.\n"); 
		exit(-1); 
	}	


      	HDR.TYPE = GDF; 
      	HDR.VERSION = 1.91; 
      	HDR.buffer = malloc(0);
      	HDR.NRec = -1; 
      	HDR.NS = NS;	
      	memset(HDR.AS.PID,32,81); 
      	HDR.AS.RID = "GRAZ"; 
      	HDR.T0 = time_t2time_gdf(time(NULL));
      	HDR.ID.Equipment = *(uint64_t*)&"b4c_0.20";

	HDR.Patient.Name 	= "X";
	HDR.Patient.Id 		= "X";
	HDR.Patient.Birthday 	= Unknown;
      	HDR.Patient.Medication 	= Unknown;
      	HDR.Patient.DrugAbuse 	= Unknown;
      	HDR.Patient.AlcoholAbuse= Unknown;
      	HDR.Patient.Smoking 	= Unknown;
      	HDR.Patient.Sex 	= Unknown;
      	HDR.Patient.Handedness 	= Unknown;
      	HDR.Patient.Impairment.Visual = Unknown;
      	HDR.Patient.Weight 	= Unknown;
      	HDR.Patient.Height 	= Unknown;
      	HDR.Dur[0] = 1;
      	HDR.Dur[1] = 1;
	memset(&HDR.IPaddr,0,6);
      	for (k1=0; k1<3; k1++) {
      		HDR.Patient.Headsize[k1] = Unknown;
      		HDR.ELEC.REF[k1] = 0.0;
      		HDR.ELEC.GND[k1] = 0.0;
      	}
	HDR.LOC[0] = 0x00292929; 		
	HDR.LOC[1] = 48*3600000+(1<<31); 	// latitude
	HDR.LOC[2] = 15*3600000+(1<<31); 	// longitude 
	HDR.LOC[3] = 35000; 	 	//altitude in centimeter above sea level
	
       	// define variable header 
	HDR.CHANNEL = calloc(HDR.NS,sizeof(CHANNEL_TYPE));
	for (k=0;k<HDR.NS;k++)	{
	      	HDR.CHANNEL[k].Label     = "C4";
	      	HDR.CHANNEL[k].Transducer= "EEG: Ag-AgCl electrodes";
	      	HDR.CHANNEL[k].PhysDim   = "uV";
	      	HDR.CHANNEL[k].PhysMax   = +100;
	      	HDR.CHANNEL[k].PhysMin   = -100;
	      	HDR.CHANNEL[k].DigMax    = +2047;
	      	HDR.CHANNEL[k].DigMin    = -2048;
	      	HDR.CHANNEL[k].GDFTYP    = 3;	// int16 	
	      	HDR.CHANNEL[k].SPR       = 1;	// one sample per block
	      	HDR.CHANNEL[k].HighPass  = 0.16;
	      	HDR.CHANNEL[k].LowPass   = 70.0;
	      	HDR.CHANNEL[k].Notch     = 50;
	      	HDR.CHANNEL[k].Impedance = 1.0/0.0;
	      	for (k1=0; k1<3; HDR.CHANNEL[k].XYZ[k1++] = 0.0);
	}      	

	// define EVENT structure
	HDR.EVENT.N = N_EVENT; 
	HDR.EVENT.SampleRate = HDR.SampleRate; 
	HDR.EVENT.POS = calloc(HDR.EVENT.N,sizeof(*HDR.EVENT.POS));
	HDR.EVENT.TYP = calloc(HDR.EVENT.N,sizeof(*HDR.EVENT.TYP));
	HDR.EVENT.DUR = calloc(HDR.EVENT.N,sizeof(*HDR.EVENT.DUR));
	HDR.EVENT.CHN = calloc(HDR.EVENT.N,sizeof(*HDR.EVENT.CHN));
	
	return(HDR);
}


/****************************************************************************/
/**                     SOPEN                                              **/
/****************************************************************************/
HDRTYPE sopen(const char* FileName, HDRTYPE HDR, const char* MODE)
/*
	MODE="r" 
		reads file and returns HDR 
	MODE="w" 
		writes HDR into file 
 */
{
    	
    	const char*	GENDER = "XMFX";  

    	int 		k, c[4];
    	unsigned int 	count,len;
    	char 		tmp[81];
    	char 		cmd[256];
    	double 		Dur; 
	char* 		Header1;
	char* 		Header2;
	char*		ptr_str;
	struct tm 	tm_time; 
	time_t		tt;
	

if (!strcmp(MODE,"r"))	
{
	   
	HDR = init_default_hdr(HDR,0,0);	// initializes fields that may stay undefined during SOPEN 

#ifdef ZLIB_H
	HDR.FILE.FID = fopen(FileName,"rb");
#else
	HDR.FILE.FID = fopen(FileName,"rb");
#endif

    	if (HDR.FILE.FID == NULL) 
    	{ 	fprintf(stdout,"ERROR %s not found\n",FileName);
		return(HDR);
    	}	    
    
    	/******** read 1st (fixed)  header  *******/	
 	Header1 = malloc(256);
/*#ifdef ZLIB_H
    	count   = gzread(HDR.FILE.FID,Header1,256);
#else
*/
    	count   = fread(Header1,1,256,HDR.FILE.FID);
//#endif
//    	fprintf(stdout,": %i not found\n",count);
	
    	if (0);
    	else if (!memcmp(Header1+1,"BIOSEMI",7)) {
    		HDR.TYPE = BDF;
    		HDR.VERSION = -1; 
    	}	
    	else if ((Header1[0]==(char)207) & (!Header1[1]) & (!Header1[154]) & (!Header1[155]))
	    	HDR.TYPE = BKR;
    	else if (!memcmp(Header1,"Version 3.0",11))
	    	HDR.TYPE = CNT;
    	else if (!memcmp(Header1,"DEMG",4))
	    	HDR.TYPE = DEMG;
    	else if (!memcmp(Header1,"0       ",8)) {
	    	HDR.TYPE = EDF;
	    	HDR.VERSION = 0; 
	}    	
    	else if (!memcmp(Header1,"fLaC",4))
	    	HDR.TYPE = FLAC;
    	else if (!memcmp(Header1,"GDF",3))
	    	HDR.TYPE = GDF; 
    	else if (!memcmp(Header1,"@  MFER ",8))
	    	HDR.TYPE = MFER;
    	else if (!memcmp(Header1,"@ MFR ",6))
	    	HDR.TYPE = MFER;
    	else if (!memcmp(Header1,"NEX1",4))
	    	HDR.TYPE = NEX1;
    	else if (!memcmp(Header1,"PLEX",4))
	    	HDR.TYPE = PLEXON;

    	if (HDR.TYPE == GDF) {
  	    	strncpy(tmp,(char*)Header1+3,5);
	    	HDR.VERSION 	= atof(tmp);
	    	HDR.HeadLen 	= *( int32_t*) (Header1+184); 
	    	HDR.NRec 	= *( int32_t*) (Header1+236); 
	    	HDR.Dur[0]  	= *(uint32_t*) (Header1+244);
	    	HDR.Dur[1]  	= *(uint32_t*) (Header1+248); 
	    	HDR.NS   	= *(uint32_t*) (Header1+252); 
	    	
	    	if (HDR.VERSION > 1.90) { 
	    		strncpy(HDR.AS.PID,Header1+8,66);
	    		HDR.Patient.Id = strtok(HDR.AS.PID," ");
	    		HDR.Patient.Name = strtok(NULL," ");
	    		
	    		HDR.Patient.Smoking      =  Header1[84]%4;
	    		HDR.Patient.AlcoholAbuse = (Header1[84]>>2)%4;
	    		HDR.Patient.DrugAbuse  	 = (Header1[84]>>4)%4;
	    		HDR.Patient.Medication   = (Header1[84]>>8)%4;
	    		HDR.Patient.Weight       =  Header1[85];
	    		HDR.Patient.Height       =  Header1[86];
	    		HDR.Patient.Sex       	 =  Header1[87]%4;
	    		HDR.Patient.Handedness   = (Header1[87]>>2)%4;
	    		HDR.Patient.Impairment.Visual = (Header1[87]>>4)%4;
	
			if (Header1[156]) {
				HDR.LOC[0] = 0x00292929;
				memcpy(&HDR.LOC[1], Header1+152, 12);
			}
			else {
				memcpy(&HDR.LOC, Header1+152, 16);
			}
			memcpy(&HDR.T0, Header1+168,8);
			memcpy(&HDR.Patient.Birthday, Header1+176, 8);

			memcpy(&HDR.ID.Equipment, Header1+192,6);
			memcpy(&HDR.IPaddr, Header1+200,6);
			memcpy(&HDR.Patient.Headsize, Header1+206,6);
			memcpy(&HDR.ELEC.REF, Header1+212,12);
			memcpy(&HDR.ELEC.GND, Header1+224,12);
	    	}
	    	else {
	    		strncpy(HDR.AS.PID,Header1+8,80);
	    		HDR.Patient.Id = strtok(HDR.AS.PID," ");
	    		HDR.Patient.Name = strtok(NULL," ");
	    		
	    		tm_time.tm_sec  = atoi(strncpy(tmp,Header1+168+12,2)); 
	    		tm_time.tm_min  = atoi(strncpy(tmp,Header1+168+10,2)); 
	    		tm_time.tm_hour = atoi(strncpy(tmp,Header1+168+8,2)); 
	    		tm_time.tm_mday = atoi(strncpy(tmp,Header1+168+6,2)); 
	    		tm_time.tm_mon  = atoi(strncpy(tmp,Header1+168+4,2)); 
	    		tm_time.tm_year = atoi(strncpy(tmp,Header1+168,4)); 
	    		HDR.T0 = time_t2time_gdf(mktime(&tm_time)); 
	    	}
    	}
    	else if ((HDR.TYPE == EDF) | (HDR.TYPE == BDF))	{
    		strncpy(HDR.AS.PID,Header1+8,80);

	    	HDR.HeadLen 	= atoi(strncpy(tmp,Header1+184,8));
	    	HDR.NRec 	= atoi(strncpy(tmp,Header1+236,8));
	    	Dur 		= atoi(strncpy(tmp,Header1+244,8));
	    	HDR.NS 		= atoi(strncpy(tmp,Header1+252,4));
		if (!Dur)
		{	
			HDR.Dur[0] = Dur; 
			HDR.Dur[1] = 1; 
		}
    		tm_time.tm_sec  = atoi(strncpy(tmp,Header1+168+14,2)); 
    		tm_time.tm_min  = atoi(strncpy(tmp,Header1+168+11,2)); 
    		tm_time.tm_hour = atoi(strncpy(tmp,Header1+168+8,2)); 
    		tm_time.tm_mday = atoi(strncpy(tmp,Header1+168,2)); 
    		tm_time.tm_mon  = atoi(strncpy(tmp,Header1+168+3,2)); 
    		tm_time.tm_year = atoi(strncpy(tmp,Header1+168+6,2)); 
    		tm_time.tm_year+= (tm_time.tm_year<85)*100;
		HDR.T0 = time_t2time_gdf(mktime(&tm_time)); 

		if (!strncmp(Header1+192,"EDF+",4)) {
	    		HDR.Patient.Id  = strtok(HDR.AS.PID," ");
	    		ptr_str = strtok(NULL," ");
	    		HDR.Patient.Sex = (ptr_str[0]=='f')*2 + (ptr_str[0]=='F')*2 + (ptr_str[0]=='M') + (ptr_str[0]=='m');
	    		ptr_str = strtok(NULL," ");	// birthday
	    		HDR.Patient.Name= strtok(NULL," ");

	    		tm_time.tm_mday = atoi(strtok(ptr_str,"-")); 
/*	    		strcpy(tmp,strtok(NULL,"-"));
	    		tm_time.tm_year = atoi(strtok(NULL,"-"))-1900; 
	    		tm_time.tm_mon  = !strcmp(tmp,"Feb")+!strcmp(tmp,"Mar")*2+!strcmp(tmp,"Apr")*3+!strcmp(tmp,"May")*4+!strcmp(tmp,"Jun")*5+!strcmp(tmp,"Jul")*6+!strcmp(tmp,"Aug")*7+!strcmp(tmp,"Sep")*8+!strcmp(tmp,"Oct")*9+!strcmp(tmp,"Nov")*10+!strcmp(tmp,"Dec")*11;
	    		tm_time.tm_sec  = 0; 
	    		tm_time.tm_min  = 0; 
	    		tm_time.tm_hour = 12; 
	    		HDR.Patient.Birthday = time_t2time_gdf(mktime(&tm_time));
*/		}
	}      	
	else if (HDR.TYPE==BKR) {
	    	Header1 = realloc(Header1,1024);
	    	count   = fread(Header1+256,1,1024-256,HDR.FILE.FID);
	    	HDR.HeadLen = 1024; 
		HDR.NS  	= *(uint16_t*) (Header1+2); 
		HDR.SampleRate  = *(uint16_t*) (Header1+4); 
		HDR.NRec  	= *(uint32_t*) (Header1+6); 
		HDR.SPR  	= *(uint32_t*) (Header1+10); 
		HDR.T0 		= Unknown;
	    	/* extract more header information */
	}
	else if (HDR.TYPE==CNT) {
	    	Header1 = realloc(Header1,900);
	    	count   = fread(Header1+256,1,900-256,HDR.FILE.FID);
		HDR.NS  = *(uint16_t*) (Header1+370); 
	    	HDR.HeadLen = 900+HDR.NS*75; 
	    	Header1 = realloc(Header1,HDR.HeadLen);
	    	count   = fread(Header1+900,1,HDR.NS*75,HDR.FILE.FID);
	    	Header2 = Header1+900; 

	    	/* extract more header information */
	}
    	
	/* ******* read 2nd (variable)  header  ****** */	
    	//HDR.Header2 = malloc(HDR.NS*256);
    	HDR.CHANNEL = calloc(HDR.NS,sizeof(CHANNEL_TYPE));
	if ((HDR.TYPE == GDF) | (HDR.TYPE == EDF) | (HDR.TYPE == BDF))  {
	    	Header1 = realloc(Header1,HDR.HeadLen);
	    	Header2 = Header1+256; 
	    	count   = fread(Header2, 1, HDR.HeadLen-256, HDR.FILE.FID);
		for (k=0; k<HDR.NS;k++)	{
			//HDR.CHANNEL[k].Label  = (HDR.Header2 + 16*k);
			//HDR.CHANNEL[k].Transducer  = (HDR.Header2 + 80*k + 16*HDR.NS);
			//HDR.CHANNEL[k].PhysDim  = (HDR.Header2 + 8*k + 96*HDR.NS);
			if (HDR.TYPE==GDF)
				HDR.CHANNEL[k].PhysMin  = *(double*) (Header2+ 8*k + 104*HDR.NS);
				HDR.CHANNEL[k].PhysMax  = *(double*) (Header2+ 8*k + 112*HDR.NS);
				HDR.CHANNEL[k].DigMin   = *(int64_t*)  (Header2+ 8*k + 120*HDR.NS);
				HDR.CHANNEL[k].DigMax   = *(int64_t*)  (Header2+ 8*k + 128*HDR.NS);
				//HDR.CHANNEL[k].PreFilt = (HDR.Header2+ 68*k + 136*HDR.NS);
				HDR.CHANNEL[k].SPR      = *(int32_t*)  (Header2+ 4*k + 216*HDR.NS);
				HDR.CHANNEL[k].GDFTYP   = *(int32_t*)  (Header2+ 4*k + 220*HDR.NS);
				if (HDR.VERSION>=1.90) {
					HDR.CHANNEL[k].LowPass  = *(float*) (Header2+ 4*k + 204*HDR.NS);
					HDR.CHANNEL[k].HighPass = *(float*) (Header2+ 4*k + 208*HDR.NS);
					HDR.CHANNEL[k].Notch    = *(float*) (Header2+ 4*k + 212*HDR.NS);
					memcpy(&HDR.CHANNEL[k].XYZ,Header2 + 4*k + 224*HDR.NS,12);
					HDR.CHANNEL[k].Impedance= ldexp(1.0, Header2[k + 236*HDR.NS]/8.0);
				}
			else {
				HDR.CHANNEL[k].PhysMin = atof(strncpy(tmp,Header2 + 8*k + 104*HDR.NS,8)); 
				HDR.CHANNEL[k].PhysMax = atof(strncpy(tmp,Header2 + 8*k + 112*HDR.NS,8)); 
				HDR.CHANNEL[k].DigMin  = atol(strncpy(tmp,Header2 + 8*k + 120*HDR.NS,8)); 
				HDR.CHANNEL[k].DigMax  = atol(strncpy(tmp,Header2 + 8*k + 128*HDR.NS,8)); 
				HDR.CHANNEL[k].SPR     = atol(strncpy(tmp,Header2+  8*k + 216*HDR.NS,8));
				
				HDR.CHANNEL[k].GDFTYP   = ((HDR.TYPE!=BDF) ? 3 : 255+24); 
				
				//HDR.CHANNEL[k].PreFilt  = (HDR.Header2+ 68*k + 136*HDR.NS);
				if (strncmp(Header1+192,"EDF+",4)) {
				// decode filter information into HDR.Filter.{Lowpass, Highpass, Notch}					
				}
			}	
		}
	
		// internal variables
    		HDR.AS.bi = calloc(HDR.NS+1,sizeof(long));
		HDR.AS.bi[0] = 0;
		for (k=0, HDR.AS.spb=0, HDR.AS.bpb=0; k<HDR.NS;) {
			HDR.AS.spb += HDR.CHANNEL[k].SPR;
			HDR.AS.bpb += GDFTYP_BYTE[HDR.CHANNEL[k].GDFTYP]*HDR.CHANNEL[k].SPR;			
			HDR.AS.bi[++k] = HDR.AS.bpb; 
		}	
	
		// READ EVENTTABLE 
		if (HDR.TYPE==GDF) {
			fseek(HDR.FILE.FID,HDR.HeadLen + HDR.AS.bpb*HDR.NRec, SEEK_SET); 
			fread(tmp,sizeof(char),8,HDR.FILE.FID);
			HDR.EVENT.SampleRate = 0; 
			memcpy(&HDR.EVENT.SampleRate,tmp+1,3);
			memcpy(&HDR.EVENT.N,tmp+4,4);
			HDR.EVENT.POS = realloc(HDR.EVENT.POS,HDR.EVENT.N*sizeof(*HDR.EVENT.POS));
			HDR.EVENT.TYP = realloc(HDR.EVENT.TYP,HDR.EVENT.N*sizeof(*HDR.EVENT.TYP));
			fread(HDR.EVENT.POS,sizeof(*HDR.EVENT.POS),HDR.EVENT.N,HDR.FILE.FID);
			fread(HDR.EVENT.TYP,sizeof(*HDR.EVENT.TYP),HDR.EVENT.N,HDR.FILE.FID);
			if (tmp[0]>1) {
				HDR.EVENT.DUR = realloc(HDR.EVENT.DUR,HDR.EVENT.N*sizeof(*HDR.EVENT.DUR));
				HDR.EVENT.CHN = realloc(HDR.EVENT.CHN,HDR.EVENT.N*sizeof(*HDR.EVENT.CHN));
				fread(HDR.EVENT.CHN,sizeof(*HDR.EVENT.CHN),HDR.EVENT.N,HDR.FILE.FID);
				fread(HDR.EVENT.DUR,sizeof(*HDR.EVENT.DUR),HDR.EVENT.N,HDR.FILE.FID);
			}
			else {
				HDR.EVENT.DUR = NULL;
				HDR.EVENT.CHN = NULL;
			}
		}	
		fseek(HDR.FILE.FID,HDR.HeadLen, SEEK_SET); 
		HDR.FILE.OPEN = 2; 	     	
		HDR.FILE.POS  = 0;
        }   //  if GDF	
        
	else { // next format 

        }   //  
        
}
else { // WRITE

    	if (HDR.TYPE==GDF) {	
	     	HDR.HeadLen = (HDR.NS+1)*256;
	    	Header1 = malloc(HDR.HeadLen);
	    	Header2 = Header1+256; 

		memset(Header1,0,HDR.HeadLen);
	     	strcpy(Header1, "GDF 1.91");
	     	strncat(Header1+8, HDR.Patient.Id,   66);
	     	strncat(Header1+8, " ",   66);
	     	strncat(Header1+8, HDR.Patient.Name, 66);

	     	Header1[84] = (HDR.Patient.Smoking%4) + ((HDR.Patient.AlcoholAbuse%4)<<2) + ((HDR.Patient.DrugAbuse%4)<<4) + ((HDR.Patient.Medication%4)<<6);
	     	Header1[85] =  HDR.Patient.Weight;
	     	Header1[86] =  HDR.Patient.Height;
	     	Header1[87] = (HDR.Patient.Sex%4) + ((HDR.Patient.Handedness%4)<<2) + ((HDR.Patient.Impairment.Visual%4)<<4);

	     	len = strlen(HDR.AS.RID);
	     	memcpy(Header1+ 88,  HDR.AS.RID, min(len,80));
		memcpy(Header1+152, &HDR.LOC, 16); 
		memcpy(Header1+168, &HDR.T0, 8); 
		memcpy(Header1+176, &HDR.Patient.Birthday, 8); 
		memcpy(Header1+184, &HDR.HeadLen, 4); 
		memcpy(Header1+192, &HDR.ID.Equipment, 8);

		memcpy(Header1+200, &HDR.IPaddr, 6);
		memcpy(Header1+206, &HDR.Patient.Headsize, 6);
		memcpy(Header1+212, &HDR.ELEC.REF, 12);
		memcpy(Header1+224, &HDR.ELEC.GND, 12);
		memcpy(Header1+236, &HDR.NRec, 4);
		memcpy(Header1+244, &HDR.Dur, 8); 
		memcpy(Header1+252, &HDR.NS, 4); 
		
	     	/* define HDR.Header2 
	     	this requires checking the arguments in the fields of the struct HDR.CHANNEL
	     	and filling in the bytes in HDR.Header2. 
	     	*/
		for (k=0;k<HDR.NS;k++)
		{
		     	len = strlen(HDR.CHANNEL[k].Label);
		     	memcpy(Header2+16*k,HDR.CHANNEL[k].Label,min(len,16));
		     	len = strlen(HDR.CHANNEL[k].Transducer);
		     	memcpy(Header2+80*k + 16*HDR.NS,HDR.CHANNEL[k].Transducer,min(len,80));
		     	len = strlen(HDR.CHANNEL[k].PhysDim);
		     	memcpy(Header2+ 8*k + 96*HDR.NS,HDR.CHANNEL[k].PhysDim,min(len,8));
	
		     	memcpy(Header2+ 8*k + 104*HDR.NS,&HDR.CHANNEL[k].PhysMin,8);
		     	memcpy(Header2+ 8*k + 112*HDR.NS,&HDR.CHANNEL[k].PhysMax,8);
		     	memcpy(Header2+ 8*k + 120*HDR.NS,&HDR.CHANNEL[k].DigMin,8);
		     	memcpy(Header2+ 8*k + 128*HDR.NS,&HDR.CHANNEL[k].DigMax,8);
		     	memcpy(Header2+ 4*k + 204*HDR.NS,&HDR.CHANNEL[k].LowPass,4);
		     	memcpy(Header2+ 4*k + 208*HDR.NS,&HDR.CHANNEL[k].HighPass,4);
		     	memcpy(Header2+ 4*k + 212*HDR.NS,&HDR.CHANNEL[k].Notch,4);
		     	memcpy(Header2+ 4*k + 216*HDR.NS,&HDR.CHANNEL[k].SPR,4);
		     	memcpy(Header2+ 4*k + 220*HDR.NS,&HDR.CHANNEL[k].GDFTYP,4);
		     	memcpy(Header2+12*k + 224*HDR.NS,&HDR.CHANNEL[k].XYZ,12);
	
	     		Header2[k+236*HDR.NS] = ceil(log10(min(39e8,HDR.CHANNEL[k].Impedance))/log10(2.0)*8.0-0.5);
		}
	    	HDR.AS.bi = calloc(HDR.NS+1,sizeof(int32_t));
		HDR.AS.bi[0] = 0;
		for (k=0, HDR.AS.spb=0, HDR.AS.bpb=0; k<HDR.NS;)
		{
			HDR.AS.spb += HDR.CHANNEL[k].SPR;
			HDR.AS.bpb += GDFTYP_BYTE[HDR.CHANNEL[k].GDFTYP] * HDR.CHANNEL[k].SPR;
			HDR.AS.bi[++k] = HDR.AS.bpb; 
		}	
	}
    	else if ((HDR.TYPE==EDF) | (HDR.TYPE==BDF)) {	
	     	HDR.HeadLen = (HDR.NS+1)*256;
	    	Header1 = malloc(HDR.HeadLen);
	    	Header2 = Header1+256; 
		if (HDR.TYPE==BDF) {	
			Header1[0] = 255;
	     		memcpy(Header1+1,"BIOSEMI",7);
		}
		else {
			memset(Header1,' ',HDR.HeadLen);
	     		memcpy(Header1,"0       ",8);
	     	}

		tt = time_gdf2time_t(HDR.Patient.Birthday); 
		if (HDR.Patient.Birthday>1) strftime(tmp,81,"%d-%b-%Y",gmtime(&tt));
		else strcpy(tmp,"X");	
		sprintf(cmd,"%s %c %s %s",HDR.Patient.Id,GENDER[HDR.Patient.Sex],tmp,HDR.Patient.Name);
	     	memcpy(Header1+8, cmd, strlen(cmd));
	     	
		tt = time_gdf2time_t(HDR.T0); 
		if (HDR.T0>1) strftime(tmp,81,"%d-%b-%Y",gmtime(&tt));
		else strcpy(tmp,"X");	
		len = sprintf(cmd,"Startdate %s X X ",tmp,HDR.Patient.Name);
	     	memcpy(Header1+88, cmd, len);
	     	memcpy(Header1+88+len, &HDR.ID.Equipment, 8);
	     	
		tt = time_gdf2time_t(HDR.T0); 
		strftime(tmp,81,"%d.%m.%y%H:%M:%S",gmtime(&tt));
	     	memcpy(Header1+168, tmp, 16);

		len = sprintf(tmp,"%i",HDR.HeadLen);
		if (len>8) fprintf(stderr,"Warning: HeaderLength is (%s) to long.\n",tmp);  
	     	memcpy(Header1+184, tmp, len);
	     	memcpy(Header1+192, "EDF+C  ", 5);

		len = sprintf(tmp,"%i",HDR.NRec);
		if (len>8) fprintf(stderr,"Warning: NRec is (%s) to long.\n",tmp);  
	     	memcpy(Header1+236, tmp, len);

		len = sprintf(tmp,"%f",((double)HDR.Dur[0])/HDR.Dur[1]);
		if (len>8) fprintf(stderr,"Warning: Duration is %s) to long.\n",tmp);  
	     	memcpy(Header1+244, tmp, len);

		len = sprintf(tmp,"%i",HDR.NS);
		if (len>4) fprintf(stderr,"Warning: NS is %s) to long.\n",tmp);  
	     	memcpy(Header1+252, tmp, len);
	     	
		for (k=0;k<HDR.NS;k++)
		{
		     	len = strlen(HDR.CHANNEL[k].Label);
			if (len>16) fprintf(stderr,"Warning: Label (%s) of channel %i is to long.\n",HDR.CHANNEL[k].Label,k);  
		     	memcpy(Header2+16*k,HDR.CHANNEL[k].Label,min(len,16));
		     	len = strlen(HDR.CHANNEL[k].Transducer);
			if (len>80) fprintf(stderr,"Warning: Transducer (%s) of channel %i is to long.\n",HDR.CHANNEL[k].Transducer,k);  
		     	memcpy(Header2+80*k + 16*HDR.NS,HDR.CHANNEL[k].Transducer,min(len,80));
		     	len = strlen(HDR.CHANNEL[k].PhysDim);
			if (len>8) fprintf(stderr,"Warning: Physical Dimension (%s) of channel %i is to long.\n",HDR.CHANNEL[k].PhysDim,k);  
		     	memcpy(Header2+ 8*k + 96*HDR.NS,HDR.CHANNEL[k].PhysDim,min(len,8));
	
			len = sprintf(tmp,"%f",HDR.CHANNEL[k].PhysMin);
			if (len>8) fprintf(stderr,"Warning: PhysMin (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 104*HDR.NS,tmp,min(8,len));
			len = sprintf(tmp,"%f",HDR.CHANNEL[k].PhysMax);
			if (len>8) fprintf(stderr,"Warning: PhysMax (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 112*HDR.NS,tmp,min(8,len));
			len = sprintf(tmp,"%i",HDR.CHANNEL[k].DigMin);
			if (len>8) fprintf(stderr,"Warning: DigMin (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 120*HDR.NS,tmp,min(8,len));
			len = sprintf(tmp,"%i",HDR.CHANNEL[k].DigMax);
			if (len>8) fprintf(stderr,"Warning: DigMax (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 128*HDR.NS,tmp,min(8,len));
		     	
			if (HDR.CHANNEL[k].Notch>0)		     	
				len = sprintf(tmp,"HP:%fHz LP:%fHz Notch:%fHz",HDR.CHANNEL[k].HighPass,HDR.CHANNEL[k].LowPass,HDR.CHANNEL[k].Notch);
			else
				len = sprintf(tmp,"HP:%fHz LP:%fHz",HDR.CHANNEL[k].HighPass,HDR.CHANNEL[k].LowPass);
		     	memcpy(Header2+ 80*k + 136*HDR.NS,tmp,min(80,len));
		     	
			len = sprintf(tmp,"%i",HDR.CHANNEL[k].SPR);
			if (len>8) fprintf(stderr,"Warning: SPR (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 216*HDR.NS,tmp,min(8,len));
		     	HDR.CHANNEL[k].GDFTYP = ( (HDR.TYPE != BDF) ? 3 : 255+24);
		}

	    	HDR.AS.bi = calloc(HDR.NS+1,sizeof(int32_t));
		HDR.AS.bi[0] = 0;
		for (k=0, HDR.AS.spb=0, HDR.AS.bpb=0; k<HDR.NS;)
		{
			HDR.AS.spb += HDR.CHANNEL[k].SPR;
			HDR.AS.bpb += GDFTYP_BYTE[HDR.CHANNEL[k].GDFTYP]*HDR.CHANNEL[k].SPR;
			HDR.AS.bi[++k] = HDR.AS.bpb; 
		}	
	}
	else if (HDR.TYPE==BKR) {
	    	Header1 = malloc(1024);
	    	HDR.HeadLen = 1024; 
	}
	else 
		return(HDR); 
	

    	HDR.FILE.FID = fopen(FileName,"wb");
    	if (HDR.FILE.FID == NULL) 
    	{ 	fprintf(stderr,"ERROR: Unable to open file %s \n",FileName);
		return(HDR);
    	}	    
    	fwrite(Header1,sizeof(char),HDR.HeadLen,HDR.FILE.FID);
	HDR.FILE.OPEN = 2; 	     	
	HDR.FILE.POS  = 0; 	

}	// end of else 
	
	
	return(HDR);
}  // end of SOPEN 


/****************************************************************************/
/**                     SREAD                                              **/
/****************************************************************************/
size_t sread(HDRTYPE *hdr, size_t nelem) {
/* 
 *	reads NELEM blocks with HDR.AS.bpb BYTES each, 
 *	data is available in hdr->buffer
 */
	size_t count; 
	
	// allocate buffer 	
	hdr->buffer = realloc(hdr->buffer, (hdr->AS.bpb)*nelem);

	// limit reading to end of data block
	nelem = max(min(nelem, hdr->NRec - hdr->FILE.POS),0);

	// read data	
	count = fread(hdr->buffer, hdr->AS.bpb, nelem, hdr->FILE.FID);

	// set position of file handle 
	hdr->FILE.POS += count;

	return(count);

}  // end of SREAD 



/****************************************************************************/
/**                     SWRITE                                             **/
/****************************************************************************/
size_t swrite(const void *ptr, size_t nelem, HDRTYPE* hdr) {
/* 
 *	writes NELEM blocks with HDR.AS.bpb BYTES each, 
 */
	size_t count; 

	// write data 
	count = fwrite((byte*)ptr, hdr->AS.bpb, nelem, hdr->FILE.FID);
	
	// set position of file handle 
	(hdr->FILE.POS) += count; 

	return(count);

}  // end of SWRITE 


/****************************************************************************/
/**                     SEOF                                               **/
/****************************************************************************/
int seof(HDRTYPE HDR)
{
	return(HDR.FILE.POS >= HDR.NRec);
}


/****************************************************************************/
/**                     SREWIND                                            **/
/****************************************************************************/
int srewind(HDRTYPE* hdr)
{
	return(sseek(hdr,0,SEEK_SET));
}


/****************************************************************************/
/**                     SSEEK                                             **/
/****************************************************************************/
int32_t sseek(HDRTYPE* hdr, uint32_t offset, int whence)
{
	uint32_t pos; 
	
	if    	(whence == SEEK_SET) 
		pos = offset*hdr->AS.bpb+(hdr->HeadLen);
	else if (whence == SEEK_CUR) 
		pos = hdr->FILE.POS + offset*hdr->AS.bpb;
	else if (whence == SEEK_END) 
		pos = (hdr->NRec + offset)*hdr->AS.bpb;
	
	if ((pos < hdr->HeadLen) | (pos > hdr->NRec * hdr->AS.bpb))
		return(-1);
	else if (fseek(hdr->FILE.FID, pos, SEEK_SET))
		return(-1);

	hdr->FILE.POS = (pos - hdr->HeadLen) / (hdr->AS.bpb); 	
	return(0);
	
}  // end of SSEEK


/****************************************************************************/
/**                     STELL                                              **/
/****************************************************************************/
int32_t stell(HDRTYPE HDR)
{
	int32_t pos; 
	
	pos = ftell(HDR.FILE.FID);	
	if (pos<0)
		return(0xffffffff);
	else if (pos != (HDR.FILE.POS * HDR.AS.bpb + HDR.HeadLen))
		return(0xffffffff);
	else 
		return(HDR.FILE.POS);
	
}  // end of STELL


/****************************************************************************/
/**                     SCLOSE                                             **/
/****************************************************************************/
HDRTYPE sclose(HDRTYPE HDR)
{
	int32_t pos, k, len; 
	char tmp[88]; 
	char flag; 
	
	if ((HDR.NRec<0) & (HDR.FILE.OPEN>1))
	if ((HDR.TYPE==GDF) | (HDR.TYPE==EDF) | (HDR.TYPE==BDF))
	{
		// WRITE HDR.NRec 
		pos = (ftell(HDR.FILE.FID)-HDR.HeadLen); 
		if (!(pos%HDR.AS.bpb) & (HDR.NRec<0)) 
		{	HDR.NRec = pos/HDR.AS.bpb;
			fseek(HDR.FILE.FID,236,SEEK_SET); 
			if (HDR.TYPE==GDF)
				fwrite(&HDR.NRec,sizeof(HDR.NRec),1,HDR.FILE.FID);
			else {
				len = sprintf(tmp,"%i",HDR.NRec);
				if (len>8) fprintf(stderr,"Warning: NRec is (%s) to long.\n",tmp);  
				fwrite(tmp,len,1,HDR.FILE.FID);
			}	
		};	
	
fprintf(stdout,"%i %i\n" , sizeof(*HDR.EVENT.POS),sizeof(*HDR.EVENT.TYP));

		// WRITE EVENTTABLE 
		if ((HDR.TYPE==GDF) & (HDR.EVENT.N>0)) {
			fseek(HDR.FILE.FID, HDR.HeadLen + HDR.AS.bpb*HDR.NRec, SEEK_SET); 
			flag = (HDR.EVENT.DUR != NULL) & (HDR.EVENT.CHN != NULL); 
			if (flag)   // any DUR or CHN is larger than 0 
				for (k=0, flag=0; (k<HDR.EVENT.N) & !flag; k++)
					flag |= HDR.EVENT.CHN[k] | HDR.EVENT.DUR[k];
			tmp[0] = (flag ? 3 : 1);
			memcpy(tmp+1, &HDR.EVENT.SampleRate, 3);
			memcpy(tmp+4, &HDR.EVENT.N, 4);
			fwrite(tmp, 8, 1, HDR.FILE.FID);
			fwrite(HDR.EVENT.POS, sizeof(*HDR.EVENT.POS), HDR.EVENT.N, HDR.FILE.FID);
			fwrite(HDR.EVENT.TYP, sizeof(*HDR.EVENT.TYP), HDR.EVENT.N, HDR.FILE.FID);
			if (tmp[0]>1) {
				fwrite(HDR.EVENT.CHN,sizeof(*HDR.EVENT.CHN),HDR.EVENT.N,HDR.FILE.FID);
				fwrite(HDR.EVENT.DUR,sizeof(*HDR.EVENT.DUR),HDR.EVENT.N,HDR.FILE.FID);
			}	
		}
	}		

	fclose(HDR.FILE.FID);
    	HDR.FILE.FID = 0;
    	if (HDR.buffer!=NULL)	
        	free(HDR.buffer);
    	if (HDR.CHANNEL!=NULL)	
        	free(HDR.CHANNEL);
    	if (HDR.AS.bi!=NULL)	
        	free(HDR.AS.bi);

    	if (HDR.EVENT.POS!=NULL)	
        	free(HDR.EVENT.POS);
    	if (HDR.EVENT.TYP!=NULL)	
        	free(HDR.EVENT.TYP);
    	if (HDR.EVENT.DUR!=NULL)	
        	free(HDR.EVENT.DUR);
    	if (HDR.EVENT.CHN!=NULL)	
        	free(HDR.EVENT.CHN);
        	
        HDR.EVENT.N = 0; 
	HDR.FILE.OPEN = 0; 	     	

    	return(HDR);
};


/****************************************************************************/
/**                                                                        **/
/**                        MAIN FUNCTION                                   **/
/**                                                                        **/
/****************************************************************************/
 
int main (int argc, char **argv)
{
/*
	currently, its used for testing SOPEN, SREAD, SWRITE, SEOF, STELL, SCLOSE, SSEEK
*/

#define NELEM (1<<15)
	unsigned k; 	
    	short s[NELEM];
    	HDRTYPE HDR, HDR2;
    	size_t count;

	if (argc < 2)  	{
		fprintf(stderr,"Warning: Invalid number of arguments\n");
		return(-1);
      	}	
	
	// initialize/generate signal 
	for (k=0; k<NELEM; s[k] = k++%2000);	
      
	// write: define header
	HDR = init_default_hdr(HDR,4,10);  // allocate memory for 4 channels, 10 events 
	HDR.Patient.Id = "test1";
	HDR.TYPE = GDF; 


	// OPEN and WRITE GDF FILE 

     	HDR   = sopen(argv[1], HDR, "w");

	swrite(&s, NELEM/HDR.NS, &HDR);

	// define events before SCLOSE; 
	for (k=0; k<HDR.EVENT.N; k++) {
		HDR.EVENT.TYP[k] = k+1;
		HDR.EVENT.POS[k] = k*100;
	};
      	HDR = sclose(HDR);


   	fprintf(stdout,"1-%i\t%i\t%i\t%i\t%u\t%u\n",sizeof(HDR.EVENT.TYP),sizeof(*HDR.EVENT.TYP),(int32_t)HDR.NRec,HDR.HeadLen,HDR.Dur[0],HDR.Dur[1]);

	// READ GDF FILE 
	HDR2 = sopen(argv[1], HDR2, "r");
   	fprintf(stdout,"2-%i\t%i\t%i\t%i\t%u\t%u\n",HDR2.AS.bpb,HDR2.FILE.OPEN,(int32_t)HDR2.NRec,HDR2.HeadLen,HDR2.Dur[0],HDR2.Dur[1]);

	while (!seof(HDR2)) {
		count = sread(&HDR2,10);
	
		fprintf(stdout,"+ %lu\t %lu\t %lu\t %u  %i\n",HDR2.NRec,count,44,*(int16_t*)HDR2.buffer,seof(HDR2));
	}	
	sseek(&HDR2,50,SEEK_SET);
fprintf(stdout,"3+ %lu\t %u\n",HDR2.FILE.POS,*(int16_t*)HDR2.buffer);	
	srewind(&HDR2);
fprintf(stdout,"4+ %lu\t %u\n",HDR2.FILE.POS,*(int16_t*)HDR2.buffer);	
	count = sread(&HDR2,10);
fprintf(stdout,"5+ %lu\t %u\n",HDR2.FILE.POS,*(int16_t*)HDR2.buffer);	
	count = sread(&HDR2,10);
fprintf(stdout,"+ %lu\t %u\n", HDR2.FILE.POS,*(int16_t*)HDR2.buffer);	
	count = sread(&HDR2,10);
//   	fprintf(stdout,"3-%i\t%i\t%i\t%i\t%u\t%u\n",HDR2.AS.bpb,HDR2.AS.spb,(long)HDR2.NRec,HDR2.HeadLen,HDR2.T0[0],HDR2.T0[1]);
	HDR2 = sclose(HDR2);
	
      	return(0);

};


/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/
