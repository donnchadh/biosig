/*

    sandbox is used for development and under constraction work
    The functions here are either under construction or experimental. 
    The functions will be either fixed, then they are moved to another place;
    or the functions are discarded. Do not rely on the interface in this function
       	

    $Id: sandbox.c,v 1.4 2009-03-27 07:16:50 schloegl Exp $
    Copyright (C) 2008 Alois Schloegl <a.schloegl@ieee.org>
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

// these functios are stubs

int sopen_asn1(HDRTYPE* hdr) {
	B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	B4C_ERRMSG = "asn1 currently not supported";
	return(0);
};

int sopen_eeprobe(HDRTYPE* hdr) {
	B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	B4C_ERRMSG = "asn1 currently not supported";
	return(0);
};

int sopen_zzztest(HDRTYPE* hdr) {
	B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	B4C_ERRMSG = "asn1 currently not supported";
	return(0);
};

int sopen_unipro_read(HDRTYPE* hdr) {
	B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	B4C_ERRMSG = "UNIPRO not supported";
	return(0);
}

#ifdef WITH_PDP
#include "../NONFREE/sopen_pdp_read.c"
#endif 
