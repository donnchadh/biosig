/*

    $Id: sopen_scp_write.c,v 1.1 2006-04-28 17:20:58 schloegl Exp $
    Copyright (C) 2005-2006 Alois Schloegl <a.schloegl@ieee.org>
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


#include <string.h>

#include "../biosig.h"
#include "SCP_Formatter.h"



HDRTYPE* sopen_SCP_write(HDRTYPE* hdr) {	
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: 
		char* Header	// contains the file content
		
	Output: 
		HDRTYPE *hdr	// defines the HDR structure accoring to "biosig.h"
*/	

fprintf(stderr,"881\n");
		cSCP_Formatter* SCP_Formatter = NULL;
fprintf(stderr,"882\n");
  		SCP_Formatter = new cSCP_Formatter();
fprintf(stderr,"883\n");
		SCP_Formatter->ResetInfo();
fprintf(stderr,"884\n");
		if (hdr->aECG==NULL) {
			hdr->aECG = (aECG_TYPE*)malloc(sizeof(aECG_TYPE));
			hdr->aECG->diastolicBloodPressure=0.0;				 
			hdr->aECG->systolicBloodPressure=0.0;
			hdr->ID.Technician = "nobody";		 
		} 	

		char OutFile[2048];
		int  errCode; 
		strcpy(OutFile,hdr->FileName);

		errCode = SCP_Formatter->LoadXMLInfo(hdr);
		if (errCode != 0) {
			fprintf(stderr,"ERROR: LoadXMLINFO #%i\n",errCode);
			delete SCP_Formatter;
			return (hdr);
		}
		
		errCode = SCP_Formatter->DoTheSCPFile(OutFile);
		if (errCode != 0) {
			fprintf(stderr,"ERROR: Do the SCP file #%i\n",errCode);
fprintf(stderr,"887a\n");
			delete SCP_Formatter;
fprintf(stderr,"887b\n");
			return (hdr);	// Errors -1, -2, ..., -15. Problems in Outfile = NULL, structures not initialized or error during the creation of the SCP file.
		} 
		else { 
fprintf(stderr,"888\n");
		    	delete SCP_Formatter;
		}    	
fprintf(stderr,"889\n");
	    	return(hdr);    // this is a hack



	return(hdr);
}


