function [HDR, s] = spikes2bursts(fn, dT_Burst)
% spikes2bursts convert spike trains into bursts. 
%  Spikes with an interspike interval smaller  
%  than 75 ms are considered a burst. The results are stored as an 
% event table. 
%
% HDR = spikes2bursts(filename)
% ... = spikes2bursts(HDR)
% ... = spikes2bursts(... [, dT_Burst ])
%   
% Input: 
%     filename  name of file containing spikes in the event table
%     HDR	header structure obtained by SOPEN, SLOAD, or meXSLOAD
%     dT_Burst	[default: 50e-3 s] am inter-spike-interval (ISI) exceeding this value, 
%		marks the beginning of a new burst  
%
% Output:  
%     HDR	header structure as defined in biosig
%     HDR.EVENT includes the detected spikes and bursts. 
%     HDR.BurstTable contains for each burst (each in a row) the following 5 numbers:
%	channel number, sweep number, OnsetTime within sweep [s], 
%	number of spikes within burst, and average inter-spike interval (ISI) [ms]
%     data	signal data, one channel per column
%		between segments, NaN values for 0.1s are introduced
%	
% References: 
%

%	$Id: detect_spikes_bursts.m 2739 2011-07-13 15:42:05Z schloegl $
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

if nargin < 1, 
	help spikes2bursts;
end;

if nargin < 2, 
	dT_Burst = 50e-3;	%%% smaller ISI is a burst [s]
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%	load data 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	if ischar(fn) && exist(fn,'file')	
		[s, HDR] = sload(fn);
	elseif isstruct(fn)
		HDR = fn; 
	else 
		help(mfilename); 
	end


	EVENT = HDR.EVENT; 
	if ~isfield(EVENT,'DUR');
		EVENT.DUR = zeros(size(EVENT.POS));
	end; 
	if ~isfield(EVENT,'CHN');
		EVENT.CHN = zeros(size(EVENT.TYP));
	end; 

	Fs = HDR.SampleRate; 
	chan = unique(EVENT.CHN);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%	Set Parameters for Burst Detection 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	HDR.BurstTable = [];

	for ch = chan(:)';	% look for each channel 	
		OnsetSpike = EVENT.POS((EVENT.CHN==ch) & (EVENT.TYP==hex2dec('0201'))); 	%% spike onset time [samples]
		if 0, %~isempty(dT_Exclude)
			% --- remove double detections < 1 ms
			OnsetSpike = OnsetSpike([1; 1+find(diff(OnsetSpike) > Fs * dT_Exclude)]); 
		end; 
		
		if isempty(OnsetSpike)
			continue;
		end;

		%%%%%%% Burst Detection %%%%%%%%%%%%%%%%%%%
		OnsetBurst = OnsetSpike ( [1; 1 + find( diff(OnsetSpike) > Fs * dT_Burst ) ] )

		OnsetBurst(end+1) = size(s,1) + 1;
		DUR        = repmat(NaN, size(OnsetBurst));
		BurstTable = repmat(NaN, length(OnsetBurst), 6);

		m2    = 0;	
		t0    = [1; EVENT.POS(EVENT.TYP==hex2dec('7ffe'))];
		for m = 1:length(OnsetBurst)-1,	% loop for each burst candidate 
			tmp = OnsetSpike( OnsetBurst(m) <= OnsetSpike & OnsetSpike < OnsetBurst(m+1) );
			d   = diff(tmp);
			if length(tmp) > 1, 
				m2 = m2 + 1;	
				DUR(m) = length(tmp)*mean(d); 
				ix = sum(t0 < OnsetBurst(m));
				T0 = t0(ix);
				BurstTable(m,:) = [ch, ix, (OnsetBurst(m) - T0)/HDR.SampleRate, length(tmp), 1000*mean(d)/HDR.SampleRate, 1000*min(d)/HDR.SampleRate];
			% else 
			% 	single spikes are not counted as bursts, DUR(m)==NaN marks them as invalid 
			end; 
		end; 

		% remove single spike bursts 
		ix             = find(~isnan(DUR));
		HDR.BurstTable = [HDR.BurstTable; BurstTable(ix,:)];
		OnsetBurst     = OnsetBurst(ix);
		DUR            = DUR(ix);
		
		EVENT.TYP = [EVENT.TYP; repmat(hex2dec('0201'), size(OnsetSpike)); repmat(hex2dec('0202'), size(OnsetBurst))]; 
		EVENT.POS = [EVENT.POS; OnsetSpike; OnsetBurst];
		EVENT.DUR = [EVENT.DUR; repmat(1,  size(OnsetSpike)); DUR]; 
		EVENT.CHN = [EVENT.CHN; repmat(ch, size(OnsetSpike,1) + size(DUR,1), 1) ]; 
	end; 


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%	Output 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	HDR.EVENT = EVENT; 


