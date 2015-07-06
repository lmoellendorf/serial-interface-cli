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
#define UINT8_TO_UINT16(u16var, u8arr) u16var = (((((uint16_t)((u8arr)[0]))<<8U) & 0xFF00U) | \
                                                  (((uint16_t)((u8arr)[1])) & 0xFFU))

/*==============================================================================
 |                                   ENUMS
 =============================================================================*/
/**
 * RX state
 */
enum rxState {
    HEADER,
    PAYLOAD,
    CRC,
    UNKNOWN,
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

struct sf_serial_mac_buffer
{
    char *memory;
    size_t length;
    size_t remains;
    SF_SERIAL_MAC_BUF_EVT callback;
};

struct sf_serial_mac_frame
{
    /** Buffer for the MAC header: [SYNC] [Length field] */
    uint8_t header[SF_SERIAL_MAC_PROTOCOL_HEADER_LEN];
    uint16_t remains;
    /** Checksum. */
    uint8_t crc[SF_SERIAL_MAC_PROTOCOL_CRC_FIELD_LEN];
};

struct sf_serial_mac_ctx
{
    void *portHandle;
    SF_SERIAL_MAC_HAL_READ_FUNC read;
    SF_SERIAL_MAC_HAL_READ_WAIT_FUNC readWait;
    SF_SERIAL_MAC_HAL_WRITE_FUNC write;
    SF_SERIAL_MAC_RX_EVT rxEvt;
    SF_SERIAL_MAC_RX_EVT rxBufEvt;
    SF_SERIAL_MAC_TX_EVT txEvt;
    SF_SERIAL_MAC_TX_EVT txBufEvt;
    struct sf_serial_mac_buffer readBuffer;
    struct sf_serial_mac_buffer writeBuffer;
    struct sf_serial_mac_buffer txHeaderBuffer;
    struct sf_serial_mac_buffer txCrcBuffer;
    struct sf_serial_mac_buffer rxHeaderBuffer;
    struct sf_serial_mac_buffer rxCrcBuffer;
    struct sf_serial_mac_frame txFrame;
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
static size_t tx(struct sf_serial_mac_ctx *ctx, struct sf_serial_mac_buffer
                 *buffer, size_t txAtMost, uint8_t *crc);
static void txProcHeaderCB(struct sf_serial_mac_ctx *ctx);
static void txProcPayloadCB(struct sf_serial_mac_ctx *ctx);
static void txProcCrcCB(struct sf_serial_mac_ctx *ctx);
static size_t rx(struct sf_serial_mac_ctx *ctx, struct sf_serial_mac_buffer
                 *buffer, size_t max, size_t min, size_t bytesWaiting);
static void rxProcHeaderCB(struct sf_serial_mac_ctx *ctx);
static void rxProcPayloadCB(struct sf_serial_mac_ctx *ctx);
static void rxProcCrcCB(struct sf_serial_mac_ctx *ctx);
static enum rxState checkRxState(struct sf_serial_mac_ctx *ctx);

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
    /** zero buffer */
    memset((void *) frame->header, 0, SF_SERIAL_MAC_PROTOCOL_HEADER_LEN);
    /** Write the sync word into the buffer */
    frame->header[0] = syncWord;
    frame->remains = 0;
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
static size_t tx(struct sf_serial_mac_ctx *ctx, struct sf_serial_mac_buffer
                 *buffer, size_t max, uint8_t *crc)
{
    size_t byteToSend = 0;
    size_t byteSent = 0;
    uint16_t crcVal = 0;

    if(ctx && buffer)
    {

        /** Check if we (still) have bytes to send */
        if (buffer->remains)
        {
            byteToSend = buffer->remains > max ? max : buffer->remains;
            /** Send the bytes */
            byteSent = ctx->write(ctx->portHandle,
                                  buffer->memory
                                  + (buffer->length - buffer->remains), byteToSend);
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
                crcVal = crc_calc(crcVal, (uint8_t*) buffer->memory
                                  + (buffer->length - buffer->remains),
                                  byteSent);
                UINT16_TO_UINT8(crc, crcVal);
            }

            /** update to the number of byte already sent */
            buffer->remains -= byteSent;
        }

        /** Check if all bytes have been sent */
        if (byteSent == byteToSend)
        {
            buffer->callback(ctx);
            /**
             * Clear the buffer, so the write event won't be called again
             * and again.
             */
            initBuffer(buffer, NULL, 0, buffer->callback);
        }
    }

    return byteSent;
}

static void txProcHeaderCB(struct sf_serial_mac_ctx *ctx)
{
    /** do nothing */
    /**
     * TODO: Maybe we could handle the frame processing state using
     * these callbacks:
     *
     * We could add a struct sf_serial_mac_buffer *currentBuffer
     * to the struct sf_serial_mac_frame which is then always
     * passed to tx().
     *
     * In sf_serial_mac_txFrameStart this pointer would then
     * be set to ctx->headerBuffer.
     * In txProcHeaderCB() the pointer would then be moved to
     * ctx->writeBuffer.
     * In sf_serial_mac_halTxCb() only one check would remain:
     * if(ctx->frame.processed >= length)</pre>. If true the current
     * buffer would be switched to ctx->crcBuffer.
     *
     * However, there is one drawback:
     *
     * Currently in sf_serial_mac_halTxCb() tx() is
     * called often enough to process a whole frame in case all data is already
     * available. If the approach mentioned above is used
     * txProcHeaderCB() and txProcPayloadCB() would
     * have to call tx() directly to achieve the same effect.
     *
     * I will commit the current state and then try this proposal.
     */
}

static void txProcPayloadCB(struct sf_serial_mac_ctx *ctx)
{
    /** inform upper layer that the buffer has been processed */
    ctx->txBufEvt(ctx->writeBuffer.length - ctx->writeBuffer.remains);
}

static void txProcCrcCB(struct sf_serial_mac_ctx *ctx)
{
    uint16_t length = 0;
    UINT8_TO_UINT16(length, ctx->txFrame.header +
                    SF_SERIAL_MAC_PROTOCOL_SYNC_WORD_LEN);
    /** inform the upper layer that a frame has been completed */
    ctx->txEvt(length);
    /**
     * Prepare the frame structure for the next frame.
     */
    initFrame(&ctx->txFrame, SF_SERIAL_MAC_PROTOCOL_SYNC_WORD);
}

//TODO: move into local rx() - add min. and max. bytes to receive, use callbacks to adjust state (just like in tx)
static size_t rx(struct sf_serial_mac_ctx *ctx, struct sf_serial_mac_buffer
                 *buffer, size_t max, size_t min, size_t bytesWaiting)
{
    size_t byteToReceive = 0;
    size_t bytesReceived = 0;

    /**
     * TODO: maybe min is not needed?
     */
    if (bytesWaiting >= min)
    {
        max = buffer->remains > max ? max : buffer->remains;
        byteToReceive = max > bytesWaiting ? bytesWaiting : max;
        if((bytesReceived = ctx->read(ctx->portHandle,
                                      (void*) (buffer->memory + buffer->length - buffer->remains),
                                      byteToReceive)) < 0)
        {
//            return SF_SERIAL_MAC_ERROR_HAL_ERROR;
            return 0;
        }
        if (byteToReceive != bytesReceived)
        {
            /** This should never happen, but if it does we can catch it. */
//            return SF_SERIAL_MAC_ERROR_EXCEPTION;
            return 0;
        }
        buffer->remains -= bytesReceived;
        if(buffer->remains == 0)
        {
            buffer->callback(ctx);
        }
    }
    return bytesReceived;
}

static void rxProcHeaderCB(struct sf_serial_mac_ctx *ctx)
{
    if(ctx->rxHeaderBuffer.memory[0] == (char) SF_SERIAL_MAC_PROTOCOL_SYNC_WORD)
    {
        /** Start the countdown */
        UINT8_TO_UINT16(ctx->rxFrame.remains, ctx->rxFrame.header +
                        SF_SERIAL_MAC_PROTOCOL_SYNC_WORD_LEN);
        /** Inform upper layer that there has been a frame header received */
        ctx->rxBufEvt(NULL, ctx->rxFrame.remains);
    }
}

static void rxProcPayloadCB(struct sf_serial_mac_ctx *ctx)
{
    //FIXME: throw error here!!
    if(ctx->rxFrame.remains <= 0)
    {
        initBuffer(&ctx->rxCrcBuffer, ctx->rxFrame.crc,
                   SF_SERIAL_MAC_PROTOCOL_CRC_FIELD_LEN, rxProcCrcCB);
    }
}

//FIXME: unify with txProcCrcCB!
static void rxProcCrcCB(struct sf_serial_mac_ctx *ctx)
{
    uint16_t crc = 0;
    uint16_t crcCheck = 0;
    uint16_t length = 0;
    UINT8_TO_UINT16(length, ctx->rxFrame.header +
                    SF_SERIAL_MAC_PROTOCOL_SYNC_WORD_LEN);
    UINT8_TO_UINT16(crc, ctx->rxCrcBuffer.memory);
    if((crcCheck = crc_calc_finalize(ctx->readBuffer.memory,
                                     length)) == crc)
    {
        /** inform the upper layer that a frame has been completed */
        ctx->rxEvt(ctx->readBuffer.memory, length);
        /**
         * Prepare the frame structure for the next frame.
         */
        initFrame(&ctx->rxFrame, 0);
    }
}

enum rxState checkRxState(struct sf_serial_mac_ctx *ctx)
{
    if(ctx->rxFrame.remains == 0 && ctx->rxCrcBuffer.memory == NULL)
    {
        return HEADER;
    }
    /**
    * Check if a read buffer for reading the payload has been assigned.
    * This works because the buffer is set to NULL if no payload is
    * pending.
    */
    else if(ctx->rxFrame.remains > 0 && ctx->readBuffer.memory != NULL)
    {
        return PAYLOAD;
    }
    else if(ctx->rxFrame.remains == 0 && ctx->rxCrcBuffer.memory != NULL)
    {
        return CRC;
    }
    else
    {
        return UNKNOWN;
    }
};


/*==============================================================================
 |                               API FUNCTIONS
 =============================================================================*/
size_t sf_serial_mac_ctx_size(void)
{
    return sizeof(struct sf_serial_mac_ctx);
}


// TODO: split into local init rx and init tx functions
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
    initBuffer(&ctx->writeBuffer, NULL, 0, txProcPayloadCB);
    initBuffer(&ctx->txHeaderBuffer, NULL, 0, txProcHeaderCB);
    initBuffer(&ctx->txCrcBuffer, NULL, 0, txProcCrcCB);
    initFrame(&ctx->txFrame, SF_SERIAL_MAC_PROTOCOL_SYNC_WORD);
    initBuffer(&ctx->readBuffer, NULL, 0, rxProcPayloadCB);
    /**
     * By assigning memory to the header buffer MAC is ready to receive a new frame
     * with the next call to sf_serial_mac_halTxCb().
     */
    initBuffer(&ctx->rxHeaderBuffer, (uint8_t*) &ctx->rxFrame.header,
               sizeof ctx->rxFrame.header, rxProcHeaderCB);
    initBuffer(&ctx->rxCrcBuffer, NULL, 0, rxProcCrcCB);
    initFrame(&ctx->rxFrame, 0);

    return SF_SERIAL_MAC_SUCCESS;
}

enum sf_serial_mac_return sf_serial_mac_txFrameStart(struct sf_serial_mac_ctx
        *ctx, size_t len)
{
    if (!ctx)
    {
        return SF_SERIAL_MAC_ERROR_NPE;
    }
    if(ctx->txCrcBuffer.memory)
    {
        return SF_SERIAL_MAC_ERROR_FRM_PENDING;
    }
    /**
     * By assigning memory to the header buffer a new frame is started with the
     * next call to sf_serial_mac_halTxCb().
     */
    initBuffer(&ctx->txHeaderBuffer, (uint8_t*) &ctx->txFrame.header,
               sizeof ctx->txFrame.header, txProcHeaderCB);
    /**
     * By assigning memory to the CRC buffer starting a new frame is prevented
     * untill sf_serial_mac_halTxCb() processed the whole frame.
     */
    initBuffer(&ctx->txCrcBuffer, (uint8_t*) &ctx->txFrame.crc,
               sizeof ctx->txFrame.crc, txProcCrcCB);
    /** Write frame length into the length field of the frame header */
    UINT16_TO_UINT8(ctx->txFrame.header + SF_SERIAL_MAC_PROTOCOL_SYNC_WORD_LEN,
                    len);
    ctx->txFrame.remains = len;
    return SF_SERIAL_MAC_SUCCESS;
}

enum sf_serial_mac_return sf_serial_mac_txFrameAppend(struct sf_serial_mac_ctx
        *ctx,
        const char *frmBufLoc, size_t frmBufSize)
{
    if (!ctx)
    {
        return SF_SERIAL_MAC_ERROR_NPE;
    }
    if(ctx->writeBuffer.memory)
    {
        return SF_SERIAL_MAC_ERROR_RW_PENDING;
    }
    /**
     * By assigning memory to the payload buffer the payload is transmitted with
     * the next call to sf_serial_mac_halTxCb() and consecutive tries
     * to assign payload buffers are prevented untill the currently assigned
     * buffer has been processed.
     */
    initBuffer(&ctx->writeBuffer, (char*) frmBufLoc, frmBufSize, txProcPayloadCB);
    return SF_SERIAL_MAC_SUCCESS;
}

enum sf_serial_mac_return sf_serial_mac_txFrame(struct sf_serial_mac_ctx *ctx,
        size_t frmLen, const char *frmBufLoc, size_t frmBufSize)
{
    sf_serial_mac_txFrameStart(ctx, frmLen);
    return sf_serial_mac_txFrameAppend(ctx, frmBufLoc, frmBufSize);
}

enum sf_serial_mac_return sf_serial_mac_rxFrame(struct sf_serial_mac_ctx *ctx,
        char *frmBufLoc,
        size_t frmBufSize)
{
    if (!ctx || !frmBufLoc)
    {
        return SF_SERIAL_MAC_ERROR_NPE;
    }
    if(ctx->readBuffer.memory != NULL)
    {
        return SF_SERIAL_MAC_ERROR_RW_PENDING;
    }
    if(frmBufSize < ctx->rxFrame.remains)
    {
        return SF_SERIAL_MAC_ERROR_BUFFER;
    }
    initBuffer(&ctx->readBuffer, frmBufLoc, frmBufSize, rxProcPayloadCB);
    memset((void *) ctx->readBuffer.memory, 0, ctx->readBuffer.length);
    return SF_SERIAL_MAC_SUCCESS;
}

enum sf_serial_mac_return sf_serial_mac_halTxCb(struct sf_serial_mac_ctx *ctx)
{
    uint16_t crc = 0;
    /** Do nothing if there is no context. */
    if (!ctx)
    {
        return SF_SERIAL_MAC_ERROR_NPE;
    }
    /** If memory has been assigned, then there is a header to process */
    if (ctx->txHeaderBuffer.memory)
    {
        tx(ctx, &ctx->txHeaderBuffer, SF_SERIAL_MAC_PROTOCOL_HEADER_LEN, NULL);
    }

    //TODO: add a getState method for tx just like in rx
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
        ctx->txFrame.remains -= tx(ctx, &ctx->writeBuffer,
                                   ctx->txFrame.remains,
                                   (uint8_t *) &ctx->txFrame.crc);
    }

    /**
     * If the number of processed bytes is greater or equal than the length of payload
     * the CRC has to be processed.
     */
    if (ctx->txCrcBuffer.memory && !ctx->txFrame.remains)
    {
        UINT8_TO_UINT16(crc, ctx->txFrame.crc);
        crc = crc_finalize(crc);
        UINT16_TO_UINT8(ctx->txFrame.crc, crc);
        tx(ctx, &ctx->txCrcBuffer, SF_SERIAL_MAC_PROTOCOL_CRC_FIELD_LEN, NULL);
    }

    return SF_SERIAL_MAC_SUCCESS;
}

enum sf_serial_mac_return sf_serial_mac_halRxCb(struct sf_serial_mac_ctx *ctx)
{
    size_t bytesWaiting = 0;
    size_t bytesMaximum = 0;
    uint16_t length = 0;

    /** Do nothing if there is no context. */
    if (!ctx)
    {
        return SF_SERIAL_MAC_ERROR_NPE;
    }

    /**
     * Check if there are bytes to be read.
     */
    if((bytesWaiting = ctx->readWait(ctx->portHandle)) < 0)
    {
        return SF_SERIAL_MAC_ERROR_HAL_ERROR;
    }
    /** Is there anything to receive? */
    if (bytesWaiting > 0)
    {

        switch (checkRxState(ctx))
        {
        case HEADER:
            rx(ctx, &ctx->rxHeaderBuffer,SF_SERIAL_MAC_PROTOCOL_HEADER_LEN,
               SF_SERIAL_MAC_PROTOCOL_HEADER_LEN, bytesWaiting);
            //TODO checkState here and fall through if state is PAYLOAD
            break;
        case PAYLOAD:
            ctx->rxFrame.remains -= rx(ctx, &ctx->readBuffer,ctx->rxFrame.remains, 1,
                                       bytesWaiting);
            /** In order to switch the state according to remaining bytes in frame: */
            rxProcPayloadCB(ctx);
            break;
        case CRC:
            rx(ctx, &ctx->rxCrcBuffer,SF_SERIAL_MAC_PROTOCOL_CRC_FIELD_LEN,
               SF_SERIAL_MAC_PROTOCOL_CRC_FIELD_LEN, bytesWaiting);
            break;
        default:
            /** This should never happen, but if it does we can catch it. */
            return SF_SERIAL_MAC_ERROR_EXCEPTION;
        }
    }
    return SF_SERIAL_MAC_SUCCESS;
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
