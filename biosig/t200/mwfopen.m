function [HDR]=mwfopen(HDR,PERMISSION,arg3,arg4,arg5,arg6)
% MWFOPEN reads MFER files 
%
% HDR = mwfopen(Filename,PERMISSION);
%
% HDR contains the Headerinformation and internal data
%
% see also: SOPEN, SREAD, SSEEK, STELL, SCLOSE, SWRITE, SEOF


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
%	$Id: mwfopen.m,v 1.1 2004-04-08 07:07:55 schloegl Exp $
%	(C) 2004 by Alois Schloegl
%	a.schloegl@ieee.org	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

if nargin<1, PERMISSION='rb'; end;
if ischar(HDR)
        tmp=HDR;
        HDR=[];
        HDR.FileName=tmp;
end;

VER = version;

HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,'ieee-le');
HDR.Endianity= 'ieee-le';

%%% Default values %%%
HDR.SampleRate = 1000;
HDR.GDFTYP = 3;        

if ~isempty(findstr(PERMISSION,'r')),		%%%%% READ 
        HDR.FILE.OPEN = 1;
        count = 1;
        %while count>0, %~feof(HDR.FILE.FID)
        while ~feof(HDR.FILE.FID)
                tag = fread(HDR.FILE.FID,1,'uchar');
                len = fread(HDR.FILE.FID,1,'char');
                fprintf(1,'[%i] Tag %i: (%i)\n',ftell(HDR.FILE.FID),tag,len);

                if (len < 0) | (len > 127),
                        l   = mod(len,128);
                        len = fread(HDR.FILE.FID,[l,1],'uchar')
                        %len = 256.^[0:l-1]*len;        
                        len = 256.^[l-1:-1:0]*len;        
                end;
                % tmp = fread(HDR.FILE.FID,[1,len],'char');
                
                if 0,
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'char');
                elseif tag==0; 
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'char');
                        
                elseif tag==1;
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'char');
                        if 0, 
                        elseif (tmp==1) & strcmp(HDR.Endianity,'ieee-le'),
                                tmp = ftell(HDR.FILE.FID);
                                HDR.Endianity='ieee-be';
                                fclose(HDR.FILE.FID);
                                HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,HDR.Endianity);
                                fseek(HDR.FILE.FID,tmp,'bof');
                        elseif (tmp==0) & strcmp(HDR.Endianity,'ieee-be'),
                                tmp = ftell(HDR.FILE.FID);
                                HDR.Endianity='ieee-le';
                                fclose(HDR.FILE.FID);
                                HDR.FILE.FID = fopen(HDR.FileName,PERMISSION,HDR.Endianity);
                                fseek(HDR.FILE.FID,tmp,'bof');
                        else
                        end;
                        
                elseif tag==2;
                        [tmp,count] = fread(HDR.FILE.FID,[1,3],'char');
                        HDR.Version = tmp;
                        
                elseif tag==3;
                        [tmp,count] = fread(HDR.FILE.FID,[1,3],'char');
                        HDR.tag03 = tmp;
                        
                elseif tag==4;          % channel number
                        if len == 1;
                                [tmp,count] = fread(HDR.FILE.FID,1,'int8');
                        elseif len == 2;
                                [tmp,count] = fread(HDR.FILE.FID,1,'int16');
                        elseif len == 3;
                                [tmp,count] = fread(HDR.FILE.FID,3,'uint8');
                                tmp = (2.^[16,8,0])*tmp;
                        elseif len == 4;
                                [tmp,count] = fread(HDR.FILE.FID,1,'int32');
                        end;
                        %[HDR.SPR,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        HDR.SPR = tmp;
                        
                elseif tag==5;
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        HDR.NS = tmp*256.^[0:len-1]';
                        
                elseif tag==6;
                        if len == 1;
                                [tmp,count] = fread(HDR.FILE.FID,1,'int8');
                        elseif len == 2;
                                [tmp,count] = fread(HDR.FILE.FID,1,'int16');
                        elseif len == 3;
                                [tmp,count] = fread(HDR.FILE.FID,3,'uint8');
                                tmp = (2.^[16,8,0])*tmp;
                        elseif len == 4;
                                [tmp,count] = fread(HDR.FILE.FID,1,'int32');
                        end;
                        HDR.NRec = tmp;
                        
                elseif tag==7;
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        HDR.Pointer = tmp*256.^[0:len-1]';
                        
                elseif tag==8;
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        HDR.tag08 = tmp;
                        
                elseif tag==9; 
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        HDR.Label=char(tmp);
                        
                elseif tag==10;
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        if tmp==0;	% int16, default
                                HDR.GDFTYP = 3;
                        elseif tmp==1;	% uint16
                                HDR.GDFTYP = 4;
                        elseif tmp==2;	% int32
                                HDR.GDFTYP = 5;
                        elseif tmp==3;	% uint8
                                HDR.GDFTYP = 3;
                        elseif tmp==4;	% 16bit status
                                HDR.GDFTYP = 4;
                        elseif tmp==5;	% int8
                                HDR.GDFTYP = 1;
                        elseif tmp==6;	% uint32
                                HDR.GDFTYP = 6;
                        elseif tmp==7;	% float32
                                HDR.GDFTYP = 16;
                        elseif tmp==8;	% float64
                                HDR.GDFTYP = 17;
                        elseif tmp==9;	% 8 bit AHA compression 
                                HDR.GDFTYP = 4;
                        end;
                        
                elseif tag==11;       
                        [tmp1,count] = fread(HDR.FILE.FID,2,'int8');
                        len = len - 2;
                        if len == 1;
                                [tmp,count] = fread(HDR.FILE.FID,1,'int8');
                        elseif len == 2;
                                [tmp,count] = fread(HDR.FILE.FID,1,'int16');
                        elseif len == 3;
                                [tmp,count] = fread(HDR.FILE.FID,1,'bit24');
                        elseif len == 4;
                                [tmp,count] = fread(HDR.FILE.FID,1,'int32');
                        end;
                        e = 10^tmp1(2);
                        if     tmp1(1)==0, 
                                HDR.Xphysdim = 'Hz';
                                HDR.SampleRate=tmp*e;
                        elseif tmp1(1)==1, 
                                HDR.Xphysdim = 's';
                                HDR.SampleRate= 1/(tmp*e);
                        elseif tmp1(1)==2, 
                                HDR.Xphysdim = 'm';
                                HDR.SampleRate=tmp*e;
                        end;
                        
                elseif tag==12;          % sensitivity, resolution, gain, calibration, 
                        [tmp,count] = fread(HDR.FILE.FID,2,'int8');
                        if     tmp(1)==0, HDR.PhysDim = 'V';
                        elseif tmp(1)==1, HDR.PhysDim = 'mmHg';
                        elseif tmp(1)==2, HDR.PhysDim = 'Pa';
                        elseif tmp(1)==3, HDR.PhysDim = 'cmH2O';
                        elseif tmp(1)==4, HDR.PhysDim = 'mmHg';
                        elseif tmp(1)==5, HDR.PhysDim = 'dyne';
                        elseif tmp(1)==6, HDR.PhysDim = 'N';
                        elseif tmp(1)==7, HDR.PhysDim = '%';
                        elseif tmp(1)==8, HDR.PhysDim = '';
                        elseif tmp(1)==9, HDR.PhysDim = '';
                        end;
                        e = 10^tmp(2);
                        len = len - 2;
                        if len == 1;
                                [tmp,count] = fread(HDR.FILE.FID,1,'int8');
                        elseif len == 2;
                                [tmp,count] = fread(HDR.FILE.FID,1,'int16');
                        elseif len == 3;
                                [tmp,count] = fread(HDR.FILE.FID,1,'bit24');
                        elseif len == 4;
                                [tmp,count] = fread(HDR.FILE.FID,1,'int32');
                        end;
                        HDR.Cal = tmp*e;
                        
                elseif tag==13;          % offset
                        [tmp,count] = fread(HDR.FILE.FID,1,gdfdatatype(HDR.GDFTYP));
                        HDR.Off=tmp;
                        
                elseif tag==14;          % compression
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        HDR.FLAG.Compresssion=tmp;
                        
                elseif tag==17;
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        HDR.tag17 = tmp;
                        
                elseif tag==18;       % null value
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        HDR.MFER.NullValue = tmp;
                        
                elseif tag==18;       % null value
                        [HDR.MFER.CompressionCode,count] = fread(HDR.FILE.FID,1,'uint16');
                        [HDR.MFER.DataLength,count] = fread(HDR.FILE.FID,1,'uint32');
                        [HDR.MFER.DataLength,count] = fread(HDR.FILE.FID,1,'uint32');
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        HDR.tag18=char(tmp);
                        
                elseif tag==21;          % number of records
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        HDR.tag21=char(tmp);
                        
                elseif tag==22;          % number of records
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        HDR.comment=char(tmp);
                        
                elseif tag==23;          % number of records
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        HDR.tag23 = char(tmp);
                        
                elseif tag==30;          % 
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        HDR.tag30 = char(tmp);
                        
                elseif tag==23;          % number of records
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'uchar');
                        HDR.tag23 = char(tmp);
                        
                elseif tag==63;     
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'char')
                        HDR.tag63 = tmp;
                        
                elseif tag==64;     % Preamble
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'char');
                        HDR.TYPE='MFER';
                        
                elseif tag==65;     % Events
                        N = HDR.EVENT.N + 1;
                        HDR.EVENT.N = N;
                        [HDR.EVENT.TYP(N),count] = fread(HDR.FILE.FID,1,'uint16');
                        if len>5,
                                [HDR.EVENT.POS(N),count] = fread(HDR.FILE.FID,1,'uint32');
                        end;
                        if len>9,
                                [HDR.EVENT.DUR(N),count] = fread(HDR.FILE.FID,1,'uint32');
                        end;
                        if len>10,
                                [HDR.EVENT.Desc{N},count] = fread(HDR.FILE.FID,len-10,'char');
                        end;
                        
                elseif tag==67;     % Sample Skew
                        [tmp,count] = fread(HDR.FILE.FID,1,'int16');
                        HDR.SampleSkew = tmp;
                        
                elseif tag==129;     % Patient Name 
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'char');
                        HDR.Patient.Name = char(tmp);
                        
                elseif tag==130;     % Patient Id 
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'char');
                        HDR.PID = char(tmp);
                        
                elseif tag==131;     % Patient Age 
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'char');
                        HDR.Patient.Age = char(tmp);
                        
                elseif tag==132;     % Patient Age 
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'char');
                        HDR.Patient.Sex = char(tmp);
                        
                elseif tag==133;     % recording time 
                        [HDR.T0(1),count] = fread(HDR.FILE.FID,1,'int16');
                        [HDR.T0(2:6),count] = fread(HDR.FILE.FID,[1,5],'uint8');
                        [tmp,count] = fread(HDR.FILE.FID,[1,2],'int16');
                        HDR.T0(6) = HDR.T0(6) + tmp(1)*1e-3 + tmp(2)+1e-6;
                        
                else
                        [tmp,count] = fread(HDR.FILE.FID,[1,len],'char');
                        fprintf(1,'[%i %i] Tag %i: (%i) %s\n',ftell(HDR.FILE.FID),count,tag,len,char(tmp));
                end;
                %                        fprintf(1,'[%i %i] Tag %i: (%i)\n',ftell(HDR.FILE.FID),count,tag,len);
                %                        count = 1;
                %                        pause
        end;
        HDR.HeadLen = ftell(HDR.FILE.FID);
        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,HDR.Cal);
end;        
