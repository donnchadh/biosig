function [S,HDR] = eegread(HDR,NoS,StartPos)
% Loads selected seconds of an EEG File
%
% [S,EEG] = eegread(EEG [,NoS [,StartPos]] )
% NoS       Number of seconds, default = 1 (second)
% StartPos  Starting position, if not provided the following data is read continously from the EDF file. 
%                    no reposition of file pointer is performed
%
% EEG=eegopen(Filename,'r',CHAN);
% [S,EEG] = eegread(EEG, NoS, StartPos)
%      	reads NoS seconds beginning at StartPos
% 
% [S,EEG] = eegread(EEG, inf) 
%      	reads til the end starting at the current position 
% 
% [S,EEG] = eegread(BKR, N*BKR.Dur) 
%	reads N trials of an BKR file 
% 
%
% See also: fread, EEGREAD, EEGWRITE, EEGCLOSE, EEGSEEK, EEGREWIND, EEGTELL, EEGEOF

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


%	$Revision: 1.14 $
%	$Id: eegread.m,v 1.14 2003-06-02 18:06:28 schloegl Exp $
%	Copyright (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	

if nargin<2, NoS = inf; end;

if strcmp(HDR.TYPE,'EDF') | strcmp(HDR.TYPE,'BDF') | strcmp(HDR.TYPE,'GDF') ,
        if nargin<3,
                [S,HDR] = sdfread(HDR, NoS );
        else
                [S,HDR] = sdfread(HDR, NoS ,StartPos);
        end;
        

elseif strmatch(HDR.TYPE,{'BKR','ISHNE','RG64'}),
        if nargin==3,
        	fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.NS*StartPos*2,'bof');        
		HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        [S,count] = fread(HDR.FILE.FID,[HDR.NS,HDR.SampleRate*NoS],'int16');
	if count,
	        S = S(HDR.SIE.InChanSelect,:)';
                HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
        end;
        if ~HDR.FLAG.UCAL,
                S = S*HDR.Cal;
        end;


elseif strcmp(HDR.TYPE,'SMA'),
        if nargin==3,
        	fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.AS.bpb*StartPos,'bof');        
		HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        
        NoS = min(NoS,(HDR.AS.endpos-HDR.FILE.POS)/HDR.SampleRate);
        [S,count] = fread(HDR.FILE.FID,[HDR.NS,HDR.SampleRate*NoS],'float'); % read data frame
        
        HDR.SMA.events = diff(sign([HDR.Filter.T0',S(HDR.SMA.EVENT_CHANNEL,:)]-HDR.SMA.EVENT_THRESH))>0;
        if size(S,2)>0,
	        HDR.Filter.T0  = S(HDR.SMA.EVENT_CHANNEL,size(S,2))';
        end;
        if count,
	        S = S(HDR.SIE.InChanSelect,:)';
                HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
        end;
       

elseif strcmp(HDR.TYPE,'RDF'),
	S = [];
        if nargin>2,
		HDR.FILE.POS = StartPos;
	end;
	POS = HDR.FILE.POS;

	NoSeg = min(NoS,length(HDR.Block.Pos)-HDR.FILE.POS);
	count = 0;
	S = zeros(NoSeg*HDR.SPR ,length(HDR.SIE.InChanSelect));

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
    		%S = [S; data(HDR.SIE.InChanSelect,:)']; 	% concatenate data blocks
		S(k*HDR.SPR+(1-HDR.SPR:0),:) = data(HDR.SIE.InChanSelect,:)';
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
	        S = S(HDR.SIE.InChanSelect,:)';
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
			S(:,2*k-1) = bitand(A(:,3*k+[-2:-1])*(2.^[0;8]),2^12-1);
			S(:,2*k)   = bitshift(bitand(A(:,3*k-1),15*16),4)+A(:,3*k);
			S = S(:,1:HDR.NS);
			S = S - 2^12*(S>=2^11);	% 2-th complement
                end
                
	elseif HDR.VERSION == 310, 
		[A,count] = fread(HDR.FILE.FID, [HDR.AS.bpb/2, DataLen], 'uint16');  % matrix with 3 rows, each 8 bits long, = 2*12bit
                A = A'; DataLen = count/HDR.AS.bpb*2;
		for k = 1:ceil(HDR.NS/3),
			k1=3*k-2; k2=3*k-1; k3=3*k;
			S(:,3*k-2) = bitand(A(:,k*2-1),2^12-2)/2;	
			S(:,3*k-1) = bitand(A(:,k*2),2^12-2)/2;	
			S(:,3*k  ) = bitshift(A(:,k*2-1),-11) + bitshift(bitshift(A(:,k*2),-11) ,5); 
			S = S(:,1:HDR.NS);
			S = S - 2^10*(S>=2^9);	% 2-th complement
		end;

	elseif HDR.VERSION == 311, 
		[A,count] = fread(HDR.FILE.FID, [HDR.AS.bpb/4, DataLen], 'uint32');  % matrix with 3 rows, each 8 bits long, = 2*12bit
                A = A'; DataLen = count/HDR.AS.bpb*4;
		for k = 1:ceil(HDR.NS/3),
			S(:,3*k-2) = bitand(A(:,k),2^11-1);	
			S(:,3*k-1) = bitand(bitshift(A(:,k),-11),2^11-1);	
			S(:,3*k)   = bitand(bitshift(A(:,k),-22),2^11-1);	
			S = S(:,1:HDR.NS);
			S = S - 2^10*(S>=2^9);	% 2-th complement
		end;

	elseif HDR.VERSION == 8, 
		[S,count] = fread(HDR.FILE.FID, [HDR.NS,DataLen], 'int8');  
		S = S'; DataLen = count/HDR.NS;               
                if HDR.FILE.POS==0,
	                HDR.mode8.accu = zeros(1,HDR.NS);
                        HDR.mode8.reset= 0;
                end; 
                if HDR.mode8.reset;
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
        
        if ~HDR.FLAG.UCAL,
		for k = 1:HDR.NS,
			S(:,k) = (S(:,k) - HDR.zerovalue(k))/HDR.gain(k);
		end;
	end;
	S = S(:,HDR.SIE.InChanSelect);
        

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
		if HDR.GDFTYP(1)==3,
			[s,c] = fread(HDR.FILE.FID,[HDR.NS,HDR.SPR],'int16');
		elseif HDR.GDFTYP(1)==3,
			[s,c] = fread(HDR.FILE.FID,[HDR.NS,HDR.SPR],'float32');
		end;
		S = [S;s'];
		HDR.FILE.POS = HDR.FILE.POS + 1;
	else
		hdr = fread(HDR.FILE.FID,86,'char');
		s = zeros(HDR.SPR,HDR.NS);
		for k1 = 1: HDR.SPR,
		for k2 = 1: HDR.NS,
			if HDR.GDFTYP(k2)==3,
			        [s(k1,k2),count] = fread(HDR.FILE.FID,1,'int16');
			elseif HDR.GDFTYP(k2)==16,
		        	[s(k1,k2),count] = fread(HDR.FILE.FID,1,'float32');
			end;	
		end;
		end;
		S = [S;s'];
		HDR.FILE.POS = HDR.FILE.POS + 1;
	end;
	end;
	
        if ~HDR.FLAG.UCAL,
                S = [ones(size(S,1),1),S]*HDR.Calib(:,HDR.InChanSelect);
	else
		S = S(:,HDR.InChanSelect);	
        end;

        
elseif strcmp(HDR.TYPE,'EGI'),
        if nargin==3,
                fseek(HDR.FILE.FID,HDR.HeadLen+HDR.AS.bpb*StartPos,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        
        if HDR.FLAG.TRIGGERED,
                NoS = min(NoS,(HDR.NRec-HDR.FILE.POS));
                S = zeros(NoS*HDR.SPR,length(HDR.SIE.InChanSelect));
                for i = (1:NoS),
                        SegmentCatIndex(HDR.FILE.POS+i)  = fread(HDR.FILE.FID,1,'integer*2');
                        SegmentStartTime(HDR.FILE.POS+i) = fread(HDR.FILE.FID,1,'integer*4');
                        [s,count] = fread(HDR.FILE.FID, [HDR.NS + HDR.eventtypes, HDR.SPR], HDR.datatype);
                        if count>0, 
                                HDR.FILE.POS = HDR.FILE.POS + 1;
                        end;
                        
                        if (HDR.eventtypes > 0),
                                for k=HDR.NS+1:size(S,1)
                                        HDR.EventData{k-HDR.NS} = [HDR.EventData{k-HDR.NS};find(S(k,:))'];
                                end;
                        end 
                        %S = [S;s(HDR.SIE.InChanSelect,:)'];
                        S(i*HDR.SPR+(1-HDR.SPR:0),:) = s(HDR.SIE.InChanSelect,:)';
                end;
                
        else
                %NoS = min(NoS,(HDR.SPR-HDR.FILE.POS)/HDR.SampleRate)
                [S,count] = fread(HDR.FILE.FID,[HDR.NS + HDR.eventtypes, HDR.SampleRate*NoS],HDR.datatype);
                HDR.FILE.POS = HDR.FILE.POS + round(count/(HDR.NS + HDR.eventtypes));
                
                if (HDR.eventtypes > 0),
                        for k=HDR.NS+1:size(S,1)
                                HDR.EventData{k-HDR.NS} = [HDR.EventData{k-HDR.NS};find(S(k,:))'];
                        end;
                end 
                S = S(HDR.SIE.InChanSelect,:)';
        end;
        
	% convert from A/D units to microvolts
	if ( HDR.bits ~= 0 & HDR.PhysMax ~= 0 )
	       S = (HDR.PhysMax/HDR.DigMax)*S;
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
        if ~HDR.FLAG.UCAL,
		S = S*diag(HDR.Cal);
        end;
	HDR.FILE.POS = HDR.FILE.POS + count/HDR.AS.bpb;

                    
elseif strcmp(HDR.TYPE,'COH'),
        warning('.COH data not supported yet')
        
elseif strcmp(HDR.TYPE,'CSA'),
        warning('.CSA data not supported yet')
        
elseif strcmp(HDR.TYPE,'EEG'),
        if nargin>2,
                fseek(HDR.FILE.FID,HDR.HeadLen+HDR.AS.bpb*StartPos,'bof');        
        end;
        
        NoS = min(NoS,HDR.NRec-HDR.FILE.POS);
        S   = zeros(NoS*HDR.SPR,length(HDR.SIE.InChanSelect));
        count = 0;
        for i = 1:NoS,%h.compsweeps,
                h.sweep(i).accept     = fread(HDR.FILE.FID,1,'uchar');
                h.sweep(i).ttype      = fread(HDR.FILE.FID,1,'ushort');
                h.sweep(i).correct    = fread(HDR.FILE.FID,1,'ushort');
                h.sweep(i).rt         = fread(HDR.FILE.FID,1,'float32');
                h.sweep(i).response   = fread(HDR.FILE.FID,1,'ushort');
                h.sweep(i).reserved   = fread(HDR.FILE.FID,1,'ushort');
                
                [signal,c] = fread(HDR.FILE.FID,[HDR.NS,HDR.SPR],HDR.GDFTYP);
                
                % S = [S;signal(HDR.SIE.InChanSelect,:)'];
                S(i*HDR.SPR+(1-HDR.SPR:0),:) = signal(HDR.SIE.InChanSelect,:)';
                count = count + c;			
        end;
        HDR.FILE.POS = HDR.FILE.POS + count/HDR.AS.spb;        
        if ~HDR.FLAG.UCAL,
                S = [ones(size(S,1),1),S]*HDR.Calib([1,1+HDR.SIE.InChanSelect],HDR.SIE.ChanSelect);
        end;
        
elseif strcmp(HDR.TYPE,'CNT'),
        if nargin>2,
                fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.NS*StartPos*2,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        
        [S,count] = fread(HDR.FILE.FID, [HDR.NS, min(HDR.SampleRate*NoS, HDR.AS.endpos-HDR.FILE.POS)], 'int16');
        
	S = S(HDR.SIE.InChanSelect,:)';
        if count==0,
                S = [];	% Octave 2.1.40 returns size(S)==[0,1], therefore the next line would fail
        else
                if ~HDR.FLAG.UCAL,
                        S = [ones(size(S,1),1),S]*HDR.Calib([1,1+HDR.SIE.InChanSelect],HDR.SIE.ChanSelect);
                end;
                HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
        end;
        
else
	fprintf(2,'Error EEGREAD: %s-format not supported yet.\n', HDR.TYPE);        
end;


