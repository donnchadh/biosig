function [out] = physicalunits(arg1)
% PHYSICALUNITS converts PhysDim inte PhysDimCode and vice versa
% according to Annex A of FEF Vital Signs Format [1]
%
%   HDR = physicalunits(HDR); 
%	adds HDR.PhysDim or HDR.PhysDimCode, if needed
%
%   PhysDim = physicalunits(PhysDimCode); 
%	converts Code of PhysicalUnits into descriptive physical units 
%
%   PhysDimCode = physicalunits(PhysDim); 
%	converts descriptive units into Code for physical units. 
%
%
% see also: SLOAD, SOPEN, doc/DecimalFactors.txt, doc/units.csv
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

%	$Id: physicalunits.m,v 1.2 2005-10-23 19:10:41 schloegl Exp $
%	Copyright (C) 2005 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/



global BIOSIG_GLOBAL;
if ~isfield(BIOSIG_GLOBAL,'ISLOADED')
	BIOSIG_GLOBAL.ISLOADED = 0 ; 
end; 
if ~BIOSIG_GLOBAL.ISLOADED; 
	f=which('getfiletype.m'); 	% identify path to biosig
	[p,f,e]=fileparts(f); 
	[p,f,e]=fileparts(p); 
	
	BIOSIG_GLOBAL.ISLOADED = 0 ; 
	fid  = fopen(fullfile(p,'doc','DecimalFactors.txt'),'r');
        line = fgetl(fid);
        N1   = 0; N2 = 0; 
        while ~feof(fid), 
		if ~strncmp(line,'#',1),
			N1 = N1 + 1;
               		[n,v,s]=str2double(line);
               		n = n(~v);
               		DecimalFactor.Code(N1,1) = n(2);
               		DecimalFactor.Cal(N1,1) = n(1);
               		s = s(~~v);
               		if any(v)
                      		DecimalFactor.Name(N1,1) = s(1);
                      		DecimalFactor.Prefix(N1,1) = s(2);
                       	end;
                end;	
                line = fgetl(fid);
        end;
	fclose(fid);
	BIOSIG_GLOBAL.DecimalFactor = DecimalFactor;

	fid = fopen(fullfile(p,'doc','units.csv'));
        line = fgetl(fid);
        N1 = 0; N2 = 0;
        while ~feof(fid),
		N2 = N2 + 1;
		if ~strncmp(line,'#',1),
			ix = mod(cumsum(line=='"'),2);
			tmp = line; 
			tmp(~~ix) = ' '; 
			ix  = find(tmp==',');
			if (length(ix)~=3)
				fprintf(2,'Warning: line (%3i: %s) not valid\n',N2,line);
			else
				t1 = line(1:ix(1)-1);
				t2 = line(ix(1)+1:ix(2)-1);
				t3 = line(ix(2)+1:ix(3)-1);
				t4 = line(ix(3)+1:end);
				Code = str2double(t1);
				if ~isempty(Code)
					N1 = N1 + 1;
					UnitsOfMeasurement.Code(N1,1)   = Code;
					ix = min(find([t2, '[' ] == '['))-1;
					UnitsOfMeasurement.Symbol{N1,1} = char(t2(2:ix-1));
				end;	
              		end;
                end;	
                line = fgetl(fid);
        end;
	fclose(fid);
	BIOSIG_GLOBAL.Units = UnitsOfMeasurement;

	BIOSIG_GLOBAL.ISLOADED = 1;
end; 


if isstruct(arg1) 
	HDR = arg1; 
	
	if 0, 
	elseif  isfield(HDR,'PhysDim') &  isfield(HDR,'PhysDimCode')
		Code = physicalunits(HDR.PhysDim);
		if ~isequal(Code(:),HDR.PhysDimCode(:))
			warning('PhysDim and PhysDimCode differ');
			[Code(:),HDR.PhysDimCode(:)]
		end;		
	elseif ~isfield(HDR,'PhysDim') &  isfield(HDR,'PhysDimCode')
		HDR.PhysDim = physicalunits(HDR.PhysDimCode);
	elseif  isfield(HDR,'PhysDim') % ~isfield(HDR,'PhysDimCode')
		HDR.PhysDimCode = physicalunits(HDR.PhysDim);
	elseif ~isfield(HDR,'PhysDim') & ~isfield(HDR,'PhysDimCode')
		warning('Neither PhysDim nor PhysDimCode defined');
	end;
	out = HDR;	

elseif isnumeric(arg1)
	s = mod(arg1,32); 
	n = bitand(arg1,2^16-32);
	for k=1:length(n); 
		t1 = BIOSIG_GLOBAL.DecimalFactor.Prefix{BIOSIG_GLOBAL.DecimalFactor.Code==s(k)};
		t2 = BIOSIG_GLOBAL.Units.Symbol{BIOSIG_GLOBAL.Units.Code==n(k)};
		PhysDim{k,1} = [t1,t2];
	end;	
	out = strvcat(PhysDim);
	
elseif ischar(arg1) | iscell(arg1) 
	arg1 = cellstr(arg1);
	N    = length(arg1); 
	Code = zeros(N,1); 	% default value is 0 (unknown)
	for k=1:N; 
		unit = deblank(arg1{k});
		if 0,
		
		elseif strcmpi(unit,'-')	% dimensionless
               		Code(k) = 512;
		elseif strcmpi(unit,'percent')
               		Code(k) = 544;
		elseif strcmpi(unit,'%')
               		Code(k) = 544;
		elseif strcmpi(unit,'degree')
               		Code(k) = 736;
		elseif strcmpi(unit,'rad')
               		Code(k) = 768;
		elseif strcmp(unit,'Hz')
              		Code(k) = 2496;
		elseif strcmp(unit,'mmHg')
              		Code(k) = 3872;
		elseif strcmp(unit,'V')
               		Code(k) = 4256;
		elseif strcmp(unit,'mV')
               		Code(k) = 4256 + 18;
		elseif strcmp(unit,'uV')
              		Code(k) = 4256 + 19;
		elseif strcmp(unit,'K')
              		Code(k) = 4384;
		elseif strcmp(unit,'°F')
              		Code(k) = 4416;
		elseif strcmp(unit,'°C')
              		Code(k) = 6048;
              	else 
	       		ix = [];
			for k1=1:length(BIOSIG_GLOBAL.DecimalFactor.Code)
			for k2=1:length(BIOSIG_GLOBAL.Units.Code)
			if strcmp(unit,[BIOSIG_GLOBAL.DecimalFactor.Prefix{k1},BIOSIG_GLOBAL.Units.Symbol{k2}])
				ix = [ix,BIOSIG_GLOBAL.Units.Code(k2) + BIOSIG_GLOBAL.DecimalFactor.Code(k1)];
			end;
			end;
			end;
			if length(ix)==1,
				Code(k) = ix; 
			end;	
                end
        end;        
        out = Code; 
        
end;
