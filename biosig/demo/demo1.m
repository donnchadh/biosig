% DEMO 1 - identifies QRS-complexes

%	$Revision: 1.2 $
%	$Id: demo1.m,v 1.2 2003-05-22 16:29:31 schloegl Exp $
%	Copyright (C) 2000-2003 by Alois Schloegl <a.schloegl@ieee.org>	

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
if exist('F')~=1,
        F=''; P='';
end
if exist(fullfile(P,F))~=2,
        [F,P]=uigetfile('*.*','Pick an ECG file');
end


CHAN = input('which channel?');
if CHAN<1, CHAN=0; end;
[s,h] = loadeeg(fullfile(P,F),CHAN);
Fs = h.SampleRate(min(length(h.SampleRate),CHAN));


% processing of ECG-envelope
MODE  = {'ECG_envelope',Fs};
[Y,Z] = processing(MODE,s(:,1));


% THRESHOLD = quantile(Y,.9) is often a good choice.  
tmp = sort(Y); TH = tmp(round(length(Y)*.9));

% identify fiducial points
qrsindex = gettrigger(Y,TH); 


% displays detection
subplot(211)
plot((1:size(s,1))/Fs,[s,Y],'-',qrsindex/Fs,-ones(size(qrsindex)),'x');

subplot(212)
semilogy((qrsindex(1:end-1)+qrsindex(2:end))/Fs/2,diff(qrsindex)/Fs);



