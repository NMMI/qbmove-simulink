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
function msfun_realtime_pacer(block)
% Help for Writing Level-2 M-File S-Functions:
%   web([docroot '/toolbox/simulink/sfg/f7-67622.html']
%   http://www.mathworks.com/access/helpdesk/help/toolbox/simulink/sfg/f7-67622.html

% Copyright 2009, The MathWorks, Inc.

% instance variables 
mySimTimePerRealTime = 1;
myRealTimeBaseline = 0;
mySimulationTimeBaseline = 0;
myResetBaseline = true;
myTotalBurnedTime = 0;
myNumUpdates = 0;

myRealTimeBaseline = 0;

setup(block);

%% ---------------------------------------------------
    function setup(block)
        % Register the number of ports.
        block.NumInputPorts  = 0;
        block.NumOutputPorts = 1

        block.SetPreCompOutPortInfoToDynamic;
        block.OutputPort(1).Dimensions  = 1;
        block.OutputPort(1).SamplingMode = 'sample';
        
        % Set up the states
        block.NumContStates = 0;
        block.NumDworks = 0;
        
        % Register the parameters.
        block.NumDialogPrms     = 1; % scale factor
        block.DialogPrmsTunable = {'Nontunable'};
        
        % Block is fixed in minor time step, i.e., it is only executed on major
        % time steps. With a fixed-step solver, the block runs at the fastest
        % discrete rate.
        block.SampleTimes = [0 1];
        
        block.SetAccelRunOnTLC(false); % run block in interpreted mode even w/ Acceleration
        
        % methods called during update diagram/compilation.
        block.RegBlockMethod('CheckParameters', @CheckPrms);
                
        % methods called at run-time
        block.RegBlockMethod('Start', @Start);
        block.RegBlockMethod('Update', @Update);
        block.RegBlockMethod('Outputs', @Output);
        block.RegBlockMethod('SimStatusChange', @SimStatusChange);
        block.RegBlockMethod('Terminate', @Terminate);
    end

%%
    function CheckPrms(block)
        try
            validateattributes(block.DialogPrm(1).Data, {'double'},{'real', 'scalar', '>', 0});
        catch %#ok<CTCH>
            throw(MSLException(block.BlockHandle, ...
                'Simulink:Parameters:BlkParamUndefined', ...
                'Enter a number greater than 0'));
        end        
    end
        
%%        
    function Start(block) 
        mySimTimePerRealTime = block.DialogPrm(1).Data;
        myTotalBurnedTime = 0;
        myNumUpdates = 0;
        myResetBaseline = true;
        myRealTimeBaseline = tic;

        if strcmp(pause('query'),'off')
            fprintf('%s: Enabling MATLAB PAUSE command\n', getfullname(block.BlockHandle));            
            pause('on');            
        end            
    end

    function Output(block)
         block.OutputPort(1).Data = toc(myRealTimeBaseline) + block.SampleTime(1);
    end

        
%%        
    function Update(block)        
        if  myResetBaseline 
            myRealTimeBaseline = tic;  
            mySimulationTimeBaseline = block.CurrentTime;  
            myResetBaseline = false; 
        else
            if isinf(mySimTimePerRealTime)
                return;
            end            

			should_wait = true;
			waited		= false;
			while(should_wait)				
				should_wait = false;	
	            elapsedRealTime = toc(myRealTimeBaseline);
	            differenceInSeconds = ((block.CurrentTime - mySimulationTimeBaseline) / mySimTimePerRealTime) - elapsedRealTime;
	            % differenceInSeconds = block.CurrentTime - elapsedRealTime;
	            if differenceInSeconds >= 0
	                % pause(differenceInSeconds);
					should_wait = true;		
				else 
					differenceInSeconds;
				end
			end
			if(waited)
				myTotalBurnedTime = myTotalBurnedTime + differenceInSeconds;
                myNumUpdates = myNumUpdates + 1;                
			end
        end            
    end
        
%%        
    function SimStatusChange(block, status)        
        if status == 0, 
            % simulation paused
            fprintf('%s: Pausing real time execution of the model (simulation time = %g sec)\n', ...
                getfullname(block.BlockHandle), block.CurrentTime);
        elseif status == 1
            % Simulation resumed
            fprintf('%s: Continuing real time execution of the model\n', ...
                getfullname(block.BlockHandle));
            myResetBaseline = true; 
        end        
    end
        
%%
    function Terminate(block) 
        if myNumUpdates > 0
            fprintf('%s: Average idle real time per major time step = %g sec\n', ...
                getfullname(block.BlockHandle),  myTotalBurnedTime / myNumUpdates);
        end
    end

end

