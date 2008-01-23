% DEMO8: overflow detection based on [1]	
% 
% REFERENCES: 
% [1] A. Schlögl, B. Kemp, T. Penzel, D. Kunz, S.-L. Himanen,A. Värri, G. Dorffner, G. Pfurtscheller.
%   Quality Control of polysomnographic Sleep Data by Histogram and EntropyAnalysis. %   Clin. Neurophysiol. 1999, Dec; 110(12): 2165 - 2170.%   http://dx.doi.org/10.1016/S1388-2457(99)00172-8


[filename, pathname, filterindex] = uigetfile('*.*', 'Pick an EEG/ECG-file');

figure(1);
fprintf(1,'Compute histograms and select Thresholds based on the work :\n');
fprintf(1,'  [1] A. Schlögl, B. Kemp, T. Penzel, D. Kunz, S.-L. Himanen,A. Värri, G. Dorffner, G. Pfurtscheller.\n'); 
fprintf(1,'   Quality Control of polysomnographic Sleep Data by Histogram and EntropyAnalysis. \n'); fprintf(1,'   Clin. Neurophysiol. 1999, Dec; 110(12): 2165 - 2170.\n'); fprintf(1,'   http://dx.doi.org/10.1016/S1388-2457(99)00172-8  or http://www.dpmi.tugraz.at/schloegl/publications/schloegl1999.pdf\n\n'); 
fprintf(1,'   EEG2HIST computes the histograms and allows \n');
fprintf(1,'   you to set the Threshold values using the mouse. \n');
fprintf(1,'   The "side lobes" caused by the saturation should \n');
fprintf(1,'   appear red. If no saturation (side lobes) occur, \n');
fprintf(1,'   select thresholds above and below the dynamic range. \n');
fprintf(1,'   If you want to change the thresholds for some channels \n');
fprintf(1,'   you can go back to these channels. When you have finished \n');
fprintf(1,'   defining thresholds, press any key on the keyboard. \n'); 

HDR=eeg2hist(fullfile(pathname,filename)); 

display('Show results of QC analysis');
plota(HDR); 

[s,HDR]=sload(HDR); 

figure(2); 
sview(s,HDR); 






