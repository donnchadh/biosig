/*
%
% $Id: swig.i,v 1.25 2009-01-19 15:36:14 schloegl Exp $
% Copyright (C) 2008,2009 Alois Schloegl <a.schloegl@ieee.org>
% This file is part of the "BioSig for C/C++" repository 
% (biosig4c++) at http://biosig.sf.net/ 


    This program is free software; you can redistribute it and/or
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


// swig.i

%module biosig
%{
#define SWIG_FILE_WITH_INIT
#include "../biosig.h"
#include <arrayobject.h>
%}


%include <inttypes.i>

// import_array() call initialises Numpy
%init
%{
        import_array();
%}

typedef int64_t gdf_time; 	/* gdf time is represented in 64 bits */

typedef int64_t nrec_t; 	/* type for number of records */


	/* list of file formats */
enum FileFormat {
	noFile, unknown, 
	ABF, ACQ, ACR_NEMA, AIFC, AIFF, AINF, alpha, AU, ASF, ATES, ATF, AVI,
	BCI2000, BDF, BIN, BKR, BLSC, BMP, BNI, BSCS, 
	BrainVision, BrainVisionVAmp, BrainVisionMarker, BZ2, 
	CDF, CFWB, CNT, CTF, DICOM, DEMG, 
	EBS, EDF, EEG1100, EEProbe, EEProbe2, EEProbeAvr, EGI, EGIS, ELF, EMBLA, ET_MEG, ETG4000, EVENT, EXIF, 
	FAMOS, FEF, FITS, FLAC, GDF, GDF1,
	GIF, GTF, GZIP, HDF, HL7aECG, JPEG, Lexicor, 
	Matlab, MFER, MIDI, MIT, MM, MSI, 
	native, NetCDF, NEX1, NIFTI, OGG, OpenXDF,
	PBMA, PBMN, PDF, PDP, Persyst, PGMA, PGMB, PLEXON, PNG, PNM, POLY5, PPMA, PPMB, PS, 
	RIFF, SCP_ECG, SIGIF, Sigma, SMA, SND, SVG, SXI,    
	TIFF, TMS32, TMSiLOG, TRC, UNIPRO, VRML, VTK, 
	WAV, WinEEG, WMF, XML, XPM,
	Z, ZIP, ZIP2,
	ASCII_IBI, ASCII, 
};

typedef struct {
	double 		PhysMin;	/* physical minimum */
	double 		PhysMax;	/* physical maximum */
	double 		DigMin;		/* digital minimum */
	double	 	DigMax;		/* digital maximum */
	double		Cal;		/* gain factor */ 
	double		Off;		/* bias */ 

	uint8_t		OnOff; 		
	char		Label[MAX_LENGTH_LABEL+1]; 	/* Label of channel */
	uint16_t	LeadIdCode;	/* Lead identification code */ 
	char 		Transducer[MAX_LENGTH_TRANSDUCER+1];	/* transducer e.g. EEG: Ag-AgCl electrodes */
#	char 		PhysDim[MAX_LENGTH_PHYSDIM+1];	/* physical dimension */
	uint16_t	PhysDimCode;	/* code for physical dimension */
	/* char* 	PreFilt;	// pre-filtering */

	float 		LowPass;	/* lowpass filter */
	float 		HighPass;	/* high pass */
	float 		Notch;		/* notch filter */
	float 		XYZ[3];		/* sensor position */
//	float 		Orientation[3];	/* sensor direction */
//	float 		Area;		/* area of sensor (in m^2 e.g. for MEG) */
	float 		Impedance;   	/* in Ohm */
	float 		fZ;	   	/* probe freqency in Hertz */
	
	uint16_t 	GDFTYP;		/* data type */
	uint32_t 	SPR;		/* samples per record (block) */
	
} CHANNEL_TYPE;


%extend CHANNEL_TYPE {
   CHANNEL_TYPE *__getitem__(int index) {
        return self+index;
   }
};

 

/*
	This structure defines the general (fixed) header  
*/
typedef struct {
	enum FileFormat TYPE; 		/* type of file format */
	float 		VERSION;	/* GDF version number */ 
	const char* 	FileName;
	
	struct {
		size_t 			size[2]; /* size {rows, columns} of data block	 */
		biosig_data_type* 	block; 	 /* data block */
	} data;

	uint32_t 	HeadLen;	/* length of header in bytes */
	uint16_t 	NS;		/* number of channels */
	uint32_t 	SPR;		/* samples per block (when different sampling rates are used, this is the LCM(CHANNEL[..].SPR) */
	int64_t  	NRec;		/* number of records/blocks -1 indicates length is unknown. */	
#	uint32_t 	Dur[2];		/* Duration of each block in seconds expressed in the fraction Dur[0]/Dur[1]  */
	double 		SampleRate;	/* Sampling rate */
	uint8_t 	IPaddr[6]; 	/* IP address of recording device (if applicable) */
	uint32_t  	LOC[4];		/* location of recording according to RFC1876 */
	gdf_time 	T0; 		/* starttime of recording */
	int16_t 	tzmin; 		/* time zone (minutes of difference to UTC */

#ifdef CHOLMOD_H
	cholmod_sparse  *Calib;                  /* re-referencing matrix */
	CHANNEL_TYPE 	*rerefCHANNEL;  
#endif 	
	/* Patient specific information */
	struct {
		char		Name[MAX_LENGTH_NAME+1]; /* because for privacy protection it is by default not supported, support is turned on with FLAG.ANONYMOUS */
//		char*		Name;	/* because for privacy protection it is by default not supported, support is turned on with FLAG.ANONYMOUS */
		char		Id[MAX_LENGTH_PID+1];	/* patient identification, identification code as used in hospital  */
		uint8_t		Weight;		/* weight in kilograms [kg] 0:unkown, 255: overflow  */
		uint8_t		Height;		/* height in centimeter [cm] 0:unkown, 255: overflow  */
		//		BMI;		/* the body-mass index = weight[kg]/height[m]^2 */
		gdf_time 	Birthday; 	/* Birthday of Patient */
		// 		Age;		/* the age is HDR.T0 - HDR.Patient.Birthday, even if T0 and Birthday are not known */ 		
		uint16_t	Headsize[3]; 	/* circumference, nasion-inion, left-right mastoid in millimeter;  */
		/* Patient classification */
		int	 	Sex;		/* 0:Unknown, 1: Male, 2: Female  */
		int		Handedness;	/* 0:Unknown, 1: Right, 2: Left, 3: Equal */
		int		Smoking;	/* 0:Unknown, 1: NO, 2: YES */
		int		AlcoholAbuse;	/* 0:Unknown, 1: NO, 2: YES */
		int		DrugAbuse;	/* 0:Unknown, 1: NO, 2: YES */
		int		Medication;	/* 0:Unknown, 1: NO, 2: YES */
		struct {
			int 	Visual;		/* 0:Unknown, 1: NO, 2: YES, 3: Corrected */
			int 	Heart;		/* 0:Unknown, 1: NO, 2: YES, 3: Pacemaker */
		} Impairment;
		
	} Patient; 
	
	struct {
		char		Recording[MAX_LENGTH_RID+1]; 	/* HL7, EDF, GDF, BDF replaces HDR.AS.RID */
		char 		Technician[MAX_LENGTH_TECHNICIAN+1];
		char* 		Hospital; 	
		uint64_t 	Equipment; 	/* identifies this software */
		struct {
			/* see 
				SCP: section1, tag14, 
				MFER: tag23:  "Manufacturer^model^version number^serial number"
				GDF: tag3:  "Manufacturer\0model\0version\0number\0serial number\0"
			*/	
//			char	_field[MAX_LENGTH_MANUF+1];	/* buffer */
			char*	Name;  
			char*	Model;
			char*	Version;
			char*	SerialNumber;
		} Manufacturer;  
	} ID;

	/* position of electrodes; see also HDR.CHANNEL[k].XYZ */
	struct {
		float		REF[3];	/* XYZ position of reference electrode */
		float		GND[3];	/* XYZ position of ground electrode */
	} ELEC;

	/*	EVENTTABLE */
	struct {
		double  	SampleRate;	/* for converting POS and DUR into seconds  */
		uint32_t  	N;	/* number of events */
		uint16_t 	*TYP;	/* defined at http://cvs.sourceforge.net/viewcvs.py/biosig/biosig/t200/eventcodes.txt?view=markup */
		uint32_t 	*POS;	/* starting position [in samples] */
		uint32_t 	*DUR;	/* duration [in samples] */
		uint16_t 	*CHN;	/* channel number; 0: all channels  */
		char		**CodeDesc;	/* describtion of "free text"/"user specific" events (encoded with TYP=0..255 */
		uint16_t	LenCodeDesc;	/* length of CodeDesc Table */
	} EVENT; 

	struct {	/* flags */
		char		OVERFLOWDETECTION; 	/* overflow & saturation detection 0: OFF, !=0 ON */
		char		UCAL; 		/* UnCalibration  0: scaling  !=0: NO scaling - raw data return  */
		char		ANONYMOUS; 	/* 1: anonymous mode, no personal names are processed */ 
		char		ROW_BASED_CHANNELS;	/* 0: column-based data [default]; 1: row-based data */ 
		char		TARGETSEGMENT; /* in multi-segment files (like Nihon-Khoden, EEG1100), it is used to select a segment */ 
	} FLAG; 

	CHANNEL_TYPE *CHANNEL;
	  
	struct {	/* File specific data  */
#ifdef ZLIB_H
		gzFile		gzFID;
#endif
#ifdef _BZLIB_H
//		BZFILE*		bzFID;
#endif
		FILE* 		FID;		/* file handle  */
		size_t 		POS;		/* current reading/writing position [in blocks] */
//		int		Des;		/* file descriptor */
		uint8_t		OPEN; 		/* 0: closed, 1:read, 2: write */
		uint8_t		LittleEndian;
		uint8_t		COMPRESSION;   /* 0: no compression 9: best compression */
//		int		DES;		/* descriptor for streams */
	} FILE; 

	/*	internal variables (not public)  */
	struct {
//		char 		PID[MAX_LENGTH_PID+1];	/* use HDR.Patient.Id instead */
//		char* 		RID;		/* recording identification */ 
//		uint32_t 	spb;		/* total samples per block */
//		uint32_t 	bpb;  		/* total bytes per block */
//		uint32_t 	bpb8;  		/* total bits per block */
//		uint32_t 	*bi __attribute__ ((deprecated)); /* this information redundant with HDR.CHANNEL[k].bi[0] - and might become obsolete */
		uint8_t*	Header; 
//		uint8_t*	rawEventData;
//		uint8_t*	rawdata; 	/* raw data block */
//		nrec_t		first;		/* first block loaded in buffer - this is equivalent to hdr->FILE.POS */
//		nrec_t		length;		/* number of block(s) loaded in buffer */
//		uint8_t*	auxBUF;		/* auxillary buffer - used for storing EVENT.CodeDesc, MIT FMT infor */
		char*		bci2000;
	} AS;
	
	void *aECG;
	
} HDRTYPE;

HDRTYPE* constructHDR(const unsigned NS, const unsigned N_EVENT);
void 	 destructHDR(HDRTYPE* hdr);
HDRTYPE* sopen(const char* FileName, const char* MODE, HDRTYPE* hdr);
int 	sclose(HDRTYPE* hdr);
size_t	sread(biosig_data_type* data, size_t start, size_t length, HDRTYPE* hdr);
size_t  swrite(const biosig_data_type *data, size_t nelem, HDRTYPE* hdr);
int	seof(HDRTYPE* hdr);
void	srewind(HDRTYPE* hdr);
int 	sseek(HDRTYPE* hdr, long int offset, int whence);
long int stell(HDRTYPE* hdr);
int	hdr2ascii(HDRTYPE* hdr, FILE *fid, int verbosity);

int RerefCHANNEL(HDRTYPE *hdr, void *ReRef, char rrtype);
const char* GetFileTypeString(enum FileFormat FMT);
HDRTYPE* sload(const char* FileName, size_t CHANLIST[], biosig_data_type** DATA);

uint16_t PhysDimCode(char* PhysDim0);
char* 	PhysDim(uint16_t PhysDimCode, char *PhysDimText);

void 	sort_eventtable(HDRTYPE *hdr);
void 	convert2to4_eventtable(HDRTYPE *hdr);
void 	convert4to2_eventtable(HDRTYPE *hdr);


/*
HDRTYPE* sopen(char *filename);
%{
	HDRTYPE* sopen(char *filename)
	{
		HDRTYPE *hdr = constructHDR(0,0);
		hdr = sopen(filename, "r", hdr);
		return hdr;
        }
%}




int sclose(HDRTYPE *hdr);
%{
	int sclose(HDRTYPE *hdr)
	{
		sclose(hdr);
		destructHDR(hdr);
		return 0;
        }
%}
*/

PyObject* sread(size_t start, size_t length, HDRTYPE* hdr);
%{
	PyObject* sread(size_t start, size_t length, HDRTYPE* hdr)
	{
		int i, dims[2];
		PyArrayObject *_array;
		size_t count;

		dims[0] = 0;
		dims[1] = length * hdr->SPR;
		for(i = 0; i < hdr->NS; ++i)
			if(hdr->CHANNEL[i].OnOff)
				++dims[0];

		hdr->FLAG.ROW_BASED_CHANNELS = 0;

		if(sizeof(biosig_data_type) != sizeof(double))
			return NULL;

		/* create the NumPy array and copy the data into it */
		_array = (PyArrayObject *)PyArray_FromDims(2, dims, PyArray_DOUBLE);
		count = sread((double*)(_array->data), start, length, hdr);

		/* the block of data is now owned by _array, destructHDR should not destroy it */
		hdr->data.block = NULL;

		/* if(count != length) {} */
		return PyArray_Return(_array);
        }
%}


void hdr2ascii(HDRTYPE* hdr, int verbosity);
%{
	void hdr2ascii(HDRTYPE* hdr, int verbosity)
	{
		hdr2ascii(hdr, stdout, verbosity);
        }
%}
