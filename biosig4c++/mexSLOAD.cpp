/*

    $Id: mexSLOAD.cpp,v 1.1 2007-08-22 22:26:56 schloegl Exp $
    Copyright (C) 2007 Alois Schloegl <a.schloegl@ieee.org>
    This file is part of the "BioSig for C/C++" repository 
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

#include "biosig.h"
#include "mex.h"
#include <stdlib.h>


void mexFunction(
    int           nlhs,           /* number of expected outputs */
    mxArray       *plhs[],        /* array of pointers to output arguments */
    int           nrhs,           /* number of inputs */
    const mxArray *prhs[]         /* array of pointers to input arguments */
)
{
	int k;
	const mxArray	*arg;
	HDRTYPE		*hdr;
	CHANNEL_TYPE*	cp; 
	size_t 		count;
	time_t 		T0;
	char 		FileName[1024];  
	int 		status, VERBOSE_LEVEL = 9; 
	
	mexPrintf("\nHello world! [nlhs=%i nrhs=%i]\n",nlhs,nrhs);
	mexPrintf("This is mexSLOAD, it is currently in an experimental state!\n");
	for (k=0; k<nrhs; k++)
	{	
		arg = prhs[k];
		if (mxIsEmpty(arg))
			mexPrintf("Empty\n");
		else if (mxIsCell(arg))
			mexPrintf("IsCell\n");
		else if (mxIsStruct(arg))
			mexPrintf("IsStruct\n");
		else if (mxIsNumeric(arg))
			mexPrintf("IsNumeric\n");
		else if (mxIsSingle(arg))
			mexPrintf("IsSingle\n");
		else if (mxIsChar(arg))
		{
			mxGetString(prhs[k], FileName, max(1022,mxGetN(prhs[k])));
			mexPrintf("IsChar[%i,%i]\n\t%s\n",mxGetM(prhs[k]),mxGetN(prhs[k]),FileName);

			hdr = sopen(FileName, "r", NULL);
			if ((status=serror())) exit(status); 
	
			if (hdr==NULL) exit(-1);
			if (VERBOSE_LEVEL>8) fprintf(stdout,"[112] SOPEN-R finished\n");

			hdr2ascii(hdr,stdout,1);	

			nlhs = 1;
			plhs[0] = mxCreateDoubleMatrix(hdr->SPR * hdr->NRec, hdr->NS, mxREAL);
			hdr->data.block = (biosig_data_type*) mxCalloc(hdr->SPR * hdr->NRec * hdr->NS, sizeof(biosig_data_type));
			mxSetPr(plhs[0],hdr->data.block);
/*			
			plhs = (mxArray**)mxCalloc(nlhs,sizeof(mxArray*));
			plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
*/			
			if (VERBOSE_LEVEL>8) fprintf(stdout,"[112] SOPEN-R finished\n");

			hdr->FLAG.OVERFLOWDETECTION = 0;
			hdr->FLAG.UCAL = 1;
	
			if (VERBOSE_LEVEL>8) fprintf(stdout,"[121]\n");
			count = sread(hdr, 0, hdr->NRec);
			if ((status=serror())) exit(status); 

			if (VERBOSE_LEVEL>8) 
				fprintf(stdout,"\n[129] SREAD on %s successful [%i,%i].\n",hdr->FileName,hdr->data.size[0],hdr->data.size[1]);

			if (VERBOSE_LEVEL>8) fprintf(stdout,"[131] going for SCLOSE\n");
			sclose(hdr);
			hdr = NULL; 
			if (VERBOSE_LEVEL>8) fprintf(stdout,"[137] SCLOSE finished\n");
		}
	}
/*		
	nlhs = 1; 	
	plhs = (mxArray*)mxCalloc(sizeof(mxArray*), nlhs);
	for (k=0; k<nrhs; k++)
	{
		plhs[k] = mxCreateNumericMatrix(k+1, k+2, mxDOUBLE_CLASS, mxREAL);
	}	
	
	mxAddField(pm, fieldname)
	mxRemoveField, 
	mxSetFieldByNumber
	mxSetPr(plhs[0], hdr->data.block);
*/
};

