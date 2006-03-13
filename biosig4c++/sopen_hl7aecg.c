/*

    $Id: sopen_hl7aecg.c,v 1.1 2006-03-13 11:17:34 schloegl Exp $
    Copyright (C) 2006 Alois Schloegl <a.schloegl@ieee.org>
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




#include <stdio.h>             // system includes
#include <libxml/tree.h>
#include <libxml/xmlreader.h>

#include "biosig.h"


HDRTYPE* sopen_HL7aECG_read(HDRTYPE* hdr) {	
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: 
		char* Header1	// contains the file content
		
	Output: 
		HDRTYPE *hdr	// defines the HDR structure accoring to "biosig.h"
*/	

	xmlDocPtr	XMLDOC;
	xmlTextReaderPtr reader;
	int	ret;
	int	k,k1;


	hdr->NS = 8; 
	hdr->SampleRate = 500; 
	hdr->NRec = 1; 
	hdr->SPR  = 5000; 
	hdr->Dur[0] = 10; 
	hdr->Dur[1] = 1; 
      	hdr->T0 = t_time2gdf_time(time(NULL));
	hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	for (k=0;k<hdr->NS;k++)	{
	      	hdr->CHANNEL[k].Label     = "C4";
	      	hdr->CHANNEL[k].Transducer= "EEG: Ag-AgCl electrodes";
	      	hdr->CHANNEL[k].PhysDim   = "uV";
	      	hdr->CHANNEL[k].PhysDimCode = 19+4256; // uV
	      	hdr->CHANNEL[k].PhysMax   = +100;
	      	hdr->CHANNEL[k].PhysMin   = -100;
	      	hdr->CHANNEL[k].DigMax    = +2047;
	      	hdr->CHANNEL[k].DigMin    = -2048;
	      	hdr->CHANNEL[k].GDFTYP    = 3;	// int16 	
	      	hdr->CHANNEL[k].SPR       = 1;	// one sample per block
	      	hdr->CHANNEL[k].HighPass  = 0.16;
	      	hdr->CHANNEL[k].LowPass   = 70.0;
	      	hdr->CHANNEL[k].Notch     = 50;
	      	hdr->CHANNEL[k].Impedance = INF;
	      	for (k1=0; k1<3; hdr->CHANNEL[k].XYZ[k1++] = 0.0);
	}      	
	


	LIBXML_TEST_VERSION;


	// Parser API
	XMLDOC = xmlParseFile(hdr->FileName);


	// Reader API 
		reader = xmlReaderForFile(hdr->FileName, NULL, XML_PARSE_DTDATTR | XML_PARSE_NOENT); 
		// XML_PARSE_DTDVALID cannot be used in aECG 

		if (reader != NULL) {
		        ret = xmlTextReaderRead(reader);
        		while (ret == 1) {
//            			processNode(reader);
            			ret = xmlTextReaderRead(reader);
        		}
        		if (ret != 0) {
            			fprintf(stderr, "%s : failed to parse\n", hdr->FileName);
        		}	
        	/*
		 * Once the document has been fully parsed check the validation results
		 */
			if (xmlTextReaderIsValid(reader) != 1) {
	    			fprintf(stderr, "Document %s does not validate\n", hdr->FileName);
			}
		        xmlFreeTextReader(reader);
			hdr->TYPE = XML; 
		}        
	


    		/*
    		 * Cleanup function for the XML library.
    		 */
		xmlCleanupParser();
    		/*
    		 * this is to debug memory for regression tests
    		 */
    		xmlMemoryDump();
    		

	fprintf(stdout,"XML: sopen_HL7aECG_read\n");
	
	
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

