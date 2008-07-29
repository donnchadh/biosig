/*

    $Id: mexSLOAD.cpp,v 1.37 2008-07-29 13:05:41 schloegl Exp $
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
#include <math.h>
#include <stdlib.h>
#include <string.h>

//#define DEBUG

void mexFunction(
    int           nlhs,           /* number of expected outputs */
    mxArray       *plhs[],        /* array of pointers to output arguments */
    int           nrhs,           /* number of inputs */
    const mxArray *prhs[]         /* array of pointers to input arguments */
)

{
	int k,k1;
	const mxArray	*arg;
	HDRTYPE		*hdr;
	CHANNEL_TYPE*	cp; 
	size_t 		count;
	time_t 		T0;
	char 		*FileName=NULL;  
	int 		status; 
	int		CHAN = 0;
	int		TARGETSEGMENT = 1; 
	double		*ChanList=NULL;
	int		NS = -1;
	char		FlagOverflowDetection=1, FlagUCAL=0;
	

	VERBOSE_LEVEL = 3; 
	
	if (nrhs<1) {
		mexPrintf("   Usage of mexSLOAD:\n");
		mexPrintf("\t[s,HDR]=sload(f)\n");
		mexPrintf("\t[s,HDR]=sload(f,chan)\n\t\tchan must be sorted in ascending order\n");
		mexPrintf("\t[s,HDR]=sload(f,chan,\"...\")\n");
		mexPrintf("\t[s,HDR]=sload(f,chan,\"OVERFLOWDETECTION:ON\")\n");
		mexPrintf("\t[s,HDR]=sload(f,chan,\"OVERFLOWDETECTION:OFF\")\n");
		mexPrintf("\t[s,HDR]=sload(f,chan,\"UCAL:ON\")\n");
		mexPrintf("\t[s,HDR]=sload(f,chan,\"UCAL:OFF\")\n\n");
		mexPrintf("\t[s,HDR]=sload(f,chan,\"TARGETSEGMENT:<N>\")\n\n");
		mexPrintf("   Input:\n\tf\tfilename\n");
		mexPrintf("\tchan\tlist of selected channels; 0=all channels [default]\n");
		mexPrintf("\tUCAL\tON: do not calibrate data; default=OFF\n");
		mexPrintf("\tOVERFLOWDETECTION\tdefault = ON\n\t\tON: values outside dynamic range are not-a-number (NaN)\n");
		mexPrintf("\tTARGETSEGMENT:<N>\n\t\tselect segment <N> in multisegment files (like Nihon-Khoden), default=1\n\t\tIt has no effect for other data formats.");
		mexPrintf("   Output:\n\ts\tsignal data, each column is one channel\n");
		mexPrintf("\tHDR\theader structure\n\n");
		return; 
	}
	
	/* process input arguments */
	for (k = 0; k < nrhs; k++)
	{	
		arg = prhs[k];
		if (mxIsEmpty(arg))
#ifdef DEBUG		
			mexPrintf("arg[%i] Empty\n",k)
#endif
			;
		else if (mxIsCell(arg))
#ifdef DEBUG		
			mexPrintf("arg[%i] IsCell\n",k)
#endif
			;
		else if (mxIsStruct(arg)) {
#ifdef DEBUG		
			mexPrintf("arg[%i] IsStruct\n",k);
#endif
			if (k==0)			
				FileName = mxArrayToString(mxGetField(prhs[k],0,"FileName"));
		}
		else if (mxIsNumeric(arg)) {
#ifdef DEBUG		
			mexPrintf("arg[%i] IsNumeric\n",k);
#endif
			ChanList = (double*)mxGetData(prhs[k]);
			NS = mxGetNumberOfElements(prhs[k]);
		}	
		else if (mxIsSingle(arg))
#ifdef DEBUG		
			mexPrintf("arg[%i] IsSingle\n",k)
#endif
			;
		else if (mxIsChar(arg)) {
#ifdef DEBUG		
			mexPrintf("arg[%i]=%s \n",k,mxArrayToString(prhs[k]));
#endif
			if (k==0)			
				FileName = mxArrayToString(prhs[k]);
			else if (!strcmp(mxArrayToString(prhs[k]),"OVERFLOWDETECTION:ON"))
				FlagOverflowDetection = 1;
			else if (!strcmp(mxArrayToString(prhs[k]),"OVERFLOWDETECTION:OFF"))
				FlagOverflowDetection = 0;
			else if (!strcmp(mxArrayToString(prhs[k]),"UCAL:ON")) 
				FlagUCAL = 1;
			else if (!strcmp(mxArrayToString(prhs[k]),"UCAL:OFF"))
				FlagUCAL = 0;
			else if (!strncmp(mxArrayToString(prhs[k]),"TARGETSEGMENT:",14))
				TARGETSEGMENT = atoi(mxArrayToString(prhs[k])+14);
		}
	}

	/* open and read file, convert into M-struct */

	hdr = constructHDR(0,0);
	hdr->FLAG.OVERFLOWDETECTION = FlagOverflowDetection; 
	hdr->FLAG.UCAL = FlagUCAL;
	hdr->FLAG.ROW_BASED_CHANNELS = 0; 
	hdr->FLAG.TARGETSEGMENT = TARGETSEGMENT;

	if (VERBOSE_LEVEL>8) 
		fprintf(stderr,"[101] SOPEN-R start\n");


	hdr = sopen(FileName, "r", hdr);

	if (hdr->FLAG.OVERFLOWDETECTION != FlagOverflowDetection)
		mexPrintf("Warning mexSLOAD: Overflowdetection not supported in file %s\n",hdr->FileName);
	if (hdr->FLAG.UCAL != FlagUCAL)
		mexPrintf("Warning mexSLOAD: Flag UCAL is %i instead of %i (%s)\n",FlagUCAL,hdr->FLAG.UCAL,hdr->FileName);

	if (VERBOSE_LEVEL>8) {
		mexPrintf("#info @%p\n",&(hdr->CHANNEL));
		for (size_t k=0; k<hdr->NS; ++k)
			mexPrintf("#%i: @%p %s\n",k,&(hdr->CHANNEL[k].Label),hdr->CHANNEL[k].Label);
	}			
	if (VERBOSE_LEVEL>8) 
		fprintf(stderr,"[111] SOPEN-R finished. Status=%i\n", B4C_ERRNUM);

	if ((status=serror())) {
		//plhs[0] = mxCreateDoubleMatrix(0,0, mxREAL);

		const char* fields[]={"TYPE","VERSION"};
		plhs[1] = mxCreateStructMatrix(1, 1, 2, fields);
		mxSetField(plhs[1],0,"TYPE",mxCreateString(GetFileTypeString(hdr->TYPE)));
		mxSetField(plhs[1],0,"VERSION",mxCreateDoubleScalar(hdr->VERSION));

		char msg[1024]; 
		sprintf(msg,"Error mexSLOAD: Cannot open file %s - format %s not supported.\n",FileName,GetFileTypeString(hdr->TYPE));
		destructHDR(hdr);
		mexErrMsgTxt(msg);
		//mexPrintf("ERROR(%i) in mexSLOAD: Cannot open file %s\n", status, FileName); 
		return; 
	}
	if (hdr==NULL) return;

	if (VERBOSE_LEVEL>8) 
		fprintf(stderr,"[112] SOPEN-R finished NS=%i %i\n",hdr->NS,NS);

	convert2to4_eventtable(hdr); 
			
	if ((NS<0) || ((NS==1) && (ChanList[0] == 0.0))) { 	// all channels
		for (k=0; k<hdr->NS; ++k)
			hdr->CHANNEL[k].OnOff = 1; 	// set
	}		
	else {		
		for (k=0; k<hdr->NS; ++k)
			hdr->CHANNEL[k].OnOff = 0; 	// reset
		for (k=0; k<NS; ++k) {
			int ch = (int)ChanList[k];
			if ((ch < 1) || (ch > hdr->NS)) 
				mexPrintf("Invalid channel number CHAN(%i) = %i!\n",k+1,ch); 
			else 	
				hdr->CHANNEL[ch-1].OnOff = 1;  // set
		}		
	}

	if (VERBOSE_LEVEL>8) 
		fprintf(stderr,"[113] NS=%i %i\n",hdr->NS,NS);

	count = sread(NULL, 0, hdr->NRec, hdr);
	//count = sread2(NULL, 0, hdr->NRec * hdr->SPR, hdr);

	sclose(hdr);

	if ((status=serror())) return;  

	if (VERBOSE_LEVEL>8) 
		fprintf(stderr,"\n[129] SREAD/SCLOSE on %s successful [%i,%i] [%Li,%i] %i.\n",hdr->FileName,hdr->data.size[0],hdr->data.size[1],hdr->NRec,count,NS);

	hdr->NRec = count; 
	// plhs[0] = mxCreateDoubleMatrix(hdr->SPR * count, NS, mxREAL);

	//if (hdr->FLAG.ROW_BASED_CHANNELS) {
	if (0) {
		/* transpose matrix */
		plhs[0] = mxCreateDoubleMatrix(hdr->data.size[1], hdr->data.size[0], mxREAL);
		for (k =0; k <hdr->data.size[0]; ++k)
		for (k1=0; k1<hdr->data.size[1]; ++k1)
			*(mxGetPr(plhs[0]) + k1*hdr->data.size[0] + k) = hdr->data.block[k1 + k*hdr->data.size[1]];
	} else {		
		plhs[0] = mxCreateDoubleMatrix(hdr->data.size[0], hdr->data.size[1], mxREAL);
		memcpy((void*)mxGetPr(plhs[0]),(void*)hdr->data.block, hdr->data.size[0]*hdr->data.size[1]*sizeof(biosig_data_type));
	}


	if (hdr->data.block != NULL) free(hdr->data.block);	
	hdr->data.block = NULL; 

//	hdr2ascii(hdr,stderr,4);	

	if (nlhs>1) { 
		char* mexFileName = (char*)mxMalloc(strlen(hdr->FileName)+1); 

		mxArray *HDR, *tmp, *tmp2, *Patient, *EVENT, *Filter, *Flag, *FileType;
		uint16_t numfields;
		const char *fnames[] = {"TYPE","VERSION","FileName","T0","FILE","Patient",\
		"NS","SPR","NRec","SampleRate", "FLAG", \
		"EVENT","Label","LeadIdCode","PhysDimCode","PhysDim","Filter",\
		"PhysMax","PhysMin","DigMax","DigMin","Transducer","Cal","Off","GDFTYP",\
		"LowPass","HighPass","Notch","ELEC","Impedance","AS","Dur",NULL};

		for (numfields=0; fnames[numfields++] != 0; );
		HDR = mxCreateStructMatrix(1, 1, --numfields, fnames);

		mxSetField(HDR,0,"TYPE",mxCreateString(GetFileTypeString(hdr->TYPE)));
		
		mxSetField(HDR,0,"VERSION",mxCreateDoubleScalar(hdr->VERSION));
		mxSetField(HDR,0,"NS",mxCreateDoubleScalar(hdr->NS));
		mxSetField(HDR,0,"SPR",mxCreateDoubleScalar(hdr->SPR));
		mxSetField(HDR,0,"NRec",mxCreateDoubleScalar(hdr->NRec));
		mxSetField(HDR,0,"SampleRate",mxCreateDoubleScalar(hdr->SampleRate));
		mxSetField(HDR,0,"Dur",mxCreateDoubleScalar(hdr->SPR/hdr->SampleRate));
		mxSetField(HDR,0,"FileName",mxCreateCharMatrixFromStrings(1,&hdr->FileName));

		mxSetField(HDR,0,"T0",mxCreateDoubleScalar(ldexp(hdr->T0,-32)));

		/* Channel information */ 
		mxArray *LeadIdCode  = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *PhysDimCode = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *GDFTYP      = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *PhysMax     = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *PhysMin     = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *DigMax      = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *DigMin      = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *Cal         = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *Off         = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *ELEC_POS    = mxCreateDoubleMatrix(hdr->NS,3, mxREAL);
		mxArray *LowPass     = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *HighPass    = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *Notch       = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *Impedance   = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *SPR         = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *Label       = mxCreateCellMatrix(hdr->NS,1);
		mxArray *Transducer  = mxCreateCellMatrix(hdr->NS,1);
		mxArray *PhysDim1    = mxCreateCellMatrix(hdr->NS,1);

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
			*(mxGetPr(SPR)+k) 	  = (double)hdr->CHANNEL[k].SPR;
			*(mxGetPr(LowPass)+k) 	  = (double)hdr->CHANNEL[k].LowPass;
			*(mxGetPr(HighPass)+k) 	  = (double)hdr->CHANNEL[k].HighPass;
			*(mxGetPr(Notch)+k) 	  = (double)hdr->CHANNEL[k].Notch;
			*(mxGetPr(ELEC_POS)+k) 	  = (double)hdr->CHANNEL[k].XYZ[0];
			*(mxGetPr(ELEC_POS)+k+hdr->NS) 	  = (double)hdr->CHANNEL[k].XYZ[1];
			*(mxGetPr(ELEC_POS)+k+hdr->NS*2)  = (double)hdr->CHANNEL[k].XYZ[2];

			mxSetCell(Label,k,mxCreateString(hdr->CHANNEL[k].Label));
			mxSetCell(Transducer,k,mxCreateString(hdr->CHANNEL[k].Transducer));
			
			char p[MAX_LENGTH_PHYSDIM+1];
			PhysDim(hdr->CHANNEL[k].PhysDimCode,p);			
			mxSetCell(PhysDim1,k,mxCreateString(p));
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
		mxSetField(HDR,0,"Impedance",Impedance);
		mxSetField(HDR,0,"Off",Off);
		mxSetField(HDR,0,"PhysDim",PhysDim1);
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
				
		/* FLAG */
		const char* field3[] = {"UCAL","OVERFLOWDETECTION",NULL};
		for (numfields=0; field3[numfields++] != 0; );
		Flag = mxCreateStructMatrix(1, 1, --numfields, field3);
		mxSetField(Flag,0,"UCAL",mxCreateDoubleScalar((double)hdr->FLAG.UCAL));
		mxSetField(Flag,0,"OVERFLOWDETECTION",mxCreateDoubleScalar((double)hdr->FLAG.OVERFLOWDETECTION));
		mxSetField(HDR,0,"FLAG",Flag);

		/* Filter */ 
		const char *filter_fields[] = {"HighPass","LowPass","Notch",NULL};
		for (numfields=0; filter_fields[numfields++] != 0; );
		Filter = mxCreateStructMatrix(1, 1, --numfields, filter_fields);
		mxSetField(Filter,0,"LowPass",LowPass);
		mxSetField(Filter,0,"HighPass",HighPass);
		mxSetField(Filter,0,"Notch",Notch);
		mxSetField(HDR,0,"Filter",Filter);

		/* annotation, marker, event table */
		const char *event_fields[] = {"SampleRate","TYP","POS","DUR","CHN","Desc",NULL};
		
		if (hdr->EVENT.DUR == NULL)
			EVENT = mxCreateStructMatrix(1, 1, 3, event_fields);
		else {	
			EVENT = mxCreateStructMatrix(1, 1, 5, event_fields);
			mxArray *DUR = mxCreateDoubleMatrix(hdr->EVENT.N,1, mxREAL);
			mxArray *CHN = mxCreateDoubleMatrix(hdr->EVENT.N,1, mxREAL);
			for (size_t k=0; k<hdr->EVENT.N; ++k) {
				*(mxGetPr(DUR)+k) = (double)hdr->EVENT.DUR[k];
				*(mxGetPr(CHN)+k) = (double)hdr->EVENT.CHN[k];
			} 
			mxSetField(EVENT,0,"DUR",DUR);
			mxSetField(EVENT,0,"CHN",CHN);
		}
		if (hdr->EVENT.CodeDesc != NULL) {
			mxAddField(EVENT, "CodeDesc");
			mxArray *CodeDesc = mxCreateCellMatrix(hdr->EVENT.LenCodeDesc-1,1);
			for (size_t k=1; k < hdr->EVENT.LenCodeDesc; ++k) {
				mxSetCell(CodeDesc,k-1,mxCreateString(hdr->EVENT.CodeDesc[k]));
			} 
			mxSetField(EVENT,0,"CodeDesc",CodeDesc);
		}	

		mxArray *TYP = mxCreateDoubleMatrix(hdr->EVENT.N,1, mxREAL);
		mxArray *POS = mxCreateDoubleMatrix(hdr->EVENT.N,1, mxREAL);
		for (size_t k=0; k<hdr->EVENT.N; ++k) {
			*(mxGetPr(TYP)+k) = (double)hdr->EVENT.TYP[k];
			*(mxGetPr(POS)+k) = (double)hdr->EVENT.POS[k];
		} 
		mxSetField(EVENT,0,"TYP",TYP);
		mxSetField(EVENT,0,"POS",POS);
		mxSetField(EVENT,0,"SampleRate",mxCreateDoubleScalar(hdr->EVENT.SampleRate));
		mxSetField(HDR,0,"EVENT",EVENT);

		/* Patient Information */ 
		const char *patient_fields[] = {"Sex","Handedness","Id","Name","Weight","Height","Birthday",NULL};
		for (numfields=0; patient_fields[numfields++] != 0; );
		Patient = mxCreateStructMatrix(1, 1, --numfields, patient_fields);
//		mxSetField(Patient,0,"Name",mxCreateCharMatrixFromStrings(1,(const char**)&(hdr->Patient.Name)));
//		mxSetField(Patient,0,"Id",mxCreateCharMatrixFromStrings(1,(const char**)&(hdr->Patient.Id)));
		mxSetField(Patient,0,"Handedness",mxCreateDoubleScalar(hdr->Patient.Handedness));
		mxSetField(Patient,0,"Sex",mxCreateDoubleScalar(hdr->Patient.Sex));
		mxSetField(Patient,0,"Weight",mxCreateDoubleScalar((double)hdr->Patient.Weight));
		mxSetField(Patient,0,"Height",mxCreateDoubleScalar((double)hdr->Patient.Height));
		mxSetField(Patient,0,"Birthday",mxCreateDoubleScalar(ldexp(hdr->Patient.Birthday,-32)));
		double d;
		if (hdr->Patient.Weight==0)		d = 0.0/0.0;	// not-a-number		
		else if (hdr->Patient.Weight==255)	d = 1.0/0.0;	// Overflow
		else					d = (double)hdr->Patient.Weight;
		mxSetField(Patient,0,"Weight",mxCreateDoubleScalar(d));
			
		if (hdr->Patient.Height==0)		d = 0.0/0.0;	// not-a-number		
		else if (hdr->Patient.Height==255)	d = 1.0/0.0;	// Overflow
		else					d = (double)hdr->Patient.Height;
		mxSetField(Patient,0,"Height",mxCreateDoubleScalar(d));
	
		mxSetField(HDR,0,"Patient",Patient);

		plhs[1] = HDR;
	}

	if (VERBOSE_LEVEL>8) fprintf(stdout,"[131] going for SCLOSE\n");
	destructHDR(hdr);
	hdr = NULL; 
	if (VERBOSE_LEVEL>8) fprintf(stdout,"[137] SCLOSE finished\n");
};

