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
