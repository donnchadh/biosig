/*

    sandbox is used for development and under constraction work
    The functions here are either under construction or experimental. 
    The functions will be either fixed, then they are moved to another place;
    or the functions are discarded. Do not rely on the interface in this function
       	

    $Id: sandbox.c,v 1.5 2009/04/16 20:19:17 schloegl Exp $
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../biosig-dev.h"

#ifdef WITH_ASN1
#include "SessionArchiveSection.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif 


void sopen_fef_read(HDRTYPE* hdr) {

#ifndef WITH_ASN1

	B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	B4C_ERRMSG = "ASN1/FEF currently not supported";

#else 
	static asn_TYPE_descriptor_t *pduType = &asn_DEF_SessionArchiveSection;
	SessionArchiveSection_t *SAS = 0; 	/* Decoded structure */
	FEFString_t *fefstr = 0;
	asn_codec_ctx_t *opt_codec_ctx = 0;

	asn_dec_rval_t rval;

if (VERBOSE_LEVEL>7) 
	fprintf(stdout,"ASN1: BER DECODING\n");
	
	size_t pos =32;	
	rval = ber_decode(0, pduType, (void **)&SAS, hdr->AS.Header+32, hdr->HeadLen-32);

	pos += rval.consumed;
/*
	while (pos<hdr->HeadLen) {

fprintf(stdout,"%i/%i\n",pos,hdr->HeadLen);
	
		rval = ber_decode(opt_codec_ctx, pduType, (void **)&SAS, hdr->AS.Header+pos, hdr->HeadLen-32);
		pos += rval.consumed;
	}
*/
if (VERBOSE_LEVEL>7) 
	fprintf(stdout,"%i/%i\nASN1: BER DECODING DONE\n",pos,hdr->HeadLen);

if (VERBOSE_LEVEL>8) {
//	asn_fprint(stdout, &asn_DEF_SessionArchiveSection, SAS);

//	SAS->s_archive_id
	asn_fprint(stdout, &asn_DEF_FEFString, &SAS->s_archive_id);
	asn_fprint(stdout, &asn_DEF_FEFString, &SAS->s_archive_name);
	asn_fprint(stdout, &asn_DEF_FEFString, SAS->s_archive_comments);

	/**********************************
		Manufacturer information 
	 **********************************/	
	asn_fprint(stdout, &asn_DEF_ManufacturerSpecificSection, SAS->manufacturerspecific);

	/**********************************
		Health Care provider
	 **********************************/	
	asn_fprint(stdout, &asn_DEF_HealthCareProviderSection, SAS->healthcareprovider);
	asn_fprint(stdout, &asn_DEF_PatientDemographicsSection, &SAS->demographics);
}

	/**********************************
		demographic information 
	 **********************************/	
	{ 
	long ival; 
	if (!asn_INTEGER2long(SAS->demographics.sex, &ival))
		hdr->Patient.Sex = ival;
	}

	{
	double val; 
	if (!asn_REAL2double(&SAS->demographics.patientweight->value, &val))
		hdr->Patient.Weight = (uint8_t)val;
	if (!asn_REAL2double(&SAS->demographics.patientheight->value, &val))
		hdr->Patient.Height = (uint8_t)val;

	strncpy(hdr->Patient.Name,SAS->demographics.characternamegroup->givenname.buf,MAX_LENGTH_NAME);
	strncat(hdr->Patient.Name," ",MAX_LENGTH_NAME);
	strncat(hdr->Patient.Name,SAS->demographics.characternamegroup->middlename.buf,MAX_LENGTH_NAME);
	strncat(hdr->Patient.Name," ",MAX_LENGTH_NAME);
	strncat(hdr->Patient.Name,SAS->demographics.characternamegroup->familyname.buf,MAX_LENGTH_NAME);
	}
	
	/**********************************
		Manufacturer information 
	 **********************************/	

hdr2ascii(hdr,stdout,2);

	if(errno) {
		/* Error message is already printed */
		exit(-1);
	}

	/* Check ASN.1 constraints */
	char errbuf[128];
	size_t errlen = sizeof(errbuf);
	if(asn_check_constraints(pduType, SAS,
		errbuf, &errlen)) {
		fprintf(stdout, "SOPEN_FEF_READ: ASN.1 constraint "
			"check failed: %s\n", errbuf);
	}

//	asn_fprint(stdout, pduType, structure);
	ASN_STRUCT_FREE(*pduType, SAS);

#endif 

}


