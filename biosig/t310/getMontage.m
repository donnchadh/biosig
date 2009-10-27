function [lap, plot_index, n_rows, n_cols] = getMontage(montage, outfile)
% Calculates spatial filter matrix for Laplacian derivations.
% 
% Returns a spatial filter matrix used to calculate Laplacian derivations as
% well as indices used to plot in a topographical layout.
% Assuming that the data vector s is of dimension <samples x channels>, the
% Laplacian derivation s_lap can then be calculated by s_lap = s * lap.
%
% Usage:
%   [lap, plot_index, n_rows, n_cols] = getMontage(montage);
%   [...] = getMontage(montage, rrfile);
%
% Input parameters:
%   montage ... Matrix containing the topographical layout of the channels. The
%               content of this matrix can be one of the following formats: 
%               (1) Channel numbers where channels are located and zeros
%                   elsewhere <NxM>;
%               (2) Ones where channels are located and zeros elsewhere <NxM>;
%               (3) Predefined layout <string>.
%               Examples for each format:
%               (1) montage = [0 3 0; ...
%                              4 1 2; ...
%                              0 5 0];
%               (2) montage = [0 1 0; ...
%                              1 1 1; ...
%                              0 1 0];
%               (3) montage = '16ch';
%
% Optional input parameters:
%   rrfile ... Name of the re-referencing file defining the spatial filter
%              <string>. If the extension is empty, '.mtx' (MatrixMarket format)
%              is added to the filename. This file can then be used with 
%              (1) save2gdf -r=rrfile ... 
%              (2) mexSLOAD(file, rrfile, ...
%              (3) SigViewer
%
% Output parameters:
%   lap        ... Laplacian filter matrix
%   plot_index ... Indices for plotting the montage
%   n_rows     ... Number of rows of the montage
%   n_cols     ... Number of columns of the montage

% Copyright by Clemens Brunner, Robert Leeb, Alois Schlögl 
% $Revision: 0.3 $ $Date: 10/27/2009 11:41:07 $
% $Id$
% E-Mail: clemens.brunner@tugraz.at

% This program is free software; you can redistribute it and/or modify it
% under the terms of the GNU General Public License as published by the
% Free Software Foundation; either version 2 of the License, or (at your
% option) any later version.
%
% This program is distributed in the hope that it will be useful, but
% WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
% Public License for more details.
%
% You should have received a copy of the GNU General Public License along
% with this program; if not, write to the Free Software Foundation, Inc.,
% 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

if ischar(montage)  % Predefined layouts
    switch montage
        case '16ch'
            temp = [0 0 1 0 0;...
                    0 1 1 1 0;...
                    0 1 1 1 0;...
                    1 1 1 1 1;...
                    0 1 1 1 0;...
                    0 0 1 0 0];
            plot_index = find(temp' == 1);
            n_rows = size(temp, 1);
            n_cols = size(temp, 2);
        case '22ch'
            temp = [0 0 0 1 0 0 0;...
                    0 1 1 1 1 1 0;...
                    1 1 1 1 1 1 1;...
                    0 1 1 1 1 1 0;...
                    0 0 1 1 1 0 0;...
                    0 0 0 1 0 0 0];
            plot_index = find(temp' == 1);
            n_rows = size(temp, 1);
            n_cols = size(temp, 2);
        case '24ch'
            temp = [0 1 0 0 1 0 0 1 0;...
                    1 1 1 1 1 1 1 1 1;...
                    1 1 1 1 1 1 1 1 1;...
                    0 1 0 0 1 0 0 1 0];
            plot_index = find(temp' == 1);
            n_rows = size(temp, 1);
            n_cols = size(temp, 2);
        case '28ch'
            temp = [0 0 0 1 0 0 0;...
                    0 1 1 1 1 1 0;...
                    1 1 1 1 1 1 1;...
                    0 1 1 1 1 1 0;...
                    0 0 1 1 1 0 0;...
                    0 0 0 1 0 0 0;...
                    1 1 1 0 1 1 1];
            plot_index = find(temp' == 1);
            n_rows = size(temp, 1);
            n_cols = size(temp, 2);
        case '30ch'
            temp = [0 0 0 1 1 1 0 0 0;...
                    1 1 1 1 1 1 1 1 1;...
                    1 1 1 1 1 1 1 1 1;...
                    1 1 1 1 1 1 1 1 1];
            plot_index = find(temp' == 1);
            n_rows = size(temp, 1);
            n_cols = size(temp, 2);
        case '58ch'
            temp = [0 0 1 1 1 1 1 0 0;...
                    1 1 1 1 1 1 1 1 1;...
                    1 1 1 1 1 1 1 1 1;...
                    1 1 1 1 1 1 1 1 1;...
                    1 1 1 1 1 1 1 1 1;...
                    1 1 1 1 1 1 1 1 1;...
                    0 0 1 1 1 1 1 0 0;...
                    0 0 0 1 1 1 0 0 0];...
                plot_index = find(temp' == 1);
            n_rows = size(temp, 1);
            n_cols = size(temp, 2);
        case '60ch'
            temp = [0 0 0 0 0 1 0 0 0 0 0;...
                    0 0 0 0 1 1 1 0 0 0 0;...
                    0 0 0 1 1 1 1 1 0 0 0;...
                    0 0 1 1 1 1 1 1 1 0 0;...
                    0 1 1 1 1 1 1 1 1 1 0;...
                    1 1 1 1 1 1 1 1 1 1 1;...
                    0 1 1 1 1 1 1 1 1 1 0;...
                    0 0 1 1 1 1 1 1 1 0 0;...
                    0 0 0 1 1 1 1 1 0 0 0;...
                    0 0 0 0 1 1 1 0 0 0 0];
            plot_index = find(temp' == 1);
            n_rows = size(temp, 1);
            n_cols = size(temp, 2);
    end;
else  % User-defined layouts in the form of a matrix
    temp = montage;
    plot_index = find(temp' == 1);
    n_rows = size(temp, 1);
    n_cols = size(temp, 2);
end;

counter = 1;
temp = temp';
lap = zeros(size(temp,1), size(temp,2));

% Use electrode positions instead of ones
positions = [];
if sum(sum(temp)) ~= (sum(sum(temp > 0)))
    [tmp, positions] = sort(temp(find(temp)));
    temp = temp > 0;
end;

for k = 1:numel(temp)
    if temp(k) == 1
        lap(k) = counter;
        counter = counter + 1;
    end;
end;

neighbors = ones(counter - 1, 4) * nan;
electrode = 0;
for (k = 1:numel(lap))
    if lap(k) ~= 0
        col = 1;
        electrode = electrode + 1;
        if (k - size(lap, 1) > 0 && lap(k - size(lap, 1)) ~= 0)  % Top
            neighbors(electrode, col) = lap(k - size(lap, 1));
            col = col + 1;
        end;
        if (mod(k+1, size(lap, 1)) ~= 1 && k < numel(lap) && lap(k+1) ~= 0)  % Left
            neighbors(electrode, col) = lap(k+1);
            col = col + 1;
        end;
        if (mod(k-1, size(lap, 1)) ~= 0 && k > 1 && lap(k-1) ~= 0)  % Right
            neighbors(electrode, col) = lap(k-1);
            col = col + 1;
        end;
        if (k + size(lap, 1) < numel(lap) && lap(k + size(lap, 1)) ~= 0)  % Bottom
            neighbors(electrode, col) = lap(k + size(lap, 1));
            col = col + 1;
        end;
    end;
end;

lap = eye(length(neighbors));

for k = 1:length(neighbors)
    temp = neighbors(k, ~isnan(neighbors(k,:)));  % Neighbors of electrode k
    lap(k,temp) = -1/length(temp);
end;

if ~isempty(positions)
    lap = lap(positions,positions);
end

lap = lap';

% Generate re-referencing matrix (MatrixMarket format)
if nargin > 1
    [f, p, e] = fileparts(outfile);
    if isempty(e)
        e = '.mtx';
    end;
    [I,J,V] = find(lap);
    fid = fopen(fullfile(f, [p, e]), 'w+');
    fprintf(fid, '%%%%MatrixMarket matrix coordinate real general\n');
    fprintf(fid, '%% generated on %04i-%02i-%02i %02i:%02i:%02.0f\n', clock);
    
    if ischar(montage)
        m = montage;
    else
        m = '? (user specified)';
    end;
    fprintf(fid, '%% Spatial Laplacian Filter for Montage %s \n', m);
    fprintf(fid, '%i %i %i\n', size(lap), length(V));
    
    for k = 1:length(V),
        fprintf(fid, '%2i %2i %f\n', I(k), J(k), V(k));
    end;
    fclose(fid);
end;