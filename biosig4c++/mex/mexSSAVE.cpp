/*

    $Id$
    Copyright (C) 2011 Alois Schloegl <alois.schloegl@gmail.com>
    This file is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.

 */

#include "mex.h"
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../biosig-dev.h"
#ifdef tmwtypes_h
  #if (MX_API_VER<=0x07020000)
    typedef int mwSize;
  #endif 
#endif 

typedef mwSize Int;
#define TRUE (1)

//#define VERBOSE_LEVEL  9 
//extern int VERBOSE_LEVEL;
//#define DEBUG

double getDouble(const mxArray *pm, size_t idx) {
	size_t n = mxGetNumberOfElements(pm);
	if (n == 0)   return(0.0/0.0);
	if (n <= idx) idx = n-1;

	switch (mxGetClassID(pm)) {
	case mxCHAR_CLASS:
	case mxLOGICAL_CLASS:
	case mxINT8_CLASS:
		return(*((int8_t*)mxGetData(pm) + idx));
	case mxUINT8_CLASS:
		return(*((uint8_t*)mxGetData(pm) + idx));
	case mxDOUBLE_CLASS:
		return(*((double*)mxGetData(pm) + idx));
	case mxSINGLE_CLASS:
		return(*((float*)mxGetData(pm) + idx));
	case mxINT16_CLASS:
		return(*((int16_t*)mxGetData(pm) + idx));
	case mxUINT16_CLASS:
		return(*((uint16_t*)mxGetData(pm) + idx));
	case mxINT32_CLASS:
		return(*((int32_t*)mxGetData(pm) + idx));
	case mxUINT32_CLASS:
		return(*((uint32_t*)mxGetData(pm) + idx));
	case mxINT64_CLASS:
		return(*((int64_t*)mxGetData(pm) + idx));
	case mxUINT64_CLASS:
		return(*((uint64_t*)mxGetData(pm) + idx));
/*
	case mxFUNCTION_CLASS:
	case mxUNKNOWN_CLASS:
	case mxCELL_CLASS:
	case mxSTRUCT_CLASS:
	default: 
*/
	}
	return(0.0/0.0);
}

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
	size_t 		count;
	time_t 		T0;
	char 		FileName[1024];  
	char 		tmpstr[128];  
	int 		status; 
	int		CHAN = 0;
	double		*ChanList=NULL;
	int		NS = -1;
	char		FlagOverflowDetection = 1, FlagUCAL = 0;
	char		FLAG_CNT32 = 0;
	void 		*data = NULL;
	mxArray *p = NULL, *p1 = NULL, *p2 = NULL;

#ifdef CHOLMOD_H
	cholmod_sparse RR,*rr=NULL;
	double dummy;
#endif 

// ToDO: output single data 
//	mxClassId	FlagMXclass=mxDOUBLE_CLASS;
	

	if (nrhs<1) {
		mexPrintf("   Usage of mexSSAVE:\n");
		mexPrintf("\tstatus=mexSSAVE(HDR,data)\n");
		mexPrintf("   Input:\n\tHDR\tHeader structure \n");
		mexPrintf("\tdata\tdata matrix, one channel per column\n");
		mexPrintf("\tHDR\theader structure\n\n");
		mexPrintf("\tstatus 0: file saves successfully\n\n");
		mexPrintf("\tstatus <>0: file could not saved\n\n");
		return; 
	}

/*
 	improve checks for input arguments
*/
	/* process input arguments */
	if (mxIsNumeric(prhs[1]) &&
	    mxIsStruct( prhs[0])) {
		data      = (void*) mxGetData(prhs[1]);
		// get number of channels
		size_t NS = mxGetN (prhs[1]);
		// get number of events
		size_t NEvt=0;
		if ( (p = mxGetField(prhs[0], 0, "EVENT") ) != NULL ) {
			if ( (p1 = mxGetField(p, 0, "POS") ) != NULL ) {
				NEvt = mxGetNumberOfElements(p1);
			}
			if ( (p1 = mxGetField(p, 0, "TYP") ) != NULL ) {
				size_t n = mxGetNumberOfElements(p1);
				if (n>NEvt) NEvt = n; 
			}
		}
		
		// allocate memory for header structure
		hdr       = constructHDR (NS, NEvt);
		data      = (biosig_data_type*) mxGetData (prhs[1]);
		hdr->NRec = mxGetM (prhs[1]);
		hdr->SPR  = 1;
	}
	else {
		mexErrMsgTxt("mexSSAVE(HDR,data) failed because HDR and data, are not a struct and numeric, resp.\n");	
		return; 
	}	


	for (k = 2; k < nrhs; k++) {	
		arg = prhs[k];
		
		if (mxIsChar(arg)) {
#ifdef DEBUG		
			mexPrintf("arg[%i]=%s \n",k,mxArrayToString(prhs[k]));
#endif
			if (!strcmp(mxArrayToString(prhs[k]), "OVERFLOWDETECTION:ON"))
				FlagOverflowDetection = 1;
			else if (!strcmp(mxArrayToString(prhs[k]), "OVERFLOWDETECTION:OFF"))
				FlagOverflowDetection = 0;
			else if (!strcmp(mxArrayToString(prhs[k]), "UCAL:ON")) 
				FlagUCAL = 1;
			else if (!strcmp(mxArrayToString(prhs[k]), "UCAL:OFF"))
				FlagUCAL = 0;
		}
		else {
#ifndef mexSOPEN
			mexPrintf("mexSSAVE: argument #%i is invalid.",k+1);	
			mexErrMsgTxt("mexSSAVE failes because of unknown parameter\n");	
#else
			mexPrintf("mexSOPEN: argument #%i is invalid.",k+1);	
			mexErrMsgTxt("mexSOPEN fails because of unknown parameter\n");	
#endif
		}
	}

	/***** SET INPUT ARGUMENTS *****/
	hdr->FLAG.OVERFLOWDETECTION = FlagOverflowDetection; 
	hdr->FLAG.UCAL = FlagUCAL;
	hdr->FLAG.CNT32 = FLAG_CNT32;
#ifdef CHOLMOD_H
	hdr->FLAG.ROW_BASED_CHANNELS = (rr!=NULL); 
#else 	
	hdr->FLAG.ROW_BASED_CHANNELS = 0; 
#endif 


	/***** CHECK INPUT HDR STUCTURE CONVERT TO libbiosig hdr *****/
	if (VERBOSE_LEVEL>7) mexPrintf("110: input arguments checked\n");

	if ( (p = mxGetField(prhs[0], 0, "TYPE") ) != NULL ) {
		mxGetString(p,tmpstr,sizeof(tmpstr));
		hdr->TYPE 	= GetFileTypeFromString(tmpstr);
	}
	if ( (p = mxGetField(prhs[0], 0, "VERSION") ) != NULL ) {
		mxGetString(p, tmpstr, sizeof(tmpstr));
		hdr->VERSION 	= atof(tmpstr);
	}
	if ( (p = mxGetField(prhs[0], 0, "T0") ) != NULL ) 		hdr->T0 	= (gdf_time)getDouble(p, 0);
	if ( (p = mxGetField(prhs[0], 0, "FileName") ) != NULL ) 	if (~mxGetString(p,FileName,1024)) hdr->FileName = FileName;
	if ( (p = mxGetField(prhs[0], 0, "SampleRate") ) != NULL ) 	hdr->SampleRate = getDouble(p, 0);
	if ( (p = mxGetField(prhs[0], 0, "NS") ) != NULL )	 	hdr->NS         = getDouble(p, 0);

#ifdef DEBUG		
			mexPrintf("mexSSAVE [400] TYPE=<%s><%s> VERSION=%f\n",tmpstr,GetFileTypeString(hdr->TYPE),hdr->VERSION);
#endif

	p1 = mxGetField(prhs[0], 0, "SPR");
	p2 = mxGetField(prhs[0], 0, "NRec");
	if      ( p1 && p2) {
		hdr->SPR  = (size_t)getDouble(p1, 0);
		hdr->NRec = (size_t)getDouble(p2, 0);
	}
	else if (!p1 && p2) {
		hdr->SPR  = hdr->NRec;
		hdr->NRec = (size_t)getDouble(p2, 0);
		hdr->SPR  /= hdr->NRec;
	}
	else if ( p1 && !p2) {
		hdr->SPR  = (size_t)getDouble(p1, 0);
		hdr->NRec /= hdr->SPR;
	}
	else if (!p1 && !p2) {
		; /* use default values SPR=1, NREC = size(data,1) */
	}

	if (hdr->NRec * hdr->SPR != mxGetM (prhs[1]) )	
		mexPrintf("mexSSAVE: warning HDR.NRec * HDR.SPR (%i*%i = %i) does not match number of rows (%i) in data.", hdr->NRec, hdr->SPR, hdr->NRec*hdr->SPR, mxGetM(prhs[1]) );	


	if ( (p = mxGetField(prhs[0], 0, "Label") ) != NULL ) {
		if ( mxIsCell(p) ) {
			for (k = 0; k < hdr->NS; k++) 
				mxGetString(mxGetCell(p,k), hdr->CHANNEL[k].Label, MAX_LENGTH_LABEL+1);
		}
	}
	if ( (p = mxGetField(prhs[0], 0, "Transducer") ) != NULL ) {
		if ( mxIsCell(p) ) {
			for (k = 0; k < hdr->NS; k++) 
				mxGetString(mxGetCell(p,k), hdr->CHANNEL[k].Transducer, MAX_LENGTH_LABEL+1);
		}
	}


	if ( (p = mxGetField(prhs[0], 0, "LowPass") ) != NULL ) {
		for (k = 0; k < hdr->NS; k++) 
			hdr->CHANNEL[k].LowPass = (float)getDouble(p,k);
	}
	if ( (p = mxGetField(prhs[0], 0, "HighPass") ) != NULL ) {
		for (k = 0; k < hdr->NS; k++) 
			hdr->CHANNEL[k].HighPass = (float)getDouble(p,k);
	}
	if ( (p = mxGetField(prhs[0], 0, "Notch") ) != NULL ) {
		for (k = 0; k < hdr->NS; k++) 
			hdr->CHANNEL[k].Notch = (float)getDouble(p,k);
	}
	if ( (p = mxGetField(prhs[0], 0, "PhysMax") ) != NULL ) {
		for (k = 0; k < hdr->NS; k++) 
			hdr->CHANNEL[k].PhysMax = (double)getDouble(p,k);
	}
	if ( (p = mxGetField(prhs[0], 0, "PhysMin") ) != NULL ) {
		for (k = 0; k < hdr->NS; k++) 
			hdr->CHANNEL[k].PhysMin = (double)getDouble(p,k);
	}
	if ( (p = mxGetField(prhs[0], 0, "DigMax") ) != NULL ) {
		for (k = 0; k < hdr->NS; k++) 
			hdr->CHANNEL[k].DigMax = (double)getDouble(p,k);
	}
	if ( (p = mxGetField(prhs[0], 0, "DigMin") ) != NULL ) {
		for (k = 0; k < hdr->NS; k++) 
			hdr->CHANNEL[k].DigMin = (double)getDouble(p,k);
	}

	if ( (p = mxGetField(prhs[0], 0, "PhysDimCode") ) != NULL ) {
		for (k = 0; k < hdr->NS; k++) 
			hdr->CHANNEL[k].PhysDimCode = (uint16_t)getDouble(p,k);
	}
	else if ( (p = mxGetField(prhs[0], 0, "PhysDim") ) != NULL ) {
		if ( mxIsCell(p) ) {
			for (k = 0; k < hdr->NS; k++) 
				mxGetString(mxGetCell(p,k), tmpstr, sizeof(tmpstr));
				hdr->CHANNEL[k].PhysDimCode = PhysDimCode(tmpstr);
		}
	}

	if ( (p = mxGetField(prhs[0], 0, "GDFTYP") ) != NULL ) {
		for (k = 0; k < hdr->NS; k++) 
			hdr->CHANNEL[k].GDFTYP = (uint16_t)getDouble(p,k);
	}
	if ( (p = mxGetField(prhs[0], 0, "TOffset") ) != NULL ) {
		for (k = 0; k < hdr->NS; k++) 
			hdr->CHANNEL[k].TOffset = (float)getDouble(p,k);
	}
	if ( (p = mxGetField(prhs[0], 0, "Impedance") ) != NULL ) {
		for (k = 0; k < hdr->NS; k++) 
			hdr->CHANNEL[k].Impedance = (float)getDouble(p,k);
	}
	if ( (p = mxGetField(prhs[0], 0, "fZ") ) != NULL ) {
		for (k = 0; k < hdr->NS; k++) 
			hdr->CHANNEL[k].fZ = (float)getDouble(p,k);
	}
	if ( (p = mxGetField(prhs[0], 0, "AS") ) != NULL ) {
		if ( (p1 = mxGetField(p, 0, "SPR") ) != NULL ) {
			// define channel-based samplingRate, HDR.SampleRate*HDR.AS.SPR(channel)/HDR.SPR; 
			for (k = 0; k < hdr->NS; k++) 
				hdr->CHANNEL[k].SPR = (double)getDouble(p1,k);
		}
	}

	if ( (p = mxGetField(prhs[0], 0, "Patient") ) != NULL ) {
		if ( (p1 = mxGetField(p, 0, "Id") ) != NULL ) 
			if (mxIsChar(p1)) mxGetString(p1, hdr->Patient.Id, MAX_LENGTH_PID+1);
		if ( (p1 = mxGetField(p, 0, "Name") ) != NULL ) 
			if (mxIsChar(p1)) mxGetString(p1, hdr->Patient.Name, MAX_LENGTH_PID+1);
		if ( (p1 = mxGetField(p, 0, "Sex") ) != NULL ) {
			if (mxIsChar(p1)) {
				char sex = toupper(*mxGetChars(p1));
				hdr->Patient.Sex = (sex=='M') + 2*(sex=='F');
			} 
			else 
				hdr->Patient.Sex = (int8_t)getDouble(p1,0);
		}

		if ( (p1 = mxGetField(p, 0, "Handedness") ) != NULL ) 
			hdr->Patient.Handedness = (int8_t)getDouble(p1,0);
		if ( (p1 = mxGetField(p, 0, "Smoking") ) != NULL ) 
			hdr->Patient.Smoking = (int8_t)getDouble(p1,0);
		if ( (p1 = mxGetField(p, 0, "AlcoholAbuse") ) != NULL ) 
			hdr->Patient.AlcoholAbuse = (int8_t)getDouble(p1,0);
		if ( (p1 = mxGetField(p, 0, "DrugAbuse") ) != NULL ) 
			hdr->Patient.DrugAbuse = (int8_t)getDouble(p1,0);
		if ( (p1 = mxGetField(p, 0, "Medication") ) != NULL ) 
			hdr->Patient.Medication = (int8_t)getDouble(p1,0);
		if ( (p1 = mxGetField(p, 0, "Impairment") ) != NULL ) {
			if ( (p2 = mxGetField(p1, 0, "Visual") ) != NULL ) 
				hdr->Patient.Impairment.Visual = (int8_t)getDouble(p2,0);
			if ( (p2 = mxGetField(p1, 0, "Heart") ) != NULL ) 
				hdr->Patient.Impairment.Heart = (int8_t)getDouble(p2,0);
		}

		if ( (p1 = mxGetField(p, 0, "Weight") ) != NULL ) 
			hdr->Patient.Weight = (uint8_t)getDouble(p1,0);
		if ( (p1 = mxGetField(p, 0, "Height") ) != NULL ) 
			hdr->Patient.Height = (uint8_t)getDouble(p1,0);
		if ( (p1 = mxGetField(p, 0, "Birthday") ) != NULL ) 
			hdr->Patient.Birthday = (gdf_time)getDouble(p1,0);
	}

	if ( (p = mxGetField(prhs[0], 0, "ID") ) != NULL ) {
		if ( (p1 = mxGetField(p, 0, "Recording") ) != NULL ) 
			if (mxIsChar(p1)) mxGetString(p1, hdr->ID.Recording, MAX_LENGTH_RID+1);
		if ( (p1 = mxGetField(p, 0, "Technician") ) != NULL ) 
			if (mxIsChar(p1)) mxGetString(p1, hdr->ID.Technician, MAX_LENGTH_TECHNICIAN+1);
		if ( (p1 = mxGetField(p, 0, "Hospital") ) != NULL ) 
			;//FIXME:// if (mxIsChar(p1)) hdr->ID.Hospital=mxGetChars(p1);
		if ( (p1 = mxGetField(p, 0, "Equipment") ) != NULL ) 
			memcpy(&hdr->ID.Equipment,mxGetData(p1), 8);
		if ( (p1 = mxGetField(p, 0, "Manufacturer") ) != NULL ) {
			uint8_t pos = 0; 
			if ( ( (p2 = mxGetField(p1, 0, "Name") ) != NULL ) &&  mxIsChar(p2)) {
					//hdr->ID.Manufacturer.Name=mxGetChars(p2);
					mxGetString(p1, hdr->ID.Manufacturer._field,MAX_LENGTH_MANUF);
					pos = strlen(hdr->ID.Manufacturer._field)+1;
				}
			else {
				hdr->ID.Manufacturer._field[pos++] = 0;
			}

			if ( ( (p2 = mxGetField(p1, 0, "Model") ) != NULL ) && mxIsChar(p2)) { 
					//hdr->ID.Manufacturer.Model=mxGetChars(p2);
					mxGetString(p1, hdr->ID.Manufacturer._field + pos, MAX_LENGTH_MANUF);
					pos += strlen(hdr->ID.Manufacturer._field + pos)+1;
				}
			else {
				hdr->ID.Manufacturer._field[pos++] = 0;
			}

			if ( ( (p2 = mxGetField(p1, 0, "Version") ) != NULL ) && mxIsChar(p2)) { 
					//hdr->ID.Manufacturer.Version=mxGetChars(p2);
					mxGetString(p1, hdr->ID.Manufacturer._field + pos, MAX_LENGTH_MANUF);
					pos += strlen(hdr->ID.Manufacturer._field+pos)+1;
				}
			else {
				hdr->ID.Manufacturer._field[pos++] = 0;
			}

			if ( ( (p2 = mxGetField(p1, 0, "SerialNumber") ) != NULL ) && mxIsChar(p2)) {
					//hdr->ID.Manufacturer.SerialNumber=mxGetChars(p2);
					mxGetString(p1, hdr->ID.Manufacturer._field + pos, MAX_LENGTH_MANUF);
					pos += strlen(hdr->ID.Manufacturer._field+pos)+1;
				}
			else {
				hdr->ID.Manufacturer._field[pos++] = 0;
			}
		}
	}

	if ( (p = mxGetField(prhs[0], 0, "FLAG") ) != NULL ) {
		if ( (p1 = mxGetField(p, 0, "OVERFLOWDETECTION") ) != NULL ) 
			hdr->FLAG.OVERFLOWDETECTION = (char)getDouble(p1,0);
		if ( (p1 = mxGetField(p, 0, "UCAL") ) != NULL ) 
			hdr->FLAG.UCAL = (char)getDouble(p1,0);
		if ( (p1 = mxGetField(p, 0, "ANONYMOUS") ) != NULL ) 
			hdr->FLAG.ANONYMOUS = (char)getDouble(p1,0);
		if ( (p1 = mxGetField(p, 0, "ROW_BASED_CHANNELS") ) != NULL ) 
			hdr->FLAG.ROW_BASED_CHANNELS = (char)getDouble(p1,0);
		if ( (p1 = mxGetField(p, 0, "CNT32") ) != NULL ) 
			hdr->FLAG.CNT32 = (char)getDouble(p1,0);
	}

	if ( (p = mxGetField(prhs[0], 0, "EVENT") ) != NULL ) {
		if ( (p1 = mxGetField(p, 0, "SampleRate") ) != NULL ) {
			hdr->EVENT.SampleRate = (double)getDouble(p1,0);
		}
		if ( (p1 = mxGetField(p, 0, "POS") ) != NULL ) {
			size_t n = mxGetNumberOfElements(p1);
			for (k = 0; k < n; k++) 
				hdr->EVENT.POS[k] = (uint32_t)getDouble(p1,k);
		}
		if ( (p1 = mxGetField(p, 0, "TYP") ) != NULL ) {
			size_t n = mxGetNumberOfElements(p1);
			for (k = 0; k < n; k++) 
				hdr->EVENT.TYP[k] = (uint16_t)getDouble(p1,k);
		}
		if ( (p1 = mxGetField(p, 0, "DUR") ) != NULL ) {
			size_t n = mxGetNumberOfElements(p1);
			for (k = 0; k < n; k++) 
				hdr->EVENT.DUR[k] = (uint32_t)getDouble(p1,k);
		}
		if ( (p1 = mxGetField(p, 0, "CHN") ) != NULL ) {
			size_t n = mxGetNumberOfElements(p1);
			for (k = 0; k < n; k++) 
				hdr->EVENT.CHN[k] = (uint16_t)getDouble(p1,k);
		}
	}

	hdr = sopen(hdr->FileName, "w", hdr);
	if (serror()) mexErrMsgTxt("mexSSAVE: sopen failed \n");	

	swrite((biosig_data_type*)data, hdr->NRec, hdr);
	if (serror()) mexErrMsgTxt("mexSSAVE: swrite failed \n");	

	destructHDR(hdr);
	if (serror()) mexErrMsgTxt("mexSSAVE: sclose failed \n");	

};

