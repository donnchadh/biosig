function [signal,H] = tload(FILENAME,TI1,CHAN,EVENTFILE,TI2)
% TLOAD loads and triggers signal data.  
%
% [signal,HDR] = tload(FILENAME, TI, [CHAN,] EVENTFILE, AI)
%       reads selected (CHAN) channels
%       if CHAN is 0, all channels are read 
%
% FILENAME  name of file, or list of filenames
% TI	    trigger interval [t1,t2] in seconds, relative to TRIGGER point
%	    	this interval defines the trigger information
% CHAN      list of selected channels
%           default=0: loads all channels
% EVENTFILE file of artifact scoring 
% AI	    Artifactinterval [t1,t2] in seconds, relative to TRIGGER point  	
%		this interval is used to check for any artifacts
%
% [signal,HDR] = tload(dir('f*.eeg'),...)
% [signal,HDR] = tload('f*.eeg', ...)
%  	loads channels CHAN from all files 'f*.emg'
%
% see also: SLOAD, SVIEW, SOPEN


%	$Id: tload.m,v 1.1 2004-12-04 19:05:07 schloegl Exp $
%	Copyright (C) 2004 by Alois Schloegl <a.schloegl@ieee.org>
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% Version 2 of the License, or (at your option) any later version.
%
% This library is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% Library General Public License for more details.
%
% You should have received a copy of the GNU Library General Public
% License along with this library; if not, write to the
% Free Software Foundation, Inc., 59 Temple Place - Suite 330,
% Boston, MA  02111-1307, USA.


if nargin<3; CHAN=0; end;
if isempty(CHAN), CHAN = 0; end;
if (CHAN<1) | ~isfinite(CHAN),
        CHAN=0;
end;

[s,HDR] = sload(FILENAME,CHAN); 

if nargin>3,
	if nargin<5, TI2 = TI1; end; 
	HDR = artifact_selection({HDR,EVENTFILE},TI2);
end;

TRIG = HDR.TRIG; 
if isfield(HDR,'ArtifactSelection')
	TRIG = TRIG(~HDR.ArtifactSelection);
end;

TI1 = TI1*HDR.SampleRate;
[signal,sz] = trigg(s,TRIG,TI1(1),TI1(2)-1);
signal = signal';

H = HDR; 
H.size = sz([2,3,1]);
H.FLAG.TRIGGERED = 1; 
H.SPR  = sz(2);
H.DUR  = diff(TI1)/H.SampleRate;
H.NRec = sz(3); 
H.TRIG = (0:H.NRec-1)'*H.SPR; 

	