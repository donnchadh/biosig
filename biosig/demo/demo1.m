% DEMO 1 - identifies QRS-complexes

%	$Revision: 1.5 $
%	$Id: demo1.m,v 1.5 2005-03-01 15:00:23 schloegl Exp $
%	Copyright (C) 2000-2003, 2005 by Alois Schloegl <a.schloegl@ieee.org>	
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


% load file
[F,P]=uigetfile('*.*','Pick an ECG file');

CHAN = 0; 
HDR  = sopen(fullfile(P,F),'r');
if HDR.NS > 1,
        tmp   = reshape(lower([HDR.Label,repmat(' ',HDR.NS,1)])',1,prod(size(HDR.Label)+[0,1]));
        CHAN  = ceil([strfind(tmp,'ecg'),strfind(tmp,'ekg')]/(size(HDR.Label,2)+1)); 
        if length(CHAN)~=1,
                HDR = sclose(HDR);
                fprintf(1,'The selected file contains the following channels: \n');
                for k = 1:HDR.NS,
                        fprintf(1,'%3i: %s\n',k,HDR.Label(k,:));
                end;
                CHAN = input('Which channel should be used for QRS-detection? ');
        end;
        HDR = sopen(fullfile(P,F),'r',CHAN);
end;
[s,HDR] = sread(HDR);
HDR = sclose(HDR);


% QRS-Detection
H2 = qrsdetect(s,HDR.SampleRate);


% Extract QRS-info according to BIOSIG/T200/EVENTCODES.TXT
idx = find(H2.EVENT.TYP == hex2dec('0501'));
qrsindex = H2.EVENT.POS(idx)/H2.EVENT.SampleRate; 


% displays detection
subplot(211)
plot((1:size(s,1))/HDR.SampleRate,s,'-',qrsindex,-ones(size(qrsindex)),'x');
xlabel('time t[s]');
ylabel(sprintf('%s [%s]',HDR.Label(CHAN,:),HDR.PhysDim(CHAN,:)));

subplot(212)
semilogy((qrsindex(1:end-1)+qrsindex(2:end))/2,diff(qrsindex));
ylabel('RRI [s]');
xlabel('time t[s]');
