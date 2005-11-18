/*

    $Id: main.c,v 1.6 2005-11-18 13:12:40 schloegl Exp $
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
//#include "biosig.c"


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
    	HDRTYPE HDR2;
    	HDRTYPE* hdr; 	
    	size_t 	count;
    	int	status;

	if (argc < 2)  	{
		fprintf(stderr,"Warning: Invalid number of arguments\n");
		return(-1);
      	}	
	
	// initialize/generate signal 
	for (k=0; k<NELEM; s[k] = k++%1031);	
      
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
	sopen(argv[1], "r", &HDR2);
   	fprintf(stdout,"2-%i\t%i\t%i\t%i\t%u\t%u\n",HDR2.AS.bpb,HDR2.FILE.OPEN,(int32_t)HDR2.NRec,HDR2.HeadLen,HDR2.Dur[0],HDR2.Dur[1]);

	while (!seof(&HDR2)) {
		count = sread(&HDR2,100,10);
//fprintf(stdout,"m1: %p\n",HDR2.data.block);
	
//		fprintf(stdout,"+ %Lu\t %u\t %u\t %u %f %i\n",HDR2.NRec,count,HDR2.SPR,*(int16_t*)HDR2.AS.rawdata,HDR2.data.block[0],seof(HDR2));
	}	
	sseek(&HDR2,50,SEEK_SET);
fprintf(stdout,"3+ %u\t %u\n",HDR2.FILE.POS,*(int16_t*)HDR2.AS.rawdata);	
	srewind(&HDR2);
fprintf(stdout,"4+ %u\t %u\n",HDR2.FILE.POS,*(int16_t*)HDR2.AS.rawdata);	
	count = sread(&HDR2,50,10);
if (count)
fprintf(stdout,"5+ %u\t %u\n",HDR2.FILE.POS,*(int16_t*)HDR2.AS.rawdata);	
	count = sread(&HDR2,60,10);
if (count)
fprintf(stdout,"+ %u\t %u\n", HDR2.FILE.POS,*(int16_t*)HDR2.AS.rawdata);	
	count = sread(&HDR2,70,10);
	status = sclose(&HDR2);
	
      	return(status);

};


/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/
