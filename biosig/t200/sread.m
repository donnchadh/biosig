function [S,HDR] = sread(HDR,NoS,StartPos)
% SREAD loads selected seconds of a signal file
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


%	$Revision: 1.49 $
%	$Id: sread.m,v 1.49 2005-03-25 11:20:22 schloegl Exp $
%	(C) 1997-2005 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

S = [];

if nargin<2, 
        NoS = inf; 
end;

if NoS<0,
        fprintf(HDR.FILE.stderr,'Error SREAD: NoS must be non-negative\n');
        return;
end;
if (nargin==3) 
        if (StartPos<0),
                fprintf(HDR.FILE.stderr,'Error SREAD: StartPos must be non-negative\n');
                return;
        end;
        tmp = HDR.SampleRate*StartPos;
        if tmp ~= round(tmp),
        %        fprintf(HDR.FILE.stderr,'Warning SREAD: StartPos yields non-integer position\n');
                StartPos = round(tmp)/HDR.SampleRate;
        end;
end;

tmp = HDR.SampleRate*NoS;
if tmp ~= round(tmp),
        %fprintf(HDR.FILE.stderr,'Warning SREAD: NoS yields non-integer position [%f, %f]\n',NoS,HDR.SampleRate);
        NoS = round(tmp)/HDR.SampleRate;
end;
STATUS = 0; 

if strcmp(HDR.TYPE,'EDF') | strcmp(HDR.TYPE,'BDF') | strcmp(HDR.TYPE,'GDF') ,
        if nargin<3,
                [S,HDR] = sdfread(HDR, NoS );
        else
                [S,HDR] = sdfread(HDR, NoS ,StartPos);
        end;
        
        
elseif strmatch(HDR.TYPE,{'BKR'}),
        if nargin==3,
                STATUS = fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.NS*StartPos*2,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        [S,count] = fread(HDR.FILE.FID,[HDR.NS,HDR.SampleRate*NoS],'int16');
        if count,
                S = S(HDR.InChanSelect,:)';
                HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;

                if any(S(:)>=HDR.DigMax) | any(S(:)<-HDR.DigMax)
                        fprintf(HDR.FILE.stderr,'Warning SREAD (BKR): range error - value is outside of interval [-DigMax, DigMax[. Overflow detection might not work correctly.\n')
                end
                S([S<=-HDR.DigMax] | [S>=(HDR.DigMax-1)] ) = -(2^15);   % mark overflow 
                
                S(S==-(2^15)) = NaN;       % Overflow detection
        end;

        
elseif strcmp(HDR.TYPE,'ACQ'),
        if nargin==3,
                STATUS = fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.AS.bpb*StartPos,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        count = 0;
        if all(HDR.GDFTYP==HDR.GDFTYP(1)) & all(HDR.SPR==HDR.SPR(1))
                [S,count] = fread(HDR.FILE.FID,[HDR.NS,HDR.SampleRate*NoS],gdfdatatype(HDR.GDFTYP(1)));
        else
                fprintf(HDR.FILE.FID,'Warning SREAD (ACQ): interleaved format not supported (yet).');
        end;
        if count,
                S = S(HDR.InChanSelect,:)';
                HDR.FILE.POS = HDR.FILE.POS + count/HDR.AS.spb;
        end;
        
elseif strmatch(HDR.TYPE,{'AIF','SND','WAV'})
        if nargin==3,
                STATUS = fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.AS.bpb*StartPos,'bof');
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;

        maxsamples = min(HDR.AS.endpos - HDR.FILE.POS, HDR.SampleRate*NoS);
        [S,count] = fread(HDR.FILE.FID,[HDR.NS,maxsamples],HDR.GDFTYP);

        S = S(HDR.InChanSelect,:)';
        HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
        
        if ~HDR.FLAG.UCAL,
                if isfield(HDR.FILE,'TYPE')
                        if HDR.FILE.TYPE==1,
                                S = mu2lin(S);
                        end;
                end;
        end;

elseif strmatch(HDR.TYPE,{'CFWB','CNT','DEMG','ISHNE','Nicolet','RG64'}),
        if nargin==3,
                STATUS = fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.AS.bpb*StartPos,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;

        maxsamples = min(HDR.SampleRate*NoS, HDR.AS.endpos-HDR.FILE.POS);
	S = []; count = 0;
	while maxsamples>0,	
    		[s,c]  = fread(HDR.FILE.FID,[HDR.NS,min(2^16,maxsamples)], gdfdatatype(HDR.GDFTYP));
		count = count + c;
		maxsamples = maxsamples - c/HDR.NS;
        	if c,
            		S = [S;s(HDR.InChanSelect,:)'];
    		end;
        end;
	HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;


elseif strcmp(HDR.TYPE,'SMA'),
        if nargin==3,
                STATUS = fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.AS.bpb*StartPos,'bof');        
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
                STATUS = fseek(HDR.FILE.FID,HDR.Block.Pos(POS+k),-1);
                
                % Read nchans and block length
                tmp = fread(HDR.FILE.FID,34+220,'uint16');
                
                %STATUS = fseek(HDR.FILE.FID,2,0);
                nchans = tmp(2); %fread(HDR.FILE.FID,1,'uint16');
                %fread(HDR.FILE.FID,1,'uint16');
                block_size = tmp(4); %fread(HDR.FILE.FID,1,'uint16');
                %ndupsamp = fread(HDR.FILE.FID,1,'uint16');
                %nrun = fread(HDR.FILE.FID,1,'uint16');
                %err_detect = fread(HDR.FILE.FID,1,'uint16');
                %nlost = fread(HDR.FILE.FID,1,'uint16');
                nevents = tmp(9); %fread(HDR.FILE.FID,1,'uint16');
                %STATUS = fseek(HDR.FILE.FID,50,0);
                
                [data,c] = fread(HDR.FILE.FID,[nchans,block_size],'int16');
                %S = [S; data(HDR.InChanSelect,:)']; 	% concatenate data blocks
                S((k-1)*HDR.SPR+(1:c/nchans),:) = data(HDR.InChanSelect,:)';
                count = count + c;
        end;
        HDR.FILE.POS = HDR.FILE.POS + NoSeg; 
        
        
elseif strcmp(HDR.TYPE,'LABVIEW'),
        if nargin==3,
                STATUS = fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.AS.bpb*StartPos,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        [S,count] = fread(HDR.FILE.FID,[HDR.NS,HDR.SampleRate*NoS],'int32');
        if count,
                S = S(HDR.InChanSelect,:)';
                HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
        end;
        
        
elseif strcmp(HDR.TYPE,'alpha'),
        if nargin==3,
                POS = HDR.SampleRate*HDR.AS.bpb*StartPos;
                if POS~=ceil(POS),
                        fprintf(HDR.FILE.stderr,'Error SREAD (alpha): starting position is non-integer\n');     
                        return;
                end
                STATUS = fseek(HDR.FILE.FID,HDR.HeadLen + POS,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        
        nr = min(HDR.SampleRate*NoS, HDR.SPR-HDR.FILE.POS)*HDR.AS.bpb;
        if (nr - round(nr)) < .01,
    		nr = round(nr);
	else
	        fprintf(HDR.FILE.stderr,'Error SREAD (alpha): can not deal with odd number of samples \n');     
                return;
        end
        
        if HDR.bits==12,
                [s,count] = fread(HDR.FILE.FID,[3,nr/3],'uint8');
                s(1,:) = s(1,:)*16 + floor(s(2,:)/16); 	
                s(3,:) = s(3,:)+ mod(s(2,:),16)*256; 	
                s = reshape(s([1,3],:),2*size(s,2),1);
                s = s - (s>=2^11)*2^12;
		nr = floor(length(s)/HDR.NS);
                S = reshape(s(1:nr*HDR.NS),HDR.NS,nr);
                count = count*2/3;
                
        elseif HDR.bits==16,
                [S,count] = fread(HDR.FILE.FID,[HDR.NS,nr],'int16');
                
        elseif HDR.bits==32,
                [S,count] = fread(HDR.FILE.FID,[HDR.NS,nr],'int32');
        end;        
        
        if count,
                S = S(HDR.InChanSelect,:)';
                HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
        end;

                
elseif strcmp(HDR.TYPE,'MIT'),
        if nargin==3,
                STATUS = fseek(HDR.FILE.FID,HDR.SampleRate*HDR.AS.bpb*StartPos,'bof');        
                tmp = HDR.SampleRate*StartPos;
                if HDR.FILE.POS~=tmp,
                        HDR.mode8.accu = zeros(1,length(HDR.InChanSelect));
                        HDR.mode8.valid= 0;
                end;
        end;

        DataLen = NoS*HDR.SampleRate/HDR.SPR;
        if HDR.VERSION == 212, 
                [A,count] = fread(HDR.FILE.FID, [1,DataLen*HDR.AS.bpb], 'uint8');  % matrix with 3 rows, each 8 bits long, = 2*12bit
		DataLen = floor(count/HDR.AS.bpb);
		if (count~= DataLen*HDR.AS.bpb) & isfinite(DataLen),
			fprintf(HDR.FILE.stderr,'Warning SREAD (MIT): non-integer block length %i,%f\n',count, DataLen*HDR.AS.bpb);
			%HDR = sseek(HDR,HDR.FILE.POS,'bof');
			%return;
                        A = A(1:DataLen*HDR.AS.bpb);
		end;
                S = [A(1:3:end) + mod(A(2:3:end),16)*256; A(3:3:end) + floor(A(2:3:end)/16)*256]; 
                clear A;
                S = S - 2^12*(S>=2^11);	% 2-th complement
                S = reshape(S,HDR.AS.spb,prod(size(S))/HDR.AS.spb)';
                
        elseif HDR.VERSION == 310, 
                [A,count] = fread(HDR.FILE.FID, [HDR.AS.bpb/2, DataLen], 'uint16'); 
                A = A'; DataLen = count/HDR.AS.bpb*2;
                for k = 1:ceil(HDR.AS.spb/3),
                        k1=3*k-2; k2=3*k-1; k3=3*k;
                        S(:,3*k-2) = floor(mod(A(:,k*2-1),2^12)/2);	
                        S(:,3*k-1) = floor(mod(A(:,k*2),2^12)/2);	
                        S(:,3*k  ) = floor(A(:,k*2-1)*(2^-11)) + floor(A(:,k*2)*(2^-11))*2^5; 
                        S = mod(S(:,1:HDR.AS.spb),2^10);
                        S = S - 2^10*(S>=2^9);	% 2-th complement
                end;
		S = S;
                
        elseif HDR.VERSION == 311, 
                [A,count] = fread(HDR.FILE.FID, [HDR.AS.bpb/4, DataLen], 'uint32');
                A = A'; DataLen = count/HDR.AS.bpb*4;
                for k = 1:ceil(HDR.AS.spb/3),
                        S(:,3*k-2) = mod(A(:,k),2^10);	
                        S(:,3*k-1) = mod(floor(A(:,k)*2^(-11)),2^10);	
                        S(:,3*k)   = mod(floor(A(:,k)*2^(-22)),2^10);	
                        S = S(:,1:HDR.AS.spb);
                        S = S - 2^10*(S>=2^9);	% 2-th complement
                end;
		S = S';
                
        elseif HDR.VERSION == 8, 
                [S,count] = fread(HDR.FILE.FID, [HDR.AS.spb,DataLen], 'int8');  
                S = S'; DataLen = count/HDR.AS.spb          
                
        elseif HDR.VERSION == 80, 
                [S,count] = fread(HDR.FILE.FID, [HDR.AS.spb,DataLen], 'uint8');  
                S = S'-128; DataLen = count/HDR.AS.spb;
                
        elseif HDR.VERSION == 160, 
                [S,count] = fread(HDR.FILE.FID, [HDR.AS.spb,DataLen], 'uint16');  
                S = S'-2^15; DataLen = count/HDR.AS.spb;
                
        elseif HDR.VERSION == 16, 
                [S,count] = fread(HDR.FILE.FID, [HDR.AS.spb,DataLen], 'int16'); 
                S = S'; DataLen = count/HDR.AS.spb;
                
        elseif HDR.VERSION == 61, 
                [S,count] = fread(HDR.FILE.FID, [HDR.AS.spb,DataLen], 'int16'); 
                S = S'; DataLen = count/HDR.AS.spb;
                
        else
                fprintf(2, 'ERROR MIT-ECG: format %i not supported.\n',HDR.VERSION); 
                
        end;
        if any(HDR.AS.SPR>1),
                A = S;
                S = zeros(size(A,1)*HDR.SPR,length(HDR.InChanSelect));
                for k = 1:length(HDR.InChanSelect),
                        ch = HDR.InChanSelect(k);
                        ix = HDR.AS.bi(ch)+1:HDR.AS.bi(ch+1);
                        S(:,k)=rs(reshape(A(:,ix)',size(A,1)*HDR.AS.SPR(ch),1),HDR.AS.SPR(ch),HDR.SPR);
                end
        else
                S = S(:,HDR.InChanSelect);
        end
        if HDR.VERSION == 8, 
                if HDR.FILE.POS==0,
                        HDR.mode8.accu = zeros(1,length(HDR.InChanSelect));
                        HDR.mode8.valid= 1;
                end; 
                if ~HDR.mode8.valid;
                        fprintf(2,'Warning SREAD: unknown offset (TYPE=MIT, mode=8) \n');
                else
                        S(1,:) = S(1,:) + HDR.mode8.accu;
                end;        
                S = cumsum(S);
                HDR.mode8.accu = S(size(S,1),:);
	end;
        HDR.FILE.POS = HDR.FILE.POS + DataLen;   	
        
        
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
                
                STATUS = fseek(HDR.FILE.FID,HDR.HeadLen+StartPos*HDR.SampleRate/HDR.SPR*(HDR.AS.bpb+86),'bof');        
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
        
        
elseif strcmp(HDR.TYPE,'EGI'),
        if nargin==3,
                STATUS = fseek(HDR.FILE.FID,HDR.HeadLen+HDR.AS.bpb*StartPos,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        
        if HDR.FLAG.TRIGGERED,
                NoS = min(NoS,(HDR.NRec-HDR.FILE.POS));
                S = zeros(NoS*HDR.SPR,length(HDR.InChanSelect))+NaN;
                for i = (1:NoS),
                        SegmentCatIndex(HDR.FILE.POS+i) = fread(HDR.FILE.FID,1,'uint16');
                        SegmentStartTime(HDR.FILE.POS+i) = fread(HDR.FILE.FID,1,'uint32');
                        
                        [s,count] = fread(HDR.FILE.FID, [HDR.NS + HDR.EVENT.N, HDR.SPR], HDR.GDFTYP);
                        tmp = (HDR.NS + HDR.EVENT.N) * HDR.SPR;
	                if isfinite(tmp) & (count < tmp),
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
                [S,count] = fread(HDR.FILE.FID,[HDR.NS + HDR.EVENT.N, HDR.SampleRate*NoS],HDR.GDFTYP);
                tmp = (HDR.NS + HDR.EVENT.N) * HDR.SampleRate * NoS;
                if isfinite(tmp) & (count < tmp),
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
        
        STATUS = fseek(HDR.FILE.FID,HDR.COH.directory(rows,cols)+8,'bof'); % skip over a small unused header of 8 bytes 
        sr = fread(HDR.FILE.FID, HDR.SPR, 'float32');  % read real part of coherence    
        si = fread(HDR.FILE.FID, HDR.SPR, 'float32');  % read imag part of coherence    
        S = sr + i * si;
        
        
elseif strcmp(HDR.TYPE,'CSA'),
        warning('.CSA data not tested yet')
        S = fread(HDR.FILE.FID, [HDR.NRec*(HDR.SPR+6)*HDR.NS], 'float32');	        
        
        
elseif strcmp(HDR.TYPE,'EEG'),
        if nargin>2,
                STATUS = fseek(HDR.FILE.FID,HDR.HeadLen+HDR.AS.bpb*StartPos,'bof');        
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
        
        
elseif strcmp(HDR.TYPE,'MFER'),
	if (HDR.FRAME.N ~= 1),
		fprintf(2,'Warning MWFOPEN: files with more than one frame not implemented, yet.\n');
		return;
	end
	
	N = 1;
	if ~isfield(HDR,'data'),
		STATUS = fseek(HDR.FILE.FID,HDR.FRAME.POS(N),'bof');
		[tmp,count] = fread(HDR.FILE.FID,HDR.FRAME.sz(N,1:2),gdfdatatype(HDR.FRAME.TYP(N)));
        	if isnan(HDR.NRec),
        		HDR.NRec = count/(HDR.SPR*HDR.NS);
        	end;

        	if count==(HDR.SPR*HDR.NS), %% alternate mode format
        		tmp = reshape(tmp,[HDR.SPR,HDR.NS]);
        	else
        	        tmp = reshape(tmp,[HDR.SPR,HDR.NS,HDR.NRec]);   % convert into 3-Dim
        	        tmp = permute(tmp,[1,3,2]);                     % re-order dimensions
        	        tmp = reshape(tmp,[HDR.SPR*HDR.NRec,HDR.NS]);   % make 2-Dim 
        	end;
		HDR.data = tmp;
	end;

	if nargin>2,
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        
        nr = min(HDR.SampleRate*NoS,size(HDR.data,1)-HDR.FILE.POS);
	S  = HDR.data(HDR.FILE.POS + (1:nr), HDR.InChanSelect);
        HDR.FILE.POS = HDR.FILE.POS + nr;
	
        
elseif strcmp(HDR.TYPE,'BCI2000'),
        if nargin==3,
                STATUS = fseek(HDR.FILE.FID,HDR.HeadLen+HDR.SampleRate*HDR.AS.bpb*StartPos,'bof');        
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        [S,count] = fread(HDR.FILE.FID,[HDR.NS,HDR.SampleRate*NoS],HDR.GDFTYP,HDR.BCI2000.StateVectorLength);
        if count,
                S = S(HDR.InChanSelect,:)';
                HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
        end;

        
elseif strmatch(HDR.TYPE,{'native','SCP'}),
	if nargin>2,
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;

	nr = min(round(HDR.SampleRate * NoS), size(HDR.data,1) - HDR.FILE.POS);
        
        S  = HDR.data(HDR.FILE.POS + (1:nr), HDR.InChanSelect);
        
        HDR.FILE.POS = HDR.FILE.POS + nr;

        
elseif strcmp(HDR.TYPE,'SIGIF'),
        if nargin==3,
                HDR.FILE.POS = StartPos;
        end;
        
        S = [];
        for k = 1:min(NoS,HDR.NRec-HDR.FILE.POS),
                HDR.FILE.POS = HDR.FILE.POS + 1;
                STATUS = fseek(HDR.FILE.FID, HDR.Block.Pos(HDR.FILE.POS), 'bof');
                if HDR.FLAG.TimeStamp,
                        HDR.Frame(k).TimeStamp = fread(HDR.FILE.FID,[1,9],'char');
                end;
                
                if HDR.FLAG.SegmentLength,
                        HDR.Block.Length(k) = fread(HDR.FILE.FID,1,'uint16');  %#26
                        STATUS = fseek(HDR.FILE.FID,HDR.Block.Length(k)*H1.Bytes_per_Sample,'cof');
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
                STATUS = fseek(HDR.FILE.FID,HDR.HeadLen+HDR.NS*HDR.SPR*4*StartPos,'bof');        
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
                STATUS = fseek(HDR.FILE.FID,HDR.SampleRate*HDR.AS.bpb,'bof');        
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

        
elseif strcmp(HDR.TYPE,'BrainVision'),   %Brainvision
        if strncmpi(HDR.BV.DataFormat, 'binary',5)
                if strncmpi(HDR.BV.DataOrientation, 'multiplexed',6),
                        if nargin>2,
                                STATUS = fseek(HDR.FILE.FID,StartPos*HDR.SampleRate*HDR.AS.bpb,'bof');        
                                HDR.FILE.POS = HDR.SampleRate*StartPos;
                        end;
                        
                        nr = min(HDR.SampleRate*NoS, HDR.AS.endpos-HDR.FILE.POS);
                        [dat, count] = fread(HDR.FILE.FID, [HDR.NS, nr], HDR.GDFTYP);
                        
                        % rename and transpose the data
                        S = dat(HDR.InChanSelect,:)';
                        HDR.FILE.POS = HDR.FILE.POS + count/HDR.NS;
                        
                elseif strncmpi(HDR.BV.DataOrientation, 'vectorized',6),
                        S = [];
                        nr = min(HDR.SampleRate*NoS, HDR.AS.endpos-HDR.FILE.POS);
                        
                        count = 0; 
                        for chan = 1:length(HDR.InChanSelect);
                                STATUS = fseek(HDR.FILE.FID, HDR.HeadLen + HDR.FILE.POS + HDR.AS.bpb*HDR.SPR*(chan-1)/HDR.NS, 'bof');
                                [s,count] = fread(HDR.FILE.FID, [nr,1], HDR.GDFTYP);
                                if count ~= nr,
                                        fprintf(2,'ERROR READ BV-bin-vec: \n');
                                        return;
                                end;    
                                S(:,chan) = s;        
                        end
                        HDR.FILE.POS = HDR.FILE.POS + count; 
                end;
        
        elseif strncmpi(HDR.BV.DataFormat, 'ascii',5)  
                if nargin>2,
                        HDR.FILE.POS = HDR.SampleRate*StartPos;
                end;
                nr = min(HDR.SampleRate*NoS, HDR.AS.endpos-HDR.FILE.POS);
                S  = HDR.BV.data(HDR.FILE.POS+(1:nr),HDR.InChanSelect);
                
        end
                
        
elseif strcmp(HDR.TYPE,'SierraECG'),   %% SierraECG  1.03  *.open.xml from PHILIPS
        if ~isfield(HDR,'data');
                [HDR.data,status] = str2double(HDR.XML.waveforms.parsedwaveforms);
                if any(status)
                        error('SREAD: compressed SierraECG (Philips) format not supported')
                end;
                HDR.data = reshape(HDR.data,length(HDR.data)/HDR.NS,HDR.NS);
                HDR.SPR = size(HDR.data,1);
        else
                % base64 - decoding 
                base64 = ['A':'Z','a':'z','0':'9','+','/'];
                decode64 = repmat(nan,256,1);
                decode64(abs(base64)) = 0:63;
                tmp = decode64(HDR.XML.waveforms.parsedwaveforms);
                tmp(isnan(tmp)) = [];
                n   = length(tmp);
                tmp = reshape([tmp;zeros(mod(n,4),1)], 4, ceil(n/4));
                t1  = tmp(1,:)*4 + floor(tmp(2,:)/16);
                t2  = mod(tmp(2,:),16)*16 + floor(tmp(3,:)/4);
                t3  = mod(tmp(3,:),4)*64 + tmp(4,:);
                tmp = reshape([t1,t2,t3], ceil(n/4)*3, 1);
        end;
        if nargin>2,
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        nr = min(HDR.SampleRate*NoS, HDR.SPR-HDR.FILE.POS);
        S  = HDR.data(HDR.FILE.POS+(1:nr),HDR.InChanSelect);
        HDR.FILE.POS = HDR.FILE.POS + nr;

        
elseif strcmp(HDR.TYPE,'TFM_EXCEL_Beat_to_Beat'); 
        if nargin>2,
		fprintf(HDR.FILE.stderr,'Warning SREAD (TFM-EXCEL): only one input argument supported.\n');
        end;
	S = HDR.TFM.S; 


elseif strcmp(HDR.TYPE,'WG1'),   %walter-graphtek
	% code from Robert Reijntjes, Amsterdam, NL 
	% modified by Alois Schloegl 19. Feb 2005 
        if nargin==3,
                HDR.FILE.POS = round(HDR.SampleRate*StartPos);
        end;

	ix1    = mod(HDR.FILE.POS, HDR.SPR);	% starting sample (minus one) within 1st block 
    	fp     = HDR.HeadLen + floor(HDR.FILE.POS/HDR.SPR)*HDR.AS.bpb;
    	status = fseek(HDR.FILE.FID, fp, 'bof');

        nr     = min(HDR.AS.endpos-HDR.FILE.POS, NoS*HDR.SampleRate);
	S      = repmat(NaN,nr,length(HDR.InChanSelect)); 
	count  = 0;
        endloop= 0;
        [offset,c] = fread(HDR.FILE.FID, HDR.WG1.szOffset, 'int32');
    	while ~endloop & (c>0) & (offset(1)~=(hex2dec('AEAE5555')-2^32)) & (count<nr);
		[databuf,c] = fread(HDR.FILE.FID,[HDR.WG1.szBlock,HDR.NS+HDR.WG1.szExtra],'uint8');
            	dt = HDR.WG1.conv(databuf(:,1:HDR.NS)+1);
            	if any(dt(:)==HDR.WG1.unknownNr),
            	    	fprintf(HDR.FILE.stderr,'Error SREAD (WG1): error in reading datastream');
            	end;
		dt(1,:) = dt(1,:) + offset(1:HDR.NS)';
		dt = cumsum(dt,1);
		
		ix2 = min(nr-count, size(dt,1)-ix1);
		S(count+1:count+ix2,:) = dt(ix1+1:ix1+ix2, HDR.InChanSelect);
		count = count + ix2; 
		ix1 = 0;	% reset starting index, 

                k = 0; 
                while (k<HDR.WG1.szExtra) & ~endloop, 
                        endloop = ~isempty(strfind(databuf(:,HDR.NS+k)',[85,85,174,174]));
                        k = k+1; 
                end;
                
	        [offset,c] = fread(HDR.FILE.FID, HDR.WG1.szOffset, 'int32');
	end;	
	%S = S(1:count,:);
	HDR.FILE.POS = HDR.FILE.POS + count;


elseif strcmp(HDR.TYPE,'XML-FDA'),   % FDA-XML Format
        if ~isfield(HDR,'data');
                tmp   = HDR.XML.component.series.derivation;
                if isfield(tmp,'Series');
                        tmp = tmp.Series.component.sequenceSet.component;
                else    % Dovermed.CO.IL version of format
                        tmp = tmp.derivedSeries.component.sequenceSet.component;
                end;
                for k = 1:length(HDR.InChanSelect);
                        HDR.data(:,k) = str2double(tmp{HDR.InChanSelect(k)+1}.sequence.value.digits)';
                end;
                HDR.SPR = size(HDR.data,1);
        end;
        if nargin>2,
                HDR.FILE.POS = HDR.SampleRate*StartPos;
        end;
        nr = min(HDR.SampleRate*NoS, HDR.SPR-HDR.FILE.POS);
        S  = HDR.data(HDR.FILE.POS+(1:nr),:);
        HDR.FILE.POS = HDR.FILE.POS + nr;

        
elseif strcmp(HDR.TYPE,'FIF'),
        % some parts of this code are from Robert Oostenveld, 
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

elseif strcmp(HDR.TYPE,'EVENT'),
        s = [];        
else
        fprintf(2,'Error SREAD: %s-format not supported yet.\n', HDR.TYPE);        
	return;
end;


%%% TOGGLE CHECK - checks whether HDR is kept consist %%% 
global SREAD_TOGGLE_CHECK
if isfield(HDR.FLAG,'TOGGLE');
        if HDR.FLAG.TOGGLE~=SREAD_TOGGLE_CHECK,
                fprintf(HDR.FILE.stderr,'Warning SREAD: [s,HDR]=sread(HDR, ...) \nYou forgot to pass HDR in %i call(s) of SREAD\n',SREAD_TOGGLE_CHECK-HDR.FLAG.TOGGLE);
        end;
else
        HDR.FLAG.TOGGLE=0;
        SREAD_TOGGLE_CHECK=0;
end;
SREAD_TOGGLE_CHECK = SREAD_TOGGLE_CHECK+1;
HDR.FLAG.TOGGLE = HDR.FLAG.TOGGLE+1;

if STATUS,
        fprintf(HDR.FILE.stderr,'WARNING SREAD: something went wrong. Please send these files %s and BIOSIGCORE to <a.schloegl@ieee.org>',HDR.FileName);
        save biosigcore.mat 
end;

if isfield(HDR,'THRESHOLD') & HDR.FLAG.OVERFLOWDETECTION,
        ix = (S~=S);
        for k=1:length(HDR.InChanSelect),
                TH = HDR.THRESHOLD(HDR.InChanSelect(k),:);
                ix(:,k) = (S(:,k)<=TH(1)) | (S(:,k)>=TH(2));
        end
        S = double(S);
        S(ix) = NaN;
elseif HDR.FLAG.OVERFLOWDETECTION,
        % no HDR.THRESHOLD defined
elseif isfield(HDR,'THRESHOLD'),
        % automated overflow detection has been turned off
end;

if ~HDR.FLAG.UCAL,
        % S = [ones(size(S,1),1),S]*HDR.Calib([1;1+HDR.InChanSelect],:); 
        % perform the previous function more efficiently and
        % taking into account some specialities related to Octave sparse
        % data. 

        if 1; %exist('OCTAVE_VERSION')
                % force octave to do a sparse multiplication
                % the difference is NaN*sparse(0) = 0 instead of NaN
                % this is important for the automatic overflow detection
		Calib = HDR.Calib;
                tmp   = zeros(size(S,1),size(Calib,2));   % memory allocation
                for k = 1:size(Calib,2),
                        chan = find(Calib(1+HDR.InChanSelect,k));
                        tmp(:,k) = double(S(:,chan)) * Calib(1+HDR.InChanSelect(chan),k) + Calib(1,k);
                end
                S = tmp; 
        else
                % S = [ones(size(S,1),1),S]*HDR.Calib([1;1+HDR.InChanSelect],:); 
                % the following is the same as above but needs less memory. 
                S = double(S) * HDR.Calib(1+HDR.InChanSelect,:);
                for k = 1:size(HDR.Calib,2),
                        S(:,k) = S(:,k) + HDR.Calib(1,k);
                end;
        end;
end;
