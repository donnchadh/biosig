/*

    $Id: sopen_scp_write.c,v 1.7 2006-05-18 15:39:07 schloegl Exp $
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
#ifdef woF
#include "SCP_Formatter.h"
#endif


int16_t	Label_to_LeadIdCode(char* Label) {
	// FIXME //  define functions for converting label to LeadIdCode 
	int ret_val = 0;	//default: unspecified 
	 	
	return(ret_val);
};

/*
uint16_t crc_ccitt(uint8_t* ptr,uint32_t LEN)

for (int k=0;k<LEN;k++)


end;
*/

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
	uint16_t*	ptr16;
	int		curSect; 
	uint32_t 	len; 
	uint16_t 	crc, crc2; 
	uint32_t	i; 

	if (hdr->aECG==NULL) {
		fprintf(stderr,"Warning: No aECG info defined\n");
		hdr->aECG = (aECG_TYPE*)malloc(sizeof(aECG_TYPE));
		hdr->aECG->diastolicBloodPressure=0.0;				 
		hdr->aECG->systolicBloodPressure=0.0;
		hdr->aECG->MedicationDrugs="";
		hdr->aECG->MedicationDrugs="";
		hdr->aECG->MedicationDrugs="";
		hdr->ID.Technician = "nobody";		 
	} 	

	char OutFile[2048];
	int  errCode; 
	strcpy(OutFile,hdr->FileName);
	
	uint8_t VERSION = round(hdr->VERSION*10); 	// implemented version number 
	for (int k=0; k<hdr->NS; k++) {
		if (hdr->CHANNEL[k].LeadIdCode==0)
			hdr->CHANNEL[k].LeadIdCode = Label_to_LeadIdCode(hdr->CHANNEL[k].Label);
	}	

#ifdef woF 
	cSCP_Formatter* SCP_Formatter = NULL;
	SCP_Formatter = new cSCP_Formatter();
	SCP_Formatter->ResetInfo();
	errCode = SCP_Formatter->LoadXMLInfo(hdr);
	if (errCode != 0) {
		fprintf(stderr,"ERROR: LoadXMLINFO #%i\n",errCode);
		delete SCP_Formatter;
		return (hdr);
	}
		
#endif
	ptr = (uint8_t*)hdr->AS.Header1;

	int NSections = 12; 
	// initialize section 0
	uint32_t sectionStart = 6+16+NSections*10;
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
		
			memcpy(ptr+sectionStart+10,"SCPECG",6); // reserved
			curSectLen += NSections*10;
		}
		else if (curSect==1)  // SECTION 1 
		{
			ptr = (uint8_t*)realloc(ptr,sectionStart+10000); 
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
			time_t T0 = gdf_time2t_time(hdr->Patient.Birthday);
			tm* T0_tm = gmtime(&T0);

			*(ptr+sectionStart+curSectLen) = 5;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(4);	// length
			*(uint16_t*)(ptr+sectionStart+curSectLen+3) = l_endian_u16(T0_tm->tm_year+1900);// Year
			*(ptr+sectionStart+curSectLen+5) = T0_tm->tm_mon + 1;	// Year
			*(ptr+sectionStart+curSectLen+6) = T0_tm->tm_mday; 	// Year
			curSectLen += 7; 

			// Tag 6 (len = 3)   Height
			*(ptr+sectionStart+curSectLen) = 6;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(3);	// length
			*(uint16_t*)(ptr+sectionStart+curSectLen+3) = l_endian_u16(hdr->Patient.Height);	// value
			*(ptr+sectionStart+curSectLen+5) = 1;	// cm
			curSectLen += len+3; 

			// Tag 7 (len = 3)	Weight
			*(ptr+sectionStart+curSectLen) = 7;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(3);	// length
			*(uint16_t*)(ptr+sectionStart+curSectLen+3) = l_endian_u16(hdr->Patient.Weight);	// value
			*(ptr+sectionStart+curSectLen+5) = 1;	// kg
			curSectLen += len+3; 

			// Tag 8 (len = 1)
			*(ptr+sectionStart+curSectLen) = 8;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(1);	// length
			*(ptr+sectionStart+curSectLen+3) = hdr->Patient.Sex;	// value
			curSectLen += len+4; 

			// Tag 11 (len = 2)
			*(ptr+sectionStart+curSectLen) = 11;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(2);	// length
			*(uint16_t*)(ptr+sectionStart+curSectLen+3) = l_endian_u16((uint16_t)hdr->aECG->diastolicBloodPressure);	// value
			curSectLen += len+5; 

			// Tag 12 (len = 2)
			*(ptr+sectionStart+curSectLen) = 12;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = 2;	// length
			*(uint16_t*)(ptr+sectionStart+curSectLen+3) = (uint16_t)hdr->aECG->systolicBloodPressure;	// value
			curSectLen += len+5; 

#ifdef woF 
// Tag 14 (max len = 2 + 2 + 2 + 1 + 1 + 6 + 1 + 1 + 1 + 1 + 1 + 16 + 1 + 25 + 25 + 25 + 25 + 25)
// Total = 161 (max value)
			*(ptr+sectionStart+curSectLen) = 14;	// tag

	tg.id = 14;
	tg.len = 0;				// Temporary
	memcpy(bBufTmp, (int8_t*) &tg, 3);
	lenTmp = 0;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->wInstNum), 2);
	lenTmp += 2;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->wDeptNum), 2);
	lenTmp += 2;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->wDevID), 2);
	lenTmp += 2;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->bDevType), 1);
	lenTmp += 1;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->bManCode), 1);
	lenTmp += 1;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->szModDesc), 6);
	lenTmp += 6;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->bSCPECGProtRevNum), 1);
	lenTmp += 1;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->bSCPECGProtCompLev), 1);
	lenTmp += 1;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->bLangSuppCode), 1);
	lenTmp += 1;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->bCapECGDev), 1);
	lenTmp += 1;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->bMainsFreq), 1);
	lenTmp += 1;
	// Reserved area (16 bytes)
	memset(&bBufTmp[3 + lenTmp], '\0', 16);
	lenTmp += 16;
	bUnit = (uint8_t) (strlen(S1I->szAnalProgRevNum) + 1);
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&bUnit), 1);
	lenTmp += 1;
	// (Max len = 25)
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->szAnalProgRevNum), ((0x00FF) & bUnit));
	lenTmp += ((0x00FF) & bUnit);
	// (Max len = 25)
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->szSerNumAcqDev), strlen(S1I->szSerNumAcqDev) + 1);
	lenTmp += (strlen(S1I->szSerNumAcqDev) + 1);
	// (Max len = 25)
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->szAcqDevSystSW), strlen(S1I->szAcqDevSystSW) + 1);
	lenTmp += (strlen(S1I->szAcqDevSystSW) + 1);
	// (Max len = 25)
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->szSCPImplSW), strlen(S1I->szSCPImplSW) + 1);
	lenTmp += (strlen(S1I->szSCPImplSW) + 1);
	// (Max len = 25)
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->szAcqDevManuf), strlen(S1I->szAcqDevManuf) + 1);
	lenTmp += (strlen(S1I->szAcqDevManuf) + 1);

	memcpy(&bBufTmp[1], (int8_t*)(&lenTmp), 2);

	memcpy(&Sect1[len1], (int8_t*)(&bBufTmp[0]), lenTmp + 3);
	len1 += (lenTmp + 3);

// Tag 20 (max len = 64)
	tg.id = 20;
	tg.len = strlen(S1I->szRefPhys) + 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], S1I->szRefPhys, tg.len);
	len1 += tg.len;

// Tag 21 (max len = 64)
	tg.id = 21;
	tg.len = strlen(S1I->szLCPhys) + 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], S1I->szLCPhys, tg.len);
	len1 += tg.len;

// Tag 24 (len = 1)
	tg.id = 24;
	tg.len = 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->bStatCode), 1);
	len1 += 1;
#endif
			// Tag 25 (len = 4)
			T0 = gdf_time2t_time(hdr->T0);
			T0_tm = gmtime(&T0);

			*(ptr+sectionStart+curSectLen) = 25;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = l_endian_u16(4);	// length
			*(uint16_t*)(ptr+sectionStart+curSectLen+3) = l_endian_u16(T0_tm->tm_year+1900);// Year
			*(ptr+sectionStart+curSectLen+5) = T0_tm->tm_mon + 1;	// Month
			*(ptr+sectionStart+curSectLen+6) = T0_tm->tm_mday; 	// Day
			curSectLen += 7; 

			// Tag 26 (len = 3)
			*(ptr+sectionStart+curSectLen) = 26;	// tag
			*(uint16_t*)(ptr+sectionStart+curSectLen+1) = 3;	// length
			*(ptr+sectionStart+curSectLen+3) = T0_tm->tm_hour;	// hour
			*(ptr+sectionStart+curSectLen+4) = T0_tm->tm_min;	// minute
			*(ptr+sectionStart+curSectLen+5) = T0_tm->tm_sec; 	// second
			curSectLen += 6; 
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
			curSectLen += len+3; 

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
			curSectLen = 16; // current section length

			// Create all the fields
			// AVM
			*(uint16_t*)(ptr+sectionStart+curSectLen) = l_endian_u16((uint16_t)hdr->CHANNEL[0].Cal);
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
				*(ptr16+i) = l_endian_u16((uint16_t)(hdr->data.block[i]));
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
		*(uint32_t*)(ptr+curSect*10+6+16+6) = l_endian_u32(sectionStart+1); 

		// write to Section ID Header
		if (curSectLen>0)
		{
			*(int16_t*)(ptr+sectionStart+2) = l_endian_u16(curSect); 	// Section ID
			*(uint32_t*)(ptr+sectionStart+4)= l_endian_u32(curSectLen); 	// section length->section header
			ptr[sectionStart+8] 		= VERSION; 	// Section Version Number 
			ptr[sectionStart+9] 		= VERSION; 	// Protocol Version Number
			crc = CRCEvaluate(ptr+sectionStart+2,curSectLen-2); // compute CRC
//			crc2 = crc_ccitt(ptr+sectionStart+2,curSectLen-2); // compute CRC
			*(uint16_t*)(ptr+sectionStart)  = l_endian_u16(crc);
			fprintf(stdout,"crc = %i \n",crc);
		}	
		sectionStart += curSectLen;	// offset for next section
	}
	
	// compute crc and len and write to preamble 
	*(uint32_t*)(ptr+2) = l_endian_u32(hdr->HeadLen); 
	crc = CRCEvaluate(ptr+2,sectionStart-2); 
	*(int16_t*)ptr      = l_endian_u16(crc);
	

#ifdef woF		
    	delete SCP_Formatter;
#endif


	hdr->AS.Header1 = ptr; 
	return(hdr);
}


