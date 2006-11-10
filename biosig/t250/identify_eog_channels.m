function chEOG=identify_eog_channels(fn); 
% IDENTIFY_EOG_CHANNELS returns bipolar EOG channels for 
%  correcting of EOG artifacts using regression analysis
% 
%  EOGchan = IDENTIFY_EOG_CHANNELS(...) 
%
% EOGchan is a sparse matrix of size number_of_channels x 2. 
% The sparsity ensures that missing samples of unrelated channels 
% do not affect the data.  
%
% see also: REGRESS_EOG, SLOAD
%
% Reference(s):
% [1] Schlogl A, Keinrath C, Zimmermann D, Scherer R, Leeb R, Pfurtscheller G. 
%	A fully automated correction method of EOG artifacts in EEG recordings.
%	Clin Neurophysiol. 2006 Nov 4;


if ischar(fn), 
	HDR=sopen(fn); 
	HDR=sclose(HDR); 
elseif isstruct(fn)
	HDR=fn; 
end; 	

if any(any(isnan(HDR.THRESHOLD(:,1:2))))
	warning('missing threshold(s)'); 
end; 	

% graz 
g1 = strmatch('EOG-left',lower(HDR.Label));
g2 = strmatch('EOG-central',lower(HDR.Label));
g3 = strmatch('EOG-right',lower(HDR.Label));

% berlin
v1 = strmatch('eogv1',lower(HDR.Label));
v2 = strmatch('eogv2',lower(HDR.Label));
v0 = strmatch('eogv',lower(HDR.Label));
v3 = strmatch('eogvp',lower(HDR.Label));
v4 = strmatch('eogvn',lower(HDR.Label));

h1 = strmatch('eogh1',lower(HDR.Label));
h2 = strmatch('eogh2',lower(HDR.Label));
h0 = strmatch('eogh' ,lower(HDR.Label));
h3 = strmatch('eoghp',lower(HDR.Label));
h4 = strmatch('eoghn',lower(HDR.Label));

g = [g1;g2;g3]; 
v = [v1,v2,v3,v4,v0];
h = [h1,h2,h3,h4,h0];
if length(g)==3,
	chEOG = sparse([g1,g2,g2,g3],[1,1,2,2],[1,-1,1-1],HDR.NS,2);
elseif length(g)==2,
	chEOG = sparse([g1,g2,g3],[1,1],[1,-1],HDR.NS,1);
else 
	c = (length(v)>0);  
	sz2 = (length(v)>0) + (length(h)>0);  
	if length(v), 
		chEOG = sparse(v,c,1,HDR.NS,sz2); 
	elseif length(v)==2, 
		chEOG = sparse(v,[c,c],[1,-1],HDR.NS,sz2); 
	else 
		chEOG = 0; 
	end;
	if length(h), 
		chEOG = chEOG+sparse(h,1+c,1,HDR.NS,1+c); 
	elseif length(h)==2, 
		chEOG = chEOG+sparse(h,[1,1]+c,[1,-1],HDR.NS,1+c); 
	end;
end; 

if size(chEOG,2)<2, 
	warning('EOG missing')
end; 
