


#include <stdio.h>             // system includes
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>

#include "../biosig.h"

#include "types.h"               // start specific by E.C. (SCP reader)
#include "structures.h"
static const U_int_S _NUM_SECTION=12U;		//consider first 11 sections of SCP
static bool add_filter=true;            // additional filtering gives better shape, but use with care
int scp_decode(HDRTYPE*, pointer_section*, DATA_DECODE&, DATA_RECORD&, DATA_INFO&, bool&);
//void remark(char*);
//                                  end specific by E.C.

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
/*
---------------------------------------------------------------------------
Copyright (C) 2006  Eugenio Cervesato.
Developed at the Associazione per la Ricerca in Cardiologia - Pordenone - Italy,

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
---------------------------------------------------------------------------
*/
	struct pointer_section *section;
	struct DATA_DECODE decode;
	struct DATA_RECORD record;
	struct DATA_INFO textual;

	if( (section = (pointer_section *)malloc(sizeof(pointer_section)*_NUM_SECTION)) ==NULL)
	{
		fprintf(stderr,"Not enough memory");  // no, exit //
		exit(2);
	}
	if (scp_decode(hdr, section, decode, record, textual, add_filter)) {
		if ((decode.flag_lead.number==8) && decode.flag_lead.all_simultaneously) {
                        hdr->VERSION= 0.1*textual.des.analyzing.protocol_revision_number;     // store version as requested (float)
                        hdr->NS = 12;   // standard 12 leads only
			hdr->SPR=decode.flag_Res.number_samples;        // should be 5000
                        hdr->Dur[0]=10; hdr->Dur[1]=1;  // duration = 10 sec
			if (decode.flag_BdR0.STM==0) hdr->SampleRate= hdr->SPR/10;  // should be 500
			else  hdr->SampleRate=1000000.0/decode.flag_BdR0.STM;
                        // acquisition time is the sum of the date and the time, converted to gdf_time
                        hdr->T0=t_time2gdf_time(textual.dev.date_acquisition2+textual.dev.time_acquisition2);
                        char buff[300];
                        strcpy(buff, textual.ana.first_name);
                        strcat(buff, ", ");
                        strcat(buff, textual.ana.last_name);
                        hdr->Patient.Name = buff;              // name is 'first name, last name'
                        hdr->Patient.Id = textual.ana.ID;
                        hdr->Patient.Birthday = t_time2gdf_time(textual.ana.date_birth2);

                        hdr->CHANNEL = (CHANNEL_TYPE *) calloc(hdr->NS, sizeof(CHANNEL_TYPE));
                        memset(hdr->CHANNEL, 0, hdr->NS * sizeof(CHANNEL_TYPE));  // blank area
                        hdr->data.size[0] = 12;
                        hdr->data.size[1] = hdr->SPR;
                        hdr->data.block = (biosig_data_type *) calloc(12 * hdr->SPR, sizeof(biosig_data_type));

                        hdr->CHANNEL[0].Label="I";      // first lead
                        hdr->CHANNEL[1].Label="II";
                        hdr->CHANNEL[2].Label="III";
                        hdr->CHANNEL[3].Label="aVr";
                        hdr->CHANNEL[4].Label="aVl";
                        hdr->CHANNEL[5].Label="aVf";
                        hdr->CHANNEL[6].Label="V1";
                        hdr->CHANNEL[7].Label="V2";
                        hdr->CHANNEL[8].Label="V3";
                        hdr->CHANNEL[9].Label="V4";
                        hdr->CHANNEL[10].Label="V5";
                        hdr->CHANNEL[11].Label="V6";    //  last lead
			for (int i=0;i<hdr->NS;i++) {
				hdr->CHANNEL[i].SPR = hdr->SPR;
				hdr->CHANNEL[i].PhysDimCode = 4275; // physical unit "uV"	
				hdr->CHANNEL[i].PhysDim     = "uV"; // physical unit "uV"	
				hdr->CHANNEL[i].Cal         = 1000; // internal data conversion factor (AVM=1uV)
				hdr->CHANNEL[i].Off         = 0;    // internal data conversion factor (AVM=1uV)
				hdr->CHANNEL[i].OnOff       = ((i<2)|(i>5)); // only first eight channels are ON
			}	
			for (int i=0;i<hdr->SPR;i++)
			{                             // data will be stored by row
				hdr->data.block[i+hdr->SPR*0]=decode.Reconstructed[i];         // data returned is in microVolt
				hdr->data.block[i+hdr->SPR*1]=decode.Reconstructed[i+decode.flag_Res.number_samples];
				hdr->data.block[i+hdr->SPR*6]=decode.Reconstructed[i+2*decode.flag_Res.number_samples];
				hdr->data.block[i+hdr->SPR*7]=decode.Reconstructed[i+3*decode.flag_Res.number_samples];
				hdr->data.block[i+hdr->SPR*8]=decode.Reconstructed[i+4*decode.flag_Res.number_samples];
				hdr->data.block[i+hdr->SPR*9]=decode.Reconstructed[i+5*decode.flag_Res.number_samples];
				hdr->data.block[i+hdr->SPR*10]=decode.Reconstructed[i+6*decode.flag_Res.number_samples];
				hdr->data.block[i+hdr->SPR*11]=decode.Reconstructed[i+7*decode.flag_Res.number_samples];

                                hdr->data.block[i+hdr->SPR*2]=hdr->data.block[i+hdr->SPR*1]-hdr->data.block[i+hdr->SPR*0];
		                hdr->data.block[i+hdr->SPR*3]=-(hdr->data.block[i+hdr->SPR*1]+hdr->data.block[i+hdr->SPR*0])/2;
		                hdr->data.block[i+hdr->SPR*4]=hdr->data.block[i+hdr->SPR*0]-hdr->data.block[i+hdr->SPR*1]/2;
		                hdr->data.block[i+hdr->SPR*5]=hdr->data.block[i+hdr->SPR*1]-hdr->data.block[i+hdr->SPR*0]/2;
			}

		} else
		{
			fprintf(stderr,"SCP_ECG: Sorry, at this time can only read 8 standard leads, recorded simultaneously!");
                        exit(2);
		}
        }
	return(hdr);
}

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

