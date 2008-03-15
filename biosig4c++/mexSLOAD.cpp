/*

    $Id: mexSLOAD.cpp,v 1.11 2008-03-15 00:28:49 schloegl Exp $
    Copyright (C) 2007,2008 Alois Schloegl <a.schloegl@ieee.org>
    This file is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.


 */

#include "biosig.h"
#include "mex.h"
#include <stdlib.h>
#include <string.h>


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
	int 		status; 
	
	VERBOSE_LEVEL = 3; 
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
			mxGetString(prhs[k], FileName, (1022 > mxGetN(prhs[k]) ? 1022 : mxGetN(prhs[k])));
			mexPrintf("IsChar[%i,%i]\n\t%s\n",mxGetM(prhs[k]),mxGetN(prhs[k]),FileName);

			hdr = sopen(FileName, "r", NULL);
			if ((status=serror())) return; 
	
			if (hdr==NULL) return;
			if (VERBOSE_LEVEL>8) fprintf(stdout,"[112] SOPEN-R finished\n");


			if (VERBOSE_LEVEL>8) fprintf(stdout,"[113] SOPEN-R finished\n");

			hdr->FLAG.OVERFLOWDETECTION = (hdr->TYPE!=MFER);
			hdr->FLAG.UCAL = 0;
	
			if (VERBOSE_LEVEL>8) fprintf(stdout,"[121]\n");

			count = sread(NULL, 0, hdr->NRec, hdr);
			if ((status=serror())) return;  

			if (VERBOSE_LEVEL>8) 
				fprintf(stdout,"\n[129] SREAD on %s successful [%i,%i].\n",hdr->FileName,hdr->data.size[0],hdr->data.size[1]);

			hdr->NRec = count; 
			plhs[0] = mxCreateDoubleMatrix(hdr->SPR * count, hdr->NS, mxREAL);
			memcpy((void*)mxGetPr(plhs[0]),(void*)hdr->data.block,hdr->SPR * count * hdr->NS * sizeof(biosig_data_type));
	        	free(hdr->data.block);	


//			hdr2ascii(hdr,stdout,4);	

			if (nlhs>1) { 
				char* mexFileName = (char*)mxMalloc(strlen(hdr->FileName)+1); 

				mxArray *HDR, *tmp, *tmp2, *Patient, *EVENT;
				uint16_t numfields;
				const char *fnames[] = {"TYPE","NS","SPR","NRec","SampleRate","FileName",\
				"FILE","Patient","EVENT","Label","LeadIdCode","PhysDimCode","PhysDim",\
				"PhysMax","PhysMin","DigMax","DigMin","Transducer","Cal","Off","GDFTYP",\
				"LowPass","HighPass","Notch","ELEC","Impedance","AS","Dur",NULL};
				for (numfields=0; fnames[numfields++] != 0; );
				HDR = mxCreateStructMatrix(1, 1, --numfields, fnames);

				mxSetField(HDR,0,"NS",mxCreateDoubleScalar(hdr->NS));
				mxSetField(HDR,0,"SPR",mxCreateDoubleScalar(hdr->SPR));
				mxSetField(HDR,0,"NRec",mxCreateDoubleScalar(hdr->NRec));
				mxSetField(HDR,0,"SampleRate",mxCreateDoubleScalar(hdr->SampleRate));
				mxSetField(HDR,0,"Dur",mxCreateDoubleScalar((double)hdr->Dur[0]/hdr->Dur[1]));

				mxSetField(HDR,0,"FileName",mxCreateCharMatrixFromStrings(1,&hdr->FileName));

				/* Channel information */ 
				mxArray *LeadIdCode  = mxCreateDoubleMatrix(hdr->NS,1, mxREAL);
				mxArray *PhysDimCode = mxCreateDoubleMatrix(hdr->NS,1, mxREAL);
				mxArray *GDFTYP      = mxCreateDoubleMatrix(hdr->NS,1, mxREAL);
				mxArray *PhysMax     = mxCreateDoubleMatrix(hdr->NS,1, mxREAL);
				mxArray *PhysMin     = mxCreateDoubleMatrix(hdr->NS,1, mxREAL);
				mxArray *DigMax      = mxCreateDoubleMatrix(hdr->NS,1, mxREAL);
				mxArray *DigMin      = mxCreateDoubleMatrix(hdr->NS,1, mxREAL);
				mxArray *Cal         = mxCreateDoubleMatrix(hdr->NS,1, mxREAL);
				mxArray *Off         = mxCreateDoubleMatrix(hdr->NS,1, mxREAL);
				mxArray *ELEC_POS    = mxCreateDoubleMatrix(hdr->NS,3, mxREAL);
				mxArray *LowPass     = mxCreateDoubleMatrix(hdr->NS,1, mxREAL);
				mxArray *HighPass    = mxCreateDoubleMatrix(hdr->NS,1, mxREAL);
				mxArray *Notch       = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
				mxArray *Impedance   = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
				mxArray *SPR         = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
				mxArray *Label       = mxCreateCellMatrix(hdr->NS,1);
				mxArray *Transducer  = mxCreateCellMatrix(hdr->NS,1);
				mxArray *PhysDim     = mxCreateCellMatrix(hdr->NS,1);
				for (size_t k=0; k<hdr->NS; ++k) {
					*(mxGetPr(LeadIdCode)+k)  = (double)hdr->CHANNEL[k].LeadIdCode;
					*(mxGetPr(PhysDimCode)+k) = (double)hdr->CHANNEL[k].PhysDimCode;
					*(mxGetPr(GDFTYP)+k) 	  = (double)hdr->CHANNEL[k].GDFTYP;
					*(mxGetPr(PhysMax)+k) 	  = (double)hdr->CHANNEL[k].PhysMax;
					*(mxGetPr(PhysMin)+k) 	  = (double)hdr->CHANNEL[k].PhysMin;
					*(mxGetPr(DigMax)+k) 	  = (double)hdr->CHANNEL[k].DigMax;
					*(mxGetPr(DigMin)+k) 	  = (double)hdr->CHANNEL[k].DigMin;
					*(mxGetPr(Cal)+k) 	  = (double)hdr->CHANNEL[k].Cal;
					*(mxGetPr(Off)+k) 	  = (double)hdr->CHANNEL[k].Off;
					*(mxGetPr(PhysDimCode)+k) = (double)hdr->CHANNEL[k].PhysDimCode;
					*(mxGetPr(SPR)+k) 	  = (double)hdr->CHANNEL[k].SPR;
					*(mxGetPr(LowPass)+k) 	  = (double)hdr->CHANNEL[k].LowPass;
					*(mxGetPr(HighPass)+k) 	  = (double)hdr->CHANNEL[k].HighPass;
					*(mxGetPr(Notch)+k) 	  = (double)hdr->CHANNEL[k].Notch;
					*(mxGetPr(ELEC_POS)+k) 	  = (double)hdr->CHANNEL[k].XYZ[0];
					*(mxGetPr(ELEC_POS)+k+hdr->NS) 	  = (double)hdr->CHANNEL[k].XYZ[1];
					*(mxGetPr(ELEC_POS)+k+hdr->NS*2)  = (double)hdr->CHANNEL[k].XYZ[2];

					mxSetCell(Label,k,mxCreateString(hdr->CHANNEL[k].Label));
					mxSetCell(Transducer,k,mxCreateString(hdr->CHANNEL[k].Transducer));
					mxSetCell(PhysDim,k,mxCreateString(hdr->CHANNEL[k].PhysDim));
				} 
				mxSetField(HDR,0,"LeadIdCode",LeadIdCode);
				mxSetField(HDR,0,"PhysDimCode",PhysDimCode);
				mxSetField(HDR,0,"GDFTYP",GDFTYP);
				mxSetField(HDR,0,"PhysMax",PhysMax);
				mxSetField(HDR,0,"PhysMin",PhysMin);
				mxSetField(HDR,0,"DigMax",DigMax);
				mxSetField(HDR,0,"DigMin",DigMin);
				mxSetField(HDR,0,"Cal",Cal);
				mxSetField(HDR,0,"Off",Off);
				mxSetField(HDR,0,"LowPass",LowPass);
				mxSetField(HDR,0,"HighPass",HighPass);
				mxSetField(HDR,0,"Notch",Notch);
				mxSetField(HDR,0,"Impedance",Impedance);
				mxSetField(HDR,0,"Off",Off);
				mxSetField(HDR,0,"PhysDim",PhysDim);
				mxSetField(HDR,0,"Transducer",Transducer);
				mxSetField(HDR,0,"Label",Label);

				const char* field[] = {"XYZ","GND","REF",NULL};
				for (numfields=0; field[numfields++] != 0; );
				tmp = mxCreateStructMatrix(1, 1, --numfields, field);
				mxSetField(tmp,0,"XYZ",ELEC_POS);
				mxSetField(HDR,0,"ELEC",tmp);

				const char* field2[] = {"SPR",NULL};
				for (numfields=0; field2[numfields++] != 0; );
				tmp2 = mxCreateStructMatrix(1, 1, --numfields, field2);
				mxSetField(tmp2,0,"SPR",SPR);
				mxSetField(HDR,0,"AS",tmp2);

				/* annotation, marker, event table */
				const char *event_fields[] = {"N","SampleRate","TYP","POS","DUR","CHN",NULL};
				for (numfields=0; event_fields[numfields++] != 0; );
				EVENT = mxCreateStructMatrix(1, 1, --numfields, event_fields);
				mxSetField(EVENT,0,"N",mxCreateDoubleScalar(hdr->EVENT.N));
				mxSetField(EVENT,0,"SampleRate",mxCreateDoubleScalar(hdr->EVENT.SampleRate));

				mxArray *TYP = mxCreateDoubleMatrix(hdr->EVENT.N,1, mxREAL);
				mxArray *POS = mxCreateDoubleMatrix(hdr->EVENT.N,1, mxREAL);
				for (size_t k=0; k<hdr->EVENT.N; ++k) {
					*(mxGetPr(TYP)+k) = (double)hdr->EVENT.TYP[k];
					*(mxGetPr(POS)+k) = (double)hdr->EVENT.POS[k];
				} 
				mxSetField(EVENT,0,"TYP",TYP);
				mxSetField(EVENT,0,"POS",POS);
				if (hdr->EVENT.DUR != NULL) {
					mxArray *DUR = mxCreateDoubleMatrix(hdr->EVENT.N,1, mxREAL);
					mxArray *CHN = mxCreateDoubleMatrix(hdr->EVENT.N,1, mxREAL);
					for (size_t k=0; k<hdr->EVENT.N; ++k) {
						*(mxGetPr(DUR)+k) = (double)hdr->EVENT.DUR[k];
						*(mxGetPr(CHN)+k) = (double)hdr->EVENT.CHN[k];
					} 
					mxSetField(EVENT,0,"DUR",DUR);
					mxSetField(EVENT,0,"CHN",CHN);
				}
				mxSetField(HDR,0,"EVENT",EVENT);

				/* Patient Information */ 
				const char *patient_fields[] = {"Sex","Handedness","Id","Name","Weight","Height",NULL};
				for (numfields=0; patient_fields[numfields++] != 0; );
				Patient = mxCreateStructMatrix(1, 1, --numfields, patient_fields);
//				mxSetField(Patient,0,"Name",mxCreateCharMatrixFromStrings(1,(const char**)&(hdr->Patient.Name)));
//				mxSetField(Patient,0,"Id",mxCreateCharMatrixFromStrings(1,(const char**)&(hdr->Patient.Id)));
				mxSetField(Patient,0,"Handedness",mxCreateDoubleScalar(hdr->Patient.Handedness));
				mxSetField(Patient,0,"Sex",mxCreateDoubleScalar(hdr->Patient.Sex));
				mxSetField(Patient,0,"Weight",mxCreateDoubleScalar(hdr->Patient.Weight));
				mxSetField(Patient,0,"Height",mxCreateDoubleScalar(hdr->Patient.Height));

				mxSetField(HDR,0,"Patient",Patient);
				
				plhs[1] = HDR;
			}	

			if (VERBOSE_LEVEL>8) fprintf(stdout,"[131] going for SCLOSE\n");
			sclose(hdr);
			hdr = NULL; 
			if (VERBOSE_LEVEL>8) fprintf(stdout,"[137] SCLOSE finished\n");
		}
	}
};

