# qbmove simulink library

### IMPORTANT: Versioning:
Each of the repositories in the following table has a version number
composed of 3 numbers in the form: **v x.y.z**

|  Tools  [T]    |  Libraries [L] |  Firmware [F]            |
|-----------------|---------------|--------------------------|
| qbmove simulink | qbAPI         | qbmove firmware          |
| qbmoveadmin     |               | qbmove advanced firmware |
| handmoveadmin   |               | hand firmware micro      |

E.g. **xT** is the **x** number of the Tools whereas **yF** is the **y** number of the Firmware version.

- Every change in the number **z** means a change in the respective repo which not implies changes in other repos.
- Every change in the number **y** in a repo is backward compatible reading the table from right to left. This means that
if you have some new feature in a Firmware, you can still use old Libraries and Tools for management, but of course
you will not be able to use the new features. In this case the rule is **yF >= yL >= yT**.
- Every change in the number **x** means a change which is not backward compatible, hence you will need to update
everything to use it. In this case the rule is **xF = xL = xT**.

Summarising
- **z** independent
- **yF >= yL >= yT**
- **xF = xL = xT**

E.g.

| Tools              | Libraries    | Firmware               | Compatible |
|--------------------|--------------|------------------------|------------|
| qbmoveadmin v4.2.3 | qbAPI v4.5.0 | qbmove firmware v4.6.7 | YES        |
| qbmoveadmin v3.2.3 | qbAPI v4.5.0 | qbmove firmware v4.6.7 | NO         |
| qbmoveadmin v4.2.3 | qbAPI v4.5.7 | qbmove firmware v4.5.0 | YES        |


## Installation:

### Downloads

To be able to correctly use these libraries, you will need to download the current APIs
You can find them [here](https://github.com/qbrobotics/qbAPI).
You will need to just download the qbAPI folder and save it side by side with the qbmove_simulink
folder. Before compiling be sure to have a folder tree like that (be sure to remove the `-master` suffix):

* working_directory
    * qbAPI
    * qbmove_simuilnk


> NOTE: if it is the first time you use the qbmove units on your computer, it is possible that
> you need to download the drivers to let your computer see the port correctly.
> To do that visit [FTDI driver](http://www.ftdichip.com/Drivers/VCP.htm) webpage -> VCP drivers
> and download the proper driver depending on your OS

### The compiler

1.  First of all be sure to have a C compiler installed in your system
    (tested with Xcode under Mac OS X and Microsoft Windows SDK for
    Windows 7 under Windows).
2.  Now execute the command "mex -setup" on your Matlab Command Window
    to let Matlab use your compiler.

    If you have trouble in this step, try follow
    [this](http://www.mathworks.it/it/help/matlab/matlab_external/what-you-need-to-build-mex-files.html)
    link.

### Compile and Include the library  

The library can be used on various operating systems such as MacOS X,
Windows, Linux... but you need to recompile it for your system.

[WARNING:   Matlab version supported are from 2014a up to 2015b
            Matlab bug advised, as soon as possible problem will be fixed
            Previous Matlab versions can download: https://github.com/qbrobotics/qbmove_simulink/commit/9cd93e137cd21c2b56a822ea5e8a742032f01692
            ]

1.  Move your position in `qbmove_simulink`,type install and press return. 
    It will install the library depending on your MATLAB configuration and 
    set necessary path.
    If no error is returned the libraries are correctly compiled and it is
    displayed a successful comment on shell.

## Use

### Create a new simulink model

1.  Click on the "Simulink Library" icon or type "simulink" in the Matlab
    Command Window.
2.  Create a new model using: "File -> New -> Model"
3.  In the new model go to: "Simulation -> Model Configuration Parameters".
    Under "Solver" select as *Type* "Fixed-Step", as *Solver* "ode1 (Euler)" or
    "discrete (no continuous states)" and as *Fixed-step size* type the
    delta_t in seconds. (e.g. if you want to retrieve positions and set new
    inputs every 5 milliseconds, type 0.005). Click "OK".

    > Every qbvmove needs at least 1 millisecond to read and set new
    > positions, so the minimum step time allowed is 1 millisecond multiplied
    > by the number of qbmoves in the chain.

4.  You should be able to see "QB Pacer" and "Qb Move Library" under
    libraries in Simulink Library Browser. From here drag and drop the
    desired blocks onto your model.

### The blocks

- QB Pacer
    You need one of this block to be in your model, otherwise the
    simulation time will not match the *real time*.
    You can set a number representing the **simulation time** **real time**
    ratio. Leave it set to 1 for normal use.

- QB Move

    This block is the interface between your computer and the real qbmove.
    By default you will see 3 input ports and 4 outputs ports. This means
    that the block is set to both receive information from the sensors
    and send new position reference and stiffness to the qbmove.
    If you double-click to the block, the Function Block Parameters will open
    and you will be able to set the proper ID for your qbmove and the
    Communication Direction. If you set Tx, you will be able to only send
    new reference input to the qbmove. If you set Rx you will be able to
    only read the the position sensors value.
    Either the inputs and the outputs are in degrees.

    > To use this block you MUST have a "QB Move Init" block connected
    > to the first input `handle`.

    > To use multiple qbmoves, just put an array containing the IDs in the ID
    > field and use a `mux` to send multiple inputs to the *eq. pos* and
    > *s. preset* ports.

    > ID 0 is broadcast ID so if you want to move a qbmove you do not know the
    > current ID, just use 0

- QB Move Init

    This block is used to provide the right handle to the other blocks
    and allows the communication between the computer and the qbmoves.

    + WINDOWS

        Under windows the qbmoves are seen as Communication Ports (COM1,
        COM2, COM#...).

        > If your qbmove is not automatically recognized by windows, try to
        > manually install the FTDI drivers from [here](http://www.ftdichip.com/Drivers/VCP.htm)

        You need to modify the port number to allow simulink using it.
        Click on START and type Device Manager.
        Go under `Ports (COM & LPT)`, right-click on the COM# associated
        with your qbmove and open `Properties`.
        Select the tab `Port Settings` and click on `Advanced...`.
        Be sure to set a port number within 1, 2, 3 or 4.
        Click `OK` and `OK` again.
        Now you can come back to your simulink model, double-click on the
        QB Move Init block and type the port you choose between single
        quote mark (e.g. 'COM1').

    + UNIX or MAC

        Under unix you do not need to make any special modification to
        your port.

        > If you use MacOSX, you probably will need to install the FTDI driver
        > following [this](http://www.ftdichip.com/Drivers/VCP.htm) link.

        To retrieve the name of the port you need to open a Terminal
        window and type `ls /dev/`
        You should see a series of interfaces:
        - MAC OS X: the interface is usually called "tty.usbserial-XX"
           where XX is the serial of the qbmove.
        - LINUX: the interface is usually called "tty.USB0" or similar

        Now you can come back to your simulink model, double-click on the
        QB Move Init block and type the port name between single quote
        e.g. '/dev/tty.usbserial-13' or '/dev/tty.USB0'


Example:

In the main folder you will find an example called "qbmove_example.slx"
This is a simple configuration which you can use to test your qbmove.
On the left there are two slider gain which can be tweaked while the
simulation is running and you can see the result on the qbmove.
On the right there are 3 displays where you can read the values in degree of
the 3 sensors (1 and 2 are the motor shafts, 3 is the qbmove output shaft).
The output error can be used to see if there are errors in communication.
It starts from zero and it is incremented by 1 every time a communication error
occurs.
As you can see in the bottom-left corner there is also the "QB Pacer" block
used to ensure the correct synchronization between simulation and real time.
If you want, you can use the output of this block to trace the time of the
simulation.

> If you take a look at the scope you can observe two lines. One is the
> simulation clock and the other one is the real time pacer output.
> If they overlap, the sample time of the simulation is properly chosen,
> this means that the step size is big enough to let the communication
> finish between two consecutive steps. If the two lines diverge, you have
> to use a bigger step size.

In this particular configuration, the step size is set to 2 milliseconds
and the computer should be able to run it in real time. This means that
every 2 milliseconds you send a new reference position to the qbmove and
the current position is read. Furthermore a current reading is done
and you can see the milliampere absorbed by each of the two motors.

### qbmove_id_utils

In the library folder you will find a file called qbmove_id_utils.m
By executing this script, a simple GUI will open and you will be able to
set and get the ID of your qbmove.
Before executing the GUI, connect one single qbmove to your computer with the
USB cable. The port should be automatically retrieved by the program and you
will be able to select it using the dropdown menu.
Click get to read the current qbmove ID.
Insert a new ID and click set if you want to change it.
