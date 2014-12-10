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

/*==============================================================================
 |                        LOCAL VARIABLE DECLARATIONS
 =============================================================================*/

/*==============================================================================
 |                              LOCAL CONSTANTS
 =============================================================================*/

/*==============================================================================
 |                         LOCAL FUNCTION PROTOTYPES
 =============================================================================*/
static void clearBuffer(BUF* buffer);

/*==============================================================================
 |                              LOCAL FUNCTIONS
 =============================================================================*/
static void clearBuffer(BUF* buffer)
{
    buffer->buffer = NULL;
    buffer->byteSent = 0;
    buffer->length = 0;
}

/*==============================================================================
 |                               API FUNCTIONS
 =============================================================================*/
SF_SERIAL_MAC_CTX *sf_serial_mac_init(SF_SERIAL_MAC_CTX *ctx, void *halCtx,
        SF_SERIAL_MAC_HAL_RX_FUNC rx, SF_SERIAL_MAC_HAL_TX_FUNC tx,
        SF_SERIAL_MAC_READ_EVT readEvt, SF_SERIAL_MAC_WRITE_EVT writeEvt)
{
    ctx->rx = rx;
    ctx->tx = tx;
    ctx->read = readEvt;
    ctx->write = writeEvt;
    ctx->halCtx = halCtx;
    return ctx;
}

int sf_serial_mac_enqueFrame(SF_SERIAL_MAC_CTX *ctx, uint8_t *frameBuffer,
        size_t frameBufferLength)
{
    //TODO: check if previous buffer has been processed
    //TODO: change name - enqueue is misleading, flush is better
    ctx->txBuffer->buffer = frameBuffer;
    ctx->txBuffer->length = frameBufferLength;
    ctx->txBuffer->byteSent = 0;
    return 0;
}

int sf_serial_mac_entry(SF_SERIAL_MAC_CTX *ctx)
{
    size_t bytesSent = 0;
    size_t bytesToSend = ctx->txBuffer->length - ctx->txBuffer->byteSent;

    /***************************************************************************
     * TX
     */

    if (ctx->txBuffer->buffer != NULL
            && ctx->txBuffer->byteSent < ctx->txBuffer->length)
    {
        bytesSent = ctx->tx(ctx->halCtx,
                ctx->txBuffer->buffer + ctx->txBuffer->byteSent, bytesToSend);
        /**
         * This should never happen, but who knows...
         * And so to prevent an buffer overrun we reset the length hardly
         * here. TODO: Maybe we should report such incidence to main() by
         * returning the number of bytes that have been send additionally?
         */
        bytesSent = bytesSent > bytesToSend ? bytesToSend : bytesSent;
        /** update to the number of byte already sent */
        ctx->txBuffer->byteSent += bytesSent;
    }
    if (ctx->txBuffer->buffer != NULL
            && (ctx->txBuffer->length <= ctx->txBuffer->byteSent))
    {
        ctx->write(ctx->txBuffer->buffer, ctx->txBuffer->length);
        clearBuffer(ctx->txBuffer);
    }

    /***************************************************************************
     * RX TODO
     */

    return 0;
}

#ifdef __cplusplus
}
#endif
