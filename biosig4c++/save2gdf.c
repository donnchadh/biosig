/*

    $Id$
    Copyright (C) 2000,2005,2007,2008,2009,2010,2011 Alois Schloegl <alois.schloegl@gmail.com>
    Copyright (C) 2007 Elias Apostolopoulos
    This file is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 
 

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. 
    
 */

// T1T2 is an experimental flag for testing segment selection 
#define T1T2


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "biosig-dev.h"

#ifndef INF
#define INF (1.0/0.0)
#endif 

#ifdef __cplusplus
extern "C" {
#endif 
int savelink(const char* filename);
#ifdef __cplusplus
} 
#endif 

#ifdef WITH_PDP 
void sopen_pdp_read(HDRTYPE *hdr);
#endif


int main(int argc, char **argv){
    
    HDRTYPE 	*hdr; 
    size_t 	count, k1, ne=0;
    uint16_t 	numopt = 0;
    char 	*source, *dest, tmp[1024], *tmpstr; 
    enum FileFormat SOURCE_TYPE, TARGET_TYPE=GDF; 		// type of file format
    int		COMPRESSION_LEVEL=0;
    int		status, k; 
    int		TARGETSEGMENT=1; 	// select segment in multi-segment file format EEG1100 (Nihon Kohden)
    int 	VERBOSE	= 1; 
    char	FLAG_CNT32 = 0; 	// assume CNT format is 16bit
    char	*argsweep = NULL;
    double	t1=0.0, t2=1.0/0.0;
#ifdef CHOLMOD_H
    cholmod_sparse *rr  = NULL; 
    char        *rrFile = NULL;
    int refarg          = 0;
#endif 
	
    for (k=1; k<argc; k++) {
    	if (!strcmp(argv[k],"-v") || !strcmp(argv[k],"--version") ) {
		fprintf(stdout,"save2gdf (BioSig4C++) v%04.2f\n", BIOSIG_VERSION);
		fprintf(stdout,"Copyright (C) 2006,2007,2008,2009 by Alois Schloegl and others\n");
		fprintf(stdout,"This file is part of BioSig http://biosig.sf.net - the free and\n");
		fprintf(stdout,"open source software library for biomedical signal processing.\n\n");
		fprintf(stdout,"BioSig is free software; you can redistribute it and/or modify\n");
		fprintf(stdout,"it under the terms of the GNU General Public License as published by\n");
		fprintf(stdout,"the Free Software Foundation; either version 3 of the License, or\n");
		fprintf(stdout,"(at your option) any later version.\n\n");
	}	
    	else if (!strcmp(argv[k],"-h") || !strcmp(argv[k],"--help") ) {
		fprintf(stdout,"\nusage: save2gdf [OPTIONS] SOURCE DEST\n");
		fprintf(stdout,"  SOURCE is the source file \n");
		fprintf(stdout,"      SOURCE can be also network file bscs://<hostname>/ID e.g. bscs://129.27.3.99/9aebcc5b4eef1024 \n");
		fprintf(stdout,"  DEST is the destination file \n");
		fprintf(stdout,"      DEST can be also network server bscs://<hostname>\n");
		fprintf(stdout,"      The corresponding ID number is reported and a bscs-link file is stored in /tmp/<SOURCE>.bscs\n");
		fprintf(stdout,"\n  Supported OPTIONS are:\n");
		fprintf(stdout,"   -v, --version\n\tprints version information\n");
		fprintf(stdout,"   -h, --help   \n\tprints this information\n");
#ifdef T1T2
		fprintf(stdout,"   [t1,t2]\n\tstart and end time in seconds (do not use any spaces). \n");
		fprintf(stdout,"\tTHIS FEATURE IS CURRENTLY EXPERIMENTAL !!!\n");
#endif
#ifdef CHOLMOD_H
		fprintf(stdout,"   -r, --ref=MM  \n\trereference data with matrix file MM. \n\tMM must be a 'MatrixMarket matrix coordinate real general' file.\n");
#endif
		fprintf(stdout,"   -cnt32\n\tmust be set for reading 32 bit CNT files\n");
		fprintf(stdout,"   -f=FMT  \n\tconverts data into format FMT\n");
		fprintf(stdout,"\tFMT must represent a valid target file format\n"); 
		fprintf(stdout,"\tCurrently are supported: HL7aECG, SCP_ECG (EN1064), GDF, EDF, BDF, CFWB, BIN, ASCII, BVA (BrainVision)\n"); 
		fprintf(stdout,"   -z=#, -z#\n\t# indicates the compression level (#=0 no compression; #=9 best compression, default #=1)\n");
		fprintf(stdout,"   -s=#\tselect target segment # (in the multisegment file format EEG1100)\n");
		fprintf(stdout,"   -SWEEP=ne,ng,ns\n\tsweep selection of HEKA/PM files\n\tne,ng, and ns select the number of experiment, the number of group, and the sweep number, resp.\n");
		fprintf(stdout,"   -VERBOSE=#, verbosity level #\n\t0=silent [default], 9=debugging\n");
		fprintf(stdout,"\n\n");
		return(0);
	}	
    	else if (!strncmp(argv[k],"-z",2))  	{
#ifdef ZLIB_H
		COMPRESSION_LEVEL = 1;	// default 
		char *s = argv[k] + 2; 
		if (s[0] == '=') s++;	// skip "="
		if (strlen(s)>0) COMPRESSION_LEVEL = atoi(s);
    		if (COMPRESSION_LEVEL<0 || COMPRESSION_LEVEL>9)
			fprintf(stderr,"Error %s: Invalid Compression Level %s\n",argv[0],argv[k]); 
#else
	     	fprintf(stderr,"Warning: option -z (compression) not supported. zlib not linked.\n");
#endif 
	}
    	else if (!strncmp(argv[k],"-VERBOSE",2))  {
	    	VERBOSE = argv[k][strlen(argv[k])-1]-48;
#ifndef VERBOSE_LEVEL
	// then VERBOSE_LEVEL is not a constant but a variable
	VERBOSE_LEVEL = VERBOSE; 
#endif
	}
    	else if (!strncmpi(argv[k],"-SWEEP=",7))  {
	    	argsweep = argv[k]+6;

	}
    	else if (!strncmp(argv[k],"-f=",3))  	{
    		if (0) {}
    		else if (!strncmp(argv[k],"-f=ASCII",8))
			TARGET_TYPE=ASCII;
    		else if (!strcmp(argv[k],"-f=BDF"))
			TARGET_TYPE=BDF;
    		else if (!strncmp(argv[k],"-f=BIN",6))
			TARGET_TYPE=BIN;
    		else if (!strncmp(argv[k],"-f=BVA",6))
			TARGET_TYPE=BrainVision;
    		else if (!strncmp(argv[k],"-f=CFWB",7))
			TARGET_TYPE=CFWB;
    		else if (!strcmp(argv[k],"-f=EDF"))
			TARGET_TYPE=EDF;
    	 	else if (!strcmp(argv[k],"-f=GDF"))
			TARGET_TYPE=GDF;
    		else if (!strcmp(argv[k],"-f=GDF1"))
			TARGET_TYPE=GDF1;
    		else if (!strncmp(argv[k],"-f=HL7",6))
			TARGET_TYPE=HL7aECG;
    		else if (!strncmp(argv[k],"-f=MFER",7))
			TARGET_TYPE=MFER;
    		else if (!strncmp(argv[k],"-f=SCP",6))
			TARGET_TYPE=SCP_ECG;
//    		else if (!strncmp(argv[k],"-f=TMSi",7))
//			TARGET_TYPE=TMSiLOG;
		else {
			fprintf(stderr,"format %s not supported.\n",argv[k]);
			return(-1);
		}	
	}

#ifdef CHOLMOD_H
    	else if ( !strncmp(argv[k],"-r=",3) || !strncmp(argv[k],"--ref=",6) )	{
    	        // re-referencing matrix
		refarg = k; 
	}
#endif

    	else if (!strncmp(argv[k],"-s=",3))  {
    		TARGETSEGMENT = atoi(argv[k]+3);
	}

    	else if (!strncmp(argv[k],"-cnt32",3))  {
    		FLAG_CNT32 = 1;
	}

    	else if (argv[k][0]=='[' && argv[k][strlen(argv[k])-1]==']' && (tmpstr=strchr(argv[k],',')) )  	{
		t1 = strtod(argv[k]+1,NULL);
		t2 = strtod(tmpstr+1,NULL);
		fprintf(stdout,"[%f,%f\n]",t1,t2);
	}
	
	else {
		numopt = k-1;
		break; 		
	}

	if (VERBOSE_LEVEL>7) 
		fprintf(stdout,"[103] save2gdf: arg%i = <%s>\n", k, argv[k]);

    }


	source = NULL;
	dest = NULL;
    	switch (argc - numopt) {
    	case 1:
		fprintf(stderr,"save2gdf: missing file argument\n");
		fprintf(stdout,"usage: save2gdf [options] SOURCE DEST\n");
		fprintf(stdout," for more details see also save2gdf --help \n");
		exit(-1);
    	case 3:
    		dest   = argv[numopt+2]; 
    	case 2:
	    	source = argv[numopt+1]; 
    	}	

	if (VERBOSE_LEVEL<0) VERBOSE=1; // default 
	if (VERBOSE_LEVEL>8) fprintf(stdout,"[108] SAVE2GDF started\n");

	tzset();
	hdr = constructHDR(0,0);
	// hdr->FLAG.OVERFLOWDETECTION = FlagOverflowDetection; 
	hdr->FLAG.UCAL = ((TARGET_TYPE==BIN) || (TARGET_TYPE==ASCII));
	hdr->FLAG.TARGETSEGMENT = TARGETSEGMENT;
	hdr->FLAG.CNT32 = FLAG_CNT32;
	// hdr->FLAG.ANONYMOUS = 0; 	// personal names are processed 
	
	if (argsweep) {
		k = 0;
		do {
	if (VERBOSE_LEVEL>7) fprintf(stdout,"SWEEP [109] %i: %s\t",k,argsweep);
			hdr->AS.SegSel[k++] = strtod(argsweep+1, &argsweep);
	if (VERBOSE_LEVEL>7) fprintf(stdout,",%i\n",hdr->AS.SegSel[k-1]);
		} while (argsweep[0]==',' && (k < 5) );
	}

	hdr->FileName = source;
	hdr = sopen(source, "r", hdr);
#ifdef WITH_PDP 
	if (B4C_ERRNUM) {
		B4C_ERRNUM = 0;  
		sopen_pdp_read(hdr);
	}	
#endif
#ifdef CHOLMOD_H
	if (refarg > 0) {
    	        rrFile = strchr(argv[refarg], '=') + 1;
	        if (RerefCHANNEL(hdr, rrFile, 1))
	                fprintf(stdout,"error: option %s not supported\n",argv[refarg]);
	} 
#endif

	if (VERBOSE_LEVEL>7) fprintf(stdout,"[112] SOPEN-R finished\n");

	if ((status=serror())) {
		destructHDR(hdr);
		exit(status); 
	} 
	
	t1 *= hdr->SampleRate / hdr->SPR;
	t2 *= hdr->SampleRate / hdr->SPR;
	if (isnan(t1)) t1 = 0.0;
	t2 = min(t2,hdr->NRec-t1);
	if ( ( t1 - floor (t1) ) || ( ( t2 - floor(t2) ) && (t2 < INF) ) ) {
		fprintf(stderr,"ERROR SAVE2GDF: cutting from parts of blocks not supported; t1 (%f) and t2 (%f) must be a multiple of block duration %f\n", t1,t2,hdr->SPR / hdr->SampleRate);
		B4C_ERRNUM = B4C_UNSPECIFIC_ERROR;
		B4C_ERRMSG = "blocks must not be split";
	} 

	if ((status=serror())) {
		destructHDR(hdr);
		exit(status); 
	} 
	
	if (VERBOSE_LEVEL>7) fprintf(stdout,"[113] SOPEN-R finished\n");

	hdr2ascii(hdr,stdout,VERBOSE);
//	hdr2ascii(hdr,stdout,3);

	// all channels are converted - channel selection currently not supported
    	for (k=0; k<hdr->NS; k++) {
    		if (!hdr->CHANNEL[k].OnOff && hdr->CHANNEL[k].SPR) {
			if ((hdr->SPR/hdr->CHANNEL[k].SPR)*hdr->CHANNEL[k].SPR != hdr->SPR)
				 fprintf(stdout,"Warning: channel %i might be decimated!\n",k+1);
    		};
    		// hdr->CHANNEL[k].OnOff = 1;	// convert all channels
    	}	

#ifdef CHOLMOD_H
        int flagREREF = hdr->Calib != NULL && hdr->rerefCHANNEL != NULL;
#else
        int flagREREF = 0;
#endif
	hdr->FLAG.OVERFLOWDETECTION = 0;
	hdr->FLAG.UCAL = hdr->FLAG.UCAL && !flagREREF && (TARGET_TYPE==SCP_ECG);
	hdr->FLAG.ROW_BASED_CHANNELS = flagREREF;
	
#ifdef CHOLMOD_H
	if (VERBOSE_LEVEL>7) 
        	fprintf(stdout,"[121] %p %p Flag.ReRef=%i\n",hdr->Calib, hdr->rerefCHANNEL,flagREREF);
#endif

	if (VERBOSE_LEVEL>7) 
		fprintf(stdout,"\n[123] SREAD [%f,%f].\n",t1,t2);

	if (dest != NULL)
		count = sread(NULL, t1, t2, hdr);

	biosig_data_type* data = hdr->data.block;
	if ((VERBOSE_LEVEL>8) && (hdr->data.size[0]*hdr->data.size[1]>500))
		fprintf(stdout,"[125] UCAL=%i %e %e %e \n",hdr->FLAG.UCAL,data[100],data[110],data[500+hdr->SPR]);
	
	if ((status=serror())) {
		destructHDR(hdr);
		exit(status);
	};

	if (VERBOSE_LEVEL>7) 
		fprintf(stdout,"\n[129] SREAD on %s successful [%i,%i].\n",hdr->FileName,hdr->data.size[0],hdr->data.size[1]);

//	fprintf(stdout,"\n %f,%f.\n",hdr->FileName,hdr->data.block[3*hdr->SPR],hdr->data.block[4*hdr->SPR]);
	if (VERBOSE_LEVEL>8) 
		fprintf(stdout,"\n[130] File  %s =%i/%i\n",hdr->FileName,hdr->FILE.OPEN,hdr->FILE.Des);

	if (dest==NULL) {
		if (ne)	/* used for testig SFLUSH_GDF_EVENT_TABLE */
		{
			if (hdr->EVENT.N > ne)
				hdr->EVENT.N -= ne;
			else 
				hdr->EVENT.N  = 0;

			// fprintf(stdout,"Status-SFLUSH %i\n",sflush_gdf_event_table(hdr));
		}

		if (VERBOSE_LEVEL>8) fprintf(stdout,"[131] going for SCLOSE\n");
		sclose(hdr);
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[137] SCLOSE(HDR) finished\n");
		destructHDR(hdr);
		exit(serror());
	}

	if (hdr->FILE.OPEN) {
		sclose(hdr); 
		free(hdr->AS.Header);
		hdr->AS.Header = NULL;
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[138] file closed\n");
	}
	if (VERBOSE_LEVEL>8) 
		fprintf(stdout,"\n[139] File %s closed sd=%i/%i\n",hdr->FileName,hdr->FILE.OPEN,hdr->FILE.Des);

	SOURCE_TYPE = hdr->TYPE;
	hdr->TYPE = TARGET_TYPE;
	if ((hdr->TYPE==GDF) && (hdr->VERSION<2)) hdr->VERSION = 2.0;

	hdr->FILE.COMPRESSION = COMPRESSION_LEVEL;

   /******************************************* 
   	make block size as small as possible  
    *******************************************/

	if (1) {
		uint32_t asGCD=hdr->SPR;
    		for (k=0; k<hdr->NS; k++)
		    	if (hdr->CHANNEL[k].OnOff && hdr->CHANNEL[k].SPR) 
				asGCD = gcd(asGCD, hdr->CHANNEL[k].SPR);
    		hdr->SPR  /= asGCD;
	    	hdr->NRec *= asGCD;
	    	for (k=0; k<hdr->NS; k++)
    			hdr->CHANNEL[k].SPR /= asGCD;
		if (hdr->Calib) 
		    	for (k=0; k<hdr->Calib->nrow; k++)
	    			hdr->rerefCHANNEL[k].SPR /= asGCD;
    	}

   /********************************* 
   	re-referencing
    *********************************/

	if (VERBOSE_LEVEL>7) fprintf(stdout,"[199] %p %p\n",hdr->CHANNEL,hdr->rerefCHANNEL);
#ifdef CHOLMOD_H
	if (VERBOSE_LEVEL>7) fprintf(stdout,"[199] %p %p\n",hdr->CHANNEL,hdr->rerefCHANNEL);

        if (hdr->Calib && hdr->rerefCHANNEL) {
	        if (VERBOSE_LEVEL>6) 
        	        hdr2ascii(hdr,stdout,3);

		hdr->NS = hdr->Calib->ncol; 

                free(hdr->CHANNEL);
                hdr->CHANNEL = hdr->rerefCHANNEL;
                hdr->rerefCHANNEL = NULL;
	if (VERBOSE_LEVEL>7) fprintf(stdout,"[200-]\n");

//                RerefCHANNEL(hdr,NULL,0);	// clear HDR.Calib und HDR.rerefCHANNEL
	if (VERBOSE_LEVEL>7) fprintf(stdout,"[200+]\n");
                hdr->Calib = NULL;

	        if (VERBOSE_LEVEL>6) 
        	        hdr2ascii(hdr,stdout,3);
        } 
#endif

   /********************************* 
   	Write data 
   *********************************/

	//************ identify Max/Min **********
	
	if (VERBOSE_LEVEL>7) fprintf(stdout,"[201]\n");

        double PhysMaxValue0 = -INF; //hdr->data.block[0];
	double PhysMinValue0 = +INF; //hdr->data.block[0];
	biosig_data_type val; 
	char FLAG_CONVERSION_TESTED = 1;
	size_t N; 
#ifdef T1T2
	N = hdr->FLAG.ROW_BASED_CHANNELS ? hdr->data.size[1] : hdr->data.size[0];   
	hdr->NRec = N/hdr->SPR;
#else
	N = hdr->NRec*hdr->SPR;
#endif
	int k2=0;
    	for (k=0; k<hdr->NS; k++)
    	if (hdr->CHANNEL[k].OnOff && hdr->CHANNEL[k].SPR) 
    	{
	if (VERBOSE_LEVEL > 7) fprintf(stdout,"[204] #%i\n",k);
	
		double MaxValue;
		double MinValue;
		if (hdr->FLAG.ROW_BASED_CHANNELS) {
			MaxValue = hdr->data.block[k2];
			MinValue = hdr->data.block[k2];
			for (k1=1; k1<N; k1++) {
				val = hdr->data.block[k2 + k1*hdr->data.size[0]];
				if (MaxValue < val) MaxValue = val;
	 			if (MinValue > val) MinValue = val;
			}
		}
		else {
			MaxValue = hdr->data.block[k2*N];
			MinValue = hdr->data.block[k2*N];
			for (k1=1; k1<N; k1++) {
				val = hdr->data.block[k2*N + k1];
				if (MaxValue < val) MaxValue = val;
	 			if (MinValue > val) MinValue = val;
			}
		}

		if (!hdr->FLAG.UCAL) {
			MaxValue = (MaxValue - hdr->CHANNEL[k].Off)/hdr->CHANNEL[k].Cal;
			MinValue = (MinValue - hdr->CHANNEL[k].Off)/hdr->CHANNEL[k].Cal;
		}
		val = MaxValue * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
		if (PhysMaxValue0 < val) PhysMaxValue0 = val;
		val = MinValue * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
		if (PhysMinValue0 > val) PhysMinValue0 = val;

		if ((SOURCE_TYPE==alpha) && (hdr->CHANNEL[k].GDFTYP==(255+12)) && (TARGET_TYPE==GDF)) 
			// 12 bit into 16 bit 
			; //hdr->CHANNEL[k].GDFTYP = 3;
		else if ((SOURCE_TYPE==ETG4000) && (TARGET_TYPE==GDF)) {
			hdr->CHANNEL[k].GDFTYP  = 16;
			hdr->CHANNEL[k].PhysMax = MaxValue * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
			hdr->CHANNEL[k].PhysMin = MinValue * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
			hdr->CHANNEL[k].DigMax  = MaxValue;
			hdr->CHANNEL[k].DigMin  = MinValue;
		}
		else if ((SOURCE_TYPE==GDF) && (TARGET_TYPE==GDF)) 
			;
		else if ((TARGET_TYPE==SCP_ECG || TARGET_TYPE==EDF ) && !hdr->FLAG.UCAL) {
			double scale = PhysDimScale(hdr->CHANNEL[k].PhysDimCode) *1e9;
			if (hdr->FLAG.ROW_BASED_CHANNELS) {
				for (k1=0; k1<N; k1++)
					hdr->data.block[k2 + k1*hdr->data.size[0]] *= scale;
			}
			else {
				for (k1=0; k1<N; k1++) 
					hdr->data.block[k2*N + k1] *= scale;
			}
	    		hdr->CHANNEL[k].GDFTYP = 3;
	    		hdr->CHANNEL[k].PhysDimCode = 4276;	// nV
	    		hdr->CHANNEL[k].DigMax = 2^15-1;
	    		hdr->CHANNEL[k].DigMin = -hdr->CHANNEL[k].DigMax;
			double PhysMax = max(fabs(PhysMaxValue0),fabs(PhysMinValue0)) * scale;
	    		hdr->CHANNEL[k].PhysMax = PhysMax;
	    		hdr->CHANNEL[k].PhysMin = -PhysMax;
		}
		else if ((hdr->CHANNEL[k].GDFTYP<10 ) && (TARGET_TYPE==GDF || TARGET_TYPE==CFWB)) {
			/* heuristic to determine optimal data type */
			if ((MaxValue <= 127) && (MinValue >= -128))
		    		hdr->CHANNEL[k].GDFTYP = 1;
			else if ((MaxValue <= 255.0) && (MinValue >= 0.0))
			    	hdr->CHANNEL[k].GDFTYP = 2;
			else if ((MaxValue <= ldexp(1.0,15)-1.0) && (MinValue >= ldexp(-1.0,15)))
		    		hdr->CHANNEL[k].GDFTYP = 3;
			else if ((MaxValue <= ldexp(1.0,16)-1.0) && (MinValue >= 0.0))
			    	hdr->CHANNEL[k].GDFTYP = 4;
			else if ((MaxValue <= ldexp(1.0,31)-1.0) && (MinValue >= ldexp(-1.0,31)))
		    		hdr->CHANNEL[k].GDFTYP = 5;
			else if ((MaxValue <= ldexp(1.0,32)-1.0) && (MinValue >= 0.0))
		    		hdr->CHANNEL[k].GDFTYP = 6;
		} 
		else {
			FLAG_CONVERSION_TESTED = 0;
		}   		
		
		if (VERBOSE_LEVEL>8) fprintf(stdout,"#%3d %d [%f %f][%f %f]\n",k,hdr->CHANNEL[k].GDFTYP,MinValue,MaxValue,PhysMinValue0,PhysMaxValue0);
		k2++;
	}
	if (!FLAG_CONVERSION_TESTED) 
		fprintf(stderr,"Warning SAVE2GDF: conversion from %s to %s not tested\n",GetFileTypeString(SOURCE_TYPE),GetFileTypeString(TARGET_TYPE));

	if (VERBOSE_LEVEL>7) fprintf(stdout,"[205] UCAL=%i\n", hdr->FLAG.UCAL);

	/* write file */
	strcpy(tmp,dest);
	if (hdr->FILE.COMPRESSION)  // add .gz extension to filename  
		strcat(tmp,".gz");

	if (VERBOSE_LEVEL>7) 
		fprintf(stdout,"[211] z=%i sd=%i\n",hdr->FILE.COMPRESSION,hdr->FILE.Des);

	hdr->FLAG.ANONYMOUS = 1; 	// no personal names are processed 

	hdr = sopen(tmp, "wb", hdr);
	if ((status=serror())) {
		destructHDR(hdr);
		exit(status); 
	}	
#ifndef WITHOUT_NETWORK
	if (hdr->FILE.Des>0) 
		savelink(source);
#endif 
	if (VERBOSE_LEVEL>7)
		fprintf(stdout,"\n[221] File %s opened. %i %i %Li Des=%i\n",hdr->FileName,hdr->AS.bpb,hdr->NS,hdr->NRec,hdr->FILE.Des);

	swrite(data, hdr->NRec, hdr);

	if (VERBOSE_LEVEL>7) fprintf(stdout,"[231] SWRITE finishes\n");
	if ((status=serror())) { 
		destructHDR(hdr);
		exit(status); 
    	}	

	if (VERBOSE_LEVEL>7) fprintf(stdout,"[236] SCLOSE finished\n");

	sclose(hdr);
	if (VERBOSE_LEVEL>7) fprintf(stdout,"[241] SCLOSE finished\n");
	destructHDR(hdr);
	exit(serror()); 
}

