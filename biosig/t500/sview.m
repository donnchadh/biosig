function [argout,s]=sview(s,H),
% SVIEW - a simple signal viewer 
%    SVIEW(filename)
%    SVIEW(HDR)
%    SVIEW(S,HDR)
%
%
% See also: SLOAD 

%	$Revision: 1.6 $
%	$Id: sview.m,v 1.6 2004-09-25 00:19:57 schloegl Exp $ 
%	Copyright (c) 2004 by Alois Schlögl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

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


if ischar(s),
        if nargin<2,
                [s,H] = sload(s);
        else
                [s,H] = sload(s,H);
        end;
elseif isstruct(s)
        [s,H] = sload(s);
else 
        return;
end;


if strcmp(H.TYPE,'BMP'),
	image(s);
	return;
end;

%s(abs(s)>1e3)=NaN;

[p,f,e]=fileparts(H.FileName);
fn=dir(fullfile(p,[f,'EOG',e]));
if 0, length(fn)==1,
        [R,tmp] = regress_eog(fullfile(p,fn.name),1:4,5:7);
        s = s*R.r0;
end; 
%[R,s0] = regress_eog('v608eog.bkr',1:4,5:7);R.r0,
%s = s*R.r0;

if isfield(H,'Label'),
        LEG = H.Label;
else
        LEG = '';
end;

t = s(:); 
%t(isnan(t))=median(t);
dd = max(t)-min(t);

plot((1:size(s,1))'/H.SampleRate,((s+(ones(size(s,1),1)*(1:size(s,2)))*dd/(-2)+4*dd)),'-');
if 0,H.EVENT.N > 0,
        hold on;
        if 0, 
        elseif isfield(H.EVENT,'DUR') & isfield(H.EVENT,'CHN');
                plot([H.EVENT.POS,H.EVENT.POS+H.EVENT.DUR]'/H.SampleRate,[dd;dd]*H.EVENT.CHN','+-');    

        elseif isfield(H.EVENT,'CHN');
                plot(H.EVENT.POS/H.SampleRate, H.EVENT.CHN*dd, 'x');
                
        elseif isfield(H.EVENT,'DUR');
                plot([H.EVENT.POS,H.EVENT.POS+H.EVENT.DUR]'/H.SampleRate,[dd;dd]*H.EVENT.CHN','+-');    
                
        else; 
                plot(H.EVENT.POS/H.SampleRate,dd*ones(H.EVENT.N,1),'^');    
                
        end;
        hold off;
        LEG = strvcat(LEG,'Events');
end;

tmp = H.FileName; tmp(tmp=='\')='/'; tmp(tmp=='_')=' ';
title([tmp, ' generated with BIOSIG tools for Octave and Matlab(R)']);
xlabel('time t[s]');
PhysDim = '';
if ~isempty(H.PhysDim),
        PhysDim = deblank(H.PhysDim(1,:));
end;
ylabel(sprintf('Amplitude [%s]',PhysDim));

if exist('OCTAVE_VERSION')<5;
        if ~isempty(LEG);
                legend(LEG);
        end;
end;

if nargout, 
        argout=H;
end;