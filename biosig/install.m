
% BIOSIG runs on Matlab and Octave. 
% To install the toolbox you need to run this program.
%
% 1) extract the files and
% 2) save the BIOSIG files in <your_directory>
% 3) start matlab
%    cd <your_directory>
%    install 

disp('BIOSIG-toolbox activated');

if exist('OCTAVE_VERSION')
	tmp = pwd;	%
        ix=max(find(tmp==filesep));
        HOME = tmp(1:ix);
else
	tmp=which('install'); 
	[HOME,f,e]=fileparts(tmp);
end;
addpath([HOME,'/biosig/demo/']);		% dataformat
addpath([HOME,'/biosig/t200/']);		% dataformat
addpath([HOME,'/biosig/t250/']);		% trigger and quality control
addpath([HOME,'/biosig/t300/']);		% signal processing and feature extraction
addpath([HOME,'/biosig/t400/']);		% classification
addpath([HOME,'/biosig/t490/']);		% evaluation criteria
addpath([HOME,'/biosig/t500/']);		% display and presentation
addpath([HOME,'/biosig/viewer/']);		% viewer

addpath([HOME,'/tsa/']);		%  Time Series Analysis
addpath([HOME,'/NaN/']);		%  Statistics analysis for missing data

% save path permanently to PATHDEF
if exist('path2rc')>1,
        tmp=path2rc;
        if tmp==0,
                fprintf(1,'Toolboxes BIOSIG, TSA and NaN added permanently in MATLABPATH\n');
        end;
end;

% test of installation 
naninsttest; 

if exist('bitand')<2,	
        fprintf(1,'warning: BITAND is not available. MIT-BIH format not supported\n');
end;
if exist('bitshift')<2,	
        fprintf(1,'warning: BITSHIFT is not available. MIT-BIH format not supported\n');
end;
try,
	c=fwrite(1,0,'bit24');
catch
        fprintf(1,'Warning: datatype BIT24 is not available. BDF-format not supported. Some GDF-files might not be supported\n');
end;        

%       Version 0.20        10 Jul 2003
%	Copyright (C) 2003 by Alois Schloegl <a.schloegl@ieee.org>	
%	$Revision: 1.1 $
%	$Id: install.m,v 1.1 2003-10-03 10:30:27 schloegl Exp $


