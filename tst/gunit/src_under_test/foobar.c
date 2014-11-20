#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*/
/*! \file   foobar.c

    \author

    \brief  Sample source code to be tested

   \version
==============================================================================*/

/*==============================================================================
                                 INCLUDE FILES
==============================================================================*/
#include "stzedn_typedefs.h"
#include "../src_extern/foobar_extern.h"

/*==============================================================================
                           FUNCTIONS
==============================================================================*/
uint16_t foo_sum(uint8_t* pc_array, uint8_t c_length)
{
	uint16_t i_return = 0U;
	uint8_t i;
	for(i=0U; i<c_length; i++)
	{
		i_return += (uint16_t)*pc_array;
		pc_array++;
	}
	return i_return;
} /* foo_sum() */

void foo_createFibo(uint8_t* pc_array, uint8_t c_num)
{
	uint8_t i;
	for(i=0U; i<c_num; i++)
	{
		if(i < 2)
		{
			*pc_array = 1U;
			pc_array++;
		}
		else
		{
			*pc_array = *(pc_array - 1U) + *(pc_array - 2U);
			pc_array++;
		}
	}
} /* foo_createFibo() */

uint8_t foo_getNumber(uint8_t c_seed)
{
	return foo_extern_getNumber(c_seed);
} /* foo_getNumber() */

#ifdef __cplusplus
}
#endif
