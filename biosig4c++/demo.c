/*

    $Id: demo.c,v 1.2 2006-03-13 11:18:34 schloegl Exp $
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
	
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "biosig.h"


/****************************************************************************/
/**                                                                        **/
/**                        UTILITY FUNCTIONS                               **/
/**                                                                        **/
/****************************************************************************/
 




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
    	uint16_t 	s[NELEM];
    	FILE	*fid; 
    	HDRTYPE *hdr; 	
    	CHANNEL_TYPE* cp; 
    	size_t 	count;
    	int	status;
    	char 	help[] = "\n Usage of BIOSIG:\n\n\tbiosig -h\t\thelp - this text\n\tbiosig filename\t\tread file if available; if not available generate file\n\n\n";  
	time_t  T0; 
	struct tm t0; 
	
	if (argc < 2)  	{
		fprintf(stderr,"Warning: Invalid number of arguments\n");
		return(-1);
      	}	

	if (!strncmp(argv[1],"-h",2)) {
		fprintf(stdout,"%s",help); 
		return(0); 
	}

	
	// READ FILE 
	fid = fopen(argv[1], "r");
	if (fid==NULL) // file does not exist 
	{ 	
		// initialize/generate signal 
		for (k=0; k<NELEM; s[k] = l_endian_u16(k % 1031),k++);	
      
		// write: define header
		hdr = create_default_hdr(4,10);  // allocate memory for 4 channels, 10 events 
		hdr->Patient.Id = "test1";

	/*
		define target file format 
	 */
		hdr->TYPE = GDF; 		// target file format is GDF
//		hdr->TYPE = SCP_ECG; 		// target file format is SCP 
//		hdr->TYPE = HL7aECG; 		// target file format is HL7aECG
		
		t0.tm_year = 100; 
		t0.tm_mon = 0; 
		t0.tm_mday = 1; 
		t0.tm_hour = 12; 
		t0.tm_min = 0; 
		t0.tm_sec = 0; 
		hdr->Patient.Birthday = 0; //tm_time2gdf_time(&t0);

		// OPEN and WRITE GDF FILE 
	     	sopen(argv[1], "w", hdr);
fprintf(stdout,"** %i\n",ftell(hdr->FILE.FID));

		swrite(&s, NELEM/hdr->NS, hdr);
fprintf(stdout,"** %i\n",ftell(hdr->FILE.FID));

		// define events before SCLOSE; 
		for (k=0; k<hdr->EVENT.N; k++) {
			hdr->EVENT.TYP[k] = k+1;
			hdr->EVENT.POS[k] = k*100;
		};
fprintf(stdout,"** %i\n",ftell(hdr->FILE.FID));
	      	status = sclose(hdr);
	
	   	fprintf(stdout,"1-%i\t%i\t%i\t%i\t%u\t%u\n",sizeof(hdr->EVENT.TYP),sizeof(*hdr->EVENT.TYP),(int32_t)hdr->NRec,hdr->HeadLen,hdr->Dur[0],hdr->Dur[1]);

	}
	else 
	{
		fclose(fid); 
		hdr = sopen(argv[1], "r", NULL);
		if (hdr==NULL)	exit(-1);

		fprintf(stdout,"FileName:\t%s\nType    :\t%i\nVersion:\t%4.2f\nHeadLen:\t%i\n",argv[1],hdr->TYPE,hdr->VERSION,hdr->HeadLen);

		fprintf(stdout,"NS:\t%i\nSPR:\t%i\nNRec:\t%Li\nDuration[s]:\t%u/%u\nFs:\t%f\n",hdr->NS,hdr->SPR,hdr->NRec,hdr->Dur[0],hdr->Dur[1],hdr->SampleRate);
		
		T0 = gdf_time2t_time(hdr->T0);
		fprintf(stdout,"Date/Time:\t%s\n",asctime(localtime(&T0))); 
		//T0 = gdf_time2t_time(hdr->Patient.Birthday);
		//fprintf(stdout,"Birthday:\t%s\n",asctime(localtime(&T0))); 
		
		fprintf(stdout,"Patient:\n\tName:\t%s\n\tId:\t%s\n\tWeigth:\t%i kg\n\tHeigth:\t%i cm\n\tAge:\t%4.1f y\n",hdr->Patient.Name,hdr->Patient.Id,hdr->Patient.Weight,hdr->Patient.Height,(hdr->T0 - hdr->Patient.Birthday)/ldexp(365.25,32)); 
		T0 = gdf_time2t_time(hdr->Patient.Birthday);
		fprintf(stdout,"\tBirthday:\t%s\n",asctime(localtime(&T0))); 
		fprintf(stdout,"EVENT:\n\tN:\t%i\n\tFs:\t%f\n\t\n",hdr->EVENT.N,hdr->EVENT.SampleRate); 
		
		fprintf(stdout,"--%i\t%i\n", hdr->FLAG.OVERFLOWDETECTION, hdr->FLAG.UCAL);
		hdr->FLAG.OVERFLOWDETECTION = 0; 
	//	hdr->FLAG.UCAL = 1;
		fprintf(stdout,"--%i\t%i\n", hdr->FLAG.OVERFLOWDETECTION, hdr->FLAG.UCAL);
	   	fprintf(stdout,"2-%u\t%i\t%i\t%i\t%u\t%u\n",hdr->AS.bpb,hdr->FILE.OPEN,(int32_t)hdr->NRec,hdr->HeadLen,hdr->Dur[0],hdr->Dur[1]);
	
		for (k=0; k<hdr->NS; k++) {
			cp = hdr->CHANNEL+k; 
			fprintf(stdout,"\n#%2i: %7s\t%s\t%s\t%i\t%5f\t%5f\t%5f\t%5f\t",k,cp->Label,cp->Transducer,cp->PhysDim,cp->PhysDimCode,cp->PhysMax,cp->PhysMin,cp->DigMax,cp->DigMin);
			fprintf(stdout,"%4.0f\t%4.0f\t%4.0f\t%5f Ohm",cp->LowPass,cp->HighPass,cp->Notch,cp->Impedance);
		}
/*
		count = sread(hdr,k,10);
fprintf(stdout,"\nm1: %f %f %f %f\n",hdr->data.block[0],hdr->data.block[1],hdr->data.block[2],hdr->data.block[3]);

		for (k=0; !seof(hdr); k+=10) {
//fprintf(stdout,"%i\t%i\t%i\tm2\n",hdr->FILE.POS,hdr->SPR,hdr->NRec);		
			count = sread(hdr,k,10);
//fprintf(stdout,"m3 [%i %i]\n",!seof(hdr),hdr->data.size[0],hdr->data.size[1] );		
//fprintf(stdout,"m1: %f %f %f %f\n",hdr->data.block[0],hdr->data.block[1],hdr->data.block[2],hdr->data.block[3]);
	
		}
		fprintf(stdout,"\n\n+ %Lu\t %u\t %u\t %u %f %i\n",hdr->NRec,count,hdr->FILE.POS,l_endian_i16(*(int16_t*)hdr->AS.rawdata),hdr->data.block[0],seof(hdr));
		sseek(hdr,50,SEEK_SET);
fprintf(stdout,"3+ %u\t %u\n",hdr->FILE.POS,l_endian_i16(*(int16_t*)hdr->AS.rawdata));	
		srewind(hdr);
fprintf(stdout,"4+ %u\t %u\n",hdr->FILE.POS,l_endian_i16(*(int16_t*)hdr->AS.rawdata));	
		count = sread(hdr,50,10);
fprintf(stdout,"++ %i\t %u\t%i\t%i\t%i\t%i\n",hdr->FILE.POS,l_endian_i16(*(int16_t*)hdr->AS.rawdata),count,hdr->data.size[0],hdr->data.size[1],hdr->NS);
fprintf(stdout,"%i\n",ftell(hdr->FILE.FID));
		hdr->CHANNEL[0].OnOff = 0;
		count = sread(hdr,50,10);
fprintf(stdout,"++ %i\t %u\t%i\t%i\t%i\t%i\n",hdr->FILE.POS,l_endian_i16(*(int16_t*)hdr->AS.rawdata),count,hdr->data.size[0],hdr->data.size[1],hdr->NS);
if (count)
fprintf(stdout,"5+ %u\t %u\n",hdr->FILE.POS,l_endian_i16(*(int16_t*)hdr->AS.rawdata));	
		count = sread(hdr,60,10);
if (count)
fprintf(stdout,"+ %u\t %u\n", hdr->FILE.POS,l_endian_i16(*(int16_t*)hdr->AS.rawdata));	
		count = sread(hdr,70,10);
	
*/

		status = sclose(hdr);
	}
      	return(status);
};


/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/
