function [S,HDR] = sread(HDR,NoS,StartPos)
% Loads selected seconds of an Signal file
%
% [S,HDR] = sread(HDR [,NoS [,StartPos]] )
% NoS       Number of seconds, default = 1 (second)
% StartPos  Starting position, if not provided the following data is read continously from the EDF file. 
%                    no reposition of file pointer is performed
%
% HDR=sopen(Filename,'r',CHAN);
% [S,HDR] = sread(HDR, NoS, StartPos)
%      	reads NoS seconds beginning at StartPos
% 
% [S,HDR] = sread(HDR, inf) 
%      	reads til the end starting at the current position 
% 
% [S,HDR] = sread(HDR, N*HDR.Dur) 
%	reads N trials of an BKR file 
% 
%
% See also: fread, SREAD, SWRITE, SCLOSE, SSEEK, SREWIND, STELL, SEOF

% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 2
% of the License, or (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


%	$Revision: 1.18 $
%	$Id: sread.m,v 1.18 2004-04-18 22:17:19 schloegl Exp $
%	Copyright (c) 1997-2004 by Alois Schloegl
%	a.schloegl@ieee.org	

if nargin<2, NoS = inf; end;

if strcmp(HDR.TYPE,'EDF') | strcmp(HDR.TYPE,'BDF') | strcmp(HDR.TYPE,'GDF') ,
        if nargin<3,
                [S,HDR] = sdfread(HDR, NoS );
        else
                [S,HDR] = sdfread(HDR, NoS ,StartPos);
        end;
        
        
elseif strmatch(HDR.TYPE,{'BKR'}),
        if nargin==3,
                fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.NS*StartPos*2,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        [S,count] = fread(HDR.FILE.FID,[HDR.NS,HDR.SampleRate*NoS],'int16');
        if count,
                S = S(HDR.InChanSelect,:)';
                HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
                S(S==HDR.SIE.THRESHOLD(1)) = NaN;       % Overflow detection
        end;
        
        
elseif strmatch(HDR.TYPE,{'ISHNE','RG64'}),
        if nargin==3,
                fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.NS*StartPos*2,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        [S,count] = fread(HDR.FILE.FID,[HDR.NS,HDR.SampleRate*NoS],'int16');
        if count,
                S = S(HDR.InChanSelect,:)';
                HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
        end;
        
        
elseif strcmp(HDR.TYPE,'SMA'),
        if nargin==3,
                fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.AS.bpb*StartPos,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        tmp = min(NoS*HDR.SampleRate,(HDR.AS.endpos-HDR.FILE.POS));
        [S,count] = fread(HDR.FILE.FID,[HDR.NS,tmp],'float32'); % read data frame
        tmp = HDR.NS*tmp;
        if count < tmp,
                fprintf(HDR.FILE.stderr,'Warning SREAD SMA: only %i out of %i samples read\n',count/HDR.NS,tmp/HDR.NS);
        end;
        S = S(HDR.InChanSelect,:)';
        HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
        
        HDR.SMA.events = diff(sign([HDR.Filter.T0',S(HDR.SMA.EVENT_CHANNEL,:)]-HDR.SMA.EVENT_THRESH))>0;
        HDR.EVENT.POS = find(HDR.SMA.events);
        HDR.EVENT.TYP = HDR.SMA.events(HDR.EVENT.POS);
        HDR.EVENT.N = length(HDR.EVENT.POS);
        
        if size(S,2) > 0,
                HDR.Filter.T0 = S(HDR.SMA.EVENT_CHANNEL,size(S,2))';
        end;
        
        
elseif strcmp(HDR.TYPE,'RDF'),
        S = [];
        if nargin>2,
                HDR.FILE.POS = StartPos;
        end;
        POS = HDR.FILE.POS;
        
        NoSeg = min(NoS,length(HDR.Block.Pos)-HDR.FILE.POS);
        count = 0;
        S = zeros(NoSeg*HDR.SPR, length(HDR.InChanSelect));
        
        for k = 1:NoSeg,
                fseek(HDR.FILE.FID,HDR.Block.Pos(POS+k),-1);
                
                % Read nchans and block length
                tmp = fread(HDR.FILE.FID,34+220,'uint16');
                
                %fseek(HDR.FILE.FID,2,0);
                nchans = tmp(2); %fread(HDR.FILE.FID,1,'uint16');
                %fread(HDR.FILE.FID,1,'uint16');
                block_size = tmp(4); %fread(HDR.FILE.FID,1,'uint16');
                %ndupsamp = fread(HDR.FILE.FID,1,'uint16');
                %nrun = fread(HDR.FILE.FID,1,'uint16');
                %err_detect = fread(HDR.FILE.FID,1,'uint16');
                %nlost = fread(HDR.FILE.FID,1,'uint16');
                nevents = tmp(9); %fread(HDR.FILE.FID,1,'uint16');
                %fseek(HDR.FILE.FID,50,0);
                
                [data,c] = fread(HDR.FILE.FID,[nchans,block_size],'int16');
                %S = [S; data(HDR.InChanSelect,:)']; 	% concatenate data blocks
                S((k-1)*HDR.SPR+(1:c/nchans),:) = data(HDR.InChanSelect,:)';
                count = count + c;
        end;
        HDR.FILE.POS = HDR.FILE.POS + NoSeg; 
        
        
elseif strcmp(HDR.TYPE,'LABVIEW'),
        if nargin==3,
                fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.AS.bpb*StartPos,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        [S,count] = fread(HDR.FILE.FID,[HDR.NS,HDR.SampleRate*NoS],'int32');
        if count,
                S = S(HDR.InChanSelect,:)';
                HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
        end;
        
        
elseif strcmp(HDR.TYPE,'MIT'),
        if nargin==3,
                fseek(HDR.FILE.FID,HDR.SampleRate*HDR.AS.bpb*StartPos,'bof');        
                tmp = HDR.SampleRate*StartPos;
                if HDR.FILE.POS~=tmp,
                        HDR.mode8.accu = zeros(1,HDR.NS);
                        HDR.mode8.valid= 0;
                end;
        end;
        if HDR.FILE.POS==0,
                HDR.mode8.accu = zeros(1,HDR.NS);
                HDR.mode8.valid= 1;
        end;
        
        DataLen = NoS*HDR.SampleRate;
        if HDR.VERSION == 212, 
                [A,count] = fread(HDR.FILE.FID, [HDR.AS.bpb, DataLen], 'uint8');  % matrix with 3 rows, each 8 bits long, = 2*12bit
                A = A'; DataLen = count/HDR.AS.bpb;
                for k = 1:ceil(HDR.NS/2),
                        S(:,2*k-1) = mod(A(:,3*k+[-2:-1])*(2.^[0;8]),2^12);
                        S(:,2*k)   = mod(floor(A(:,3*k-1)/16),16)*256+A(:,3*k);
                        S = S(:,1:HDR.NS);
                        S = S - 2^12*(S>=2^11);	% 2-th complement
                end
                
        elseif HDR.VERSION == 310, 
                [A,count] = fread(HDR.FILE.FID, [HDR.AS.bpb/2, DataLen], 'uint16');  % matrix with 3 rows, each 8 bits long, = 2*12bit
                A = A'; DataLen = count/HDR.AS.bpb*2;
                for k = 1:ceil(HDR.NS/3),
                        k1=3*k-2; k2=3*k-1; k3=3*k;
                        S(:,3*k-2) = floor(mod(A(:,k*2-1),2^12)/2);	
                        S(:,3*k-1) = floor(mod(A(:,k*2),2^12)/2);	
                        S(:,3*k  ) = floor(A(:,k*2-1)*(2^-11)) + floor(A(:,k*2)*(2^-11))*2^5; 
                        S = S(:,1:HDR.NS);
                        S = S - 2^10*(S>=2^9);	% 2-th complement
                end;
                
        elseif HDR.VERSION == 311, 
                [A,count] = fread(HDR.FILE.FID, [HDR.AS.bpb/4, DataLen], 'uint32');  % matrix with 3 rows, each 8 bits long, = 2*12bit
                A = A'; DataLen = count/HDR.AS.bpb*4;
                for k = 1:ceil(HDR.NS/3),
                        S(:,3*k-2) = mod(A(:,k),2^11);	
                        S(:,3*k-1) = mod(floor(A(:,k)*2^(-11)),2^11);	
                        S(:,3*k)   = mod(floor(A(:,k)*2^(-22)),2^11);	
                        S = S(:,1:HDR.NS);
                        S = S - 2^10*(S>=2^9);	% 2-th complement
                end;
                
        elseif HDR.VERSION == 8, 
                [S,count] = fread(HDR.FILE.FID, [HDR.NS,DataLen], 'int8');  
                S = S'; DataLen = count/HDR.NS;               
                if HDR.FILE.POS==0,
                        HDR.mode8.accu = zeros(1,HDR.NS);
                        HDR.mode8.valid= 1;
                end; 
                if ~HDR.mode8.valid;
                        fprintf(2,'Warning EDFREAD: unknown offset (TYPE=MIT, mode=8) \n');
                else
                        S(1,:) = S(1,:) + HDR.mode8.accu;
                end;        
                S = cumsum(S);
                HDR.mode8.accu = S(size(S,1),:);
                
        elseif HDR.VERSION == 80, 
                [S,count] = fread(HDR.FILE.FID, [HDR.NS,DataLen], 'uint8');  
                S = S'-128; DataLen = count/HDR.NS;
                
        elseif HDR.VERSION == 160, 
                [S,count] = fread(HDR.FILE.FID, [HDR.NS,DataLen], 'uint16');  
                S = S'-2^15; DataLen = count/HDR.NS;
                
        elseif HDR.VERSION == 16, 
                [S,count] = fread(HDR.FILE.FID, [HDR.NS,DataLen], 'int16'); 
                S = S'; DataLen = count/HDR.NS;
                
        elseif HDR.VERSION == 61, 
                [S,count] = fread(HDR.FILE.FID, [HDR.NS,DataLen], 'int16'); 
                S = S'; DataLen = count/HDR.NS;
                
        else
                fprintf(2, 'ERROR MIT-ECG: format %i not supported.\n',HDR.VERSION); 
                
        end;
        HDR.FILE.POS = HDR.FILE.POS + DataLen;   	
        S = S(:,HDR.InChanSelect);
        
        
elseif strcmp(HDR.TYPE,'TMS32'),
        tmp = NoS*HDR.SampleRate/HDR.SPR;
        if tmp~=round(tmp)	
                fprintf(2,'ERROR: NoS %f is not multiple of TMS32-blocksize %f. This is not supported yet.\n',NoS,HDR.SPR/HDR.SampleRate);
                return;
        end;
        NoBlks = min(tmp,HDR.NRec-HDR.FILE.POS);
        
        if nargin==3,
                tmp = StartPos*HDR.SampleRate/HDR.SPR;
                if tmp~=round(tmp)	
                        fprintf(2,'ERROR: StartPos %f is not multiple of TMS32-blocksize %f. This is not supported yet.\n',StartPos,HDR.SPR/HDR.SampleRate);
                        return;
                end;
                
                fseek(HDR.FILE.FID,HDR.HeadLen+StartPos*HDR.SampleRate/HDR.SPR*(HDR.AS.bpb+86),'bof');        
                HDR.FILE.POS = HDR.SampleRate/HDR.SPR*StartPos;
        end;
        
        S = [];
        for k = 1:NoBlks, 
                if all(HDR.GDFTYP==HDR.GDFTYP(1))
                        hdr = fread(HDR.FILE.FID,86,'char');
                        [s,c] = fread(HDR.FILE.FID,[HDR.NS,HDR.SPR],gdfdatatype(HDR.GDFTYP(1)));
                        S = [S;s'];
                        HDR.FILE.POS = HDR.FILE.POS + 1;
                else
                        hdr = fread(HDR.FILE.FID,86,'char');
                        s = zeros(HDR.SPR,HDR.NS);
                        for k1 = 1: HDR.SPR,
                                for k2 = 1: HDR.NS,
                                        [s(k1,k2),count] = fread(HDR.FILE.FID,1,gdfdatatype(HDR.GDFTYP(k2)));
                                end;
                        end;
                        S = [S;s'];
                        HDR.FILE.POS = HDR.FILE.POS + 1;
                end;
        end;
        S = S(:,HDR.InChanSelect);	
        
        
elseif strcmp(HDR.TYPE,'DEMG'),
        if nargin==3,
                fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.AS.bpb*StartPos,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        [S,count] = fread(HDR.FILE.FID,[HDR.NS,HDR.SampleRate*NoS],gdfdatatype(HDR.GDFTYP));
        if count,
                S = S(HDR.InChanSelect,:)';
                HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
        end;
        
        
elseif strcmp(HDR.TYPE,'CFWB'),
        if nargin==3,
                fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.AS.bpb*StartPos,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        [S,count] = fread(HDR.FILE.FID,[HDR.NS,HDR.SampleRate*NoS],gdfdatatype(HDR.GDFTYP));
        if count,
                S = S(HDR.InChanSelect,:)';
                HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
        end;
        
        
elseif strcmp(HDR.TYPE,'AIF') | strcmp(HDR.TYPE,'SND') | strcmp(HDR.TYPE,'WAV'),
        if nargin==3,
                fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.AS.bpb*StartPos,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        maxsamples = min(HDR.SPR,HDR.SampleRate*NoS)-HDR.FILE.POS;
        if maxsamples>0,
                [S,count] = fread(HDR.FILE.FID,[HDR.NS,maxsamples],gdfdatatype(HDR.GDFTYP));
        else
                S = []; count = 0;
        end;	
        S = S(HDR.InChanSelect,:)';
        HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
        
        if ~HDR.FLAG.UCAL,
                if isfield(HDR.FILE,'TYPE')
                        if HDR.FILE.TYPE==1,
                                S = mu2lin(S);
                        end;
                end;
        end;
        
        
elseif strcmp(HDR.TYPE,'EGI'),
        if nargin==3,
                fseek(HDR.FILE.FID,HDR.HeadLen+HDR.AS.bpb*StartPos,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        
        if HDR.FLAG.TRIGGERED,
                NoS = min(NoS,(HDR.NRec-HDR.FILE.POS));
                S = zeros(NoS*HDR.SPR,length(HDR.InChanSelect))+NaN;
                for i = (1:NoS),
                        SegmentCatIndex(HDR.FILE.POS+i) = fread(HDR.FILE.FID,1,'uint16');
                        SegmentStartTime(HDR.FILE.POS+i) = fread(HDR.FILE.FID,1,'uint32');
                        
                        [s,count] = fread(HDR.FILE.FID, [HDR.NS + HDR.EVENT.N, HDR.SPR], HDR.datatype);
                        tmp = (HDR.NS + HDR.EVENT.N) * HDR.SPR;
                        if count < tmp,
                                fprintf(HDR.FILE.stderr,'Warning SREAD EGI: only %i out of %i samples read\n',count,tmp);
                        end;
                        HDR.FILE.POS = HDR.FILE.POS + count/tmp;
                        
                        if (HDR.EVENT.N > 0),
                                [HDR.EVENT.POS,HDR.EVENT.CHN,HDR.EVENT.TYP] = find(s(HDR.NS+1:size(s,1),:)');
                                HDR.EVENT.N = length(HDR.EVENT.POS);
                        end 
                        S((i-1)*HDR.SPR + (1:size(s,2)),:) = s(HDR.InChanSelect,:)';
                end;
        else
                [S,count] = fread(HDR.FILE.FID,[HDR.NS + HDR.EVENT.N, HDR.SampleRate*NoS],HDR.datatype);
                tmp = (HDR.NS + HDR.EVENT.N) * HDR.SampleRate * NoS;
                if count < tmp,
                        fprintf(HDR.FILE.stderr,'Warning SREAD EGI: only %i out of %i samples read\n',count,tmp);
                end;
                HDR.FILE.POS = HDR.FILE.POS + round(count/(HDR.NS + HDR.EVENT.N));
                
                if (HDR.EVENT.N > 0),
                        [HDR.EVENT.POS,HDR.EVENT.CHN,HDR.EVENT.TYP] = find(S(HDR.NS+1:size(S,1),:)');
                        HDR.EVENT.N = length(HDR.EVENT.POS);
                end 
                S = S(HDR.InChanSelect,:)';
        end;
        
        
elseif strcmp(HDR.TYPE,'AVG'),
        S = repmat(nan,HDR.SPR,HDR.NS);
        count = 0;
        for i = 1:HDR.NS, 
                [tmp,c]     = fread(HDR.FILE.FID,5,'char'); % no longer used 
                count = count + c;
                [S(:,i), c] = fread(HDR.FILE.FID,HDR.SPR,'float');
                count = count + c*4;
        end
        S = S(:,HDR.InChanSelect);
        HDR.FILE.POS = HDR.FILE.POS + count/HDR.AS.bpb;
        
        
elseif strcmp(HDR.TYPE,'COH'),
        warning('.COH data not tested yet')
        if prod(size(NoS))==1 & nargin>2, 
                rows = NoS; cols = StartPos;
        elseif prod(size(NoS))==2
                rows = NoS(1); cols = NoS(2);
        else
                fprintf(HDR.FILE.stderr,'Error SREAD mode=COH: invalid arguments.\n');
        end;
        
        fseek(HDR.FILE.FID,HDR.COH.directory(rows,cols)+8,'bof'); % skip over a small unused header of 8 bytes 
        sr = fread(HDR.FILE.FID, HDR.SPR, 'float32');  % read real part of coherence    
        si = fread(HDR.FILE.FID, HDR.SPR, 'float32');  % read imag part of coherence    
        S = sr + i * si;
        
        
elseif strcmp(HDR.TYPE,'CSA'),
        warning('.CSA data not tested yet')
        S = fread(HDR.FILE.FID, [HDR.NRec*(HDR.SPR+6)*HDR.NS], 'float32');	        
        
        
elseif strcmp(HDR.TYPE,'EEG'),
        if nargin>2,
                fseek(HDR.FILE.FID,HDR.HeadLen+HDR.AS.bpb*StartPos,'bof');        
        end;
        
        NoS = min(NoS, HDR.NRec-HDR.FILE.POS);
        S   = zeros(NoS*HDR.SPR, length(HDR.InChanSelect));
        count = 0;
        for i = 1:NoS, %h.compsweeps,
                h.sweep(i).accept   = fread(HDR.FILE.FID,1,'uchar');
                tmp		    = fread(HDR.FILE.FID,2,'ushort');
                h.sweep(i).ttype    = tmp(1);
                h.sweep(i).correct  = tmp(2);
                h.sweep(i).rt       = fread(HDR.FILE.FID,1,'float32');
                tmp  		    = fread(HDR.FILE.FID,2,'ushort');
                h.sweep(i).response = tmp(1);
                h.sweep(i).reserved = tmp(2);
                
                [signal,c] = fread(HDR.FILE.FID, [HDR.NS,HDR.SPR], gdfdatatype(HDR.GDFTYP));
                
                S(i*HDR.SPR+(1-HDR.SPR:0),:) = signal(HDR.InChanSelect,:)';
                count = count + c;
        end;
        HDR.FILE.POS = HDR.FILE.POS + count/HDR.AS.spb;        
        
        
elseif strcmp(HDR.TYPE,'CNT'),
        if nargin>2,
                fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.NS*StartPos*2,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        
        [S,count] = fread(HDR.FILE.FID, [HDR.NS, min(HDR.SampleRate*NoS, HDR.AS.endpos-HDR.FILE.POS)], 'int16');
        
        S = S(HDR.InChanSelect,:)';
        HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
        
        
elseif strcmp(HDR.TYPE,'SIGIF'),
        if nargin==3,
                HDR.FILE.POS = StartPos;
        end;
        
        S = [];
        for k = 1:min(NoS,HDR.NRec-HDR.FILE.POS),
                HDR.FILE.POS = HDR.FILE.POS + 1;
                fseek(HDR.FILE.FID, HDR.Block.Pos(HDR.FILE.POS), 'bof');
                if HDR.FLAG.TimeStamp,
                        HDR.Frame(k).TimeStamp = fread(HDR.FILE.FID,[1,9],'char');
                end;
                
                if HDR.FLAG.SegmentLength,
                        HDR.Block.Length(k) = fread(HDR.FILE.FID,1,'uint16');  %#26
                        fseek(HDR.FILE.FID,HDR.Block.Length(k)*H1.Bytes_per_Sample,'cof');
                else
                        tmp = HDR.Segment_separator-1;
                        [dat,c] = fread(HDR.FILE.FID,[HDR.NS,HDR.Block.Length/HDR.NS],HDR.GDFTYP);
                        [tmpsep,c] = fread(HDR.FILE.FID,1,HDR.GDFTYP);
                        
                        if  (tmpsep~=HDR.Segment_separator);
                                fprintf(HDR.FILE.stderr,'Error SREAD Type=SIGIF: blockseparator not found\n');
                        end;
                end;
                S = [S; dat(HDR.InChanSelect,:)'];
        end;
        
        
elseif strcmp(HDR.TYPE,'CTF'),
        if nargin>2,
                fseek(HDR.FILE.FID,HDR.HeadLen+HDR.NS*HDR.SPR*4*StartPos,'bof');        
                HDR.FILE.POS = StartPos;
        end;
	
	nr = min(NoS, HDR.NRec - HDR.FILE.POS);
	
        S = []; count = 0; 
	for k = 1:nr,
	        %[tmp,c] = fread(HDR.FILE.FID, 1, 'int32')
	        [s,c] = fread(HDR.FILE.FID, [HDR.SPR, HDR.NS], 'int32');
		S = [S; s(:,HDR.InChanSelect)];
		count = count + c;
	end;
	
        HDR.FILE.POS = HDR.FILE.POS + count/(HDR.SPR*HDR.NS);
        
        
elseif strcmp(HDR.TYPE,'EEProbe-CNT'),
        if nargin>2,
                fseek(HDR.FILE.FID,HDR.SampleRate*HDR.AS.bpb,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        
        nr = min(HDR.SampleRate*NoS, HDR.SPR-HDR.FILE.POS);
        try
                % it appears to be a EEProbe file with continuous EEG data
                tmp = read_eep_cnt(HDR.FileName, HDR.FILE.POS+1, HDR.FILE.POS+nr);
                HDR.FILE.POS = HDR.FILE.POS + nr;
                S = tmp.data(HDR.InChanSelect,:)';
        catch
                fprintf(HDR.FILE.stderr,'ERROR SREAD (EEProbe): Cannot open EEProbe-file, because read_eep_cnt.mex not installed. \n');
                fprintf(HDR.FILE.stderr,'ERROR SREAD (EEProbe): You can downlad it from http://www.smi.auc.dk/~roberto/eeprobe/\n');
        end

        
elseif strcmp(HDR.TYPE,'EEProbe-AVR'),
        if nargin>2,
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;

        nr = min(HDR.SPR-HDR.FILE.POS,NoS*HDR.SampleRate);

        S = HDR.EEP.data(HDR.FILE.POS+(1:nr),:);
        HDR.FILE.POS = HDR.FILE.POS + nr;

        
elseif strcmp(HDR.TYPE,'BVbinmul'), %Brainvision, binary, multiplexed
        if nargin>2,
                fseek(HDR.FILE.FID,HDR.SampleRate*HDR.AS.bpb,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        
        nr = min(HDR.SampleRate*NoS, HDR.AS.endpos-HDR.FILE.POS);
        [dat, count] = fread(HDR.FILE.FID, [HDR.NS, nr], gdfdatatype(HDR.GDFTYP));
        
        % rename and transpose the data
        S = dat(HDR.InChanSelect,:)';
        HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;

        
elseif strcmp(HDR.TYPE,'BVbinvec'), %Brainvision, binary, vectorized
        S = [];
        
        nr = min(HDR.SampleRate*NoS, HDR.AS.endpos-HDR.FILE.POS);

        for chan = 1:length(HDR.InChanSelect);
                fseek(HDR.FILE.FID,HDR.HeadLen + HDR.FILE.POS + HDR.AS.bpb/HDR.NS * HDR.SPR,'bof');
                [s,count] = fread(HDR.FILE.FID,[nr,1],gdfdatatype(HDR.GDFTYP));
                if count~=nr,
                        fprintf(2,'ERROR READ BV-bin-vec: \n');
                        return;
                end;    
                S(:,chan) = s;        
        end
        HDR.FILE.POS = HDR.FILE.POS + count; 

        
elseif strcmp(HDR.TYPE,'BVascii'), %Brainvision, ascii
        if nargin>2,
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        nr = min(HDR.SampleRate*NoS, HDR.AS.endpos-HDR.FILE.POS);
        S  = HDR.BV.data(HDR.FILE.POS+(1:nr),HDR.InChanSelect);
        
        
elseif strcmp(HDR.TYPE,'BrainVision'),   %Brainvision, unknown
        error('SREAD (BrainVision): unsupported fileformat for data');

        
elseif strcmp(HDR.TYPE,'FIF'),
        %%%% #### FIX ME ####
        % some parts of this code is from Robert Oostenveld, 
        if ~(exist('rawdata')==3 & exist('channames')==3)
                error('cannot find Neuromag import routines on your Matlab path (see http://boojum.hut.fi/~kuutela/meg-pd)');
        end
        if nargin<3, 
                StartPos = HDR.FILE.POS/HDR.SampleRate;
        end
        if nargin>2,
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;

        t1 = rawdata('goto', HDR.FILE.POS/HDR.SPR);
        t2 = t1;
        dat = [];
        count = 0;
        status = 'ok';
        
        while (t2<(StartPos + NoS)) & ~strcmp(status,'eof'),
                [buf, status] = rawdata('next'); 
                if ~strcmp(status, 'ok')
                        error('error reading selected data from fif-file');
                else
                        count = count + size(buf,2);
                        dat = [dat; buf(HDR.InChanSelect,:)'];
                end
                t2 = rawdata('t');
        end

        t = t1*HDR.SampleRate+1:t2*HDR.SampleRate;
        ix = (t>StartPos*HDR.SampleRate) & (t<=(StartPos+NoS)*HDR.SampleRate);
        S = dat(ix,HDR.InChanSelect);
        HDR.FILE.POS = t2*HDR.SampleRate;        
        
else
        fprintf(2,'Error SREAD: %s-format not supported yet.\n', HDR.TYPE);        
end;


if ~HDR.FLAG.UCAL,
        if exist('OCTAVE_VERSION')
                % force octave to do a sparse multiplication
                % the difference is NaN*sparse(0) = 0 instead of NaN
                % this is important for the automatic overflow detection

		Calib = full(HDR.Calib);   % Octave can not index structed sparse matrix 
                tmp = zeros(size(S,1),size(Calib,2));
                for k = 1:size(Calib,2),
                        chan = find(Calib(1+HDR.InChanSelect,k));
                        tmp(:,k)=[ones(size(S,1),1),S(:,chan)]*Calib([1;1+HDR.InChanSelect(chan)],k);
                end
                S = tmp; 
        else
                S = [ones(size(S,1),1),S]*HDR.Calib([1;1+HDR.InChanSelect],:);
        end;
end;
