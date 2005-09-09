/*
%
% $Id: biosig.c,v 1.3 2005-09-09 14:35:15 schloegl Exp $
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

	reading and writing of GDF files is demonstrated 
	
	Features: 
	- reading a GDF2.0pre file (fixed and variable header, data and eventtable)  
	- writing a GDF2.0pre file (fixed and variable header, data and eventtable)  

	- reading fixed header of EDF, BDF and GDF1.x files 
   
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "biosig.h"

/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/


/****************************************************************************/
/**                     INIT HDR                                           **/
/****************************************************************************/
/* 
	The purpose is to set the define all parameters at an initial step. 
	No undefine parameters should remain.
 */
HDRTYPE init_default_hdr(HDRTYPE HDR, const unsigned NS, const unsigned N_EVENT)
{
    	int k,k1;
    	time_t t0; 
    	double T0; 

      	HDR.TYPE = GDF; 
      	HDR.VERSION = 1.91; 
      	HDR.NRec = -1; 
      	HDR.NS = NS;	
      	HDR.PID = "test1"; 
      	HDR.RID = "GRAZ"; 
      	t0 = time(NULL); 	// number of seconds since 01-Jan-1970
      	T0 = t0/(3600.0*24);
      	HDR.T0[1] = floor(T0) + 719529;			// number of days since 01-Jan-0000
      	HDR.T0[0] = floor(ldexp(T0-floor(T0),32));  	// fraction x/2^32; one day is 2^32 
	HDR.Patient.Birthday[0] = 0;
	HDR.Patient.Birthday[1] = 0;
      	HDR.Patient.Medication = 0;
      	HDR.Patient.DrugAbuse = 0;
      	HDR.Patient.AlcoholAbuse = 0;
      	HDR.Patient.Smoking = 0;
      	HDR.Patient.Gender = 0;
      	HDR.Patient.Handedness = 0;
      	HDR.Patient.Impairment.Visual = 0;
      	HDR.Patient.Weight = 0;
      	HDR.Patient.Height = 0;
      	HDR.Dur[0] = 1;
      	HDR.Dur[1] = 1;
	memset(&HDR.IPaddr,0,6);
      	for (k1=0; k1<3; k1++) {
      		HDR.Headsize[k1] = 0;
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
{
    	int k, c[4];
    	FILE* fid;
    	unsigned int count,len;
    	char tmp[81];
    	//unsigned long int Dur[2];
    	double Dur; 
	char* Header1;
	char* Header2;
	struct tm tm_time; 
    	time_t t0; 
    	double T0; 

if (!strcmp(MODE,"r"))	
{
	fid = fopen(FileName,"rb");
    
    	if (fid == NULL) 
    	{ 	fprintf(stdout,"ERROR %s not found\n",FileName);
		return(HDR);
    	}	    
    
    	HDR.FILE.FID = fid; 
 	Header1 = malloc(256);
    
    	/******** read 1st (fixed)  header  *******/	
    	count=fread(Header1,1,256,fid);
    
    	if (0);
    	else if (!memcmp(Header1+1,"BIOSEMI",7))
	    	HDR.TYPE = BDF;
    	else if (!memcmp(Header1,"Version 3.0",11))
	    	HDR.TYPE = CNT;
    	else if (!memcmp(Header1,"DEMG",4))
	    	HDR.TYPE = DEMG;
    	else if (!memcmp(Header1,"0       ",8))
	    	HDR.TYPE = EDF;
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
	    	HDR.HeadLen 	= *(long int*) (Header1+184); 
	    	HDR.NRec 	= *(long int*) (Header1+236); 
	    	HDR.Dur[0]  	= *(unsigned long int*) (Header1+244);
	    	HDR.Dur[1]  	= *(unsigned long int*) (Header1+248); 
	    	HDR.NS   	= *(unsigned long int*) (Header1+252); 
	    	
	    	if (HDR.VERSION>1.90) { 
	    		HDR.Patient.Smoking      = Header1[84]%4;
	    		HDR.Patient.AlcoholAbuse = (Header1[84]>>2)%4;
	    		HDR.Patient.DrugAbuse  	 = (Header1[84]>>4)%4;
	    		HDR.Patient.Medication   = (Header1[84]>>8)%4;
	    		HDR.Patient.Weight       = Header1[85];
	    		HDR.Patient.Height       = Header1[86];
	    		HDR.Patient.Gender       = Header1[87]%4;
	    		HDR.Patient.Handedness   = (Header1[87]>>2)%4;
	    		HDR.Patient.Impairment.Visual = (Header1[87]>>4)%4;

			if (Header1[156]) {
				HDR.LOC[0] = 0x00292929;
				memcpy(&HDR.LOC[1], Header1+152,12);
			}
			else {
				memcpy(&HDR.LOC, Header1+152,16);
			}
			memcpy(&HDR.T0, Header1+168,8);
			memcpy(&HDR.Patient.Birthday, Header1+176,8);

			memcpy(&HDR.IPaddr, Header1+200,6);
			memcpy(&HDR.Headsize, Header1+206,6);
			memcpy(&HDR.ELEC.REF, Header1+212,12);
			memcpy(&HDR.ELEC.GND, Header1+224,12);
	    	}
	    	else {
	    		memcpy(tmp,Header1+168,14);
	    		tmp[14]=0;
	    		tm_time.tm_sec = atoi(tmp+12); 
	    		tmp[12]=0;
	    		tm_time.tm_min = atoi(tmp+10); 
	    		tmp[10]=0;
	    		tm_time.tm_hour = atoi(tmp+8); 
	    		tmp[8]=0;
	    		tm_time.tm_mday = atoi(tmp+6); 
	    		tmp[6]=0;
	    		tm_time.tm_mon = atoi(tmp+4); 
	    		tmp[4]=0;
	    		tm_time.tm_year = atoi(tmp); 
	    		t0 = mktime(&tm_time);
		      	T0 = t0/(3600.0*24);
		      	HDR.T0[1] = floor(T0) + 719529;			// number of days since 01-Jan-0000
		      	HDR.T0[0] = floor(ldexp(T0-floor(T0),32));  	// fraction x/2^32; one day is 2^32 
	    	}
    	}
    	else if ((HDR.TYPE == EDF) | (HDR.TYPE == BDF))	{
	    	HDR.HeadLen 	= atoi(strncpy(tmp,Header1+184,8));
	    	HDR.NRec 	= atoi(strncpy(tmp,Header1+236,8));
	    	Dur 		= atoi(strncpy(tmp,Header1+244,8));
	    	HDR.NS 		= atoi(strncpy(tmp,Header1+252,4));
		if (!Dur)
		{	
			HDR.Dur[0] = Dur; 
			HDR.Dur[1] = 1; 
		}


		// HDR.T0 
    		strncpy(tmp,Header1+168,16);
    		tm_time.tm_sec = atoi(tmp+14); 
    		tmp[13]=0;
    		tm_time.tm_min = atoi(tmp+11); 
    		tmp[10]=0;
    		tm_time.tm_hour = atoi(tmp+8); 
    		tmp[8]=0;
    		tm_time.tm_year = atoi(tmp+6); 
    		tm_time.tm_year += (tm_time.tm_year<85)*100;
    		tmp[5]=0;
    		tm_time.tm_mon = atoi(tmp+3)-1; 
    		tmp[2]=0;
    		tm_time.tm_mday = atoi(tmp); 
    		
    		t0 = mktime(&tm_time);
	      	T0 = t0/(3600.0*24);
	      	HDR.T0[1] = floor(T0) + 719529;			// number of days since 01-Jan-0000
	      	HDR.T0[0] = floor(ldexp(T0-floor(T0),32));  	// fraction x/2^32; one day is 2^32 
	      	
	      	if (HDR.TYPE == BDF)
			for (k=0;k<HDR.NS;k++)
			      	HDR.CHANNEL[k].GDFTYP    = 255+24;	// bit24 	
    	}
	else if (HDR.TYPE==BKR) {
	    	Header1 = realloc(Header1,1024);
	    	count   = fread(Header1+256,1,1024-256,fid);
	    	HDR.HeadLen = 1024; 
	}
    	
    	   	
	/* ******* read 2nd (variable)  header  ****** */	
    	//HDR.Header2 = malloc(HDR.NS*256);
    	HDR.CHANNEL = calloc(HDR.NS,sizeof(CHANNEL_TYPE));

	if (HDR.TYPE == GDF) {
	    	Header1 = realloc(Header1,(HDR.NS+1)*256);
	    	Header2 = Header1+256; 
	    	count = fread(Header2,1,256*HDR.NS,fid);
		for (k=0; k<HDR.NS;k++)	{
			//HDR.CHANNEL[k].Label  = (HDR.Header2 + 16*k);
			//HDR.CHANNEL[k].Transducer  = (HDR.Header2 + 80*k + 16*HDR.NS);
			//HDR.CHANNEL[k].PhysDim  = (HDR.Header2 + 8*k + 96*HDR.NS);
			HDR.CHANNEL[k].PhysMin  = *(float*) (Header2+ 8*k + 104*HDR.NS);
			HDR.CHANNEL[k].PhysMax  = *(float*) (Header2+ 8*k + 112*HDR.NS);
			HDR.CHANNEL[k].DigMin   = *(long long*) (Header2+ 8*k + 120*HDR.NS);
			HDR.CHANNEL[k].DigMax   = *(long long*) (Header2+ 8*k + 128*HDR.NS);
			//HDR.CHANNEL[k].PreFilt  = (HDR.Header2+ 68*k + 136*HDR.NS);
			HDR.CHANNEL[k].SPR      = *(unsigned long*) (Header2+ 4*k + 216*HDR.NS);
			HDR.CHANNEL[k].GDFTYP   = *(unsigned long*) (Header2+ 4*k + 220*HDR.NS);
			if (HDR.VERSION>=1.90) {
				HDR.CHANNEL[k].LowPass  = *(float*) (Header2+ 4*k + 204*HDR.NS);
				HDR.CHANNEL[k].HighPass = *(float*) (Header2+ 4*k + 208*HDR.NS);
				HDR.CHANNEL[k].Notch    = *(float*) (Header2+ 4*k + 212*HDR.NS);
				memcpy(&HDR.CHANNEL[k].XYZ,Header2+ 4*k + 224*HDR.NS,12);
				HDR.CHANNEL[k].Impedance= ldexp(1,Header2[k + 236*HDR.NS]/8.0);
			};
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
		fseek(HDR.FILE.FID,HDR.HeadLen + HDR.AS.bpb*HDR.NRec, SEEK_SET); 
		fread(tmp,sizeof(char),8,HDR.FILE.FID);
		HDR.EVENT.SampleRate = 0; 
		memcpy(&HDR.EVENT.SampleRate,tmp+1,3);
		memcpy(&HDR.EVENT.N,tmp+4,4);
		HDR.EVENT.POS = calloc(HDR.EVENT.N,sizeof(*HDR.EVENT.POS));
		HDR.EVENT.TYP = calloc(HDR.EVENT.N,sizeof(*HDR.EVENT.TYP));
		c[0]=fread(HDR.EVENT.POS,sizeof(*HDR.EVENT.POS),HDR.EVENT.N,HDR.FILE.FID);
		c[1]=fread(HDR.EVENT.TYP,sizeof(*HDR.EVENT.TYP),HDR.EVENT.N,HDR.FILE.FID);
		if (tmp[0]>1) {
			HDR.EVENT.DUR = calloc(HDR.EVENT.N,sizeof(*HDR.EVENT.DUR));
			HDR.EVENT.CHN = calloc(HDR.EVENT.N,sizeof(*HDR.EVENT.CHN));
			fread(HDR.EVENT.CHN,sizeof(*HDR.EVENT.CHN),HDR.EVENT.N,HDR.FILE.FID);
			fread(HDR.EVENT.DUR,sizeof(*HDR.EVENT.DUR),HDR.EVENT.N,HDR.FILE.FID);
		}
		else {
			HDR.EVENT.DUR = NULL;
			HDR.EVENT.CHN = NULL;
		}
		fseek(HDR.FILE.FID,HDR.HeadLen, SEEK_SET); 
		HDR.FILE.OPEN = 2; 	     	
		HDR.FILE.POS  = 0; 	     	
			
        }   //  if GDF	
}
else { // WRITE

    	if (HDR.TYPE==GDF)  	{	
	    	Header1 = calloc(sizeof(char),(HDR.NS+1)*256);
	    	Header2 = Header1+256; 

	     	memcpy(Header1,"GDF 1.91",8);
	     	len = strlen(HDR.PID);
	     	memcpy(Header1+8,HDR.PID,(len<66?len:66));
	     	len = strlen(HDR.RID);
	     	Header1[84] = (HDR.Patient.Smoking%4) + ((HDR.Patient.AlcoholAbuse%4)<<2) + ((HDR.Patient.DrugAbuse%4)<<4) + ((HDR.Patient.Medication%4)<<6);
	     	Header1[85] = HDR.Patient.Weight;
	     	Header1[86] = HDR.Patient.Height;
	     	Header1[87] = (HDR.Patient.Gender%4) + ((HDR.Patient.Handedness%4)<<2) + ((HDR.Patient.Impairment.Visual%4)<<4);
	     	memcpy(Header1+88,HDR.RID,(len<80?len:80));
		memcpy(Header1+168,&HDR.T0,8); 
		memcpy(Header1+176,&HDR.Patient.Birthday,8); 
	     	HDR.HeadLen = (HDR.NS+1)<<8;
		memcpy(Header1+184,&HDR.HeadLen,4); 
		//memcpy(HDR.Header1+192,"BIOSIG4C",8);  // include when tested 
		memcpy(Header1+192,"b4c 0.10",8);    // include when tested 
		memcpy(Header1+236,&HDR.NRec,4);
		memcpy(Header1+244,&HDR.Dur,8); 
		memcpy(Header1+252,&HDR.NS,4); 
		
	     	/* define HDR.Header2 
	     	this requires checking the arguments in the fields of the struct HDR.CHANNEL
	     	and filling in the bytes in HDR.Header2. 
	     	*/
		for (k=0;k<HDR.NS;k++)
		{
		     	len = strlen(HDR.CHANNEL[k].Label);
		     	memcpy(Header2+16*k,HDR.CHANNEL[k].Label,((len<16) ? len : 16));
		     	len = strlen(HDR.CHANNEL[k].Transducer);
		     	memcpy(Header2+80*k + 16*HDR.NS,HDR.CHANNEL[k].Transducer,((len < 80) ? len : 80));
		     	len = strlen(HDR.CHANNEL[k].PhysDim);
		     	memcpy(Header2+ 8*k + 96*HDR.NS,HDR.CHANNEL[k].PhysDim,(len<8?len:8));
	
		     	memcpy(Header2+ 8*k + 104*HDR.NS,&HDR.CHANNEL[k].PhysMin,8);
		     	memcpy(Header2+ 8*k + 112*HDR.NS,&HDR.CHANNEL[k].PhysMax,8);
		     	memcpy(Header2+ 8*k + 120*HDR.NS,&HDR.CHANNEL[k].DigMin,8);
		     	memcpy(Header2+ 8*k + 128*HDR.NS,&HDR.CHANNEL[k].DigMax,8);
		     	memcpy(Header2+ 4*k + 204*HDR.NS,&HDR.CHANNEL[k].LowPass,4);
		     	memcpy(Header2+ 4*k + 208*HDR.NS,&HDR.CHANNEL[k].HighPass,4);
		     	memcpy(Header2+ 4*k + 212*HDR.NS,&HDR.CHANNEL[k].LowPass,4);
		     	memcpy(Header2+ 4*k + 216*HDR.NS,&HDR.CHANNEL[k].SPR,4);
		     	memcpy(Header2+ 4*k + 220*HDR.NS,&HDR.CHANNEL[k].GDFTYP,4);
		     	memcpy(Header2+12*k + 224*HDR.NS,&HDR.CHANNEL[k].XYZ,12);
	
			if (HDR.CHANNEL[k].Impedance>39e8)
				tmp[0]=255;
			else 	
		     		tmp[0] = ceil(log10(HDR.CHANNEL[k].Impedance)/log10(2.0)*8.0-0.5);
		     	Header2[k+236*HDR.NS] = tmp[0];
		     	
		}
	    	HDR.AS.bi = calloc(HDR.NS+1,sizeof(long));
		HDR.AS.bi[0] = 0;
		for (k=0, HDR.AS.spb=0, HDR.AS.bpb=0; k<HDR.NS;)
		{
			HDR.AS.spb += HDR.CHANNEL[k].SPR;
			HDR.AS.bpb += GDFTYP_BYTE[HDR.CHANNEL[k].GDFTYP]*HDR.CHANNEL[k].SPR;
			HDR.AS.bi[++k] = HDR.AS.bpb; 
		}	
	
	
	    	HDR.FILE.FID = fopen(FileName,"wb");
	    
	    	if (HDR.FILE.FID == NULL) 
	    	{ 	fprintf(stdout,"ERROR %s not found\n",FileName);
			return(HDR);
	    	}	    
	    	
	    	fwrite(Header1,sizeof(char),256,HDR.FILE.FID);
	    	fwrite(Header2,sizeof(char),256*HDR.NS,HDR.FILE.FID);
		HDR.FILE.OPEN = 2; 	     	
		HDR.FILE.POS  = 0; 	
		
	}	// end of if
	else if (HDR.TYPE==BKR) {
	    	Header1 = malloc(1024);
	    	HDR.HeadLen = 1024; 

	}
}	// end of else 
	
	
	return(HDR);
}  // end of SOPEN 


/****************************************************************************/
/**                     SCLOSE                                             **/
/****************************************************************************/
HDRTYPE sclose(HDRTYPE HDR)
{
	long pos, k; 
	unsigned char tmp[8]; 
	char flag; 
	
	if ((HDR.NRec<0) & (HDR.FILE.OPEN>1))
	{
		// WRITE HDR.NRec 
		pos = (ftell(HDR.FILE.FID)-HDR.HeadLen); 
		if (!(pos%HDR.AS.bpb) & (HDR.NRec<0)) 
		{	HDR.NRec = pos/HDR.AS.bpb;
			fseek(HDR.FILE.FID,236,SEEK_SET); 
			fwrite(&HDR.NRec,sizeof(HDR.NRec),1,HDR.FILE.FID);
		};	
		// WRITE EVENTTABLE 
		if (HDR.EVENT.N>0) {
			fseek(HDR.FILE.FID,HDR.HeadLen + HDR.AS.bpb*HDR.NRec, SEEK_SET); 
			flag = (HDR.EVENT.DUR!=NULL) & (HDR.EVENT.CHN != NULL); 
			if (flag)	// any DUR or CHN is larger than 0 
				for (k=0, flag=0; (k<HDR.EVENT.N) & !flag; k++)
					flag |= HDR.EVENT.DUR[k] | HDR.EVENT.DUR[k];
			tmp[0] = ( flag ? 3 : 1);
			memcpy(tmp+1,&HDR.EVENT.SampleRate,3);
			memcpy(tmp+4,&HDR.EVENT.N,4);
			fwrite(tmp,sizeof(tmp),1,HDR.FILE.FID);
			fwrite(HDR.EVENT.POS,sizeof(*HDR.EVENT.POS),HDR.EVENT.N,HDR.FILE.FID);
			fwrite(HDR.EVENT.TYP,sizeof(*HDR.EVENT.TYP),HDR.EVENT.N,HDR.FILE.FID);
			if (tmp[0]>1) {
				fwrite(HDR.EVENT.CHN,sizeof(*HDR.EVENT.CHN),HDR.EVENT.N,HDR.FILE.FID);
				fwrite(HDR.EVENT.DUR,sizeof(*HDR.EVENT.DUR),HDR.EVENT.N,HDR.FILE.FID);
			}	
		}
	}		

    	fclose(HDR.FILE.FID);
    	HDR.FILE.FID = 0;
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

#define NELEM (1<<15)
	unsigned k, count; 	
    	short s[NELEM];
    	HDRTYPE HDR, HDR2;
    
    if (argc < 2) 
      	{	fprintf(stdout,"Warning: Invalid number of arguments\n");
		return(-1);
      	}	

	
	// initialize/generate signal 
	for (k=0; k<NELEM; s[k] = k++%2000);	
      
	// write: define fixed header
	HDR = init_default_hdr(HDR,4,10);
	// OPEN and WRITE GDF FILE 
      	HDR = sopen(argv[1], HDR,"w");
      	count = fwrite(s,sizeof(short),NELEM,HDR.FILE.FID);     
	
	// define events before SCLOSE; 
	for (k=0; k<HDR.EVENT.N; k++) {
		HDR.EVENT.TYP[k] = k+1;
		HDR.EVENT.POS[k] = k*100;
	};
      	HDR = sclose(HDR);

//   	fprintf(stdout,"1-%i\t%i\t%i\t%i\t%u\t%u\n",sizeof(HDR.EVENT.TYP),sizeof(*HDR.EVENT.TYP),(long)HDR.NRec,HDR.HeadLen,HDR.Dur[0],HDR.Dur[1]);
      

	// READ GDF FILE 
	HDR2 = init_default_hdr(HDR2,0,0);
	HDR2 = sopen(argv[1], HDR2,"r");
      	HDR2 = sclose(HDR2);

//   	fprintf(stdout,"3-%i\t%i\t%i\t%i\t%u\t%u\n",HDR2.AS.bpb,HDR2.AS.spb,(long)HDR2.NRec,HDR2.HeadLen,HDR2.T0[0],HDR2.T0[1]);
	
      	return(0);

};


/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/
