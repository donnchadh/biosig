function [CNT,h,e]=cntopen(arg1,PERMISSION,CHAN,arg4,arg5,arg6)
% CNTOPEN opens neuroscan files (but does not read the data). 
% However, it is recommended to use SOPEN instead of CNTOPEN.
% For loading whole Neuroscan data files, use SLOAD. 
%
% see also: SLOAD, SOPEN, SREAD, SCLOSE, SEOF, STELL, SSEEK.

% HDR=cntopen(Filename, PERMISSION, [, ChanList ]);
%
% FILENAME 
% PERMISSION is one of the following strings 
%	'rb'	read 
% ChanList	(List of) Channel(s)
%		default=0: loads all channels

%	$Revision: 1.18 $
%	$Id: cntopen.m,v 1.18 2003-12-11 17:00:18 schloegl Exp $
%	Copyright (C) 1997-2003 by  Alois Schloegl
%	a.schloegl@ieee.org	

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

if nargin<2, 
        PERMISSION='rb'; 
elseif ~any(PERMISSION=='b');
        PERMISSION = [PERMISSION,'b']; % force binary open. 
end;
if nargin<3, CHAN=0; end;

if isstruct(arg1),
	CNT=arg1;
	if CNT.FILE.OPEN,
		fseek(CNT.FILE.FID,0,'bof');	
	else
		CNT.FILE.FID = fopen(CNT.FileName,PERMISSION,'ieee-le');          
		CNT.FILE.OPEN= 1;
	end;
else
	CNT.FileName = arg1;
	CNT.FILE.FID = fopen(CNT.FileName,PERMISSION,'ieee-le');          
	if CNT.FILE.FID<0,
		fprintf(2,'Error CNTOPEN: file %s not found.\n',CNT.FileName); 
		return;
	end;
	CNT.FILE.OPEN = 1;
end;

fid = CNT.FILE.FID;

%%%%% READ HEADER
if 0,   % old header
        %h.rev               = fread(fid,12,'char');
        %h.nextfile          = fread(fid,1,'long');
        %h.prevfile          = fread(fid,1,'long');
        h.type              = fread(fid,1,'char');
        h.id                = fread(fid,20,'char');
        h.oper              = fread(fid,20,'char');
        h.doctor            = fread(fid,20,'char');
        h.referral          = fread(fid,20,'char');
        h.hospital          = fread(fid,20,'char');
        h.patient           = fread(fid,20,'char');
        h.age               = fread(fid,1,'short');
        h.sex               = fread(fid,1,'char');
        h.hand              = fread(fid,1,'char');
        h.med               = fread(fid,20,'char');
        h.category          = fread(fid,20,'char');
        h.state             = fread(fid,20,'char');
        h.label             = fread(fid,20,'char');
        h.date              = fread(fid,10,'char');
        h.time              = fread(fid,12,'char');
        h.avgmode           = fread(fid,1,'char');
        h.review            = fread(fid,1,'char');
        h.nsweeps           = fread(fid,1,'ushort');
        h.compsweeps        = fread(fid,1,'ushort');
        h.pnts              = fread(fid,1,'ushort');
        h.nchannels         = fread(fid,1,'short');
        h.avgupdate         = fread(fid,1,'short');
        h.domain            = fread(fid,1,'char');
        h.rate              = fread(fid,1,'ushort');
        h.scale             = fread(fid,1,'double');
        h.veogcorrect       = fread(fid,1,'char');
        h.veogtrig          = fread(fid,1,'float');
        h.veogchnl          = fread(fid,1,'short');
        h.heogcorrect       = fread(fid,1,'char');
        h.heogtrig          = fread(fid,1,'float');
        h.heogchnl          = fread(fid,1,'short');
        h.baseline          = fread(fid,1,'char');
        h.offstart          = fread(fid,1,'float');
        h.offstop           = fread(fid,1,'float');
        h.reject            = fread(fid,1,'char');
        h.rejchnl1          = fread(fid,1,'char');
        h.rejchnl2          = fread(fid,1,'char');
        h.rejchnl3          = fread(fid,1,'char');
        h.rejchnl4          = fread(fid,1,'char');
        h.rejstart          = fread(fid,1,'float');
        h.rejstop           = fread(fid,1,'float');
        h.rejmin            = fread(fid,1,'float');
        h.rejmax            = fread(fid,1,'float');
        h.trigtype          = fread(fid,1,'char');
        h.trigval           = fread(fid,1,'float');
        h.trigchnl          = fread(fid,1,'char');
        h.trigisi           = fread(fid,1,'float');
        h.trigmin           = fread(fid,1,'float');
        h.trigmax           = fread(fid,1,'float');
        h.trigdur           = fread(fid,1,'float');
        h.dir               = fread(fid,1,'char');
        h.dispmin           = fread(fid,1,'float');
        h.dispmax           = fread(fid,1,'float');
        h.xmin              = fread(fid,1,'float');
        h.xmax              = fread(fid,1,'float');
        h.ymin              = fread(fid,1,'float');
        h.ymax              = fread(fid,1,'float');
        h.zmin              = fread(fid,1,'float');
        h.zmax              = fread(fid,1,'float');
        h.lowcut            = fread(fid,1,'float');
        h.highcut           = fread(fid,1,'float');
        h.common            = fread(fid,1,'char');
        h.savemode          = fread(fid,1,'char');
        h.manmode           = fread(fid,1,'char');
        h.ref               = fread(fid,20,'char');
        h.screen            = fread(fid,80,'char');
        h.seqfile           = fread(fid,80,'char');
        h.montage           = fread(fid,80,'char');
        h.heegcorrect       = fread(fid,1,'char');
        h.variance          = fread(fid,1,'char');
        h.acceptcnt         = fread(fid,1,'ushort');
        h.rejectcnt         = fread(fid,1,'ushort');
        h.reserved74        = fread(fid,74,'char');
       
        for n = 1:64,%h.nchannels
                e(n).lab            = fread(fid,10,'char');
                e(n).x_coord        = fread(fid,1,'float');
                e(n).y_coord        = fread(fid,1,'float');
                e(n).alpha_wt       = fread(fid,1,'float');
                e(n).beta_wt        = fread(fid,1,'float');
        end

        
else    % new header
        h.rev               = fread(fid,12,'char');
        h.nextfile          = fread(fid,1,'uint32');
        h.prevfile          = fread(fid,1,'uint32');
        h.type              = fread(fid,1,'char');
        h.id                = fread(fid,20,'char');
        h.oper              = fread(fid,20,'char');
        h.doctor            = fread(fid,20,'char');
        h.referral          = fread(fid,20,'char');
        h.hospital          = fread(fid,20,'char');
        h.patient           = fread(fid,20,'char');
        h.age               = fread(fid,1,'short');
        h.sex               = fread(fid,1,'char');
        h.hand              = fread(fid,1,'char');
        h.med               = fread(fid,20, 'char');
        h.category          = fread(fid,20, 'char');
        h.state             = fread(fid,20, 'char');
        h.label             = fread(fid,20, 'char');
        h.date              = fread(fid,10, 'char');
        h.time              = fread(fid,12, 'char');
        h.mean_age          = fread(fid,1,'float');
        h.stdev             = fread(fid,1,'float');
        h.n                 = fread(fid,1,'short');
        h.compfile          = fread(fid,38,'char');
        h.spectwincomp      = fread(fid,1,'float');
        h.meanaccuracy      = fread(fid,1,'float');
        h.meanlatency       = fread(fid,1,'float');
        h.sortfile          = fread(fid,46,'char');
        h.numevents         = fread(fid,1,'int');
        h.compoper          = fread(fid,1,'char');
        h.avgmode           = fread(fid,1,'char');
        h.review            = fread(fid,1,'char');
        h.nsweeps           = fread(fid,1,'ushort');
        h.compsweeps        = fread(fid,1,'ushort');
        h.acceptcnt         = fread(fid,1,'ushort');
        h.rejectcnt         = fread(fid,1,'ushort');
        h.pnts              = fread(fid,1,'ushort');
        h.nchannels         = fread(fid,1,'ushort');
        h.avgupdate         = fread(fid,1,'ushort');
        h.domain            = fread(fid,1,'char');
        h.variance          = fread(fid,1,'char');
        h.rate              = fread(fid,1,'ushort');
        h.scale             = fread(fid,1,'double');
        h.veogcorrect       = fread(fid,1,'char');
        h.heogcorrect       = fread(fid,1,'char');
        h.aux1correct       = fread(fid,1,'char');
        h.aux2correct       = fread(fid,1,'char');
        h.veogtrig          = fread(fid,1,'float');
        h.heogtrig          = fread(fid,1,'float');
        h.aux1trig          = fread(fid,1,'float');
        h.aux2trig          = fread(fid,1,'float');
        h.heogchnl          = fread(fid,1,'short');
        h.veogchnl          = fread(fid,1,'short');
        h.aux1chnl          = fread(fid,1,'short');
        h.aux2chnl          = fread(fid,1,'short');
        h.veogdir           = fread(fid,1,'char');
        h.heogdir           = fread(fid,1,'char');
        h.aux1dir           = fread(fid,1,'char');
        h.aux2dir           = fread(fid,1,'char');
        h.veog_n            = fread(fid,1,'short');
        h.heog_n            = fread(fid,1,'short');
        h.aux1_n            = fread(fid,1,'short');
        h.aux2_n            = fread(fid,1,'short');
        h.veogmaxcnt        = fread(fid,1,'short');
        h.heogmaxcnt        = fread(fid,1,'short');
        h.aux1maxcnt        = fread(fid,1,'short');
        h.aux2maxcnt        = fread(fid,1,'short');
        h.veogmethod        = fread(fid,1,'char');
        h.heogmethod        = fread(fid,1,'char');
        h.aux1method        = fread(fid,1,'char');
        h.aux2method        = fread(fid,1,'char');
        h.ampsensitivity    = fread(fid,1,'float');

        h.lowpass           = fread(fid,1,'char');
        h.highpass          = fread(fid,1,'char');
        h.notch             = fread(fid,1,'char');
        h.autoclipadd       = fread(fid,1,'char');
        h.baseline          = fread(fid,1,'char');
        h.offstart          = fread(fid,1,'float');
        h.offstop           = fread(fid,1,'float');
        h.reject            = fread(fid,1,'char');
        h.rejstart          = fread(fid,1,'float');
        h.rejstop           = fread(fid,1,'float');
        h.rejmin            = fread(fid,1,'float');
        h.rejmax            = fread(fid,1,'float');
        h.trigtype          = fread(fid,1,'char');
        h.trigval           = fread(fid,1,'float');
        h.trigchnl          = fread(fid,1,'char');
        h.trigmask          = fread(fid,1,'short');
        h.trigisi           = fread(fid,1,'float');
        h.trigmin           = fread(fid,1,'float');
        h.trigmax           = fread(fid,1,'float');
        h.trigdir           = fread(fid,1,'char');
        h.autoscale         = fread(fid,1,'char');
        h.n2                = fread(fid,1,'short');
        h.dir               = fread(fid,1,'char');
        h.dispmin           = fread(fid,1,'float');
        h.dispmax           = fread(fid,1,'float');
        h.xmin              = fread(fid,1,'float');
        h.xmax              = fread(fid,1,'float');
        h.automin           = fread(fid,1,'float');
        h.automax           = fread(fid,1,'float');
        h.zmin              = fread(fid,1,'float');
        h.zmax              = fread(fid,1,'float');
        h.lowcut            = fread(fid,1,'float');
        h.highcut           = fread(fid,1,'float');
        h.common            = fread(fid,1,'char');
        h.savemode          = fread(fid,1,'char');
        h.manmode           = fread(fid,1,'char');
        h.ref               = fread(fid,10,'char');
        h.rectify           = fread(fid,1,'char');
        h.displayxmin       = fread(fid,1,'float');
        h.displayxmax       = fread(fid,1,'float');
        h.phase             = fread(fid,1,'char');
        h.screen            = fread(fid,16,'char');
        
        h.calmode           = fread(fid,1,'short');
        h.calmethod         = fread(fid,1,'short');
        h.calupdate         = fread(fid,1,'short');
        h.calbaseline       = fread(fid,1,'short');
        h.calsweeps         = fread(fid,1,'short');
        h.calattenuator     = fread(fid,1,'float');
        h.calpulsevolt      = fread(fid,1,'float');
        h.calpulsestart     = fread(fid,1,'float');
        h.calpulsestop      = fread(fid,1,'float');
        h.calfreq           = fread(fid,1,'float');
        h.taskfile          = fread(fid,34,'char');
        h.seqfile           = fread(fid,34,'char');
        h.spectmethod       = fread(fid,1,'char');
        h.spectscaling      = fread(fid,1,'char');
        h.spectwindow       = fread(fid,1,'char');
        h.spectwinlength    = fread(fid,1,'float');
        h.spectorder        = fread(fid,1,'char');
        h.notchfilter       = fread(fid,1,'char');
        h.headgain          = fread(fid,1,'short');
        h.additionalfiles   = fread(fid,1,'int');
        h.unused            = fread(fid,5,'char');
        h.fspstopmethod     = fread(fid,1,'short');
        h.fspstopmode       = fread(fid,1,'short');
        h.fspfvalue         = fread(fid,1,'float');
        h.fsppoint          = fread(fid,1,'short');
        h.fspblocksize      = fread(fid,1,'short');
        h.fspp1             = fread(fid,1,'ushort');
        h.fspp2             = fread(fid,1,'ushort');
        h.fspalpha          = fread(fid,1,'float');
        h.fspnoise          = fread(fid,1,'float');
        h.fspv1             = fread(fid,1,'short');
        h.montage           = fread(fid,40,'char');
        h.eventfile         = fread(fid,40,'char');
        h.fratio            = fread(fid,1,'float');
        h.minor_rev         = fread(fid,1,'char');
        h.eegupdate         = fread(fid,1,'short');
        h.compressed        = fread(fid,1,'char');
        h.xscale            = fread(fid,1,'float');
        h.yscale            = fread(fid,1,'float');
        h.xsize             = fread(fid,1,'float');
        h.ysize             = fread(fid,1,'float');
        h.acmode            = fread(fid,1,'char');
        h.commonchnl        = fread(fid,1,'uchar');
        h.xtics             = fread(fid,1,'char');
        h.xrange            = fread(fid,1,'char');
        h.ytics             = fread(fid,1,'char');
        h.yrange            = fread(fid,1,'char');
        h.xscalevalue       = fread(fid,1,'float');
        h.xscaleinterval    = fread(fid,1,'float');
        h.yscalevalue       = fread(fid,1,'float');
        h.yscaleinterval    = fread(fid,1,'float');
        h.scaletoolx1       = fread(fid,1,'float');
        h.scaletooly1       = fread(fid,1,'float');
        h.scaletoolx2       = fread(fid,1,'float');
        h.scaletooly2       = fread(fid,1,'float');
        h.port              = fread(fid,1,'short');
        h.numsamples        = fread(fid,1,'uint32');

        h.filterflag        = fread(fid,1,'char');
        h.lowcutoff         = fread(fid,1,'float');
        h.lowpoles          = fread(fid,1,'short');
        h.highcutoff        = fread(fid,1,'float');
        h.highpoles         = fread(fid,1,'short');
        h.filtertype        = fread(fid,1,'char');
        h.filterdomain      = fread(fid,1,'char');
        h.snrflag           = fread(fid,1,'char');
        h.coherenceflag     = fread(fid,1,'char');
        h.continuoustype    = fread(fid,1,'char');
        h.eventtablepos     = fread(fid,1,'int32');
        h.continuousseconds = fread(fid,1,'float');
        h.channeloffset     = fread(fid,1,'uint32');
        h.autocorrectflag   = fread(fid,1,'char');
        h.dcthreshold       = fread(fid,1,'uchar');
        
        if ftell(fid)~=900,
                warning(['supicous Neuroscan file ',FILENAME]);
        end;
        
        for n = 1:h.nchannels,%h.nchannels
                e.lab(1:10,n)         = fread(fid,10,'char');
                e.reference(1,n)      = fread(fid,1,'char');
                e.skip(1,n)           = fread(fid,1,'char');
                e.reject(1,n)         = fread(fid,1,'char');
                e.display(1,n)        = fread(fid,1,'char');
                e.bad(1,n)            = fread(fid,1,'char');
                e.n(1,n)              = fread(fid,1,'ushort');
                e.avg_reference(1,n)  = fread(fid,1,'char');
                e.clipadd(1,n)        = fread(fid,1,'char');
                e.x_coord(1,n)        = fread(fid,1,'float');
                e.y_coord(1,n)        = fread(fid,1,'float');
                e.veog_wt(1,n)        = fread(fid,1,'float');
                e.veog_std(1,n)       = fread(fid,1,'float');
                e.snr(1,n)            = fread(fid,1,'float');
                e.heog_wt(1,n)        = fread(fid,1,'float');
                e.heog_std(1,n)       = fread(fid,1,'float');
                e.baseline(1,n)       = fread(fid,1,'short');
                e.filtered(1,n)       = fread(fid,1,'char');
                e.fsp(1,n)            = fread(fid,1,'char');
                e.aux1_wt(1,n)        = fread(fid,1,'float');
                e.aux1_std(1,n)       = fread(fid,1,'float');
                e.sensitivity(1,n)    = fread(fid,1,'float');
                e.gain(1,n)           = fread(fid,1,'char');
                e.hipass(1,n)         = fread(fid,1,'char');
                e.lopass(1,n)         = fread(fid,1,'char');
                e.page(1,n)           = fread(fid,1,'uchar');
                e.size(1,n)           = fread(fid,1,'uchar');
                e.impedance(1,n)      = fread(fid,1,'uchar');
                e.physicalchnl(1,n)   = fread(fid,1,'uchar');
                e.rectify(1,n)        = fread(fid,1,'char');
                e.calib(1,n)          = fread(fid,1,'float');
        end
        
        if ftell(fid)~=(900+h.nchannels*75),
                warning(['supicous Neuroscan file ',FILENAME]);
        end;
        
end;

CNT.VERSION = str2num(char(h.rev(8:12)'));
CNT.CNT.type = h.type;
CNT.PID = h.id;
CNT.ID.Operator = char(h.oper');	%
CNT.ID.Doctor   = char(h.doctor');	%
CNT.ID.referral = char(h.referral');	%
CNT.ID.Hospital = char(h.hospital');	%
CNT.Patient.Name= char(h.patient');	%
CNT.Patient.Age = h.age;	%
CNT.Patient.Sex = char(h.sex');	%
CNT.Patient.Handedness=char(h.hand');	%	
CNT.Patient.Medication=char(h.med');	%	
CNT.Patient.Classification=char(h.category');	%	 
CNT.Patient.State=char(h.state');	%	
CNT.Session.Label=char(h.label');	%	
CNT.Date=char(h.date');	%	
CNT.Time=char(h.time');	%	
CNT.T0 = [str2num(CNT.Date(7:length(CNT.Date))),str2num(CNT.Date(1:2)),str2num(CNT.Date(4:5)),str2num(CNT.Time(1:2)),str2num(CNT.Time(4:5)),str2num(CNT.Time(7:8))];
if     CNT.T0(1) > 99,
elseif CNT.T0(1) > 80, 	CNT.T0(1) = CNT.T0(1) + 1900;
else			CNT.T0(1) = CNT.T0(1) + 2000;
end;

CNT.NS=h.nchannels;	%	
CNT.SampleRate=h.rate;	% D-to-A rate	
CNT.Scale=h.scale;	% scale factor for calibration
CNT.Scale2=h.ampsensitivity;
CNT.ChanTyp=zeros(CNT.NS,1);
%CNT.ChanTyp(h.trigchnl)=
CNT.HeadLen = 900 + 75*CNT.NS;
CNT.PhysDim = 'µV';


tmp = [30, 40, 50, 70, 100, 200, 500, 1000, 1500, 2000, 2500, 3000]; % LOWPASS
CNT.Filter.LowPass  = tmp(h.lowpass+1);
tmp = [0, 0, .05, .1, .15, .3, 1, 5, 30, 100, 150, 300]; %HIGHPASS
CNT.Filter.HighPass = tmp(h.highpass+1);
CNT.Filter.Notch   = h.notch;
CNT.CNT.Filter.LowCutOff  = h.lowcutoff;
CNT.CNT.Filter.HighCutOff = h.highcutoff;
CNT.CNT.Filter.NotchOn = h.filterflag;
CNT.CNT.Filter.ON   = [e(:).filtered];
CNT.CNT.minor_revision = h.minor_rev;
CNT.CNT.ChannelOffset = h.channeloffset;
CNT.CNT.NumSamples = h.numsamples;
CNT.Label = setstr(e.lab');

if CHAN==0, CHAN=1:CNT.NS; end;
CNT.SIE.ChanSelect = CHAN;
CNT.SIE.InChanSelect = CHAN;

CNT.FILE.POS = 0;
if (h.type==0),
	if ~strcmp(upper(CNT.FILE.Ext),'AVG'),
		fprinf(2,'Warning CNTOPEN: filetype 0 (AVG) does not match file extension (%s).\n',CNT.FILE.Ext); 
	end;
	CNT.TYPE='AVG';
        CNT.AS.endpos = 1;
	CNT.NRec = 1;
        CNT.SPR  = h.pnts;
        CNT.Cal  = e.calib./e.n;   % scaling
	CNT.AS.bpb = h.pnts*h.nchannels*4+5;
	CNT.AS.spb = h.pnts*h.nchannels;
	CNT.Dur = CNT.SPR/CNT.SampleRate;
        
        
elseif strcmp(upper(CNT.FILE.Ext),'COH')        
        warning('.COH data not supported yet')
        CNT.COH.directory = fread(CNT.FILE.FID,[CNT.NS,CNT.NS],'int32');
        CNT.SPR = h.pnts;
        
        
elseif strcmp(upper(CNT.FILE.Ext),'CSA')        
        warning('.CSA data not supported yet')
        CNT.SPR  = h.pnts;
        CNT.NRec = h.compsweeps;
        
        
elseif (h.type==1),
	if ~strcmp(upper(CNT.FILE.Ext),'EEG'),
		fprinf(2,'Warning CNTOPEN: filetype 1 (EEG) does not match file extension (%s).\n',CNT.FILE.Ext); 
	end;
	CNT.TYPE   = 'EEG';
        CNT.SPR    = h.pnts;
        CNT.NRec   = h.compsweeps;
        CNT.AS.spb = CNT.NS*CNT.SPR;	% Samples per Block
        
        % Sometimes h.eventtablepos seems to need a correction, also I've not figured out why. 
        % The Manual SCAN 4.2 Vol II, Page Headers-7 refers to "286 SCAN manual". Maybe this could bring a clarification. 
        % Anyway, the following code deals with the problem.   
        CNT.AS.bpb = -1;
        if CNT.CNT.minor_revision==12,
                CNT.AS.bpb = 2*CNT.AS.spb+1+2+2+4+2+2;
                CNT.GDFTYP = 3; %'int16';
                % correct(?) eventtablepos
                h.eventtablepos = CNT.HeadLen + CNT.NRec*CNT.AS.bpb;    	    
        else
                if CNT.CNT.minor_revision~=16,
                        fprintf(CNT.FILE.stderr,'Warning CNTOPEN: EEG-Format Minor-Revision %i not tested.\n',CNT.CNT.minor_revision);
                end;
                
                tmp = (CNT.AS.spb*2+1+2+2+4+2+2);
                if (h.eventtablepos-CNT.HeadLen)==(tmp*CNT.NRec),
                        CNT.AS.bpb = tmp;
                        CNT.GDFTYP = 3; %'int16';
                end;
                tmp = (CNT.AS.spb*4+1+2+2+4+2+2);
                if (h.eventtablepos-CNT.HeadLen)==(tmp*CNT.NRec),
                        CNT.AS.bpb = tmp;
                        CNT.GDFTYP = 5; %'int32';
                end;
        end; 
        if CNT.AS.bpb < 0;
                fprintf(CNT.FILE.stderr,'Error CNTOPEN: header information of file %s corrupted.\n',CNT.FileName);
                fclose(CNT.FILE.FID);
                CNT.FILE.FID = -1;
                return;
	end;
        
        CNT.Calib = [-[e.baseline];eye(CNT.NS)]*diag([e.sensitivity].*[e.calib]/204.8);
        CNT.AS.endpos = CNT.NRec;
	CNT.FLAG.TRIGGERED = 1;
	CNT.Dur = CNT.SPR/CNT.SampleRate;
        
else  % if any(h.type==[2,184]),
	if ~strcmp(upper(CNT.FILE.Ext),'CNT'),
		fprinf(2,'Warning CNTOPEN: filetype 2 (CNT) does not match file extension (%s).\n',CNT.FILE.Ext); 
	end;
        CNT.TYPE = 'CNT';
        %CNT.SPR    = h.numsamples;
	CNT.AS.bpb = CNT.NS*2;	% Bytes per Block
	CNT.AS.spb = CNT.NS;	% Samples per Block
        CNT.SPR    = (h.eventtablepos-CNT.HeadLen)/CNT.AS.bpb;
	CNT.AS.endpos = CNT.SPR;
        
        CNT.NRec   = 1;
	CNT.Calib  = [-[e.baseline];eye(CNT.NS)]*diag([e.sensitivity].*[e.calib]/204.8);
	CNT.FLAG.TRIGGERED = 0;	        
	CNT.Dur = 1/CNT.SampleRate;
end;


%%%%% read event table 
CNT.EVENT.N     = h.numevents;
if (CNT.EVENT.N > 0);
        fseek(CNT.FILE.FID,h.eventtablepos,'bof');
        CNT.EVENT.TeegType   = fread(fid,1,'uchar');	%	
        CNT.EVENT.TeegSize   = fread(fid,1,'int32');	%	
        CNT.EVENT.TeegOffset = fread(fid,1,'int32');	%	
        
        fseek(CNT.FILE.FID,CNT.EVENT.TeegOffset,'cof');
        
        k=0;
        K=1;
        Teeg=[];
        while K < CNT.EVENT.TeegSize,
                k = k + 1;
                Teeg.Stimtype =  fread(fid,1,'int16');        
                Teeg.Keyboard =  fread(fid,1,'char');        
                tmp =  fread(fid,1,'uint8');        
                Teeg.KeyPad = rem(tmp,16); %bitand(tmp,15);
                Teeg.Accept = (fix(tmp/16)*16)==13; % (bitshift(tmp,-4)==13);  % 0xd = accept, 0xc = reject 
                
                Teeg.Offset   =  fread(fid,1,'int32');        
                K = K + 8;
                if CNT.EVENT.TeegType==2,
                        Teeg.Type =  fread(fid,1,'int16');        
                        Teeg.Code =  fread(fid,1,'int16');        
                        Teeg.Latency =  fread(fid,1,'float32');        
                        Teeg.EpochEvent =  fread(fid,1,'char');        
                        Teeg.Accept2  =  fread(fid,1,'char');        
                        Teeg.Accuracy =  fread(fid,1,'char');        
                        K = K + 11;        
                end;        
	        CNT.EVENT.Teeg(k) = Teeg;
        end
        CNT.EVENT.TYP =  cat(1,CNT.EVENT.Teeg(:).Stimtype);
        CNT.EVENT.POS = (cat(1,CNT.EVENT.Teeg(:).Offset) - CNT.HeadLen) ./ CNT.AS.bpb;
end;

fseek(CNT.FILE.FID, CNT.HeadLen, 'bof');
