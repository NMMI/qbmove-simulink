// -----------------------------------------------------------------------------
// Copyright (C)  qbrobotics. All rights reserved.
// www.qbrobotics.com
// -----------------------------------------------------------------------------
// File:        getImuReadings_v2.cpp
//
// Description: Communication s-function for obtaining IMU values.
//              To be used with RS-485 on a Virtual COM.
//------------------------------------------------------------------------------


//==============================================================================
//                                                             main  definitions
//==============================================================================

#define S_FUNCTION_NAME  getImuReadings_v2
#define S_FUNCTION_LEVEL 2

//==============================================================================
//                                                                      includes
//==============================================================================

#include "simstruc.h"
#include "../../../qbAPI/src/qbmove_communications.h"
#include "../../../qbAPI/src/cp_communications.h"
#include <math.h>
// #include <windows.h>

//==============================================================================
//                                                                   definitions
//==============================================================================

//===============================================================     parameters

#define params_qbot_id(i)     ( mxGetPr( ssGetSFcnParam( S, 0 ) )[ \
                                     i >= NUM_OF_QBOTS ? NUM_OF_QBOTS -1 : i ] )
#define N_IMU					( (int) mxGetScalar( ssGetSFcnParam( S, 1 ) ) )
#define params_daisy_chaining  ( (bool)mxGetScalar( ssGetSFcnParam( S, 2 ) ) )
#define PARAM_UNITY_ACC_FCN  ( (int) mxGetScalar( ssGetSFcnParam( S, 3 ) ) )
#define PARAM_UNITY_GYRO_FCN ( (int) mxGetScalar( ssGetSFcnParam( S, 4 ) ) )
#define PARAM_UNITY_MAG_FCN ( (int) mxGetScalar( ssGetSFcnParam( S, 5 ) ) )

#define G                       1
#define M_S2                    2
#define DEG_S                   1
#define RAD_S                   2
#define UT                    	1
#define G_TO_MS2				9.79
#define DEG_TO_RAD             (3.14159265359 / 180.0)

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
char n_imu = 0;                // number of connected imus
uint8_T qbot_id;               // qbot id's
uint8_T* imu_ids;
uint8_T* imu_table;
uint8_T* mag_cal;
float* imu_values;

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
  	n_imu = N_IMU;
	
	
//===================================================================     states

    ssSetNumContStates( S, 0 );
    ssSetNumDiscStates( S, 1 );

//===============================================================     parameters

    ssSetNumSFcnParams( S, 6 ); // 6 parameters:
                                //      - qbot I2C id
								//		- n_imu
                                //      - daisy chaining
						  // - acc_unity, gyro_unity, mag_unity
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
		if (!ssSetNumOutputPorts(S, 1 + 5)) return;

///////////////////////////////// 3 ) com handle    ////////////////////////////
		ssSetOutputPortWidth   ( S, 5, 1             );
		ssSetOutputPortDataType( S, 5, COM_HANDLE_id );
	}
	else
	{
		if (!ssSetNumOutputPorts(S, 5)) return;
	}
			

////////////////////////////////   values      //////////////////////////////
    if (n_imu <= 1){
    	for (int i=0; i < n_imu; i++){
            DECL_AND_INIT_DIMSINFO(di);
        	int_T dims[2];
    
            di.numDims = 2;
            dims[0] = NUM_OF_QBOTS;
            
            // Acc output
            dims[1] = 3;
            di.dims = dims;
            di.width = NUM_OF_QBOTS * 3 * n_imu;
            ssSetOutputPortDimensionInfo(S, 0, &di);
            ssSetOutputPortDataType ( S, 0, SS_DOUBLE );

            // Gyro output
            dims[1] = 3;
            di.dims = dims;
            di.width = NUM_OF_QBOTS * 3 * n_imu;
            ssSetOutputPortDimensionInfo(S, 1, &di);
            ssSetOutputPortDataType ( S, 1, SS_DOUBLE );

            // Mag output
            dims[1] = 3;
            di.dims = dims;
            di.width = NUM_OF_QBOTS * 3 * n_imu;
            ssSetOutputPortDimensionInfo(S, 2, &di);
            ssSetOutputPortDataType ( S, 2, SS_DOUBLE );

            // Quat output
            dims[1] = 4;
            di.dims = dims;
            di.width = NUM_OF_QBOTS * 4 * n_imu;
            ssSetOutputPortDimensionInfo(S, 3, &di);
            ssSetOutputPortDataType ( S, 3, SS_DOUBLE );

            // Temp output
            dims[1] = 1;
            di.dims = dims;
            di.width = NUM_OF_QBOTS * n_imu;
            ssSetOutputPortDimensionInfo(S, 4, &di);
            ssSetOutputPortDataType ( S, 4, SS_DOUBLE );
        }
    }
    else {
        int_T dims[3];
        DECL_AND_INIT_DIMSINFO(di);
        di.numDims = 3;
        dims[0] = NUM_OF_QBOTS;

        // Acc output
        dims[1] = 3;
        dims[2] = n_imu;
        di.dims = dims;
        di.width = NUM_OF_QBOTS * 3 * n_imu;
        ssSetOutputPortDimensionInfo(S, 0, &di);
        ssSetOutputPortDataType ( S, 0, SS_DOUBLE );

        // Gyro output
        dims[1] = 3;
        dims[2] = n_imu;
        di.dims = dims;
        di.width = NUM_OF_QBOTS * 3 * n_imu;
        ssSetOutputPortDimensionInfo(S, 1, &di);
        ssSetOutputPortDataType ( S, 1, SS_DOUBLE );

        // Mag output
        dims[1] = 3;
        dims[2] = n_imu;
        di.dims = dims;
        di.width = NUM_OF_QBOTS * 3 * n_imu;
        ssSetOutputPortDimensionInfo(S, 2, &di);
        ssSetOutputPortDataType ( S, 2, SS_DOUBLE );

        // Quat output
        dims[1] = 4;
        dims[2] = n_imu;
        di.dims = dims;
        di.width = NUM_OF_QBOTS * 4 * n_imu;
        ssSetOutputPortDimensionInfo(S, 3, &di);
        ssSetOutputPortDataType ( S, 3, SS_DOUBLE );

        // Temp output
        dims[1] = 1;
        dims[2] = n_imu;
        di.dims = dims;
        di.width = NUM_OF_QBOTS * n_imu;
        ssSetOutputPortDimensionInfo(S, 4, &di);
        ssSetOutputPortDataType ( S, 4, SS_DOUBLE );
    }
	

//=============================================================     sample times

    ssSetNumSampleTimes(S, 1);

//=============================================================     work vectors

    ssSetNumDWork(S, NUM_OF_QBOTS);     // 0 dwork vector elements
    ssSetNumRWork(S, 0);                // 0 real work vector elements
    ssSetNumIWork(S, 0);                // 0 work vector elements
    ssSetNumPWork(S, 0);                // 0 pwork vector elements:
    ssSetNumModes(S, 0);                // 0 mode work vector elements
    ssSetNumNonsampledZCs(S, 0);        // 0 nonsampled zero crossings

    for( i = 0; i < NUM_OF_QBOTS; ++i)
    {
        ssSetDWorkWidth(S, i, 19*6);
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
	uint8_T aux_string[2000];
	uint8_T PARAM_SLOT_BYTES = 50;
	uint8_T n_imu_local;
    uint8_T qbot_id;                                // qbot id's
	uint8_T num_imus_id_params = 7;
	uint8_T num_mag_cal_params = 0;
	uint8_T first_imu_parameter = 2;
	uint8_T i = 0;	
    
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
	
    if (commGetIMUParamList(&comm_settings_t, qbot_id, 0, NULL, 0, 0, aux_string) < 0){
        // If commGetIMUParamList returns -1, the connected board is a PSoC3 board instead of a STM32 or PSoC5 board
        // so call the commGetParamList instead
        commGetParamList(&comm_settings_t, qbot_id, 0, NULL, 0, 0, aux_string);
        num_imus_id_params = 6;
    }

	//aux_string[6] <-> packet_data[2] on the firmware
	n_imu_local = aux_string[8];
	printf("Number of connected IMUs: %d\n", n_imu_local);
	
	// Control on number of connected IMUs
	if (n_imu != n_imu_local){
		char msg[256];
		sprintf(msg,"Error: Number of connected IMUs (%d) does not match number of configured IMUs (%d)", n_imu_local, n_imu);
		ssSetErrorStatus(S,msg);
		return;
	}
	
	// Compute number of read parameters depending on global_args.n_imu and
	// update packet_length
	num_mag_cal_params = (n_imu / 2);
	if ( (n_imu - num_mag_cal_params*2) > 0 ) num_mag_cal_params++;
		
	imu_ids = (uint8_t *) calloc(n_imu, sizeof(uint8_t));
	i = 0;
	for (int k = 1; k <= num_imus_id_params; k++){
		if (aux_string[k*PARAM_SLOT_BYTES + 8] != 255) {
			imu_ids[i] = aux_string[k*PARAM_SLOT_BYTES + 8];
			i++;
		}
		if (aux_string[k*PARAM_SLOT_BYTES + 9] != 255) {
			imu_ids[i] = aux_string[k*PARAM_SLOT_BYTES + 9];
			i++;
		}
		if (aux_string[k*PARAM_SLOT_BYTES + 10] != 255) {
			imu_ids[i] = aux_string[k*PARAM_SLOT_BYTES + 10];
			i++;
		}
	}
	
	// Retrieve magnetometer calibration parameters
	mag_cal = (uint8_t *) calloc(n_imu, 3*sizeof(uint8_t));
	i = 0;
	for (int k=1; k <= num_mag_cal_params; k++) {
		mag_cal[3*i + 0] = aux_string[num_imus_id_params*PARAM_SLOT_BYTES + k*PARAM_SLOT_BYTES + 8];
		mag_cal[3*i + 1] = aux_string[num_imus_id_params*PARAM_SLOT_BYTES + k*PARAM_SLOT_BYTES + 9];
		mag_cal[3*i + 2] = aux_string[num_imus_id_params*PARAM_SLOT_BYTES + k*PARAM_SLOT_BYTES + 10];
		//printf("MAG PARAM: %d %d %d\n", mag_cal[3*i + 0], mag_cal[3*i + 1], mag_cal[3*i + 2]);
		i++;
		
		if (aux_string[num_imus_id_params*PARAM_SLOT_BYTES + k*PARAM_SLOT_BYTES + 7] == 6) {
			mag_cal[3*i + 0] = aux_string[num_imus_id_params*PARAM_SLOT_BYTES + k*PARAM_SLOT_BYTES + 11];
			mag_cal[3*i + 1] = aux_string[num_imus_id_params*PARAM_SLOT_BYTES + k*PARAM_SLOT_BYTES + 12];
			mag_cal[3*i + 2] = aux_string[num_imus_id_params*PARAM_SLOT_BYTES + k*PARAM_SLOT_BYTES + 13];
			//printf("MAG PARAM: %d %d %d\n", mag_cal[3*i + 0], mag_cal[3*i + 1], mag_cal[3*i + 2]);
			i++;
		}
	}
	
	first_imu_parameter = 1 + num_imus_id_params + num_mag_cal_params + 1;
	imu_table = (uint8_t *) calloc(n_imu, 5*sizeof(uint8_t));
	for (int i=0; i< n_imu; i++){
		imu_table[5*i + 0] = aux_string[first_imu_parameter*PARAM_SLOT_BYTES + 8 + 50*i];
		imu_table[5*i + 1] = aux_string[first_imu_parameter*PARAM_SLOT_BYTES + 9 + 50*i];
		imu_table[5*i + 2] = aux_string[first_imu_parameter*PARAM_SLOT_BYTES + 10 + 50*i];
        imu_table[5*i + 3] = aux_string[first_imu_parameter*PARAM_SLOT_BYTES + 11 + 50*i];
        imu_table[5*i + 4] = aux_string[first_imu_parameter*PARAM_SLOT_BYTES + 12 + 50*i];
		//printf("ID: %d  - %d, %d, %d, %d, %d\n", imu_ids[i], imu_table[5*i + 0], imu_table[5*i + 1], imu_table[5*i + 2], imu_table[5*i + 3], imu_table[5*i + 4]);
		
	}
	
    // Imu values is a 3 sensors x 3 axes x n_imu values
    imu_values = (float *) calloc(n_imu, (3*3 + 4 + 1)*sizeof(float));
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
	
	// Change Unity of Measurement
    switch(PARAM_UNITY_ACC_FCN){
        case G:
            meas_unity_acc = 1;
            break;
        case M_S2:
            meas_unity_acc = G_TO_MS2;
            break;
        default:
            meas_unity_acc = 1;
    }
	switch(PARAM_UNITY_GYRO_FCN){
        case DEG_S:
            meas_unity_gyro = 1;
            break;
        case RAD_S:
            meas_unity_gyro = DEG_TO_RAD;
            break;
        default:
            meas_unity_gyro = 1;
    }
	switch(PARAM_UNITY_MAG_FCN){
        case UT:
            meas_unity_gyro = 1;
            break;
        default:
            meas_unity_gyro = 1;
    }
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
	  
		commGetImuReadings(&comm_settings_t, qbot_id, imu_table, mag_cal, n_imu, imu_values);

		for (i = 0; i < n_imu; i++) {
			
			// Acc outputs
			out(0)[3*i+0] = (float)imu_values[(3*3+4+1)*i+0] * meas_unity_acc;;
			out(0)[3*i+1] = (float)imu_values[(3*3+4+1)*i+1] * meas_unity_acc;;
			out(0)[3*i+2] = (float)imu_values[(3*3+4+1)*i+2] * meas_unity_acc;;
			
			// Gyro outputs
			out(1)[3*i+0] = (float)imu_values[(3*3+4+1)*i+3] * meas_unity_gyro;
			out(1)[3*i+1] = (float)imu_values[(3*3+4+1)*i+4] * meas_unity_gyro;
			out(1)[3*i+2] = (float)imu_values[(3*3+4+1)*i+5] * meas_unity_gyro;
			
			// Mag outputs
			out(2)[3*i+0] = (float)imu_values[(3*3+4+1)*i+6] * meas_unity_mag;
			out(2)[3*i+1] = (float)imu_values[(3*3+4+1)*i+7] * meas_unity_mag;
			out(2)[3*i+2] = (float)imu_values[(3*3+4+1)*i+8] * meas_unity_mag;
			
			// Quat outputs
			out(3)[4*i+0] = (float)imu_values[(3*3+4+1)*i+9];
			out(3)[4*i+1] = (float)imu_values[(3*3+4+1)*i+10];
			out(3)[4*i+2] = (float)imu_values[(3*3+4+1)*i+11];
			out(3)[4*i+3] = (float)imu_values[(3*3+4+1)*i+12];

			// Temp outputs
			out(4)[i] = (float)imu_values[(3*3+4+1)*i+13];
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