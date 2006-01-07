function H=plota(X,arg2,arg3,arg4,arg5,arg6,arg7)
% PLOTA plots all kind of data types
%
% PLOTA(X [,Mode]) 
%
% X.datatype determines type of data
%    DATATYPE   Mode
%   'MVAR'      'AutoSpectrum'
%   'MVAR'      'SPECTRUM'
%   'MVAR'      'Phase'
%   'MVAR',	'COHERENCE'
%   'MVAR'      'DTF'  
%   'MVAR'      'PDC'  
%   'TF-MVAR'	Time-frequency MVAR analysis	
%		e.g. plota(X, 'PDC', hf, [,alpha]);	%
%
%   'MEAN+STD'    
%       plota(X,hf,minmean,maxmean,maxstd [,trigger]) 
%       arg1 ... R
%       arg2 ... hf (handles to figures)
%       arg3 ... minmean (minimum of mean)
%       arg4 ... maxmean (maximum of mean)
%       arg5 ... maxstd (maximum of standard deviation)
%       arg6 ... trigger (trigger instant) [optional]
%
%   'HISTOGRAM'	'log'	chansel
%   'HISTOGRAM'	'log+'	chansel
%   'HISTOGRAM'	'log '	chansel
%   'HISTOGRAM'	'lin'	chansel
%
%   'SIESTA_HISTOGRAM'	chansel
%
%   'DBI-EPs'
%   'TSD1'
%   'TSD_BCI7'
%   'MDist-matrix'
%   'MD'
%   'SCATTER'
%   'STAT2'     
%   ''
%   ''
%   'REV' Mode='3D'
%   'REV' Mode='2D'
%
% REFERENCE(S):


%	$Id: plota.m,v 1.45 2006-01-07 23:49:58 schloegl Exp $
%	Copyright (C) 1999-2005 by Alois Schloegl <a.schloegl@ieee.org>
%       This is part of the BIOSIG-toolbox http://biosig.sf.net/

% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 2
% of the  License, or (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

h = [];
if isfield(X,'datatype');
elseif isfield(X,'TYPE');
	if strcmp(X.TYPE,'EVENT')
		ix = find(X.EVENT.TYP==hex2dec('0501'));
		if ~isempty(ix) 
			X.datatype = 'QRS_events';
			X.QRS_event = X.EVENT.POS(ix) / X.EVENT.SampleRate;
			H = X;
		end
	end;
else
	return; 
end;

if 0, 

elseif strcmp(X.datatype,'MVAR'),
	if ~isfield(X,'A') | ~isfield(X,'B'),
		fprintf(2,'Error PLOTA: MVAR missing input data\n');
		return;
	end;
	
	[K1,K2] = size(X.A);
	p = K2/K1-1;
	[K1,K2] = size(X.B);
	q = K2/K1-1;
	if ~isfield(X,'C');
		X.C=ones(K1,K1);
	end;

	if isfield(X,'SampleRate'); 
		Fs = X.SampleRate;
	elseif nargin < 4,
		Fs = 1; pi*2;	
	else
		Fs = arg4;
	end;

	if nargin<2,
		Mode= 'DTF';
		Fs  = 1;
		f   = (0:128)/128*pi;
	else
		Mode = arg2;
	end;

	if nargin<3,
		N=512;
	else
		N=arg3;
	end;

	if all(size(N)==1)	
		f = (0:N)/(2*N)*Fs;
	else
		f = N;
		N = length(N);
	end;

	if isfield(X,'Label'); 
		Label = cellstr(X.Label);
	elseif isfield(X,'ElectrodeName'); 
		if isstruct(X.ElectrodeName);
			Label = X.ElectrodeName;
		else
			for k=1:K1,
				Label{k}=X.ElectrodeName(k,:);
			end;
		end;
	else
		for k=1:K1,
			Label{k}=sprintf('#%02i',k);
		end;
	end;

	[S,h,PDC,COH,DTF,DC,pCOH,dDTF,ffDTF, pCOH2, PDCF, coh]=mvfreqz(X.B,X.A,X.C,f,Fs);

	range = [0,1]; % default range 
	if ~isempty(strfind(Mode,'Auto')),
		if 0, 

		elseif strcmpi(Mode,'AutoSpectrum'),    
			R = abs(S);   
			range = [min(R(:)),max(R(:))];
			range = [1e-8,1e0];
		else 
			error(['unknown MVAR-parameter: ',arg2])    
		end;	
		M = size(S,1); 	      
		K1 = ceil(sqrt(M)); 
		K2 = ceil(M/K1); 
		for k1=1:M;
			subplot(K1,K2,k1);
			semilogy(f,squeeze(R(k1,k1,:)));	
			axis([0,max(f),range]);
			ylabel(Label{k1});
		end;

	else
		if 0, 

		elseif strcmpi(Mode,'Spectrum') | strcmpi(Mode,'logS'),    
			R = abs(S);   
			range = [min(R(:)),max(R(:))];
	    		range(1) = min(range(1),range(2)/100);
			%range=[1e-2,1e2];			
		elseif strcmpi(Mode,'Phase') | strcmpi(Mode,'phaseS') ,    
			R = zeros(size(S));   
		   	for k1=1:K1;
		    	for k2=1:K2;
				R(k1,k2,:) = unwrap(squeeze(angle(S(k1,k2,:))))*180/pi;	
			end;
			end;
			range = [-180,180]*2;	
			range = [min(R(:)),max(R(:))];
		elseif strcmpi(Mode,'Phase') | strcmpi(Mode,'phaseh') ,    
			R = zeros(size(h));   
		   	for k1=1:K1;
		    	for k2=1:K2;
				R(k1,k2,:) = unwrap(squeeze(angle(h(k1,k2,:))))*180/pi;	
			end;
			end;
			range = [-180,180]*2;	
			range = [min(R(:)),max(R(:))];
		elseif strcmpi(Mode,'PDC'),    
			R = PDC;   
		elseif strcmpi(Mode,'Coherence') | strcmpi(Mode,'COH'),    
			R = abs(COH); 
		elseif strcmpi(Mode,'iCOH'),    
			R = imag(COH); 
			range = [-1,1];	
		elseif strcmpi(Mode,'pCOH'),    
			R = abs(pCOH); 
		elseif strcmpi(Mode,'pCOH2'),    
			R = abs(pCOH2); 
		elseif strcmpi(Mode,'imagCOH'),    
			R = imag(COH); 
		elseif strcmpi(Mode,'coh'),    
			R = abs(coh); 
		elseif strcmpi(Mode,'icoh'),    
			R = imag(coh); 
			range = [-1,1];	
		elseif strcmpi(Mode,'PDC'),    
			R = PDC; 
		elseif strcmpi(Mode,'PDCF'),    
			R = PDCF; 
		elseif strcmpi(Mode,'DTF'),    
			R = DTF; 
		elseif strcmpi(Mode,'dDTF'),    
			R = dDTF; 
		elseif strcmpi(Mode,'ffDTF'),
			R = ffDTF; 
		elseif strcmpi(Mode,'DCF1'),
			R = S;
			for k1=1:K1,
			for k2=1:K1,
				R(k1,k2,:) = sqrt(X.C(k2,k2)/(2*pi*Fs))*abs(h(k1,k2,:))./sqrt(S(k1,k1,:));
			end;	
			end;	
			range = [0,1];	
%			range = [min(R(:)),max(R(:))];
		elseif strcmpi(Mode,'DCF2'),
			R = S;
			for k1=1:K1,
			for k2=1:K1,
				R(k1,k2,:) = abs(h(k1,k2,:))./sqrt(S(k1,k1,:));
			end;	
			end;	
			range = [0,1];	
		elseif strcmpi(Mode,'DCF'),
			tmp = S;
			for k=1:K1,
				tmp(k,:,:) = tmp(k,repmat(k,1,K1),:);
			end;	
			R = h./sqrt(tmp); 
			range = [-1,1];	
%			range = [min(R(:)),max(R(:))];
		else 
			error(['unknown MVAR-parameter: ',arg2])    
		end;
			      
		for k1=1:K1;
			for k2=1:K2;
				subplot(K1,K2,k2+(k1-1)*K1);
				if strcmpi(Mode,'Spectrum') | strcmpi(Mode,'logS'),    
			    		semilogy(f,squeeze(R(k1,k2,:)));       
				else
					area(f,squeeze(R(k1,k2,:)));	
				end;
				axis([0,max(f),range]);
				if k2==1;
					ylabel(Label{k1});
				end;
				if k1==1;
					title(Label{k2});
				end;
			end;
		end;
	end;
	if exist('suptitle','file')
		suptitle(Mode);
	end;

		
elseif 0, strcmp(X.datatype,'TF-MVAR') & (nargin>1) %& ~any(strmatch(arg2,{'S1','logS1',})),
	
	%GF = {'C','DC','AR','PDC','DTF','dDTF','ffDTF','COH','pCOH','pCOH2','S','h','phaseS','phaseh','coh','logh','logS'};
	
	if nargin<2,
		arg2 = 'S1';
	end;
	if nargin<4,
		alpha = 1; 
	elseif isnumeric(arg4),
		alpha = arg4;
	elseif ischar(arg4) & isempty(str2num(arg4))
		alpha = flag_implicit_significance;
	else
		alpha = arg4;
	end;
	if nargin<5,
		Y = [];
	else
		Y = arg5; 
	end;
	     
	gf = arg2;
	if ~isfield(X.M,gf)
		error('PLOTA TFAR_ALL: field %s is unknown\n',gf);
	end;
	
	%ClassList = {'left','right','foot','tongue'};
	
	M   = size(X.M.AR,1);
	tmp = size(X.M.AR);
	MOP = tmp(2)/tmp(1);
	
	if ~isfield(X,'Label'),
		for k1 = 1:M,
			Label{k1}=['# ',int2str(k1)];
		end;
	end;
	nr = ceil(sqrt(M));
	nc = ceil(M/nr);
	if (nargin>2) & ~isempty(arg3),
		hf = arg3;
	else
		for k = 1:M,
			hf(k)=subplot(nr,nc,k);
		end;
	end;

	if isempty(Y);
		x0 = real(getfield(X.M,gf));
	else
		x0 = (real(getfield(X.M,gf)) - real(getfield(Y.M,gf)))./(real(getfield(X.SE,gf))*X.N + real(getfield(Y.SE,gf))*Y.N);
	end;

	
	clim = [min(x0(:)),max(x0(:))]
	caxis(clim);
	cm = colormap;
	for k = 1:M,
		subplot(hf(k));
		%imagesc(X.T,X.F,squeeze(X.M.logS1(k,:,:)))
	
		x = x0(k,1:length(X.F),:);
		ci = getfield(X.SE,gf)*(X.N-1);
		ci = ci(k,1:length(X.F),:);
		if 1,%alpha < .5,
			xc = 2 + round(62*(squeeze(x)-clim(1))/diff(clim));
			sz = size(x);
			%x = x(:);
			bf = prod(size(x));
			%xc(abs(x) < (ci*norminv(1-alpha/(2*bf))))  = 1;
			xc(abs(x) < (ci*tinv(1-alpha/(2*bf),X.N))) = 1;
			%x(abs(x) < .5) = NaN;
			%x = reshape(x,sz);
			cm(1,:) = [1,1,1];
			colormap(cm);
		else
			xc = 1+round(63*(squeeze(x)-clim(1))/diff(clim));
			colormap('default');
		end;

		x1 = reshape(cm(xc,1),size(xc));
		x2 = reshape(cm(xc,2),size(xc));
		x3 = reshape(cm(xc,3),size(xc));
		
		%h = imagesc(X.T,X.F,cat(3,x1,x2,x3)*diff(clim)+clim(1),clim);
		%imagesc(X.T,X.F,squeeze(X.M.logS1(k,:,:)))
		h = imagesc(X.T,X.F,cat(3,x1,x2,x3),clim);
	end;
	if isfield(X,'TITLE');	
		TIT = X.TITLE;
		TIT(TIT=='_')=' ';
		if exist('suptitle','file')
			suptitle(TIT);
		end;
	else
		TIT = '';
	end
	
	
elseif strncmp(X.datatype,'TF-MVAR',7)    % logS2 and S1 
	
	%GF = {'C','DC','AR','PDC','DTF','dDTF','ffDTF','COH','pCOH','pCOH2','S','h','phaseS','phaseh','coh','logh','logS'};

	if nargin<2,
		arg2 = 'logS1';
	end;
	if nargin<3,
		alpha = .01; 
	elseif isnumeric(arg3),
		alpha = arg3;
	elseif isempty(str2num(arg3))
		alpha = flag_implicit_significance;
	else
		alpha = arg3;
	end;
	if nargin<4,
		Y = [];
	else
		Y = arg4; 
	end;

	gf = arg2;
	AUTO = ~isempty(strfind(lower(gf),'auto'));
	if AUTO,
		gf = strrep(gf,'Auto','');
	end;	
	GF = strtok(gf);
	if ~isfield(X.M,GF)
		warning('PLOTA TFMVAR_ALL: field %s is unknown\n',GF);
	end;
	MONO = strcmp(GF,'logS1') | strcmp(GF,'S1');
	if strcmp(GF,'AR1') | strcmp(GF,'C1');
		MONO=1;
		f = 1:size(getfield(X.M,GF),2);
	else
		f = X.F; 	
	end;	

	fidx = repmat(logical(1),size(f)); 	% selection of frequency band 
	nix = 1:length(X.T);
	if 1, 
		% ERD MVAR chapter 
		warning('use project-specific setting: f<45Hz, t<7.0s')
		fidx = (f<=45)&(f>0); 	% selection of frequency band 
		X.T(X.T>=7.0) = [];	% selection of segments (in time)
		nix = 2:length(X.T); 	% hack to remove reference segment
	end;
	
	
	%ClassList = {'left','right','foot','tongue'};
	
	M   = size(X.M.AR,1);
	tmp = size(X.M.AR);
	MOP = tmp(2)/tmp(1);
	
	if ~isfield(X,'Label'),
		for k1 = 1:M,
			Label{k1}=['# ',int2str(k1)];
		end;
	else 
		Label = X.Label; 
	end;
	nr = ceil(sqrt(M));
	nc = ceil(M/nr);
	if 0, nargin>2,
		hf = arg3;
	elseif AUTO | MONO, 
		NR = ceil(sqrt(M)); 
		NC = ceil(M/NR); 
		for k1 = 1:M,
			hf(k1) = subplot(NR,NC,k1);
		end;		
	elseif M*M<200,
		for k1 = 1:M,
			for k2 = 1:M,
%				hf(k1,k2)=subplot(M,M,(k2-1)*M+k1);	%transposed 
			end;
		end;
	else	
		subplot(111); 
	end;

	if strcmp(X.datatype,'TF-MVAR') 
%		warning('TF-MVAR 1.0 has');
		Xnormfactor = X.N;
	else
		Xnormfactor = 1; 
	end; 

	if ~isempty(strfind(gf,'eventrelated'))
		[tmp]  = str2double(gf); 
		[gf,r] = strtok(gf);
		[t2,r] = strtok(r);
		[t3,r] = strtok(r);

		ix  = min(find(X.T==max(tmp)));
		nix = [nix(1):ix-1,ix+1:length(X.T)];
		rix = repmat(ix,size(nix));
		
		m   = real(getfield(X.M,gf));
		se  = real(getfield(X.SE,gf))*Xnormfactor;
		if MONO,
			x0  = m(:,fidx,nix) - m(:,fidx,rix);
			ci0 = sqrt(se(:,fidx,nix).^2 + se(:,fidx,rix).^2);
		else
			x0  = m(:,:,fidx,nix) - m(:,:,fidx,rix);
			ci0 = sqrt(se(:,:,fidx,nix).^2 + se(:,:,fidx,rix).^2);
		end;
		X.T = X.T(nix); 
		
	elseif isempty(Y);
		X.T = X.T(nix); 
		x0  = real(getfield(X.M,gf));
		ci0 = getfield(X.SE,gf)*Xnormfactor;
		if MONO,
			x0  = x0(:,fidx,nix);
			ci0 = ci0(:,fidx,nix);
		else	
			x0  = x0(:,:,fidx,nix);
			ci0 = ci0(:,:,fidx,nix);
		end;

	else
		X.T = X.T(nix); 
		x0  = (real(getfield(X.M,gf)) - real(getfield(Y.M,gf))); 
		ci0 = sqrt((real(getfield(X.SE,gf))*Xnormfactor).^2 + (real(getfield(Y.SE,gf))*Ynormfactor).^2);

		x0  = x0(:,:,fidx,nix);
		ci0 = ci0(:,:,fidx,nix);
	end;

	if strcmp(gf,'DC') | strcmp(gf,'C')
		clim = [min(x0(:)),max(x0(:))];
		caxis(clim);
		cm = colormap;
		for k1 = 1:M,
			for k2 = 1:M,
				subplot(hf(k1*M-M+k2));
				x  = x0(k1,k2,1,:);
				ci = ci0(k1,k2,1,:);
				if 1,
				elseif alpha < .5,
					xc = 2 + round(62*(squeeze(x)-clim(1))/diff(clim));
					sz = size(x);
					%x = x(:);
					bf = prod(size(x));
					%xc(abs(x) < (ci*norminv(1-alpha/(2*bf)))) = 1;
					x(abs(x) < (ci*tinv(1-alpha,X.N))) = NaN;
					%x(abs(x) < .5) = NaN;
					%x = reshape(x,sz);
					cm(1,:) = [1,1,1];
					colormap(cm);
				else
					xc = 1+round(63*(squeeze(x)-clim(1))/diff(clim));
					colormap('default');
				end;
				
				%h = semilogy(X.T,[x(:),ci(:)]*[1,1,1;0,-1,1]);
				h = plot(X.T,[x(:),ci(:)]*[1,1,1;0,-1,1]);
				set(h(1),'color',[0,0,1]);
				set(h(2),'color',[0.5,0.5,1]);
				set(h(3),'color',[0.5,0.5,1]);
				v  = axis; v(1)=min(X.T);v(2)=max(X.T);axis(v);
				axis([min(X.T),max(X.T),clim])
				%h  = imagesc(X.T,X.F,squeeze(x),clim);
				if k2==1, title(Label{k1}); end;
				if k1==1, ylabel(Label{k2});end;
			end;
		end;

	elseif AUTO | MONO,
		clim = [min(x0(:)),max(x0(:))];
		caxis(clim);
		cm = colormap;

		for k1 = 1:M,
				subplot(hf(k1));
				if MONO,
					x  = x0(k1,:,:);
					ci = ci0(k1,:,:);
				else	
					x  = x0(k1,k1,1:length(X.F),:);
					ci = ci0(k1,k1,1:length(X.F),:);
				end;
				if alpha < .5,
					xc = 2 + round(62*(squeeze(x)-clim(1))/diff(clim));
					sz = size(x);
					%x = x(:);
					bf = prod(size(x));
					%xc(abs(x) < (ci*norminv(1-alpha/(2*bf)))) = 1;
					xc(abs(x) < (ci*tinv(1-alpha,X.N))) = 1;
					%x(abs(x) < .5) = NaN;
					%x = reshape(x,sz);
					cm(1,:) = [1,1,1];
					colormap(cm);
				else
					xc = 1+round(63*(squeeze(x)-clim(1))/diff(clim));
					colormap('default');
				end;
				x1 = reshape(cm(xc,1),size(xc));
				x2 = reshape(cm(xc,2),size(xc));
				x3 = reshape(cm(xc,3),size(xc));
				
				%h = imagesc(X.T,X.F,cat(3,x1,x2,x3)*diff(clim)+clim(1),clim);
				h = imagesc(X.T,f(fidx),cat(3,x1,x2,x3),clim);
				%h  = imagesc(X.T,X.F,squeeze(x),clim);
				ylabel(Label{k1});
		end;	
	elseif M*M<200,
		clim = [min(x0(:)),max(x0(:))];
		caxis(clim);
		cm = colormap;
		for k1 = 1:M,
			for k2 = 1:M,
%				subplot(hf(k1*M-M+k2));		% display transposed because of predefined hf above
				subplot(M,M,(k1-1)*M+k2);	% display transposed
%				subplot(M,M,(k2-1)*M+k1);
				x  = x0(k1,k2,:,:);
				ci = ci0(k1,k2,:,:);
				if alpha < .5,
					xc = 2 + round(62*(squeeze(x)-clim(1))/diff(clim));
					sz = size(x);
					%x = x(:);
					bf = prod(size(x));
					%xc(abs(x) < (ci*norminv(1-alpha/(2*bf)))) = 1;
					xc(abs(x) < (ci*tinv(1-alpha,X.N))) = 1;
					%x(abs(x) < .5) = NaN;
					%x = reshape(x,sz);
					cm(1,:) = [1,1,1];
					colormap(cm);
				else
					xc = 1+round(63*(squeeze(x)-clim(1))/diff(clim));
					colormap('default');
				end;
				x1 = reshape(cm(xc,1),size(xc));
				x2 = reshape(cm(xc,2),size(xc));
				x3 = reshape(cm(xc,3),size(xc));
				
				%h = imagesc(X.T,X.F,cat(3,x1,x2,x3)*diff(clim)+clim(1),clim);
				h = imagesc(X.T,X.F(fidx),cat(3,x1,x2,x3),clim);
				%h  = imagesc(X.T,X.F,squeeze(x),clim);
				if k1==1, title(Label{k2}); end;
				if k2==1, ylabel(Label{k1});end;
			end;
		end;
	else
		sz = size(x0); 
		x  = reshape(permute(x0 ,[3,1,4,2]),[prod(sz([1,3])),prod(sz([2,4]))]);
		ci = reshape(permute(ci0,[3,1,4,2]),[prod(sz([1,3])),prod(sz([2,4]))]);
		% transposed 
%		x  = reshape(permute(x0 ,[3,2,4,1]),[prod(sz([2,3])),prod(sz([1,4]))]);
%		ci = reshape(permute(ci0,[3,2,4,1]),[prod(sz([2,3])),prod(sz([1,4]))]);

		clim = [min(x(:)),max(x(:))];
		caxis(clim);
		cm = colormap;
		for k1 = 1,%:M,
			for k2 = 1,%:M,
				%subplot(hf(k1*M-M+k2));
				%x  = x0(k1,k2,fidx,:);
				%ci = ci0(k1,k2,fidx,:);
				if alpha < .5,
					xc = 2 + round(62*(squeeze(x)-clim(1))/diff(clim));
					%x = x(:);
					bf = prod(size(x));
					%xc(abs(x) < (ci*norminv(1-alpha/(2*bf)))) = 1;
					xc(abs(x) < (ci*tinv(1-alpha,X.N))) = 1;
					%x(abs(x) < .5) = NaN;
					cm(1,:) = [1,1,1];
					colormap(cm);
				else
					xc = 1+round(63*(squeeze(x)-clim(1))/diff(clim));
					colormap('default');
				end;
				x1 = reshape(cm(xc,1),size(xc));
				x2 = reshape(cm(xc,2),size(xc));
				x3 = reshape(cm(xc,3),size(xc));
				
				%h = imagesc(X.T,X.F,cat(3,x1,x2,x3)*diff(clim)+clim(1),clim);
%				h = imagesc(X.T,X.F(fidx),cat(3,x1,x2,x3),clim);
				h = imagesc(repmat(X.T,1,sz(1)),repmat(X.F(fidx),1,sz(2)),cat(3,x1,x2,x3),clim);
%				h = imagesc(cat(3,x1,x2,x3),clim);

%				set(gca,'XTick',.5+(0:sz(3)-1)*sz(1));
%				set(gca,'YTick',.5+(0:sz(4)-1)*sz(2));
%				set(gca,'XTickMinor',1:sz(3)*sz(1));
%				set(gca,'YTickMinor',1:sz(4)*sz(2));
%				set(gca,'XGrid','ON','XMinorTick','ON');
%				set(gca,'YGrid','ON','YMinorTick','ON');

				%h  = imagesc(X.T,X.F,squeeze(x),clim);
				if k1==1, title(Label{k2}); end;
				if k2==1, ylabel(Label{k1});end;
			end;
		end;
	end;
	%caxis = clim;
	%h   = colorbar;
	%tmp = get(h,'ytick')'/64*diff(clim)+clim(1);
	%set(h,'yticklabel',num2str(tmp));
	
	if isfield(X,'TITLE');	
		TIT = X.TITLE;
		TIT(TIT=='_')=' ';
		if exist('suptitle','file')
			suptitle(TIT);
		end;
	else
		TIT = gf; 
		if ~isempty(strfind(gf,'eventrelated'))
			TIT = ['er',TIT];
		end;
	end
	
	
elseif strcmp(X.datatype,'EDF'),
	data = arg2;
	[nr,nc]=size(data);
	for k = 1:nc,
		subplot(nc,1,k);
		%t = (X.Block.number(3)+1:X.Block.number(4))/X.AS.MAXSPR*X.Dur;
		
		t = X.FILE.POS*X.Dur+(1-nr:0)/X.AS.MAXSPR*X.Dur/size(X.SIE.T,2)*size(X.SIE.T,1);
		plot(t, data(:,k));
		ylabel(deblank(X.Label(X.SIE.ChanSelect(k),:)));    
	end;
	xlabel('t [s]')
	
	
elseif strcmp(X.datatype,'confusion'),
	if nargin>1,
		[kap,sd,H,z,OA,SA,MI]=kappa(X.data);
		fprintf(1,'%s\n',repmat('-',1,8*(size(X.data,1)+1)));
		fprintf(1,'Kappa = %5.3f %c %4.3f(%s)\tOverall Accuracy = %4.1f%%\n',kap,177,sd,repmat('*',sum(-z<norminv([.05,.01,.001]/2)),1),OA*100);
		%disp([X.data,sum(X.data,2);sum(X.data,1),sum(X.data(:))])       
		
		fprintf(1,'%s\n',repmat('-',1,8*(size(X.data,1)+1)));
		for k=1:size(X.data,1),
			fprintf(1,'%4.1f%%\t',X.data(k,:)./sum(X.data,1)*100);
			fprintf(1,'| %4.0f\n',sum(X.data(k,:),2));
		end;
		fprintf(1,'%s\n',repmat('-',1,8*(size(X.data,1)+1)));
		fprintf(1,'%4.0f\t',sum(X.data,1));
		fprintf(1,'| %4.0f\n\n',sum(X.data(:)));
	else
		[kap,sd,H,z,OA,SA,MI]=kappa(X.data);
		fprintf(1,'%s\n',repmat('-',1,8*(size(X.data,1)+2)));
		fprintf(1,'Kappa = %5.3f %c %4.3f(%s)\tOverall Accuracy = %4.1f%%\n',kap,177,sd,repmat('*',sum(-z<norminv([.05,.01,.001]/2)),1),OA*100);
		%disp([X.data,sum(X.data,2);sum(X.data,1),sum(X.data(:))])       
		fprintf(1,'%s\n',repmat('-',1,8*(size(X.data,1)+2)));
		
		for k=1:size(X.data,1),
			fprintf(1,'%6.1f\t',X.data(k,:));
			fprintf(1,'|%6.1f\t| %4.1f%%\n',sum(X.data(k,:),2),X.data(k,k)/sum(X.data(k,:),2)*100);
		end;
		fprintf(1,'%s\n',repmat('-',1,8*(size(X.data,1)+2)));
		fprintf(1,'%6.0f\t',sum(X.data,1));
		fprintf(1,'|%6.0f\t|\n',sum(X.data(:)));
		fprintf(1,'%s\n',repmat('-',1,8*(size(X.data,1)+1)));
		fprintf(1,'%5.1f%%\t',diag(X.data)'./sum(X.data,1)*100);
		fprintf(1,'|\n\n');
		
	end;
	if isfield(X,'Label'),
		fprintf(1,'%s\nStage:\t\t',repmat('-',1,8*(size(X.data,1)+2)));
		for k = 1:length(X.Label)
			fprintf(1,'%-7s\t',X.Label{k});
		end;
	end;
	fprintf(1,'\nSpec.Acc:\t');
	fprintf(1,'%4.1f%%\t',SA*100);
	fprintf(1,'\n%s\n',repmat('-',1,8*(size(X.data,1)+2)));
	

elseif strcmpi(X.datatype,'pfurtscheller_spectral_difference'),
	nc = ceil(sqrt(X.NS));
	nr = ceil(X.NS/nc);
	nch = size(X.AR,1)/X.NS;
	f = (0:.1:X.SampleRate/2)';
	H = zeros(length(f),X.NC+1);
	for k1=1:nc,
		for k2=1:nr,
			c = k1+(k2-1)*nc;
			if nargin>1,
				H = X.S(:,c+X.NS*(0:X.NC));	
				F = 0:size(X.S,1)-1;
			else
				for k3 = 1:X.NC+1;
					ix = c + X.NS*(k3-1);
					[H(:,k3), F] = freqz(sqrt(X.PE(ix,end)/X.SampleRate),ar2poly(X.AR(ix,:)),f,X.SampleRate);
				end
			end;
			subplot(nc,nr,c);
			semilogy(F,abs(H),'-');
			legend({'ref','1','2'});
			ylabel(sprintf('%s/[%s]^{1/2}',X.PhysDim,X.samplerate_units));
			v=axis;v(2:4)=[max(F),1e-2,10];axis(v);
			%hold on;
			grid on;
			if isfield(X,'Label');
				if iscell(X.Label)
					title(X.Label{c});
				else
					title(X.Label(c,:));
				end;
			else
				title(['channel # ',int2str(c)]);
			end;
		end
	end;
	

elseif strcmpi(X.datatype,'spectrum') | strcmp(X.datatype,'qualitycontrol'),

	if nargin>1,
		Mode=arg2;
	else
		Mode='log';
	end;
	
	if strcmp(X.datatype,'qualitycontrol'),
		fprintf(1,'\n  [%s]',X.PhysDim(1,:));
		fprintf(1,'\t#%02i',1:size(X.AR,1));
		fprintf(1,'\nMEAN  ');
		fprintf(1,'\t%+7.3f',X.MEAN);
		fprintf(1,'\nRMS');
		fprintf(1,'\t%+7.3f',X.RMS);
		fprintf(1,'\nSTD');
		fprintf(1,'\t%+7.3f',X.STD);
		fprintf(1,'\nQuant');
		fprintf(1,'\t%+7.3f',X.QUANT);
		fprintf(1,'\n  [bit]\nEntropy');
		fprintf(1,'\t%+4.1f',X.ENTROPY);
		fprintf(1,'\n\n');
	end;

	if ~isfield(X,'samplerate_units')
		X.samplerate_units = 'Hz';    
	end;
	if ~isfield(X,'PhysDim')
		X.PhysDim = '[1]';    
	end;
	if ~isfield(X,'QUANT')
		X.QUANT = 0;    
	end;
	if ~isfield(X,'Impedance')
		X.Impedance=5000; %5kOHM
	end;
	
	if isfield(X,'Label')
		Label = cellstr(X.Label); 
	else
		NS = size(X.AR,1);
		Q.Label = [repmat('#',NS,1),int2str([1:NS]')];
	end;	
	
	[n,p] = size(X.AR);
	H=[]; F=[];
	for k=1:size(X.AR,1);
		[h,f] = freqz(sqrt(X.PE(k,size(X.AR,2)+1)/(X.SampleRate*2*pi)),ar2poly(X.AR(k,:)),(0:64*p)/(128*p)*X.SampleRate',X.SampleRate);
		H(:,k)=h(:);F(:,k)=f(:);
	end;
	if strcmp(lower(Mode),'log')
		h=semilogy(F,abs(H),'-',[0,X.SampleRate/2]',[1;1]*mean(X.QUANT)/sqrt(12*X.SampleRate),'k:',[0,X.SampleRate/2]',1e6*[1;1]*sqrt(4*310*138e-25*X.Impedance),'k');
		ylabel(sprintf('%s/[%s]^{1/2}',X.PhysDim(1,:),X.samplerate_units));
		Label = [Label;{'Quantization'};{'Impedance'}]; 

	elseif strcmp(lower(Mode),'log2')
		semilogy(F,real(H).^2+imag(H).^2,'-',[0,X.SampleRate/2]',[1;1]*X.QUANT.^2/(12*X.SampleRate),'k:');
		ylabel(sprintf('[%s]^2/%s',X.PhysDim(1,:),X.samplerate_units));
		Label = [Label;{'Quantization'}]; 
		
	elseif strcmp(lower(Mode),'lin')
		plot(F,abs(H),'-',[0,X.SampleRate/2]',[1;1]*X.QUANT/sqrt(12*X.SampleRate),'k:');
		ylabel(sprintf('%s/[%s]^{1/2}',X.PhysDim(1,:),X.samplerate_units));
		Label = [Label;{'Quantization'}]; 
		
	elseif strcmp(lower(Mode),'lin2')
		plot(F,real(H).^2+imag(H).^2,'-',[0,X.SampleRate/2]',[1;1]*X.QUANT.^2/(12*X.SampleRate),'k:');
		ylabel(sprintf('[%s]^2/%s',X.PhysDim(1,:),X.samplerate_units));
		Label = [Label;{'Quantization'}]; 
	end;
	xlabel(sprintf('f [%s]',X.samplerate_units));
	if isfield(X,'Title'), title(X.Title);
	elseif isfield(X,'FileName'); tmp=X.FileName; tmp(tmp=='_')=' '; title(tmp); end
	H = X;
	legend(Label); 
	
elseif strcmp(X.datatype,'SIESTA_HISTOGRAM')
	if nargin<2,
		chansel=0;
	else
		chansel=arg2;
	end;
	
	cname=computer;
	if cname(1:2)=='PC',
		PFAD='s:/';
	else
		PFAD='/home/schloegl/';	
	end;
	
	H = load([PFAD,'siesta/t300/',lower(X.filename),'his.mat']);
	R = load([PFAD,'siesta/t300/',lower(X.filename),'res.mat']);
	
	fn=[PFAD,'siesta/t900/',lower(X.filename),'th.mat'];
	if exist(fn)==2,
		T = load(fn);
	else
		fprintf(2,'Warning: no thresholds available for %s\n',X.filename);
		T = [];    
	end;
	
	H.X = [ones(2^16,1),repmat((-2^15:2^15-1)',1,R.EDF.NS)]*R.EDF.Calib;
	if chansel>0,
		H.H = H.HISTOG(:,chansel);
	else
		H.H = H.HISTOG;
		chansel = 1:R.EDF.NS;
	end;
	H.datatype = 'HISTOGRAM';
	H.N = full(sum(H.H,1));
	
	if ~isempty(T),
		if any(T.TRESHOLD>=2^15),
			T.TRESHOLD=T.TRESHOLD-2^15-1;
			fprintf(2,'Thresholds in %s were by 2^15+1 to high: corrected\n',X.filename);
		end;
		
		%T.TRESHOLD(:,1)=max(T.TRESHOLD(:,1),R.RES.MU'-7*sqrt(R.RES.SD2'));
		%T.TRESHOLD(:,2)=min(T.TRESHOLD(:,2),R.RES.MU'+7*sqrt(R.RES.SD2'));
	else
		%T.TRESHOLD = ones(R.EDF.NS,1)*[-2^15,2^15-1]; %repmat(nan,R.EDF.NS,2);
		T.TRESHOLD = repmat([2^15-1,-2^15],R.EDF.NS,1)';
		H.Threshold = [ones(2,1),T.TRESHOLD']*R.EDF.Calib(:,chansel);
	end;
	
	plota(H,'log+');
	suptitle(X.filename);
	
elseif strcmp(X.datatype,'HISTOGRAM')
	if nargin<3,
		chansel=0;
	else
		chansel=arg3;
	end;
	if nargin<2 
		yscale='lin '; 
	else
		yscale=arg2;
	end;
	
	if ~isfield(X,'N');
		X.N = full(sum(X.H,1));
	end;
	
	if chansel<=0, 
		chansel = 1:size(X.H,2);
	end;
	
	N=ceil(sqrt(size(X.H,2)));
	for K = chansel; 
		%min(K,size(X.X,2))
		t = X.X(:,min(K,size(X.X,2)));
		%HISTO=hist2pdf(HISTO);
		h = X.H(:,K);   
		
		mu = (t'*h)/X.N(K);%sumskipnan(repmat(t,size(h)./size(t)).*h,1)./sumskipnan(h,1);
		x  = t-mu; %(repmat(t,size(h)./size(t))-repmat(mu,size(h)./size(mu)));
		sd2= sumskipnan(x.*x.*h,1)./X.N(K);
		
		[tmp,tmp2]=find(h>0);
		
		if isfield(X,'Threshold'),
			MaxMin=X.Threshold(:,K)';	
			MaxMin=[max(MaxMin),min(MaxMin)];
		else
			MaxMin=t([max(tmp) min(tmp)]);
		end;
		
		if strcmp(yscale,'lin '),
			subplot(ceil(size(X.H,2)/N),N,K);
			plot(t,[h],'-');
		elseif strcmp(yscale,'lin+'),
			subplot(ceil(size(X.H,2)/N),N,K);
			tmp = diff(t);
			dT  = 1;min(tmp(tmp>0));
			tmp=max(h)/2;
			tmp=sum(h)/sqrt(2*pi*sd2)*dT/2;
			%plot(t,[h],'-',t,exp(-(t-mu).^2./sd2/2)./sqrt(2*pi*sd2).*sum(h),'c',mu+sqrt(sd2)*[-5 -3 -1 0 1 3 5],tmp*ones(7,1),'+-',MaxMin,tmp,'rx' );
			plot(t,[h]+.01,'-',t,exp(-((t-mu).^2)/(sd2*2))/sqrt(2*pi*sd2)*sum(h)*dT,'c',mu+sqrt(sd2)*[-5 -3 -1 0 1 3 5],tmp*ones(7,1),'+-',MaxMin,tmp,'rx');
			v=axis; v=[MaxMin(2) MaxMin(1) 1 max(h)]; axis(v);
		elseif strcmp(yscale,'log ') | strcmp(yscale,'log'),
			subplot(ceil(size(X.H,2)/N),N,K);
			tmp = diff(t);
			dT  = min(tmp(tmp>0));
			tmp = sqrt(sum(h)/sqrt(2*pi*sd2)*dT);
			%semilogy(t,[h],'-')
			semilogy(t,[h+.01,exp(-((t-mu).^2)/(sd2*2))/sqrt(2*pi*sd2)*sum(h)*dT]);
		elseif strcmp(yscale,'log+'),
			subplot(ceil(size(X.H,2)/N),N,K);
			tmp = diff(t);
			dT  = min(tmp(tmp>0));
			tmp = sqrt(sum(h)/sqrt(2*pi*sd2)*dT);
			%semilogy(t,[h]+.01,'-',t,exp(-(t*ones(size(mu))-ones(size(t))*mu).^2./(ones(size(t))*sd2)/2)./(ones(size(t))*(sqrt(2*pi*sd2)./sum(h))),'c',mu+sqrt(sd2)*[-5 -3 -1 0 1 3 5],tmp*ones(7,1),'+-',MaxMin,tmp,'rx');
			%semilogy(t,[h]+.01,'-',t,exp(-(t(:,ones(size(mu)))-mu(ones(size(t)),:)).^2./sd2(ones(size(t)),:)/2)./sqrt(2*pi*sd2(ones(size(t)),:)).*(ones(size(t))*sum(h)),'c',mu+sqrt(sd2)*[-5 -3 -1 0 1 3 5],tmp*ones(7,1),'+-',MaxMin,tmp,'rx');
			%semilogy(t,[h]+.01,'-',t,exp(-((t-mu).^2)/(sd2*2))/sqrt(2*pi*sd2)*sum(h)*dT,'c',mu+sqrt(sd2)*[-5 -3 -1 0 1 3 5]',tmp*ones(7,1),'+-',MaxMin,tmp,'rx');
			semilogy(t,[h+.01,exp(-((t-mu).^2)/(sd2*2))/sqrt(2*pi*sd2)*sum(h)*dT],'-',mu+sqrt(sd2)*[-5 -3 -1 0 1 3 5]',tmp*ones(7,1),'+-',MaxMin,tmp,'rx');
			v=axis; v=[MaxMin(2)+0.1*diff(MaxMin) MaxMin(1)-0.1*diff(MaxMin) 1 max(h)]; axis(v);
			%v=axis; v=[v(1:2) 1 max(h)]; axis(v);
		elseif strcmp(yscale,'qq'),
			subplot(ceil(size(X.H,2)/N),N,K);
			tmp=.5;sum(h)/2;
			%plot(t,cumsum(h)/sum(h),'-',t,cumsum(exp(-(t-mu).^2/sd2/2)/sqrt(2*pi*sd2)/X.N(K)),'c',mu+sqrt(sd2)*[-5 -3 -1 0 1 3 5]',tmp*ones(7,1),'+-',MaxMin,tmp,'rx');
			plot(cumsum(h)/sum(h),normcdf(t,mu,sqrt(sd2)),'xb',[0,1],[0,1],'-c');
		elseif strcmp(yscale,'csum'),
			subplot(ceil(size(X.H,2)/N),N,K);
			tmp=.5;sum(h)/2;
			%plot(t,cumsum(h)/sum(h),'-',t,cumsum(exp(-(t-mu).^2/sd2/2)/sqrt(2*pi*sd2)/X.N(K)),'c',mu+sqrt(sd2)*[-5 -3 -1 0 1 3 5]',tmp*ones(7,1),'+-',MaxMin,tmp,'rx');
			plot(t,cumsum(h)/sum(h),'-',t,normcdf(t,mu,sqrt(sd2)),'c',mu+sqrt(sd2)*[-5 -3 -1 0 1 3 5]',tmp*ones(7,1),'+-',MaxMin,tmp,'rx');
			v=axis; v(1:2)=[MaxMin(2)+0.1*diff(MaxMin) MaxMin(1)-0.1*diff(MaxMin)]; axis(v);
		elseif strcmp(yscale,'CDF'),
			%subplot(ceil(size(X.H,2)/N),N,K);
			tmp=sum(h)/2;
			%semilogx(X.X,cumsum(X.H,1)./X.N(ones(size(X.X,1),1),:),'-');
			plot([X.X(1,:)-eps;X.X],[zeros(1,size(X.H,2));cumsum(X.H,1)]./X.N(ones(size(X.X,1)+1,1),:),'-');
			t = [t(1)-eps;t];
			%plot(t,[cumsum([0;h])/X.N(K),normcdf(t,mu,sqrt(sd2))])
			%plot(t,cumsum([0;h])/X.N(K))
			%v=axis; v(1:2)=[MaxMin(2)+0.1*diff(MaxMin) MaxMin(1)-0.1*diff(MaxMin)]; axis(v);
			v=axis; v(3:4)=[0,1]; axis(v);
		elseif strcmp(yscale,'stacked'),
			bar(t,h,'stacked');	
		end;
	end;	
elseif strcmp(X.datatype,'DBI-EPs'),
	if nargin<2,
		arg2='b';	
	end;
	if nargin<3,
		arg3=100;
	end;
	
	for k=1:length(X.eegchan),
		subplot(13,10,k);
		
		mu=X.RES(k).SUM./X.RES(k).N;
		sd=sqrt((X.RES(k).SSQ-X.RES(k).SUM.*mu)./max(X.RES(k).N,0)); 
		se=sd./sqrt(X.RES(k).N);
		
		h=plot(X.t,[mu(:),sd(:),se(:)]*[1,1,1,1,1;-1,1,0,0,0;0,0,-1,1,0],arg2);
		set(h(5),'linewidth',2);
		set(h(1),'color',[0,.75,.75]);
		set(h(2),'color',[0,.75,.75]);
		set(h(3),'color',[.5,.5,1]);
		set(h(4),'color',[.5,.5,1]);
		axis([-3,3,-arg3,arg3]);
		title(sprintf('#%i',X.eegchan(k))); 
	end;	
	
	
elseif strcmp(X.datatype,'SCATTER'),
	s='x';
	if nargin<2,
		
	elseif nargin==2,
		if length(arg2)==1
			s = arg2;
		else
			Labels=arg2;
		end
	elseif nargin>2,
		s = arg2;
		Labels = arg3;
	end;
	
	if length(X)==1,
		if ~isfield(X,'R');
			[X.R,X.p,X.CIL,X.CIU] = corrcoef(X.data,'Rank');	
		end;	
		[nr,nc] = size(X.data);
		nc=nc-1;       
		for k   = 1:nc,
			for l = k+1:nc+1,%[1:k-1,k+1:nc],
				h=subplot(nc,nc,(k-1)*nc+l-1);
				plot(X.data(:,l),X.data(:,k),s);
				%ht=title(sprintf('r=%.3f %s [%.3f,%.3f]',X.R(k,l),char('*'*(X.p(k,l)<[.05,.01,.001])),X.CIL(k,l),X.CIU(k,l)));
				ht=title(sprintf('r = %.4f %s',X.R(k,l),char('*'*(X.p(k,l)<[.05,.01,.001]))));
				pos=get(ht,'position');
				%pos(2)=max(X.data(k));
				%set(ht,'position',pos);
				% title gives the r-value, its confidence interval (see CORRCOFF for which level), 
				%      and indicates the significance for alpha=0.05 (*), 0.01(**) and 0.001(***)
				if l == (k+1),
					xlabel(Labels{l});	
					ylabel(Labels{k});	
				else
					set(h,'xtick',[]);    
					set(h,'ytick',[]);    
				end;
			end;
		end;
	else
		if ~isfield(X,'R');
			[X.R,X.p,X.CIL,X.CIU] = corrcoef(X(1).data,X(2).data,'Rank');	
		end;
		[nr,nc2] = size(X(2).data);
		for k   = 1:nc,
			for l = 1:nc2,%[1:k-1,k+1:nc],
				h=subplot(nc,nc2,(k-1)*nc2+l-1);
				plot(X(2).data(:,l),X(1).data(:,k),s);
				ht=title(sprintf('r=%.3f %s [%.3f,%.3f]',X.R(k,l),char('*'*(X.p(k,l)<[.05,.01,.001])),X.CIL(k,l),X.CIU(k,l)));
				%ht=title(sprintf('r = %.4f %s',X.R(k,l),char('*'*(X.p(k,l)<[.05,.01,.001]))));
				pos=get(ht,'position');
				%pos(2)=max(X.data(k));
				%set(ht,'position',pos);
				% title gives the r-value, its confidence interval (see CORRCOFF for which level), 
				%      and indicates the significance for alpha=0.05 (*), 0.01(**) and 0.001(***)
				if l == 1,
					%xlabel(arg3{l});	
					ylabel(arg3{k});	
				else
					%set(h,'xtick',[]);    
					set(h,'ytick',[]);    
				end;
			end;
			if k == nc,
				xlabel(arg3{l});	
				%ylabel(arg3{k});	
			else
				set(h,'xtick',[]);    
				%set(h,'ytick',[]);    
			end;
		end;
	end;
	
elseif strcmp(X.datatype,'STAT2'),
	if nargin<2,
		arg2='b';	
	end;
	if isfield(X,'t')
		t=X.t;	
	else
		t=1:length(X.SUM);	
	end;
	if nargin>2
		t=arg3;
	end;
	
	mu=X.SUM./X.N;
	sd=sqrt((X.SSQ-X.SUM.*mu)./max(X.N-1,0));
	se=sd./sqrt(X.N);
	
	h=plot(t,[mu(:),sd(:),se(:)]*[1,1,1,1,1;0,-1,1,0,0;0,0,0,-1,1],arg2);
	set(h(1),'linewidth',2);
	tmp=get(h(1),'color');
	set(h(2),'color',1-(1-tmp)/2);
	set(h(3),'color',1-(1-tmp)/2);
	%set(h(4),'color',[0,0,1]);
	%set(h(5),'color',[0,0,1]);

	
elseif strcmp(X.datatype,'TSD1'),
	if nargin<2,
		arg2='b';	
	end;
	h=plot(X.t,[X.mu,X.sd]*[1,1,1;0,-1,1],arg2);
	set(h(1),'linewidth',2);
	hold on
	h=plot(X.TI(:),1,'.k');
	
	
elseif strcmp(X.datatype,'MEAN+STD')
	
	nchns = min(size(X.MEAN));  % Number of channels
	
	if nargin < 2
	    clf;
	    nf = [];
	else
	    nf = arg2;  % Handles to subplots
	end;
	if isempty(nf)
	    for k = 1:nchns
		nf(k) = subplot(ceil(nchns/ceil(sqrt(nchns))),ceil(sqrt(nchns)),k);  % Handles to subplots
	    end;
	end;
	
	if isfield(X,'Label')
	    if ischar(X.Label)
		X.Label = cellstr(X.Label);
	    end;
	end;
	
	if nargin>2
		minmean = arg3;
		maxmean = arg4;
		maxstd  = arg5;
	else
		minmean = floor(min(min(X.MEAN)));
		maxmean = ceil(max(max(X.MEAN)));
		maxstd = ceil(max(max(X.STD)));
	end;
	

	for k = 1:nchns  % For each channel

	    subplot(nf(k));
	    [ax,h1,h2] = plotyy(X.T,X.MEAN(k,:),X.T,X.STD(k,:));
	    drawnow;
	    set(ax(1),'FontSize',8);
	    set(ax(2),'FontSize',8);

	    % Sets the axes limits to avoid overlapping of the two functions
	    set(ax(1),'YLim',[minmean-maxstd maxmean]);
	    set(ax(2),'YLim',[0 -minmean+maxstd+maxmean]);
	    set(ax,'XLim',[min(X.T) max(X.T)]);

	    set(ax,'YTickLabel',[]);
	    set(ax,'YTick',[]);

	    % Label y1-axis (mean)
	    temp = [floor(minmean/10) * 10 : 10 : ceil(maxmean/10) * 10];  % Label only ..., -20, -10, 0, 10, 20, 30, ...
	    set(ax(1),'YTick',temp);

	    set(ax(1),'YTickLabel',temp);

	    % Label y2-axis (standard deviation)
	    temp = [0 : 10 : ceil(maxstd/10) * 10];  % Label only 0, 10, 20, 30, ...
	    set(ax(2),'YTick',temp);

	    set(ax(2),'YTickLabel',temp);

	    % Label x-axis

	    xlabel('Time (s)');
		grid on;

	    if isfield(X,'Label')  % Print label of each channel (if such a label exists)
		if k <= length(X.Label)
		    title(X.Label{k},'FontSize',8,'Interpreter','none');
		end;
	    end;

	    if isfield(X,'trigger')  % Mark trigger
		line([X.T(X.trigger),X.T(X.trigger)],[minmean-maxstd,maxmean],'Color',[1 0 0]);
	    end;
	end;
	drawnow;
	%set(0,'DefaultTextInterpreter','none');  % Avoid having TeX interpretation in title string
	%suptitle(X.Title);
	
	
	
elseif strcmp(X.datatype,'CORRELATION_WITH_REFERENCE')
	if isfield(X,'ELEC') | isfield(X,'ELPOS')
		fprintf(2,'PLOTA: X.ELPOS, X.ELEC not supported yet.\n'); 
	end; 
	FLAG.TOPOMAP = 1; 
	if nargin>1,
		if ~exist(arg2,'file');
			fprintf(2,'Warning PLOTA: electrode position file not found.\n'); 
			FLAG.TOPOMAP = 0; 
		end;		
	else
	%	fprintf(2,'Warning PLOTA: electrode position file not specified.\n'); 
		FLAG.TOPOMAP = 0; 
	end;		
	if FLAG.TOPOMAP,
		topoplot(X.corr,arg2,'maplimits',[-1,1]);
		colorbar;
	else
		plot(X.corr); 
		axis([1,length(X.corr),-1,1]);
	end;
	
	
elseif strcmp(X.datatype,'Classifier')
	if ~isfield(X,'tsc'),
		X.tsc=X.TI*16+[-15,1];
	end;
	if (nargin==1);
		arg2 = 'all';
		for k=1:4,
			hf(k) = subplot(1,4,k);
		end
	elseif (nargin==2);
		if isnumeric(arg2)
			hf = arg2; 
			if length(hf)==3,
				arg2='all';
			elseif length(hf)==1,
				arg2='acc';
				subplot(hf);
			end;
		else   % arg2=arg2;
		end
	elseif (nargin==3);
		if isnumeric(arg2)
			hf = arg2; 
			arg2 = arg3;
		else
			hf = arg3;
		end;		
	else
		
	end;
	
	if ~isfield(X,'T');
		%X.T = (1:size(X.acc,1))';
		X.T = (1:size(X.MDA.ACC00,1))';
		if isfield(X,'Fs'),
			X.T = X.T/X.Fs;
		end;
	else;
		
	end;	
	LEG = [];
	if isfield(X,'Classes'),
		if ischar(X.Classes)
			LEG = X.Classes;
		elseif isnumeric(X.Classes)
			LEG = num2str(X.Classes(:));
		end;
	end;
	if strncmpi(arg2,'acc',3)
		if isfield(X,'tsc'),
			patch(X.T(X.tsc([1,1,2,2,1])),[0,1,1,0,0]*100,[1,1,1]*.8);
		end;
		hold on;
		%plot(X.T,X.acc*100,'-',X.T([1,end]),[100,100]./size(X.acc,2),'k:');
		plot(X.T,X.MDA.ACC00*100,'-',X.T([1,end]),[100,100]./size(X.MDA.ACC00,2),'k:');
		hold off;
		v=axis;v(3:4)=[0,100];axis(v);
		
		ylabel('Accuracy [%]');
		grid on;
		if ~isempty(LEG)
			legend(LEG);
		end
		
	elseif strcmpi(arg2,'fixed') |  strcmpi(arg2,'fixed-MDA'),
		if 0,isfield(X,'tsc'),
			patch(X.T(X.tsc([1,1,2,2,1])),[0,1,1,0,0]*100,[1,1,1]*.8);
		end;
		hold on;
		plot(X.T,X.MDA.acc*100,'-',X.T([1,end]),[100,100]./size(X.MDA.acc,2),'k:');
		hold off;
		v=axis;v(3:4)=[0,100];axis(v);
		
		ylabel('mean recognistion rate [%]');
		grid on;
		if ~isempty(LEG)
			legend(LEG);
		end
		
	elseif strcmpi(arg2,'fixed-LLH')
		if 0,isfield(X,'tsc'),
			patch(X.T(X.tsc([1,1,2,2,1])),[0,1,1,0,0]*100,[1,1,1]*.8);
		end;
		hold on;
		plot(X.T,X.LLH.acc*100,'-',X.T([1,end]),[100,100]./size(X.LLH.acc,2),'k:');
		hold off;
		v=axis;v(3:4)=[0,100];axis(v);
		
		ylabel('Accuracy [%]');
		grid on;
		if ~isempty(LEG)
			legend(LEG);
		end
		
	elseif strcmpi(arg2,'KAPPA') | strcmpi(arg2,'KAPPA-MDA')
		if isfield(X,'tsc'),
			patch(X.T(X.tsc([1,1,2,2,1])),[0,1,1,0,0]*100,[1,1,1]*.8);
		end;
		hold on;
		plot(X.T,[X.MDA.KAP00,X.MDA.ACC00]*100);
		hold off;
		grid on;
		%v=axis; v(3:4)=[-10,100]; axis(v);
		v=axis; v(3:4)=[0,100]; axis(v);
		ylabel('Kappa [%], Accuracy [%]')
		xlabel('time t [s]');
		legend('Kappa', 'Accuracy');
		
	elseif strcmpi(arg2,'KAPPA-LLH')
		if isfield(X,'tsc'),
			patch(X.T(X.tsc([1,1,2,2,1])),[0,1,1,0,0]*100,[1,1,1]*.8);
		end;
		hold on;
		plot(X.T,[X.LLH.KAP00,X.LLH.ACC00]*100);
		hold off;
		grid on;
		%v=axis; v(3:4)=[-10,100]; axis(v);
		v=axis; v(3:4)=[0,100]; axis(v);
		ylabel('Kappa [%], Accuracy [%]')
		xlabel('time t [s]');
		legend('Kappa', 'Accuracy');
		
	elseif strncmpi(arg2,'MI',2)
		if isfield(X,'tsc'),
			patch(X.T(X.tsc([1,1,2,2,1])),[0,1,1,0,0],[1,1,1]*.8);
		end;
		hold on;
		if isfield(X,'I0') %& any(X.I0(:)~=0);
			h=plot(X.T,[X.I0,X.I])
		else
			h=plot(X.T,[X.MD2.I0,sum(X.MD2.I0,2)]);
			%h=plot(X.T,[X.GRB.I0,X.GRB.I2]);
		end;
		hold off
		grid on;
		v=axis; v(3:4)=[0,1]; axis(v);
		set(h(end),'linewidth',2)
		ylabel('MI [bit]');
		xlabel('time t [s]');
		if ~isempty(LEG)
			legend(LEG);
		end
		
	elseif strncmpi(arg2,'all',3)
		plota(X,'acc',hf(1));
		plota(X,'KAPPA',hf(2));
		plota(X,'fixed',hf(3));
		plota(X,'MI',hf(4));
	end;
	
elseif strncmp(X.datatype,'TSD_BCI',7) & (nargin>1) & strcmpi(arg2,'TSD');
	N = length(X.CL);
	if N>2,
		for k=1:N, 
			nf(k)=subplot(ceil(sqrt(N)),N/ceil(sqrt(N)),k); 
		end;
	end;
	
	if N==2, N=1; end;
	for k=1:N,
		if N>1,
			subplot(nf(k));
		end;
		h=plot(X.T,[X.MEAN1(:,k),X.MEAN2(:,k),X.SD1(:,k),X.SD2(:,k)]*[1,0,1,0,1,0; 0,1,0,1,0,1; 0,0,1,0,-1,0; 0,0,0,1,0,-1]);
		set(h(1),'linewidth',2);
		set(h(2),'linewidth',2);
		mset(h(3:6),'linewidth',1);
		mset(h([1,3,5]),'color','b');
		mset(h([2,4,6]),'color','g');
		ylabel('Average TSD');
		v=axis;v(1:2)=[min(X.T),max(X.T)];axis(v);
	end;
	
elseif strcmp(X.datatype,'TSD_BCI9') 
		if nargin<2,
			clf;
			for k=1:6, 
				nf(k)=subplot(3,2,k); 
			end;
		else
			nf=arg2;
		end;
		
		fid = 1;
		tix = X.tix(1);
		N = length(X.CL);
		c = (N-1)/N; 

		fprintf(fid,'Error:		     %4.1f %% \n',100-X.ACC00(tix)*100);
		fprintf(fid,'Accuracy:		  %4.1f %% \nspecific Accuracy:	 ',X.ACC00(tix)*100);
		[kap,sd,H,z,OA,SA,MI] = kappa(X.optCMX);
		fprintf(fid,'%4.1f   ',SA*100);
		fprintf(fid,'\nKappa:		     %4.2f ± %4.2f\n',X.KAP00(tix),X.Ksd00(tix));
		fprintf(fid,'I(Wolpaw):		 %4.2f bit\n',wolpaw_entropy(X.ACC00(tix),N));
		fprintf(fid,'I(Nykopp):		 %4.2f bit\n',X.I_Nykopp(tix));
		fprintf(fid,'I(Continous):	      SUM = %4.2f  [ ',sum(X.I(tix,:))*c);
		fprintf(fid,'%4.2f   ',X.I(tix,:));
		t = X.T; t(t<3.5)=NaN;
		fprintf(fid,' ] \nSTMI:		      %4.2f  [ ',max([sum(X.I,2)]./[t-3])*c);
		fprintf(fid,'   %4.2f',max(X.I./[t(:,ones(1,size(X.I,2)))-3]));
		fprintf(fid,' ] \nSNR:		       ')
		fprintf(fid,'%4.2f   ',X.SNR(tix,:));
		fprintf(fid,'\ncorrelation (parametric):  ')
		fprintf(fid,'%4.2f   ',X.r(tix,:));
		fprintf(fid,'\nrank correlation:	  ');
%		fprintf(fid,'%4.2f   ',X.rankcorrelation(tix,:));
		fprintf(fid,'\nAUC:		       ');
		fprintf(fid,'%4.2f   ',X.AUC(tix,:));
		fprintf(fid,'\n');
		
		
		subplot(nf(1));
		if ~isfield(X,'Labels')
			for k=1:length(X.CL),Labels{k}=int2str(X.CL(k));end;
		else
			Labels = X.Labels; 
		end;
		%plota(X);
		plot(X.T,100*[X.ACC00, X.KAP00])
		axis([0,9,-20,100])
		xlabel('time [s]')
		title('Accuracy and Kappa')
		legend({'Accuracy [%]','Kappa [%]'})
		
		subplot(nf(3));
		plot(X.T,[sum(X.I,2)*c,X.I_Nykopp(:),wolpaw_entropy(X.ACC00,N)])
		legend({'I_{continous}','I_{Nykopp}','I_{Wolpaw}'})
		title('Mutual information')
		ylabel('I [bit]');
		xlabel('time [s]')
		v=axis; axis([0,9,0,1.5]);
		
		subplot(nf(5));
		plot(X.T,X.MEAN2)
		hold on 
		plot(X.T,X.MEAN1)
		hold off
		Title('average output of one-vs-rest classifiers')
		ylabel('TSD')
		xlabel('time [s]')
		tmp = strvcat(Labels); 
		legend(cellstr([tmp,repmat('(+)',size(tmp,1),1);tmp,repmat('(-)',size(tmp,1),1)]))
		v=axis; axis([0,9,v(3:4)]);
		
		subplot(nf(2));
		plot(X.T,X.AUC)
		xlabel('time [s]')
		ylabel('AUC [1]')
		title('area-under-the-(ROC)-curve'); 
		legend(Labels)
		v=axis; axis([0,9,v(3:4)]);
		
		subplot(nf(4));
		plot(X.T,[X.I,sum(X.I,2)*c,])
		xlabel('time [s]')
		ylabel('I [bit]');
		title('Mutual Information of continous output'); 
		legend([Labels,{'SUM'}])
		v=axis; axis([0,9,0,1.5]);
		
		subplot(nf(6));
		plot(X.T,X.r)
		xlabel('time [s]')
		ylabel('r [1]')
		title('correlation coefficient (parametric)'); 
		legend(Labels)
		v=axis; axis([0,9,-.2,1]);
		
elseif strcmp(X.datatype,'TSD_BCI8')    % obsolote
	if ~isfield(X,'T');
		X.T = [1:size(X.ACC00,1)]';
	end;
	h = plot(X.T, 100*[X.ACC00, X.KAP00*[1,1,1,0] + X.Ksd00*[0,-1,1,1]],'-k');
	v = axis; v(3:4)=[-20,129];axis(v)
	set(h(1),'color',[0,0,1]);
	set(h(2),'color',[0,.5,0]);
	set(h(3),'color',[0,1,0]);
	set(h(4),'color',[0,1,0]);
	legend('Accuracy [%]','kappa ± s.d. [%]')
	xlabel('t [s]');
	
elseif strcmp(X.datatype,'TSD_BCI7')    % obsolete
	if (nargin>1) & strcmpi(arg2,'balken2');
		%elseif strcmpi(X.datatype,'Balken_Diagramm'),
		
		dy = 0.3;
		F  = 25;
		mn = rs([X.MEAN2(:),X.MEAN1(:)],F,1)';
		pc = rs([X.BCG2(:),X.BCG1(:)],F,1)';
		barh(rs(X.T,F,1),rs([1-2*X.BCG1,X.BCG2*2-1],F,1))
		
	elseif (nargin>1) & strcmpi(arg2,'balken');
		%elseif strcmpi(X.datatype,'Balken_Diagramm'),
		
		dy = 0.3;
		F  = 125;
		if isfield(X,'Fs'),
			if X.Fs==125,
				F = 50;
			elseif X.Fs == 128,
				F = 32;
			end
		end;
		mn = [rs(X.MEAN2(:),F,1)';rs(X.MEAN1(:),F,1)'];
		pc = [rs(X.BCG2(:),F,1)';rs(X.BCG1(:),F,1)'];
		%barh(rs(X.T,F,1),rs([X.BCG1,X.BCG2],F,1),1)
		
		samples = (1:length(mn))';
		time = X.T(F:F:end)';
		%time = time - time(1);
		
		% the following sequence is from R. Scherer 
		for k = 1:size(mn, 2),
			if abs(mn(1,k)) > abs(mn(2,k))
				patch([0 0 mn(1,k) mn(1,k) 0], [k-dy k+dy k+dy k-dy k-dy], 'k');
				patch([0 0 mn(2,k) mn(2,k) 0], [k-dy k+dy k+dy k-dy k-dy], 'w');
			else
				patch([0 0 mn(2,k) mn(2,k) 0], [k-dy k+dy k+dy k-dy k-dy], 'w');
				patch([0 0 mn(1,k) mn(1,k) 0], [k-dy k+dy k+dy k-dy k-dy], 'k');
			end
			pos1 = min(min(mn(:,k)), 0);
			pos2 = max(max(mn(:,k)), 0);
			
			text(pos1, k, [' ' num2str(100*pc(1,k), '%.0f') '% '], 'HorizontalAlignment', 'Right');
			text(pos2, k, [' ' num2str(100*pc(2,k), '%.0f') '% '], 'HorizontalAlignment', 'Left');
		end
		
		mx = max(max(mn));
		mn = min(min(mn));
		
		%xlim(1.2 * [mn mx]);
		xlim([-1,1]);
		set(gca, 'ytick', 1:length(time), 'yticklabel', num2str(time(:), '%02.2f'));
		ylabel('time in seconds');
		xlabel('distance')
		ylim([0.5 length(samples)+0.5]);
		%	title(caption);
		
		pos = get(gca, 'position');
		axes('position', pos, 'color', 'none', 'YTick', [], 'XTick', []);
		set(gca, 'YAxisLocation', 'right');
		
		xlim(1.2 * [mn mx]);
		ylim([0.5 length(samples)+0.5]);
		set(gca, 'ytick', 1:length(time), 'yticklabel', num2str(100*mean(pc)', '%.0f'));
		
		ylabel('overall classification result')
		
		%elseif strcmp(X.datatype,'TSD_BCI7') | strcmp(X.datatype,'SNR'), ,
	else
		if nargin<2,
			clf;
			for k=1:3, 
				nf(k)=subplot(1,3,k); 
			end;
		else
			nf=arg2;
		end;
		if isfield(X,'KAP00');
			if nargin<2,
				hf(1) = subplot(121);
				hf(2) = subplot(122);
			else
				hf = arg2;
			end;
			if ~isfield(X,'T')
				X.T=(1:size(X.KAP00,1));	
			end;
			subplot(hf(1));
			plot(X.T,[X.KAP00,X.ACC00]*100);
			grid on;
			v=axis; v(3:4)=[-10,100]; axis(v);
			ylabel('Kappa [%], Accuracy [%]')
			xlabel('time t [s]');
			legend('Kappa', 'Accuracy');
			
			subplot(hf(2));
			h=plot(X.T,[X.I0,X.I]);
			grid on;
			v=axis; v(3:4)=[0,1]; axis(v);
			set(h(end),'linewidth',2)
			ylabel('MI [bit]');
			xlabel('time t [s]');
			LEG=[];
			for k = 1:length(X.MD),
				LEG{k} = int2str(k);
			end;
			LEG{k+1}='all';
			%legend(LEG);
			return;
		end;
		
		if ~isfield(X,'Fs'),
			X.Fs=128;
		end;
		N=length(X.I);
		if isfield(X,'T')
			t=X.T;%(min(X.T(:)):max(X.T(:)))/X.Fs;
		else
			t=(1:N)/X.Fs;	
		end;
		
		subplot(nf(1));
		plot(t,X.ERR*100);
		grid on;
		ylabel('Error rate [%]')
		v=axis;v=[0,max(t),0,100];axis(v);
		
		subplot(nf(2));
		%if strcmp(X.datatype,'SNR'),
		if isfield(X,'MEAN1'),
			h=plot(t,[X.MEAN1(:),X.MEAN2(:),X.SD1(:),X.SD2(:)]*[1,0,0,0; 0,1,0,0; 1,0,1,0; 1,0,-1,0; 0,1,0,1; 0,1,0,-1]','b',t([1,length(t)]),[0,0],'k');
		else
			h=plot(t,[X.M1(:),X.M2(:),X.SD1(:),X.SD2(:)]*[1,0,0,0; 0,1,0,0; 1,0,1,0; 1,0,-1,0; 0,1,0,1; 0,1,0,-1]','b',t([1,length(t)]),[0,0],'k');
		end;
		set(h(1),'linewidth',2);
		set(h(2),'linewidth',2);
		set(h(7),'linewidth',2);
		set(h(2),'color','g');
		set(h(5),'color','g');
		set(h(6),'color','g');
		ylabel('Average TSD');
		v=axis;v(1:2)=[0,max(t)];axis(v);
		
		subplot(nf(3));
		if isfield(X,'T')
			tmp = repmat(NaN,size(X.T));
			tmp((X.T>=3.5) & X.ERR<=.50) = 0;
			plot(t,[X.I,X.I./(X.T-3)+tmp]);
		else
			plot(t,X.I);
		end;
		ylabel('Mutual Information [bits]')
		v=axis;v=[0,max(t),0,.5];axis(v);
		
		if length(nf)>3,
			axes(nf(4))
			plota(X,'balken');
		elseif 0,
			plot(t,X.corcov)
			v=axis;v=[0,10,-1,1];axis(v);
			grid('on')
			ylabel('correlation coefficient')	
		end;
	end;	
	
	
elseif strcmp(X.datatype,'QRS_events'),
	ix = X.QRS_event;
	l =length(ix);
	semilogy((ix(1:l-1)+ix(2:l))/2,diff(ix));
	
	
elseif strcmp(X.datatype,'AMARMA')

	if X.MOP(3)~=0, return; end; 
	if (X.MOP(1)==1) & (size(X.AAR,2)==X.MOP(2)+1), 
		m0 = X.AAR(:,1)./(1-sum(X.AAR(:,2:end),2));
		ix_aar = 2:X.MOP(2)+1;
	elseif (X.MOP(1)==0) & (size(X.AAR,2)==X.MOP(2)),
		m0 = zeros(size(X.AAR,1),1);
		ix_aar = 1:X.MOP(2);
	else
		return;
	end;	 
	
	MODE = [];
	if ~isempty(findstr(upper(arg3),'TIME')),	MODE = [MODE,1]; end;
	if ~isempty(findstr(upper(arg3),'VLHF')),	MODE = [MODE,2]; end;
	if ~isempty(findstr(upper(arg3),'IMAGE')),	MODE = [MODE,3]; end;
	if ~isempty(findstr(upper(arg3),'3D')),		MODE = [MODE,4]; end;
	if ~isempty(findstr(upper(arg3),'n.u.')),	MODE = [MODE,5]; end;
	if ~isempty(findstr(upper(arg3),'LF-BW')),	MODE = [MODE,6]; end;
	if ~isempty(findstr(upper(arg3),'F0')),		MODE = [MODE,7]; end;
	if ~isempty(findstr(upper(arg3),'F0+-B')),	MODE = [MODE,8]; end;
	if ~isempty(findstr(upper(arg3),'ALL')),	MODE = [1,2,3,6]; end;

	if nargin<4,
		if any(MODE==4)
			hf = gca;
		else
			for k = 1:length(MODE);
				hf(k) = subplot(length(MODE),1,k);	
			end;
		end;	
	else
		hf = arg4;
	end;
	if ~isfield(X,'T')
		X.T = (0:size(X.AAR,1)-1);
	end;
	
	HHHH = [];
	K = 0;
	if any(MODE==1)	
		K = K + 1;
		subplot(hf(K));
		%plot(X.T,[signal,m0,sqrt([tmp(:,8),X.PE])]);
		
		if isfield(X,'S')
			ha=plot(X.T,[X.S,m0,sqrt(X.PE)]);
			MM1 = max([X.S,m0,sqrt(X.PE)]);
			MM2 = min([X.S,m0,sqrt(X.PE)]);
		else
			ha=plot(X.T,[m0,sqrt(X.PE)]);
			MM1 = max([m0,sqrt(X.PE)]);
			MM2 = min([m0,sqrt(X.PE)]);
		end;
		MM = [max(MM1),min(MM2)];
		if isfield(X,'EVENT')
			hold on
			%text(X.EVENT.POS,repmat(sqrt(max(X.PE))*2,length(X.EVENT.POS),1),'r+');
			plot(X.EVENT.POS*[1,1],MM*[1.2,0;-.20,1],':k');
			for k = 1:length(X.EVENT.POS), 
				ha(k)=text(X.EVENT.POS(k),MM*[1;0],X.EVENT.Desc{k});
				set(ha(k),'VerticalAlignment','top');
				set(ha(k),'Rotation',90);
			end;
			hold off	
		end;	
		HHHH = ha;
			
		v = axis; v(2) = max(X.T); v(3)=0; axis(v);
		ylabel([X.Label,' [',deblank(X.PhysDim),']']);
		hc= colorbar;
		pos=get(gca,'position');
		delete(hc);
		set(gca,'position',pos);
		if isfield(X,'S')
			legend('Raw','Mean','RMS')
		else
			legend('mean','RMS')
		end;
	end;	

	if prod(size(arg2))==1,    
		f0 = repmat(arg2,size(X.AAR,1),1);
	elseif prod(size(arg2))>1,    
		f0 = arg2;
	end;

	if any(MODE==2)
		[w,A,B,R,P,F,ip] = ar_spa(X.AAR(:,ix_aar),f0,X.PE);
		ix = (imag(F)==0);

	elseif any(MODE==5) | any(MODE==6) | any(MODE==7) | any(MODE==8)	
		[w,A,B,R,P,F,ip] = ar_spa(X.AAR(:,ix_aar),1,X.PE);
		ix = (imag(F)==0);

	end;

	if any(MODE==2)	
		ixVLF = ((w>=0)  & (w<.04)); F1 = real(F); F1(~ixVLF)= NaN;
		ixLF  = (w>=.04) & (w<=.15); F2 = real(F); F2(~ixLF) = NaN;
		ixHF  = (w>.15)  & (w<=.4) ; F3 = real(F); F3(~ixHF) = NaN;
		
		tmp = [sumskipnan(real(F),2), sumskipnan(F1,2), sumskipnan(F2,2), sumskipnan(F3,2)];
		tmp(:,5) = tmp(:,3)./tmp(:,4);
		tmp(:,6) = tmp(:,3)./(tmp(:,1)-tmp(:,2));
		tmp(:,7) = tmp(:,4)./(tmp(:,1)-tmp(:,2));
		tmp(:,8) = sum(tmp(:,2:4),2);

		K = K + 1;
		subplot(hf(K));
		semilogy(X.T,tmp(:,[3,4,8,1])); % 1
		v = axis; v(2) = max(X.T); axis(v);
		ylabel(sprintf('%s [%s^2]',X.Label,X.PhysDim));
		hc= colorbar;
		pos=get(gca,'position');
		delete(hc);
		set(gca,'position',pos);

		%ylabel(sprintf('%s [%s^2/%s]',X.Label,X.PhysDim,'s'));
		legend({'LF','HF','VLF+LF+HF','total'})
	end;	
		
	if any(MODE==5)	
		ixVLF = ((w>=0)  & (w<.04)); F1 = real(F); F1(~ixVLF)= NaN;
		ixLF  = (w>=.04) & (w<=.15); F2 = real(F); F2(~ixLF) = NaN;
		%ixHF  = (w>.15)  & (w<=.4) ; 
		ixHF  = (w>.17*f0(:,ones(1,size(w,2)))) & (w<=.40*f0(:,ones(1,size(w,2)))); 
		F3 = real(F); F3(~ixHF) = NaN;
    		
		tmp = [sumskipnan(real(F),2), sumskipnan(F1,2), sumskipnan(F2,2), sumskipnan(F3,2)];
		tmp(:,5) = tmp(:,3)./tmp(:,4);
		tmp(:,6) = tmp(:,3)./(tmp(:,1)-tmp(:,2));
		tmp(:,7) = tmp(:,4)./(tmp(:,1)-tmp(:,2));
		tmp(:,8) = sum(tmp(:,2:4),2);
		

		K = K + 1;
		subplot(hf(K));
		%semilogy(X.T,tmp(:,[3,4,8])); % 1
		plot(X.T,tmp(:,[6,7])*100); % 1
		v = axis; v(2:4) = [max(X.T),0,100]; axis(v);
		ylabel(sprintf('%s [n.u. %%]',X.Label));
		hc= colorbar;
		pos=get(gca,'position');
		delete(hc);
		set(gca,'position',pos);
	

		%ylabel(sprintf('%s [%s^2/%s]',X.Label,X.PhysDim,'s'));
		legend({'LF (0.05-0.15Hz)','HF (0.15*f0-0.40*f0)'})
	end;	
		
	if any(MODE==3), 	
		DN = max(1,ceil(size(X.AAR,1)/1000)); %assume 1000 pixels 
		N  = size(X.AAR,1);
		clear h h2 F3
		for l = 1:DN:N,  %N/2; [k,size(sdf),N],%length(AR);
			k = ceil(l/DN);
			% [h(:,k),F(:,k)] = freqz(sqrt(X.PE(l)/(2*pi*X.AAR(l,1))),[1, -X.AAR(l,2:end)]',128,f0(l,1));
			%[h2(:,k),F3] = freqz(sqrt(X.PE(l)/(2*pi*X.AAR(l,1))),[1, -X.AAR(l,ix_aar)]',[0:100]'/64,f0(l,1));
			[h2(:,k),F3] = freqz(sqrt(X.PE(l)/(2*pi*f0(l,1))),[1, -X.AAR(l,ix_aar)]',[0:100]'/64,f0(l,1));
			h2(find(F3>f0(l,1)/2),k)=NaN;
		end;
	
		K = K + 1;
		subplot(hf(K));
		if 0;%FB{1}=='B';
			h=imagesc(X.T(1:DN:N),F3,2*log10(abs(h2(:,end:-1:1)))); 
		else
			h=imagesc(X.T(1:DN:N),F3,2*log10(abs(h2))); 
		end;
		xlabel('beats');
		ylabel('f [1/s]');
		pos0 = get(gca,'position');
		
		hc= colorbar;
		%pos1 = get(gca,'position')
		v = get(hc,'yticklabel');
		v = 10.^str2num(v)*2;
		pos2 = get(hc,'position');
		%pos2(1) = 1 - (1 - pos0(1) - pos0(3))/4;
		%pos2(1) = pos0(1) + pos0(3) + pos2(3)/2;
		%pos2(3) = pos2(3)/2
		%set(hc,'yticklabel',v,'position',pos2);
		
		%set(gca,'position',pos0);
		title(sprintf('%s [%s^2/%s]',deblank(X.Label),deblank(X.PhysDim),'s'));
	end;

	if any(MODE==4), 	
		DN = 8; %10;
		N  = size(X.AAR,1);
		clear h h2 F3
		for l = 1:DN:N,  %N/2; [k,size(sdf),N],%length(AR);
			k = ceil(l/DN);
			[h(:,k),F4(:,k)] = freqz(sqrt(X.PE(l)/(2*pi*X.AAR(l,1))),[1, -X.AAR(l,2:end)]',128,f0(l,1));
			% [h2(:,k),F3] = freqz(sqrt(X.PE(l)/(2*pi*X.AAR(l,1))),[1, -X.AAR(l,2:end)]',[0:70]'/64,f0(l,1));
			% h2(find(F3>f0(l,1)/2),k)=NaN;
		end;

		K = K + 1;
		ah=subplot(hf(K));
		plot3(repmat(1:DN:ceil(N),128,1),F4,6+2*log10(abs(h))); 
		xlabel('beats');
		ylabel('f [1/s]');
		zlabel(sprintf('%s [%s^2/%s]',X.Label,X.PhysDim,'s'));
		v = get(ah,'zticklabel');
		v = 10.^str2num(v)*2;
		set(ah,'zticklabel',v);
		
		view([60,-45]);	
		pos=get(ah,'position'); %pos(4)=.5;
		set(ah,'position',pos);
		set(1,'PaperUnits','inches','PaperOrientation','portrait','PaperPosition',[0.25 .5 8 10]);
		HHHH = ah;
	end;	
		
	if any(MODE==6)	
		ixVLF = ((w>=0)  & (w<.04)); F1 = real(F); F1(~ixVLF)= NaN;
		ixLF  = (w>=.04) & (w<=.15); F2 = real(F); F2(~ixLF) = NaN;
    		ixHF  = (w>.15)  & (w<=.4) ; F3 = real(F); F3(~ixHF) = NaN;
			
		B2 = B; B2(~ixLF) = NaN; 
		w2 = w; w2(~ixLF) = NaN; 

		HHHH = mean(mean(B2,2).*f0);
		fprintf(1,'Mean LF Bandwidth %f\n',HHHH);
	
		K = K + 1;
		subplot(hf(K));
		plot(X.T, [mean(w2,2), mean(B2,2).*f0]);
		% hold on
		% errorbar(X.T,mean(w2,2),mean(B2,2));
		% hold off;
		
		v = axis; v(2:4) = [max(X.T),0,.2]; axis(v);

		hc= colorbar;
		pos=get(gca,'position');
		delete(hc);
		set(gca,'position',pos);

		%legend('f0(LF)','bandwidth(LF)'); 
		xlabel('time')
		ylabel('f [Hz]');
		legend('f0(LF)','bandwidth(LF)'); 
		drawnow

	end;	

	if any(MODE==7)	
		clim = [0,max(A(:))];
		cm = colormap;

		B2 = B; B2(w<0) = NaN; 
		w2 = w; w2(w<0) = NaN; 
		for k2 = 1:size(w2,2),
		for k1 = 1:size(w2,1),
		if w2(k1,k2)>0,
			H = plot(X.T(k1),w2(k1,k2),'d');
			set(H,'color',cm(ceil(A(k1,k2)/clim(2)*size(cm,1)),:));
		end;
		end;
		end;	
	end;
	if any(MODE==8)	
		B2 = B; B2(w<0) = NaN; 
		w2 = w; w2(w<0) = NaN; 
		HHHH = plot(X.T,w2,'d', X.T,w2+B,'v', X.T,w2-B,'^');
	end;	
	h = HHHH; 

elseif strcmp(X.datatype,'REV'),
	if nargin<2
		arg2=[];
	end;
	
	if strcmpi(arg2,'3d')	
		R=X.REV;
		[mxR,ix]=min(R);
		[myR,iy]=min(R');
		[IX,IY]=find(R==min(R(:)));
		[IX,IY],
		h=mesh(R');hold on
		plot3(ix,1:length(ix),mxR,'go-')
		plot3(1:length(iy),iy,myR,'cx-')
		plot3(IX,IY,min(R(:)),'r*');
		hold off
		%v=axis;v(5:6)=[0.1,.2];axis(v);
		a=gca;
		set(a,'xtick',X.P);
		t=1:5:length(X.UC);
		set(a,'ytick',t);
		set(a,'yticklabel',X.UC(t));
		ylabel('update coefficent UC')
		xlabel('model order p')
		zlabel('REV = MSE/MSY')
	else  %if strcmpi(arg2,'2d')	
		subplot(121)
		semilogx(X.UC,X.REV')
		v=axis;v=[1e-6,1e-1,0.1,.15];axis(v);
		xlabel('update coefficent UC')
		ylabel('REV = MSE/MSY')
		
		subplot(122)
		h=plot(X.P,X.REV)
		v=axis;v(3:4)=[0.1,.15];axis(v);
		xlabel('model order p')
		ylabel('REV = MSE/MSY')
		
	end;
	
elseif strcmp(X.datatype,'MDist-matrix'),
	if nargin<2
		arg2=[];
	end;
	h=mesh(X.D)
	h=get(gca,'xtick');
	%set(gca,'xticklabel',X.t(h(h>0)))
	h=get(gca,'ytick');
	%set(gca,'yticklabel',X.t(h(h>0)))
	
elseif strcmp(X.datatype,'MD'),
	if nargin<2
		arg2=[];
	end;
	
	if strcmpi(arg2,'3d')	
		R=X.REV;
		[mxR,ix]=max(R);
		[myR,iy]=max(R');
		[IX,IY]=find(R==max(R(:)));
		h=mesh(R');hold on
		plot3(ix,1:length(ix),mxR,'go-')
		plot3(1:length(iy),iy,myR,'cx-')
		plot3(IX,IY,min(R(:)),'r*');
		hold off
		%		v=axis;v(5:6)=[0.1,.2];axis(v);
		a=gca;
		set(a,'xtick',X.P);
		t=1:5:length(X.UC);
		set(a,'ytick',t);
		set(a,'yticklabel',X.UC(t));
		ylabel('update coefficent UC')
		xlabel('model order p')
		zlabel('log_{10}(MD)')
	else  %if strcmpi(arg2,'2d')	
		subplot(121)
		semilogx(X.UC,X.REV')
		%		v=axis;v=[1e-6,1e-1,0.1,.15];axis(v);
		xlabel('update coefficent UC')
		ylabel('log_{10}(MD)')
		
		subplot(122)
		h=plot(X.P,X.REV)
		%	       v=axis;v(3:4)=[0.1,.15];axis(v);
		xlabel('model order p')
		ylabel('log_{10}(MD)')
	end;
end;


if nargout,
	H = h;
end;
