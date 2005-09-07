/*
%
% $Id: biosig.c,v 1.2 2005-09-07 17:08:21 schloegl Exp $
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
	- reading fixed header of GDF, EDF, BDF
	- writing fixed an variable header of GDF2.0 except for HDR.T0, HDR.Impedance
   
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "biosig.h"


HDRTYPE sopen(const char* FileName, HDRTYPE HDR, const char* MODE)
{

    	int k,l,m;
    	FILE* fid;
    	unsigned int count,len;
    	char tmp[81];
    	//unsigned long int Dur[2];
    	double Dur; 

if (!strcmp(MODE,"r"))	
{
	fid = fopen(FileName,"rb");
    
    	if (fid == NULL) 
    	{ 	fprintf(stdout,"ERROR %s not found\n",FileName);
		return(HDR);
    	}	    
    
    	HDR.fid = fid; 
    
    	/******** read 1st (fixed)  header  *******/	
    	count=fread(HDR.Header1,1,256,fid);
    
    	if (!memcmp(HDR.Header1,"0       ",8))
	{
	    	HDR.VERSION = 0.0; 
	    	HDR.TYPE = EDF;
	}
    	else if (!memcmp(HDR.Header1+1,"BIOSEMI",7))
	{
	    	HDR.VERSION = -1.0; 
	    	HDR.TYPE = BDF;
	}
    	else if (!memcmp(HDR.Header1,"GDF",3))
	{
	    	HDR.TYPE = GDF; 
  	    	strncpy(tmp,(char*)HDR.Header1+3,5);
	    	HDR.VERSION = atof(tmp);
	};

    	if (HDR.TYPE == GDF)  /* GDF */
    	{
	    	HDR.HeadLen = *(long int*) (HDR.Header1+184); 
	    	HDR.NRec = *(long int*) (HDR.Header1+236); 
	    	//HDR.Dur  = (*(unsigned long int*) (HDR.Header1+244)) / (*(unsigned long int*) (HDR.Header1+248)); 
	    	HDR.Dur[0]  = *(unsigned long int*) (HDR.Header1+244); // (*(unsigned long int*) (HDR.Header1+248)); 
	    	HDR.Dur[1]  = *(unsigned long int*) (HDR.Header1+248); 
	   	//HDR.Dur = ((double)Dur[0])/((double)Dur[1]);

	    	HDR.NS   = *(unsigned long int*) (HDR.Header1+252); 
    	}
    	else if ((HDR.TYPE == EDF) | (HDR.TYPE == BDF))	/* EDF, BDF */
    	{
  	    	strncpy(tmp,(char*)HDR.Header1+184,8);
	    	HDR.HeadLen = atoi(tmp);
  	    	strncpy(tmp,(char*)HDR.Header1+236,8);
	    	HDR.NRec = atoi(tmp);
  	    	strncpy(tmp,(char*)HDR.Header1+244,8);
	    	Dur = atoi(tmp);
		if (!Dur)
		{	
			HDR.Dur[0] = Dur; 
			HDR.Dur[1] = 1; 
		}
		else 
			error();

  	    	strncpy(tmp,(char*)HDR.Header1+252,4);
	    	HDR.NS = atoi(tmp);
    	}
        HDR.PID = HDR.Header1+8;
        HDR.RID = HDR.Header1+88;
    
    	HDR.Header2 = malloc(HDR.NS*256);
    	HDR.CHANNEL = calloc(HDR.NS,sizeof(CHANNEL_TYPE));
    	count=fread(HDR.Header2,1,256*HDR.NS,fid);

	for (k=0; k<HDR.NS;k++)
	{
		//HDR.CHANNEL[k].Label  = (HDR.Header2 + 16*k);
		//HDR.CHANNEL[k].Transducer  = (HDR.Header2 + 80*k + 16*HDR.NS);
		//HDR.CHANNEL[k].PhysDim  = (HDR.Header2 + 8*k + 96*HDR.NS);
		HDR.CHANNEL[k].PhysMin  = *(float*) (HDR.Header2+ 8*k + 104*HDR.NS);
		HDR.CHANNEL[k].PhysMax  = *(float*) (HDR.Header2+ 8*k + 112*HDR.NS);
		HDR.CHANNEL[k].DigMin   = *(long long*) (HDR.Header2+ 8*k + 120*HDR.NS);
		HDR.CHANNEL[k].DigMax   = *(long long*) (HDR.Header2+ 8*k + 128*HDR.NS);
		//HDR.CHANNEL[k].PreFilt  = (HDR.Header2+ 68*k + 136*HDR.NS);
		HDR.CHANNEL[k].LowPass  = *(float*) (HDR.Header2+ 4*k + 204*HDR.NS);
		HDR.CHANNEL[k].HighPass = *(float*) (HDR.Header2+ 4*k + 208*HDR.NS);
		HDR.CHANNEL[k].Notch    = *(float*) (HDR.Header2+ 4*k + 212*HDR.NS);
		HDR.CHANNEL[k].SPR      = *(unsigned long*) (HDR.Header2+ 4*k + 216*HDR.NS);
		HDR.CHANNEL[k].GDFTYP   = *(unsigned long*) (HDR.Header2+ 4*k + 220*HDR.NS);
	}	
        
    	/* ******* read 2nd (variable)  header  ****** */	
}
else // WRITE
{
    	if (HDR.TYPE==GDF)
     	{	
     	/* 
     	define HDR.Header1 
     	this requires checking the arguments in the fields of the struct HDR
     	and filling in the bytes in HDR.Header1. 
     	
     	argument checking is crucial and MUST be implemented here. 
     	*/
     	memset(HDR.Header1,0,256);
     	memcpy(HDR.Header1,"GDF 1.91",8);
     	len = strlen(HDR.PID);
     	memcpy(HDR.Header1+8,HDR.PID,(len<66?len:66));
     	len = strlen(HDR.RID);
     	HDR.Header1[84] = (HDR.Smoking%4) + (HDR.AlcoholAbuse%4)<<2 + (HDR.DrugAbuse%4)<<4 + (HDR.Medication%4)<<6;
     	HDR.Header1[85] = HDR.Weight;
     	HDR.Header1[86] = HDR.Height;
     	HDR.Header1[87] = (HDR.Gender%4) + (HDR.Handedness%4)<<2 + (HDR.VisualImpairment%4)<<4;
     	memcpy(HDR.Header1+88,HDR.RID,(len<80?len:80));
	memcpy(HDR.Header1+168,&HDR.T0,8); 
	memcpy(HDR.Header1+176,&HDR.Birthday,8); 
     	HDR.HeadLen = (HDR.NS+1)<<8;
	memcpy(HDR.Header1+184,&HDR.HeadLen,4); 
	//memcpy(HDR.Header1+192,"BIOSIG4C",8);  // include when tested 
	memcpy(HDR.Header1+192,"testing",7);  // include when tested 
	memcpy(HDR.Header1+236,&HDR.NRec,4);
	memcpy(HDR.Header1+252,&HDR.NS,4); 
	
/*	this needs some work  
	
	if (fmod(HDR.Dur,1.0)==0.0)
	{	Dur[0] = (unsigned long int) HDR.Dur;
		Dur[1] = 1;
	}
	else if (fmod(1.0/HDR.Dur,1.0)==0.0) 
	{
		Dur[0] = 1;
		Dur[1] = (unsigned long int)HDR.Dur;
	}
	else  
	{
	}
*/
	memcpy(HDR.Header1+244,&HDR.Dur,8); 
	

      	HDR.Header2 = malloc(HDR.NS*256);
     	/* define HDR.Header2 
     	this requires checking the arguments in the fields of the struct HDR.CHANNEL
     	and filling in the bytes in HDR.Header2. 
     	*/
	for (k=0;k<HDR.NS;k++)
	{
	     	len = strlen(HDR.CHANNEL[k].Label);
	     	memcpy(HDR.Header2+16*k,HDR.CHANNEL[k].Label,((len<16) ? len : 16));
	     	len = strlen(HDR.CHANNEL[k].Transducer);
	     	memcpy(HDR.Header2+80*k + 16*HDR.NS,HDR.CHANNEL[k].Transducer,((len < 80) ? len : 80));
	     	len = strlen(HDR.CHANNEL[k].PhysDim);
	     	memcpy(HDR.Header2+ 8*k + 96*HDR.NS,HDR.CHANNEL[k].PhysDim,(len<8?len:8));

	     	memcpy(HDR.Header2+ 8*k + 104*HDR.NS,&HDR.CHANNEL[k].PhysMin,8);
	     	memcpy(HDR.Header2+ 8*k + 112*HDR.NS,&HDR.CHANNEL[k].PhysMax,8);
	     	memcpy(HDR.Header2+ 8*k + 120*HDR.NS,&HDR.CHANNEL[k].DigMin,8);
	     	memcpy(HDR.Header2+ 8*k + 128*HDR.NS,&HDR.CHANNEL[k].DigMax,8);
	     	memcpy(HDR.Header2+ 4*k + 204*HDR.NS,&HDR.CHANNEL[k].LowPass,4);
	     	memcpy(HDR.Header2+ 4*k + 208*HDR.NS,&HDR.CHANNEL[k].HighPass,4);
	     	memcpy(HDR.Header2+ 4*k + 212*HDR.NS,&HDR.CHANNEL[k].LowPass,4);
	     	memcpy(HDR.Header2+ 4*k + 216*HDR.NS,&HDR.CHANNEL[k].SPR,4);
	     	memcpy(HDR.Header2+ 4*k + 220*HDR.NS,&HDR.CHANNEL[k].GDFTYP,4);
	     	memcpy(HDR.Header2+12*k + 224*HDR.NS,&HDR.CHANNEL[k].XYZ,12);
//	     	tmp[0] = ceil(log10(HDR.CHANNEL[k].Impedance)/log10(2)*8);
	     	HDR.Header2[k+236*HDR.NS] = 255; //tmp[0];
	     	
	}

    	HDR.fid = fopen(FileName,"wb");
    
    	if (HDR.fid == NULL) 
    	{ 	fprintf(stdout,"ERROR %s not found\n",FileName);
		return(HDR);
    	}	    
    	
    	fwrite(HDR.Header1,sizeof(char),256,HDR.fid);
    	fwrite(HDR.Header2,sizeof(char),256*HDR.NS,HDR.fid);
     	
}	// end of ELSE
}	// end of IF 

    	HDR.bi = calloc(HDR.NS+1,sizeof(long));
	HDR.bi[0] = 0;
	for (k=0, HDR.spb=0, HDR.bpb=0; k<HDR.NS;)
	{
		HDR.spb += HDR.CHANNEL[k].SPR;
		HDR.bpb += GDFTYP_BYTE[HDR.CHANNEL[k].GDFTYP-1]*HDR.CHANNEL[k].SPR;
		HDR.bi[++k] = HDR.bpb; 
	}	

return(HDR);
}  // end of SOPEN 


HDRTYPE sclose(HDRTYPE HDR)
{
	long pos; 
	if (HDR.NRec<0)
	{
		pos = (ftell(HDR.fid)-HDR.HeadLen); 
		if (!(pos%HDR.bpb))
		{	HDR.NRec = pos/HDR.bpb;
			fseek(HDR.fid,236,SEEK_SET); 
			fwrite(&HDR.NRec,sizeof(HDR.NRec),1,HDR.fid);
		};	
	}		

    	fclose(HDR.fid);
    	HDR.fid=0;
    	if (HDR.Header2!=NULL)	
        	free(HDR.Header2);
    	if (HDR.CHANNEL!=NULL)	
        	free(HDR.CHANNEL);
    	if (HDR.bi!=NULL)	
        	free(HDR.bi);
    	return(HDR);
};


 
int main (int argc, char **argv)
{

#define NELEM (1<<15)
    //short *block;
    int count,k,k1;
    time_t T0; 
    short s[NELEM];
    HDRTYPE HDR,HDR2;
    
    if (argc < 2) 
      	{	fprintf(stdout,"Warning: Invalid number of arguments\n");
		return(-1);
      	}	

	// write: define header structure 
      	HDR.TYPE = GDF; 
      	HDR.VERSION = 1.91; 
      	HDR.NRec = -1; 
      	HDR.NS = 4;	
      	HDR.PID = "test1"; 
      	HDR.RID = "GRAZ"; 
      	T0 = time(NULL)/(3600.0*24); 
	HDR.Birthday[0] = 0;
	HDR.Birthday[1] = 0;
	HDR.T0[1] = 719529;
/*      	HDR.T0[1] = floor(T0) + 719529;
      	HDR.T0[0] = floor((T0-floor(T0)) * ((long long)1<<32));
*/
      	HDR.Dur[0] = 1;
      	HDR.Dur[1] = 250;
	HDR.CHANNEL = calloc(HDR.NS,sizeof(CHANNEL_TYPE));
	for (k=0;k<HDR.NS;k++)
	{
      	HDR.CHANNEL[k].Label = "C4";
      	HDR.CHANNEL[k].Transducer = "EEG: Ag-AgCl electrodes";
      	HDR.CHANNEL[k].PhysDim = "uV";
      	HDR.CHANNEL[k].PhysMax = 100;
      	HDR.CHANNEL[k].PhysMin = -100;
      	HDR.CHANNEL[k].DigMax = 2047;
      	HDR.CHANNEL[k].DigMin = -2048;
      	HDR.CHANNEL[k].GDFTYP = 3;	// int16 
      	HDR.CHANNEL[k].SPR    = 1;
      	HDR.CHANNEL[k].HighPass = 0.16;
      	HDR.CHANNEL[k].LowPass = 70.0;
      	HDR.CHANNEL[k].Notch = 50;
      	HDR.CHANNEL[k].Impedance = 1.0/0.0;
      	for (k1=0; k1<3; HDR.CHANNEL[k].XYZ[k1++] = 0.0);
	}      	
       	// define fields of HDR.CHANNEL here 
      
      	for (k=0; k<NELEM; s[k]=k++);

   	fprintf(stdout,"1-%f\t%i\t%i\t%i\t%u\t%u\n",HDR.VERSION,HDR.NS,(long)HDR.NRec,HDR.HeadLen,HDR.Dur[0],HDR.Dur[1]);
      
      	HDR = sopen(argv[1], HDR,"w");
      	count = fwrite(s,sizeof(short),NELEM,HDR.fid);     
      	HDR = sclose(HDR);

   	fprintf(stdout,"2-%f\t%i\t%i\t%i\t%u\t%u\n",HDR.VERSION,HDR.NS,(long)HDR.NRec,HDR.HeadLen,HDR.Dur[0],HDR.Dur[1]);


	// read: 
      

	HDR2 = sopen(argv[1], HDR2,"r");
      	HDR2 = sclose(HDR2);
   	fprintf(stdout,"3-%f\t%i\t%i\t%i\t%u\t%u\n",HDR2.VERSION,HDR2.NS,(long)HDR2.NRec,HDR2.HeadLen,HDR2.Dur[0],HDR2.Dur[1]);
//   	fprintf(stdout,"%f\t%i\t%i\t%i\t%f\n",HDR.VERSION,HDR.NS,HDR.NRec,HDR.HeadLen,HDR.Dur);
	
      	return;

};
