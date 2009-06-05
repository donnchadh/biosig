/*

    $Id: pdp2gdf.c,v 1.1 2009-03-23 22:01:51 schloegl Exp $
    Copyright (C) 2009 Alois Schloegl <a.schloegl@ieee.org>
    This file is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 

    BioSig is free software; you can redistribute it and/or
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




#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "biosig-network.h"

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


int main (int argc, char *argv[]) {

	int k,s,status; 
    	register int sd;
	struct stat info;
	int state; 
	char *filename = NULL;	
	char *hostname = "129.27.3.99";	//"localhost"
	char *dest = NULL;
	int VERBOSE	= 1; 
	int  numopt = 0;
	
   	for (k=1; k<argc; k++)
    	if (!strcmp(argv[k],"-v") || !strcmp(argv[k],"--version") ) {
		fprintf(stdout,"pdp2gdf (BioSig4C++) v%04.2f\n", BIOSIG_VERSION);
		fprintf(stdout,"Copyright (C) 2009 by Alois Schloegl\n");
		fprintf(stdout,"This file is part of BioSig http://biosig.sf.net - the free and\n");
		fprintf(stdout,"open source software library for biomedical signal processing.\n\n");
		fprintf(stdout,"BioSig is free software; you can redistribute it and/or modify\n");
		fprintf(stdout,"it under the terms of the GNU General Public License as published by\n");
		fprintf(stdout,"the Free Software Foundation; either version 3 of the License, or\n");
		fprintf(stdout,"(at your option) any later version.\n\n");
	}	
    	else if (!strcmp(argv[k],"-h") || !strcmp(argv[k],"--help") ) {
		fprintf(stdout,"\nusage: pdf2gdf <SOURCE>\n");
		fprintf(stdout,"  <SOURCE> is the source file (an unencrypted pdp-file)\n");
		fprintf(stdout,"  The converted data is stored in <SOURCE>.gdf\n");
		fprintf(stdout,"  A link file is also stored in /tmp/<SOURCE>.bscs[.n]\n");
		fprintf(stdout,"  The data can be also loaded from the server into Matlab/Octave with [data,HDR]=mexSLOAD('/tmp/<SOURCE>.bscs[.n]');\n");
		fprintf(stdout,"  or with [data,HDR]=mexSLOAD('bscs://<hostname>/ID');\n");
		fprintf(stdout,"\n  Supported OPTIONS are:\n");
		fprintf(stdout,"   -v, --version\n\tprints version information\n");
		fprintf(stdout,"   -h, --help   \n\tprints this information\n");
		fprintf(stdout,"\n\n");
		return(0);
	}	
    	else if (!strncmp(argv[k],"-VERBOSE",2))  	{
	    	VERBOSE = argv[k][strlen(argv[k])-1]-48;
#ifndef VERBOSE_LEVEL
	// then VERBOSE_LEVEL is not a constant but a variable
	VERBOSE_LEVEL = VERBOSE; 
#endif
	}
	else if (numopt==0) {
		filename = argv[k];
		numopt++;
	}	
	else if (numopt==1) {
		hostname = argv[k];
		numopt++;
	}	
	else if (numopt==2) {
		dest = argv[k];
		numopt++;
	}	
	else 
		fprintf(stdout,"arg%i: <%s>\n",k,argv[k]);
		
	if (filename==NULL) {
		fprintf(stdout,"PDP2GDF send PDP data to a server and retrieves the corresponding data in GDF.\n For more details see pdp2gdf -h\n");
		return(0); 
	}	


#define VERBOSE_LEVEL 8		
	sd = bscs_connect(hostname); 
   	if(sd<0) {
      		perror("cannot connect to server");
      		exit(1);
   	}
	SERVER_STATE = STATE_INIT; 
	uint64_t ID = 0; 

	ID = 0; 
	s = bscs_open(sd, &ID);
	if (s) fprintf(stderr,"bscs open failed %x %lx\n",s,ID); 

	savelink(filename);	// store 
	
	if (VERBOSE_LEVEL>7) fprintf(stdout,"bscs opened \n"); 

//	s = bscs_nop(sd); 
	
	s = bscs_put_file(sd, filename); 
	if (s) fprintf(stderr,"put file failed %x\n",s); 

	if (VERBOSE_LEVEL>7) fprintf(stdout,"put file %lx done \n",ID); 


	
	char *fn = (char*) malloc(strlen(hostname)+25);
#ifdef _WIN32
	sprintf(fn,"bscs://%s/%08lx%08lx",hostname,*((uint32_t*)&ID+1),*((uint32_t*)&ID));
#else
	sprintf(fn,"bscs://%s/%016lx",hostname,ID);
#endif

	if (VERBOSE_LEVEL>7) fprintf(stdout,"get file %s \n",fn); 


	if (!dest) {
		dest = (char*)malloc(strlen(filename)+5);
		strcpy(dest,filename);
		strcat(dest,".gdf");
		bscs_get_file(sd,ID,dest);
		free(dest);
	}
	else 
		bscs_get_file(sd,ID,dest);
	
/*	
	fprintf(stdout,"Data is available from %s\n",fn);
	HDRTYPE *hdr = sopen(fn,"r",NULL);
	if ((status=serror())) {
		destructHDR(hdr);
		fprintf(stderr,"Could not load file from server (%s)\n",fn);
		exit(status); 
	}	
	sread(NULL,0,hdr->NRec,hdr);
	sclose(hdr);	
	

	fn = (char*) realloc(fn,strlen(filename)+5);
	strcpy(fn,filename); 
	strcat(fn,".gdf"); 
	
	hdr->TYPE = GDF; 
	hdr->VERSION = 2.11; 
	hdr->FileName = fn; 
	hdr = sopen(fn, "wb", hdr);
	if ((status=serror())) {
		destructHDR(hdr);
		exit(status); 
	}	
	swrite(hdr->data.block, hdr->NRec, hdr);
	sclose(hdr);
	destructHDR(hdr);
*/

	s = bscs_close(sd);
	if (VERBOSE_LEVEL>7) fprintf(stdout,"bscs close \n"); 
	
	bscs_disconnect(sd); 

}
