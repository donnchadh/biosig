% BIOSIG/T200 contains Matlab/Octave functions to access various biosignal dataformats
% For simiplicity we call all supported files "Biosig"-files.  
% Currently, the following formats are are supported:
%	AIFF/C	Apple's audio interchange format 
%	ACQ	Biopac ACQ format 
%	ADI/CFWB Chart (translated binary) format from ADInstruments
%	alpha	alpha trace 
%	BDF	Biosemi data format (EDF with 24bit integers)
%	BKR	Dept for Medical informatics, University of Technology Graz
%	BV	Brainvision
%     	CNT	Neuroscan (continous data)
%	CTF-MEG	CTF System
%	DEMG	DelSys Inc.
%	EDF+	European Data Format
%       EEG	Neuroscan (triggered data)
%       EGI	Format from Electrical Geodesics
%	EEProbe	ANT Software
%	FIFF	NeuroMag FIF format
%	GDF	General data format (EDF different datatypes)
%	ISHNE	The ISHNE Holter Standard Output File Format
%	MFER	medical waveform format encoding rules
%	MIT/ECG	PhysioNet data format
%	SCP-ECG	Standard Communication Protocol - Computer-assisted electrocardiography
%	ISHNE	The ISHNE Holter Standard Output File Format
%	Poly5/TMS32 Format used for recordings with the Porti-system and the PortiLab software
%	RDF	EPRSS Data Format
%	Sierra ECG 1.03	Philips XML format for ECG data
%	SMA	Snap-Master file format
%	SND/AU	Next/SUN sound file
%	WAV	Audio, Microsoft's PCM data format
%
%
% A united interface is provided for all data formats:
%  	SOPEN 	opens an Biosignal file (and reads all header information)
%	SREAD	reads data blockwise
%       SEOF	checks end-of-file
%  	STELL	returns position of file handle
%	SSEEK	moves file handle to position
%  	SREWIND moves file handle to beginning 
%	SCLOSE 	closes an biosignal file 
%  	SWRITE 	writes data blocks 
%
%       GETFILETYPE identifies the type (format) of a file. 
%	SLOAD 	Opens, reads and closes signal files. 
%	SSAVE 	Opens, writes and closes signal files. 
%		SLOAD and SSAVE provide a simple interface to signal files. 
% 
% UTILITY FUNCTIONS. In general, it is not recommended 
%	to use them directly. Use them only if you absolute sure what 
%	you are doing. You are warned! 
% 	
%       sload('eventcodes.txt')   loads latest version of table for event codes
%	SAVE2BKR
%	SAVE2TXT
% 	EEGCHKHDR	
%	OPENLDR
%	BKROPEN
%	CNTOPEN
%	SDFOPEN
%	SDFREAD
%	SDFERROR
%	GDFDATATYP
%	ADB2EVENT  converts artifact scorings into Event information
%	WSCORE2EVENT  converts events in WSCORE format into BIOSIG event information
%	
%
% Other functions from earlier projects are included, 
% but they might become obsolete in future. 
% 
%
% REFERENCES: 
% [1] http://www.dpmi.tu-graz.ac.at/~schloegl/matlab/eeg/
% [2] http://biosig.sf.net/
%

%	$Revision: 1.14 $
%	$Id: Contents.m,v 1.14 2004-09-09 15:21:37 schloegl Exp $
%	CopyLeft (c) 1997-2004 by Alois Schloegl <a.schloegl@ieee.org>	
%	This is part of the BIOSIG project http://biosig.sf.net/

