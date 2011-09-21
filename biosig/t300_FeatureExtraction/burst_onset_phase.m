%function RES = burst_onset_phase(fn,ch,band, EVT)
% BURST_ONSET_PHASE computes the phase of burst onsets 
%
% ... = burst_onset_phase(fn, ch, [f1, f2], EVT)
% ... = burst_onset_phase(s, HDR, [f1, f2], EVT)
% [RES] = burst_onset_phase(...)
% 
% Input: 
%  fn	filename 
%  ch	channel number(s), default=0 (i.e. all)	
%  s	signal data 
%  HDR  header structure
%  [f1,f2]   edge frequencies of bandpass filter
%  EVT  header structure containing events of 
%       burst onset (0x0202) and 
%       start of new segments (0x7ffe)	
%
% Output:
%  RES  a complex matrix containing Amplitude and Phase 
%	for each burst onset (number of rows) and for 
%	each channel (number of columns). 
%  angle(RES) returns the phase at burst onset

%	$Id$
%	Copyright (C) 2011 by Alois Schloegl <alois.schloegl@gmail.com>
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/
%
%    BioSig is free software: you can redistribute it and/or modify
%    it under the terms of the GNU General Public License as published by
%    the Free Software Foundation, either version 3 of the License, or
%    (at your option) any later version.
%
%    BioSig is distributed in the hope that it will be useful,
%    but WITHOUT ANY WARRANTY; without even the implied warranty of
%    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%    GNU General Public License for more details.
%
%    You should have received a copy of the GNU General Public License
%    along with BioSig.  If not, see <http://www.gnu.org/licenses/>.

if ~isnumeric(band) || numel(band)~=2 
	error('arg3 (band) must have 2 numeric elements');
else
	f1 = band(1); 
	f2 = band(2); 
end;


if ischar(EVT) && exist(EVT,'file')
	[s,EVT] = sload(EVT);
elseif isstruct(EVT) && isfield(EVT,'POS') && isfield(EVT,'TYP')
	tmp = EVT; 
	EVT = []; 
	EVT.EVENT=tmp;
end; 
if ~isfield(EVT,'EVENT')
	error('arg4 (EVT) does not contain event structure');
end; 

winlen = 20000;

[s, HDR] = sload(fn, 1, 'NUMBER_OF_NAN_IN_BREAK', winlen);
ixSegStart0 = sort([1; HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('7ffe')); size(s,1)+1]);
ixSegStart1 = sort([1; EVT.EVENT.POS(EVT.EVENT.TYP==hex2dec('7ffe')); size(s,1)+1]);

if length(ixSegStart0)-length(ixSegStart1)
	error('number of segments differ between raw data and event file');
	return; 
end

iv = isnan(s);
s(iv) = 0;

S = fft(s,[],1); 
f = (0:size(s,1)-1)*HDR.SampleRate/size(s,1);

S(f1 > f | f > f2, :) = 0; 
if f1==0, S(f==0, : ) = S(f==0, : ) / 2; end; 

s     = 2 * ifft(S,[],1); 
s(iv) = NaN; 

tixOn = EVT.EVENT.POS(EVT.EVENT.TYP==hex2dec('0202'));
ix = []; 
for k = 1:length(ixSegStart0)-1,
	ix0 = ixSegStart0(k); 
	ix1 = ixSegStart1(k);
	t = tixOn(ixSegStart1(k) <= tixOn & tixOn < ixSegStart1(k+1));
	ix = [ix; ix0 - ix1 + t(:)];
end; 

%%% hack: workaround to nonsense markers 
ix = ix(ix<size(s,1));

RES = s(ix,:);




