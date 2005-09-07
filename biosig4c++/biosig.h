/*
%
% $Id: biosig.h,v 1.1 2005-09-07 14:08:32 schloegl Exp $
% Author: Alois Schloegl <a.schloegl@ieee.org>
% Copyright (C) 2000,2005 A.Schloegl
% 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

/* 

	Headers for the BioSig4C library
	
	Currently, the GDF-Header is defined. 
   
*/


enum FileFormat {unknown, GDF, EDF, BDF, CNT, BKR}; // list file formats 

typedef struct {
	char* Label;
	char* Transducer;
	char* PhysDim;

	float LowPass;
	float HighPass;
	float Notch;
	float XYZ[3];
	
	float PhysMin;
	float PhysMax;
	long DigMin;
	long DigMax;

	//char PreFilt[81];
	unsigned long GDFTYP;
	
	unsigned long SPR;
	float Impedance;
} CHANNEL_TYPE ;

typedef struct {
	FILE * fid; 
	unsigned char Header1[256];
	unsigned char *Header2;
	//char  TYPE[4];
	enum FileFormat TYPE; 
	float VERSION;
	char* FileName;
	char* PID;
	char Smoking;
	char AlcoholAbuse;
	char DrugAbuse;
	char Medication;
	unsigned int Height;
	unsigned int Weight;
	char Gender; 
	char Handedness;
	char VisualImpairment; 
	char* RID;
//	uint32 Location[3];

	unsigned long Birthday[2]; 	
	unsigned long T0[2]; 	
		// T0[0]*2^32/(24*3600) are the seconds of the day since midnight
		// T0[1] contains the number of days since 01-Jan-0000; 01-Jan-1970 is day 719529
		// Birthday uses the same format as T0.
	
	time_t Date;
	char *Time;
	unsigned long int HeadLen;
	long int NRec;
	double Dur;
	// unsigned long int Dur[2];
	unsigned long int NS;
	unsigned long int SPR;
	CHANNEL_TYPE *CHANNEL;
    } HDRTYPE;



HDRTYPE sopen(const char* FileName, HDRTYPE HDR, const char* MODE);
HDRTYPE sclose(HDRTYPE HDR);


