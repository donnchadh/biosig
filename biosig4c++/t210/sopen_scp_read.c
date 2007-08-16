/*

    $Id: sopen_scp_read.c,v 1.28 2007-08-16 10:33:57 schloegl Exp $
    Copyright (C) 2005,2006,2007 Alois Schloegl <a.schloegl@ieee.org>
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
#include <time.h>
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



HDRTYPE* sopen_SCP_read(HDRTYPE* hdr) {	
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: 
		char* Header	// contains the file content
		
	Output: 
		HDRTYPE *hdr	// defines the HDR structure accoring to "biosig.h"
*/	

	struct pointer_section *section;
	struct DATA_DECODE decode;
	struct DATA_RECORD record;
	struct DATA_INFO textual;
	bool AS_DECODE = 0; 


	uint8_t*	ptr; 	// pointer to memory mapping of the file layout
	uint8_t*	PtrCurSect;	// point to current section 
	uint8_t*	Ptr2datablock=NULL; 	// pointer to data block 
	int		curSect; 	// current section
	uint32_t 	len; 
	uint16_t 	crc; 
	uint32_t	i,k1,k2; 
	size_t		curSectPos;
	size_t 	sectionStart; 
	tm 		t0,t1;
	int 		NSections = 12;
	uint8_t		tag;
	float 		HighPass=0, LowPass=1.0/0.0, Notch=-1; 	// filter settings
	uint16_t	Cal5=0,Cal6=0;


	/* 
	   Try direct conversion SCP->HDR to internal data structure
		+ whole data is loaded once, then no further File I/O is needed. 
		- currently Huffman and Bimodal compression is not supported. 
	*/	

	if (hdr->aECG == NULL) {
		hdr->aECG = (aECG_TYPE*)malloc(sizeof(aECG_TYPE));
		hdr->aECG->diastolicBloodPressure=0.0;				 
		hdr->aECG->systolicBloodPressure=0.0;
		hdr->aECG->MedicationDrugs = "\0";
		hdr->aECG->ReferringPhysician="\0";
		hdr->aECG->LatestConfirmingPhysician="\0";
		hdr->aECG->Diagnosis="\0";
		hdr->aECG->EmergencyLevel=0;
		hdr->ID.Technician = "nobody";
	}
	hdr->aECG->FLAG.HUFFMAN  = 0; 
	hdr->aECG->FLAG.DIFF     = 0; 
	hdr->aECG->FLAG.REF_BEAT = 0; 
	hdr->aECG->FLAG.BIMODAL  = 0;
	
	ptr = hdr->AS.Header; 
	hdr->NRec = 1;
//        hdr->Dur[0]=10; hdr->Dur[1]=1;  // duration = 10 sec

	sectionStart = 6;
	PtrCurSect = ptr+sectionStart;	

	/**** SECTION 0 ****/
	len = l_endian_u32(*(uint32_t*)(PtrCurSect+4)); 
	NSections = (len-16)/10;
	for (int K=0; K<NSections; K++)	{
		curSect 	= l_endian_u16(*(uint16_t*)(ptr+6+16+K*10));
		len 		= l_endian_u32(*(uint32_t*)(ptr+6+16+K*10+2));
		sectionStart 	= l_endian_u32(*(uint32_t*)(ptr+6+16+K*10+6))-1;
		
		 /***** empty section *****/	
	if ((len==0) || (sectionStart==0)) continue;
		
		PtrCurSect = ptr+sectionStart;
		crc 	   = l_endian_u16(*(uint16_t*)(PtrCurSect));
		uint16_t tmpcrc = CRCEvaluate((uint8_t*)(PtrCurSect+2),len-2); 
		if (crc != tmpcrc)
			fprintf(stderr,"Warning SOPEN(SCP-READ): faulty CRC in section %i: crc=%x, %x\n",curSect,crc,tmpcrc);
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
			hdr->Patient.Birthday = 0; 
			while ((*(PtrCurSect+curSectPos)!=255) | (*(uint16_t*)(PtrCurSect+curSectPos+1)!=0)) {
				tag = *(PtrCurSect+curSectPos);
				len = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos+1));
				curSectPos += 3; 
				if (tag==0) {
					hdr->Patient.Name = (char*)(PtrCurSect+curSectPos);
				}
				else if (tag==1) {
//					hdr->Patient.FirstName = (char*)(PtrCurSect+curSectPos);
				}
				else if (tag==2) {
					hdr->Patient.Id = (char*)(PtrCurSect+curSectPos);
					if (!strcmp(hdr->Patient.Id,"UNKNOWN"))
						hdr->Patient.Id = NULL;
				}
				else if (tag==3) {
				}
				else if (tag==4) {
				}
				else if (tag==5) {
					t1.tm_year = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos))-1900;
					t1.tm_mon  = *(PtrCurSect+curSectPos+2)-1;
					t1.tm_mday = *(PtrCurSect+curSectPos+3);
					t1.tm_hour = 12; 
					t1.tm_min  =  0; 
					t1.tm_sec  =  0; 
					t1.tm_isdst= -1; // daylight saving time: unknown
					hdr->Patient.Birthday = tm_time2gdf_time(&t1);
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
					hdr->aECG->diastolicBloodPressure = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos));
				}
				else if (tag==12) {
					hdr->aECG->systolicBloodPressure  = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos));
				}
				else if (tag==13) {
					hdr->aECG->Diagnosis = (char*)(PtrCurSect+curSectPos);
				}
				else if (tag==14) {
					hdr->VERSION = *(PtrCurSect+curSectPos+14)/10.0;	// tag 14, byte 15
					hdr->aECG->Section1.Tag14.INST_NUMBER = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos));
					hdr->aECG->Section1.Tag14.DEPT_NUMBER = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos+2));
					hdr->aECG->Section1.Tag14.DEVICE_ID   = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos+4));
					hdr->aECG->Section1.Tag14.DEVICE_TYPE = *(PtrCurSect+curSectPos+ 6);
					hdr->aECG->Section1.Tag14.MANUF_CODE  = *(PtrCurSect+curSectPos+ 7);	// tag 14, byte 7 (MANUF_CODE has to be 255)
					hdr->aECG->Section1.Tag14.MOD_DESC    = (char*)(PtrCurSect+curSectPos+8); 
					hdr->aECG->Section1.Tag14.VERSION     = *(PtrCurSect+curSectPos+14);
					hdr->aECG->Section1.Tag14.PROT_COMP_LEVEL = *(PtrCurSect+curSectPos+15); 	// tag 14, byte 15 (PROT_COMP_LEVEL has to be 0xA0 => level II)
					hdr->aECG->Section1.Tag14.LANG_SUPP_CODE  = *(PtrCurSect+curSectPos+16);	// tag 14, byte 16 (LANG_SUPP_CODE has to be 0x00 => Ascii only, latin and 1-byte code)
					hdr->aECG->Section1.Tag14.ECG_CAP_DEV     = *(PtrCurSect+curSectPos+17);	// tag 14, byte 17 (ECG_CAP_DEV has to be 0xD0 => Acquire, (No Analysis), Print and Store)
					hdr->aECG->Section1.Tag14.MAINS_FREQ      = *(PtrCurSect+curSectPos+18);	// tag 14, byte 18 (MAINS_FREQ has to be 0: unspecified, 1: 50 Hz, 2: 60Hz)

					hdr->aECG->Section1.Tag14.ANAL_PROG_REV_NUM = (char*)(PtrCurSect+curSectPos+36);
					int tmp = strlen((char*)(PtrCurSect+curSectPos+36));					
					hdr->aECG->Section1.Tag14.SERIAL_NUMBER_ACQ_DEV = (char*)(PtrCurSect+curSectPos+36+tmp+1);
					tmp += strlen((char*)(PtrCurSect+curSectPos+36+tmp+1));					
					hdr->aECG->Section1.Tag14.ACQ_DEV_SYS_SW_ID = (char*)(PtrCurSect+curSectPos+36+tmp+1);
					tmp += strlen((char*)(PtrCurSect+curSectPos+36+tmp+1));					
					hdr->aECG->Section1.Tag14.ACQ_DEV_SCP_SW = (char*)(PtrCurSect+curSectPos+36+tmp+1); 	// tag 14, byte 38 (SCP_IMPL_SW has to be "OpenECG XML-SCP 1.00")
					tmp += strlen((char*)(PtrCurSect+curSectPos+36+tmp+1));
					hdr->aECG->Section1.Tag14.ACQ_DEV_MANUF  = (char*)(PtrCurSect+curSectPos+36+tmp+1);	// tag 14, byte 38 (ACQ_DEV_MANUF has to be "Manufacturer")
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
					hdr->aECG->ReferringPhysician = (char*)(PtrCurSect+curSectPos);
				}
				else if (tag==21) {
					hdr->aECG->MedicationDrugs = (char*)(PtrCurSect+curSectPos);
				}
				else if (tag==22) {
				}
				else if (tag==23) {
				}
				else if (tag==24) {
					hdr->aECG->EmergencyLevel = *(PtrCurSect+curSectPos);
				}
				else if (tag==25) {
					t0.tm_year = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos)-1900);
					t0.tm_mon  = *(PtrCurSect+curSectPos+2)-1;
					t0.tm_mday = *(PtrCurSect+curSectPos+3);
					hdr->T0    = tm_time2gdf_time((&t0));
				}
				else if (tag==26) {
					t0.tm_hour = *(PtrCurSect+curSectPos);
					t0.tm_min  = *(PtrCurSect+curSectPos+1);
					t0.tm_sec  = *(PtrCurSect+curSectPos+2);
					hdr->T0    = tm_time2gdf_time((&t0));
				}
				else if (tag==27) {
					HighPass   = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos))/100.0;
				}
				else if (tag==28) {
					LowPass    = l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos));
				}
				else if (tag==29) {
					uint8_t bitmap = *(PtrCurSect+curSectPos);
					if (bitmap==0)
						Notch = NaN;	// undefined 
					else if ((bitmap & 0x03)==0)
						Notch = -1;	// notch off
					else if (bitmap & 0x01)
						Notch = 60.0; 	// notch 60Hz
					else if (bitmap & 0x02)
						Notch = 50.0; 	// notch 50Hz
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
			hdr->aECG->FLAG.HUFFMAN = 1; 
		}

		/**** SECTION 3 ****/
		else if (curSect==3)  
		{
			hdr->NS = *(PtrCurSect+curSectPos);
			curSectPos++;
			hdr->aECG->FLAG.REF_BEAT = (*(PtrCurSect+curSectPos) & 0x01);
			curSectPos++;
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
				hdr->CHANNEL[i].LeadIdCode =  *(PtrCurSect+curSectPos+8);
				hdr->CHANNEL[i].Label[0]= 0;
				hdr->CHANNEL[i].LowPass = LowPass; 
				hdr->CHANNEL[i].HighPass= HighPass; 
				hdr->CHANNEL[i].Notch 	= Notch; 
				curSectPos += 9;
			}

		}
		/**** SECTION 4 ****/
		else if (curSect==4)  {
		}

		/**** SECTION 5 ****/
		else if (curSect==5)  {
			Cal5 			= l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos));
			double Fs5	 	= 1e6/l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos+2));
			/*
			FLAG5.DIFF 	= *(PtrCurSect+curSectPos+4);		
			*/
		}

		/**** SECTION 6 ****/
		else if (curSect==6)  {
			Cal6 			= l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos));
			hdr->SampleRate 	= 1e6/l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos+2));
			uint16_t 	GDFTYP 	= 5;	// int32: internal raw data type   
			hdr->aECG->FLAG.DIFF 	= *(PtrCurSect+curSectPos+4);		
			hdr->aECG->FLAG.BIMODAL = *(PtrCurSect+curSectPos+5);
			
			len = 0; 
			for (i=0; i < hdr->NS; i++) {
				hdr->CHANNEL[i].SPR 	    = hdr->SPR;
				hdr->CHANNEL[i].PhysDimCode = 4275; // PhysDimCode("uV") physical unit "uV"
				hdr->CHANNEL[i].Cal 	    = Cal6*1e-3;
				hdr->CHANNEL[i].Off         = 0;
				hdr->CHANNEL[i].OnOff       = 1;    // 1: ON 0:OFF
				hdr->CHANNEL[i].Transducer[0] = '\0';
				hdr->CHANNEL[i].GDFTYP      = GDFTYP;  
				len += l_endian_u16(*(uint16_t*)(PtrCurSect+curSectPos+6+i*2));

				// ### these values should represent the true saturation values ###//
				hdr->CHANNEL[i].DigMax      = ldexp(1.0,20)-1;
				hdr->CHANNEL[i].DigMin      = ldexp(-1.0,20);
				hdr->CHANNEL[i].PhysMax     = hdr->CHANNEL[i].DigMax * hdr->CHANNEL[i].Cal;
				hdr->CHANNEL[i].PhysMin     = hdr->CHANNEL[i].DigMin * hdr->CHANNEL[i].Cal;
			}

			Ptr2datablock   = (PtrCurSect+curSectPos + 6 + hdr->NS*2);   // pointer for huffman decoder
			hdr->AS.rawdata = (uint8_t*)malloc(4*hdr->NS*hdr->SPR*hdr->NRec); 

			int32_t* data = (int32_t*)hdr->AS.rawdata;
			size_t ix; 			
			
			if (hdr->aECG->FLAG.HUFFMAN) 
			{
			//	fprintf(stderr,"Warning SCOPEN(SCP-READ): huffman compression not supported.\n");
				AS_DECODE = 1; 
			}
			if (hdr->aECG->FLAG.BIMODAL)
			{
			//	fprintf(stderr,"Warning SCOPEN(SCP-READ): bimodal compression not supported (yet).\n");
				AS_DECODE = 1; 
			}
if (AS_DECODE) continue;
			for (k1 = 0; k1 < hdr->NS; k1++) {
				for (k2 = 0; k2 < hdr->SPR; k2++) {
					ix = (k2 + k1*hdr->SPR);
					data[ix] = l_endian_i16(*(int16_t*)(Ptr2datablock+2*ix));
				}
			}
			
			if (hdr->aECG->FLAG.DIFF==1)
				for (k1 = 0; k1 < hdr->NS; k1++)
				for (ix = k1*hdr->SPR+1; ix < (k1+1)*hdr->SPR; ix++)
					data[ix] += data[ix-1];

			else if (hdr->aECG->FLAG.DIFF==2)
				for (k1 = 0; k1 < hdr->NS; k1++)
				for (ix = k1*hdr->SPR+2; ix < (k1+1)*hdr->SPR; ix++)
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
	if (hdr->SampleRate!=round(hdr->SampleRate))
		fprintf(stderr,"Warning: SCP-OPEN Sampling rate (%f Hz) is not integer.\n",hdr->SampleRate);
	hdr->Dur[1] = (uint32_t)hdr->SampleRate;



    if (hdr->aECG->FLAG.HUFFMAN || hdr->aECG->FLAG.REF_BEAT || hdr->aECG->FLAG.BIMODAL) {


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

	/* Fall back method: 
		+ implements Huffman, reference beat and Bimodal compression. 
		- uses piece-wise file access
		- defines intermediate data structure
	*/	

	fprintf(stdout, "\nUse SCP_DECODE (Huffman=%i RefBeat=%i Bimodal=%i)\n",hdr->aECG->FLAG.HUFFMAN, hdr->aECG->FLAG.REF_BEAT, hdr->aECG->FLAG.BIMODAL);

        if( (section = (pointer_section *)malloc(sizeof(pointer_section)*_NUM_SECTION)) ==NULL)
        {
                fprintf(stderr,"Not enough memory");  // no, exit //
                exit(2);
        }

	// greatest common divisor of scaling from section 5 and 6
	uint16_t g = 1; 	
	if      (Cal5==0 && Cal6 >1) g = Cal6;
	else if (Cal5 >1 && Cal6==0) g = Cal5;
	else if (Cal5 >1 && Cal6 >1) g = gcd(Cal5,Cal6);

	if (scp_decode(hdr, section, decode, record, textual, add_filter)) {
		if (g>1)
			for (i=0; i < hdr->NS * hdr->SPR * hdr->NRec; ++i)
				decode.Reconstructed[i] /= g;
		hdr->AS.rawdata = (uint8_t*)decode.Reconstructed;
	}
	else { 
		B4C_ERRNUM = B4C_CANNOT_OPEN_FILE;
		B4C_ERRMSG = "SCP-DECODE can not read file"; 
		return(hdr);
	}

	for (i=0; i < hdr->NS; i++) {
		hdr->CHANNEL[i].PhysDimCode = 4275; // PhysDimCode("uV") physical unit "uV" 	
		hdr->CHANNEL[i].Cal 	    = g*1e-3;
		hdr->CHANNEL[i].PhysMax     = hdr->CHANNEL[i].DigMax * hdr->CHANNEL[i].Cal;
		hdr->CHANNEL[i].PhysMin     = hdr->CHANNEL[i].DigMin * hdr->CHANNEL[i].Cal;
	}

    }	 // end of fall back method 
		
#ifdef __BIG_ENDIAN
		/* Endian conversion is done in SCP_DECODE and SREAD.
		in order to avoid special case in SREAD, the conversion is reverted here */
		int32_t *ptr32 = (int32_t*)hdr->AS.rawdata;
		for (i=0; i < hdr->SPR*hdr->NS; i++)
			*(ptr32+i) = l_endian_i32(*(ptr32+i));
#endif
	return(hdr);

};

