% DEMO8: overflow detection based on [1]	
% 
% REFERENCES: 
% [1] A. Schlögl, B. Kemp, T. Penzel, D. Kunz, S.-L. Himanen,A. Värri, G. Dorffner, G. Pfurtscheller.
%   Quality Control of polysomnographic Sleep Data by Histogram and EntropyAnalysis. %   Clin. Neurophysiol. 1999, Dec; 110(12): 2165 - 2170.%   http://dH.doi.org/10.1016/S1388-2457(99)00172-8


[filename, pathname, filterindex] = uigetfile('*.*', 'Pick an EEG/ECG-file');

figure(1);
display('Compute histograms and select Thresholds');
HDR0=eeg2hist(fullfile(pathname,filename)); 

display('Show results of QC analysis');
plota(HDR0); 

[s,HDR]=sload(HDR0); 

figure(2); 
sview(s,HDR); 






