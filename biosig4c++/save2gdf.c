/*

    $Id: save2gdf.c,v 1.20 2007-08-16 10:35:27 schloegl Exp $
    Copyright (C) 2000,2005,2007 Alois Schloegl <a.schloegl@ieee.org>
    Copyright (C) 2007 Elias Apostolopoulos
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "biosig.h"

int main(int argc, char **argv){
    
    HDRTYPE 	*hdr; 
    CHANNEL_TYPE* 	cp; 
    size_t 	count, k, k1;
    uint16_t 	numopt = 0;
    time_t  	T0;
    char 	*source, *dest, tmp[1024]; 
    enum FileFormat TARGET_TYPE=GDF; 		// type of file format
    int		COMPRESSION_LEVEL=0;
    int		VERBOSE_LEVEL=-1;
    int		status; 
	
    if (argc<2)
    	;
    else 
    {
    	for (k=1; k<argc && argv[k][0]=='-'; k++)
    	if (!strcmp(argv[k],"-v") | !strcmp(argv[k],"--version") )
    	{
		fprintf(stdout,"save2gdf (biosig4c++) 0.45+\n");
		fprintf(stdout,"Written by Alois Schloegl and others\n\n");
		fprintf(stdout,"This is free software.\n");
	}	
    	else if (!strcmp(argv[k],"-h") | !strcmp(argv[k],"--help") )
    	{
		fprintf(stdout,"usage: save2gdf [OPTIONS] SOURCE DEST\n");
		fprintf(stdout,"  SOURCE is the source file \n");
		fprintf(stdout,"  DEST is the destination file \n");
		fprintf(stdout,"  Supported OPTIONS are:\n");
		fprintf(stdout,"   -v, --version\n\tprints version information\n");
		fprintf(stdout,"   -h, --help   \n\tprints this information\n");
		fprintf(stdout,"   -f=FMT  \n\tconverts data into format FMT\n");
		fprintf(stdout,"\tFMT must represent and valid target file format\n"); 
		fprintf(stdout,"\tCurrently are supported: HL7aECG, SCP_ECG(EN1064), GDF, EDF, BDF, CFWB\n"); 
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
		else {
			fprintf(stderr,"format %s not supported.\n",argv[1]);
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
//	if (dest==NULL ) VERBOSE_LEVEL=2; // default 

	if (VERBOSE_LEVEL<0) VERBOSE_LEVEL=1; // default 
	if (VERBOSE_LEVEL>8) fprintf(stdout,"[111] SAVE2GDF started\n");

	hdr = sopen(source, "r", NULL);
	if ((status=serror())) exit(status); 
	
	if (hdr==NULL) exit(-1);
	if (VERBOSE_LEVEL>8) fprintf(stdout,"[112] SOPEN-R finished\n");

	if (VERBOSE_LEVEL) {
	//if (0) {
		if (VERBOSE_LEVEL>2) {
		/* display header information */
		fprintf(stdout,"FileName:\t%s\nType    :\t%i\nVersion:\t%4.2f\nHeadLen:\t%i\n",source,hdr->TYPE,hdr->VERSION,hdr->HeadLen);
		fprintf(stdout,"NS:\t%i\nSPR:\t%i\nNRec:\t%Li\nDuration[s]:\t%u/%u\nFs:\t%f\n",hdr->NS,hdr->SPR,hdr->NRec,hdr->Dur[0],hdr->Dur[1],hdr->SampleRate);
		fprintf(stdout,"Events/Annotations:\t%i\n",hdr->EVENT.N); 

		T0 = gdf_time2t_time(hdr->T0);
		fprintf(stdout,"Date/Time:\t%s\n",asctime(localtime(&T0))); 
		//T0 = gdf_time2t_time(hdr->Patient.Birthday);
		//fprintf(stdout,"Birthday:\t%s\n",asctime(localtime(&T0)));
		}
		
		if (VERBOSE_LEVEL>0) {
		fprintf(stdout,"PID:\t|%s|\nPatient:\n",hdr->AS.PID);
		fprintf(stdout,"\tName:\t%s\n",hdr->Patient.Name); 
		fprintf(stdout,"\tId:\t%s\n\tWeigth:\t%i kg\n\tHeigth:\t%i cm\n\tAge:\t%4.1f y\n",hdr->Patient.Id,hdr->Patient.Weight,hdr->Patient.Height,(hdr->T0 - hdr->Patient.Birthday)/ldexp(365.25,32)); 
		fprintf(stdout,"\tGender:\t"); 
		if (hdr->Patient.Sex==1)
			fprintf(stdout,"male\n"); 
		else if (hdr->Patient.Sex==2)
			fprintf(stdout,"female\n"); 
		else 
			fprintf(stdout,"unknown\n"); 
		T0 = gdf_time2t_time(hdr->Patient.Birthday);
		fprintf(stdout,"\tBirthday        :\t%s",asctime(localtime(&T0))); 
		T0 = gdf_time2t_time(hdr->T0);
		fprintf(stdout,"\tStartOfRecording:\t%s",asctime(localtime(&T0))); 
		}
		
		if (VERBOSE_LEVEL>1) {
		fprintf(stdout,"\nCHAN#  LeadId Label\tCal\tOff\tPhysDim PhysDimCode PhysMax  PhysMin DigMax DigMin GDFTYP (Bytes)");
		for (int k=0; k<hdr->NS; k++) {
			cp = hdr->CHANNEL+k; 
			fprintf(stdout,"\n#%2i: %3i %7s\t%5f %5f %s\t%i\t%5f\t%5f\t%5f\t%5f\t%i(%i bytes)",k,cp->LeadIdCode,cp->Label,cp->Cal,cp->Off,cp->PhysDim,cp->PhysDimCode,cp->PhysMax,cp->PhysMin,cp->DigMax,cp->DigMin,cp->GDFTYP,GDFTYP_BYTE[cp->GDFTYP]);
		}
		}
	}

	hdr->FLAG.OVERFLOWDETECTION = 0;
	hdr->FLAG.UCAL = 1;
	
	if (VERBOSE_LEVEL>8) fprintf(stdout,"[121]\n");
	count = sread(hdr, 0, hdr->NRec);
	if ((status=serror())) exit(status); 

	if (VERBOSE_LEVEL>8) 
		fprintf(stdout,"\n[129] SREAD on %s successful [%i,%i].\n",hdr->FileName,hdr->data.size[0],hdr->data.size[1]);
//	fprintf(stdout,"\n %f,%f.\n",hdr->FileName,hdr->data.block[3*hdr->SPR],hdr->data.block[4*hdr->SPR]);
	if (dest==NULL) {
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[131] going for SCLOSE\n");
		sclose(hdr);
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[137] SCLOSE finished\n");
		free(hdr);
		exit(serror());
	}

	if (hdr->FILE.OPEN){
		FCLOSE(hdr); 
		hdr->FILE.FID = 0;
		free(hdr->AS.Header);
		hdr->AS.Header = NULL;
		if (VERBOSE_LEVEL>8) fprintf(stdout,"[138] file closed\n");
	}
	if (VERBOSE_LEVEL>8) 
		fprintf(stdout,"\n[139] File %s closed \n",hdr->FileName);
	
   /********************************* 
   	Write data 
   *********************************/

    	hdr->TYPE = TARGET_TYPE;
	hdr->FILE.COMPRESSION=COMPRESSION_LEVEL;
	if (COMPRESSION_LEVEL>0 && hdr->TYPE==HL7aECG)	{
		fprintf(stderr,"Warning: on-the-fly compression (%i) is not supported for HL7aECG.\n",COMPRESSION_LEVEL); 
		hdr->FILE.COMPRESSION = 0;
	}

	double PhysMaxValue0 = hdr->data.block[0];
	double PhysMinValue0 = hdr->data.block[0];
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
		val = MaxValue*hdr->CHANNEL[k].Cal+hdr->CHANNEL[k].Off;		
		if (PhysMaxValue0 < val)
			PhysMaxValue0 = val;
		val = MinValue*hdr->CHANNEL[k].Cal+hdr->CHANNEL[k].Off;		
 		if (PhysMinValue0 > MinValue)
 			PhysMinValue0 = MinValue;

		if (hdr->TYPE==GDF || hdr->TYPE==CFWB) {
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
	}
	if (0) //(hdr->TYPE==SCP_ECG && !hdr->FLAG.UCAL) 
	    	for (k=0; k<hdr->NS; k++) {
	    		hdr->CHANNEL[k].GDFTYP = 3;
	    		hdr->CHANNEL[k].PhysMax = max(PhysMaxValue0,-PhysMinValue0);
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
	hdr = sopen(tmp, "wb", hdr);
	if ((status=serror())) exit(status); 
	if (VERBOSE_LEVEL>8)
		fprintf(stdout,"\n[221] File %s opened. %i %i \n",hdr->FileName,hdr->AS.bpb,hdr->NS);

	swrite(hdr->data.block, hdr->NRec, hdr);
	if (VERBOSE_LEVEL>8) fprintf(stdout,"[231] SWRITE finishes\n");
	if (status=serror()) { 
		free(hdr);
		exit(status); 
    	}	
    	sclose(hdr);
	if (VERBOSE_LEVEL>8) fprintf(stdout,"[241] SCLOSE finished\n");
    	free(hdr);
	exit(serror()); 
}
