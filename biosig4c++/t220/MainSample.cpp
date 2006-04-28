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
// MainSample.cpp (SCPECG_writer.cpp)
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SCPECG_Writer.h"
#include "Section1_Info.h"
#include "ECGSignal_Info.h"
#include "XMLECG_Parser.h"
#include "SCP_Formatter.h"

cXML_Parser* XML_Parser = NULL;
cSCP_Formatter* SCP_Formatter = NULL;

int main(long, char*[]);
int16_t WriteSCPECGFile(int8_t*);
void RLTrim(int8_t*);

int main(long argc, char* argv[])
{
	if (argc != 2)
		return -1;

	if(WriteSCPECGFile(argv[1]) != 0)
		return -2;

	return 0;
}

int16_t WriteSCPECGFile(int8_t* InFile)
{
	FILE* fh;
	int8_t InFile1[2048];
	int8_t OutFile[2048];

	// Check parameter correctness
	if (InFile == NULL)
		return (-1);

	if (strlen(InFile) > 2046)
		return (-1);

	// Infile and Outfile
	strcpy(InFile1, InFile);
	RLTrim(InFile1);

	strcpy(OutFile, InFile1);
	strcat(OutFile, "w");

	// Check file existance
	fh = fopen(InFile1, "rb");
	if (fh == NULL) {
		return (-2);
	}
	fclose(fh);

	XML_Parser = new cXML_Parser();

	if (XML_Parser->Initialize()) {
		delete XML_Parser;
		return (-3);
	}

	if (XML_Parser->ParseECGInfo(InFile1) != 0) {
		XML_Parser->Close();
		delete XML_Parser;
		return (-4);
	}

	SCP_Formatter = new cSCP_Formatter();
	SCP_Formatter->ResetInfo();
	if (SCP_Formatter->LoadXMLInfo(&XML_Parser->XMLaECGParsedData) != 0) {
		XML_Parser->Close();
		delete XML_Parser;
		delete SCP_Formatter;
		return (-5);
	}
	if (SCP_Formatter->DoTheSCPFile(OutFile) != 0) {
		XML_Parser->Close();
		delete XML_Parser;
		delete SCP_Formatter;
		return (-6);	// Errors -1, -2, ..., -15. Problems in Outfile = NULL, structures not initialized or error during the creation of the SCP file.
	}

	XML_Parser->Close();

	delete XML_Parser;
	delete SCP_Formatter;

	return (0);
}                                     

void RLTrim(int8_t* fnam)
{
	int16_t i, j, len;

	// Left trim
	len = strlen(fnam);
	i = 0;
	if(len > 0) {
		while(fnam[i] == ' ') {
			if(i == len)
				break;
			i++;
		}
	}
	if(i != 0) {
		for(j = i; j <= len; j++) {
			fnam[j - i] = fnam[j];
		}
	}

	// Right trim
	len = strlen(fnam);
	i = len - 1;
	if(len > 0) {
		while(fnam[i] == ' ') {
			if(i == 0)
				break;
			i--;
		}
	}
	fnam[i + 1] = '\0';
}
