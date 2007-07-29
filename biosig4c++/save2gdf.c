/*

    $Id: save2gdf.c,v 1.11 2007-07-29 21:41:47 schloegl Exp $
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
    size_t 	count;
    uint16_t 	numopt = 0;
    time_t  	T0;
    char 	*source, *dest; 
    enum FileFormat TARGET_TYPE=GDF; 		// type of file format
	
    if (argc<2)
    	;
    else if (argv[1][0]=='-')
    {
	numopt++;
    	if (!strcmp(argv[1],"-v") | !strcmp(argv[1],"--version") )
    	{
		fprintf(stdout,"save2gdf (biosig4c++) 0.42+\n");
		fprintf(stdout,"Written by Alois Schloegl and others\n\n");
		fprintf(stdout,"This is free software.\n");
		return(0);
	}	
    	else if (!strcmp(argv[1],"-h") | !strcmp(argv[1],"--help") )
    	{
		fprintf(stdout,"usage: save2gdf SOURCE DEST\n");
		fprintf(stdout,"usage: save2gdf SOURCE \n");
		fprintf(stdout,"    reads file only.\n");
		fprintf(stdout,"usage: save2gdf [OPTION]\n\n");
		fprintf(stdout,"usage: save2gdf [OPTION] SOURCE DEST\n");
		fprintf(stdout,"   -v, --version\n\tprints version information\n");
		fprintf(stdout,"   -h, --help   \n\tprints this information\n");
		fprintf(stdout,"   -f=FMT  \n\tconverts data into format FMT\n");
		fprintf(stdout,"\tFMT must represent and valid target file format\n"); 
		fprintf(stdout,"\tCurrently are supported: HL7aECG, SCP_ECG(EN1064), GDF, EDF, BDF\n"); 
		fprintf(stdout,"\n\n");
		return(0);
	}	
    	else if (!strncmp(argv[1],"-f=",3))
    	{ 	if (!strcmp(argv[1],"-f=GDF"))
			TARGET_TYPE=GDF;
    		else if (!strcmp(argv[1],"-f=EDF"))
			TARGET_TYPE=EDF;
    		else if (!strcmp(argv[1],"-f=BDF"))
			TARGET_TYPE=BDF;
    		else if (!strncmp(argv[1],"-f=HL7",6) )
			TARGET_TYPE=HL7aECG;
    		else if (!strncmp(argv[1],"-f=SCP",6))
			TARGET_TYPE=SCP_ECG;
		else {
			fprintf(stderr,"format %s not supported.\n",argv[1]);
			return(-1);
		}	
	}
		
    }
	dest = NULL;
    	switch (argc - numopt) {
    	case 1:
		fprintf(stderr,"save2gdf: missing file argument\n");
		fprintf(stdout,"usage: save2gdf [options] SOURCE DEST\n");
		return(-1);
    	case 3:
    		dest   = argv[numopt+2]; 
    	case 2:
	    	source = argv[numopt+1]; 
    	}	

    
    hdr = sopen(source, "r", NULL);
    
    if (hdr==NULL) exit(-1);
    fprintf(stderr,"FileName:\t%s\nType    :\t%i\nVersion:\t%4.2f\nHeadLen:\t%i\n",source,hdr->TYPE,hdr->VERSION,hdr->HeadLen);
    fprintf(stderr,"NS:\t%i\nSPR:\t%i\nNRec:\t%Li\nSpB:\t%i\nDuration[s]:\t%u/%u\nFs:\t%f\n",hdr->NS,hdr->SPR,hdr->NRec,hdr->AS.bpb,hdr->Dur[0],hdr->Dur[1],hdr->SampleRate);

    T0 = gdf_time2t_time(hdr->T0);
    fprintf(stderr,"Date/Time:\t%s\n",asctime(localtime(&T0))); 
		//T0 = gdf_time2t_time(hdr->Patient.Birthday);
		//fprintf(stdout,"Birthday:\t%s\n",asctime(localtime(&T0)));

    fprintf(stderr,"Patient:\n\tName:\t%s\n\tId:\t%s\n\tWeigth:\t%i kg\n\tHeigth:\t%i cm\n\tAge:\t%4.1f y\n",hdr->Patient.Name,hdr->Patient.Id,hdr->Patient.Weight,hdr->Patient.Height,(hdr->T0 - hdr->Patient.Birthday)/ldexp(365.25,32)); 
    T0 = gdf_time2t_time(hdr->Patient.Birthday);
    fprintf(stderr,"\tBirthday:\t%s\n",asctime(localtime(&T0))); 
    fprintf(stdout,"EVENT:\n\tN:\t%i\n\tFs:\t%f\n",hdr->EVENT.N,hdr->EVENT.SampleRate); 
		
    for (int k=0; k<hdr->NS; k++) {
	cp = hdr->CHANNEL+k; 
/*	fprintf(stdout,"\n#%2i: %7s\t%s\t%s\t%i\t%5f\t%5f\t%5f\t%5f\t%5f\t",k,cp->Label,cp->Transducer,cp->PhysDim,cp->PhysDimCode,cp->PhysMax,cp->PhysMin,cp->DigMax,cp->DigMin,cp->Cal);
	fprintf(stderr,"\n%4.0f\t%4.0f\t%4.0f\t%5f Ohm",cp->LowPass,cp->HighPass,cp->Notch,cp->Impedance);
*/
	fprintf(stdout,"\n#%2i: %7s\t%5f %5f %s\t%i\t%5f\t%5f\t%5f\t%5f\t",k,cp->Label,cp->Cal,cp->Off,cp->PhysDim,cp->PhysDimCode,cp->PhysMax,cp->PhysMin,cp->DigMax,cp->DigMin);
    }
    fprintf(stdout,"\nm1: %d %d %d %d\n",*((int16_t *)hdr->AS.rawdata),*((int16_t *)hdr->AS.rawdata+2),*(int16_t *)(hdr->AS.rawdata+4),*(int16_t *)(hdr->AS.rawdata+6));

    hdr->FLAG.OVERFLOWDETECTION = 0;
    hdr->FLAG.UCAL = 1;
    
    count = sread(hdr, 0, hdr->NRec);

    fprintf(stdout,"\nFile %s read successfully %i %i\n",hdr->FileName,hdr->EVENT.N,hdr->SPR);

//    fprintf(stdout,"%i\t%i\n", hdr->data.block[0], hdr->data.block[hdr->SPR]);
//    fprintf(stdout,"%i\t%i\n", hdr->data.block[1], hdr->data.block[hdr->SPR+1]);

	if (dest==NULL) {
		sclose(hdr);
		free(hdr);
		return(0);
	}

    if (hdr->FILE.OPEN){
#ifdef ZLIB_H
	gzclose(hdr->FILE.FID);
#else
	fclose(hdr->FILE.FID);
#endif
	hdr->FILE.FID = 0;
	free(hdr->AS.Header1);
	hdr->AS.Header1 = NULL;
    }

    fprintf(stdout,"\nFile %s closed \n",hdr->FileName);

   /********************************* 
   	Write data 
   *********************************/

    	hdr->TYPE = TARGET_TYPE;

	size_t N=hdr->NRec*hdr->SPR;
    	for (int k=0; k<hdr->NS; k++) {
		double PhysMax = hdr->data.block[k*N];
		double PhysMin = hdr->data.block[k*N];
    fprintf(stdout,"\nFile %s closed %i\n",hdr->FileName,k);
		/* Maximum and Minimum for channel k */ 
		for (uint32_t k1=1; k1<N; k1++) {
			if (PhysMax < hdr->data.block[k*N+k1])
			 	PhysMax = hdr->data.block[k*N+k1];
		 	if (PhysMin > hdr->data.block[k*N+k1])
		 		PhysMin = hdr->data.block[k*N+k1];   		
		}

	    	if ((hdr->FLAG.UCAL == 1) && (hdr->TYPE==GDF))
		{
			/* heuristic to determine optimal data type */
			if ((PhysMax <= 127) && (PhysMin >= -128))
		    		hdr->CHANNEL[k].GDFTYP = 1;
			else if ((PhysMax <= 255.0) && (PhysMin >= 0.0))
			    	hdr->CHANNEL[k].GDFTYP = 2;
			else if ((PhysMax <= ldexp(1.0,15)-1.0) && (PhysMin >= ldexp(-1.0,15)))
		    		hdr->CHANNEL[k].GDFTYP = 3;
			else if ((PhysMax <= ldexp(1.0,16)-1.0) && (PhysMin >= 0.0))
			    	hdr->CHANNEL[k].GDFTYP = 4;
			else if ((PhysMax <= ldexp(1.0,31)-1.0) && (PhysMin >= ldexp(-1.0,31)))
		    		hdr->CHANNEL[k].GDFTYP = 5;
			else if ((PhysMax <= ldexp(1.0,32)-1.0) && (PhysMin >= 0.0))
		    		hdr->CHANNEL[k].GDFTYP = 6;
		}    		
	}
	

if (0) { // ### MAYBE OBSOLETE ### 
    	// header conditioning
	double PhysMax = hdr->data.block[0];
	double PhysMin = hdr->data.block[0];
    	for (int k=0; k<hdr->NS; k++) {
	/*    	
		hdr->CHANNEL[k].PhysMax = hdr->CHANNEL[k].DigMax * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off; 
		hdr->CHANNEL[k].PhysMin = hdr->CHANNEL[k].DigMin * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off; 
	*/
		uint32_t N=hdr->NRec*hdr->SPR;
		hdr->CHANNEL[k].PhysMax = hdr->data.block[k*N];
		hdr->CHANNEL[k].PhysMin = hdr->data.block[k*N];
		for (uint32_t k1=1; k1<N; k1++) {
			if (hdr->CHANNEL[k].PhysMax < hdr->data.block[k*N+k1])
			 	hdr->CHANNEL[k].PhysMax = hdr->data.block[k*N+k1];
		 	if (hdr->CHANNEL[k].PhysMin > hdr->data.block[k*N+k1])
		 		hdr->CHANNEL[k].PhysMin = hdr->data.block[k*N+k1];   		
		}

		if (0)
		 	;
		else if (hdr->TYPE==SCP_ECG) {
/*			
fprintf(stdout,"SOPEN-SCP-W2: #%i %e\n",k,hdr->CHANNEL[k].Cal);
			hdr->CHANNEL[k].Cal *= PhysDimScale(hdr->CHANNEL[k].PhysDimCode)*1e9; 
fprintf(stdout,"SOPEN-SCP-W3: #%i %e\n",k,hdr->CHANNEL[k].Cal);
			hdr->CHANNEL[k].PhysDimCode = 4276; //PhysDimCode("nV"); 
fprintf(stdout,"SOPEN-SCP-W4: %i.\n",k);
			if (PhysMax < hdr->CHANNEL[k].PhysMax)
		 		PhysMax = hdr->CHANNEL[k].PhysMax;   		
			if (PhysMin > hdr->CHANNEL[k].PhysMin)
		 		PhysMin = hdr->CHANNEL[k].PhysMin;   		
*/
    		}
		else if (hdr->TYPE==EDF)	{
			hdr->CHANNEL[k].DigMax = ldexp(1.0,15)-1.0; 
			hdr->CHANNEL[k].DigMin = ldexp(-1.0,15); 
    		}
		else if (hdr->TYPE==BDF) {
			hdr->CHANNEL[k].DigMax = ldexp(1.0,20)-1.0; 
			hdr->CHANNEL[k].DigMin = ldexp(-1.0,20); 
    		}
    		else if (hdr->TYPE==GDF) {
    			// ### Phys/Dig/Max/Min depend on GDFTYP
			hdr->CHANNEL[k].GDFTYP = 16;
			hdr->CHANNEL[k].DigMax = hdr->CHANNEL[k].PhysMax; 
			hdr->CHANNEL[k].DigMin = hdr->CHANNEL[k].PhysMin; 
    		}
	}
	
	double Cal1 = PhysMax/(ldexp(1.0,15)-1.0);
	double Cal2 = PhysMin/ldexp(-1.0,15);
	double Cal  = (Cal1 > Cal2 ? Cal1 : Cal2);
    	for (int k=0; k<hdr->NS; k++) {
		if (0)
		; 
		else if (hdr->TYPE==SCP_ECG) {
/*			hdr->CHANNEL[k].Cal = Cal;
			hdr->CHANNEL[k].Off = 0.0;
*/		}
		else {
			hdr->CHANNEL[k].Cal = (hdr->CHANNEL[k].PhysMax-hdr->CHANNEL[k].PhysMin)/(hdr->CHANNEL[k].DigMax-hdr->CHANNEL[k].DigMin);
			hdr->CHANNEL[k].Off =  hdr->CHANNEL[k].PhysMin-hdr->CHANNEL[k].Cal * hdr->CHANNEL[k].DigMin;
		}
	}
}
    fprintf(stdout,"File %s : -sopen-write\n", hdr->FileName);

    hdr = sopen(dest, "w", hdr);
    fprintf(stdout,"File %s : sopen-write\n", hdr->FileName);
    {	
	// write data 
	// write(hdr->AS.rawdata, 4 ,hdr->NRec*hdr->SPR*hdr->NS, hdr->FILE.FID);
	swrite(hdr->data.block, hdr->NRec, hdr);
    }
    sclose(hdr);
    free(hdr);
}
