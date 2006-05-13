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
/*
// SCP_Formatter.h: interface for the cSCP_Formatter class.
*/

#include "Section1_Info.h"
#include "ECGSignal_Info.h"
#include "SCPECG_Writer.h"


#if !defined SCP_FORMATTER
#define SCP_FORMATTER

#ifndef VS_DEF
#include "../biosig.h"
#else
#include "biosig_vs.h"
#endif

class cSCP_Formatter
{
public:
	cSection1_Info* S1I;
	cECGSignal_Info* ESI;

	SCPFILE_HEAD	FileHead;
	SCPSECT0		Sect0;

	uint8_t	Sect1[1000];
	uint8_t	Sect3[300];
	//uint8_t	Sect4[1000];
	//uint8_t	Sect5[75000];
	uint8_t	Sect6[150000];
	//uint8_t	Sect7[3000];
	//uint8_t	Sect8[3000];
	//uint8_t	Sect10[3000];

	uint8_t*	TotalFile;

	uint32_t	lenSect0;
	uint32_t	lenSect1;
	uint32_t	lenSect3;
	//uint32_t	lenSect4;
	//uint32_t	lenSect5;
	uint32_t	lenSect6;
	//uint32_t	lenSect7;
	//uint32_t	lenSect8;
	//uint32_t	lenSect10;

public:
	cSCP_Formatter(void);
	virtual ~cSCP_Formatter(void);
	void  ResetInfo();
/*
	int16_t DoTheSCPFile(int8_t*);
	int16_t SetLastName(int8_t*);
	int16_t GetLastName(int8_t*);
	int16_t SetFirstName(int8_t*);
	int16_t GetFirstName(int8_t*);
	int16_t SetPatientID(int8_t*);
	int16_t GetPatientID(int8_t*);
	int16_t SetSecondLastName(int8_t*);
	int16_t GetSecondLastName(int8_t*);
*/
	int16_t DoTheSCPFile(char*);
	int16_t SetLastName(char*);
	int16_t GetLastName(char*);
	int16_t SetFirstName(char*);
	int16_t GetFirstName(char*);
	int16_t SetPatientID(char*);
	int16_t GetPatientID(char*);
	int16_t SetSecondLastName(char*);
	int16_t GetSecondLastName(char*);

	int16_t SetDateOfBirth(int16_t, int16_t, int16_t);
	int16_t GetDateOfBirth(int16_t*, int16_t*, int16_t*);
	int16_t SetHeight(int16_t);
	int16_t GetHeight(int16_t*);
	int16_t SetWeight(int16_t);
	int16_t GetWeight(int16_t*);
	int16_t SetSex(int16_t);
	int16_t GetSex(int16_t*);
	int16_t SetSBP(int16_t);
	int16_t GetSBP(int16_t*);
	int16_t SetDBP(int16_t);
	int16_t GetDBP(int16_t*);
/*
	int16_t SetReferringPhysician(int8_t*);
	int16_t GetReferringPhysician(int8_t*);
	int16_t SetLastConfirmingPhys(int8_t*);
	int16_t GetLastConfirmingPhys(int8_t*);
*/
	int16_t SetReferringPhysician(char*);
	int16_t GetReferringPhysician(char*);
	int16_t SetLastConfirmingPhys(char*);
	int16_t GetLastConfirmingPhys(char*);

	int16_t SetStatCode(int16_t);
	int16_t GetStatCode(int16_t*);
	int16_t SetDateOfAcquisition(int16_t, int16_t, int16_t);
	int16_t GetDateOfAcquisition(int16_t*, int16_t*, int16_t*);
	int16_t SetTimeOfAcquisition(int16_t, int16_t, int16_t);
	int16_t GetTimeOfAcquisition(int16_t*, int16_t*, int16_t*);
	int16_t SetSequenceNumber(int8_t*);
	int16_t GetSequenceNumber(int8_t*);
	int16_t SetTimeZone(int16_t);
	int16_t GetTimeZone(int16_t*);
	int16_t LoadXMLInfo(HDRTYPE*);

private:
	bool CreateSCPFileHeaderDraft(void);
	bool CreateSCPSection0Draft(void);
	bool CreateSCPSection1(void);
	bool CreateSCPSection3(void);
	//bool CreateSCPSection4(void);
	//bool CreateSCPSection5(void);
	bool CreateSCPSection6(void);
	//bool CreateSCPSection7(void);
	//bool CreateSCPSection8(void);
	//bool CreateSCPSection10(void);
	bool CorrectSCPSection0(void);
	bool CorrectSCPHeader(void);
	bool WriteSCPFile(char*);
	uint16_t CRCEvaluate(uint8_t*, uint32_t);
	int16_t CRCCheck(uint8_t*, uint32_t);
};

#endif
