// -----------------------------------------------------------------------------
// BSD 3-Clause License
//
// Copyright (c) 2021, Centro "E. Piaggio"
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// -----------------------------------------------------------------------------
// File:        readAnalog.cpp
//
// Description: Communication s-function to obtain ADC measurements from a board.
//              To be used with RS-485 on a Virtual COM.
//------------------------------------------------------------------------------


//==============================================================================
//                                                             main  definitions
//==============================================================================

#define S_FUNCTION_NAME  readAnalog
#define S_FUNCTION_LEVEL 2

//==============================================================================
//                                                                      includes
//==============================================================================

#include "definitions.h"
#include "simstruc.h"
#include "../../../qbAPI/src/qbmove_communications.h"
#include "../../../qbAPI/src/cp_communications.h"
// #include <windows.h>


//==============================================================================
//                                                                   definitions
//==============================================================================

//===============================================================     parameters


#define params_qbot_id(i)     ( mxGetPr( ssGetSFcnParam( S, 0 ) )[ \
                                     i >= NUM_OF_QBOTS ? NUM_OF_QBOTS -1 : i ] )
#define N_CHANNELS			( (int) mxGetScalar( ssGetSFcnParam( S, 1 ) ) )
#define params_daisy_chaining  ( (bool)mxGetScalar( ssGetSFcnParam( S, 2 ) ) )

//===================================================================     inputs

#define in_handle ( *(const HANDLE* *)ssGetInputPortSignal( S, 0 ) )[0]

//==================================================================     outputs

#define out(i)        ( ssGetOutputPortRealSignal       ( S, i ) )
                                     
#define out_handle_single ( (HANDLE* *)ssGetOutputPortSignal( S, 0 ) )[0]
#define out_handle_full   ( (HANDLE* *)ssGetOutputPortSignal( S, 2 ) )[0]

//==================================================================      dworks
#define dwork_out(i)      ( (real_T *)ssGetDWork( S, i ) )

//================================================================     constants

#define BUFFER_SIZES            15

comm_settings comm_settings_t; 


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
    uint8_T qbot_id;                                // qbot id's
	
    char n_channels = 0;                // number of connected ADC sensors

    ssAllowSignalsWithMoreThan2D(S);
//======================================================     new type definition

    COM_HANDLE_id = ssRegisterDataType( S, "COM_HANDLE" );
    if( COM_HANDLE_id == INVALID_DTYPE_ID ) return;
    status = ssSetDataTypeSize( S, COM_HANDLE_id, sizeof(handle_aux) );
    if( status == 0)  return;
    status = ssSetDataTypeZero( S, COM_HANDLE_id, &handle_aux );
    if( status == 0 ) return;

    qbot_id = params_qbot_id(0);
    qbot_id = qbot_id <= 0   ? 1    : qbot_id;  // inferior limit
    qbot_id = qbot_id >= 128 ? 127  : qbot_id;  // superior limit
    
    
//==================================================== 
  	n_channels = N_CHANNELS;
	

//===================================================================     states

    ssSetNumContStates( S, 0 );
    ssSetNumDiscStates( S, 1 );

//===============================================================     parameters

    ssSetNumSFcnParams( S, 3 ); // 6 parameters:
                                //      - qbot I2C id
								//		- n_channels
                                //      - daisy chaining
						  
//===================================================================     inputs

    if ( !ssSetNumInputPorts( S, 1 ) ) return;

/////////////////////////////////////// 0 ) pointer to HANDLE   ////////////////
    ssSetInputPortWidth             ( S, 0, 1 );
    ssSetInputPortDataType          ( S, 0, COM_HANDLE_id     );
    ssSetInputPortDirectFeedThrough ( S, 0, 1                 );
    ssSetInputPortRequiredContiguous( S, 0, 1                 );


//==================================================================     outputs
  
	if(params_daisy_chaining)
	{
		if (!ssSetNumOutputPorts(S, 1 + n_channels)) return;

///////////////////////////////// 3 ) com handle    ////////////////////////////
		ssSetOutputPortWidth   ( S, n_channels, 1             );
		ssSetOutputPortDataType( S, n_channels, COM_HANDLE_id );
	}
	else
	{
		if (!ssSetNumOutputPorts(S, n_channels)) return;
	}

////////////////////////////////   values      //////////////////////////////
	for (int i=0; i < n_channels; i++){
		
		DECL_AND_INIT_DIMSINFO(di);
		int_T dims[2];

		di.numDims = 2;
		dims[0] = NUM_OF_QBOTS;
		dims[1] = 1;
		di.dims = dims;
		di.width = NUM_OF_QBOTS * (1);
		ssSetOutputPortDimensionInfo(S, i, &di);

		ssSetOutputPortDataType ( S, i, SS_DOUBLE );
	}

//=============================================================     sample times

    ssSetNumSampleTimes(S, 1);

//=============================================================     work vectors

    ssSetNumDWork(S, NUM_OF_QBOTS);     // 0 dwork vector elements
    ssSetNumRWork(S, 0);                // 0 real work vector elements
    ssSetNumIWork(S, 0);                // 0 work vector elements
    ssSetNumPWork(S, 1);                // 1 pwork vector elements: adc_raw
    ssSetNumModes(S, 0);                // 0 mode work vector elements
    ssSetNumNonsampledZCs(S, 0);        // 0 nonsampled zero crossings

    for( i = 0; i < NUM_OF_QBOTS; ++i)
    {
        ssSetDWorkWidth(S, i, 12*6);
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
    uint8_T qbot_id;   		// qbot id's
	uint8_T i = 0;	
    uint8_T adc_map[100];
    uint8_T tot_adc_channels = 0;
    uint8_T used_adc_channels = 0;
    
    char n_channels = N_CHANNELS;                // number of connected ADC sensors
    short int* adc_raw;
//====================================================     should we keep going?

    #if defined(_WIN32) || defined(_WIN64)
        if(in_handle == INVALID_HANDLE_VALUE) return;
    #else
        if(in_handle == -1) return;
    #endif

    qbot_id = params_qbot_id(0);
    qbot_id = qbot_id <= 0   ? 1    : qbot_id;  // inferior limit
    qbot_id = qbot_id >= 128 ? 127  : qbot_id;  // superior limit
    
//==========================================     asking imu reading

    comm_settings_t.file_handle = in_handle;
	
    commGetADCConf(&comm_settings_t, qbot_id, &tot_adc_channels, adc_map);
    
    for (int i=0; i< tot_adc_channels; i++) {
        if (adc_map[i] == 1) {
            used_adc_channels++;
        }
    }

	printf("Number of enabled ADC channels IMUs: %d\n", used_adc_channels);
	
	// Control on number of connected IMUs
	if (n_channels != used_adc_channels){
		char msg[256];
		sprintf(msg,"Error: Number of enabled ADC channels (%d) does not match number of configured ADC channels (%d)", used_adc_channels, n_channels);
		ssSetErrorStatus(S,msg);
		return;
	}

    adc_raw = (short int *) calloc(n_channels, (1)*sizeof(short int));

    void **PWork = ssGetPWork(S);
    
    PWork[0] = adc_raw;
	
}
#endif /* MDL_START */

//==============================================================================
//                                                                    mdlOutputs
//==============================================================================
// Values are assigned to requested outputs in this function.
//==============================================================================

static void mdlOutputs( SimStruct *S, int_T tid )
{
    short int val;
    uint8_T qbot_id;                                // qbot id's
    char* values;
    int c = 0;
    int c_s = 0;
	int c_id;
	uint8_T flags[3] = {0,0,0};
    int imu_data_size;
    int i;

	float meas_unity_acc = 1;
	float meas_unity_gyro = 1;
	float meas_unity_mag = 1;
	
    char n_channels = N_CHANNELS;                // number of connected ADC sensors
    short int* adc_raw;
//=============================    retrieve data from work vectors
    adc_raw   = (short int*) ssGetPWorkValue(S,0);

//=============================     should an output handle appear in the block?

    if(params_daisy_chaining) showOutputHandle(S);

//====================================================     should we keep going?

    #if defined(_WIN32) || defined(_WIN64)
        if(in_handle == INVALID_HANDLE_VALUE) return;
    #else
        if(in_handle == -1) return;
    #endif


//==========================================     asking imu reading

    comm_settings_t.file_handle = in_handle;

    for(i = 0; i < NUM_OF_QBOTS; i++)
    {
//============================================================      qbot ID check

     qbot_id = params_qbot_id(i);
     qbot_id = qbot_id <= 0   ? 1    : qbot_id;  // inferior limit
     qbot_id = qbot_id >= 128 ? 127  : qbot_id;  // superior limit
  
     commGetADCRawValues(&comm_settings_t, qbot_id, n_channels, adc_raw);

     for (int i = 0; i < n_channels; i++) {
         out(i)[0] = (int)adc_raw[i];
     }
	
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
    char aux_char;
    int try_counter;
    int i;
    comm_settings comm_settings_t;

    comm_settings_t.file_handle = in_handle;

    if (comm_settings_t.file_handle == INVALID_HANDLE_VALUE) {
        closeRS485(&comm_settings_t);
        return;
    }
 
    ssSetPWorkValue(S,0,NULL);      //adc_raw

    closeRS485(&comm_settings_t);
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