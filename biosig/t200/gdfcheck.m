function [EDF]=gdfcheck(EDF,Mode,Version)
% Validates GDF headers and adds missing information
% [EDF]=gdfcheck(EDF,Mode)
% Mode = 1 [default] checks if header info is correct
%        2 selects channels according to EDF.Chan_Select and gnerates new Header
%
% INPUT
%   EDF.VERSION
%   EDF.NS
%   EDF.DUR
%
%   EDF.PhysMin
%   EDF.PhysMax
%   EDF.DigMin
%   EDF.DigMax
%
%
% OUTPUT
%   EDF.Cal
%   EDF.Off
%   EDF.Calib
%
%   EDF.AS.bi
%   EDF.AS.spb
%   EDF.AS.bpb
%   EDF.AS.GDFbi
%
%   EDF.Label
%   EDF.Transducer
%   EDF.PhysDim
%   EDF.PreFilt
%

INFO='(C) 1997-2003 by Alois Schloegl, 17.7.2003';
%	$Revision: 1.1 $
%	$Id: gdfcheck.m,v 1.1 2003-07-17 12:07:05 schloegl Exp $
%	Copyright (C) 2000-2003 by Alois Schloegl <a.schloegl@ieee.org>	


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

%%%%% Define Valid Data types %%%%%%
%GDFTYPES=[0 1 2 3 4 5 6 7 16 17 255+(1:64) 511+(1:64)];
GDFTYPES=[0 1 2 3 4 5 6 7 16 17 255+[1 12 22 24] 511+[1 12 22 24]];

%%%%% Define Size for each data type %%%%%
GDFTYP_BYTE=zeros(1,512+64);
GDFTYP_BYTE(256+(1:64))=(1:64)/8;
GDFTYP_BYTE(512+(1:64))=(1:64)/8;
GDFTYP_BYTE(1:18)=[1 1 1 2 2 4 4 8 8 4 8 0 0 0 0 0 4 8]';



if nargin<2 Mode=1; end;

GDF=strcmp(EDF.VERSION(1:3),'GDF');

if ~GDF
        % Y2K compatibility until year 2090
        if EDF.T0(1) < 91
                EDF.T0(1)=2000+EDF.T0(1);
        elseif EDF.T0(1) < 100
                EDF.T0(1)=1900+EDF.T0(1);
        end;
end;

EDF.ERROR=[];
EDF.ERRNO=0;
if ~all([size(EDF.SPR,1) size(EDF.GDFTYP,2) size(EDF.Label,1) size(EDF.Transducer,1) size(EDF.PhysMax,1) size(EDF.PhysMin,1) size(EDF.DigMax,1) size(EDF.DigMin,1) size(EDF.PreFilt,1) ] == EDF.NS)
        fprintf(2,'Warning GDFCHECK: Dimension mismatch in Header information\n');
        EDF.ERROR=[EDF.ERROR 'GDFCHECK: Dimension mismatch in Header information\n'];
        EDF.ERRNO=-1;
        return;
end;

%%%%% Check Validity %%%%%%
if ~all(any(EDF.GDFTYP(ones(size(GDFTYPES)),:)==(ones(EDF.NS,1)*GDFTYPES)',1))
        fprintf(2,'Warning GDFCHECK: invalid EDF.GDFTYP in file %s\n',EDF.FileName);
        disp(EDF.GDFTYP)
        EDF.ERROR=[EDF.ERROR 'GDFCHECK: invalid EDF.GDFTYP\n'];
        EDF.ERRNO=-2;
        return;
end;

if exist('OCTAVE_VERSION')
        tmp = find(EDF.GDFTYP==0); 
else
        tmp = (EDF.GDFTYP==0);   % | (EDF.GDFTYP==16) | (EDF.GDFTYP==17);
end;
EDF.PhysMax(tmp)= 1;
EDF.PhysMin(tmp)= 0;
EDF.DigMax(tmp) = 1;
EDF.DigMin(tmp) = 0;

EDF.Cal = (EDF.PhysMax-EDF.PhysMin)./(EDF.DigMax-EDF.DigMin);
EDF.Off = EDF.PhysMin - EDF.Cal .* EDF.DigMin;
EDF.Calib=[EDF.Off';(diag(EDF.Cal))];

EDF.SampleRate = EDF.SPR / EDF.Dur;

if any(EDF.Cal<0)
        tmp = find(EDF.Cal < 0);
        EDF.Cal(tmp) = ones(size(tmp));
        EDF.Off(tmp) = zeros(size(tmp));
        fprintf(2,'Warning GDFCHECK: negativ calibration value\n');
        EDF.ERROR=[EDF.ERROR 'GDFCHECK: negativ calibration value\n'];
        EDF.ERRNO=-3;
        %return;
end;


bi=[0;cumsum(EDF.SPR)]; 
EDF.AS.spb = sum(EDF.SPR);	% Samples per Block
EDF.AS.bi=bi;
EDF.AS.BPR  = ceil(EDF.SPR.*GDFTYP_BYTE(EDF.GDFTYP+1)'); 
%EDF.AS.GDFbi= [0;cumsum(EDF.AS.BPR)];
EDF.AS.GDFbi= [0;cumsum(ceil(EDF.SPR.*GDFTYP_BYTE(EDF.GDFTYP+1)'))]; 
EDF.AS.bpb = sum(ceil(EDF.SPR.*GDFTYP_BYTE(EDF.GDFTYP+1)'));	% Bytes per Block

if 0 
	if Mode==1 
        	EDF.Chan_Select=(EDF.SPR==max(EDF.SPR));
	end;

    for k=1:EDF.NS;
	if EDF.Chan_Select(k);
	    EDF.ChanTyp(k)='N';
	else
	    EDF.ChanTyp(k)=' ';
	end;         
	if findstr(upper(EDF.Label(k,:)),'ECG')
	    EDF.ChanTyp(k)='C';
	elseif findstr(upper(EDF.Label(k,:)),'EKG')
	    EDF.ChanTyp(k)='C';
	elseif findstr(upper(EDF.Label(k,:)),'EEG')
	    EDF.ChanTyp(k)='E';
	elseif findstr(upper(EDF.Label(k,:)),'EOG')
	    EDF.ChanTyp(k)='O';
	elseif findstr(upper(EDF.Label(k,:)),'EMG')
	    EDF.ChanTyp(k)='M';
	elseif findstr(upper(EDF.Label(k,:)),'RESP')
	    EDF.ChanTyp(k)='R';
	end;
    end;
end;

EDF.PID=eval('EDF.PID',';');
EDF.RID=eval('EDF.RID',';');
EDF.T0=eval('EDF.T0','clock');
EDF.Label=eval('EDF.Label','setstr(32*zeros(EDF.NS,16))');
EDF.Transducer=eval('EDF.Transducer','setstr(32*zeros(EDF.NS,80))');
EDF.PhysDim=eval('EDF.PhysDim','setstr(32*zeros(EDF.NS,8))');
EDF.PreFilt=eval('EDF.PreFilt','setstr(32*zeros(EDF.NS,80))');

tmp=size(EDF.Label,2);
if tmp<16
        EDF.Label=[EDF.Label setstr(32*zeros(EDF.NS,16-tmp))];
elseif tmp>16
        EDF.Label=EDF.Label(:,1:16);        
end;

tmp=size(EDF.Transducer,2);
if tmp<80
        EDF.Transducer=[EDF.Transducer setstr(32*zeros(EDF.NS,80-tmp))];
elseif tmp>80
        EDF.Transducer=EDF.Transducer(:,1:80);        
end;

tmp=size(EDF.PhysDim,2);
if tmp<8
        EDF.PhysDim=[EDF.PhysDim setstr(32*zeros(EDF.NS,8-tmp))];
elseif tmp>8
        EDF.PhysDim=EDF.PhysDim(:,1:8);        
end;

tmp=size(EDF.PreFilt,2);
if tmp<80
        EDF.PreFilt=[EDF.PreFilt setstr(32*zeros(EDF.NS,80-tmp))];
elseif tmp>80
        EDF.PreFilt=EDF.PreFilt(:,1:80);        
end;


if Mode ~=2 return; end;

%%%%% Selects Channels according to EDF.Chan_Select	
ChSel=find(EDF.Chan_Select);
EDF.NS=length(ChSel);
EDF.SPR=EDF.SPR(ChSel);
EDF.GDFTYP=EDF.GDFTYP(ChSel);
EDF.Label  =EDF.Label(ChSel,:);
EDF.Transducer=EDF.Transducer(ChSel,:);
EDF.PhysDim=EDF.PhysDim(ChSel,:);
EDF.PhysMin=EDF.PhysMin(ChSel);
EDF.PhysMax=EDF.PhysMax(ChSel);
EDF.DigMin =EDF.DigMin(ChSel);
EDF.DigMax =EDF.DigMax(ChSel);
EDF.PreFilt=EDF.PreFilt(ChSel,:);

if isfield(EDF,'ChanTyp')
	EDF.ChanTyp=EDF.ChanTyp(ChSel);
end;

bi=[0;cumsum(EDF.SPR)];
EDF.AS.spb = sum(EDF.SPR);	% Samples per Block
EDF.AS.bi=bi;
EDF.AS.GDFbi=[0;cumsum(ceil(EDF.SPR.*GDFTYP_BYTE(EDF.GDFTYP+1)'))]; 
EDF.AS.bpb = sum(ceil(EDF.SPR.*GDFTYP_BYTE(EDF.GDFTYP+1)'));	% Bytes per Block

if 0
        bi=[0;cumsum(EDF.SPR)]; 
        EDF.AS.spb = sum(EDF.SPR);	% Samples per Block
        EDF.AS.bi=bi;

        idx=[];idx2=[];
        for k=1:EDF.NS, 
                idx2=[idx2, (k-1)*max(EDF.SPR)+(1:EDF.SPR(k))];
        end;
        maxspr=max(EDF.SPR);
        idx3=zeros(EDF.NS*maxspr,1);
        for k=1:EDF.NS, idx3(maxspr*(k-1)+(1:maxspr))=bi(k)+ceil((1:maxspr)'/maxspr*EDF.SPR(k));end;

        EDF.AS.IDX2=idx2;
        EDF.AS.IDX3=idx3;
end;


