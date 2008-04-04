/*

    $Id: mexSLOAD.cpp,v 1.19 2008-04-04 20:30:21 schloegl Exp $
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
	char 		*FileName=NULL;  
	int 		status; 
	int		CHAN = 0;
	double		*ChanList=NULL;
	int		NS = -1;
	char		FlagOverflowDetection=0, FlagUCAL=0;
	
	
	VERBOSE_LEVEL = 3; 
	
	if (nrhs<1) {
		mexPrintf("   Usage of mexSLOAD:\n");
		mexPrintf("\t[s,HDR]=sload(f)\n");
		mexPrintf("\t[s,HDR]=sload(f,chan)\n\t\tchan must be sorted in ascending order\n");
		mexPrintf("\t[s,HDR]=sload(f,chan,\"OVERFLOWDETECTION:ON\")\n");
		mexPrintf("\t[s,HDR]=sload(f,chan,\"OVERFLOWDETECTION:OFF\")\n");
		mexPrintf("\t[s,HDR]=sload(f,chan,\"UCAL:ON\")\n");
		mexPrintf("\t[s,HDR]=sload(f,chan,\"UCAL:OFF\")\n\n");
		mexPrintf("   Input:\n\tf\tfilename\n");
		mexPrintf("\tchan\tlist of selected channels; 0=all channels [default]\n");
		mexPrintf("\tUCAL\tON: do not calibrate data; default=OFF\n");
		mexPrintf("\tOVERFLOWDETECTION\tdefault = ON\n\t\tON: values outside dynamic range are not-a-number (NaN)\n");
		mexPrintf("   Output:\n\ts\tsignal data, each column is one channel\n");
		mexPrintf("\tHDR\theader structure\n\n");
		return; 
	}
	
	/* process input arguments */
	mexPrintf("This is mexSLOAD, it is currently in an experimental state!\n");
	for (k = 0; k < nrhs; k++)
	{	
		arg = prhs[k];
		if (mxIsEmpty(arg))
			mexPrintf("arg[%i] Empty\n",k);
		else if (mxIsCell(arg))
			mexPrintf("arg[%i] IsCell\n",k);
		else if (mxIsStruct(arg)) {
			mexPrintf("arg[%i] IsStruct\n",k);
			if (k==0)			
				FileName = mxArrayToString(mxGetField(prhs[k],0,"FileName"));
		}
		else if (mxIsNumeric(arg)) {
			mexPrintf("arg[%i] IsNumeric\n",k);
			ChanList = (double*)mxGetData(prhs[k]);
			NS = mxGetNumberOfElements(prhs[k]);
		}	
		else if (mxIsSingle(arg))
			mexPrintf("arg[%i] IsSingle\n",k);
		else if (mxIsChar(arg)) {
			mexPrintf("arg[%i]=%s \n",k,mxArrayToString(prhs[k]));
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
		}
	}

	/* open and read file, convert into M-struct */

	hdr = sopen(FileName, "r", NULL);
	if ((status=serror())) {
		mexPrintf("ERROR=%i: Cannot open file %s (%s)\n", status, FileName, B4C_ERRMSG); 
		return; 
	}
			
	if (hdr==NULL) return;

	if (VERBOSE_LEVEL>8) fprintf(stdout,"[112] SOPEN-R finished\n");

	hdr->FLAG.OVERFLOWDETECTION = FlagOverflowDetection; 
	hdr->FLAG.UCAL = FlagUCAL;

	if ((NS<0) || ((NS==1) && (ChanList[0] == 0.0))) { 	// all channels
		for (k=0; k<hdr->NS; ++k)
			hdr->CHANNEL[k].OnOff = 1; // set 
	}		
	else {		
		for (k=0; k<hdr->NS; ++k)
			hdr->CHANNEL[k].OnOff = 0; // reset 
		for (k=0; k<NS; ++k) {
			int ch = (int)ChanList[k];
			if ((ch < 1) || (ch > hdr->NS)) 
				mexPrintf("Invalid channel number CHAN(%i) = %i!\n",k+1,ch); 
			else 	
				hdr->CHANNEL[ch-1].OnOff = 1;  // set
		}		
	}
				
	for (k=0,NS=0; k<hdr->NS; ++k)
		NS += (hdr->CHANNEL[k].OnOff ? 1 : 0); // count number of channels read 
			
	if (VERBOSE_LEVEL>8) fprintf(stdout,"[121] NoOfChan=%i\n",NS);

	count = sread(NULL, 0, hdr->NRec, hdr);
	if ((status=serror())) return;  

	if (VERBOSE_LEVEL>8) 
		fprintf(stdout,"\n[129] SREAD on %s successful [%i,%i].\n",hdr->FileName,hdr->data.size[0],hdr->data.size[1]);

	hdr->NRec = count; 
	plhs[0] = mxCreateDoubleMatrix(hdr->SPR * count, NS, mxREAL);
	memcpy((void*)mxGetPr(plhs[0]),(void*)hdr->data.block,hdr->SPR * count * NS * sizeof(biosig_data_type));
	free(hdr->data.block);	


//	hdr2ascii(hdr,stdout,4);	

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

		FileType = mxCreateString("unknown");
		switch (hdr->TYPE) {
		case ABF: 	{ FileType = mxCreateString("ABF"); break; }
		case ACQ: 	{ FileType = mxCreateString("ACQ"); break; }
		case ACR_NEMA: 	{ FileType = mxCreateString("ACR_NEMA"); break; }
		case AINF: 	{ FileType = mxCreateString("AINF"); break; }
		case AIFC: 	{ FileType = mxCreateString("AIFC"); break; }
		case AIFF: 	{ FileType = mxCreateString("AIFF"); break; }
		case AU: 	{ FileType = mxCreateString("AU"); break; }

		case BCI2000: 	{ FileType = mxCreateString("BCI2000"); break; }
		case BDF: 	{ FileType = mxCreateString("BDF"); break; }
		case BMP: 	{ FileType = mxCreateString("BMP"); break; }
		case BrainVision: 	{ FileType = mxCreateString("BrainVision"); break; }
		case BZ2: 	{ FileType = mxCreateString("BZ2"); break; }
		case BKR: 	{ FileType = mxCreateString("BKR"); break; }

		case DEMG: 	{ FileType = mxCreateString("DEMG"); break; }
		case CFWB: 	{ FileType = mxCreateString("CFWB"); break; }
		case CNT: 	{ FileType = mxCreateString("CNT"); break; }
		case DICOM: 	{ FileType = mxCreateString("DICOM"); break; }

		case EDF: 	{ FileType = mxCreateString("EDF"); break; }
		case EEProbe: 	{ FileType = mxCreateString("EEProbe"); break; }
		case EGI: 	{ FileType = mxCreateString("EGI"); break; }
		case ETG4000: 	{ FileType = mxCreateString("ETG4000"); break; }
		case EXIF: 	{ FileType = mxCreateString("EXIF"); break; }
		case FAMOS: 	{ FileType = mxCreateString("FAMOS"); break; }
		case FEF: 	{ FileType = mxCreateString("FEF"); break; }
		case FITS: 	{ FileType = mxCreateString("FITS"); break; }
		case FLAC: 	{ FileType = mxCreateString("FLAC"); break; }

		case GDF: 	{ FileType = mxCreateString("GDF"); break; }
		case GIF: 	{ FileType = mxCreateString("GIF"); break; }
		case GZIP: 	{ FileType = mxCreateString("GZIP"); break; }
		case HL7aECG: 	{ FileType = mxCreateString("HL7aECG"); break; }
		case JPEG: 	{ FileType = mxCreateString("JPEG"); break; }

		case Matlab: 	{ FileType = mxCreateString("MAT"); break; }
		case MFER: 	{ FileType = mxCreateString("MFER"); break; }
		case MIDI: 	{ FileType = mxCreateString("MIDI"); break; }
		case NetCDF: 	{ FileType = mxCreateString("NetCDF"); break; }
		case NEX1: 	{ FileType = mxCreateString("NEX1"); break; }
		case OGG: 	{ FileType = mxCreateString("OGG"); break; }

		case RIFF: 	{ FileType = mxCreateString("RIFF"); break; }
		case SCP_ECG: 	{ FileType = mxCreateString("SCP"); break; }
		case SIGIF: 	{ FileType = mxCreateString("SIGIF"); break; }
		case SMA: 	{ FileType = mxCreateString("SMA"); break; }
		case SND: 	{ FileType = mxCreateString("SND"); break; }
		case SVG: 	{ FileType = mxCreateString("SVG"); break; }
		case TIFF: 	{ FileType = mxCreateString("TIFF"); break; }
		case VRML: 	{ FileType = mxCreateString("VRML"); break; }
		case VTK: 	{ FileType = mxCreateString("VTK"); break; }

		case WAV: 	{ FileType = mxCreateString("WAV"); break; }
		case WMF: 	{ FileType = mxCreateString("WMF"); break; }
		case XML: 	{ FileType = mxCreateString("XML"); break; }
		case ZIP: 	{ FileType = mxCreateString("ZIP"); break; }
		case ZIP2: 	{ FileType = mxCreateString("ZIP2"); break; }
		case Z: 	{ FileType = mxCreateString("Z"); break; }
		default: 	  FileType = mxCreateString("unknown");
		}

		mxSetField(HDR,0,"TYPE",FileType);
		mxSetField(HDR,0,"VERSION",mxCreateDoubleScalar(hdr->VERSION));
		mxSetField(HDR,0,"NS",mxCreateDoubleScalar(hdr->NS));
		mxSetField(HDR,0,"SPR",mxCreateDoubleScalar(hdr->SPR));
		mxSetField(HDR,0,"NRec",mxCreateDoubleScalar(hdr->NRec));
		mxSetField(HDR,0,"SampleRate",mxCreateDoubleScalar(hdr->SampleRate));
		mxSetField(HDR,0,"Dur",mxCreateDoubleScalar((double)hdr->Dur[0]/hdr->Dur[1]));
		mxSetField(HDR,0,"FileName",mxCreateCharMatrixFromStrings(1,&hdr->FileName));

		mxSetField(HDR,0,"T0",mxCreateDoubleScalar(ldexp(hdr->T0,-32)));

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
		mxArray *LowPass     = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
		mxArray *HighPass    = mxCreateDoubleMatrix(1,hdr->NS, mxREAL);
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
		const char *event_fields[] = {"SampleRate","TYP","POS","DUR","CHN",NULL};
		for (numfields=0; event_fields[numfields++] != 0; );
		EVENT = mxCreateStructMatrix(1, 1, --numfields, event_fields);
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
	
		mxSetField(HDR,0,"Patient",Patient);

		plhs[1] = HDR;
	}
	if (VERBOSE_LEVEL>8) fprintf(stdout,"[131] going for SCLOSE\n");
	sclose(hdr);
	hdr = NULL; 
	if (VERBOSE_LEVEL>8) fprintf(stdout,"[137] SCLOSE finished\n");

};

