function [HDR]=eegwrite(HDR,data)
% EEGWRITE writes biosig data. 
% HDR = eegwrite(HDR,data)
% Appends data to an Biosig File 

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


%	$Revision: 1.5 $
%	$Id: eegwrite.m,v 1.5 2003-08-18 16:41:30 schloegl Exp $
%	Copyright (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	


if HDR.FILE.OPEN==1,
	fprintf(HDR.FILE.stderr,'Error EEGWRITE can not be applied, File %s is open in READ mode\n',HDR.FileName);
	return;
end;


if strcmp(HDR.TYPE,'EDF') | strcmp(HDR.TYPE,'GDF') | strcmp(HDR.TYPE,'BDF'),
        
        %%% this tests are necessary, since Octave skips the high bits,
        if exist('OCTAVE_VERSION')>2,
                if strcmp(HDR.TYPE,'EDF'),
                        data(data> 2^15-1) =  2^15-1;
                        data(data<-2^15  ) = -2^15;
                end;
                if strcmp(HDR.TYPE,'BDF'),
                        data(data> 2^23-1) =  2^23-1;
                        data(data<-2^23  ) = -2^23;
                end;
                if strcmp(HDR.TYPE,'GDF'),
                        fprintf(2,'Warning GDF-WRITE: overflow check not implemented.\n');
                end;
        end;
        
        % 	for GDF only one datatype is supported
        if ~all(HDR.GDFTYP==HDR.GDFTYP(1)) 
                fprintf(2,'Error EEGWRITE: different GDFTYPs not supported yet!\n');
                return;
        end;
        
        if HDR.SIE.RAW,
                if isnan(HDR.AS.spb),  %first call of sdfwrite
                        [HDR.AS.spb,HDR.NRec]=size(data);
                        HDR.AS.bpb = 2*HDR.AS.spb; %only for HDR
                end;
                if sum(HDR.SPR)~=size(data,1)
                        fprintf(2,'Warning EDFWRITE: datasize must fit to the Headerinfo %i %i %i\n',HDR.AS.spb,size(data));
                        fprintf(2,'Define the Headerinformation correctly.\n',HDR.AS.spb,size(data));
                end;
                count = fwrite(HDR.FILE.FID,data,gdfdatatype(HDR.GDFTYP(1)));
        else
                
                %   Support of only 1 sample rate
                if ~all(HDR.SPR==HDR.SPR(1)) 
                        fprintf(2,'Error EEGWRITE: different Samplingrates require RAW-data mode !\n');
                        return;
                end;
                
                [nr,HDR.NS] = size(data);
                if isnan(HDR.AS.spb),  %first call of sdfwrite
                        HDR.AS.MAXSPR = floor(61440/2/HDR.NS);
                        HDR.SPR = ones(HDR.NS,1)*HDR.AS.MAXSPR;
                        HDR.AS.spb = sum(HDR.SPR);
                        HDR.AS.bpb = 2*HDR.AS.spb; %only for HDR
                end;        
                
                count=0;
                for k = 0:floor(nr/HDR.AS.MAXSPR)-1;
                        c = fwrite(HDR.FILE.FID,data(k*HDR.AS.MAXSPR+(1:HDR.AS.MAXSPR),:),gdfdatatype(HDR.GDFTYP(1)));
                        count = count + c;
                end;
                tmp = rem(nr,HDR.AS.MAXSPR);
                if tmp,
                        fprintf(2,'Warning EEGWRITE (EDF/GDF/BDF): last block is too short\n');
                end;
        end;
        HDR.AS.numrec = HDR.AS.numrec + count/HDR.AS.spb;
        HDR.FILE.POS  = HDR.FILE.POS  + count/HDR.AS.spb;
        
        
elseif strcmp(HDR.TYPE,'BKR'),
        count=0;
        if HDR.NS~=size(data,2) & HDR.NS==size(data,1),
                fprintf(2,'EEGWRITE: number of channels fits number of rows. Transposed data\n');
                data = data';
        end
        
        if any(HDR.NS==[size(data,2),0]),	% check if HDR.NS = 0 (unknown channel number) or number_of_columns 
                if HDR.NS==0, HDR.NS = size(data,2); end;   % if HDR.NS not set, set it. 
                if ~HDR.FLAG.UCAL,
                        data = data*(HDR.DigMax/HDR.PhysMax);
                end;
                % Overflow detection
                data(data>2^15-1)=  2^15-1;	
                data(data<-2^15) = -2^15;
                count = fwrite(HDR.FILE.FID,data','short');
        else
                fprintf(2,'Error EEGWRITE: number of columns (%i) does not fit Header information (number of channels HDR.NS %i)',size(data,2),HDR.NS);
                return;
        end;
	if HDR.NRec > 1,
	        HDR.FILE.POS = HDR.FILE.POS + size(data,1)/HDR.SPR;
	else
		HDR.FILE.POS = 1;		% untriggered data
	end;
        %HDR.AS.endpos = HDR.AS.endpos + size(data,1);
end;                        
