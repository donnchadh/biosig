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
// Section1_Info.cpp: implementation of the cSection1_Info class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Section1_Info.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cSection1_Info::cSection1_Info()
{
	wSCPECGRevNum = 20;				// Rev. 2.0
	szLastName[0] = '\0';
	szFirstName[0] = '\0';
	strcpy(szPatientID, "UNKNOWN");	// UNKNOWN patient (before providing info)
	szSecondLastName[0] = '\0';
	DOB.dd = 0;
	DOB.mm = 0;
	DOB.yyyy = 0;
	wHeight = 0;
	bHeightUnit = 1;				// cm
	wWeight = 0;
	bWeightUnit = 1;				// Kg
	bSex = 0;
	wSBP = 0;
	wDBP = 0;
	wInstNum = 0;
	wDeptNum = 0;
	wDevID = 0;
	bDevType = 0;					// Cart
	bManCode = 255;					// The manufacturer name is specified in the final bytes
	strcpy(szModDesc, "Cart1");		// "Cart1" cardiograph
	bSCPECGProtRevNum = 20;
	// No reference beat is available in XML aECG, thus data format category is II
	bSCPECGProtCompLev = 0xA0;		// Data format category II
	bLangSuppCode = 0x00;			// Ascii only, latin and 1-byte code
	bCapECGDev = 0xD0;				// Acquire, (No Analysis), Print and Store
	bMainsFreq = 1;					// 50 Hz;
	szAnalProgRevNum[0] = '\0';
	szSerNumAcqDev[0] = '\0';
	szAcqDevSystSW[0] = '\0';
	strcpy(szSCPImplSW, "OpenECG XML-SCP 1.00");	// The conversion to SCP-ECG file is made by the
													// OpenECG XML to SCP Converter 1.00
	strcpy(szAcqDevManuf, "Manufacturer");	// The device used for acquisition is manufactured by "Manufacturer"
	szRefPhys[0] = '\0';
	szLCPhys[0] = '\0';
	szTechnician[0] = '\0';
	wBaseLineFilter = 0;
	bFilterBitMap = 0;
	bStatCode = 0;					// Routine;
	DOA.dd = 0;
	DOA.mm = 0;
	DOA.yyyy = 0;
	TOA.hh = 0;
	TOA.mm = 0;
	TOA.ss = 0;
	szSeqNum[0] = '\0';
	wDateTimeZoneOffset = 60;		// Central Europe
	wDateTimeZoneIndex = 0;			// Index not used
	szDateTimeZoneDesc[0] = '\0';	// Description not present
}

cSection1_Info::~cSection1_Info()
{
}
