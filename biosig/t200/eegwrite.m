function [HDR]=eegwrite(HDR,data)
% EEGWRITE writes biosig data. 
% count = eegwrite(HDR,data)
% Appends data to an Biosig File 


%	$Revision: 1.1 $
%	$Id: eegwrite.m,v 1.1 2003-02-01 15:03:46 schloegl Exp $
%	Copyright (c) 1997-2003 by Alois Schloegl
%	a.schloegl@ieee.org	


if HDR.FILE.OPEN==1,
	fprintf(HDR.FILE.stderr,'Error EEGWRITE can not be applied, File %s is open in READ mode\n',HDR.FileName);
	return;
end;

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

if strcmp(HDR.TYPE,'EDF') | strcmp(HDR.TYPE,'GDF') | strcmp(HDR.TYPE,'BDF'),
        if HDR.SIE.RAW,
                if isnan(HDR.AS.spb),  %first call of sdfwrite
                        [HDR.AS.spb,HDR.NRec]=size(data);
                        HDR.AS.bpb = 2*HDR.AS.spb; %only for HDR
                end;
                if sum(HDR.SPR)~=size(data,1)
                        fprintf(2,'Warning EDFWRITE: datasize must fit to the Headerinfo %i %i %i\n',HDR.AS.spb,size(data));
                        fprintf(2,'Define the Headerinformation correctly.\n',HDR.AS.spb,size(data));
                end;
                count = fwrite(HDR.FILE.FID,data,'integer*2');
        else
                [nr,HDR.NS] = size(data);
                if isnan(HDR.AS.spb),  %first call of sdfwrite
                        HDR.AS.MAXSPR = floor(61440/2/HDR.NS);
                        HDR.SPR = ones(HDR.NS,1)*HDR.AS.MAXSPR;
                        HDR.AS.spb = sum(HDR.SPR);
                        HDR.AS.bpb = 2*HDR.AS.spb; %only for HDR
                end;        
                for k = 0:floor(nr/HDR.AS.MAXSPR)-1;
                        count = fwrite(HDR.FILE.FID,data(k*HDR.SPR+(1:HDR.AS.MAXSPR),:),'integer*2');
                end;
                tmp = rem(nr,HDR.AS.MAXSPR);
                if tmp,
                        fprintf(2,'Warning SDFWRITE: last block is too short\n');
                end;
        end;
        HDR.FILE.POS = HDR.FILE.POS + count/HDR.AS.bpb;
        
elseif strcmp(HDR.TYPE,'BKR'),
        count=0;
        if HDR.NS~=size(data,2) & HDR.NS==size(data,1),
                fprintf(2,'EEGWRITE: number of channels fits number of rows. Transposed data\n');
                data = data';
        end
        
        if HDR.NS==size(data,2),
                if ~HDR.FLAG.UCAL,
	                data = data*(HDR.DigMax/HDR.PhysMax);
                end;
                % Overflow detection
                data(data>2^15-1)=  2^15-1;	
                data(data<-2^15) = -2^15;
                count = fwrite(HDR.FILE.FID,data','short');
        else
                fprintf(2,'EEGWRITE: number of columns (%i) does not fit Header information (number of channels HDR.NS %i)',size(data,2),HDR.NS)
        end;
	if HDR.NRec > 1,
	        HDR.FILE.POS = HDR.FILE.POS + size(data,1)/HDR.SPR;
	else
		HDR.FILE.POS = 1;		% untriggered data
	end;
        %HDR.AS.endpos = HDR.AS.endpos + size(data,1);
end;                        
