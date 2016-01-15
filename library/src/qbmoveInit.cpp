// ----------------------------------------------------------------------------
// Copyright (C)  qbrobotics. All rights reserved.
// www.qbrobotics.com
// ----------------------------------------------------------------------------
// File:        qbmoveInit.cpp
//
// Description: QB Move initialization
//              S-function
//
//
// ----------------------------------------------------------------------------
// Long Description
//
// This function is part of a set that allows the use of the QB Move usin RS485.
//
// The HANDLE to the virtual COM is opened and then stored in a PWORK
// vector (which ain't a DWORK with type SS_POINTER, that is void).
// (The type HANDLE also ain't void , but a cast is needed with microsoft
// compiler)
// A type COM_HANDLE is registered and used for the output port.
// On output port, the address of the PWORK is put.
//
// Other communication parameters are set too: baudrate, parity, stop bits
// and timeouts of the opened COM; serial interface and speed of
// communication of the bridge (since it supports both I2C and UART with
// a few speed options).
//
// ----------------------------------------------------------------------------
// Copyright (C)  qbrobotics. All rights reserved.
// ----------------------------------------------------------------------------
// Permission is granted to copy, modify and redistribute this file, provided
// header message is retained.
// ----------------------------------------------------------------------------


//==============================================================================
//                                                             main  definitions
//==============================================================================

#define S_FUNCTION_NAME qbmoveInit
#define S_FUNCTION_LEVEL 2

//==============================================================================
//                                                                      includes
//==============================================================================

#include "definitions.h"
#include "simstruc.h"
#include "../../../qbAPI/src/qbmove_communications.h"

// #include <windows.h>

#if !(defined(_WIN32) || defined(_WIN64))
    #include <unistd.h>
#endif

//==============================================================================
//                                                                   definitions
//==============================================================================

//===============================================================     baud rate

#if (defined(_WIN32) || defined(_WIN64))
    #define BAUD_RATE_57600     CBR_57600
    #define BAUD_RATE_115200    CBR_115200          
    #define BAUD_RATE_460800    460800              
    #define BAUD_RATE_2000000   2000000             ///< Virtual COM baud rate - WINDOWS
#elif (defined(__APPLE__))
    #define BAUD_RATE_57600     57600
    #define BAUD_RATE_115200    115200              
    #define BAUD_RATE_460800    460800              
    #define BAUD_RATE_2000000   2000000             ///< Virtual COM baud rate
#else
    #define BAUD_RATE_57600     B57600
    #define BAUD_RATE_115200    B115200
    #define BAUD_RATE_460800    B460800              
    #define BAUD_RATE_2000000   B2000000            ///< Virtual COM baud rate - UNIX
#endif

//===============================================================     parameters

#define param_com_port(i)   (mxGetPr( ssGetSFcnParam( S, 0 ) )[i])
#define param_com_baudrate  (int)*mxGetPr(ssGetSFcnParam(S, 1))

//=============================================================     work vectors

#define pwork_handle    (*(HANDLE *)ssGetPWork(S))

//===================================================================     ouputs

#define out_handle  (*(HANDLE **)ssGetOutputPortSignal(S,0))

//==============================================================================
//                                           Configuration and execution methods
//==============================================================================

static void mdlInitializeSizes(SimStruct *S)
{
    int_T   status;                                 // for new type definition
    DTypeId COM_HANDLE_id;                          // for new type definition
    HANDLE  handle_aux;                             // for new type definition

//======================================================     new type definition

    COM_HANDLE_id = ssRegisterDataType(S, "COM_HANDLE");
    if(COM_HANDLE_id == INVALID_DTYPE_ID) return;
    status = ssSetDataTypeSize(S, COM_HANDLE_id, sizeof(handle_aux));
    if(status == 0) return;
    status = ssSetDataTypeZero(S, COM_HANDLE_id, &handle_aux);
    if(status == 0) return;

//===============================================================     parameters

    ssSetNumSFcnParams(S, 2);                           // 2 parameters:
                                                        //      - COM port
                                                        //      - baudrate
    //Parameter mismatch will be reported by Simulink
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))
        return;

//===================================================================     inputs

   if (!ssSetNumInputPorts(S, 0)) return;               // 0 inputs

//==================================================================     outputs

    if (!ssSetNumOutputPorts(S, 1)) return;             // 1 outputs:
                                                        //      - COM handle

//////////////////////////////   com handle   //////////////////////////////
    ssSetOutputPortWidth(S,0,1);                        // input 1 width
    ssSetOutputPortDataType(S, 0, COM_HANDLE_id);       // input 1 datatype


//=============================================================     sample times

    ssSetNumSampleTimes(  S, 1);                // number of sample times

//=============================================================     work vectors

    ssSetNumRWork(S, 0);                        // 0 real work vector elements
    ssSetNumIWork(S, 0);                        // 0 work vector elements
    ssSetNumPWork(S, 1);                        // 1 pwork vector elements:
                                                //      - COM handle
    ssSetNumModes(S, 0);                        // 0  mode work vector elements
    ssSetNumNonsampledZCs(S, 0);                // 0  nonsampled zero crossings
}

//==============================================================================
//                                                      mdlInitializeSampleTimes
//==============================================================================
// TODO: DESCRIPTION
//==============================================================================

static void mdlInitializeSampleTimes(SimStruct *S)
{
    // Register one pair for each sample time
    ssSetSampleTime(S, 0, INHERITED_SAMPLE_TIME);
    ssSetOffsetTime(S, 0, 0.0);

}

//==============================================================================
//                                                                     MDL_START
//==============================================================================
// This function is called once at start of model execution.
//==============================================================================

#define MDL_START                         // Change to #undef to remove function
#if defined(MDL_START)
static void mdlStart(SimStruct *S)
{
    char    string_aux[255];                // auxiliar string
    char    serial_port_path[255];          // auxiliar string
    int baud_rate;
    comm_settings comm_settings_t;

    ssPrintf("qbmove simulink library version: %s\n", QBMOVE_SIMULINK_VERSION);

    //======================================================     opening serial port

    for (int i = 0; param_com_port(i); ++i)
        serial_port_path[i] = (char) param_com_port(i);    

    switch(param_com_baudrate){
        case 1:
            baud_rate = BAUD_RATE_2000000;
            break;
        case 2:
            baud_rate = BAUD_RATE_460800;
            break;
        case 3:
            baud_rate = BAUD_RATE_115200;
            break;
        case 4:
            baud_rate = BAUD_RATE_57600;
            break;
    }

    openRS485(&comm_settings_t, serial_port_path, baud_rate);
    
    pwork_handle = comm_settings_t.file_handle;

    #if defined(_WIN32) || defined(_WIN64)
        if(pwork_handle == INVALID_HANDLE_VALUE)
    #else
        if(pwork_handle == -1)
    #endif
    {
        ssPrintf("Check your COM port. \nCould not connect to %s\n", serial_port_path);
        out_handle = &pwork_handle;

        // Stop simulation
        mexEvalString("set_param(bdroot, 'SimulationCommand', 'stop')");

        return;
    }

    out_handle = &pwork_handle;
}
#endif /*  MDL_START */

//==============================================================================
//                                                                    mdlOutputs
//==============================================================================
// TODO: In this function, you compute the outputs of your S-function
// block. Generally outputs are placed in the output vector(s),
// ssGetOutputPortSignal.
//==============================================================================

static void mdlOutputs(SimStruct *S, int_T tid)
{
    out_handle = &pwork_handle;
}

//==============================================================================
//                                                                  mdlTerminate
//==============================================================================
// Actions that are necessary at the termination of a simulation.
//==============================================================================

static void mdlTerminate(SimStruct *S)
{

}

//==============================================================================
//                                                   Required S-function trailer
//==============================================================================

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif