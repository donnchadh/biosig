function [HDR]=eegseek(HDR,offset,origin)
% EEGSEEK repositions file position indicator
% [HDR]=eegseek(HDR,offset,origin)
%
% The meaning of "offset" and "origin" differs for different formats. 
% 	EDF: number of (EDF) records. 
%	BKR: number of samplepoints	
%
% See also: FSEEK, EEGREAD, EEGWRITE, EEGCLOSE, EEGREWIND, EEGTELL, EEGEOF

%	$Revision: 1.3 $
%	$Id: eegseek.m,v 1.3 2003-04-26 10:22:08 schloegl Exp $
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

OFFSET = HDR.AS.bpb*offset;

if origin == -1, 
        HDR.FILE.POS = offset;
        HDR.FILE.status = fseek(HDR.FILE.FID,HDR.HeadLen+OFFSET,-1);
elseif origin == 0, 
        HDR.FILE.POS = HDR.FILE.POS + offset;
        HDR.FILE.status = fseek(HDR.FILE.FID,OFFSET,0);
elseif origin == 1, 
	if strcmp(HDR.TYPE,'EDF') | strcmp(HDR.TYPE,'GDF') | strcmp(HDR.TYPE,'BDF'),
		HDR.FILE.POS = HDR.NRec+offset;
		HDR.FILE.status = fseek(HDR.FILE.FID,OFFSET,1);
	elseif strcmp(HDR.TYPE,'BKR') | strcmp(HDR.TYPE,'ISHNE'),
		HDR.FILE.POS = HDR.AS.endpos+offset;
		HDR.FILE.status = fseek(HDR.FILE.FID,OFFSET,1);
	elseif strcmp(HDR.TYPE,'CNT'),
		HDR.FILE.POS = HDR.AS.endpos+offset;
		HDR.FILE.status = fseek(HDR.FILE.FID,HDR.HeadLen+HDR.AS.endpos*HDR.AS.bpb+OFFSET,-1);
	else
		fprintf(2,'Waring EEGSEEK: format %s not supported.\n',HDR.TYPE);	
	end;
else
        fprintf(2,'error EEGSEEK: 3rd argument "%s" invalid\n',origin);
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
                        fprintf(2,'WARNING SDFSEEK: Repositioning deletes AFIR-filter status\n');
                end;
        end;
        if 1; %isfield(EDF,'Filter');
                if HDR.SIE.FILT,
                        [tmp,HDR.Filter.Z] = filter(HDR.Filter.B,HDR.Filter.A,zeros(length(HDR.Filter.B+1),length(HDR.SIE.ChanSelect)));
                        HDR.FilterOVG.Z = HDR.Filter.Z;
                        fprintf(2,'WARNING SDFSEEK: Repositioning deletes Filter status of Notch\n');
                end;
        end;
        
        if 1; %isfield(EDF,'TECG')
                if HDR.SIE.TECG,
                        fprintf(2,'WARNING SDFSEEK: Repositioning deletes TECG filter status\n');
                end;
        end;
end;
