# qbmove simulink library

## Installation:

### The compiler

1.  First of all be sure to have installed a C compiler in your system
    (tested with Xcode under Mac OS X and Microsoft Windows SDK for
    Windows 7 under Windows).
2.  Now execute the command "mex -setup" on your Matlab Command Window
    to let Matlab use your compiler.

    If you have trouble in this step, try follow
    [this](http://www.mathworks.it/it/help/matlab/matlab_external/what-you-need-to-build-mex-files.html) link.

### Include the library

1.  In Matlab go to "Set Path", click on "Add with subfolders" and include
    the folder `qbmove_simulink` on your Matlab paths.
2.  Click "Save", then "Close".
3.  Navigate to the `qbmove_simulink` folder and execute `install.m` which is
    a script that simply choose the proper library version based on your Matlab
    version and rename it subtracting the YEAR extension.  
    e.g.  qbmove_library_2013a.slx -> qbmove_library.slx


### Compile the library

The library can be used on varius operating systems such as MacOS X,
Windows, Linux... so you need to recompile it for your system.

#continua qui
1.  Navigate in the folder `library`. 
2.  On the Matlab Command Window type "make" and press return. If no error
    is returned the libraries are correctly compiled.

NOTE: you do not necessary need to recompile the library. Actually it is
precompiled under Windows 7 64bit and MacOS X Maveriks.



Use:

Create a new simulink model
1)  Click on the "Simulink Library" icon or type "Simulink" in the Matlab
    Command Window.
2)  Create a new model using: "File -> New -> Model"
3)  In the new model go to: "Simulation -> Model Configuration Parameters".
    Under "Solver" select as Type "Fixed-Step", as Solver "ode1 (Euler)" or
    "discrete (no continuous states)" and as Fixed-step size type the
    delta_t in seconds. (e.g. if you want to retrieve positions and set new
    inputs every 5 milliseconds, type 0.005). Click "OK".
    NOTE: Every qbvmove needs at least 1 millisecond to read and set new
    positions, so the minimum step time allowed is 1 millisecond multiplied
    by the number of qbmove in the chain.
4)  You should be able to see "QB Pacer" and "Qb Move Library" under
    libraries in Simulink Library Browser. From here drag and drop the
    desired blocks onto your model.

The blocks:

- QB Pacer
    You need one of this block to be in your model, otherwise the
    simulation time will not match the real time. The default value is ok
    so you just have to put it in a corner of your model and it will do the
    rest of the job.

- QB Move
    This block is the interface between your computer and the real qbmove.
    By default you will see 3 input ports and 4 outputs ports. This means
    that the block is set to both receive information from the sensors
    and send new position reference and stiffness to the qbmove.
    If you double-click the block the Function Block Parameters will open
    and you will be able to set the proper ID for your qbmove and the
    Communication Direction. If you set Tx, you will be able to only send
    new reference input to the qbmove. If you set Rx you will be able to
    only read the the position sensors value.
    Either the inputs and the outputs are in degrees.
    NOTE: to use this block you MUST have a "QB Move Init" block connected
    to the first input (handle).

- QB Move Init
    This block is used to provide the right handle to the other blocks
    and allow the communication between the computer and the qbmoves.
    - WINDOWS
        Under windows the QB Move are seen as Communication Ports (COM1,
        COM2, COM#...).
        You need to modify some paramters to allow the communication.
        Click on START and type Device Management.
        Go under "Ports (COM & LPT)", right-click on the COM# associated
        with your QB Move and open "Properties".
        Select the tab "Port Settings" and click on "Advanced...".
        Be sure to set a port number within 1, 2, 3 or 4 and a "Latency"
        of 1 millisecond. Click "OK" and "OK" again.
        Now you can come back to your simulink model, double-click on the
        QB Move Init block and type the port you choose between single
        quote mark (e.g. 'COM1')
        
    - UNIX or MAC
        Under unix you do not need to make any special modification to
        your port.
        To retrieve the name of the port you need to open a Terminal
        window and type "ls /dev/"
        You shoul see a series of interfaces:
         - MAC OS X: the interface is usually called "tty.usbserial-XX"
           where XX is the serial of the QB Move
         - LINUX: the interface is usually called "tty.USB0" or similar
        Now you can come back to your simulink model, double-click on the
        QB Move Init block and type the port name between single quote
        e.g. '/dev/tty.usbserial-13' or '/dev/tty.USB0'


Example:

In the main folder you will find an example called "qbmove_example.mdl"
This is a simple configuration which you can use to test your qbmove.
On the left there are two slider gain which can be tweaked while the
simulation is running and you can see the result on the qbmove.
On the right there are 3 display where you can read the value in degree of
the 3 sensors (1 and 2 are the motor shafts, 3 is the qbmove output shaft).
The ouput error can be used to see if there are errors in communication.
It starts from zero and it is incremented by 1 every time a packet error
receiving occurs.
As you can see in the bottom-left corner there is also the "QB Pacer" block
used to ensure the correct temporization between simulation and real time.
If you want, you can use the output of this block to trace the time of the
simulation.
In this particular configuration, the step size is set to 2 milliseconds
and the computer should be able to run it in real time. This means that
every 2 milliseconds you send a new reference position to the qbmove and a
new value for the current sensor value is sent back from the qbmove to the
computer.
