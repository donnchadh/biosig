% BIOSIG runs on Matlab and Octave. 
% This is a script installing all components in an automatically.
%  
% 1) extract the files and
% 2) save the BIOSIG files in <your_directory>
% 3) start matlab
%    cd <your_directory>
%    install 
% 4) For a permanent installation, save the default path with 
%     PATH2RC or
%     PATHTOOL and click on the "SAVE" button. 
% 5) For removing the toolbox 
%    remove the path to 
%       HOME/tsa
%       HOME/NaN
%       HOME/BIOSIG/ and all its subdirectories
% 
%  NOTE: by default also the NaN-toolbox is installed - 
%  - a statistical toolbox for handling missing values - which 
%  changes the behaviour of some standard functions. For more  
%  information see NaN/README.TXT . In case you do not want this, 
%  you can excluded the path to NaN/*. The BIOSIG tools will still 
%  work, but does not support the handling of NaN's.

%	Copyright (C) 2003,2004 by Alois Schloegl <a.schloegl@ieee.org>	
%	$Revision: 1.2 $
%	$Id: install.m,v 1.2 2004-01-23 12:37:00 schloegl Exp $

if exist('OCTAVE_VERSION')
        tmp = pwd;	%
        ix = max(find(tmp==filesep));
        HOME = tmp(1:ix);
else
        tmp = which('install'); 
        [HOME,f,e] = fileparts(tmp);
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

% test of installation 
naninsttest; 

try,
        c = fwrite(1,0,'bit24');
catch
        fprintf(1,'Warning: datatype BIT24 is not available. BDF-format not supported. Some GDF-files might not be supported\n');
end;        

disp('BIOSIG-toolbox activated');
