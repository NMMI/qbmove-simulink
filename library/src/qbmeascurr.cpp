// -----------------------------------------------------------------------------
// Copyright (C)  qbrobotics. All rights reserved.
// www.qbrobotics.com
// -----------------------------------------------------------------------------
// File:        qbotISS_ISS.cpp
//
// Description: Communication s-function for the QB Move. To be used with RS-485
//              on a Virtual COM.
//------------------------------------------------------------------------------

//==============================================================================
//                                                             main  definitions
//==============================================================================

#define S_FUNCTION_NAME  qbmeascurr
#define S_FUNCTION_LEVEL 2

//==============================================================================
//                                                                      includes
//==============================================================================

#include "definitions.h"
#include "simstruc.h"
#include "../../../qbAPI/src/qbmove_communications.h"


#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #define sleep(x) Sleep(1000 * (x))
    #define usleep(x) Sleep((x) / 1000)
#endif

#if !(defined(_WIN32) || defined(_WIN64))
    #include <unistd.h>
#endif


//==============================================================================
//                                                                   definitions
//==============================================================================

//===============================================================     parameters

#define params_qbot_id(i)       ( mxGetPr( ssGetSFcnParam( S, 0 ) )[ \
                                    i >= NUM_OF_QBOTS ? NUM_OF_QBOTS - 1 : i ] )
#define params_com_direction      ( (int) mxGetScalar( ssGetSFcnParam( S, 1 ) ) )
#define params_qbot_mode          ( (int) mxGetScalar( ssGetSFcnParam( S, 2 ) ) )
#define params_daisy_chaining  ( (bool) mxGetScalar( ssGetSFcnParam( S, 3 ) ) )
#define params_angle_range(i)  ( mxGetPr( ssGetSFcnParam( S, 4 ) )[ \
                              i >= ANG_RANGE_WIDTH ? ANG_RANGE_WIDTH - 1 : i ] )
#define params_sw_lim_range(i) ( mxGetPr( ssGetSFcnParam(S, 5) )[ \
                                     i >= SW_LIM_WIDTH ? SW_LIM_WIDTH - 1 : i] )
#define params_joint_offset(i) ( mxGetPr( ssGetSFcnParam(S, 6) )[\
                                     i >= OFFSET_WIDTH ? OFFSET_WIDTH - 1 : i] )

#define PARAM_ACTIVE_STARTUP_FCN  ( (bool) mxGetScalar( ssGetSFcnParam( S, 7 ) ) )
#define PARAM_WDT_FCN  ( (short int) mxGetScalar( ssGetSFcnParam( S, 8 ) ) )
#define PARAM_UNITY_FCN  ( (int) mxGetScalar( ssGetSFcnParam( S, 9 ) ) )


//===================================================================     inputs

#define in_handle ( *(const HANDLE* *)ssGetInputPortSignal( S, 0 ) )[0]
#define in_ref_a  ( (const real_T *)ssGetInputPortSignal(   S, 1 ) )
#define in_ref_b  ( (const real_T *)ssGetInputPortSignal(   S, 2 ) )

#define in_ref_activation(i)  ( (const real_T *)ssGetInputPortSignal(   S, i ) ) 

//==================================================================     outputs

#define out_pos_a         ( ssGetOutputPortRealSignal       ( S, 0 ) )
#define out_pos_b         ( ssGetOutputPortRealSignal       ( S, 1 ) )
#define out_pos_link      ( ssGetOutputPortRealSignal       ( S, 2 ) )
#define out_curr_1        ( ssGetOutputPortRealSignal       ( S, 3 ) )
#define out_curr_2        ( ssGetOutputPortRealSignal       ( S, 4 ) )
#define out_debug         ( ssGetOutputPortRealSignal       ( S, 5 ) )
#define out_handle_single ( (HANDLE* *)ssGetOutputPortSignal( S, 0 ) )[0]
#define out_handle_full   ( (HANDLE* *)ssGetOutputPortSignal( S, 3 ) )[0]

//==================================================================      dworks
#define dwork_out(i)      ( (real_T *)ssGetDWork( S, i ) )

//================================================================     constants

#define BUFFER_SIZES            15
#define ANG_TO_DEG              (720.0/65536.0)
#define DEG_TO_ANG              (65536.0/720.0)
#define PERC_TO_NUM             (32767.0/100.0)
#define MAX_STIFF               4000
#define MAX_POS                 15000

#define ON                      true
#define OFF                     false
#define TICK                    3
#define RADIANTS                2
#define DEGREES                 1
#define RAD_TO_DEG              (180.0 / 3.14159265359)

#define WIDTH_MISMATCH_A        0
#define WIDTH_MISMATCH_B        1
#define WIDTH_MISMATCH_ACTIVATE 2
#define REACH_QBOTS_MAX         3
//=============================================================     enumerations

enum    QBOT_MODE { PRIME_MOVERS_POS = 1, EQ_POS_AND_PRESET = 2 , EQ_POS_AND_STIFF_PERC = 3};
enum    COMM_DIRS { RX = 1, TX = 2, BOTH = 3, NONE = 4 };

//===================================================================     macros

#define NUM_OF_QBOTS    ( (int)mxGetNumberOfElements( ssGetSFcnParam( S, 0 ) ) )
#define ANG_RANGE_WIDTH ( (int)mxGetNumberOfElements( ssGetSFcnParam( S, 4 ) ) )
#define SW_LIM_WIDTH    ( (int)mxGetNumberOfElements( ssGetSFcnParam( S, 5 ) ) )
#define OFFSET_WIDTH    ( (int)mxGetNumberOfElements( ssGetSFcnParam( S, 6 ) ) )
#define REF_A_WIDTH     ssGetInputPortWidth( S, 1 )
#define REF_B_WIDTH     ssGetInputPortWidth( S, 2 )
#define SIGN(x)         ( ( (x) < 0) ? -1 : ( (x) > 0 ) )

#define REF_ACTIVATE_WIDTH     ssGetInputPortWidth( S, 3 )
//==============================================================================
//                                                           function prototypes
//==============================================================================

unsigned char checksum_ ( unsigned char * buf, int size );
void    showOutputHandle( SimStruct *S );
void    activation(SimStruct *s, bool flag, const int ID = -1);
void    errorHandle(SimStruct *S, const int);

//==============================================================================
//                                                              Global Variables
//==============================================================================

int activation_state[255];

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
    if( COM_HANDLE_id == INVALID_DTYPE_ID )
        return;
    status = ssSetDataTypeSize( S, COM_HANDLE_id, sizeof(handle_aux) );
    if( status == 0)
        return;
    status = ssSetDataTypeZero( S, COM_HANDLE_id, &handle_aux );
    if( status == 0)
        return;

//===================================================================     states

    ssSetNumContStates( S, 0 );
    ssSetNumDiscStates( S, 1 );

//===============================================================     parameters

    ssSetNumSFcnParams( S, 10 ); // 10 parameters:
                                 //    - qbot I2C id
                                 //      - comm. direction: rx/tx/both
                                 //      - qbot mode: q1-q2 or qs-qd
                                 //      - daisy chaining
                                 //      - angle range
                                 //    - software limit on equilibrium position
                                 //    - joint offset for equilibrium position
                                 //    - activation on startup button
                                 //    - watchdog timer
                                 //    - measurement unity

//===================================================================     inputs

    // Set number of inputs

    switch(params_com_direction)
    {
        case TX :
        case BOTH :
            i = 3;
        break;
        case RX :
        default :
            i = 1;
        break;
    }

    if ( !PARAM_ACTIVE_STARTUP_FCN )
        i++;        

    if ( !ssSetNumInputPorts( S, i ) ) 
        return;

/////////////////////////////////////// 0 ) pointer to HANDLE   ////////////////
    ssSetInputPortWidth             ( S, 0, DYNAMICALLY_SIZED );
    ssSetInputPortDataType          ( S, 0, COM_HANDLE_id     );
    ssSetInputPortDirectFeedThrough ( S, 0, 1                 );
    ssSetInputPortRequiredContiguous( S, 0, 1                 );


    // IF transmitting only OR both tx and rx
    if( (params_com_direction == TX) | (params_com_direction == BOTH) )
    {
//////////////////////////////////////// 1 ) reference A    ////////////////////
        ssSetInputPortWidth             ( S, 1, DYNAMICALLY_SIZED );
        ssSetInputPortDataType          ( S, 1, SS_DOUBLE         );
        ssSetInputPortDirectFeedThrough ( S, 1, 0                 );
        ssSetInputPortRequiredContiguous( S, 1, 1                 );

//////////////////////////////////////// 2 ) reference B    ////////////////////
        ssSetInputPortWidth             ( S, 2, DYNAMICALLY_SIZED );
        ssSetInputPortDataType          ( S, 2, SS_DOUBLE         );
        ssSetInputPortDirectFeedThrough ( S, 2, 0                 );
        ssSetInputPortRequiredContiguous( S, 2, 1                 );
     
        if( !PARAM_ACTIVE_STARTUP_FCN )
        {
//////////////////////////////////////// 3 ) external activation ///////////////
            ssSetInputPortWidth             ( S, 3, DYNAMICALLY_SIZED );
            ssSetInputPortDataType          ( S, 3, SS_DOUBLE         );
            ssSetInputPortDirectFeedThrough ( S, 3, 1                 );
            ssSetInputPortRequiredContiguous( S, 3, 1                 );
        }
    }
    else{ // RX or BOTH
        if( !PARAM_ACTIVE_STARTUP_FCN )
        {
//////////////////////////////////////// 1 ) external activation ///////////////
            ssSetInputPortWidth             ( S, 1, DYNAMICALLY_SIZED );
            ssSetInputPortDataType          ( S, 1, SS_DOUBLE         );
            ssSetInputPortDirectFeedThrough ( S, 1, 1                 );
            ssSetInputPortRequiredContiguous( S, 1, 1                 );
        }

    }


//==================================================================     outputs

    // IF transmitting only OR no communication
    if( (params_com_direction == TX) | (params_com_direction == NONE) )
    {    // IF daisy chaining activated
        if(params_daisy_chaining)
        {
            if (!ssSetNumOutputPorts(S, 1)) 
                return;

///////////////////////////////// 0 ) com handle    ////////////////////////////
            ssSetOutputPortWidth    ( S, 0, 1             );
            ssSetOutputPortDataType ( S, 0, COM_HANDLE_id );
        }
        else
        {
            if (!ssSetNumOutputPorts(S, 0)) 
                return;
        }
    }

    // IF receiving only OR both tx and rx
    if((params_com_direction == RX) | (params_com_direction == BOTH))
    {    // IF daisy chaining activated
        if(params_daisy_chaining)
        {
            if (!ssSetNumOutputPorts(S, 7)) return;

///////////////////////////////// 3 ) com handle    ////////////////////////////
            ssSetOutputPortWidth   ( S, 6, 1             );
            ssSetOutputPortDataType( S, 6, COM_HANDLE_id );
        }
        else
        {
            if (!ssSetNumOutputPorts(S, 6)) return;
        }

//////////////////////////////// 0 ) position A   //////////////////////////////
        ssSetOutputPortWidth    ( S, 0, NUM_OF_QBOTS );
        ssSetOutputPortDataType ( S, 0, SS_DOUBLE    );

//////////////////////////////// 1 ) position B   //////////////////////////////
        ssSetOutputPortWidth    ( S, 1, NUM_OF_QBOTS );
        ssSetOutputPortDataType ( S, 1, SS_DOUBLE    );

//////////////////////////////// 2 ) position link    //////////////////////////
        ssSetOutputPortWidth    ( S, 2, NUM_OF_QBOTS );
        ssSetOutputPortDataType ( S, 2, SS_DOUBLE    );

//////////////////////////////// 3 ) debug integer    //////////////////////////
        ssSetOutputPortWidth    ( S, 3, NUM_OF_QBOTS );
        ssSetOutputPortDataType ( S, 3, SS_DOUBLE    );
        
//////////////////////////////// 3 ) debug integer    //////////////////////////
        ssSetOutputPortWidth    ( S, 4, NUM_OF_QBOTS );
        ssSetOutputPortDataType ( S, 4, SS_DOUBLE    );

//////////////////////////////// 3 ) debug integer    //////////////////////////
        ssSetOutputPortWidth    ( S, 5, NUM_OF_QBOTS );
        ssSetOutputPortDataType ( S, 5, SS_DOUBLE    );
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

    for( i = 0; i < NUM_OF_QBOTS; i++)
    {
        ssSetDWorkWidth(S, i, 6);
        ssSetDWorkDataType(S, i, SS_DOUBLE);
    }

//===================================================================     others

    ssSetOptions(S, SS_OPTION_ALLOW_INPUT_SCALAR_EXPANSION);
    //ssSetOptions(S, SS_OPTION_CALL_TERMINATE_ON_EXIT);

//=======================================================     mask modifications

    switch( params_qbot_mode )
    {
        case PRIME_MOVERS_POS:
            mexEvalString(
                "set_param( gcb, 'CONTROL_MODE', 'Prime Movers Positions')" );
            break;
        case EQ_POS_AND_PRESET:
            mexEvalString(
                "set_param( gcb, 'CONTROL_MODE', 'Equilibrium Position and Stiffness Preset')" );
            break;
        
        case EQ_POS_AND_STIFF_PERC:
            mexEvalString(
                "set_param( gcb, 'CONTROL_MODE', 'Equilibrium Position and Stiffness Preset Percentage')" );
            break;
    }

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
  switch( portIndex ) {
      case 4 :
          // IF transmittinging only OR both tx and rx AND daisy chaining activated
          // there is an output number 3 (the 4^th)
          // but there is nothing to be done
          break;
      case 0 :
          // IF NOT receiving only AND NOT both tx and rx AND daisy chaining activated
          // output 0 (the 1^st) is the handle pointer output
          // hence there is nothing to be done
          if( (params_com_direction != RX) & (params_com_direction != BOTH) ) break;
      default :
          ssSetOutputPortWidth( S, portIndex, NUM_OF_QBOTS );
          break;
  }

  if(portIndex != 4)
  {
      ssSetOutputPortWidth( S, portIndex, NUM_OF_QBOTS );
  }
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
// It performs initialization of qbots after the check of parameters.
//==============================================================================

#define MDL_START  // Change to #undef to remove function
#if     defined(MDL_START)
static void mdlStart( SimStruct *S )
{
    int8_T qbot_id;
    int try_counter;
    char aux_char;
    comm_settings comm_settings_t;

//=============================                           Check inputs integrity 

    if( (params_com_direction == TX) | (params_com_direction == BOTH) ){

        if ((REF_A_WIDTH > 1) && (REF_A_WIDTH != NUM_OF_QBOTS))
            return errorHandle(S, WIDTH_MISMATCH_A);    

        if ((REF_B_WIDTH > 1) && (REF_B_WIDTH != NUM_OF_QBOTS))    
            return errorHandle(S, WIDTH_MISMATCH_B);

        if ( !PARAM_ACTIVE_STARTUP_FCN && (REF_ACTIVATE_WIDTH != NUM_OF_QBOTS))
            return errorHandle(S, WIDTH_MISMATCH_ACTIVATE);
    }

//=============================     should an output handle appear in the block?

    if(params_daisy_chaining) 
        showOutputHandle(S);

//======================================     should initialization be evaluated?

    #if defined(_WIN32) || defined(_WIN64)
        if(in_handle == INVALID_HANDLE_VALUE) return;
    #else
        if(in_handle == -1) return;
    #endif 

    //RS485InitCommSettings(&comm_settings_t);
    comm_settings_t.file_handle = in_handle;

    if (PARAM_ACTIVE_STARTUP_FCN)
        activation(S, ON);       
    
    // Disable Activation on startup Flag and Setting ID
    mexEvalString(" set_param(gcb,'MaskEnables',{'off','on','on','off','off','off','off','off','on','on'})");

    if (NUM_OF_QBOTS > 255)
        return errorHandle(S, REACH_QBOTS_MAX);

    for (int i = NUM_OF_QBOTS; i--;){
        activation_state[i] = 0;

        qbot_id = params_qbot_id(i);

        qbot_id = qbot_id <= 0   ? 1    : qbot_id;  // inferior limit
        qbot_id = qbot_id >= 128 ? 127  : qbot_id;  // superior limit

        //commSetWatchDog(&comm_settings_t, qbot_id, PARAM_WDT_FCN);
    }

}
#endif /* MDL_START */

//==============================================================================
//                                                                    mdlOutputs
//==============================================================================
// Values are assigned to requested outputs in this function.
// Prime movers positions and output shaft's position are retrieved from I2C BUS
// for each qbot id specified in the appropriate parameter.
// Those values are copied to dedicated outputs.
// If daisy chaining is activated, also the pointer to the COM handle is output.
//==============================================================================

//==============================================================================
//                                                                     mdlUpdate
//==============================================================================
// This function is called once for every major integration time step.
// Input values are sent on I2C BUS in this function, if it is the case.
//==============================================================================

#define MDL_UPDATE  // Change to #undef to remove function
#if defined(MDL_UPDATE)
static void  mdlUpdate( SimStruct *S, int_T tid )
{
    double   auxa, auxb;
    char aux_char;
    int rx_tx; // Dynamic size of in_ref_activation

    // Measurements unity
    double meas_unity = 1;
    // Shalf Direction [1 Direction direction / -1 Inverse direction]
    double shalf_dir = 1;
    
    // Change Unity of Measurement
    switch(PARAM_UNITY_FCN){
        case DEGREES:
            meas_unity = DEG_TO_ANG;
            break;
        case RADIANTS:
            meas_unity = DEG_TO_ANG * RAD_TO_DEG;
            break;
        default: // TICK
            meas_unity = 1;
    }

    int16_T  refs[2];                           // auxiliary value
    int8_T  qbot_id;                            // qbot id's
    int16_T  ref_a, ref_b;                      // reference values (16 bits)
    int16_T measurements[5];

    int i;
    comm_settings comm_settings_t;

//=======================================     should this function be evaluated?

    #if defined(_WIN32) || defined(_WIN64)
        if(in_handle == INVALID_HANDLE_VALUE) return;
    #else
        if(in_handle == -1) return;
    #endif

//=============================     should an output handle appear in the block?

    if(params_daisy_chaining) 
        showOutputHandle(S);

//===========================================     sending command for each motor

    comm_settings_t.file_handle = in_handle;
    
    if( (params_com_direction == TX) || (params_com_direction == BOTH) ) 
        rx_tx = 3;
    else
        rx_tx = 1;

    // Activation after start up    
    if (!PARAM_ACTIVE_STARTUP_FCN){
        for (i = 0; i < NUM_OF_QBOTS; i++){
            if ((activation_state[i] == 0) && ((int) in_ref_activation(rx_tx)[i] != 0))
                    activation(S, ON, i);
            else{
                if ((activation_state[i] != 0) && ((int) in_ref_activation(rx_tx)[i] == 0))
                    activation(S, OFF, i);
            }
        }
        // Update old value

        for (i = 0; i < NUM_OF_QBOTS; i++)
            activation_state[i] = (int) in_ref_activation(rx_tx)[i];
    }
    

//==========================================     asking positions for each motor

    for(i = 0; i < NUM_OF_QBOTS; i++)
    {
//============================================================     qbot ID check

        qbot_id = params_qbot_id(i);

        if (qbot_id < 0){
            shalf_dir = -1;
            qbot_id = abs(qbot_id);
        }
        else
            shalf_dir = 1;


        qbot_id = qbot_id <= 0   ? 1    : qbot_id;  // inferior limit
        qbot_id = qbot_id >= 128 ? 127  : qbot_id;  // superior limit

        // Read Measurements

        if( (params_com_direction == RX) || (params_com_direction == BOTH) ){

            out_curr_1[i]     = dwork_out(i)[0];
            out_curr_2[i]     = dwork_out(i)[1];
            out_pos_a[i]      = dwork_out(i)[3];
            out_pos_b[i]      = dwork_out(i)[4];
            out_pos_link[i]   = dwork_out(i)[5];

            if(!commGetCurrAndMeas(&comm_settings_t, qbot_id, measurements))
            {
                out_pos_a[i]       = shalf_dir * ((double) measurements[2]) / meas_unity;
                out_pos_b[i]       = shalf_dir * ((double) measurements[3]) / meas_unity;
                out_pos_link[i]    = shalf_dir * ((double) measurements[4]) / meas_unity;
                out_curr_1[i]      = (int) measurements[0];
                out_curr_2[i]      = (int) measurements[1];
               
            } else
                out_debug[i] += 1;
            
            dwork_out(i)[0] = out_curr_1[i];
            dwork_out(i)[1] = out_curr_2[i];
            dwork_out(i)[2] = out_pos_a[i];
            dwork_out(i)[3] = out_pos_b[i];
            dwork_out(i)[4] = out_pos_link[i];

        }
        
        // Set References

        if( (params_com_direction == TX) || (params_com_direction == BOTH) ) {
            switch( params_qbot_mode )
            {
                case PRIME_MOVERS_POS:

                    auxa = (int)( (shalf_dir * in_ref_a[i >= REF_A_WIDTH ? REF_A_WIDTH - 1 : i] * meas_unity) );
                    auxb = (int)( (shalf_dir * in_ref_b[i >= REF_B_WIDTH ? REF_B_WIDTH - 1 : i] * meas_unity) );

                    refs[0] = (int16_T)( auxa );
                    refs[1] = (int16_T)( auxb );
                    
                    commSetInputs(&comm_settings_t, qbot_id, refs);
                    break;

                case EQ_POS_AND_PRESET: 

                    auxa = (int)( (in_ref_a[i >= REF_A_WIDTH ? REF_A_WIDTH - 1 : i] * meas_unity * shalf_dir) );
                    auxb = (int)( (in_ref_b[i >= REF_B_WIDTH ? REF_B_WIDTH - 1 : i] * meas_unity) );

                    if (auxb < 0) {
                        auxb = 0;
                    } else if (auxb > MAX_STIFF) {
                        auxb = MAX_STIFF;
                    }

                    if ((auxa + auxb) > MAX_POS) {
                        auxa = MAX_POS - auxb;
                    } else if ((auxa - auxb) < -MAX_POS) {
                        auxa = -MAX_POS + auxb;
                    }

                    refs[0] = (int16_T)( auxa + auxb);
                    refs[1] = (int16_T)( auxa - auxb);
                    
                    commSetInputs(&comm_settings_t, qbot_id, refs);
                    break;
                    
                case EQ_POS_AND_STIFF_PERC:

                    auxa = (int)( (in_ref_a[i >= REF_A_WIDTH ? REF_A_WIDTH - 1 : i] * meas_unity * shalf_dir) );
                    auxb = (int)( (in_ref_b[i >= REF_B_WIDTH ? REF_B_WIDTH - 1 : i] * PERC_TO_NUM) );

                    if (auxa > MAX_POS) {
                        auxa = MAX_POS;
                    } else if (auxa < -MAX_POS) {
                        auxa = -MAX_POS;
                    }
                    
                    if (auxb > 32767) {
                        auxb = 32767;
                    } else if (auxb <= 0) {
                        auxb = 0;
                    }
                    
                    refs[0] = (int16_T)(auxa);
                    refs[1] = (int16_T)(auxb);
                    
                    commSetPosStiff(&comm_settings_t, qbot_id, refs);
                    
                    break;
                default:
                    break;
            }
        }
    }
}
#endif /* MDL_UPDATE */

static void mdlOutputs( SimStruct *S, int_T tid ){ 
}

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

    activation(S, OFF);
    
    // Enable Activation on startup Flag and Setting ID
    mexEvalString(" set_param(gcb,'MaskEnables',{'on','on','on','off','off','off','off','on','on','on'})");

    closeRS485(&comm_settings_t);
}

//==============================================================================
//                                                              showOutputHandle
//==============================================================================
// TODO
//==============================================================================

void showOutputHandle( SimStruct *S )
{
    if((params_com_direction == TX) | (params_com_direction == NONE))
        out_handle_single   = (HANDLE *) &in_handle;    // appear in output 0
    if((params_com_direction == RX) | (params_com_direction == BOTH))
        out_handle_full     = (HANDLE *) &in_handle;    // appear in output 3
}

//==============================================================================
//                                                                    activation
//==============================================================================
// Function which activate or deactivate cubes [ON-Activate // OFF-Deactivate]
//==============================================================================

void activation(SimStruct *S, bool flag, const int ID){

    char aux_char;
    int aux;
    int start_count, end_count;
    int qbot_id;

    comm_settings comm_settings_t;
    comm_settings_t.file_handle = in_handle;

    if (flag == ON){
        aux_char = 0x00;
        aux = 3;
    }
    else{
        aux_char = 0x03;
        aux = 0;
    }

    // When ID is different -1 activate/disactivate all cubes
    // When ID is equal -1 activate/disactivate only ID cube

    if (ID != -1){
        start_count = ID;
        end_count = ID + 1;
    }else{
        start_count = 0;
        end_count = NUM_OF_QBOTS;
    }

    // General activate/disactivate function


    for(int i = start_count; i < end_count; i++)
    {
        qbot_id = abs((int) params_qbot_id(i));
        if (flag == ON)
            ssPrintf("Activating cube ID %d: ", (int) qbot_id);
        else 
            ssPrintf("Dectivating cube ID %d: ", (int) qbot_id);

        for (int try_counter = 0; try_counter < 5; try_counter++) {
            ssPrintf("%d ", (try_counter + 1));
            
            commActivate(&comm_settings_t, qbot_id, aux);
            usleep(1000);
            commGetActivate(&comm_settings_t, qbot_id, &aux_char);
            
            if ( ((flag == ON) && (aux_char == 0x03)) || ((flag == OFF) && (aux_char == 0x00)) ) {
                ssPrintf("DONE\n");
                break;
            }
            else
                usleep(10000);

        }
        
        if ((flag == ON) && (aux_char != 0x03)) 
            ssPrintf("Unable to activate\n");
        else
            if ((flag == OFF) && (aux_char != 0x00)) 
                ssPrintf("Unable to deactivate\n");
  
    }
}

//==============================================================================
//                                                                   errorHandle
//==============================================================================
// Error handle 
//==============================================================================

void errorHandle(SimStruct *S, const int error){

    switch(error){
        case WIDTH_MISMATCH_A:
            ssSetErrorStatus(S, "[ERROR] First input size mismatch");
            break;
        case WIDTH_MISMATCH_B:
            ssSetErrorStatus(S, "[ERROR] Second input size mismatch");
            break;
        case WIDTH_MISMATCH_ACTIVATE:
            ssSetErrorStatus(S, "[ERROR] Activation input size mismatch");
            break;
        case REACH_QBOTS_MAX:
            ssSetErrorStatus(S, "[ERROR] Max number of QBOTs available reached");
            break;
        default:
            ssSetErrorStatus(S, "[ERROR] Generic");
            break;
    }

    mdlTerminate(S);
    
    return;
}

//==============================================================================
//                                                   Required S-function trailer
//==============================================================================

#ifdef  MATLAB_MEX_FILE    // Is this file being compiled as a MEX-file?
#include "simulink.c"      // MEX-file interface mechanism
#else
#include "cg_sfun.h"       // Code generation registration function
#endif