function [HDR] = leadidcodexyz(arg1)
% LeadIdCodeXYZ uses the Label information for computing the 
%       LeadIdCode and the XYZ position of the EEG Electrodes
%       according to Annex A of FEF Vital Signs Format [1]
%
%   HDR = leadidcodexyz(HDR); 
%	adds HDR.LeadIdCode and HDR.ELEC.XYZ, if needed. 
%
% see also: SLOAD, SOPEN, PHYSICALUNITS, doc/leadidtable_scpecg.txt, doc/elecpos.txt
%
% Reference(s): 
% [1] CEN/TC251/PT40 (2001)	
% 	File Exchange Format for Vital Signs - Annex A 


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

%	$Id: leadidcodexyz.m,v 1.4 2006-08-11 16:35:21 schloegl Exp $
%	Copyright (C) 2006 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/




global BIOSIG_GLOBAL;
if ~isfield(BIOSIG_GLOBAL,'ISLOADED_XYZ')
	BIOSIG_GLOBAL.ISLOADED_XYZ = 0 ; 
end; 
if ~BIOSIG_GLOBAL.ISLOADED_XYZ; 
	f = which('getfiletype.m'); 	% identify path to biosig
	[p,f,e] = fileparts(f); 
	[p,f,e] = fileparts(p); 
	
        BIOSIG_GLOBAL.ISLOADED_XYZ = 0 ;

        % load table 
        fid = fopen(fullfile(p,'doc','elecpos.txt'),'r');
        t = char(fread(fid,[1,inf],'char'));
        fclose(fid);

        % extract table information       
        N = 0;
        Code = repmat(NaN,100,1); Phi=Code; Theta=Code;
        while ~isempty(t)
                [x,t] = strtok(t,[10,13]);
                if isempty(x)
                elseif strncmp(x,'#',1)
                else
                        N = N+1;
                        [num,status,strarray] = str2double(x);
                        Code(N) = num(1);
                        Labels{N,1}=upper(strarray{2});
                        Phi(N) = num(3);
                        Theta(N) = num(4);
                end;
        end;
        Phi   = Phi(:)  *pi/180;
        Theta = Theta(:)*pi/180;
        
        % loading is done only once. 
        BIOSIG_GLOBAL.XYZ   = [sin(Theta).*cos(Phi), sin(Theta).*sin(Phi), cos(Theta)];
        BIOSIG_GLOBAL.LeadIdCode = Code;
        BIOSIG_GLOBAL.Label = Labels;

        BIOSIG_GLOBAL.ISLOADED_XYZ = 1 ;
end; 


if nargin<1,
        HDR.LeadIdCode = BIOSIG_GLOBAL.LeadIdCode;
        HDR.Label = BIOSIG_GLOBAL.Label;
        HDR.ELEC.XYZ = BIOSIG_GLOBAL.XYZ; 
        HDR.TYPE = 'ELPOS'; 
        
else    % electrode code and position
        if isstruct(arg1)
                HDR = arg1; 
        else
                HDR.Label = arg1; 
        end; 
        tmp.flag1 = isfield(HDR,'ELEC');
        if tmp.flag1,
                tmp.flag1 = isfield(HDR.ELEC,'XYZ');
        end;
        tmp.flag2 = isfield(HDR,'LeadIdCode');

        if (~tmp.flag1 | ~tmp.flag2),
                HDR.Label = cellstr(HDR.Label); 
                for k = 1:length(HDR.Label);
                        ix = strmatch(upper(deblank(HDR.Label{k})),BIOSIG_GLOBAL.Label);
                        if (length(ix)==1),
                                LeadIdCode = BIOSIG_GLOBAL.LeadIdCode(ix);
                                XYZ = BIOSIG_GLOBAL.XYZ(ix,:);
                        else
                                LeadIdCode = 0;
                                XYZ = [0,0,0]; 
                        end;
                        
                        if ~tmp.flag2
                                HDR.LeadIdCode(k,1) = LeadIdCode;
                        end;
                        if ~tmp.flag1
                                HDR.ELEC.XYZ(k,1:3) = XYZ;
                        end;
                end;
        end;
end;

