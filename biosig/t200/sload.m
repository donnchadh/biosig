function [signal,H] = sload(FILENAME,CHAN,MODE)
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
% [signal,header] = sload(FILENAME [,CHANNEL [,MODE]])
% FILENAME      name of file, or list of filenames
% channel       list of selected channels
%               default=0: loads all channels
% Fs            force target samplerate Fs (only 
%               integer and 256->100 conversion is supported) 
% MODE          'UCAL'  uncalibrated data
%               'OVERFLOWDETECTION:OFF' turns off automated overflow detection
%
% [signal,header] = sload(dir('f*.emg'), CHAN)
% [signal,header] = sload('f*.emg', CHAN)
%  	loads channels CHAN from all files 'f*.emg'
%
% see also: SVIEW, SOPEN, SREAD, SCLOSE, SAVE2BKR, TLOAD
%
% Reference(s):
%


%	$Revision: 1.55 $
%	$Id: sload.m,v 1.55 2005-02-12 14:59:29 schloegl Exp $
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
if nargin<3; MODE = ''; end;


if CHAN<1 | ~isfinite(CHAN),
        CHAN=0;
end;

%%% resolve wildcards %%%
if ischar(FILENAME) 
if any(FILENAME=='*')
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
end;


if ((iscell(FILENAME) | isstruct(FILENAME)) & (length(FILENAME)>1)),
	signal = [];
	for k = 1:length(FILENAME),
		if iscell(FILENAME(k))
			f = FILENAME{k};
		else 
			f = FILENAME(k);
		end	

                [s,h] = sload(f,CHAN,MODE);
		if k==1,
			H = h;
			signal = s;  
			H.SegLen = [0,size(s,1)];
		else
			H.FILE(k) = h.FILE;
			if ~isnan(h.SampleRate) & (H.SampleRate ~= h.SampleRate),
				fprintf(2,'Warning SLOAD: sampling rates of multiple files differ %i!=%i.\n',H.SampleRate, h.SampleRate);
			end;

                        if size(s,2)==size(signal,2), %(H.NS == h.NS) 
				signal = [signal; repmat(NaN,100,size(s,2)); s];
				H.SegLen = [H.SegLen,size(signal,1)];
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
if nargin<3
        MODE = '';
end;
if ~isnumeric(CHAN),
        MODE = CHAN;
        CHAN = 0; 
end
if isnumeric(MODE),
        Fs  = MODE;
else
        Fs  = NaN;
end;

signal = [];

H = getfiletype(FILENAME);
if isempty(H),
	fprintf(2,'Warning SLOAD: no file found\n');
	return;
else	
	% FILENAME can be fn.name struct, or HDR struct. 
	FILENAME = H.FileName; 
end;
    
if strncmp(H.TYPE,'IMAGE:',5)
	[H,signal] = iopen(H);
	if H.FILE.OPEN,
		signal = iread(H);
		H.FILE.OPEN = 0; 
		fclose(H.FILE.FID);
	end;
	return;
end;

H = sopen(H,'r',CHAN,MODE);
if 0,
        
elseif (H.FILE.OPEN > 0) | any(strmatch(H.TYPE,{'native','TFM_EXCEL_Beat_to_Beat'})); 
        [signal,H] = sread(H);
        H = sclose(H);


elseif strcmp(H.TYPE,'EVENTCODES')
        signal = H.EVENT;
        

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
	
        
elseif strcmp(H.TYPE,'unknown')
        TYPE = upper(H.FILE.Ext);
        if 0, 
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

