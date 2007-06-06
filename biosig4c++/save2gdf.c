/*

    $Id: save2gdf.c,v 1.3 2007-06-06 16:13:38 schloegl Exp $
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
    time_t  	T0; 
    char 	*source, *dest; 
    enum FileFormat TARGET_TYPE=GDF; 		// type of file format
	
    if (argc < 2)
    {
	fprintf(stderr,"save2gdf: missing file argument\n");
	fprintf(stdout,"usage: save2gdf SOURCE DEST\n");
	return(-1);
    } 
    else if (argv[1][0]=='-')
    {
    	if (!strcmp(argv[1],"-v") | !strcmp(argv[1],"--version") )
    	{
		fprintf(stdout,"save2gdf (biosig4c++) 0.40+\n");
		fprintf(stdout,"Written by Alois Schloegl and others\n\n");
		fprintf(stdout,"This is free software.\n");
		return(0);
	}	
    	else if (!strcmp(argv[1],"-h") | !strcmp(argv[1],"--help") )
    	{
		fprintf(stdout,"usage: save2gdf SOURCE DEST\n");
		fprintf(stdout,"usage: save2gdf [OPTION]\n\n");
		fprintf(stdout,"usage: save2gdf [OPTION] SOURCE DEST\n");
		fprintf(stdout,"   -v, --version\n\tprints version information\n");
		fprintf(stdout,"   -h, --help   \n\tprints this information\n");
		fprintf(stdout,"   -f=FMT  \n\tconverts data into format FMT\n");
		fprintf(stdout,"\tFMT must represent and valid target file format\n"); 
		fprintf(stdout,"\tCurrently are supported: HL7aECG, SCP_ECG(EN1064), GDF, EDF\n"); 
		fprintf(stdout,"\n\n");
		return(0);
	}	
    	else if (!strcmp(argv[1],"-f=GDF"))
		TARGET_TYPE=GDF;
    	else if (!strcmp(argv[1],"-f=EDF"))
		TARGET_TYPE=EDF;
    	else if (!strncmp(argv[1],"-f=HL7",6) )
		TARGET_TYPE=HL7aECG;
    	else if (!strncmp(argv[1],"-f=SCP",6))
		TARGET_TYPE=SCP_ECG;
		
    	if (argc==2) return(0);
    	source = argv[2]; 
    	dest   = argv[3]; 
    }
    else 
    {
	source = argv[1]; 
    	dest   = argv[2]; 
    }	    
    
    hdr = sopen(source, "r", NULL);
    
    if (hdr==NULL)exit(-1);

    fprintf(stderr,"FileName:\t%s\nType    :\t%i\nVersion:\t%4.2f\nHeadLen:\t%i\n",source,hdr->TYPE,hdr->VERSION,hdr->HeadLen);
    fprintf(stderr,"NS:\t%i\nSPR:\t%i\nNRec:\t%Li\nDuration[s]:\t%u/%u\nFs:\t%f\n",hdr->NS,hdr->SPR,hdr->NRec,hdr->Dur[0],hdr->Dur[1],hdr->SampleRate);

    T0 = gdf_time2t_time(hdr->T0);
    fprintf(stderr,"Date/Time:\t%s\n",asctime(localtime(&T0))); 
		//T0 = gdf_time2t_time(hdr->Patient.Birthday);
		//fprintf(stdout,"Birthday:\t%s\n",asctime(localtime(&T0)));

    fprintf(stderr,"Patient:\n\tName:\t%s\n\tId:\t%s\n\tWeigth:\t%i kg\n\tHeigth:\t%i cm\n\tAge:\t%4.1f y\n",hdr->Patient.Name,hdr->Patient.Id,hdr->Patient.Weight,hdr->Patient.Height,(hdr->T0 - hdr->Patient.Birthday)/ldexp(365.25,32)); 
    T0 = gdf_time2t_time(hdr->Patient.Birthday);
    fprintf(stderr,"\tBirthday:\t%s\n",asctime(localtime(&T0))); 
/*  fprintf(stdout,"EVENT:\n\tN:\t%i\n\tFs:\t%f\n\t\n",hdr->EVENT.N,hdr->EVENT.SampleRate); 
		
    fprintf(stdout,"--%i\t%i\n", hdr->FLAG.OVERFLOWDETECTION, hdr->FLAG.UCAL);
    hdr->FLAG.OVERFLOWDETECTION = 0; 
	//	hdr->FLAG.UCAL = 1;
    fprintf(stdout,"--%i\t%i\n", hdr->FLAG.OVERFLOWDETECTION, hdr->FLAG.UCAL);
    fprintf(stdout,"2-%u\t%i\t%i\t%i\t%u\t%u\n",hdr->AS.bpb,hdr->FILE.OPEN,(int32_t)hdr->NRec,hdr->HeadLen,hdr->Dur[0],hdr->Dur[1]);
*/
    for (int k=0; k<hdr->NS; k++) {
	cp = hdr->CHANNEL+k; 
	fprintf(stdout,"\n#%2i: %7s\t%s\t%s\t%i\t%5f\t%5f\t%5f\t%5f\t%5f\t",k,cp->Label,cp->Transducer,cp->PhysDim,cp->PhysDimCode,cp->PhysMax,cp->PhysMin,cp->DigMax,cp->DigMin,cp->Cal);
	fprintf(stderr,"\n%4.0f\t%4.0f\t%4.0f\t%5f Ohm",cp->LowPass,cp->HighPass,cp->Notch,cp->Impedance);
    }
    fprintf(stderr,"\nm1: %d %d %d %d\n",*((int16_t *)hdr->AS.rawdata),*((int16_t *)hdr->AS.rawdata+2),*(int16_t *)(hdr->AS.rawdata+4),*(int16_t *)(hdr->AS.rawdata+6));

    fprintf(stderr,"\nFile %s read successfully %i\n",hdr->FileName,ftell(hdr->FILE.FID));
    hdr->FLAG.OVERFLOWDETECTION = 0;
    hdr->FLAG.UCAL = 1;
    
    count = sread(hdr, 0, hdr->NRec);

    if (hdr->FILE.OPEN){
	fclose(hdr->FILE.FID);
	hdr->FILE.FID = 0;
    }

    hdr->TYPE = TARGET_TYPE;
    hdr = sopen(dest, "w", hdr);
    fprintf(stderr,"File %s : sopen-write\n", hdr->FileName);
    if ( (hdr->TYPE != SCP_ECG) & (hdr->TYPE != HL7aECG) ) /* SCP_ECG and HL7aECG write data during SOPEN */
    {	
#if __BYTE_ORDER == __BIG_ENDIAN
	// fix endianity of the data
	for (k1=0;k1<hdr->NRec*hdr->SPR*hdr->NS;k1++) 	{
		//   hdr->data.block[k1] = l_endian_f64(hdr->data.block[k1]);
		*(int32_t*)(hdr->AS.rawdata+k1*4) = l_endian_i32(*(int32_t*)(hdr->AS.rawdata+k1*4));
	}
#endif 
	// write data 
	fwrite(hdr->AS.rawdata, 4 ,hdr->NRec*hdr->SPR*hdr->NS, hdr->FILE.FID);
    }
    sclose(hdr);
    free(hdr);
}
