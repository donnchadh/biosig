/*

    $Id: biosig-network.c,v 1.4 2009-02-16 16:59:33 schloegl Exp $
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

//#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "biosig-network.h"


int c64ta(uint64_t ID, char *txt) {
#if (__SIZEOF_INT__==4)
	const char hex[] = "0123456789abcdef";
	for (int k=0; k<16; k++) {
		txt[k] = hex[ID & 0x0f]; 
		ID>>=4;
	}	
//	fprintf(stdout,"c64ta8: %016lx.gdf\n",ID);
	sprintf(txt,"%016Lx.gdf",ID);
#else 
//	fprintf(stdout,"c64ta9: %016Lx.gdf\n",ID);
	sprintf(txt,"%016lx.gdf",ID);
#endif
}

int cat64(char* txt, uint64_t *ID) {
	*ID = 0; 
	sscanf(txt,"%16lx",ID);
}


uint32_t SERVER_STATE; // state of server, useful for preliminary error checking 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


/****************************************************************************************
	OPEN CONNECTION TO SERVER
 ****************************************************************************************/
int bscs_connect(char* hostname) {

	int sd; 
    	struct sockaddr_in localAddr;

	if (hostname==NULL) hostname = "129.27.3.99"; 
	
#if 0	// IPv4 and IPv6
	int status;
	struct addrinfo hints, *servinfo, *p;
    	char s[INET6_ADDRSTRLEN];


	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	// get ready to connect
	status = getaddrinfo(hostname, "SERVER_PORT", &hints, &servinfo);
   	if(status) {
	        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
	   	return(BSCS_UNKNOWN_HOST);
   	}
   	if(servinfo==NULL) return(BSCS_UNKNOWN_HOST);

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
        	char hostname1[NI_MAXHOST] = "";
        	int error = getnameinfo(p->ai_addr, p->ai_addrlen, hostname1, NI_MAXHOST, NULL, 0, 0); 
        	if (*hostname1)
        		printf("hostname: %s\n", hostname1);

		if ((sd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
		if (connect(sd, p->ai_addr, p->ai_addrlen) == -1) {
#ifndef _WIN32		
			close(sd);
#else
			closesocket(sd);
#endif
			perror("client: connect");
			continue;
		}
	        break;
    	}
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return(BSCS_CANNOT_CONNECT);
	}

    	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

#else 

	// IPv4 only 	
    	struct hostent *h;
    	struct sockaddr_in sain;

	
   	h = gethostbyname(hostname);
   	if(h==NULL) return(BSCS_UNKNOWN_HOST);

   	sain.sin_family = h->h_addrtype;
   	memcpy((char *) &sain.sin_addr.s_addr, h->h_addr_list[0],h->h_length);
   	sain.sin_port = htons(SERVER_PORT);

   	/* create socket */
   	sd = socket(AF_INET, SOCK_STREAM, 0);
   	if(sd<0) return(BSCS_CANNOT_OPEN_SOCKET);

   	/* bind any port number */
   	localAddr.sin_family = AF_INET;
   	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   	localAddr.sin_port = htons(0);
            
   	/* connect to server */
   	int rc = connect(sd, (struct sockaddr *) &sain, sizeof(sain));
   	if(rc<0) return(BSCS_CANNOT_CONNECT);

#endif 
   
	/* identification */	
	mesg_t msg; 
	recv(sd,&msg,8,0);
	int len = b_endian_u32(msg.LEN); 	
	if ((msg.STATE & (VER_MASK|CMD_MASK)) != (BSCS_VERSION_01 | BSCS_SEND_MSG))	// currently only V0.1 is supported 
	{
		close(sd);
		return(BSCS_SERVER_NOT_SUPPORTED);
	}
	
	char *greeting = (char*)malloc(len+1); 
	recv(sd,greeting,len,0);
	greeting[len]=0; 
	fprintf(stdout,"%s",greeting);
	free(greeting);
   	return(sd);
}



/****************************************************************************************
	CLOSE CONNECTION TO SERVER
 ****************************************************************************************/
int bscs_disconnect(int sd) {
	close(sd); 
}


int send_packet(int sd, uint32_t state, uint32_t len, void* load)
{
	mesg_t msg; 
	msg.STATE = state; 
	msg.LEN = b_endian_u32(len); 
	send(sd,&msg,8,0);
	if (len>0) send(sd,load,len,0);
}

/****************************************************************************************
	OPEN FILE with ID 
 ****************************************************************************************/
int bscs_open(int sd, uint64_t* ID) {

	if (SERVER_STATE != STATE_INIT) return(BSCS_ERROR);

	size_t LEN;
	mesg_t msg; 
	
	if (*ID==0) {
		msg.STATE = BSCS_VERSION_01 | BSCS_OPEN_W | STATE_INIT | BSCS_NO_ERROR;
		LEN   = 0;
	}
	else {
		msg.STATE = BSCS_VERSION_01 | BSCS_OPEN_R | STATE_INIT | BSCS_NO_ERROR;
		*(uint64_t*)&msg.LOAD  = leu64p(ID);
		LEN = BSCS_ID_BITLEN>>3;
	}	
if (VERBOSE_LEVEL>8) fprintf(stdout,"open: %16lx %16lx\n",*ID,msg.LOAD);
	msg.LEN   = b_endian_u32(LEN);
	int s = send(sd, &msg, LEN+8, 0);	

	// wait for reply 
	ssize_t count = 0; 
	count = recv(sd, &msg, 8, 0);
	LEN = b_endian_u32(msg.LEN);
	SERVER_STATE = msg.STATE & STATE_MASK;

if (VERBOSE_LEVEL>8) fprintf(stdout,"BSCS_OPEN %i:%i: ID=%16Lx LEN=%x STATE=0x%08x\n",s,count,*ID,msg.LEN,b_endian_u32(msg.STATE));

	if ((*ID==0) && (LEN==8) && (msg.STATE==(BSCS_VERSION_01 | BSCS_OPEN_W | BSCS_REPLY | STATE_OPEN_WRITE_HDR | BSCS_NO_ERROR)) ) 
	{
if (VERBOSE_LEVEL>8) fprintf(stdout,"123\n");
		// write access: get new ID
		count += recv(sd, ID, LEN, 0);
if (VERBOSE_LEVEL>8) fprintf(stdout,"123 %i:\n",count);
		*ID = l_endian_u64(*ID);
		return(0);
	}
	
	if ((*ID != 0) && (LEN==0) && (msg.STATE==(BSCS_VERSION_01 | BSCS_OPEN_R | BSCS_REPLY | STATE_OPEN_READ | BSCS_NO_ERROR)) ) 
	{
if (VERBOSE_LEVEL>8) fprintf(stdout,"124\n");
		; // read access 
		return(0); 
	}
	
if (VERBOSE_LEVEL>8) fprintf(stdout,"125\n");
	uint8_t buf[8];
	count = 0;
	while (LEN>count) 
		count += recv(sd, &buf, min(8,max(0,LEN-count)), 0);
	 	// invalid packet or error opening file 

	fprintf(stdout,"ERR: state= %08x %08x len=%i\n",b_endian_u32(msg.STATE),BSCS_VERSION_01 | BSCS_OPEN_R | BSCS_REPLY | STATE_OPEN_READ | BSCS_NO_ERROR,LEN);
	
	return(msg.STATE);
}

/****************************************************************************************
	CLOSE FILE 
 ****************************************************************************************/
int bscs_close(int sd) {
	int s;
	size_t LEN;
	mesg_t msg; 
	
	msg.STATE = BSCS_VERSION_01 | BSCS_CLOSE | BSCS_NO_ERROR | SERVER_STATE;
if (VERBOSE_LEVEL>8) fprintf(stdout,"close1: %08x \n",msg.STATE);
	msg.LEN   = b_endian_u32(0);
if (VERBOSE_LEVEL>8) fprintf(stdout,"close2: %08x %i %i\n",msg.STATE,sizeof(msg),msg.LEN);

	s = send(sd, &msg, 8, 0);	

if (VERBOSE_LEVEL>8) fprintf(stdout,"close3: %08x %i\n",msg.STATE,s);

	// wait for reply 
	s = recv(sd, &msg, 8, 0);
	LEN = b_endian_u32(msg.LEN);
	SERVER_STATE = msg.STATE & STATE_MASK;

if (VERBOSE_LEVEL>8) fprintf(stdout,"s=%i state= %08x len=%i %i  %08x\n",s,msg.STATE& ~STATE_MASK,LEN,s,(BSCS_VERSION_01 | BSCS_CLOSE | BSCS_REPLY));

	if ((LEN==0) && ((msg.STATE & ~STATE_MASK)==(BSCS_VERSION_01 | BSCS_CLOSE | BSCS_REPLY | BSCS_NO_ERROR)) ) 
		// close without error 
		return(0);

	if ((LEN==0) && ((msg.STATE & ~STATE_MASK & ~ERR_MASK)==(BSCS_VERSION_01 | BSCS_CLOSE | BSCS_REPLY)) ) 
		// close with error 
		return(msg.STATE & ERR_MASK);

	 	// invalid packet or error opening file 
	fprintf(stdout,"ERR: state= %08x len=%i\n",msg.STATE,LEN);
	return(msg.STATE);
}

/****************************************************************************************
	REQUEST HEADER
 ****************************************************************************************/
int bscs_send_hdr(int sd, HDRTYPE *hdr) {
/* hdr->AS.Header must contain GDF header information 
   hdr->HeadLen   must contain header length 
  -------------------------------------------------------------- */
	// ToDo: convert HDR into AS.Header

	if (SERVER_STATE != STATE_OPEN_WRITE_HDR) return(BSCS_ERROR);

	mesg_t msg; 
	size_t LEN;

	hdr->TYPE = GDF;
	LEN = struct2gdfbin(hdr); 
	//hdr2ascii(hdr,stdout,3);

	msg.STATE = BSCS_VERSION_01 | BSCS_SEND_HDR | STATE_OPEN_WRITE_HDR | BSCS_NO_ERROR;
	msg.LEN   = b_endian_u32(hdr->HeadLen);
	int s = send(sd, &msg, 8, 0);	
if (VERBOSE_LEVEL>8) fprintf(stdout,"SND HDR  %i %i %i\n",LEN,hdr->HeadLen,s); 
	s = send(sd, hdr->AS.Header, hdr->HeadLen, 0);	

if (VERBOSE_LEVEL>8) fprintf(stdout,"SND HDR %i %i\n",hdr->HeadLen,s); 

	// wait for reply 
	ssize_t count = recv(sd, &msg, 8, 0);

if (VERBOSE_LEVEL>8) fprintf(stdout,"SND HDR %i %i %i %08x\n",hdr->HeadLen,s,count,msg.STATE); 

	LEN = b_endian_u32(msg.LEN);
	SERVER_STATE = msg.STATE & STATE_MASK;

	if ((LEN==0) && (msg.STATE==(BSCS_VERSION_01 | BSCS_SEND_HDR | BSCS_REPLY | STATE_OPEN_WRITE | BSCS_NO_ERROR)) ) 
		// close without error 
		return(0);

	if ((LEN==0) && ((msg.STATE & ~ERR_MASK)==(BSCS_VERSION_01 | BSCS_SEND_HDR | BSCS_REPLY | STATE_OPEN_WRITE_HDR)) ) 
		// could not write header  
		return((msg.STATE & ~ERR_MASK) | BSCS_ERROR_COULD_NOT_WRITE_HDR);

	 	// invalid packet or error opening file 
	return(msg.STATE);
}

/****************************************************************************************
	SEND DATA
 ****************************************************************************************/
int bscs_send_dat(int sd, void* buf, size_t len ) {

	if (SERVER_STATE != STATE_OPEN_WRITE) return(BSCS_ERROR);

	size_t LEN;
	mesg_t msg; 
	msg.STATE = BSCS_VERSION_01 | BSCS_SEND_DAT | STATE_OPEN_WRITE | BSCS_NO_ERROR;
	msg.LEN   = b_endian_u32(len);
	send(sd, &msg, 8, 0);	
	send(sd, buf, len, MSG_EOR);	

	// wait for reply 
	ssize_t count = recv(sd, &msg, 8, 0);
	LEN = b_endian_u32(msg.LEN);
	SERVER_STATE = msg.STATE & STATE_MASK;

	if ((LEN==0) && (msg.STATE==(BSCS_VERSION_01 | BSCS_SEND_DAT | BSCS_REPLY | STATE_OPEN_WRITE | BSCS_NO_ERROR)) ) 
		// close without error 
		return(0);

	if (LEN>0) 
		return (BSCS_VERSION_01 | BSCS_SEND_DAT | BSCS_REPLY | STATE_OPEN_WRITE | BSCS_INCORRECT_REPLY_PACKET_LENGTH);

	if ((msg.STATE & ~ERR_MASK)==(BSCS_VERSION_01 | BSCS_SEND_DAT | BSCS_REPLY | STATE_OPEN_WRITE)) 
		// could not write header  
		return ((msg.STATE & ~ERR_MASK) | BSCS_ERROR_COULD_NOT_WRITE_DAT);

 	// invalid packet or error opening file 
	return(msg.STATE);
}

/****************************************************************************************
	SEND EVENTS
 ****************************************************************************************/
int bscs_send_evt(int sd, HDRTYPE *hdr) {

	if (SERVER_STATE != STATE_OPEN_WRITE) return(BSCS_ERROR);

	size_t N = hdr->EVENT.N;
	int sze;
	char flag;

	size_t LEN;
	mesg_t msg; 

	if ((hdr->EVENT.DUR!=NULL) && (hdr->EVENT.CHN!=NULL)) {
		sze = 12; 
		flag = 3; 
	} else {
		sze = 6; 
		flag = 1;
	}	

	size_t len = hdrEVT2rawEVT(hdr); 

if (VERBOSE_LEVEL>8) fprintf(stdout,"write evt: len=%i\n",len); 

	msg.STATE = BSCS_VERSION_01 | BSCS_SEND_EVT | STATE_OPEN_WRITE | BSCS_NO_ERROR;
	msg.LEN   = b_endian_u32(len);
	int s1 = send(sd, &msg, 8, 0);	
	int s2 = send(sd, hdr->AS.rawEventData, len, 0);	

if (VERBOSE_LEVEL>8) fprintf(stdout,"write evt2: %08x len=%i\n",msg.STATE,len); 
	
	// wait for reply 
	ssize_t count = recv(sd, &msg, 8, 0);
	LEN = b_endian_u32(msg.LEN);
	SERVER_STATE = msg.STATE & STATE_MASK;

if (VERBOSE_LEVEL>8) fprintf(stdout,"write evt2: %08x len=%i count=%i\n",msg.STATE,LEN,count); 
	
	if ((LEN==0) && (msg.STATE==(BSCS_VERSION_01 | BSCS_SEND_EVT | BSCS_REPLY | STATE_OPEN_WRITE | BSCS_NO_ERROR)) ) 
		// close without error 
		return(0);

	if (LEN>0) 
		return (BSCS_VERSION_01 | BSCS_SEND_EVT | BSCS_REPLY | STATE_OPEN_WRITE | BSCS_INCORRECT_REPLY_PACKET_LENGTH);

	if (msg.STATE & ERR_MASK) 
		// could not write evt  
		return (BSCS_ERROR_COULD_NOT_WRITE_EVT);

 	// invalid packet or error opening file 
	return(msg.STATE);
}

/****************************************************************************************
	REQUEST HEADER
 ****************************************************************************************/
int bscs_requ_hdr(int sd, HDRTYPE *hdr) {
	mesg_t msg; 
	int count; 
	

	if (SERVER_STATE != STATE_OPEN_READ) return(BSCS_ERROR);
	
	msg.STATE = BSCS_VERSION_01 | BSCS_REQU_HDR | BSCS_NO_ERROR | (SERVER_STATE & STATE_MASK);
	msg.LEN   = 0;
	count = send(sd, &msg, 8, 0);	

   	count = recv(sd, &msg, 8, 0);
	hdr->HeadLen   = b_endian_u32(msg.LEN);
    	hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header,hdr->HeadLen);
    	hdr->TYPE      = GDF; 
	count = 0; 
	while (hdr->HeadLen>count) {
		count += recv(sd, hdr->AS.Header+count, hdr->HeadLen-count, 0);
	}

if (VERBOSE_LEVEL>8) fprintf(stdout,"REQ HDR: %i %s\n",count,GetFileTypeString(hdr->TYPE));
	
	gdfbin2struct(hdr); 
	
	
   	return(count-hdr->HeadLen); 
}

/****************************************************************************************
	REQUEST DATA
 ****************************************************************************************/
ssize_t bscs_requ_dat(int sd, size_t start, size_t length, HDRTYPE *hdr) {
/*
	bufsiz should be equal to hdr->AS.bpb*length	
*/	
	mesg_t msg; 
	size_t LEN;
	
	if (SERVER_STATE != STATE_OPEN_READ) return(BSCS_ERROR);
	
	msg.STATE = BSCS_VERSION_01 | BSCS_REQU_DAT | BSCS_NO_ERROR | (SERVER_STATE & STATE_MASK);
	msg.LEN   = b_endian_u32(8);


	*(uint32_t*)(msg.LOAD+4) = l_endian_u32(start);	
	*(uint32_t*)(msg.LOAD+0) = l_endian_u32(length);	
	int s = send(sd, &msg, 16, 0);	
	
	ssize_t count = recv(sd, &msg, 8, 0);
	LEN = b_endian_u32(msg.LEN);
	
	hdr->AS.rawdata = (uint8_t*) realloc(hdr->AS.rawdata,LEN);
	count = 0; 
	while (LEN>count) {
		count += recv(sd, hdr->AS.rawdata+count, LEN-count, 0);
	}	
	
	hdr->AS.first = start;
if (VERBOSE_LEVEL>8) fprintf(stdout,"REQ DAT: %i %i\n",count,hdr->AS.bpb);
	hdr->AS.length= count/hdr->AS.bpb;  
if (VERBOSE_LEVEL>8) fprintf(stdout,"REQ DAT: %i %i\n",hdr->AS.first,hdr->AS.length);

	return(0);
}


/****************************************************************************************
	REQUEST EVENT TABLE 
 ****************************************************************************************/
int bscs_requ_evt(int sd, HDRTYPE *hdr) {
	mesg_t msg; 
	size_t LEN;

if (VERBOSE_LEVEL>8) fprintf(stdout,"REQ EVT %08x %08x\n",SERVER_STATE, STATE_OPEN_READ);

	if (SERVER_STATE != STATE_OPEN_READ) return(BSCS_ERROR);

	msg.STATE = BSCS_VERSION_01 | BSCS_REQU_EVT | BSCS_NO_ERROR | (SERVER_STATE & STATE_MASK);
	msg.LEN   = b_endian_u32(0);
	int s = send(sd, &msg, 8, 0);	

	// wait for reply 
	s = recv(sd, &msg, 8, 0);
	LEN = b_endian_u32(msg.LEN); 	 

if (VERBOSE_LEVEL>8) fprintf(stdout,"REQ EVT: %i %i \n",s,LEN);

	if (LEN>0) {
	    	hdr->AS.rawEventData = (uint8_t*)realloc(hdr->AS.rawEventData,LEN);
		int count = 0; 
		while (LEN>count) {
			count += recv(sd, hdr->AS.rawEventData+count, LEN-count, 0);
		}
	   	rawEVT2hdrEVT(hdr); // TODO: replace this function because it is inefficient  
   	}

if (VERBOSE_LEVEL>8) fprintf(stdout,"REQ EVT: %i %i \n",s,LEN);
#if 0 
			uint8_t *buf = hdr->AS.rawEventData; 
			if (hdr->VERSION < 1.94) {
				if (buf[1] | buf[2] | buf[3])
					hdr->EVENT.SampleRate = buf[1] + (buf[2] + buf[3]*256.0)*256.0; 
				else {
					fprintf(stdout,"Warning GDF v1: SampleRate in Eventtable is not set in %s !!!\n",hdr->FileName);
					hdr->EVENT.SampleRate = hdr->SampleRate; 
				}	
				hdr->EVENT.N = leu32p(buf + 4);
			}	
			else	{
				hdr->EVENT.N = buf[1] + (buf[2] + buf[3]*256)*256; 
				hdr->EVENT.SampleRate = lef32p(buf + 4);
			}	
			int sze = (buf[0]>1) ? 12 : 6;
	
	 		hdr->EVENT.POS = (uint32_t*) realloc(hdr->EVENT.POS, hdr->EVENT.N*sizeof(*hdr->EVENT.POS) );
			hdr->EVENT.TYP = (uint16_t*) realloc(hdr->EVENT.TYP, hdr->EVENT.N*sizeof(*hdr->EVENT.TYP) );
#endif 


	return(0);    	
}

/****************************************************************************************
	SEND MESSAGE 
 ****************************************************************************************/
int bscs_send_msg(int sd, char *mesg) {
	size_t LEN=strlen(mesg);
	mesg_t msg; 

	msg.STATE = (BSCS_VERSION_01 | BSCS_SEND_MSG | BSCS_NO_ERROR) | SERVER_STATE;
	msg.LEN   = b_endian_u32(LEN);

	int s = send(sd, &msg, 8, 0);	
	s = send(sd, mesg, LEN, 0);	
	return(s); 
}

int bscs_error(int sd, int ERRNUM, char* ERRMSG) {
	// obsolete ? 
	size_t len;
	if (ERRMSG==NULL) len = 0; 
	else len = strlen(ERRMSG); 
	uint8_t* buf = (uint8_t*)malloc(6+2+len);
	buf[0] = BSCS_VERSION; 	// version 
	buf[1] = BSCS_ERROR; 
	*(uint32_t*)(buf+4) = htonl(2+strlen(ERRMSG)); 	 
	buf[6] = ERRNUM; 
	memcpy(buf+7,ERRMSG,strlen(ERRMSG));
	buf[7+len] = 0;	// terminating \0
	int s = send(sd,buf,len+8, 0);	
	free(buf);
	return(s);  
}

