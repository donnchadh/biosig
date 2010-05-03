/*

    $Id: sopen_hl7aecg.c,v 1.36 2009/04/09 13:54:04 schloegl Exp $
    Copyright (C) 2006,2007,2009 Alois Schloegl <a.schloegl@ieee.org>
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


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
                convert time in string format into gdf-time format
                Currently, the following formats are supported 
                        YYYYMMDDhhmmss.uuuuuu        
                        YYYYMMDDhhmmss        
                        YYYYMMDD
                in case of error, zero is returned        
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
EXTERN_C gdf_time str_time2gdf_time(const char *t1) {
        
        
	struct tm t0; 
	gdf_time T;
	double fracsec = 0.0; 
        double f;
        int len = strlen(t1);
#define MAXLEN  22        
	char t[MAXLEN+1];

        if (len>MAXLEN) return(0);
	strncpy(t,t1,MAXLEN);
        t[MAXLEN] = 0;	
 
        if (VERBOSE_LEVEL>8) 
                fprintf(stdout,"str_time2gdf_time: [%i]<%s>\n",len,t1);
 
        char *p = strrchr(t,'.');
        if (p==NULL) {
                // no comma
                p = t+len; 
        }
        else { 
                for (p++, f=0.1; p[0]; p++, f=f/10) {
                        if (p[0]<'0' || p[0]>'9') return(0);
                        fracsec += (p[0]-'0')*f;
                }        
                p = strrchr(t,'.');
        }       

        if (VERBOSE_LEVEL>8) 
                fprintf(stdout,"str_time2gdf_time: [%i]<%s>\n",len,t1);
 
        if (len>=14) {
                // decode hhmmss
        	p[0] = '\0'; p-=2;
	        t0.tm_sec  = atoi(p);  	
	        p[0] = '\0'; p-=2;
	        t0.tm_min  = atoi(p);
	        p[0] = '\0'; p-=2;
	        t0.tm_hour = atoi(p);
        	p[0] = '\0'; 
	}
	else {
	        t0.tm_sec  = 0;  	
	        t0.tm_min  = 0;
	        t0.tm_hour = 0;
	}
	p -= 2;
	t0.tm_mday = atoi(p);

	p[0] = '\0'; p-=2;
	t0.tm_mon  = atoi(p)-1;

	p[0] = '\0'; p-=4;
	t0.tm_year = atoi(t)-1900;
	t0.tm_isdst  = -1;
	T = tm_time2gdf_time(&t0);

	if (fracsec>0)
	        T += ldexp(fracsec/86400,32);
	        
        if (VERBOSE_LEVEL>8) 
                fprintf(stdout,"str_time2gdf_time: [%i]<%s>\n",len,t1);
 
        return(T);
}


EXTERN_C int sopen_HL7aECG_read(HDRTYPE* hdr) {
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

	if (VERBOSE_LEVEL>8)
		fprintf(stdout,"hl7r: [411]\n"); 

	if(doc.LoadFile()){

		if (VERBOSE_LEVEL>8) fprintf(stdout,"hl7r: [412]\n"); 


	    TiXmlHandle hDoc(&doc);
		if (VERBOSE_LEVEL>8) fprintf(stdout,"hl7r: [412]\n"); 
	    TiXmlHandle geECG = hDoc.FirstChild("CardiologyXML");
	    TiXmlHandle IHE = hDoc.FirstChild("IHEDocumentList");
	    TiXmlHandle aECG = hDoc.FirstChild("AnnotatedECG");
		if (VERBOSE_LEVEL>8) fprintf(stdout,"hl7r: [412]\n"); 
	    TiXmlHandle SierraECG = hDoc.FirstChild("restingECG");
		if (VERBOSE_LEVEL>8) fprintf(stdout,"hl7r: [412]\n"); 
	    if (SierraECG.Element()) {
		fprintf(stdout,"Great! Philips Sierra ECG is recognized\n");
	    }	    
	    else if (geECG.Element()) {

			struct tm t0; 
			t0.tm_hour = atoi(geECG.FirstChild("ObservationDateTime").FirstChild("Hour").Element()->GetText());
			t0.tm_min  = atoi(geECG.FirstChild("ObservationDateTime").FirstChild("Minute").Element()->GetText());
			t0.tm_sec  = atoi(geECG.FirstChild("ObservationDateTime").FirstChild("Second").Element()->GetText());
			t0.tm_mday = atoi(geECG.FirstChild("ObservationDateTime").FirstChild("Day").Element()->GetText());
			t0.tm_mon  = atoi(geECG.FirstChild("ObservationDateTime").FirstChild("Month").Element()->GetText())-1;
			t0.tm_year = atoi(geECG.FirstChild("ObservationDateTime").FirstChild("Year").Element()->GetText())-1900;
			hdr->T0    = tm_time2gdf_time(&t0);

			hdr->ID.Manufacturer.Name = "GE";
			strncpy(hdr->ID.Manufacturer._field, geECG.FirstChild("Device-Type").Element()->GetText(),MAX_LENGTH_PID);
			hdr->ID.Manufacturer.Model = hdr->ID.Manufacturer._field;			

			strncpy(hdr->Patient.Id, geECG.FirstChild("PatientInfo").FirstChild("PID").Element()->GetText(),MAX_LENGTH_PID);
			const char *tmp = geECG.FirstChild("PatientInfo").FirstChild("PID").Element()->GetText();
			hdr->Patient.Sex = (toupper(tmp[0])=='M') + 2*(toupper(tmp[0])=='F');
			if (!hdr->FLAG.ANONYMOUS) {
				strncpy(hdr->Patient.Name, geECG.FirstChild("PatientInfo").FirstChild("Name").FirstChild("FamilyName").Element()->GetText(),MAX_LENGTH_PID);
				strncat(hdr->Patient.Name, " ",MAX_LENGTH_PID);
				strncat(hdr->Patient.Name, geECG.FirstChild("PatientInfo").FirstChild("Name").FirstChild("GivenName").Element()->GetText(),MAX_LENGTH_PID);
			}
			hdr->NS = atoi(geECG.FirstChild("StripData").FirstChild("NumberOfLeads").Element()->GetText());
			hdr->SPR = 1;
			hdr->NRec = atoi(geECG.FirstChild("StripData").FirstChild("ChannelSampleCountTotal").Element()->GetText());
			hdr->SampleRate = atof(geECG.FirstChild("StripData").FirstChild("SampleRate").Element()->GetText());
			double Cal = atof(geECG.FirstChild("StripData").FirstChild("Resolution").Element()->GetText());

			double LP = atof(geECG.FirstChild("FilterSetting").FirstChild("LowPass").Element()->GetText());
			double HP = atof(geECG.FirstChild("FilterSetting").FirstChild("HighPass").Element()->GetText());
			double Notch = 0; 
			if (!strcmpi("yes",geECG.FirstChild("FilterSetting").FirstChild("Filter50Hz").Element()->GetText()))
				Notch = 50; 
			else if (!strcmpi("yes",geECG.FirstChild("FilterSetting").FirstChild("Filter60Hz").Element()->GetText()))
				Notch = 60; 
			
			uint16_t gdftyp = 3; 
			hdr->AS.bpb = 0; 
			hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));
			hdr->AS.rawdata = (uint8_t*) calloc(hdr->NS,hdr->NRec*hdr->SPR*GDFTYP_BITS[gdftyp]>>3);

			TiXmlElement *C = geECG.FirstChild("StripData").FirstChild("WaveformData").Element();
			for (int k=0; C && (k<hdr->NS); k++) {
				CHANNEL_TYPE *hc = hdr->CHANNEL + k;
				// default values 
				hc->GDFTYP	= gdftyp;	
				hc->PhysDimCode	= 4275; //PhysDimCode("uV");	
				hc->DigMin 	= (double)(int16_t)0x8000;			
				hc->DigMax	= (double)(int16_t)0x7fff;	
				strncpy(hc->Label, C->Attribute("lead"), MAX_LENGTH_LABEL);

				hc->LeadIdCode	= 0;
				size_t j;
				for (j=0; strcmpi(hdr->CHANNEL[k].Label, LEAD_ID_TABLE[j]) && LEAD_ID_TABLE[j][0]; j++) {}; 
				if (LEAD_ID_TABLE[j][0])	
					hdr->CHANNEL[k].LeadIdCode = j;

				hc->LowPass	= LP;
				hc->HighPass	= HP;
				hc->Notch	= Notch;
				hc->Impedance	= NaN;
				hc->XYZ[0] 	= 0.0;
				hc->XYZ[1] 	= 0.0;
				hc->XYZ[2] 	= 0.0;
				
				// defined 
				hc->Cal		= Cal;
				hc->Off		= 0.0;
				hc->SPR		= 1; 
				hc->OnOff	= 1;	
				hc->bi  	= hdr->AS.bpb;
				hdr->AS.bpb    += hc->SPR * GDFTYP_BITS[hc->GDFTYP]>>3;

				    /* read data samples */	
				std::vector<std::string> vector;
				stringtokenizer(vector, C->GetText(), ",");
				
				int16_t* data = (int16_t*)(hdr->AS.rawdata);
				hc->DigMax	= 0; 
				hc->DigMin	= 0; 
				for(j=0; j<vector.size(); ++j) {
					int d = atoi(vector[j].c_str());

					data[j*hdr->NS+k] = d;
					/* get Min/Max */
					if(d > hc->DigMax) hc->DigMax = d;
					if(d < hc->DigMin) hc->DigMin = d;
				}
				for(; j<hdr->NRec; ++j) {
					data[j*hdr->NS+k] = (double)(int16_t)0x8000;	// set to NaN 
				}

				hc->PhysMax	= hc->DigMax * hc->Cal + hc->Off; 
				hc->PhysMin	= hc->DigMin * hc->Cal + hc->Off; 

				C = C->NextSiblingElement();
			}
			hdr->AS.first = 0; 	
			hdr->AS.length = hdr->NRec; 	

	    }
	    else if (IHE.Element()) {

		fprintf(stderr,"XML IHE: support for IHE XML is experimental - some important features are not implmented yet \n"); 

		TiXmlHandle activityTime = IHE.FirstChild("activityTime");
		TiXmlHandle recordTarget = IHE.FirstChild("recordTarget");
		TiXmlHandle author = IHE.FirstChild("author");
		/* 
			an IHE file can contain several segments (i.e. components)
		 	need to implement TARGET_SEGMENT feature
		*/
		TiXmlHandle component = IHE.FirstChild("component");

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"IHE: [413] \n"); 

#if 0 
		// this is not HDR.T0 !!!!!!!
		if (activityTime.Element()) {
 			hdr->T0 = str_time2gdf_time(activityTime.Element()->Attribute("value"));
		}	
#endif 

		if (author.FirstChild("assignedAuthor").Element()) {
			// TiXmlHandle noteText = author.FirstChild("noteText").Element();
			TiXmlHandle assignedAuthor = author.FirstChild("assignedAuthor").Element();
			if (assignedAuthor.FirstChild("assignedDevice").Element()) {
				TiXmlHandle assignedDevice = assignedAuthor.FirstChild("assignedDevice").Element();
				hdr->ID.Manufacturer.Name = hdr->ID.Manufacturer._field;
				
				if (assignedDevice.Element()) {
	 				strncpy(hdr->ID.Manufacturer._field, assignedDevice.FirstChild("manufacturerModelName").Element()->GetText(), MAX_LENGTH_MANUF);	
					int len = strlen(hdr->ID.Manufacturer._field)+1;
					hdr->ID.Manufacturer.Model = hdr->ID.Manufacturer._field+len;
					strncpy(hdr->ID.Manufacturer._field+len, assignedDevice.FirstChild("code").Element()->Attribute("code"),MAX_LENGTH_MANUF-len);
					len += strlen(hdr->ID.Manufacturer.Model)+1;
				}
			}	
		}	

		if (recordTarget.FirstChild("patient").Element()) {
			TiXmlHandle patient = recordTarget.FirstChild("patient").Element();

			TiXmlHandle id = patient.FirstChild("id").Element();
			TiXmlHandle patientPatient = patient.FirstChild("patientPatient").Element();
			TiXmlHandle providerOrganization = patient.FirstChild("providerOrganization").Element();
			
			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"IHE: [414] %p %p %p\n",id.Element(),patientPatient.Element(),providerOrganization.Element()); 

			if (id.Element()) {	
			    	char *strtmp = strdup(id.Element()->Attribute("root"));
			    	size_t len = strlen(strtmp); 
				strncpy(hdr->ID.Recording,strtmp,MAX_LENGTH_RID);
				free(strtmp); 
				strncat(hdr->ID.Recording," ",MAX_LENGTH_RID);
			    	strtmp = strdup(id.Element()->Attribute("extension"));
			    	len += 1+strlen(strtmp); 
				strncat(hdr->ID.Recording,strtmp,MAX_LENGTH_RID);
				free(strtmp); 
		    		if (len>MAX_LENGTH_RID)	
					fprintf(stdout,"Warning HL7aECG(read): length of Recording ID exceeds maximum length %i>%i\n",len,MAX_LENGTH_PID); 
				fprintf(stdout,"IHE (read): length of Recording ID %i,%i\n",len,MAX_LENGTH_PID); 
			}	
			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"IHE: [414] RID= %s\n",hdr->ID.Recording); 
			
			if (providerOrganization.Element()) {
				hdr->ID.Hospital = strdup(providerOrganization.FirstChild("name").Element()->GetText());
			}	
			
			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"IHE: [414] hospital %s\n",hdr->ID.Hospital); 

			if (patientPatient.Element()) {
				if (!hdr->FLAG.ANONYMOUS) {
				TiXmlHandle Name = patientPatient.FirstChild("name").Element();
				if (Name.Element()) {
					strncpy(hdr->Patient.Name, Name.FirstChild("family").Element()->GetText(), MAX_LENGTH_NAME);
					strncat(hdr->Patient.Name, ", ", MAX_LENGTH_NAME);
					strncat(hdr->Patient.Name, Name.FirstChild("given").Element()->GetText(), MAX_LENGTH_NAME);
				}
				}
				TiXmlHandle Gender = patientPatient.FirstChild("administrativeGenderCode").Element();
				TiXmlHandle Birth = patientPatient.FirstChild("birthTime").Element();

				if (Gender.Element()) {
					const char *gender = Gender.Element()->Attribute("code");
					hdr->Patient.Sex = (tolower(gender[0])=='m') + (tolower(gender[0])=='f');
				}
				if (Birth.Element()) {
			 		hdr->Patient.Birthday = str_time2gdf_time(Birth.Element()->Attribute("value"));
				}
			}
		}
		
		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"IHE: [415] \n"); 

	    }
	    else if(aECG.Element()){

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"hl7r: [412]\n"); 

	    	size_t len = strlen(aECG.FirstChild("id").Element()->Attribute("root")); 

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"hl7r: [413]\n"); 

		strncpy(hdr->ID.Recording,aECG.FirstChild("id").Element()->Attribute("root"),MAX_LENGTH_RID);
	    	if (len>MAX_LENGTH_RID)	
			fprintf(stdout,"Warning HL7aECG(read): length of Recording ID exceeds maximum length %i>%i\n",len,MAX_LENGTH_PID); 

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"hl7r: [414]\n"); 


		TiXmlHandle effectiveTime = aECG.FirstChild("effectiveTime");

		char *T0 = NULL;
		if(effectiveTime.FirstChild("low").Element())
		    T0 = (char *)effectiveTime.FirstChild("low").Element()->Attribute("value");
		else if(effectiveTime.FirstChild("center").Element())
		    T0 = (char *)effectiveTime.FirstChild("center").Element()->Attribute("value");

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"hl7r: [413 2] <%s>\n", T0); 

                if (T0 != NULL) 
                        hdr->T0 = str_time2gdf_time(T0);

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"hl7r: [413 4]\n"); 

		TiXmlHandle demographic = aECG.FirstChild("componentOf").FirstChild("timepointEvent").FirstChild("componentOf").FirstChild("subjectAssignment").FirstChild("subject").FirstChild("trialSubject");

		TiXmlElement *id = demographic.FirstChild("id").Element();
		if(id) {
			const char* tmpstr = id->Attribute("extension");
			size_t len = strlen(tmpstr); 
			if (len>MAX_LENGTH_PID)
				fprintf(stdout,"Warning HL7aECG(read): length of Patient Id exceeds maximum length %i>%i\n",len,MAX_LENGTH_PID); 
		    	strncpy(hdr->Patient.Id,tmpstr,MAX_LENGTH_PID);
		}    

		if (VERBOSE_LEVEL>7)
			fprintf(stdout,"hl7r: [413]\n"); 

		if (!hdr->FLAG.ANONYMOUS) 
		{
			demographic = demographic.FirstChild("subjectDemographicPerson");
			TiXmlElement *Name1 = demographic.FirstChild("name").Element();


			if (Name1 != NULL) {
				const char *name = Name1->GetText();
				if (name != NULL) {
					size_t len = strlen(name);
	
					if (len>MAX_LENGTH_NAME)
						fprintf(stdout,"Warning HL7aECG(read): length of Patient Name exceeds maximum length %i>%i\n",len,MAX_LENGTH_PID); 
					strncpy(hdr->Patient.Name, name, MAX_LENGTH_NAME);
				}	
				else {
					fprintf(stderr,"Warning: composite subject name is not supported.\n");
					for (int k=1;k<40;k++)
						fprintf(stderr,"%c.",((char*)Name1)[k]);

					//hdr->Patient.Name[0] = 0;
/*
				### FIXME: support of composite patient name.  

				const char *Name11 = Name1->Attribute("family");
				fprintf(stdout,"Patient Family Name %p\n", Name11);
				char *Name2 = Name.FirstChild("given")->GetText();

				if ((Name1!=NULL) || (Name2!=NULL)) {
					strncpy(hdr->Patient.Name, Name1, MAX_LENGTH_NAME);
				}
*/
				}	
			}	
			else {
				hdr->Patient.Name[0] = 0;
				fprintf(stderr,"Warning: Patient Name not available could not be read.\n");
			}	
		}		

		if (VERBOSE_LEVEL>7)
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

		if (VERBOSE_LEVEL>7)
			fprintf(stdout,"hl7r: [415]\n"); 

		if (height) {
		    uint16_t code = PhysDimCode(strcpy(tmp,height->Attribute("unit")));	
		    if ((code & 0xFFE0) != 1280) 
		    	fprintf(stderr,"Warning: incorrect height unit (%s) %i \n",height->Attribute("unit"),code);	
		    else	// convert to centimeter
			hdr->Patient.Height = (uint8_t)(atof(height->Attribute("value"))*PhysDimScale(code)*1e+2);
		}
		
		if (VERBOSE_LEVEL>7)
			fprintf(stdout,"hl7r: [416]\n"); 

		TiXmlElement *birthday = demographic.FirstChild("birthTime").Element();
		if(birthday){
		    T0 = (char *)birthday->Attribute("value");
		    if (T0==NULL) T0=(char *)birthday->GetText();  // workaround for reading two different formats 
		    hdr->Patient.Birthday = str_time2gdf_time(T0);
		}

		if (VERBOSE_LEVEL>8)
			fprintf(stdout,"hl7r: [417]\n"); 

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
		
                /*************** Annotations **********************/
		TiXmlHandle AnnotationSet = aECG.FirstChild("component").FirstChild("series").FirstChild("subjectOf").FirstChild("annotationSet");
		TiXmlHandle Annotation = AnnotationSet.Child("component", 1).FirstChild("annotation").FirstChild("component").FirstChild("annotation"); 
		size_t N_Event = 0, N=14; 
		for(int i = 1; i<12; ++i) {
        		for(int j = 0; j<3; ++j) {

		                Annotation = AnnotationSet.Child("component", i).FirstChild("annotation").Child("component",j).FirstChild("annotation");

        		        const char *code = Annotation.FirstChild("value").Element()->Attribute("code");

                                uint16_t EventTyp1 = 0, EventTyp2 = 0;

                                if (!strcmp(code,"MDC_ECG_WAVC_PWAVE")) {
                                        EventTyp1 = 0x0502;
                                        EventTyp2 = 0x8502;
                                }
                                else if (!strcmp(code,"MDC_ECG_WAVC_QRSWAVE")) {
                                        EventTyp1 = 0x0503;
                                        EventTyp2 = 0x8505;
                                }
                                else if (!strcmp(code,"MDC_ECG_WAVC_TWAVE")) {
                                        EventTyp1 = 0x0506;
                                        EventTyp2 = 0x8506;
                                }    

                                if ((N+3) > N_Event) {
                                	N_Event = 2*(N+2);
                                	hdr->EVENT.TYP = (typeof(hdr->EVENT.TYP)) realloc(hdr->EVENT.TYP,N_Event*sizeof(*hdr->EVENT.TYP));
                                	hdr->EVENT.POS = (typeof(hdr->EVENT.POS)) realloc(hdr->EVENT.POS,N_Event*sizeof(*hdr->EVENT.POS));
                                }

        		        TiXmlHandle Boundary = Annotation.FirstChild("support").FirstChild("supportingROI").FirstChild("component").FirstChild("boundary").FirstChild("value");

                                int64_t pos1=0, pos2=0;
        		        if (Boundary.FirstChild("low").Element()) {
                                        const char *tmpstr = (Boundary.FirstChild("low").Element()->Attribute("value"));
                                        pos1 = (ldexp((str_time2gdf_time(tmpstr)-hdr->T0)*86400*hdr->SampleRate,-32));
                                        hdr->EVENT.TYP[N] = EventTyp1;
                                        hdr->EVENT.POS[N] = pos1; 
                                        N++;        
                                }        

        		        if (Boundary.FirstChild("high").Element()) {
                                        const char *tmpstr = (Boundary.FirstChild("high").Element()->Attribute("value"));
                                        pos2 = (ldexp((str_time2gdf_time(tmpstr)-hdr->T0)*86400*hdr->SampleRate,-32));
                                        hdr->EVENT.TYP[N] = EventTyp2;
                                        hdr->EVENT.POS[N] = pos2;
                                        N++;        
                                }        
               		}
       		}
       		hdr->EVENT.N = N;

		TiXmlHandle channel = channels.Child("component", 1).FirstChild("sequence");
		for(hdr->NS = 0; channel.Element(); ++(hdr->NS), channel = channels.Child("component", hdr->NS+1).FirstChild("sequence")) {};
		hdr->CHANNEL = (CHANNEL_TYPE*) calloc(hdr->NS,sizeof(CHANNEL_TYPE));

		channel = channels.Child("component", 1).FirstChild("sequence");
		hdr->AS.bpb = 0; 
		for(int i = 0; channel.Element(); ++i, channel = channels.Child("component", i+1).FirstChild("sequence")){

		    const char *code = channel.FirstChild("code").Element()->Attribute("code");
		    
  		    CHANNEL_TYPE *hc = hdr->CHANNEL+i;
  		    if (VERBOSE_LEVEL>8)
				fprintf(stdout,"hl7r: [420] %i\n",i); 

		    strncpy(hc->Label,code,min(40,MAX_LENGTH_LABEL));
		    hc->Label[MAX_LENGTH_LABEL] = '\0';
		    hc->Transducer[0] = '\0';
		    hc->GDFTYP = 16;	// float32

		    std::vector<std::string> vector;
		    stringtokenizer(vector, channel.FirstChild("value").FirstChild("digits").Element()->GetText());

		    hc->SPR = vector.size();
		    if (i==0) {
		    	hdr->SPR = hc->SPR;
			hdr->AS.rawdata = (uint8_t *)realloc(hdr->AS.rawdata, 4*hdr->NS*hdr->SPR*hdr->NRec);
		    }
		    else if (hdr->SPR != hc->SPR) {
			if (hdr->SPR != lcm(hdr->SPR, hc->SPR)) 
			{
				fprintf(stderr,"Error: number of samples %i of #%i differ from %i in #0.\n",hc->SPR,i+1,hdr->SPR);
				B4C_ERRNUM = B4C_UNSPECIFIC_ERROR;
				B4C_ERRMSG = "HL7aECG: initial sample rate is not a multiple of all samplerates";
				exit(-5);
			}	
			
		    }	

		    /* read data samples */	
		    float *data = (float*)(hdr->AS.rawdata + (GDFTYP_BITS[hc->GDFTYP]>>3)*i*(hdr->SPR));
		    size_t DIV = hdr->SPR/hc->SPR;
		    for(size_t j=0; j<hc->SPR; ++j) {
			size_t k=0;
			data[j*DIV+k] = atof(vector[j].c_str());
			while (++k<DIV) data[j*DIV+k] = data[j*DIV+k-1]; 
			  
			/* get Min/Max */
			if(data[j] > hc->DigMax) {
			    hc->DigMax = data[j];
			}
			if(data[j] < hc->DigMin){
			    hc->DigMin = data[j];
			}
		    }
		    hc->OnOff = 1;
#ifndef NO_BI
		    hc->bi = hdr->AS.bpb;
#endif 
  		    hdr->AS.bpb += hc->SPR*GDFTYP_BITS[hc->GDFTYP]>>3;
 	    

		    /* scaling factors */ 
		    hc->Cal  = atof(channel.FirstChild("value").FirstChild("scale").Element()->Attribute("value"));
		    hc->Off  = atof(channel.FirstChild("value").FirstChild("origin").Element()->Attribute("value"));
		    hc->DigMax += 1;
		    hc->DigMin -= 1;
		    hc->PhysMax = hc->DigMax*hc->Cal + hc->Off;
		    hc->PhysMin = hc->DigMin*hc->Cal + hc->Off;

		    /* Physical units */ 
		    strncpy(tmp, channel.FirstChild("value").FirstChild("origin").Element()->Attribute("unit"),20);
 		    hc->PhysDimCode = PhysDimCode(tmp);
 		    
		    hc->LowPass  = LowPass;
		    hc->HighPass = HighPass;
		    hc->Notch    = Notch;
// 			hc->XYZ[0]   = l_endian_f32( *(float*) (Header2+ 4*k + 224*hdr->NS) );
// 			hc->XYZ[1]   = l_endian_f32( *(float*) (Header2+ 4*k + 228*hdr->NS) );
// 			hc->XYZ[2]   = l_endian_f32( *(float*) (Header2+ 4*k + 232*hdr->NS) );
// 				//memcpy(&hdr->CHANNEL[k].XYZ,Header2 + 4*k + 224*hdr->NS,12);
// 			hc->Impedance= ldexp(1.0, (uint8_t)Header2[k + 236*hdr->NS]/8);

//		    hc->Impedance = INF;
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

EXTERN_C void sopen_HL7aECG_write(HDRTYPE* hdr) {
	size_t k;
	for (k=0; k<hdr->NS; k++) {
		hdr->CHANNEL[k].GDFTYP = 16; //float32
		hdr->CHANNEL[k].SPR *= hdr->NRec;
	}
	hdr->SPR *= hdr->NRec;
	hdr->NRec = 1; 
	hdr->FILE.OPEN=2;
	return;
};

EXTERN_C int sclose_HL7aECG_write(HDRTYPE* hdr){
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: HDR structure
		
	Output: 
		char* HDR.AS.Header 	// contains the content which will be written to the file in SOPEN
*/	

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
	t0 = gdf_time2tm_time(t1);
	t2 = tm_time2gdf_time(t0);
	double dT;
	dT = ldexp(t1-t2,-32)*(3600*24);
	dT = round(dT*1000);
	sprintf(timelow, "%4d%2d%2d%2d%2d%2d.%3d", t0->tm_year+1900, t0->tm_mon+1, t0->tm_mday, t0->tm_hour, t0->tm_min, t0->tm_sec,(int)ceil(dT));

	t1 = hdr->T0 + ldexp((hdr->SPR/hdr->SampleRate)/(3600.0*24),32);	
	t0 = gdf_time2tm_time(t1);
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
		t0 = gdf_time2tm_time(hdr->Patient.Birthday);

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

#ifdef NO_BI
    size_t bi = 0; 
#endif
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

	if (VERBOSE_LEVEL>7) fprintf(stdout,"[967] %i %f\n",i,*(float*)(hdr->AS.rawdata + hdr->CHANNEL[i].bi));

	size_t sz = GDFTYP_BITS[hdr->CHANNEL[i].GDFTYP]>>3;
	for (unsigned int j=0; j<hdr->CHANNEL[i].SPR; ++j) {
#ifndef NO_BI
	    	digitsStream << (*(float*)(hdr->AS.rawdata + hdr->CHANNEL[i].bi + (j*sz))) << " ";
	}
#else
	    	digitsStream << (*(float*)(hdr->AS.rawdata + bi + (j*sz))) << " ";
	}
	bi += hdr->CHANNEL[i].SPR*sz;
#endif
	if (VERBOSE_LEVEL>8) fprintf(stdout,"970 %i \n",i);
//	if (VERBOSE_LEVEL>8) fprintf(stdout,"<%s>\n",digitsStream.str().c_str());

	digitsText = new TiXmlText(digitsStream.str().c_str());
	valueDigits->LinkEndChild(digitsText);
    }

	if (VERBOSE_LEVEL>8) fprintf(stdout,"980 [%i]\n", hdr->FILE.COMPRESSION);
	doc.SaveFile(hdr->FileName, hdr->FILE.COMPRESSION);
//	doc.SaveFile(hdr);
	if (VERBOSE_LEVEL>8) fprintf(stdout,"989\n");

    return(0);
};
