function [val] = read_ini(filename, elem, format, number)

% READ_INI reads a specified element from an *.ini text file
% this is used as EEG header file by ASA and by BrainVision
%
% val = read_ini(filename, element, type, number)
%
% where the element is a string such as
%	NumberSlices
%	NumberPositions
%	Rows
%	Columns
%	etc.
%
% and format specifies the datatype according to
%	%d	(integer value)
%	%f	(floating point value)
%	%s	(string)
%
% number is optional to specify how many lines of data should be read
% The default is 1 for strings and Inf for numbers.

% Copyright (C) 2002, Robert Oostenveld
%
% $Log: not supported by cvs2svn $
% Revision 1.3  2004/03/29 15:15:12  roberto
% unknown change, seems related to handling of empty lines ?
%
% Revision 1.2  2003/03/11 15:24:51  roberto
% updated help and copyrights
%

fid = fopen(filename, 'rt');
if fid==-1
  error(sprintf('could not open file %s', filename));
end

if nargin<4 
  if strcmp(format, '%s')
    number = 1;
  else
    number = Inf;
  end
end


val = [];
elem = deblank2(lower(elem));

while (1)
  line = fgetl(fid);
  if ~isempty(line) & line==-1
    % prematurely reached end of file
    return
  end
  line = deblank2(line);
  lower_line = lower(line);
  if strmatch(elem, lower_line)
    data = line((length(elem)+1):end);
    break
  end
end

while isempty(data)
  line = fgetl(fid);
  if line==-1
    % prematurely reached end of file
    return
  end
  data = deblank2(line);
end

if strcmp(format, '%s')
  if number==1
    % interpret the data as a single string, create char-array
    val = deblank2(data);
    return
  end
  % interpret the data as a single string, create cell-array
  val{1} = deblank2(data);
  count = 1;
  % read the remaining strings
  while count<number
    line = fgetl(fid);
    if ~isempty(line) & line==-1
      return
    end
    tmp = sscanf(line, format);
    if isempty(tmp)
      return
    else
      count = count + 1;
      val{count} = deblank2(line);
    end
  end

else
  % interpret the data as numeric, create numeric array
  count = 1;
  data = sscanf(data, format)';
  if isempty(data)
    return
  else
    val(count,:) = data;
  end
  % read remaining numeric data
  while count<number
    line = fgetl(fid);
    if ~isempty(line) & line==-1
      return
    end
    data = sscanf(line, format)';
    if isempty(data)
      return
    else
      count = count+1;
      val(count,:) = data;
    end
  end
end 

fclose(fid);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [out] = deblank2(in)
out = fliplr(deblank(fliplr(deblank(in))));
