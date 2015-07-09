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
#define CRC_TABLE FALSE
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
 (SF_SERIAL_MAC_PROTOCOL_SYNC_WORD_LEN + SF_SERIAL_MAC_PROTOCOL_LENGTH_FIELD_LEN)

#define UINT16_TO_UINT8(u8arr, u16var)         ((u8arr)[0] = (uint8_t)((uint8_t)((u16var)>>8U) & 0xFFU)); \
                                      ((u8arr)[1] = (uint8_t)((u16var) & 0xFFU))
#define UINT8_TO_UINT16(u8arr) (((((uint16_t)((u8arr)[0]))<<8U) & 0xFF00U) | \
                                                  (((uint16_t)((u8arr)[1])) & 0xFFU))

/*==============================================================================
 |                                   ENUMS
 =============================================================================*/
/**
 * A frame consists of the elements:
 * <ul>
 * <li>header (H)</li>
 * <li>payload</li>
 * <li>CRC (C)</li>
 * </ul>
 *
 * All three elements have their own serial MAC buffer which are held in the
 * serial MAC context.
 *
 * The MAC keeps track of the number of payload bytes that still needs to be processed.
 *
 * Those 3 buffers then are processed sequentially in this order:
 *
 * |HHH|payload...|CC|
 *
 * 4 states have to be distinguished:
 */
enum rxTxState {
    /** MAC is idle. */
    IDLE,
    /** HEADER is about to be processed. */
    HEADER,
    /** PAYLOAD is about to be processed. */
    PAYLOAD,
    /** CRC is about to be processed. */
    CRC,
};

/*==============================================================================
 |                       STRUCTURES AND OTHER TYPEDEFS
 =============================================================================*/
/**
 * Signature of APP's callback function to be called by the MAC
 * when a buffer has been processed.
 *
 * @param byteWritten Number of written byte.
 */
typedef void (*SF_SERIAL_MAC_BUF_EVT)(struct sf_serial_mac_ctx *ctx);

/**
 * Context of an internal serial MAC buffer.
 */
struct sf_serial_mac_buffer
{
    /** Memory for the bytes to be processed. */
    char *memory;
    /** length of the buffer memory in bytes. */
    size_t length;
    /** Bytes that still needs to be processed. */
    size_t remains;
    /** Function to be called when all bytes are proccessed. */
    SF_SERIAL_MAC_BUF_EVT callback;
};

/**
 * Context of an serial MAC frame.
 *
 * There is no memory for the payload, because this is handed over by the upper layer.
 */
struct sf_serial_mac_frame
{
    enum rxTxState state;
    /** Payload bytes that still needs to be processed. */
    uint16_t remains;
    /** Memory for the MAC header: [SYNC] [Length field] */
    uint8_t headerMemory[SF_SERIAL_MAC_PROTOCOL_HEADER_LEN];
    /** Memory for the CRC. */
    uint8_t crcMemory[SF_SERIAL_MAC_PROTOCOL_CRC_FIELD_LEN];
    /** Buffer for the frame header to transmit. */
    struct sf_serial_mac_buffer headerBuffer;
    /** Buffer for the frame payload to transmit. */
    struct sf_serial_mac_buffer payloadBuffer;
    /** Buffer for the frame CRC to transmit. */
    struct sf_serial_mac_buffer crcBuffer;
};

/**
 * Context of the serial MAC.
 */
struct sf_serial_mac_ctx
{
    /** Handle of the serial port that is passed through to the lower HAL. */
    void *portHandle;
    /** Read function of the lower HAL. */
    SF_SERIAL_MAC_HAL_READ_FUNC read;
    /** Function of the lower HAL that returns number of byte waiting for reading in HAL's buffer. */
    SF_SERIAL_MAC_HAL_READ_WAIT_FUNC readWait;
    /** Write function of the lower HAL. */
    SF_SERIAL_MAC_HAL_WRITE_FUNC write;
    /** Function to be called when a whole buffer has been received. */
    SF_SERIAL_MAC_RX_EVT rxEvt;
    /** Function to be called when a RX buffer is needed to receive a frame. */
    SF_SERIAL_MAC_RX_EVT rxBufEvt;
    /** Function to be called when a whole buffer has been sent. */
    SF_SERIAL_MAC_TX_EVT txEvt;
    /** Function to be called when a TX buffer has been processed. */
    SF_SERIAL_MAC_TX_EVT txBufEvt;
    /** Context of the frame to send. */
    struct sf_serial_mac_frame txFrame;
    /** Context of the frame to receive. */
    struct sf_serial_mac_frame rxFrame;
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
    struct sf_serial_mac_buffer *buffer, char *memory, size_t length,
    SF_SERIAL_MAC_BUF_EVT callback);
static void txInit(struct sf_serial_mac_ctx *ctx);
static enum sf_serial_mac_return tx(struct sf_serial_mac_ctx *ctx,
                                    struct sf_serial_mac_buffer
                                    *buffer, uint8_t *crc);
static void txProcHeaderCB(struct sf_serial_mac_ctx *ctx);
static void txProcPayloadCB(struct sf_serial_mac_ctx *ctx);
static void txProcCrcCB(struct sf_serial_mac_ctx *ctx);
static void rxInit(struct sf_serial_mac_ctx *ctx);
static enum sf_serial_mac_return rx(struct sf_serial_mac_ctx *ctx,
                                    struct sf_serial_mac_buffer
                                    *buffer, size_t bytesWaiting);
static void rxProcHeaderCB(struct sf_serial_mac_ctx *ctx);
static void rxProcPayloadCB(struct sf_serial_mac_ctx *ctx);
static void rxProcCrcCB(struct sf_serial_mac_ctx *ctx);

/*==============================================================================
 |                              LOCAL FUNCTIONS
 =============================================================================*/
static struct sf_serial_mac_buffer *initBuffer(
    struct sf_serial_mac_buffer *buffer, char *memory, size_t length,
    SF_SERIAL_MAC_BUF_EVT callback)
{
    if (buffer)
    {
        buffer->memory = memory;
        buffer->length = length;
        buffer->remains = length;
        buffer->callback = callback;
    }
    return buffer;
}

static void initFrame(struct sf_serial_mac_frame *frame, uint8_t syncWord)
{
    frame->remains = 0;
    /** zero buffer */
    memset((void *) frame->headerMemory, 0, SF_SERIAL_MAC_PROTOCOL_HEADER_LEN);
    /** zero buffer */
    memset((void *) frame->crcMemory, 0, SF_SERIAL_MAC_PROTOCOL_CRC_FIELD_LEN);
    /** Write the sync word into the buffer */
    frame->headerMemory[0] = syncWord;
}

static void txInit(struct sf_serial_mac_ctx *ctx)
{
    initBuffer(&ctx->txFrame.headerBuffer, (uint8_t*) &ctx->txFrame.headerMemory,
               SF_SERIAL_MAC_PROTOCOL_HEADER_LEN, txProcHeaderCB);
    initBuffer(&ctx->txFrame.payloadBuffer, NULL, 0, txProcPayloadCB);
    initBuffer(&ctx->txFrame.crcBuffer, (uint8_t*) &ctx->txFrame.crcMemory,
               SF_SERIAL_MAC_PROTOCOL_CRC_FIELD_LEN, txProcCrcCB);
    initFrame(&ctx->txFrame, SF_SERIAL_MAC_PROTOCOL_SYNC_WORD);
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
static enum sf_serial_mac_return tx(struct sf_serial_mac_ctx *ctx,
                                    struct sf_serial_mac_buffer
                                    *buffer, uint8_t *crc)
{
    size_t byteSent = 0;

    /** Check if we (still) have bytes to send */
    if (buffer->remains)
    {
        /** Send the bytes */
        if((byteSent = ctx->write(ctx->portHandle,
                                  buffer->memory
                                  + (buffer->length - buffer->remains), buffer->remains)) < 0)
        {
            return SF_SERIAL_MAC_ERROR_HAL_ERROR;
        }
        if (buffer->remains < byteSent)
        {
            /** This should never happen, but if it does we can catch it. */
            return SF_SERIAL_MAC_ERROR_EXCEPTION;
        }
        if(crc)
        {
            UINT16_TO_UINT8(crc, crc_calc(UINT8_TO_UINT16(crc), (uint8_t*) buffer->memory
                                          + (buffer->length - buffer->remains), byteSent));
        }

        /** update to the number of byte already sent */
        buffer->remains -= byteSent;
    }

    /** Check if all bytes have been sent */
    if (!buffer->remains)
    {
        buffer->callback(ctx);
        return SF_SERIAL_MAC_SUCCESS;
    }
    else
    {
        return SF_SERIAL_MAC_ERROR_HAL_BUSY;
    }
}

static void txProcHeaderCB(struct sf_serial_mac_ctx *ctx)
{
    ctx->txFrame.state = PAYLOAD;
}

static void txProcPayloadCB(struct sf_serial_mac_ctx *ctx)
{
    size_t processed = ctx->txFrame.payloadBuffer.length -
                       ctx->txFrame.payloadBuffer.remains;
    /**
     * Clear the buffer, so the write event won't be called again
     * and again for this buffer.
     */
    initBuffer(&ctx->txFrame.payloadBuffer, NULL, 0, txProcPayloadCB);
    ctx->txFrame.remains -= processed;
    if(ctx->txFrame.remains <= 0)
    {
        UINT16_TO_UINT8(ctx->txFrame.crcMemory,
                        crc_finalize(UINT8_TO_UINT16(ctx->txFrame.crcMemory)));
        ctx->txFrame.state = CRC;
    }
    /** inform upper layer that the buffer has been processed */
    ctx->txBufEvt(processed);
}

static void txProcCrcCB(struct sf_serial_mac_ctx *ctx)
{
    size_t length = UINT8_TO_UINT16(ctx->txFrame.headerMemory +
                                    SF_SERIAL_MAC_PROTOCOL_SYNC_WORD_LEN);
    txInit(ctx);
    ctx->txFrame.state = IDLE;
    ctx->txEvt(length);
}

static void rxInit(struct sf_serial_mac_ctx *ctx)
{
    initBuffer(&ctx->rxFrame.headerBuffer, (uint8_t*) &ctx->rxFrame.headerMemory,
               SF_SERIAL_MAC_PROTOCOL_HEADER_LEN, rxProcHeaderCB);
    initBuffer(&ctx->rxFrame.payloadBuffer, NULL, 0, rxProcPayloadCB);
    initBuffer(&ctx->rxFrame.crcBuffer, ctx->rxFrame.crcMemory,
               SF_SERIAL_MAC_PROTOCOL_CRC_FIELD_LEN, rxProcCrcCB);
    initFrame(&ctx->rxFrame, 0);
}

static enum sf_serial_mac_return rx(struct sf_serial_mac_ctx *ctx,
                                    struct sf_serial_mac_buffer
                                    *buffer, size_t bytesWaiting)
{
    size_t byteToReceive = 0;
    size_t bytesReceived = 0;

    byteToReceive = buffer->remains > bytesWaiting ? bytesWaiting : buffer->remains;
    if (byteToReceive)
    {
        if((bytesReceived = ctx->read(ctx->portHandle,
                                      (void*) (buffer->memory + buffer->length - buffer->remains),
                                      byteToReceive)) < 0)
        {
            return SF_SERIAL_MAC_ERROR_HAL_ERROR;
        }
        if (byteToReceive != bytesReceived)
        {
            /** This should never happen, but if it does we can catch it. */
            return SF_SERIAL_MAC_ERROR_EXCEPTION;
        }
        buffer->remains -= bytesReceived;
        if(!buffer->remains)
        {
            buffer->callback(ctx);
        }
    }
    return SF_SERIAL_MAC_SUCCESS;
}

static void rxProcHeaderCB(struct sf_serial_mac_ctx *ctx)
{
    /** Start the countdown */
    ctx->rxFrame.remains = UINT8_TO_UINT16(ctx->rxFrame.headerMemory +
                                           SF_SERIAL_MAC_PROTOCOL_SYNC_WORD_LEN);
    /** Inform upper layer that there has been a frame header received */
    ctx->rxBufEvt(NULL, ctx->rxFrame.remains);
    ctx->rxFrame.state = PAYLOAD;
}

static void rxProcPayloadCB(struct sf_serial_mac_ctx *ctx)
{
    ctx->rxFrame.remains -= ctx->rxFrame.payloadBuffer.length;
    /**
     * Unlike the TX branch, in RX there is only one payload buffer allowed.
     * That is why the state is switched here.
     */
    ctx->rxFrame.state = CRC;
}

static void rxProcCrcCB(struct sf_serial_mac_ctx *ctx)
{
    uint16_t length = 0;
    /** The length is needed for calculating the CRC and for signaling the upper layer. */
    length = UINT8_TO_UINT16(ctx->rxFrame.headerMemory +
                             SF_SERIAL_MAC_PROTOCOL_SYNC_WORD_LEN);
    /** The received CRC has to checked. */
    if(crc_calc_finalize(ctx->rxFrame.payloadBuffer.memory,
                         length) == UINT8_TO_UINT16(ctx->rxFrame.crcBuffer.memory))
    {
        /** inform the upper layer that a frame has been completed */
        ctx->rxEvt(ctx->rxFrame.payloadBuffer.memory, length);
    }
    /** Regardless of the CRC, start waiting for the next frame. */
    rxInit(ctx);
    ctx->rxFrame.state = IDLE;
}

/*==============================================================================
 |                               API FUNCTIONS
 =============================================================================*/
size_t sf_serial_mac_ctx_size(void)
{
    return sizeof(struct sf_serial_mac_ctx);
}


enum sf_serial_mac_return sf_serial_mac_init(struct sf_serial_mac_ctx *ctx,
        void *portHandle, SF_SERIAL_MAC_HAL_READ_FUNC read,
        SF_SERIAL_MAC_HAL_READ_WAIT_FUNC readWaiting,
        SF_SERIAL_MAC_HAL_WRITE_FUNC write, SF_SERIAL_MAC_RX_EVT rxEvt,
        SF_SERIAL_MAC_RX_EVT rxBufEvt,
        SF_SERIAL_MAC_TX_EVT txEvt, SF_SERIAL_MAC_TX_EVT txBufEvt)
{
    if (!ctx)
    {
        return SF_SERIAL_MAC_ERROR_NPE;
    }
    ctx->portHandle = portHandle;
    ctx->read = read;
    ctx->readWait = readWaiting;
    ctx->write = write;
    ctx->rxEvt = rxEvt;
    ctx->rxBufEvt = rxBufEvt;
    ctx->txEvt = txEvt;
    ctx->txBufEvt = txBufEvt;
    txInit(ctx);
    ctx->txFrame.state = IDLE;
    rxInit(ctx);
    ctx->rxFrame.state = IDLE;

    return SF_SERIAL_MAC_SUCCESS;
}

enum sf_serial_mac_return sf_serial_mac_txFrameStart(struct sf_serial_mac_ctx
        *ctx, size_t len)
{
    if (!ctx)
    {
        return SF_SERIAL_MAC_ERROR_NPE;
    }
    if(ctx->txFrame.state != IDLE)
    {
        return SF_SERIAL_MAC_ERROR_FRM_PENDING;
    }

    /** Write frame length into the length field of the frame header */
    UINT16_TO_UINT8(ctx->txFrame.headerMemory +
                    SF_SERIAL_MAC_PROTOCOL_SYNC_WORD_LEN,
                    len);
    ctx->txFrame.remains = len;
    ctx->txFrame.state = HEADER;
    return SF_SERIAL_MAC_SUCCESS;
}

enum sf_serial_mac_return sf_serial_mac_txFrameAppend(struct sf_serial_mac_ctx
        *ctx, const char *frmBufLoc, size_t frmBufSize)
{
    size_t buff = 0;
    if (!ctx)
    {
        return SF_SERIAL_MAC_ERROR_NPE;
    }
    /**
     * Check if a payload buffer has been assigned before and is not
     * completely processed. And prevent upper layer to append more payload
     * before the previous frame has been started.
     */
    if(ctx->txFrame.payloadBuffer.memory || ctx->txFrame.state == CRC)
    {
        return SF_SERIAL_MAC_ERROR_RW_PENDING;
    }
    buff = frmBufSize > ctx->txFrame.remains ? ctx->txFrame.remains : frmBufSize;
    /**
     * By assigning memory to the payload buffer the payload is transmitted with
     * the next call to sf_serial_mac_halTxCb() and consecutive tries
     * to assign payload buffers are prevented untill the currently assigned
     * buffer has been processed.
     */
    initBuffer(&ctx->txFrame.payloadBuffer, (char*) frmBufLoc, buff,
               txProcPayloadCB);
    return SF_SERIAL_MAC_SUCCESS;
}

enum sf_serial_mac_return sf_serial_mac_txFrame(struct sf_serial_mac_ctx *ctx,
        size_t frmLen, const char *frmBufLoc, size_t frmBufSize)
{
    sf_serial_mac_txFrameStart(ctx, frmLen);
    return sf_serial_mac_txFrameAppend(ctx, frmBufLoc, frmBufSize);
}

enum sf_serial_mac_return sf_serial_mac_rxFrame(struct sf_serial_mac_ctx *ctx,
        char *frmBufLoc, size_t frmBufSize)
{
    if (!ctx || !frmBufLoc)
    {
        return SF_SERIAL_MAC_ERROR_NPE;
    }
    if(ctx->rxFrame.payloadBuffer.memory != NULL)
    {
        return SF_SERIAL_MAC_ERROR_RW_PENDING;
    }
    if(frmBufSize < ctx->rxFrame.remains)
    {
        return SF_SERIAL_MAC_ERROR_BUFFER;
    }
    initBuffer(&ctx->rxFrame.payloadBuffer, frmBufLoc, ctx->rxFrame.remains,
               rxProcPayloadCB);
    memset((void *) ctx->rxFrame.payloadBuffer.memory, 0,
           ctx->rxFrame.payloadBuffer.length);
    return SF_SERIAL_MAC_SUCCESS;
}

enum sf_serial_mac_return sf_serial_mac_halTxCb(struct sf_serial_mac_ctx *ctx)
{
    enum sf_serial_mac_return ret = SF_SERIAL_MAC_SUCCESS;

    /** Do nothing if there is no context. */
    if (!ctx)
    {
        return SF_SERIAL_MAC_ERROR_NPE;
    }

    switch (ctx->txFrame.state)
    {
    case IDLE:
        /** Nothing to do. */
        break;
    case HEADER:
        ret = tx(ctx, &ctx->txFrame.headerBuffer, NULL);
        break;
    case PAYLOAD:
        /**
         * If a write buffer has been assigned process it.
         */
        if (ctx->txFrame.payloadBuffer.memory)
        {
            /**
             * Send the payload and calculate the CRC.
             * The second parameter contains the payload, the last parameter is
             * for storing the CRC which is calculated by tx().
             */
            ret = tx(ctx, &ctx->txFrame.payloadBuffer, (uint8_t *) &ctx->txFrame.crcMemory);
        }
        break;
    case CRC:
        ret = tx(ctx, &ctx->txFrame.crcBuffer, NULL);
        break;
    }

    return ret;
}

enum sf_serial_mac_return sf_serial_mac_halRxCb(struct sf_serial_mac_ctx *ctx)
{
    size_t bytesWaiting = 0;
    enum sf_serial_mac_return ret = SF_SERIAL_MAC_SUCCESS;

    /** Do nothing if there is no context. */
    if (!ctx)
    {
        return SF_SERIAL_MAC_ERROR_NPE;
    }

    /** Is there anything to receive? FIXME: prevent DOS attack */
    while ((bytesWaiting = ctx->readWait(ctx->portHandle)) > 0
            && ret == SF_SERIAL_MAC_SUCCESS)
    {

        switch (ctx->rxFrame.state)
        {
        case IDLE:
            ret = rx(ctx, &ctx->rxFrame.headerBuffer, SF_SERIAL_MAC_PROTOCOL_SYNC_WORD_LEN);
            /** FIXME: this only works for sync words of 1 byte length! */
            if(ctx->rxFrame.headerBuffer.memory[0] == (char)
                    SF_SERIAL_MAC_PROTOCOL_SYNC_WORD)
            {
                ctx->rxFrame.state = HEADER;
            }

            break;
        case HEADER:
            ret = rx(ctx, &ctx->rxFrame.headerBuffer, bytesWaiting);
            break;
        case PAYLOAD:
            ret = rx(ctx, &ctx->rxFrame.payloadBuffer, bytesWaiting);
            break;
        case CRC:
            ret = rx(ctx, &ctx->rxFrame.crcBuffer, bytesWaiting);
            break;
        default:
            /** This should never happen, but if it does we can catch it. */
            ret = SF_SERIAL_MAC_ERROR_EXCEPTION;
        }
    }
    /** Check for HAL error. */
    if(bytesWaiting < 0)
    {
        ret = SF_SERIAL_MAC_ERROR_HAL_ERROR;
    }
    return ret;
}

void sf_serial_mac_entry(struct sf_serial_mac_ctx *ctx)
{
    /***************************************************************************
     * TX
     */
    sf_serial_mac_halTxCb(ctx);

    /***************************************************************************
     * RX
     */
    sf_serial_mac_halRxCb(ctx);
}

#ifdef __cplusplus
}
#endif
