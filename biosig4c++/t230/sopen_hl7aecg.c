/*

    $Id: sopen_hl7aecg.c,v 1.28 2008-12-01 08:14:16 schloegl Exp $
    Copyright (C) 2006,2007 Alois Schloegl <a.schloegl@ieee.org>
    Copyright (C) 2007 Elias Apostolopoulos
    This file is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 


    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.


 */


#include <stdio.h>             // system includes
#include <vector>
#include <string>
#include <sstream>

#include "../biosig-dev.h"
#include "../XMLParser/tinyxml.h"
#include "../XMLParser/Tokenizer.h"

int sopen_HL7aECG_read(HDRTYPE* hdr) {
/*
	this function is a stub or placeholder and need to be defined in order to be useful.
	It will be called by the function SOPEN in "biosig.c"

	Input: 
		char* Header	// contains the file content
		
	Output: 
		HDRTYPE *hdr	// defines the HDR structure accoring to "biosig.h"
*/
	char tmp[80]; 
	TiXmlDocument doc(hdr->FileName);

	if(doc.LoadFile()){
	    TiXmlHandle hDoc(&doc);
	    TiXmlHandle aECG = hDoc.FirstChild("AnnotatedECG");
	    if(aECG.Element()){
	    	char *strtmp = strdup(aECG.FirstChild("id").Element()->Attribute("root"));
	    	size_t len = strlen(strtmp); 
	    	if (len>MAX_LENGTH_RID)	
			fprintf(stdout,"Warning HL7aECG(read): length of Recording ID exceeds maximum length %i>%i\n",len,MAX_LENGTH_PID); 
		strncpy(hdr->ID.Recording,strtmp,min(len,MAX_LENGTH_RID));
		free(strtmp); 

		TiXmlHandle effectiveTime = aECG.FirstChild("effectiveTime");

		char *T0 = NULL;

		if(effectiveTime.FirstChild("low").Element())
		    T0 = (char *)effectiveTime.FirstChild("low").Element()->Attribute("value");
		else if(effectiveTime.FirstChild("center").Element())
		    T0 = (char *)effectiveTime.FirstChild("center").Element()->Attribute("value");

		struct tm t0; 
		T0[14] = '\0';
		// ### ?FIXME?: compensate local time and DST in mktime used in tm_time2gdf_time below 

#ifdef __APPLE__
		// ### FIXME: for some (unknown) reason, timezone does not work on MacOSX
		t0.tm_sec  = atoi(T0+12);  	
		printf("Warning SOPEN(HL7aECG,read): timezone not supported\n");
#else
		t0.tm_sec  = atoi(T0+12)-timezone;	
#endif 
		T0[12] = '\0';
		t0.tm_min  = atoi(T0+10);
		T0[10] = '\0';
		t0.tm_hour = atoi(T0+8);
		T0[8]  = '\0';
		t0.tm_mday = atoi(T0+6);
		T0[6]  = '\0';
		t0.tm_mon  = atoi(T0+4)-1;
		T0[4]  = '\0';
		t0.tm_year = atoi(T0)-1900;
		t0.tm_isdst  = -1;
 		hdr->T0 = tm_time2gdf_time(&t0);

		TiXmlHandle demographic = aECG.FirstChild("componentOf").FirstChild("timepointEvent").FirstChild("componentOf").FirstChild("subjectAssignment").FirstChild("subject").FirstChild("trialSubject");

		TiXmlElement *id = demographic.FirstChild("id").Element();
		if(id) {
			const char* tmpstr = id->Attribute("extension");
			size_t len = strlen(tmpstr); 
			if (len>MAX_LENGTH_PID)
				fprintf(stdout,"Warning HL7aECG(read): length of Patient Id exceeds maximum length %i>%i\n",len,MAX_LENGTH_PID); 
		    	strncpy(hdr->Patient.Id,tmpstr,MAX_LENGTH_PID);
		}    

		if (!hdr->FLAG.ANONYMOUS) 
		{
			demographic = demographic.FirstChild("subjectDemographicPerson");
		
			TiXmlElement *name = demographic.FirstChild("name").Element();
			if (name) {
				size_t len = strlen(name->GetText());
				if (len>MAX_LENGTH_NAME)
					fprintf(stdout,"Warning HL7aECG(read): length of Patient Name exceeds maximum length %i>%i\n",len,MAX_LENGTH_PID); 
				strncpy(hdr->Patient.Name, name->GetText(), MAX_LENGTH_NAME);
			}	
			else {
				hdr->Patient.Name[0] = 0;
				fprintf(stderr,"Warning: Patient Name could not be read.\n");
			}	
		}		

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"hl7r: [414]\n"); 

		/* non-standard fields height and weight */
		TiXmlElement *weight = demographic.FirstChild("weight").Element();
		if (weight) {
		    uint16_t code = PhysDimCode(strcpy(tmp,weight->Attribute("unit")));	
		    if ((code & 0xFFE0) != 1728) 
		    	fprintf(stderr,"Warning: incorrect weight unit (%s)\n",weight->Attribute("unit"));	
		    else 	// convert to kilogram
			hdr->Patient.Weight = (uint8_t)(atof(weight->Attribute("value"))*PhysDimScale(code)*1e-3);  
		}
		TiXmlElement *height = demographic.FirstChild("height").Element();

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"hl7r: [415]\n"); 

		if (height) {
		    uint16_t code = PhysDimCode(strcpy(tmp,height->Attribute("unit")));	
		    if ((code & 0xFFE0) != 1280) 
		    	fprintf(stderr,"Warning: incorrect height unit (%s) %i \n",height->Attribute("unit"),code);	
		    else	// convert to centimeter
			hdr->Patient.Height = (uint8_t)(atof(height->Attribute("value"))*PhysDimScale(code)*1e+2);
		}
		
		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"hl7r: [416]\n"); 

		TiXmlElement *birthday = demographic.FirstChild("birthTime").Element();
		if(birthday){
		    T0 = (char *)birthday->Attribute("value");
		    if (T0==NULL) T0=(char *)birthday->GetText();  // workaround for reading two different formats 
		}

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"hl7r: [417]\n"); 

		if (T0==NULL) 
			;
		else if (strlen(T0)>14) {
		    T0[14] = '\0';
		    t0.tm_sec = atoi(T0+12);
		    T0[12] = '\0';
		    t0.tm_min = atoi(T0+10);
		    T0[10] = '\0';
		    t0.tm_hour = atoi(T0+8);
		    T0[8] = '\0';
		    t0.tm_mday = atoi(T0+6);
		    T0[6] = '\0';
		    t0.tm_mon = atoi(T0+4)-1;
		    T0[4] = '\0';
		    t0.tm_year = atoi(T0)-1900;

		    hdr->Patient.Birthday = tm_time2gdf_time(&t0);
		}
		
		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"hl7r: [418]\n"); 

		TiXmlElement *sex = demographic.FirstChild("administrativeGenderCode").Element();
		if(sex){

		    if (sex->Attribute("code")==NULL)
			hdr->Patient.Sex = 0;
		    else if(!strcmp(sex->Attribute("code"),"F"))
			hdr->Patient.Sex = 2;
		    else if(!strcmp(sex->Attribute("code"),"M"))
			hdr->Patient.Sex = 1;
		    else
			hdr->Patient.Sex = 0;
		}else{
		    hdr->Patient.Sex = 0;
		}

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"hl7r: [419]\n"); 


		int LowPass=0, HighPass=0, Notch=0;
		TiXmlHandle channels = aECG.FirstChild("component").FirstChild("series").FirstChild("component").FirstChild("sequenceSet");
		TiXmlHandle variables = aECG.FirstChild("component").FirstChild("series");

		for(TiXmlElement *tmpvar = variables.FirstChild("controlVariable").Element(); tmpvar; tmpvar = tmpvar->NextSiblingElement("controlVariable")){
		    if(!strcmp(tmpvar->FirstChildElement("controlVariable")->FirstChildElement("code")->Attribute("code"), "MDC_ATTR_FILTER_NOTCH"))
			Notch = atoi(tmpvar->FirstChildElement("controlVariable")->FirstChildElement("component")->FirstChildElement("controlVariable")->FirstChildElement("value")->Attribute("value"));
		    else if(!strcmp(tmpvar->FirstChildElement("controlVariable")->FirstChildElement("code")->Attribute("code"), "MDC_ATTR_FILTER_LOW_PASS"))
			LowPass = atoi(tmpvar->FirstChildElement("controlVariable")->FirstChildElement("component")->FirstChildElement("controlVariable")->FirstChildElement("value")->Attribute("value"));
		    else if(!strcmp(tmpvar->FirstChildElement("controlVariable")->FirstChildElement("code")->Attribute("code"), "MDC_ATTR_FILTER_HIGH_PASS"))
			HighPass = atoi(tmpvar->FirstChildElement("controlVariable")->FirstChildElement("component")->FirstChildElement("controlVariable")->FirstChildElement("value")->Attribute("value"));
		}
		hdr->NRec = 1;
//		hdr->SPR = 1;
//		hdr->AS.rawdata = (uint8_t *)malloc(hdr->SPR);
//		int32_t *data;
		
		hdr->SampleRate = 1.0/atof(channels.FirstChild("component").FirstChild("sequence").FirstChild("value").FirstChild("increment").Element()->Attribute("value"));
		
		TiXmlHandle channel = channels.Child("component", 1).FirstChild("sequence");
		for(hdr->NS = 0; channel.Element(); ++(hdr->NS), channel = channels.Child("component", hdr->NS+1).FirstChild("sequence")) {};
		hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));

		channel = channels.Child("component", 1).FirstChild("sequence");
		for(int i = 0; channel.Element(); ++i, channel = channels.Child("component", i+1).FirstChild("sequence")){

		    const char *code = channel.FirstChild("code").Element()->Attribute("code");
		    
			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"hl7r: [420] %i\n",i); 

		    strncpy(hdr->CHANNEL[i].Label,code,min(40,MAX_LENGTH_LABEL));
		    hdr->CHANNEL[i].Label[MAX_LENGTH_LABEL] = '\0';
		    hdr->CHANNEL[i].Transducer[0] = '\0';
		    hdr->CHANNEL[i].GDFTYP = 5;	// int32

		    std::vector<std::string> vector;
		    stringtokenizer(vector, channel.FirstChild("value").FirstChild("digits").Element()->GetText());

		    hdr->CHANNEL[i].SPR = vector.size();
		    if (i==0) {
		    	hdr->SPR = hdr->CHANNEL[i].SPR;
			hdr->AS.rawdata = (uint8_t *)realloc(hdr->AS.rawdata, 4*hdr->NS*hdr->SPR*hdr->NRec);
		    }
		    else if (hdr->SPR != hdr->CHANNEL[i].SPR) {
			if (hdr->SPR != lcm(hdr->SPR, hdr->CHANNEL[i].SPR)) 
			{
				fprintf(stderr,"Error: number of samples %i of #%i differ from %i in #0.\n",hdr->CHANNEL[i].SPR,i+1,hdr->SPR);
				B4C_ERRNUM = B4C_UNSPECIFIC_ERROR;
				B4C_ERRMSG = "HL7aECG: initial sample rate is not a multiple of all samplerates";
				exit(-5);
			}	
			
		    }	

		    /* read data samples */	
		    int32_t* data = (int32_t*)(hdr->AS.rawdata + (GDFTYP_BITS[hdr->CHANNEL[i].GDFTYP]>>3)*i*(hdr->SPR));
		    size_t DIV = hdr->SPR/hdr->CHANNEL[i].SPR;
		    for(size_t j=0; j<hdr->CHANNEL[i].SPR; ++j) {
			size_t k=0;
			data[j*DIV+k] = atoi(vector[j].c_str());
			while (++k<DIV) data[j*DIV+k] = data[j*DIV+k-1]; 
			  
			/* get Min/Max */
			if(data[j] > hdr->CHANNEL[i].DigMax) {
			    hdr->CHANNEL[i].DigMax = data[j];
			}
			if(data[j] < hdr->CHANNEL[i].DigMin){
			    hdr->CHANNEL[i].DigMin = data[j];
			}
		    }
		    hdr->CHANNEL[i].OnOff = 1;
      		    hdr->CHANNEL[i].SPR = hdr->SPR; 	    
 	    

		    /* scaling factors */ 
		    hdr->CHANNEL[i].Cal  = atof(channel.FirstChild("value").FirstChild("scale").Element()->Attribute("value"));
		    hdr->CHANNEL[i].Off  = atof(channel.FirstChild("value").FirstChild("origin").Element()->Attribute("value"));
		    hdr->CHANNEL[i].DigMax += 1;
		    hdr->CHANNEL[i].DigMin -= 1;
		    hdr->CHANNEL[i].PhysMax = hdr->CHANNEL[i].DigMax*hdr->CHANNEL[i].Cal + hdr->CHANNEL[i].Off;
		    hdr->CHANNEL[i].PhysMin = hdr->CHANNEL[i].DigMin*hdr->CHANNEL[i].Cal + hdr->CHANNEL[i].Off;

		    /* Physical units */ 
		    strncpy(tmp, channel.FirstChild("value").FirstChild("origin").Element()->Attribute("unit"),20);
 		    hdr->CHANNEL[i].PhysDimCode = PhysDimCode(tmp);
 		    
		    hdr->CHANNEL[i].LowPass  = LowPass;
		    hdr->CHANNEL[i].HighPass = HighPass;
		    hdr->CHANNEL[i].Notch    = Notch;
// 			hdr->CHANNEL[i].XYZ[0]   = l_endian_f32( *(float*) (Header2+ 4*k + 224*hdr->NS) );
// 			hdr->CHANNEL[i].XYZ[1]   = l_endian_f32( *(float*) (Header2+ 4*k + 228*hdr->NS) );
// 			hdr->CHANNEL[i].XYZ[2]   = l_endian_f32( *(float*) (Header2+ 4*k + 232*hdr->NS) );
// 				//memcpy(&hdr->CHANNEL[k].XYZ,Header2 + 4*k + 224*hdr->NS,12);
// 			hdr->CHANNEL[i].Impedance= ldexp(1.0, (uint8_t)Header2[k + 236*hdr->NS]/8);

//		    hdr->CHANNEL[i].Impedance = INF;
//		    for(int k1=0; k1<3; hdr->CHANNEL[index].XYZ[k1++] = 0.0);
		}
		hdr->FLAG.OVERFLOWDETECTION = 0;

		if (VERBOSE_LEVEL>8) {
			fprintf(stdout,"hl7r: [430] %i\n",B4C_ERRNUM); 
			hdr2ascii(hdr,stdout,3);
			fprintf(stdout,"hl7r: [431] %i\n",B4C_ERRNUM); 
		}

	    } else {
		fprintf(stderr, "%s : failed to parse (2)\n", hdr->FileName);
	    }
	}
	else
	    fprintf(stderr, "%s : failed to parse (1)\n", hdr->FileName);

	return(0);

};


int sopen_HL7aECG_write(HDRTYPE* hdr){
	size_t k;
	for (k=0; k<hdr->NS; k++) {
		hdr->CHANNEL[k].GDFTYP = 5; //int32
		hdr->CHANNEL[k].SPR *= hdr->NRec;
	}
	hdr->SPR *= hdr->NRec;
	hdr->NRec = 1; 
	hdr->FILE.OPEN=2;
	return(0);
};


int sclose_HL7aECG_write(HDRTYPE* hdr){
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: HDR structure
		
	Output: 
		char* HDR.AS.Header 	// contains the content which will be written to the file in SOPEN
*/	

	time_t T0;
	struct tm *t0;
    char tmp[80];	
    TiXmlDocument doc;
    
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "UTF-8", "");
    doc.LinkEndChild(decl);
    
	if (VERBOSE_LEVEL>8) fprintf(stdout,"910 %i\n",1);

    TiXmlElement *root = new TiXmlElement("AnnotatedECG");
    root->SetAttribute("xmlns", "urn:hl7-org:v3");
    root->SetAttribute("xmlns:voc", "urn:hl7-org:v3/voc");
    root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    root->SetAttribute("xsi:schemaLocation", "urn:hl7-org:v3/HL7/aECG/2003-12/schema/PORT_MT020001.xsd");
    root->SetAttribute("classCode", "OBS");
    root->SetAttribute("moodCode", "EVN");
    doc.LinkEndChild(root);

    TiXmlElement *rootid = new TiXmlElement("id");
    rootid->SetAttribute("root", strdup(hdr->ID.Recording));
    root->LinkEndChild(rootid);
	
    TiXmlElement *rootCode = new TiXmlElement("code");
    rootCode->SetAttribute("code", "93000");
    rootCode->SetAttribute("codeSystem", "2.16.840.1.113883.6.12");
    rootCode->SetAttribute("codeSystemName", "CPT-4");
    root->LinkEndChild(rootCode);
    
	if (VERBOSE_LEVEL>8) fprintf(stdout,"910 %i\n",2);

	char timelow[24], timehigh[24];
	gdf_time t1,t2;
	t1 = hdr->T0;// + ldexp(timezone/(3600.0*24),32);	
	T0 = gdf_time2t_time(t1);
	t0 = localtime(&T0);
	t2 = tm_time2gdf_time(t0);
	double dT;
	dT = ldexp(t1-t2,-32)*(3600*24);
	dT = round(dT*1000);
	sprintf(timelow, "%4d%2d%2d%2d%2d%2d.%3d", t0->tm_year+1900, t0->tm_mon+1, t0->tm_mday, t0->tm_hour, t0->tm_min, t0->tm_sec,(int)ceil(dT));

	t1 = hdr->T0 + ldexp((hdr->SPR/hdr->SampleRate)/(3600.0*24),32);	
	T0 = gdf_time2t_time(t1);
	t0 = localtime(&T0);
	t2 = tm_time2gdf_time(t0);
	dT = ldexp(t1-t2,-32)*(3600*24);
	dT = floor(dT*1000);
	sprintf(timehigh, "%4d%2d%2d%2d%2d%2d.%3d", t0->tm_year+1900, t0->tm_mon+1, t0->tm_mday, t0->tm_hour, t0->tm_min, t0->tm_sec,(int)ceil(dT));
	for(int i=0; i<18; ++i) {
		if (VERBOSE_LEVEL>8) fprintf(stdout,"920 %i\n",i);
		if(timelow[i] == ' ')
			timelow[i] = '0';
		if(timehigh[i] == ' ')
			timehigh[i] = '0';
	}

	if (VERBOSE_LEVEL>8) fprintf(stdout,"930\n");

    TiXmlElement *effectiveTime = new TiXmlElement("effectiveTime");
    TiXmlElement *effectiveTimeLow = new TiXmlElement("low");
    effectiveTimeLow->SetAttribute("value", timelow);
    effectiveTime->LinkEndChild(effectiveTimeLow);
    TiXmlElement *effectiveTimeHigh = new TiXmlElement("high");
    effectiveTimeHigh->SetAttribute("value", timehigh);
    effectiveTime->LinkEndChild(effectiveTimeHigh);
    root->LinkEndChild(effectiveTime);

	if (VERBOSE_LEVEL>8) fprintf(stdout,"931\n");

    TiXmlElement *rootComponentOf = new TiXmlElement("componentOf");
    rootComponentOf->SetAttribute("typeCode", "COMP");
    rootComponentOf->SetAttribute("contextConductionInd", "true");
    root->LinkEndChild(rootComponentOf);

	if (VERBOSE_LEVEL>8) fprintf(stdout,"932\n");

    TiXmlElement *timePointEvent = new TiXmlElement("timepointEvent");
    timePointEvent->SetAttribute("classCode", "CTTEVENT");
    timePointEvent->SetAttribute("moodCode", "EVN");
    rootComponentOf->LinkEndChild(timePointEvent);
    
    TiXmlElement *timePointComponentOf = new TiXmlElement("componentOf");
    timePointComponentOf->SetAttribute("typeCode", "COMP");
    timePointComponentOf->SetAttribute("contextConductionInd", "true");
    timePointEvent->LinkEndChild(timePointComponentOf);

    TiXmlElement *subjectAssignment = new TiXmlElement("subjectAssignment");
    subjectAssignment->SetAttribute("classCode", "CLNTRL");
    subjectAssignment->SetAttribute("moodCode", "EVN");
    timePointComponentOf->LinkEndChild(subjectAssignment);

    TiXmlElement *subject = new TiXmlElement("subject");
    subject->SetAttribute("typeCode", "SBJ");
    subject->SetAttribute("contextControlCode", "OP");
    subjectAssignment->LinkEndChild(subject);

    TiXmlElement *trialSubject = new TiXmlElement("trialSubject");
    trialSubject->SetAttribute("classCode", "RESBJ");
    subject->LinkEndChild(trialSubject);
    
    if (strlen(hdr->Patient.Id)>0) {	
    	TiXmlElement *trialSubjectId = new TiXmlElement("id");
    	trialSubjectId->SetAttribute("extension", hdr->Patient.Id);
    	trialSubject->LinkEndChild(trialSubjectId);
    }

    TiXmlElement *trialSubjectDemographicPerson = new TiXmlElement("subjectDemographicPerson");
    trialSubjectDemographicPerson->SetAttribute("classCode", "PSN");
    trialSubjectDemographicPerson->SetAttribute("determinerCode", "INSTANCE");
    trialSubject->LinkEndChild(trialSubjectDemographicPerson);

	if (VERBOSE_LEVEL>8) fprintf(stdout,"933\n");

    if (strlen(hdr->Patient.Name)>0)
    if (!hdr->FLAG.ANONYMOUS) 
    {	
	TiXmlElement *subjectDemographicPersonName = new TiXmlElement("name");
    	TiXmlText *nameText = new TiXmlText(hdr->Patient.Name);
    	subjectDemographicPersonName->LinkEndChild(nameText);
    	trialSubjectDemographicPerson->LinkEndChild(subjectDemographicPersonName);
    }
    
    TiXmlElement *subjectDemographicPersonGender = new TiXmlElement("administrativeGenderCode");
    if(hdr->Patient.Sex == 1){
	subjectDemographicPersonGender->SetAttribute("code", "M");
	subjectDemographicPersonGender->SetAttribute("displayName", "Male");
    }
    else if(hdr->Patient.Sex == 2){
	subjectDemographicPersonGender->SetAttribute("code", "F");
	subjectDemographicPersonGender->SetAttribute("displayName", "Female");
    }
    else{
	subjectDemographicPersonGender->SetAttribute("code", "UN");
	subjectDemographicPersonGender->SetAttribute("displayName", "Undefined");
    }
    subjectDemographicPersonGender->SetAttribute("codeSystem", "2.16.840.1.113883.5.1");
    subjectDemographicPersonGender->SetAttribute("codeSystemName", "AdministrativeGender");
    trialSubjectDemographicPerson->LinkEndChild(subjectDemographicPersonGender);

	if (hdr->Patient.Birthday>0) {
		T0 = gdf_time2t_time(hdr->Patient.Birthday);

		t0 = gmtime(&T0);
		// t0 = localtime(&T0);

		// TODO: fixme if "t0->tm_sec"
		sprintf(tmp, "%04d%02d%02d%02d%02d%02d.000", t0->tm_year+1900, t0->tm_mon+1, t0->tm_mday, t0->tm_hour, t0->tm_min, t0->tm_sec);

		TiXmlElement *subjectDemographicPersonBirthtime = new TiXmlElement("birthTime");
		subjectDemographicPersonBirthtime->SetAttribute("value", tmp);
		trialSubjectDemographicPerson->LinkEndChild(subjectDemographicPersonBirthtime);
	}	

	/* write non-standard fields height and weight */
    if (hdr->Patient.Weight) {
    	sprintf(tmp,"%i",hdr->Patient.Weight); 
    	TiXmlElement *subjectDemographicPersonWeight = new TiXmlElement("weight");
    	subjectDemographicPersonWeight->SetAttribute("value", tmp);
    	subjectDemographicPersonWeight->SetAttribute("unit", "kg");
    	trialSubjectDemographicPerson->LinkEndChild(subjectDemographicPersonWeight);
    }
    if (hdr->Patient.Height) {	
    	sprintf(tmp,"%i",hdr->Patient.Height); 
    	TiXmlElement *subjectDemographicPersonHeight = new TiXmlElement("height");
    	subjectDemographicPersonHeight->SetAttribute("value", tmp);
    	subjectDemographicPersonHeight->SetAttribute("unit", "cm");
    	trialSubjectDemographicPerson->LinkEndChild(subjectDemographicPersonHeight);
    }


	if (VERBOSE_LEVEL>8) fprintf(stdout,"937\n");

    TiXmlElement *subjectAssignmentComponentOf = new TiXmlElement("componentOf");
    subjectAssignmentComponentOf->SetAttribute("typeCode", "COMP");
    subjectAssignmentComponentOf->SetAttribute("contextConductionInd", "true");
    subjectAssignment->LinkEndChild(subjectAssignmentComponentOf);

    TiXmlElement *clinicalTrial = new TiXmlElement("clinicalTrial");
    clinicalTrial->SetAttribute("classCode", "CLNTRL");
    clinicalTrial->SetAttribute("moodCode", "EVN");
    subjectAssignmentComponentOf->LinkEndChild(clinicalTrial);

    TiXmlElement *clinicalTrialId = new TiXmlElement("id");
    clinicalTrialId->SetAttribute("root", "GRATZ");
    clinicalTrialId->SetAttribute("extension", "CLINICAL_TRIAL");
    clinicalTrial->LinkEndChild(clinicalTrialId);
    
    TiXmlElement *rootComponent = new TiXmlElement("component");
    rootComponent->SetAttribute("typeCode", "COMP");
    rootComponent->SetAttribute("contextConductionInd", "true");
    root->LinkEndChild(rootComponent);
    
	if (VERBOSE_LEVEL>8) fprintf(stdout,"939\n");

    TiXmlElement *series = new TiXmlElement("series");
    series->SetAttribute("classCode", "OBSSER");
    series->SetAttribute("moodCode", "EVN");
    rootComponent->LinkEndChild(series);
    
    TiXmlElement *seriesCode = new TiXmlElement("code");
    seriesCode->SetAttribute("code", "RHYTHM");
    seriesCode->SetAttribute("seriesCode", "2.16.840.1.113883.5.4");
    series->LinkEndChild(seriesCode);
    
    TiXmlElement *seriesEffectiveTime = new TiXmlElement("effectiveTime");
    TiXmlElement *seriesEffectiveTimeLow = new TiXmlElement("low");
    seriesEffectiveTimeLow->SetAttribute("value", timelow);
    seriesEffectiveTime->LinkEndChild(seriesEffectiveTimeLow);
    TiXmlElement *seriesEffectiveTimeHigh = new TiXmlElement("high");
    seriesEffectiveTimeHigh->SetAttribute("value", timehigh);
    seriesEffectiveTime->LinkEndChild(seriesEffectiveTimeHigh);
    series->LinkEndChild(seriesEffectiveTime);
    
    for(int i=3; i; --i){

	if (VERBOSE_LEVEL>8) fprintf(stdout,"950 %i\n",i);

	    TiXmlElement *seriesControlVariable = new TiXmlElement("controlVariable");
	    seriesControlVariable->SetAttribute("typeCode", "CTRLV");
	    series->LinkEndChild(seriesControlVariable);
	    
	    TiXmlElement *CTRLControlVariable = new TiXmlElement("controlVariable");
	    CTRLControlVariable->SetAttribute("classCode", "OBS");
	    seriesControlVariable->LinkEndChild(CTRLControlVariable);
	    
	    TiXmlElement *controlVariableCode = new TiXmlElement("code");
	    CTRLControlVariable->LinkEndChild(controlVariableCode);
    
	    TiXmlElement *controlVariableComponent = new TiXmlElement("component");
	    controlVariableComponent->SetAttribute("typeCode", "COMP");
	    CTRLControlVariable->LinkEndChild(controlVariableComponent);
	    
	    TiXmlElement *componentControlVariable = new TiXmlElement("controlVariable");
	    componentControlVariable->SetAttribute("classCode", "OBS");
	    controlVariableComponent->LinkEndChild(componentControlVariable);
	    
	    TiXmlElement *componentControlVariableCode = new TiXmlElement("code");
	    componentControlVariable->LinkEndChild(componentControlVariableCode);
	    
	    TiXmlElement *componentControlVariableValue = new TiXmlElement("value");
	    componentControlVariableValue->SetAttribute("xsi:type", "PQ");
	    componentControlVariable->LinkEndChild(componentControlVariableValue);
	    
	    switch(i){
		case 3:
		    controlVariableCode->SetAttribute("code", "MDC_ATTR_FILTER_NOTCH");
		    componentControlVariableCode->SetAttribute("code", "MDC_ATTR_NOTCH_FREQ");
		    componentControlVariableValue->SetDoubleAttribute("value", hdr->CHANNEL[0].Notch);
		    break;
		case 2:		    
		    controlVariableCode->SetAttribute("code", "MDC_ATTR_FILTER_LOW_PASS");
		    componentControlVariableCode->SetAttribute("code", "MDC_ATTR_FILTER_CUTOFF_FREQ");
		    componentControlVariableValue->SetDoubleAttribute("value", hdr->CHANNEL[0].LowPass);
		    break;
		case 1:
		    controlVariableCode->SetAttribute("code", "MDC_ATTR_FILTER_HIGH_PASS");
		    componentControlVariableCode->SetAttribute("code", "MDC_ATTR_FILTER_CUTOFF_FREQ");
		    componentControlVariableValue->SetDoubleAttribute("value", hdr->CHANNEL[0].HighPass);
		    break;
	    }
	    
	    controlVariableCode->SetAttribute("codeSystem", "2.16.840.1.113883.6.24");
	    controlVariableCode->SetAttribute("codeSystemName", "MDC");
	    componentControlVariableCode->SetAttribute("codeSystem", "2.16.840.1.113883.6.24");
	    componentControlVariableCode->SetAttribute("codeSystemName", "MDC");
	    componentControlVariableValue->SetAttribute("unit", "Hz");
	    
	    switch(i){
		case 3:
		    controlVariableCode->SetAttribute("displayName", "Notch Filter");
		    componentControlVariableCode->SetAttribute("displayName", "Notch Frequency");
		    break;
		case 2:		    
		    controlVariableCode->SetAttribute("displayName", "Low Pass Filter");
		    componentControlVariableCode->SetAttribute("displayName", "Cutoff Frequency");
		    break;
		case 1:
		    controlVariableCode->SetAttribute("displayName", "High Pass Filter");
		    componentControlVariableCode->SetAttribute("displayName", "Cutoff Frequency");
		    break;
	    }
    }
    
    TiXmlElement *seriesComponent = new TiXmlElement("component");
    seriesComponent->SetAttribute("typeCode", "COMP");
    seriesComponent->SetAttribute("contextConductionInd", "true");
    series->LinkEndChild(seriesComponent);
    
    TiXmlElement *sequenceSet = new TiXmlElement("sequenceSet");
    sequenceSet->SetAttribute("classCode", "OBSCOR");
    sequenceSet->SetAttribute("moodCode", "EVN");
    seriesComponent->LinkEndChild(sequenceSet);
    
    TiXmlElement *sequenceSetComponent = new TiXmlElement("component");
    sequenceSetComponent->SetAttribute("typeCode", "COMP");
    sequenceSetComponent->SetAttribute("contextConductionInd", "true");
    sequenceSet->LinkEndChild(sequenceSetComponent);
    
    TiXmlElement *sequence = new TiXmlElement("sequence");
    sequence->SetAttribute("classCode", "OBS");
    sequence->SetAttribute("moodCode", "EVN");    
    sequenceSetComponent->LinkEndChild(sequence);
    
    TiXmlElement *sequenceCode = new TiXmlElement("code");
    sequenceCode->SetAttribute("code", "TIME_ABSOLUTE");
    sequenceCode->SetAttribute("codeSystem", "2.16.840.1.113883.6.24");
    sequence->LinkEndChild(sequenceCode);

    TiXmlElement *sequenceValue = new TiXmlElement("value");
    sequenceValue->SetAttribute("xsi:type", "GLIST_TS");
    sequence->LinkEndChild(sequenceValue);

    TiXmlElement *valueHead = new TiXmlElement("head");
    valueHead->SetAttribute("value", timelow);
    valueHead->SetAttribute("unit", "s");   // TODO: value is date/time of the day - unit=[s] does not make sense 
    sequenceValue->LinkEndChild(valueHead);

    TiXmlElement *valueIncrement = new TiXmlElement("increment");
    valueIncrement->SetDoubleAttribute("value", 1/hdr->SampleRate);
    valueIncrement->SetAttribute("unit", "s");
    sequenceValue->LinkEndChild(valueIncrement);

    TiXmlText *digitsText;

    for(int i=0; i<hdr->NS; ++i)
    if (hdr->CHANNEL[i].OnOff)
    {

	if (VERBOSE_LEVEL>8) fprintf(stdout,"960 %i\n",i);


	sequenceSetComponent = new TiXmlElement("component");
	sequenceSetComponent->SetAttribute("typeCode", "COMP");
	sequenceSetComponent->SetAttribute("contextConductionInd", "true");
	sequenceSet->LinkEndChild(sequenceSetComponent);

	sequence = new TiXmlElement("sequence");
	sequence->SetAttribute("classCode", "OBS");
	sequence->SetAttribute("moodCode", "EVN");
	sequenceSetComponent->LinkEndChild(sequence);

	sequenceCode = new TiXmlElement("code");
	
	if (hdr->CHANNEL[i].LeadIdCode) {
		strcpy(tmp,"MDC_ECG_LEAD_");
		strcat(tmp,LEAD_ID_TABLE[hdr->CHANNEL[i].LeadIdCode]);
	}
	else 
		strcpy(tmp,hdr->CHANNEL[i].Label);
	sequenceCode->SetAttribute("code", tmp);

	sequenceCode->SetAttribute("codeSystem", "2.16.840.1.113883.6.24");
	sequenceCode->SetAttribute("codeSystemName", "MDC");
	sequence->LinkEndChild(sequenceCode);
    
	sequenceValue = new TiXmlElement("value");
	sequenceValue->SetAttribute("xsi:type", "SLIST_PQ");
	sequence->LinkEndChild(sequenceValue);

	// store physical unit in tmp 
	PhysDim(hdr->CHANNEL[i].PhysDimCode,tmp); 

	valueHead = new TiXmlElement("origin");
	valueHead->SetDoubleAttribute("value", hdr->CHANNEL[i].Off);
	// valueHead->SetDoubleAttribute("value", 0);
	valueHead->SetAttribute("unit", tmp);
	sequenceValue->LinkEndChild(valueHead);

	valueIncrement = new TiXmlElement("scale");
	valueIncrement->SetDoubleAttribute("value", hdr->CHANNEL[i].Cal);
	//valueIncrement->SetDoubleAttribute("value", 1);
	valueIncrement->SetAttribute("unit", tmp);
	sequenceValue->LinkEndChild(valueIncrement);

	TiXmlElement *valueDigits = new TiXmlElement("digits");
	sequenceValue->LinkEndChild(valueDigits);

	std::stringstream digitsStream;

	if (VERBOSE_LEVEL>8) fprintf(stdout,"967 %i\n",i);

	for(unsigned int j=0; j<hdr->CHANNEL[i].SPR; ++j) {

	    digitsStream << (*(int32_t*)(hdr->AS.rawdata + hdr->AS.bi[i] + (j*GDFTYP_BITS[hdr->CHANNEL[i].GDFTYP]>>3))) << " ";
//	    digitsStream << hdr->data.block[hdr->SPR*i + j] << " ";
	}

	if (VERBOSE_LEVEL>8) fprintf(stdout,"970 %i\n",i);


	digitsText = new TiXmlText(digitsStream.str().c_str());
	valueDigits->LinkEndChild(digitsText);
    }

	if (VERBOSE_LEVEL>8) fprintf(stdout,"980\n");
    doc.SaveFile(hdr->FileName);
//    doc.SaveFile(hdr);
	if (VERBOSE_LEVEL>8) fprintf(stdout,"989\n");

    return(0);
};
