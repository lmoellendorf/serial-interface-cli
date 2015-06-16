#ifdef __cplusplus
extern "C"
{
#endif
/*! ============================================================================
 *
 * @file:    stackforce_serial_mac_api.c
 *
 * @date:    08.12.2014
 * @author:  © by STACKFORCE, Heitersheim, Germany, http://www.stackforce.de
 * @author:  Lars Möllendorf
 *
 * @brief:   TODO: Sample header of the source code
 *
 * @version:
 *
 =============================================================================*/

/*==============================================================================
 |                               INCLUDE FILES
 =============================================================================*/
/*! definition of portable data types */
#include <stdint.h>
/*! Common definitions most notably NULL */
#include <stddef.h>
#include <string.h>

#include "stackforce_serial_mac_api.h"

/*==============================================================================
 |                                   MACROS
 =============================================================================*/

/*==============================================================================
 |                                   ENUMS
 =============================================================================*/

/*==============================================================================
 |                       STRUCTURES AND OTHER TYPEDEFS
 =============================================================================*/
struct sf_serial_mac_buffer
{
    const char *memory;
//    uint8_t *currentPosition;
    size_t length;
    size_t byteProcessed;
};

struct sf_serial_mac_ctx
{
    void *portHandle;
    SF_SERIAL_MAC_HAL_READ_FUNC read;
    SF_SERIAL_MAC_HAL_WRITE_FUNC write;
    SF_SERIAL_MAC_READ_EVT readEvt;
    SF_SERIAL_MAC_WRITE_EVT writeEvt;
    struct sf_serial_mac_buffer readBuffer;
    struct sf_serial_mac_buffer writeBuffer;
};

/*==============================================================================
 |                        LOCAL VARIABLE DECLARATIONS
 =============================================================================*/

/*==============================================================================
 |                              LOCAL CONSTANTS
 =============================================================================*/

/*==============================================================================
 |                         LOCAL FUNCTION PROTOTYPES
 =============================================================================*/
static struct sf_serial_mac_buffer* initBuffer(
    struct sf_serial_mac_buffer* buffer, const char *memory, size_t length,
    size_t byteProcessed);

/*==============================================================================
 |                              LOCAL FUNCTIONS
 =============================================================================*/
static struct sf_serial_mac_buffer* initBuffer(
    struct sf_serial_mac_buffer* buffer, const char *memory, size_t length,
    size_t byteProcessed)
{
    if (buffer)
    {
        buffer->memory = memory;
        buffer->length = length;
        buffer->byteProcessed = byteProcessed;
    }
    return buffer;
}

/*==============================================================================
 |                               API FUNCTIONS
 =============================================================================*/
size_t sf_serial_mac_ctx_size(void)
{
    return sizeof(struct sf_serial_mac_ctx);
}

void* sf_serial_mac_init(struct sf_serial_mac_ctx *ctx, void *portHandle,
                         SF_SERIAL_MAC_HAL_READ_FUNC rx, SF_SERIAL_MAC_HAL_WRITE_FUNC tx,
                         SF_SERIAL_MAC_READ_EVT readEvt, SF_SERIAL_MAC_WRITE_EVT writeEvt)
{
    if (ctx)
    {
        ctx->read = rx;
        ctx->write = tx;
        ctx->readEvt = readEvt;
        ctx->writeEvt = writeEvt;
        ctx->portHandle = portHandle;
        initBuffer(&ctx->writeBuffer, NULL, 0, 0);
        initBuffer(&ctx->readBuffer, NULL, 0, 0);
    }
    return ctx;
}

void* sf_serial_mac_txFrame(struct sf_serial_mac_ctx *ctx,
        const char *frmBufLoc, size_t frmBufSize)
{
    if (ctx)
    {
        //TODO: check if previous buffer has been processed
        ctx->writeBuffer.memory = frmBufLoc;
        ctx->writeBuffer.length = frmBufSize;
        ctx->writeBuffer.byteProcessed = 0;
    }
    return ctx;
}

void* sf_serial_mac_rxFrame(struct sf_serial_mac_ctx *ctx, char *frmBufLoc,
                            size_t frmBufSize)
{
    if (ctx)
    {
        //TODO: check if previous buffer has been processed?
        ctx->readBuffer.memory = frmBufLoc;
        ctx->readBuffer.length = frmBufSize;
        ctx->readBuffer.byteProcessed = 0;
        /** zero buffer */
        memset((void *) ctx->readBuffer.memory, 0, ctx->readBuffer.length);
    }
    return ctx;
}

void* sf_serial_mac_halTxCb(struct sf_serial_mac_ctx *ctx)
{
    /** Do nothing if there is no context. */
    if (ctx)
    {
        /**
         * Check if a write buffer has been assigned - otherwise this means
         * there is nothing to do.
         */
        if (ctx->writeBuffer.memory)
        {
            /** Check if we (still) have bytes to send */
            if (ctx->writeBuffer.byteProcessed < ctx->writeBuffer.length)
            {
                size_t bytesToSend = 0;
                size_t bytesSent = 0;
                bytesToSend = ctx->writeBuffer.length
                              - ctx->writeBuffer.byteProcessed;
                /* Send the bytes */
                //TODO: add frame building here
                bytesSent = ctx->write(ctx->portHandle,
                                       ctx->writeBuffer.memory
                                       + ctx->writeBuffer.byteProcessed, bytesToSend);
                /**
                 * This should never happen, but who knows...
                 * And so to prevent an buffer overrun we reset the length hardly
                 * here. TODO: Maybe we should report such incidence to main() by
                 * returning the number of bytes that have been send additionally?
                 */
                bytesSent = bytesSent > bytesToSend ? bytesToSend : bytesSent;
                /** update to the number of byte already sent */
                ctx->writeBuffer.byteProcessed += bytesSent;
            }
            /* Check if all bytes have been sent */
            if ((ctx->writeBuffer.length <= ctx->writeBuffer.byteProcessed))
            {
                /** Inform the upper layer that we are finished */
                ctx->writeEvt(ctx->writeBuffer.memory, ctx->writeBuffer.length);
                /**
                 * Clear the buffer, so the write event won't be called again
                 * and again.
                 */
                initBuffer(&ctx->writeBuffer, NULL, 0, 0);
            }
        }
    }
    return ctx;
}

void* sf_serial_mac_halRxCb(struct sf_serial_mac_ctx *ctx)
{
    /** Do nothing if there is no context. */
    if (ctx)
    {
        /**
         * Check if a read buffer has been assigned - otherwise this means
         * there is nothing to do.
         */
        if (ctx->readBuffer.memory)
        {
            if ( //TODO: signal buffer overflow
                (ctx->readBuffer.byteProcessed < ctx->readBuffer.length))
            {
                int recv = 0;
                do
                {
                    recv = ctx->read(ctx->portHandle,
                                     (char *) (ctx->readBuffer.memory
                                               + ctx->readBuffer.byteProcessed), 1);
                    if (recv > 0)
                    {
                        //TODO: here the parsing of data begins
                        if ('\n'
                                == ctx->readBuffer.memory[ctx->readBuffer.byteProcessed]) // this is just a proof of concept
                        {
                            // end of line is reached - inform the app
                            ctx->readEvt(ctx->readBuffer.memory,
                                         ctx->readBuffer.byteProcessed + recv);
                            /** Leave the loop */
                            break; /** recv = 0; would have the same effect - at least now */
                        }
                        else
                        {
                            ctx->readBuffer.byteProcessed += recv;
                        }

                    }

                }
                while (recv);
            }
        }
    }
    return ctx;
}

void* sf_serial_mac_entry(struct sf_serial_mac_ctx *ctx)
{
    /***************************************************************************
     * TX
     */
    sf_serial_mac_halTxCb(ctx);

    /***************************************************************************
     * RX
     */
    sf_serial_mac_halRxCb(ctx);

    return ctx;
}

#ifdef __cplusplus
}
#endif
