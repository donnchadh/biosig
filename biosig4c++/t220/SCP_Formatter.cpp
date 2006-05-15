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
// SCP_Formatter.cpp: implementation of the cSCP_Formatter class.
*/

#include "StdAfx.h"
#include "SCPECG_Writer.h"
#include "Section1_Info.h"
#include "ECGSignal_Info.h"
#include "SCP_Formatter.h"
#ifndef VS_DEF
#include <unistd.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cSCP_Formatter::cSCP_Formatter()
{
	S1I = NULL;
	ESI = NULL;

	lenSect0 = 0;
	lenSect1 = 0;
	lenSect3 = 0;
//	lenSect4 = 0;
//	lenSect5 = 0;
	lenSect6 = 0;
//	lenSect7 = 0;
//	lenSect8 = 0;
//	lenSect10 = 0;
}

cSCP_Formatter::~cSCP_Formatter()
{
}

void  cSCP_Formatter::ResetInfo()
{
	if (S1I != NULL) {
		delete S1I;
		S1I = NULL;
	}

	S1I = new cSection1_Info();

	if (ESI != NULL) {
		delete ESI;
		ESI = NULL;
	}

	ESI = new cECGSignal_Info();
}

int16_t cSCP_Formatter::DoTheSCPFile(char* szFilePathName)
{
	if (szFilePathName == NULL)
		return (-1);

	if ((S1I == NULL) || (ESI == NULL))
		return (-2);

	if (!CreateSCPFileHeaderDraft())
		return (-3);
	if (!CreateSCPSection0Draft())
		return (-4);
	if (!CreateSCPSection1())
		return (-5);
	if (!CreateSCPSection3())
		return (-6);
	//No reference beat is available in XML aECG
	//if (!CreateSCPSection4())
	//	return (-7);
	//if (!CreateSCPSection5())
	//	return (-8);
	if (!CreateSCPSection6())
		return (-9);
	//Global measurements, lead specific measurements and diagnosis are not available
	//if (!CreateSCPSection7())
	//	return (-10);
	//if (!CreateSCPSection8())
	//	return (-11);
	//if (!CreateSCPSection10())
	//	return (-12);

	if (!CorrectSCPSection0())
		return (-13);
	if (!CorrectSCPHeader())
		return (-14);

	if (!WriteSCPFile(szFilePathName))
		return (-15);

	return (0);
}

int16_t cSCP_Formatter::SetLastName(char* szLastName)
{
	if (szLastName == NULL)
		return (-1);

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	strncpy(S1I->szLastName, szLastName, 63);
	S1I->szLastName[63] = '\0';

	if (strlen(szLastName) > 63)
		return (-3);
	else
		return (0);
}

int16_t cSCP_Formatter::GetLastName(char* szLastName)
{
	if (szLastName == NULL)
		return (-1);

	szLastName[0] = '\0';

	if (S1I == NULL)
		return (-2);

	strncpy(szLastName, S1I->szLastName, 63);
	szLastName[63] = '\0';

	return (0);
}

int16_t cSCP_Formatter::SetFirstName(char* szFirstName)
{
	if (szFirstName == NULL)
		return (-1);

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	strncpy(S1I->szFirstName, szFirstName, 63);
	S1I->szFirstName[63] = '\0';

	if (strlen(szFirstName) > 63)
		return (-3);
	else
		return (0);
}

int16_t cSCP_Formatter::GetFirstName(char* szFirstName)
{
	if (szFirstName == NULL)
		return (-1);

	szFirstName[0] = '\0';

	if (S1I == NULL)
		return (-2);

	strncpy(szFirstName, S1I->szFirstName, 63);
	szFirstName[63] = '\0';

	return (0);
}

int16_t cSCP_Formatter::SetPatientID(char* szPatientID)
{
	if (szPatientID == NULL)
		return (-1);

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	strncpy(S1I->szPatientID, szPatientID, 63);
	S1I->szPatientID[63] = '\0';

	if (strlen(szPatientID) > 63)
		return (-3);
	else
		return (0);
}

int16_t cSCP_Formatter::GetPatientID(char* szPatientID)
{
	if (szPatientID == NULL)
		return (-1);

	szPatientID[0] = '\0';

	if (S1I == NULL)
		return (-2);

	strncpy(szPatientID, S1I->szPatientID, 63);
	szPatientID[63] = '\0';

	return (0);
}

int16_t cSCP_Formatter::SetSecondLastName(char* szSecondLastName)
{
	if (szSecondLastName == NULL)
		return (-1);

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	strncpy(S1I->szSecondLastName, szSecondLastName, 63);
	S1I->szSecondLastName[63] = '\0';

	if (strlen(szSecondLastName) > 63)
		return (-3);
	else
		return (0);
}

int16_t cSCP_Formatter::GetSecondLastName(char* szSecondLastName)
{
	if (szSecondLastName == NULL)
		return (-1);

	szSecondLastName[0] = '\0';

	if (S1I == NULL)
		return (-2);

	strncpy(szSecondLastName, S1I->szSecondLastName, 63);
	szSecondLastName[63] = '\0';

	return (0);
}

int16_t cSCP_Formatter::SetDateOfBirth(int16_t dd, int16_t mm, int16_t yyyy)
{
	if ((dd < 1) || (dd > 31))
		return (-1);
	if ((mm < 1) || (mm > 12))
		return (-1);

// Inserted further checks on the date of birth correctness
	// April, June, September and November have 30 days
	if ((mm == 4) || (mm == 6) || (mm == 9) || (mm == 11))
		if (dd == 31)
			return (-1);
	// February cannot have more than 29 days
	if (mm == 2)
		if (dd > 29)
			return (-1);
	// Check on leap years
	if ((yyyy % 4) == 0) {
		if (((yyyy % 100) == 0) && ((yyyy % 400) != 0)) {
			// Not leap
			if (dd == 29)
				return (-1);
		}
		else {
			// Leap (do nothing)
		}
	}

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	S1I->DOB.dd = (uint8_t) ((0x00FF) & dd);
	S1I->DOB.mm = (uint8_t) ((0x00FF) & mm);
	S1I->DOB.yyyy = yyyy;

	return (0);
}

int16_t cSCP_Formatter::GetDateOfBirth(int16_t* dd, int16_t* mm, int16_t* yyyy)
{
	if ((dd == NULL) || (mm == NULL) || (yyyy == NULL)) {
		if (dd != NULL)
			*dd = 0;
		if (mm != NULL)
			*mm = 0;
		if (yyyy != NULL)
			*yyyy = 0;
		return (-1);
	}

	*dd = 0;
	*mm = 0;
	*yyyy = 0;

	if (S1I == NULL)
		return (-2);

	*dd = S1I->DOB.dd;
	*mm = S1I->DOB.mm;
	*yyyy = S1I->DOB.yyyy;

	return (0);
}

int16_t cSCP_Formatter::SetHeight(int16_t height)
{
	if (height < 0)
		return (-1);

// Eventually insert further checks on the height correctness

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	S1I->wHeight = height;

	return (0);
}

int16_t cSCP_Formatter::GetHeight(int16_t* height)
{
	if (height == NULL)
		return (-1);

	*height = 0;

	if (S1I == NULL)
		return (-2);

	*height = S1I->wHeight;

	return (0);
}

int16_t cSCP_Formatter::SetWeight(int16_t weight)
{
	if (weight < 0)
		return (-1);

// Eventually insert further checks on the height correctness

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	S1I->wWeight = weight;

	return (0);
}

int16_t cSCP_Formatter::GetWeight(int16_t* weight)
{
	if (weight == NULL)
		return (-1);

	*weight = 0;

	if (S1I == NULL)
		return (-2);

	*weight = S1I->wWeight;

	return (0);
}

int16_t cSCP_Formatter::SetSex(int16_t sex)
{
	if ((sex != 0) && (sex != 1) && (sex != 2) && (sex != 9))
		return (-1);

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	S1I->bSex = (uint8_t) sex;

	return (0);
}

int16_t cSCP_Formatter::GetSex(int16_t* sex)
{
	if (sex == NULL)
		return (-1);

	*sex = 0;

	if (S1I == NULL)
		return (-2);

	*sex = S1I->bSex;

	return (0);
}

int16_t cSCP_Formatter::SetSBP(int16_t SBP)
{
	if (SBP < 0)
		return (-1);

// Eventually insert further checks on the SBP correctness

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	S1I->wSBP = SBP;

	return (0);
}

int16_t cSCP_Formatter::GetSBP(int16_t* SBP)
{
	if (SBP == NULL)
		return (-1);

	*SBP = 0;

	if (S1I == NULL)
		return (-2);

	*SBP = S1I->wSBP;

	return (0);
}

int16_t cSCP_Formatter::SetDBP(int16_t DBP)
{
	if (DBP < 0)
		return (-1);

// Eventually insert further checks on the DBP correctness

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	S1I->wDBP = DBP;

	return (0);
}

int16_t cSCP_Formatter::GetDBP(int16_t* DBP)
{
	if (DBP == NULL)
		return (-1);

	*DBP = 0;

	if (S1I == NULL)
		return (-2);

	*DBP = S1I->wDBP;

	return (0);
}

int16_t cSCP_Formatter::SetReferringPhysician(char* RefPhys)
{
	if (RefPhys == NULL)
		return (-1);

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	strncpy(S1I->szRefPhys, RefPhys, 63);
	S1I->szRefPhys[63] = '\0';

	if (strlen(RefPhys) > 63)
		return (-3);
	else
		return (0);
}

int16_t cSCP_Formatter::GetReferringPhysician(char* RefPhys)
{
	if (RefPhys == NULL)
		return (-1);

	RefPhys[0] = '\0';

	if (S1I == NULL)
		return (-2);

	strncpy(RefPhys, S1I->szRefPhys, 63);
	RefPhys[63] = '\0';

	return (0);
}

int16_t cSCP_Formatter::SetLastConfirmingPhys(char* LCPhys)
{
	if (LCPhys == NULL)
		return (-1);

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	strncpy(S1I->szLCPhys, LCPhys, 63);
	S1I->szLCPhys[63] = '\0';

	if (strlen(LCPhys) > 63)
		return (-3);
	else
		return (0);
}

int16_t cSCP_Formatter::GetLastConfirmingPhys(char* LCPhys)
{
	if (LCPhys == NULL)
		return (-1);

	LCPhys[0] = '\0';

	if (S1I == NULL)
		return (-2);

	strncpy(LCPhys, S1I->szLCPhys, 63);
	LCPhys[63] = '\0';

	return (0);
}

int16_t cSCP_Formatter::SetStatCode(int16_t StatCode)
{
	if ((StatCode < 0) || (StatCode > 10))	// 0 = routine; 1 = lowest emergency level,
											// .........., 10 = highest emergency level
		return (-1);

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	S1I->bStatCode = (uint8_t) StatCode;

	return (0);
}

int16_t cSCP_Formatter::GetStatCode(int16_t* StatCode)
{
	if (StatCode == NULL)
		return (-1);

	*StatCode = 0;

	if (S1I == NULL)
		return (-2);

	*StatCode = S1I->bStatCode;

	return (0);
}

int16_t cSCP_Formatter::SetDateOfAcquisition(int16_t dd, int16_t mm, int16_t yyyy)
{
	if ((dd < 1) || (dd > 31))
		return (-1);
	if ((mm < 1) || (mm > 12))
		return (-1);

// Inserted further checks on the date of birth correctness
	// April, June, September and November have 30 days
	if ((mm == 4) || (mm == 6) || (mm == 9) || (mm == 11))
		if (dd == 31)
			return (-1);
	// February cannot have more than 29 days
	if (mm == 2)
		if (dd > 29)
			return (-1);
	// Check on leap years
	if ((yyyy % 4) == 0) {
		if (((yyyy % 100) == 0) && ((yyyy % 400) != 0)) {
			// Not leap
			if (dd == 29)
				return (-1);
		}
		else {
			// Leap (do nothing)
		}
	}

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	S1I->DOA.dd = (uint8_t) ((0x00FF) & dd);
	S1I->DOA.mm = (uint8_t) ((0x00FF) & mm);
	S1I->DOA.yyyy = yyyy;

	return (0);
}

int16_t cSCP_Formatter::GetDateOfAcquisition(int16_t* dd, int16_t* mm, int16_t* yyyy)
{
	if ((dd == NULL) || (mm == NULL) || (yyyy == NULL)) {
		if (dd != NULL)
			*dd = 0;
		if (mm != NULL)
			*mm = 0;
		if (yyyy != NULL)
			*yyyy = 0;
		return (-1);
	}

	*dd = 0;
	*mm = 0;
	*yyyy = 0;

	if (S1I == NULL)
		return (-2);

	*dd = S1I->DOA.dd;
	*mm = S1I->DOA.mm;
	*yyyy = S1I->DOA.yyyy;

	return (0);
}

int16_t cSCP_Formatter::SetTimeOfAcquisition(int16_t hh, int16_t mm, int16_t ss)
{
	if ((hh < 0) || (hh > 23))
		return (-1);
	if ((mm < 0) || (mm > 59))
		return (-1);
	if ((ss < 0) || (ss > 59))
		return (-1);

// Eventually insert further checks on the time of acquisition correctness

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	S1I->TOA.hh = (uint8_t) ((0x00FF) & hh);
	S1I->TOA.mm = (uint8_t) ((0x00FF) & mm);
	S1I->TOA.ss = (uint8_t) ((0x00FF) & ss);

	return (0);
}

int16_t cSCP_Formatter::GetTimeOfAcquisition(int16_t* hh, int16_t* mm, int16_t* ss)
{
	if ((hh == NULL) || (mm == NULL) || (ss == NULL)) {
		if (hh != NULL)
			*hh = 0;
		if (mm != NULL)
			*mm = 0;
		if (ss != NULL)
			*ss = 0;
		return (-1);
	}

	*hh = 0;
	*mm = 0;
	*ss = 0;

	if (S1I == NULL)
		return (-2);

	*hh = S1I->TOA.hh;
	*mm = S1I->TOA.mm;
	*ss = S1I->TOA.ss;

	return (0);
}

int16_t cSCP_Formatter::SetSequenceNumber(int8_t* SeqNum)
{
	if (SeqNum == NULL)
		return (-1);

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	memcpy(S1I->szSeqNum, SeqNum, 11);
	S1I->szSeqNum[11] = '\0';

	if (strlen((char*)SeqNum) > 11)
		return (-3);
	else
		return (0);
}

int16_t cSCP_Formatter::GetSequenceNumber(int8_t* SeqNum)
{
	if (SeqNum == NULL)
		return (-1);

	SeqNum[0] = '\0';

	if (S1I == NULL)
		return (-2);

	memcpy(SeqNum, S1I->szSeqNum, 11);
	SeqNum[11] = '\0';

	return (0);
}

int16_t cSCP_Formatter::SetTimeZone(int16_t TimeZone)
{
// Insert some tests to check the correctness of the input parameter

	if (S1I == NULL) {
		S1I = new cSection1_Info();
	}

	if (S1I == NULL)
		return (-2);

	S1I->wDateTimeZoneOffset = TimeZone;

	return (0);
}

int16_t cSCP_Formatter::GetTimeZone(int16_t* TimeZone)
{
	if (TimeZone == NULL)
		return (-1);

	*TimeZone = 0;

	if (S1I == NULL)
		return (-2);

	*TimeZone = S1I->wDateTimeZoneOffset;

	return (0);
}

int16_t cSCP_Formatter::LoadXMLInfo(HDRTYPE* XMLaECGParsedData)
{
	int16_t val1, val2;
	uint16_t i;
//	uint16_t k, j;

	// Name should be provided by the XML parser in terms of first name, last name, second last name
	strncpy(S1I->szFirstName, XMLaECGParsedData->Patient.Name, min(strlen(XMLaECGParsedData->Patient.Name),63));
	S1I->szFirstName[63] = '\0';
	strncpy(S1I->szLastName, XMLaECGParsedData->Patient.Name, min(strlen(XMLaECGParsedData->Patient.Name),63));
	S1I->szLastName[63] = '\0';
	strncpy(S1I->szSecondLastName, XMLaECGParsedData->Patient.Name, min(strlen(XMLaECGParsedData->Patient.Name),63));
	S1I->szSecondLastName[63] = '\0';

	if (strlen(XMLaECGParsedData->Patient.Id) != 0) { // In case len = 0 the PatID field will be written with "UNKNOWN"
		strncpy(S1I->szPatientID, XMLaECGParsedData->Patient.Id, min(strlen(XMLaECGParsedData->Patient.Id),63));
		S1I->szPatientID[63] = '\0';
	}

	time_t Birthday = gdf_time2t_time(XMLaECGParsedData->Patient.Birthday);
	tm* Birthday_tm = gmtime(&Birthday);

	S1I->DOB.dd = (uint8_t) Birthday_tm->tm_mday;
	S1I->DOB.mm = (uint8_t) Birthday_tm->tm_mon + 1;
	S1I->DOB.yyyy = (uint8_t) Birthday_tm->tm_year + 1900;

	S1I->wHeight = (uint16_t) XMLaECGParsedData->Patient.Height;
	//S1I->bHeightUnit is always cm

	S1I->wWeight = (uint16_t) XMLaECGParsedData->Patient.Weight;
	//S1I->bWeightUnit is always Kg

	if(XMLaECGParsedData->Patient.Sex == 0)
		S1I->bSex = 0;	// Not Known
	else if (XMLaECGParsedData->Patient.Sex == 1)
		S1I->bSex = 1;	// Male
	else if (XMLaECGParsedData->Patient.Sex == 2)
		S1I->bSex = 2;	// Female
	else
		S1I->bSex = 9;	// Not specified

	val1 = (uint16_t) XMLaECGParsedData->aECG->systolicBloodPressure;
	val2 = (uint16_t) XMLaECGParsedData->aECG->diastolicBloodPressure;

	if (val1 < val2) {
		S1I->wDBP = val1;
		S1I->wSBP = val2;
	}
	else {
		S1I->wDBP = val2;
		S1I->wSBP = val1;
	}

	// Info not provided by XML aECG
	//S1I->wInstNum = (uint16_t) atoi(XMLaECGParsedData->szRecAqrInstID);
	//S1I->wDeptNum = (uint16_t) atoi(XMLaECGParsedData->szRecAqrDeptID);
	//strncpy(S1I->szAnalProgRevNum, XMLaECGParsedData->szRecAnaProgRev, 24);
	//S1I->szAnalProgRevNum[24] = '\0';
	//strncpy(S1I->szAcqDevSystSW, XMLaECGParsedData->szRecAqrDevRev, 24);
	//S1I->szAcqDevSystSW[24] = '\0';

	// Info not provided by XML aECG
	//strncpy(S1I->szRefPhys, XMLaECGParsedData->ReferringPhys, 63);
	//S1I->szRefPhys[63] = '\0';
	
	strncpy(S1I->szTechnician, XMLaECGParsedData->ID.Technician, min(strlen(XMLaECGParsedData->ID.Technician),63));
	S1I->szTechnician[63] = '\0';

	// Info not provided by XML aECG
	//S1I->wBaseLineFilter = XMLaECGParsedData->BaseLineFilter;
	//S1I->bFilterBitMap = (uint8_t) XMLaECGParsedData->FilterBitMap;
	
	// Info not provided by XML aECG
	//S1I->bStatCode = (uint8_t) XMLaECGParsedData->StatCode;
	//if (S1I->bStatCode > 10)
	//	S1I->bStatCode = 0;

	time_t AcquisitionDT = gdf_time2t_time(XMLaECGParsedData->T0);
	tm* AcquisitionDT_tm = gmtime(&AcquisitionDT);

	S1I->DOA.dd = (uint8_t) AcquisitionDT_tm->tm_mday;
	S1I->DOA.mm = (uint8_t) AcquisitionDT_tm->tm_mon + 1;
	S1I->DOA.yyyy = (int16_t) AcquisitionDT_tm->tm_year + 1900;

	S1I->TOA.hh = (uint8_t) AcquisitionDT_tm->tm_hour;
	S1I->TOA.mm = (uint8_t) AcquisitionDT_tm->tm_min;
	S1I->TOA.ss = (uint8_t) AcquisitionDT_tm->tm_sec;

	ESI->dwEndSampleR = XMLaECGParsedData->SPR;
	// LSB is assumed to be the same over all the stored leads
	ESI->wAmplR = (uint16_t) XMLaECGParsedData->CHANNEL[0].Cal;
	ESI->wIntvR = (uint16_t) XMLaECGParsedData->SampleRate;
	// No reference beat seems to be available in XML aECG
	ESI->dwEndSampleA = 1;
	// LSB is assumed to be 1 uV
	ESI->wAmplA = 1;
	ESI->wIntvA = (uint16_t) XMLaECGParsedData->SampleRate;

	// max 15 leads are allowed
	if(XMLaECGParsedData->NS > 15)
		return (-1);

	ESI->bNumLead = (uint8_t) XMLaECGParsedData->NS;
// Situations with number of leads simultaneouly recorded != total number of leads are not supported
	ESI->bNumLeadSimultRecord = (uint8_t) XMLaECGParsedData->NS;

	// Max 15 leads are supported
	// Same lead codes are used between XML aECG and SCP-ECG ver. 2.0. This is not strictly correct because
	// in SCP 2.0 only codes till 78 are the same, while from 86 on the codes are not defined, but we can
	// assume the use of the same codes like manufacturer dependent codes
	for (i = 0; i < ESI->bNumLead; i++) {
		ESI->LeadR_codes[i] = *XMLaECGParsedData->CHANNEL[i].Label;
	}
	for (i = 0; i < ESI->bNumLead; i++) {
		ESI->LeadR[i] = XMLaECGParsedData->data.block + i*XMLaECGParsedData->data.size[0];
	}
	// No reference beat seems to be available in XML aECG
	for (i = 0; i < ESI->bNumLead; i++) {
		ESI->LeadA_codes[i] = 0;
	}
	for (i = 0; i < ESI->bNumLead; i++) {
		ESI->LeadA[i] = NULL;
	}

	// No global measurements, lead specific measurements or diagnosis are provided
	//ESI->wPOn = XMLaECGParsedData->GlobalMeas.P_onset;
	//ESI->wPOff = XMLaECGParsedData->GlobalMeas.P_end;
	//ESI->wQRSOn = XMLaECGParsedData->GlobalMeas.QRS_onset;
	//ESI->wQRSOff = XMLaECGParsedData->GlobalMeas.QRS_end;
	//ESI->wTOff = XMLaECGParsedData->GlobalMeas.T_end;

	//ESI->wAverFiducial = XMLaECGParsedData->GlobalMeas.QRS_onset;	// QRSOn
	//if (ESI->wAverFiducial == 0)
	//	ESI->wAverFiducial = 100;

	//ESI->wNoOfQRS = XMLaECGParsedData->GlobalMeas.Num_Cpxs;
	//ESI->pwTblQRSType = (uint16_t*) &XMLaECGParsedData->GlobalMeas.QRS_type;
	//ESI->pwTblQRSOn = (uint32_t*) &XMLaECGParsedData->GlobalMeas.QRS_start_refbeat_sub;
	//ESI->pwTblQRSOff = (uint32_t*) &XMLaECGParsedData->GlobalMeas.QRS_end_refbeat_sub;
	
	//ESI->szInterpText = XMLaECGParsedData->MeansMorfologia;

	////ESI->wGlobMeas[0] = XMLaECGParsedData->GlobalMeas.HR;
	//ESI->wGlobMeas[1] = XMLaECGParsedData->GlobalMeas.RR_median;
	//ESI->wGlobMeas[2] = XMLaECGParsedData->GlobalMeas.PQ_intv;
	////ESI->wGlobMeas[3] = XMLaECGParsedData->GlobalMeas.PP;
	//ESI->wGlobMeas[4] = XMLaECGParsedData->GlobalMeas.QRS_dur;
	//ESI->wGlobMeas[5] = XMLaECGParsedData->GlobalMeas.QT_intv;
	//ESI->wGlobMeas[6] = XMLaECGParsedData->GlobalMeas.QTc_intv;
	//ESI->wGlobMeas[7] = XMLaECGParsedData->GlobalMeas.P_axis;
	//ESI->wGlobMeas[8] = XMLaECGParsedData->GlobalMeas.QRS_axis;
	//ESI->wGlobMeas[9] = XMLaECGParsedData->GlobalMeas.STT_axis;

	//for (k = 0; k < ESI->bNumLead; i++) {
	//	for (j = 0; j < 31; j++) {
	//		ESI->wLeadMeas[k][j] = ((int16_t*) &XMLaECGParsedData->LeadMeas[k])[j * 2];
	//	}
	//}

	return (0);
}

bool cSCP_Formatter::CreateSCPFileHeaderDraft()
{
	FileHead.crc = 0;			// Temporary
	FileHead.len = 0;			// Temporary

	return (true);
}

bool cSCP_Formatter::CreateSCPSection0Draft()
{
	if (S1I == NULL)
		return (false);

// Create the header
	Sect0.shead.crc = 0;		// Temporary
	Sect0.shead.sectid = 0;
	Sect0.shead.len = 136;		// 16 bytes of header and 12 rows of 10 bytes each
	Sect0.shead.sectver = S1I->bSCPECGProtRevNum;
	Sect0.shead.protver = S1I->bSCPECGProtRevNum;
	Sect0.shead.SCPres[0] = 'S';
	Sect0.shead.SCPres[1] = 'C';
	Sect0.shead.SCPres[2] = 'P';
	Sect0.shead.SCPres[3] = 'E';
	Sect0.shead.SCPres[4] = 'C';
	Sect0.shead.SCPres[5] = 'G';

// Create all the rows
	Sect0.sect_0.sectid = 0;
	Sect0.sect_0.sectlen = 136;
	Sect0.sect_0.index = 7;
	Sect0.sect_1.sectid = 1;
	Sect0.sect_1.sectlen = 0;	// Temporary
	Sect0.sect_1.index = 143;
	Sect0.sect_2.sectid = 2;
	Sect0.sect_2.sectlen = 0;
	Sect0.sect_2.index = 0;
	Sect0.sect_3.sectid = 3;
	Sect0.sect_3.sectlen = 0;	// Temporary
	Sect0.sect_3.index = 0;		// Temporary
	Sect0.sect_4.sectid = 4;
	Sect0.sect_4.sectlen = 0;	
	Sect0.sect_4.index = 0;		
	Sect0.sect_5.sectid = 5;
	Sect0.sect_5.sectlen = 0;	
	Sect0.sect_5.index = 0;		
	Sect0.sect_6.sectid = 6;
	Sect0.sect_6.sectlen = 0;	// Temporary
	Sect0.sect_6.index = 0;		// Temporary
	Sect0.sect_7.sectid = 7;
	Sect0.sect_7.sectlen = 0;	
	Sect0.sect_7.index = 0;		
	Sect0.sect_8.sectid = 8;
	Sect0.sect_8.sectlen = 0;	
	Sect0.sect_8.index = 0;		
	Sect0.sect_9.sectid = 9;
	Sect0.sect_9.sectlen = 0;
	Sect0.sect_9.index = 0;
	Sect0.sect_10.sectid = 10;
	Sect0.sect_10.sectlen = 0;	
	Sect0.sect_10.index = 0;	
	Sect0.sect_11.sectid = 11;
	Sect0.sect_11.sectlen = 0;
	Sect0.sect_11.index = 0;

// Store the length of Section1
	lenSect0 = Sect0.shead.len;

	return (true);
}

bool cSCP_Formatter::CreateSCPSection1()
{
	SCPSECT_HEAD	Sect1Head;
	TAG				tg;
	uint32_t		len1;
	uint32_t		lenTmp;
	uint16_t		crc;
	uint8_t			bUnit;
	uint8_t			bBufTmp[256];

	if (S1I == NULL)
		return (false);

// Create the header
	Sect1Head.crc = 0;				// Temporary
	Sect1Head.sectid = 1;
	Sect1Head.len = 0;				// Temporary
	Sect1Head.sectver = S1I->bSCPECGProtRevNum;
	Sect1Head.protver = S1I->bSCPECGProtRevNum;
	Sect1Head.SCPres[0] = '\0';
	Sect1Head.SCPres[1] = '\0';
	Sect1Head.SCPres[2] = '\0';
	Sect1Head.SCPres[3] = '\0';
	Sect1Head.SCPres[4] = '\0';
	Sect1Head.SCPres[5] = '\0';

	memcpy(Sect1, (int8_t*) &Sect1Head, 16);
	len1 = 16;

// Create all the tags

// Tag 0 (max len = 64)
	tg.id = 0;
	tg.len = strlen(S1I->szLastName) + 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], S1I->szLastName, tg.len);
	len1 += tg.len;

// Tag 1 (max len = 64)
	tg.id = 1;
	tg.len = strlen(S1I->szFirstName) + 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], S1I->szFirstName, tg.len);
	len1 += tg.len;

// Tag 2 (max len = 64)
	tg.id = 2;
	tg.len = strlen(S1I->szPatientID) + 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], S1I->szPatientID, tg.len);
	len1 += tg.len;

// Tag 3 (max len = 64)
	tg.id = 3;
	tg.len = strlen(S1I->szSecondLastName) + 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], S1I->szSecondLastName, tg.len);
	len1 += tg.len;

// Tag 5 (len = 4)
	tg.id = 5;
	tg.len = 4;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->DOB.yyyy), 2);
	len1 += 2;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->DOB.mm), 1);
	len1 += 1;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->DOB.dd), 1);
	len1 += 1;

// Tag 6 (len = 3)
	tg.id = 6;
	tg.len = 3;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->wHeight), 2);
	len1 += 2;
	if (S1I->wHeight == 0)
		bUnit = 0;
	else
		bUnit = 1;
	memcpy(&Sect1[len1], (int8_t*)(&bUnit), 1);
	len1 += 1;

// Tag 7 (len = 3)
	tg.id = 7;
	tg.len = 3;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->wWeight), 2);
	len1 += 2;
	if (S1I->wWeight == 0)
		bUnit = 0;
	else
		bUnit = 1;
	memcpy(&Sect1[len1], (int8_t*)(&bUnit), 1);
	len1 += 1;

// Tag 8 (len = 1)
	tg.id = 8;
	tg.len = 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->bSex), 1);
	len1 += 1;

// Tag 11 (len = 2)
	if (S1I->wSBP != 0) {
		tg.id = 11;
		tg.len = 2;
		memcpy(&Sect1[len1], (int8_t*) &tg, 3);
		len1 += 3;
		memcpy(&Sect1[len1], (int8_t*)(&S1I->wSBP), 2);
		len1 += 2;
	}

// Tag 12 (len = 2)
	if (S1I->wDBP != 0) {
		tg.id = 12;
		tg.len = 2;
		memcpy(&Sect1[len1], (int8_t*) &tg, 3);
		len1 += 3;
		memcpy(&Sect1[len1], (int8_t*)(&S1I->wDBP), 2);
		len1 += 2;
	}

// Tag 14 (max len = 2 + 2 + 2 + 1 + 1 + 6 + 1 + 1 + 1 + 1 + 1 + 16 + 1 + 25 + 25 + 25 + 25 + 25)
// Total = 161 (max value)
	tg.id = 14;
	tg.len = 0;				// Temporary
	memcpy(bBufTmp, (int8_t*) &tg, 3);
	lenTmp = 0;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->wInstNum), 2);
	lenTmp += 2;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->wDeptNum), 2);
	lenTmp += 2;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->wDevID), 2);
	lenTmp += 2;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->bDevType), 1);
	lenTmp += 1;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->bManCode), 1);
	lenTmp += 1;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->szModDesc), 6);
	lenTmp += 6;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->bSCPECGProtRevNum), 1);
	lenTmp += 1;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->bSCPECGProtCompLev), 1);
	lenTmp += 1;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->bLangSuppCode), 1);
	lenTmp += 1;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->bCapECGDev), 1);
	lenTmp += 1;
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->bMainsFreq), 1);
	lenTmp += 1;
	// Reserved area (16 bytes)
	memset(&bBufTmp[3 + lenTmp], '\0', 16);
	lenTmp += 16;
	bUnit = (uint8_t) (strlen(S1I->szAnalProgRevNum) + 1);
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&bUnit), 1);
	lenTmp += 1;
	// (Max len = 25)
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->szAnalProgRevNum), ((0x00FF) & bUnit));
	lenTmp += ((0x00FF) & bUnit);
	// (Max len = 25)
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->szSerNumAcqDev), strlen(S1I->szSerNumAcqDev) + 1);
	lenTmp += (strlen(S1I->szSerNumAcqDev) + 1);
	// (Max len = 25)
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->szAcqDevSystSW), strlen(S1I->szAcqDevSystSW) + 1);
	lenTmp += (strlen(S1I->szAcqDevSystSW) + 1);
	// (Max len = 25)
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->szSCPImplSW), strlen(S1I->szSCPImplSW) + 1);
	lenTmp += (strlen(S1I->szSCPImplSW) + 1);
	// (Max len = 25)
	memcpy(&bBufTmp[3 + lenTmp], (int8_t*)(&S1I->szAcqDevManuf), strlen(S1I->szAcqDevManuf) + 1);
	lenTmp += (strlen(S1I->szAcqDevManuf) + 1);

	memcpy(&bBufTmp[1], (int8_t*)(&lenTmp), 2);

	memcpy(&Sect1[len1], (int8_t*)(&bBufTmp[0]), lenTmp + 3);
	len1 += (lenTmp + 3);

// Tag 20 (max len = 64)
	tg.id = 20;
	tg.len = strlen(S1I->szRefPhys) + 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], S1I->szRefPhys, tg.len);
	len1 += tg.len;

// Tag 21 (max len = 64)
	tg.id = 21;
	tg.len = strlen(S1I->szLCPhys) + 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], S1I->szLCPhys, tg.len);
	len1 += tg.len;

// Tag 24 (len = 1)
	tg.id = 24;
	tg.len = 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->bStatCode), 1);
	len1 += 1;

// Tag 25 (len = 4)
	tg.id = 25;
	tg.len = 4;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->DOA.yyyy), 2);
	len1 += 2;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->DOA.mm), 1);
	len1 += 1;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->DOA.dd), 1);
	len1 += 1;

// Tag 26 (len = 3)
	tg.id = 26;
	tg.len = 3;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->TOA.hh), 1);
	len1 += 1;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->TOA.mm), 1);
	len1 += 1;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->TOA.ss), 1);
	len1 += 1;

// Tag 31 (max len = 12)
	tg.id = 31;
	tg.len = strlen(S1I->szSeqNum) + 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], S1I->szSeqNum, tg.len);
	len1 += tg.len;

// Tag 34 (max len = 29)
	tg.id = 34;
	tg.len = 4 + strlen(S1I->szDateTimeZoneDesc) + 1;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->wDateTimeZoneOffset), 2);
	len1 += 2;
	memcpy(&Sect1[len1], (int8_t*)(&S1I->wDateTimeZoneIndex), 2);
	len1 += 2;
	memcpy(&Sect1[len1], S1I->szDateTimeZoneDesc, strlen(S1I->szDateTimeZoneDesc) + 1);
	len1 += (strlen(S1I->szDateTimeZoneDesc) + 1);

// Tag 255 (len = 0)
	tg.id = 255;
	tg.len = 0;
	memcpy(&Sect1[len1], (int8_t*) &tg, 3);
	len1 += 3;

// Evaluate the size and correct it if odd
	if ((len1 % 2) != 0) {
		memset(&Sect1[len1], '\0', 1);
		len1 += 1;
	}

// Correct the len in the section header
	Sect1Head.len = len1;
	memcpy(Sect1, (int8_t*) &Sect1Head, 16);

// Evaluate the CRC
	crc = CRCEvaluate(&Sect1[2], len1 - 2);

// Correct the CRC in the section header
	memcpy(Sect1, (int8_t*) &crc, 2);

// Store the length of Section1
	lenSect1 = len1;

	return (true);
}

bool cSCP_Formatter::CreateSCPSection3()
{
	SCPSECT_HEAD	Sect3Head;
	uint32_t		len1;
	uint16_t		crc;
	uint16_t		i;
	uint8_t			bNum, bNum1;

	if (ESI == NULL)
		return (false);

// Create the header
	Sect3Head.crc = 0;				// Temporary
	Sect3Head.sectid = 3;
	Sect3Head.len = 0;				// Temporary
	Sect3Head.sectver = S1I->bSCPECGProtRevNum;
	Sect3Head.protver = S1I->bSCPECGProtRevNum;
	Sect3Head.SCPres[0] = '\0';
	Sect3Head.SCPres[1] = '\0';
	Sect3Head.SCPres[2] = '\0';
	Sect3Head.SCPres[3] = '\0';
	Sect3Head.SCPres[4] = '\0';
	Sect3Head.SCPres[5] = '\0';

	memcpy(Sect3, (int8_t*) &Sect3Head, 16);
	len1 = 16;

// Create all the fields
// Situations with a number of leads > 15 are not supported
	if (ESI->bNumLead > 15)
		return (false);

// Number of leads enclosed
	memcpy(&Sect3[len1], (int8_t*) &ESI->bNumLead, 1);
	len1 += 1;

// Flags
// Situations with reference beat subtraction are not supported
	if (ESI->fRefBeatUsedForCompr)
		return (false);
// Situations with not all the leads simultaneously recorded are not supported
	if (!ESI->fLeadsAllSimultRecord)
		return (false);

// Situations number of leads simultaneouly recorded != total number of leads are not supported
	if (ESI->bNumLeadSimultRecord != ESI->bNumLead)		return (false);

	bNum = 0;
	if (ESI->fRefBeatUsedForCompr)
		bNum |= 0x01;
	if (ESI->fLeadsAllSimultRecord)
		bNum |= 0x04;
// We assume hat all the leads are recorded simultaneously
	bNum1 = (0x1F) & ESI->bNumLeadSimultRecord;
	bNum |= (bNum1 << 3);
	memcpy(&Sect3[len1], (int8_t*) &bNum, 1);
	len1 += 1;

// We assume to have max 15 leads
	for (i = 0; i < ESI->bNumLead; i++) {
		bNum = ESI->LeadR_codes[i];
		memcpy(&Sect3[len1], (int8_t*) &ESI->dwStartSampleR, 4);
		len1 += 4;
		memcpy(&Sect3[len1], (int8_t*) &ESI->dwEndSampleR, 4);
		len1 += 4;
		memcpy(&Sect3[len1], (int8_t*) &bNum, 1);
		len1 += 1;
	}

// Evaluate the size and correct it if odd
	if ((len1 % 2) != 0) {
		memset(&Sect3[len1], '\0', 1);
		len1 += 1;
	}

// Correct the len in the section header
	Sect3Head.len = len1;
	memcpy(Sect3, (int8_t*) &Sect3Head, 16);

// Evaluate the CRC
	crc = CRCEvaluate(&Sect3[2], len1 - 2);

// Correct the CRC in the section header
	memcpy(Sect3, (int8_t*) &crc, 2);

// Store the length of Section3
	lenSect3 = len1;

	return (true);
}

bool cSCP_Formatter::CreateSCPSection6()
{
	SCPSECT_HEAD	Sect6Head;
	uint32_t		len1;
	uint32_t		numByteCompRhythm;
	uint16_t		crc;
	uint16_t		i, num;
	uint8_t			bNum;

	if (ESI == NULL)
		return (false);

// Create the header
	Sect6Head.crc = 0;				// Temporary
	Sect6Head.sectid = 6;
	Sect6Head.len = 0;				// Temporary
	Sect6Head.sectver = S1I->bSCPECGProtRevNum;
	Sect6Head.protver = S1I->bSCPECGProtRevNum;
	Sect6Head.SCPres[0] = '\0';
	Sect6Head.SCPres[1] = '\0';
	Sect6Head.SCPres[2] = '\0';
	Sect6Head.SCPres[3] = '\0';
	Sect6Head.SCPres[4] = '\0';
	Sect6Head.SCPres[5] = '\0';

	memcpy(Sect6, (int8_t*) &Sect6Head, 16);
	len1 = 16;

// Create all the fields

// AVM
	memcpy(&Sect6[len1], (int8_t*) &ESI->wAmplR, 2);
	len1 += 2;

// Sample interval
	memcpy(&Sect6[len1], (int8_t*) &ESI->wIntvR, 2);
	len1 += 2;

// Situations with first or second differences are not supported
	if (ESI->wEncodingType != 0)
		return (false);

// Diff used
	bNum = (uint8_t) ESI->wEncodingType;
	memcpy(&Sect6[len1], (int8_t*) &bNum, 1);
	len1 += 1;

// Situations with bimodal compression are not supported
	if (ESI->fBimodal)
		return (false);

// Bimodal/Non-bimodal
	bNum = 0;
	memcpy(&Sect6[len1], (int8_t*) &bNum, 1);
	len1 += 1;

// Fill the length block
	numByteCompRhythm = ESI->dwEndSampleR * 2;
// Each sample is stored on 2 bytes for each of the 15 leads (we assume to have max 15 leads)
	for (i = 0; i < ESI->bNumLead; i++) {
		memcpy(&Sect6[len1], (int8_t*) &numByteCompRhythm, 2);
		len1 += 2;
	}

// Fill tha data block with the ECG samples
// Write the ECG samples (we assume to have max 15 leads)
	for (i = 0; i < ESI->bNumLead; i++) {
		for (num = 0; num < ESI->dwEndSampleR; num++) {
		/* ##FIXME## this is a hack 
			it would be best if this could be done within functions SWRITE (not defined yet)
		*/ 
			int16_t val = (int16_t) ESI->LeadR[i][num];
			memcpy(&Sect6[len1], (int8_t*) &val, 2);
			len1 += 2;
		}
	}

// Evaluate the size and correct it if odd
	if ((len1 % 2) != 0) {
		memset(&Sect6[len1], '\0', 1);
		len1 += 1;
	}

// Correct the len in the section header
	Sect6Head.len = len1;
	memcpy(Sect6, (int8_t*) &Sect6Head, 16);

// Evaluate the CRC
	crc = CRCEvaluate(&Sect6[2], len1 - 2);

// Correct the CRC in the section header
	memcpy(Sect6, (int8_t*) &crc, 2);

// Store the length of Section6
	lenSect6 = len1;

	return (true);
}

bool cSCP_Formatter::CorrectSCPSection0()
{
	uint16_t	crc;

// Write the proper length of each section
	Sect0.sect_1.sectlen = lenSect1;
	Sect0.sect_3.sectlen = lenSect3;
	Sect0.sect_6.sectlen = lenSect6;

// Correct the indexes
	Sect0.sect_3.index = Sect0.sect_1.index + Sect0.sect_1.sectlen;
	Sect0.sect_6.index = Sect0.sect_3.index + Sect0.sect_3.sectlen;

// Evaluate the CRC
	crc = CRCEvaluate(&((uint8_t*)(&Sect0))[2], lenSect0 - 2);

// Correct the CRC in the section header
	memcpy((int8_t*)(&Sect0), (int8_t*) &crc, 2);

	return (true);
}

bool cSCP_Formatter::CorrectSCPHeader()
{
	uint32_t	len1;
	uint16_t	crc;

	TotalFile = (uint8_t*)malloc(6 + lenSect0 + lenSect1 + lenSect3 + lenSect6);
	if (TotalFile == NULL)
		return (false);

// Copy the sections in the final buffer
	memcpy(TotalFile, (int8_t*)&FileHead, 6);
	len1 = 6;
	memcpy(&TotalFile[len1], (int8_t*)&Sect0, lenSect0);
	len1 += lenSect0;
	memcpy(&TotalFile[len1], (int8_t*)&Sect1, lenSect1);
	len1 += lenSect1;
	memcpy(&TotalFile[len1], (int8_t*)&Sect3, lenSect3);
	len1 += lenSect3;
	memcpy(&TotalFile[len1], (int8_t*)&Sect6, lenSect6);
	len1 += lenSect6;

// Write the proper length of the file
	memcpy(&((int8_t*)&FileHead)[2], (int8_t*)&len1, 4);
	memcpy(TotalFile, (int8_t*)&FileHead, 6);

// Evaluate the CRC
	crc = CRCEvaluate(&TotalFile[2], len1 - 2);

// Correct the CRC in the section header
	memcpy((int8_t*)&FileHead, (int8_t*)&crc, 2);
	memcpy(TotalFile, (int8_t*)&FileHead, 6);

	return (true);
}

bool cSCP_Formatter::WriteSCPFile(char* szFilePathName)
{
	FILE*		fp;
	uint32_t	len1;

	fp = fopen(szFilePathName, "wb");
	if (fp == NULL) {
		free(TotalFile);
		return (false);
	}

	len1 = 6 + lenSect0 + lenSect1 + lenSect3 + lenSect6;

	if (fwrite(TotalFile, len1, 1, fp) != 1) {
		free(TotalFile);
		fclose(fp);
		unlink(szFilePathName);
		return (false);
	}

	free(TotalFile);
	fclose(fp);

	return (true);
}

/********************************************************************
*	CRCEvaluate														*
*																	*
* Parameters: datablock is the buffer on which to evaluate the CRC.	*
*			  datalength is the length of the whole buffer			*
*																	*
* Description:	Evaluate the SCP-ECG CRC on a data block			*
*				(all file or a section)								*
*																	*
 ********************************************************************/

uint16_t cSCP_Formatter::CRCEvaluate(uint8_t* datablock, uint32_t datalength) {
	uint32_t	i;
	uint16_t	crc_tot;
	uint8_t		crchi, crclo;
	uint8_t		a, b;
	uint8_t		tmp1, tmp2;

	crchi = 0xFF;
	crclo = 0xFF;

	for (i = 0; i < datalength; i++) {
		a = datablock[i];
		a ^= crchi;
		crchi = a;
		a >>= 4;
		a &= 0x0F;
		a ^= crchi;
		crchi = crclo;
		crclo = a;
		tmp1 = ((a & 0x0F) << 4) & 0xF0;
		tmp2 = ((a & 0xF0) >> 4) & 0x0F;
		a = tmp1 | tmp2;
		b = a;
		tmp1 = ((a & 0x7F) << 1) & 0xFE;
		tmp2 = ((a & 0x80) >> 7) & 0x01;
		a = tmp1 | tmp2;
		a &= 0x1F;
		crchi ^= a;
		a = b & 0xF0;
		crchi ^= a;
		tmp1 = ((b & 0x7F) << 1) & 0xFE;
		tmp2 = ((b & 0x80) >> 7) & 0x01;
		b = tmp1 | tmp2;
		b &= 0xE0;
		crclo ^= b;
	}

	crc_tot = ((0x00FF & (uint16_t) crchi) << 8) & 0xFF00;
	crc_tot |= (0x00FF & (uint16_t) crclo);

	return (crc_tot);
}

/********************************************************************
*	CRCCheck														*
*																	*
* Parameters: datablock is the buffer on which to verify the CRC.	*
*			  It starts with the two CRC-CCITT bytes.				*
*			  datalength is the length of the whole buffer			*
*			  (including the two CRC bytes)							*
*																	*
* Description:	Check the SCP-ECG CRC on a data block				*
*				(all file or a section)								*
*																	*
 ********************************************************************/

int16_t cSCP_Formatter::CRCCheck(uint8_t* datablock, uint32_t datalength)
{
	uint16_t crc;

	crc = 0;

	if (datalength <= 2)
		return (-1);

	// Evaluate CRC
	crc = CRCEvaluate((uint8_t*) (datablock + 2), (uint32_t) (datalength - 2));
	if (((uint8_t) ((crc & 0xFF00) >> 8) != (uint8_t) datablock[1]) ||
		((uint8_t) (crc & 0x00FF) != (uint8_t) datablock[0]))
		return (0);
	else
		return (1);
}
