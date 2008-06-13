/*
% $Id: biosig.h,v 1.107 2008-06-13 15:05:23 schloegl Exp $
% Copyright (C) 2005,2006,2007,2008 Alois Schloegl <a.schloegl@ieee.org>
% This file is part of the "BioSig for C/C++" repository 
% (biosig4c++) at http://biosig.sf.net/ 


    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.

 */

/* External API definitions */

/****************************************************************************/
/**                                                                        **/
/**                 DEFINITIONS, TYPEDEFS AND MACROS                       **/
/**                                                                        **/
/****************************************************************************/

#ifndef __BIOSIG_EXT_H__
#define __BIOSIG_EXT_H__



#ifdef _VCPP_DEF
#define __BYTE_ORDER  __LITTLE_ENDIAN
typedef unsigned __int64	uint64_t;
typedef __int64			int64_t;
typedef unsigned long		uint32_t;
typedef long			int32_t;
typedef unsigned short		uint16_t;
typedef short			int16_t;
typedef unsigned char		uint8_t;
typedef char			int8_t;

#else
#include <inttypes.h>

#endif

#ifdef WITH_HDF5
#include <hdf5.h>
#endif 
#ifdef WITH_NIFTI
#include <nifti1.h>
#endif 
/* 
	Including ZLIB enables reading gzipped files (they are decompressed on-the-fly)  
	The output files can be zipped, too. 
 */

#ifdef WITH_ZLIB
#include <zlib.h>
#endif 
//#include <bz2lib.h>

#include <stdio.h>


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	biosig_data_type    data type of  internal data format 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef double biosig_data_type;


/****************************************************************************/
/**                                                                        **/
/**                CONSTANTS and Global variables                          **/
/**                                                                        **/
/****************************************************************************/


/* for error handling */ 
enum B4C_ERROR {
	B4C_NO_ERROR,
	B4C_FORMAT_UNKNOWN,
	B4C_FORMAT_UNSUPPORTED,
	B4C_CANNOT_OPEN_FILE,
	B4C_CANNOT_WRITE_FILE,
	B4C_INSUFFICIENT_MEMORY,
	B4C_ENDIAN_PROBLEM,
	B4C_CRC_ERROR,
	B4C_DATATYPE_UNSUPPORTED,
	B4C_SCLOSE_FAILED,
	B4C_DECOMPRESSION_FAILED,
	B4C_UNSPECIFIC_ERROR,
};


	/* list of file formats */
enum FileFormat {
	noFile, unknown, 
	ABF, ACQ, ACR_NEMA, AIFC, AIFF, AINF, alpha, AU, ASF, ATES, ATF, AVI,
	BCI2000, BDF, BKR, BLSC, BMP, BrainVision, BZ2, 
	CDF, CFWB, CNT, CTF, DICOM, DEMG, 
	EDF, EEG1100, EEProbe, EGI, ELF, ETG4000, EVENT, EXIF, 
	FAMOS, FEF, FITS, FLAC, GDF, GDF1,
	GIF, GTF, GZIP, HDF, HL7aECG, JPEG, 
	Matlab, MFER, MIDI, MIT, NetCDF, NEX1, NIFTI, OGG, 
	PBMA, PBMN, PDF, PGMA, PGMB, PLEXON, PNG, PNM, POLY5, PPMA, PPMB, PS, 
	RIFF, SCP_ECG, SIGIF, SMA, SND, SVG, SXI,    
	TIFF, TMS32, VRML, VTK, WAV, WMF, XML, XPM,
	Z, ZIP, ZIP2,
	ASCII_IBI
};


extern int   B4C_ERRNUM;
extern const char *B4C_ERRMSG;
extern int   VERBOSE_LEVEL; 



/****************************************************************************/
/**                                                                        **/
/**                 DEFINITIONS, TYPEDEFS AND MACROS                       **/
/**                                                                        **/
/****************************************************************************/



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	Conversion of time formats between Unix and GDF format.

	The default time format in BIOSIG uses a 64-bit fixed point format with 
	reference date 01-Jan-0000 00h00m00s (value=0). 
	One unit indicates the 2^(-32) part of 1 day (ca 20 us). Accordingly, 
	the higher 32 bits count the number of days, the lower 32 bits describe 
	the fraction of a day.  01-Jan-1970 is the day 719529. 

      	time_t t0; 
      	t0 = time(NULL); 
      	T0 = (double)t0/86400.0;	// convert seconds in days since 1970-Jan-01
      	floor(T0) + 719529;		// number of days since 01-Jan-0000
      	floor(ldexp(T0-floor(T0),32));  // fraction x/2^32; one day is 2^32 
      	
      	The following macros define the conversions between the unix time and the 
      	GDF format. 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef int64_t 		gdf_time; /* gdf time is represented in 64 bits */
#define t_time2gdf_time(t)	((gdf_time)round(ldexp(((double)(t))/86400.0 + 719529.0, 32)))
/* #define t_time2gdf_time(t)	((gdf_time)floor(ldexp(difftime(t,0)/86400.0 + 719529.0, 32)))  */
#define gdf_time2t_time(t)	((time_t)((ldexp((double)(t),-32) - 719529) * 86400))
#define tm_time2gdf_time(t) 	t_time2gdf_time(mktime(t))
/* #define gdf_time2tm_time(t)        gmtime(gdf_time2t_time(t)) */
#define	ntp_time2gdf_time(t)	((gdf_time)ldexp(ldexp(((double)(t)),-32)/86400 + 719529.0 - 70,32))
#define	gdf_time2ntp_time(t)	((int64_t)ldexp((ldexp(((double)(t)),-32) - 719529.0 + 70) * 86400,32))


/****************************************************************************/
/**                                                                        **/
/**                     TYPEDEFS AND STRUCTURES                            **/
/**                                                                        **/
/****************************************************************************/

/*
	This structure defines the header for each channel (variable header) 
 */
#define MAX_LENGTH_LABEL 	40 
#define MAX_LENGTH_TRANSDUCER 	80
#define MAX_LENGTH_PHYSDIM 	20
#define MAX_LENGTH_PID	 	80      // length of Patient ID: MFER<65, GDF<67, EDF/BDF<81, etc. 
#define MAX_LENGTH_RID		80	// length of Recording ID: EDF,GDF,BDF<80, HL7 ?  	
#define MAX_LENGTH_NAME 	128	// max length of personal name: MFER<=128
#define MAX_LENGTH_MANUF 	128	// max length of manufacturer field: MFER<128

#define ATT_ALI __attribute__ ((aligned (8)))	/* Matlab v7.3+ requires 8 byte alignment*/

typedef struct {
	double 		PhysMin ATT_ALI;	/* physical minimum */
	double 		PhysMax ATT_ALI;	/* physical maximum */
	double 		DigMin 	ATT_ALI;	/* digital minimum */
	double	 	DigMax 	ATT_ALI;	/* digital maximum */
	double		Cal 	ATT_ALI;	/* gain factor */ 
	double		Off 	ATT_ALI;	/* bias */ 

	char		OnOff	ATT_ALI; 		
	char		Label[MAX_LENGTH_LABEL+1] ATT_ALI; 	/* Label of channel */
	uint16_t	LeadIdCode ATT_ALI;	/* Lead identification code */ 
	char 		Transducer[MAX_LENGTH_TRANSDUCER+1] ATT_ALI;	/* transducer e.g. EEG: Ag-AgCl electrodes */
	char 		PhysDim[MAX_LENGTH_PHYSDIM+1] ATT_ALI  	__attribute__ ((deprecated));	/* physical dimension */
			/*PhysDim is now depreciated - use function PhysDim(PhysDimCode,PhysDimText) instead */
	uint16_t	PhysDimCode ATT_ALI;	/* code for physical dimension */
	/* char* 	PreFilt;	// pre-filtering */

	float 		LowPass	ATT_ALI;	/* lowpass filter */
	float 		HighPass	ATT_ALI;	/* high pass */
	float 		Notch	ATT_ALI;		/* notch filter */
	float 		XYZ[3]	ATT_ALI;		/* electrode position */
	float 		Impedance	ATT_ALI;   	/* in Ohm */
	
	uint16_t 	GDFTYP 	ATT_ALI;	/* data type */
	uint32_t 	SPR 	ATT_ALI;	/* samples per record (block) */
	
} CHANNEL_TYPE	ATT_ALI;


/*
	This structure defines the general (fixed) header  
*/
typedef struct {
	
	enum FileFormat TYPE 	ATT_ALI; 	/* type of file format */
	float 		VERSION ATT_ALI;	/* GDF version number */ 
	const char* 	FileName ATT_ALI;
	
	struct {
		size_t 			size[2] ATT_ALI; /* size {rows, columns} of data block	 */
		biosig_data_type* 	block ATT_ALI; 	 /* data block */
	} data ATT_ALI;

	uint32_t 	HeadLen ATT_ALI;	/* length of header in bytes */
	uint16_t 	NS 	ATT_ALI;	/* number of channels */
	uint32_t 	SPR 	ATT_ALI;	/* samples per block (when different sampling rates are used, this is the LCM(CHANNEL[..].SPR) */
	int64_t  	NRec 	ATT_ALI;	/* number of records/blocks -1 indicates length is unknown. */	
	uint32_t 	Dur[2] 	__attribute__ ((deprecated));	/* Duration of each block in seconds expressed in the fraction Dur[0]/Dur[1]  */
	double 		SampleRate ATT_ALI;	/* Sampling rate */
	uint8_t 	IPaddr[6] ATT_ALI; 	/* IP address of recording device (if applicable) */
	uint32_t  	LOC[4] 	ATT_ALI;	/* location of recording according to RFC1876 */
	gdf_time 	T0 	ATT_ALI; 	/* starttime of recording */
	int16_t 	tzmin 	ATT_ALI;	 		/* time zone (minutes of difference to UTC */

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
		} Impairment;
		
	} Patient ATT_ALI; 
	
	struct {
		char		Recording[MAX_LENGTH_RID+1]; 	/* HL7, EDF, GDF, BDF replaces HDR.AS.RID */
		char* 		Technician; 	
		char* 		Hospital; 	
		uint64_t 	Equipment; 	/* identifies this software */
		struct {
			/* see 
				SCP: section1, tag14, 
				MFER: tag23:  "Manufacturer^model^version number^serial number"
			*/	
			char	_field[MAX_LENGTH_MANUF+1];	/* buffer */
			char*	Name;  
			char*	Model;
			char*	Version;
			char*	SerialNumber;
		} Manufacturer;  
		char 		EquipmentManufacturer[20];
		char		EquipmentModel[20]; 
		char		EquipmentSerialNumber[20];
	} ID ATT_ALI;

	/* position of electrodes; see also HDR.CHANNEL[k].XYZ */
	struct {
		float		REF[3];	/* XYZ position of reference electrode */
		float		GND[3];	/* XYZ position of ground electrode */
	} ELEC ATT_ALI;

	/* EVENTTABLE */
	struct {
		double  	SampleRate ATT_ALI;	/* for converting POS and DUR into seconds  */
		uint32_t  	N ATT_ALI;	/* number of events */
		uint16_t 	*TYP ATT_ALI;	/* defined at http://cvs.sourceforge.net/viewcvs.py/biosig/biosig/t200/eventcodes.txt?view=markup */
		uint32_t 	*POS ATT_ALI;	/* starting position [in samples] */
		uint32_t 	*DUR ATT_ALI;	/* duration [in samples] */
		uint16_t 	*CHN ATT_ALI;	/* channel number; 0: all channels  */
		char		**CodeDesc ATT_ALI;	/* describtion of "free text"/"user specific" events (encoded with TYP=0..255 */
		uint16_t	LenCodeDesc ATT_ALI;	/* length of CodeDesc Table */
	} EVENT ATT_ALI; 

	struct {	/* flags */
		char		OVERFLOWDETECTION; 	/* overflow & saturation detection 0: OFF, !=0 ON */
		char		UCAL; 		/* UnCalibration  0: scaling  !=0: NO scaling - raw data return  */
		char		ANONYMOUS; 	/* 1: anonymous mode, no personal names are processed */ 
		char		ROW_BASED_CHANNELS; 	        /* 0: column-based data [default]; 1: row-based data */ 
	} FLAG ATT_ALI; 

	struct {	/* File specific data  */
#ifdef ZLIB_H
		gzFile		gzFID;
#endif
#ifdef _BZLIB_H
//		BZFILE*		bzFID;
#endif
		FILE* 		FID;		/* file handle  */
		size_t 		POS;		/* current reading/writing position [in blocks] */
		//size_t 		POS2;		/* current reading/writing position [in samples] */
		uint8_t		OPEN; 		/* 0: closed, 1:read, 2: write */
		uint8_t		LittleEndian;   /* 1 if file is LittleEndian data format and 0 for big endian data format*/  
		uint8_t		COMPRESSION;    /* 0: no compression 9: best compression */
	} FILE ATT_ALI; 

	/*	internal variables (not public)  */
	struct {
//		char 		PID[MAX_LENGTH_PID+1];	/* use HDR.Patient.Id instead */
//		char* 		RID;		/* recording identification */ 
		uint32_t 	spb;		/* total samples per block */
		uint32_t 	bpb;  		/* total bytes per block */
		uint32_t 	*bi;
		uint8_t*	Header; 
		uint8_t*	rawdata; 	/* raw data block */
		size_t		rawdata_curblock;
		size_t		rawdata_nextblock;
		uint8_t*	auxBUF 	__attribute__ ((deprecated));		/* auxillary buffer - used for storing EVENT.CodeDesc, MIT FMT infor */
		char*		bci2000;
	} AS ATT_ALI;
	
	CHANNEL_TYPE 	*CHANNEL ATT_ALI;  

	void *aECG;
	
} HDRTYPE;


/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/


HDRTYPE* constructHDR(const unsigned NS, const unsigned N_EVENT);
/* 	allocates memory initializes header HDR of type HDRTYPE 
	with NS channels an N_EVENT event elements
 ---------------------------------------------------------------*/


void 	 destructHDR(HDRTYPE* hdr);
/* 	destroys the header *hdr and frees allocated memory
 ---------------------------------------------------------------*/


HDRTYPE* sopen(const char* FileName, const char* MODE, HDRTYPE* hdr);
/*	FileName: name of file 
	Mode: "r" is reading mode, requires FileName and hdr=NULL
	Mode: "w" is writing mode, hdr contains the header information  
	After calling sopen, the file header is read or written, and 
	the position pointer points to the beginning of the data section
 ---------------------------------------------------------------*/


int 	sclose(HDRTYPE* hdr);
/* 	closes the file corresponding to hdr
	file handles are closed, the position pointer becomes meaningless
	Note: hdr is not destroyed; use destructHDR() to free the memory of hdr 		
 ---------------------------------------------------------------*/


size_t	sread(biosig_data_type* DATA, size_t START, size_t LEN, HDRTYPE* hdr);
size_t	sread1(biosig_data_type* DATA, size_t START, size_t LEN, HDRTYPE* hdr);
/*	LEN data segments are read from file associated with hdr, starting from 
	segment START. The data is copied into DATA; if DATA == NULL, a 
	sufficient amount of memory is allocated; otherwise, it's assumed 
	that sufficient memory is already allocated. 
	Only those channels with (hdr->CHANNEL[].OnOff != 0) are read. 
	In total, LEN*hdr->SPR*hdr->NS samples are read and stored in 
	data type of biosig_data_type (currently double). 
	The number of successfully read data blocks is returned.

 	A pointer to the data block is also available from hdr->data.block, 
	the number of columns and rows is available from 
	hdr->data.size[0] and hdr->data.size[1] respectively. 
 
	The following flags will influence the result.
 	hdr->FLAG.UCAL = 0 	scales the data to its physical values
 	hdr->FLAG.UCAL = 1 	does not apply the scaling factors

 	hdr->FLAG.OVERFLOWDETECTION = 0 does not apply overflow detection 
 	hdr->FLAG.OVERFLOWDETECTION = 1: replaces all values that exceed 
		the dynamic range (defined by Phys/Dig/Min/Max)

	hdr->FLAG.ROW_BASED_CHANNELS = 0 each channel is in one column 	
	hdr->FLAG.ROW_BASED_CHANNELS = 1 each channel is in one row 	
 ---------------------------------------------------------------*/

size_t	sread2(biosig_data_type* DATA, size_t START, size_t LEN, HDRTYPE* hdr);
size_t	sread3(biosig_data_type* DATA, size_t START, size_t LEN, HDRTYPE* hdr);
/*	SREAD2 and SREAD3 are experimental 
	it is similar to SREAD but START and LENGTH are in samples.  
 ---------------------------------------------------------------*/


size_t  swrite(const biosig_data_type *DATA, size_t NELEM, HDRTYPE* hdr);
/*	DATA contains the next NELEM data segment(s) for writing. 
 *	hdr contains the definition of the header information and was generated by sopen 
 *	the number of successfully written segments is returned;	
 ---------------------------------------------------------------*/


int	seof(HDRTYPE* hdr);
/*	returns 1 if end of file is reached. 
 ---------------------------------------------------------------*/


void	srewind(HDRTYPE* hdr);
/*	postions file pointer to the beginning
 *
 *	Currently, this function is meaning less because sread requires always the start value
 ---------------------------------------------------------------*/


int 	sseek(HDRTYPE* hdr, long int offset, int whence);
/*	positions file pointer 
 *
 *	Currently, this function is meaning less because sread requires always the start value
 ---------------------------------------------------------------*/


long int stell(HDRTYPE* hdr);
/*	returns position of file point in segments
 ---------------------------------------------------------------*/


int 	serror();
/*	handles errors; it reports whether an error has occured. 
 *	if yes, an error message is displayed, and the error status is reset. 
 * 	the return value is 0 if no error has occured, otherwise the error code 
 *	is returned.
 ---------------------------------------------------------------*/


int 	sflush_gdf_event_table(HDRTYPE* hdr);
/*	writes the event table of file hdr. hdr must define a file in GDF format  
 *  	and can be opened as read or write. 
 *	In case of success, the return value is 0. 
 ---------------------------------------------------------------*/


int	hdr2ascii(HDRTYPE* hdr,FILE *fid, int VERBOSITY);
/*	writes the header information is ascii format the stream defined by fid 
 *	Typically fid is stdout. VERBOSITY defines how detailed the information is. 
 *	VERBOSITY=0 or 1 report just some basic information,
 *	VERBOSITY=2 reports als the channel information 
 *	VERBOSITY=3 provides in addition the event table. 
 ---------------------------------------------------------------*/


const char* GetFileTypeString(enum FileFormat FMT);
/*	returns a string with file format
 ---------------------------------------------------------------*/


HDRTYPE* sload(const char* FileName, size_t CHANLIST[], biosig_data_type** DATA);
/*	FileName: name of file 
	returns a HDR struct containing the header information 
	CHANLIST[0] contains the length of the channel list 
	CHANLIST[1..CHANLIST[0]] contains the selected channels 
	and DATA points to a matrix with HDR.SPR*HDR.NRec samples and HDR.NS channels
 ---------------------------------------------------------------*/


uint16_t PhysDimCode(char* PhysDim0);
/* Encodes  Physical Dimension as 16bit integer according to 
   ISO/IEEE 11073-10101:2004 Vital Signs Units of Measurement
 ---------------------------------------------------------------*/

char* PhysDim(uint16_t PhysDimCode, char *PhysDimText);
/* converts HDR.CHANNEL[k].PhysDimCode into a readable Physical Dimension
   the memory for PhysDim must be preallocated, its maximum length is 
   defined by (MAX_LENGTH_PHYSDIM+1)  
 ---------------------------------------------------------------*/

void sort_eventtable(HDRTYPE *hdr);
/* sort event table with respect to hdr->EVENT.POS    
  --------------------------------------------------------------*/

void convert2to4_eventtable(HDRTYPE *hdr);
/* converts event table from {TYP,POS} to [TYP,POS,CHN,DUR} format   
  --------------------------------------------------------------*/

void convert4to2_eventtable(HDRTYPE *hdr);
/* converts event table from [TYP,POS,CHN,DUR} to {TYP,POS} format 
	all CHN[k] must be 0
  --------------------------------------------------------------*/



/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/

#endif	/* __BIOSIG_EXT_H__ */
