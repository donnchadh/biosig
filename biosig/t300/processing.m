function [Y,Z] = processing(MODE,X,Z)
% Data processing in the BIOSIG toolbox, 
% This functions can be used as template as well as a wrapper 
% around different signal processing methods.  
%
% [Y,Zf] = processing(MODE,X,Zi)
%
%    X input signal 
%    Zi input status (optional), 
%	initial condition
%	if empty or not available, this initializes the method
%    Y  output signal	
%    Zf final condition 
%    MODE is a struct and determines the signal processing method
%
%    MODE =
%	{'ECG_envelope',Fs}       [1]
%    	'xyz'		-none-	
%
%
% Reference(s):
% [1] M.-E. Nygards, L. Sörnmo, Delineation of the QRS complex using the envelope of the e.c.g
%         Med. & Biol. Eng. & Comput., 1983, 21, 538-547.
%
%


%	$Revision: 1.1 $
%	$Id: processing.m,v 1.1 2003-05-16 13:30:15 schloegl Exp $
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


if nargin<3, Z=[]; end;

if 0,~isstruct(MODE)
        D.method = MODE;
else
        D = MODE;
end;


if 0
        
elseif strcmp(D{1},'ECG_envelope')
        if isempty(Z),
                if length(D)<2,
                        fprintf(2,'ERROR: SampleRate not defined.\n');        
                        return;
                end;
                
                Fs = D{2};%.SampleRate;
                
                
                tmp= Fs/200; 
                
                T = .005; % s
                K = floor(T*Fs+1);
                PAR.HI = zeros(1,4*(K+1)-1);
                PAR.HI(1:2:end) = 2/pi*1./[-2*K-1:2:2*K+1];
                PAR.HI(2*K+2) = -j;
                PAR.HI(:) = -j*PAR.HI(:);
                PAR.trgL = ceil(0.04*Fs);              % 40ms

                Z.B2 = [1:PAR.trgL+1 PAR.trgL:-1:1];
                Z.A2 = sum(abs(Z.B2));
                
                Z.B1 = conv(PAR.HI,[ones(1,T*Fs),-ones(1,T*Fs)]*1000/Fs);       
                Z.A1 = 1; %sum(abs(MAT.ExFilt.B{26}));
                
                Z.Z1 = [];
                Z.Z2 = [];
        end;
        
        [tmp, Z.Z1] = filter(Z.B1,Z.A1,X,Z.Z1);
        [Y,   Z.Z2] = filter(Z.B2,Z.A2,abs(tmp),Z.Z2);
        
else 
        
end;
