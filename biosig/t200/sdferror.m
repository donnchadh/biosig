function [EDF,status]=sdferror(EDF)
% Inquires EDF file error status 
% [EDF,status]=sdferror(EDF)
% 
% EDF.ErrNo-1024
%              1: ascii(0) in 1st header
%              2: ascii(0) in 2nd header
%              3: invalid gain (zero or infinity) 
%              4: invalid SPR
%              4: invalid samples_per_record-values
%              8: date not in EDF-format (tries to guess correct date, see also E2)
%             16: invalid number_of-channels-value
%             32: invalid value of the EDF header length
%             64: invalid value of block_duration
%            128: Polarity of #7 probably inverted  
%
% EDF.ErrNo    1: first 8 bytes different to '0       ', this violates the EDF spec.
%              2: Invalid date (unable to guess correct date)
%              4: Incorrect date information (later than actual date) 
%             16: incorrect filesize, Header information does not match actual size
% 
% See also: ferror, SDFREAD, SDFWRITE, SDFCLOSE, SDFSEEK, SDFREWIND, SDFTELL, SDFEOF
%

%
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


%	Version 0.84
%	05 Mar 2002
%	Copyright (c) 1997-2002 by Alois Schloegl
%	a.schloegl@ieee.org	

% transformation of error codes from binary representation to a list
status=EDF.ErrNo;

if (EDF.T0(1)>EDF.AS.Date(1)) 
%        fprintf(EDF.FILE.stderr,'Warning %s: recording date %4i.%02i.%02i %02i:%02i:%02i is in the future or computer clock %4i.%02i.%02i %02i:%02i:%02i is not correct ''%s'' \n',upper(EDF.AS.Method),EDF.T0,EDF.AS.Date,EDF.FileName); 
end;
EDF.FILE.stderr=2;


if any(EDF.ErrNo==1)
        fprintf(EDF.FILE.stderr,'Error %s: Invalid or corrupted EDF/GDF file or invalid Version number ''%s''\n',upper(EDF.AS.Method),EDF.VERSION); 
        EDF.ErrNo(EDF.ErrNo==1)=[];
end;
if any(EDF.ErrNo==2)
        fprintf(EDF.FILE.stderr,'Error %s: Invalid recording date %4i.%02i.%02i %02i:%02i:%02i \n', upper(EDF.AS.Method),EDF.T0); 
        EDF.ErrNo(EDF.ErrNo==2)=[];
end;
if any(EDF.ErrNo==4)
        fprintf(EDF.FILE.stderr,'Error %s: Invalid recording date %4i.%02i.%02i %02i:%02i:%02i \n', upper(EDF.AS.Method),EDF.T0); 
        EDF.ErrNo(EDF.ErrNo==4)=[];
end;
if any(EDF.ErrNo==8)
        fprintf(EDF.FILE.stderr,'Error %s: unable to read header in %s\n',upper(EDF.AS.Method),EDF.FileName); 
        EDF.ErrNo(EDF.ErrNo==8)=[];
end;
if any(EDF.ErrNo==16)
        fprintf(EDF.FILE.stderr,'Warning %s: Incorrect HeaderInformation in "%s" does not fit to filesize\n',upper(EDF.AS.Method),EDF.FileName);
        fprintf(EDF.FILE.stderr,'endpos %i\tEDF.HeadLen %i\tEDF.NRec %i\tEDF.AS.bpb %i\tDIFF %f blocks\t\n',EDF.AS.endpos,EDF.HeadLen,EDF.NRec,EDF.AS.bpb,(EDF.AS.endpos-EDF.HeadLen)/EDF.AS.bpb-EDF.NRec);
        %fprintf(EDF.FILE.stderr,'Error %s: filesize doesnot fit to header information %s\n',upper(EDF.AS.Method),EDF.FileName); 
        EDF.NRec = floor((EDF.AS.endpos - EDF.HeadLen) / EDF.AS.bpb);
        EDF.ErrNo(EDF.ErrNo==16)=[];
end;
if any(EDF.ErrNo==32)
	fprintf(2,'Invalid Filename %s\n', FILENAME);
        EDF.ErrNo(EDF.ErrNo==32)=[];
end;
if any(EDF.ErrNo==64)
        fprintf(EDF.FILE.stderr,'Error %s-R: Fixed Header not read correctly (%i bytes read )\n',upper(EDF.AS.Method));
        EDF.ErrNo(EDF.ErrNo==64)=[];
end;

if any(EDF.ErrNo==1025)
        fprintf(EDF.FILE.stderr,'Warning %s: fixed Header in %s contains non-printable ASCII characters\n',upper(EDF.AS.Method),EDF.FileName); 
        EDF.ErrNo(EDF.ErrNo==1025)=[];
end;
if any(EDF.ErrNo==1026)
        fprintf(EDF.FILE.stderr,'Warning %s: variable Header in %s contains non-printable ASCII characters\n',upper(EDF.AS.Method),EDF.FileName); 
        EDF.ErrNo(EDF.ErrNo==1026)=[];
end;
if any(EDF.ErrNo==1027)
        fprintf(EDF.FILE.stderr, 'Warning SDFOPEN: invalid NRec-value in header of %s\n',EDF.FILE.Name);
        EDF.ErrNo(EDF.ErrNo==1027)=[];
end;
if any(EDF.ErrNo==1028)
        fprintf(EDF.FILE.stderr, 'Warning SDFOPEN: invalid SPR-value in header of %s\n',EDF.FILE.Name);
        EDF.ErrNo(EDF.ErrNo==1028)=[];
end;
if any(EDF.ErrNo==1029)
        fprintf(EDF.FILE.stderr, 'Warning SDFOPEN: invalid scaling factor(s), gain is zero in  %s\n',EDF.FILE.Name);
        EDF.ErrNo(EDF.ErrNo==1029)=[];
end;
if any(EDF.ErrNo==1030)
        fprintf(EDF.FILE.stderr, 'Warning SDFOPEN: invalid scaling factor(s), gain is infinity in  %s\n',EDF.FILE.Name);
        EDF.ErrNo(EDF.ErrNo==1030)=[];
end;
if any(EDF.ErrNo==1032)
        fprintf(EDF.FILE.stderr,'Warnig %s: Invalid recording date; guessed date/time is  %04i/%02i/%02i %02i:%02i:%02i \n', upper(EDF.AS.Method),EDF.T0); 
        EDF.ErrNo(EDF.ErrNo==1032)=[];
end;
if any(EDF.ErrNo==1040)
        fprintf(EDF.FILE.stderr,'Warning %s: Incorrect HeaderInformation in "%s" does not fit to filesize\n',upper(EDF.AS.Method),EDF.FileName);
        fprintf(EDF.FILE.stderr,'endpos %i\tEDF.HeadLen %i\tEDF.NRec %i\tEDF.AS.bpb %i\tDIFF %f blocks\t\n',endpos,EDF.HeadLen,EDF.NRec,EDF.AS.bpb,(endpos-EDF.HeadLen)/EDF.AS.bpb-EDF.NRec);
        EDF.ErrNo(EDF.ErrNo==1040)=[];
end;
if any(EDF.ErrNo==1056)
        fprintf(EDF.FILE.stderr, 'Warning SDFOPEN: invalid HeadLen-value in header of %s\n',EDF.FILE.Name);
        EDF.ErrNo(EDF.ErrNo==1056)=[];
end;
if any(EDF.ErrNo==1088)
        fprintf(EDF.FILE.stderr, 'Warning SDFOPEN: invalid Dur-value in header of %s\n',EDF.FILE.Name);
        EDF.ErrNo(EDF.ErrNo==1088)=[];
end;

while any(EDF.ErrNo) 
        fprintf(EDF.FILE.stderr, 'WARNING SDFOPEN: Unidentified error code %i in %s\n',EDF.ErrNo(1),EDF.FILE.Name);
	EDF.ErrNo=EDF.ErrNo(2:length(EDF.ErrNo));        
end;

return;

