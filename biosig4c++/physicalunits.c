/*

    $Id: physicalunits.c,v 1.1 2008-06-18 19:33:04 schloegl Exp $
    Copyright (C) 2008 Alois Schloegl <a.schloegl@ieee.org>
    This file is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 
 

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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "biosig.h"

#ifndef INF
#define INF (1.0/0.0)
#endif 

int main(int argc, char **argv){
    
    	if (argc<1) return(0);

	int k=1;
    	if (!strcmp(argv[k],"-v") || !strcmp(argv[k],"--version") )
    	{
		fprintf(stdout,"physicalunits (BioSig4C++) v0.64\n");
		fprintf(stdout,"Written by Alois Schloegl\n\n");
		fprintf(stdout,"This program is free software; you can redistribute it and/or modify\n");
		fprintf(stdout,"it under the terms of the GNU General Public License as published by\n");
		fprintf(stdout,"the Free Software Foundation; either version 3 of the License, or\n");
		fprintf(stdout,"(at your option) any later version.\n");
	}	
    	else if (!strcmp(argv[k],"-h") || !strcmp(argv[k],"--help") )
    	{
		fprintf(stdout,"PHYSICALUNITS encodes and decodes physical dimensions \n");
		fprintf(stdout,"according to ISO/DIS 11073-10101:2003. ");
		fprintf(stdout,"The lower 5 bits \nencode the prefix, the upper 11 bits are used to encode the base unit.\n");
		fprintf(stdout,"\nusage: physicalunits p.u. \n");
		fprintf(stdout,"  returns the code for physical unit p.u.");
		fprintf(stdout,"\nusage: physicalunits NNN \n");
		fprintf(stdout,"  returns the physical unit encoded by NNN.\n");
		fprintf(stdout,"\nExample(s):");
		fprintf(stdout,"\n    physicalunits \"mA m-1\" returns\n");
		fprintf(stdout,"\tPhysDimCode(mA m-1) => 4242 (0x1092)");
		fprintf(stdout,"\n    physicalunits 4242 returns\n");
		fprintf(stdout,"\tPhysDim(4242) = \"mA m-1\"\n\n");
		return(0);
	}	
    	else if (!strncmp(argv[k],"-",1))  	{
	     	fprintf(stderr,"error: unknown option %s .\n",argv[k]);
	     	return(0);
	}
    	else {
    		int c,q;
		q=sscanf(argv[k],"%i ",&c);
		char s[MAX_LENGTH_PHYSDIM+1]; 
    		if (q>0) {
    			fprintf(stdout,"PhysDim(%i) = \"%s\"\n",c,PhysDim(c,s));
    		}
    		else {
    			c = PhysDimCode(argv[k]);
    			fprintf(stdout,"PhysDimCode(%s) => %i (0x%04x)\n",argv[k],c,c);
    		}
	}
}
