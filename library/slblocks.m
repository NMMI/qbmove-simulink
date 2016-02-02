function blkStruct = slblocks

% %retrieve current matlab version
% ver = version('-release');
% 
% if (strcmp(ver, '2013a') | strcmp(ver, '2013b') | strcmp(ver, '2014a') | strcmp(ver, '2014b'))
%     old_file = strcat('qbmove_library_', ver);
%     old_file = strcat(old_file, '.slx');
%     disp(old_file);
%     copyfile(old_file, 'qbmove_library.slx');
% else
%     old_file = strcat('qbmove_library_', ver);
%     old_file = strcat(old_file, '.mdl');
%     copyfile(old_file, 'qbmove_library.mdl');
% end
    Browser.Library = 'qbmove_library';
    Browser.Name = 'Qb Move Library';
    
    blkStruct.Browser = Browser;
end
% 
% blkStruct.Name = 'Qb Move Library';
% blkStruct.OpenFcn = 'qbmove_library';
% blkStruct.MaskInitialization = '';
