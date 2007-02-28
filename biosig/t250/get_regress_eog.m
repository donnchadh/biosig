function [h0, s00] = get_regress_eog(fn,Mode)
% GET_REGRESS_EOG tries to obtain the regression coefficients
%    for EOG correction. According to [1], some extra recordings 
%    with large eye movements (i.e. EOG artifacts) are needed. 
%    GET_REGRESS_EOG tries to identify this data. 
% 
%   hdr = get_regress_eog(file)
%   hdr = get_regress_eog(file, Mode)
% 
% INPUT: 
%   file	filename which should be corrected
%		usually, the eye movements are stored in a different file	
%   Mode	'bf-'	beamformer, assume zero-activity reference electrode
%		'bf+'	beamformer, take into account activity of reference electrode 
%		'msec'	modified (without averaging) MSEC method [2]
%		'PCA-k'	removes the k-largest PCA components (k must be a positive integer)
%		'REG'	[default] regression with one or two bipolar EOG channels
%		'REG+PCA' regression and PCA, return 3 "EOG" components
%		'REG+ICA' [not implemented]
%		The following modifiers can be combined with any of the above	
%		'x'  	2nd player of season2 data
%		'f16','+f16'	[default] filter 1-6hz
%		'-f16'	turn filter off 
%
% OUTPUT:
%   hdr.regress.r0 	correction coefficients
% 
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
% [2] Berg P, Scherg M.
%	A multiple source approach to the correction of eye artifacts.
%	Electroencephalogr Clin Neurophysiol. 1994 Mar;90(3):229-41.

%	$Id: get_regress_eog.m,v 1.4 2007-02-28 10:58:57 schloegl Exp $
%	Copyright (C) 2006,2007 by Alois Schloegl 
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% Version 2 of the License, or (at your option) any later version.



	% extract information from BBCI data for EOG correction
	if nargin<3,
		CHAN = 0 ; 
	end; 	
	if nargin<2,
		Mode = ''; 
	end; 	

	[p,f,e] = fileparts(fn); 

	%%%=== search for eye-movement recordings (in order to obtain clean EOG artifacts) ===%%%
	%	an heuristic approach is to support lab-specific standards
	% 	currently, the convention of the GrazBCI and the BBCI Lab are supported
	
	f0 = dir(fullfile(p,['arte*',e])); % BBCI files
	LAB_ID = 'BBCI'; 
	if length(f0)>1,
		% hack to ignore "arte*eog_mono" data
		ix = regexp(strvcat({f0.name}),'eog_mono');
		for k=1:length(ix)
			if ix{k},
				f0(k) = [];
			end;	
		end;		
	end; 

	if 0,
	elseif length(f0)==1,
		feog = fullfile(p,f0.name);
		LAB_ID = 'BBCI'; 
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

	% load eye movement calibration data
        [s00,h0] = sload(feog);

	FLAG.SEASON2DATA_PLAYER2 = 0;
	if strcmp(LAB_ID,'BBCI'),
		if ~isempty(strfind(Mode,'x'))	%%%% this is a hack for the SEASON2DATASET
			FLAG.SEASON2DATA_PLAYER2 = 1;
		end;
		
	        % find eye movements in BBCI recordings

        	tmp = bitand(2^15-1,h0.EVENT.TYP);
		tmp = find((tmp > hex2dec('0430')) & (tmp<=hex2dec('0439')));
		ix1 = min(tmp); 
		ix2 = max(tmp); 

	        % extract segment with large eye movements/EOG artifacts
	        s00 = s00(h0.EVENT.POS(ix1):h0.EVENT.POS(ix2)+h0.EVENT.DUR(ix2),:); 
	        
	        if size(s00,1)<h0.SampleRate*10,
	        	fprintf(2,'WARNING GET_REGRESS_EOG: in %s no eye movements identified\n',h0.FileName);
	        end; 	
	end;

	ix = strfind(Mode,'PCA-');
	FLAG.PCA = ~isempty(ix);
	if FLAG.PCA,
		FLAG.PCA = str2double(strtok(Mode(ix+4:end),' '));
	end;	
	FLAG.MSEC = ~isempty(strfind(upper(Mode),'MSEC'));
	if FLAG.MSEC,
		FLAG.PCA=3;
		FLAG.MSEC3 = ~isempty(strfind(upper(Mode),'MSEC3'));
	end;
	
	% 	filtering the data should improve the estimated        
	FLAG.FILTER_16 = isempty(strfind(Mode,'-f16'));
	if FLAG.FILTER_16,	
	        b = fir1(h0.SampleRate,[1,6]/h0.SampleRate*2); a=1;
        else
		b = 1; a = 1;
	end;
	FLAG.REG = ~isempty(strfind(upper(Mode),'REG'));
        
	%%%%% identify EOG components %%%%%	
	if 0,
	
	elseif ~isempty(strfind(Mode,'bf-'))	
		% beamformer approach
		T = load('EOG_DIPOLS3'); 	% load lead field matrix
		T.Label = T.sa.clab_electrodes;
		T = leadidcodexyz(T);
		V = zeros(h0.NS,3);
		for k=find(h0.LeadIdCode)',
			ix = find(T.LeadIdCode==h0.LeadIdCode(k));
			if length(ix),
				V(k,:) = T.v(ix,4:6);
			end;	
		end;	
		eogchan = pinv(V)';
		if FLAG.SEASON2DATA_PLAYER2,
			eogchan = eogchan([h0.NS/2+1:h0.NS,1:h0.NS/2],:);
		end;
		
	elseif ~isempty(strfind(Mode,'bf+'))	
		% beamformer approach
		T = load('EOG_DIPOLS3');  % load lead field matrix
		T.Label = T.sa.clab_electrodes;
		T = leadidcodexyz(T);
		V = zeros(h0.NS,3);
		for k=find(h0.LeadIdCode)',
			ix = find(T.LeadIdCode==h0.LeadIdCode(k));
			if length(ix),
				V(k,:) = T.v(ix,4:6);
			end;	
		end;	
		R = eye(64);
		R(1:54,1:54)=R(1:54,1:54)-1/54;
		eogchan = [R*pinv(V(1:64,:))';zeros(64,3)];
		if FLAG.SEASON2DATA_PLAYER2,
			eogchan = eogchan([h0.NS/2+1:h0.NS,1:h0.NS/2],:);
		end;

	elseif FLAG.PCA,
		% identify EOG channels
	        eogchan = identify_eog_channels(h0); 

		% identify channels used for PCA
		if isempty(strfind(upper(Mode),'MSEC1'))
			% MSEC,MSEC2 [default]
		        chan = find([(h0.LeadIdCode>=996) & (h0.LeadIdCode<=1302)] | any(eogchan,2));
		else  % MSEC1 
		        chan = find((h0.LeadIdCode>=996) & (h0.LeadIdCode<=1302));
		end;
	        nc      = length(chan); 
	        chan    = sparse(chan,1:nc,1,h0.NS,nc)*(eye(nc) - 1/nc); % Common Average Reference (CAR)
		r       = [chan,eogchan];  % CAR and bipolar EOG
		if FLAG.SEASON2DATA_PLAYER2,
			r = r([h0.NS/2+1:h0.NS,1:h0.NS/2],:);
		end;

		tmp = filter(b,a,center(s00,1)*r);
		tmp = tmp(~any(isnan(tmp),2),:); 
		[u,s,eogchan] = svds(tmp, FLAG.PCA); % get k (i.e. FLAG.PCA) largests PCA's
		eogchan = r*eogchan;
		h0.pca.v= (diag(s).^2)/sum(tmp(:).^2);
		%h0.SVDS.SSE = sum(tmp(:).^2);
		 
	elseif FLAG.REG, %REGRESSION
	        % define xEOG channels
	        FLAG.REG = 1; 
	        if FLAG.SEASON2DATA_PLAYER2,	
		        eogchan = identify_eog_channels(h0,'x'); 
		else        
			eogchan = identify_eog_channels(h0); 
		end;
	        %FLAG.REG = size(eogchan,2); % number of components
	end;        
	end;

       	% remove EOG channels for list of corrected channels
	chan  = 1:h0.NS;
	%chan = find(~any(eogchan,2)); 

        % regression analysis - compute correction coefficients. 
	[h0.REGRESS, s01] = regress_eog(filter(b,a,s00),chan,eogchan); 


	%%%%% post-regression improvement
	if ~isempty(strfind(Mode,'REG+'))	
		% select EEG and EOG channels and do CAR
	        echan = find([(h0.LeadIdCode>=996) & (h0.LeadIdCode<=1302)] | any(eogchan,2));
	        nc    = length(echan); 
	        echan = sparse(echan,1:nc,1,h0.NS,nc)*(eye(nc) - 1/nc); % Common Average Reference (CAR)

		if FLAG.SEASON2DATA_PLAYER2,
			echan = echan([h0.NS/2+1:h0.NS,1:h0.NS/2],:);
		end;

        	tmp = center(s01,1)*echan;
		tmp = tmp(~any(isnan(tmp),2),:); 
		if ~isempty(strfind(Mode,'REG+PCA'))	
			 % get largests PCA's
			[u,s,zeog] = svds(tmp, 3-size(eogchan,2));
			
	        elseif 0, ~isempty(strfind(Mode,'REG+ICA'))	
			%%% ICA: not implemented, yet		
			
		else 	
			fprintf(2,'Mode %s not supported - used regression only\n',Mode); 
			return;
		end; 
		
        	[h0.REGRESS,s01] = regress_eog(filter(b,a,s00),chan,[eogchan,echan*zeog]); 
	end
	
