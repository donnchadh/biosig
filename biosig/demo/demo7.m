% Demo7 - MULTIVARIATE AUTOREGRESSIVE Analysis: 
%
%      1) Simulates a MVAR process
%      2) Estimatess MVAR parameters
%      3) Displays the PDC for the original parameters and its estimates
%
% see also: BACCALA2001, MVAR, MVFILTER, PLOTA
%
% Reference(s):
%  [1] Baccala LA, Sameshima K. (2001)
%       Partial directed coherence: a new concept in neural structure determination.
%       Biol Cybern. 2001 Jun;84(6):463-74. 
%  [2] M. Kaminski, M. Ding, W. Truccolo, S.L. Bressler, Evaluating causal realations in neural systems:
%	Granger causality, directed transfer functions and statistical assessment of significance.
%	Biol. Cybern., 85,145-157 (2001)
%  [3] http://biosig.sf.net/

%       $Revision: 1.1 $
%	$Id: demo7.m,v 1.1 2004-11-08 14:06:40 schloegl Exp $
%	Copyright (C) 1999-2004 by Alois Schloegl <a.schloegl@ieee.org>
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 2
% of the  License, or (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

% MULTIVARIATE AUTOREGRESSIVE Analysis: 
[A1,A2,A3,A4,A5] = baccala2001;       

AR0 = A3;        % select parameter set

M = size(AR0,1);
% Simulated MVAR prosses can be produced with 
y = mvfilter(eye(M),[eye(M),-AR0],randn(M,1000));  

% Estimate AR parameters
[AR,RC,PE] = mvar(y',2);

% The PDF and the DTF can be displayed with the following functions
X.A = [eye(5),-AR0]; X.B = eye(size(X.A,1)); 
X.datatype = 'MVAR';
figure(1)
plota(X,'PDC')         
if exist('suptitle','file')
        suptitle('PDC of true MVAR parameters');
end;

% The PDF and the DTF can be displayed with the following functions
X.A = [eye(5),-AR]; X.B = eye(size(X.A,1)); 
X.datatype = 'MVAR';
figure(2)
plota(X,'PDC')         
if exist('suptitle','file')
        suptitle('PDC of MVAR estimates');
end;


