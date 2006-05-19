/*

    $Id: biosig.c,v 1.49 2006-05-19 23:16:35 schloegl Exp $
    Copyright (C) 2005,2006 Alois Schloegl <a.schloegl@ieee.org>
		    
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

/* 

	reading and writing of GDF files is demonstrated 
	
	Features: 
	- reading and writing of EDF, BDF and GDF2.0pre files 
	- reading of GDF1.x, BKR, CFWB, CNT files 
	
	implemented functions: 
	- SOPEN, SREAD, SWRITE, SCLOSE, SEOF, SSEEK, STELL, SREWIND 
	
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//#include <libxml/xmlreader.h>


#include "biosig.h"
//#include "zlib.h"


const int16_t GDFTYP_BYTE[] = {
	1, 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 0, 0, 0, 0, 0,   /* 0  */ 
	4, 8,16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 16 */ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 32 */ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 48 */ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 64 */ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 128 */ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2,    /* 256 */  
	0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4,    /* 255+24 = bit24, 3 byte */ 
	0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 6, 
	0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 8, 
	0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0,10, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 384 */ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2,    /* 512 */ 
	0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4, 
	0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 6, 
	0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 8, 
	0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0,10  };


/****************************************************************************/
/**                                                                        **/
/**                      INTERNAL FUNCTIONS                                **/
/**                                                                        **/
/****************************************************************************/
 
// greatest common divisor 
size_t gcd(size_t A,size_t B) 
{	size_t t; 
	while (!B) {
		t = B; 
		B = A%B;
		A = t; 
	}
	return(A);
};

// least common multiple
size_t lcm(size_t A,size_t B) 
{
	return(A*B/gcd(A,B));
};

#if __BYTE_ORDER == __BIG_ENDIAN
float l_endian_f32(float x) 
{
	union {
		float f32;
		uint32_t u32;
	} b1,b2; 
	b1.f32 = x; 
	b2 = b1; 
	b2.u32 = bswap_32(b1.u32);
	return(b2.f32);
}

double l_endian_f64(double x) 
{
	union {
		double f64;
		uint32_t u32[2];
	} b1,b2; 
	b1.f64 = x; 
	b2 = b1; 
	b2.u32[0] = bswap_32(b1.u32[1]);
	b2.u32[1] = bswap_32(b1.u32[0]);
	return(b2.f64);
}
#endif



/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/


/****************************************************************************/
/**                     INIT HDR                                           **/
/****************************************************************************/
HDRTYPE* create_default_hdr(const unsigned NS, const unsigned N_EVENT)
{
/*
	HDR is initialized, memory is allocated for 
	NS channels and N_EVENT number of events. 

	The purpose is to set the define all parameters at an initial step. 
	No parameters must remain undefined.
 */
	HDRTYPE* hdr = (HDRTYPE*)malloc(sizeof(HDRTYPE));

	union {
		uint32_t testword;
		uint8_t  testbyte[sizeof(uint32_t)];
	} EndianTest; 
    	int k,k1;

	EndianTest.testword = 0x4a3b2c1d; 
	hdr->FILE.LittleEndian = (EndianTest.testbyte[0]==0x1d); 
	
	if  ((hdr->FILE.LittleEndian) && (__BYTE_ORDER == __BIG_ENDIAN))	{
		fprintf(stderr,"Panic: mixed results on Endianity test.\n"); 
		exit(-1); 
	}		
	if  ((!hdr->FILE.LittleEndian) && (__BYTE_ORDER == __LITTLE_ENDIAN))	{
		fprintf(stderr,"Panic: mixed results on Endianity test.\n"); 
		exit(-1); 
	}	

	hdr->FILE.OPEN = 0;
	hdr->FILE.FID = 0;

	hdr->AS.Header1 = NULL;

      	hdr->TYPE = GDF; 
      	hdr->VERSION = 1.94;
      	hdr->AS.rawdata = (uint8_t*) malloc(10);
      	hdr->NRec = 0; 
      	hdr->NS = NS;	
	hdr->SampleRate = 4321.5;
      	memset(hdr->AS.PID,32,81); 
      	hdr->AS.RID = "GRAZ"; 
	hdr->AS.bi = (uint32_t*)calloc(hdr->NS+1,sizeof(uint32_t));
	hdr->data.size[0] = 0; 	// rows 
	hdr->data.size[1] = 0;  // columns 
	hdr->data.block = (biosig_data_type*)malloc(410); 
      	hdr->T0 = t_time2gdf_time(time(NULL));
      	hdr->ID.Equipment = *(uint64_t*)&"b4c_0.35";

	hdr->Patient.Name 	= "X";
	hdr->Patient.Id 	= "X";
	hdr->Patient.Birthday 	= (gdf_time)0;        // Unknown;
      	hdr->Patient.Medication = 0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.DrugAbuse 	= 0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.AlcoholAbuse= 0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.Smoking 	= 0;	// 0:Unknown, 1: NO, 2: YES
      	hdr->Patient.Sex 	= 0;	// 0:Unknown, 1: Male, 2: Female
      	hdr->Patient.Handedness = 0;	// 0:Unknown, 1: Right, 2: Left, 3: Equal
      	hdr->Patient.Impairment.Visual = 0;	// 0:Unknown, 1: NO, 2: YES, 3: Corrected
      	hdr->Patient.Weight 	= 0;	// 0:Unknown
      	hdr->Patient.Height 	= 0;	// 0:Unknown
      	hdr->Dur[0] = 1;
      	hdr->Dur[1] = 1;
	memset(&hdr->IPaddr,0,6);
      	for (k1=0; k1<3; k1++) {
      		hdr->Patient.Headsize[k1] = 0;        // Unknown;
      		hdr->ELEC.REF[k1] = 0.0;
      		hdr->ELEC.GND[k1] = 0.0;
      	}
	hdr->LOC[0] = 0x00292929; 		
	hdr->LOC[1] = 48*3600000+(1<<31); 	// latitude
	hdr->LOC[2] = 15*3600000+(1<<31); 	// longitude 
	hdr->LOC[3] = 35000; 	 	//altitude in centimeter above sea level

	hdr->FLAG.UCAL = 0; 		// un-calibration OFF (auto-scaling ON) 
	hdr->FLAG.OVERFLOWDETECTION = 1; 		// overflow detection ON
	
       	// define variable header 
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

	// define EVENT structure
	hdr->EVENT.N = N_EVENT; 
	hdr->EVENT.SampleRate = hdr->SampleRate; 
	hdr->EVENT.POS = (uint32_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.POS));
	hdr->EVENT.TYP = (uint16_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.TYP));
	hdr->EVENT.DUR = (uint32_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.DUR));
	hdr->EVENT.CHN = (uint16_t*) calloc(hdr->EVENT.N,sizeof(*hdr->EVENT.CHN));
	
	// initialize "Annotated ECG structure"
	hdr->aECG = NULL; 
	return(hdr);
}


/****************************************************************************/
/**                     SOPEN                                              **/
/****************************************************************************/
HDRTYPE* sopen(const char* FileName, const char* MODE, HDRTYPE* hdr)
/*
	MODE="r" 
		reads file and returns HDR 
	MODE="w" 
		writes HDR into file 
 */
{
    	
    	const char*	GENDER = "XMFX";  
    	const uint16_t	CFWB_GDFTYP[] = {17,16,3};  
	const float	CNT_SETTINGS_NOTCH[] = {0.0, 50.0, 60.0}; 
	const float	CNT_SETTINGS_LOWPASS[] = {30, 40, 50, 70, 100, 200, 500, 1000, 1500, 2000, 2500, 3000};
	const float	CNT_SETTINGS_HIGHPASS[] = {NaN, 0, .05, .1, .15, .3, 1, 5, 10, 30, 100, 150, 300};

#ifdef __XML_XMLREADER_H__
	xmlTextReaderPtr reader;
	xmlDocPtr doc; /* the resulting document tree */
	int ret;
#endif

    	int 		k,id;
    	uint32_t	k32u; 
    	size_t	 	count,len,pos;
    	uint8_t 	buf[81];
    	char 		tmp[81];
    	char 		cmd[256];
    	double 		Dur; 
	char* 		Header1;
	char* 		Header2;
	char*		ptr_str;
	struct tm 	tm_time; 
	time_t		tt;
	unsigned	EventChannel = 0;
	struct	{
		int	number_of_sections;
	} SCP;


if (!strcmp(MODE,"r"))	
{
	hdr = create_default_hdr(0,0);	// initializes fields that may stay undefined during SOPEN 

#ifdef ZLIB_H
	hdr->FILE.FID = fopen(FileName,"rb");
#else
	hdr->FILE.FID = fopen(FileName,"rb");
#endif

	hdr->FileName = FileName; 
    	if (hdr->FILE.FID == NULL) 
    	{ 	
    		free(hdr);    		
		return(NULL);
    	}	    
    
    	/******** read 1st (fixed)  header  *******/	
 	Header1 = (char*)malloc(256);
/*#ifdef ZLIB_H
    	count   = gzread(hdr->FILE.FID,Header1,256);
#else
*/
    	count   = fread(Header1,1,256,hdr->FILE.FID);
//#endif
	
    	if (0);
    	else if (!memcmp(Header1+1,"BIOSEMI",7)) {
    		hdr->TYPE = BDF;
    		hdr->VERSION = -1; 
    	}	
    	else if ((Header1[0]==(char)207) & (!Header1[1]) & (!Header1[154]) & (!Header1[155]))
	    	hdr->TYPE = BKR;
    	else if (!memcmp(Header1,"CFWB\1\0\0\0",8))
	    	hdr->TYPE = CFWB;
    	else if (!memcmp(Header1,"Version 3.0",11))
	    	hdr->TYPE = CNT;
    	else if (!memcmp(Header1,"DEMG",4))
	    	hdr->TYPE = DEMG;
    	else if (!memcmp(Header1,"0       ",8)) {
	    	hdr->TYPE = EDF;
	    	hdr->VERSION = 0; 
	}    	
    	else if (!memcmp(Header1,"fLaC",4))
	    	hdr->TYPE = FLAC;
    	else if (!memcmp(Header1,"GDF",3))
	    	hdr->TYPE = GDF; 
    	else if (!memcmp(Header1,"@  MFER ",8))
	    	hdr->TYPE = MFER;
    	else if (!memcmp(Header1,"@ MFR ",6))
	    	hdr->TYPE = MFER;
    	else if (!memcmp(Header1,"NEX1",4))
	    	hdr->TYPE = NEX1;
    	else if (!memcmp(Header1,"PLEX",4))
	    	hdr->TYPE = PLEXON;
    	else if (!memcmp(Header1+16,"SCPECG",6))
	    	hdr->TYPE = SCP_ECG;
    	else {
		fclose(hdr->FILE.FID); 
		free(Header1);

#ifdef __XML_XMLREADER_H__
		LIBXML_TEST_VERSION
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
		else
#endif
		{
			hdr->TYPE = unknown; 
	//		sclose(hdr); 
	//		free(hdr); 
		}	
	}	

    	if (hdr->TYPE == GDF) {
  	    	strncpy(tmp,(char*)Header1+3,5);
	    	hdr->VERSION 	= atof(tmp);
	    	hdr->NRec 	= l_endian_i64( *( int64_t*) (Header1+236) ); 
	    	hdr->Dur[0]  	= l_endian_u32( *(uint32_t*) (Header1+244) );
	    	hdr->Dur[1]  	= l_endian_u32( *(uint32_t*) (Header1+248) ); 
	    	hdr->NS   	= l_endian_u16( *(uint16_t*) (Header1+252) ); 
	    	
	    	if (hdr->VERSION > 1.90) { 
		    	hdr->HeadLen 	= l_endian_u16( *(uint16_t*) (Header1+184) )<<8; 
	    		strncpy(hdr->AS.PID,Header1+8,66);
	    		hdr->Patient.Id = strtok(hdr->AS.PID," ");
	    		hdr->Patient.Name = strtok(NULL," ");
	    		
	    		hdr->Patient.Smoking      =  Header1[84]%4;
	    		hdr->Patient.AlcoholAbuse = (Header1[84]>>2)%4;
	    		hdr->Patient.DrugAbuse    = (Header1[84]>>4)%4;
	    		hdr->Patient.Medication   = (Header1[84]>>6)%4;
	    		hdr->Patient.Weight       =  Header1[85];
	    		hdr->Patient.Height       =  Header1[86];
	    		hdr->Patient.Sex       	  =  Header1[87]%4;
	    		hdr->Patient.Handedness   = (Header1[87]>>2)%4;
	    		hdr->Patient.Impairment.Visual = (Header1[87]>>4)%4;
	
			*(uint32_t*)(Header1+156) = l_endian_u32( *(uint32_t*) (Header1+156) );
			*(uint32_t*)(Header1+160) = l_endian_u32( *(uint32_t*) (Header1+160) );
			*(uint32_t*)(Header1+164) = l_endian_u32( *(uint32_t*) (Header1+164) );
			if (Header1[156]) {
				hdr->LOC[0] = 0x00292929;
				memcpy(&hdr->LOC[1], Header1+156, 12);
			}
			else {
				*(uint32_t*) (Header1+152) = l_endian_u32(*(uint32_t*) (Header1+152));
				memcpy(&hdr->LOC, Header1+152, 16);
			}

			hdr->T0 		= l_endian_i64( *(int64_t*) (Header1+168) );
			hdr->Patient.Birthday 	= l_endian_i64( *(int64_t*) (Header1+176) );
			// memcpy(&hdr->T0, Header1+168,8);
			// memcpy(&hdr->Patient.Birthday, Header1+176, 8);

			hdr->ID.Equipment 	= l_endian_i64( *(int64_t*) (Header1+192) );
			memcpy(&hdr->IPaddr, Header1+200,6);
			hdr->Patient.Headsize[0]= l_endian_u16( *(uint16_t*)(Header1+206) );
			hdr->Patient.Headsize[1]= l_endian_u16( *(uint16_t*)(Header1+208) );
			hdr->Patient.Headsize[2]= l_endian_u16( *(uint16_t*)(Header1+210) );

			//memcpy(&hdr->ELEC.REF, Header1+212,12);
			//memcpy(&hdr->ELEC.GND, Header1+224,12);
			hdr->ELEC.REF[0]   = l_endian_f32( *(float*)(Header1+ 212) );
			hdr->ELEC.REF[1]   = l_endian_f32( *(float*)(Header1+ 216) );
			hdr->ELEC.REF[2]   = l_endian_f32( *(float*)(Header1+ 220) );
			hdr->ELEC.GND[0]   = l_endian_f32( *(float*)(Header1+ 212) );
			hdr->ELEC.GND[1]   = l_endian_f32( *(float*)(Header1+ 216) );
			hdr->ELEC.GND[2]   = l_endian_f32( *(float*)(Header1+ 220) );
	    	}
	    	else {
	    		strncpy(hdr->AS.PID,Header1+8,80);
	    		hdr->Patient.Id = strtok(hdr->AS.PID," ");
	    		hdr->Patient.Name = strtok(NULL," ");
	    		
	    		tm_time.tm_sec  = atoi(strncpy(tmp,Header1+168+12,2)); 
	    		tm_time.tm_min  = atoi(strncpy(tmp,Header1+168+10,2)); 
	    		tm_time.tm_hour = atoi(strncpy(tmp,Header1+168+8,2)); 
	    		tm_time.tm_mday = atoi(strncpy(tmp,Header1+168+6,2)); 
	    		tm_time.tm_mon  = atoi(strncpy(tmp,Header1+168+4,2)); 
	    		tm_time.tm_year = atoi(strncpy(tmp,Header1+168,4)); 
	    		hdr->T0 = t_time2gdf_time(mktime(&tm_time)); 
		    	hdr->HeadLen 	= l_endian_u64( *(uint64_t*) (Header1+184) ); 
	    	}

	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	    	Header1 = (char*)realloc(Header1,hdr->HeadLen);
	    	Header2 = Header1+256; 
	    	count   = fread(Header2, 1, hdr->HeadLen-256, hdr->FILE.FID);
		for (k=0; k<hdr->NS; k++)	{
			Header2[16*k + 15] = 0;
			hdr->CHANNEL[k].Label   = (Header2 + 16*k);
			Header2[16*hdr->NS + 80*k + 79] = 0;
			hdr->CHANNEL[k].Transducer  = (Header2 + 16*hdr->NS + 80*k);
			
			hdr->CHANNEL[k].PhysMin = l_endian_f64( *(double*) (Header2+ 8*k + 104*hdr->NS) );
			hdr->CHANNEL[k].PhysMax = l_endian_f64( *(double*) (Header2+ 8*k + 112*hdr->NS) );

			hdr->CHANNEL[k].SPR     = l_endian_u32( *(uint32_t*) (Header2+ 4*k + 216*hdr->NS) );
			hdr->CHANNEL[k].GDFTYP  = l_endian_u16( *(uint16_t*) (Header2+ 4*k + 220*hdr->NS) );
			if (hdr->VERSION < 1.90) {
				Header2[96*hdr->NS + 16*k + 15] = 0;
				hdr->CHANNEL[k].PhysDim = (Header2 + 96*hdr->NS + 8*k);
				/* ###FIXME###
				hdr->CHANNEL[k].PhysDimCode = 
				hdr->CHANNEL[k].PreFilt = (hdr->Header2+ 68*k + 136*hdr->NS);
				*/

				hdr->CHANNEL[k].DigMin   = (double) l_endian_i64( *(int64_t*)(Header2+ 8*k + 120*hdr->NS) );
				hdr->CHANNEL[k].DigMax   = (double) l_endian_i64( *(int64_t*)(Header2+ 8*k + 128*hdr->NS) );
			}	
			else {
				/* ###FIXME###
				hdr->CHANNEL[k].PhysDim = 
				*/
				hdr->CHANNEL[k].PhysDimCode = l_endian_u16( *(uint16_t*)(Header2+ 2*k + 102*hdr->NS) );

				hdr->CHANNEL[k].DigMin   = l_endian_f64( *(double*)(Header2+ 8*k + 120*hdr->NS) );
				hdr->CHANNEL[k].DigMax   = l_endian_f64( *(double*)(Header2+ 8*k + 128*hdr->NS) );

				hdr->CHANNEL[k].LowPass  = l_endian_f32( *(float*) (Header2+ 4*k + 204*hdr->NS) );
				hdr->CHANNEL[k].HighPass = l_endian_f32( *(float*) (Header2+ 4*k + 208*hdr->NS) );
				hdr->CHANNEL[k].Notch    = l_endian_f32( *(float*) (Header2+ 4*k + 212*hdr->NS) );
				hdr->CHANNEL[k].XYZ[0]   = l_endian_f32( *(float*) (Header2+ 4*k + 224*hdr->NS) );
				hdr->CHANNEL[k].XYZ[1]   = l_endian_f32( *(float*) (Header2+ 4*k + 228*hdr->NS) );
				hdr->CHANNEL[k].XYZ[2]   = l_endian_f32( *(float*) (Header2+ 4*k + 232*hdr->NS) );
				//memcpy(&hdr->CHANNEL[k].XYZ,Header2 + 4*k + 224*hdr->NS,12);
				hdr->CHANNEL[k].Impedance= ldexp(1.0, (uint8_t)Header2[k + 236*hdr->NS]/8);
			}

			hdr->CHANNEL[k].Cal   	= (hdr->CHANNEL[k].PhysMax-hdr->CHANNEL[k].PhysMin)/(hdr->CHANNEL[k].DigMax-hdr->CHANNEL[k].DigMin);
			hdr->CHANNEL[k].Off   	=  hdr->CHANNEL[k].PhysMin-hdr->CHANNEL[k].Cal*hdr->CHANNEL[k].DigMin;

		}
		for (k=0, hdr->SPR=1, hdr->AS.spb=0, hdr->AS.bpb=0; k<hdr->NS;k++) {
			hdr->AS.spb += hdr->CHANNEL[k].SPR;
			hdr->AS.bpb += GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP]*hdr->CHANNEL[k].SPR;			
			hdr->SPR = lcm(hdr->SPR,hdr->CHANNEL[k].SPR);
		}	
		hdr->SampleRate = ((double)(hdr->SPR))*hdr->Dur[1]/hdr->Dur[0];

		// READ EVENTTABLE 
		fseek(hdr->FILE.FID, hdr->HeadLen + hdr->AS.bpb*hdr->NRec, SEEK_SET); 
		fread(buf, sizeof(uint8_t), 8, hdr->FILE.FID);

		if (hdr->VERSION < 1.94) {
			hdr->EVENT.SampleRate = (float)buf[1] + (buf[2] + buf[3]*256.0)*256.0; 
			hdr->EVENT.N = l_endian_u32( *(uint32_t*) (buf + 4) );
		}	
		else	{
			hdr->EVENT.N = buf[1] + (buf[2] + buf[3]*256)*256; 
			hdr->EVENT.SampleRate = l_endian_f32( *(float*) (buf + 4) );
		}	
		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, hdr->EVENT.N*sizeof(*hdr->EVENT.POS) );
		hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP) );
		fread(hdr->EVENT.POS, sizeof(*hdr->EVENT.POS), hdr->EVENT.N, hdr->FILE.FID);
		fread(hdr->EVENT.TYP, sizeof(*hdr->EVENT.TYP), hdr->EVENT.N, hdr->FILE.FID);
		for (k32u=0; k32u < hdr->EVENT.N; k32u++) {
			hdr->EVENT.POS[k32u] = l_endian_u32(hdr->EVENT.POS[k32u]); 
			hdr->EVENT.TYP[k32u] = l_endian_u16(hdr->EVENT.TYP[k32u]); 
		}
		if (tmp[0]>1) {
			hdr->EVENT.DUR = (uint32_t*) realloc(hdr->EVENT.DUR,hdr->EVENT.N*sizeof(*hdr->EVENT.DUR));
			hdr->EVENT.CHN = (uint16_t*) realloc(hdr->EVENT.CHN,hdr->EVENT.N*sizeof(*hdr->EVENT.CHN));
			fread(hdr->EVENT.CHN,sizeof(*hdr->EVENT.CHN),hdr->EVENT.N,hdr->FILE.FID);
			fread(hdr->EVENT.DUR,sizeof(*hdr->EVENT.DUR),hdr->EVENT.N,hdr->FILE.FID);
			for (k32u=0; k32u<hdr->EVENT.N; k32u++) {
				hdr->EVENT.DUR[k32u] = l_endian_u32(hdr->EVENT.DUR[k32u]); 
				hdr->EVENT.CHN[k32u] = l_endian_u16(hdr->EVENT.CHN[k32u]); 
			}
		}
		else {
			hdr->EVENT.DUR = NULL;
			hdr->EVENT.CHN = NULL;
		}	
    	}
    	else if ((hdr->TYPE == EDF) | (hdr->TYPE == BDF))	{
    		strncpy(hdr->AS.PID,Header1+8,80);

	    	hdr->HeadLen 	= atoi(strncpy(tmp,Header1+184,8));
	    	hdr->NRec 	= atoi(strncpy(tmp,Header1+236,8));
	    	Dur 		= atoi(strncpy(tmp,Header1+244,8));
	    	hdr->NS		= atoi(strncpy(tmp,Header1+252,4));
		if (!Dur)
		{	
			hdr->Dur[0] = lround(Dur*1e7); 
			hdr->Dur[1] = 10000000L; 
		}
    		tm_time.tm_sec  = atoi(strncpy(tmp,Header1+168+14,2)); 
    		tm_time.tm_min  = atoi(strncpy(tmp,Header1+168+11,2)); 
    		tm_time.tm_hour = atoi(strncpy(tmp,Header1+168+8,2)); 
    		tm_time.tm_mday = atoi(strncpy(tmp,Header1+168,2)); 
    		tm_time.tm_mon  = atoi(strncpy(tmp,Header1+168+3,2)); 
    		tm_time.tm_year = atoi(strncpy(tmp,Header1+168+6,2)); 
    		tm_time.tm_year+= (tm_time.tm_year<85)*100;
		hdr->T0 = tm_time2gdf_time(&tm_time); 

		if (!strncmp(Header1+192,"EDF+",4)) {
	    		hdr->Patient.Id  = strtok(hdr->AS.PID," ");
	    		ptr_str = strtok(NULL," ");
	    		hdr->Patient.Sex = (ptr_str[0]=='f')*2 + (ptr_str[0]=='F')*2 + (ptr_str[0]=='M') + (ptr_str[0]=='m');
	    		ptr_str = strtok(NULL," ");	// birthday
	    		hdr->Patient.Name= strtok(NULL," ");

			if (strlen(ptr_str)==11) {
		    		tm_time.tm_mday = atoi(strtok(ptr_str,"-")); 
		    		strcpy(tmp,strtok(NULL,"-"));
		    		tm_time.tm_year = atoi(strtok(NULL,"-"))-1900; 
		    		tm_time.tm_mon  = !strcmp(tmp,"Feb")+!strcmp(tmp,"Mar")*2+!strcmp(tmp,"Apr")*3+!strcmp(tmp,"May")*4+!strcmp(tmp,"Jun")*5+!strcmp(tmp,"Jul")*6+!strcmp(tmp,"Aug")*7+!strcmp(tmp,"Sep")*8+!strcmp(tmp,"Oct")*9+!strcmp(tmp,"Nov")*10+!strcmp(tmp,"Dec")*11;
		    		tm_time.tm_sec  = 0; 
		    		tm_time.tm_min  = 0; 
		    		tm_time.tm_hour = 12; 
		    		hdr->Patient.Birthday = t_time2gdf_time(mktime(&tm_time));
		    	}	
		}

	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	    	Header1 = (char*) realloc(Header1,hdr->HeadLen);
	    	Header2 = Header1+256; 
	    	count   = fread(Header2, 1, hdr->HeadLen-256, hdr->FILE.FID);
		for (k=0; k<hdr->NS; k++)	{
			hdr->CHANNEL[k].Label   = (Header2 + 16*k);
			hdr->CHANNEL[k].Label[15]=0;//   hack
			hdr->CHANNEL[k].Transducer  = (Header2 + 80*k + 16*hdr->NS);
			hdr->CHANNEL[k].Transducer[79]=0;//   hack
			
			hdr->CHANNEL[k].PhysDim = (Header2 + 8*k + 96*hdr->NS);
			tmp[8] = 0;
			strncpy(tmp,Header2 + 8*k + 104*hdr->NS,8);
			// PhysDim -> PhysDimCode belongs here 
			hdr->CHANNEL[k].PhysDim[7]=0; //hack
			
			hdr->CHANNEL[k].PhysMin = atof(strncpy(tmp,Header2 + 8*k + 104*hdr->NS,8)); 
			hdr->CHANNEL[k].PhysMax = atof(strncpy(tmp,Header2 + 8*k + 112*hdr->NS,8)); 
			hdr->CHANNEL[k].DigMin  = atof(strncpy(tmp,Header2 + 8*k + 120*hdr->NS,8)); 
			hdr->CHANNEL[k].DigMax  = atof(strncpy(tmp,Header2 + 8*k + 128*hdr->NS,8)); 
			hdr->CHANNEL[k].Cal     = (hdr->CHANNEL[k].PhysMax-hdr->CHANNEL[k].PhysMin)/(hdr->CHANNEL[k].DigMax-hdr->CHANNEL[k].DigMin);
			hdr->CHANNEL[k].Off     =  hdr->CHANNEL[k].PhysMin-hdr->CHANNEL[k].Cal*hdr->CHANNEL[k].DigMin;

			hdr->CHANNEL[k].SPR     = atol(strncpy(tmp,Header2+  8*k + 216*hdr->NS,8));
			hdr->CHANNEL[k].GDFTYP  = ((hdr->TYPE!=BDF) ? 3 : 255+24); 
			
			//hdr->CHANNEL[k].PreFilt = (Header2+ 80*k + 136*hdr->NS);
			if ((hdr->TYPE==EDF) & strncmp(Header1+192,"EDF+",4)) {
				hdr->CHANNEL[k].OnOff = memcmp(hdr->CHANNEL[k].Label,"EDF Annotations",15);
				EventChannel = k; 
			// decode filter information into hdr->Filter.{Lowpass, Highpass, Notch}					
			}	
			else if (hdr->TYPE==BDF) {
				hdr->CHANNEL[k].OnOff = strcmp(hdr->CHANNEL[k].Label,"Status");
				EventChannel = k; 
			}	
			else	{
				hdr->CHANNEL[k].OnOff = 1;
			}
		}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// EDF does not support automated overflow and saturation detection

		for (k=0, hdr->SPR=1, hdr->AS.spb=0, hdr->AS.bpb=0; k<hdr->NS; k++) {
			hdr->AS.spb += hdr->CHANNEL[k].SPR;
			hdr->AS.bpb += GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP]*hdr->CHANNEL[k].SPR;
			if (hdr->CHANNEL[k].OnOff)
				hdr->SPR = lcm(hdr->SPR,hdr->CHANNEL[k].SPR);
		}	
		hdr->SampleRate = ((double)(hdr->SPR))*hdr->Dur[1]/hdr->Dur[0];
	}      	
	else if (hdr->TYPE==BKR) {
	    	Header1 = (char*) realloc(Header1,1024);
	    	count   = fread(Header1+256,1,1024-256,hdr->FILE.FID);
	    	hdr->HeadLen 	 = 1024; 
		hdr->NS  	 = l_endian_u16( *(uint16_t*) (Header1+2) ); 
		hdr->SampleRate  = (double)l_endian_u16( *(uint16_t*) (Header1+4) ); 
		hdr->NRec   	 = l_endian_u32( *(uint32_t*) (Header1+6) ); 
		hdr->SPR  	 = l_endian_u32( *(uint32_t*) (Header1+10) ); 
		hdr->NRec 	*= hdr->SPR; 
		hdr->SPR  	 = 1; 
		hdr->T0 	 = 0;        // Unknown;
	    	/* extract more header information */
	    	hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS;k++)	{
		    	hdr->CHANNEL[k].GDFTYP 	 = 3; 
		    	hdr->CHANNEL[k].SPR 	 = 1; // *(int32_t*)(Header1+56);
		    	*(uint32_t*)(Header1+22) = l_endian_u32(*(uint32_t*)Header1+22);
		    	*(uint32_t*)(Header1+26) = l_endian_u32(*(uint32_t*)Header1+26);
		    	hdr->CHANNEL[k].LowPass	 = *(float*)(Header1+22);
		    	hdr->CHANNEL[k].HighPass = *(float*)(Header1+26);
		    	hdr->CHANNEL[k].Notch	 = -1.0;
		    	*(uint16_t*)(Header1+14) = l_endian_u16(*(uint16_t*)Header1+14);
		    	*(uint16_t*)(Header1+16) = l_endian_u16(*(uint16_t*)Header1+16);
		    	hdr->CHANNEL[k].PhysMax	 = (double)*(uint16_t*)(Header1+14);
		    	hdr->CHANNEL[k].DigMax	 = (double)*(uint16_t*)(Header1+16);
		    	hdr->CHANNEL[k].Cal	 =  ((double)hdr->CHANNEL[k].PhysMax)/hdr->CHANNEL[k].DigMax;
		    	hdr->CHANNEL[k].Off	 = 0.0;
			hdr->CHANNEL[k].OnOff    = 1;
		}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// BKR does not support automated overflow and saturation detection
	}
	else if (hdr->TYPE==CFWB) {
	    	*(uint64_t*)(Header1+8) = l_endian_u64(*(uint64_t*)Header1+8);
	    	hdr->SampleRate = *(double*) (Header1+8);
	    	tm_time.tm_year = l_endian_u32( *(int32_t*)(Header1+16) );
	    	tm_time.tm_mon  = l_endian_u32( *(int32_t*)(Header1+20) );
	    	tm_time.tm_mday = l_endian_u32( *(int32_t*)(Header1+24) );
	    	tm_time.tm_hour = l_endian_u32( *(int32_t*)(Header1+28) );
	    	tm_time.tm_min  = l_endian_u32( *(int32_t*)(Header1+32) );
	    	*(uint64_t*)(Header1+36) = l_endian_u64(*(uint64_t*)Header1+36);
	    	tm_time.tm_sec  = (int)*(double*) (Header1+36);
    		hdr->T0 	= tm_time2gdf_time(&tm_time);
	    	// = *(double*)(Header1+44);
	    	hdr->NS   	= l_endian_u32( *(int32_t*)(Header1+52) );
	    	hdr->NRec	= l_endian_u32( *(int32_t*)(Header1+56) );
	    	//  	= *(int32_t*)(Header1+60);	// TimeChannel
	    	//  	= *(int32_t*)(Header1+64);	// DataFormat

	    	Header1 = (char*) realloc(Header1,68+96*hdr->NS);
	    	Header2 = Header1+96; 
	    	hdr->HeadLen = 68+96*hdr->NS; 
	    	fseek(hdr->FILE.FID,68,SEEK_SET);
		count   = fread(Header2,1,96*hdr->NS,hdr->FILE.FID);
	    	
	    	hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL,hdr->NS*sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS; k++)	{
		    	hdr->CHANNEL[k].GDFTYP 	= CFWB_GDFTYP[l_endian_u32(*(uint32_t*)(Header1+64))-1];
		    	hdr->CHANNEL[k].SPR 	= 1; // *(int32_t*)(Header1+56);
		    	hdr->CHANNEL[k].Label	= Header2+k*96;
		    	hdr->CHANNEL[k].PhysDim	= Header2+k*96+32;
		    	hdr->CHANNEL[k].Cal	= l_endian_f64(*(double*)(Header2+k*96+64));
		    	hdr->CHANNEL[k].Off	= l_endian_f64(*(double*)(Header2+k*96+72));
		    	hdr->CHANNEL[k].PhysMax	= l_endian_f64(*(double*)(Header2+k*96+80));
		    	hdr->CHANNEL[k].PhysMin	= l_endian_f64(*(double*)(Header2+k*96+88));
			hdr->CHANNEL[k].OnOff    = 1;
		}
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// CFWB does not support automated overflow and saturation detection
	}
	else if (hdr->TYPE==CNT) {
	    	Header1 = (char*) realloc(Header1,900);
	    	hdr->VERSION = atof(Header1+8);
	    	count   = fread(Header1+256,1,900-256,hdr->FILE.FID);
	    	ptr_str = Header1+136;
    		hdr->Patient.Sex = (ptr_str[0]=='f')*2 + (ptr_str[0]=='F')*2 + (ptr_str[0]=='M') + (ptr_str[0]=='m');
	    	ptr_str = Header1+137;
	    	hdr->Patient.Handedness = (ptr_str[0]=='r')*2 + (ptr_str[0]=='R')*2 + (ptr_str[0]=='L') + (ptr_str[0]=='l');
    		tm_time.tm_sec  = atoi(strncpy(tmp,Header1+225+16,2)); 
    		tm_time.tm_min  = atoi(strncpy(tmp,Header1+225+13,2)); 
    		tm_time.tm_hour = atoi(strncpy(tmp,Header1+225+10,2)); 
    		tm_time.tm_mday = atoi(strncpy(tmp,Header1+225,2)); 
    		tm_time.tm_mon  = atoi(strncpy(tmp,Header1+225+3,2)); 
    		tm_time.tm_year = atoi(strncpy(tmp,Header1+225+6,2)); 
	    	if (tm_time.tm_year<=80)    	tm_time.tm_year += 2000;
	    	else if (tm_time.tm_year<100) 	tm_time.tm_year += 1900;
		hdr->T0 = tm_time2gdf_time(&tm_time); 
		
		hdr->NS  = l_endian_u16( *(uint16_t*) (Header1+370) ); 
	    	hdr->HeadLen = 900+hdr->NS*75; 
		hdr->SampleRate = l_endian_u16( *(uint16_t*) (Header1+376) ); 
		hdr->SPR = 1; 
#define _eventtablepos (*(uint32_t*)(Header1+886))
		hdr->NRec = (_eventtablepos-hdr->HeadLen)/(hdr->NS*2);
		
	    	Header1 = (char*) realloc(Header1,hdr->HeadLen);
	    	Header2 = Header1+900; 
	    	count   = fread(Header2,1,hdr->NS*75,hdr->FILE.FID);

	    	hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
		for (k=0; k<hdr->NS;k++)	{
		    	hdr->CHANNEL[k].GDFTYP 	= 3;
		    	hdr->CHANNEL[k].SPR 	= 1; // *(int32_t*)(Header1+56);
		    	hdr->CHANNEL[k].Label	= Header2+k*75;
		    	hdr->CHANNEL[k].PhysDim	= "uV";
		    	hdr->CHANNEL[k].PhysDimCode = 4256+19;
		    	hdr->CHANNEL[k].Cal	= l_endian_f32(*(float*)(Header2+k*75+59));
		    	hdr->CHANNEL[k].Cal    *= l_endian_f32(*(float*)(Header2+k*75+71))/204.8;
		    	hdr->CHANNEL[k].Off	= l_endian_f32(*(float*)(Header2+k*75+47)) * hdr->CHANNEL[k].Cal;
		    	hdr->CHANNEL[k].HighPass= CNT_SETTINGS_HIGHPASS[(uint8_t)Header2[64+k*75]];
		    	hdr->CHANNEL[k].LowPass	= CNT_SETTINGS_LOWPASS[(uint8_t)Header2[65+k*75]];
		    	hdr->CHANNEL[k].Notch	= CNT_SETTINGS_NOTCH[(uint8_t)Header1[682]];
			hdr->CHANNEL[k].OnOff    = 1;
		}
	    	/* extract more header information */
	    	// eventtablepos = l_endian_u32( *(uint32_t*) (Header1+886) );
	    	fseek(hdr->FILE.FID,_eventtablepos,SEEK_SET);
		hdr->FLAG.OVERFLOWDETECTION = 0; 	// automated overflow and saturation detection not supported
	}
	
	else if (hdr->TYPE==SCP_ECG) {
		fseek(hdr->FILE.FID,0,-1);
		hdr = sopen_SCP_read(Header1,hdr);
	}
	
	else if (hdr->TYPE==XML) 
	{

		hdr = sopen_HL7aECG_read(hdr);

	}
	
	else // if (hdr->TYPE==unknown) 
	{
		fprintf(stdout,"unknown file format %s \n",hdr->FileName);
	}
	
	
}
else { // WRITE

    	if (hdr->TYPE==GDF) {	
	     	hdr->HeadLen = (hdr->NS+1)*256;
	    	Header1 = (char*) malloc(hdr->HeadLen);
	    	Header2 = Header1+256; 

		memset(Header1,0,hdr->HeadLen);
	     	sprintf(Header1,"GDF %4.2f",hdr->VERSION);
	     	strncat(Header1+8, hdr->Patient.Id,   66);
	     	strncat(Header1+8, " ",   66);
	     	strncat(Header1+8, hdr->Patient.Name, 66);

	     	Header1[84] = (hdr->Patient.Smoking%4) + ((hdr->Patient.AlcoholAbuse%4)<<2) + ((hdr->Patient.DrugAbuse%4)<<4) + ((hdr->Patient.Medication%4)<<6);
	     	Header1[85] =  hdr->Patient.Weight;
	     	Header1[86] =  hdr->Patient.Height;
	     	Header1[87] = (hdr->Patient.Sex%4) + ((hdr->Patient.Handedness%4)<<2) + ((hdr->Patient.Impairment.Visual%4)<<4);

	     	len = strlen(hdr->AS.RID);
	     	memcpy(Header1+ 88,  hdr->AS.RID, min(len,80));
		memcpy(Header1+152, &hdr->LOC, 16);  
		*(uint32_t*) (Header1+152) = l_endian_u32( *(uint32_t*) (Header1+152) );
		*(uint32_t*) (Header1+156) = l_endian_u32( *(uint32_t*) (Header1+156) );
		*(uint32_t*) (Header1+160) = l_endian_u32( *(uint32_t*) (Header1+160) );
		*(uint32_t*) (Header1+164) = l_endian_u32( *(uint32_t*) (Header1+164) );
	
		//memcpy(Header1+168, &hdr->T0, 8); 
		*(uint64_t*) (Header1+168) = l_endian_u64(hdr->T0);
		//memcpy(Header1+176, &hdr->Patient.Birthday, 8); 
		*(uint64_t*) (Header1+176) = l_endian_u64(hdr->Patient.Birthday);
		// *(uint16_t*)(Header1+184) = (hdr->HeadLen>>8)+(hdr->HeadLen%256>0); 
		*(uint16_t*) (Header1+184) = l_endian_u16(hdr->HeadLen>>8); 
		
		memcpy(Header1+192, &hdr->ID.Equipment, 8);

		memcpy(Header1+200, &hdr->IPaddr, 6);
		memcpy(Header1+206, &hdr->Patient.Headsize, 6);
		*(float*) (Header1+212) = l_endian_f32(hdr->ELEC.REF[0]);
		*(float*) (Header1+216) = l_endian_f32(hdr->ELEC.REF[1]);
		*(float*) (Header1+220) = l_endian_f32(hdr->ELEC.REF[2]);
		*(float*) (Header1+224) = l_endian_f32(hdr->ELEC.GND[0]);
		*(float*) (Header1+228) = l_endian_f32(hdr->ELEC.GND[1]);
		*(float*) (Header1+232) = l_endian_f32(hdr->ELEC.GND[2]);

		//memcpy(Header1+236, &hdr->NRec, 8);
		*(uint64_t*) (Header1+236) = l_endian_u64(hdr->NRec);
		//memcpy(Header1+244, &hdr->Dur, 8); 
		*(uint32_t*) (Header1+244) = l_endian_u32(hdr->Dur[0]);
		*(uint32_t*) (Header1+248) = l_endian_u32(hdr->Dur[1]);
		//memcpy(Header1+252, &hdr->NS, 2); 
		*(uint16_t*) (Header1+252) = l_endian_u16(hdr->NS);

	     	/* define HDR.Header2 
	     	this requires checking the arguments in the fields of the struct HDR.CHANNEL
	     	and filling in the bytes in HDR.Header2. 
	     	*/
		for (k=0;k<hdr->NS;k++)
		{
		     	len = strlen(hdr->CHANNEL[k].Label);
		     	memcpy(Header2+16*k,hdr->CHANNEL[k].Label,min(len,16));
		     	len = strlen(hdr->CHANNEL[k].Transducer);
		     	memcpy(Header2+80*k + 16*hdr->NS,hdr->CHANNEL[k].Transducer,min(len,80));
		     	len = strlen(hdr->CHANNEL[k].PhysDim);
		     	if (hdr->VERSION<1.9)
		     		memcpy(Header2+ 8*k + 96*hdr->NS,hdr->CHANNEL[k].PhysDim,min(len,8));
		     	else {	
		     		memcpy(Header2+ 6*k + 96*hdr->NS,hdr->CHANNEL[k].PhysDim,min(len,6));
		     		*(uint16_t*)(Header2+ 2*k +102*hdr->NS) = l_endian_u16(hdr->CHANNEL[k].PhysDimCode);
			};
		     	*(double*)(Header2 + 8*k + 104*hdr->NS)   = l_endian_f64(hdr->CHANNEL[k].PhysMin);
		     	*(double*)(Header2 + 8*k + 112*hdr->NS)   = l_endian_f64(hdr->CHANNEL[k].PhysMax);
		     	*(double*)(Header2 + 8*k + 120*hdr->NS)   = l_endian_f64(hdr->CHANNEL[k].DigMin);
		     	*(double*)(Header2 + 8*k + 128*hdr->NS)   = l_endian_f64(hdr->CHANNEL[k].DigMax);

		     	*(float*) (Header2+ 4*k + 204*hdr->NS)    = l_endian_f32(hdr->CHANNEL[k].LowPass);
		     	*(float*) (Header2+ 4*k + 208*hdr->NS)    = l_endian_f32(hdr->CHANNEL[k].HighPass);
		     	*(float*) (Header2+ 4*k + 212*hdr->NS)    = l_endian_f32(hdr->CHANNEL[k].Notch);
		     	*(uint32_t*) (Header2+ 4*k + 216*hdr->NS) = l_endian_u32(hdr->CHANNEL[k].SPR);
		     	*(uint32_t*) (Header2+ 4*k + 220*hdr->NS) = l_endian_u32(hdr->CHANNEL[k].GDFTYP);

			*(float*) (Header2+ 4*k + 224*hdr->NS)    = l_endian_f32(hdr->CHANNEL[k].XYZ[0]);
			*(float*) (Header2+ 4*k + 228*hdr->NS)    = l_endian_f32(hdr->CHANNEL[k].XYZ[1]);
			*(float*) (Header2+ 4*k + 232*hdr->NS)    = l_endian_f32(hdr->CHANNEL[k].XYZ[2]);

	     		Header2[k+236*hdr->NS] = (uint8_t)ceil(log10(min(39e8,hdr->CHANNEL[k].Impedance))/log10(2.0)*8.0-0.5);
		}
	    	hdr->AS.bi = (uint32_t*) realloc(hdr->AS.bi,(hdr->NS+1)*sizeof(int32_t));
		hdr->AS.bi[0] = 0;
		for (k=0, hdr->AS.spb=0, hdr->AS.bpb=0; k<hdr->NS;)
		{
			hdr->AS.spb += hdr->CHANNEL[k].SPR;
			hdr->AS.bpb += GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP] * hdr->CHANNEL[k].SPR;
			hdr->AS.bi[++k] = hdr->AS.bpb; 
		}	
		hdr->AS.Header1 = (uint8_t*)Header1; 
	}
    	else if ((hdr->TYPE==EDF) | (hdr->TYPE==BDF)) {	
	     	hdr->HeadLen = (hdr->NS+1)*256;
	    	Header1 = (char*) malloc(hdr->HeadLen);
	    	Header2 = Header1+256; 
		if (hdr->TYPE==BDF) {	
			Header1[0] = 255;
	     		memcpy(Header1+1,"BIOSEMI",7);
		}
		else {
			memset(Header1,' ',hdr->HeadLen);
	     		memcpy(Header1,"0       ",8);
	     	}

		tt = gdf_time2t_time(hdr->Patient.Birthday); 
		if (hdr->Patient.Birthday>1) strftime(tmp,81,"%d-%b-%Y",localtime(&tt));
		else strcpy(tmp,"X");	
		sprintf(cmd,"%s %c %s %s",hdr->Patient.Id,GENDER[hdr->Patient.Sex],tmp,hdr->Patient.Name);
	     	memcpy(Header1+8, cmd, strlen(cmd));
	     	
		tt = gdf_time2t_time(hdr->T0); 
		if (hdr->T0>1) strftime(tmp,81,"%d-%b-%Y",localtime(&tt));
		else strcpy(tmp,"X");	
		len = sprintf(cmd,"Startdate %s X X ",tmp);
	     	memcpy(Header1+88, cmd, len);
	     	memcpy(Header1+88+len, &hdr->ID.Equipment, 8);
	     	
		tt = gdf_time2t_time(hdr->T0); 
		strftime(tmp,81,"%d.%m.%y%H:%M:%S",localtime(&tt));
	     	memcpy(Header1+168, tmp, 16);

		len = sprintf(tmp,"%i",hdr->HeadLen);
		if (len>8) fprintf(stderr,"Warning: HeaderLength is (%s) to long.\n",tmp);  
	     	memcpy(Header1+184, tmp, len);
	     	memcpy(Header1+192, "EDF+C  ", 5);

		len = sprintf(tmp,"%lu",hdr->NRec);
		if (len>8) fprintf(stderr,"Warning: NRec is (%s) to long.\n",tmp);  
	     	memcpy(Header1+236, tmp, len);

		len = sprintf(tmp,"%f",((double)hdr->Dur[0])/hdr->Dur[1]);
		if (len>8) fprintf(stderr,"Warning: Duration is %s) to long.\n",tmp);  
	     	memcpy(Header1+244, tmp, len);

		len = sprintf(tmp,"%i",hdr->NS);
		if (len>4) fprintf(stderr,"Warning: NS is %s) to long.\n",tmp);  
	     	memcpy(Header1+252, tmp, len);
	     	
		for (k=0;k<hdr->NS;k++)
		{
		     	len = strlen(hdr->CHANNEL[k].Label);
			if (len>16) fprintf(stderr,"Warning: Label (%s) of channel %i is to long.\n",hdr->CHANNEL[k].Label,k);  
		     	memcpy(Header2+16*k,hdr->CHANNEL[k].Label,min(len,16));
		     	len = strlen(hdr->CHANNEL[k].Transducer);
			if (len>80) fprintf(stderr,"Warning: Transducer (%s) of channel %i is to long.\n",hdr->CHANNEL[k].Transducer,k);  
		     	memcpy(Header2+80*k + 16*hdr->NS,hdr->CHANNEL[k].Transducer,min(len,80));
		     	len = strlen(hdr->CHANNEL[k].PhysDim);
			if (len>8) fprintf(stderr,"Warning: Physical Dimension (%s) of channel %i is to long.\n",hdr->CHANNEL[k].PhysDim,k);  
		     	memcpy(Header2+ 8*k + 96*hdr->NS,hdr->CHANNEL[k].PhysDim,min(len,8));
	
			len = sprintf(tmp,"%f",hdr->CHANNEL[k].PhysMin);
			if (len>8) fprintf(stderr,"Warning: PhysMin (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 104*hdr->NS,tmp,min(8,len));
			len = sprintf(tmp,"%f",hdr->CHANNEL[k].PhysMax);
			if (len>8) fprintf(stderr,"Warning: PhysMax (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 112*hdr->NS,tmp,min(8,len));
			len = sprintf(tmp,"%f",hdr->CHANNEL[k].DigMin);
			if (len>8) fprintf(stderr,"Warning: DigMin (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 120*hdr->NS,tmp,min(8,len));
			len = sprintf(tmp,"%f",hdr->CHANNEL[k].DigMax);
			if (len>8) fprintf(stderr,"Warning: DigMax (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 128*hdr->NS,tmp,min(8,len));
		     	
			if (hdr->CHANNEL[k].Notch>0)		     	
				len = sprintf(tmp,"HP:%fHz LP:%fHz Notch:%fHz",hdr->CHANNEL[k].HighPass,hdr->CHANNEL[k].LowPass,hdr->CHANNEL[k].Notch);
			else
				len = sprintf(tmp,"HP:%fHz LP:%fHz",hdr->CHANNEL[k].HighPass,hdr->CHANNEL[k].LowPass);
		     	memcpy(Header2+ 80*k + 136*hdr->NS,tmp,min(80,len));
		     	
			len = sprintf(tmp,"%i",hdr->CHANNEL[k].SPR);
			if (len>8) fprintf(stderr,"Warning: SPR (%s) of channel %i is to long.\n",tmp,k);  
		     	memcpy(Header2+ 8*k + 216*hdr->NS,tmp,min(8,len));
		     	hdr->CHANNEL[k].GDFTYP = ( (hdr->TYPE != BDF) ? 3 : 255+24);
		}
		hdr->AS.Header1 = (uint8_t*)Header1; 
	}
    	else if (hdr->TYPE==SCP_ECG) {	
    		hdr->FileName = FileName;
//		hdr->AS.Header1 = Header1; 
    		hdr = sopen_SCP_write(hdr);
	}
    	else if (hdr->TYPE==HL7aECG) {	
    		hdr = sopen_HL7aECG_write(hdr);
	}
	else {
    	 	fprintf(stderr,"ERROR: Writing of format (%c) not supported\n",hdr->TYPE);
		return(NULL); 
	}

	hdr->FILE.FID = fopen(FileName,"wb");
	if (hdr->FILE.FID == NULL) 
	{ 	fprintf(stderr,"ERROR: Unable to open file %s \n",FileName);
		return(NULL);
    	}	    
    	fwrite(hdr->AS.Header1,sizeof(char),hdr->HeadLen,hdr->FILE.FID);
	hdr->FILE.OPEN = 2; 	     	
	hdr->FILE.POS  = 0;

}	// end of else 

	// internal variables
	hdr->AS.bi = (uint32_t*) realloc(hdr->AS.bi,(hdr->NS+1)*sizeof(uint32_t));
	hdr->AS.bi[0] = 0;
	for (k=0, hdr->SPR = 1, hdr->AS.spb=0, hdr->AS.bpb=0; k<hdr->NS;) {
		hdr->AS.spb += hdr->CHANNEL[k].SPR;
		hdr->AS.bpb += GDFTYP_BYTE[hdr->CHANNEL[k].GDFTYP]*hdr->CHANNEL[k].SPR;			
		hdr->SPR = lcm(hdr->SPR,hdr->CHANNEL[k].SPR);
		hdr->AS.bi[++k] = hdr->AS.bpb; 
	}	

	return(hdr);
}  // end of SOPEN 


/****************************************************************************/
/**	SREAD                                                              **/
/****************************************************************************/
size_t sread(HDRTYPE* hdr, size_t start, size_t length) {
/* 
 *	reads LENGTH blocks with HDR.AS.bpb BYTES each, 
 *	rawdata is available in hdr->AS.rawdata
 *      output data is available in hdr->data.block
 *	size of output data is availabe in hdr->data.size
 *
 *	channel selection is controlled by hdr->CHANNEL[k}.OnOff
 * 
 *        start <0: read from current position
 *             >=0: start reading from position start
 *        length  : try to read length blocks
 *
 */

	size_t			count,k1,k2,k3,k4,k5,DIV,SZ,nelem; 
	int 			GDFTYP;
	uint8_t*		ptr;
	CHANNEL_TYPE*		CHptr;
	int32_t			int32_value;
	biosig_data_type 	sample_value; 
	

	// check reading segment 
	if (start >= 0) {
		if (start > hdr->NRec)
			return(0);
		else if (fseek(hdr->FILE.FID, start*hdr->AS.bpb + hdr->HeadLen, SEEK_SET))
			return(0);
		hdr->FILE.POS = start; 	
	}

	if (hdr->TYPE != SCP_ECG) {	
		// allocate AS.rawdata 	
		hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata, (hdr->AS.bpb)*length);

		// limit reading to end of data block
		nelem = max(min(length, hdr->NRec - hdr->FILE.POS),0);

		// read data	
		count = fread(hdr->AS.rawdata, hdr->AS.bpb, nelem, hdr->FILE.FID);
		if (count<nelem)
			fprintf(stderr,"warning: only %i instead of %i blocks read - something went wrong\n",count,nelem,hdr->FILE.POS,hdr->AS.bpb); 
	}
	else 	{  // SCP format 
		// hdr->AS.rawdata was defined in SOPEN	
		count = hdr->NRec;
	}
	
	// set position of file handle 
	hdr->FILE.POS += count;

	// transfer RAW into BIOSIG data format 
	hdr->data.block   = (biosig_data_type*) realloc(hdr->data.block, (hdr->SPR) * count * (hdr->NS) * sizeof(biosig_data_type));

	for (k1=0,k2=0; k1<hdr->NS; k1++) {
		CHptr 	= hdr->CHANNEL+k1;
	if (1) //(CHptr->OnOff != 0) 
	{
		DIV 	= hdr->SPR/CHptr->SPR; 
		GDFTYP 	= CHptr->GDFTYP;
		SZ  	= GDFTYP_BYTE[GDFTYP];
		int32_value = 0; 



		for (k4 = 0; k4 < count; k4++)
		for (k5 = 0; k5 < CHptr->SPR; k5++) {



			// get source address 	
			ptr = hdr->AS.rawdata + k4*hdr->AS.bpb + hdr->AS.bi[k1] + k5*SZ;
			
			// mapping of raw data type to (biosig_data_type)
			if (0); 
			else if (GDFTYP==3)
				sample_value = (biosig_data_type)l_endian_i16(*(int16_t*)ptr); 
			else if (GDFTYP==4)
				sample_value = (biosig_data_type)l_endian_u16(*(uint16_t*)ptr); 
			else if (GDFTYP==16) 
				sample_value = (biosig_data_type)(l_endian_f32(*(float*)(ptr)));
			else if (GDFTYP==17) 
				sample_value = (biosig_data_type)(l_endian_f64(*(double*)(ptr))); 
			else if (GDFTYP==0)
				sample_value = (biosig_data_type)(*(char*)ptr); 
			else if (GDFTYP==1)
				sample_value = (biosig_data_type)(*(int8_t*)ptr); 
			else if (GDFTYP==2)
				sample_value = (biosig_data_type)(*(uint8_t*)ptr); 
			else if (GDFTYP==5)
				sample_value = (biosig_data_type)l_endian_i32(*(int32_t*)ptr); 
			else if (GDFTYP==6)
				sample_value = (biosig_data_type)l_endian_u32(*(uint32_t*)ptr); 
			else if (GDFTYP==7)
				sample_value = (biosig_data_type)l_endian_i64(*(int64_t*)ptr); 
			else if (GDFTYP==8)
				sample_value = (biosig_data_type)l_endian_u64(*(uint64_t*)ptr); 
			else if (GDFTYP==255+24) {
				int32_value = (*(uint8_t*)(ptr)) + (*(uint8_t*)(ptr+1)<<8) + (*(int8_t*)(ptr+2)*(1<<16)); 
				sample_value = (biosig_data_type)int32_value; 
			}	
			else if (GDFTYP==511+24) {
				int32_value = (*(uint8_t*)(ptr)) + (*(uint8_t*)(ptr+1)<<8) + (*(uint8_t*)(ptr+2)<<16); 
				sample_value = (biosig_data_type)int32_value; 
			}	
			else {
				fprintf(stderr,"Error SREAD: datatype %i not supported\n",GDFTYP);
				exit(-1);
			}

			// overflow and saturation detection 
			if ((hdr->FLAG.OVERFLOWDETECTION) && ((sample_value<=hdr->CHANNEL[k1].DigMin) || (sample_value>=hdr->CHANNEL[k1].DigMax)))
				sample_value = NaN; 	// missing value 
			else if (!hdr->FLAG.UCAL)	// scaling 
				sample_value = sample_value * CHptr->Cal + CHptr->Off;
			// resampling 1->DIV samples
			for (k3=0; k3 < DIV; k3++) 
				hdr->data.block[k2*count*hdr->SPR + k4*CHptr->SPR + k5 + k3] = sample_value; 
		}
		k2++;
	}}
	hdr->data.size[0] = hdr->SPR*count;	// rows	
	hdr->data.size[1] = k2;			// columns 

	return(count);

}  // end of SREAD 



/****************************************************************************/
/**	SREAD2                                                             **/
/****************************************************************************/
size_t sread2(biosig_data_type** channels_dest, size_t start, size_t length, HDRTYPE* hdr) {
/* 
 *      no memory allocation is done 
 *      data is moved into channel_dest
 *	size of output data is availabe in hdr->data.size
 *
 *	channel selection is controlled by hdr->CHANNEL[k}.OnOff
 * 
 *      start <0: read from current position
 *           >=0: start reading from position start
 *      length  : try to read length blocks
 *
 */

	size_t			count,k1,k2,k3,k4,k5,DIV,SZ,nelem; 
	int 			GDFTYP;
	uint8_t*		ptr;
	CHANNEL_TYPE*		CHptr;
	int32_t			int32_value;
	biosig_data_type 	sample_value; 
	

	// check reading segment 
	if (start >= 0) {
		if (start > hdr->NRec)
			return(0);
		else if (fseek(hdr->FILE.FID, start*hdr->AS.bpb + hdr->HeadLen, SEEK_SET))
			return(0);
		hdr->FILE.POS = start; 	
	}

	if (hdr->TYPE != SCP_ECG) {	
		// allocate AS.rawdata 	
		hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata, (hdr->AS.bpb)*length);

		// limit reading to end of data block
		nelem = max(min(length, hdr->NRec - hdr->FILE.POS),0);

		// read data	
		count = fread(hdr->AS.rawdata, hdr->AS.bpb, nelem, hdr->FILE.FID);
		if (count<nelem)
			fprintf(stderr,"warning: only %i instead of %i blocks read - something went wrong\n",count,nelem,hdr->FILE.POS,hdr->AS.bpb); 
	}
	else 	{  // SCP format 
		// hdr->AS.rawdata was defined in SOPEN	
		count = hdr->NRec;
	}
	
	// allocate AS.rawdata 	
	hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata, (hdr->AS.bpb)*length);

	// limit reading to end of data block
	nelem = max(min(length, hdr->NRec - hdr->FILE.POS),0);

	// read data	
	count = fread(hdr->AS.rawdata, hdr->AS.bpb, nelem, hdr->FILE.FID);

	// set position of file handle 
	hdr->FILE.POS += count;

	// transfer RAW into BIOSIG data format 
	// hdr->data.block   = realloc(hdr->data.block, (hdr->SPR) * count * (hdr->NS) * sizeof(biosig_data_type));

	for (k1=0,k2=0; k1<hdr->NS; k1++) {
	CHptr 	= hdr->CHANNEL+k1;
	if (CHptr->OnOff != 0) {
		DIV 	= hdr->SPR/CHptr->SPR; 
		GDFTYP 	= CHptr->GDFTYP;
		SZ  	= GDFTYP_BYTE[GDFTYP];
		int32_value = 0; 

		for (k4 = 0; k4 < count; k4++)
		for (k5 = 0; k5 < CHptr->SPR; k5++) {

			// get source address 	
			ptr = hdr->AS.rawdata + k4*hdr->AS.bpb + hdr->AS.bi[k1] + k5*SZ;
			
			// mapping of raw data type to (biosig_data_type)
			if (0); 
			else if (GDFTYP==3)
				sample_value = (biosig_data_type)l_endian_i16(*(int16_t*)ptr); 
			else if (GDFTYP==4)
				sample_value = (biosig_data_type)l_endian_u16(*(uint16_t*)ptr); 
			else if (GDFTYP==16) 
				sample_value = (biosig_data_type)(l_endian_f32(*(float*)(ptr)));
			else if (GDFTYP==17) 
				sample_value = (biosig_data_type)(l_endian_f64(*(double*)(ptr))); 
			else if (GDFTYP==0)
				sample_value = (biosig_data_type)(*(char*)ptr); 
			else if (GDFTYP==1)
				sample_value = (biosig_data_type)(*(int8_t*)ptr); 
			else if (GDFTYP==2)
				sample_value = (biosig_data_type)(*(uint8_t*)ptr); 
			else if (GDFTYP==5)
				sample_value = (biosig_data_type)l_endian_i32(*(int32_t*)ptr); 
			else if (GDFTYP==6)
				sample_value = (biosig_data_type)l_endian_u32(*(uint32_t*)ptr); 
			else if (GDFTYP==7)
				sample_value = (biosig_data_type)l_endian_i64(*(int64_t*)ptr); 
			else if (GDFTYP==8)
				sample_value = (biosig_data_type)l_endian_u64(*(uint64_t*)ptr); 
			else if (GDFTYP==255+24) {
				int32_value = (*ptr) + (*(ptr+1)<<8) + (*(ptr+2)*(1<<16)); 
				sample_value = (biosig_data_type)int32_value; 
			}	
			else if (GDFTYP==511+24) {
				int32_value = (*ptr) + (*(ptr+1)<<8) + (*(ptr+2)<<16); 
				sample_value = (biosig_data_type)int32_value; 
			}	
			else {
				fprintf(stderr,"Error SREAD: datatype %i not supported\n",GDFTYP);
				exit(-1);
			}

			// overflow and saturation detection 
			if ((hdr->FLAG.OVERFLOWDETECTION) && ((sample_value<=hdr->CHANNEL[k1].DigMin) || (sample_value>=hdr->CHANNEL[k1].DigMax)))
				sample_value = NaN; 	// missing value 

			else if (!(hdr->FLAG.UCAL))	// scaling 
				sample_value = sample_value * CHptr->Cal + CHptr->Off;

			// resampling 1->DIV samples
			for (k3=0; k3 < DIV; k3++) 
				*(channels_dest[k2] + k4*CHptr->SPR + k5 + k3) = sample_value; 
				//hdr->data.block[k2*count*hdr->SPR + k4*CHptr->SPR + k5 + k3] = sample_value; 
		}
		k2++;
	}}
	hdr->data.size[0] = hdr->SPR*count;	// rows	
	hdr->data.size[1] = k2;			// columns 

	return(count);

}  // end of SREAD2 


/****************************************************************************/
/**                     SWRITE                                             **/
/****************************************************************************/
size_t swrite(const void *ptr, size_t nelem, HDRTYPE* hdr) {
/* 
 *	writes NELEM blocks with HDR.AS.bpb BYTES each, 
 */
	size_t count; 


//	fwrite(hdr->data.block, sizeof(biosig_data_type),hdr->NRec*hdr->SPR*hdr->NS, hdr->FILE.FID);

	// write data 
	count = fwrite((uint8_t*)ptr, hdr->AS.bpb, nelem, hdr->FILE.FID);


/*
	for (k1=0,k2=0; k1<hdr->NS; k1++) {
	CHptr 	= hdr->CHANNEL+k1;
	if (CHptr->OnOff != 0) {
		DIV 	= hdr->SPR/CHptr->SPR; 
		GDFTYP 	= CHptr->GDFTYP;
		SZ  	= GDFTYP_BYTE[GDFTYP];
		int32_value = 0; 

		for (k4 = 0; k4 < count; k4++)
		for (k5 = 0; k5 < CHptr->SPR; k5++) {

			// get source address 	
			ptr = hdr->AS.rawdata + k4*hdr->AS.bpb + hdr->AS.bi[k1] + k5*SZ;
			
			// mapping of raw data type to (biosig_data_type)
			if (0); 
			else if (GDFTYP==3)
				sample_value = (biosig_data_type)l_endian_i16(*(int16_t*)ptr); 
			else if (GDFTYP==4)
			else ;
		}
		}
	}
	}	
*/
	
	// set position of file handle 
	(hdr->FILE.POS) += count; 

	return(count);

}  // end of SWRITE 


/****************************************************************************/
/**                     SEOF                                               **/
/****************************************************************************/
int seof(HDRTYPE* hdr)
{
	return(hdr->FILE.POS >= hdr->NRec);
}


/****************************************************************************/
/**                     SREWIND                                            **/
/****************************************************************************/
void srewind(HDRTYPE* hdr)
{
	sseek(hdr,0,SEEK_SET);
	return;
}


/****************************************************************************/
/**                     SSEEK                                              **/
/****************************************************************************/
int sseek(HDRTYPE* hdr, long int offset, int whence)
{
	long int pos=0; 
	
	if    	(whence < 0) 
		pos = offset * hdr->AS.bpb; 
	else if (whence == 0) 
		pos = (hdr->FILE.POS + offset) * hdr->AS.bpb;
	else if (whence > 0) 
		pos = (hdr->NRec + offset) * hdr->AS.bpb;
	
	if ((pos < 0) | (pos > hdr->NRec * hdr->AS.bpb))
		return(-1);
	else if (fseek(hdr->FILE.FID, pos + hdr->HeadLen, SEEK_SET))
		return(-1);

	hdr->FILE.POS = pos / (hdr->AS.bpb); 	
	return(0);
	
}  // end of SSEEK


/****************************************************************************/
/**                     STELL                                              **/
/****************************************************************************/
long int stell(HDRTYPE* hdr)
{
	size_t pos; 
	
	pos = ftell(hdr->FILE.FID);	
	if (pos<0)
		return(-1);
	else if (pos != (hdr->FILE.POS * hdr->AS.bpb + hdr->HeadLen))
		return(-1);
	else 
		return(hdr->FILE.POS);
	
}  // end of STELL


/****************************************************************************/
/**                     SCLOSE                                             **/
/****************************************************************************/
int sclose(HDRTYPE* hdr)
{
	int32_t 	pos, len; 
	uint32_t	k32u; 
	uint8_t 	buf[88]; 
	char tmp[88]; 
	char flag; 
	
	if ((hdr->NRec<0) & (hdr->FILE.OPEN>1))
	if ((hdr->TYPE==GDF) | (hdr->TYPE==EDF) | (hdr->TYPE==BDF))
	{
		// WRITE HDR.NRec 
		pos = (ftell(hdr->FILE.FID)-hdr->HeadLen); 
		if (hdr->NRec<0)
		{	if (pos>0) 	hdr->NRec = pos/hdr->AS.bpb;
			else		hdr->NRec = 0; 	
			fseek(hdr->FILE.FID,236,SEEK_SET); 
			if (hdr->TYPE==GDF) {
				*(uint64_t*)tmp = l_endian_u64(hdr->NRec);
				fwrite(tmp,sizeof(hdr->NRec),1,hdr->FILE.FID);
			}	
			else {
				len = sprintf(tmp,"%Lu",hdr->NRec);
				if (len>8) fprintf(stderr,"Warning: NRec is (%s) to long.\n",tmp);  
				fwrite(tmp,len,1,hdr->FILE.FID);
			}	
		};	
	
		// WRITE EVENTTABLE 
		if ((hdr->TYPE==GDF) & (hdr->EVENT.N>0)) {
			fseek(hdr->FILE.FID, hdr->HeadLen + hdr->AS.bpb*hdr->NRec, SEEK_SET); 
			flag = (hdr->EVENT.DUR != NULL) & (hdr->EVENT.CHN != NULL); 
			if (flag)   // any DUR or CHN is larger than 0 
				for (k32u=0, flag=0; (k32u<hdr->EVENT.N) & !flag; k32u++)
					flag |= hdr->EVENT.CHN[k32u] | hdr->EVENT.DUR[k32u];
			buf[0] = (flag ? 3 : 1);
			if (hdr->VERSION < 1.94) {
				k32u   = lround(hdr->EVENT.SampleRate); 
				buf[1] =  k32u      & 0x000000FF;
	    			buf[2] = (k32u>>8 ) & 0x000000FF;
				buf[3] = (k32u>>16) & 0x000000FF;
				*(uint32_t*)(buf+4) = l_endian_u32(hdr->EVENT.N);
			}
			else {
				k32u   = hdr->EVENT.N; 
				buf[1] =  k32u      & 0x000000FF;
	    			buf[2] = (k32u>>8 ) & 0x000000FF;
				buf[3] = (k32u>>16) & 0x000000FF;
				*(float*)(buf+4) = l_endian_f32(hdr->EVENT.SampleRate);
			};
			
			fwrite(buf, 8, 1, hdr->FILE.FID);
			for (k32u=0; k32u<hdr->EVENT.N; k32u++) {
				hdr->EVENT.POS[k32u] = l_endian_u32(hdr->EVENT.POS[k32u]); 
				hdr->EVENT.TYP[k32u] = l_endian_u16(hdr->EVENT.TYP[k32u]); 
			}
			fwrite(hdr->EVENT.POS, sizeof(*hdr->EVENT.POS), hdr->EVENT.N, hdr->FILE.FID);
			fwrite(hdr->EVENT.TYP, sizeof(*hdr->EVENT.TYP), hdr->EVENT.N, hdr->FILE.FID);
			if (tmp[0]>1) {
				for (k32u=0; k32u<hdr->EVENT.N; k32u++) {
					hdr->EVENT.DUR[k32u] = l_endian_u32(hdr->EVENT.DUR[k32u]); 
					hdr->EVENT.CHN[k32u] = l_endian_u16(hdr->EVENT.CHN[k32u]); 
				}
				fwrite(hdr->EVENT.CHN,sizeof(*hdr->EVENT.CHN),hdr->EVENT.N,hdr->FILE.FID);
				fwrite(hdr->EVENT.DUR,sizeof(*hdr->EVENT.DUR),hdr->EVENT.N,hdr->FILE.FID);
			}	
		}
	}		

	if (hdr->TYPE != XML) {
		fclose(hdr->FILE.FID);
    		hdr->FILE.FID = 0;
    	}	

    	if (hdr->aECG != NULL)	
        	free(hdr->aECG);
    	if (hdr->AS.rawdata != NULL)	
        	free(hdr->AS.rawdata);

    	if (hdr->data.block != NULL) {	
        	free(hdr->data.block);
        	hdr->data.size[0]=0;
        	hdr->data.size[1]=0;
        }	

    	if (hdr->CHANNEL != NULL)	
        	free(hdr->CHANNEL);
    	if (hdr->AS.bi != NULL)	
        	free(hdr->AS.bi);
    	if (hdr->AS.Header1 != NULL)	
        	free(hdr->AS.Header1);

    	if (hdr->EVENT.POS != NULL)	
        	free(hdr->EVENT.POS);
    	if (hdr->EVENT.TYP != NULL)	
        	free(hdr->EVENT.TYP);
    	if (hdr->EVENT.DUR != NULL)	
        	free(hdr->EVENT.DUR);
    	if (hdr->EVENT.CHN != NULL)	
        	free(hdr->EVENT.CHN);
        	
        hdr->EVENT.N   = 0; 
	hdr->FILE.OPEN = 0; 	     	

    	return(0);
}


/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/
// big-endian platforms
