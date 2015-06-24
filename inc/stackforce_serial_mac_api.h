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
/*! @defgroup STACKFORCE_SERIAL_MAC_API_STRUCTS STACKFORCE TODO: Example structures
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the structures used by the STACKFORCE Example
 implementation.
 */
/*! @addtogroup STACKFORCE_SERIAL_MAC_API_STRUCTS
 *  @{ */

struct sf_serial_mac_ctx;

/*!@} end of STACKFORCE_SERIAL_MAC_API_STRUCTS */

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

/**
 * Signature of HAL's read function to be used by the MAC for RX.
 */
typedef ssize_t (*SF_SERIAL_MAC_HAL_READ_FUNC)(void *portHandle,
        void *frameBuffer, size_t frameBufferLength);
/**
 * Signature of HAL's write function to be used by the MAC for TX.
 */
typedef ssize_t (*SF_SERIAL_MAC_HAL_WRITE_FUNC)(void *portHandle,
        const void *frameBuffer, size_t frameBufferLength);
/**
 * Signature of APP's callback function to be called by the MAC
 * when a whole frame has been received.
 */
typedef void (*SF_SERIAL_MAC_READ_EVT)(const char *frameBuffer,
                                       size_t frameBufferLength);
/**
 * Signature of APP's callback function to be called by the MAC
 * when a whole frame has been sent.
 */
typedef void (*SF_SERIAL_MAC_WRITE_EVT)(size_t byteWritten);

typedef enum sf_serial_mac_return
{
    SF_SERIAL_MAC_SUCCESS = 0,
    /** Null pointer exception */
    SF_SERIAL_MAC_ERROR_NPE,
    /**
     * A previously assigned TX buffer is still processed.
     * Retry later.
     */
    SF_SERIAL_MAC_ERROR_TX_PENDING,
    /**
     * A previously started frame is still processed.
     * Wait for SF_SERIAL_MAC_WRITE_EVT before starting a new frame.
     */
    SF_SERIAL_MAC_ERROR_FRM_PENDING,
    /** The HAL reports an error. */
    SF_SERIAL_MAC_ERROR_HAL_ERROR,
} SF_SERIAL_MAC_RETURN;

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

size_t sf_serial_mac_ctx_size(void);

void* sf_serial_mac_init(struct sf_serial_mac_ctx *ctx,
                         void *portHandle, SF_SERIAL_MAC_HAL_READ_FUNC rx,
                         SF_SERIAL_MAC_HAL_WRITE_FUNC tx, SF_SERIAL_MAC_READ_EVT readEvt,
                         SF_SERIAL_MAC_WRITE_EVT writeEvt, SF_SERIAL_MAC_WRITE_EVT bufTxEvt);

SF_SERIAL_MAC_RETURN sf_serial_mac_txFrameStart(struct sf_serial_mac_ctx *ctx,
        size_t frmLen);

SF_SERIAL_MAC_RETURN sf_serial_mac_txFrameAppend(struct sf_serial_mac_ctx *ctx,
        const char *frmBufLoc,
        size_t frmBufSize);

void* sf_serial_mac_rxFrame(struct sf_serial_mac_ctx *ctx, char *frmBufLoc,
                            size_t frmBufSize);

void* sf_serial_mac_halTxCb(struct sf_serial_mac_ctx *ctx);
void* sf_serial_mac_halRxCb(struct sf_serial_mac_ctx *ctx);

void* sf_serial_mac_entry(struct sf_serial_mac_ctx *ctx);

/*!@} end of STACKFORCE_SERIAL_MAC_API_API */

/*!@} end of STACKFORCE_SERIAL_MAC_API */
/******************************************************************************/
#endif /* STACKFORCE_SERIAL_MAC_API_H_ */
#ifdef __cplusplus
}
#endif
