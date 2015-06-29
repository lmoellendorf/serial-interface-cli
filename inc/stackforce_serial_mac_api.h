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
/*! @defgroup STACKFORCE_SERIAL_MAC_API STACKFORCE Serial MAC Module
 This section describes the STACKFORCE Example module.
 This group will always be the "root group" of the software module.
 A text describing the group. This text will be displayed at the beginning
 of the chapter. Preferably, the description should always be located here
 and not at the "@addtogroup" tag in order to easily find the text add
 some more description.
 @{  */

/* ****************************************************************************/
/* ! @defgroup STACKFORCE_SERIAL_MAC_API_COMPILE_TIME_SET Compile time settings
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the compile time settings for this STACKFORCE Example
 implementation.
 */

/* ! @addtogroup STACKFORCE_SERIAL_MAC_API_COMPILE_TIME_SET
 @{ */

/* !@} end of STACKFORCE_SERIAL_MAC_API_COMPILE_TIME_SET */

/* *****************************************************************************/
/* !
 @defgroup STACKFORCE_SERIAL_MAC_API_MACROS Macros
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the macros used by the STACKFORCE Example implementation.
 */

/* ! @addtogroup STACKFORCE_SERIAL_MAC_API_MACROS
 *  @{ */

/* !@} end of STACKFORCE_SERIAL_MAC_API_MACROS */

/* *****************************************************************************/
/* !
 @defgroup STACKFORCE_SERIAL_MAC_API_ENUMS Enumerations
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the enums used by the STACKFORCE Example implementation.
 */

/* ! @addtogroup STACKFORCE_SERIAL_MAC_API_ENUMS
 *  @{ */


/* !@} end of STACKFORCE_SERIAL_MAC_API_ENUMS */

/******************************************************************************/
/*! @defgroup STACKFORCE_SERIAL_MAC_API_STRUCTS Structures
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the structures used by the STACKFORCE serial MAC.
 */
/*! @addtogroup STACKFORCE_SERIAL_MAC_API_STRUCTS
 *  @{ */

/**
 * Structure used by the MAC to store its context.
 */
struct sf_serial_mac_ctx;

/*! @} end of STACKFORCE_SERIAL_MAC_API_STRUCTS */

/******************************************************************************/
/*!
 @defgroup STACKFORCE_SERIAL_MAC_API_TYPEDEFS Type definitions
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the type definitions provided and used by the STACKFORCE
 serial MAC.
 */
/*! @addtogroup STACKFORCE_SERIAL_MAC_API_TYPEDEFS
 *  @{ */

/**
 * Signature of HAL's read function to be used by the MAC for RX.
 */
typedef ssize_t (*SF_SERIAL_MAC_HAL_READ_FUNC)(void *portHandle,
        void *frameBuffer, size_t frameBufferLength);
/**
 * Signature of HAL's write function to be used by the MAC for TX.
 *
 * @param portHandle Points to the port handle that may be needed by the HAL.
 * @param frameBuffer Points to the buffer that has to be written.
 * @param frameBufferLength Length of the buffer to be written in bytes.
 * @return Number of bytes successfully written.
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
 *
 * @param byteSent Number of bytes sent with the frame.
 */
typedef void (*SF_SERIAL_MAC_WRITE_EVT)(size_t byteSent);

/**
 * Return values.
 */
typedef enum sf_serial_mac_return
{
    /** No error occured (always equals 0). */
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
     * Wait for SF_SERIAL_MAC_WRITE_EVT() before starting a new frame.
     */
    SF_SERIAL_MAC_ERROR_FRM_PENDING,
    /** The HAL reports an error. */
    SF_SERIAL_MAC_ERROR_HAL_ERROR,
} SF_SERIAL_MAC_RETURN;

/*! @} end of STACKFORCE_SERIAL_MAC_API_TYPEDEFS */

/* *****************************************************************************/
/* ! @defgroup STACKFORCE_SERIAL_MAC_API_GLOBALS Global variables
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the global variables.
 */
/* ! @addtogroup STACKFORCE_SERIAL_MAC_API_GLOBALS
 *  @{ */

/* !@} end of STACKFORCE_SERIAL_MAC_API_GLOBALS */

/******************************************************************************/
/*! @defgroup STACKFORCE_SERIAL_MAC_API_FUNCTIONS Functions
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the functions provided by the STACKFORCE serial MAC.
 */

/*! @addtogroup STACKFORCE_SERIAL_MAC_API_FUNCTIONS
 *  @{ */

/**
 * @return The size of the MAC context structure.
 */
size_t sf_serial_mac_ctx_size(void);

/**
 * Initialization of STACKFORCE Serial MAC.
 *
 * This function must be called once before the MAC can be used and may be called
 * whenever the MAC should be reset.
 *
 * @param ctx Points to the memory region the MAC can use for its context. Please use
 * sf_serial_mac_ctx_size() to get the memory size needed by the MAC context.
 * @param portHandle Points to the handle of the serial port which is passed to the underlying HAL.
 * @param rx Read function of the underlying HAL.
 * @param tx Write function of the underlying HAL.
 * @param readEvt Callback function to be called by the MAC when a whole frame has been received.
 * @param writeEvt Callback function to be called by the MAC when a whole frame has been sent.
 * @param bufTxEvt Callback function to be called by the MAC when an outgoing buffer has been processed.
 * @return Error state.
 */
SF_SERIAL_MAC_RETURN sf_serial_mac_init(struct sf_serial_mac_ctx *ctx,
                                        void *portHandle, SF_SERIAL_MAC_HAL_READ_FUNC rx,
                                        SF_SERIAL_MAC_HAL_WRITE_FUNC tx, SF_SERIAL_MAC_READ_EVT readEvt,
                                        SF_SERIAL_MAC_WRITE_EVT writeEvt, SF_SERIAL_MAC_WRITE_EVT bufTxEvt);

/**
 * Start a MAC frame with given length, i.e. initialize frame buffers and send
 * the frame header.
 *
 * This is a non-blocking function.
 *
 * Use sf_serial_mac_txFrameAppend() to append paylaod data.
 *
 * @param ctx Points to the memory region the MAC uses to store its context.
 * @param frmLen Length of the frame to send.
 * @return Error state.
 */
SF_SERIAL_MAC_RETURN sf_serial_mac_txFrameStart(struct sf_serial_mac_ctx *ctx,
        size_t frmLen);

/**
 * Append data to the current frame's payload.
 * Call sf_serial_mac_txFrameStart() first.
 *
 * This is a non-blocking function.
 *
 * As soon as all bytes in the buffer have been sent bufTxEvt() is
 * called. The number of processed buffer bytes is passed to bufTxEvt().
 *
 * As soon as the payload length specified in sf_serial_mac_txFrameStart()
 * has been reached the frame is completed with an CRC and writeEvt()
 * is called.
 *
 * Remaining payload that did not fit into the frame is ignored. The number of
 * processed payload bytes is passed to writeEvt().
 *
 * @param ctx Points to the memory region the MAC uses to store its context.
 * @param frmBufLoc Points to the buffer containing the payload to be appent to the frame.
 * @param frmBufSize Length of the buffer containing the payload to be appent to the frame.
 * @return Error state.
 */
SF_SERIAL_MAC_RETURN sf_serial_mac_txFrameAppend(struct sf_serial_mac_ctx *ctx,
        const char *frmBufLoc,
        size_t frmBufSize);

/**
 * Start a frame iff not already done and append given payload.
 * It is save to always use this function instead of
 * sf_serial_mac_txFrameStart() and
 * sf_serial_mac_txFrameAppend().
 * This function is a combination of sf_serial_mac_txFrameStart() and
 * sf_serial_mac_txFrameAppend().
 *
 * This is a non-blocking function.
 *
 * @param ctx Points to the memory region the MAC uses to store its context.
 * @param frmLen Length of the frame to send.
 * @param frmBufLoc Points to the buffer containing the payload to be appent to the frame.
 * @param frmBufSize Length of the buffer containing the payload to be appent to the frame.
 * @return Error state.
 */
SF_SERIAL_MAC_RETURN sf_serial_mac_txFrame(struct sf_serial_mac_ctx *ctx,
        size_t frmLen, const char *frmBufLoc, size_t frmBufSize);

SF_SERIAL_MAC_RETURN sf_serial_mac_rxFrame(struct sf_serial_mac_ctx *ctx,
        char *frmBufLoc,
        size_t frmBufSize);

/**
 *
 * @param ctx Points to the memory region the MAC uses to store its context.
 * @return Error state.
 */
SF_SERIAL_MAC_RETURN sf_serial_mac_halTxCb(struct sf_serial_mac_ctx *ctx);
/**
 *
 * @param ctx Points to the memory region the MAC uses for its context.
 * @return Error state.
 */
SF_SERIAL_MAC_RETURN sf_serial_mac_halRxCb(struct sf_serial_mac_ctx *ctx);

/**
 * @param ctx Points to the memory region the MAC uses to store its context.
 */
void sf_serial_mac_entry(struct sf_serial_mac_ctx *ctx);

/*! @} end of STACKFORCE_SERIAL_MAC_API_FUNCTIONS */

/*! @} end of STACKFORCE_SERIAL_MAC_API */
/******************************************************************************/
#endif /* STACKFORCE_SERIAL_MAC_API_H_ */
#ifdef __cplusplus
}
#endif
