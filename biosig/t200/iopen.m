function [HDR,data] = iopen(HDR,PERMISSION,CHAN,MODE,arg5,arg6)
% IOPEN opens image files for reading and writing and returns 
%       the header information. Many different IMAGE formats are supported.
%
% HDR = iopen(Filename, PERMISSION, [, CHAN [, MODE]]);
%
% PERMISSION is one of the following strings 
%	'r'	read header
%	'w'	write header
%
% HDR contains the Headerinformation and internal data
%
% see also: SLOAD, SREAD, SSEEK, STELL, SCLOSE, SWRITE, SEOF


% This program is free software; you can redistribute it and/or
% modify it under the terms of the GNU General Public License
% as published by the Free Software Foundation; either version 2
% of the License, or (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

%	$Id: iopen.m,v 1.3 2005-07-01 22:36:36 schloegl Exp $
%	(C) 2005 by Alois Schloegl <a.schloegl@ieee.org>	
%    	This is part of the BIOSIG-toolbox http://biosig.sf.net/

data = [];

if isstruct(HDR),

elseif ischar(HDR) & exist(HDR)==2,
	HDR = getfiletype(HDR);
end;


if 0, 

elseif strcmp(HDR.TYPE,'IMAGE:BMP'),
        HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le');
        fseek(HDR.FILE.FID,10,-1);
        
        tmp = fread(HDR.FILE.FID,4,'uint32');
        HDR.HeadLen = tmp(1);
        HDR.BMP.sizeBitmapInfoHeader = tmp(2);
        HDR.IMAGE.Size = tmp(3:4)';
        
        tmp = fread(HDR.FILE.FID,2,'uint16');
        HDR.BMP.biPlanes = tmp(1);
        HDR.bits = tmp(2);
        
        tmp = fread(HDR.FILE.FID,6,'uint32');
        HDR.BMP.biCompression = tmp(1);
        HDR.BMP.biImageSize = tmp(2);
        HDR.BMP.biXPelsPerMeter = tmp(3);
        HDR.BMP.biYPelsPerMeter = tmp(4);
        HDR.BMP.biColorUsed = tmp(5);
        HDR.BMP.biColorImportant = tmp(6);
	HDR.FILE.OPEN = 1; 
	
        
elseif strcmp(HDR.TYPE,'IMAGE:FITS'),
        HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-be');

	KK = 0; 
	HDR.HeadLen = ftell(HDR.FILE.FID);
	BlockSize = 0;
	while (HDR.FILE.size > HDR.HeadLen(max(KK,1))+BlockSize),
	KK = KK+1;
	FLAG.END = 0; 
	HDR.FITS{KK} = [];
	while ~FLAG.END,
		[tmp,c] = fread(HDR.FILE.FID,[80,36],'uchar');
		data  = char(tmp)';
		for k = 1:size(data,1),
			s = data(k,:);
			if strncmp(s,'COMMENT',7);
			    
			elseif strncmp(s,'HISTORY',7);

			elseif all(s(9:10)=='= ');
				len = min([80,find(s=='/')-1]);
				[key, t] = strtok(s, '= '); 
				key(key=='-') = '_';
				if s(11)==char(39),		% string
					[val, t] = strtok(s(11:len),char(39));
					HDR.FITS{KK} = setfield(HDR.FITS{KK},key,val);
				elseif any(s(30)=='TF'),    	% logical
					HDR.FITS{KK} = setfield(HDR.FITS{KK},key,s(30)=='T');
				elseif all(s(11:len)==' '), 	% empty
					HDR.FITS{KK} = setfield(HDR.FITS{KK},key,[]);
				elseif all(s(11:len)=='('), 	% complex
					[val,status] = str2double(s(11:len),[],'(,)');
					HDR.FITS{KK} = setfield(HDR.FITS{KK},key,val(1)+i*val(2));
				else 				% numerical
					[val,status] = str2double(s(11:len));
					if any(status),
						s(s=='D')='E';
						[val,status] = str2double(s(11:len));
					end;
					if any(status),
						fprintf(2,'Warning SOPEN (FITS): Expected numerical value - found string \n\t%s: %s\n',key,s(11:len));
						HDR.FITS{KK} = setfield(HDR.FITS{KK},key,s(11:len));
					else
						HDR.FITS{KK} = setfield(HDR.FITS{KK},key,val);
					end;	
				end	

			elseif strncmp(s,'END',3)
				FLAG.END = 1;
			
			%elseif strncmp(s,'         ',8),
			
			else
			%	fprintf(2,'ERROR SOPEN (FITS): "%s"\n',s);	
    			end;
		end;
	end;

	HDR.HeadLen(KK) = ftell(HDR.FILE.FID);
	HDR.IMAGE(KK).Size = [0,0];
	for k = 1:HDR.FITS{KK}.NAXIS,
		HDR.IMAGE(KK).Size(k) = getfield(HDR.FITS{KK},['NAXIS',int2str(k)]);
	end;
	HDR.IMAGE_Size(KK) = prod(HDR.IMAGE(KK).Size);

	HDR.GDFTYP = ['uint',num2str(HDR.FITS{1}.BITPIX)];

	%data = fread(HDR.FILE.FID,prod(HDR.IMAGE.Size),HDR.GDFTYP);
	%data = reshape(data,HDR.IMAGE.Size);  % * HDR.Cal + HDR.Off;
	
	BlockSize = ceil(prod(HDR.IMAGE(KK).Size)*abs(HDR.FITS{KK}.BITPIX)/(8*2880))*2880;
	fseek(HDR.FILE.FID,BlockSize,'cof');
	end;
	%fclose(HDR.FILE.FID);	
	HDR.FILE.OPEN = 1; 
	

elseif strcmp(HDR.TYPE,'IMAGE:IFS'),    % Ultrasound file format
        HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le');
        HDR.HeadLen = 512;
        hdr = fread(HDR.FILE.FID,[1,HDR.HeadLen],'uchar');
        HDR.Date = char(hdr(77:100));
        tmp = char(hdr(213:220));
        if strncmp(tmp,'32flt',5)
                HDR.GDFTYP = 'float32';
        elseif strncmp(tmp,'u8bit',5)
                HDR.GDFTYP = 'uint8';
        else
                
        end
        fclose(HDR.FILE.FID);

        
elseif strcmp(HDR.TYPE,'IMAGE:EXIF') | strncmp(HDR.TYPE,'IMAGE:JPG',9), 
        GDFTYP = {'uint8','char','uint16','uint32','2*uint32','int8','uint8','int16','int32','2*int32','float32','float64'};
        GDFTYP = {'uint8','char','uint16','uint32','uint64','int8','uint8','int16','int32','int64','float32','float64'};

        HDR.EXIF = [];
        HDR.JPEG = [];
        HDR.FILE.FID = fopen(HDR.FileName,'rb',HDR.Endianity);
        tag = fread(HDR.FILE.FID,[1],'uint16');
        if tag == hex2dec('FFD8'), % compressed: JPEG or EXIF
                
                POS = ftell(HDR.FILE.FID); 
                tag = fread(HDR.FILE.FID,1,'uint16');
                LEN = fread(HDR.FILE.FID,1,'uint16');
                while ftell(HDR.FILE.FID)<HDR.FILE.size,
                        if 0, 
                        elseif (tag == hex2dec('F9FE'))       % image data
                                HDR.data.compressed = fread(HDR.FILE.FID,LEN-2,'uint8');
                        elseif (tag == hex2dec('FEFA'))       % JPEG field: (thumbnail?)
                                HDR.JPEG.FEFA = fread(HDR.FILE.FID,LEN-2,'uint8');
                        elseif (tag == hex2dec('FF00'))       % JPEG field: (thumbnail?)
                                HDR.JPEG.FF00 = fread(HDR.FILE.FID,LEN-2,'uint8');
                        elseif (tag == hex2dec('FFC0'))       % JPEG field: start of frame
                                HDR.JPEG.BITS = fread(HDR.FILE.FID,1,'uint8');
                                HDR.JPEG.XYLines = fread(HDR.FILE.FID,[1,2],'uint16');
                                HDR.JPEG.SOF = fread(HDR.FILE.FID,10,'uint8');
                        elseif (tag == hex2dec('FFD9'))       % JPEG field: end of data
                                HDR.EXIF.EOD = 1; 
                        elseif (tag == hex2dec('FFC4'))       % JPEG field:
                                tmp = fread(HDR.FILE.FID,LEN-2,'uint8');
                                if isfield(HDR.JPEG,'C4');
                                        HDR.JPEG.C4{length(HDR.JPEG.C4)+1}=tmp;
                                else
                                        HDR.JPEG.C4{1} = tmp;
                                end;
                        elseif (tag == hex2dec('FFDA'))       % JPEG field: start of scan
                                HDR.JPEG.SOS = fread(HDR.FILE.FID,10,'uint8');
                        elseif (tag == hex2dec('FFDB'))       % EXIF field: Quantization Table 
                                tmp = fread(HDR.FILE.FID,[65,3],'uint8');
                                if isfield(HDR.JPEG,'DQT');
                                        HDR.JPEG.DQT{length(HDR.JPEG.DQT)+1}=tmp;
                                else
                                        HDR.JPEG.DQT{1} = tmp;
                                end;

                        elseif (tag == hex2dec('FFDD'))       % EXIF field: 
                                HDR.EXIF.DRI = fread(HDR.FILE.FID,1,'uint16');
                        elseif (tag == hex2dec('FFE0'))       % JPEG
                                HDR.JPEG.E0 = fread(HDR.FILE.FID,LEN-2,'uint8');
                        elseif 0, (tag == hex2dec('FFE1'))       % EXIF field: APP1
                                tmp = fread(HDR.FILE.FID,6,'uint8');
                                pos = ftell(HDR.FILE.FID); 
                                tmp = fread(HDR.FILE.FID,8,'uint8');
                                offset.IFD0 = fread(HDR.FILE.FID,1,'uint32')
                                fseek(HDR.FILE.FID,pos+offset.IFD0,'bof');
                                tmp = fread(HDR.FILE.FID,1,'uint16'),
                                for k=1:11,
                                        tmp = fread(HDR.FILE.FID,2,'uint16');
                                        tagid(k) = tmp(1);
                                        typ(k) = tmp(2);
                                        tmp = fread(HDR.FILE.FID,2,'uint32');
                                        count(k) = tmp(1);
                                        valoffset(k) = tmp(2);
                                end;
                                count,
                                len = fread(HDR.FILE.FID,1,'uint16') % jump to next IFD
                                if len>0, LEN = len; end; 
                                for k = 1:11, 
                                        dec2hex(tagid(k)),
                                        fseek(HDR.FILE.FID,pos+valoffset(k)-8,'bof');
                                        if (typ(k)==5)
                                                tmp = fread(HDR.FILE.FID,[count(k),2],'int32');
                                                tmp = tmp(:,1)/tmp(:,2);
                                        else
                                                k,
                                                tmp = fread(HDR.FILE.FID,count(k),GDFTYP{typ(k)});
                                        end;
                                        if tagid(k)==hex2dec('010e')
                                                HDR.EXIF.ImageDesc = char(tmp(:)');
                                        elseif tagid(k)==hex2dec('010f')
                                                HDR.EXIF.Make = char(tmp(:)');
                                        elseif tagid(k)==hex2dec('0110')
                                                HDR.EXIF.Model = char(tmp(:)');
                                        elseif tagid(k)==hex2dec('0121')
                                                HDR.EXIF.Orientation = tmp;
                                        elseif tagid(k)==hex2dec('011A')
                                                HDR.EXIF.XResolution = tmp;
                                        elseif tagid(k)==hex2dec('011B')
                                                HDR.EXIF.YResolution = tmp;
                                        elseif tagid(k)==hex2dec('0128')
                                                HDR.EXIF.ResolutionUnit = char(tmp);
                                        elseif tagid(k)==hex2dec('0132')
                                                HDR.EXIF.DateTime = char(tmp);
                                        elseif tagid(k)==hex2dec('0213')
                                                HDR.EXIF.YCbCrPostionining = tmp;
                                        elseif tagid(k)==hex2dec('8298')
                                                HDR.EXIF.Copyright = char(tmp);
                                        elseif tagid(k)==hex2dec('8769')
                                                HDR.EXIF.IdPointer = tmp;
                                        end;
                                end
                                %HDR.EXIF.APP1 = fread(HDR.FILE.FID,LEN-2-6-4,'uint8');
                        elseif (tag == hex2dec('FFE2'))       % EXIF field: APP2
                                HDR.EXIF.APP2 = fread(HDR.FILE.FID,LEN-2,'uint8');
                        elseif (tag == hex2dec('FFE3'))       % EXIF field: APP3
                                HDR.EXIF.APP3 = fread(HDR.FILE.FID,LEN-2,'uint8');
                        elseif (tag == hex2dec('FFEC'))       % JPEG
                                tmp = char(fread(HDR.FILE.FID,[1,LEN-2],'uint8'));
                                if isfield(HDR.JPEG,'EC');
                                        HDR.JPEG.EC{length(HDR.JPEG.EC)+1}=tmp;
                                else
                                        HDR.JPEG.EC{1} = tmp;
                                end;
                        elseif (tag == hex2dec('FFED'))       % JPEG
                                tmp = char(fread(HDR.FILE.FID,[1,LEN-2],'uint8'));
                                if isfield(HDR.JPEG,'ED');
                                        HDR.JPEG.ED{length(HDR.JPEG.ED)+1}=tmp;
                                else
                                        HDR.JPEG.ED{1} = tmp;
                                end;
                        elseif (tag == hex2dec('FFFD'))       % JPEG
                                tmp = char(fread(HDR.FILE.FID,[1,LEN-2],'uint8'));
                                if isfield(HDR.JPEG,'FD');
                                        HDR.JPEG.FD{length(HDR.JPEG.FD)+1}=tmp;
                                else
                                        HDR.JPEG.FD{1} = tmp;
                                end;
                        elseif (tag == hex2dec('FFEE'))       % JPEG
                                tmp = char(fread(HDR.FILE.FID,[1,LEN-2],'uint8'));
                                if isfield(HDR.JPEG,'EE');
                                        HDR.JPEG.EE{length(HDR.JPEG.EE)+1}=tmp;
                                else
                                        HDR.JPEG.EE{1} = tmp;
                                end;
                        elseif (tag == hex2dec('FFFE'))       % JPEG
                                tmp = char(fread(HDR.FILE.FID,[1,LEN-2],'uint8'));
                                if isfield(HDR.JPEG,'FE');
                                        HDR.JPEG.FE{length(HDR.JPEG.FE)+1}=tmp;
                                else
                                        HDR.JPEG.FE{1} = tmp;
                                end;
                        elseif 0, tag>=15/16*2^16; 
                                fprintf(1,'%5i\t%4x\t%5i\n',LEN,tag,tag);
                        else
                        end;
                        fseek(HDR.FILE.FID,POS+LEN+2,'bof');
                        POS = ftell(HDR.FILE.FID); 
                        tag = fread(HDR.FILE.FID,1,'uint16');
                        LEN = fread(HDR.FILE.FID,1,'uint16');
                end;
        end;
        fclose(HDR.FILE.FID);	

        
elseif strcmp(HDR.TYPE,'IMAGE:JPG'),  
        GDFTYP = {'uint8','char','uint16','uint32','2*uint32','int8','uint8','int16','int32','2*int32','float32','float64'};
        GDFTYP = {'uint8','char','uint16','uint32','uint64','int8','uint8','int16','int32','int64','float32','float64'};

        HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le');
        HDR.JPEG.H1 = fread(HDR.FILE.FID,[1,8],'uchar');

        % IFD
        TAG = fread(HDR.FILE.FID,1,'uint16');
        type = fread(HDR.FILE.FID,1,'uint16');
        count = fread(HDR.FILE.FID,1,'uint32');
        offset = fread(HDR.FILE.FID,1,'uint32');
        
        
	fclose(HDR.FILE.FID);	
	
        

elseif strcmp(HDR.TYPE,'IMAGE:PBMA') | strcmp(HDR.TYPE,'IMAGE:PGMA')  | strcmp(HDR.TYPE,'IMAGE:PPMA') ,
        HDR.FILE.FID = fopen(HDR.FileName,'rt','ieee-le');

	N = NaN;
	K = 1;
	s = [];
	HDR.IMAGE.Size = [inf,inf];
	data = [];
	while ~feof(HDR.FILE.FID) & (length(data)<prod(HDR.IMAGE.Size))
        	line = fgetl(HDR.FILE.FID);

		if isempty(line),
		elseif strncmp(line,'P1',2),
			N = 1; 
		elseif strncmp(line,'P2',2),
			N = 2; 
		elseif strncmp(line,'P3',2),
			N = 2; 
		elseif line(1)=='#',
		elseif isnumeric(line),
		elseif K==1,
			[tmp, status] = str2double(line);
			K = K + 1;
			HDR.IMAGE.Size = tmp;
			if status
				error('IOPEN (PPMA)');
			end;
		elseif K==N,
			[tmp, status] = str2double(line);
			K = K + 1;
			HDR.DigMax = tmp; 
		else
			line = line(1:min([find(line=='#'),length(line)]));	% remove comment
			[tmp,status] = str2double(char(line)); %,[],[9,10,13,32])
			if ~any(status),
				s = [s; tmp'];
			end;	
		end;
	end;	
	fclose(HDR.FILE.FID);

	if strcmp(HDR.TYPE,'IMAGE:PPMA'),
		if prod(HDR.IMAGE.Size)*3~=length(s),
			fprintf(HDR.FILE.stderr,'SLOAD(P3): %i * %i != %i \n',HDR.IMAGE.Size,length(s));
		else
			data = repmat(NaN,[HDR.IMAGE.Size,3]);
			data(:,:,1) = reshape(s(1:3:end),HDR.IMAGE.Size)';
			data(:,:,2) = reshape(s(2:3:end),HDR.IMAGE.Size)';
			data(:,:,3) = reshape(s(3:3:end),HDR.IMAGE.Size)';
	        end;
	else
		if prod(HDR.IMAGE.Size)~=length(s),
			fprintf(HDR.FILE.stderr,'SLOAD(P1/P2): %i * %i != %i \n',HDR.IMAGE.Size,length(s));
		else
			data = reshape(s,HDR.IMAGE.Size)';
	        end;
        end;


elseif strcmp(HDR.TYPE,'IMAGE:PBMB'),
        HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le');
	status = fseek(HDR.FILE.FID, HDR.HeadLen, 'bof');
	[tmp,count] = fread(HDR.FILE.FID,[HDR.IMAGE.Size(2)/8,HDR.IMAGE.Size(1)],'uint8');
        fclose(HDR.FILE.FID);
	
	data = zeros(HDR.IMAGE.Size)';
	
	for k = 1:8,
		data(:,k:8:HDR.IMAGE.Size(1)) = bitand(tmp',2^(8-k))>0;
	end;		

elseif strcmp(HDR.TYPE,'IMAGE:PGMB'),
        HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le');
	status = fseek(HDR.FILE.FID, HDR.HeadLen, 'bof');
	[data,count] = fread(HDR.FILE.FID,[HDR.IMAGE.Size(2),HDR.IMAGE.Size(1)],'uint8');
        fclose(HDR.FILE.FID);
	data = data';

elseif strcmp(HDR.TYPE,'IMAGE:PPMB'),
        HDR.FILE.FID = fopen(HDR.FileName,'rb','ieee-le');
	status = fseek(HDR.FILE.FID, HDR.HeadLen, 'bof');
	[tmp,count] = fread(HDR.FILE.FID,[3*HDR.IMAGE.Size(2),HDR.IMAGE.Size(1)],'uint8');
        fclose(HDR.FILE.FID);

	data = zeros([HDR.IMAGE.Size(1:2),3]);
	data(:,:,1) = tmp(1:3:end,:)';
	data(:,:,2) = tmp(2:3:end,:)';
	data(:,:,3) = tmp(3:3:end,:)';
	
        
elseif strcmp(HDR.TYPE,'IMAGE:TIFF'),
        GDFTYP = {'uint8','char','uint16','uint32','2*uint32','int8','uint8','int16','int32','2*int32','float32','float64'};
        GDFTYP = {'uint8','char','uint16','uint32','uint64','int8','uint8','int16','int32','int64','float32','float64'};
        SIZEOF = [1,1,2,4,8,1,1,2,4,8,4,8];
        
        HDR.FILE.FID = fopen(HDR.FileName,'rb',HDR.Endianity);
        [tmp,c] = fread(HDR.FILE.FID,2,'uint32');
        OFFSET = tmp(2);
        
        % read IFD
        K = 1;
        while OFFSET, 
                status = fseek(HDR.FILE.FID, OFFSET, 'bof');
                [NIFD,c] = fread(HDR.FILE.FID,1,'uint16');
                for k = 1:NIFD,
                        POS = ftell(HDR.FILE.FID);
                        [tmp,c] = fread(HDR.FILE.FID,2,'uint16');
                        TAG = tmp(1);
                        TYP = tmp(2);
                        [COUNT,c] = fread(HDR.FILE.FID,1,'uint32');
                        
                        FLAG = (TYP>0) & (TYP<=length(GDFTYP)); 
                        if FLAG,
                                if (COUNT * SIZEOF(TYP)) > 4,
                                        [OFFSET, c] = fread(HDR.FILE.FID, 1, 'uint32');
                                        status = fseek(HDR.FILE.FID, OFFSET, 'bof');
                                end;
                                
                                [VALUE,c] = fread(HDR.FILE.FID, COUNT, GDFTYP{TYP});
                                if any(TAG==[5,10])
                                        %	VALUE = VALUE(1:2:end)./VALUE(2:2:end);
                                end;
                        end;	
                        
                        if ~FLAG,
                                
                        elseif TAG==254
                                HDR.TIFF.NewSubFileType = VALUE;
                        elseif TAG==255
                                HDR.TIFF.SubFileType = VALUE;
                        elseif TAG==256	
                                HDR.IMAGE.Size(2) = VALUE;
                        elseif TAG==257	
                                HDR.IMAGE.Size(1) = VALUE;
                        elseif TAG==258	
                                HDR.Bits = VALUE(:)';
                                HDR.IMAGE.Size(3) = length(VALUE);
				if any(VALUE~=VALUE(1))
					fprintf(HDR.FILE.stderr,'Warning IOPEN: different BitsPerSample not supported.\n');
				end;
                        elseif TAG==259	
                                HDR.TIFF.Compression = VALUE;
                        elseif TAG==262,
                                HDR.FLAG.PhotometricInterpretation = ~VALUE;
                        elseif TAG==263,
                                HDR.FLAG.Thresholding = VALUE;
                        elseif TAG==264,
                                HDR.FLAG.CellWidth = VALUE;
                        elseif TAG==265,
                                HDR.FLAG.CellLength = VALUE;
                        elseif TAG==266,
                                HDR.FLAG.FillOrder = VALUE;
                        elseif TAG==269,
                                HDR.TIFF.DocumentName = char(VALUE);
                        elseif TAG==270,
                                HDR.TIFF.ImageDescription = char(VALUE);
                        elseif TAG==271,
                                HDR.TIFF.Maker = char(VALUE);
                        elseif TAG==272,
                                HDR.TIFF.Model = char(VALUE);
                        elseif TAG==273,
                                HDR.TIFF.StripOffset = VALUE;
                        elseif TAG==274,
                                HDR.TIFF.Orientation = VALUE;
                        elseif TAG==277,
                                HDR.TIFF.SamplesPerPixel = VALUE;
                        elseif TAG==278,
                                HDR.TIFF.RowsPerStrip = VALUE;
                        elseif TAG==279,
                                HDR.TIFF.StripByteCounts = VALUE;
                        elseif TAG==280,
                                HDR.DigMin = VALUE;
                        elseif TAG==281,
                                HDR.DigMax = VALUE;
                        elseif TAG==282,
                                HDR.TIFF.XResolution = VALUE;
                        elseif TAG==283,
                                HDR.TIFF.YResolution = VALUE;
                        elseif TAG==284,
                                HDR.TIFF.PlanarConfiguration = VALUE;
                        elseif TAG==285,
                                HDR.TIFF.PageName = char(VALUE);
                        elseif TAG==286,
                                HDR.TIFF.Xposition = VALUE;
                        elseif TAG==287,
                                HDR.TIFF.Yposition = VALUE;
                        elseif TAG==288,
                                HDR.TIFF.FreeOffset = VALUE;
                        elseif TAG==289,
                                HDR.TIFF.FreeBytesCount = VALUE;
                        elseif TAG==290,
                                HDR.TIFF.GrayResponseUnit = VALUE;
                        elseif TAG==291,
                                HDR.TIFF.GrayResponseCurve = VALUE;
                        elseif TAG==292,
                                HDR.TIFF.T4Options = VALUE;
                        elseif TAG==293,
                                HDR.TIFF.T6Options = VALUE;
                        elseif TAG==296,
                                if VALUE==1,
                                        HDR.TIFF.ResolutionUnit = '';
                                elseif VALUE==2,
                                        HDR.TIFF.ResolutionUnit = 'Inch';
                                elseif VALUE==3,
                                        HDR.TIFF.ResolutionUnit = 'cm';
                                end;	
                        elseif TAG==297,
                                HDR.TIFF.PageNumber = VALUE;
                        elseif TAG==301,
                                HDR.TIFF.TansferFunction = VALUE;
                        elseif TAG==305,
                                HDR.Software = char(VALUE');
                        elseif TAG==306,
                                HDR.TIFF.DateTime = char(VALUE');
                                [tmp,status] = str2double(char(VALUE'),': ');
                                if ~any(status)
                                        HDR.T0 = tmp;
                                end;	
                        elseif TAG==315,
                                HDR.Artist = char(VALUE);
                        elseif TAG==316,
                                HDR.TIFF.HostComputer = char(VALUE);
                        elseif TAG==317,
                                HDR.TIFF.Predictor = VALUE;
                        elseif TAG==318,
                                HDR.TIFF.WhitePoint = VALUE;
                        elseif TAG==319,
                                HDR.TIFF.PrimaryChromatics = VALUE;
                        elseif TAG==320,
                                HDR.TIFF.ColorMap = reshape(VALUE,3,2^HDR.Bits(1))';
                        elseif TAG==321,
                                HDR.TIFF.HalftoneHints = VALUE;
                        elseif TAG==322,
                                HDR.TIFF.TileWidth = VALUE;
                        elseif TAG==323,
                                HDR.TIFF.TileLength = VALUE;
                        elseif TAG==324,
                                HDR.TIFF.TileOffset = VALUE;
                        elseif TAG==325,
                                HDR.TIFF.TileByteCount = VALUE;
                        elseif TAG==332,
                                HDR.TIFF.InkSet = VALUE;
                        elseif TAG==333,
                                HDR.TIFF.InkNames = VALUE;
                        elseif TAG==334,
                                HDR.TIFF.NumberOfInks = VALUE;
                        elseif TAG==336,
                                HDR.TIFF.DotRange = VALUE;
                        elseif TAG==337,
                                HDR.TIFF.TargetPrinter = VALUE;
                        elseif TAG==338,
                                HDR.TIFF.ExtraSamples = char(VALUE);
                        elseif TAG==339,
                                HDR.TIFF.SampleFormat = VALUE;
                        elseif TAG==340,
                                HDR.TIFF.SMinSampleValue = VALUE;
                        elseif TAG==341,
                                HDR.TIFF.SMaxSampleValue = VALUE;
                        elseif TAG==342,
                                HDR.TIFF.TransferRange = VALUE;
                                
                        elseif TAG==512,
                                HDR.TIFF.JPEGProc = VALUE;
                        elseif TAG==513,
                                HDR.TIFF.JPEGInterchangeFormat = VALUE;
                        elseif TAG==514,
                                HDR.TIFF.JPEGInterchangeFormatLength = VALUE;
                        elseif TAG==515,
                                HDR.TIFF.JPEGRestartInterval = VALUE;
                                
                        elseif TAG==517,
                                HDR.TIFF.JPEGLosslessPredictors = VALUE;
                        elseif TAG==518,
                                HDR.TIFF.JPEGPointTransforms = VALUE;
                        elseif TAG==519,
                                HDR.TIFF.JPEGQTables = VALUE;
                        elseif TAG==520,
                                HDR.TIFF.JPEGDCTables = VALUE;
                        elseif TAG==521,
                                HDR.TIFF.JPEGACTables = VALUE;
                        elseif TAG==529,
                                HDR.TIFF.YCbCrCoefficients = VALUE;
                        elseif TAG==530,
                                HDR.TIFF.YCbCrSubSampling = VALUE;
                        elseif TAG==531,
                                HDR.TIFF.YCbCrPositioning = VALUE;
                        elseif TAG==532,
                                HDR.TIFF.ReferenceBlackWhite = VALUE;
                                
                        elseif TAG==1024,
                                HDR.GeoTIFF.GTModelTypeGeoKey = VALUE;
                        elseif TAG==1025,
                                HDR.GeoTIFF.GTRasterTypeGeoKey = VALUE;
                        elseif TAG==1026,
                                HDR.GeoTIFF.GTCitationGeoKey = VALUE;
                        elseif TAG==2048,
                                HDR.GeoTIFF.GeographicTypeGeoKey = VALUE;
                        elseif TAG==2049,
                                HDR.GeoTIFF.GeogCitationGeoKey = VALUE;
                        elseif TAG==2050,
                                HDR.GeoTIFF.GeogGeodeticDatumGeoKey = VALUE;
                        elseif TAG==2051,
                                HDR.GeoTIFF.GeogPrimeMeridianGeoKey = VALUE;
                        elseif TAG==2052,
                                HDR.GeoTIFF.GeogLinearUnitsGeoKey = VALUE;
                        elseif TAG==2053,
                                HDR.GeoTIFF.GeogLinearUnitSizeGeoKey = VALUE;
                        elseif TAG==2054,
                                HDR.GeoTIFF.GeogAngularUnitsGeoKey = VALUE;
                        elseif TAG==2055,
                                HDR.GeoTIFF.GeogAngularUnitSizeGeoKey = VALUE;
                        elseif TAG==2061,
                                HDR.GeoTIFF.GeogPrimeMeridianLongGeoKey = VALUE;
                        elseif TAG==2061,
                                HDR.GeoTIFF.GeogPrimeMeridianLongGeoKey = VALUE;
                                
                        elseif TAG==3074,
                                HDR.GeoTIFF.ProjectionGeoKey = VALUE;
                                
                        elseif TAG==33432,
                                HDR.Copyright = char(VALUE);
                                
                        elseif TAG==34016,
                                HDR.TIFF.IT8.Site = VALUE;
                        elseif TAG==34017,
                                HDR.TIFF.IT8.ColorSequence = VALUE;
                        elseif TAG==34018,
                                HDR.TIFF.IT8.Header = VALUE;
                        elseif TAG==34019,
                                HDR.TIFF.IT8.RasterPadding = VALUE;
                        elseif TAG==34020,
                                HDR.TIFF.IT8.BitsPerRunLength = VALUE;
                        elseif TAG==34021,
                                HDR.TIFF.IT8.BitsPerExtendedRunLength = VALUE;
                        elseif TAG==34022,
                                HDR.TIFF.IT8.ColorTable = VALUE;
                        elseif TAG==34023,
                                HDR.TIFF.IT8.ImageColorIndicator = VALUE;
                        elseif TAG==34024,
                                HDR.TIFF.IT8.BKGColorIndicator = VALUE;
                        elseif TAG==34025,
                                HDR.TIFF.IT8.ImageColorValue = VALUE;
                        elseif TAG==34026,
                                HDR.TIFF.IT8.BKGColorValue = VALUE;
                        elseif TAG==34027,
                                HDR.TIFF.IT8.PixelIntensityRange = VALUE;
                        elseif TAG==34028,
                                HDR.TIFF.IT8.TransparencyIndicator = VALUE;
                        elseif TAG==34029,
                                HDR.TIFF.IT8.ColorCharacterization = VALUE;
                        elseif TAG==34030,
                                HDR.TIFF.IT8.t34030 = VALUE;
                                
                        elseif 1, %TAG<2^15,        
                                fprintf(HDR.FILE.stdout,'IOPEN(TIFF): TAG %d %xH used\n',TAG,TAG)
                        else
                        end;	
                        K = K + 1;
                        status = fseek(HDR.FILE.FID,POS+12,'bof');
                end;
                [OFFSET, c] = fread(HDR.FILE.FID, 1, 'uint32');
        end;
        HDR.GDFTYP = ['uint',int2str(HDR.Bits(1))];
        HDR.FILE.OPEN = 1; 

        
elseif strcmp(HDR.TYPE,'IMAGE:XBM'),
	HDR.FILE.FID = fopen(HDR.FileName,'rt','ieee-le');
	K = 0; 
	while K<2
		tmp = fgetl(HDR.FILE.FID);
		if strncmp(tmp,'/*',2)
		elseif strncmp(tmp,'#define ',7)
			K = K+1;
			[t1,t2] = strtok(tmp(8:end));
			if strfind(tmp,'width')
				HDR.IMAGE.Size(2) = str2double(t2);
			elseif strfind(tmp,'height')
				HDR.IMAGE.Size(1) = str2double(t2);
			end
		end;	
	end;
	t = fread(HDR.FILE.FID,[1,inf],'uchar=>uchar');
	fclose(HDR.FILE.FID);
	t(t==10)=' ';	
	t(t==13)=' ';	
	[tmp,t] = strtok(t,'{}');
	[r,t]   = strtok(t,'{}');
	K = 0; 
	N = repmat(NaN,ceil(HDR.IMAGE.Size([2,1])./[8,1]));
	data = repmat(NaN,HDR.IMAGE.Size([2,1]));
	while ~isempty(t)
		[t,r] = strtok(r,' ,');
		n = NaN;
		if strncmp(t,'0x',2)
			n = hex2dec(t(3:end));
		elseif strncmp(t,'0',1)
			n = base2dec(t,8);
		else
			n = str2double(t);
		end;
		if ~isnan(n) & ~isempty(n),
			K = K + 1;
			N(K) = n; 
		end	
	end;
	data = repmat(NaN,HDR.IMAGE.Size([2,1]));
	for k = 1:8,
		ix = k:8:HDR.IMAGE.Size(2);
		data(ix,:) = logical(bitand(N(1:length(ix),:),2^(k-1)));
	end
	data = data';

        
elseif strcmp(HDR.TYPE,'IMAGE:XPM'),
	HDR.FILE.FID = fopen(HDR.FileName,'rt','ieee-le');
		line = '';
		while ~any(line=='{'),
	                line = fgetl(HDR.FILE.FID);
		end;

                line = fgetl(HDR.FILE.FID);
		[s,t]=strtok(line,char(34));
		[tmp,status] = str2double(s);

		code1 = repmat(NaN,tmp(3),1);
		code2 = repmat(0,256,1);
		Palette = repmat(NaN,tmp(3),3);
		HDR.IMAGE.Size = tmp([2,1]);
		k1 = tmp(3);

		for k = 1:k1,
	                line = fgetl(HDR.FILE.FID);
			[s,t]= strtok(line,char(34));
			code1(k) = s(1);
			code2(s(1)+1) = k;
			Palette(k,:) = [hex2dec(s(6:9)),hex2dec(s(10:13)),hex2dec(s(14:17))];
		end;
		Palette = (Palette/2^16);
		R = Palette(:,1);
		G = Palette(:,2);
		B = Palette(:,3);
		HDR.Code1 = code1; 
		HDR.Code2 = code2; 
		HDR.IMAGE.Palette = Palette; 

		data = repmat(NaN,[HDR.IMAGE.Size]);
		for k = 1:HDR.IMAGE.Size(1),
	                line = fgetl(HDR.FILE.FID);
			[s,t]= strtok(line,char(34));
			data(k,:) = abs(s);
		end;
        fclose(HDR.FILE.FID);
	data(:,:,1) = code2(data+1);

	data(:,:,3) = B(data(:,:,1));
	data(:,:,2) = G(data(:,:,1));
	data(:,:,1) = R(data(:,:,1));
        
end;
