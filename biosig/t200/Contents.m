% DATAFORMATS contains Matlab/Octave functions to access various biosignal dataformats
% For simiplicity we call all supported files "Biosig"-files.  
% Currently, the following formats are are supported:
%	BKR	Dept for Medical informatics, University of Technology Graz
%     	CNT	Neuroscan 
%	EDF	European Data Format
%	BDF	Biosemi data format (EDF with 24bit integers)
%	GDF	General data format (EDF different datatypes)
%       EGI	Format from Electrical Geodesics
%
% A united interface is provided for all data formats:
%	LOADEEG Opens, reads all data and closes file Biosig files. 
%
%  	EEGOPEN opens an Biosignal file (and reads all header information)
%	EEGREAD	reads data blockwise
%       EEGEOF	checks end-of-file
%  	EEGTELL	returns position of file handle
%	EEGSEEK	moves file handle to position
%  	EEGREWIND moves file handle to beginning 
%	EEGCLOSE closes an biosignal file 
%  	EEGWRITE (only BKR and EDF implemented)
%
%    UTILITY FUNCTIONS. In general, it is not recommended 
%	to use them directly. Use them only if you absolute sure what 
%	you are doing. You are warned! 
% 	
%	SAVE2BKR
% 	EEGCHKHDR	
%	OPENLDR
%	BKROPEN
%	CNTOPEN
%	OPENEGI
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
% http://www.dpmi.tu-graz.ac.at/~schloegl/matlab/eeg

%	$Revision: 1.2 $
%	$Id: Contents.m,v 1.2 2003-02-03 17:00:07 schloegl Exp $
%	CopyLeft (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	

