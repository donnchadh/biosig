function varargout = sviewer_channel(varargin)
% SVIEWER_CHANNEL
% Select HELP in the Info menu 
%
% Version 1.0, November 2004
% Copyright by (C) Franz Einspieler <znarfi5@hotmail.com> and
%                  Alois Schloegl   <a.schloegl@ieee.org>
% University of Technology Graz, Austria
%
% This is part of the BIOSIG-toolbox http://biosig.sf.net/
% Comments or suggestions may be sent to the author.
% This Software is subject to the GNU public license.

% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% Version 2 of the License, or (at your option) any later version.
%
% This library is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% Library General Public License for more details.
%
% You should have received a copy of the GNU Library General Public
% License along with this library; if not, write to the
% Free Software Foundation, Inc., 59 Temple Place - Suite 330,
% Boston, MA  02111-1307, USA.

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @sviewer_channel_OpeningFcn, ...
                   'gui_OutputFcn',  @sviewer_channel_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin & isstr(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% --- Executes just before sviewer_channel is made visible.
function sviewer_channel_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to sviewer_channel (see VARARGIN)

% Choose default command line output for sviewer_channel
handles.output = hObject;
set(gcf,'Color',[0.949,0.949,1]);
% Update handles structure
guidata(hObject, handles);

Data=get(findobj('Tag','sviewer'),'UserData');
if isempty(Data)
    return;
else
    Data = get(findobj('Tag', 'sviewer'), 'UserData');
    set(findobj('Tag', 'listbox_selected'), 'String', Data.Channel(:,1));
    pos = Data.Channel(:,2);
    len_pos = length(pos);
    allChannel = Data.allChannel;
    for i= 1:len_pos
        pos_chn(i) = pos{i};
    end
    allChannel(pos_chn') = [];
    set(findobj('Tag', 'listbox_file'), 'String', allChannel);
    set(findobj('Tag','sviewer'),'UserData',Data);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Executes on selection change in listbox_file.
function listbox_file_Callback(hObject, eventdata, handles)
% hObject    handle to listbox_file (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

index_selected = get(handles.listbox_file,'Value');
Channel = get(findobj('Tag', 'sviewer_channel'), 'UserData');
Channel.Selected = index_selected;
set(findobj('Tag','sviewer_channel'),'UserData',Channel);
% if double click
if strcmp(get(findobj('Tag','sviewer_channel'),'SelectionType'),'open')
    fill_listbox('listbox_file','listbox_selected');
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Executes on button press in button_add.
function button_add_Callback(hObject, eventdata, handles)
% hObject    handle to button_add (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

fill_listbox('listbox_file','listbox_selected');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Executes on button press in button_addall.
function button_addall_Callback(hObject, eventdata, handles)
% hObject    handle to button_addall (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

Data = get(findobj('Tag', 'sviewer'), 'UserData');
if isempty(Data)
    close;
    errordlg('SViewer is not open!', 'Error');
    return;
end

set(findobj('Tag', 'listbox_selected'), 'String', Data.allChannel);
set(findobj('Tag','listbox_file'),'String',[]);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Executes on button press in button_remove.
function button_remove_Callback(hObject, eventdata, handles)
% hObject    handle to button_remove (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

fill_listbox('listbox_selected','listbox_file');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Executes on button press in button_removeall.
function button_removeall_Callback(hObject, eventdata, handles)
% hObject    handle to button_removeall (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

Data = get(findobj('Tag', 'sviewer'), 'UserData');
if isempty(Data)
    close;
    errordlg('SViewer is not open!', 'Error');
    return;
end

set(findobj('Tag', 'listbox_selected'), 'String', []);
set(findobj('Tag','listbox_file'),'String',Data.allChannel);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% fill listboxes
function fill_listbox(listb1,listbox2)

index_selected = get(findobj('Tag',listb1),'Value');

Data = get(findobj('Tag', 'sviewer'), 'UserData');
if isempty(Data)
    close;
    errordlg('SViewer is not open!', 'Error');
    return;
end

Channel = get(findobj('Tag', 'sviewer_channel'), 'UserData');
sel_channel = get(findobj('Tag',listb1),'String');

if isempty(sel_channel)
    return;
end

numb_sel = size(index_selected);
b=1;
for i=1:numb_sel(2)
    addnewchannels(b,:) = sel_channel(index_selected(i),:);
    b=b+1;
end
a=0;
for i= 1:numb_sel(2)
    sel_channel(index_selected(i)-a,:)=[];
    a=a+1;
end

set(findobj('Tag',listb1),'Value',1);
set(findobj('Tag',listb1),'String',sel_channel);
added_channel = get(findobj('Tag',listbox2),'String');
addchannels = [added_channel;addnewchannels];
set(findobj('Tag',listbox2),'String',addchannels);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Executes on button press in button_OK.
function button_OK_Callback(hObject, eventdata, handles)
% hObject    handle to button_OK (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

Data = get(findobj('Tag', 'sviewer'), 'UserData');
if isempty(Data)
    close;
    errordlg('SViewer is not open!', 'Error');
    return;
end

channel = get(findobj('Tag','listbox_selected'),'String');
if isempty(channel)
    close;
    return;
end
Data.Channel = channel;
Data.changeChannel = 'yes';
set(findobj('Tag', 'Slider1'), 'Value',0);
set(findobj('Tag','sviewer'),'UserData',Data);
close;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Executes on button press in button_cancel.
function button_cancel_Callback(hObject, eventdata, handles)
% hObject    handle to button_cancel (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
Data = get(findobj('Tag', 'sviewer'), 'UserData');
Data.changeChannel = 'no';
set(findobj('Tag','sviewer'),'UserData',Data);
close;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Outputs from this function are returned to the command line.
function varargout = sviewer_channel_OutputFcn(hObject, eventdata, handles)
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

varargout{1} = handles.output;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Executes during object creation, after setting all properties.
function listbox_file_CreateFcn(hObject, eventdata, handles)
% hObject    handle to listbox_file (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Executes on selection change in listbox_selected.
function listbox_selected_Callback(hObject, eventdata, handles)
% hObject    handle to listbox_selected (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if strcmp(get(findobj('Tag','sviewer_channel'),'SelectionType'),'open')
    fill_listbox('listbox_selected','listbox_file');
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Executes during object creation, after setting all properties.
function listbox_selected_CreateFcn(hObject, eventdata, handles)
% hObject    handle to listbox_selected (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%