function [A1,A2,A3,A4,A5] = baccala2001();
% BACCALA2001 returns the MVAR-Parameters for 
%    simulating MVAR processes according to [1].  
%
%  [A1,A2,A3,A4,A5] = baccala2001(); 
%       A1 ... A5 are 5 different sets of MVAR parameters
%  baccala2001; 
%       displays corresponding PDC and DTF for A1 ... A5        
%       
% Simulated MVAR prosses can be produced with 
%       y = mvfilter(X.B,X.A,randn(5,10000));  
%
% The PDF and the DTF can be displayed with the following functions
%       X.A = [eye(5),-A5]; X.B = eye(size(X.A,1)); 
%       X.datatype = 'MVAR';
%       plota(X,'PDC')         
%       plota(X,'DTF')         
%
% see also: PLOTA, MVAR, MVFILTER 
%
% REFERENCES:
% [1] Baccala LA, Sameshima K. (2001)
%       Partial directed coherence: a new concept in neural structure determination.
%       Biol Cybern. 2001 Jun;84(6):463-74. 

%	$Id: baccala2001.m,v 1.1 2004-11-08 14:06:33 schloegl Exp $
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

% Baccala et al. 2001, example 2
A1 = [.5, .3, .4; -.5, .3, 1; 0, -.3, -.2];

% Baccala et al. 2001, Fig 2
A2 =    [.95*sqrt(2), 0, 0, 0, 0,       -.9025, 0, 0, 0, 0,     0, 0, 0, 0, 0; ...
        0,0,0,0,0,                      .5, 0, 0, 0, 0,         0, 0, 0, 0, 0; ...
        0,0,0,0,0,                      0,0,0,0,0,              -.4, 0, 0, 0, 0; ... 
        0,0,0,sqrt(2)/4,sqrt(2)/4,      -.5,0,0,0,0,            0, 0, 0, 0, 0;  ... 
        0,0,0,-sqrt(2)/4,sqrt(2)/4,     0,0,0,0,0,              0, 0, 0, 0, 0]; 

% Baccala et al. 2001, Fig 3
A3 = [ .95*sqrt(2), 0, 0, 0, 0,         -.9025, 0, 0, 0, 0; ...
       .5,0,0,0,0,                      0, 0, 0, 0, 0; ...
       0,0,0,0,0,                       0,.4,0,0,0; ... 
       0,0,-.5,sqrt(2)/4,sqrt(2)/4,     0,0,0,0,0; ... 
       0,0,0,-sqrt(2)/4,sqrt(2)/4,      0,0,0,0,0; ... 
];

% Baccala et al. 2001, Fig 4
A4 = [ .95*sqrt(2), 0, 0, 0, 0,         -.9025, 0, 0, 0, .5; ...
       .5,0,0,0,0,                      0, 0, 0, 0, 0; ...
       0,0,0,0,0,                       0,.4,0,0,0; ... 
       0,0,-.5,sqrt(2)/4,sqrt(2)/4,     0,0,0,0,0; ... 
       0,0,0,-sqrt(2)/4,sqrt(2)/4,      0,0,0,0,0; ... 
];

% Baccala et al. 2001, Fig 5
A5 = [ .95*sqrt(2), 0, 0, 0, 0,         -.9025, 0, 0, 0, 0; ...
       .5,0,0,0,0,                      0, 0, 0, 0, 0; ...
       .1,0,0,0,0,                      0,.4,0,0,0; ... 
       0,0,-.5,sqrt(2)/4,sqrt(2)/4,     0,0,0,0,0; ... 
       0,0,0,-sqrt(2)/4,sqrt(2)/4,      0,0,0,0,0;    ... 
];

if nargout>0, return; end; 

for k = 1:5, 
        AR = eval(['A',int2str(k)]);
        X.A = [eye(size(AR,1)),-AR]; 
        X.B = eye(size(X.A,1));
        X.datatype = 'MVAR';

        % Display PDC
        figure(2*k-1);
        plota(X,'PDC',512,1);         
        tmp = sprintf('Figure %ia in Baccala & Sameshima (2001)',k);
        if exist('suptitle','file')
                suptitle(tmp);        
        end;
        % Display DTF
        figure(2*k);
        plota(X,'DTF',512,1);         
        tmp = sprintf('Figure %ib in Baccala & Sameshima (2001)',k);
        if exist('suptitle','file')
                suptitle(tmp);        
        end;
end;
