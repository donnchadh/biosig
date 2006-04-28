/*
	These functions are stubs (placeholder) and need to be defined. 

*/

/*

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

#include "biosig.h"

/*
HDRTYPE* sopen_SCP_read(char* Header1, HDRTYPE* hdr) {	
*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: 
		char* Header	// contains the file content
		
	Output: 
		HDRTYPE *hdr	// defines the HDR structure accoring to "biosig.h"
				// hdr->aECG contains the fields of the annotated ECG
	
	hdr->aECG = (aECG_TYPE*) malloc(sizeof(aECG_TYPE));
*	
	
	return(hdr);	
};
*/

/*
HDRTYPE* sopen_SCP_write(HDRTYPE* hdr) {	
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: HDR structure
		hdr->aECG contains the fields of the annotated ECG
		
	Output: 
		char* HDR.AS.Header1 	// contains the content which will be written to the file in SOPEN
	return(hdr);	
	
};
*/



HDRTYPE* sopen_HL7aECG_read(char* Header1, HDRTYPE* hdr) {	
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: 
		char* Header1	// contains the file content
		
	Output: 
		HDRTYPE *hdr	// defines the HDR structure accoring to "biosig.h"
				// if successful, hdr->aECG contains the fields of the annotated ECG
				//		and hdr->TYPE = HL7aECG
				// else hdr->aECG is NULL and hdr->TYPE stays XML
*/	

    xmlTextReaderPtr reader;
    int ret;

    reader = xmlNewTextReaderFilename(hdr->FileName);
    if (reader != NULL) {
        ret = xmlTextReaderRead(reader);
        while (ret == 1) {
//            processNode(reader);
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0) {
            printf("%s : failed to parse\n", filename);
        }
    } else {
        printf("Unable to open %s\n", filename);
    }
}


	if (1) //successful 
		hdr->TYPE = HL7aECG;	
	return(hdr);	
};


HDRTYPE* sopen_HL7aECG_write(HDRTYPE* hdr) {	
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: HDR structure
		hdr->aECG contains the fields of the annotated ECG
		
	Output: 
		char* HDR.AS.Header1 	// contains the content which will be written to the file in SOPEN
*/	
	return(hdr);	
	
};

