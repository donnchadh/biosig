function [HDR] = eload(filename,Fs)
% ELOAD loads EVENT data 
% Event information is often stored in different formats. 
% ELOAD tries to load different formats into a unified 
% form 
% 
% HDR = eload(filename)
%
% filename	Filename of Event information 
% HDR.EVENT contains the EVENT information
% 
% 
% see also: SLOAD, SVIEW, SOPEN 
%


%	$Revision: 1.1 $
%	$Id: eload.m,v 1.1 2004-12-02 16:38:23 schloegl Exp $
%	Copyright (C) 1997-2004 by Alois Schloegl 
%	a.schloegl@ieee.org	
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


HDR = getfiletype(filename);

if strcmp(HDR.TYPE,'MAT')
        tmp = load('-mat',filename);
        if isfield(tmp,'eventmatrix') & isfield(tmp,'samplerate') 
                %%% F. Einspieler's Event information 
                HDR.EVENT.POS = tmp.eventmatrix(:,1);
                HDR.EVENT.TYP = tmp.eventmatrix(:,2);
                HDR.EVENT.CHN = tmp.eventmatrix(:,3);
                HDR.EVENT.DUR = tmp.eventmatrix(:,4);
                HDR.EVENT.Fs  = tmp.samplerate;
                HDR.TYPE = 'EVENT';
                
        elseif isfield(tmp,'EVENT') 
                HDR.EVENT = EVENT; 
                HDR.TYPE = 'EVENT';
        end;
        
elseif strcmp(HDR.TYPE,'GDF');
        H = sopen(HDR,'r'); H=sclose(H);
        HDR.EVENT = H.EVENT; 
        HDR.EVENT.Fs = H.SampleRate; 
        HDR.TYPE = 'EVENT';
        
        %%% Artifact database of the sleep EEG 
elseif strcmp(HDR.FILE.Ext,'txt') & strmatch(HDR.FILE.Name,['h000201';'h000901';'h001001']);
        HDR.EVENT = adb2event(filename,100);        
        HDR.TYPE = 'EVENT';
elseif strcmp(HDR.FILE.Ext,'txt') & strmatch(HDR.FILE.Name,['b000101';'b000401';'c000701';'c001701';'m000401';'m000901']);
        HDR.EVENT = adb2event(filename,200);        
        HDR.TYPE = 'EVENT';
elseif strcmp(HDR.FILE.Ext,'txt') & strmatch(HDR.FILE.Name,['n000101';'n000401';'p000101';'p000201';'s000201']);
        HDR.EVENT = adb2event(filename,256);        
        HDR.TYPE = 'EVENT';
elseif strcmp(HDR.FILE.Ext,'txt') & strmatch(HDR.FILE.Name,['u000601']);
        HDR.EVENT = adb2event(filename,400);        
        HDR.TYPE = 'EVENT';

elseif strcmp(HDR.TYPE,'WSCORE_EVENT')
        %HDR.EVENT.POS = HDR.EVENT.POS;         % already defined
        HDR.EVENT.TYP = HDR.EVENT.WSCORETYP;         % code assignment not
        fprintf(2,'Warning ELOAD: Event Codes in file %s do not not follow the standard codes of BIOSIG.\n',filename);
        %defined 
        
else
        fprintf(2,'Warning ELOAD: file %s is not recognized as event file.\n',filename);
        
end;