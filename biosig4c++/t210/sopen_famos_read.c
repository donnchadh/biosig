/*

    $Id: sopen_famos_read.c,v 1.5 2009/04/06 07:38:33 schloegl Exp $
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

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../biosig-dev.h"

/* 
   FIXME: 
   	
   The switch __4HAERTEL__ is for some specific FAMOS file not supported yet by the default code. 
   currently, it is not clear (at least to me) how support for such a file should 
   be incorporated it the standard solution.    
   
#define __4HAERTEL__ 
*/

EXTERN_C int sopen_FAMOS_read(HDRTYPE* hdr) {
#define Header1 ((char*)hdr->AS.Header)	

		size_t count = hdr->HeadLen;
	    	
		char *t, *t1, *t2;
		const char EOL[] = "|;\xA\xD";
		size_t pos, l1, len;
		pos  = strspn(Header1, EOL);
		uint16_t gdftyp, CHAN=0;
		char OnOff=1;		
		double Fs = NaN;
		uint32_t NoChanCurrentGroup = 0;	// number of (undefined) channels of current group 
		int level = 0; 	// to check consistency of file

		char flag_AbstandFile = 0;		// interleaved format ??? used for experimental code 
		
		fprintf(stdout,"SOPEN(FAMOS): support is experimental. Only time series with equidistant sampling and single sampling rate are supported.\n");		

		while (pos < count-20) {
			t       = Header1+pos;	// start of line

			l1      = strcspn(t+5, ",");
			t[l1+5] = 0;
			len     = atol(t+5);
			pos    += 6+l1;
			t2      = Header1+pos;	// start of line
			if (count < max(pos,hdr->HeadLen)+256) {
		    		size_t bufsiz = 4095;
			    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, count+bufsiz+1);
	    			count += ifread(hdr->AS.Header+count,1,bufsiz,hdr);
			}
			pos    += len+1;
				
			
			if (VERBOSE_LEVEL>7)
				fprintf(stdout,"FAMOS %i <%s>: %i,%i OnOff=%i\n",pos,t,l1,len,OnOff);
			

			if (!strncmp(t,"CF,2",4) && (level==0)) {
				level = 1; 
			}	
			else if (!strncmp(t,"CK,1",4) && (level==1)) {
				level = 2;
			}
			else if (!strncmp(t,"NO,1",4) && ((level==1) || (level==2))) {
				int p;
				// Ursprung
				p = strcspn(t2,",");
				t2[p] = 0;
				// NameLang
				t2 += p+1;
				p = strcspn(t2,",");
				t2[p] = 0;
				size_t len = min(MAX_LENGTH_MANUF,atol(t2));
				// Name 
				t2 += p+1;
				strncpy(hdr->ID.Manufacturer._field,t2,len); 
				hdr->ID.Manufacturer._field[len]=0;
				hdr->ID.Manufacturer.Name =hdr->ID.Manufacturer._field; 
			}
			else if (!strncmp(t,"CT,1",4) && (level>1)) {
			}

			else if (!strncmp(t,"Cb,1",4)) 	// Buffer Beschreibung
			{
				// AnzahlBufferInKey
				int p = strcspn(t2,",");
				t2[p] = 0;
				if (atoi(t2) != 1) {
					B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
					B4C_ERRMSG = "FAMOS: more than one buffer not supported (yet)";
				}
				// BytesInUserInfo
				t2 += 1+p;
				p = strcspn(t2,",");
				t2[p] = 0;
				// Buffer Referenz
				t2 += 1+p;
				p = strcspn(t2,",");
				t2[p] = 0;
				// IndexSamplesKey
				t2 += 1+p;
				p = strcspn(t2,",");
				t2[p] = 0;
				// OffsetBufferInSamplesKey
				t2 += 1+p;
				p = strcspn(t2,",");
				t2[p] = 0;
				hdr->CHANNEL[CHAN].bi = atol(t2);
				// BufferLangBytes
				t2 += 1+p;
				p = strcspn(t2,",");
				t2[p] = 0;
				// OffsetFirstSampleInBuffer, 
				t2 += 1+p;
				p = strcspn(t2,",");
				t2[p] = 0;
				// BufferFilledBytes,
				t2 += 1+p;
				p = strcspn(t2,",");
				t2[p] = 0;
				size_t bpb = atol(t2);
				hdr->CHANNEL[CHAN].SPR = 8*bpb/GDFTYP_BITS[hdr->CHANNEL[CHAN].GDFTYP];


				if (VERBOSE_LEVEL) fprintf(stdout,"famos123: %i %i %i\n", OnOff, CHAN, hdr->CHANNEL[CHAN].SPR);
				
				if ((OnOff) && (CHAN==0)) hdr->SPR = hdr->CHANNEL[CHAN].SPR;  
				
				if (hdr->SPR != hdr->CHANNEL[CHAN].SPR) {
					fprintf(stdout,"Warning SOPEN(FAMOS): multiple sampling (%i:%i) rates not supported. Channel %i ignored!\n", hdr->SPR, hdr->CHANNEL[CHAN].SPR, CHAN+1);
					OnOff = 0;  
				}
				if (!OnOff) hdr->CHANNEL[CHAN].SPR = 0;

				hdr->AS.bpb = hdr->CHANNEL[CHAN].bi + bpb;
				
				// 0, 
				t2 += 1+p;
				p = strcspn(t2,",");
				t2[p] = 0;
				// X0, 
				t2 += 1+p;
				p = strcspn(t2,",");
				t2[p] = 0;
				// AddZeit, 
				t2 += 1+p;
				p = strcspn(t2,",");
				t2[p] = 0;
				// UserInfo
				t2 += 1+p;
				p = strcspn(t2,EOL);
			}

			else if (!strncmp(t,"CB,1",4) && (level>1)) {
			// Gruppen Definition
				// Index der Gruppe
				int p = strcspn(t2,",");
				t2[p] = 0;
				if (atol(t2)>1) {
					B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
					B4C_ERRMSG = "FAMOS: more than one group not supported (yet)";
				}	
				level = 2; 
			}

			else if (!strncmp(t,"CI,1",4)) {
			}

			else if (!strncmp(t,"CG,1",4)) // Definition eines Datenfeldes 
			{
				int p;
				// Anzahl Komponenten				
				p = strcspn(t2,",");
				t2[p] = 0;
				NoChanCurrentGroup = atol(t2);	// additional channels
				hdr->NS += NoChanCurrentGroup;
				// Feldtyp
				p = strcspn(t2,",");
				t2[p] = 0;
				OnOff = 1; 
				if (atoi(t2) != 1) {
//					OnOff = 0; 
//					B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
//					B4C_ERRMSG = "FAMOS: data is not real and aquidistant sampled";
				}
				// Dimension 
				p = strcspn(t2,",");
				t2[p] = 0;

				hdr->CHANNEL = (CHANNEL_TYPE*)realloc(hdr->CHANNEL, hdr->NS*sizeof(CHANNEL_TYPE));
				level = 3;
			}

			else if (!strncmp(t,"CD,",3) && (level>=3)) {
				int p;
				// dx
				p = strcspn(t2,",");	
				t2[p] = 0;
				Fs  = 1.0/atof(t2);
				// kalibriert
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				// EinheitLang
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				size_t len = max(atol(t2),MAX_LENGTH_PHYSDIM);
				// Einheit
				t2 += p+1;
				if (strncmp(t,"s",strlen(t))) {
					// turn off channel if unit of x-axis is not seconds [s]
//					OnOff = 0;
				}
				
				//PhysDimCode(t2);
					
			}
			else if (!strncmp(t,"NT,1",4) && (level>=3)) {
				struct tm tm_time;
			
				int p = strcspn(t2,",");
				t2[p] = 0;
				tm_time.tm_mday  = atoi(t2);
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				tm_time.tm_mon = atoi(t2)-1;
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				tm_time.tm_year = atoi(t2)-1900;
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				tm_time.tm_hour  = atoi(t2);
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				tm_time.tm_min = atoi(t2);
				p   = strcspn(t2,EOL);
				// t2[p] = 0;
				char tmp[10];
				strncpy(tmp,t2,p);
				tmp[p]=0;
				tm_time.tm_sec = atoi(tmp);

				hdr->T0 = tm_time2gdf_time(&tm_time); 
			}
			else if (!strncmp(t,"CZ,1",4) && (level==3)) {
			}
			else if (!strncmp(t,"CC,1",4) && (level>=3)) {
				if (NoChanCurrentGroup<1) {
					B4C_ERRNUM = B4C_UNSPECIFIC_ERROR;
					B4C_ERRMSG = "FAMOS: too many CC definitions in group";
				}
				CHAN = hdr->NS - NoChanCurrentGroup--;

				if (CHAN==0) 
					hdr->SampleRate = Fs; 
				else if (OnOff && (abs(hdr->SampleRate - Fs)>1e-9*Fs)) {
					fprintf(stdout,"ERR2: %i %f %f\n",CHAN,hdr->SampleRate, Fs);
//					B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
//					B4C_ERRMSG = "FAMOS: multiple sampling rates not supported";
				} 
				if (VERBOSE_LEVEL>7)
					fprintf(stdout,"CC: %i#%i Fs=%f,%i\n",OnOff,CHAN,Fs,len);

/*
				int p = strcspn(t2,",");
				t2[p] = 0;
				Fs  = 1.0/atof(t2);
*/
				level = 4; 
			}
			else if (!strncmp(t,"CP,1",4) && (level==4)) {
				int p;
				
				char s[21]; strncpy(s,t2,20);s[20]=0;	
				if (VERBOSE_LEVEL>7)
					fprintf(stdout,"CHAN=%i tag=<%s>\n",CHAN,s);

				// Bufferreferenz
				p = strcspn(t2,",");
				t2[p] = 0;

				// Bytes
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;

				// Zahlenformat  
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				gdftyp = atoi(t2);

				if (VERBOSE_LEVEL>7)
					fprintf(stdout,"CHAN=%i tag=<%s> gdf=%i\n",CHAN,s,gdftyp);
					  				
				// SignBits  
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;

				// Mask  
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				if (atoi(t2)) {
					B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
					B4C_ERRMSG = "FAMOS: Mask != 0 not supported";
				};

				// Offset  
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				if (atoi(t2)) {
					fprintf(stdout,"Offset:<%s>\n",t2);
					flag_AbstandFile = 1;
#ifndef __4HAERTEL__
					B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
					B4C_ERRMSG = "FAMOS: Offset != 0 not supported";
#endif
				};
				// DirekteFolgeAnzahl  
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				if (atoi(t2) != 1) {
					B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
					B4C_ERRMSG = "FAMOS: DirekteFolgeAnzahl != 1 not supported";
				};

				// AbstandBytes  
				t2 += p+1;
				p   = strcspn(t2,EOL);
				t2[p] = 0;
				if (atoi(t2)) {
					fprintf(stdout,"Abstandbytes:<%s>\n",t2);
					flag_AbstandFile = 1;
#ifndef __4HAERTEL__
					B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
					B4C_ERRMSG = "FAMOS: AbstandBytes != 0 not supported";
#endif
				};
				
				double digmax=1e6,digmin=-1e6;
				switch (gdftyp) {
				case 1:
					gdftyp = 2; // uint8
					digmax = 255;
					digmin = 0;
					break;
				case 2:
					gdftyp = 1; // int8
					digmax = 127;
					digmin = -128;
					break;
				case 3:
				case 9:
				case 11:
					gdftyp = 4; // uint16
					digmax = 65535;
					digmin = 0;
					break;
				case 4:
					gdftyp = 3; // int16
					digmax =  32767;
					digmin = -32768;
					break;
				case 5:
					gdftyp = 6; // uint32
					digmax = (uint32_t)0xffffffff;
					digmin = 0;
					break;
				case 6:
					gdftyp = 5; // int32
					digmax = (int32_t)0x7fffffff;
					digmin = (int32_t)0x80000000;
					break;
				case 7:
					gdftyp = 16; // float32
					break;
				case 8:
					gdftyp = 17; // float64
					break;
				case 10:
					gdftyp = 0; 
					break;
				case 13:
					gdftyp = 511+48; // float64
					break;
				default:
					gdftyp = 0;	
					B4C_ERRNUM = B4C_DATATYPE_UNSUPPORTED;
					B4C_ERRMSG = "FAMOS: unknown datatype";
				};
				
				hdr->CHANNEL[CHAN].LeadIdCode = 0; 
				hdr->CHANNEL[CHAN].OnOff   = OnOff; 
				hdr->CHANNEL[CHAN].GDFTYP  = gdftyp; 
				hdr->CHANNEL[CHAN].DigMax  = digmax; 
				hdr->CHANNEL[CHAN].DigMin  = digmin; 
				hdr->CHANNEL[CHAN].PhysMax = digmax; 
				hdr->CHANNEL[CHAN].PhysMin = digmin; 

				// initialize undefined values
				hdr->CHANNEL[CHAN].LowPass = -1.0; 
				hdr->CHANNEL[CHAN].HighPass= -1.0; 
				hdr->CHANNEL[CHAN].Notch   = -1.0; 
			      	hdr->CHANNEL[CHAN].Label[0]  = 0;
	      			hdr->CHANNEL[CHAN].Transducer[0]=0;
			      	hdr->CHANNEL[CHAN].PhysDimCode = 0;
			      	hdr->CHANNEL[CHAN].SPR       = 1;	// one sample per block
			      	hdr->CHANNEL[CHAN].Impedance = INF;
	      			hdr->CHANNEL[CHAN].XYZ[0] = 0.0;
	      			hdr->CHANNEL[CHAN].XYZ[1] = 0.0;
	      			hdr->CHANNEL[CHAN].XYZ[2] = 0.0;

				if (VERBOSE_LEVEL>7)
					fprintf(stdout,"#%i\t%i %i\n",CHAN,gdftyp, hdr->CHANNEL[CHAN].GDFTYP);

			}
			else if (!strncmp(t,"CR,1",4)) {
				int p;
				// Transformieren
				p = strcspn(t2,",");
				t2[p] = 0;
				int flagTransform = atoi(t2);
				// Faktor
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				hdr->CHANNEL[CHAN].Cal = flagTransform ? atof(t2) : 1.0;
				// Offset
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				hdr->CHANNEL[CHAN].Off = flagTransform ? atof(t2) : 0.0;

				hdr->CHANNEL[CHAN].PhysMax = hdr->CHANNEL[CHAN].DigMax * hdr->CHANNEL[CHAN].Cal + hdr->CHANNEL[CHAN].Off;
				hdr->CHANNEL[CHAN].PhysMin = hdr->CHANNEL[CHAN].DigMin * hdr->CHANNEL[CHAN].Cal + hdr->CHANNEL[CHAN].Off;

				// Kalibriert
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				// EinheitLang
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				int len = min(atoi(t2),MAX_LENGTH_PHYSDIM); 
				// Einheit
				t2 += p+1;
				/*
				strncpy(hdr->CHANNEL[CHAN].PhysDim,t2,len);
				hdr->CHANNEL[CHAN].PhysDim[len] = 0;
				hdr->CHANNEL[CHAN].PhysDimCode  = PhysDimCode(hdr->CHANNEL[CHAN].PhysDim);
				*/
				hdr->CHANNEL[CHAN].PhysDimCode  = PhysDimCode(t2);

			}
			else if (!strncmp(t,"ND,1",4) && (level==4)) {
				// Display properties 
			}
			else if (!strncmp(t,"CN,1",4) && (level==4)) {
				int p;
				// Indexgruppe
				p   = strcspn(t2,",");
				t2[p] = 0;
				// 0
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				// IndexBit
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				// NameLang
				t2 += p+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				int len = atoi(t2);
				// Name
				t2 += p+1;
				len   = min(len,MAX_LENGTH_LABEL);
				strncpy(hdr->CHANNEL[CHAN].Label, t2, len);
				hdr->CHANNEL[CHAN].Label[len] = 0;
				// KommLang
				t2 += len+1;
				p   = strcspn(t2,",");
				t2[p] = 0;
				// Kommentar
				t2 += p+1;
				p   = strcspn(t2,",");

			}

			else if (!strncmp(t,"CS,1",4)) {
				int p = strcspn(t2,",");
				t2[p] = 0;
				if (atol(t2)>1) {
					B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
					B4C_ERRMSG = "FAMOS: more than one CS section not supported (yet)";
				}	
				hdr->HeadLen = pos-len+p;

				if (VERBOSE_LEVEL>8)
					fprintf(stdout,"FAMOS: CS section reached\n");
			

				break;
			}
			pos += strcspn(Header1+pos,EOL);
			pos += strspn(Header1+pos,EOL);
		}
		ifseek(hdr,hdr->HeadLen,SEEK_SET);
		hdr->NRec = 1; 

#ifdef __4HAERTEL__
		if (flag_AbstandFile==1) {
			
			size_t bpb = 0; 
			uint16_t k;
			for (k=0; k<hdr->NS; k++) {
				if (hdr->CHANNEL[k].SPR>=1)
					hdr->SPR = lcm(hdr->SPR,hdr->CHANNEL[k].SPR);
				hdr->CHANNEL[k].SPR = 1; 
				hdr->CHANNEL[k].bi  = bpb; 
				bpb += GDFTYP_BITS[hdr->CHANNEL[k].GDFTYP]>>3;
			}

			hdr->NRec *= hdr->SPR;
			hdr->SPR = 1; 
			hdr->AS.bpb = bpb;

		 	// This part is necessary for broken files (if header information does not fit file size) 
			struct stat FileBuf;
			stat(hdr->FileName,&FileBuf);
			size_t tmp = (FileBuf.st_size - hdr->HeadLen)/bpb;
			if (tmp < hdr->NRec) hdr->NRec =  tmp; 
		}
#endif
			
};
// End of SOPEN_FAMOS_READ
