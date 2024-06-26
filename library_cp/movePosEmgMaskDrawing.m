%% BSD 3-Clause License
%
% Copyright (c) 2019 Centro "E. Piaggio"
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
%-------------------------------------------------------------------------------
% Mask drawing routine
% Sets ports label and parameters visibility according to choices made in the
% dialog box of the sfunction qbot_ISS
%-------------------------------------------------------------------------------
%==========       UNLOCK LIBRARY IF IT CALLED THE SCRIPT (done to avoid warning)
if strcmpi( gcs, 'qb_library_ISS' )
    set_param('qb_library_ISS', 'lock', 'off');
end
%==================================================       BLOCK BACKGROUND IMAGE
image( imread('motor_emg.jpg') );
%=======================================================     TEMPORARY VARIABLES
tmp_A_C_DIRS = get_param( gcb, 'ACTIVE_COMM_DIRECTIONS' );
% tmp_MaskEnables = { 'on'; 'on'; 'off'; 'on'; 'on'; 'off'; 'off' };
% tmp_MaskVisibilities = { 'on'; 'on'; 'off'; 'on'; 'on'; 'off'; 'off' };
tmp_MaskEnables = get_param( gcb, 'MaskEnables' );              % actual config
tmp_MaskVisibilities = get_param( gcb, 'MaskVisibilities' );   % actual config
%====================================================================     INPUTS
port_label( 'input', 1, 'handle' );
if( strcmp( tmp_A_C_DIRS, 'Tx' ) | strcmp( tmp_A_C_DIRS, 'Both' ) )
    if( strcmp( get_param( gcb, 'CONTROL_MODE'), ...
                                'Equilibrium Position and Stiffness Preset' )  )
      port_label( 'input', 2, 'eq.pos.' );
      port_label( 'input', 3, 's.preset' );
    else
        if (strcmp( get_param( gcb, 'CONTROL_MODE'), ...
                                    'Equilibrium Position and Stiffness Preset Percentage' ))
          port_label( 'input', 2, 'eq.pos.' );
          port_label( 'input', 3, 'stiff 0-100%' );

        else
          port_label( 'input', 2, 'pos.1' );
          port_label( 'input', 3, 'pos.2' );
        end
    end
    
    if ( strcmp(get_param( gcb, 'ACTIVATE'), 'off') )
      port_label( 'input', 4, 'activation' );
    end
else
  if ( strcmp(get_param( gcb, 'ACTIVATE'), 'off') )
    port_label( 'input', 2, 'activation' );
  end
end

%==================================================================      OUTPUTS
if( strcmp( tmp_A_C_DIRS, 'Tx' ) | strcmp( tmp_A_C_DIRS, 'None' ) )
  if( strcmp( get_param( gcb, 'DAISY_CHAINING'), 'on') )
    port_label( 'output', 1, 'handle' );
  end
end
if( strcmp( tmp_A_C_DIRS, 'Rx' ) | strcmp( tmp_A_C_DIRS, 'Both' ) )
  port_label( 'output', 1, 'pos.1' );
  port_label( 'output', 2, 'pos.2' );
  port_label( 'output', 3, 'pos.L' );
  port_label( 'output', 4, 'emg_1' );
  port_label( 'output', 5, 'emg_2' );
  port_label( 'output', 6, 'error' );
  if( strcmp( get_param( gcb, 'DAISY_CHAINING'), 'on') )
    port_label( 'output', 7, 'handle' );
  end
end
%===========================================================     MASK PARAMETERS
%     Mask Enables depending on ACTIVE_COMM_DIRECTIONS (DEFAULT is CASE 'Both':)
%     1   ID                        on
%     2   ACTIVE_COMM_DIRECTIONS    on
%     3   CONTROL_MODE              on
%     4   DAISY_CHAINING            off
%     5   MAX_ANG                   on
%     6   SW_LIMIT                  off
%     7   OFFSET                    off
%     8   ACTIVATE                  on
%     9   WDT                       on
%     10  UNITY                     on
% CASE 'Tx' : nothing to do
if( strcmp( tmp_A_C_DIRS, 'Both' ) | strcmp( tmp_A_C_DIRS, 'Tx' ) )
  tmp_MaskEnables{3} = 'on';
  tmp_MaskEnables{6} = 'off';
  tmp_MaskEnables{8} = 'on';
  tmp_MaskEnables{9} = 'on';
  tmp_MaskEnables{10} = 'on';
end

% CASE 'Rx' : set 'off' mask enables for SW_LIMIT and OFFSET
if( strcmp( tmp_A_C_DIRS, 'Rx' ) )
  tmp_MaskEnables{3} = 'off';
  tmp_MaskEnables{6} = 'off';
  tmp_MaskEnables{7} = 'off';
  tmp_MaskEnables{8} = 'on';
  tmp_MaskEnables{9} = 'on';
  tmp_MaskEnables{10} = 'on';
end

% CASE 'None' : set 'off' mask enables for MAX_ANG, SW_LIMIT and OFFSET
if( strcmp( tmp_A_C_DIRS, 'None' ) )
  tmp_MaskEnables{3} = 'off';
  tmp_MaskEnables{5} = 'off';
  tmp_MaskEnables{6} = 'off';
  tmp_MaskEnables{7} = 'off';
  tmp_MaskEnables{8} = 'on';
  tmp_MaskEnables{9} = 'on';
  tmp_MaskEnables{10} = 'off';
end

set_param( gcb, 'MaskEnables', tmp_MaskEnables );
set_param( gcb, 'MaskVisibilities', tmp_MaskVisibilities );
