/*

    sandbox is used for development and under constraction work
    The functions here are either under construction or experimental. 
    The functions will be either fixed, then they are moved to another place;
    or the functions are discarded. Do not rely on the interface in this function
       	

    $Id: sandbox.c,v 1.5 2009/04/16 20:19:17 schloegl Exp $
    Copyright (C) 2008,2009 Alois Schloegl <a.schloegl@ieee.org>
    This file is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 

    BioSig is free software; you can redistribute it and/or
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


#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../biosig-dev.h"

#ifdef WITH_ASN1
#include "SessionArchiveSection.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif 

extern int VERBOSE_LEVEL;

void sopen_fef_read(HDRTYPE* hdr) {

#ifndef WITH_ASN1

	B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	B4C_ERRMSG = "ASN1/FEF currently not supported";

#else 
	static asn_TYPE_descriptor_t *pduType = &asn_DEF_SessionArchiveSection;
	SessionArchiveSection_t *SAS = NULL;
	SessionTestSection_t *STS = NULL;
	SessionPhaseSection_t *SPS = NULL; 	/* Decoded structure */
	FEFString_t *fefstr = 0;
	asn_codec_ctx_t *opt_codec_ctx = 0;

	asn_dec_rval_t rval;

if (VERBOSE_LEVEL>7) 
	fprintf(stdout,"ASN1: BER DECODING\n");
	
	size_t pos =32;	
	rval = ber_decode(0, pduType, (void **)&SAS, hdr->AS.Header+32, hdr->HeadLen-32);

	pos += rval.consumed;
/*
	while (pos<hdr->HeadLen) {

fprintf(stdout,"%i/%i\n",pos,hdr->HeadLen);
	
		rval = ber_decode(opt_codec_ctx, pduType, (void **)&SAS, hdr->AS.Header+pos, hdr->HeadLen-32);
		pos += rval.consumed;
	}
*/
if (VERBOSE_LEVEL>7) 
	fprintf(stdout,"%i/%i\nASN1: BER DECODING DONE\n",pos,hdr->HeadLen);

if (VERBOSE_LEVEL>8) {
//	asn_fprint(stdout, &asn_DEF_SessionArchiveSection, SAS);

//	SAS->s_archive_id
	asn_fprint(stdout, &asn_DEF_FEFString, &SAS->s_archive_id);
	asn_fprint(stdout, &asn_DEF_FEFString, &SAS->s_archive_name);
	asn_fprint(stdout, &asn_DEF_FEFString, SAS->s_archive_comments);

	/**********************************
		Manufacturer information 
	 **********************************/	
	asn_fprint(stdout, &asn_DEF_ManufacturerSpecificSection, SAS->manufacturerspecific);

	/**********************************
		Health Care provider
	 **********************************/	
	asn_fprint(stdout, &asn_DEF_HealthCareProviderSection, SAS->healthcareprovider);
	asn_fprint(stdout, &asn_DEF_PatientDemographicsSection, &SAS->demographics);
}

	/**********************************
		demographic information 
	 **********************************/	
	long ival; 
	if (!asn_INTEGER2long(SAS->demographics.sex, &ival))
		hdr->Patient.Sex = ival;

	{
	double val; 
	if (!asn_REAL2double(&SAS->demographics.patientweight->value, &val))
		hdr->Patient.Weight = (uint8_t)val;
	if (!asn_REAL2double(&SAS->demographics.patientheight->value, &val))
		hdr->Patient.Height = (uint8_t)val;

	strncpy(hdr->Patient.Name,SAS->demographics.characternamegroup->givenname.buf,MAX_LENGTH_NAME);
	strncat(hdr->Patient.Name," ",MAX_LENGTH_NAME);
	strncat(hdr->Patient.Name,SAS->demographics.characternamegroup->middlename.buf,MAX_LENGTH_NAME);
	strncat(hdr->Patient.Name," ",MAX_LENGTH_NAME);
	strncat(hdr->Patient.Name,SAS->demographics.characternamegroup->familyname.buf,MAX_LENGTH_NAME);
	}
	
	/**********************************
		Test Section information 
	 **********************************/	

	size_t N=0; 

//	asn_fprint(stdout, &asn_DEF_SessionTestSection, SAS->sessions.list.array[0]);
//		asn_fprint(stdout, &asn_DEF_SessionTestSection, SAS->sessions.list.array[k]);
	if (!asn_INTEGER2long(&SAS->sessions.list.count,&ival)) N = ival; 
	if (!N) while (SAS->sessions.list.array[N]) N++; 
	fprintf(stdout,"Number of TestSections %i\n",N);

	if (N>1) { 
		B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
		B4C_ERRMSG = "FEF: multiple test sections are not supported, yet";
	}			

	STS = SAS->sessions.list.array[0];
//	asn_fprint(stdout, &asn_DEF_SessionTestSection, STS);

	/******* Manufacturer ************/		
//	asn_fprint(stdout, &asn_DEF_MedicalDeviceSystemSection, &STS->medicaldevicesystem);
	strncpy(hdr->ID.Manufacturer._field,STS->medicaldevicesystem.systemmodel.manufacturer.buf,MAX_LENGTH_MANUF);
	int LEN = strlen(hdr->ID.Manufacturer._field)+1; 
	hdr->ID.Manufacturer.Name = hdr->ID.Manufacturer._field;

	strncpy(hdr->ID.Manufacturer._field+LEN, STS->medicaldevicesystem.systemmodel.model_number.buf, MAX_LENGTH_MANUF-LEN);
	hdr->ID.Manufacturer.Model = hdr->ID.Manufacturer._field+LEN;
	
	/******* Multimedia ************/		

	/******* Session Phase Section SPS ************/		

//	asn_fprint(stdout, &asn_DEF_SessionTestSection, SAS->sessions.list.array[0]);
//	asn_fprint(stdout, &asn_DEF_SessionTestSection, SAS->sessions.list.array[k]);
	if (!asn_INTEGER2long(&STS->phases.list.count,&ival)) N = ival; 
	if (!N) while (STS->phases.list.array[N]) N++; 
	if (N>1) { 
		B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
		B4C_ERRMSG = "FEF: multiple phases sections are not supported, yet";
	}			
	SPS = STS->phases.list.array[0];
//	asn_fprint(stdout, &asn_DEF_SessionPhaseSection, SPS);
//	asn_fprint(stdout, &asn_DEF_DescriptiveDataSection, &SPS->descriptivedata);

	/**************** Descriptive Data Section DescDS ****************/

//	SPS->descriptivedata->realtimesadescs
//	asn_fprint(stdout, &asn_DEF_SessionTestSection, SAS->sessions.list.array[0]);
	N = 0; 
	if (!asn_INTEGER2long(&SPS->descriptivedata.realtimesadescs->list.count,&ival)) N = ival; 
	if (!N) while (SPS->descriptivedata.realtimesadescs->list.array[N]) N++;
	
	hdr->NS = N;
	hdr->CHANNEL = (CHANNEL_TYPE*)calloc(hdr->NS,sizeof(CHANNEL_TYPE));
	int k; 
	uint64_t FsN=1,FsD=1;
	for (k=0; k<hdr->NS; k++)	{
	
			RealTimeSampleArrayDescriptiveDataSection_t *RTSADDS = SPS->descriptivedata.realtimesadescs->list.array[k];
			if (VERBOSE_LEVEL>7) {
				fprintf(stdout,"[FEF 212] #=%i/%i\n",k,hdr->NS);
				asn_fprint(stdout, &asn_DEF_RealTimeSampleArrayDescriptiveDataSection, RTSADDS);
			}	

			CHANNEL_TYPE *hc = hdr->CHANNEL+k;

		      	strncpy(hc->Label, RTSADDS->labelstring->buf, MAX_LENGTH_LABEL);

			if (!asn_INTEGER2long(RTSADDS->unitcode,&ival))
			      	hc->PhysDimCode= (uint16_t)ival;
			else
			      	hc->PhysDimCode= 0;

			if (!hc->PhysDimCode) 
				hc->PhysDimCode = PhysDimCode(RTSADDS->unitlabelstring->buf);
			
			//******************* samplerate *****************/
			unsigned n=0, d=0;
			asn_INTEGER2long(&RTSADDS->sampleperiod.denominator,&d);
			asn_INTEGER2long(&RTSADDS->sampleperiod.numerator,&n);
			fprintf(stdout,"#%i: %li %li\n",k,n,d);
			if (n && d) {
				FsN = lcm(FsN,n);
				FsD = lcm(FsD,d);
			}
			hc->SPR = d;
			 
			// double Fs = (double)n / d;

			//******************* scaling *****************/
	      		hc->PhysMax   = +100;
		      	hc->PhysMin   = -100;
		      	hc->DigMax    = +2047;
		      	hc->DigMin    = -2048;
			if (asn_REAL2double(&RTSADDS->scaleandrangespec.lowerabsolutevalue,&hc->PhysMin)
			 || asn_REAL2double(&RTSADDS->scaleandrangespec.upperabsolutevalue,&hc->PhysMax)
			 || asn_REAL2double(&RTSADDS->scaleandrangespec.lowervaluescaled,&hc->DigMin)
			 || asn_REAL2double(&RTSADDS->scaleandrangespec.uppervaluescaled,&hc->DigMax) )
				fprintf(stderr,"Warning (FEF): scaling factors in channel %i not available\n",k+1);

			hc->Cal = (hc->PhysMax-hc->PhysMin)/(hc->DigMax-hc->DigMin);
			hc->Off =  hc->PhysMin-hc->Cal*hc->DigMin;
			
			//******************* bits *****************/
			int bits=-1,dt=-1;
			asn_INTEGER2long(&RTSADDS->saspecification.storagesize,&bits);
			asn_INTEGER2long(&RTSADDS->saspecification.storagedatatype,&dt);
			hc->GDFTYP = 0;
			if (dt==2) 	// ieee 754
			{
				if (bits==32)
					hc->GDFTYP = 16; // float32
				else if (bits==64)
					hc->GDFTYP = 17; // float64
				else if (bits==128)
					hc->GDFTYP = 18; // float128
			}
			else if (dt==1) // signed 
			{
				if (bits==8) 		hc->GDFTYP = 1; // 
				else if (bits==16)	hc->GDFTYP = 3; //
				else if (bits==24)	hc->GDFTYP = 255+24; //
				else if (bits==32)	hc->GDFTYP = 5; //
				else if (bits==64)	hc->GDFTYP = 7; //
			}		
			else if (dt==0) // unsigned 
			{
				if (bits==8)		hc->GDFTYP = 2; // 
				else if (bits==16)	hc->GDFTYP = 4; //
				else if (bits==24)	hc->GDFTYP = 511+24; //
				else if (bits==32)	hc->GDFTYP = 6; //
				else if (bits==64)	hc->GDFTYP = 8; //
			}
			int arraysize=0; 
			asn_INTEGER2long(&RTSADDS->saspecification.storagesize,&arraysize);
			if (!arraysize) hc->SPR = arraysize; 
			// else 
			/* Number of samples of this signal in a subblock of SAMDB. 
			   Arraysize is actually redundant information in other cases 
			   than distribution sample array. Therefore, if arraysize = 0, 
			   the number of samples is calculated as <subblocklength> 
			   divided by <sampleperiod> of the signal.
			*/

			//******************* filters *****************/
			if (RTSADDS->sasignalfrequency) {
				asn_REAL2double(&RTSADDS->sasignalfrequency->lowedgefreq, &hc->HighPass);
				asn_REAL2double(&RTSADDS->sasignalfrequency->highedgefreq, &hc->LowPass);
			} else 	
			{
			      	hc->HighPass  = -1;
			      	hc->LowPass   = INF;
			}

			// TODO: MetricCalEntry, MetricCalType,
			
			// TODO: 				
		      	hc->LeadIdCode= 0;
	      		strcpy(hc->Transducer, "EEG: Ag-AgCl electrodes");
		      	hc->bi 	      = 2*k;
	      		hc->OnOff     = 1;
	      		hc->Notch     = -1;
		      	hc->Impedance = INF;
		      	hc->XYZ[0] 	  = 0.0;
	      		hc->XYZ[1] 	  = 0.0;
		      	hc->XYZ[2] 	  = 0.0;
	}
	size_t d = gcd(FsD,FsN);
	fprintf(stdout,"Fs=%i/%i\n",FsN,FsD);
	hdr->SampleRate = ((double)(FsD/d))/((double)(FsN/d));
	hdr->SPR = FsD;

	/************** Measured Data Section MeasDS ***********************/
	N = 0; 
	if (!asn_INTEGER2long(&SPS->measureddata.list.count,&ival)) N = ival; 
	if (!N) while (SPS->measureddata.list.array[N]) N++;
	fprintf(stdout,"number of MeasuredData",N);
	
	for (k=0; k<N; k++) {
		asn_fprint(stdout, &asn_DEF_MeasuredDataSection, SPS->measureddata.list.array[k]);
		SPS->measureddata.list.array[k].realtimesas
	}
 	 
 	 
	/****** Notes Section information SNS **********************************/	
	if (!asn_INTEGER2long(&SAS->notes->list.count,&ival)) N = ival; 
	for (k=0; (!N && SAS->notes->list.array[k]) || (k<N) ; k++) {
		asn_fprint(stdout, &asn_DEF_SessionNotesSection, SAS->notes->list.array[k++]);
	}
	
	/******* post checks ************/		

hdr2ascii(hdr,stdout,3);

	if(errno) {
		/* Error message is already printed */
		exit(-1);
	}

	/* Check ASN.1 constraints */
	char errbuf[128];
	size_t errlen = sizeof(errbuf);
	if(asn_check_constraints(pduType, SAS,
		errbuf, &errlen)) {
		fprintf(stdout, "SOPEN_FEF_READ: ASN.1 constraint "
			"check failed: %s\n", errbuf);
	}

//	asn_fprint(stdout, pduType, structure);
	ASN_STRUCT_FREE(*pduType, SAS);

#endif 

}


