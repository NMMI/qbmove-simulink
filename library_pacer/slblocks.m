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

%%
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