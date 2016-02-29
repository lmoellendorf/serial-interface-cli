#ifdef __cplusplus
extern "C"
{
#endif
/**
 * @code
 *  ___ _____ _   ___ _  _____ ___  ___  ___ ___
 * / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 * \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 * |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 * embedded.connectivity.solutions.==============
 * @endcode
 *
 * @file
 * @copyright  STACKFORCE GmbH, Heitersheim, Germany, http://www.stackforce.de
 * @author     STACKFORCE
 * @author     Lars Möllendorf
 * @brief      STACKFORCE Serial MAC Module
 *
 * @details Please consult the
 * @ref introduction "README" for a general overview and
 * @ref usage "how to use" the STACKFORCE Serial MAC.
 *
 * This file is part of the STACKFORCE Serial MAC Library
 * (below "libserialmac").
 *
 * libserialmac is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libserialmac is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with libserialmac.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __STACKFORCE_SERIALMAC_H_
#define __STACKFORCE_SERIALMAC_H_

/** This API makes use of size_t */
#include <stddef.h>


/**
 * Return values.
 */
enum sf_serialmac_return
{
    /** No error occurred (always equals 0). */
    SF_SERIALMAC_SUCCESS = 0,
    /** Null pointer exception */
    SF_SERIALMAC_ERROR_NPE,
    /**
     * There is already an operation in progress. Retry later.
     */
    SF_SERIALMAC_ERROR_RW_PENDING,
    /**
     * A previously started frame is still being processed.
     * Wait for for the TX callback before starting a new frame.
     */
    SF_SERIALMAC_ERROR_FRM_PENDING,
    /** The HAL is busy (or you are too fast ;)) and did send nothing. */
    SF_SERIALMAC_ERROR_HAL_BUSY,
    /**
     * The HAL is slow (or you are too fast ;)) but did send at least one byte
     * (this is needed to work around slow serial handlers on Windows).
     */
    SF_SERIALMAC_ERROR_HAL_SLOW,
    /** The HAL reports an error. */
    SF_SERIALMAC_ERROR_HAL_ERROR,
    /** There was an error that should never have happened ;). */
    SF_SERIALMAC_ERROR_EXCEPTION,
    /** There was an error in buffer handling */
    SF_SERIALMAC_ERROR_BUFFER,
};


/**
 * Structure used by the MAC to store its context.
 */
struct sf_serialmac_ctx;


/**
 * Signature of HAL's read function to be used by the MAC for RX.
 * This has to be passed on @ref initialization to sf_serialmac_init().
 */
typedef size_t ( *SF_SERIALMAC_HAL_READ_FUNCTION ) ( void *port_handle,
        char *frame_buffer, size_t frame_buffer_length );
/**
 * Signature of HAL's function which returns the number of bytes waiting on
 * input to be used by the MAC for RX.
 * This has to be passed on @ref initialization to sf_serialmac_init().
 */
typedef size_t ( *SF_SERIALMAC_HAL_READ_WAIT_FUNCTION ) ( void *port_handle );
/**
 * Signature of HAL's write function to be used by the MAC for TX.
 * This has to be passed on @ref initialization to sf_serialmac_init().
 *
 * @param port_handle Points to the port handle that may be needed by the HAL.
 * @param frame_buffer Points to the buffer that has to be written.
 * @param frame_buffer_length Length of the buffer to be written in bytes.
 * @return Number of bytes successfully written.
 */
typedef size_t ( *SF_SERIALMAC_HAL_WRITE_FUNCTION ) ( void *port_handle,
        char *frame_buffer, size_t frame_buffer_length );
/**
 * Signature of upper layer's callback functions to be called by the MAC
 * on events.
 * These functions have to be passed on @ref initialization to
 * sf_serialmac_init().
 */
typedef void ( *SF_SERIALMAC_EVENT ) ( void *mac_context,
                                       char *frame_buffer,
                                       size_t frame_buffer_length );


/**
 * Returns the size of the MAC context structure.
 *
 * On @ref initialization the upper layer has to provide memory the STACKFORCE
 * Serial MAC uses to store its context into. This is done by passing a pointer
 * memory allocated by the upper layer to sf_serialmac_init(). This function
 * returns the size of memory that has to be provided.
 *
 * @return The size of the MAC context structure.
 */
size_t sf_serialmac_ctx_size ( void );


/**
 * Initialization of STACKFORCE Serial MAC.
 *
 * This function must be called once before the MAC can be used and may be
 * called whenever the MAC should be reset.
 *
 * @param ctx Points to the memory region the MAC can use for its context.
 * Please use sf_serialmac_ctx_size() to get the memory size needed by the
 * MAC context.
 * @param port_handle Points to the handle of the serial port which is passed to
 * the underlying HAL.
 * @param read Read function of the underlying HAL.
 * @param read_wait Function of the underlying HAL, that returns the number of
 * bytes waiting in the input buffer.
 * @param write Write function of the underlying HAL.
 * @param rx_event Callback function to be called by the MAC when a whole frame
 * has been received.
 * @param rx_buffer_event Callback function to be called by the MAC when an
 * ingoing buffer has to be provided. If this function is called a frame is
 * ready to be received using sf_serialmac_rxFrame().
 * @param tx_event Callback function to be called by the MAC when a whole frame
 * has been sent.
 * @param tx_buffer_event Callback function to be called by the MAC when an
 * outgoing buffer has been processed.
 * @return Error state.
 */
enum sf_serialmac_return sf_serialmac_init ( struct sf_serialmac_ctx *ctx,
        void *port_handle, SF_SERIALMAC_HAL_READ_FUNCTION read,
        SF_SERIALMAC_HAL_READ_WAIT_FUNCTION read_wait,
        SF_SERIALMAC_HAL_WRITE_FUNCTION write, SF_SERIALMAC_EVENT rx_event,
        SF_SERIALMAC_EVENT rx_buffer_event,
        SF_SERIALMAC_EVENT tx_event, SF_SERIALMAC_EVENT tx_buffer_event );


/**
 * This function can be passed to the HAL layer as callback function to be
 * called on TX events.
 *
 * This is an alternative to the usage of sf_serialmac_entry().
 *
 * @param ctx Points to the memory region the MAC uses to store its context.
 * @return Error state:
 *         - SF_SERIALMAC_ERROR_HAL_ERROR: The HAL reported an error.
 *         - SF_SERIALMAC_ERROR_HAL_BUSY: The HAL is busy.
 *         - SF_SERIALMAC_ERROR_HAL_SLOW: The HAL is busy, but you may retry
 *                                        immediatly (Workaround for slow HALs).
 *         - SF_SERIALMAC_SUCCESS: The payload buffer has been processed.
 *         - SF_SERIALMAC_ERROR_EXCEPTION: You hit a bug.
 */
enum sf_serialmac_return sf_serialmac_hal_tx_callback ( struct sf_serialmac_ctx
        *ctx );


/**
 * This function can be passed to the HAL layer as callback function to be
 * called on RX events.
 *
 * This is an alternative to the usage of sf_serialmac_entry().
 *
 * @param ctx Points to the memory region the MAC uses for its context.
 * @return Error state.
 */
enum sf_serialmac_return sf_serialmac_hal_rx_callback ( struct sf_serialmac_ctx
        *ctx );


/**
 * This function has to be called periodically so the MAC can process incoming
 * data from the underlying serial interface and outgoing data from the upper
 * layer. Alternatively sf_serialmac_hal_tx_callback() and
 * sf_serialmac_hal_rx_callback() can be used.
 *
 * @param ctx Points to the memory region the MAC uses to store its context.
 */
void sf_serialmac_entry ( struct sf_serialmac_ctx *ctx );


/**
 * Start a MAC frame with given length, i.e. initialize frame buffers and send
 * the frame header.
 *
 * This is a non-blocking function.
 *
 * Use sf_serialmac_txFrameAppend() to append payload data.
 *
 * @param ctx Points to the memory region the MAC uses to store its context.
 * @param frame_length Length of the frame to send.
 * @return Error state:
 *         - SF_SERIALMAC_ERROR_NPE: ctx is NULL
 *         - SF_SERIALMAC_ERROR_FRM_PENDING: A frame has been started already
 *                                           and not completed yet.
 */
enum sf_serialmac_return sf_serialmac_tx_frame_start ( struct sf_serialmac_ctx
        *ctx, size_t frame_length );


/**
 * Append data to the current frame's payload.
 * Call sf_serialmac_tx_frame_start() first.
 *
 * This is a non-blocking function.
 *
 * As soon as all bytes in the buffer have been sent tx_buffer_event() is
 * called. The number of processed buffer bytes is passed to tx_buffer_event().
 *
 * As soon as the payload length specified in sf_serialmac_tx_frame_start()
 * has been reached the frame is completed with an CRC and tx_event()
 * is called.
 *
 * Remaining payload that did not fit into the frame is ignored. The number of
 * processed payload bytes is passed to tx_event().
 *
 * @param ctx Points to the memory region the MAC uses to store its context.
 * @param frame_buffer Points to the buffer containing the payload to be
 * appended to the frame.
 * @param frame_buffer_size Length of the buffer containing the payload to be
 * appended to the frame.
 * @return Error state:
 *         - SF_SERIALMAC_ERROR_NPE: ctx is NULL
 *         - SF_SERIALMAC_ERROR_RW_PENDING: There is still a payload buffer in
 *                                          progress.
 */
enum sf_serialmac_return sf_serialmac_tx_frame_append ( struct sf_serialmac_ctx
        *ctx, const char *frame_buffer, size_t frame_buffer_size );


/**
 * Start a frame iff not already done and append given payload.
 * It is save to always use this function instead of
 * sf_serialmac_tx_frame_start() and sf_serialmac_tx_frame_append().
 * This function is a combination of sf_serialmac_tx_frame_start() and
 * sf_serialmac_tx_frame_append().
 *
 * This is a non-blocking function.
 *
 * @param ctx Points to the memory region the MAC uses to store its context.
 * @param frame_length Length of the frame to send.
 * @param frame_buffer Points to the buffer containing the payload to be
 * appended to the frame.
 * @param frame_buffer_size Length of the buffer containing the payload to be
 * appended to the frame.
 * @return Error state:
 *         - SF_SERIALMAC_ERROR_NPE: ctx is NULL
 *         - SF_SERIALMAC_ERROR_FRM_PENDING: A frame has been started already
 *                                           and not processed yet.
 *         - SF_SERIALMAC_ERROR_RW_PENDING: There is still a payload buffer in
 *                                          progress.
 */
enum sf_serialmac_return sf_serialmac_tx_frame ( struct sf_serialmac_ctx *ctx,
        size_t frame_length, const char *frame_buffer, size_t frame_buffer_size
                                               );


/**
 * The upper layer has to call this function whenever the MAC has notified it
 * about the reception of a frame header by calling the upper layers callback
 * function registered on initialization as
 * SF_SERIALMAC_RX_EVENT rx_buffer_event().
 * The MAC expects a frame_buffer with a frame_buffer_size greater or equal
 * the frame length.
 *
 * @param ctx Points to the memory region the MAC uses to store its context.
 * @param frame_buffer Points to the memory where the MAC shall store the
 * payload of the expected frame.
 * @param frame_buffer_size The size of the frame_buffer in bytes.
 */
enum sf_serialmac_return sf_serialmac_rx_frame ( struct sf_serialmac_ctx *ctx,
        char *frame_buffer, size_t frame_buffer_size );


#endif /* STACKFORCE_SERIALMAC_H_ */
#ifdef __cplusplus
}
#endif
