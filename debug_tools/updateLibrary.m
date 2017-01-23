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

%                                                             UPDATELIBRARY
% This code update qbmove and qbpacer library to be backward compatibility 
% with old matlab versions

% USE THIS ONLY IF YOU KNOW WHAT YOU ARE DOING, QBMOVE DEVELOPERS ONLY 

close all
clear all
clc

cd ..

% Eliminate old versions file

if exist([pwd '/library/vers'], 'dir')
    rmdir([pwd '/library/vers'], 's');
end

if exist([pwd '/library_pacer/vers'], 'dir')
    rmdir([pwd '/library_pacer/vers'], 's');
end

% Create version folders

mkdir([pwd '/library/vers']);

mkdir([pwd '/library_pacer/vers']);

% QB Library

folderName = 'library/';
libName = 'qbmove_library';

ver = version('-release');
release = ver(end);
ver = str2double(ver(1:end-1));

last_version = ver;
last_release = release;

save('debug_tools/last_version_release', 'last_version', 'last_release');

open_system(strcat(folderName, libName), 'loadonly');

% Save library for current Version

qb_vers = strcat('qbmove_library_', num2str(ver));
qb_vers = strcat(qb_vers, release);

if ((ver >= 2013))
    qb_vers = strcat(qb_vers, '.slx');
else
    qb_vers = strcat(qb_vers, '.mdl');
end

copyfile(strcat(folderName, strcat(libName,qb_vers(end-3:end))), strcat(folderName, strcat('vers/', qb_vers)));

% Save library for previous Version

while (ver >= 2010)
    if (strcmp(release, 'a'))
        ver = ver - 1;
        release = 'b';
    else
        release = 'a';
    end
    
    qb_vers = strcat('qbmove_library_', num2str(ver));
    qb_vers = strcat(qb_vers, release);
    
    if ((ver >= 2013))
        qb_vers = strcat(qb_vers, '.slx');
    end
    
    save_system(strcat(folderName, libName), strcat(folderName, strcat('vers/', qb_vers)), 'ExportToVersion', strcat('R', strcat(num2str(ver), upper(release))));
   
end


% QB Pacer

folderName = 'library_pacer/';
libName = 'QB_pacer_lib';

ver = version('-release');
release = ver(end);
ver = str2double(ver(1:end-1));

open_system(strcat(folderName, libName), 'loadonly');

% Save library for current Version

qb_vers = strcat('QB_pacer_lib_', num2str(ver));
qb_vers = strcat(qb_vers, release);

if ((ver >= 2013))
    qb_vers = strcat(qb_vers, '.slx');
else
    qb_vers = strcat(qb_vers, '.mdl');
end

copyfile(strcat(folderName, strcat(libName,qb_vers(end-3:end))), strcat(folderName, strcat('vers/', qb_vers)));

% Save library for previous Version

while (ver >= 2010)
    if (strcmp(release, 'a'))
        ver = ver - 1;
        release = 'b';
    else
        release = 'a';
    end
    
    qb_vers = strcat('QB_pacer_lib_', num2str(ver));
    qb_vers = strcat(qb_vers, release);
    
    if ((ver >= 2013))
        qb_vers = strcat(qb_vers, '.slx');
    end
    
    save_system(strcat(folderName, libName), strcat(folderName, strcat('vers/', qb_vers)), 'ExportToVersion', strcat('R', strcat(num2str(ver), upper(release))));
   
end


close all
clear all
clc
display('[INFO] QBmove library version update successful');
