function [HDR]=sclose(HDR)
% [EEG]=sclose(EEG)
% Closes an Signal-File 
%
% see also: SOPEN, SREAD, SSEEK, STELL, SCLOSE, SWRITE

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

%	$Revision: 1.1 $
%	$Id: sclose.m,v 1.1 2003-09-06 17:19:32 schloegl Exp $
%	Copyright (C) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org


if HDR.FILE.FID<0, return; end;

if HDR.FILE.OPEN>=2,
	% check file length - simple test for file integrity         
        EndPos = ftell(HDR.FILE.FID);          % get file length
        % force file pointer to the end, otherwise Matlab 6.5 R13 on PCWIN
        status = fseek(HDR.FILE.FID, 0, 'eof'); % go to end-of-file
        
        if strcmp(HDR.TYPE,'BKR');
                if HDR.NS<1, 
                        fprintf(2,'Error EEGOPEN BKR: number of channels (HDR.NS) must be larger than zero.\n');
                        return;
                end;
                if HDR.NRec<1, 
                        fprintf(2,'Error EEGOPEN BKR: number of blocks (HDR.NRec) must be larger than zero.\n');
                        return;
                end;
                % check file length and write Headerinfo.
                HDR.SPR       = (EndPos-HDR.HeadLen)/(HDR.NRec*HDR.NS*2);
                if isnan(HDR.SPR), HDR.SPR=0; end;
                if HDR.FILE.OPEN==3;
			fclose(HDR.FILE.FID);
			HDR.FILE.FID = fopen(HDR.FileName,'r+');
                        fseek(HDR.FILE.FID,2,'bof');
               		count = fwrite(HDR.FILE.FID,HDR.NS,'uint16');             % channel
        		fseek(HDR.FILE.FID,6,'bof');
                        count = fwrite(HDR.FILE.FID,[HDR.NRec,HDR.SPR],'uint32');           % trials/samples/trial
        		fseek(HDR.FILE.FID,32,'bof');
                        HDR.FLAG.TRIGGERED   = HDR.NRec>1;	% Trigger Flag
                        count = fwrite(HDR.FILE.FID,HDR.FLAG.TRIGGERED,'int16');           % FLAG TRIGGERED
                end;
		HDR.FILE.status = fclose(HDR.FILE.FID);
	        HDR.FILE.OPEN = 0;

	elseif strcmp(HDR.TYPE,'EDF') | strcmp(HDR.TYPE,'BDF') | strcmp(HDR.TYPE,'GDF'),
        
         	tmp = floor((EndPos - HDR.HeadLen) / HDR.AS.bpb);  % calculate number of records
        	if ~isnan(tmp)
        	        HDR.NRec=tmp;
			fseek(HDR.FILE.FID,236,'bof');
			if strcmp(HDR.TYPE,'GDF')
			        c=fwrite(HDR.FILE.FID,[HDR.NRec,0],'int32');
			else	
			        fprintf(HDR.FILE.FID,'%-8i',HDR.NRec);
			end;
		end;
        	%fclose(HDR.FILE.FID);
        	%HDR=sdfopen(HDR,'w+');                    % update header information
	end;
end;

if HDR.FILE.OPEN,
        HDR.FILE.OPEN = 0;
        HDR.ErrNo = fclose(HDR.FILE.FID);
end;
        