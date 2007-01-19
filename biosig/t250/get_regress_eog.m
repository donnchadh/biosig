function [h0, s01] = get_regress_eog(fn,arg2)
% GET_REGRESS_EOG tries to obtain the regression coefficients
%    for EOG correction. According to [1], some extra recordings 
%    with large eye movements (i.e. EOG artifacts) are needed. 
%    GET_REGRESS_EOG tries to identify this data. 
% 
%    Some heuristics is used, which is based on lab-specific standards. 
%    If your data is not supported, contact <a.schloegl@ieee.org>
% 
% Warning: this is a utility function used by biosig. do not use it 
% directly, unless you know what you are doing. At least you are warned. 
%
% See also: SLOAD, IDENTIFY_EOG_CHANNELS, BV2BIOSIG_EVENTS, REGRESS_EOG 
%
% Reference(s):
% [1] Schlogl A, Keinrath C, Zimmermann D, Scherer R, Leeb R, Pfurtscheller G. 
%	A fully automated correction method of EOG artifacts in EEG recordings.
%	Clin Neurophysiol. 2007 Jan;118(1):98-104. Epub 2006 Nov 7.
% 	http://dx.doi.org/10.1016/j.clinph.2006.09.003
%       http://www.dpmi.tugraz.at/~schloegl/publications/schloegl2007eog.pdf

%	$Id: get_regress_eog.m,v 1.3 2007-01-19 15:54:58 schloegl Exp $
%	Copyright (C) 2006,2007 by Alois Schloegl 
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% Version 2 of the License, or (at your option) any later version.



	% extract information from BBCI data for EOG correction
	if nargin<2,
		CHAN = 0 ; 
	end; 	

	[p,f,e]=fileparts(fn); 

	%%%=== search for eye-movement recordings (in order to obtain clean EOG artifacts) ===%%%
	%	an heuristic approach is to support lab-specific standards
	% 	currently, the convention of the GrazBCI and the BBCI Lab are supported
	%
	%
	
	f0 = dir(fullfile(p,'arte*.vhdr')); % BBCI files 
	LAB_ID = 'BBCI'; 
	if length(f0)==1,
		feog = fullfile(p,f0.name);
	else
		f0 = dir(fullfile(p,'EOG/*.gdf')); % Graz BCI files 
		LAB_ID = 'GrazBCI'; 
		if length(f0)==1,
			feog = fullfile(p,'EOG',f0.name);
		end; 	
	end

	if length(f0)<1,
		fprintf('error: no artefact file found\n'); 
		return; 
	end
	if length(f0)>1,
		fprintf('error: more than one artefact file found!\n'); 
		{f0.name}
		return; 
	end; 

	% load EOG artefacts data
        [s00,h0] = sload(feog);
        
	if nargin==1,
	        % define EOG channels
	        eogchan = identify_eog_channels(h0); 
	else
	        % define xEOG channels
	        eogchan = identify_eog_channels(h0,arg2); 
	end;
       	% remove EOG channels for list of corrected channels
	chan = find(~any(eogchan,2)); 

		
	if strcmp(LAB_ID,'BBCI'),                
	        % find eye movements in BBCI recordings

        	tmp = bitand(2^15-1,h0.EVENT.TYP);
		tmp = find((tmp > hex2dec('0430')) & (tmp<=hex2dec('0439')));
		ix1 = min(tmp); 
		ix2 = max(tmp); 

	        % extract segment with large eye movements/EOG artifacts
	        s00 = s00(h0.EVENT.POS(ix1):h0.EVENT.POS(ix2)+h0.EVENT.DUR(ix2),:); 
	end;

        % regression analysis - compute correction coefficients. 
        
        [h0.REGRESS, s01] = regress_eog(s00,chan,eogchan); 

        if 0, 
        	%%% if you have a method that can identify an additional EOG component, 
        	%%% (maybe ICA or something else) this should go in here. 
        
        	echan = []; % select EEG channels for ICA analysis
		ochan = find(any(eogchan,2)); % include global average EOG into ICA analysis
		rx  = sparse([eogchan(:);echan(:)],[ones(size(ochan(:)));echan(:)+1],1,HDR.NS,length(echan)+1); 

		% identify the third EOG component, using perhaps ICA or something else
		%  zeog should contain the additional EOG component(s) 
		%  zeog = ica(s01*rx, ... );   % identify additional EOG component 
		
		eogchan = [eogchan, rx*zeog(:)];
        	[h0.REGRESS,s01] = regress_eog(s00,chan,eogchan); 
	end
	
