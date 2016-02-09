%                                                             	    INSTALL
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


