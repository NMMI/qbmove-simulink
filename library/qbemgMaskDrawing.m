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
% image( imread('qbot.jpg') );
%=======================================================     TEMPORARY VARIABLES
% tmp_MaskEnables = { 'on'; 'on'; 'off'; 'on'; 'on'; 'off'; 'off' };
% tmp_MaskVisibilities = { 'on'; 'on'; 'off'; 'on'; 'on'; 'off'; 'off' };
tmp_MaskEnables = get_param( gcb, 'MaskEnables' );              % actual config
tmp_MaskVisibilities = get_param( gcb, 'MaskVisibilities' );   % actual config
%====================================================================     INPUTS
port_label( 'input', 1, 'handle' );

%==================================================================      OUTPUTS

port_label( 'output', 1, 'emg_1' );
port_label( 'output', 2, 'emg_2' );