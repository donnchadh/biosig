function [HDR, s] = detect_spikes_bursts(fn, chan, varargin)
% DETECT_SPIKES_BURSTS detects spikes and bursts of spikes in
% neural recordings.
% Spikes are detected when voltages increase is larger than 20 V/s
% within a 0.2 ms window. Spikes with an interspike interval smaller
% than 75 ms are considered a burst. The results are stored as an
% event table.
%
%
% HDR = detect_spikes_bursts(filename, chan)
% ... = detect_spikes_bursts(HDR, data)
% ... = detect_spikes_bursts(... ,'OptimumJK')
% ... = detect_spikes_bursts(... ,'-o',outputFilename)
% ... = detect_spikes_bursts(... ,'-e',eventFilename)
% ... = detect_spikes_bursts(... ,'-b',burstFilename)
% ... = detect_spikes_bursts(... ,'slopeThreshold',slopeThreshold)
% ... = detect_spikes_bursts(... ,'winlen',winlen)
% ... = detect_spikes_bursts(... ,'dT_Burst',dT_Burst)
% ... = detect_spikes_bursts(... ,'dT_Exclude',dT_Exclude)
% ... = detect_spikes_bursts(... ,[slopeThreshold [, winlen [, dT_Burst [,dT_Exclude ]]] ])
% ... = detect_spikes_bursts(... ,'OptimumJK')
% ... = detect_spikes_bursts(... ,'-o',outputFilename)
% ... = detect_spikes_bursts(... ,'-b',burstFilename)
% [HDR, data] = detect_spikes_bursts(...)
%
% Input:
% 	filename: name of source file 
%	chan	list of channels that should be analyzed (default is 0: all channels)
%	HDR	header structure obtained by SOPEN, SLOAD, or meXSLOAD
%	data	signal data that should be analyized
%	slopeThreshold	[default: 5V/s] Spike is detected when
%		slope (over time winlen) exceeds this value
%	winlen	[default: .2e-3 s] windowlength in seconds for computing slope
%	dT_Burst	[default: 50e-3 s] am inter-spike-interval (ISI) exceeding this value,
%		marks the beginning of a new burst
%	dT_Exclude an interspike interval smaller than this value, indicates a
%		double detection, and the second detection is deleted.
%		in case of several consecutive ISI's smaller than this value,
%		all except the first spikes are deleted.
%	'OptimumJK' overrides previously defined settings, and uses the optimal setting based on
%		an JK's data set. It sets slopeThreshold = 2.5, dT=450e-6, dT_Exclude = 2e-3;
%		These settings were found based on 18 different recordings containing
%		23999 spikes, 3967 bursts, and over 9700 seconds of recorded data.
%	outputFilename
%		name of file for storing the resulting data with the
%		detected spikes and bursts in GDF format.
%	eventFilename
%		filename to store event inforamation in GDF format. this is similar to 
%		the outputFile, except that the signal data is not included and is, therefore,
%		much smaller than the outputFile
%	burstFilename
%		filename for the "burst table", containing basic properties of each burst,
%		(it is an ASCII file in <tab>-delimited format)
%
%	Arguments can appear in any order and multiple times (except for filename, chan, HDR and data),
%	In case of conflicting definitions, the latest definition has highest precedence and is used.
%
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

%%%%% default settings %%%%%
dT = .2e-3;		%%% smoothing window length [s]
dT_Burst = 50e-3;	%%% smaller ISI is a burst [s]
dT_Exclude = [];	%%% for identifying double detections, spikes with smaller ISI are excluded
slopeThreshold = 5; 	%% [V/s]
outFile = [];
evtFile = [];
burstFile = [];


%%%%% analyze input arguments %%%%%
k = 1;
K = 0;
while k <= length(varargin)
	if ischar(varargin{k})
		if (strcmp(varargin{k},'-o'))
			k = k + 1;
			outFile = varargin{k};
		elseif (strcmp(varargin{k},'-e'))
			k = k + 1;
			evtFile = varargin{k};
		elseif (strcmp(varargin{k},'-b'))
			k = k + 1;
			burstFile = varargin{k};
		elseif (strcmpi(varargin{k},'slopeThreshold'))
			k = k + 1;
			slopeThreshold = varargin{k};
		elseif (strcmpi(varargin{k},'winlen'))
			k = k + 1;
			dT = varargin{k};
		elseif (strcmpi(varargin{k},'dT_Burst'))
			k = k + 1;
			dT_Burst = varargin{k};
		elseif (strcmpi(varargin{k},'dT_Exclude'))
			k = k + 1;
			dT_Exclude = varargin{k};
		elseif (strcmpi(varargin{k},'OptimumJK'))
			slopeThreshold = 2.5; %
			dT = .450e-3;
			dT_Exclude = 2e-3;
		else
			warning(sprintf('unknown input argument <%s>- ignored',varargin{k}));
		end;
	elseif isnumeric(varargin{k})
		K = K + 1
		switch (K)
		case {1}
			slopeThreshold = varargin{k}; %% [V/s]
		case {2}
			dT = varargin{k};		%%% smoothing window length [s]
		case {3}
			dT_Burst = varargin{k};		%%% smaller ISI is a burst [s]
		case {4}
			dT_Exclude = varargin{k};	%%% smaller ISI is a burst [s]
		otherwise
			warning(sprintf('unknown input argument <%f> - ignored',varargin{k}));
		end;
	end;
	k = k+1;
end;


Fs = 20000; 	% assumed samplerate
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%	load data
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	if ischar(fn) && exist(fn,'file') && any(size(chan)==1)
		winlen = Fs*.1;
		[s, HDR] = sload(fn, 0, 'NUMBER_OF_NAN_IN_BREAK', winlen);
		if Fs < HDR.SampleRate,
			winlen   = HDR.SampleRate * .1;
			[s, HDR] = sload(fn, 0, 'NUMBER_OF_NAN_IN_BREAK', winlen);
		end;
		if chan==0, chan = 1:HDR.NS; end;
	elseif isstruct(fn)
		HDR = fn;
		s = chan;
		chan = 1:size(s,2);
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

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%	Set Parameters for Spike and Burst Detection
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	B  = [.5; zeros(HDR.SampleRate*dT - 1, 1); -.5];
	HDR.BurstTable = [];

	for ch = chan(:)';	% look for each channel

		%%%%%%%	Spike Detection %%%%%%%%%%%%%%%%%%%
		tmp = filter(B, dT, s(:,ch));
		OnsetSpike = find( diff (tmp > slopeThreshold) > 0); 	%% spike onset time [samples]
		% --- remove double detections < 1 ms
		if ~isempty(dT_Exclude)
			OnsetSpike = OnsetSpike([1; 1+find(diff(OnsetSpike) > Fs * dT_Exclude)]);
		end;
if 1, %% new
		EVENT.TYP = [EVENT.TYP; repmat(hex2dec('0201'), size(OnsetSpike))];
		EVENT.POS = [EVENT.POS; OnsetSpike];
		EVENT.DUR = [EVENT.DUR; repmat(1,  size(OnsetSpike))];
		EVENT.CHN = [EVENT.CHN; repmat(ch, size(OnsetSpike,1), 1) ];
else
		%%%%%%% Burst Detection %%%%%%%%%%%%%%%%%%%
		OnsetBurst = OnsetSpike ( [1; 1 + find( diff(OnsetSpike) > Fs * dT_Burst ) ] );

		OnsetBurst(end+1) = size(s,1) + 1;
		DUR        = repmat(NaN, size(OnsetBurst));
		BurstTable = repmat(NaN, length(OnsetBurst), 6);

		m2  = 0;
		t0  = [1; HDR.EVENT.POS(HDR.EVENT.TYP==hex2dec('7ffe'))];
		for m = 1:length(OnsetBurst)-1,	% loop for each burst candidate
			tmp = OnsetSpike( OnsetBurst(m) <= OnsetSpike & OnsetSpike < OnsetBurst(m+1) );
			d = diff(tmp);
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
		ix = find(~isnan(DUR));
		HDR.BurstTable = [HDR.BurstTable; BurstTable(ix,:)];
		OnsetBurst     = OnsetBurst(ix);
		DUR            = DUR(ix);

		EVENT.TYP = [EVENT.TYP; repmat(hex2dec('0201'), size(OnsetSpike)); repmat(hex2dec('0202'), size(OnsetBurst))];
		EVENT.POS = [EVENT.POS; OnsetSpike; OnsetBurst];
		EVENT.DUR = [EVENT.DUR; repmat(1,  size(OnsetSpike)); DUR];
		EVENT.CHN = [EVENT.CHN; repmat(ch, size(OnsetSpike,1) + size(DUR,1), 1) ];
end
	end;

	HDR.EVENT = EVENT;
	vararg = {'dT_Burst', dT_Burst, 'dT_Exclude', dT_Exclude};
	if ~isempty(evtFile) 
		vararg{end+1}='-e';
		vararg{end+1}=evtFile;
	end;
	if ~isempty(burstFile) 
		vararg{end+1}='-b';
		vararg{end+1}=burstFile;
	end;
	HDR = spikes2bursts(HDR,vararg{:});

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%	Output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	if ~isempty(outFile)
		%%% write data to output
		HDR.TYPE  = 'GDF';
		HDR.VERSION = 3;
		%[p,f,e]=fileparts(fn);
		HDR.FILE = [];
		HDR.FileName  = outFile;
		HDR.FILE.Path = '';
		HDR.PhysMax   = max(s);
		HDR.PhysMin   = min(s);
		HDR.DigMax(:) =  2^15-1;
		HDR.DigMin(:) = -2^15;
		HDR.GDFTYP(:) = 3;
		HDR.FLAG.UCAL = 0;
		HDR.NRec = size(s,1);
		HDR.SPR = 1;
		HDR.Dur = 1/HDR.SampleRate;
		HDR = rmfield(HDR,'AS');
		HDR = sopen(HDR,'w');
	if (HDR.FILE.FID<0) fprintf(2,'Warning can not open file <%s> - GDF file can not be written\n',HDR.FileName); break; end;
		HDR = swrite(HDR,s);
		HDR = sclose(HDR);
	end;

return;
% the following part has moved into spikes2bursts

	if ~isempty(burstFile),
		fid = fopen(burstFile,'w');
	if (fid<0) fprintf(2,'Warning can not open file <%s> - burst table not written\n',burstFile); break; end;
		fprintf(fid,'#Burst\t#chan\t#sweep\tOnset [s] \t#spikes\tavgISI [ms] \tminISI [ms] \n');
		for m = 1:size(HDR.BurstTable,1);
			fprintf(fid,'%3d\t%d\t%d\t%9.5f\t%d\t%6.2f\t%6.2f\n', m, HDR.BurstTable(m,:) );
		end;
		if (fid>3) fclose(fid); end;
	end;

