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
// ECGSignal_Info.cpp: implementation of the cECGSignal_Info class.
*/

#include "StdAfx.h"
#include "ECGSignal_Info.h"

/*
// Construction/Destruction
*/

cECGSignal_Info::cECGSignal_Info()
{
	uint16_t i, j;

// Only the 8 standard leads are supported
	bNumLead = 15;
	for (i = 0; i < 15; i++) {
		LeadR_codes[i] = 0;
		LeadR[i] = NULL;
		LeadA_codes[i] = 0;
		LeadA[i] = NULL;
	}
// Reference beat subtraction and bimodal compression are not supported
	fRefBeatUsedForCompr = false;			// No ref beat subtraction
	fLeadsAllSimultRecord = true;			// All simultaneously recorded
	bNumLeadSimultRecord = 15;
	fBimodal = false;						// No bimodal compression
// The lead-dependent measurements can include 12 leads
	dwStartSampleR = 1;
	dwEndSampleR = 5000;					// 5000 samples overall
	wAmplR = 0;								// 0.000 uV
	wIntvR = 0;								// 0.000 ms
	dwStartSampleA = 1;
	dwEndSampleA = 1000;					// 1000 samples overall
	wAmplA = 0;								// 0.000 uV
	wIntvA = 0;								// 0.000 ms
	wEncodingType = 0;						// Real data used
	wAverFiducial = 0;
	wNoOfQRS = 0;
	pwTblQRSOn = NULL;
	pwTblQRSOff = NULL;
	pwTblQRSType = NULL;
	wPOn = 0;
	wPOff = 0;
	wQRSOn = 0;
	wQRSOff = 0;
	wTOff = 0;
	szInterpText = NULL;
	for (i = 0; i < 10; i++) {
		wGlobMeas[i] = 0;
	}
	for (i = 0; i < 15; i++) {
		LeadMeas_codes[i] = 0;
		for (j = 0; j < 33; j++) {
			wLeadMeas[i][j] = 0;
		}
	}
}

cECGSignal_Info::~cECGSignal_Info()
{
}
