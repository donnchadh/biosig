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

#include <asn_application.h>
//#include <asn_internal.h>	/* for _ASN_DEFAULT_STACK_MAX */

#include "../biosig-dev.h"

#ifdef WITH_ASN1
#include "SessionArchiveSection.h"

#endif


#ifdef __cplusplus
extern "C" {
#endif 


//int sopen_fef_read(HDRTYPE* hdr) {
int sopen_asn1(HDRTYPE* hdr) {

	static asn_TYPE_descriptor_t *pduType = &asn_DEF_SessionArchiveSection;
	SessionArchiveSection_t *SAS = 0;
	void *structure;	/* Decoded structure */

	asn_dec_rval_t rval;

	rval = ber_decode(0, pduType, (void **)&structure, hdr->AS.Header+32, hdr->HeadLen-32);

	if(errno) {
		/* Error message is already printed */
		exit(-1);
	}

	/* Check ASN.1 constraints */
	char errbuf[128];
	size_t errlen = sizeof(errbuf);
	if(asn_check_constraints(pduType, structure,
		errbuf, &errlen)) {
		fprintf(stderr, "SOPEN_FEF_READ: ASN.1 constraint "
			"check failed: %s\n", errbuf);
		exit(-1);
	}

	asn_fprint(stdout, pduType, structure);
	ASN_STRUCT_FREE(*pduType, structure);

}


