% BIOSIG toolbox contains many useful functions for biomedical signal processing
% Version 0.26 02-Oct-2003
%
% Copyright (C) 2003 by Alois Schloegl <a.schloegl@ieee.org>
% WWW: http://biosig.sf.net/
% $Revision: 1.2 $ 
% $Id: Contents.m,v 1.2 2003-10-03 10:26:34 schloegl Exp $
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
% 	demo1		QRS-detection
% 	demo2		estimates and validates BCI classifier
% 	
% 
% T100: [Data Acquistion] 
% ---------------------------------------------
% 
% 
% 
% T200: Data Formats
% ---------------------------------------------
% 	EEGOPEN		opens biosig data and reads header information
% 	EEGREAD		reads biosig data
% 	EEGCLOSE	closes biosig file
% 	EEGWRITE	writes biosig data (currently only BKR, EDF, BDF implemented)
% 	EEGSEEK		set file positon indicator
% 	EEGTELL		returns file position indicator
% 	EEGEOF		checks for end-of-file
% 
% 	LOADEEG 	Opens, reads all data and closes file Biosig files. 
%  	and some utility functions
% 
% 
% T250: Quality Control and Artifact Processing
% ---------------------------------------------
% 	EEG2HIST	calculates histogram
% 	GETTRIGGER	gets trigger points
% 	TRIGG		extract fixed-length trials around trigger points	
% 
% 
% T300: Signal Processing and Feature extraction
% ---------------------------------------------
% 	processing	general framework for blockwise-dataprocessing
% 	getar0		initial AAR parameters
%	QRScorr		correctiong of QRS-detection
%	ECTBcorr	correction of Ectopic beat effect
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
% 	BCI3EVAL	Evaluation of BCI results
% 	
% 
% T500: Presentation, Output
% ---------------------------------------------
% 	PLOTA		general plot functions for various data structures
% 
% 
% T550: Topographic Mapping, 3-dimensional display
% ---------------------------------------------
% 
% 
% T600: Interactive Viewer and Scoring  
% ---------------------------------------------
% 	VIEWEDF		EDF-Viewer 



