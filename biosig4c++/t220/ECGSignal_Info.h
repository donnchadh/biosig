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
// ECGSignal_Info.h: interface for the cECGSignal_Info class.
//
//////////////////////////////////////////////////////////////////////

#if !defined ECGSIGNAL_INFO
#define ECGSIGNAL_INFO

#ifndef VS_DEF
#include "biosig.h"
#else
#include "biosig_vs.h"
#endif

class cECGSignal_Info  
{
public:
	cECGSignal_Info();
	virtual ~cECGSignal_Info();

public:
	uint8_t bNumLead;
	uint8_t LeadR_codes[15];
	double* LeadR[15];
	uint8_t LeadA_codes[15];
	double* LeadA[15];
	bool fRefBeatUsedForCompr;
	bool fLeadsAllSimultRecord;
	uint8_t bNumLeadSimultRecord;
	bool fBimodal;
	uint32_t dwStartSampleR;
	uint32_t dwEndSampleR;
	uint16_t wAmplR;
	uint16_t wIntvR;
	uint32_t dwStartSampleA;
	uint32_t dwEndSampleA;
	uint16_t wAmplA;
	uint16_t wIntvA;
	uint16_t wEncodingType;
	uint16_t wAverFiducial;
	uint16_t wNoOfQRS;
	uint32_t* pwTblQRSOn;
	uint32_t* pwTblQRSOff;
	uint16_t* pwTblQRSType;
	uint16_t wPOn;
	uint16_t wPOff;
	uint16_t wQRSOn;
	uint16_t wQRSOff;
	uint16_t wTOff;
	int8_t* szInterpText;
	uint16_t wGlobMeas[10];
	uint8_t LeadMeas_codes[15];
	uint16_t wLeadMeas[15][33];
};

#endif
