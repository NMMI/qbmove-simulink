function blkStruct = slblocks

% %retrieve current matlab version
% ver = version('-release');
% 
% if (strcmp(ver, '2013a') | strcmp(ver, '2013b') | strcmp(ver, '2014a') | strcmp(ver, '2014b'))
%     lib_pac = strcat('QB_pacer_lib_', ver);
%     lib_pac = strcat(lib_pac, '.slx');
% %     system([cp lib_pac 'QB_pacer_lib.slx']);
%     pwd
%     copyfile(lib_pac, 'QB_pacer_lib.slx');
% else
%     lib_pac = strcat('QB_pacer_lib_', ver);
%     lib_pac = strcat(lib_pac, '.mdl');
%     copyfile(lib_pac, 'QB_pacer_lib.mdl');
% end


blkStruct.Name = 'QB Pacer';
blkStruct.OpenFcn = 'QB_pacer_lib';
blkStruct.MaskInitialization = '';