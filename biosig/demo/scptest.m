% Test procedure for SCP-ECG data
%
% The biosig-files are loaded, 
%   the signal data is stored in an ASCII text file. 
%   and the signal data is displayed.


% Copyright (C) 2004 by Alois Schloegl <a.schloegl@ieee.org>
% WWW: http://biosig.sf.net/
% $Revision: 1.2 $ 
% $Id: scptest.m,v 1.2 2004-02-09 22:09:14 schloegl Exp $
%
% LICENSE:
%     This program is free software; you can redistribute it and/or modify
%     it under the terms of the GNU General Public License as published by
%     the Free Software Foundation; either version 2 of the License, or
%     (at your option) any later version.
% 
%     This program is distributed in the hope that it will be useful,
%     but WITHOUT ANY WARRANTY; without even the implied warranty of
%     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%     GNU General Public License for more details.
% 
%     You should have received a copy of the GNU General Public License
%     along with this program; if not, write to the Free Software
%     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 


% get all files in current directory
try
        fn = dir('*.*c*');
        %fn = {fn.name}';
        ssf = 'ASDF';
catch,
        %else
        !unix('ls *.*[cC]* >tmp.tmp','-echo');
        fid = fopen('tmp.tmp','r');
        s = fread(fid,inf,'char');
        fclose(fid);
        ssf = char(s');
        fprintf(2,'Warning: plot might not work correctly in Octave4Windows 2.1.50\n');
end;

fprintf(1,'No\tFilename\tFs [Hz]\tsize(s) \ttime [s]\n=========================================================\n');
k = 0;
status = [];
while ~isempty(ssf),
	k = k + 1;
        if ~strcmp(ssf,'ASDF')
        	[f,ssf] = strtok(ssf,[10,13,32]);	
        else
        	f = fn(k).name;
	end;

        status(k) = -1;	
        H = [];
        try
                fprintf(1,'\n%i\t%s\t',k,f);
                tic;
                [H]=sopen(f);
                status(k) = -2;
                %[H]=save2txt(f);	% ASCII dump of Text file
                S = H.SCP.data;
                status(k) = -3;
                fprintf(1,'%i\t%i x %i\t%3.1f',H.SampleRate,size(S),toc);
                [p,f,e]=fileparts(f);		% a hack, to avoid loading data twice.
                save('-ascii',[f,'.txt'] ,'S'); 
                status(k) = -4;
                
                RRmx = sparse([1:8],[1,2,7:12],ones(1,8),8,12);
                RRmx = RRmx + sparse([1,2,1,2,1,2,1,2],[3,3,4,4,5,5,6,6],[-1,1,-1/2,-1/2,1,-1/2,-1,1/2],8,12);
                
                status(k) = -5;
                cal = mod([1:H.N+2*H.SampleRate]',H.SampleRate) < H.SampleRate/2;
                s = [[S*RRmx; repmat(NaN,2*H.SampleRate,12)],cal];
                status(k) = -6;
                
                t = s(:); t(~isnan(t));
                dd = max(t)-min(t);
                
                plot((1:size(s,1))'/H.SampleRate,((s+(ones(size(s,1),1)*(1:size(s,2)))*dd/(-2)+4*dd)));
                %title(sprintf('%s   -  generated with BIOSIG tools for Octave and Matlab(R)',H.FileName));
                %xlabel('time t[s]');
                %ylabel(sprintf('Amplitude [%s]',H.PhysDim));
                
                %Label = {H.Label{1:2},'III','aVR','aVL','VF',H.Label{3:8},'1mV/1Hz'}';
                %legend(Label);
                
                
                
                status(k) = 0;
                %drawnow;
                %pause;
        catch;
                if status(k)==-2,
                        if ~isstruct(H)
                                status(k) = 1001;
                                %fprintf(1,' cannot handle file');
                        elseif ~isfield(H,'TYPE')
                                status(k) = 1002;
                                %fprintf(1,' unknown TYPE');
                        elseif ~strcmp(H.TYPE,'SCPECG')
                                status(k) = 1003;
                                % fprintf(1,' no SCP file. Type: %s ',H.TYPE);
                        end;
                end; 
                fprintf(1,'[%i]', 	status(k));
        end;
end;
