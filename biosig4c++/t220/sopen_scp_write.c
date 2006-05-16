/*

    $Id: sopen_scp_write.c,v 1.4 2006-05-16 17:57:25 schloegl Exp $
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../biosig.h"
#include "SCP_Formatter.h"



int16_t	Label_to_LeadIdCode(char* Label) {
	// FIXME //  define functions for converting label to LeadIdCode 
	int ret_val = 0;	//default: unspecified 
	
	 	
	return(ret_val);
}


HDRTYPE* sopen_SCP_write(HDRTYPE* hdr) {	
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: 
		char* Header	// contains the file content
		
	Output: 
		HDRTYPE *hdr	// defines the HDR structure accoring to "biosig.h"
*/	
	uint8_t*	ptr;
	int		curSect; 
	uint32_t 	len; 
	uint16_t 	crc; 
		

		cSCP_Formatter* SCP_Formatter = NULL;
  		SCP_Formatter = new cSCP_Formatter();
		SCP_Formatter->ResetInfo();
		if (hdr->aECG==NULL) {
			fprintf(stderr,"Warning: No aECG info defined\n");
			hdr->aECG = (aECG_TYPE*)malloc(sizeof(aECG_TYPE));
			hdr->aECG->diastolicBloodPressure=0.0;				 
			hdr->aECG->systolicBloodPressure=0.0;
			hdr->ID.Technician = "nobody";		 
		} 	

		char OutFile[2048];
		int  errCode; 
		strcpy(OutFile,hdr->FileName);
		for (int k=0; k<hdr->NS; k++) {
			hdr->CHANNEL[k].LeadIdCode = Label_to_LeadIdCode(hdr->CHANNEL[k].Label);
		}	

		errCode = SCP_Formatter->LoadXMLInfo(hdr);
		if (errCode != 0) {
			fprintf(stderr,"ERROR: LoadXMLINFO #%i\n",errCode);
			delete SCP_Formatter;
			return (hdr);
		}

		
		ptr = (uint8_t*)hdr->AS.Header1;


	int NSections = 12; 
	// initialize section 0
	uint32_t sectionStart = 6+16+NSections*10; //
	ptr = (uint8_t*)realloc(ptr,sectionStart); 
	memset(ptr,0,sectionStart);
	
	uint32_t curSectLen = 0; // current section length
	int SectionList = [1,3,6,0];
	for (curSect=NSections-1; curSect>=0; curSect--) {

		curSectLen = 0; // current section length
		//ptr = (uint8_t*)realloc(ptr,sectionStart+curSectLen); 

		if (curSect==0)  // SECTION 0 
		{
			hdr->HeadLen = sectionStart; // length of all other blocks together
			ptr = (uint8_t*)realloc(ptr,hdr->HeadLen); // total file length

			curSectLen = 16; // current section length
			sectionStart = 6; 
		
			memcpy(ptr+sectionStart+10,"SCPECG",6); // reserved
			curSectLen += NSections*10;

			
		}
		else if (curSect==1)  // SECTION 1 
		{
			curSectLen = 16; // current section length
			if (!SCP_Formatter->CreateSCPSection1())
				fprintf(stderr,"Error Creating Section 1\n");

			curSectLen = SCP_Formatter->lenSect1;
			ptr = (uint8_t*)realloc(ptr,sectionStart+curSectLen); 

			memcpy(ptr+sectionStart+16,SCP_Formatter->Sect1+16,curSectLen-16);	
		}
		else if (curSect==2)  // SECTION 2 
		{
		}
    		else if (curSect==3)  // SECTION 3 
		{
			curSectLen = 16; // current section length
			if (!SCP_Formatter->CreateSCPSection3())
				fprintf(stderr,"Error Creating Section 1\n");
//				return (-3);
				
			curSectLen = SCP_Formatter->lenSect3;
			ptr = (uint8_t*)realloc(ptr,sectionStart+curSectLen); 
			memcpy(ptr+sectionStart+16,SCP_Formatter->Sect3+16,curSectLen-16);	
			memset(ptr+sectionStart+10,0,6); // reserved
		}
		else if (curSect==4)  // SECTION 4 
		{
		}
		else if (curSect==5)  // SECTION 5 
		{
		}
		else if (curSect==6)  // SECTION 6 
		{
			curSectLen = 16; // current section length
			if (!SCP_Formatter->CreateSCPSection6())
				fprintf(stderr,"Error Creating Section 1\n");
				
			curSectLen = SCP_Formatter->lenSect6;
			ptr = (uint8_t*)realloc(ptr,sectionStart+curSectLen); 
			memcpy(ptr+sectionStart+16,SCP_Formatter->Sect6+16,curSectLen-16);	
			memset(ptr+sectionStart+10,0,6); // reserved
		}
		else if (curSect==7)  // SECTION 7 
		{
		}
		else if (curSect==8)  // SECTION 8 
		{
		}
		else if (curSect==9)  // SECTION 9 
		{
		}
		else if (curSect==10)  // SECTION 10 
		{
		}
		else if (curSect==11)  // SECTION 11
		{
		}
		else {
		}

//fprintf(stdout,"+ %i\t%i\t%i\t%i\n",curSect,curSectLen,sectionStart,curSectLen+sectionStart);		

		// write to pointer field in Section 0 
		*(uint16_t*)(ptr+curSect*10+6+16)   = curSect; // 
		*(uint32_t*)(ptr+curSect*10+6+16+2) = curSectLen; // length
		*(uint32_t*)(ptr+curSect*10+6+16+6) = sectionStart; // Section start

		// write to Section ID Header
		if (curSectLen>0)
		{
			*(int16_t*)(ptr+sectionStart+2) = curSect; // Section ID
			*(uint32_t*)(ptr+sectionStart+4)= curSectLen; // section length->section header
			ptr[sectionStart+8] 		= 13; // Section Version Number 
			ptr[sectionStart+9] 		= 13; // Protocol Version Number
			*(uint16_t*)(ptr+sectionStart)  = SCP_Formatter->CRCEvaluate(ptr+sectionStart+2,curSectLen-2); // compute CRC
		}	
		sectionStart += curSectLen;	// offset for next section
	}
	
	// compute crc and len and write to preamble 
	*(uint32_t*)(ptr+2) = hdr->HeadLen; 
	*(int16_t*)ptr      = SCP_Formatter->CRCEvaluate(ptr+2,sectionStart-2); 
	

/*		
		errCode = SCP_Formatter->DoTheSCPFile(OutFile);
		if (errCode != 0) {
			fprintf(stderr,"ERROR: DoTheSCPFile #%i\n",errCode);
		} 
*/
    	delete SCP_Formatter;


	hdr->AS.Header1 = ptr; 
	return(hdr);
}


