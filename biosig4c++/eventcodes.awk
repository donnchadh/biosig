#!/usr/bin/gawk -f
#
#    Converts eventcodex.txt into eventcodes.i and eventcodegroups.i
#
#    $Id$
#    Copyright (C) 2011 Alois Schloegl <a.schloegl@ieee.org>
#    This file is part of the "BioSig for C/C++" repository
#    (biosig4c++) at http://biosig.sf.net/
#
#    BioSig is free software; you can redistribute it and/or
#    modify it under the terms of the GNU General Public License
#    as published by the Free Software Foundation; either version 3
#    of the License, or (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.


BEGIN { FS = "\t" 
	nG = 0;
	nC = 0; 
}

# list of event groups
/^### 0x/ { 
        nG++;
	g = substr($1,7,4); 
	gsub(/_/, "0", g);
	G[nG,1] = g;
	G[nG,2] = $2;
} 

# list of event codes
/^[^#]/ { 
	if (255 < strtonum($1)) {
		# ignore user-specified events
	        nC++;
        	C[nC,1] = $1;
        	C[nC,2] = g;
        	C[nC,3] = $2;
	}
} 
        
END {
        for (i=1; i<=nG; i++) {
                printf("\t{ 0x%s, \"%s\" },\n",G[i,1],G[i,2]) | "sort > eventcodegroups.i"
        } 
       
        for (i=1; i<256; i++) {
 		# add pre-defined user-specified events 0x0001-0x00ff
               printf("\t{ 0x%04x, 0x0000, \"condition %i\" },\n",i,i) > "eventcodes.i"
        } 
        for (i=1; i<=nC; i++) {
                printf("\t{ %s, 0x%s, \"%s\" },\n",C[i,1],C[i,2],C[i,3]) | "sort >> eventcodes.i"
        } 
}

