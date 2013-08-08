// ----------------------------------------------------------------------------
// Copyright (C)  qbrobotics. All rights reserved.
// www.qbrobotics.com
// ----------------------------------------------------------------------------
// File:		qbotISS_ISS.cpp
//
// Description: Communication s-function for the QB Move. To be used with RS-485 
//				on a Virtual COM.
// 
// Soft. Ver:			0.1 beta 1
// Date:                Dec 29, 2011
// 

//==============================================================================
//															   main	 definitions
//==============================================================================

#define S_FUNCTION_NAME	 qbmove
#define S_FUNCTION_LEVEL 2

//==============================================================================
//                                                                      includes
//==============================================================================

#include "simstruc.h"
#include "qbmoveAPI/qbmove_communications.h"
// #include <windows.h>

//==============================================================================
//																	 definitions
//==============================================================================

//===============================================================	  parameters

#define params_qbot_id(i)	  ( mxGetPr( ssGetSFcnParam( S, 0 ) )[ \
									 i >= NUM_OF_QBOTS ? NUM_OF_QBOTS -1 : i ] )
#define params_com_direction	  ( (int)mxGetScalar( ssGetSFcnParam( S, 1 ) ) )
#define params_qbot_mode		  ( (int)mxGetScalar( ssGetSFcnParam( S, 2 ) ) )
#define params_daisy_chaining  ( (bool)mxGetScalar( ssGetSFcnParam( S, 3 ) ) )
#define params_angle_range(i)  ( mxGetPr( ssGetSFcnParam( S, 4 ) )[ \
							  i >= ANG_RANGE_WIDTH ? ANG_RANGE_WIDTH - 1 : i ] )
#define params_sw_lim_range(i) ( mxGetPr( ssGetSFcnParam(S, 5) )[ \
									 i >= SW_LIM_WIDTH ? SW_LIM_WIDTH - 1 : i] )
#define params_joint_offset(i) ( mxGetPr( ssGetSFcnParam(S, 6) )[\
									 i >= OFFSET_WIDTH ? OFFSET_WIDTH - 1 : i] )

//===================================================================	  inputs

#define in_handle ( *(const HANDLE* *)ssGetInputPortSignal( S, 0 ) )[0]
#define in_ref_a  ( (const real_T *)ssGetInputPortSignal(	S, 1 ) )
#define in_ref_b  ( (const real_T *)ssGetInputPortSignal(	S, 2 ) )

//==================================================================	 outputs

#define out_pos_a		  ( ssGetOutputPortRealSignal		( S, 0 ) )
#define out_pos_b		  ( ssGetOutputPortRealSignal		( S, 1 ) )
#define out_pos_link	  ( ssGetOutputPortRealSignal		( S, 2 ) )
#define out_debug		  ( ssGetOutputPortRealSignal		( S, 3 ) )
#define out_handle_single ( (HANDLE* *)ssGetOutputPortSignal( S, 0 ) )[0]
#define out_handle_full	  ( (HANDLE* *)ssGetOutputPortSignal( S, 3 ) )[0]

//==================================================================	  dworks
#define dwork_out(i)	  ( (real_T *)ssGetDWork( S, i ) )

//================================================================	   constants

#define BUFFER_SIZES 			15
#define ANG_TO_DEG  			1
#define DEG_TO_ANG				1

//=============================================================		enumerations

enum	QBOT_MODE { PRIME_MOVERS_POS	= 1, EQ_POS_AND_PRESET	= 2 };
enum	COMM_DIRS { RX = 1, TX = 2, BOTH = 3, NONE = 4 };

//===================================================================	  macros

#define NUM_OF_QBOTS	( (int)mxGetNumberOfElements( ssGetSFcnParam( S, 0 ) ) )
#define ANG_RANGE_WIDTH	( (int)mxGetNumberOfElements( ssGetSFcnParam( S, 4 ) ) )
#define SW_LIM_WIDTH	( (int)mxGetNumberOfElements( ssGetSFcnParam( S, 5 ) ) )
#define OFFSET_WIDTH	( (int)mxGetNumberOfElements( ssGetSFcnParam( S, 6 ) ) )
#define REF_A_WIDTH		ssGetInputPortWidth( S, 1 )
#define REF_B_WIDTH		ssGetInputPortWidth( S, 2 )
#define SIGN(x)			( ( (x) < 0) ? -1 : ( (x) > 0 ) )

//==============================================================================
//															 function prototypes
//==============================================================================

unsigned char checksum_	( unsigned char * buf, int size );
void	showOutputHandle( SimStruct *S );
// unsigned int verifychecksum_( unsigned char * buffer );

//==============================================================================
//															  mdlInitializeSizes
//==============================================================================
// The sizes information is used by Simulink to determine the S-function block's
// characteristics (number of inputs, outputs, states, etc.).
//==============================================================================

static void mdlInitializeSizes( SimStruct *S )
{	
	int_T	status;				   // for new type definition
	DTypeId COM_HANDLE_id;		   // for new type definition
	HANDLE	handle_aux;	           // for new type definition
	int i;                         // for cycles

//======================================================	 new type definition

	COM_HANDLE_id = ssRegisterDataType( S, "COM_HANDLE" );
	if( COM_HANDLE_id == INVALID_DTYPE_ID ) return;
	status = ssSetDataTypeSize( S, COM_HANDLE_id, sizeof(handle_aux) );
	if( status == 0)  return;
	status = ssSetDataTypeZero( S, COM_HANDLE_id, &handle_aux );
	if( status == 0 ) return;

//===================================================================	  states

	ssSetNumContStates( S, 0 );
	ssSetNumDiscStates( S, 1 );

//===============================================================	  parameters

	ssSetNumSFcnParams( S, 7 );	// 7 parameters:
								//	  - qbot I2C id
								//		- comm. direction: rx/tx/both
								//		- qbot mode: q1-q2 or qs-qd
								//		- daisy chaining
								//		- angle range
								//	  - software limit on equilibrium position
								//	  - joint offset for equilibrium position

//===================================================================	  inputs

	switch(params_com_direction)
	{
		case TX :
		case BOTH :
			if ( !ssSetNumInputPorts( S, 3 ) ) return;
		break;
		case RX :
		default :
			if ( !ssSetNumInputPorts( S, 1 ) ) return;
		break;
	}

/////////////////////////////////////// 0 ) pointer to HANDLE	////////////////
	ssSetInputPortWidth				( S, 0, DYNAMICALLY_SIZED );
	ssSetInputPortDataType			( S, 0, COM_HANDLE_id 	  );
	ssSetInputPortDirectFeedThrough	( S, 0, 1				  );
	ssSetInputPortRequiredContiguous( S, 0, 1				  );


	// IF transmitting only OR both tx and rx
	if( (params_com_direction == TX) | (params_com_direction == BOTH) )
	{
//////////////////////////////////////// 1 ) reference A	////////////////////
		ssSetInputPortWidth				( S, 1, DYNAMICALLY_SIZED );
		ssSetInputPortDataType			( S, 1, SS_DOUBLE		  );
		ssSetInputPortDirectFeedThrough	( S, 1, 1				  );
		ssSetInputPortRequiredContiguous( S, 1, 1				  );

//////////////////////////////////////// 2 ) reference B	////////////////////
		ssSetInputPortWidth				( S, 2, DYNAMICALLY_SIZED );
		ssSetInputPortDataType			( S, 2, SS_DOUBLE		  );
		ssSetInputPortDirectFeedThrough ( S, 2, 1				  );
		ssSetInputPortRequiredContiguous( S, 2, 1				  );
	}

//==================================================================	 outputs

	// IF transmitting only OR no communication
	if( (params_com_direction == TX) | (params_com_direction == NONE) )
	{	 // IF daisy chaining activated
		if(params_daisy_chaining)
		{
			if (!ssSetNumOutputPorts(S, 1)) return;

///////////////////////////////// 0 ) com handle	////////////////////////////
			ssSetOutputPortWidth	( S, 0, 1			  );
			ssSetOutputPortDataType ( S, 0, COM_HANDLE_id );
		}
		else
		{
	  if (!ssSetNumOutputPorts(S, 0)) return;
		}
	}

	// IF receiving only OR both tx and rx
	if((params_com_direction == RX) | (params_com_direction == BOTH))
	{	 // IF daisy chaining activated
		if(params_daisy_chaining)
		{
			if (!ssSetNumOutputPorts(S, 5)) return;

///////////////////////////////// 3 ) com handle	////////////////////////////
			ssSetOutputPortWidth   ( S, 4, 1			 );
			ssSetOutputPortDataType( S, 4, COM_HANDLE_id );
		}
		else
		{
			if (!ssSetNumOutputPorts(S, 4)) return;
		}

//////////////////////////////// 0 ) position A	  //////////////////////////////
		ssSetOutputPortWidth	( S, 0, NUM_OF_QBOTS );
		ssSetOutputPortDataType ( S, 0, SS_DOUBLE	 );

//////////////////////////////// 1 ) position B	  //////////////////////////////
		ssSetOutputPortWidth	( S, 1, NUM_OF_QBOTS );
		ssSetOutputPortDataType ( S, 1, SS_DOUBLE	 );

//////////////////////////////// 2 ) position link	  //////////////////////////
		ssSetOutputPortWidth	( S, 2, NUM_OF_QBOTS );
		ssSetOutputPortDataType ( S, 2, SS_DOUBLE	 );
		
//////////////////////////////// 3 ) debug integer	  //////////////////////////
		ssSetOutputPortWidth	( S, 3, NUM_OF_QBOTS );
		ssSetOutputPortDataType ( S, 3, SS_DOUBLE	 );
	}

//=============================================================		sample times

	ssSetNumSampleTimes(S, 1);

//=============================================================		work vectors

	ssSetNumDWork(S, NUM_OF_QBOTS);		// 0 dwork vector elements
	ssSetNumRWork(S, 0);				// 0 real work vector elements
	ssSetNumIWork(S, 0);				// 0 work vector elements
	ssSetNumPWork(S, 0);				// 0 pwork vector elements:
	ssSetNumModes(S, 0);				// 0 mode work vector elements
	ssSetNumNonsampledZCs(S, 0);		// 0 nonsampled zero crossings
	
	for( i = 0; i < NUM_OF_QBOTS; i++)
	{
		ssSetDWorkWidth(S, i, 4);
		ssSetDWorkDataType(S, i, SS_DOUBLE);
	}

//===================================================================	  others

	ssSetOptions(S, SS_OPTION_ALLOW_INPUT_SCALAR_EXPANSION);
	
//=======================================================	  mask modifications

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
	}
}

//==============================================================================
//															mdlSetInputPortWidth
//==============================================================================
// This method is called with the candidate width for a dynamically sized port.
// If the proposed width is acceptable, the actual port width is set using
// ssSetInputPortWidth.
//==============================================================================

#undef MDL_SET_INPUT_PORT_WIDTH	  // Change to #undef to remove function
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
//														   mdlSetOutputPortWidth
//==============================================================================
// This method is called with the candidate width for a dynamically sized port.
// If the proposed width is acceptable, the actual port width is set using
// ssSetOutputPortWidth.
//==============================================================================

#undef MDL_SET_OUTPUT_PORT_WIDTH   // Change to #undef to remove function
#if defined(MDL_SET_OUTPUT_PORT_WIDTH) && defined(MATLAB_MEX_FILE)
static void mdlSetOutputPortWidth( SimStruct *S, int portIndex, int width )
{
  switch( portIndex )
  {
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
//														mdlInitializeSampleTimes
//==============================================================================
// This function is used to specify the sample time(s) for your S-function.
// o  The sample times are specified as pairs "[sample_time, offset_time]" via
//	  the following macros:
//	  ssSetSampleTime(S, sampleTimePairIndex, sample_time)
//	  ssSetOffsetTime(S, offsetTimePairIndex, offset_time)
//	  Where sampleTimePairIndex starts at 0.
// o  A discrete function that changes at a specified rate should register the
//	  discrete sample time pair
//			[discrete_sample_period, offset]
//	  where
//			discrete_sample_period > 0.0
//	  and
//			0.0 <= offset < discrete_sample_period
//==============================================================================

static void mdlInitializeSampleTimes( SimStruct *S )
{
	ssSetSampleTime(S, 0, INHERITED_SAMPLE_TIME);
	ssSetOffsetTime(S, 0, 0.0);
}

//==============================================================================
//																		mdlStart
//==============================================================================
// This function is called once at start of model execution.
// It performs initialization of qbots after the check of parameters.
//==============================================================================

#define MDL_START  // Change to #undef to remove function
#if		defined(MDL_START)
static void mdlStart( SimStruct *S )
{
    int i;
    comm_settings comm_settings_t;    
    
//=============================		should an output handle appear in the block?

	if(params_daisy_chaining) showOutputHandle(S);

//======================================	 should initialization be evaluated?

    #if defined(_WIN32) || defined(_WIN64)
        if(in_handle == INVALID_HANDLE_VALUE) return;	 
    #else
        if(in_handle == -1) return;
    #endif

    RS485InitCommSettings(&comm_settings_t);
    comm_settings_t.file_handle = in_handle;	    

	for(i = 0; i < NUM_OF_QBOTS; i++)
	{
        commActivate(&comm_settings_t, params_qbot_id(i), 3);
    }

}
#endif /* MDL_START */

//==============================================================================
//																	  mdlOutputs
//==============================================================================
// Values are assigned to requested outputs in this function.
// Prime movers positions and output shaft's position are retrieved from I2C BUS
// for each qbot id specified in the appropriate parameter.
// Those values are copied to dedicated outputs.
// If daisy chaining is activated, also the pointer to the COM handle is output.
//==============================================================================

static void mdlOutputs( SimStruct *S, int_T tid )
{	
    short int measurements[3];
	uint8_T qbot_id;		 						// qbot id's
    comm_settings comm_settings_t;
	
	int i;

//=============================		should an output handle appear in the block?

	if(params_daisy_chaining) showOutputHandle(S);

//====================================================	   should we keep going?

    #if defined(_WIN32) || defined(_WIN64)
        if(in_handle == INVALID_HANDLE_VALUE) return;	 
    #else
        if(in_handle == -1) return;
    #endif


	if( (params_com_direction != RX) & (params_com_direction != BOTH) ) return;

//==========================================	 asking positions for each motor

    RS485InitCommSettings(&comm_settings_t);
    comm_settings_t.file_handle = in_handle;

	for(i = 0; i < NUM_OF_QBOTS; i++)
	{
//============================================================      qbot ID check

     qbot_id = params_qbot_id(i);
     qbot_id = qbot_id <= 0   ? 1    : qbot_id;  // inferior limit
     qbot_id = qbot_id >= 128 ? 127  : qbot_id;  // superior limit
	 

     out_debug[i] = 1;
     out_pos_a[i]    = dwork_out(i)[0];
     out_pos_b[i]    = dwork_out(i)[1];
     out_pos_link[i] = dwork_out(i)[2];
     
     if(!commGetMeasurements(&comm_settings_t, qbot_id, measurements))
     {
         out_pos_a[i]       = ANG_TO_DEG * (double) measurements[0];
         out_pos_b[i]       = ANG_TO_DEG * (double) measurements[1];
         out_pos_link[i]    = ANG_TO_DEG * (double) measurements[2];
     
     
         dwork_out(i)[0] = out_pos_a[i];
         dwork_out(i)[1] = out_pos_b[i];
         dwork_out(i)[2] = out_pos_link[i];
         out_debug[i] = 0;
     }
    }

}


//==============================================================================
//																	   mdlUpdate
//==============================================================================
// This function is called once for every major integration time step.
// Input values are sent on I2C BUS in this function, if it is the case.
//==============================================================================

#define MDL_UPDATE	// Change to #undef to remove function
#if defined(MDL_UPDATE)
static void mdlUpdate( SimStruct *S, int_T tid )
{
	double	 aux;
	int16_T	 refs[2];					 				// auxiliary value
	uint8_T  qbot_id;			 				// qbot id's
	int16_T  ref_a, ref_b;		 				// reference values (16 bits)
	int i;
    comm_settings comm_settings_t;
	
//=======================================	  should this function be evaluated?

    #if defined(_WIN32) || defined(_WIN64)
        if(in_handle == INVALID_HANDLE_VALUE) return;	 
    #else
        if(in_handle == -1) return;
    #endif

	if( (params_com_direction != TX) & (params_com_direction != BOTH) ) return;

//===========================================	  sending command for each motor

    RS485InitCommSettings(&comm_settings_t);
    comm_settings_t.file_handle = in_handle;

	for(i = 0; i < NUM_OF_QBOTS; i++)
	{

//============================================================	   qbot ID check

		qbot_id = params_qbot_id(i);
		qbot_id = qbot_id <= 0	 ? 1	: qbot_id;	// inferior limit
		qbot_id = qbot_id >= 128 ? 127	: qbot_id;	// superior limit

//==============================================	 input references convertion
// - if input reference width is to small, use the last value
// - check minimum and maximum limits
// - finish conversion to a 16 bits word

////////////////////////////////   reference A	 ///////////////////////////////
		aux = in_ref_a[i >= REF_A_WIDTH ? REF_A_WIDTH - 1 : i];
////////////////////////////////   adding offset to eq.pos. only	////////////

		refs[0]  = (int16_T)( aux );
		
//====================================================================		TODO
//														  Insert here the use of
//														  params_sw_lim_range
//														  and
//														  params_joint_offset.

////////////////////////////////   reference B	 ///////////////////////////////

		aux = in_ref_b[i >= REF_B_WIDTH ? REF_B_WIDTH - 1 : i];
		ref_b = (int16_T)( aux );
		refs[1] = (int16_T)( aux );

        commSetInputs(&comm_settings_t, qbot_id, refs);

	}
}
#endif /* MDL_UPDATE */

//==============================================================================
//																	mdlTerminate
//==============================================================================
// In this function, you should perform any actions that are necessary at the
// termination of a simulation.
//==============================================================================

static void mdlTerminate( SimStruct *S )
{
}

//==============================================================================
//																showOutputHandle
//==============================================================================
// TODO
//==============================================================================

void showOutputHandle( SimStruct *S )
{
	if((params_com_direction == TX) | (params_com_direction == NONE))
		out_handle_single	= (HANDLE *) &in_handle;	// appear in output 0
	if((params_com_direction == RX) | (params_com_direction == BOTH))
		out_handle_full		= (HANDLE *) &in_handle;	// appear in output 3	
}

//==============================================================================
//													 Required S-function trailer
//==============================================================================

#ifdef	MATLAB_MEX_FILE	   // Is this file being compiled as a MEX-file?
#include "simulink.c"	   // MEX-file interface mechanism
#else
#include "cg_sfun.h"	   // Code generation registration function
#endif