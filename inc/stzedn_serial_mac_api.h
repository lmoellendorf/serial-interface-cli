/*============================================================================*/
/*! \file   stzedn_serial_mac_api.h

    \author Copyright (c) by STZEDN, Heitersheim, Germany, http://www.stzedn.de

    \brief  API of the serial MAC driver module.
            
   \version 0.1
   
   \date    
*/
/*============================================================================*/
#ifndef __STZEDN_SERIAL_MAC_API_H__
#define __STZEDN_SERIAL_MAC_API_H__

/*==============================================================================
                                 INCLUDE FILES
==============================================================================*/
#include <stdint.h>
#include <stdbool.h>

/*==============================================================================
                                     MACROS
==============================================================================*/

/*==============================================================================
                                     ENUMS
==============================================================================*/

/*==============================================================================
                         STRUCTURES AND OTHER TYPEDEFS
==============================================================================*/

/*==============================================================================
                          GLOBAL VARIABLE DECLARATIONS
==============================================================================*/

/*==============================================================================
                         FUNCTION PROTOTYPES OF THE API
==============================================================================*/

/** \defgroup serial_mac_api Serial MAC API */
/* @{ */

/*! Callback to write to UART */
typedef uint16_t (pf_cbUartWrite_t)(uint8_t *pc_data,  uint16_t i_len);
/*! Callback to read from UART */
typedef uint16_t (pf_cbUartRead_t)(uint8_t *pc_data,  uint16_t i_len);

typedef struct
{
  /*! Required: Callback to write to serial port. */
  pf_cbUartWrite_t *p_serial_write;
  
  /*! Required: Callback to read from serial port. */
  pf_cbUartRead_t *p_serial_read;
  
  /*! Optional, but recommended to be used: 
      RX event callback that is called if a serial MAC frame is received 
      completely. */
  void *p_cb_rx;
  
  /*! Optional: TX event callback that is called if a serial MAC frame is 
      transmitted completely. */
  void *p_cb_tx;

} s_serial_mac_t;

/*============================================================================*/
/*!
 * @brief  Initializes the serial MAC driver to be ready for operation.
 *         Might be optional.
 *
 * @param  None.
 * @return None.
 */
/*============================================================================*/
void stzedn_serial_mac_init(void);

/*============================================================================*/
/*!
 * @brief  Starts the serial MAC layer for the given context and initializes
 *         callbacks.
 *
 * @param ps_ctx   Pointer to context.
 * @param ps_write
 * @param ps_read
 * @param p_cb_rx
 * @param p_cb_tx
 * @return TRUE, if started successfully. FALSE otherwise.
 */
/*============================================================================*/
bool stzedn_serial_mac_start( s_serial_mac_t   *ps_ctx,
                                pf_cbUartWrite_t *ps_write,
                                pf_cbUartRead_t  *ps_read,
                                void *p_cb_rx,
                                void *p_cb_tx);

/*============================================================================*/
/*!
 * @brief  Runs the serial MAC driver.
 *         This function has to be called as often as possible to run the driver 
 *         properly.
 * 
 * @param ps_ctx   Pointer to context.
 * @return None.
 */
/*============================================================================*/
void stzedn_serial_mac_run( s_serial_mac_t *ps_ctx );

/*============================================================================*/
/*!
 * @brief  Reads data from the serial MAC RX buffer and stores it to given memory.
 * 
 * @details Reads data out of the serial MAC buffer that was received through
 *          the serial interface. Once a byte is read using this function it is 
 *          cleared in the serial MAC buffer. 
 *          If no new data was received (meaning no new data is present to by 
 *          read) this function will not copy any data to the given memory and 
 *          return a read length of 0 instead.
 *
 * @param ps_ctx   Pointer to context.
 * @param pc_data  Pointer to the memory where to copy the read bytes to.
 * @param i_len    Number of bytes to read.
 * @return         Number of read bytes. Returns 0 if no new data is received
 *                 and therefore no data is copied to the given memory.
 */
/*============================================================================*/
uint16_t stzedn_serial_mac_read( s_serial_mac_t *ps_ctx, 
                                 uint8_t *pc_data, 
                                 uint16_t i_len );

/*============================================================================*/
/*!
 * @brief  Writes data to the serial MAC TX buffer to be transmitted through the 
 *         serial interface.
 * 
 * @details The first call to this function signals to the serial MAC module 
 *          that a new MAC frame is started. It is possible to call this 
 *          function a multiple times to append payload to the serial MAC frame. 
 *          Therefore, the data written to the driver using this function is 
 *          buffered in the driver first. The current serial MAC frame is 
 *          finalized by a call to @ref stzedn_serial_mac_txFlush().
 *
 * @param ps_ctx   Pointer to context.
 * @param pc_data  Pointer to the memory holding the data to be transmitted.
 * @param i_len    Number of bytes to transmit.
 * @return         Number of bytes written to the TX buffer successfully.
 *                 Returns 0 if the TX buffer is full and no data can be added.
 */
/*============================================================================*/
uint16_t stzedn_serial_mac_write( s_serial_mac_t *ps_ctx, 
                                  uint8_t *pc_data, 
                                  uint16_t i_len );

/*============================================================================*/
/*!
 * @brief Triggers flushing the TX data buffer of the serial MAC driver.
 * 
 * @details  The function is called to finalize the current serial MAC frame and
 *           to trigger flushing the data currently stored in the TX buffer.
 *           Therefore, this function will be called in order to indicate that a 
 *           complete serial frame is written to this driver 
 *           (using @ref stzedn_serial_mac_write()). Internal actions started by
 *           a call to this function are therefore:
 *           - Transmit SYNC byte
 *           - Transmit LENGTH field (current number of bytes in TX buffer)
 *           - Transmit user given PAYLOAD stored in TX buffer
 *           - Finalize CRC calculation on serial MAC frame stored in TX buffer
 *           - Transmit calculated CRC of serial MAC frame
 *
 *           Please note that the serial MAC layer is implemented unblocking and
 *           therefore the function @ref stzedn_serial_mac_run() must be called
 *           periodically to ensure proper transmission of the data.
 *
 *           Also please note that calling @ref stzedn_serial_mac_write()
 *           next time will result in a new serial MAC frame.
 *
 * @param  ps_ctx - Pointer to context.
 * @return None.
 */
/*============================================================================*/
void stzedn_serial_mac_txFlush(s_serial_mac_t *ps_ctx);

/*============================================================================*/
/*!
 * @brief Returns the number of received but not read bytes within the RX buffer.
 * 
 * @details  Returns the number of bytes in the RX buffer that is received 
 *           through the serial MAC but not yet read using 
 *           @ref stzedn_serial_mac_read().
 *           Actually, this function is optional as 
 *           @ref stzedn_serial_mac_read() can be called any time and returns 0 
 *           in case no new data is read.
 * 
 * @param  ps_ctx - Pointer to context.
 * @return Number of received bytes.
 */
/*============================================================================*/
uint16_t stzedn_serial_mac_cntRxBytes(s_serial_mac_t *ps_ctx);

/*============================================================================*/
/*!
 * @brief Returns the number of bytes written to the TX buffer.
 * 
 * @details  Returns the number of bytes in the TX buffer that is not yet 
 *           transmitted through the serial.
 *           Actually, this function is optional.
 * 
 * @param  ps_ctx - Pointer to context.
 * @return Number of bytes in the TX buffer.
 */
/*============================================================================*/
uint16_t stzedn_serial_mac_cntTxBytes(s_serial_mac_t *ps_ctx);

/* @} */ /* end of serial_mac_api */

/*============================================================================*/
#endif /* __STZEDN_SERIAL_MAC_API_H__ */
