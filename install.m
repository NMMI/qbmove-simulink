lib = 'library/';
lib_pac = 'library_pacer/';

%retrieve current matlab version
ver = version('-release');


old_file = strcat('', lib);
old_file = strcat(old_file, 'qbmove_library_');
old_file = strcat(old_file, ver);

new_file = strcat('', lib);
new_file = strcat(new_file, 'qbmove_library');

    
if (strcmp(ver, '2013a') | strcmp(ver, '2013b') | strcmp(ver, '2014a') | strcmp(ver, '2014b'))
    old_file = strcat(old_file, '.slx');
    new_file = strcat(new_file, '.slx');
    copyfile(old_file, new_file);
else
    old_file = strcat(old_file, '.mdl');
    new_file = strcat(new_file, '.mdl');
    copyfile(old_file, new_file);
end



old_file = '';
new_file = '';


old_file = strcat('', lib_pac);
old_file = strcat(old_file, 'QB_pacer_lib_');
old_file = strcat(old_file, ver);

new_file = strcat('', lib_pac);
new_file = strcat(new_file, 'QB_pacer_lib');

    
if (strcmp(ver, '2013a') | strcmp(ver, '2013b') | strcmp(ver, '2014a') | strcmp(ver, '2014b'))
    old_file = strcat(old_file, '.slx');
    new_file = strcat(new_file, '.slx');
    copyfile(old_file, new_file);
else
    old_file = strcat(old_file, '.mdl');
    new_file = strcat(new_file, '.mdl');
    copyfile(old_file, new_file);
end