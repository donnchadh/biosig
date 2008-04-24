/*

    $Id: save2gdf.c,v 1.40 2008-04-24 08:06:03 schloegl Exp $
    Copyright (C) 2000,2005,2007,2008 Alois Schloegl <a.schloegl@ieee.org>
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "biosig.h"

#ifndef INF
#define INF (1.0/0.0)
#endif 

int main(int argc, char **argv){
    
    HDRTYPE 	*hdr; 
    size_t 	count, k1, ne=0;
    uint16_t 	numopt = 0;
    char 	*source, *dest, tmp[1024]; 
    enum FileFormat SOURCE_TYPE, TARGET_TYPE=GDF; 		// type of file format
    int		COMPRESSION_LEVEL=0;
    int		status, k; 
	
    if (argc<2)
    	;
    else 
    {
    	for (k=1; k<argc && argv[k][0]=='-'; k++)
    	if (!strcmp(argv[k],"-v") || !strcmp(argv[k],"--version") )
    	{
		fprintf(stdout,"save2gdf (BioSig4C++) v0.55\n");
		fprintf(stdout,"Written by Alois Schloegl and others\n\n");
		fprintf(stdout,"This program is free software; you can redistribute it and/or modify\n");
		fprintf(stdout,"it under the terms of the GNU General Public License as published by\n");
		fprintf(stdout,"the Free Software Foundation; either version 3 of the License, or\n");
		fprintf(stdout,"(at your option) any later version.\n");
	}	
    	else if (!strcmp(argv[k],"-h") || !strcmp(argv[k],"--help") )
    	{
		fprintf(stdout,"\nusage: save2gdf [OPTIONS] SOURCE DEST\n");
		fprintf(stdout,"  SOURCE is the source file \n");
		fprintf(stdout,"  DEST is the destination file \n");
		fprintf(stdout,"  Supported OPTIONS are:\n");
		fprintf(stdout,"   -v, --version\n\tprints version information\n");
		fprintf(stdout,"   -h, --help   \n\tprints this information\n");
		fprintf(stdout,"   -f=FMT  \n\tconverts data into format FMT\n");
		fprintf(stdout,"\tFMT must represent a valid target file format\n"); 
		fprintf(stdout,"\tCurrently are supported: HL7aECG, SCP_ECG (EN1064), GDF (v2), GDF1 (v1), EDF, BDF, CFWB\n"); 
		fprintf(stdout,"   -z=#, compression level \n");
		fprintf(stdout,"\t#=0 no compression; #=9 best compression\n");
		fprintf(stdout,"   -VERBOSE=#, verbosity level #\n\t0=silent, 9=debugging");
		fprintf(stdout,"\n\n");
		return(0);
	}	
    	else if (!strncmp(argv[k],"-z",3))  	{
#ifdef ZLIB_H
		COMPRESSION_LEVEL = 1;  
		if (strlen(argv[k])>3) {
	    		COMPRESSION_LEVEL = argv[k][3]-48;
	    		if (COMPRESSION_LEVEL<0 || COMPRESSION_LEVEL>9)
				fprintf(stderr,"Error %s: Invalid Compression Level %s\n",argv[0],argv[k]); 
    		}   
#else
	     	fprintf(stderr,"Warning: option -z (compression) not supported. zlib not linked.\n");
#endif 
	}
    	else if (!strncmp(argv[k],"-VERBOSE",2))  	{
	    	VERBOSE_LEVEL = argv[k][strlen(argv[k])-1]-48;
	}
    	else if (!strncmp(argv[k],"-f=",3))  	{
    	 	if (!strcmp(argv[k],"-f=GDF"))
			TARGET_TYPE=GDF;
    		else if (!strcmp(argv[k],"-f=GDF1"))
			TARGET_TYPE=GDF1;
    		else if (!strcmp(argv[k],"-f=EDF"))
			TARGET_TYPE=EDF;
    		else if (!strcmp(argv[k],"-f=BDF"))
			TARGET_TYPE=BDF;
    		else if (!strncmp(argv[k],"-f=HL7",6) )
			TARGET_TYPE=HL7aECG;
    		else if (!strncmp(argv[k],"-f=SCP",6))
			TARGET_TYPE=SCP_ECG;
    		else if (!strncmp(argv[k],"-f=CFWB",6))
			TARGET_TYPE=CFWB;
    		else if (!strncmp(argv[k],"-f=MFER",6))
			TARGET_TYPE=MFER;
		else {
			fprintf(stderr,"format %s not supported.\n",argv[k]);
			return(-1);
		}	
	}

	numopt = k-1;	
		
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

	if (VERBOSE_LEVEL<0) VERBOSE_LEVEL=1; // default 
	if (VERBOSE_LEVEL>8) fprintf(stdout,"[111] SAVE2GDF started\n");

	tzset();
	hdr = sopen(source, "r", NULL);
	if ((status=serror())) {
		destructHDR(hdr);
		exit(status); 
	} 
	
/* obsolete 
	if (hdr->SPR * hdr->Dur[1] != hdr->SampleRate * hdr->Dur[0])
		fprintf(stdout,"ERROR SAVE2GDF: SPR=%i Fs=%f Dur=%i/%i\n",hdr->SPR, hdr->SampleRate, hdr->Dur[0], hdr->Dur[1]);
*/ 

	if (VERBOSE_LEVEL>8) fprintf(stdout,"[112] SOPEN-R finished\n");

	hdr2ascii(hdr,stdout,VERBOSE_LEVEL);
	
	// all channels are converted - channel selection currently not supported
    	for (k=0; k<hdr->NS; k++) {
    		if (!hdr->CHANNEL[k].OnOff) {
			if ((hdr->SPR/hdr->CHANNEL[k].SPR)*hdr->CHANNEL[k].SPR != hdr->SPR)
				 fprintf(stdout,"Warning: channel %i might be decimated!\n",k+1);
    		};
    		hdr->CHANNEL[k].OnOff = 1;	// convert all channels
    	}	

	hdr->FLAG.OVERFLOWDETECTION = 0;
	hdr->FLAG.UCAL = 1;
	
	if (VERBOSE_LEVEL>8) fprintf(stdout,"[121]\n");
//	count = sread(hdr, 0, hdr->NRec);
	if (dest!=NULL)
		count = sread(NULL, 0, hdr->NRec, hdr);

	biosig_data_type* data = hdr->data.block;
	if (VERBOSE_LEVEL>8) 
		fprintf(stdout,"[122] UCAL=%i %e %e %e \n",hdr->FLAG.UCAL,data[100],data[110],data[500+hdr->SPR]);
	
	if ((status=serror())) {
		destructHDR(hdr);
		exit(status);
	};
	
	if (VERBOSE_LEVEL>8) 
		fprintf(stdout,"\n[129] SREAD on %s successful [%i,%i].\n",hdr->FileName,hdr->data.size[0],hdr->data.size[1]);

//	fprintf(stdout,"\n %f,%f.\n",hdr->FileName,hdr->data.block[3*hdr->SPR],hdr->data.block[4*hdr->SPR]);

	if (dest==NULL) {
		if (ne)	/* used for testig SFLUSH_GDF_EVENT_TABLE */
		{	
			if (hdr->EVENT.N > ne)
				hdr->EVENT.N -= ne;
			else 
				hdr->EVENT.N  = 0;
					
			fprintf(stdout,"Status-SFLUSH %i\n",sflush_gdf_event_table(hdr));
		}	
		
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[131] going for SCLOSE\n");
		sclose(hdr);
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[137] SCLOSE finished\n");
		destructHDR(hdr);
		exit(serror());
	}

	if (hdr->FILE.OPEN){
		sclose(hdr); 
		free(hdr->AS.Header);
		hdr->AS.Header = NULL;
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[138] file closed\n");
	}
	if (VERBOSE_LEVEL>8) 
		fprintf(stdout,"\n[139] File %s closed \n",hdr->FileName);
	
   /********************************* 
   	Write data 
   *********************************/

    	SOURCE_TYPE = hdr->TYPE;
    	hdr->TYPE = TARGET_TYPE;
	hdr->FILE.COMPRESSION=COMPRESSION_LEVEL;
	if (COMPRESSION_LEVEL>0 && TARGET_TYPE==HL7aECG)	{
		fprintf(stderr,"Warning: on-the-fly compression (%i) is not supported for HL7aECG.\n",COMPRESSION_LEVEL); 
		hdr->FILE.COMPRESSION = 0;
	}

	double PhysMaxValue0 = -INF; //hdr->data.block[0];
	double PhysMinValue0 = +INF; //hdr->data.block[0];
	double val; 
	size_t N = hdr->NRec*hdr->SPR;
    	for (k=0; k<hdr->NS; k++) {
		double MaxValue = hdr->data.block[k*N];
		double MinValue = hdr->data.block[k*N];
		
		/* Maximum and Minimum for channel k */ 
		for (k1=1; k1<N; k1++) {
			if (MaxValue < hdr->data.block[k*N+k1])
		 		MaxValue = hdr->data.block[k*N+k1];
	 		if (MinValue > hdr->data.block[k*N+k1])
	 			MinValue = hdr->data.block[k*N+k1];
		}
		if (!hdr->FLAG.UCAL) {
			MaxValue = (MaxValue - hdr->CHANNEL[k].Off)/hdr->CHANNEL[k].Cal;
			MinValue = (MinValue - hdr->CHANNEL[k].Off)/hdr->CHANNEL[k].Cal;
		}
		val = MaxValue * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;		
		if (PhysMaxValue0 < val)
			PhysMaxValue0 = val;
		val = MinValue * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;		
 		if (PhysMinValue0 > val)
 			PhysMinValue0 = val;

		if ((SOURCE_TYPE==ETG4000) && (TARGET_TYPE==GDF)) {
			hdr->CHANNEL[k].GDFTYP  = 16;
			hdr->CHANNEL[k].PhysMax = MaxValue * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
			hdr->CHANNEL[k].PhysMin = MinValue * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
			hdr->CHANNEL[k].DigMax  = MaxValue;
			hdr->CHANNEL[k].DigMin  = MinValue;
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
		
    		// hdr->CHANNEL[k].GDFTYP = 3;
		if (VERBOSE_LEVEL>8) fprintf(stdout,"#%3d %d [%f %f][%f %f]\n",k,hdr->CHANNEL[k].GDFTYP,MinValue,MaxValue,PhysMinValue0,PhysMaxValue0);
	}
	if (0) //(hdr->TYPE==SCP_ECG && !hdr->FLAG.UCAL) 
	    	for (k=0; k<hdr->NS; k++) {
	    		hdr->CHANNEL[k].GDFTYP = 3;
	    		hdr->CHANNEL[k].PhysMax = (PhysMaxValue0 > -PhysMinValue0 ? PhysMaxValue0 : -PhysMinValue0);
	    		hdr->CHANNEL[k].PhysMin = -hdr->CHANNEL[k].PhysMax;
	    		hdr->CHANNEL[k].Cal = ceil(hdr->CHANNEL[k].PhysMax/(ldexp(1.0,15)-1));
	    		hdr->CHANNEL[k].Off = 0.0;
	    		hdr->CHANNEL[k].DigMax = hdr->CHANNEL[k].PhysMax/hdr->CHANNEL[k].Cal;
	    		hdr->CHANNEL[k].DigMin = -hdr->CHANNEL[k].DigMax;
		}

	/* write file */
	strcpy(tmp,dest);
	if (hdr->FILE.COMPRESSION)  // add .gz extension to filename  
		strcat(tmp,".gz");

	if (VERBOSE_LEVEL>8) fprintf(stdout,"[211]\n");

	hdr->FLAG.ANONYMOUS = 1; 	// no personal names are processed 

	hdr = sopen(tmp, "wb", hdr);
	if ((status=serror())) {
		destructHDR(hdr);
		exit(status); 
	}	
	if (VERBOSE_LEVEL>8)
		fprintf(stdout,"\n[221] File %s opened. %i %i \n",hdr->FileName,hdr->AS.bpb,hdr->NS);

	swrite(data, hdr->NRec, hdr);
	if (VERBOSE_LEVEL>8) fprintf(stdout,"[231] SWRITE finishes\n");
	if ((status=serror())) { 
		destructHDR(hdr);
		exit(status); 
    	}	

	sclose(hdr);
	if (VERBOSE_LEVEL>8) fprintf(stdout,"[241] SCLOSE finished\n");
	destructHDR(hdr);
	exit(serror()); 
}
