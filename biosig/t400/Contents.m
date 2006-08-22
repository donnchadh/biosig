% BIOSIG/T400 contains classifiers.  
%
% TRAIN_SC      train classifier
% TEST_SC       test  classifier
% UNTRAIN_SC	decremental training of classifier (depre
% FC0   generates classifier from "asychronous" data
% FINDCLASSIFIER generates BCI classifier [1-3]
%
% obsolete: 
% FINDCLASSIFIER1 generate classifier for a BCI [1,2]
% FINDCLASSIFIER2 2nd generation classifier for BCI [2,3]
%
%  helper function%
% PERM	permutations of indices in trials 
% TRAIN_LDA_SPARSE sparse LDA classifier
%
% see also: NaN/COVM, 
%
% REFERENCES: 
% [1] Schlögl A, Neuper C, Pfurtscheller G
% 	Estimating the mutual information of an EEG-based Brain-Computer-Interface
%  	Biomedizinische Technik 47(1-2): 3-8, 2002.
% [2] Schlögl A, Keinrath C, Scherer R, Pfurtscheller G,
%	Information transfer of an EEG-based Bran-computer interface.
%	Proceedings of the 1st International IEEE EMBS Conference on Neural Engineering, Capri, Italy, Mar 20-22, 2003 
% [3] Schlögl A, Lee FY, Bischof H, Pfurtscheller G
%	Characterization of Four-Class Motor Imagery EEG Data for the BCI-Competition 2005.
%	Journal of neural engineering 2 (2005) 4, S. L14-L22
% [4] Schlögl A, Kronegg J, Huggins JE, Mason SG;
%	Evaluation criteria in BCI research.
%	(Eds.) G. Dornhege, J.R. Millan, T. Hinterberger, D.J. McFarland, K.-R.Müller;
%	Towards Brain-Computer Interfacing, MIT press (accepted)
% [5] Schlögl A, Brunner C, Scherer R, Glatz A
%       BioSig - an open source software library for BCI research.
%       (Eds.) G. Dornhege, J.R. Millan, T. Hinterberger, D.J. McFarland, K.-R. Müller;
%       Towards Brain-Computer Interfacing, MIT press (accepted).

%	$Id: Contents.m,v 1.6 2006-08-22 17:45:37 schloegl Exp $
%	Copyright (c) 1997-2005,2006 by Alois Schloegl
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

