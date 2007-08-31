/*

    $Id: gztest.c,v 1.2 2007-08-31 13:18:15 schloegl Exp $
    Copyright (C) 2007 Alois Schloegl <a.schloegl@ieee.org>
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


#include "../biosig.h"
#include <string.h>

int main(int argc, char **argv){
    

    HDRTYPE 	HDR, *hdr; 
    CHANNEL_TYPE* 	cp; 
    size_t 	count;
    uint16_t 	numopt = 0, k;
    time_t  	T0;
    char 	*source, *dest; 
    enum FileFormat TARGET_TYPE=GDF; 		// type of file format
    int		COMPRESSION_LEVEL=0;
    uint8_t     mem[100000];
    char tmp[100], p[10]; 

	source=argv[1];

	hdr = &HDR; 
	hdr->TYPE = unknown; 
	hdr->FILE.COMPRESSION = 0; 
	hdr->FileName = source; 
	hdr = FOPEN(hdr,"rb");
	count = FREAD(mem,1,100000,hdr);
	hdr->AS.Header1 = mem;
	hdr = getfiletype(hdr);  
	FCLOSE(hdr); 
fprintf(stdout,"1: %s %i %i %i %i %i %i %i %i \n",hdr->FileName, count,hdr->TYPE,unknown,GDF,GZIP,SCP_ECG,HL7aECG,BDF);
	
	hdr->FILE.COMPRESSION = 0; 
	hdr->FileName = source; 
	hdr = FOPEN(hdr,"rb");
	count = FREAD(mem,1,100000,hdr);
	hdr->AS.Header1 = mem;
	hdr = getfiletype(hdr);  
	FCLOSE(hdr); 
fprintf(stdout,"2: %s %i %i %i %i %i %i %i %i \n",hdr->FileName, count,hdr->TYPE,unknown,GDF,GZIP,SCP_ECG,HL7aECG,BDF);
/*	
	hdr->FILE.COMPRESSION = 0; 
	hdr->FileName = "out0.fil"; 
	hdr = FOPEN(hdr,"wb");
	count = FWRITE(mem,1,count,hdr);
	FCLOSE(hdr); 
		
	hdr->FILE.COMPRESSION = 0; 
	hdr->FileName = "out00.fil"; 
	hdr = FOPEN(hdr,"wb");
	count = FWRITE(mem,1,count,hdr);
	FCLOSE(hdr); 
		
	for (k=0;k<10;k++) {
		hdr->FILE.COMPRESSION = 1; 
		strcpy(tmp,"out10.fil"); 
		*(tmp+4)=k+48;  
		strncpy(p,"wb",5); 
		p[strlen(p)]=k+48;  
		hdr->FileName = tmp;
		hdr = FOPEN(hdr,p);
		count = FWRITE(mem,1,count,hdr);
		FCLOSE(hdr);
	}	 
*/
}
