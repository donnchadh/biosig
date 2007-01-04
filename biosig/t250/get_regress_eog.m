function h0=get_regress_eog(fn)
% GET_REGRESS_EOG tries to obtain the regression coefficients
%    for EOG correction. This function tries to find the 
%    data with EOG movement in an automated way. Some heuristics 
%    is used, which is based on lab-specific standards. 
% 
% Warning: this is a utility function used by biosig. do not use it 
% directly, unless you know what you are doing. At least you are warned. 
%
% See also: SLOAD, IDENTIFY_EOG_CHANNELS, BV2BIOSIG_EVENTS, REGRESS_EOG 

%	$Id: get_regress_eog.m,v 1.1 2007-01-04 10:24:09 schloegl Exp $
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
        
        % define EOG channels
        eogchan = identify_eog_channels(h0); 
        % remove EOG channels for list of corrected channels
	chan = find(~any(eogchan,2)); 

	if strcmp(LAB_ID,'BBCI'),                
	        %%%% convert BV (BBCI) events into BioSig Event-Codes
        	% h0 = bv2biosig_events(h0);         

	        % find eye movements in BBCI recordings
        	%ix1 = min([strmatch('Augen',h0.EVENT.Desc);strmatch('augen',h0.EVENT.Desc)]);
	        ix1 = min(find(bitand(hex2dec('7ff0'),h0.EVENT.TYP)==hex2dec('0430'))); % first eye movement
        	%ix2 = min(strmatch('blinzeln',h0.EVENT.Desc)+1); 
	        ix2 = max(find(bitand(2^15-1,h0.EVENT.TYP)==hex2dec('0439'))); % end of blinks
	                
	        % extract segment with large eye movements/EOG artifacts
	        s00 = s00(h0.EVENT.POS(ix1):h0.EVENT.POS(ix2),:); 
	end;
	        
        % regression analysis - compute correction coefficients. 
        [h0.REGRESS, s01] = regress_eog(s00,chan,eogchan); 

	zeog = []; 
	% ICA analysis for identifying the third EOG component (?)
	%  zeog should contain the additional EOG component(s) 

        if ~isempty(zeog),
		eogchan = [eogchan,zeog(:)];
        	[h0.REGRESS,s01] = regress_eog(s00(h0.EVENT.POS(ix1):h0.EVENT.POS(ix2),:),chan,eogchan); 
	end; 
	       

	