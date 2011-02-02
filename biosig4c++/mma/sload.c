/*
% $Id$
% Copyright (C) 2010, 2011 Alois Schloegl <a.schloegl@ieee.org>
% This file is part of the "BioSig for C/C++" repository 
% (biosig4c++) at http://biosig.sf.net/ 

    This program is free software; you can redistribute it and/or
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

#include "../biosig.h"
#include "mathlink.h"

#define VERBOSE_LEVEL 0

void sload(const char *fn) {
	HDRTYPE *hdr = NULL;

if (VERBOSE_LEVEL > 5)
	fprintf(stdout,"=== start sload ===\n");

	// ********* open file and read header ************
	hdr = sopen(fn, "r", hdr);
	if (serror()) {
		fprintf(stdout,"Cannot open file <%s>\n", fn);
		return;
	}

if (VERBOSE_LEVEL > 5)
	fprintf(stdout,"open filename <%s>NoOfChans=%i\n", fn, hdr->NS);

	// ********** read data ********************
	hdr->FLAG.ROW_BASED_CHANNELS = 1;
	sread(NULL, 0, hdr->NRec*hdr->SPR, hdr);

#ifdef _WIN32
	long int sz[2];
#else
	size_t sz[2];
#endif
	sz[0] = hdr->data.size[1];
	sz[1] = hdr->data.size[0];
	if (!MLPutRealArray(stdlink, hdr->data.block, sz, NULL, 2))	
		fprintf(stdout,"unable to send the double array\n");

if (VERBOSE_LEVEL > 5) {
	int k;
	for (k=0;k<hdr->NS;k++)
		fprintf(stdout,"%f ",hdr->data.block[k]);
	fprintf(stdout,"\n\nopen filename <%s>@%p sz=[%i,%i]\n", fn, hdr->data.block, hdr->data.size[0], hdr->data.size[1]);
	}

	// *********** close file *********************
	sclose(hdr);
	destructHDR(hdr);
	return;
}



int main(int argc, char *argv[]) {
   return MLMain(argc, argv);
}



