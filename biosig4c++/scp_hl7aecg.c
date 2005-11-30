


#include <stdio.h>

#include "biosig.h"


/*
	These functions are stubs (placeholder) and need to be defined. 

*/



HDRTYPE* sopen_SCP_read(char* Header1, HDRTYPE* hdr) {	
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: 
		char* Header	// contains the file content
		
	Output: 
		HDRTYPE *hdr	// defines the HDR structure accoring to "biosig.h"
*/	
	
	return(hdr);	
};


HDRTYPE* sopen_SCP_write(HDRTYPE* hdr) {	
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: HDR structure
		
	Output: 
		char* HDR.AS.Header1 	// contains the content which will be written to the file in SOPEN
*/	
	return(hdr);	
	
};

HDRTYPE* sopen_HL7aECG_read(char* Header1, HDRTYPE* hdr) {	
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: 
		char* Header1	// contains the file content
		
	Output: 
		HDRTYPE *hdr	// defines the HDR structure accoring to "biosig.h"
*/	
	
	return(hdr);	
};


HDRTYPE* sopen_HL7aECG_write(HDRTYPE* hdr) {	
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: HDR structure
		
	Output: 
		char* HDR.AS.Header1 	// contains the content which will be written to the file in SOPEN
*/	
	return(hdr);	
	
};

