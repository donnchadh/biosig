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

%	$Id: leadidcodexyz.m,v 1.10 2007-02-09 15:18:17 schloegl Exp $
%	Copyright (C) 2006,2007 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/




global BIOSIG_GLOBAL;
% BIOSIG_GLOBAL=0; %%% used for debugging, only. 

if ~isfield(BIOSIG_GLOBAL,'ISLOADED_XYZ')
	BIOSIG_GLOBAL.ISLOADED_XYZ = 0 ; 
end; 
if ~BIOSIG_GLOBAL.ISLOADED_XYZ; 
	f = which('getfiletype.m'); 	% identify path to biosig
	[p,f,e] = fileparts(f); 
	[p,f,e] = fileparts(p); 
	
        BIOSIG_GLOBAL.ISLOADED_XYZ = 0 ;

        N = 0;
        fid = fopen(fullfile(p,'doc','leadidtable_scpecg.txt'),'r');
        s = char(fread(fid,[1,inf],'char')); 
        fclose(fid);
        
        Code = repmat(NaN, 200, 1); Phi = Code; Theta = Code;
        while ~isempty(s),
        	[t,s] = strtok(s,[10,13]);
                if ~length(t)
                elseif ~strncmp(t,'#',1)    
                	ix3 = strfind(t,'MDC_ECG_LEAD_');
                        [t1,t2] = strtok(t(1:ix3-1),[9,32]);
                        [t2,t3] = strtok(t2,[9,32]);
                        id = str2double(t2);
                        N  = N + 1;
                        Labels{N,1}	  = t1;
                        Code(N,1)         = id;
                        Description{N,1}  = deblank(t3);
                        MDC_ECG_LEAD{N,1} = t(ix3+13:end);
                end;
        end;
        N1 = N;

        % load table 
        fid = fopen(fullfile(p,'doc','elecpos.txt'),'r');
        t = char(fread(fid,[1,inf],'char'));
        fclose(fid);

        % extract table information       
        while ~isempty(t)
                [x,t] = strtok(t,[10,13]);
                if isempty(x)
                elseif strncmp(x,'#',1)
                else
                        N = N + 1;
                        [num,status,strarray] = str2double(x);
                        Code(N,1)   = num(1);
                        Labels{N,1} = upper(strarray{2});
                        Phi(N,1)    = num(3);
                        Theta(N,1)  = num(4);
                end;
        end;
        Phi   = Phi(:)  *pi/180;
        Theta = Theta(:)*pi/180;

        
        % loading is done only once. 
        BIOSIG_GLOBAL.XYZ = [sin(Theta).*cos(Phi), sin(Theta).*sin(Phi), cos(Theta)];
        BIOSIG_GLOBAL.LeadIdCode   = Code;
        BIOSIG_GLOBAL.Label        = Labels;
        BIOSIG_GLOBAL.Description  = Description;
        BIOSIG_GLOBAL.MDC_ECG_LEAD = MDC_ECG_LEAD;

        BIOSIG_GLOBAL.ISLOADED_XYZ = 1;
end; 


if nargin<1,
        HDR.LeadIdCode = BIOSIG_GLOBAL.LeadIdCode;
        HDR.Label      = BIOSIG_GLOBAL.Label;
        HDR.ELEC.XYZ   = BIOSIG_GLOBAL.XYZ; 
        HDR.TYPE       = 'ELPOS'; 
        
else    % electrode code and position

        if isstruct(arg1)
                HDR = arg1; 
        elseif isnumeric(arg1),
                HDR.LeadIdCode = arg1; 
        else
                HDR.Label = arg1; 
        end;

        tmp.flag1 = isfield(HDR,'ELEC');
        if tmp.flag1,
                tmp.flag1 = isfield(HDR.ELEC,'XYZ');
        end;
        tmp.flag2 = isfield(HDR,'LeadIdCode');
        tmp.flag3 = isfield(HDR,'Label');

        if (~tmp.flag1 | ~tmp.flag2 | ~tmp.flag3),
        	if tmp.flag3;
                        if ischar(HDR.Label)
                                HDR.Label = cellstr(HDR.Label);
                        end;
	                NS = length(HDR.Label); 
	        else
	        	NS = length(HDR.LeadIdCode); 
	        end;      

                if tmp.flag3,
	                if ~tmp.flag1,
        	               HDR.ELEC.XYZ   = repmat(NaN,NS,3);
	        	end;
                	if ~tmp.flag2,
                       		HDR.LeadIdCode = repmat(NaN,NS,1);
	        	end;
                	for k = 1:NS;
	                        ix = strmatch(upper(deblank(HDR.Label{k})),BIOSIG_GLOBAL.Label,'exact');
	                        if length(ix)==2,
	                        	%%%%% THIS IS A HACK %%%%%
	                        	%% solve ambiguity for 'A1','A2'; could be EEG or ECG
	                        	if sum(HDR.LeadIdCode(1:k)>=996)>sum(HDR.LeadIdCode(1:k)<996)
	                        		%% majority are EEG electrodes,
	                        		ix = ix(find(BIOSIG_GLOBAL.LeadIdCode(ix)>996));
	                        	else	
	                        		%% majority are ECG electrodes,
	                        		ix = ix(find(BIOSIG_GLOBAL.LeadIdCode(ix)<996));
	                        	end;
	                        end; 	
        	                if (length(ix)==1),
                	                LeadIdCode = BIOSIG_GLOBAL.LeadIdCode(ix);
                        	        XYZ = BIOSIG_GLOBAL.XYZ(ix,:);
                        	else	
                                	LeadIdCode = 0;
	                                XYZ = [NaN,NaN,NaN]; 
        	                end;

                	        if ~tmp.flag1,
                        	        HDR.ELEC.XYZ(k,1:3) = XYZ;
                        	end;

                        	if ~tmp.flag2,
                                	HDR.LeadIdCode(k,1) = LeadIdCode;
	                        end;
                        end;
                else
                       	HDR.Label = cell(NS,1);
	               	for k = 1:NS;
                        	ix = find(BIOSIG_GLOBAL.LeadIdCode==HDR.LeadIdCode(k));
        	                if (length(ix)==1),
	                                HDR.Label{k} = BIOSIG_GLOBAL.Label{ix};
		                        if ~tmp.flag1,
        		                        HDR.ELEC.XYZ(k,1:3) = BIOSIG_GLOBAL.XYZ(ix,1:3);
                		        end;
               		        else
               		        	HDR.Label{k} = ['#',int2str(k)];
               		        end;
	               	end;
                end;
        end;
end;

