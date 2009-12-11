/*

    $Id: biosig_server.c,v 1.7 2009-04-08 15:55:56 schloegl Exp $
    Copyright (C) 2009 Alois Schloegl <a.schloegl@ieee.org>
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

#include "biosig-network.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <syslog.h>
#include <sys/wait.h>
#include <signal.h>

#ifdef WITH_MICROMED 
void sopen_trc_read(HDRTYPE *hdr);
#endif
#ifdef WITH_PDP 
void sopen_pdp_read(HDRTYPE *hdr);
#endif

char *LOGFILE = "/tmp/biosig/biosigd.log";

/* 
	Key ID table and related functions 
*/
struct IDTABLE_T {
	uint64_t id;
	uint64_t gtime;	// time of file generation 
	struct sockaddr_in addr;
} *IdTablePtr; 
size_t IdTableLen; 


/*
	compare uint32_t
*/
int u64cmp(const void *a,const void *b) 
{
	unsigned int k=7;
	int r=0;
	while (!r && (k<sizeof(uint64_t))) {
		r = ((uint8_t*)a)[k] - ((uint8_t*)b)[k];
		k--; 
	}	
	return(r); 	 	
}

/*
	generate new unique id
 */
uint64_t GetNewId() {

	int k; 
	uint64_t c;
	FILE *fid = fopen("/dev/urandom","r"); 
	do {
		fread(&c,sizeof(c),1,fid); 
	} while (bsearch(&c, IdTablePtr, IdTableLen, sizeof(IDTABLE_T), &u64cmp)); 
	fclose(fid); 
		
	k = IdTableLen; 
	IdTableLen++; 
	IdTablePtr = (IDTABLE_T*)realloc(IdTablePtr,IdTableLen*sizeof(IDTABLE_T));
	while ((c < IdTablePtr[k].id) && (k>0)) {
		IdTablePtr[k].id = IdTablePtr[k-1].id;
		k--;
	}	
	IdTablePtr[k].id = c;
	IdTablePtr[k].gtime = t_time2gdf_time(time(NULL));
//	memcpy(&IdTablePtr[k].addr, addr, sizeof(struct sockaddr_in));
	
	return(c); 
}; 


/*TODO: semaphore */
size_t LoadIdTable(const char *path) {

	IdTableLen = 0; 
	DIR *d = opendir(path); 
	if (d==NULL) {
		mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		return(0); 
	}	

	uint64_t ID; 
	size_t N=0; 
    	struct dirent *dp;
    	do {
        	errno = 0;
        	if ((dp = readdir(d)) != NULL) {
			IdTablePtr = (IDTABLE_T*)realloc(IdTablePtr,(N+1)*sizeof(IDTABLE_T));
			cat64(dp->d_name,&ID);
			if (ID != 0) {
				IdTablePtr[N].id = ID;
				N++;
			}	
        	}
	} while (dp != NULL);
	closedir(d); 

	qsort(IdTablePtr,N,sizeof(IDTABLE_T),&u64cmp);
	IdTableLen = N; 

	fprintf(stdout,"Load Id Table: %i entries found.\n",N);
		
	if (VERBOSE_LEVEL>8)
		for (int k=0; k<N; k++)
			fprintf(stdout,"%3i %016lx\n",k+1,IdTablePtr[k].id);

	return(N);
}; 


void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}


//#define VERBOSE_LEVEL 8

const char   path[] = "/tmp/biosig/\0                .gdf";
void DoJob(int ns)
{
		char fullfilename[] = "/tmp/biosig/                .gdf";
		const size_t pathlen = strlen(path); 
		char *filename = fullfilename+pathlen;  

//    		size_t sizbuf = BSCS_MAX_BUFSIZ+8;
		uint8_t inbuf[BSCS_MAX_BUFSIZ+8];
		HDRTYPE *hdr = NULL; 
		uint64_t ID = 0; 
		uint32_t STATUS = STATE_INIT;
		char stopflag = 0; 
		size_t datalen = 0; 
		mesg_t msg; 	

		if ((VERBOSE_LEVEL>7) && errno) {
			errno = 0; 
			fprintf(stdout,"## errno=%i %s\n",errno,strerror(errno));
		}	 

		/* Create a new SID for the child process */
		pid_t sid = setsid();
		if (sid < 0) {
			exit(-1);
		}

		/*server identification */
		const char *greeting="Hi there,\n this is your experimental BSCS server. \n It is useful for testing the BioSig client-server architecture.\n";
		msg.STATE = BSCS_VERSION_01 | BSCS_SEND_MSG | STATE_INIT | BSCS_NO_ERROR;
		msg.LEN = b_endian_u32(strlen(greeting)); 
		int s; 
		s = send(ns,&msg,8,0);
		s = send(ns, greeting, strlen(greeting), 0);
		
	    	while (!stopflag) // wait for command 
		{
		
			fprintf(stdout,":> "); 
	   		ssize_t count = recv(ns, &msg, 8, 0);
			fprintf(stdout,"STATUS=%08x c=%i MSG=%08x len=%i,errno=%i %s\n",STATUS,count,b_endian_u32(msg.STATE),b_endian_u32(msg.LEN),errno,strerror(errno)); 
			size_t  LEN = b_endian_u32(msg.LEN);  

			FILE *fid = fopen(LOGFILE,"a");
			fprintf(stdout,"STATUS=%08x c=%i MSG=%08x len=%i,errno=%i %s\n",STATUS,count,b_endian_u32(msg.STATE),b_endian_u32(msg.LEN),errno,strerror(errno)); 
			fclose(fid); 
			errno = 0; 

	   		if (count==0) {
				fprintf(stdout,"connection lost! %i %i %i %s\n",stopflag,count,errno,strerror(errno)); 
				LEN = 0;
				stopflag=1;
				continue;
	   		} 
	   		else if (count<0) {
				fprintf(stdout,"invalid packet! %i %i %i %s\n",stopflag,count,errno,strerror(errno)); 
				LEN = 0;
	   		} 
			else if ((msg.STATE & CMD_MASK) == BSCS_NOP) 
			{	// no operation 

				count = 0; 
				while (count<LEN) {
			   		count += recv(ns, inbuf, min(BSCS_MAX_BUFSIZ,LEN-count), 0);
				}
				msg.STATE = BSCS_VERSION_01 | BSCS_NOP | BSCS_REPLY | (msg.STATE & STATE_MASK) | BSCS_NO_ERROR;

				// send reply 
				msg.LEN = b_endian_u32(0);
				s = send(ns, &msg, 8, 0);	
			}

			else if ((STATUS == STATE_INIT) && 
			    	 ((msg.STATE & ~ERR_MASK) == (BSCS_VERSION_01 | BSCS_OPEN | STATE_INIT))) 
			{
/****************************************************************************************
	OPEN FILE with ID 
 ****************************************************************************************/

				if (VERBOSE_LEVEL>7) fprintf(stdout,"open %i %i %016lx\n",LEN,count,ID);

				hdr = constructHDR(0,0);
				if (LEN==0) //|| ((LEN==8) && (ID==0))) 	// no ID, send back id, open(w) 
				{
					ID = GetNewId(); 
					c64ta(ID, filename); 
					hdr->FLAG.ANONYMOUS = 1; 	// do not store name 
					STATUS = STATE_OPEN_WRITE_HDR; 
					msg.STATE = BSCS_VERSION_01 | BSCS_OPEN_W | BSCS_REPLY | STATE_OPEN_WRITE_HDR;
					msg.LEN = b_endian_u32(8);
					*(uint64_t*)&msg.LOAD  = l_endian_u64(ID); 
					s = send(ns, &msg, 16, 0);	
				}
				else if (LEN == 8) 		// open(r)
				{
		   			count = recv(ns, &msg.LOAD, 8, 0);
					ID = leu64p(&msg.LOAD);

					c64ta(ID, filename); 
					// ToDo: replace SOPEN with simple function, e.g. access to event table not needed here 
					hdr->FLAG.ANONYMOUS = 1; 	// do not store name 
					hdr->FileName = fullfilename;
					hdr = sopen(fullfilename,"r",hdr);
					msg.LEN = b_endian_u32(0);
					if (hdr->FILE.OPEN==0) {
						msg.STATE = BSCS_VERSION_01 | BSCS_OPEN_R | BSCS_REPLY | STATE_INIT | BSCS_ERROR_CANNOT_OPEN_FILE;
						STATUS = STATE_INIT; 
					} else if (serror() || (hdr->NRec < 0)) {
						sclose(hdr);
						msg.STATE = BSCS_VERSION_01 | BSCS_OPEN_R | BSCS_REPLY | STATE_INIT | BSCS_ERROR_CANNOT_OPEN_FILE;
						STATUS = STATE_INIT; 
					} else 	{
						msg.STATE = BSCS_VERSION_01 | BSCS_OPEN_R | BSCS_REPLY | STATE_OPEN_READ;
						STATUS = STATE_OPEN_READ; 
					}
					s = send(ns, &msg, 8, 0);	
				}
				else //if (LEN != 8) 
				{ 
					STATUS = STATE_INIT; 
					msg.STATE = BSCS_VERSION_01 | BSCS_OPEN_W | BSCS_REPLY | STATE_INIT | BSCS_ERROR_INCORRECT_PACKET_LENGTH;
					msg.LEN = b_endian_u32(0);
					s = send(ns, &msg, 8, 0);	
				}
			}	

			else if ((STATUS == (msg.STATE & STATE_MASK)) && 
				 (LEN == 0) && 
				 ((msg.STATE & (VER_MASK | CMD_MASK)) == (BSCS_VERSION_01 | BSCS_CLOSE))) 
			{	// close 
/****************************************************************************************
	CLOSE FILE 
 ****************************************************************************************/

				if (STATUS != (msg.STATE & STATE_MASK)) fprintf(stdout,"Close: status does not fit\n");
	
				msg.STATE = BSCS_VERSION_01 | BSCS_CLOSE | BSCS_REPLY | STATE_INIT;
				msg.LEN = b_endian_u32(0);

				if (LEN != 0) 
					msg.STATE = BSCS_VERSION_01 | BSCS_CLOSE | BSCS_REPLY | STATE_INIT | BSCS_ERROR_INCORRECT_PACKET_LENGTH;

				if ((STATUS == STATE_OPEN_WRITE_HDR) || (STATUS == STATE_OPEN_WRITE)) {
					if (hdr->NRec != datalen/hdr->AS.bpb)
						hdr->NRec = -1; // this triggers sclose to compute the correct size  
				} 
								
				if ((hdr!=NULL) && ((STATUS != STATE_OPEN_READ) || (STATUS != STATE_OPEN_READ)) ) 
					sclose(hdr); 

				if (STATUS != STATE_INIT) {
					destructHDR(hdr); 
					hdr = NULL;
				}	

				hdr = NULL; 
				if (serror())
					msg.STATE = BSCS_VERSION_01 | BSCS_CLOSE | BSCS_REPLY | STATE_INIT | BSCS_ERROR_CLOSE_FILE;

				STATUS = STATE_INIT; 

				s = send(ns, &msg, 8, 0);	

			}

			else if ((STATUS == STATE_OPEN_WRITE_HDR) && 
				 ((msg.STATE & ~ERR_MASK) == (BSCS_VERSION_01 | BSCS_SEND_HDR | STATE_OPEN_WRITE_HDR))) 
			{	
/****************************************************************************************
	SEND HEADER
 ****************************************************************************************/
 
				//case BSCS_SEND_HDR:  	// send header information
				hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,LEN); 
				hdr->HeadLen = LEN; 
				count = 0; 
				while (count<LEN) {
if (VERBOSE_LEVEL>8) fprintf(stdout,"SND HDR: c=%i,LEN=%i\n",count,LEN);				
					count += recv(ns, hdr->AS.Header+count, LEN-count, 0);
				}
if (VERBOSE_LEVEL>8) fprintf(stdout,"SND HDR: LEN=%i\n",LEN,count);				

				hdr->TYPE = GDF; 
				hdr->FLAG.ANONYMOUS = 1; 	// do not store name 
				gdfbin2struct(hdr);

				hdr->EVENT.N = 0; 
				hdr->EVENT.POS = NULL; 
				hdr->EVENT.TYP = NULL; 
				hdr->EVENT.DUR = NULL; 
				hdr->EVENT.CHN = NULL; 
				
				c64ta(ID, filename); 
				hdr->FileName  = fullfilename;
				hdr->FILE.COMPRESSION = 0; 
				ifopen(hdr,"w"); 
				hdr->FILE.OPEN = 2; 
				count=ifwrite(hdr->AS.Header, 1, hdr->HeadLen, hdr);
				datalen = 0;

if (VERBOSE_LEVEL>8) fprintf(stdout,"SND HDR: count=%i\n",count);				
	
				if (errno) {	
					// check for errors in gdfbin2struct, ifopen and ifwrite		
					STATUS = STATE_OPEN_WRITE_HDR; 
					msg.STATE = BSCS_VERSION_01 | BSCS_SEND_HDR | BSCS_REPLY | STATE_OPEN_WRITE_HDR | BSCS_ERROR_COULD_NOT_WRITE_HDR;
				}	
				else {
					STATUS = STATE_OPEN_WRITE; 
					msg.STATE = BSCS_VERSION_01 | BSCS_SEND_HDR | BSCS_REPLY | STATE_OPEN_WRITE;
				}	
				msg.LEN = b_endian_u32(0);
if (VERBOSE_LEVEL>8) fprintf(stdout,"SND HDR RPLY: %08x\n",msg.STATE);				
				s = send(ns, &msg, 8, 0);	
			}


			else if ((STATUS == STATE_OPEN_WRITE) && 
				 ((msg.STATE & ~ERR_MASK) == (BSCS_VERSION_01 | BSCS_SEND_DAT | STATE_OPEN_WRITE))) 
			{	
/****************************************************************************************
	SEND DATA
 ****************************************************************************************/
				//case BSCS_SEND_DAT:   	// send data block 
				// ToDo: check corrupt packet
					count = 0; 
					while (count<LEN) {
						// ToDo: check if streams would be faster 
						// fprintf(stdout,"SEND_DAT %li %li %li\n",datalen,count,LEN);
					   	ssize_t c = recv(ns, inbuf, min(BSCS_MAX_BUFSIZ,LEN-count), 0);
						datalen += ifwrite(inbuf, 1, c, hdr);
					   	count += c;
					}
					//STATUS = STATE_OPEN_WRITE; 

				if (errno) {	
					// check for errors in ifwrite		
					msg.STATE = BSCS_VERSION_01 | BSCS_SEND_DAT | BSCS_REPLY | STATE_OPEN_WRITE | BSCS_ERROR_COULD_NOT_WRITE_DAT;
				}	
				else {
					msg.STATE = BSCS_VERSION_01 | BSCS_SEND_DAT | BSCS_REPLY | STATE_OPEN_WRITE;
				}	
				msg.LEN = b_endian_u32(0);
				s = send(ns, &msg, 8, 0);	
			}

			else if ((STATUS == STATE_OPEN_WRITE) && 
				 ((msg.STATE & ~ERR_MASK) == (BSCS_VERSION_01 | BSCS_SEND_EVT | STATE_OPEN_WRITE))) 
			{	
/****************************************************************************************
	SEND EVENTS
 ****************************************************************************************/
				//case BSCS_SEND_EVT:  	// send event information 

				
		   		count = recv(ns, inbuf+8, min(8,LEN), 0);
		   		if (count<8) continue; 

				char  flag = inbuf[8];
				int   N = inbuf[9] + (inbuf[10] + inbuf[11]*256)*256; 
				//float Fs = lef32p(inbuf+12); 
				
				if (LEN != (8 + N * (flag==3 ? 12u : 6u)))
				{
					msg.STATE = BSCS_VERSION_01 | BSCS_SEND_EVT | BSCS_REPLY | STATE_OPEN_WRITE | BSCS_ERROR_INCORRECT_PACKET_LENGTH;
				}

				else if (N>0) {
					size_t n = hdr->EVENT.N;
					hdr->EVENT.N += N;
	 				hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, hdr->EVENT.N*sizeof(*hdr->EVENT.POS) );
					hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP) );
					hdr->EVENT.DUR = (uint32_t*) realloc(hdr->EVENT.DUR, hdr->EVENT.N*sizeof(*hdr->EVENT.DUR) );
					hdr->EVENT.CHN = (uint16_t*) realloc(hdr->EVENT.CHN, hdr->EVENT.N*sizeof(*hdr->EVENT.CHN) );
	
					// read EVENT.POS
					count = 0; 
					LEN = N*sizeof(*hdr->EVENT.POS);
					while (count < LEN) {
						count += recv(ns, (uint8_t*)(hdr->EVENT.POS+n)+count, LEN-count, 0);
					}
					// read EVENT.TYP
					count = 0; 
					LEN = N*sizeof(*hdr->EVENT.TYP);
					while (count < LEN) {
						count += recv(ns, (uint8_t*)(hdr->EVENT.TYP+n)+count, LEN-count, 0);
					}
					// read EVENT.DUR and EVENT.CHN
					if (flag==3) {
						count = 0; 
						LEN = N*sizeof(*hdr->EVENT.POS);
						while (count < LEN) {
							count += recv(ns, (uint8_t*)(hdr->EVENT.DUR+n)+count, LEN-count, 0);
						}
						count = 0; 
						LEN = N*sizeof(*hdr->EVENT.TYP);
						while (count < LEN) {
							count += recv(ns, (uint8_t*)(hdr->EVENT.CHN+n)+count, LEN-count, 0);
						}
					}	
					else {
						for (size_t k=n; k<hdr->EVENT.N; k++) {
							hdr->EVENT.DUR[k] = 0; 
							hdr->EVENT.CHN[k] = 0; 
						}
					}
#if __BYTE_ORDER == __BIG_ENDIAN
					// do byte swapping if needed 
					for (size_t k=n; k<hdr->EVENT.N; k++) {
						hdr->EVENT.POS[k] = l_endian_u32(hdr->EVENT.POS[k]); 
						hdr->EVENT.TYP[k] = l_endian_u16(hdr->EVENT.TYP[k]); 
					}
					if (flag==3) {	
						for (size_t k=n; k<hdr->EVENT.N; k++) {
							hdr->EVENT.DUR[k] = l_endian_u32(hdr->EVENT.DUR[k]); 
							hdr->EVENT.CHN[k] = l_endian_u16(hdr->EVENT.CHN[k]); 
						}
					} 
#endif
					msg.STATE = BSCS_VERSION_01 | BSCS_SEND_EVT | BSCS_REPLY | STATE_OPEN_WRITE;
				}
				msg.LEN = b_endian_u32(0);
				s = send(ns, &msg, 8, 0);	
			}

			else if ((msg.STATE & CMD_MASK) ==  BSCS_SEND_MSG ) 
			{	// might become obsolet (?)
				// ToDo: check corrupt packet
				count = 0; 
				ssize_t c=0;
				while (count<LEN) {
					int len = min(BSCS_MAX_BUFSIZ-1,LEN-count);	
				   	c = recv(ns, inbuf, len, 0);
			   		inbuf[c-1]=0;
					fprintf(stdout,"got message <%s>\n",(char*)(inbuf));
				   	count += c;
				}
				//system((char*)(inbuf));
			// TODO: reply message
			}

			else if ((STATUS == STATE_OPEN_READ) && 
				 ((msg.STATE & ~ERR_MASK) == (BSCS_VERSION_01 | BSCS_REQU_HDR | STATE_OPEN_READ))) 
			{	
/****************************************************************************************
	REQUEST HEADER
 ****************************************************************************************/
				msg.STATE = BSCS_VERSION_01 | BSCS_REQU_HDR | BSCS_REPLY | STATE_OPEN_READ;
				msg.LEN = b_endian_u32(hdr->HeadLen);
				s = send(ns, &msg, 8, 0);	
				s = send(ns, hdr->AS.Header, hdr->HeadLen, 0);	

			}

			else if ((STATUS == STATE_OPEN_READ) && 
				 ((msg.STATE & ~ERR_MASK) == (BSCS_VERSION_01 | BSCS_REQU_DAT | STATE_OPEN_READ))) 
			{	
/****************************************************************************************
	REQUEST DATA
 ****************************************************************************************/
	   			count += recv(ns, inbuf, 8, 0);
		   		size_t length = leu32p(inbuf);
		   		size_t start = leu32p(inbuf+4);
				sread_raw(start, length, hdr, 0);

				length = min(length, hdr->AS.first + hdr->AS.length - start);
				msg.STATE = BSCS_VERSION_01 | BSCS_REQU_DAT | BSCS_REPLY | STATE_OPEN_READ;
				msg.LEN = b_endian_u32(hdr->AS.bpb*length);
				s = send(ns, &msg, 8, 0);	
				s = send(ns, hdr->AS.rawdata + hdr->AS.bpb*(start-hdr->AS.first), hdr->AS.bpb*length, 0);	
				//send(ns, hdr->AS.rawdata, (hdr->AS.length-hdr->AS.first)*hdr->AS.bpb,0);
			}

			else if ((STATUS == STATE_OPEN_READ) && 
				 ((msg.STATE & ~ERR_MASK) == (BSCS_VERSION_01 | BSCS_REQU_EVT | STATE_OPEN_READ))) 
			{	
/****************************************************************************************
	REQUEST EVENT TABLE 
 ****************************************************************************************/
				size_t len = 0; 
				if ((hdr->TYPE == GDF) && (hdr->AS.rawEventData == NULL) && (hdr->NRec>=0)) {
					// event table from GDF file 
					ifseek(hdr, hdr->HeadLen + hdr->AS.bpb*hdr->NRec, SEEK_SET);
					// READ EVENTTABLE 
					hdr->AS.rawEventData = (uint8_t*)realloc(hdr->AS.rawEventData,8);
					int c = ifread(hdr->AS.rawEventData, sizeof(uint8_t), 8, hdr);
	    				uint8_t *buf = hdr->AS.rawEventData;
			
					if (c<8) {
						hdr->EVENT.N = 0;
					}	
					else if (hdr->VERSION < 1.94) {
						hdr->EVENT.N = leu32p(buf + 4);
					}	
					else {
						hdr->EVENT.N = buf[1] + (buf[2] + buf[3]*256)*256; 
					}	
					int sze = (buf[0]>1) ? 12 : 6;
					len = 8+hdr->EVENT.N*sze;
					hdr->AS.rawEventData = (uint8_t*)realloc(hdr->AS.rawEventData,len);
					ifread(hdr->AS.rawEventData+8, 1, len-8, hdr);
					ifseek(hdr, hdr->HeadLen+hdr->AS.bpb*hdr->FILE.POS, SEEK_SET); 
					// note: no conversion to HDR.EVENT structure needed on server-side 
				}
				else if ((hdr->TYPE != GDF) && (hdr->AS.rawEventData == NULL))
					len = hdrEVT2rawEVT(hdr); 

				else if ((hdr->TYPE == GDF) && (hdr->AS.rawEventData != NULL)){
					uint8_t *buf = hdr->AS.rawEventData; 
					if (hdr->VERSION < 1.94) 
						hdr->EVENT.N = leu32p(buf + 4);
					else	
						hdr->EVENT.N = buf[1] + (buf[2] + buf[3]*256)*256; 
				
					int sze = (buf[0]>1) ? 12 : 6;
					
					if (hdr->EVENT.N>0) {
						len = 8+sze*hdr->EVENT.N;
					}
				}

				msg.STATE = BSCS_VERSION_01 | BSCS_REQU_HDR | BSCS_REPLY | STATE_OPEN_READ;
				if (len <= 8) {
					msg.LEN = b_endian_u32(0);
					s = send(ns, &msg, 8, 0);
				} else {
					msg.LEN = b_endian_u32(len);
					s = send(ns, &msg, 8, 0);
					s = send(ns, hdr->AS.rawEventData, len, 0);
				}
			}

			else if ((STATUS == STATE_OPEN_WRITE_HDR) && 
				 ((msg.STATE & ~ERR_MASK) == (BSCS_VERSION_01 | BSCS_PUT_FILE | STATE_OPEN_WRITE_HDR))) 
			{	
/****************************************************************************************
	PUT FILE 
 ****************************************************************************************/

 
				//case BSCS_PUT_FILE:  	// send header information
				uint32_t errcode = 0; 

				c64ta(ID, filename); 
				char *f2 = (char*)malloc(strlen(fullfilename)+5); 
				strcpy(f2,fullfilename); 
				strcat(f2,".tmp"); 

				/*********** temporary file - not checked *********/
				int sdo = open(f2,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); 
				
				HDRTYPE *hdr = constructHDR(0,0);
				hdr->FileName = f2;
#ifdef WITH_PDP 
				hdr->AS.Header = (uint8_t*) malloc(LEN+1);
#endif

				const int BUFLEN = 1024;
				char buf[BUFLEN]; 

				count  = 0; 
				while (count<LEN) {
					size_t len = recv(ns, buf, min(LEN-count, BUFLEN), 0);
#ifdef WITH_PDP 
					memcpy(hdr->AS.Header+count,buf,len);
#endif
					count += write(sdo, buf, len);
					//fprintf(stdout,"\b\b\b\b%02i%% ",100.0*count/LEN);
				}
				close(sdo);
				hdr->AS.Header[count]=0;
				hdr->HeadLen = count;

				if (LEN-count) {
					errcode = 1;
//fprintf(stdout,"errcode=1 %i\n",LEN-count);
				}
				else {
				/************ read temporary file, ... ************/
				int status = 0;
#ifdef WITH_PDP 
				sopen_pdp_read(hdr);
				
				sread(NULL,0,hdr->NRec,hdr);	// rawdata -> data.block
				status=serror();

				if (status) {
					errcode = 2;
#else
        				{
#endif
					sopen(f2,"r",hdr);
					if (status=serror()) 
					        errcode = 11;
					
					else {
						count = sread(NULL,0,hdr->NRec,hdr);
						if (status=serror()) 
							errcode = 12;
						
						sclose(hdr);
						if (status=serror()) 
							errcode = 13;
						
					}
				}

				if (VERBOSE_LEVEL>7) fprintf(stdout,"put file: errcode=%i\n",errcode); 

				/********* ... and convert to GDF file *************/
				if (!status) {
					hdr->FileName = fullfilename;
					hdr->TYPE = GDF;
					hdr->VERSION = 2; 
					sopen(fullfilename,"w",hdr);
					if (status=serror()) {
						errcode = 21;
					} else {
						//count = swrite(hdr->data.block, hdr->NRec, hdr);
						ifwrite(hdr->AS.rawdata,hdr->AS.bpb,hdr->NRec,hdr);
						if (status=serror())
							errcode = 22;
						sclose(hdr);
						if (status=serror())
							errcode = 23;
					}	
				}

				if (VERBOSE_LEVEL>7) fprintf(stdout,"put file: errcode=%i\n",errcode); 

				}
				destructHDR(hdr);
				free(f2);

				STATUS = STATE_INIT;
				msg.STATE = BSCS_VERSION_01 | BSCS_PUT_FILE | BSCS_REPLY | STATE_INIT | b_endian_u32(errcode);
				msg.LEN = b_endian_u32(0);
				s = send(ns, &msg, 8, 0);
			}

			else if (( 
				 (msg.STATE & ~ERR_MASK & ~STATE_MASK) == (BSCS_VERSION_01 | BSCS_GET_FILE))) 
			{	
/****************************************************************************************
	GET FILE 
 ****************************************************************************************/

	   			count = recv(ns, &msg.LOAD, 8, 0);
				ID = leu64p(&msg.LOAD);
				c64ta(ID, filename); 

				if (VERBOSE_LEVEL>7) fprintf(stdout,"get file: %s\n",filename); 

fprintf(stdout,"get file: %016lx\n",ID);

				int sdi = open(fullfilename,O_RDONLY,S_IFREG);

fprintf(stdout,"get file: %016lx %i\n",ID,sdi);

				if (sdi<0) {
					msg.STATE = BSCS_VERSION_01 | BSCS_GET_FILE | BSCS_REPLY | STATUS | BSCS_ERROR_CANNOT_OPEN_FILE;
					msg.LEN = b_endian_u32(0);
					s = send(ns, &msg, 8, 0);	
				} 
				else {		
					struct stat FileBuf;
					stat(fullfilename,&FileBuf);
					uint32_t LEN = FileBuf.st_size;

					msg.STATE = BSCS_VERSION_01 | BSCS_GET_FILE | BSCS_REPLY | STATUS | BSCS_NO_ERROR;
					msg.LEN = b_endian_u32(LEN);
					s = send(ns, &msg, 8, 0);	

					const int BUFLEN = 1024;
					char *buf[BUFLEN]; 
					count = 0; 
					while (count<LEN) {
						size_t len = read(sdi, buf, min(LEN-count,BUFLEN));
						count += send(ns, buf, len, 0); 
					}
					//if (FileBuf.st_size-count); // TODO: error handling
					close(sdi); 
				}
				s = send(ns, &msg, 8, 0);	
			}

			else if ((msg.STATE & CMD_MASK) == BSCS_NOP) 
			{	
/****************************************************************************************
	NO OPERATION
 ****************************************************************************************/
				;
			}


			else if (msg.STATE & BSCS_REPLY) 
			{	// ignore reply messages
				;
	   		} 
			else 
			{
				fprintf(stdout,"unknown packet: state=%08x len=%i\n",b_endian_u32(msg.STATE),b_endian_i32(msg.LEN)); 
				msg.STATE = BSCS_VERSION_01 | BSCS_ERROR;
				msg.LEN = b_endian_u32(0);
				s = send(ns, &msg, 8, 0);	
			}
		}
}

//int main (int argc, char *argv[]) {
int main () {

	int sd, ns; 		
	socklen_t fromlen;
	int addrlen;
	struct sigaction sa;
	time_t timer;	
	pid_t sid; 

	fprintf(stdout,"01## errno=%i %s\n",errno,strerror(errno)); 
	LoadIdTable(path); 
	fprintf(stdout,"11## errno=%i %s\n",errno,strerror(errno)); 

	timer=time(NULL);	
	char *t = asctime(localtime(&timer));
	t[24]=0;

	FILE *fid = fopen(LOGFILE,"a");
	fprintf(fid,"%s\tserver started\n",t); 
	fclose(fid); 


#ifdef IPv6	// IPv4 and IPv6
	int status;
	struct addrinfo hints, *servinfo, *p;
    	struct sockaddr_storage sain;
    	char s[INET6_ADDRSTRLEN];
    	int rv;
    	int yes=1;


	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
   	hints.ai_flags = AI_PASSIVE; // use my IP


	// get ready to connect
	status = getaddrinfo(NULL, "SERVER_PORT", &hints, &servinfo);
   	if(status<0) {
	        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	   	return(BSCS_UNKNOWN_HOST);
	}
   	if(servinfo==NULL) return(BSCS_UNKNOWN_HOST);

    	// loop through all the results and bind to the first we can
    	for(p = servinfo; p != NULL; p = p->ai_next) {
        	if ((sd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
	            perror("server: socket");
        	    continue;
        	}

	        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
            		exit(1);
        	}

        	if (bind(sd, p->ai_addr, p->ai_addrlen) == -1) {
            		close(sd);
            		perror("server: bind");
            		continue;
        	}
        	break;
    	}
    	if (p == NULL)  {
        	fprintf(stderr, "server: failed to bind\n");
		return(BSCS_CANNOT_BIND_PORT);
    	}

    	freeaddrinfo(servinfo); // all done with this structure


#else

    	struct sockaddr_in sain;
    	fromlen = sizeof(sain); 

    	/*
    	 * Get a socket to work with.  This socket will
    	 * be in the UNIX domain, and will be a
    	 * stream socket.
    	 */
    	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        	perror("server: socket");
        	exit(1);
    	}

   	/* bind server port */
    	sain.sin_family = AF_INET;
    	sain.sin_addr.s_addr = htonl(INADDR_ANY);
    	sain.sin_port = htons(SERVER_PORT);

   	if(bind(sd, (struct sockaddr *) &sain, sizeof(sain))<0) {
      		perror("cannot bind port ");
      		return(-1); 
   	}

#endif 

    	/*
    	 * Listen on the socket.
    	 */
    	if (listen(sd, 10) < 0) {
        	perror("server: listen");
        	exit(1);
    	}

 	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

    	while(1) {
    		/*
    		 * Accept connections.  When we accept one, ns
	    	 * will be connected to the client.  fsain will
    		 * contain the address of the client.
	    	 */
	    	fromlen = sizeof(sain); 	
		if ((ns = accept(sd, (struct sockaddr*)&sain, &fromlen)) < 0) {
        		perror("server: accept");
	        	exit(1);
    		}
#ifdef IPv6
		inet_ntop(sain.ss_family, get_in_addr((struct sockaddr *)&sain),s, sizeof s);
	        printf("server: got connection from %s\n", s);
#else 
		char hostname[100],service[100];
  		bzero(hostname,100);
  		bzero(service,100);
  		addrlen = sizeof(struct sockaddr);
		if (getnameinfo((struct sockaddr *)&sain,addrlen,hostname,100,service,100,0) != 0)
     		{
		fprintf(stdout,"--- errno=%i %s\n",errno,strerror(errno)); 
//    			perror("getnameinfo");
     		}

		printf("Connection received from host (%s) on remote port (%s)\n",hostname,service);
#endif

#ifndef VERBOSE_LEVEL
//VERBOSE_LEVEL = 8; 
#endif 

		// TODO: LOG FILE 

		char dst[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET, &sain.sin_addr,dst, INET6_ADDRSTRLEN);
		timer=time(NULL);	
		char *t = asctime(localtime(&timer));
		t[24]=0;


		FILE *fid = fopen(LOGFILE,"a");
		fprintf(fid,"%s\t%s\t%s\n",t,dst,hostname); 
		fclose(fid); 
		
	        if (!fork()) { // this is the child process
			close(sd); // child doesn't need the listener

			/* Change the file mode mask */
			umask(0);

			/* Change the current working directory.  This prevents the current
   			   directory from being locked; hence not being able to remove it. */

			if ((chdir("/")) < 0) {
				syslog( LOG_ERR, "unable to change directory to %s, code %d (%s)","/", errno, strerror(errno) );
			        exit(-1);
			}
			    
			/* Redirect standard files to /dev/null */

			freopen( "/dev/null", "r", stdin);
			freopen( "/dev/null", "w", stdout);
			freopen( "/dev/null", "w", stderr);
			
			if (VERBOSE_LEVEL>7) fprintf(stdout,"server123 err=%i %s\n",errno,strerror(errno));

			errno = 0; 
			DoJob(ns);
	
		    	close(ns);
        	    	exit(0);
	        }
    		close(ns);
	} 	

    	close(sd);
    	exit(0);
}
