function [H,HDR]=eeg2hist(FILENAME,CHAN);
% EEG2HIST Histogram test 
% Calculates the histogram and derived measures of biosignal recordings. 
% This is useful for quality control of biosignal recordings [1-3].  
%
% [HISTOG,HDR]=EEG2HIST(FILENAME,CHAN)
%
% input: FILENAME   EEG-File
%        CHAN       Channel select
% output: 
%        HISTOG    each column is the histogram of one EDF channel
%	 HDR	   header information
% 
% see also: tsa/hist2res, plota
%
%
% REFERENCES:
% [1] A. Schlögl, B. Kemp, T. Penzel, D. Kunz, S.-L. Himanen,A. Värri, G. Dorffner, G. Pfurtscheller.
% Quality Control of polysomnographic Sleep Data by Histogram and EntropyAnalysis. 
% Clin. Neurophysiol. 1999, Dec; 110(12): 2165 - 2170.
%
% [2] A. Schlögl, G. Klösch, W. Koele, J. Zeitlhofer, P.Rappelsberger, G. Pfurtscheller
% Qualitätskontrolle von Biosignalen,
% Jahrestagung der Österreichischen Gesellschaft für KlinischeNeurophysiologie, 27. Nov. 1999, Vienna.
%
% [3] http://www.dpmi.tu-graz.ac.at/~schloegl/lectures/Q/index.htm
%
% [4] A. Schlögl, Time Series Analysis toolbox for Matlab. 1996-2003
% http://www.dpmi.tu-graz.ac.at/~schloegl/matlab/tsa/


if nargin<2, CHAN=0; end; 


HDR = eegopen(FILENAME,'r',CHAN,'UCAL');	% open EEG file in uncalibrated mode (no scaling of the data)
if HDR.FILE.FID<0,
        fprintf(2,'EEG2HIST: couldnot open file %s.\n',FILENAME); 
        return;
end;

if CHAN<1, CHAN=1:HDR.NS; end;

H.datatype='HISTOGRAM';

if strcmp(HDR.TYPE,'BKR') | strcmp(HDR.TYPE,'CNT') | strcmp(HDR.TYPE,'EEG'),
        [s,HDR]=eegread(HDR);
        
        H.H = zeros(2^16,HDR.NS);
        for l = 1:HDR.NS,
                H.H(:,l)=sparse(s(:,l)'+2^15+1,1,1,2^16,1);
                %for k = s(:,l)'+2^15+1, H.H(k,l) = H.H(k,l)+1;  end;
        end;
        tmp = find(any(H.H,2));
        H.X = tmp-2^15-1; 	%int16
        H.H = H.H(tmp,:);

        
elseif strcmp(HDR.TYPE,'BDF'),
        [s,HDR] = eegread(HDR);
        
        H.H = sparse(2^24,HDR.NS);
        for l = 1:HDR.NS,
                H.H(:,l)=sparse(s(:,l)'+2^23+1,1,1,2^24,1);
                %for k = s(:,l)'+2^23+1, H.H(k,l) = H.H(k,l)+1; end;
        end;
        tmp = find(any(H.H,2));
        H.X = tmp-2^23-1; 
        H.H = H.H(tmp,:);
        
elseif strcmp(HDR.TYPE,'EDF');
        NoBlks=ceil(60/EDF.Dur);
        bi=[0;cumsum(HDR.SPR)];     
        ns=length(CHAN);
        
        H.H = zeros(2^16,HDR.NS);
        
        l=1;
        while (l<HDR.NRec) & ~feof(HDR.FILE.FID)
                % READ BLOCKS of DATA
                [S, count] = fread(HDR.FILE.FID,[HDR.AS.spb,NoBlks],'int16');
                if count < HDR.AS.spb*NoBlks
                        fprintf(2,'    Warning EEG2HIST: read error, only %i of %i read\n', count, spb*NoBlks);
                end;
                
                %%%%% HISTOGRAM
                h = zeros(2^16,ns);
                for l=1:ns,
                        %for k=reshape(S(bi(CHAN(l))+1:bi(CHAN(l)+1),:),1,HDR.SPR(l)*NoBlks)+2^15+1, H.H(k,l) = H.H(k,l)+1; end;     
                        h = sparse(S(bi(CHAN(l))+1:bi(CHAN(l)+1),:)+2^15+1,1,1,2^16,1);	                        
                        H.H(:,ns) = H.H(:,ns) + h;
                end;
                
                l=l+NoBlks; 
        end; % WHILE     
        tmp = find(any(H.H,2));
        H.X = tmp-2^15-1; 	%int16
        H.H = H.H(tmp,:);
else
        fprintf(2,'EEG2HIST: format %s not implemented yet.\n',HDR.TYPE);
        
end;
H.N = sum(H.H);

HDR=eegclose(HDR);

