function [HDR]=gtfopen(HDR,PERMISSION,arg3,arg4,arg5,arg6)
% GTFOPEN reads files in the Galileo Transfer format (GTF) 
%
% HDR = gtfopen(Filename,PERMISSION);
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

%	$Id: gtfopen.m,v 1.3 2005-04-01 10:57:25 schloegl Exp $
%	(C) 2005 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/



        HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le');
        
        % read 3 header blocks 
        HDR.H1 = fread(HDR.FILE.FID,[1,512],'uint8');
        HDR.H2 = fread(HDR.FILE.FID,[1,15306],'int8');
        HDR.H3 = fread(HDR.FILE.FID,[1,8146],'uint8');           
        
        HDR.L1 = char(reshape(HDR.H3(1:650),65,10)');                   
        HDR.L2 = char(reshape(HDR.H3(650+(1:20*16)),16,20)');
        HDR.L3 = reshape(HDR.H3(1070+32*3+(1:232*20)),232,20)';
        
        HDR.Label = char(reshape(HDR.H3(1071:1070+32*3),3,32)');        % channel labels
        
        [H.i8,count] = fread(HDR.FILE.FID,inf,'int8');
        fclose(HDR.FILE.FID);
        
        if all(HDR.H1(33:43)==abs('ST24256.TDR'))
                HDR.NS   = 24; 
                HDR.SampleRate = 256; 
        else
                fprintf(2,'ERROR SOPEN (%s): Currently, only driver ST24256.TDR (24 channels and 256 Hz Sampling Rate) is supported.\n\t If you want support for your data, send the data to <a.schloegl@ieee.org>.\n',HDR.FileName);
                return; 
        end
        HDR.Dur  = 10; 
        HDR.SPR  = 2560; 
        HDR.bits = 8; 
        HDR.TYPE = 'native'; 
        HDR.THRESHOLD = repmat([-127,127],HDR.NS,1);    % support of overflow detection
        HDR.FILE.POS = 0; 
        HDR.Label = HDR.Label(1:HDR.NS,:);
        
        % Scaling(Calibration) not supported yet
        fprintf(2,'Warning SOPEN (%s): implementation of GTF-format is not completed.\n\t- Scaling is not implemented\n',HDR.FileName);
        HDR.Calib = sparse(2:HDR.NS+1,1:HDR.NS,1);
        HDR.FLAG.UCAL = 1;
        
        t2 = 9248+(0:floor(count/63488)-1)*63488;
        HDR.NRec = length(t2);
        [s2,sz]  = trigg(H.i8,t2,1,61440);
        HDR.data = reshape(s2,[HDR.NS,sz(2)/HDR.NS*HDR.NRec])';
        
        HDR.GTF.i8 = H.i8; 
        HDR.GTF.t2 = t2; 

        
t2 = HDR.GTF.t2; 
% check 
[s3,sz] = trigg(HDR.GTF.i8,t2,-2000,-86);
s3 = squeeze(reshape(s3,sz));
ix = find(~all(s3==s3(:,ones(1,HDR.NRec)),2));
if length(ix)>0;
        fprintf(2,'Warning GTFOPEN (%s): decoding corrupted. Something might be wrong!!!. Contact <a.schloegl@ieee.org> for further checks.\n',HDR.FileName);
end;

[s4,sz]  = trigg(HDR.GTF.i8,t2,-2047,0);
s4 = squeeze(reshape(s4,sz));
ix = find(~all(s4==s4(:,ones(1,HDR.NRec)),2))
s4(ix,:)

% these may contain some information, but its not clear what kind of informations
ix4 = [4:13,1964:1965]-1;       
HDR.GTK.s4 = s4(ix4,:);
