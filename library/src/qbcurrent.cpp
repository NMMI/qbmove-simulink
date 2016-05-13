// -----------------------------------------------------------------------------
// Copyright (C)  qbrobotics. All rights reserved.
// www.qbrobotics.com
// -----------------------------------------------------------------------------
// File:        qbotISS_ISS.cpp
//
// Description: Communication s-function for obtaining currents of the QB Move.
//              To be used with RS-485 on a Virtual COM.
//------------------------------------------------------------------------------


//==============================================================================
//                                                             main  definitions
//==============================================================================

#define S_FUNCTION_NAME  qbcurrent
#define S_FUNCTION_LEVEL 2

//==============================================================================
//                                                                      includes
//==============================================================================

#include "definitions.h"
#include "simstruc.h"
#include "../../../qbAPI/src/qbmove_communications.h"
// #include <windows.h>

//==============================================================================
//                                                                   definitions
//==============================================================================

//===============================================================     parameters

#define params_qbot_id(i)     ( mxGetPr( ssGetSFcnParam( S, 0 ) )[ \
                                     i >= NUM_OF_QBOTS ? NUM_OF_QBOTS -1 : i ] )
#define params_daisy_chaining  ( (bool)mxGetScalar( ssGetSFcnParam( S, 1 ) ) )

//===================================================================     inputs

#define in_handle ( *(const HANDLE* *)ssGetInputPortSignal( S, 0 ) )[0]

//==================================================================     outputs

#define out_current_a         ( ssGetOutputPortRealSignal       ( S, 0 ) )
#define out_current_b         ( ssGetOutputPortRealSignal       ( S, 1 ) )
#define out_handle_single ( (HANDLE* *)ssGetOutputPortSignal( S, 0 ) )[0]
#define out_handle_full   ( (HANDLE* *)ssGetOutputPortSignal( S, 2 ) )[0]

//==================================================================      dworks
#define dwork_out(i)      ( (real_T *)ssGetDWork( S, i ) )

//================================================================     constants

#define BUFFER_SIZES            15

//=============================================================     enumerations

enum    QBOT_MODE { PRIME_MOVERS_POS    = 1, EQ_POS_AND_PRESET  = 2 };
enum    COMM_DIRS { RX = 1, TX = 2, BOTH = 3, NONE = 4 };

//===================================================================     macros

#define NUM_OF_QBOTS    ( (int)mxGetNumberOfElements( ssGetSFcnParam( S, 0 ) ) )
#define REF_A_WIDTH     ssGetInputPortWidth( S, 1 )
#define REF_B_WIDTH     ssGetInputPortWidth( S, 2 )
#define SIGN(x)         ( ( (x) < 0) ? -1 : ( (x) > 0 ) )

//==============================================================================
//                                                           function prototypes
//==============================================================================

unsigned char checksum_ ( unsigned char * buf, int size );
void    showOutputHandle( SimStruct *S );
// unsigned int verifychecksum_( unsigned char * buffer );

//==============================================================================
//                                                            mdlInitializeSizes
//==============================================================================
// The sizes information is used by Simulink to determine the S-function block's
// characteristics (number of inputs, outputs, states, etc.).
//==============================================================================

static void mdlInitializeSizes( SimStruct *S )
{
    int_T   status;                // for new type definition
    DTypeId COM_HANDLE_id;         // for new type definition
    HANDLE  handle_aux;            // for new type definition
    int i;                         // for cycles

//======================================================     new type definition

    COM_HANDLE_id = ssRegisterDataType( S, "COM_HANDLE" );
    if( COM_HANDLE_id == INVALID_DTYPE_ID ) return;
    status = ssSetDataTypeSize( S, COM_HANDLE_id, sizeof(handle_aux) );
    if( status == 0)  return;
    status = ssSetDataTypeZero( S, COM_HANDLE_id, &handle_aux );
    if( status == 0 ) return;

//===================================================================     states

    ssSetNumContStates( S, 0 );
    ssSetNumDiscStates( S, 1 );

//===============================================================     parameters

    ssSetNumSFcnParams( S, 2 ); // 2 parameters:
                                //      - qbot I2C id
                                //      - daisy chaining

//===================================================================     inputs

    if ( !ssSetNumInputPorts( S, 1 ) ) return;

/////////////////////////////////////// 0 ) pointer to HANDLE   ////////////////
    ssSetInputPortWidth             ( S, 0, DYNAMICALLY_SIZED );
    ssSetInputPortDataType          ( S, 0, COM_HANDLE_id     );
    ssSetInputPortDirectFeedThrough ( S, 0, 1                 );
    ssSetInputPortRequiredContiguous( S, 0, 1                 );


//==================================================================     outputs

        if(params_daisy_chaining)
        {
            if (!ssSetNumOutputPorts(S, 3)) return;

///////////////////////////////// 3 ) com handle    ////////////////////////////
            ssSetOutputPortWidth   ( S, 2, 1             );
            ssSetOutputPortDataType( S, 2, COM_HANDLE_id );
        }
        else
        {
            if (!ssSetNumOutputPorts(S, 2)) return;
        }

////////////////////////////////   current A      //////////////////////////////
        ssSetOutputPortWidth    ( S, 0, NUM_OF_QBOTS );
        ssSetOutputPortDataType ( S, 0, SS_DOUBLE    );

////////////////////////////////   current B      //////////////////////////////
        ssSetOutputPortWidth    ( S, 1, NUM_OF_QBOTS );
        ssSetOutputPortDataType ( S, 1, SS_DOUBLE    );


//=============================================================     sample times

    ssSetNumSampleTimes(S, 1);

//=============================================================     work vectors

    ssSetNumDWork(S, NUM_OF_QBOTS);     // 0 dwork vector elements
    ssSetNumRWork(S, 0);                // 0 real work vector elements
    ssSetNumIWork(S, 0);                // 0 work vector elements
    ssSetNumPWork(S, 0);                // 0 pwork vector elements:
    ssSetNumModes(S, 0);                // 0 mode work vector elements
    ssSetNumNonsampledZCs(S, 0);        // 0 nonsampled zero crossings

    for( i = 0; i < NUM_OF_QBOTS; i++)
    {
        ssSetDWorkWidth(S, i, 4);
        ssSetDWorkDataType(S, i, SS_DOUBLE);
    }

//===================================================================     others

    ssSetOptions(S, SS_OPTION_ALLOW_INPUT_SCALAR_EXPANSION);

}

//==============================================================================
//                                                          mdlSetInputPortWidth
//==============================================================================
// This method is called with the candidate width for a dynamically sized port.
// If the proposed width is acceptable, the actual port width is set using
// ssSetInputPortWidth.
//==============================================================================

#undef MDL_SET_INPUT_PORT_WIDTH   // Change to #undef to remove function
#if defined(MDL_SET_INPUT_PORT_WIDTH) && defined(MATLAB_MEX_FILE)
static void mdlSetInputPortWidth( SimStruct *S, int portIndex, int width )
{
    switch( portIndex )
    {
        case 0:
            ssSetInputPortWidth( S, portIndex, 1 );
        break;
        default:
            ssSetInputPortWidth( S, portIndex, width );
        break;
    }
}
#endif /* MDL_SET_INPUT_PORT_WIDTH */

//==============================================================================
//                                                         mdlSetOutputPortWidth
//==============================================================================
// This method is called with the candidate width for a dynamically sized port.
// If the proposed width is acceptable, the actual port width is set using
// ssSetOutputPortWidth.
//==============================================================================

#undef MDL_SET_OUTPUT_PORT_WIDTH   // Change to #undef to remove function
#if defined(MDL_SET_OUTPUT_PORT_WIDTH) && defined(MATLAB_MEX_FILE)
static void mdlSetOutputPortWidth( SimStruct *S, int portIndex, int width )
{
        ssSetOutputPortWidth( S, portIndex, NUM_OF_QBOTS );
}
#endif /* MDL_SET_OUTPUT_PORT_WIDTH */

//==============================================================================
//                                                      mdlInitializeSampleTimes
//==============================================================================
// This function is used to specify the sample time(s) for your S-function.
// o  The sample times are specified as pairs "[sample_time, offset_time]" via
//    the following macros:
//    ssSetSampleTime(S, sampleTimePairIndex, sample_time)
//    ssSetOffsetTime(S, offsetTimePairIndex, offset_time)
//    Where sampleTimePairIndex starts at 0.
// o  A discrete function that changes at a specified rate should register the
//    discrete sample time pair
//          [discrete_sample_period, offset]
//    where
//          discrete_sample_period > 0.0
//    and
//          0.0 <= offset < discrete_sample_period
//==============================================================================

static void mdlInitializeSampleTimes( SimStruct *S )
{
    ssSetSampleTime(S, 0, INHERITED_SAMPLE_TIME);
    ssSetOffsetTime(S, 0, 0.0);
}

//==============================================================================
//                                                                      mdlStart
//==============================================================================
// This function is called once at start of model execution.
//==============================================================================

#define MDL_START  // Change to #undef to remove function
#if     defined(MDL_START)
static void mdlStart( SimStruct *S )
{
}
#endif /* MDL_START */

//==============================================================================
//                                                                    mdlOutputs
//==============================================================================
// Values are assigned to requested outputs in this function.
//==============================================================================

static void mdlOutputs( SimStruct *S, int_T tid )
{
    short int currents[2];
    uint8_T qbot_id;                                // qbot id's
    comm_settings comm_settings_t;

    int i;

//=============================     should an output handle appear in the block?

    if(params_daisy_chaining) showOutputHandle(S);

//====================================================     should we keep going?

    #if defined(_WIN32) || defined(_WIN64)
        if(in_handle == INVALID_HANDLE_VALUE) return;
    #else
        if(in_handle == -1) return;
    #endif


//==========================================     asking currents for each motor

    //RS485InitCommSettings(&comm_settings_t);
    comm_settings_t.file_handle = in_handle;

    for(i = 0; i < NUM_OF_QBOTS; i++)
    {
//============================================================      qbot ID check

     qbot_id = params_qbot_id(i);
     qbot_id = qbot_id <= 0   ? 1    : qbot_id;  // inferior limit
     qbot_id = qbot_id >= 128 ? 127  : qbot_id;  // superior limit


     out_current_a[i]    = dwork_out(i)[0];
     out_current_b[i]    = dwork_out(i)[1];

     if(!commGetCurrents(&comm_settings_t, qbot_id, currents))
     {
         out_current_a[i] = (double) currents[0];
         out_current_b[i] = (double) currents[1];
     }

     dwork_out(i)[0] = out_current_a[i];
     dwork_out(i)[1] = out_current_b[i];
     
    }

}


//==============================================================================
//                                                                     mdlUpdate
//==============================================================================
// This function is called once for every major integration time step.
//==============================================================================

#define MDL_UPDATE  // Change to #undef to remove function
#if defined(MDL_UPDATE)
static void mdlUpdate( SimStruct *S, int_T tid )
{

}
#endif /* MDL_UPDATE */

//==============================================================================
//                                                                  mdlTerminate
//==============================================================================
// In this function, you should perform any actions that are necessary at the
// termination of a simulation.
//==============================================================================

static void mdlTerminate( SimStruct *S )
{
}

//==============================================================================
//                                                              showOutputHandle
//==============================================================================
// TODO
//==============================================================================

void showOutputHandle( SimStruct *S )
{
        out_handle_full     = (HANDLE *) &in_handle;    // appear in output 3
}

//==============================================================================
//                                                   Required S-function trailer
//==============================================================================

#ifdef  MATLAB_MEX_FILE    // Is this file being compiled as a MEX-file?
#include "simulink.c"      // MEX-file interface mechanism
#else
#include "cg_sfun.h"       // Code generation registration function
#endif