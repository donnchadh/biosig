/*

    $Id: main.c,v 1.2 2005-09-26 15:03:32 schloegl Exp $
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
#include "biosig.c"


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
    	short s[NELEM];
    	HDRTYPE HDR, HDR2;
    	size_t count;

	if (argc < 2)  	{
		fprintf(stderr,"Warning: Invalid number of arguments\n");
		return(-1);
      	}	
	
	// initialize/generate signal 
	for (k=0; k<NELEM; s[k] = k++%1031);	
      
	// write: define header
	HDR = init_default_hdr(HDR,4,10);  // allocate memory for 4 channels, 10 events 
	HDR.Patient.Id = "test1";
	HDR.TYPE = GDF; 

/*
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
*/
	// READ GDF FILE 
	HDR2 = sopen(argv[1], HDR2, "r");
   	fprintf(stdout,"2-%i\t%i\t%i\t%i\t%u\t%u\n",HDR2.AS.bpb,HDR2.FILE.OPEN,(int32_t)HDR2.NRec,HDR2.HeadLen,HDR2.Dur[0],HDR2.Dur[1]);

	while (!seof(HDR2)) {
		count = sread(&HDR2,10);
//fprintf(stdout,"m1: %p\n",HDR2.data.block);
	
//		fprintf(stdout,"+ %Lu\t %u\t %u\t %u %f %i\n",HDR2.NRec,count,HDR2.SPR,*(int16_t*)HDR2.AS.rawdata,HDR2.data.block[0],seof(HDR2));
	}	
	sseek(&HDR2,50,SEEK_SET);
fprintf(stdout,"3+ %u\t %u\n",HDR2.FILE.POS,*(int16_t*)HDR2.AS.rawdata);	
	srewind(&HDR2);
fprintf(stdout,"4+ %u\t %u\n",HDR2.FILE.POS,*(int16_t*)HDR2.AS.rawdata);	
	count = sread(&HDR2,10);
if (count)
fprintf(stdout,"5+ %u\t %u\n",HDR2.FILE.POS,*(int16_t*)HDR2.AS.rawdata);	
	count = sread(&HDR2,10);
if (count)
fprintf(stdout,"+ %u\t %u\n", HDR2.FILE.POS,*(int16_t*)HDR2.AS.rawdata);	
	count = sread(&HDR2,10);
   	fprintf(stdout,"3-%i\t%i\t%i\t%i\t%u\t%u\n",HDR2.AS.bpb,HDR2.AS.spb,(long)HDR2.NRec,HDR2.HeadLen,HDR2.Dur[0],HDR2.Dur[1]);
	HDR2 = sclose(HDR2);
	
      	return(0);

};


/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/
