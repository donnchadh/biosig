/*
 
    sandbox is used for development and under constraction work
    The functions here are either under construction or experimental. 
    The functions will be either fixed, then they are moved to another place;
    or the functions are discarded. Do not rely on the interface in this function

    $Id: sandbox.c,v 1.5 2009-04-16 20:19:17 schloegl Exp $
    Copyright (C) 2008,2009 Alois Schloegl <a.schloegl@ieee.org>
    This file is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 

    BioSig is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. 


 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../biosig-dev.h"

// these functios are stubs

#ifdef WITH_DCMTK
#undef WITH_DICOM
#undef WITH_GDCM


extern "C" int sopen_dicom_read(HDRTYPE* hdr) {
	fprintf(stdout,"DCMTK is used to read dicom files.\n");

}

#endif 



#ifdef WITH_GDCM
#undef WITH_DICOM

#include "gdcmReader.h"
//#include "gdcmImageReader.h"
//#include "gdcmWriter.h"
#include "gdcmDataSet.h"
#include "gdcmAttribute.h"

//#include "gdcmCommon.h"
#include "gdcmPreamble.h"
#include "gdcmFile.h"
#include "gdcmFileMetaInformation.h"
#include "gdcmReader.h"
#include "gdcmImageReader.h"
#include "gdcmWriter.h"
#include "gdcmDataSet.h"
#include <gdcmAttribute.h>
#include <gdcmWaveform.h>

extern "C" int sopen_dicom_read(HDRTYPE* hdr) {

	fprintf(stdout,"GDCM is used to read dicom files.\n");

	gdcm::Reader r;
        const gdcm::DataElement *de;
	r.SetFileName( hdr->FileName );
	if( !r.Read() )
		return 1;

	gdcm::File &file = r.GetFile(); 
	gdcm::FileMetaInformation &header = file.GetHeader();
	if ( header.FindDataElement( gdcm::Tag(0x0002, 0x0013 ) ) )
		const gdcm::DataElement &de = header.GetDataElement( gdcm::Tag(0x0002, 0x0013) );

	gdcm::DataSet &ds = file.GetDataSet();

	if ( header.FindDataElement( gdcm::Tag(0x0002, 0x0010 ) ) )
		de = &header.GetDataElement( gdcm::Tag(0x0002, 0x0010) );


fprintf(stdout,"attr <0x0002,0x0010> len=%i\n",de->GetByteValue() );


/*
	{
		gdcm::Attribute<0x28,0x100> at;
		at.SetFromDataElement( ds.GetDataElement( at.GetTag() ) );
		if( at.GetValue() != 8 ) 
			return 1;
		//at.SetValue( 32 );
		//ds.Replace( at.GetAsDataElement() );
	}
	{

fprintf(stdout,"attr <0x0008,0x002a>\n");
		gdcm::Attribute<0x0008,0x002a> at;
fprintf(stdout,"attr <0x0008,0x002a>\n");
 		ds.GetDataElement( at.GetTag() );
fprintf(stdout,"attr <0x0008,0x002a>\n");
		at.SetFromDataElement( ds.GetDataElement( at.GetTag() ) );

		fprintf(stdout,"DCM: [0008,002a]: %i %p\n",at.GetNumberOfValues(), at.GetValue());
	}
*/
	{

fprintf(stdout,"attr <0x0008,0x0023>\n");
		gdcm::Attribute<0x0008,0x0023> at;
fprintf(stdout,"attr <0x0008,0x0023>\n");
 		ds.GetDataElement( at.GetTag() );
fprintf(stdout,"attr <0x0008,0x0023>\n");
//		at.SetFromDataElement( ds.GetDataElement( at.GetTag() ) );

//		fprintf(stdout,"DCM: [0008,0023]: %i %p\n",at.GetNumberOfValues(), at.GetValue());
	}


/*
				{
				struct tm t0; 
				hdr->AS.Header[pos+14]=0;
				t0.tm_sec = atoi((char*)hdr->AS.Header+pos+12);
				hdr->AS.Header[pos+12]=0;
				t0.tm_min = atoi((char*)hdr->AS.Header+pos+10);
				hdr->AS.Header[pos+10]=0;
				t0.tm_hour = atoi((char*)hdr->AS.Header+pos+8);
				hdr->AS.Header[pos+8]=0;
				t0.tm_mday = atoi((char*)hdr->AS.Header+pos+6);
				hdr->AS.Header[pos+6]=0;
				t0.tm_mon = atoi((char*)hdr->AS.Header+pos+4)-1;
				hdr->AS.Header[pos+4]=0;
				t0.tm_year = atoi((char*)hdr->AS.Header+pos)-1900;

				hdr->T0 = tm_time2gdf_time(&t0); 
				break;
				}
			
*/
}
#endif



#ifdef __cplusplus
extern "C" {
#endif 


int sopen_eeprobe(HDRTYPE* hdr) {
	B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	B4C_ERRMSG = "asn1 currently not supported";
	return(0);
};


#define ITX_MAXLINELENGTH 400

size_t NumSegments = 0, NSEGS=0;
struct SegElem_t {
	size_t group;	// 0: undefined 
	size_t series;	// 0: undefined
	size_t sweep;	// 0: undefined
	size_t chan;	// 0: undefined
	size_t SPR;
	double Fs;
	double Cal;
	double Off;
	char   Label[ITX_MAXLINELENGTH+1];
} *SegElem;


void ReadPatchMasterTree(char *mem) {
	char SWAP = (*(uint32_t*)mem == 0x54726565) ^ (__BYTE_ORDER == __BIG_ENDIAN);
	uint32_t N = *(uint32_t*)(mem+4);
	if (SWAP) N = bswap_32(N);
	uint32_t M[9];
	for (int k=0; k<N; k++) {
		M[k] = *(uint32_t*)(mem + 8 + 4*k);
		if (SWAP) M[k] = bswap_32(M[k]);
	}
}

char *IgorChanLabel(char *inLabel, HDRTYPE *hdr, size_t *ngroup, size_t *nseries, size_t *nsweep, size_t *ns) {
	/*
		extract Channel Label of IGOR ITX data format 
	*/
	
	*ns = 0; 
	static char Label[ITX_MAXLINELENGTH+1];
	int k, s = 0, pos4=0, pos1=0;
	for (k = strlen(inLabel); inLabel[k] < ' '; k--);
	inLabel[k+1] = 0;
	
	while (inLabel[k] >= ' ') {
		while ( inLabel[k] >= '0' && inLabel[k] <= '9' ) 
			k--;
		if (inLabel[k]=='_') {
			s++;
			if (s==1) pos4 = k;
			if (s==4) pos1 = k;
			k--;
		}
		if ( inLabel[k] < '0' || inLabel[k] > '9' ) 
			break;
	}

	if (3 < s) {	
		char nvar = 0;
		for (k = strlen(inLabel); 0 < k && nvar < 4; k--) {
			if (inLabel[k] == '_') {
				inLabel[k] = 0;
				char  *v = inLabel+k+1;
				size_t n = atol(v);
				
				switch (nvar) {
				case 0: *ns = n;
					nvar++;
					break;
				case 1: *nsweep = n;
					nvar++;
					break;
				case 2: *nseries = n;
					nvar++;
					break;
				case 3: *ngroup = n;
					nvar++;
					break;
				}
				inLabel[k] = 0;
			}
		}
		for (k=1; inLabel[pos4+k-1]; k++) {
			inLabel[pos1+k] = inLabel[pos4+k]; 
		}
	}

	for (k=0; k<hdr->NS; k++) {

	}

	if ((*ns)+1 > hdr->NS) {	// another channel 
		hdr->NS = (*ns)+1;
		hdr->CHANNEL = (CHANNEL_TYPE*)realloc(hdr->CHANNEL, hdr->NS * sizeof(CHANNEL_TYPE));
	}

	return(inLabel);
}

int sopen_zzztest(HDRTYPE* hdr) {
	size_t count = hdr->HeadLen;

	if (0) {

	}
	else if (hdr->TYPE==HEKA && hdr->VERSION == 2) {
		fprintf(stdout,"Warning: support for HEKA-PatchMaster is very experimental\n");

    		// HEKA PatchMaster file format

if (VERBOSE_LEVEL>7) fprintf(stdout,"HEKA 111\n");

		if (hdr->HeadLen < 256 ) {	
			count = hdr->HeadLen; 
			hdr->AS.Header = (uint8_t*) realloc(hdr->AS.Header, 257);
			hdr->HeadLen += ifread(hdr->AS.Header+count, 1, 256-hdr->HeadLen, hdr);
			hdr->AS.Header[hdr->HeadLen] = 0;
		}

if (VERBOSE_LEVEL>7) fprintf(stdout,"HEKA 114\n");

		hdr->FILE.LittleEndian = *(uint8_t*)(hdr->AS.Header+52) > 0;
		char SWAP = (hdr->FILE.LittleEndian && (__BYTE_ORDER == __BIG_ENDIAN)) ||	  \
			    (!hdr->FILE.LittleEndian && (__BYTE_ORDER == __LITTLE_ENDIAN));

		double oTime;
		uint32_t nItems; 
		if (hdr->FILE.LittleEndian) {
			oTime  = lef64p(hdr->AS.Header+40);
			nItems = leu32p(hdr->AS.Header+48);
		}
		else {
			oTime  = bef64p(hdr->AS.Header+40);
			nItems = beu32p(hdr->AS.Header+48);
		}

if (VERBOSE_LEVEL>7) fprintf(stdout,"HEKA 117\n");

		char magic[4];
		int32_t Levels;
		int32_t *Sizes=NULL; 
		uint32_t StartOfData; 
if (VERBOSE_LEVEL>7) fprintf(stdout,"HEKA 121 nItems=%i\n",nItems);

		if (hdr->VERSION == 2)
		for (int k=0; k < min(12,nItems); k++) {

if (VERBOSE_LEVEL>7) fprintf(stdout,"HEKA 131 nItems=%i\n",k);

			uint32_t start  = *(uint32_t*)(hdr->AS.Header+k*16+64);
			uint32_t length = *(uint32_t*)(hdr->AS.Header+k*16+64+4);
			if (SWAP) {
				start = bswap_32(start);
				length = bswap_32(length);
			}
			uint8_t *ext = hdr->AS.Header + k*16 + 64 + 8;

if (VERBOSE_LEVEL>7) fprintf(stdout,"HEKA #%i: <%s> [%i:+%i]\n",k,ext,start,length);

			if (!start) break; 

			if (!memcmp(ext,".pul\0\0\0\0",8)) {
				// find pulse data
				uint16_t gdftyp = 3;
				ifseek(hdr,start,SEEK_SET);

				ifread(magic, 1, 4, hdr);
				ifread(&Levels, 1, 4, hdr);
				if (SWAP) Levels = bswap_32(Levels);
if (VERBOSE_LEVEL>7) fprintf(stdout,"HEKA123 #%i    Levels=%i\n",k,Levels);
				Sizes = (int32_t*)realloc(Sizes, Levels*4);
if (VERBOSE_LEVEL>7) fprintf(stdout,"HEKA124 #%i    Levels=%i\n",k,Levels);
				ifread(Sizes, Levels, 4, hdr);	
if (VERBOSE_LEVEL>7) fprintf(stdout,"HEKA125 #%i    Levels=%i\n",k,Levels);
				if (SWAP) for (int l=0; l < Levels; l++) Sizes[l] = bswap_32(Sizes[l]);
if (VERBOSE_LEVEL>7) for (int l=0; l < Levels; l++) fprintf(stdout,"HEKA #%i       %i\n",l, Sizes[l]);
				size_t Position = iftell(hdr);	

				hdr->AS.rawdata = (uint8_t*)realloc(hdr->AS.rawdata,length);
				ifread(hdr->AS.rawdata, 1, length, hdr);

			}

			else if (!memcmp(ext,".dat\0\0\0\0",8)) {
				StartOfData = start;												
			}
		}

if (VERBOSE_LEVEL>7) fprintf(stdout,"HEKA 989\n");

		if (!Sizes) free(Sizes); Sizes=NULL;
		ifseek(hdr, StartOfData, SEEK_SET);

if (VERBOSE_LEVEL>7) fprintf(stdout,"HEKA 999\n");

	}

	else if (hdr->TYPE==ITX) {

		fprintf(stdout,"Warning: support for ITX is very experimental\n");

/*
		while (~ifeof(hdr)) {	
			hdr->Header = realloc(hdr->Header,count*2+1);
			count += ifread(hdr->Header+count,1,count,hdr);
		}
		hdr->Header[count]=0;
*/
    		char line[ITX_MAXLINELENGTH+1];
    		char flag = 0;
		double *data = NULL; 		

	    	size_t ns=0, NS=0, spr = 0, SPR = 0, DIV = 1, k; 
		size_t ngroup=0, nseries=0, nsweep=0, NSWEEP=0;
		double DUR[32];
		double deltaT = -1.0/0.0;
		hdr->SPR = 0;

		ifseek(hdr,0,SEEK_SET);	    			
		while (!ifeof(hdr)) {
		    	ifgets(line, ITX_MAXLINELENGTH, hdr);
			if (!strlen(line))
				;
			else if (!strncmp(line,"BEGIN",5)) {
				flag = 1;
				spr = 0;
		}	
		    	else if (!strncmp(line,"END",3)) {
				flag = 0;
				hdr->CHANNEL[ns].SPR += spr;
		    	}
		    	else if (!strncmp(line,"X SetScale/P x,",15)) {
				strtok(line,",");
				strtok(NULL,",");
		    		double dur = atof(strtok(NULL,","));
		    		char *p = strchr(line,'"');
				if (p != NULL) {
					p++;
		    			char *p2 = strchr(p,'"');
		    			if (p2!=NULL) *p2=0;
					dur *= PhysDimScale(PhysDimCode(p));
				}
				if (deltaT < 0.0) {
					deltaT = dur; 
		    			hdr->SampleRate = 1.0 / dur;
				}
				else if (deltaT != dur) {
					B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
					B4C_ERRMSG = "different sampling rates not supported for ITX format";
				}	
		    	}
	    		else if (!strncmp(line,"X SetScale y,",13)) {
	    			char *p = strchr(line,'"');
		    		if (p!=NULL) {
		    			p++;
	    				char *p2 = strchr(p,'"');
	    				if (p2!=NULL) *p2=0;
	    				hdr->CHANNEL[ns].PhysDimCode = PhysDimCode(p);
		    		}
		    		ns++;
	    		}
		    	else if (!strncmp(line,"WAVES",5)) {
		    		NumSegments++;
	    			if (NumSegments >= NSEGS) {
	    				NSEGS = max(NSEGS*2,16);
	    				SegElem = (SegElem_t*)realloc(SegElem,NSEGS*sizeof(SegElem_t));
	    			}
	    			
				IgorChanLabel(line+6, hdr, &ngroup, &nseries, &nsweep, &ns); 	// terminating \0
				CHANNEL_TYPE *hc = hdr->CHANNEL+ns;			
			
				hc->OnOff    = 1;
				hc->SPR      = 0;
        			hc->GDFTYP   = 17;
        			hc->DigMax   = (double)(int16_t)(0x7fff);
        			hc->DigMin   = (double)(int16_t)(0x8000);

				hc->Cal      = 1.0; 
				hc->Off      = 0.0;
				hc->Transducer[0] = '\0';
				hc->LowPass  = NaN;
				hc->HighPass = NaN;
				hc->PhysMax  = hc->Cal * hc->DigMax;
				hc->PhysMin  = hc->Cal * hc->DigMin;
	    		}
	    		else if (flag) 
	    			spr++;
	    	}		
		hdr->NS = ns;

		hdr->SPR = 0;
		for (ns=0; ns<NS; ns++) {
			hdr->CHANNEL[ns].bi  = SPR*sizeof(double); 
			hdr->CHANNEL[ns].bi8 = SPR*sizeof(double)*8; 
			SPR += hdr->CHANNEL[ns].SPR; 
			if (hdr->SPR < hdr->CHANNEL[ns].SPR) 
				hdr->SPR = hdr->CHANNEL[ns].SPR;
			hdr->CHANNEL[ns].SPR = 0;	// reset
		}	
		
		data = (double*)realloc(hdr->AS.rawdata, SPR*sizeof(double));
		hdr->FILE.LittleEndian = (__BYTE_ORDER == __LITTLE_ENDIAN);   // no swapping 
		hdr->AS.rawdata = (uint8_t*) data;
	    	
    		ifseek(hdr, 0, SEEK_SET);
    		while (!ifeof(hdr)) {
	    		ifgets(line, ITX_MAXLINELENGTH, hdr);
	    		if (!strncmp(line,"BEGIN",5)) {
	    			flag = 1;
	    			spr = 0;
	    			
	    		}
	    		else if (!strncmp(line,"END",3)) {
	    			flag = 0;
	    			hdr->CHANNEL[ns].SPR += spr;
	    		}
	    		else if (!strncmp(line,"WAVES",5)) {
	    			// get current channel
				IgorChanLabel(line+6, hdr, &ngroup, &nseries, &nsweep, &ns); 	// terminating \0
			}	
	    		else if (flag) {
	    			data[hdr->CHANNEL[ns].SPR + hdr->CHANNEL[ns].bi + spr++] = atof(line); 
	    		}
	    	}
		hdr->NRec = 1;
		hdr->AS.first  = 0;
		hdr->AS.length = 1;
	}

	return(0);
};

int sopen_unipro_read(HDRTYPE* hdr) {
	B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	B4C_ERRMSG = "UNIPRO not supported";
	return(0);
}


#ifdef WITH_DICOM
int sopen_dicom_read(HDRTYPE* hdr) {

		fprintf(stdout,"home-made parser is used to read dicom files.\n");

		char FLAG_implicite_VR = 0;	
		int EndOfGroup2=-1; 

		if (hdr->HeadLen<132) {
			hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, 132);
		    	hdr->HeadLen += ifread(hdr->AS.Header+hdr->HeadLen, 1, 132-hdr->HeadLen, hdr);
		}
		size_t count = hdr->HeadLen;
		size_t pos = 128; 
		while (!hdr->AS.Header[pos] && (pos<128)) pos++;
		if ((pos==128) && !memcmp(hdr->AS.Header+128,"DICM",4)) {
//			FLAG_implicite_VR = 0;	
			pos = 132;
		}	
		else
			pos = 0;

		size_t bufsiz = 16384;
		while (!ifeof(hdr)) {
			hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, count+bufsiz+1);
		    	count += ifread(hdr->AS.Header+count, 1, bufsiz, hdr);
		    	bufsiz *= 2;
		}
	    	ifclose(hdr); 
	    	hdr->AS.Header[count] = 0;

		uint16_t nextTag[2];

		struct tm T0; 
		char flag_t0=0; 
		char flag_ignored; 
		uint32_t Tag;
		uint32_t Len;
		nextTag[0] = *(uint16_t*)(hdr->AS.Header+pos);
		nextTag[1] = *(uint16_t*)(hdr->AS.Header+pos+2);
		while (pos < count) {	

			if ((__BYTE_ORDER == __BIG_ENDIAN) ^ !hdr->FILE.LittleEndian) {
				// swapping required  
				Tag  = (((uint32_t)bswap_16(nextTag[0])) << 16) + bswap_16(nextTag[1]);
				pos += 4;
				if (FLAG_implicite_VR) {
					Len = bswap_32(*(uint32_t*)(hdr->AS.Header+pos));
					pos += 4; 
				}	
				else {
					// explicite_VR
					if (pos+4 > count) break;
					
					if (memcmp(hdr->AS.Header+pos,"OB",2) 
					 && memcmp(hdr->AS.Header+pos,"OW",2)
					 && memcmp(hdr->AS.Header+pos,"OF",2)
					 && memcmp(hdr->AS.Header+pos,"SQ",2)
					 && memcmp(hdr->AS.Header+pos,"UT",2)
					 && memcmp(hdr->AS.Header+pos,"UN",2) ) { 
						Len = bswap_16(*(uint16_t*)(hdr->AS.Header+pos+2));
						pos += 4; 
					}	
					else {	
						Len = bswap_32(*(uint32_t*)(hdr->AS.Header+pos+4));
						pos += 8; 
					}
				}	
			}
			else {
				// no swapping 
				Tag  = (((uint32_t)nextTag[0]) << 16) + nextTag[1];
				pos += 4; 
				if (FLAG_implicite_VR) {
					Len = *(uint32_t*)(hdr->AS.Header+pos);
					pos += 4; 
				}	
				else {
					// explicite_VR
					if (pos+4 > count) break;
					
					if (memcmp(hdr->AS.Header+pos,"OB",2) 
					 && memcmp(hdr->AS.Header+pos,"OW",2)
					 && memcmp(hdr->AS.Header+pos,"OF",2)
					 && memcmp(hdr->AS.Header+pos,"SQ",2)
					 && memcmp(hdr->AS.Header+pos,"UT",2)
					 && memcmp(hdr->AS.Header+pos,"UN",2) ) { 
						Len = *(uint16_t*)(hdr->AS.Header+pos+2);
						pos += 4; 
					}	
					else {	
						Len = *(uint32_t*)(hdr->AS.Header+pos+4);
						pos += 8; 
					}
				}	
			}

			/*
			    backup next tag, this allows setting of terminating 0
			*/
			if (pos+Len < count) {
				nextTag[0] = *(uint16_t*)(hdr->AS.Header+pos+Len);
				nextTag[1] = *(uint16_t*)(hdr->AS.Header+pos+Len+2);
				hdr->AS.Header[pos+Len] = 0;
	    		}	
			

			flag_ignored = 0;
			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"        %6x:   (%04x,%04x) %8d\t%s\n",pos,Tag>>16,Tag&0x0ffff,Len,(char*)hdr->AS.Header+pos);			
				
			switch (Tag) {


			/* elements of group 0x0002 use always 
				Explicite VR little Endian encoding 	
			*/
			case 0x00020000: {
				int c = 0; 
				if (!memcmp(hdr->AS.Header+pos-8,"UL",2))
					c = leu32p(hdr->AS.Header+pos);
				else if (!memcmp(hdr->AS.Header+pos-8,"SL",2))
					c = lei32p(hdr->AS.Header+pos);
				else if (!memcmp(hdr->AS.Header+pos-8,"US",2))
					c = leu16p(hdr->AS.Header+pos);
				else if (!memcmp(hdr->AS.Header+pos-8,"SS",2))
					c = lei16p(hdr->AS.Header+pos);
				else  {
					B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
					B4C_ERRMSG = "DICOM (0002,0000): unsupported";
				}
				EndOfGroup2 = c + pos; 
				break;
				}
			case 0x00020001:
				break;

			case 0x00020002: {
				hdr->NS = 1; 
				char *t = (char*)hdr->AS.Header+pos;
				while (isspace(*t)) t++;	// deblank
				char *ct[] = {	"1.2.840.10008.5.1.4.1.1.9.1.1", 
						"1.2.840.10008.5.1.4.1.1.9.1.2",
						"1.2.840.10008.5.1.4.1.1.9.1.3",
						"1.2.840.10008.5.1.4.1.1.9.2.1",
						"1.2.840.10008.5.1.4.1.1.9.3.1",
						"1.2.840.10008.5.1.4.1.1.9.4.1"
						};			
				if (!strcmp(t,*ct)) hdr->NS = 12; 
				break;
				}	

			case 0x00020003:
				break;

			case 0x00020010: {
				char *t = (char*)hdr->AS.Header+pos;
				while (isspace(*t)) t++;	// deblank
				char *ct[] = {	"1.2.840.10008.1.2", 
						"1.2.840.10008.1.2.1",
						"1.2.840.10008.1.2.1.99",
						"1.2.840.10008.1.2.2"
						};

				if      (!strcmp(t,*ct))   FLAG_implicite_VR = 1;	
				else if (!strcmp(t,*ct+1)) FLAG_implicite_VR = 0;	
				else if (!strcmp(t,*ct+3)) FLAG_implicite_VR = 1;	
				break;
				}

			case 0x00080020:  // StudyDate 
			case 0x00080023:  // ContentDate 
				{
				hdr->AS.Header[pos+8]=0;
				T0.tm_mday = atoi((char*)hdr->AS.Header+pos+6);
				hdr->AS.Header[pos+6]=0;
				T0.tm_mon = atoi((char*)hdr->AS.Header+pos+4)-1;
				hdr->AS.Header[pos+4]=0;
				T0.tm_year = atoi((char*)hdr->AS.Header+pos)-1900;
				flag_t0 |= 1; 
				break;
				}
			case 0x0008002a:  // AcquisitionDateTime 
				{
				struct tm t0; 
				hdr->AS.Header[pos+14]=0;
				t0.tm_sec = atoi((char*)hdr->AS.Header+pos+12);
				hdr->AS.Header[pos+12]=0;
				t0.tm_min = atoi((char*)hdr->AS.Header+pos+10);
				hdr->AS.Header[pos+10]=0;
				t0.tm_hour = atoi((char*)hdr->AS.Header+pos+8);
				hdr->AS.Header[pos+8]=0;
				t0.tm_mday = atoi((char*)hdr->AS.Header+pos+6);
				hdr->AS.Header[pos+6]=0;
				t0.tm_mon = atoi((char*)hdr->AS.Header+pos+4)-1;
				hdr->AS.Header[pos+4]=0;
				t0.tm_year = atoi((char*)hdr->AS.Header+pos)-1900;

				hdr->T0 = tm_time2gdf_time(&t0); 
				break;
				}
			case 0x00080030:  // StudyTime 
			case 0x00080033:  // ContentTime 
				{
				hdr->AS.Header[pos+6]=0;
				T0.tm_sec = atoi((char*)hdr->AS.Header+pos+4);
				hdr->AS.Header[pos+4]=0;
				T0.tm_min = atoi((char*)hdr->AS.Header+pos+2);
				hdr->AS.Header[pos+2]=0;
				T0.tm_hour = atoi((char*)hdr->AS.Header+pos);
				flag_t0 |= 2; 
				break;
				}
			case 0x00080070:  // Manufacturer 
				{
				strncpy(hdr->ID.Manufacturer._field,(char*)hdr->AS.Header+pos,MAX_LENGTH_MANUF);
				hdr->ID.Manufacturer.Name = hdr->ID.Manufacturer._field;
				break;
				}	
			case 0x00081050:  // Performing Physician
				{
				strncpy(hdr->ID.Technician,(char*)hdr->AS.Header+pos,MAX_LENGTH_TECHNICIAN);
				break;
				}	
			case 0x00081090: // Manufacturer Model
				{
				const size_t nn = strlen(hdr->ID.Manufacturer.Name)+1;
				strncpy(hdr->ID.Manufacturer._field+nn,(char*)hdr->AS.Header+pos,MAX_LENGTH_MANUF-nn-1);
				hdr->ID.Manufacturer.Model = hdr->ID.Manufacturer._field+nn;
				break;
				}	
				
			case 0x00100010: // Name 
				if (!hdr->FLAG.ANONYMOUS) {
					strncpy(hdr->Patient.Name,(char*)hdr->AS.Header+pos,MAX_LENGTH_NAME);
					hdr->Patient.Name[MAX_LENGTH_NAME]=0;
				}
				break;		
			case 0x00100020: // ID 
				strncpy(hdr->Patient.Id,(char*)hdr->AS.Header+pos,MAX_LENGTH_NAME);
				hdr->Patient.Id[MAX_LENGTH_PID]=0;
				break;
				
			case 0x00100030: // Birthday 
				{
				struct tm t0; 
				t0.tm_sec = 0;
				t0.tm_min = 0;
				t0.tm_hour = 12;

				hdr->AS.Header[pos+8]=0;
				t0.tm_mday = atoi((char*)hdr->AS.Header+pos+6);
				hdr->AS.Header[pos+6]=0;
				t0.tm_mon = atoi((char*)hdr->AS.Header+pos+4)-1;
				hdr->AS.Header[pos+4]=0;
				t0.tm_year = atoi((char*)hdr->AS.Header+pos)-1900;

				hdr->Patient.Birthday = tm_time2gdf_time(&t0); 
				break;
				}
			case 0x00100040: 
				hdr->Patient.Sex = (toupper(hdr->AS.Header[pos])=='M') + 2*(toupper(hdr->AS.Header[pos])=='F');
				break;

			case 0x00101010: //Age 
				break;
			case 0x00101020: 
				hdr->Patient.Height = (uint8_t)(atof((char*)hdr->AS.Header+pos)*100.0);
				break;
			case 0x00101030: 
				hdr->Patient.Weight = (uint8_t)atoi((char*)hdr->AS.Header+pos);
				break;
				
			default:
				flag_ignored = 1;
				if (VERBOSE_LEVEL<7)
					fprintf(stdout,"ignored %6x:   (%04x,%04x) %8d\t%s\n",pos,Tag>>16,Tag&0x0ffff,Len,(char*)hdr->AS.Header+pos);			
			
			}

			if (VERBOSE_LEVEL>6) {
			if (!FLAG_implicite_VR || (Tag < 0x00030000))	
				fprintf(stdout,"%s %6x:   (%04x,%04x) %8d %c%c \t%s\n",(flag_ignored?"ignored":"       "),pos,Tag>>16,Tag&0x0ffff,Len,hdr->AS.Header[pos-8],hdr->AS.Header[pos-7],(char*)hdr->AS.Header+pos);
			else 	
				fprintf(stdout,"%s %6x:   (%04x,%04x) %8d\t%s\n",(flag_ignored?"ignored":"       "),pos,Tag>>16,Tag&0x0ffff,Len,(char*)hdr->AS.Header+pos);
			}					
			pos += Len + (Len & 0x01 ? 1 : 0); // even number of bytes	

		}
		if (flag_t0 == 3) hdr->T0 = tm_time2gdf_time(&T0); 
	return(0);
}
#endif 

#ifdef WITH_PDP
#include "../NONFREE/sopen_pdp_read.c"
#endif 

#ifdef WITH_TRC
#include "../NONFREE/sopen_trc_read.c"
#endif 

#ifdef __cplusplus
}
#endif 

