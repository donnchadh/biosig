/*

    $Id: main.c,v 1.8 2005-11-21 00:23:53 schloegl Exp $
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
    	short 	s[NELEM];
    	HDRTYPE *hdr,*hdr2; 	
    	size_t 	count;
    	int	status;

	if (argc < 2)  	{
		fprintf(stderr,"Warning: Invalid number of arguments\n");
		return(-1);
      	}	
	
	// initialize/generate signal 
	for (k=0; k<NELEM; s[k] = l_endian_u16(k++%1031));	
      
	// write: define header
	hdr = create_default_hdr(4,10);  // allocate memory for 4 channels, 10 events 
	hdr->Patient.Id = "test1";
	hdr->TYPE = GDF; 

	// OPEN and WRITE GDF FILE 
     	sopen(argv[1], "w", hdr);

	swrite(&s, NELEM/hdr->NS, hdr);

	// define events before SCLOSE; 
	for (k=0; k<hdr->EVENT.N; k++) {
		hdr->EVENT.TYP[k] = k+1;
		hdr->EVENT.POS[k] = k*100;
	};
      	status = sclose(hdr);

   	fprintf(stdout,"1-%i\t%i\t%i\t%i\t%u\t%u\n",sizeof(hdr->EVENT.TYP),sizeof(*hdr->EVENT.TYP),(int32_t)hdr->NRec,hdr->HeadLen,hdr->Dur[0],hdr->Dur[1]);

	// READ GDF FILE 
	hdr2 = sopen(argv[1], "r", NULL);
	if (hdr2==NULL) return(-1); 

	fprintf(stdout,"--%i\t%i\n", hdr2->FLAG.OVERFLOWDETECTION, hdr2->FLAG.UCAL);
	hdr2->FLAG.OVERFLOWDETECTION = 0; 
//	hdr2->FLAG.UCAL = 1;
	fprintf(stdout,"--%i\t%i\n", hdr2->FLAG.OVERFLOWDETECTION, hdr2->FLAG.UCAL);
   	fprintf(stdout,"2-%u\t%i\t%i\t%i\t%u\t%u\n",hdr2->AS.bpb,hdr2->FILE.OPEN,(int32_t)hdr2->NRec,hdr2->HeadLen,hdr2->Dur[0],hdr2->Dur[1]);
	
	for (k=0; k<hdr2->NS; k++) {
		fprintf(stdout,"#%i: %f   %f\n",k,hdr2->CHANNEL[k].PhysMax,hdr2->CHANNEL[k].PhysMin);
		fprintf(stdout,"#    %f   %f\n",hdr2->CHANNEL[k].DigMax,hdr2->CHANNEL[k].DigMin);
		fprintf(stdout,"     %f   %f\n",hdr2->CHANNEL[k].Cal,hdr2->CHANNEL[k].Off);
	}
	
	for (k=0; !seof(hdr2); k+=10) {
		count = sread(hdr2,k,10);
//fprintf(stdout,"m1: %p\n",hdr2->data.block);
	
	}	
	fprintf(stdout,"+ %Lu\t %u\t %u\t %u %f %i\n",hdr2->NRec,count,hdr2->FILE.POS,*(int16_t*)hdr2->AS.rawdata,hdr2->data.block[0],seof(hdr2));
	sseek(hdr2,50,SEEK_SET);
fprintf(stdout,"3+ %u\t %u\n",hdr2->FILE.POS,*(int16_t*)hdr2->AS.rawdata);	
	srewind(hdr2);
fprintf(stdout,"4+ %u\t %u\n",hdr2->FILE.POS,*(int16_t*)hdr2->AS.rawdata);	
	count = sread(hdr2,50,10);
fprintf(stdout,"++ %i\t %u\t%i\t%i\t%i\t%i\n",hdr2->FILE.POS,*(int16_t*)hdr2->AS.rawdata,count,hdr2->data.size[0],hdr2->data.size[1],hdr2->NS);	
	hdr2->CHANNEL[0].OnOff = 0; 
	count = sread(hdr2,50,10);
fprintf(stdout,"++ %i\t %u\t%i\t%i\t%i\t%i\n",hdr2->FILE.POS,*(int16_t*)hdr2->AS.rawdata,count,hdr2->data.size[0],hdr2->data.size[1],hdr2->NS);	
if (count)
fprintf(stdout,"5+ %u\t %u\n",hdr2->FILE.POS,*(int16_t*)hdr2->AS.rawdata);	
	count = sread(hdr2,60,10);
if (count)
fprintf(stdout,"+ %u\t %u\n", hdr2->FILE.POS,*(int16_t*)hdr2->AS.rawdata);	
	count = sread(hdr2,70,10);
	status = sclose(hdr2);
	
      	return(status);

};


/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/
