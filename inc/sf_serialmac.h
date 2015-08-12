#ifdef __cplusplus
extern "C"
{
#endif
/**
 * @file
 * @copyright  STACKFORCE GmbH, Heitersheim, Germany, http://www.stackforce.de
 * @author     STACKFORCE
 * @author     Lars Möllendorf
 * @brief      STACKFORCE Serial MAC Module
 * @section Introduction Introduction
 *
 * @subsection Purpose Purpose
 *
 * The STACKFORCE Serial MAC provides framing for serial interfaces.
 *
 * On TX the STACKFORCE Serial MAC takes over the task to wrap up data in frames
 * before sending them over the serial interface.
 * On RX the STACKFORCE Serial MAC listens for incoming frames, verifies their
 * CRC and provides the payload to the upper layer.
 *
 * @subsection Frame Frame format
 *
 * The Frame format is:
 *
 * <pre>
 * +--------------+--------+-- - - --+-----+
 * | SYNC BYTE(S) | LENGTH | payload | CRC |
 * +--------------+--------+-- - - --+-----+
 * </pre>
 *
 * @subsection Features Features
 *
 * The STACKFORCE Serial MAC is written with cross-platform portability in mind.
 * It should be usable within operating systems as well as bare metal devices.
 *
 * <ul>
 * <li>All API functions are non-blocking.</li>
 * <li>The MAC has no direct dependencies (besides standard C libs and
 * STACKFORCE utilities that are hardware/OS independent, e.g. CRC module).</li>
 * <li>The MAC is usable with any HAL library that provides non-blocking
 * functions to read from and write to the serial interface and a function
 * which returns the number of bytes waiting on input.</li>
 * <li>Buffer allocation and management is completely left to the upper
 * layer.</li>
 * </ul>
 *
 * @section Usage Usage
 *
 * @subsection Init Initialization
 *
 * To use the STACKFORCE Serial MAC you have to initialize it using
 * sf_serialmac_init()
 *
 * @subsection Events Reacting to events
 *
 * The STACKFORCE Serial MAC is event driven. You can use the MAC by calling
 * sf_serialmac_entry() periodically.
 *
 * Or you can add sf_serialmac_hal_tx_callback() and
 * sf_serialmac_hal_rx_callback() as callback function to the corresponding
 * serial port events. (TODO: How to?)
 *
 * @subsection Rx Receiving frames
 *
 * Whenever the STACKFORCE Serial MAC receives the header of a frame it calls
 * the upper layers callback function registered as SF_SERIALMAC_RX_EVENT
 * rx_buffer_event() on Initialization. To receive the frame the upper layer has
 * to provide a memory location for the payload passed to the MAC by calling
 * sf_serialmac_rx_frame(). As soon as the frame has been completed or rejected
 * due to CRC error or time out, the upper layer's callback function is called
 * which has been registered as SF_SERIALMAC_RX_EVENT rx_event() on
 * initialization.
 *
 * @subsection Tx Transmitting frames
 *
 * Frames can be transmitted at once using sf_serialmac_tx_frame(). Or by
 * starting a frame with sf_serialmac_tx_frame_start() and successively
 * appending the payload using sf_serialmac_tx_frame_append() until the frame
 * is filled.
 *
 * Whenever the MAC completed the transmission of a frame the upper layer's
 * callback called that has been registered as SF_SERIALMAC_TX_EVENT tx_event()
 * on initialization.
 *
 * Whenever the MAC processed a buffer with payload the upper layer's callback
 * is called which has been registered as SF_SERIALMAC_TX_EVENT tx_buf_event on
 * initialization. The upper layer must not touch the buffer memory passed with
 * sf_serialmac_tx_frame() or sf_serialmac_tx_frame_append() before this
 * callback has been called. Also all calls to sf_serialmac_tx_frame_append()
 * are ignored until the previously provided buffer has been processed.
 */

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

/******************************************************************************/
/*!
 @defgroup STACKFORCE_SERIAL_MAC_API_ENUMS Enumerations
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the enums used by the STACKFORCE Example implementation.
 */

/*! @addtogroup STACKFORCE_SERIAL_MAC_API_ENUMS
 * @{ */
/**
 * Return values.
 */
enum sf_serial_mac_return
{
    /** No error occured (always equals 0). */
    SF_SERIAL_MAC_SUCCESS = 0,
    /** Null pointer exception */
    SF_SERIAL_MAC_ERROR_NPE,
    /**
     * There is already an operation in progress. Retry later.
     */
    SF_SERIAL_MAC_ERROR_RW_PENDING,
    /**
     * A previously started frame is still processed.
     * Wait for SF_SERIAL_MAC_WRITE_EVT() before starting a new frame.
     */
    SF_SERIAL_MAC_ERROR_FRM_PENDING,
    /** The HAL is busy (or you are too fast ;)) and did send nothing. */
    SF_SERIAL_MAC_ERROR_HAL_BUSY,
    /**
     * The HAL is slow (or you are too fast ;)) but did send at least one byte
     * (this is needed to work around slow serial handlers on Windows).
     */
    SF_SERIAL_MAC_ERROR_HAL_SLOW,
    /** The HAL reports an error. */
    SF_SERIAL_MAC_ERROR_HAL_ERROR,
    /** There was an error that should never have happened ;). */
    SF_SERIAL_MAC_ERROR_EXCEPTION,
    /** There was an error in buffer handling */
    SF_SERIAL_MAC_ERROR_BUFFER,
};
/*! @} end of STACKFORCE_SERIAL_MAC_API_ENUMS */

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
        char *frameBuffer, size_t frameBufferLength);
/**
 * Signature of HAL's function which return the number of bytes waiting on input
 * to be used by the MAC for RX.
 */
typedef ssize_t (*SF_SERIAL_MAC_HAL_READ_WAIT_FUNC)(void *portHandle);
/**
 * Signature of HAL's write function to be used by the MAC for TX.
 *
 * @param portHandle Points to the port handle that may be needed by the HAL.
 * @param frameBuffer Points to the buffer that has to be written.
 * @param frameBufferLength Length of the buffer to be written in bytes.
 * @return Number of bytes successfully written.
 */
typedef ssize_t (*SF_SERIAL_MAC_HAL_WRITE_FUNC)(void *portHandle,
        const char *frameBuffer, size_t frameBufferLength);
/**
 * Signature of APP's callback function to be called by the MAC
 * when a whole frame has been received.
 */
typedef void (*SF_SERIAL_MAC_RX_EVT)(const char *frameBuffer,
                                     size_t frameBufferLength);
/**
 * Signature of APP's callback function to be called by the MAC
 * when a whole frame has been sent.
 *
 * @param byteSent Number of bytes sent with the frame.
 */
typedef void (*SF_SERIAL_MAC_TX_EVT)(size_t byteSent);

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
/*! @defgroup STACKFORCE_SERIAL_MAC_API_FUNCTIONS API
 @ingroup  STACKFORCE_SERIAL_MAC_API
 This section describes the functions provided by the STACKFORCE serial MAC.
 */

/*! @addtogroup STACKFORCE_SERIAL_MAC_API_FUNCTIONS
 *  @{ */

/**
 * Returns the size of the MAC context structure.
 *
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
 * @param read Read function of the underlying HAL.
 * @param readWaiting Function of the underlying HAL, that returns the number of bytes waiting in the input buffer.
 * @param write Write function of the underlying HAL.
 * @param rxEvt Callback function to be called by the MAC when a whole frame has been received.
 * @param rxBufEvt Callback function to be called by the MAC when an ingoing buffer has to be provided.
 *                 If this function is called a frame is ready to be received using sf_serial_mac_rxFrame().
 * @param txEvt Callback function to be called by the MAC when a whole frame has been sent.
 * @param txBufEvt Callback function to be called by the MAC when an outgoing buffer has been processed.
 * @return Error state.
 */
enum sf_serial_mac_return sf_serial_mac_init(struct sf_serial_mac_ctx *ctx,
        void *portHandle, SF_SERIAL_MAC_HAL_READ_FUNC read,
        SF_SERIAL_MAC_HAL_READ_WAIT_FUNC readWaiting,
        SF_SERIAL_MAC_HAL_WRITE_FUNC write, SF_SERIAL_MAC_RX_EVT rxEvt,
        SF_SERIAL_MAC_RX_EVT rxBufEvt,
        SF_SERIAL_MAC_TX_EVT txEvt, SF_SERIAL_MAC_TX_EVT txBufEvt);

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
enum sf_serial_mac_return sf_serial_mac_txFrameStart(struct sf_serial_mac_ctx
        *ctx, size_t frmLen);

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
enum sf_serial_mac_return sf_serial_mac_txFrameAppend(struct sf_serial_mac_ctx
        *ctx, const char *frmBufLoc, size_t frmBufSize);

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
enum sf_serial_mac_return sf_serial_mac_txFrame(struct sf_serial_mac_ctx *ctx,
        size_t frmLen, const char *frmBufLoc, size_t frmBufSize);

enum sf_serial_mac_return sf_serial_mac_rxFrame(struct sf_serial_mac_ctx *ctx,
        char *frmBufLoc, size_t frmBufSize);

/**
 *
 * @param ctx Points to the memory region the MAC uses to store its context.
 * @return Error state.
 */
enum sf_serial_mac_return sf_serial_mac_halTxCb(struct sf_serial_mac_ctx *ctx);
/**
 *
 * @param ctx Points to the memory region the MAC uses for its context.
 * @return Error state.
 */
enum sf_serial_mac_return sf_serial_mac_halRxCb(struct sf_serial_mac_ctx *ctx);

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
