function [y1]=rs(y1,T,f2)
% [y2] = rs(y1, T) resamples y1 to the target sampling rate y2 using T
% [y2] = rs(y1, f1, f2) resamples y1 with f1 to the target sampling rate f2 
%
% RS does not require overlap data. 

%	Version 1.12
%	21.Sep.2001
%	Copyright (c) by 1999-2001 Alois Schloegl
%	a.schloegl@ieee.org	

% .CHANGELOG
% 21-09-2001 	f2>f1 implemented 

if nargin==3
        f1=T;
        if f1==f2
                return;
        elseif f1>f2
                D=f1/f2;
                [yr,yc]=size(y1);
                LEN=yr/D;
		y2=zeros(yr*f2/f1,yc);
                for k=0:LEN-1
                        y2(k+1,:)=sum(y1(k*D+(1:D),:),1)/D;
                end;
		y1=y2;
        else %f1<f2
		y1=y1(ceil((1:size(y1,1)*f2/f1)/f2*f1),:);                
        end;
elseif nargin==2
        [f1,f2]=size(T);
        if f1==f2,
                return;
        end;
        [yr,yc]=size(y1);
        LEN=yr/f1;
	y2=zeros(yr*f2/f1,yc);
        for k=0:LEN-1
                y2(k*f2+(1:f2),:)=T'*y1(k*f1+(1:f1),:);
        end;
	y1=y2;
end;
