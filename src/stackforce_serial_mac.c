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
#include <sf_crc.h>

#include "stackforce_serial_mac_api.h"

/*==============================================================================
 |                                   MACROS
 =============================================================================*/
/*! SYNC word of the STACKFORCE serial protocol */
#define SF_SERIAL_MAC_PROTOCOL_SYNC_WORD              0xA5U
/*! Length of the STACKFORCE serial protocol SYNC word field. */
#define SF_SERIAL_MAC_PROTOCOL_SYNC_WORD_LEN          0x01U
/*! Length of the STACKFORCE serial protocol length field */
#define SF_SERIAL_MAC_PROTOCOL_LENGTH_FIELD_LEN       0x02U
/*! Length of the STACKFORCE serial protocol CRC field */
#define SF_SERIAL_MAC_PROTOCOL_CRC_FIELD_LEN          0x02U
/*! Length of the serial MAC frame header */
#define SF_SERIAL_MAC_PROTOCOL_HEADER_LEN      \
 SF_SERIAL_MAC_PROTOCOL_SYNC_WORD_LEN + SF_SERIAL_MAC_PROTOCOL_LENGTH_FIELD_LEN

#define UINT16_TO_UINT8(u8arr, u16var)         ((u8arr)[0] = (uint8_t)((uint8_t)((u16var)>>8U) & 0xFFU)); \
                                      ((u8arr)[1] = (uint8_t)((u16var) & 0xFFU))
#define UINT8_TO_UINT16(u16var, u8arr) u16var = (((((uint16_t)((u8arr)[0]))<<8U) & 0xFF00U) | \
                                                  (((uint16_t)((u8arr)[1])) & 0xFFU))

/*==============================================================================
 |                                   ENUMS
 =============================================================================*/

/*==============================================================================
 |                       STRUCTURES AND OTHER TYPEDEFS
 =============================================================================*/
struct sf_serial_mac_buffer
{
    const char *memory;
    size_t length;
    size_t byteProcessed;
};

struct sf_serial_mac_frame
{
    /* Buffer for the MAC header: [SYNC] [Length field] */
    uint8_t header[SF_SERIAL_MAC_PROTOCOL_HEADER_LEN];
    uint16_t processed;
    /*! Checksum. */
    uint8_t crc[SF_SERIAL_MAC_PROTOCOL_CRC_FIELD_LEN];
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
    struct sf_serial_mac_buffer headerBuffer;
    struct sf_serial_mac_buffer crcBuffer;
    struct sf_serial_mac_frame frame;
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
    struct sf_serial_mac_buffer *buffer, const char *memory, size_t length,
    size_t byteProcessed);
static int tx(struct sf_serial_mac_ctx *ctx, struct sf_serial_mac_buffer
              *buffer, uint8_t *crc);

/*==============================================================================
 |                              LOCAL FUNCTIONS
 =============================================================================*/
static struct sf_serial_mac_buffer *initBuffer(
    struct sf_serial_mac_buffer *buffer, const char *memory, size_t length,
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

static void initFrame(struct sf_serial_mac_frame *frame)
{
    /** Set a pointer to the sync word for your convinience */
    frame->header[0] = SF_SERIAL_MAC_PROTOCOL_SYNC_WORD;
    frame->processed = 0;
    /** zero buffer */
    memset((void *) frame->crc, 0, SF_SERIAL_MAC_PROTOCOL_CRC_FIELD_LEN);
}

/**
 * Transmit up to sf_serial_mac_buffer.length byte from given buffer.
 * Initialize buffer when everything is transmitted.
 *
 * @param *ctx the current context
 * @param *buffer the buffer to transmit
 * @param *crc place to store crc value to. Set to null if crc shall not be
 *             calculated.
 */
static int tx(struct sf_serial_mac_ctx *ctx, struct sf_serial_mac_buffer
              *buffer, uint8_t *crc)
{
    size_t byteToSend = 0;
    size_t byteSent = 0;
    uint16_t crcVal = 0;

    if(ctx && buffer)
    {

        /** Check if we (still) have bytes to send */
        if (buffer->byteProcessed < buffer->length)
        {
            byteToSend = buffer->length
                         - buffer->byteProcessed;
            /* Send the bytes */
            //TODO: add frame building here
            byteSent = ctx->write(ctx->portHandle,
                                  buffer->memory
                                  + buffer->byteProcessed, byteToSend);
            /**
             * This should never happen, but who knows...
             * And so to prevent an buffer overrun we reset the length hardly
             * here. TODO: Maybe we should report such incidence to main() by
             * returning the number of bytes that have been send additionally?
             */
            byteSent = byteSent > byteToSend ? byteToSend : byteSent;
            if(crc)
            {
                UINT8_TO_UINT16(crcVal, crc);
                crcVal = crc_calc(crcVal, (uint8_t*) buffer->memory + buffer->byteProcessed,
                                  byteSent);
                UINT16_TO_UINT8(crc, crcVal);
            }

            /** update to the number of byte already sent */
            buffer->byteProcessed += byteSent;
        }

        /* Check if all bytes have been sent */
        if ((buffer->length <= buffer->byteProcessed))
        {
            /**
             * Clear the buffer, so the write event won't be called again
             * and again.
             */
            initBuffer(buffer, NULL, 0, 0);
        }
    }

    return byteSent;
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
        initBuffer(&ctx->headerBuffer, NULL, 0, 0);
        initBuffer(&ctx->crcBuffer, NULL, 0, 0);
        initFrame(&ctx->frame);
    }
    return ctx;
}

void sf_serial_mac_txFrameStart(struct sf_serial_mac_ctx *ctx, uint16_t len)
{
    if (ctx && !ctx->crcBuffer.memory)
    {
        /** This assigns the propper buffer */
        initBuffer(&ctx->headerBuffer, (const char*) &ctx->frame.header,
                   sizeof ctx->frame.header, 0);
        /** This assigns the propper buffer and locks the MAC so that no other frame
        can be started until the CRC buffer is cleard */
        initBuffer(&ctx->crcBuffer, (char*) &ctx->frame.crc, sizeof ctx->frame.crc, 0);
        /** write length */
        UINT16_TO_UINT8(ctx->frame.header + SF_SERIAL_MAC_PROTOCOL_SYNC_WORD_LEN, len);
        ctx->frame.processed = 0;
    }
}


void* sf_serial_mac_txFrameAppend(struct sf_serial_mac_ctx *ctx,
                                  const char *frmBufLoc, size_t frmBufSize)
{
    if (ctx)
    {
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
    uint16_t length = 0;
    uint16_t crc = 0;
    /** Do nothing if there is no context. */
    if (ctx)
    {
        UINT8_TO_UINT16(length, ctx->frame.header +
                        SF_SERIAL_MAC_PROTOCOL_SYNC_WORD_LEN);

        /** If memory has been assigned, then there is a header to process */
        if (ctx->headerBuffer.memory)
        {
            tx(ctx, &ctx->headerBuffer, NULL);
        }

        /**
         * If a write buffer has been assigned process it.
         */
        if (ctx->writeBuffer.memory)
        {
            /**
             * Send the payload and calculate the CRC.
             * The second parameter contains the payload, the last parameter is
             * for storing the CRC which is calculated by tx().
             */
            ctx->frame.processed += tx(ctx, &ctx->writeBuffer, (uint8_t *) &ctx->frame.crc);
        }

        /**
         * If the number of processed bytes is greater or equal then the lenght of payload
         * but smaller then the length of payload and the length of CRC, the latter has not
         * been (fully) processed yet.
         */
        if (ctx->frame.processed >= length &&
                ctx->frame.processed < length + SF_SERIAL_MAC_PROTOCOL_CRC_FIELD_LEN)
        {
            UINT8_TO_UINT16(crc, ctx->frame.crc);
            crc = crc_finalize(crc);
            UINT16_TO_UINT8(ctx->frame.crc, crc);
            ctx->frame.processed += tx(ctx, &ctx->crcBuffer, NULL);
        }

        /**
         * If the number of processed bytes is greater than the payload
         * length plus the length of the CRC field, the latter has been processed.
         */
        if (ctx->frame.processed >= length + SF_SERIAL_MAC_PROTOCOL_CRC_FIELD_LEN)
        {
            /**
             * Prepare the frame structure for the next frame.
             */
            initFrame(&ctx->frame);
            /** Inform the upper layer that we are finished */
            ctx->writeEvt();
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
