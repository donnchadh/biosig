% BIOSIG runs on Matlab and Octave. 
% This is a script installing all components in an automatically.
%  
% 1) extract the files and
% 2) save the BIOSIG files in <your_directory>
% 3) start matlab
%    cd <your_directory>
%    biosig_installer 
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

%	Copyright (C) 2003-2005 by Alois Schloegl <a.schloegl@ieee.org>	
%	$Revision: 1.4 $
%	$Id: install.m,v 1.4 2005-02-28 09:37:07 schloegl Exp $

if exist('OCTAVE_VERSION')
        HOME = pwd;	%
else
        tmp = which('install'); 
        if ~isempty(tmp),
                [HOME,f,e] = fileparts(tmp);
                if ~isempty(findstr(lower(HOME),'biosig'))
                        [HOME,f,e] = fileparts(HOME);
                end;
        else
                tmp = which('biosig_installer'); 
                [HOME,f,e] = fileparts(tmp);
        end;
end;
path([HOME,'/biosig/'],path);			% dataformat
path([HOME,'/biosig/demo/'],path);		% dataformat
path([HOME,'/biosig/t200/'],path);		% dataformat
path([HOME,'/biosig/t250/'],path);		% trigger and quality control
path([HOME,'/biosig/t300/'],path);		% signal processing and feature extraction
path([HOME,'/biosig/t400/'],path);		% classification
path([HOME,'/biosig/t490/'],path);		% evaluation criteria
path([HOME,'/biosig/t500/'],path);		% display and presentation
%path([HOME,'/biosig/t600/'],path);
path([HOME,'/biosig/viewer/'],path);		% viewer
path([HOME,'/biosig/viewer/utils'],path);	% viewer
path([HOME,'/biosig/viewer/help'],path);	% viewer

path([HOME,'/tsa/'],path);		%  Time Series Analysis
% some users might get confused by this
path([HOME,'/NaN/'],path);		%  Statistics analysis for missing data

if exist([HOME,'/biosig/eeglab/'],'dir'),
	path([HOME,'/biosig/eeglab/'],path);
end;
if exist([HOME,'/eeglab/'],'dir'),
	path([HOME,'/eeglab/'],path);
end;
%%% NONFREE %%%
if exist([HOME,'/biosig/NONFREE/EEProbe'],'dir'),
	path(path,[HOME,'/biosig/NONFREE/EEProbe']);	% Robert Oostenveld's MEX-files to access EEProbe data
end;
if exist([HOME,'/biosig/NONFREE/meg-pd-1.2-4/'],'dir'),
        path(path,[HOME,'/biosig/NONFREE/meg-pd-1.2-4/']);	% Kimmo Uutela's library to access FIF data
end;

path(path,[HOME,'/maybe-missing/']);

% test of installation 
fun = {'isdir','ischar','strtok','str2double','strcmpi','strmatch','bitand','bitshift','sparse','strfind'};
for k = 1:length(fun),
        x = which(fun{k});
        if isempty(x) | strcmp(x,'undefined'),
                fprintf(2,'Function %s is missing\n',upper(fun{k}));     
        end;
end;
        
if exist('OCTAVE_VERSION') > 2,	% OCTAVE
        fun = {'bitand'};
        for k = 1:length(fun),
                try,
                        bitand(5,7);
                catch
                        unix('mkoctfile maybe-missing/bitand.cc');
                end;

                try,
                        x = which(fun{k});
                catch
                        x = [];
                end;	

                if isempty(x) | strcmp(x,'undefined'),
                        fprintf(2,'Function %s is missing. \n',upper(fun{k}));     
                end;
        end;
	if any(size(sparse(5,4))<0)
        	fprintf(2,'Warning: Size of Sparse does not work\n')
	end;
end;

% test of installation 
% 	Some users might get confused by it. 
% nantest;	
% naninsttest; 

disp('BIOSIG-toolbox activated');
if ~exist('OCTAVE_VERSION'),	% OCTAVE
    disp('	If you want BIOSIG permanently installed, use the command PATH2RC.')
    disp('	or use PATHTOOL to select and deselect certain components.')
end; 
