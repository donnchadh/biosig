% DATAFORMATS contains Matlab/Octave functions to access various biosignal dataformats
% For simiplicity we call all supported files "Biosig"-files.  
% Currently, the following formats are are supported:
%	BDF	Biosemi data format (EDF with 24bit integers)
%	BKR	Dept for Medical informatics, University of Technology Graz
%     	CNT	Neuroscan (continous data)
%	EDF	European Data Format
%       EEG	Neuroscan (triggered data)
%       EGI	Format from Electrical Geodesics
%	GDF	General data format (EDF different datatypes)
%	ISHNE	The ISHNE Holter Standard Output File Format
%	MIT/ECG	PhysioNet data format
%	SCP-ECG	Standard Communication Protocol - Computer-assisted electrocardiography
%	ISHNE	The ISHNE Holter Standard Output File Format
%	Poly5/TMS32 Format used for recordings with the Porti-system and the PortiLab software
%	RDF	EPRSS Data Format
%	SMA	Snap-Master file format
%	WAV	Audio, Microsoft's PCM data format
%	AIFF/C	Apple's audio interchange format 
%	SND/AU	Next/SUN sound file
%	ADI/CFWB Chart (translated binary) format from ADInstruments
%
%
% A united interface is provided for all data formats:
%	SLOAD Opens, reads all data and closes file Biosig files. 
%
%  	SOPEN 	opens an Biosignal file (and reads all header information)
%	SREAD	reads data blockwise
%       SEOF	checks end-of-file
%  	STELL	returns position of file handle
%	SSEEK	moves file handle to position
%  	SREWIND moves file handle to beginning 
%	SCLOSE 	closes an biosignal file 
%  	SWRITE 	(only BKR and EDF/BDF/GDF implemented)
%
%
% UTILITY FUNCTIONS. In general, it is not recommended 
%	to use them directly. Use them only if you absolute sure what 
%	you are doing. You are warned! 
% 	
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

%	$Revision: 1.9 $
%	$Id: Contents.m,v 1.9 2004-02-07 16:51:31 schloegl Exp $
%	CopyLeft (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	

