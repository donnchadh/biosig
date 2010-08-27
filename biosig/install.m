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

%	$Id$
%	Copyright (C) 2003-2005,2006,2007,2008,2009,2010 by Alois Schloegl <a.schloegl@ieee.org>
%	This is part of the BIOSIG-toolbox http://biosig.sf.net/

BIOSIG_HOME = pwd;	%
if exist('./t200_FileAccess','dir')
	% install.m may reside in .../biosig/ or above (...)
        [BIOSIG_HOME,f,e] = fileparts(BIOSIG_HOME);
elseif exist('biosig','dir')
else 
        fprintf(2,'Error: biosig subdirectories not found\n');
        return;
end; 

path([BIOSIG_HOME,'/biosig'],path);			% 
path([BIOSIG_HOME,'/biosig/demo'],path);		% demos
path([BIOSIG_HOME,'/biosig/doc'],path);		% docus, Eventtable etc. 
path([BIOSIG_HOME,'/biosig/t200_FileAccess'],path);		% dataformat
path([BIOSIG_HOME,'/biosig/t250_ArtifactPreProcessingQualityControl'],path);		% trigger and quality control
path([BIOSIG_HOME,'/biosig/t300_FeatureExtraction'],path);		% signal processing and feature extraction
path([BIOSIG_HOME,'/biosig/t400_Classification'],path);		% classification
path([BIOSIG_HOME,'/biosig/t450_MultipleTestStatistic'],path);		% statistics, false discovery rates
path([BIOSIG_HOME,'/biosig/t490_EvaluationCriteria'],path);		% evaluation criteria
path([BIOSIG_HOME,'/biosig/t500_Visualization'],path);		% display and presentation
path([BIOSIG_HOME,'/biosig/t501_VisualizeCoupling'],path);		% visualization ofcoupling analysis

if ~exist('OCTAVE_VERSION','builtin'),	
	%% Matlab
	path([BIOSIG_HOME,'/biosig/viewer'],path);		% viewer
	path([BIOSIG_HOME,'/biosig/viewer/utils'],path);	% viewer
	path([BIOSIG_HOME,'/biosig/viewer/help'],path);	% viewer

	path(path,[BIOSIG_HOME,'/freetb4matlab/signal'],'-end');	% Octave-Forge signal processing toolbox converted with freetb4matlab
else 
	%% Octave
	path(path,[BIOSIG_HOME,'/freetb4matlab/signal']);	% Octave-Forge signal processing toolbox converted with freetb4matlab
end;

path(path,[BIOSIG_HOME,'/freetb4matlab/basic'],'-end');	% some basic functions used in Octave but not available in Matlab
path(path,[BIOSIG_HOME,'/freetb4matlab/statistics/distributions']);	% Octave-Forge statistics toolbox converted with freetb4matlab 
path(path,[BIOSIG_HOME,'/freetb4matlab/statistics/tests']);	% Octave-Forge statistics toolbox converted with freetb4matlab 

path([BIOSIG_HOME,'/tsa'],path);		%  Time Series Analysis
%path([BIOSIG_HOME,'/tsa/inst'],path);		%  Time Series Analysis
% some users might get confused by this

fprintf(1,'\nThe NaN-toolbox is going to be installed\n'); 
fprintf(1,'The NaN-toolbox is a powerful statistical and machine learning toolbox, \nwhich is also able to handle data with missing values.\n');
fprintf(1,'Typically, samples with NaNs are simply skipped.\n');
fprintf(1,'If your data contains NaNs, installing the NaN-toolbox will \nmodify the following functions in order to ignore NaNs:\n');
fprintf(1,'\tcor, corrcoef, cov, geomean, harmmean, iqr, kurtosis, mad, mahal, mean, \n\tmedian, moment, quantile, prctile, skewness, std, var.\n');
fprintf(1,'If you do not have NaN, the behaviour is the same; if you have NaNs in your data, you will get more often a reasonable result instead of a NaN-result.\n');
fprintf(1,'If you do not want this behaviour, remove the directory NaN/inst from your path.\n'); 
fprintf(1,'Moreover, NaN-provides also a number of other useful functions. Installing NaN-toolbox is recommended.\n\n');

	%% add NaN-toolbox: a toolbox for statistics and machine learning for data with Missing Values
path([BIOSIG_HOME,'/NaN'],path);
path([BIOSIG_HOME,'/NaN/inst'],path);
path([BIOSIG_HOME,'/NaN/src'],path);

p = pwd; 
try
	if ~exist('OCTAVE_VERSION','builtin') && ~strcmp(computer,'PCWIN'),
		mex -setup
	end; 
        if ~strcmp(computer,'PCWIN')
        	cd([BIOSIG_HOME,'/NaN/src']);
	        make
	end;         
end;
cd(p);

%%% NONFREE %%%
if exist([BIOSIG_HOME,'/biosig/NONFREE/EEProbe'],'dir'),
	path(path,[BIOSIG_HOME,'/biosig/NONFREE/EEProbe']);	% Robert Oostenveld's MEX-files to access EEProbe data
end;
if exist([BIOSIG_HOME,'/biosig/NONFREE/meg-pd-1.2-4'],'dir'),
        path(path,[BIOSIG_HOME,'/biosig/NONFREE/meg-pd-1.2-4']);	% Kimmo Uutela's library to access FIF data
end;

% test of installation 
fun = {};
for k = 1:length(fun),
        x = which(fun{k});
        if isempty(x) | strcmp(x,'undefined'),
                fprintf(2,'Function %s is missing\n',upper(fun{k}));     
        end;
end;
try 
    x = betainv(.5, 1, 2);
catch     
    disp('statistics/distribution toolbox (betainv) is missing');	
end; 
try 
    [b,a] = butter(5,[.08,.096]);
catch
    disp('signal processing toolbox (butter) is missing');	
end; 
try 
    x = mod(1:10,3)'-1;
    [Pxx,f]=periodogram(x,[],10,100);
catch
    disp('function periodogram() is missing or not up to date.');	
end; 

disp('BIOSIG-toolbox activated');
disp('	If you want BIOSIG permanently installed, use the command SAVEPATH.')
disp('	or use PATHTOOL to select and deselect certain components.')
