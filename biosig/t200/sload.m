function [signal,H] = sload(FILENAME,CHAN,Fs)
% SLOAD loads signal data of various data formats
% 
% Currently are the following data formats supported: 
%    EDF, CNT, EEG, BDF, GDF, BKR, MAT(*), 
%    PhysioNet (MIT-ECG), Poly5/TMS32, SMA, RDF, CFWB,
%    Alpha-Trace, DEMG, SCP-ECG.
%
% [signal,header] = sload(FILENAME,CHAN)
%       reads selected (CHAN) channels
%       if CHAN is 0, all channels are read 
% [signal,header] = sload(FILENAME [,CHANNEL [,Fs]])
% FILENAME      name of file, or list of filenames
% channel       list of selected channels
%               default=0: loads all channels
% Fs            force target samplerate Fs (only 
%               integer and 256->100 conversion is supported) 
%
% [signal,header] = sload(dir('f*.emg'), CHAN)
% [signal,header] = sload('f*.emg', CHAN)
%  	loads channels CHAN from all files 'f*.emg'
%
% see also: SVIEW, SOPEN, SREAD, SCLOSE, SAVE2BKR, TLOAD
%
% Reference(s):
%


%	$Revision: 1.49 $
%	$Id: sload.m,v 1.49 2005-01-08 21:25:22 schloegl Exp $
%	Copyright (C) 1997-2005 by Alois Schloegl 
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% Version 2 of the License, or (at your option) any later version.
%
% This library is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% Library General Public License for more details.
%
% You should have received a copy of the GNU Library General Public
% License along with this library; if not, write to the
% Free Software Foundation, Inc., 59 Temple Place - Suite 330,
% Boston, MA  02111-1307, USA.


if nargin<2; CHAN=0; end;
if nargin<3; Fs=NaN; end;

if CHAN<1 | ~isfinite(CHAN),
        CHAN=0;
end;

%%% resolve wildcards %%%
if (ischar(FILENAME) & any(FILENAME=='*'))
        p = fileparts(FILENAME);
        f = dir(FILENAME);
        EOGix = zeros(1,length(f));
        for k = 1:length(f);
                f(k).name = fullfile(p,f(k).name);
                [p,g,e]=fileparts(f(k).name);
                lg = length(g);
                if (lg>2) & strcmp(upper(g(lg+(-2:0))),'EOG')
                        EOGix(k) = 1;
                end
        end;
        FILENAME=f([find(EOGix),find(~EOGix)]);
end;        


if ((iscell(FILENAME) | isstruct(FILENAME)) & (length(FILENAME)>1)),
	signal = [];
	for k = 1:length(FILENAME),
		if iscell(FILENAME(k))
			f = FILENAME{k};
		else 
			f = FILENAME(k);
		end	

                [s,h] = sload(f,CHAN,Fs);
		if k==1,
			H = h;
			signal = s;  
			LEN = size(s,1);
		else
			H.FILE(k) = h.FILE;
			if ~isnan(h.SampleRate) & (H.SampleRate ~= h.SampleRate),
				fprintf(2,'Warning SLOAD: sampling rates of multiple files differ %i!=%i.\n',H.SampleRate, h.SampleRate);
			end;

                        if size(s,2)==size(signal,2), %(H.NS == h.NS) 
				signal = [signal; repmat(NaN,100,size(s,2)); s];
			else
				fprintf(2,'ERROR SLOAD: incompatible channel numbers %i!=%i of multiple files\n',H.NS,h.NS);
				return;
			end;

                        if ~isempty(h.EVENT.POS),
                                H.EVENT.POS = [H.EVENT.POS; h.EVENT.POS+size(signal,1)-size(s,1)];
                                H.EVENT.TYP = [H.EVENT.TYP; h.EVENT.TYP];
                                if isfield(H.EVENT,'CHN');
                                        H.EVENT.CHN = [H.EVENT.CHN; h.EVENT.CHN];
                                end;
                                if isfield(H.EVENT,'DUR');
                                        H.EVENT.DUR = [H.EVENT.DUR; h.EVENT.DUR];
                                end;
                                if isfield(H.EVENT,'Desc');	% TFM-Excel-Beat-to-Beat
                                        H.EVENT.Desc = [H.EVENT.Desc; h.EVENT.Desc];
                                end;
                        end;			
                        if isfield(h,'TRIG'), 
                                if ~isfield(H,'TRIG'),
                                        H.TRIG = [];
                                end;
                                H.TRIG = [H.TRIG(:); h.TRIG(:)+size(signal,1)-size(s,1)];
                        end;
                        
                        if isfield(H,'TriggerOffset'),
                                if H.TriggerOffset ~= h.TriggerOffset,
                                        fprintf(2,'Warning SLOAD: Triggeroffset does not fit.\n',H.TriggerOffset,h.TriggerOffset);
                                        return;
                                end;
                        end;
                        if isfield(H,'Classlabel'),
                                if isfield(H,'ArtifactSelection')
                                        if isfield(h,'ArtifactSelection'),
                                                if any(h.ArtifactSelection>1) | (length(h.ArtifactSelection) < length(h.Classlabel))
                                                        sel = zeros(size(h.Classlabel));
                                                        sel(h.ArtifactSelection) = 1; 
                                                else
                                                        sel = h.ArtifactSelection(:);
                                                end;
                                                H.ArtifactSelection = [H.ArtifactSelection; h.ArtifactSelection(:)];
                                        elseif isfield(H,'ArtifactSelection'),
                                                H.ArtifactSelection = [H.ArtifactSelection;zeros(length(h.Classlabel),1)];
                                        end;
                                end;
                                H.Classlabel = [H.Classlabel(:);h.Classlabel(:)];
                        end;
                        clear s
                end;
	end;
        
	fprintf(1,'  SLOAD: data segments are concanated with NaNs in between.\n');
	return;	
end;
%%% end of multi-file section 


%%%% start of single file section
if ~isnumeric(CHAN),
        MODE = CHAN;
        CHAN = 0; 
else
        MODE = '';
end;

signal = [];

H = sopen(FILENAME,'rb',CHAN);
if isempty(H),
	fprintf(2,'Warning SLOAD: no file found\n');
	return;
else	
	% FILENAME can be fn.name struct, or HDR struct. 
	FILENAME = H.FileName; 
end;
    
if H.FILE.OPEN > 0,
        [signal,H] = sread(H);
        H = sclose(H);


elseif any(strmatch(H.TYPE,{'native','TFM_EXCEL_Beat_to_Beat'})); 
        [signal,H] = sread(H);
        H = sclose(H);

        
elseif strcmp(H.TYPE,'EVENTCODES')
        signal = H.EVENT;


elseif strcmp(H.TYPE,'AKO')
        signal = fread(H.FILE.FID,inf,'uint8')*H.Calib(2,1)+H.Calib(1,1);
        
        fclose(H.FILE.FID);
        

elseif strcmp(H.TYPE,'DAQ')
	fprintf(1,'Loading a matlab DAQ data file - this can take a while.\n');
	tic;
        [signal, tmp, H.DAQ.T0, H.DAQ.events, DAQ.info] = daqread(H.FileName);
        fprintf(1,'Loading DAQ file finished after %.0f s.\n',toc);
        H.NS   = size(signal,2);
        
        H.SampleRate = DAQ.info.ObjInfo.SampleRate;
        sz     = size(signal);
        if length(sz)==2, sz=[1,sz]; end;
        H.NRec = sz(1);
        H.Dur  = sz(2)/H.SampleRate;
        H.NS   = sz(3);
        H.FLAG.TRIGGERED = H.NRec>1;
        H.FLAG.UCAL = 1;
        
        H.PhysDim = {DAQ.info.ObjInfo.Channel.Units};
        H.DAQ   = DAQ.info.ObjInfo.Channel;
        
        H.Cal   = diff(cat(1,DAQ.info.ObjInfo.Channel.InputRange),[],2).*(2.^(-DAQ.info.HwInfo.Bits));
        H.Off   = cat(1,DAQ.info.ObjInfo.Channel.NativeOffset); 
        H.Calib = sparse([H.Off';eye(H.NS)]*diag(H.Cal));
        
        if CHAN<1,
                CHAN = 1:H.NS; 
        end;
        if ~H.FLAG.UCAL,
                Calib = H.Calib;	% Octave can not index sparse matrices within a struct
                signal = [ones(size(signal,1),1),signal]*Calib(:,CHAN);
        end;
        
        
elseif strcmp(H.TYPE,'BIFF'),
	try, 
                [H.TFM.S,H.TFM.E] = xlsread(H.FileName,'Beat-To-Beat');
                if size(H.TFM.S,1)+1==size(H.TFM.E,1),
                        H.TFM.S = [repmat(NaN,1,size(H.TFM.S,2));H.TFM.S];
                end;
                H.TYPE = 'TFM_EXCEL_Beat_to_Beat'; 
	catch
	end; 	

	if strcmp(H.TYPE, 'TFM_EXCEL_Beat_to_Beat');
                if ~isempty(strfind(H.TFM.E{3,1},'---'))
                        H.TFM.S(3,:) = [];    
                        H.TFM.E(3,:) = [];    
                end;
                
                H.Label   = strvcat(H.TFM.E(4,:)');
                H.PhysDim = strvcat(H.TFM.E(5,:)');
           
                H.TFM.S = H.TFM.S(6:end,:);
                H.TFM.E = H.TFM.E(6:end,:);
                
                ix = find(isnan(H.TFM.S(:,2)) & ~isnan(H.TFM.S(:,1)));
                H.EVENT.Desc = H.TFM.E(ix,2);
                H.EVENT.POS  = ix;
                
                if any(CHAN),
			H.TFM.S = H.TFM.S(:,CHAN);
			H.TFM.E = H.TFM.E(:,CHAN);
		end;
		[H.SPR,H.NS] = size(H.TFM.S);
		H.NRec = 1; 
		H.THRESHOLD  = repmat([0,NaN],H.NS,1);

		signal  = H.TFM.S;
		signal(signal==0) = NaN;
        end;


elseif strcmp(H.TYPE,'BMP'),
        H.FILE.FID = fopen(H.FileName,'rb','ieee-le');
        fseek(H.FILE.FID,10,-1);
        
        tmp = fread(H.FILE.FID,4,'uint32');
        H.HeadLen = tmp(1);
        H.BMP.sizeBitmapInfoHeader = tmp(2);
        H.IMAGE.Size = tmp(3:4)';
        
        tmp = fread(H.FILE.FID,2,'uint16');
        H.BMP.biPlanes = tmp(1);
        H.bits = tmp(2);
        
        tmp = fread(H.FILE.FID,6,'uint32');
        H.BMP.biCompression = tmp(1);
        H.BMP.biImageSize = tmp(2);
        H.BMP.biXPelsPerMeter = tmp(3);
        H.BMP.biYPelsPerMeter = tmp(4);
        H.BMP.biColorUsed = tmp(5);
        H.BMP.biColorImportant = tmp(6);
        
        fseek(H.FILE.FID,H.HeadLen,'bof');
        nc = ceil((H.bits*H.IMAGE.Size(1))/32)*4;
        
        if (H.bits==1)
                signal = fread(H.FILE.FID,[nc,H.IMAGE.Size(2)*8],'ubit1');
                signal = signal(1:H.IMAGE.Size(1),:)';
                
        elseif (H.bits==4)
                palr   = [  0,128,  0,128,  0,128,  0,192,128,255,  0,255,  0,255,  0,255]; 
                palg   = [  0,  0,128,128,  0,  0,128,192,128,  0,255,255,  0,  0,255,255]; 
                palb   = [  0,  0,  0,  0,128,128,128,192,128,  0,  0,  0,255,255,255,255]; 
                tmp    = uint8(fread(H.FILE.FID,[nc,H.IMAGE.Size(2)*2],'ubit4'));
                signal        = palr(tmp(1:H.IMAGE.Size(1),:)'+1);
                signal(:,:,2) = palg(tmp(1:H.IMAGE.Size(1),:)'+1);
                signal(:,:,3) = palb(tmp(1:H.IMAGE.Size(1),:)'+1);
                signal = signal(H.IMAGE.Size(2):-1:1,:,:);
                
        elseif (H.bits==8)
                pal = uint8(colormap*256);
                tmp = fread(H.FILE.FID,[nc,H.IMAGE.Size(2)],'uint8');
                signal        = pal(tmp(1:H.IMAGE.Size(1),:)'+1,1);
                signal(:,:,2) = pal(tmp(1:H.IMAGE.Size(1),:)'+1,2);
                signal(:,:,3) = pal(tmp(1:H.IMAGE.Size(1),:)'+1,3);
                signal = signal(H.IMAGE.Size(2):-1:1,:,:);
                
        elseif (H.bits==24)
                [signal]    = uint8(fread(H.FILE.FID,[nc,H.IMAGE.Size(2)],'uint8'));
                H.BMP.Red   = signal((1:H.IMAGE.Size(1))*3,:)';
                H.BMP.Green = signal((1:H.IMAGE.Size(1))*3-1,:)';
                H.BMP.Blue  = signal((1:H.IMAGE.Size(1))*3-2,:)';
                signal = H.BMP.Red;
                signal(:,:,2) = H.BMP.Green;
                signal(:,:,3) = H.BMP.Blue;
                signal = signal(H.IMAGE.Size(2):-1:1,:,:);
        else
                
        end;
        fclose(H.FILE.FID);

        
elseif strcmp(H.TYPE,'MatrixMarket'),
        H.FILE.FID = fopen(H.FileName,'rt','ieee-le');

    	line = fgetl(H.FILE.FID);
	
	H.FLAG.Coordinate = ~isempty(strfind(line,'coordinate'));
	H.FLAG.Array 	  = ~isempty(strfind(line,'array'));

	H.FLAG.Complex = ~isempty(strfind(line,'complex'));
	H.FLAG.Real = ~isempty(strfind(line,'real'));
	H.FLAG.Integer = ~isempty(strfind(line,'integer'));
	H.FLAG.Pattern = ~isempty(strfind(line,'pattern'));
	
	H.FLAG.General = ~isempty(strfind(line,'general'));
	H.FLAG.Symmetric = ~isempty(strfind(line,' symmetric'));
	H.FLAG.SkewSymmetric = ~isempty(strfind(line,'skew-symmetric'));
	H.FLAG.Hermitian = ~isempty(strfind(lower(line),'hermitian'));

	while strncmp(line,'%',1)
        	line = fgetl(H.FILE.FID);
	end;

	[tmp,status] = str2double(line);
	if any(status)
		fprintf(H.FILE.stderr,'SLOAD (MM): invalid size %s\n',line);
	else
		H.MATRIX.Size = tmp;
	end;	

	if length(H.MATRIX.Size)==3,
		H.Length = tmp(3);
		signal = sparse([],[],[],tmp(1),tmp(2),tmp(3));
		for k = 1:H.Length,
	        	line = fgetl(H.FILE.FID);
			[tmp,status] = str2double(line);
			if any(status)
				fprintf(H.FILE.stderr,'SLOAD (MM): invalid size %s\n',line);
			elseif length(tmp)==4,	
		    		val = tmp(3) + i*tmp(4);
			elseif length(tmp)==3,	
				val = tmp(3);
			elseif length(tmp)==2,	
				val = 1;
			else
				fprintf(H.FILE.stderr,'SLOAD (MM): invalid size %s\n',line);
			end;

			if H.FLAG.General,
				signal(tmp(1),tmp(2)) = val;
			elseif H.FLAG.Symmetric,
				signal(tmp(1),tmp(2)) = val;
				signal(tmp(2),tmp(1)) = val;
			elseif H.FLAG.SkewSymmetric,
				signal(tmp(1),tmp(2)) = val;
				signal(tmp(2),tmp(1)) =-val;
			elseif H.FLAG.Hermitian,
				signal(tmp(1),tmp(2)) = val;
				signal(tmp(2),tmp(1)) = conj(val);
			else	
				fprintf(H.FILE.stderr,'SLOAD (MM): invalid size %s\n',line);
			end;	
		end;
					
	elseif length(H.MATRIX.Size)==2
		H.Length = prod(tmp);
		signal = zeros(H.MATRIX.Size);
		if H.FLAG.General==1,
			[IX,IY]=find(ones(H.MATRIX.Size));
		else
			[IX,IY]=find(cumsum(eye(H.MATRIX.Size)));
		end;
				
		for k = 1:H.Length,
	        	line = fgetl(H.FILE.FID);
			[tmp,status] = str2double(line);
			if any(status)
				error('SLOAD (MM)');
			elseif length(tmp)==2,	
				val=tmp(1) + i*tmp(2);
			elseif length(tmp)==1,	
				val=tmp(1);
			else
				fprintf(H.FILE.stderr,'SLOAD (MM): invalid size %s\n',line);
			end;

			signal(IX(k),IY(k)) = val;
			if H.FLAG.Symmetric,
				signal(IY(k),IX(k)) = val;
			elseif H.FLAG.SkewSymmetric,
				signal(IY(k),IX(k)) =-val;
			elseif H.FLAG.Hermitian,
				signal(IY(k),IX(k)) = conj(val);
			else	
				fprintf(H.FILE.stderr,'SLOAD (MM): invalid size %s\n',line);
			end;	
		end;
        end;
        fclose(H.FILE.FID);

        
elseif strcmp(H.TYPE,'OFF'),
	        H.FILE.FID = fopen(H.FileName,'rt','ieee-le');
		
                line1 = fgetl(H.FILE.FID);
                line2 = fgetl(H.FILE.FID);
		while ~feof(H.FILE.FID) & (line2(1)=='#'),
	                line2 = fgetl(H.FILE.FID);
		end;
		[tmp,status] = str2double(line2);
		if status | (size(tmp,2)~=3), 
			fclose(H.FILE.FID);
			error('SOPEN (OFF)');
		else
			H.VertexCount = tmp(1);
			H.FaceCount = tmp(2);
			H.EdgeCount = tmp(3);
		end	
		
		H.Vertex = repmat(NaN,H.VertexCount,3);
		for k = 1:H.VertexCount,
			line = '';
			while isempty(line) | strncmp(line,'#',1)
				line = fgetl(H.FILE.FID);
			end;
			len = min(length(line),min(find(line=='#')));
			tmp = str2double(line(1:len));
			H.Vertex(k,:) = tmp(1:H.ND);
		end;	
		
%		H.Face = repmat(NaN,H.FaceCount,3);
		for k = 1:H.FaceCount,
			line = '';
			while isempty(line) | strncmp(line,'#',1)
				line = fgetl(H.FILE.FID);
			end;
			len = min(length(line),min(find(line=='#')));
			tmp = str2double(line(1:len));
			H.Ngon(k) = tmp(1);
			H.Face{k} = tmp(2:tmp(1)+1) + 1;
		end;	
		if all(H.Ngon(1)==H.Ngon),
			H.Face = cat(1,H.Face{:});
		end;	
                fclose(H.FILE.FID);
        
        
elseif strcmp(H.TYPE,'POLY'),
        H.FILE.FID = fopen(H.FileName,'rt','ieee-le');

	K = 0;
	while ~feof(H.FILE.FID)
        	line = fgetl(H.FILE.FID);
		if isempty(line),
		elseif line(1)=='#',
		else
			K = K + 1;
			
		end;
        end;
        fclose(H.FILE.FID);
        
        
elseif strcmp(H.TYPE,'PBMA') | strcmp(H.TYPE,'PGMA')  | strcmp(H.TYPE,'PPMA') ,
        H.FILE.FID = fopen(H.FileName,'rt','ieee-le');

	N = NaN;
	K = 1;
	s = [];
	H.IMAGE.Size = [inf,inf];
	while ~feof(H.FILE.FID) & (length(signal)<prod(H.IMAGE.Size))
        	line = fgetl(H.FILE.FID);

		if isempty(line),
		elseif strncmp(line,'P1',2),
			N = 1; 
		elseif strncmp(line,'P2',2),
			N = 2; 
		elseif strncmp(line,'P3',2),
			N = 2; 
		elseif line(1)=='#',
		elseif isnumeric(line),
		elseif K==1,
			[tmp, status] = str2double(line);
			K = K + 1;
			H.IMAGE.Size = tmp([2,1]);
			if status
				error('SLOAD (PPMA)');
			end;
		elseif K==N,
			[tmp, status] = str2double(line);
			K = K + 1;
			H.DigMax = tmp; 
		else
			line = line(1:min([find(line=='#'),length(line)]));	% remove comment
			[tmp,status] = str2double(char(line)); %,[],[9,10,13,32])
			if ~any(status),
				s = [s; tmp'];
			end;	
		end;
	end;	
	fclose(H.FILE.FID);
	H.s=s;
	if strcmp(H.TYPE,'PPMA'),
	if prod(H.IMAGE.Size)*3~=length(s),
		fprintf(H.FILE.stderr,'SLOAD(P3): %i * %i != %i \n',H.IMAGE.Size,length(s));
	else
		signal = repmat(NaN,[H.IMAGE.Size,3]);
		signal(:,:,1) = reshape(s(1:3:end),H.IMAGE.Size)';
		signal(:,:,2) = reshape(s(2:3:end),H.IMAGE.Size)';
		signal(:,:,3) = reshape(s(3:3:end),H.IMAGE.Size)';
        end;
	else
	if prod(H.IMAGE.Size)~=length(s),
		fprintf(H.FILE.stderr,'SLOAD(P1/P2): %i * %i != %i \n',H.IMAGE.Size,length(s));
	else
		signal = reshape(s,H.IMAGE.Size)';
        end;
        end;

elseif strcmp(H.TYPE,'PBMB'),
        H.FILE.FID = fopen(H.FileName,'rb','ieee-le');
	status = fseek(H.FILE.FID, H.HeadLen, 'bof');
	[tmp,count] = fread(H.FILE.FID,[H.IMAGE.Size(2)/8,H.IMAGE.Size(1)],'uint8');
        fclose(H.FILE.FID);
	
	signal = zeros(H.IMAGE.Size)';
	
	for k = 1:8,
		signal(:,k:8:H.IMAGE.Size(1)) = bitand(tmp',2^(8-k))>0;
	end;		

elseif strcmp(H.TYPE,'PGMB'),
        H.FILE.FID = fopen(H.FileName,'rb','ieee-le');
	status = fseek(H.FILE.FID, H.HeadLen, 'bof');
	[signal,count] = fread(H.FILE.FID,[H.IMAGE.Size(2),H.IMAGE.Size(1)],'uint8');
        fclose(H.FILE.FID);
	signal = signal';

elseif strcmp(H.TYPE,'PPMB'),
        H.FILE.FID = fopen(H.FileName,'rb','ieee-le');
	status = fseek(H.FILE.FID, H.HeadLen, 'bof');
	[tmp,count] = fread(H.FILE.FID,[3*H.IMAGE.Size(2),H.IMAGE.Size(1)],'uint8');
        fclose(H.FILE.FID);

	signal = zeros([H.IMAGE.Size(1:2),3]);
	signal(:,:,1) = tmp(1:3:end,:)';
	signal(:,:,2) = tmp(2:3:end,:)';
	signal(:,:,3) = tmp(3:3:end,:)';
	
        
elseif strcmp(H.TYPE,'SMF'),
	        H.FILE.FID = fopen(H.FileName,'rt','ieee-le');
		
		VertexCount = 0;
		FaceCount = 0;
		PalLen = 0; 
		K = 1;
		while ~feof(H.FILE.FID)
	                line = fgetl(H.FILE.FID);
			if isempty(line)
			elseif line(1)=='#';

			elseif line(1)=='v';
				[tmp,status] = str2double(line(3:end));
				if ~any(status)
					VertexCount = VertexCount + 1 ;
					H.Vertex(VertexCount,:) = tmp;
				else
					fprintf(H.FILE.stderr,'Warning SLOAD: could not read line %i in file %s\n',K,H.FileName); 	
				end;	

			elseif line(1)=='f';
				[tmp,status] = str2double(line(3:end));
				if ~any(status)
					FaceCount  = FaceCount + 1; 
					H.Ngon(FaceCount) = length(tmp);
					H.Face{FaceCount} = tmp;
				else
					fprintf(H.FILE.stderr,'Warning SLOAD: could not read line %i in file %s\n',K,H.FileName); 	
				end;	

			elseif line(1)=='n';
				[tmp,status] = str2double(line(3:end));
				if ~any(status)
					H.NormalVector = tmp;
				else
					fprintf(H.FILE.stderr,'Warning SLOAD: could not read line %i in file %s\n',K,H.FileName); 	
				end;	

			elseif line(1)=='c';
				[tmp,status] = str2double(line(3:end));
				if ~any(status)
					PalLen = PalLen +1; 
					H.Palette(PalLen,:)= tmp;
				else
					fprintf(H.FILE.stderr,'Warning SLOAD: could not read line %i in file %s\n',K,H.FileName); 	
				end;	
			else

			end;
			K = K+1;
		end;
		fclose(H.FILE.FID);
		if all(H.Ngon(1)==H.Ngon),
			H.Face = cat(1,H.Face{:});
		end;	
	
        
elseif strcmp(H.TYPE,'FITS'),
	if CHAN>1,
		KK = CHAN;
	else	
		[tmp, KK] = max(H.IMAGE_Size);   % select block
	end;
	
	for KK = 1:length(H.FITS),

	status = fseek(H.FILE.FID,H.HeadLen(KK),'bof');

	H.AS.bps = abs(H.FITS{KK}.BITPIX)/8;
	if H.FITS{KK}.BITPIX == 8,
		H.GDFTYP = 'uint8';
	elseif H.FITS{KK}.BITPIX == 16,
		H.GDFTYP = 'int16';
	elseif H.FITS{KK}.BITPIX == 32,
		H.GDFTYP = 'int32';
	elseif H.FITS{KK}.BITPIX == 64,
		H.GDFTYP = 'int64';
	elseif H.FITS{KK}.BITPIX == -32,
		H.GDFTYP = 'float32';
	elseif H.FITS{KK}.BITPIX == -64,
		H.GDFTYP = 'float64';
	else
		warning('SOPEN (FITS{KK})');
	end;	
	
	if isfield(H.FITS{KK},'BZERO')		H.Off = H.FITS{KK}.BZERO;
	else					H.Off = 0;			end;		
	if isfield(H.FITS{KK},'BSCALE')		H.Cal = H.FITS{KK}.BSCALE;
	else					H.Cal = 1;			end;		
	if isfield(H.FITS{KK},'BUNIT'),		H.PhysDim = H.FITS{KK}.BUNIT;
	else					H.PhysDim = '[1]';		end;		
	if isfield(H.FITS{KK},'DATAMAX'),	H.PhysMax = H.FITS{KK}.DATAMAX;
	else					H.PhysMax = NaN;		end;
	if isfield(H.FITS{KK},'DATAMIN'),	H.PhysMin = H.FITS{KK}.DATAMIN;
	else					H.PhysMin = NaN;	 	end;

	if ~isfield(H.FITS{KK},'XTENSION')
		[signal,c] = fread(H.FILE.FID, prod(H.IMAGE(KK).Size), H.GDFTYP);
		signal = reshape(signal,H.IMAGE(KK).Size) * H.Cal + H.Off;

	elseif strncmp(H.FITS{KK}.XTENSION,'TABLE',5)
		for k = 1:H.FITS{KK}.TFIELDS,
			f = ['TTYPE',int2str(k)];
			if isfield(H.FITS{KK},f)
				H.TABLE{KK}.Label(k,:) = getfield(H.FITS{KK},f);
			end;	
			tmp = getfield(H.FITS{KK},['TFORM',int2str(k)]);
			typ(k) = tmp(1);
			ix(k)  = getfield(H.FITS{KK},['TBCOL',int2str(k)]);
		end; 
		ix(k+1) = H.FITS{KK}.NAXIS1+1;

		sa = {};
		[signal,c] = fread(H.FILE.FID, H.IMAGE(KK).Size, H.GDFTYP);
		signal = signal';
		s = repmat(NaN, H.FITS{KK}.NAXIS2, H.FITS{KK}.TFIELDS);
		for k = 1:H.FITS{KK}.TFIELDS,
			[s(:,k), status,sa(:,k)] = str2double(char(signal(:,ix(k):ix(k+1)-1)));
		end;
		H.TABLE{KK} = s;
        
	elseif strncmp(H.FITS{KK}.XTENSION,'BINTABLE',8)
		for k = 1:H.FITS{KK}.TFIELDS,
			f = ['TTYPE',int2str(k)];
			if isfield(H.FITS{KK},f)
				H.TABLE{KK}.Label(k,:) = getfield(H.FITS{KK},f);
			end;
			tmp = getfield(H.FITS{KK},['TFORM',int2str(k)]);
			ix  = min(find(tmp>'9'));
			sz(k) = str2double(tmp(1:ix-1)); 
			H.FITS{KK}.TYP(k) = tmp(ix);

			if 0, 
			elseif tmp(ix)=='L', 	GDFTYP{k} = 'char';	% char T, F
			elseif tmp(ix)=='X', 	GDFTYP{k} = 'ubit1';	%ubit1
			elseif tmp(ix)=='B', 	GDFTYP{k} = 'uint8';	% uint8
			elseif tmp(ix)=='I', 	GDFTYP{k} = 'int16';	% int16
			elseif tmp(ix)=='J', 	GDFTYP{k} = 'int32';	% int32
			elseif tmp(ix)=='A', 	GDFTYP{k} = 'uchar';	% char
			elseif tmp(ix)=='E', 	GDFTYP{k} = 'float32';	% float32
			elseif tmp(ix)=='D', 	GDFTYP{k} = 'float64';	% double
			elseif tmp(ix)=='C', sz(k) = sz(k)*2;	GDFTYP{k} = 'float32';	% [1+i]*float32
			elseif tmp(ix)=='M', sz(k) = sz(k)*2;	GDFTYP{k} = 'float64';	% [1+i]*double
			elseif tmp(ix)=='P', 	GDFTYP{k} = 'uint64';	% uint64, array desc ? 
			else
			end;
		end; 

		for k2 = 1:H.FITS{KK}.NAXIS2,
		for k1 = 1:H.FITS{KK}.TFIELDS,
			[s,c] = fread(H.FILE.FID, [1,sz(k1)], GDFTYP{k1});

			if 0, 
			elseif H.FITS{KK}.TYP(k1)=='L',
				sig{k1}(k2,:) = (s=='T');
			elseif H.FITS{KK}.TYP(k1)=='A',
				sig{k1}(k2,:) = char(s);
			elseif any(H.FITS{KK}.TYP(k1)=='CM'),
				sig{k1}(k2,:) = [1,i]*reshape(s,2,sz(k1)/2);
			else
				sig{k1}(k2,:) = s;
			end;	
 		end;
		end;
		H.TABLE{KK} = sig;
        
	elseif strncmp(H.FITS{KK}.XTENSION,'IMAGE',5)
	H.GDFTYP,
	ftell(H.FILE.FID),
		[signal,c] = fread(H.FILE.FID, prod(H.IMAGE(KK).Size), H.GDFTYP);
		
		[c,size(signal),H.IMAGE(KK).Size]
		signal = reshape(signal,H.IMAGE(KK).Size) * H.Cal + H.Off;

	else
		[signal,c] = fread(H.FILE.FID, prod(H.IMAGE(KK).Size), H.GDFTYP);
		signal = reshape(signal,H.IMAGE(KK).Size) * H.Cal + H.Off;

	end;
	end;
	fclose(H.FILE.FID);	
	
	
elseif strcmp(H.TYPE,'TVF 1.1A'),
        H.FILE.FID = fopen(H.FileName,'rt');

	tmp = fgetl(H.FILE.FID);
	H.TVF.Name = fgetl(H.FILE.FID);
	H.TVF.Desc = fgetl(H.FILE.FID);
	[tmp,status] = str2double(fgetl(H.FILE.FID));
	H.TVF.NTR = tmp(1);
	H.TVF.NV  = tmp(2);
	H.FLAG.CULL = ~~tmp(3);
	[tmp,status] = str2double(fgetl(H.FILE.FID));
	H.TVF.NTRC  = tmp(1);
	H.TVF.NTRM  = tmp(2);
	[tmp,status] = str2double(fgetl(H.FILE.FID));
	H.TVF.NVC   = tmp(1);
	H.TVF.NVN   = tmp(2);
	[tmp,status] = str2double(fgetl(H.FILE.FID));
	H.TVF.GlobalColor = tmp;
	[tmp,status] = str2double(fgetl(H.FILE.FID));
	H.TVF.GlobalMtrlProps = tmp;
	
	H.TVF.Triangles = repmat(NaN,[H.TVF.NTR,3]);
	for k = 1:H.TVF.NTR,
		[tmp, status] = str2double(fgetl(H.FILE.FID));
		H.TVF.Triangles(k,:) = tmp;
	end;	
	H.TVF.TrColorSets = reshape(NaN,[H.TVF.NTRC,H.TVF.NTR]);
	for k = 1:H.TVF.NTRC,
		[tmp, status] = str2double(fgetl(H.FILE.FID));
		H.TVF.TrColorSets(k,:) = tmp;
	end;
	H.TVF.TrMtrlSets = repmat(NaN, [H.TVF.NTRM,H.TVF.NTR]);
	for k = 1:H.TVF.NTRM,
		[tmp, status] = str2double(fgetl(H.FILE.FID));
		H.TVF.TrMtrlSets(k,:) = tmp;
	end;

	H.TVF.Vertices   = repmat(NaN, [H.TVF.NV,3]);
	for k = 1:H.TVF.NV,
		[tmp, status] = str2double(fgetl(H.FILE.FID));
		H.TVF.Vertices(k,:) = tmp;
	end;
	H.TVF.VColorSets = repmat(NaN, [H.TVF.NVC,H.TVF.NV]);
	for k = 1:H.TVF.NVC,
		[tmp, status] = str2double(fgetl(H.FILE.FID));
		H.TVF.VColorSets(k,:) = tmp;
	end;
	H.TVF.VNrmlSets  = repmat(NaN, [H.TVF.NVN,3]);
	for k = 1:H.TVF.NVN,
		[tmp, status] = str2double(fgetl(H.FILE.FID));
		H.TVF.VNrmlSets(k,:) = tmp;
	end;

	fclose(H.FILE.FID);	

        
elseif strcmp(H.TYPE,'TVF 1.1B'),
        H.FILE.FID = fopen(H.FileName,'rb',H.Endianity);

	tmp = fread(H.FILE.FID,12,'uchar');
	H.TVF.Name = char(fread(H.FILE.FID,32,'uchar'));
	H.TVF.Desc = char(fread(H.FILE.FID,80,'uchar'));
	tmp = fread(H.FILE.FID,7,'uint32');
	H.TVF.NTR = tmp(1);
	H.TVF.NV  = tmp(2);
	H.FLAG.CULL = ~~tmp(3);
	H.TVF.NTRC  = tmp(4);
	H.TVF.NTRM  = tmp(5);
	H.TVF.NVC   = tmp(6);
	H.TVF.NVN   = tmp(7);
	tmp = fread(H.FILE.FID,[4,2],'float32');
	H.TVF.GlobalColor = tmp(:,1)';
	H.TVF.GlobalMtrlProps = tmp(:,2)';

	H.TVF.Triangles   = fread(H.FILE.FID, [3,H.TVF.NTR],'uint32')';
	H.TVF.TrColorSets = fread(H.FILE.FID, [H.TVF.NTR,H.TVF.NTRC],'uint32')';
	H.TVF.TrMtrlSets  = fread(H.FILE.FID, [H.TVF.NTR,H.TVF.NTRM],'uint32')';

	H.TVF.Vertices   = fread(H.FILE.FID, [3,H.TVF.NV],'float32')';
	H.TVF.VColorSets = fread(H.FILE.FID, [H.TVF.NV,H.TVF.NVC],'uint32')';
	H.TVF.VNrmlSets  = fread(H.FILE.FID, [3,H.TVF.NVN],'float32')';

	fclose(H.FILE.FID);	

        
elseif strcmp(H.TYPE,'VTK'),
                H.FILE.FID = fopen(H.FileName,'rt','ieee-le');
                
                H.VTK.version = fgetl(H.FILE.FID);
                H.VTK.Title   = fgetl(H.FILE.FID);
                H.VTK.type    = fgetl(H.FILE.FID);
		
		while ~feof(H.FILE.FID),
			tline = fgetl(H.FILE.FID);
			
			if 0, 
			elseif strncmp(tline,'CELLS',5);
			elseif strncmp(tline,'CELL_DATA',9);
			elseif strncmp(tline,'DATASET',7);
		                H.VTK.DATASET = tline(8:end);
			elseif strncmp(tline,'LINES',6);
			elseif strncmp(tline,'POINTS',6);
			elseif strncmp(tline,'POINT_DATA',10);
			elseif strncmp(tline,'POLYGON',7);
			elseif strncmp(tline,'SCALARS',7);
				[t1,r]=strtok(tline);
				[dataName,r]=strtok(r);
				[dataType,r]=strtok(r);
				[numComp ,r]=strtok(r);
				if isempty(numComp), numComp=1;
				else numComp = str2double(numComp); end;
				tline = fgetl(fid);
				if strcmp(tline,'LOOKUP_TABLE');
	    				[t1,r]=strtok(tline);
	    				[tableName,r]=strtok(tline);
				else	
					tline = fgetl(fid);
				end;
			end;

		
		end;
                fclose(H.FILE.FID);

                fprintf(H.FILE.stderr,'Warning SOPEN: VTK-format not supported, yet.\n');
	
        
elseif strcmp(H.TYPE,'XPM'),
	H.FILE.FID = fopen(H.FileName,'rt','ieee-le');
		line = '';
		while ~any(line=='{'),
	                line = fgetl(H.FILE.FID);
		end;

                line = fgetl(H.FILE.FID);
		[s,t]=strtok(line,char(34));
		[tmp,status] = str2double(s);

		code1 = repmat(NaN,tmp(3),1);
		code2 = repmat(0,256,1);
		Palette = repmat(NaN,tmp(3),3);
		H.IMAGE.Size = tmp([2,1]);
		k1 = tmp(3);

		for k = 1:k1,
	                line = fgetl(H.FILE.FID);
			[s,t]= strtok(line,char(34));
			code1(k) = s(1);
			code2(s(1)+1) = k;
			Palette(k,:) = [hex2dec(s(6:9)),hex2dec(s(10:13)),hex2dec(s(14:17))];
		end;
		Palette = (Palette/2^16);
		R = Palette(:,1);
		G = Palette(:,2);
		B = Palette(:,3);
		H.Code1 = code1; 
		H.Code2 = code2; 
		H.IMAGE.Palette = Palette; 

		signal = repmat(NaN,[H.IMAGE.Size]);
		for k = 1:H.IMAGE.Size(1),
	                line = fgetl(H.FILE.FID);
			[s,t]= strtok(line,char(34));
			signal(k,:) = abs(s);
		end;
        fclose(H.FILE.FID);
	signal(:,:,1) = code2(signal+1);

	signal(:,:,3) = B(signal(:,:,1));
	signal(:,:,2) = G(signal(:,:,1));
	signal(:,:,1) = R(signal(:,:,1));

        
elseif strcmp(H.TYPE,'IFS'),    % Ultrasound file format
        H.FILE.FID = fopen(H.FileName,'rb','ieee-le');
        H.HeadLen = 512;
        hdr = fread(H.FILE.FID,[1,H.HeadLen],'uchar');
        H.Date = char(hdr(77:100));
        tmp = char(hdr(213:220));
        if strncmp(tmp,'32flt',5)
                H.GDFTYP = 'float32';
        elseif strncmp(tmp,'u8bit',5)
                H.GDFTYP = 'uint8';
        else
                
        end
        fclose(H.FILE.FID);
        
        
elseif strcmp(H.TYPE,'unknown')
        TYPE = upper(H.FILE.Ext);
        if strcmp(TYPE,'DAT')
                loaddat;     
                signal = Voltage(:,CHAN);
        elseif strcmp(TYPE,'RAW')
                loadraw;
        elseif strcmp(TYPE,'RDT')
                [signal] = loadrdt(FILENAME,CHAN);
                fs = 128;
        elseif strcmp(TYPE,'XLS')
                loadxls;
        elseif strcmp(TYPE,'DA_')
                fprintf('Warning SLOAD: Format DA# in testing state and is not supported\n');
                loadda_;
        elseif strcmp(TYPE,'RG64')
                [signal,H.SampleRate,H.Label,H.PhysDim,H.NS]=loadrg64(FILENAME,CHAN);
                %loadrg64;
        else
                fprintf('Error SLOAD: Unknown Data Format\n');
                signal = [];
        end;
end;

if strcmp(H.TYPE,'CNT');    
        f = fullfile(H.FILE.Path, [H.FILE.Name,'.txt']); 
        if exist(f,'file'),
                fid = fopen(f,'r');
		tmp = fread(fid,inf,'char');
		fclose(fid);
		[tmp,v] = str2double(char(tmp'));
		if ~any(v), 
            		H.Classlabel=tmp(:);                        
	        end;
        end
        f = fullfile(H.FILE.Path, [H.FILE.Name,'.par']); 
        if exist(f,'file'),
                fid = fopen(f,'r');
		tmp = fread(fid,inf,'char');
		fclose(fid);
		[tmp,v] = str2double(char(tmp'));
		if ~any(v), 
            		H.Classlabel=tmp(:);                        
	        end;
        end
        f = fullfile(H.FILE.Path, [H.FILE.Name,'.mat']);
        if exist(f,'file'),
                tmp = load(f);
                if isfield(tmp,'classlabel') & ~isfield(H,'Classlabel')
                        H.Classlabel=tmp.classlabel(:);                        
                elseif isfield(tmp,'classlabel') & isfield(tmp,'header') & isfield(tmp.header,'iniFile') & strcmp(tmp.header.iniFile,'oom.ini'), %%% for OOM study only. 
                        H.Classlabel=tmp.classlabel(:);                        
                end;
        end;
        f = fullfile(H.FILE.Path, [H.FILE.Name,'c.mat']);
        if exist(f,'file'),
                tmp = load(f);
                if isfield(tmp,'classlabel') & ~isfield(H,'Classlabel')
                        H.Classlabel=tmp.classlabel(:);                        
                end;
        end;
        f = fullfile(H.FILE.Path, [H.FILE.Name,'_classlabel.mat']);
        if exist(f,'file'),
                tmp = load(f);
                if isfield(tmp,'Classlabel') & (size(tmp.Classlabel,2)==4)
                        [x,H.Classlabel] = max(tmp.Classlabel,[],2);                        
                end;
                if isfield(tmp,'classlabel') & (size(tmp.classlabel,2)==4)
                        [x,H.Classlabel] = max(tmp.classlabel,[],2);                        
                end;
        end;
        f=fullfile(H.FILE.Path,[H.FILE.Name,'.sel']);
        if ~exist(f,'file'),
                f=fullfile(H.FILE.Path,[H.FILE.Name,'.SEL']);
        end
        if exist(f,'file'),
                fid = fopen(f,'r');
		tmp = fread(fid,inf,'char');
		fclose(fid);
		[tmp,v] = str2double(char(tmp'));
		if ~any(v), 
            		H.ArtifactSelection = tmp(:);         
                        if any(H.ArtifactSelection>1) | (length(H.ArtifactSelection)<length(H.Classlabe))
                                sel = zeros(size(H.Classlabel));
                                sel(H.ArtifactSelection) = 1; 
                                H.ArtifactSelection = sel;
                        end;
	        end;
        end;
end;

if ~isempty(strfind(upper(MODE),'TSD'));
        f = fullfile(H.FILE.Path, [H.FILE.Name,'.tsd']);
        if ~exist(f,'file'),
                        fprintf(2,'Warning SLOAD-TSD: file %s.tsd found\n',H.FILE(1).Name,H.FILE(1).Name);
        else
                fid = fopen(f,'rb');
                tsd = fread(fid,inf,'float');
                fclose(fid);
                nc = size(signal,1)\size(tsd,1);
                if (nc == round(nc)),
                        signal = [signal, reshape(tsd,nc,size(tsd,1)/nc)'];
                else
                        fprintf(2,'Warning SLOAD: size of %s.tsd does not fit to size of %s.bkr\n',H.FILE(1).Name,H.FILE(1).Name);
                end;
        end;
end;


if (strcmp(H.TYPE,'GDF') & isempty(H.EVENT.TYP)),
        %%%%% if possible, load Reinhold's configuration files
        f = fullfile(H.FILE.Path, [H.FILE.Name,'.mat']);
        if exist(f,'file'),
                x = load(f,'header');
		if isfield(x,'header'),
	                H.BCI.Paradigm = x.header.Paradigm;
    		        if isfield(H.BCI.Paradigm,'TriggerTiming');
    		                H.TriggerOffset = H.BCI.Paradigm.TriggerTiming;
            		elseif isfield(H.BCI.Paradigm,'TriggerOnset');
                    		H.TriggerOffset = H.BCI.Paradigm.TriggerOnset;
            		end;

                        if isfield(H,'Classlabel') & isempty(H.Classlabel),
                                H.Classlabel = x.header.Paradigm.Classlabel;
                        end;
		end;
        end;
end;    


% resampling 
if ~isnan(Fs) & (H.SampleRate~=Fs);
        tmp = ~mod(H.SampleRate,Fs) | ~mod(Fs,H.SampleRate);
        tmp2= ~mod(H.SampleRate,Fs*2.56);
        if tmp,
                signal = rs(signal,H.SampleRate,Fs);
                H.EVENT.POS = H.EVENT.POS/H.SampleRate*Fs;
                if isfield(H.EVENT,'DUR');
                        H.EVENT.DUR = H.EVENT.DUR/H.SampleRate*Fs;
                end;
               H.SampleRate = Fs;
        elseif tmp2,
                x = load('resample_matrix.mat');
                signal = rs(signal,x.T256100);
                if H.SampleRate*100~=Fs*256,
                        signal = rs(signal,H.SampleRate/(Fs*2.56),1);
                end;
                H.EVENT.POS = H.EVENT.POS/H.SampleRate*Fs;
                if isfield(H.EVENT,'DUR');
                        H.EVENT.DUR = H.EVENT.DUR/H.SampleRate*Fs;
                end;
                H.SampleRate = Fs;
        else 
                fprintf(2,'Warning SLOAD: resampling %f Hz to %f Hz not implemented.\n',H.SampleRate,Fs);
        end;                
end;

