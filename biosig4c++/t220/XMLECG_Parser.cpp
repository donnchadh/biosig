/*
---------------------------------------------------------------------------
Copyright (C) 2005-2006  Franco Chiarugi
Developed at the Foundation for Research and Technology - Hellas, Heraklion, Crete

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
---------------------------------------------------------------------------
*/
//////////////////////////////////////////////////////////////////////
//
// XMLECG_Parser.cpp: implementation of the cXML_Parser class.
// -------- Draft, just an example --------
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "XMLECG_Parser.h"

cXML_Parser::cXML_Parser()
{
}

cXML_Parser::~cXML_Parser()
{
}

int16_t cXML_Parser::Initialize()
{
	// All necessary initializations

	// If no errors
	return (0);
}

int16_t cXML_Parser::Close()
{
	// All necessary cleanups

	// If no errors
	return (0);
}

/***********************************************************************
	ParseECGInfo

	RETURN CODE:
	 0	-> SUCCESS
	-1	-> Error
 ***********************************************************************/

int16_t cXML_Parser::ParseECGInfo(int8_t* InFile)
{
	if (InFile == NULL)
		return (-1);

	// Get All the ECG Data
	// Fill in the XMLaECGParsedData structure

	// If no errors
	return (0);
}                                     
