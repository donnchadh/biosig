function [argout,s]=sview(s,H),
% SVIEW - a simple signal viewer 
%    SVIEW(filename)
%    SVIEW(HDR)
%    SVIEW(S,HDR)
%
% See also: SLOAD 

%	$Revision: 1.8 $
%	$Id: sview.m,v 1.8 2005-01-22 23:02:05 schloegl Exp $ 
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

elseif isnumeric(s) & (length(H.InChanSelect)==size(s,2))

else 
        return;
end;

%if strmatch(H.TYPE,{'BMP','PBMA','PGMA','PPMA','PBMB','PGMB','PPMB','XPM'}),
if isfield(H,'IMAGE');
	if exist('OCTAVE_VERSION','builtin')
		if (length(size(s))==3) & (size(s,3)==3)
			imshow(s(:,:,1),s(:,:,2),s(:,:,3));
		else
			imshow(s);
		end;	
	else	
		image(s);
	end;
	argout=H;
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
        LEG = H.Label(H.InChanSelect,:);
else
        LEG = '';
end;

t = s(:); 
%t(isnan(t))=median(t);
dd = max(t)-min(t);
%dd = max(std(s))*5;
%s = zscore(s);
%dd = 20;

H.AS.TIMECHAN = strmatch('Time',H.Label);
if (length(H.FILE)==1) & any(H.AS.TIMECHAN) & any(H.AS.TIMECHAN==H.InChanSelect),
	t = s(:,H.AS.TIMECHAN);
	s(:,H.AS.TIMECHAN) = NaN;
	X_Label = [H.Label(H.AS.TIMECHAN,:),' [',H.PhysDim(H.AS.TIMECHAN,:),']'];
elseif isnan(H.SampleRate)
	t = (1:size(s,1))';
	X_Label = 'samples';
else
	t = (1:size(s,1))'/H.SampleRate;
	X_Label = 'time [s]';
end;

if 0,
elseif isfield(H,'SegLen'), 
	EVENT.POS = H.SegLen(1:end-1)'+1;
	EVENT.Desc = {H.FILE.Name}';
	t = (1:size(s,1))';
	X_Label = 'samples';
elseif isfield(H.EVENT,'Desc'),
	EVENT = H.EVENT;
elseif ~isfield(H.EVENT,'Desc') & (length(H.EVENT.TYP)>0),
	g = sload('eventcodes.txt');
	ix = sparse(g.CodeIndex,1,1:length(g.CodeIndex));
	EVENT.POS = H.EVENT.POS;
	EVENT.Desc = {g.CodeDesc{ix(H.EVENT.TYP)}};
end;

plot(t,((s+(ones(size(s,1),1)*(1:size(s,2)))*dd/(-2)+4*dd)),'-');
if isfield(EVENT,'Desc')
	v = axis;
	hold on
	N = length(EVENT.POS);
	plot([1;1]*t(EVENT.POS)',v(3:4),':k')
	for k=1:length(EVENT.POS),
		txt = EVENT.Desc{k}; 
		if isempty(txt),txt='';end; 
		txt(txt=='_')=' ';
		h=text(t(EVENT.POS(k)),v(3:4)*[-.2;.8],txt);
		set(h,'Rotation',90,'VerticalAlignment','top');
	end;	
	v(4) = v(3:4)*[-.7;1.3]; axis(v);
	hold off;
end;

if 0, length(H.EVENT.POS) > 0,
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

if length(H.FILE)==1,
    tmp = H.FileName; tmp(find(tmp==92))='/'; tmp(find(tmp=='_'))=' ';
else
    tmp = '';
end;
title([tmp, ' generated with BIOSIG tools for Octave and Matlab(R)']);
xlabel(X_Label);
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
