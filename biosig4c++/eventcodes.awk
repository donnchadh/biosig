#!/usr/bin/awk -f
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


BEGIN { FS="\\t" 
	#clear files 
	print "" >"eventcodes.i"	
	print "" >"eventcodegroups.i"	
}

# list of groups
/^### 0x/	{ 
	g = substr($1,7,4); 
	gsub(/_/, "0", g);
	h = substr($1,11); 
	print    "\t{ 0x" g ", \"" $2 "\" }," >> "eventcodegroups.i"
	#print "###\t0x" g ",\t\"" $2 "\"," 
} 

# list of eventcodes
/^[^#]/ { 
          print sprintf("\t%s, 0x%s, \"%s\",", $1, g, $2) >> "eventcodes.i"
} 
        
