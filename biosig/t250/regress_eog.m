function [R,S] = regress_eog(D,ny,nx)
%  REGRESS_EOG yields the regression coefficients for 
%  correcting EOG artifacts in EEG recordings.
%  
%  The correction of a single record is obtained like this:      
%   [R,S2] = regress_eog(filename, el, ol)
%   [R,S2] = regress_eog(S1, el, ol)
%       
%  Correction matrix is obtained through
%   [R] = regress_eog(covm(S1,'E'), el, ol)
%   S2 = S1 * R.ro;    % without offset correction
%   S2 = [ones(size(S1,1),1),S1] * R.r1;    % with offset correction
%  
%  S1   recorded data
%  el   list of eeg channels: those channels will be corrected   
%  ol   list of eog channels 
%  R    rereferencing matrix for correction artifacts with regression analysis
%  S2   corrected EEG-signal      
%
% see also: SAVE2BKR
%
% Reference(s):
% [1] A. Schlögl and G. Pfurtscheller,
%    EOG and ECG minimization based on regression analysis, Technical Report - SIESTA, 1997.
%    available online: http://www.dpmi.tu-graz.ac.at/%7Eschloegl/publications/Eog_min.pdf


% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 2
% of the License, or (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

%	$Revision: 1.3 $
%	$Id: regress_eog.m,v 1.3 2004-07-05 10:55:51 schloegl Exp $
%	(C) 1997-2004 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/


R.datatype = 'ArtifactCorrection_Regression';
R.signalchannel = ny;
R.noise_channel = nx;
R.Mode = 1; % correction of the mean 
R.Mode = 0; % correction without mean 

nx = nx(:);
ny = ny(:);

if ischar(D),
        [D,H] = sload(D);
        C = covm(D,'E');
        
elseif size(D,1)==size(D,2)
        C = D; 
        H.NS = size(C,2)-1;
else
        H.NS = size(D,2);
        C = covm(D,'E');
end

r0 = eye(H.NS);
r1 = sparse(2:H.NS+1,1:H.NS,1);

b0 = -inv(C([1;nx+1],[1;nx+1]))*C([1;nx+1],ny+1);
r0(nx,ny) = b0(2:end,:);       % rearrange channel order
r1([1;1+nx],ny) = b0;       % rearrange channel order

R.r0 = r0;
R.r1 = r1;

if size(D,1)==size(D,2),
        % R = R;         
        
else    
        if ischar(D),
                %R.Calib = [[1;zeros(H.NS,1)],r1] * H.Calib;
                R.Calib = H.Calib * R.r0;
        end;

        % S = D * R.r0;   % without offset correction 
        S = [ones(size(D,1),1),D] * R.r1; % with offset correction
end

