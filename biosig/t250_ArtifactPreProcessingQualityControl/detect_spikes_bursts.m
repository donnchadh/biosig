function [HDR, s] = detect_spikes_bursts(fn, chan)
% DETECT_SPIKES_BURSTS detects spikes and bursts of spikes in 
% neural recordings. 
% Spikes are detected when voltages increase is larger than 20 V/s
% within a 0.2 ms window. Spikes with an interspike interval smaller  
% than 75 ms are considered a burst. The results are stored as an 
% event table. 
%
% HDR = detect_spikes_bursts(filename, [, chan])
% [HDR,data] = detect_spikes_bursts(...)
%   
% Input: 
%     filename  
%     chan	list of channels that should be analyzed (default is 0: all channels)
% Output:  
%     HDR	header structure as defined in biosig
%     HDR.EVENT includes the detected spikes and bursts. 
%     HDR.BurstTable contains for each burst (each in a row) the following 5 numbers:
%	channel number, sweep number, OnsetTime within sweep [s], 
%	number of spikes within burst, and average inter-spike interval (ISI) [ms]
%     data	signal data, one channel per column
%	
% References: 
%

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

if nargin < 1, 
	help detect_spikes_bursts;
end;

if nargin < 2, 
	chan = 0; 
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%	load data 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	[s, HDR] = sload(fn);
	if chan==0, chan = 1:HDR.NS; end; 

	EVENT = HDR.EVENT; 
	if ~isfield(EVENT,'DUR');
		EVENT.DUR = zeros(size(EVENT.POS));
	end; 
	if ~isfield(EVENT,'CHN');
		EVENT.CHN = zeros(size(EVENT.TYP));
	end; 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%	Set Parameters for Spike and Burst Detection 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	TH = 20;	%%% steepness	[V/s]
	dT = .2e-3;	%%% smoothing window length [s]	
	dT_Burst = 75e-3;	%%% smaller ISI is a burst [s]
	B  = [.5; zeros(HDR.SampleRate*dT - 1, 1); -.5];
	HDR.BurstTable = [];

	for ch = chan(:)';	% look for each channel 	

		%%%%%%%	Spike Detection %%%%%%%%%%%%%%%%%%%
		tmp = filter(B, dT, s(:,ch));
		OnsetSpike = find( diff (tmp > TH) > 0); 	%% spike onset time [samples]

		%%%%%%% Burst Detection %%%%%%%%%%%%%%%%%%%
		OnsetBurst = OnsetSpike ( [1; 1 + find( diff(OnsetSpike) / HDR.SampleRate > dT_Burst ) ] );

		OnsetBurst(end+1) = size(s,1) + 1;
		DUR        = repmat(NaN, size(OnsetBurst));
		BurstTable = repmat(NaN, length(OnsetBurst), 5);

		m2  = 0;	
		t0  = [1; HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('7ffe'))];
		for m = 1:length(OnsetBurst)-1,	% loop for each burst candidate 
			tmp = OnsetSpike( OnsetBurst(m) <= OnsetSpike & OnsetSpike < OnsetBurst(m+1) );
			d = mean(diff(tmp));
			if length(tmp) > 1, 
				m2 = m2 + 1;	
				DUR(m) = length(tmp)*d; 
				ix = sum(t0 < OnsetBurst(m));
				T0 = t0(ix);
				BurstTable(m,:) = [ch, ix, (OnsetBurst(m) - T0)/HDR.SampleRate, length(tmp), 1000*d/HDR.SampleRate];
			% else 
			% 	single spikes are not counted as bursts, DUR(m)==NaN marks them as invalid 
			end; 
		end; 

		% remove single spike bursts 
		ix = find(~isnan(DUR));
		HDR.BurstTable = [HDR.BurstTable; BurstTable(ix,:)];
		OnsetBurst     = OnsetBurst(ix);
		DUR            = DUR(ix);
		
		EVENT.TYP = [EVENT.TYP; repmat(hex2dec('0201'), size(OnsetSpike)); repmat(hex2dec('0202'), size(OnsetBurst))]; 
		EVENT.POS = [EVENT.POS; OnsetSpike; OnsetBurst];
		EVENT.DUR = [EVENT.DUR; repmat(1,size(OnsetSpike)); DUR]; 
		EVENT.CHN = [EVENT.CHN; repmat(ch,size(EVENT.DUR)) ]; 
	end; 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%	Output 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	HDR.EVENT = EVENT; 


