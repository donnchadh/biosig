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

#ifdef WITH_GDCM
#undef WITH_DICOM

#include "gdcmReader.h"
//#include "gdcmImageReader.h"
//#include "gdcmWriter.h"
#include "gdcmDataSet.h"
#include "gdcmAttribute.h"

extern "C" int sopen_dicom_read(HDRTYPE* hdr) {

	gdcm::Reader r;
  	r.SetFileName( hdr->FileName );
  	if( !r.Read() )   
  		return 1;

  	gdcm::File &file = r.GetFile();
  	gdcm::DataSet& ds = file.GetDataSet();
/*
	{
		gdcm::Attribute<0x28,0x100> at;
		at.SetFromDataElement( ds.GetDataElement( at.GetTag() ) );
		if( at.GetValue() != 8 ) 
			return 1;
		//at.SetValue( 32 );
		//ds.Replace( at.GetAsDataElement() );
	}
*/
	{

fprintf(stdout,"attr <0x0008,0x002a>\n");
		gdcm::Attribute<0x0008,0x002a> at;
 		ds.GetDataElement( at.GetTag() );
//		at.SetFromDataElement( ds.GetDataElement( at.GetTag() ) );

		fprintf(stdout,"DCM: [0008,002a]: %i %p\n",at.GetNumberOfValues(), at.GetValue());
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
	else if (hdr->TYPE==CFS) {

		// DO NOT USE THESE STRUCTS UNLESS YOU ARE SURE THERE ARE NO ALIGNMENT ERRORS 
		struct CFSGeneralHeader_t {
			char 	Marker[8];
			char 	Filename[14];
			uint32_t Filesize;
			char 	Time[8];
			char 	Date[8];
			uint16_t NChannels;
			uint16_t NFileVars;
			uint16_t NDataSectionVars;
			uint16_t SizeHeader;
			uint16_t SizeData;
			uint32_t LastDataHeaderSectionOffset;
			uint16_t NDataSections;
			uint16_t DiskBlockSizeRounding;
			char 	Comment[73];
			uint32_t PointerTableOffset;
			char	reserved[40];
		} *CFSGeneralHeader=NULL;

		struct CFSChannelDev_t {
			char 	Channelname[22];
			char 	YYnits[10];
			char 	XUnits[10];
			uint8_t datatype;
			uint8_t datakind;
			uint16_t ByteSpaceBetweenElements;
			uint16_t NextChannel;
		} *CFSChannelDev=NULL;

		fprintf(stdout,"Warning: support for CFS is very experimental\n");

#define H1LEN (8+14+4+8+8+2+2+2+2+2+4+2+2+74+4+40)
	
		while (!ifeof(hdr)) {	
			hdr->AS.Header = (uint8_t*) realloc(hdr->AS.Header,count*2+1);
			count += ifread(hdr->AS.Header+count,1,count,hdr);
		}
		hdr->AS.Header[count] = 0;
		hdr->FLAG.OVERFLOWDETECTION = 0; 
		
		uint8_t k;
		CFSGeneralHeader = (CFSGeneralHeader_t*)hdr->AS.Header;
		hdr->NS = l_endian_u16(CFSGeneralHeader->NChannels);
		/* General Header */			
		uint32_t filesize = leu32p(hdr->AS.Header+22);
		hdr->NS    = leu16p(hdr->AS.Header+42);	// 6  number of channels 
		uint8_t  n = leu16p(hdr->AS.Header+44);	// 7  number of file variables 
		uint16_t d = leu16p(hdr->AS.Header+46);	// 8  number of data section variables 
		uint16_t FileHeaderSize = leu16p(hdr->AS.Header+48);	// 9  byte size of file header 
		uint16_t DataHeaderSize = leu16p(hdr->AS.Header+50);	// 10 byte size of data section header 
		uint32_t LastDataSectionHeaderOffset = leu32p(hdr->AS.Header+52);	// 11 last data section header offset 
		uint16_t NumberOfDataSections = leu16p(hdr->AS.Header+56);	// 12 last data section header offset

		if (NumberOfDataSections) {
			hdr->EVENT.TYP = (typeof(hdr->EVENT.TYP)) realloc(hdr->EVENT.TYP, (hdr->EVENT.N + NumberOfDataSections - 1) * sizeof(*hdr->EVENT.TYP));
			hdr->EVENT.POS = (typeof(hdr->EVENT.POS)) realloc(hdr->EVENT.POS, (hdr->EVENT.N + NumberOfDataSections - 1) * sizeof(*hdr->EVENT.POS));
			hdr->EVENT.CHN = (typeof(hdr->EVENT.CHN)) realloc(hdr->EVENT.CHN, (hdr->EVENT.N + NumberOfDataSections - 1) * sizeof(*hdr->EVENT.CHN));
			hdr->EVENT.DUR = (typeof(hdr->EVENT.DUR)) realloc(hdr->EVENT.DUR, (hdr->EVENT.N + NumberOfDataSections - 1) * sizeof(*hdr->EVENT.DUR));
		}
		

if (VERBOSE_LEVEL>7) fprintf(stdout,"CFS 131 - %d,%d,%d,0x%x,0x%x,0x%x,%d,0x%x\n",hdr->NS,n,d,FileHeaderSize,DataHeaderSize,LastDataSectionHeaderOffset,NumberOfDataSections,leu32p(hdr->AS.Header+0x86));
		
		/* channel information */
		hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS, sizeof(CHANNEL_TYPE));
#define H2LEN (22+10+10+1+1+2+2)
 		char* H2 = (char*)(hdr->AS.Header + H1LEN);
		double xPhysDimScale[100];		// CFS is limited to 99 channels 
		for (k = 0; k < hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL + k;
			/* 
				1 offset because CFS uses pascal type strings (first byte contains string length)
				in addition, the strings are \0 terminated.
			*/ 
			hc->OnOff = 1; 

			char len = min(21, MAX_LENGTH_LABEL);
			strncpy(hc->Label, H2 + 1 + k*H2LEN, len);
			len = strlen(hc->Label);
			while (isspace(hc->Label[len])) len--;		// remove trailing blanks
			hc->Label[len+1] = 0;

			hc->PhysDimCode  = PhysDimCode (H2 + 22 + 1 + k*H2LEN);
			xPhysDimScale[k] = PhysDimScale(PhysDimCode(H2 + 32 + 1 + k*H2LEN));

			uint8_t gdftyp   = H2[42 + k*H2LEN];
			hc->GDFTYP = gdftyp < 5 ? gdftyp+1 : gdftyp+11;
			if (H2[43 + k * H2LEN]) {
				B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
				B4C_ERRMSG = "(CFS)Subsidiary or Matrix data not supported";
			}
			hc->LowPass  = NaN;
			hc->HighPass = NaN;
			hc->Notch    = NaN;
if (VERBOSE_LEVEL>7) fprintf(stdout,"Channel #%i: [%s](%i/%i) <%s>/<%s> ByteSpace%i,Next#%i\n",k+1, H2 + 1 + k*H2LEN, gdftyp, H2[43], H2 + 23 + k*H2LEN, H2 + 33 + k*H2LEN, leu16p(H2+44+k*H2LEN), leu16p(H2+46+k*H2LEN));
		}

		size_t datapos = H1LEN + H2LEN*hdr->NS;

		/* file variable information */
		// n*36 bytes		
if (VERBOSE_LEVEL>7) fprintf(stdout,"\n******* file variable information *********\n");
		for (k = 0; k < n; k++) {
			int i; double f;
			size_t pos = datapos + k*36;
			uint16_t typ = leu16p(hdr->AS.Header+pos+22);
			uint16_t off = leu16p(hdr->AS.Header+pos+34);

//fprintf(stdout,"\n%3i @0x%6x: <%s>  %i  [%s] %i ",k, pos, hdr->AS.Header+pos+1,typ,hdr->AS.Header+pos+25,off);
			size_t p3 = H1LEN + H2LEN*hdr->NS + (n+d)*36 + off + 42;

			switch (typ) {
			case 0:
			case 1: i = hdr->AS.Header[p3]; break;
			case 2: i = lei16p(hdr->AS.Header+p3); break;
			case 3: i = leu16p(hdr->AS.Header+p3); break;
			case 4: i = lei32p(hdr->AS.Header+p3); break;
			case 5: f = lef32p(hdr->AS.Header+p3); break;
			case 6: f = lef64p(hdr->AS.Header+p3); break;
			}
if (VERBOSE_LEVEL>7) 	{
			if (typ<5) fprintf(stdout," *0x%x = [%d]",p3,i);
			else if (typ<7) fprintf(stdout," *0x%x = [%g]",p3,f);
			else if (typ==7) fprintf(stdout," *0x%x = <%s>",p3,hdr->AS.Header+p3);
			}
		}
		
if (VERBOSE_LEVEL>7) fprintf(stdout,"\n******* DS variable information *********\n");
		datapos = LastDataSectionHeaderOffset; //H1LEN + H2LEN*hdr->NS + n*36;

//		void *VarChanInfoPos = hdr->AS.Header + datapos + 30;  // unused
		char flag_ChanInfoChanged = 0; 
		hdr->NRec = NumberOfDataSections;
		size_t SPR = 0, SZ = 0;
		for (char m = 0; m < NumberOfDataSections; m++) {
			if (!leu32p(hdr->AS.Header+datapos+8)) continue; 	// empty segment 

//			flag_ChanInfoChanged |= memcmp(VarChanInfoPos, hdr->AS.Header + datapos + 30, 24*hdr->NS);

if (VERBOSE_LEVEL>7) fprintf(stdout,"\n******* DATA SECTION --%03i-- %i *********\n",m,flag_ChanInfoChanged);
if (VERBOSE_LEVEL>7) fprintf(stdout,"\n[DS#%3i] 0x%x 0x%x [0x%x 0x%x szChanData=%i] 0x02%x\n", m, FileHeaderSize, datapos, leu32p(hdr->AS.Header+datapos), leu32p(hdr->AS.Header+datapos+4), leu32p(hdr->AS.Header+datapos+8), leu16p(hdr->AS.Header+datapos+12));

			uint32_t sz    = 0;
			uint32_t bpb   = 0, spb = 0, spr = 0;
			hdr->AS.first  = 0; 
			hdr->AS.length = 0; 
			for (k = 0; k < hdr->NS; k++) {
				void *pos = hdr->AS.Header + datapos + 30 + 24 * k;
				
				CHANNEL_TYPE *hc = hdr->CHANNEL + k;

				uint32_t p  = leu32p(pos);
				hc->SPR     = leu32p(pos+4);
				hc->Cal     = lef32p(pos+8);
				hc->Off     = lef32p(pos+12);
				double Xcal = lef32p(pos+16);
				//double Xoff = lef32p(pos+20);// unused
				hc->OnOff   = 1;

if (VERBOSE_LEVEL>7) fprintf(stdout,"CFS 409: %i #%i: SPR=%i=%i=%i  x%f+-%f %i\n",m,k,spr,SPR,hc->SPR,hc->Cal,hc->Off,p);

				double Fs = 1.0 / (xPhysDimScale[k] * Xcal);
				if (!m && !k) {
					hdr->SampleRate = Fs;
				}	
				else if (fabs(hdr->SampleRate - Fs) > 1e-3) {
					B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
					B4C_ERRMSG = "CED/CFS: different sampling rates are not supported";
				}	 

				spr  = hc->SPR;	
				spb += hc->SPR;	
				sz  += hc->SPR * GDFTYP_BITS[hc->GDFTYP] >> 3;
				bpb += GDFTYP_BITS[hc->GDFTYP]>>3;	// per single sample 
				hdr->AS.length += hc->SPR; 
			}

			if (NumberOfDataSections > 1) {
				// hack: copy data into a single block, only if more than one section	
				hdr->AS.rawdata = (uint8_t*)realloc(hdr->AS.rawdata, (hdr->NS * SPR + spb) * sizeof(double));
			
/*
				if (VERBOSE_LEVEL>7) 
				 	fprintf(stdout,"CFS 411: @%p %i @%p\n",hdr->AS.rawdata, (hdr->NS * SPR + spb), srcaddr);						
*/
				hdr->AS.first = 0; 
				for (k = 0; k < hdr->NS; k++) {
					CHANNEL_TYPE *hc = hdr->CHANNEL + k;

					uint32_t memoffset = leu32p(hdr->AS.Header + datapos + 30 + 24 * k);
					uint8_t *srcaddr = hdr->AS.Header+leu32p(hdr->AS.Header+datapos + 4) + memoffset;
				
				if (VERBOSE_LEVEL>7) 
				 	fprintf(stdout,"CFS 412 #%i %i: @%p %i\n", k, hc->SPR, srcaddr, leu32p(hdr->AS.Header+datapos + 4) + leu32p(hdr->AS.Header + datapos + 30 + 24 * k));						

					int16_t szz = (GDFTYP_BITS[hc->GDFTYP]>>3);
					for (int k2 = 0; k2 < hc->SPR; k2++) {
						uint8_t *ptr = srcaddr + k2*szz; 
						double val; 

						switch (hc->GDFTYP) {
						// reorder for performance reasons - more frequent gdftyp's come first	
						case 3:  val = lei16p(ptr); break;
						case 4:  val = leu16p(ptr); break;
						case 16: val = lef32p(ptr); break;
						case 17: val = lef64p(ptr); break;
						case 0:  val = *(   char*) ptr; break;
						case 1:  val = *( int8_t*) ptr; break;
						case 2:  val = *(uint8_t*) ptr; break;
						case 5:  val = lei32p(ptr); break;
						case 6:  val = leu32p(ptr); break;
						case 7:  val = lei64p(ptr); break;
						case 8:  val = leu64p(ptr); break;
						default:
							B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
							B4C_ERRMSG = "CED/CFS: invalid data type";
						}

if (VERBOSE_LEVEL>8) 
 	fprintf(stdout,"CFS read: %2i #%2i:%5i [%i]: %f -> %f  @%p\n",m,k,k2,bpb,SPR,val,val*hc->Cal + hc->Off, hdr->AS.rawdata + ((SPR + k2) * hdr->NS + k) * sizeof(double));						

						*(double*) (hdr->AS.rawdata + k * sizeof(double) + (SPR + k2) * hdr->NS * sizeof(double)) = val * hc->Cal + hc->Off;
					}
//					srcaddr += hdr->CHANNEL[k].SPR * GDFTYP_BITS[hdr->CHANNEL[k].GDFTYP] >> 3;
				}
			}
			else {
				hdr->AS.rawdata = (uint8_t*)realloc(hdr->AS.rawdata,sz);
				memcpy(hdr->AS.rawdata, hdr->AS.Header + leu32p(hdr->AS.Header+datapos + 4), leu32p(hdr->AS.Header+datapos + 8));
				hdr->AS.bpb = sz; 
			}
			if (k>0) {
				hdr->EVENT.TYP[hdr->EVENT.N] = 0x7ffe;
				hdr->EVENT.POS[hdr->EVENT.N] = SPR;
				hdr->EVENT.CHN[hdr->EVENT.N] = 0;
				hdr->EVENT.DUR[hdr->EVENT.N] = 0;
				hdr->EVENT.N++;
			}	
			SPR += spr;	
			SZ  += sz; 

if (VERBOSE_LEVEL>7) fprintf(stdout,"CFS 414: SPR=%i,%i,%i NRec=%i, @%p\n",spr,SPR,hdr->SPR,hdr->NRec, hdr->AS.rawdata);
			
			// for (k = 0; k < d; k++) {
			for (k = 0; k < 0; k++) {
			// read data variables of each block - this currently broken. 	
				//size_t pos = leu16p(hdr->AS.Header + datapos + 30 + hdr->NS * 24 + k * 36 + 34);
				size_t pos = datapos + 30 + hdr->NS * 24;
				int i; double f;
				uint16_t typ = leu16p(hdr->AS.Header + pos + 22 + k*36) ;
				uint16_t off = leu16p(hdr->AS.Header + pos + 34 + k*36);
				uint32_t p3  = pos + off;

if (VERBOSE_LEVEL>7) fprintf(stdout,"\n[DS#%3i/%3i] @0x%6x+0x%3x: <%s>  %i  [%s] :", m, k, pos, off, hdr->AS.Header+pos+off+1, typ, hdr->AS.Header+pos+off+25);

				switch (typ) {
				case 0:
				case 1: i = hdr->AS.Header[p3];        break;
				case 2: i = lei16p(hdr->AS.Header+p3); break;
				case 3: i = leu16p(hdr->AS.Header+p3); break;
				case 4: i = lei32p(hdr->AS.Header+p3); break;
				case 5: f = lef32p(hdr->AS.Header+p3); break;
				case 6: f = lef64p(hdr->AS.Header+p3); break;
				}
				if (typ<5) fprintf(stdout," *0x%x = %d",p3,i);
				else if (typ<7) fprintf(stdout," *0x%x = %g", p3,f);
				else if (typ==7) fprintf(stdout," *0x%x = <%s>",p3,hdr->AS.Header+p3);
			}
			datapos = leu32p(hdr->AS.Header + datapos);
		}

if (VERBOSE_LEVEL>7) fprintf(stdout,"CFS 419: SPR=%i=%i NRec=%i  @%p\n",SPR,hdr->SPR,hdr->NRec, hdr->AS.rawdata);

		hdr->AS.first = 0;
		if (NumberOfDataSections<=1) {
			// hack: copy data into a single block, only if more than one section	
			hdr->FLAG.UCAL = 0; 
			hdr->SPR  = SPR;
			hdr->NRec = 1;
			hdr->AS.length = 1;
		}
		else  {			
			hdr->FLAG.UCAL = 1; 
			hdr->SPR       = 1;
			hdr->NRec      = SPR;
			hdr->AS.bpb    = hdr->NS * sizeof(double);
			hdr->AS.length = SPR;
			for (k = 0; k < hdr->NS; k++) {
				CHANNEL_TYPE *hc = hdr->CHANNEL + k;
				hc->GDFTYP  = 17;	// double 
				hc->SPR     = hdr->SPR;
				hc->Cal     = 1.0;
				hc->Off     = 0.0;
			}
		}	

if (VERBOSE_LEVEL>7) fprintf(stdout,"CFS 429: SPR=%i=%i NRec=%i\n",SPR,hdr->SPR,hdr->NRec);
		datapos   = FileHeaderSize;  //+DataHeaderSize;
		
		if (flag_ChanInfoChanged) {
			B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
			B4C_ERRMSG = "CED/CFS: varying channel information not supported";
		}
		for (k = 0; k < hdr->NS; k++) {

			switch (hdr->CHANNEL[k].GDFTYP) {
			case 0:
			case 1:
				hdr->CHANNEL[k].DigMax  =  127; 
				hdr->CHANNEL[k].DigMin  = -128;
				break; 
			case 2:
				hdr->CHANNEL[k].DigMax  =  255; 
				hdr->CHANNEL[k].DigMin  =  0;
				break; 
			case 3:
				hdr->CHANNEL[k].DigMax  = (int16_t)0x7fff; 
				hdr->CHANNEL[k].DigMin  = (int16_t)0x8000;
				break; 
			case 4:
				hdr->CHANNEL[k].DigMax  = 0xffff; 
				hdr->CHANNEL[k].DigMin  = 0;
				break; 
			case 16:
			case 17:
				hdr->CHANNEL[k].DigMax  = 1e9; 
				hdr->CHANNEL[k].DigMin  = -1e9;
				break; 
			}
			hdr->CHANNEL[k].PhysMax = hdr->CHANNEL[k].DigMax * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off; 
			hdr->CHANNEL[k].PhysMin = hdr->CHANNEL[k].DigMin * hdr->CHANNEL[k].Cal + hdr->CHANNEL[k].Off;
		}
#undef H1LEN 
	}

    	else if (hdr->TYPE==HEKA) {
		fprintf(stdout,"Warning: support for HEKA-PatchMaster is very experimental\n");

    		// HEKA PatchMaster file format
    		
		hdr->FILE.LittleEndian = !!*(uint32_t*)(hdr->AS.Header+52);
		char SWAP = (hdr->FILE.LittleEndian && (__BYTE_ORDER == __BIG_ENDIAN)) ||	  \
			    (!hdr->FILE.LittleEndian && (__BYTE_ORDER == __LITTLE_ENDIAN));
		
    		for (int k=0; k<12; k++) {
    			uint32_t start  = *(uint32_t*)(hdr->AS.Header+k*16+64);
    			uint32_t length = *(uint32_t*)(hdr->AS.Header+k*16+64+4);
    			if (SWAP) {
    				start = bswap_32(start);
    				length = bswap_32(length);
    			}
    			uint8_t *ext = hdr->AS.Header + k*16 + 64 + 8;
    			if (!start) break; 
    			if (!memcmp(ext,".dat\0\0\0\0",8)) {
    				uint16_t gdftyp = 3;
				ifseek(hdr,start,SEEK_SET);
				hdr->AS.rawdata = (uint8_t*)realloc(hdr->AS.rawdata,length);
				ifread(hdr->AS.rawdata, 1, length, hdr);
    			}
    		}
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
		char FLAG_implicite_VR = 1;	
		if (hdr->HeadLen<132) {
			hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, 132);
		    	hdr->HeadLen += ifread(hdr->AS.Header+hdr->HeadLen, 1, 132-hdr->HeadLen, hdr);
		}
		size_t pos = 0; 
		size_t count = hdr->HeadLen;
		while (!hdr->AS.Header[pos] && (pos<128)) pos++;
		if ((pos==128) && !memcmp(hdr->AS.Header+128,"DICM",4)) {
			FLAG_implicite_VR = 0;	
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

			if (hdr->FILE.LittleEndian) {
				Tag  = (((uint32_t)l_endian_u16(nextTag[0])) << 16) + l_endian_u16(nextTag[1]);
				pos += 4;
				if (FLAG_implicite_VR) {
					Len = leu32p(hdr->AS.Header+pos);
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
						Len = leu16p(hdr->AS.Header+pos+2);
						pos += 4; 
					}	
					else {	
						Len = leu32p(hdr->AS.Header+pos+4);
						pos += 8; 
					}
				}	
			}
			else {
				fprintf(stdout,"Warning BigEndian Dicom not tested\n");			
				Tag  = (((uint32_t)b_endian_u16(nextTag[0])) << 16) + b_endian_u16(nextTag[1]);
				//Tag = (beu16p(hdr->AS.Header+pos)<<16) + beu16p(hdr->AS.Header+pos+2);
				Len =  beu16p(hdr->AS.Header+pos+6);
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
			case 0x00020010: {
				char *t = (char*)hdr->AS.Header+pos;
				while (isspace(*t)) t++;	// deblank
				char *ct[] = {	"1.2.840.10008.1.2", 
						"1.2.840.10008.1.2.1",
						"1.2.840.10008.1.2.1.99",
						"1.2.840.10008.1.2.2"
						};
				if (!strcmp(t,*ct)) FLAG_implicite_VR = 1;	
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
			if (VERBOSE_LEVEL>6)
				fprintf(stdout,"%s %6x:   (%04x,%04x) %8d\t%s\n",(flag_ignored?"ignored":"       "),pos,Tag>>16,Tag&0x0ffff,Len,(char*)hdr->AS.Header+pos);			
			
			pos += Len; 
		}
		if (flag_t0 == 3) hdr->T0 = tm_time2gdf_time(&T0); 

}
#endif 

#ifdef WITH_PDP
#include "../NONFREE/sopen_pdp_read.c"
#endif 

#ifdef __cplusplus
}
#endif 

