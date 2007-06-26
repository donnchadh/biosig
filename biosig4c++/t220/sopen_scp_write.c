/*

    $Id: sopen_scp_write.c,v 1.15 2007-06-26 20:46:16 schloegl Exp $
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
    along with this program; if not, write to the Free Software18;
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "../biosig.h"




int16_t	Label_to_LeadIdCode(char* Label) {
	// FIXME //  define functions for converting label to LeadIdCode 
	int ret_val = 0;	//default: unspecified 
	 	
	return(ret_val);
};



HDRTYPE* sopen_SCP_write(HDRTYPE* hdr) {	
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: 
		char* Header	// contains the file content
		
	Output: 
		HDRTYPE *hdr	// defines the HDR structure accoring to "biosig.h"
*/	
	uint8_t*	ptr; 	// pointer to memory mapping of the file layout
	uint8_t*	PtrCurSect;	// point to current section 
	uint16_t*	ptr16;
	int		curSect; 
	uint32_t 	len; 
	uint16_t 	crc, crc2; 
	uint32_t	i; 
	uint32_t 	sectionStart; 
	time_t 		T0;
	tm* 		T0_tm;
	double 		AVM; 
	

	if ((fabs(hdr->VERSION - 1.3)<0.01) && (fabs(hdr->VERSION-2.0)<0.01))  
		fprintf(stderr,"Warning SOPEN (SCP-WRITE): Version %f not supported\n",hdr->VERSION);
		
	uint8_t VERSION = 20; // (uint8_t)round(hdr->VERSION*10); // implemented version number 
	for (int k=0; k<hdr->NS; k++) {
		if (hdr->CHANNEL[k].LeadIdCode==0)
			hdr->CHANNEL[k].LeadIdCode = Label_to_LeadIdCode(hdr->CHANNEL[k].Label);
	}	

	if (hdr->aECG==NULL) {
		fprintf(stderr,"Warning SOPEN_SCP_WRITE: No aECG info defined\n");
		hdr->aECG = (aECG_TYPE*)malloc(sizeof(aECG_TYPE));
		hdr->aECG->diastolicBloodPressure=0.0;				 
		hdr->aECG->systolicBloodPressure=0.0;
		hdr->aECG->MedicationDrugs="/0";
		hdr->aECG->ReferringPhysician="/0";
		hdr->aECG->LatestConfirmingPhysician="/0";
		hdr->aECG->Diagnosis="/0";
		hdr->aECG->EmergencyLevel=0;
		hdr->ID.Technician = "nobody";
	}

//fprintf(stdout,"SCP-Write: IIb %s\n",hdr->aECG->ReferringPhysician);
	/* predefined values */
	hdr->aECG->Section1.Tag14.INST_NUMBER 	= 0;		// tag 14, byte 1-2 
	hdr->aECG->Section1.Tag14.DEPT_NUMBER 	= 0;		// tag 14, byte 3-4 
	hdr->aECG->Section1.Tag14.DEVICE_ID 	= 0;		// tag 14, byte 5-6 
	hdr->aECG->Section1.Tag14.DEVICE_TYPE 	= 0;		// tag 14, byte 7: 0: Cart, 1: System (or Host) 
	hdr->aECG->Section1.Tag14.MANUF_CODE 	= 255;		// tag 14, byte 8 (MANUF_CODE has to be 255)
	hdr->aECG->Section1.Tag14.MOD_DESC  	= "Cart1";	// tag 14, byte 9 (MOD_DESC has to be "Cart1")
	hdr->aECG->Section1.Tag14.VERSION	= VERSION;	// tag 14, byte 15 (VERSION * 10)
	hdr->aECG->Section1.Tag14.PROT_COMP_LEVEL = 0xA0;	// tag 14, byte 16 (PROT_COMP_LEVEL has to be 0xA0 => level II)
	hdr->aECG->Section1.Tag14.LANG_SUPP_CODE  = 0x00;	// tag 14, byte 17 (LANG_SUPP_CODE has to be 0x00 => Ascii only, latin and 1-byte code)
	hdr->aECG->Section1.Tag14.ECG_CAP_DEV 	= 0xD0;		// tag 14, byte 18 (ECG_CAP_DEV has to be 0xD0 => Acquire, (No Analysis), Print and Store)
	hdr->aECG->Section1.Tag14.MAINS_FREQ  	= 0;		// tag 14, byte 19 (MAINS_FREQ has to be 0: unspecified, 1: 50 Hz, 2: 60Hz)
	hdr->aECG->Section1.Tag14.ANAL_PROG_REV_NUM 	= "";
	hdr->aECG->Section1.Tag14.SERIAL_NUMBER_ACQ_DEV = "";
	hdr->aECG->Section1.Tag14.ACQ_DEV_SYS_SW_ID 	= "";
	hdr->aECG->Section1.Tag14.ACQ_DEV_SCP_SW	= "OpenECG XML-SCP 1.00"; // tag 14, byte 38 (SCP_IMPL_SW has to be "OpenECG XML-SCP 1.00")
	hdr->aECG->Section1.Tag14.ACQ_DEV_MANUF 	= "Manufacturer";	// tag 14, byte 38 (ACQ_DEV_MANUF has to be "Manufacturer")

	/*  */
	hdr->aECG->FLAG.HUFFMAN = 0;
	hdr->aECG->FLAG.REF_BEAT= 0;
	hdr->aECG->FLAG.DIFF 	= 0;
	hdr->aECG->FLAG.BIMODAL = 0;


	ptr = (uint8_t*)hdr->AS.Header1;

	int NSections = 12; 
	// initialize section 0
	sectionStart  = 6+16+NSections*10;
	ptr = (uint8_t*)realloc(ptr,sectionStart); 
	memset(ptr,0,sectionStart);
	
	uint32_t curSectLen = 0; // current section length
	for (curSect=NSections-1; curSect>=0; curSect--) {

		curSectLen = 0; // current section length
		//ptr = (uint8_t*)realloc(ptr,sectionStart+curSectLen); 


		if (curSect==0)  // SECTION 0 
		{
			hdr->HeadLen = sectionStart; // length of all other blocks together
			ptr = (uint8_t*)realloc(ptr,hdr->HeadLen); // total file length

			curSectLen = 16; // current section length
			sectionStart = 6; 
		
			memcpy(ptr+16,"SCPECG",6); // reserved
			curSectLen += NSections*10;
		}
		else if (curSect==1)  // SECTION 1 
		{
			ptr = (uint8_t*)realloc(ptr,sectionStart+10000); 
			PtrCurSect = ptr+sectionStart; 
			curSectLen = 16; // current section length
/*
			// Tag 0 (max len = 64)
			*(ptr+sectionStart+curSectLen) = 0;	// tag
			len = strlen(hdr->Patient.Name) + 1;
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = len;	// length
			strncpy((char*)ptr+sectionStart+curSectLen+3,hdr->Patient.Name,len);	// field
			curSectLen += len+3; 
*/			
			// Tag 1 (max len = 64) Firstname 
/*
			*(ptr+sectionStart+curSectLen) = 1;	// tag
			len = strlen(hdr->Patient.Name) + 1;
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(len);	// length
			strncpy(ptr+sectionStart+curSectLen+3,hdr->Patient.Name,len);	// field
			curSectLen += len+3; 
*/
			
			// Tag 2 (max len = 64) Patient ID 
			*(ptr+sectionStart+curSectLen) = 2;	// tag
			if (!strlen(hdr->Patient.Id))	hdr->Patient.Id = "UNKNOWN"; 
			len = strlen(hdr->Patient.Id) + 1;
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(len);	// length
			strncpy((char*)ptr+sectionStart+curSectLen+3,hdr->Patient.Id,len);	// field
			curSectLen += len+3; 

			// Tag 3 (max len = 64) Second Last Name 
/*
			*(ptr+sectionStart+curSectLen) = 3;	// tag
			len = strlen(hdr->Patient.Name) + 1;
			*(uint16_t)(ptr+sectionStart+curSectLen+1) = l_endian_u16(len);	// length
			strncpy(ptr+sectionStart+curSectLen+3,hdr->Patient.Name,len);	// field
			curSectLen += len+3; 
*/
			
			// Tag 5 (len = 4) 
			if (hdr->Patient.Birthday > 0) {
				T0 = gdf_time2t_time(hdr->Patient.Birthday);
				T0_tm = gmtime(&T0);
				*(ptr+sectionStart+curSectLen) = 5;	// tag
				*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(4);	// length
				*(uint16_t*)(ptr+sectionStart+curSectLen+3) = l_endian_u16(T0_tm->tm_year+1900);// year
				*(ptr+sectionStart+curSectLen+5) = (uint8_t)T0_tm->tm_mon + 1;	// month
				*(ptr+sectionStart+curSectLen+6) = (uint8_t)T0_tm->tm_mday; 	// day
				curSectLen += 7;
			} 

			// Tag 6 (len = 3)   Height
			if (hdr->Patient.Height>0.0) {
				*(ptr+sectionStart+curSectLen) = 6;	// tag
				*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(3);	// length
				*(uint16_t*)(ptr+sectionStart+curSectLen+3) = l_endian_u16(hdr->Patient.Height);	// value
				*(ptr+sectionStart+curSectLen+5) = 1;	// cm
				curSectLen += 6;
			}	 

			// Tag 7 (len = 3)	Weight
			if (hdr->Patient.Weight>0.0) {
				*(ptr+sectionStart+curSectLen) = 7;	// tag
				*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(3);	// length
				*(uint16_t*)(ptr+sectionStart+curSectLen+3) = l_endian_u16(hdr->Patient.Weight);	// value
				*(ptr+sectionStart+curSectLen+5) = 1;	// kg
				curSectLen += 6;
			}	 

			// Tag 8 (len = 1)
			if (hdr->Patient.Sex != 0) {
				*(ptr+sectionStart+curSectLen) = 8;	// tag
				*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(1);	// length
				*(ptr+sectionStart+curSectLen+3) = hdr->Patient.Sex;	// value
				curSectLen += 4;
			}	 

			// Tag 11 (len = 2)
			if (hdr->aECG->diastolicBloodPressure>0.0) {
				*(ptr+sectionStart+curSectLen) = 11;	// tag
				*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(2);	// length
				*(uint16_t*)(ptr+sectionStart+curSectLen+3) = l_endian_u16((uint16_t)hdr->aECG->diastolicBloodPressure);	// value
				curSectLen += 5;
			};
			// Tag 12 (len = 2)
			if (hdr->aECG->systolicBloodPressure>0.0) {
				*(ptr+sectionStart+curSectLen) = 12;	// tag
				*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(2);	// length
				*(uint16_t*)(ptr+sectionStart+curSectLen+3) = l_endian_u16((uint16_t)hdr->aECG->systolicBloodPressure);	// value
				curSectLen += 5;
			};	 

			// Tag 13 (max len = 80)
			hdr->aECG->Diagnosis="";
			len = strlen(hdr->aECG->Diagnosis); 
			if (len>0) {
				*(ptr+sectionStart+curSectLen) = 13;	// tag
				len = min(64,len+1);
				*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(len);	// length
				strncpy((char*)(ptr+sectionStart+curSectLen+3),hdr->aECG->Diagnosis,len);
				curSectLen += 3+len;
			};	 

			// Tag 14 (max len = 2 + 2 + 2 + 1 + 1 + 6 + 1 + 1 + 1 + 1 + 1 + 16 + 1 + 25 + 25 + 25 + 25 + 25)
			// Total = 161 (max value)
			*(ptr+sectionStart+curSectLen) = 14;	// tag
			//len = 41; 	// minimum length
			//*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(len);	// length
			memset(ptr+sectionStart+curSectLen+3,0,41);  // dummy value 
			
			curSectLen += 3; 
			*(uint16_t*)(ptr+sectionStart+curSectLen)   = hdr->aECG->Section1.Tag14.INST_NUMBER;
			*(uint16_t*)(ptr+sectionStart+curSectLen+2) = hdr->aECG->Section1.Tag14.DEPT_NUMBER;
			*(uint16_t*)(ptr+sectionStart+curSectLen+4) = hdr->aECG->Section1.Tag14.DEVICE_ID;
			*(ptr+sectionStart+curSectLen+ 6) = hdr->aECG->Section1.Tag14.DEVICE_TYPE;
			*(ptr+sectionStart+curSectLen+ 7) = hdr->aECG->Section1.Tag14.MANUF_CODE;	// tag 14, byte 7 (MANUF_CODE has to be 255)
			strncpy((char*)(ptr+sectionStart+curSectLen+8), hdr->aECG->Section1.Tag14.MOD_DESC, 6);	// tag 14, byte 7 (MOD_DESC has to be "Cart1")
			*(ptr+sectionStart+curSectLen+14) = VERSION;		// tag 14, byte 14 (VERSION has to be 20)
			*(ptr+sectionStart+curSectLen+14) = hdr->aECG->Section1.Tag14.VERSION;
			*(ptr+sectionStart+curSectLen+15) = hdr->aECG->Section1.Tag14.PROT_COMP_LEVEL; 		// tag 14, byte 15 (PROT_COMP_LEVEL has to be 0xA0 => level II)
			*(ptr+sectionStart+curSectLen+16) = hdr->aECG->Section1.Tag14.LANG_SUPP_CODE;		// tag 14, byte 16 (LANG_SUPP_CODE has to be 0x00 => Ascii only, latin and 1-byte code)
			*(ptr+sectionStart+curSectLen+17) = hdr->aECG->Section1.Tag14.ECG_CAP_DEV;		// tag 14, byte 17 (ECG_CAP_DEV has to be 0xD0 => Acquire, (No Analysis), Print and Store)
			*(ptr+sectionStart+curSectLen+18) = hdr->aECG->Section1.Tag14.MAINS_FREQ;		// tag 14, byte 18 (MAINS_FREQ has to be 0: unspecified, 1: 50 Hz, 2: 60Hz)
			*(ptr+sectionStart+curSectLen+35) = strlen(hdr->aECG->Section1.Tag14.ANAL_PROG_REV_NUM)+1;		// tag 14, byte 34 => length of ANAL_PROG_REV_NUM + 1 = 1
			uint16_t len1 = 36;

			char* tmp; 
			tmp = hdr->aECG->Section1.Tag14.ANAL_PROG_REV_NUM;	
			len = min(25, strlen(tmp) + 1);
			strncpy((char*)(ptr+sectionStart+curSectLen+len1), tmp, len);
			len1 += len;

			tmp = hdr->aECG->Section1.Tag14.SERIAL_NUMBER_ACQ_DEV;	
			len = min(25, strlen(tmp) + 1);
			strncpy((char*)(ptr+sectionStart+curSectLen+len1), tmp, len);
			len1 += len;

			tmp = hdr->aECG->Section1.Tag14.ACQ_DEV_SYS_SW_ID;	
			len = min(25, strlen(tmp) + 1);
			strncpy((char*)(ptr+sectionStart+curSectLen+len1), tmp, len);
			len1 += len;

			tmp = hdr->aECG->Section1.Tag14.ACQ_DEV_SCP_SW;	
			len = min(25, strlen(tmp) + 1);
			strncpy((char*)(ptr+sectionStart+curSectLen+len1), tmp, len);
			len1 += len;

			tmp = hdr->aECG->Section1.Tag14.ACQ_DEV_MANUF;	
			len = min(25, strlen(tmp) + 1);
			strncpy((char*)(ptr+sectionStart+curSectLen+len1), tmp, len);
			len1 += len;

			*(uint16_t*)(ptr+sectionStart+curSectLen+1-3) = l_endian_u16(len1);	// length
			curSectLen += len1; 

			// Tag 20 (max len = 64 ) 
			len = strlen(hdr->aECG->ReferringPhysician);
			if (len>0) {
				*(ptr+sectionStart+curSectLen) = 20;	// tag
				len = min(64,len+1);
				*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(len);	// length
				strncpy((char*)(ptr+sectionStart+curSectLen+3),hdr->aECG->ReferringPhysician,len);
				curSectLen += 3+len;
			};	 

			// Tag 21 (max len = 64 )
			len = strlen(hdr->aECG->MedicationDrugs); 
			if (len>0) {
				*(ptr+sectionStart+curSectLen) = 21;	// tag
				len = min(64,len+1);
				*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(len);	// length
				strncpy((char*)(ptr+sectionStart+curSectLen+3),hdr->aECG->MedicationDrugs,len);
				curSectLen += 3+len;
			};	 

			// Tag 24 ( len = 1 ) 
			*(ptr+sectionStart+curSectLen) = 24;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(1);	// length
			*(ptr+sectionStart+curSectLen+3) = hdr->aECG->EmergencyLevel;
			curSectLen += 4;

			// Tag 25 (len = 4)
			T0 = gdf_time2t_time(hdr->T0);
			T0_tm = localtime(&T0);

			*(ptr+sectionStart+curSectLen) = 25;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(4);	// length
			*(uint16_t*)(ptr+sectionStart+curSectLen+3) = l_endian_u16((uint16_t)(T0_tm->tm_year+1900));// year
			*(ptr+sectionStart+curSectLen+5) = (uint8_t)(T0_tm->tm_mon + 1);	// month
			*(ptr+sectionStart+curSectLen+6) = (uint8_t)T0_tm->tm_mday; 	// day
			curSectLen += 7; 

			// Tag 26 (len = 3)
			*(ptr+sectionStart+curSectLen) = 26;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = 3;	// length
			*(ptr+sectionStart+curSectLen+3) = (uint8_t)T0_tm->tm_hour;	// hour
			*(ptr+sectionStart+curSectLen+4) = (uint8_t)T0_tm->tm_min;	// minute
			*(ptr+sectionStart+curSectLen+5) = (uint8_t)T0_tm->tm_sec; 	// second
			curSectLen += 6; 

			if (hdr->NS>0)  {
			// Tag 27 (len = 3) highpass filter 
			*(ptr+sectionStart+curSectLen) = 27;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = 2;	// length
			*(uint16_t*)(ptr+sectionStart+curSectLen+3) = (uint16_t)hdr->CHANNEL[1].HighPass;	// hour
			curSectLen += 5; 

			// Tag 28 (len = 3)  lowpass filter
			*(ptr+sectionStart+curSectLen) = 28;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = 2;	// length
			*(uint16_t*)(ptr+sectionStart+curSectLen+3) = (uint16_t)hdr->CHANNEL[1].LowPass;	// hour
			curSectLen += 5; 

			// Tag 29 (len = 1) filter bitmap
			uint8_t bitmap = 0; 
        		if (fabs(hdr->CHANNEL[1].LowPass-60.0)<0.01) 
				bitmap = 1; 
        		else if (fabs(hdr->CHANNEL[1].LowPass-50.0)<0.01) 
				bitmap = 2;
			else 
				bitmap = 0; 		 
			*(ptr+sectionStart+curSectLen) = 29;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = 1;	// length			
			*(ptr+sectionStart+curSectLen+3) = bitmap; 
			curSectLen += 4; 

			}

#ifdef woF 
// Tag 31 (max len = 12)
	tg.id = 31;
	tg.len = strlen(S1I->szSeqNum) + 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], S1I->szSeqNum, tg.len);
	len1 += tg.len;

// Tag 34 (max len = 29)
	tg.id = 34;
	tg.len = 4 + strlen(S1I->szDateTimeZoneDesc) + 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->wDateTimeZoneOffset), 2);
	len1 += 2;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->wDateTimeZoneIndex), 2);
	len1 += 2;
	memcpy(&Sect1[len1], S1I->szDateTimeZoneDesc, strlen(S1I->szDateTimeZoneDesc) + 1);
	len1 += (strlen(S1I->szDateTimeZoneDesc) + 1);
#endif
			// Tag 255 (len = 0)
			*(ptr+sectionStart+curSectLen) = 255;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(0);	// length
			curSectLen += 3; 

			// Evaluate the size and correct it if odd
			if ((curSectLen % 2) != 0) {
				*(ptr+sectionStart+curSectLen++) = 0; 
			}

//			if (!SCP_Formatter->CreateSCPSection1())
//				fprintf(stderr,"Error Creating Section 1\n");

//			curSectLen = SCP_Formatter->lenSect1;
//			ptr = (uint8_t*)realloc(ptr,sectionStart+curSectLen); 

//			memcpy(ptr+sectionStart+16,SCP_Formatter->Sect1+16,curSectLen-16);	
		}
		else if (curSect==2)  // SECTION 2 
		{
		}
    		else if (curSect==3)  // SECTION 3 
		{
			ptr = (uint8_t*)realloc(ptr,sectionStart+16+2+9*hdr->NS+1); 
			PtrCurSect = ptr+sectionStart; 
			curSectLen = 16; // current section length

			// Number of leads enclosed
			*(ptr+sectionStart+curSectLen++) = hdr->NS;

// Situations with reference beat subtraction are not supported
// Situations with not all the leads simultaneously recorded are not supported
// Situations number of leads simultaneouly recorded != total number of leads are not supported
// We assume hat all the leads are recorded simultaneously
			*(ptr+sectionStart+curSectLen++) = hdr->NS<<3 | 0x04;

			for (i = 0; i < hdr->NS; i++) {
				*(uint32_t*)(ptr+sectionStart+curSectLen) = l_endian_u32(1L);
				*(uint32_t*)(ptr+sectionStart+curSectLen+4) = l_endian_u32(hdr->data.size[0]);
				*(ptr+sectionStart+curSectLen+8) = (uint8_t)hdr->CHANNEL[i].LeadIdCode;
				curSectLen += 9;
			}

			// Evaluate the size and correct it if odd
			if ((curSectLen % 2) != 0) {
				*(ptr+sectionStart+curSectLen++) = 0; 
			}
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
			ptr = (uint8_t*)realloc(ptr,sectionStart+16+6+2*hdr->NS+2*(hdr->data.size[0]*hdr->data.size[1])); 
			PtrCurSect = ptr+sectionStart; 
			curSectLen = 16; // current section length

			// Create all the fields
			// Amplitude Value Multiplier (AMV)
			// check for physical dimension and adjust scaling factor to "nV"
			AVM = hdr->CHANNEL[0].Cal*1e9*PhysDimScale(hdr->CHANNEL[0].PhysDimCode);
			for (i = 1; i < hdr->NS; i++) {
				double avm; 
				// check whether all channels have the same scaling factor
				avm = hdr->CHANNEL[i].Cal*1e9*PhysDimScale(hdr->CHANNEL[i].PhysDimCode);
				if (abs((AVM - avm)/AVM)>1e-14)
					fprintf(stderr,"Warning SOPEN (SCP-WRITE): scaling factors differ between channels. Scaling factor of 1st channel is used %e.\n",avm2);
			};	
			*(uint16_t*)(ptr+sectionStart+curSectLen) = l_endian_u16((uint16_t)AVM);
			curSectLen += 2;

			// Sample interval
			*(uint16_t*)(ptr+sectionStart+curSectLen) = l_endian_u16((uint16_t)(1e6/hdr->SampleRate));
			curSectLen += 2;

			// Diff used
			*(ptr+sectionStart+curSectLen++) = 0;

			// Bimodal/Non-bimodal
			*(ptr+sectionStart+curSectLen++) = 0;


			/* DATA COMPRESSION
			    currently, no compression method is supported. In case of data compression, the
			    data compression can happen here. 
			    

			*/
			
			// Fill the length block
			//	numByteCompRhythm = ESI->dwEndSampleR * 2;
			// Each sample is stored on 2 bytes for each of the 15 leads (we assume to have max 15 leads)
			for (i = 0; i < hdr->NS; i++) {
				*(uint16_t*)(ptr+sectionStart+curSectLen) = l_endian_u16((uint16_t)hdr->data.size[0]*2);
				curSectLen += 2;
			}

			// Fill the data block with the ECG samples
			ptr16 = (uint16_t*)(ptr+sectionStart+curSectLen);
			for (i = 0; i < (hdr->data.size[0]*hdr->data.size[1]); i++) {
				*(ptr16+i) = l_endian_u16((int16_t)(hdr->data.block[i]));
				curSectLen += 2;
				/* ##FIXME## this is a hack 
					it would be best if this could be done within functions SWRITE (not defined yet)
				*/ 
			}

			// Evaluate the size and correct it if odd
			if ((curSectLen % 2) != 0) {
				fprintf(stderr,"Warning Section 6 has an odd length\n"); 
				*(ptr+sectionStart+curSectLen++) = 0; 
			}

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
		*(uint16_t*)(ptr+curSect*10+6+16)   = l_endian_u16(curSect); // 
		*(uint32_t*)(ptr+curSect*10+6+16+2) = l_endian_u32(curSectLen); // length
		// Section start - must be odd. See EN1064:2005(E) Section 5.2.1 

		// write to Section ID Header
		if (curSectLen>0)
		{
			// Section 0: startpos in pointer field 
			*(uint32_t*)(ptr+curSect*10+6+16+6) = l_endian_u32(sectionStart+1); 

			// Section ID header (16 bytes)
			*(int16_t*)(ptr+sectionStart+2) = l_endian_u16(curSect); 	// Section ID
			*(uint32_t*)(ptr+sectionStart+4)= l_endian_u32(curSectLen); 	// section length->section header
			ptr[sectionStart+8] 		= VERSION; 	// Section Version Number 
			ptr[sectionStart+9] 		= VERSION; 	// Protocol Version Number
			crc = CRCEvaluate(ptr+sectionStart+2,curSectLen-2); // compute CRC
//			crc2 = crc_ccitt(ptr+sectionStart+2,curSectLen-2); // compute CRC
			*(uint16_t*)(ptr+sectionStart)  = l_endian_u16(crc);
//			fprintf(stdout,"crc = %i \n",crc);
		}	
		sectionStart += curSectLen;	// offset for next section
	}
	
	// compute crc and len and write to preamble 
	*(uint32_t*)(ptr+2) = l_endian_u32(hdr->HeadLen); 
	crc = CRCEvaluate(ptr+2,hdr->HeadLen-2); 
	*(int16_t*)ptr      = l_endian_u16(crc);
	
	hdr->AS.Header1 = ptr; 
	return(hdr);
}


