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

/*! Define the "external include guard" before including the module header */
#define __DECL_EXAMPLE_H__
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
    size_t byteSent;
};

struct sf_serial_mac_ctx
{
    int fd;
    SF_SERIAL_MAC_HAL_RX_FUNC rx;
    struct sf_serial_mac_buffer rxBuffer;
    SF_SERIAL_MAC_HAL_TX_FUNC tx;
    struct sf_serial_mac_buffer txBuffer;
    SF_SERIAL_MAC_READ_EVT read;
    struct sf_serial_mac_buffer readBuffer;
    SF_SERIAL_MAC_WRITE_EVT write;
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
static void clearBuffer(struct sf_serial_mac_buffer* buffer);

/*==============================================================================
 |                              LOCAL FUNCTIONS
 =============================================================================*/
static void clearBuffer(struct sf_serial_mac_buffer* buffer)
{
    if (buffer)
    {
        buffer->memory = NULL;
        buffer->byteSent = 0;
        buffer->length = 0;
    }
}

/*==============================================================================
 |                               API FUNCTIONS
 =============================================================================*/
size_t sf_serial_mac_ctx_size(void)
{
    return sizeof(struct sf_serial_mac_ctx);
}

struct sf_serial_mac_ctx *sf_serial_mac_init(struct sf_serial_mac_ctx *ctx,
        int fd, SF_SERIAL_MAC_HAL_RX_FUNC rx, SF_SERIAL_MAC_HAL_TX_FUNC tx,
        SF_SERIAL_MAC_READ_EVT readEvt, SF_SERIAL_MAC_WRITE_EVT writeEvt)
{
    ctx->rx = rx;
    ctx->tx = tx;
    ctx->read = readEvt;
    ctx->write = writeEvt;
    ctx->fd = fd;
    clearBuffer(&ctx->txBuffer);
    clearBuffer(&ctx->rxBuffer);
    return ctx;
}

int sf_serial_mac_enqueFrame(struct sf_serial_mac_ctx *ctx,
        const char *frameBuffer, size_t frameBufferLength)
{
    //TODO: check if previous buffer has been processed
    //TODO: change name - enqueue is misleading, flush is better
    ctx->txBuffer.memory = frameBuffer;
    ctx->txBuffer.length = frameBufferLength;
    ctx->txBuffer.byteSent = 0;
    return 0;
}

int sf_serial_mac_entry(struct sf_serial_mac_ctx *ctx)
{

    /***************************************************************************
     * TX
     */

    /* Check if we (still) have bytes to send */
    if (ctx && ctx->txBuffer.memory
            && ctx->txBuffer.byteSent < ctx->txBuffer.length)
    {
        size_t bytesToSend = 0;
        size_t bytesSent = 0;
        bytesToSend = ctx->txBuffer.length - ctx->txBuffer.byteSent;
        /* Send the bytes */
        bytesSent = ctx->tx(ctx->fd,
                ctx->txBuffer.memory + ctx->txBuffer.byteSent, bytesToSend);
        /**
         * This should never happen, but who knows...
         * And so to prevent an buffer overrun we reset the length hardly
         * here. TODO: Maybe we should report such incidence to main() by
         * returning the number of bytes that have been send additionally?
         */
        bytesSent = bytesSent > bytesToSend ? bytesToSend : bytesSent;
        /** update to the number of byte already sent */
        ctx->txBuffer.byteSent += bytesSent;
    }
    /* Check if all bytes have been sent */
    if (ctx && ctx->txBuffer.memory
            && (ctx->txBuffer.length <= ctx->txBuffer.byteSent))
    {
        ctx->write(ctx->txBuffer.memory, ctx->txBuffer.length);
        clearBuffer(&ctx->txBuffer);
    }

    /***************************************************************************
     * RX TODO
     */

    return 0;
}

#ifdef __cplusplus
}
#endif
