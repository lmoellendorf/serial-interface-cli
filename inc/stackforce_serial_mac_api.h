#ifdef __cplusplus
extern "C"
{
#endif
/*! ============================================================================
 *
 * @file:    stackforce_serial_mac_api.h
 *
 * @date:    08.12.2014
 * @author:  © by STACKFORCE, Heitersheim, Germany, http://www.stackforce.de
 * @author:  Lars Möllendorf
 *
 * @brief:   Sample header of the source code
 *
 * @version: 
 *
 =============================================================================*/

#ifndef __STACKFORCE_SERIAL_MAC_API_H_
#define __STACKFORCE_SERIAL_MAC_API_H_
#ifndef __DECL_STACKFORCE_SERIAL_MAC_API_H_
#define __DECL_STACKFORCE_SERIAL_MAC_API_H_ extern
#endif

/*==============================================================================
 |                               INCLUDE FILES
 =============================================================================*/

#include <stdint.h>
#include <sys/types.h>

/******************************************************************************/
/*! @defgroup STACKFORCE_SERIAL_MAC_API TODO: STACKFORCE Example description
 This section describes the STACKFORCE Example module.
 This group will always be the "root group" of the software module.
 A text describing the group. This text will be displayed at the beginning
 of the chapter. Preferably, the description should always be located here
 and not at the "@addtogroup" tag in order to easily find the text add
 some more description.
 @{  */

/******************************************************************************/
/*! @defgroup STACKFORCE_SERIAL_MAC_API_COMPILE_TIME_SET
 TODO: STACKFORCE Example compile time settings
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the compile time settings for this STACKFORCE Example
 implementation.
 */

/*! @addtogroup STACKFORCE_SERIAL_MAC_API_COMPILE_TIME_SET
 @{ */

/*!@} end of STACKFORCE_SERIAL_MAC_API_COMPILE_TIME_SET */

/******************************************************************************/
/*!
 @defgroup STACKFORCE_SERIAL_MAC_API_MACROS TODO: STACKFORCE Example macros
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the macros used by the STACKFORCE Example implementation.
 */

/*! @addtogroup STACKFORCE_SERIAL_MAC_API_MACROS
 *  @{ */

/*!@} end of STACKFORCE_SERIAL_MAC_API_MACROS */

/******************************************************************************/
/*!
 @defgroup STACKFORCE_SERIAL_MAC_API_ENUMS TODO: STACKFORCE Example enums
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the enums used by the STACKFORCE Example implementation.
 */

/*! @addtogroup STACKFORCE_SERIAL_MAC_API_ENUMS
 *  @{ */

/*!@} end of STACKFORCE_SERIAL_MAC_API_ENUMS */

/******************************************************************************/
/*!
 @defgroup STACKFORCE_SERIAL_MAC_API_TYPEDEFS STACKFORCE
 TODO: Example type definitions
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the type definitions used by the STACKFORCE Example
 implementation.
 */
/*! @addtogroup STACKFORCE_SERIAL_MAC_API_STRUCTS
 *  @{ */

typedef ssize_t (*SF_SERIAL_MAC_HAL_RX_FUNC)(void *serial_ctx,
        uint8_t *frameBuffer, size_t frameBufferLength);
typedef ssize_t (*SF_SERIAL_MAC_HAL_TX_FUNC)(void *serial_ctx,
        uint8_t *frameBuffer, size_t frameBufferLength);

typedef void (*SF_SERIAL_MAC_READ_EVT)(uint8_t *frameBuffer,
        size_t frameBufferLength);
typedef void (*SF_SERIAL_MAC_WRITE_EVT)(uint8_t *frameBuffer,
        size_t frameBufferLength);

/*!@} end of STACKFORCE_SERIAL_MAC_API_STRUCTS */

/******************************************************************************/
/*! @defgroup STACKFORCE_SERIAL_MAC_API_STRUCTS STACKFORCE TODO: Example structures
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the structures used by the STACKFORCE Example
 implementation.
 */
/*! @addtogroup STACKFORCE_SERIAL_MAC_API_STRUCTS
 *  @{ */

typedef struct sf_serial_mac_buffer
{
    uint8_t *buffer;
//    uint8_t *currentPosition;
    size_t length;
    size_t byteSent;
} BUF;

typedef struct sf_serial_mac_ctx
{
    SF_SERIAL_MAC_HAL_RX_FUNC rx;
    BUF *rxBuffer;
    SF_SERIAL_MAC_HAL_TX_FUNC tx;
    BUF *txBuffer;
    SF_SERIAL_MAC_READ_EVT read;
    BUF *readBuffer;
    SF_SERIAL_MAC_WRITE_EVT write;
    BUF *writeBuffer;
    void *halCtx;
} SF_SERIAL_MAC_CTX;

/*!@} end of STACKFORCE_SERIAL_MAC_API_STRUCTS */

/******************************************************************************/
/*! @defgroup STACKFORCE_SERIAL_MAC_API_GLOBALS TODO: STACKFORCE Example global variables
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the global variables used by the STZEDN Example
 implementation.
 */
/*! @addtogroup STACKFORCE_SERIAL_MAC_API_GLOBALS
 *  @{ */

/*!@} end of STACKFORCE_SERIAL_MAC_API_GLOBALS */

/******************************************************************************/
/*! @defgroup STACKFORCE_SERIAL_MAC_API_API TODO: STACKFORCE Example API
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the API for the STACKFORCE Example implementation.
 */

/*! @addtogroup STACKFORCE_SERIAL_MAC_API_API
 *  @{ */

SF_SERIAL_MAC_CTX *sf_serial_mac_init(SF_SERIAL_MAC_CTX *ctx, void *halCtx,
        SF_SERIAL_MAC_HAL_RX_FUNC rx, SF_SERIAL_MAC_HAL_TX_FUNC tx,
        SF_SERIAL_MAC_READ_EVT readEvt, SF_SERIAL_MAC_WRITE_EVT writeEvt);

int sf_serial_mac_enqueFrame(SF_SERIAL_MAC_CTX *ctx, uint8_t *frameBuffer,
        size_t frameBufferLength);

int sf_serial_mac_entry(SF_SERIAL_MAC_CTX *ctx);

/*!@} end of STACKFORCE_SERIAL_MAC_API_API */

/*!@} end of STACKFORCE_SERIAL_MAC_API */
/******************************************************************************/
#endif /* STACKFORCE_SERIAL_MAC_API_H_ */
#ifdef __cplusplus
}
#endif
