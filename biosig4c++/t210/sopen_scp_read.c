/*

    $Id: sopen_scp_read.c,v 1.5 2006-05-20 21:54:35 schloegl Exp $
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




#include <stdio.h>             // system includes
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>

#include "../biosig.h"

#include "structures.h"
static const U_int_S _NUM_SECTION=12U;	//consider first 11 sections of SCP
static bool add_filter=true;            // additional filtering gives better shape, but use with care
int scp_decode(HDRTYPE*, pointer_section*, DATA_DECODE&, DATA_RECORD&, DATA_INFO&, bool&);
//void remark(char*);
//                                  end specific by E.C.

/*
	These functions are stubs (placeholder) and need to be defined. 

*/


/*
	SCP format internals 
*/
typedef struct {
	struct {	
		uint8_t	HUFFMAN;
		uint8_t	DIFF;
		uint8_t	BIMODAL;
	} FLAG;
        struct {
                char* Tag14;	// only used for SCP2SCP conversion 
        } Section1;
} SCPECG_TYPE;




HDRTYPE* sopen_SCP_read(HDRTYPE* hdr) {	
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
	bool AS_DECODE = 0; 


	uint8_t*	ptr; 	// pointer to memory mapping of the file layout
	uint8_t*	PtrCurSect;	// point to current section 
	uint8_t*	Ptr2datablock=NULL; 	// pointer to data block 
	uint16_t*	ptr16;
	int		curSect; 	// current section
	uint32_t 	len; 
	uint16_t 	crc, crc2; 
	uint32_t	i,k1,k2,k3; 
	size_t 		sectionStart; 
	time_t 		T0;
	tm 		t0;
	int 		_NUM_SECTION = 12;
	uint8_t		tag, VERSION;
	size_t		curSectPos;
	float HighPass, LowPass, Notch; 	// filter settings
	uint8_t		FLAG_HUFFMAN = 0;		
	uint8_t		FLAG_DIFF = 0;		
	SCPECG_TYPE	SCP; 


if (AS_DECODE==0) 
	/* Try direct conversion SCP->HDR to internal data structure
		+ whole data is loaded once, then no further File I/O is needed. 
		- currently Huffman and Bimodal compression is not supported. 
	*/	
{	
	SCP.FLAG.HUFFMAN = 0; 
	SCP.FLAG.DIFF = 0; 
	
	ptr = hdr->AS.Header1; 
	hdr->NRec = 1; 
        hdr->Dur[0]=10; hdr->Dur[1]=1;  // duration = 10 sec

	sectionStart = 6;
	PtrCurSect = ptr+sectionStart;	

	/**** SECTION 0 ****/
	len = *(uint32_t*)(PtrCurSect+4); 
	_NUM_SECTION = (len-16)/10;
	for (int K=0; K<_NUM_SECTION; K++)	{
		curSect 	= l_endian_u16(*(uint16_t*)(ptr+6+16+K*10));
		len 		= l_endian_u32(*(uint32_t*)(ptr+6+16+K*10+2));
		sectionStart 	= l_endian_u32(*(uint32_t*)(ptr+6+16+K*10+6))-1;
		
		 /***** empty section *****/	
		if ((len==0) || (sectionStart==0)) /* empty section */
		{
			continue; 			
		}
		
		PtrCurSect 	= ptr+sectionStart;
		crc 		= l_endian_u16(*(uint16_t*)(PtrCurSect)); 
		if (curSect != l_endian_u16(*(uint16_t*)(PtrCurSect+2)))
			fprintf(stderr,"Warning SOPEN(SCP-READ): Current Section No does not match field in sections (%i %i)\n",curSect,l_endian_u16(*(uint16_t*)(PtrCurSect+2))); 
		if (len != l_endian_u32(*(uint32_t*)(PtrCurSect+4)))
			fprintf(stderr,"Warning SOPEN(SCP-READ): length field in pointer section (%i) does not match length field in sections (%i %i)\n",K,len,l_endian_u32(*(uint32_t*)(PtrCurSect+4))); 

		curSectPos = 16;
			
		/**** SECTION 0 ****/
		if (curSect==0)  
		{
		}		
				
		/**** SECTION 1 ****/
		else if (curSect==1)  
		{
			while ((*(PtrCurSect+curSectPos)!=255) | (*(uint16_t*)(PtrCurSect+curSectPos+1)!=0)) {
				tag = *(PtrCurSect+curSectPos);
				len = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos+1));
				curSectPos += 3; 
				if (tag==0) {
				}
				else if (tag==1) {
				}
				else if (tag==2) {
					hdr->Patient.Id = (char*)(PtrCurSect+curSectPos);
				}
				else if (tag==3) {
				}
				else if (tag==4) {
				}
				else if (tag==5) {
					t0.tm_year = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos));
					t0.tm_mon  = *(PtrCurSect+curSectPos+2);
					t0.tm_mday = *(PtrCurSect+curSectPos+3);
					hdr->Patient.Birthday = tm_time2gdf_time(&t0);
				}
				else if (tag==6) {
					hdr->Patient.Height = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos));
				}
				else if (tag==7) {
					hdr->Patient.Weight = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos));
				}
				else if (tag==8) {
					hdr->Patient.Sex = *(PtrCurSect+curSectPos);
				}
				else if (tag==9) {
				}
				else if (tag==10) {
				}
				else if (tag==11) {
				}
				else if (tag==12) {
				}
				else if (tag==13) {
				}
				else if (tag==14) {
					VERSION = *(PtrCurSect+curSectPos+14);
					hdr->VERSION = VERSION/10.0;
				}
				else if (tag==15) {
				}
				else if (tag==16) {
				}
				else if (tag==17) {
				}
				else if (tag==18) {
				}
				else if (tag==19) {
				}
				else if (tag==20) {
				}
				else if (tag==21) {
				}
				else if (tag==22) {
				}
				else if (tag==23) {
				}
				else if (tag==24) {
				}
				else if (tag==25) {
					t0.tm_year = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos));
					t0.tm_mon  = *(PtrCurSect+curSectPos+2);
					t0.tm_mday = *(PtrCurSect+curSectPos+3);
					hdr->T0 = tm_time2gdf_time(&t0);
				}
				else if (tag==26) {
					t0.tm_hour = *(PtrCurSect+curSectPos);
					t0.tm_min  = *(PtrCurSect+curSectPos+1);
					t0.tm_sec  = *(PtrCurSect+curSectPos+2);
					hdr->T0 = tm_time2gdf_time(&t0);
				}
				else if (tag==27) {
					HighPass = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos))/100.0;
				}
				else if (tag==28) {
					LowPass = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos));
				}
				else if (tag==29) {
					uint8_t bitmap = *(PtrCurSect+curSectPos);
					if (bitmap==0)
						Notch = NaN;	// undefined 
					else if ((bitmap & 0x03)==0)
						Notch = -1;	// notch off
					else if (bitmap & 0x01)
						Notch = 60.0; 	// notch 60Hz
					else if (bitmap & 0x01)
						Notch = 50.0; 	// notch 50Hz
				}
				else if (tag==29) {
				}
				else if (tag==30) {
				}
 				else if (tag==31) {
				}
				else if (tag==32) {
				}
				curSectPos += len; 
			}
		}

		/**** SECTION 2 ****/
		else if (curSect==2)  {
			AS_DECODE = 1; 
			SCP.FLAG.HUFFMAN = 1; 
		}

		/**** SECTION 3 ****/
		else if (curSect==3)  
		{
			hdr->NS = *(PtrCurSect+curSectPos);
			curSectPos += 2;
                        hdr->CHANNEL = (CHANNEL_TYPE *) calloc(hdr->NS, sizeof(CHANNEL_TYPE));
                        memset(hdr->CHANNEL, 0, hdr->NS * sizeof(CHANNEL_TYPE));  // blank area

			uint32_t startindex, endindex; 
			for (i = 0, hdr->SPR=1; i < hdr->NS; i++) {
				startindex = l_endian_u32(*(uint32_t*)(PtrCurSect+curSectPos));
				endindex   = l_endian_u32(*(uint32_t*)(PtrCurSect+curSectPos+4));
				if (startindex != 1)
					fprintf(stderr,"Warning SCOPEN(SCP-READ): starting sample not 1 but %x\n",startindex);
					
				hdr->CHANNEL[i].SPR 	= endindex - startindex + 1;
				hdr->SPR 		= lcm(hdr->SPR,hdr->CHANNEL[i].SPR);
				hdr->CHANNEL[i].LeadIdCode = *(PtrCurSect+curSectPos+8);
				hdr->CHANNEL[i].Label 	= "";   //lead_identification(hdr->CHANNEL[i].LeadIdCode);
				hdr->CHANNEL[i].LowPass = LowPass; 
				hdr->CHANNEL[i].HighPass= HighPass; 
				hdr->CHANNEL[i].Notch 	= Notch; 
				curSectPos += 9;
			}

			/* this is moved to SREAD
                        hdr->data.size[0] = hdr->NS;
                        hdr->data.size[1] = hdr->SPR;
                        hdr->data.block = (biosig_data_type *) calloc(hdr->NS * hdr->SPR, sizeof(biosig_data_type));
			*/
			
		}
		/**** SECTION 4 ****/
		else if (curSect==4)  {
		}

		/**** SECTION 5 ****/
		else if (curSect==5)  {
		}

		/**** SECTION 6 ****/
		else if (curSect==6)  {
			double Cal 		= l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos));
			hdr->SampleRate 	= 1e6/l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos+2));
			uint16_t 	GDFTYP 	= 5;	// int32  
			SCP.FLAG.DIFF 		= *(PtrCurSect+curSectPos+4);		
			SCP.FLAG.BIMODAL 	= *(PtrCurSect+curSectPos+5);
			
			len = 0; 
			for (i=0; i < hdr->NS; i++) {
				hdr->CHANNEL[i].SPR 	    = hdr->SPR;
				hdr->CHANNEL[i].PhysDimCode = 4276; // physical unit "uV"	
				hdr->CHANNEL[i].PhysDim     = "nV"; // physical unit "uV"	
				hdr->CHANNEL[i].Cal 	    = Cal;
				hdr->CHANNEL[i].Off         = 0;    // internal data conversion factor (AVM=1uV)
				hdr->CHANNEL[i].OnOff       = 1;    // 1: ON 0:OFF
				hdr->CHANNEL[i].Transducer  = ""; 
				hdr->CHANNEL[i].GDFTYP      = GDFTYP;  
				len += l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos+6+i*2));
					// ### FIXME ### these must be still defined //
				/*
				hdr->CHANNEL[i].DigMax      = 
				hdr->CHANNEL[i].DigMin      = 
				hdr->CHANNEL[i].PhysMax     = hdr->CHANNEL[i].DigMax * hdr->CHANNEL[i].Cal;
				hdr->CHANNEL[i].PhysMin     = hdr->CHANNEL[i].DigMin * hdr->CHANNEL[i].Cal;
				*/	
			}

			Ptr2datablock   = (PtrCurSect+curSectPos + 6 + hdr->NS*2);   // pointer for huffman decoder
			hdr->AS.rawdata = (uint8_t*)malloc(4*hdr->NS*hdr->SPR*hdr->NRec); 

			int32_t* data = (int32_t*)hdr->AS.rawdata;
			size_t ix; 			
			
			if (SCP.FLAG.HUFFMAN) 
			{
			//	fprintf(stderr,"Warning SCOPEN(SCP-READ): huffman compression not supported.\n");
				AS_DECODE = 1; 
			}
			if (SCP.FLAG.BIMODAL)
			{
			//	fprintf(stderr,"Warning SCOPEN(SCP-READ): bimodal compression not supported (yet).\n");
				AS_DECODE = 1; 
			}
			
			for (k1 = 0; k1 < hdr->NS; k1++) {
				size_t nBytes = *(uint16_t*)(PtrCurSect+curSectPos + 6 + k1*2);
				for (k2 = 0; k2 < hdr->SPR; k2++) {
					ix = (k2 + k1*hdr->SPR);
					data[ix] = l_endian_i16(*(int16_t*)(Ptr2datablock+2*ix));
				}
			}
			
			if (SCP.FLAG.DIFF==1)
				for (k1 = 0; k1 < hdr->NS; k1++)
				for (k2 = 1, ix = k1*hdr->SPR; k2 < hdr->SPR; k2++, ix++)
					data[ix] += data[ix-1];

			else if (SCP.FLAG.DIFF==2)
				for (k1 = 0; k1 < hdr->NS; k1++)
				for (k2 = 2, ix = k1*hdr->SPR; k2 < hdr->SPR; k2++, ix++)
					data[ix] += 2*data[ix-1] - data[ix-2];
			
			curSectPos += 6 + 2*hdr->NS + len;
		}

		/**** SECTION 7 ****/
		else if (curSect==7)  {
		}

		/**** SECTION 8 ****/
		else if (curSect==8)  {
		}

		/**** SECTION 9 ****/
		else if (curSect==9)  {
		}

		/**** SECTION 10 ****/
		else if (curSect==10)  {
		}

		/**** SECTION 11 ****/
		else if (curSect==11)  {
		}
		else {
		}
	}	
	hdr->Dur[0] = hdr->SPR;
	hdr->Dur[1] = hdr->SampleRate;
}

else if (AS_DECODE!=0)	
	/* Fall back method: 
		+ implements Huffman and Bimodal compression. 
		- uses piece-wise file access
		- defines intermediate data structure
	*/	
  	{
	fprintf(stdout, "\nUse SCP_DECODE\n");
	fseek(hdr->FILE.FID, 0, SEEK_SET);


        if( (section = (pointer_section *)malloc(sizeof(pointer_section)*_NUM_SECTION)) ==NULL)
        {
                fprintf(stderr,"Not enough memory");  // no, exit //
                exit(2);
        }

	if (scp_decode(hdr, section, decode, record, textual, add_filter)) {
		if (1) {
                        hdr->VERSION= 0.1*textual.des.analyzing.protocol_revision_number;     // store version as requested (float)
                        hdr->NS = decode.flag_lead.number;
			hdr->SPR= decode.flag_Res.number_samples;        // should be 5000
			hdr->NRec = 1; 
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
			hdr->AS.rawdata = (uint8_t*)decode.Reconstructed; 
			// hdr->AS.bpb, and hdr->AS.spb will be defined at the and of SOPEN
                        hdr->data.size[0] = hdr->NS;
                        hdr->data.size[1] = hdr->SPR;
                        hdr->data.block = (biosig_data_type *) calloc(12 * hdr->SPR, sizeof(biosig_data_type));

			for (int i=0; i<hdr->NS; i++) {
				hdr->CHANNEL[i].SPR 	    = hdr->SPR;
				hdr->CHANNEL[i].PhysDimCode = 4276; // physical unit "uV"	
				hdr->CHANNEL[i].PhysDim     = "nV"; // physical unit "uV"	
				hdr->CHANNEL[i].Cal         = 1;    // internal data conversion factor (AVM=1uV)
				hdr->CHANNEL[i].Off         = 0;    // internal data conversion factor (AVM=1uV)
				hdr->CHANNEL[i].OnOff       = 1;    // 1: ON 0:OFF
				hdr->CHANNEL[i].Transducer  = ""; 
				hdr->CHANNEL[i].GDFTYP      = 5;    // int32
				
				// ### FIXME ### these must be still defined //
				hdr->CHANNEL[i].Label       = "";   //lead_identification[decode.data_lead[i].ID];
				hdr->CHANNEL[i].LeadIdCode  = decode.data_lead[i].ID;
				/*
				hdr->CHANNEL[i].LowPass     = 
				hdr->CHANNEL[i].HighPass    = 
				hdr->CHANNEL[i].Notch       = 
				hdr->CHANNEL[i].DigMax      = 
				hdr->CHANNEL[i].DigMin      = 
				hdr->CHANNEL[i].PhysMax     = hdr->CHANNEL[i].DigMax * hdr->CHANNEL[i].Cal;
				hdr->CHANNEL[i].PhysMin     = hdr->CHANNEL[i].DigMin * hdr->CHANNEL[i].Cal;
				*/
			}	
/*
			for (int j=0;j<hdr->NS;j++)
			for (int i=0;i<hdr->SPR;i++)
				hdr->data.block[i+hdr->SPR*j]=decode.Reconstructed[i+j*decode.flag_Res.number_samples]*hdr->CHANNEL[j].Cal;
*/
		}
		else if ((decode.flag_lead.number==8) && decode.flag_lead.all_simultaneously) {
                        hdr->VERSION= 0.1*textual.des.analyzing.protocol_revision_number;     // store version as requested (float)
                        hdr->NS = 12;   // standard 12 leads only
			hdr->SPR=decode.flag_Res.number_samples;        // should be 5000
			hdr->NRec = 1; 
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
                        hdr->data.size[0] = hdr->NS;
                        hdr->data.size[1] = hdr->SPR;
                        hdr->data.block = (biosig_data_type *) calloc(hdr->NS * hdr->SPR, sizeof(biosig_data_type));

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
				hdr->CHANNEL[i].Cal         = 1E-3; // internal data conversion factor (AVM=1uV)
				hdr->CHANNEL[i].Off         = 0;    // internal data conversion factor (AVM=1uV)
				hdr->CHANNEL[i].OnOff       = 1; //((i<2)|(i>5)); // only first eight channels are ON
				hdr->CHANNEL[i].Transducer  = ""; 
				hdr->CHANNEL[i].GDFTYP      = 5;    // int32
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
}
return(hdr);
}

