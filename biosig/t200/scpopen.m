function [HDR]=scpopen(HDR,PERMISSION,arg3,arg4,arg5,arg6)
% SCPOPEN reads SCP-ECG files 
%
% HDR = scpopen(Filename,PERMISSION);
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

%	$Revision: 1.3 $
%	$Id: scpopen.m,v 1.3 2004-02-02 19:31:00 schloegl Exp $
%	(C) 2004 by Alois Schloegl
%	a.schloegl@ieee.org	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

if nargin<1, PERMISSION='rb'; end;

fid = fopen(HDR.FileName,PERMISSION,'ieee-le');
if ~isempty(findstr(PERMISSION,'r')),		%%%%% READ 
        HDR.FILE.CRC = fread(fid,1,'uint16');
        HDR.FILE.Length = fread(fid,1,'uint32');
        
        DHT = [0,1,-1,2,-2,3,-3,4,-4,5,-5,6,-6,7,-7,8,-8,9,-9;0,1,5,3,11,7,23,15,47,31,95,63,191,127,383,255,767,511,1023]';
        prefix  = [1,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,10,10];
        %PREFIX = [0,4,5,12,13,28,29,60,61,124,125,252,253,508,509,1020,1021,1022,1023];
        PREFIX  = [0,4,5,12,13,28,29,60,61,124,125,252,253,508,509,1020,1021,1022,1023]'.*2.^[32-prefix]';
        codelength = [1,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,18,26];
        mask    = [1,7,7,15,15,31,31,63,63,127,127,255,255,511,511,1023,1023,1023,1023]'.*2.^[32-prefix]';
        %mask   = [1,7,7,15,15,31,31,63,63,127,127,255,255,511,511,1023,1023,1023,1023]';
        K  = 0;
        section.CRC = fread(fid,1,'uint16');
        while ~feof(fid)
                pos = ftell(fid);
                section.ID  = fread(fid,1,'uint16');
                section.Length = fread(fid,1,'uint32');
                section.Version= fread(fid,[1,2],'uint8');
                tmp = fread(fid,6,'uint8');
                
                K = K + 1;
                HDR.Section{K} = section;
                if section.ID==0, 
                        for k = 1:((section.Length-16)/10),
                                HDR.Block(k).id = fread(fid,1,'uint16');    
                                HDR.Block(k).length = fread(fid,1,'uint32');    
                                HDR.Block(k).startpos = fread(fid,1,'uint32');    
                        end;
                        
                elseif section.ID==1,
                        tag = 0; 
                        k1 = 0;
                        while tag~=255,
                                tag = fread(fid,1,'uchar');    
                                len = fread(fid,1,'uint16');    
                                field = fread(fid,[1,len],'uchar');
                                
                                if tag == 0,	
                                        HDR.Patient.LastName = char(field);
                                elseif tag == 1,
                                        HDR.Patient.FirstName = char(field);
                                elseif tag == 2,
                                        HDR.Patient.ID = char(field);
                                elseif tag == 3,
                                        HDR.Patient.LastName2 = char(field);
                                elseif tag == 4,
                                        HDR.Patient.Age = str2double(char(field(1:2)));
                                        tmp = field(3);
                                        if tmp==0, unit=' ';
                                        elseif tmp==1, unit='Y';
                                        elseif tmp==2, unit='M';
                                        elseif tmp==3, unit='W';
                                        elseif tmp==4, unit='d';
                                        elseif tmp==5, unit='h';
                                        end;
                                        HDR.Patient.AgeUnit = unit;
                                elseif tag == 5,
                                        HDR.Patient.DateOfBirth = [field(1:2)*[1;256],field(3:4)];
                                elseif tag == 6,
                                        HDR.Patient.Height = field(1:2)*[1;256];
                                        tmp = field(3);
                                        if tmp==0, unit=' ';
                                        elseif tmp==1, unit='cm';
                                        elseif tmp==2, unit='inches';
                                        elseif tmp==3, unit='mm';
                                        end;
                                        HDR.Patient.HeightUnit = unit;
                                elseif tag == 7,
                                        HDR.Patient.Weight = field(1:2)*[1;256];
                                        tmp = field(3);
                                        if tmp==0, unit=' ';
                                        elseif tmp==1, unit='kg';
                                        elseif tmp==2, unit='g';
                                        elseif tmp==3, unit='pound';
                                        elseif tmp==4, unit='ounce';
                                        end;
                                        HDR.Patient.WeightUnit = unit;
                                elseif tag == 8,
                                        HDR.Patient.Sex = field;
                                elseif tag == 9,
                                        HDR.Patient.Race = field;
                                elseif tag == 10,
                                        HDR.Patient.Medication = field;
                                elseif tag == 11,
                                        HDR.Patient.BloodPressure.Systolic = field*[1;256];
                                elseif tag == 12,
                                        HDR.Patient.BloodPressure.Diastolic = field*[1;256];
                                elseif tag == 13,
                                        HDR.Patient.Diagnosis = char(field);
                                elseif tag == 14,
                                        HDR.SCP1.AcquiringDeviceID = char(field);
                                elseif tag == 15,
                                        HDR.SCP1.AnalyisingDeviceID = char(field);
                                elseif tag == 16,
                                        HDR.SCP1.AcquiringInstitution = char(field);
                                elseif tag == 17,
                                        HDR.SCP1.AnalyzingInstitution = char(field);
                                elseif tag == 18,
                                        HDR.SCP1.AcquiringDepartment = char(field);
                                elseif tag == 19,
                                        HDR.SCP1.AnalyisingDepartment = char(field);
                                elseif tag == 20,
                                        HDR.SCP1.Physician = char(field);
                                elseif tag == 21,
                                        HDR.SCP1.LatestComfirmingPhysician = char(field);
                                elseif tag == 22,
                                        HDR.SCP1.Technician = char(field);
                                elseif tag == 23,
                                        HDR.SCP1.Room = char(field);
                                elseif tag == 24,
                                        HDR.SCP1.Emergency = field;
                                elseif tag == 25,
                                        HDR.T0(1,1:3) = [field(1:2)*[1;256],field(3:4)];
                                elseif tag == 26,
                                        HDR.T0(1,4:6) = field(1:3);
                                elseif tag == 27,
                                        HDR.Filter.HighPass = field(1:2)*[1;256]/100;
                                elseif tag == 28,
                                        HDR.Filter.LowPass = field(1:2)*[1;256]/100;
                                elseif tag == 28,
                                        HDR.Filter.BitMap = field;
                                elseif tag == 30,
                                        HDR.SCP1.FreeText = char(field);
                                elseif tag == 31,
                                        HDR.SCP1.ECGSequenceNumber = char(field);
                                elseif tag == 32,
                                        HDR.SCP1.MedicalHistoryCodes = char(field);
                                elseif tag == 33,
                                        HDR.SCP1.ElectrodeConfigurationCodes = field;
                                elseif tag == 34,
                                        HDR.SCP1.Timezone = field;
                                elseif tag == 35,
                                        HDR.SCP1.MedicalHistory = char(field);
                                elseif tag == 255,
                                        % section terminator	
                                end;
                        end;
                        
                elseif section.ID==2, 	% Huffman tables 
                        HDR.SCP2.NHT = fread(fid,1,'uint16');    
                        HDR.SCP2.NCT = fread(fid,1,'uint16');    
                        tmp = fread(fid,3,'uint8');    
                        HDR.SCP2.prefix = tmp(1);
                        HDR.SCP2.codelength = tmp(2);
                        HDR.SCP2.TableModeSwitch = tmp(3);
                        
                elseif section.ID==3, 
                        HDR.NS = fread(fid,1,'char');    
                        HDR.FLAG.Byte = fread(fid,1,'char');    
                        HDR.FLAG.ReferenceBeat = mod(HDR.FLAG.Byte,2);    
                        %HDR.NS = floor(mod(HDR.FLAG.Byte,128)/8);    
                        for k = 1:HDR.NS,
                                HDR.LeadPos(k,1:2) = fread(fid,[1,2],'uint32');    
                                HDR.Lead(k,1) = fread(fid,1,'uint8');    
                        end;
                        LeadIdTable = 	{'I';'II';'V1';'V2';'V3';'V4';'V5';'V6';'V7';'V2R';'V3R';'V4R';'V5R';'V6R';'V7R';'X';'Y';'Z';'CC5';'CM5';'left arm';'right arm';'left leg';'I';'E';'C';'A';'M';'F';'H';'I-cal'};
                        if max(HDR.Lead)<length(LeadIdTable),        
                                HDR.Label = LeadIdTable(HDR.Lead);
                        end;
                        
                elseif section.ID==4, 
                        HDR.SCP4.L = fread(fid,1,'int16');    
                        HDR.SCP4.M = fread(fid,1,'int16');    
                        HDR.SCP4.N = fread(fid,1,'int16');    
                        for k=1:HDR.SCP4.N,
                                HDR.SCP4.B(k).type = fread(fid,1,'uint16');   
                                HDR.SCP4.B(k).snum = fread(fid,1,'uint32');   
                                HDR.SCP4.B(k).fc1 = fread(fid,1,'uint32');   
                                HDR.SCP4.B(k).ty0 = fread(fid,1,'uint32');   
                        end;
                        HDR.SCP4.PA = fread(fid,[2,HDR.SCP4.N],'uint32')';   
                        
                elseif section.ID==5,
                        HDR.Cal = fread(fid,1,'int16');    
                        HDR.Dur = fread(fid,1,'int16');    
                        HDR.SampleRate = 1e6/HDR.Dur;
                        HDR.CodeTyp = fread(fid,1,'uint8');    
                        HDR.CodeTyp2 = fread(fid,1,'uint8');    
                        HDR.PhysDim = 'nV';
                        
                elseif section.ID==6, 
                        HDR.Cal = fread(fid,1,'int16');    
                        HDR.Dur = fread(fid,1,'int16');    
                        HDR.SampleRate2 = 1e6/HDR.Dur;
                        HDR.FLAG.DIFF = fread(fid,1,'uint8');    
                        HDR.FLAG.bimodal_compression = fread(fid,1,'uint8');    
                        HDR.SPR     = fread(fid,HDR.NS,'int16');
                        HDR.HeadLen = ftell(fid);
                        HDR.tmp.x = [];
                        
                        if ~isfield(HDR,'SCP2'),
                                for k = 1:HDR.NS,
                                        HDR.SCP6.S(:,k) = fread(fid,HDR.SPR(k),'int16');    
                                end;
                        else
                                
                                for k = 1:HDR.NS,
                                        SCP6.data{k} = fread(fid,HDR.SPR(k),'uint8');    
                                end;
                                
                                if HDR.SCP2.NHT~=19999,
                                        fprintf(HDR.FILE.stderr,'Warning SOPEN SCP-ECG: user specified Huffman Table not supported\n');
                                        return;
                                elseif HDR.SCP2.NHT==19999,
                                        HuffTab = DHT;
                                end;
                                
                                for k = 1:HDR.NS,
                                        tmp = SCP6.data{k};
                                        accu = [tmp(4)+256*tmp(3)+65536*tmp(2)+2^24*tmp(1)];
                                        %accu = bitshift(accu,HDR.SCP2.prefix,32);
                                        c  = 0; %HDR.SCP2.prefix;
                                        l  = 4;
                                        l2 = 0;
                                        clear x;
                                        Ntmp = length(tmp);
                                        tmp = [tmp; zeros(4,1)];
                                        while c<=32, %1:HDR.SPR(k),
                                                ixx = 1;
                                                while (bitand(accu,mask(ixx)) ~= PREFIX(ixx)), 
                                                        ixx = ixx + 1;
                                                end;
                                                if ixx < 18,
                                                        c = c + prefix(ixx);
                                                        %accu  = bitshift(accu, prefix(ixx),32);
                                                        accu  = mod(accu.*(2^prefix(ixx)),2^32);
                                                        l2    = l2 + 1;
                                                        x(l2) = HuffTab(ixx,1);
                                                        
                                                elseif ixx == 18,
                                                        c = c + prefix(ixx) + 8;
                                                        %accu  = bitshift(accu, prefix(ixx),32);
                                                        accu  = mod(accu.*(2^prefix(ixx)),2^32);
                                                        l2    = l2 + 1;
                                                        x(l2) = mod(floor(accu*2^(-24)),256);
                                                        
                                                        acc1 = mod(floor(accu*2^(-8)),2^8);
                                                        %accu = bitshift(accu, 8, 32);
                                                        accu = mod(accu*256, 2^32);
                                                        x(l2) = acc1-(acc1>=2^7)*2^6;
                                                        acc2 = 0;
                                                        for kk = 1:8,
                                                                acc2 = acc2*2 + mod(acc1,2);
                                                                acc1 = floor(acc1/2);
                                                        end;
                                                        %x(l2) = acc2;
                                                       
                                                elseif ixx == 19,
                                                        c = c + prefix(ixx);
                                                        %accu  = bitshift(accu, prefix(ixx),32);
                                                        accu  = mod(accu.*(2^prefix(ixx)),2^32);
                                                        l2    = l2 + 1;
                                                        while (c > 7) & (l < Ntmp),
                                                                l = l+1;
                                                                c = c-8;
                                                                accu = accu + tmp(l)*2^c;
                                                        end;
                                                        
                                                        acc1 = mod(floor(accu*2^(-16)),2^16);
                                                        %accu = bitshift(accu, 16, 32);
                                                        accu = mod(accu.*(2^16), 2^32);
                                                        
                                                        x(l2) = acc1-(acc1>=2^15)*2^16;
                                                        acc2 = 0;
                                                        for kk=1:16,
                                                                acc2 = acc2*2+mod(acc1,2);
                                                                acc1 = floor(acc1/2);
                                                        end;
                                                        %x(l2) = acc2;
                                                        c = c + 16;
                                                end;
                                                
                                                while (c > 7) & (l < Ntmp),
                                                        l = l+1;
                                                        c = c-8;
                                                        accu = accu + tmp(l)*(2^c);
                                                end;
                                                %accu,
                                        end;
                                        HDR.SCP6.S(:,k) = x(1:end-1)';
                                end;
                        end;
                        
                        S2 = HDR.SCP6.S;
                        HDR.SCP.data = S2;
                        
                        S2 = HDR.SCP.data;
                        if HDR.FLAG.DIFF==0,
                                HDR.SCP.data = S2;    
                        elseif HDR.FLAG.DIFF==1,
                                HDR.SCP.data = cumsum(S2);    
                        elseif HDR.FLAG.DIFF==2,
                                for k1 = 3:size(S2,1);
                                        S2(k1,:) = S2(k1,:) + [2,-1]*S2(k1-(1:2),:);
                                end;
                                HDR.SCP.data = S2;
                        end;
                        
                        if HDR.FLAG.bimodal_compression,
                                F = HDR.SampleRate/HDR.SampleRate2;
                                S1 = HDR.SCP.data;
                                N  = size(S1,1);
                                ix = ones(F,1)*[1:N]; ix = ix(:);
                                S2 = S1(ix,:);
                                
                                if F==4,
                                        D = diff(S1)/F;
                                        ix = (1:N-1)*F; 
                                        S2(ix,:) = S2(ix,:) + D;
                                        
                                        ix = (1:N-1)*F+1; 
                                        S2(ix,:) = S2(ix,:) - 2*D;
                                        
                                        ix = (1:N-1)*F+2; 
                                        S2(ix,:) = S2(ix,:) - D;
                                else
                                        fprintf(HDR.FILE.stderr,'Error SOPEN SCP-ECG: Biomodal compression with Factor %i not supported\n',F);	
                                        return;
                                end;
                                HDR.SCP.data = S2;
                        end;
                        
                elseif section.ID==7, 
                        HDR.SCP7.byte1 = fread(fid,1,'uint8');    
                        HDR.SCP7.Nspikes = fread(fid,1,'uint8');    
                        HDR.SCP7.meanPPI = fread(fid,1,'uint16');    
                        HDR.SCP7.avePPI = fread(fid,1,'uint16');    
                        
                        for k=1:HDR.SCP7.byte1,
                                HDR.SCP7.RefBeat{k} = fread(fid,16,'uint8');    
                                %HDR.SCP7.RefBeat1 = fread(fid,16,'uint8');    
                        end;
                        
                        for k=1:HDR.SCP7.Nspikes,
                                tmp = fread(fid,16,'uint16');    
                                tmp(1,2) = fread(fid,16,'int16');    
                                tmp(1,3) = fread(fid,16,'uint16');    
                                tmp(1,4) = fread(fid,16,'int16');    
                                HDR.SCP7.ST(k,:) = tmp;
                        end;
                        for k=1:HDR.SCP7.Nspikes,
                                tmp = fread(fid,6,'uint8');    
                                HDR.SCP7.ST2(k,:) = tmp;
                        end;
                        HDR.SCP7.Nqrs = fread(fid,1,'uint16');    
                        HDR.SCP7.beattype = fread(fid,HDR.SCP7.Nqrs,'uint8');    
                        
                        HDR.SCP7.VentricularRate = fread(fid,1,'uint16');    
                        HDR.SCP7.AterialRate = fread(fid,1,'uint16');    
                        HDR.SCP7.QTcorrected = fread(fid,1,'uint16');    
                        HDR.SCP7.TypeHRcorr = fread(fid,1,'uint8');    
                        
                        len = fread(fid,1,'uint16');
                        tag = 255*(len==0); 
                        k1 = 0;
                        while tag~=255,
                                tag = fread(fid,1,'uchar');    
                                len = fread(fid,1,'uint16');    
                                field = fread(fid,[1,len],'uchar');    
                                
                                if tag == 0,	
                                        HDR.Patient.LastName = char(field);
                                elseif tag == 1,
                                        
                                end;
                        end;
                        HDR.SCP7.P_onset = fread(fid,1,'uint16');    
                        HDR.SCP7.P_offset = fread(fid,1,'uint16');    
                        HDR.SCP7.QRS_onset = fread(fid,1,'uint16');    
                        HDR.SCP7.QRS_offset = fread(fid,1,'uint16');    
                        HDR.SCP7.T_offset = fread(fid,1,'uint16');    
                        HDR.SCP7.P_axis = fread(fid,1,'uint16');    
                        HDR.SCP7.QRS_axis = fread(fid,1,'uint16');    
                        HDR.SCP7.T_axis = fread(fid,1,'uint16');    
                        
                        
                elseif section.ID==8, 
                        HDR.SCP8.byte1 = fread(fid,1,'uint8');    
                        
                elseif section.ID==9, 
                        HDR.SCP9.byte1 = fread(fid,1,'uint8');    
                        
                elseif section.ID==10, 
                        HDR.SCP10.byte1 = fread(fid,1,'uint8');    
                        
                elseif section.ID==11, 
                        
                end;
                
                %tmp = fread(fid,min(section.Length-16,1000),'uchar');    
                
                fseek(fid, pos+section.Length-2, -1);
                section.CRC = fread(fid,1,'uint16');
        end;
        
        HDR.FILE.FID = fid;
        HDR.FILE.OPEN = 0; 
        HDR.FILE.POS = 0; 
        HDR.AS.bpb = 2 * HDR.NS;
        
        fclose(HDR.FILE.FID);
end;

