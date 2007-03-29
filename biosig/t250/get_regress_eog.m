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
%   Mode	'REG'	[default] regression with one or two bipolar EOG channels
%		'REG+CAR' regression and common average reference
%			removes 2 bipolar + averaged monopolar EOG
%		'REG+PCA' regression and PCA, 
%			removes 3 "EOG" components
%		'REG+ICA' regression + ICA [3]
%			removes 3 "EOG" components 
%		'PCA-k'	removes the k-largest PCA components, k must be a positive integer
%		'ICA-k'	removes the k-largest ICA components [3], k must be a positive integer
%		'msec'	same as PCA-3, modified (without averaging) MSEC method [2]
%		'bf-'	beamformer, assume zero-activity reference electrode
%		'bf+'	beamformer, take into account activity of reference electrode 
%	The following modifiers can be combined with any of the above	
%		'FILT###-###Hz  filtering between ### and ### Hz. ### must be numeric
%		'Fs=###Hz'  downsampling to ### Hz, ### must be numeric 
%		'x'  	2nd player of season2 data
%
% OUTPUT:
%   hdr.regress.r0 	correction coefficients
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
% [3] JADE algorithm, Jean-François Cardoso.

%	$Id: get_regress_eog.m,v 1.6 2007-03-29 12:28:09 schloegl Exp $
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

	ix = strfind(Mode,'ICA-');
	FLAG.ICA = ~isempty(ix);
	if FLAG.ICA,
		FLAG.ICA = str2double(strtok(Mode(ix+4:end),' '));
	end;	
	ix = strfind(Mode,'PCA-');
	FLAG.PCA = ~isempty(ix);
	if FLAG.PCA,
		FLAG.PCA = str2double(strtok(Mode(ix+4:end),' '));
	end;	
	if ~isempty(strfind(upper(Mode),'MSEC'));
		FLAG.PCA = 3;
	end;
	
	FLAG.REG = ~isempty(strfind(upper(Mode),'REG'));
        
	% downsampling 
	ix1 = strfind(upper(Mode),'FS=');
	if ~isempty(ix1),
		ix2 = strfind(upper(Mode(ix1:end)),'HZ');
		Fs  = Mode(ix1+3:ix1+ix2-2);
		Fs  = str2double(Fs);
		if length(Fs)==1 & isfinite(Fs),
			s00 = rs(s00,h0.SampleRate,Fs);
			h0.SampleRate = Fs; 
		else
			fprintf(2,'Warning GET_REGRESS_EOG: Fs="%s" could not be decoded. No filter is applied\n',tmp);
		end;	
	end;

	% filter data  
	ix1 = strfind(Mode,'FILT');
	if ~isempty(ix1),
		ix2 = strfind(upper(Mode(ix1:end)),'HZ');
		tmp = Mode(ix1+4:ix1+ix2-2);
		tmp((tmp=='-')|(tmp=='='))=' ';
		B   = str2double(tmp);
		if (length(B)==2) & all(isfinite(B))
			FLAG.Filter = B; 
			tmp = center(s00,1); tmp(isnan(tmp)) = 0;
		        tmp = fft(tmp); f=[0:size(s00,1)-1]*h0.SampleRate/size(s00,1);
		        w   = ((f>B(1)) & (f<B(2))) | ((f>h0.SampleRate-B(2)) & (f<h0.SampleRate-B(1)));
		        tmp(~w,:) = 0;
		        s00 = real(ifft(tmp));
		else
			fprintf(2,'Warning GET_REGRESS_EOG: Filter="%s" could not be decoded. No filter is applied\n',tmp);
		end;			
	end;
		
	%%%%% define EEG and EOG channels %%%%%
	CHANTYP = repmat(' ',h0.NS,1);
	CHANTYP([(h0.LeadIdCode>=996) & (h0.LeadIdCode<=1302)])='E'; % EEG
 	eogchan = identify_eog_channels(h0); 
        CHANTYP(any(eogchan,2)) = 'O';	% EOG
      	if FLAG.SEASON2DATA_PLAYER2,	
	        CHANTYP = CHANTYP([65:128,1:64]);
	        eogchan = eogchan([65:128,1:64],:);
	end;

	%%%%% #### OBSOLETE:START ####
	% 	filtering the data should improve the estimated        
	FLAG.FILTER_FFT16 = ~isempty(strfind(Mode,'fft16'));
	FLAG.FILTER_16 = ~isempty(strfind(Mode,'f16'));
	FLAG.FILTER_FFT0140 = ~isempty(strfind(Mode,'fft0140'));
	FLAG.FILTER_100Hz0140 = ~isempty(strfind(Mode,'100Hz0140'));

	if 0,
	elseif FLAG.FILTER_100Hz0140 | FLAG.FILTER_FFT0140,
		if FLAG.FILTER_100Hz0140,
			s00 = rs(s00,h0.SampleRate,100);
			h0.SampleRate = 100; 
		end;
		tmp = center(s00,1); tmp(isnan(tmp)) = 0;
	        tmp = fft(tmp); f=[0:size(s00,1)-1]*h0.SampleRate/size(s00,1);
	        w = ((f>.1) & (f<40)) | ((f>h0.SampleRate-40) & (f<h0.SampleRate-.1));
	        tmp(~w,:) = 0;
	        s00 = real(ifft(tmp));
	        warning('use of FLAG=0140 is obsolete. Use FILT0.1-40Hz instead.');

	elseif FLAG.FILTER_FFT16,
		tmp = center(s00,1); tmp(isnan(tmp)) = 0;
	        tmp = fft(tmp); f=[0:size(s00,1)-1]*h0.SampleRate/size(s00,1);
	        w = ((f>1) & (f<6)) | ((f>h0.SampleRate-6) & (f<h0.SampleRate-1));
	        tmp(~w,:) = 0;
	        s00 = real(ifft(tmp));
	        warning('use of FLAG=FFT16 is obsolete. Use FILT1-6Hz instead.');

	elseif FLAG.FILTER_16,
	        s00 = filter(b,a,s00);
	        warning('use of FLAG=F16 is obsolete. Use FILT1-6Hz instead.');
	end;        
	%%%%% #### OBSOLETE:END ####

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

	elseif (FLAG.PCA>0) | (FLAG.ICA>0),
		% identify channels used for PCA
		if isempty(strfind(upper(Mode),'MSEC1'))
			% MSEC,MSEC2 [default]
			chan = find((CHANTYP=='E') | (CHANTYP=='O'));
		else  % MSEC1 
			chan = find(CHANTYP=='E');
		end;
	        nc      = length(chan); 
	        chan    = sparse(chan,1:nc,1,h0.NS,nc)*(eye(nc) - 1/nc); % Common Average Reference (CAR)
		r       = [chan,eogchan];  % CAR and bipolar EOG

		tmp = s00*r; 
		tmp = tmp(~any(isnan(tmp),2),:); 
		if FLAG.PCA,
			[u,s,ochan] = svds(tmp, FLAG.PCA); % get k (i.e. FLAG.PCA) largests PCA's
		elseif FLAG.ICA
			[A] = jade(tmp', FLAG.ICA); % get k (i.e. FLAG.PCA) largests PCA's
			ochan = pinv(A)';
		end;
		eogchan = r*ochan;
		 
	elseif FLAG.REG, %REGRESSION
	        % define xEOG channels
	        FLAG.REG = 1; 
		if ~isempty(strfind(upper(Mode),'REG+CAR1'));
	        	eogchan = [eogchan,any(eogchan,2)];
		elseif ~isempty(strfind(upper(Mode),'REG+CAR2'));
			eogchan = [eogchan,double([CHANTYP=='O']/sum(CHANTYP=='O')-[CHANTYP>' ']/sum(CHANTYP>' '))];
		end;
	end;        

       	% remove EOG channels for list of corrected channels
	% chan = 1:h0.NS;
	chan = find(CHANTYP=='E'); 
        % regression analysis - compute correction coefficients. 
	[h0.REGRESS, s01] = regress_eog(s00,chan,eogchan); 
	h0.REGRESS.FLAG = FLAG;	

	%%%%% post-regression improvement
	if length(Mode)>6,
	if ~isempty(strfind(Mode([1:4,6,7]),'REG+CA'))	
		% select EEG and EOG channels and do CAR
		echan = find(CHANTYP>' ');
	        nc    = length(echan); 
	        echan = sparse(echan,1:nc,1,h0.NS,nc)*(eye(nc) - 1/nc); % Common Average Reference (CAR)

		tmp = strtok(Mode)
		nc = str2double(tmp(8:end));
		if isempty(nc) | isnan(nc),
			nc = 1;
		else
		end;		
        	tmp = center(s01,1)*echan;
		tmp = tmp(~any(isnan(tmp),2),:); 
		if ~isempty(strfind(Mode,'REG+PCA'))	
			% get largest PCA's
			[u,s,zeog] = svds(tmp, nc);
	        elseif ~isempty(strfind(Mode,'REG+ICA'))
	        	%%% ICA: identify one component
			[A,s] = jade(tmp',nc);
			zeog  = pinv(A)';
		else 	
			fprintf(2,'Mode %s not supported - regression is used only.\n',Mode); 
			return;
		end; 
		
		w = sparse([eogchan,h0.REGRESS.r0*echan*zeog]); 
		[h0.REGRESS,s01] = regress_eog(s00,chan,w);
	end
	end
	h0.REGRESS.FLAG = FLAG;	

