% BIOSIG toolbox contains many useful functions for biomedical signal processing
%     http://biosig.sf.net/
%
% Copyright (C) 2003, 2004 by Alois Schloegl <a.schloegl@ieee.org>
% WWW: http://biosig.sf.net/
% $Revision: 1.5 $ 
% $Id: Contents.m,v 1.5 2004-12-03 14:19:49 schloegl Exp $
% This is part of the BIOSIG-toolbox http://biosig.sf.net/
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
% 
% 
% 
% 
% === INDEX  BIOSIG ===
% 
% DOC: Documentation
% ---------------------------------------------
% 	header.txt	specification of header (Dokumentation) 
% 
% 
% demo: [demo's, examples] 
% ---------------------------------------------
% 	demo1	QRS-detection
% 	demo2	estimates and validates BCI classifier
%	demo3	demonstrates how the generate an EDF/GDF/BDF file
%	demo4	Demonstrates how the generate an BKR file
%	demo5	Demonstrates how the generate an WAV file
%       demo6   transfer functions of lumped circuit model
%       scptest tests loading routine of SCP-ECG data
% 	
% 
% T100: [Data Acquistion] 
% ---------------------------------------------
% 
% 
% 
% T200: Data Formats
% ---------------------------------------------
% 	SOPEN		opens biosig data and reads header information
% 	SREAD		reads biosig data
% 	SCLOSE	closes biosig file
% 	SWRITE	writes biosig data (currently only BKR, EDF, BDF implemented)
% 	SSEEK		set file positon indicator
% 	STELL		returns file position indicator
% 	SEOF		checks for end-of-file
%	SREWIND		sets file pointer to the start
% 
% 	SLOAD 	Opens, reads all data and closes file Biosig files. 
%  	and some utility functions
% 
% 
% T250: Quality Control and Artifact Processing
% ---------------------------------------------
%	ARTIFACT_SELECTION	converts artifact scorings into trial selections
% 	EEG2HIST	calculates histogram
% 	GETTRIGGER	gets trigger points
% 	TRIGG		extract fixed-length trials around trigger points	
%	detect_muscle	detection of muscle artefacts using a inverse filter
% 
% 
% T300: Signal Processing and Feature extraction
% ---------------------------------------------
% 	processing	general framework for blockwise-dataprocessing
% 	getar0		initial AAR parameters
%	QRScorr		correctiong of QRS-detection
%	ECTBcorr	correction of Ectopic beat effect
%	TFMVAR		time-frequency multivariate autoregressive modelling
% 	LUMPED          Lumped Circuit model for the EEG alpha rhythm
%       paynter         Paynter filter for Amplitude demodulation of EMG
% 	
% 
% T400: Classification, Single Trial Analysis, Statistics,
% ---------------------------------------------
% 	LDBC		linear discriminant analysis
% 	MDBC		mahalanobis distance based classifier
% 	LLBC		log-likelihood based classifier
% 	DECOVM		decomposes an "extended" covariance matrix	
% 	GETCLASSIFIER	estimates classifier from labelled data
% 	FINDCLASSIFIER1	obtains classifier includeding performance test. 
% 	
% 
% T490: Evaluation criteria
% ---------------------------------------------
% 	AUC		area under the curve
% 	ROC		receiver-operator-characteristics 	
% 	MUTINFO		mutual information
% 	QCMAHAL		quality check of multiple discriminator
% 	KAPPA		kappa statistics
% 	BCI3EVAL	Evaluation of BCI results (triggered output)
% 	BCI4EVAL	Evaluation of BCI results (continous output)
%	CRITERIA2005IIIb   calculates maximum steepness of mutual information
% 	
% 
% T500: Presentation, Output
% ---------------------------------------------
% 	PLOTA		general plot functions for various data structures
%	SVIEW		simple signal viewer 
%       ELPOS           2-D electrode positions
%       ELPOS3          3-D electrode positions	
% 
%
% T550: Topographic Mapping, 3-dimensional display
% ---------------------------------------------
% 
% 
% T600: Interactive Viewer and Scoring  
% ---------------------------------------------
%	SVIEWER		Interactive Viewer and Scoring
% 	VIEWEDF		EDF-Viewer 
%



