function argout=sview(s,H),
% SVIEW - a simple signal viewer 
%    SVIEW(filename)
%    SVIEW(HDR)
%    SVIEW(S,HDR)
%
%
% See also: SLOAD 

%	$Revision: 1.2 $
%	$Id: sview.m,v 1.2 2004-04-08 16:47:47 schloegl Exp $ 
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
        if exist(s)==2,
                [s,H] = sload(s);
        end;
elseif isstruct(H)
        [s,H] = sload(H);
else 
        return;
end;

t  = s(:); 
t(isnan(t))=median(t);
dd = max(t)-min(t);

plot((1:size(s,1))'/H.SampleRate,((s+(ones(size(s,1),1)*(1:size(s,2)))*dd/(-2)+4*dd)),'-');
if 0, H.EVENT.N > 0,
        hold on;
        if ~isfield(H.EVENT,'DUR');
                plot(H.EVENT.POS/H.SampleRate, H.EVENT.CHN*dd, 'x');
                
        elseif 1,
                plot([H.EVENT.POS,H.EVENT.POS+H.EVENT.DUR]'/H.SampleRate,[dd;dd]*H.EVENT.CHN','+-');    
                
        else
                for k = 1:H.EVENT.N,
                        plot(H.EVENT.POS(k)+[0,H.EVENT.DUR(k)],[0,0],'+-');    
                end;
        end;
        hold off;
end;

title([H.FileName, ' generated with BIOSIG tools for Octave and Matlab(R)']);
xlabel('time t[s]');
ylabel(sprintf('Amplitude [%s]',H.PhysDim(1,:)));

if exist('OCTAVE_VERSION')<5;
        legend(H.Label);
end;

if nargout
        argout=H;
end;