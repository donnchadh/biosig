function [HDR]=sseek(HDR,offset,origin)
% SSEEK repositions file position indicator
% [HDR]=sseek(HDR,offset,origin)
%
% The meaning of "offset" and "origin" differs for different formats. 
% 	EDF: number of (EDF) records. 
%	BKR: number of samplepoints	
%
% See also: SOPEN, SREAD, SWRITE, SCLOSE, SSEEK, SREWIND, STELL, SEOF

%	$Revision: 1.11 $
%	$Id: sseek.m,v 1.11 2004-12-28 20:35:12 schloegl Exp $
%	Copyright (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	

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



if strcmp(origin,'bof')
	origin = -1;        
elseif strcmp(origin,'cof')
	origin = 0;        
elseif strcmp(origin,'eof')
	origin = 1;        
end;

if origin == -1, 
        HDR.FILE.POS = offset;
        if strmatch(HDR.TYPE,{'MFER','SCP','native'}),
	elseif HDR.FILE.FID>2,
                POS = HDR.HeadLen+HDR.AS.bpb*offset;
                if POS~=ceil(POS),  % for alpha format
                        fprintf(HDR.FILE.stderr,'Error STELL (alpha): starting position is non-integer\n');     
                        return;
                end
                HDR.FILE.status = fseek(HDR.FILE.FID,POS,-1);
        end
        
elseif origin == 0, 
        HDR.FILE.POS = HDR.FILE.POS + offset;
        if strmatch(HDR.TYPE,{'MFER','SCP','native'}),
	elseif HDR.FILE.FID>2,
                POS = HDR.AS.bpb*offset;
                if POS~=ceil(POS),  % for alpha format
                        fprintf(HDR.FILE.stderr,'Error STELL (alpha): starting position is non-integer\n');     
                        return;
                end
                HDR.FILE.status = fseek(HDR.FILE.FID,POS,0);
        end
        
elseif origin == 1, 
	if 0, %strmatch(HDR.TYPE,{}),
		HDR.FILE.POS = HDR.NRec+offset;
		HDR.FILE.status = fseek(HDR.FILE.FID,HDR.AS.bpb*offset,1);
	elseif strmatch(HDR.TYPE,{'ACQ','BDF','CTF','EDF','GDF','Nicolet'}),
		POS = HDR.AS.endpos+offset*HDR.AS.bpb;
		HDR.FILE.status = fseek(HDR.FILE.FID,POS,-1);
		HDR.FILE.POS = (POS-HDR.HeadLen)/HDR.AS.bpb;
	elseif strmatch(HDR.TYPE,{'BKR','ISHNE','RG64','MIT','LABVIEW','SMA','BVbinmul','BCI2000'}),
		HDR.FILE.POS = HDR.AS.endpos+offset;
		HDR.FILE.status = fseek(HDR.FILE.FID,HDR.AS.bpb*offset,1);
	elseif strmatch(HDR.TYPE,{'CNT','EEG','AVG','EGI','SND','WAV','AIF','CFWB','DEMG'}),
		HDR.FILE.POS = HDR.AS.endpos+offset;
		HDR.FILE.status = fseek(HDR.FILE.FID,HDR.HeadLen+(HDR.AS.endpos+offset)*HDR.AS.bpb,-1);
        elseif strmatch(HDR.TYPE,{'alpha'}),
                POS = HDR.HeadLen+(HDR.AS.endpos+offset)*HDR.AS.bpb;
                if POS~=ceil(POS),  % for alpha format
                        fprintf(HDR.FILE.stderr,'Error STELL (alpha): starting position is non-integer\n');     
                        return;
                end
		HDR.FILE.POS = HDR.AS.endpos+offset;
		HDR.FILE.status = fseek(HDR.FILE.FID,POS,-1);
        elseif strmatch(HDR.TYPE,{'RDF','SIGIF'}),
		HDR.FILE.POS = length(HDR.Block.Pos)+offset;
        elseif strmatch(HDR.TYPE,{'BVascii','BVbinvec','EEProbe-CNT','EEProbe-AVR','FIF','native','MFER','SCP'}),
		HDR.FILE.POS = HDR.AS.endpos+offset;
	else
		fprintf(HDR.FILE.stderr,'Warning SSEEK: format %s not supported.\n',HDR.TYPE);	
	end;
else
        fprintf(2,'error SSEEK: 3rd argument "%s" invalid\n',origin);
        return;
end;

if strcmp(HDR.TYPE,'EDF') | strcmp(HDR.TYPE,'GDF') | strcmp(HDR.TYPE,'BDF'),
        HDR.AS.startrec = HDR.FILE.POS;
        HDR.AS.numrec = 0;
        %HDR = sdftell(HDR); % not really needed, only for double check of algorithms
        
        % Initialization of Bufferblock for random access (without EDF-blocklimits) of data 
        if ~HDR.SIE.RAW & HDR.SIE.TimeUnits_Seconds,
                HDR.Block.number = [0 0 0 0]; %Actual Blocknumber, start and end time of loaded block, diff(HDR.Block.number(1:2))==0 denotes no block is loaded;
                % HDR.Blcok.number(3:4) indicate start and end of the returned data, [units]=samples.
                HDR.Block.data = [];
                HDR.Block.dataOFCHK = [];
        end;
        
        if 1; %isfield(EDF,'AFIR');
                if HDR.SIE.AFIR,
                        HDR.AFIR.w = zeros(HDR.AFIR.nC,max(HDR.AFIR.nord));
                        HDR.AFIR.x = zeros(1,HDR.AFIR.nord);
                        HDR.AFIR.d = zeros(HDR.AFIR.delay,HDR.AFIR.nC);
                        fprintf(2,'WARNING SSEEK: Repositioning deletes AFIR-filter status\n');
                end;
        end;
        if 1; %isfield(EDF,'Filter');
                if HDR.SIE.FILT,
                        [tmp,HDR.Filter.Z] = filter(HDR.Filter.B,HDR.Filter.A,zeros(length(HDR.Filter.B+1),length(HDR.SIE.ChanSelect)));
                        HDR.FilterOVG.Z = HDR.Filter.Z;
                        fprintf(2,'WARNING SSEEK: Repositioning deletes Filter status of Notch\n');
                end;
        end;
        
        if 1; %isfield(EDF,'TECG')
                if HDR.SIE.TECG,
                        fprintf(2,'WARNING SSEEK: Repositioning deletes TECG filter status\n');
                end;
        end;
end;
