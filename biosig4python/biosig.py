"""BIOSIG Toolbox for Python
    Copyright (C) 2005-2006 by Martin Hieden <martin.hieden@gmx.at> 
	             and Alois Schloegl <a.schloegl@ieee.org>

    $Id: biosig.py,v 1.2 2008-05-13 11:21:25 schloegl Exp $
    This function is part of the "BioSig for Python" repository 
    (biosig4python) at http://biosig.sf.net/ 


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


FUNCTIONS
===================
	sopen
	sclose
	sread
	swrite
	seof
	srewind
	sseek
	stell


HEADER
===================
	TYPE:		type of file format
	VERSION:	GDF version number 
	FileName:	name of opened file
	HeadLen:	length of header in bytes
	NS:		number of channels
	SPR:		samples per block (when different sampling rates are used, this is the LCM(CHANNEL[..].SPR)
	NRec:		number of records/blocks -1 indicates length is unknown.
	Dur:		Duration of each block in seconds expressed in the fraction Dur[0]/Dur[1]
	SampleRate:	Sampling rate
	IPaddr:		IP address of recording device (if applicable)
	T0:		starttime of recording


	data		last data read
	------------------------------
		block:		data block  
		size:		size {rows, columns} of data block	


	Patient:	Patient information
	-----------------------------------
		Name:		not recommended because of privacy protection 
		Id:		identification code as used in hospital 
		Weight:		weight in kilograms [kg] 0:unkown, 255: overflow 
		Height:		height in centimeter [cm] 0:unkown, 255: overflow 
		Birthday:	Birthday of Patient
		Age:		Age of Patient
		Headsize:	circumference, nasion-inion, left-right mastoid in millimeter
		Sex
		Handedness
		Smoking
		AlcoholAbuse
		DrugAbuse
		Medication
		Impairment
			Visual


	ID:		recording identification
	----------------------------------------
		Technician
		Hospital
		Equipment:	identfies this software


	LOC:		location of recording according to RFC1876
	----------------------------------------------------------
		VertPre
		HorizPre
		Size
		Version
		Latitude:	in degrees
		Longitude:	in degrees
		Altitude:	in metres


	ELEC:		position of electrodes; see also HDR.CHANNEL[k].XYZ
	-------------------------------------------------------------------
		REF:		XYZ position of reference electrode
		GND:		XYZ position of ground electrode


	EVENT:		EVENTTABLE
	--------------------------
		SampleRate:	for converting POS and DUR into seconds 
		N:		number of events
		TYP:		defined at http://cvs.sourceforge.net/viewcvs.py/biosig/biosig/t200/eventcodes.txt?view=markup
		POS:		starting position [in samples]
		DUR:		duration [in samples]
		CHN:		channel number; 0: all channels 


	FLAG:		flags
	---------------------
		OVERFLOWDETECTION:	overflow & saturation detection 0: OFF, !=0 ON
		UCAL:			UnCalibration  0: scaling  !=0: NO scaling - raw data return 


	FILE:		File specific data
	----------------------------------
		FID:		file handle 
		POS:		current reading/writing position in samples 
		OPEN:		0: closed, 1:read, 2: write
		LittleEndian:	not in use


	AS:		internal variables
	----------------------------------
		PID:		patient identification
		RID:		recording identification 
		spb:		total samples per block
		bpb:		total bytes per block
		bi:		not in use
		Header1:	not in use
		rawdata:	raw data block 


	CHANNEL[k]:	channel specific data
	-------------------------------------
		Label:		Label of channel 
		Transducer:	transducer e.g. EEG: Ag-AgCl electrodes
		PhysDim:	physical dimension
		PhysDimCode:	code for physical dimension
		PreFilt:	pre-filtering
	
		LowPass:	lowpass filter
		HighPass:	high pass
		Notch:		notch filter
		XYZ:		electrode position
		Impedance:	in Ohm
	
		PhysMin:	physical minimum
		PhysMax:	physical maximum
		DigMin:		digital minimum
		DigMax:		digital maximum
	
		GDFTYP:		data type
		SPR:		samples per record (block)
		bpr:		bytes per record (block)
	
		OnOff:		1: include, 0: exclude in sread
		Cal:		gain factor 
		Off:		bias"""

#########################
# FATAL ERROR EXCEPTION #
#########################

class __FATALERROR(Exception):
	def __init__(self, value):
		self.value = value
	def __str__(self):
		return repr(self.value)

try:
################
# MODUL IMPORT #
################
	
	try:
		import numpy
		from numpy import NaN
	except ImportError:
		raise __FATALERROR('NumPy not found!\nPlease visit numpy.scipy.org for more information.')
	import math
	import struct
	import datetime
	
###############
# GLOBAL DATA #
###############
	
	# not used so far:	
	#	__FileFormat = ('ACQ', 'BKR', 'BDF', 'CNT', 'DEMG', 'EDF', 'EVENT', 'FLAC', 'GDF', 'MFER', 'NEX1', 'PLEXON') 
	type_not_supported = '%s not supported by this Numpy version!\nPlease visit numpy.scipy.org for more information.'
	__HANDEDNESS = ('Unknown', 'Right', 'Left', 'Equal')
	__GENDER = ('Unknown', 'Male', 'Female')
	__SCALE = ('Unknown', 'No', 'Yes', 'Corrected')
	
	__GDFTYP_NAME = [None]
	try:
		from numpy import int8
		from numpy import uint8
		from numpy import int16
		from numpy import uint16
		from numpy import int32
		from numpy import uint32
		__GDFTYP_NAME.append(int8)
		__GDFTYP_NAME.append(uint8)
		__GDFTYP_NAME.append(int16)
		__GDFTYP_NAME.append(uint16)
		__GDFTYP_NAME.append(int32)
		__GDFTYP_NAME.append(uint32)
	except NameError:
		raise __FATALERROR(type_not_supported % 'Standard datatypes')
	try:
		from numpy import int64
		__GDFTYP_NAME.append(int64)
	except NameError:
		__GDFTYP_NAME.append(None)
		print type_not_supported % 'int64'
	try:
		from numpy import uint64
		__GDFTYP_NAME.append(uint64)
	except NameError:
		__GDFTYP_NAME.append(None)
		print type_not_supported % 'uint64'
	__GDFTYP_NAME.append(None)
	__GDFTYP_NAME.append(None)
	__GDFTYP_NAME.append(None)
	__GDFTYP_NAME.append(None)
	__GDFTYP_NAME.append(None)
	__GDFTYP_NAME.append(None)
	__GDFTYP_NAME.append(None)
	try:
		from numpy import float32
		__GDFTYP_NAME.append(float32)
	except NameError:
		raise __FATALERROR(type_not_supported % 'Standard datatypes')
	try:
		from numpy import float64 as float64
		__GDFTYP_NAME.append(float64)
	except NameError:
		__GDFTYP_NAME.append(None)
		print type_not_supported % 'float64'
	#try:
	#	from numpy import float128
	#	__GDFTYP_NAME.append(float128)
	#except NameError:
	#	__GDFTYP_NAME.append(None)
	#	print 'float128 is not supported by this SciPy version.'
	#	print 'Please visit www.numpy.org or numeric.numpy.org for more information.'
	
	__GDFTYP_BYTE = (1, 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 0, 0, 0, 0, 0, 4, 8, 16)
	
	
#########################
# FATAL ERROR EXCEPTION #
#########################

except __FATALERROR, e:
	print ''
	print 'FATAL ERROR:'
	print '============'
	print e.value
	print '============'
	print ''


################
# UNTERKLASSEN #
################

class CHANNEL_TYPE:
	OnOff = 1	#
	Label = ''	# Label of channel 
	Transducer = ''	# transducer e.g. EEG: Ag-AgCl electrodes
	PhysDim = ''	# physical dimension
	PhysDimCode = 0	# code for physical dimension
	PreFilt = ''	# pre-filtering

	LowPass = 0	# lowpass filter
	HighPass = 0	# high pass
	Notch = 0	# notch filter
	XYZ = 0		# electrode position
	Impedance = 0	# in Ohm

	PhysMin = 0	# physical minimum
	PhysMax = 0	# physical maximum
	DigMin = 0	# digital minimum
	DigMax = 0	# digital maximum

	GDFTYP = 0	# data type
	SPR = 0		# samples per record (block)
	bpr = 0		# bytes per record (block)

	Cal = 1		# gain factor 
	Off = 0		# bias 

class DATA_TYPE:
	size = numpy.array([0,0])	# size {rows, columns} of data block	
	block =  numpy.array([])	# data block  
	
class IMPAIRMENT_TYPE:
	Visual = 'Unknown'

# Patient specific information
class PATIENT_TYPE:
	Name = ''	# not recommended because of privacy protection 
	Id = ''		# identification code as used in hospital 
	Weight = 0	# weight in kilograms [kg] 0:unkown, 255: overflow 
	Height = 0	# height in centimeter [cm] 0:unkown, 255: overflow 
	Birthday = 0	# Birthday of Patient
	Age = 0
	Headsize = 0	# circumference, nasion-inion, left-right mastoid in millimeter; 
	Sex = 'Unknown' 	
	Handedness = 'Unknown'	
	# Patient classification
	Smoking = 'Unknown'
	AlcoholAbuse = 'Unknown'
	DrugAbuse = 'Unknown'
	Medication = 'Unknown'
	Impairment = IMPAIRMENT_TYPE()

class ID_TYPE:
	Technician = '' 	
	Hospital = ''
	Equipment = ''	# identfies this software

# location of recording according to RFC1876
class LOC_TYPE:
	VertPre = 0
	HorizPre = 0
	Size = 0
	Version = 0
	Latitude = 0	# in degrees
	Longitude = 0	# in degrees
	Altitude = 0	# in metres

# position of electrodes; see also HDR.CHANNEL[k].XYZ
class ELEC_TYPE:
	REF = ''	# XYZ position of reference electrode
	GND = ''	# XYZ position of ground electrode

# EVENTTABLE 
class EVENT_TYPE:
	SampleRate = 0	# for converting POS and DUR into seconds 
	N = 0		# number of events
	TYP = ''	# defined at http://cvs.sourceforge.net/viewcvs.py/biosig/biosig/t200/eventcodes.txt?view=markup
	POS = 0		# starting position [in samples]
	DUR = 0		# duration [in samples]
	CHN = 0		# channel number; 0: all channels 

# flags
class FLAG_TYPE:
	OVERFLOWDETECTION = 1	# overflow & saturation detection 0: OFF, !=0 ON
	UCAL = 0	# UnCalibration  0: scaling  !=0: NO scaling - raw data return 

# File specific data
class FILE_TYPE:
	FID = ''	# file handle 
	POS = 0		# current reading/writing position in samples 
	OPEN = 0	# 0: closed, 1:read, 2: write
	LittleEndian = ''	# 

# internal variables (not public) 
class AS_TYPE:
	PID = ''	# patient identification
	RID = ''	# recording identification 
	spb = 0		# total samples per block
	bpb = 0		# total bytes per block
	bi = 0
	Header1 = ''
	rawdata = numpy.array([])	# raw data block 

class HDR_TYPE:
	TYPE = ''	# type of file format
	VERSION = 0	# GDF version number 
	FileName = ''

	HeadLen = 0	# length of header in bytes
	NS = 0		# number of channels
	SPR = 0		# samples per block (when different sampling rates are used, this is the LCM(CHANNEL[..].SPR)
	NRec = -1	# number of records/blocks -1 indicates length is unknown.	
	Dur = ''	# Duration of each block in seconds expressed in the fraction Dur[0]/Dur[1]
	SampleRate = 0	# Sampling rate
	IPaddr = ''	# IP address of recording device (if applicable)	
	T0 = ''		# starttime of recording
	
	data = DATA_TYPE()
	Patient = PATIENT_TYPE()
	ID = ID_TYPE()
	LOC = LOC_TYPE()
	ELEC = ELEC_TYPE()
	EVENT = EVENT_TYPE()
	FLAG = FLAG_TYPE()
	FILE = FILE_TYPE()
	AS = AS_TYPE()
	CHANNEL = []

###########################
# FUNCTION IMPLEMENTATION #
###########################
	
	###########
	#  SOPEN  #
	###########
def sopen(FileName, MODE = 'r', HDR = HDR_TYPE()):
	"""input:  string FileName, [r,w] MODE, HDR_TYPE HDR
	output: HDR_TYPE
	Opens a file for reading.
	Writing is not yet implemented.
	Supported dataformats: BDF, EDF, GDF, GDF2"""
	
	try:
		
		# READ
		if MODE == 'r':
			HDR.FILE.FID = open(FileName, 'rb')
			version = HDR.FILE.FID.read(8)
			
			# BDF & EDF
			if version[1:] == 'BIOSEMI' or version[:] == '0       ':
				if version[1:] == 'BIOSEMI':
					HDR.TYPE = 'BDF'
					HDR.VERSION = -1
				else:
					HDR.TYPE = 'EDF'
					HDR.VERSION = 0
				
				#FIXED HEADER
				HDR.AS.PID = HDR.FILE.FID.read(80)
				HDR.AS.RID = HDR.FILE.FID.read(80)
				tm = HDR.FILE.FID.read(16)
				if int(tm[6:8]) < 85:
					tm = '20' + tm[6:8] + tm[3:5] + tm[0:2] + tm[8:10] + tm[11:13] + tm[14:16] + '00'
				else:
					tm = '19' + tm[6:8] + tm[3:5] + tm[0:2] + tm[8:10] + tm[11:13] + tm[14:16] + '00'
				HDR.T0 = __gdf_time2py_time(tm)
				HDR.HeadLen = int(HDR.FILE.FID.read(8))
				reserved = HDR.FILE.FID.read(44)	#44bytes reserved
				if reserved[0:4] == 'EDF+':
					pid = HDR.AS.PID.split(' ')	#PID split up in 'patient identification code', 'sex', 'birthday', 'patient name' and 'patient classification'
					if len(pid) >= 4:
						HDR.Patient.Id = pid[0]
						if pid[1][0].lower() == 'f':
							HDR.Patient.Sex = __GENDER[2]
						else:
							HDR.Patient.Sex = __GENDER[1]
						bday = pid[2].split('-')
						if len(bday) >= 3:
							if bday[1].lower() == 'jan':
								bday[1] = '01'
							elif bday[1].lower() == 'feb':
								bday[1] = '02'
							elif bday[1].lower() == 'mar':
								bday[1] = '03'
							elif bday[1].lower() == 'apr':
								bday[1] = '04'
							elif bday[1].lower() == 'may':
								bday[1] = '05'
							elif bday[1].lower() == 'jun':
								bday[1] = '06'
							elif bday[1].lower() == 'jul':
								bday[1] = '07'
							elif bday[1].lower() == 'aug':
								bday[1] = '08'
							elif bday[1].lower() == 'sep':
								bday[1] = '09'
							elif bday[1].lower() == 'oct':
								bday[1] = '10'
							elif bday[1].lower() == 'nov':
								bday[1] = '11'
							elif bday[1].lower() == 'dec':
								bday[1] = '12'
							HDR.Patient.Birthday = __gdf_time2py_time(bday[2] + bday[1] + bday[0] + '12000000')
						HDR.Patient.Name = pid[3]
				
				HDR.NRec = int(HDR.FILE.FID.read(8))
				HDR.Dur = numpy.array([float(HDR.FILE.FID.read(8)),1.])
				HDR.NS = int(HDR.FILE.FID.read(4))
				
				#VARIABLE HEADER
				HDR.SPR = 1
				vh = HDR.FILE.FID.read(HDR.HeadLen-256)
				for k in range(HDR.NS):
					HDR.CHANNEL.append(CHANNEL_TYPE())
					i = k*16
					HDR.CHANNEL[k].Label = vh[i:i+16]
					i = i + (HDR.NS-k)*16 + k*80
					HDR.CHANNEL[k].Transducer = vh[i:i+80]
					i = i + (HDR.NS-k)*80 + k*8
					HDR.CHANNEL[k].PhysDim = vh[i:i+8]
					i = i + (HDR.NS-k)*8 + k*8
					HDR.CHANNEL[k].PhysMin = float(vh[i:i+8])
					i = i + (HDR.NS-k)*8 + k*8
					HDR.CHANNEL[k].PhysMax = float(vh[i:i+8])
					i = i + (HDR.NS-k)*8 + k*8
					HDR.CHANNEL[k].DigMin = float(vh[i:i+8])
					i = i + (HDR.NS-k)*8 + k*8
					HDR.CHANNEL[k].DigMax = float(vh[i:i+8])
					i = i + (HDR.NS-k)*8 + k*80
					HDR.CHANNEL[k].PreFilt = vh[i:i+80]
					i = i + (HDR.NS-k)*80 + k*8
					HDR.CHANNEL[k].SPR = long(vh[i:i+8])
					HDR.SPR = __lcm(HDR.SPR, HDR.CHANNEL[k].SPR)	# least common SPR
					HDR.AS.spb = HDR.AS.spb + HDR.CHANNEL[k].SPR
					i = i + (HDR.NS-k)*8 + k*32
					if HDR.TYPE == 'BDF':
						HDR.CHANNEL[k].GDFTYP = 255 + 24
						HDR.CHANNEL[k].bpr = 3 * HDR.CHANNEL[k].SPR
						HDR.AS.bpb = HDR.AS.bpb + HDR.CHANNEL[k].bpr
					else:
						HDR.CHANNEL[k].GDFTYP = 3
						HDR.CHANNEL[k].bpr = __GDFTYP_BYTE[HDR.CHANNEL[k].GDFTYP] * HDR.CHANNEL[k].SPR
						HDR.AS.bpb = HDR.AS.bpb + HDR.CHANNEL[k].bpr
					
					#reserved
					i = i +(HDR.NS-k)*32
						
					HDR.CHANNEL[k].Cal = (HDR.CHANNEL[k].PhysMax - HDR.CHANNEL[k].PhysMin) / (HDR.CHANNEL[k].DigMax - HDR.CHANNEL[k].DigMin)
					HDR.CHANNEL[k].Off = HDR.CHANNEL[k].PhysMin - HDR.CHANNEL[k].Cal * HDR.CHANNEL[k].DigMin
						
					if i != len(vh):
						raise __FATALERROR('Error reading variable Header!\nSignal: ' + str(k))
				
				#Finished reading Header
				HDR.FLAG.OVERFLOWDETECTION = 0	#EDF does not support automated overflow and saturation detection
				HDR.FILE.OPEN = 1
				HDR.FILE.FID.seek(HDR.HeadLen)
				HDR.FILE.POS = 0
				HDR.SampleRate = float(HDR.SPR) * HDR.Dur[1] / HDR.Dur[0]
				print 'Finished reading header'
				return HDR
			
			# GDF
			# fromstring(...).tolist()[0] is necessary because of wrong type (..._arrtype) returned by fromstring(...)[0]
			elif version[:3] == 'GDF':
				HDR.TYPE = 'GDF'
				HDR.VERSION = float(version[4:])
				
				# GDF 1.x
				if HDR.VERSION < 1.9:
					#FIXED HEADER
					HDR.AS.PID = HDR.FILE.FID.read(80)
					pid = HDR.AS.PID.split(' ', 2)	#PID split up in 'patient identification code', 'patient name' and 'patient classification'
					if len(pid) >= 2:
						HDR.Patient.Id = pid[0]
						HDR.Patient.Name = pid[1]
					
					HDR.AS.RID = HDR.FILE.FID.read(80)
					HDR.T0 = __gdf_time2py_time(HDR.FILE.FID.read(16))
					HDR.HeadLen = numpy.fromstring(HDR.FILE.FID.read(8), int64).tolist()[0]
					HDR.ID.Equipment = numpy.fromstring(HDR.FILE.FID.read(8), uint8)
					HDR.ID.Hospital = numpy.fromstring(HDR.FILE.FID.read(8), uint8)
					HDR.ID.Technician = numpy.fromstring(HDR.FILE.FID.read(8), uint8)
					HDR.FILE.FID.seek(20,1)	#20bytes reserved
					HDR.NRec = numpy.fromstring(HDR.FILE.FID.read(8), int64).tolist()[0]
					HDR.Dur = numpy.fromstring(HDR.FILE.FID.read(8), uint32)
					HDR.NS = numpy.fromstring(HDR.FILE.FID.read(4), uint32).tolist()[0]
					
					#VARIABLE HEADER
					HDR.SPR = 1
					vh = HDR.FILE.FID.read(HDR.HeadLen-256)
					for k in range(HDR.NS):
						HDR.CHANNEL.append(CHANNEL_TYPE())
						i = k*16
						HDR.CHANNEL[k].Label = vh[i:i+16]
						i = i + (HDR.NS-k)*16 + k*80
						HDR.CHANNEL[k].Transducer = vh[i:i+80]
						i = i + (HDR.NS-k)*80 + k*8
						HDR.CHANNEL[k].PhysDim = vh[i:i+8]
						i = i + (HDR.NS-k)*8 + k*8
						HDR.CHANNEL[k].PhysMin = numpy.fromstring(vh[i:i+8], float64).tolist()[0]
						i = i + (HDR.NS-k)*8 + k*8
						HDR.CHANNEL[k].PhysMax = numpy.fromstring(vh[i:i+8], float64).tolist()[0]
						i = i + (HDR.NS-k)*8 + k*8
						HDR.CHANNEL[k].DigMin = numpy.fromstring(vh[i:i+8], int64).tolist()[0]
						i = i + (HDR.NS-k)*8 + k*8
						HDR.CHANNEL[k].DigMax = numpy.fromstring(vh[i:i+8], int64).tolist()[0]
						i = i + (HDR.NS-k)*8 + k*80
						HDR.CHANNEL[k].PreFilt = vh[i:i+80]
						i = i + (HDR.NS-k)*80 + k*4
						HDR.CHANNEL[k].SPR = numpy.fromstring(vh[i:i+4], uint32).tolist()[0]
						HDR.SPR = __lcm(HDR.SPR, HDR.CHANNEL[k].SPR)	# least common SPR
						HDR.AS.spb = HDR.AS.spb + HDR.CHANNEL[k].SPR
						i = i + (HDR.NS-k)*4 + k*4
						HDR.CHANNEL[k].GDFTYP = numpy.fromstring(vh[i:i+4], uint32).tolist()[0]
						HDR.CHANNEL[k].bpr = __GDFTYP_BYTE[HDR.CHANNEL[k].GDFTYP] * HDR.CHANNEL[k].SPR
						HDR.AS.bpb = HDR.AS.bpb + HDR.CHANNEL[k].bpr
						i = i + (HDR.NS-k)*4 + k*32
						#reserved
						i = i +(HDR.NS-k)*32
						
						HDR.CHANNEL[k].Cal = (HDR.CHANNEL[k].PhysMax - HDR.CHANNEL[k].PhysMin) / (HDR.CHANNEL[k].DigMax - HDR.CHANNEL[k].DigMin)
						HDR.CHANNEL[k].Off = HDR.CHANNEL[k].PhysMin - HDR.CHANNEL[k].Cal * HDR.CHANNEL[k].DigMin
						
						if i != len(vh):
							raise __FATALERROR('Error reading variable Header!\nSignal: ' + str(k))
					
					#EVENT TABLE
					etp = HDR.HeadLen + HDR.NRec*HDR.AS.bpb
					HDR.FILE.FID.seek(etp)
					etmode = HDR.FILE.FID.read(1)
					if etmode != '':
						etmode = numpy.fromstring(etmode, uint8).tolist()[0]
						sr = numpy.fromstring(HDR.FILE.FID.read(3), uint8)
						HDR.EVENT.SampleRate = sr[0]
						for i in range(1,len(sr)):
							HDR.EVENT.SampleRate = HDR.EVENT.SampleRate + sr[i]*2**i
						
						HDR.EVENT.N = numpy.fromstring(HDR.FILE.FID.read(4), uint32).tolist()[0]
						HDR.EVENT.POS = numpy.fromstring(HDR.FILE.FID.read(HDR.EVENT.N*4), uint32)
						HDR.EVENT.TYP = numpy.fromstring(HDR.FILE.FID.read(HDR.EVENT.N*2), uint16)
						
						if etmode == 3:
							HDR.EVENT.CHN = numpy.fromstring(HDR.FILE.FID.read(HDR.EVENT.N*2), uint16)
							HDR.EVENT.DUR = numpy.fromstring(HDR.FILE.FID.read(HDR.EVENT.N*4), uint32)
					
					#Finished reading Header
					HDR.FILE.OPEN = 1
					HDR.FILE.FID.seek(HDR.HeadLen)
					HDR.FILE.POS = 0
					HDR.SampleRate = float(HDR.SPR) * HDR.Dur[1] / HDR.Dur[0]
					if HDR.EVENT.SampleRate == 0:
						HDR.EVENT.SampleRate = HDR.SampleRate
					print 'Finished reading header'
					return HDR
				
				# GDF 2.0
				else:
					#FIXED HEADER
					HDR.AS.PID = HDR.FILE.FID.read(66)
					pid = HDR.AS.PID.split(' ', 2)	#PID split up in 'patient identification code', 'patient name' and 'patient classification'
					if len(pid) >= 2:
						HDR.Patient.Id = pid[0]
						HDR.Patient.Name = pid[1]
					
					HDR.FILE.FID.seek(10,1)	#10bytes reserved
					sadm = numpy.fromstring(HDR.FILE.FID.read(1), uint8).tolist()[0]	#Smoking / Alcohol abuse / drug abuse / medication
					HDR.Patient.Smoking = __SCALE[sadm%4]
					HDR.Patient.AlcoholAbuse = __SCALE[(sadm>>2)%4]
					HDR.Patient.DrugAbuse = __SCALE[(sadm>>4)%4]
					HDR.Patient.Medication = __SCALE[(sadm>>6)%4]
					
					HDR.Patient.Weight = numpy.fromstring(HDR.FILE.FID.read(1), uint8).tolist()[0]
					if HDR.Patient.Weight == 0 or HDR.Patient.Weight == 255:
						HDR.Patient.Weight = NaN
					HDR.Patient.Height = numpy.fromstring(HDR.FILE.FID.read(1), uint8).tolist()[0]
					if HDR.Patient.Height == 0 or HDR.Patient.Height == 255:
						HDR.Patient.Height = NaN
					ghi = numpy.fromstring(HDR.FILE.FID.read(1), uint8).tolist()[0]	#Gender / Handedness / Visual Impairment
					HDR.Patient.Sex = __GENDER[ghi%4]
					HDR.Patient.Handedness = __HANDEDNESS[(ghi>>2)%4]
					HDR.Patient.Impairment.Visual = __SCALE[(ghi>>4)%4]
					
					HDR.AS.RID = HDR.FILE.FID.read(64)
					rl = HDR.FILE.FID.read(16)	#Recording location (Lat, Long, Alt)
					vhsv = numpy.fromstring(rl[0:4], uint8)
					if vhsv[3] == 0:
						HDR.LOC.VertPre = 10 * int(vhsv[0]>>4) + int(vhsv[0]%16)
						HDR.LOC.HorzPre = 10 * int(vhsv[1]>>4) + int(vhsv[1]%16)
						HDR.LOC.Size = 10 * int(vhsv[2]>>4) + int(vhsv[2]%16)
					else:
						HDR.LOC.VertPre = 29
						HDR.LOC.HorizPre = 29
						HDR.LOC.Size = 29
					HDR.LOC.Version = 0
					HDR.LOC.Latitude = float(numpy.fromstring(rl[4:8], uint32).tolist()[0]) / 3600000
					HDR.LOC.Longitude = float(numpy.fromstring(rl[8:12], uint32).tolist()[0]) / 3600000
					HDR.LOC.Altitude = float(numpy.fromstring(rl[12:16], int32).tolist()[0]) / 100
					
					HDR.T0 = __gdf2_time2py_time(numpy.fromstring(HDR.FILE.FID.read(8), uint64).tolist()[0])
					HDR.Patient.Birthday = __gdf2_time2py_time(numpy.fromstring(HDR.FILE.FID.read(8), uint64).tolist()[0])
					if HDR.Patient.Birthday != datetime.datetime(1,1,1,0,0):
						today = datetime.datetime.today()
						HDR.Patient.Age = today.year - HDR.Patient.Birthday.year
						today = today.replace(year=HDR.Patient.Birthday.year)
						if today < HDR.Patient.Birthday:
							HDR.Patient.Age -= 1
					else:
						Age = NaN
					
					HDR.HeadLen = numpy.fromstring(HDR.FILE.FID.read(2), uint16).tolist()[0]*256
					HDR.FILE.FID.seek(6,1)	#6bytes reserved
					HDR.ID.Equipment = numpy.fromstring(HDR.FILE.FID.read(8), uint8)
					HDR.IPaddr = numpy.fromstring(HDR.FILE.FID.read(6), uint8)
					HDR.Patient.Headsize = numpy.fromstring(HDR.FILE.FID.read(6), uint16)
					HDR.Patient.Headsize = numpy.asarray(HDR.Patient.Headsize, float32)
					HDR.Patient.Headsize = numpy.ma.masked_array(HDR.Patient.Headsize, numpy.equal(HDR.Patient.Headsize, 0), NaN).filled()
					
					HDR.ELEC.REF = numpy.fromstring(HDR.FILE.FID.read(12), float32)
					HDR.ELEC.GND = numpy.fromstring(HDR.FILE.FID.read(12), float32)
					HDR.NRec = numpy.fromstring(HDR.FILE.FID.read(8), int64).tolist()[0]
					HDR.Dur = numpy.fromstring(HDR.FILE.FID.read(8), uint32)
					HDR.NS = numpy.fromstring(HDR.FILE.FID.read(2), uint16).tolist()[0]
					HDR.FILE.FID.seek(2,1)	#2bytes reserved
					
					#VARIABLE HEADER
					HDR.SPR = 1
					vh = HDR.FILE.FID.read(HDR.HeadLen-256)
					for k in range(HDR.NS):
						HDR.CHANNEL.append(CHANNEL_TYPE())
						i = k*16
						HDR.CHANNEL[k].Label = vh[i:i+16]
						i = i + (HDR.NS-k)*16 + k*80
						HDR.CHANNEL[k].Transducer = vh[i:i+80]
						i = i + (HDR.NS-k)*80 + k*6
						HDR.CHANNEL[k].PhysDim = vh[i:i+6]
						i = i + (HDR.NS-k)*6 + k*2
						HDR.CHANNEL[k].PhysDimCode = numpy.fromstring(vh[i:i+2], uint16).tolist()[0]
						i = i + (HDR.NS-k)*2 + k*8
						HDR.CHANNEL[k].PhysMin = numpy.fromstring(vh[i:i+8], float64).tolist()[0]
						i = i + (HDR.NS-k)*8 + k*8
						HDR.CHANNEL[k].PhysMax = numpy.fromstring(vh[i:i+8], float64).tolist()[0]
						i = i + (HDR.NS-k)*8 + k*8
						HDR.CHANNEL[k].DigMin = numpy.fromstring(vh[i:i+8], float64).tolist()[0]
						i = i + (HDR.NS-k)*8 + k*8
						HDR.CHANNEL[k].DigMax = numpy.fromstring(vh[i:i+8], float64).tolist()[0]
						i = i + (HDR.NS-k)*8 + k*68
						HDR.CHANNEL[k].PreFilt = vh[i:i+68]
						i = i + (HDR.NS-k)*68 + k*4
						HDR.CHANNEL[k].LowPass = numpy.fromstring(vh[i:i+4], float32).tolist()[0]
						i = i + (HDR.NS-k)*4 + k*4
						HDR.CHANNEL[k].HighPass = numpy.fromstring(vh[i:i+4], float32).tolist()[0]
						i = i + (HDR.NS-k)*4 + k*4
						HDR.CHANNEL[k].Notch = numpy.fromstring(vh[i:i+4], float32).tolist()[0]
						i = i + (HDR.NS-k)*4 + k*4
						HDR.CHANNEL[k].SPR = numpy.fromstring(vh[i:i+4], uint32).tolist()[0]
						HDR.SPR = __lcm(HDR.SPR, HDR.CHANNEL[k].SPR)	# least common SPR
						HDR.AS.spb = HDR.AS.spb + HDR.CHANNEL[k].SPR
						i = i + (HDR.NS-k)*4 + k*4
						HDR.CHANNEL[k].GDFTYP = numpy.fromstring(vh[i:i+4], uint32).tolist()[0]
						HDR.CHANNEL[k].bpr = __GDFTYP_BYTE[HDR.CHANNEL[k].GDFTYP] * HDR.CHANNEL[k].SPR
						HDR.AS.bpb = HDR.AS.bpb + HDR.CHANNEL[k].bpr
						i = i + (HDR.NS-k)*4 + k*12
						HDR.CHANNEL[k].XYZ = numpy.fromstring(vh[i:i+12], float32)
						i = i + (HDR.NS-k)*12 + k*1
						HDR.CHANNEL[k].Impedance= pow(2,float(numpy.fromstring(vh[i:i+1], uint8)[0])/8)
						i = i + (HDR.NS-k)*1 + k*19
						#reserved
						i = i +(HDR.NS-k)*19
						
						HDR.CHANNEL[k].Cal = (HDR.CHANNEL[k].PhysMax - HDR.CHANNEL[k].PhysMin) / (HDR.CHANNEL[k].DigMax - HDR.CHANNEL[k].DigMin)
						HDR.CHANNEL[k].Off = HDR.CHANNEL[k].PhysMin - HDR.CHANNEL[k].Cal * HDR.CHANNEL[k].DigMin
						
						if i != len(vh):
							print 'Error reading variable Header!'
							break
					
					#EVENT TABLE
					etp = HDR.HeadLen + HDR.NRec*HDR.AS.bpb
					HDR.FILE.FID.seek(etp)
					etmode = HDR.FILE.FID.read(1)
					if etmode != '':
						etmode = numpy.fromstring(etmode, uint8).tolist()[0]
						
						if HDR.VERSION < 1.94:
							sr = numpy.fromstring(HDR.FILE.FID.read(3), uint8)
							HDR.EVENT.SampleRate = sr[0]
							for i in range(1,len(sr)):
								HDR.EVENT.SampleRate = HDR.EVENT.SampleRate + sr[i]*2**i
							HDR.EVENT.N = numpy.fromstring(HDR.FILE.FID.read(4), uint32).tolist()[0]
						else:
							ne = numpy.fromstring(HDR.FILE.FID.read(3), uint8)
							HDR.EVENT.N = ne[0]
							for i in range(1,len(ne)):
								HDR.EVENT.N = HDR.EVENT.N + ne[i]*2**i
							HDR.EVENT.SampleRate = numpy.fromstring(HDR.FILE.FID.read(4), float32).tolist()[0]

						HDR.EVENT.POS = numpy.fromstring(HDR.FILE.FID.read(HDR.EVENT.N*4), uint32)
						HDR.EVENT.TYP = numpy.fromstring(HDR.FILE.FID.read(HDR.EVENT.N*2), uint16)
						
						if etmode == 3:
							HDR.EVENT.CHN = numpy.fromstring(HDR.FILE.FID.read(HDR.EVENT.N*2), uint16)
							HDR.EVENT.DUR = numpy.fromstring(HDR.FILE.FID.read(HDR.EVENT.N*4), uint32)
					
					#Finished reading Header
					HDR.FILE.OPEN = 1
					HDR.FILE.FID.seek(HDR.HeadLen)
					HDR.FILE.POS = 0
					HDR.SampleRate = float(HDR.SPR) * HDR.Dur[1] / HDR.Dur[0]
					if HDR.EVENT.SampleRate == 0:
						HDR.EVENT.SampleRate = HDR.SampleRate
					print 'Finished reading header'
					return HDR
			
			# other File Formats
			else:
				print 'This file format is not implemented yet!'
				return HDR
			
		# WRITE
		elif MODE == 'w':
			print 'Writing is not implemented yet!'
			return HDR
		
	except __FATALERROR, e:
		print ''
		print 'FATAL ERROR:'
		print '============'
		print e.value
		print '============'
		print ''
	# End of SOPEN
	
	
	###########
	# SCLOSE  #
	###########
def sclose(HDR):
	"""input:  HDR_TYPE HDR
	output: [-1, 0]
	Closes the according file.
	Returns 0 if successfull, -1 otherwise."""
	
	if HDR.FILE.OPEN != 0:
		HDR.FILE.FID.close()
		HDR.FILE.FID = 0
		HDR.FILE.OPEN = 0
		return 0
	return -1
	# End of SCLOSE
	
	
	###########
	#  SREAD  #
	###########
def sread(HDR, length = -1, start = 0):
	"""input: HDR_TYPE HDR, int length, int start
	output: array
	Reads and returns data according to length and start.
	length is the number of blocks to read. Use -1 to read all blocks until the end.
	start is the block to begin reading with.
	Use HDR.CHANNEL[k].OnOff to exclude single channels from reading."""
	
	raw = ''			# const input string
	channel = []			# const channels to read
	ns = 0				# const number of signals to read
	
	i = 0				# index for writing in output matrix
	bstart = 0			# start index (in bytes) of next value on the input string
	bend = 0			# length (in bytes) of the next value on the input string and next bstart
	row = 0				# row in output matrix
	column = 0			# column in output matrix
	ch = 0				# actual channel
	value = 0			# actual value to insert into output matrix
	
	for ch in range(HDR.NS):
		if HDR.CHANNEL[ch].OnOff != 0:
			channel.append(ch)
	ns = len(channel)
	
	seek = 0
	if start >= 0:
		seek = sseek(HDR, start, -1)
	
	if seek == 0:
		if length == -1:
			length = HDR.NRec	# read all blocks
		length = max(min(length, HDR.NRec - HDR.FILE.POS),0)		# number of blocks to read
		HDR.data.block = numpy.ones((HDR.SPR * length,ns), float64) * NaN	# right sized output matrix filled with NaN's
		raw = HDR.FILE.FID.read(HDR.AS.bpb * length)			# read input string from file
		
		for i in range(numpy.size(HDR.data.block)):
			row = i / (ns * HDR.SPR) * HDR.SPR + i % HDR.SPR
			column = (i / HDR.SPR) % ns
			ch = channel[column]
			
			# OnOff calculations
			# not all channels are being read and it's the first row in a block
			if (ns != HDR.NS) and (row % HDR.SPR == 0):
				# first channel is missing
				if (column == 0) and (channel[0] != 0):
					# first block -> take only the leading missing channels in account
					# other blocks -> take trailing and leading missing channels in account
					if i != 0:
						# for all channels being left out between channel[-1] (last wanted channel) and HDR.NS (last possible channel)
						for leftout in range(channel[-1] + 1, HDR.NS):
							bstart += HDR.CHANNEL[leftout].bpr
					# for all channels being left out between 0 and channel[column]
					for leftout in range(ch):
						bstart += HDR.CHANNEL[leftout].bpr
				
				# channels in between are missing
				elif ch != channel[column - 1] + 1:
					# for all channels being left out between channel[column - 1] and channel[column]
					for leftout in range(channel[column - 1] + 1, ch):
						bstart += HDR.CHANNEL[leftout].bpr
			
			
			# reading from input string, calculating right value and inserting in output matrix
			if (row * HDR.CHANNEL[ch].SPR) % HDR.SPR == 0:
				# reading and calculating of value
				# BDF
				if HDR.CHANNEL[ch].GDFTYP == 255 + 24:
					bend = bstart + 3
					value = numpy.fromstring(raw[bstart:bend], uint8).tolist()
					if value[2] >= 128:	# minus
						value = value[0] + value[1] * 2**8 + (value[2] - 256) * 2**16
					else:			# plus
						value = value[0] + value[1] * 2**8 + value[2] * 2**16
				# EDF, GDF
				elif HDR.CHANNEL[ch].GDFTYP < 18:
					bend = bstart + __GDFTYP_BYTE[HDR.CHANNEL[ch].GDFTYP]
					value = numpy.fromstring(raw[bstart:bend], __GDFTYP_NAME[HDR.CHANNEL[ch].GDFTYP]).tolist()[0]
					
				else:
					raise __FATALERROR('Error SREAD: datatype ' + HDR.CHANNEL[ch].GDFTYP + ' not supported!')
					
				# calculating new input string position
				bstart = bend
				
				# overflow and saturation detection
				if (HDR.FLAG.OVERFLOWDETECTION != 0) and ((value <= HDR.CHANNEL[ch].DigMin) or (value >= HDR.CHANNEL[ch].DigMax)):
					value = NaN
					
				# calibration
				elif HDR.FLAG.UCAL == 0:
					value = value * HDR.CHANNEL[ch].Cal + HDR.CHANNEL[ch].Off

				# inserting in output matrix
				HDR.data.block[row, column] = value
			
			
			# inserting same value multiply times (HDR.CHANNEL[].SPR != HDR.SPR)
			else:
				HDR.data.block[row, column] = value
		
		
		HDR.data.size = HDR.data.block.shape
		HDR.FILE.POS = HDR.FILE.POS + length
	
		print 'Finished reading data'
	return HDR.data.block
	# End of SREAD
	
	
	###########
	# SWRITE  #
	###########
def swrite(HDR, ptr, nelem):
	"""Writing is not implemented yet!"""
	print 'Writing is not implemented yet!'
	#End of SWRITE
	
	
	###########
	#  SEOF   #
	###########
def seof(HDR):
	"""input: HDR_TYPE HDR
	output: [-1, 0]
	Indicates if end of data is reached.
	Returns 0 after the last block, -1 otherwise."""
	
	if HDR.FILE.POS >= HDR.NRec:
		return 0
	return -1
	#End of SEOF
	
	
	###########
	# SREWIND #
	###########
def srewind(HDR):
	"""input: HDR_TYPE HDR
	output: None
	Sets the data pointer back to the beginning.
	No return value."""
	
	if HDR.FILE.OPEN != 0:
		HDR.FILE.FID.seek(HDR.HeadLen)
		HDR.FILE.POS = 0
	#End of SREWIND
	
	
	###########
	#  SSEEK  #
	###########
def sseek(HDR, offset = 1, whence = 0):
	"""input: HDR_TYPE HDR, int offset, [-1, 0, 1] whence
	output: [-1, 0]
	Sets the position pointer to the desired position.
	offset is measured in blocks
	whence:	-1 -> from beginning of data
		 0 -> from actual position
		 1 -> from end of data
	If an error occurs, the data pointer is not moved and function returns -1, 0 otherwise."""
	
	if HDR.FILE.OPEN != 0:
		if whence < 0:
			pos = HDR.HeadLen + offset * HDR.AS.bpb
		elif whence == 0:
			pos = HDR.HeadLen + (HDR.FILE.POS + offset) * HDR.AS.bpb
		elif whence > 0:
			pos = HDR.HeadLen + (HDR.NRec + offset) * HDR.AS.bpb
		
		if pos < HDR.HeadLen or pos > HDR.HeadLen + HDR.NRec * HDR.AS.bpb:
			return -1
		else:
			HDR.FILE.FID.seek(pos)
		
		HDR.FILE.POS = (pos - HDR.HeadLen) / HDR.AS.bpb
		return 0
	return -1
	#End of SSEEK
	
	
	###########
	#  STELL  #
	###########
def stell(HDR):
	"""input: HDR_TYPE HDR
	output: int
	Returns the actual position of the data pointer in blocks.
	If an error occurs, function returns -1."""
	
	if HDR.FILE.OPEN != 0:
		pos = HDR.FILE.FID.tell()
		if pos == HDR.FILE.POS * HDR.AS.bpb + HDR.HeadLen:
			return HDR.FILE.POS
	return -1
	#End of STELL
	
	
######################
# INTERNAL FUNCTIONS #
######################

def __gcd(a, b):
	while (b != 0):
		t = b
		b = a % b
		a = t
	return a

def __lcm(a, b):
	return (abs(a*b)/__gcd(a,b))

def __py_time2t_time(t):
	delta = t - datetime.datetime(1970,1,1)
	return (delta.days * 86400 + float(delta.seconds + delta.microseconds * pow(10,-6)))

def __t_time2py_time(t):
	return (datetime.datetime(1970,1,1) + datetime.timedelta(t / 86400))

def __gdf_time2py_time(t):
	if t[14:16] == '  ':
		t[14:16] = '00'
	return (datetime.datetime(int(t[0:4]),int(t[4:6]),int(t[6:8]),int(t[8:10]),int(t[10:12]),int(t[12:14]),int(t[14:16])*pow(10,4)))

def __py_time2gdf_time(t):
	return (t.strftime('%Y%m%d%H%M%S') + str(t.microsecond/pow(10,4)))

def __gdf2_time2py_time(t):
	if t == 0:
		t = 367 * pow(2,32)
	return (datetime.datetime(1,1,1) + datetime.timedelta(t * pow(2,-32) - 367))	# don't know if there's an error in datetime but the 367 days shift gives the right result

def __py_time2gdf2_time(t):
	delta = t - datetime.datetime(1,1,1)
	if t == datetime.datetime(1,1,1):
		delta = datetime.timedelta(-367)
	return int((delta.days + float(delta.seconds + delta.microseconds * pow(10,-6)) / 86400 + 367) * pow(2,32))

def __gdf2_time2t_time(t):
	return ((t * pow(2,-32) - 719529) * 86400)

def __t_time2gdf2_time(t):
	return int((float(t) / 86400 + 719529) * pow(2,32))
