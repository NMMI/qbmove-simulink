%% BSD 3-Clause License
%
% Copyright (c) 2017, qbrobotics
% All rights reserved.
%
% Redistribution and use in source and binary forms, with or without
% modification, are permitted provided that the following conditions are met:
%
% * Redistributions of source code must retain the above copyright notice, this
%  list of conditions and the following disclaimer.
%
% * Redistributions in binary form must reproduce the above copyright notice,
%  this list of conditions and the following disclaimer in the documentation
%  and/or other materials provided with the distribution.
%
% * Neither the name of the copyright holder nor the names of its
%  contributors may be used to endorse or promote products derived from
%  this software without specific prior written permission.
%
% THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
% AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
% IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
% DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
% FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
% DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
% SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
% CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
% OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
% OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


%%                                                            	    INSTALL
% This code install qbmove and qbpacer library in your system and update
% 'path' variable with necessary paths 

clear all
clc
% Install library

cd library
make
cd ..

% Load last version available
load('debug_tools/last_version_release');
last_version_flag = false;

% LIBRARY UPDATE

lib = 'library/';
lib_pac = 'library_pacer/';

%retrieve current matlab version
ver = version('-release');

old_file = strcat('', lib);
old_file = strcat(old_file, 'vers/qbmove_library_');
old_file = strcat(old_file, ver);

new_file = strcat('', lib);
new_file = strcat(new_file, 'qbmove_library');

if ( (str2double(ver(1:end-1)) >= 2013) || ( str2double(ver(1:end-1)) == 2012 && (ver(end) == 'b') ) )
    old_file = strcat(old_file, '.slx');
    new_file = strcat(new_file, '.slx');
    
    if ~exist(old_file, 'file')
        last_version_flag = true;
        old_file = strcat('library/vers/qbmove_library_', num2str(last_version));
        old_file = strcat(old_file, last_release);
        old_file = strcat(old_file, '.slx');
    end    
else
    old_file = strcat(old_file, '.mdl');
    new_file = strcat(new_file, '.mdl');
end

copyfile(old_file, new_file);

old_file = '';
new_file = '';

% PACER UPDATE

old_file = strcat('', lib_pac);
old_file = strcat(old_file, 'vers/QB_pacer_lib_');
old_file = strcat(old_file, ver);

new_file = strcat('', lib_pac);
new_file = strcat(new_file, 'QB_pacer_lib');

    
if ( (str2double(ver(1:end-1)) >= 2013) || ( str2double(ver(1:end-1)) == 2012 && (ver(end) == 'b') ) )
    old_file = strcat(old_file, '.slx');
    new_file = strcat(new_file, '.slx');
    
    if ~exist(old_file, 'file')
        last_version_flag = true;
        old_file = strcat('library_pacer/vers/QB_pacer_lib_', num2str(last_version));
        old_file = strcat(old_file, last_release);
        old_file = strcat(old_file, '.slx');
    end
else
    old_file = strcat(old_file, '.mdl');
    new_file = strcat(new_file, '.mdl');
end

copyfile(old_file, new_file);

% Add path to Set Path field
path(genpath(cd), path);

clc

flag_path = savepath;

while flag_path
    alternative_path = input('Chose an alternative path for pathdef.m file (without end slash): ', 's');
    
    if exist(alternative_path,'dir')
    
        alternative_path = strcat(alternative_path, '/pathdef.m');

        flag_path = savepath(alternative_path);

        if (flag_path)
            disp(['[ERROR] Path not correct. ' alaternative_path]);
        end
    else
        disp(['[ERROR] This is not a directory. ' alaternative_path]);
    end
end



if last_version_flag
    display('[WARNING] MATLAB version is not supported, last version available set.')
end

display('[INFO] QBmove library install successful');

clear all


