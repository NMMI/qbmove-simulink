%% BSD 3-Clause License
%
% Copyright (c) 2015-2018, qbrobotics
% Copyright (c) 2018-2024, Centro di Ricerca "E. Piaggio"
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

wb = waitbar(0, 'Please wait...', 'Name', 'Installing Simulink libraries');
pause(.2);
cd library
waitbar(.33, wb, 'Compiling QB Move library');
make
cd ..

cd library_cp
waitbar(.67, wb, 'Compiling Centro Piaggio library');
make
cd ..


% LIBRARY UPDATE

lib = 'library/';
lib_pac = 'library_pacer/';
lib_cp = 'library_cp/';

%retrieve current matlab version
ver = version('-release');

old_file = strcat('', lib);
old_file = strcat(old_file, 'vers/qbmove_library_');
old_file = strcat(old_file, ver);
old_file = strcat(old_file, '.slx');

new_file = strcat('', lib);
new_file = strcat(new_file, 'qbmove_library.slx');

copyfile(old_file, new_file);

old_file = '';
new_file = '';

% PACER UPDATE

old_file = strcat('', lib_pac);
old_file = strcat(old_file, 'vers/QB_pacer_lib_');
old_file = strcat(old_file, ver);
old_file = strcat(old_file, '.slx');

new_file = strcat('', lib_pac);
new_file = strcat(new_file, 'QB_pacer_lib.slx');

copyfile(old_file, new_file);

old_file = '';
new_file = '';

% CP LIBRARY UPDATE

old_file = strcat('', lib_cp);
old_file = strcat(old_file, 'vers/CP_library_');
old_file = strcat(old_file, ver);
old_file = strcat(old_file, '.slx');

new_file = strcat('', lib_cp);
new_file = strcat(new_file, 'CP_library.slx');

copyfile(old_file, new_file,'f');

old_file = '';
new_file = '';


% Add path to Set Path field (only for libraries)
waitbar(.85, wb, 'Updating Matlab Search Path');
cd library
path(genpath(cd), path);
cd ..
cd library_cp
path(genpath(cd), path);
cd ..
cd library_pacer
path(genpath(cd), path);
cd ..
clc

flag_path = savepath;


if flag_path
    disp('[WARN] Matlab was not able to update search path file.');
    disp('[WARN] Make sure write permissions are granted and try again.');
    c = input(['Press ''q'' to quit. Instead, if you want to use an ' ...
        'alternative path press ''c'' to continue: '], 's');
end
while flag_path
    if (c == 'c')
        while flag_path
    alternative_path = input(['Choose an alternative path for pathdef.m ' ...
        'file (without ending slash): '], 's');
    
            if exist(alternative_path,'dir')
            
                alternative_path = strcat(alternative_path, '/pathdef.m');
        
                flag_path = savepath(alternative_path);
        
                if (flag_path)
                    disp(['[ERROR] Path not correct: ' alternative_path]);
                end
            else
                disp(['[ERROR] This is not a directory: ' alternative_path]);
            end
        end
    elseif (c == 'q')
        disp('[INFO] QBmove and CP libraries NOT installed');
        disp(['[INFO] Search path update failed. Run this script again ' ...
            'with the right permissions.']);
        close(wb);
        return
    else
        c = input('Bad choice. Press ''q'' to quit or ''c'' to continue: ', 's');
    end 
end

waitbar(1, wb, 'Finishing');
pause(.2);
close(wb);
disp('[INFO] QBmove and Centro Piaggio libraries installed successfully.');
disp('[INFO] All the blocksets are now available in the Simulink Library Browser.');
disp('[INFO] Check out the examples folder to know how to use them.');

clear all


