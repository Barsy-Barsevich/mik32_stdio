#include "mik32_stdout.h"
#include <sys/reent.h>

static char mik32_stdout_buffer[PRINTF_BUFFER_SIZE];
static uint32_t mik32_stdout_cnt;
static usart_transaction_t mik32_stdout_trans;
static bool mik32_stdout_blocking_transmit = true;

bool mik32_stdout_uart_init(UART_TypeDef *host, uint32_t baudrate)
{
    switch ((uint32_t)host)
    {
        case (uint32_t)UART_0:
            __HAL_PCC_UART_0_CLK_ENABLE();
            uint8_t uart0_txd_pin = 6;
            PAD_CONFIG->PORT_0_CFG &= ~(0b11 << (2 * uart0_txd_pin));
            PAD_CONFIG->PORT_0_CFG |= (0b01 << (2 * uart0_txd_pin));
            break;
        case (uint32_t)UART_1:
            __HAL_PCC_UART_1_CLK_ENABLE();
            uint8_t uart1_txd_pin = 9;
            PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * uart1_txd_pin));
            PAD_CONFIG->PORT_1_CFG |= (0b01 << (2 * uart1_txd_pin));
            break;
        default: return false;
    }
    /* UART init */
    host->CONTROL3 |= UART_CONTROL3_DMAT_M | UART_CONTROL3_DMAR_M;
    host->CONTROL1 |= UART_CONTROL1_UE_M | UART_CONTROL1_TE_M | UART_CONTROL1_M_8BIT_M;    /* Baudrate */
    uint32_t apbp_clk = HAL_PCC_GetSysClockFreq() / ((PM->DIV_AHB+1)*(PM->DIV_APB_P+1));
    uint32_t divider = apbp_clk / baudrate;
    if (divider < 16) return false;
    host->DIVIDER = divider;
    
    bool host_ready = false;
    uint32_t timeout_us = 100000;
    uint32_t init_start_time = HAL_Micros();
    while (!host_ready && (HAL_Micros() - init_start_time < timeout_us))
    {
        host_ready = (host->FLAGS & UART_FLAGS_TEACK_M) != 0;
    }
    if (HAL_Micros() - init_start_time < timeout_us) return true;
    else return false;
}

void mik32_stdout_init(UART_TypeDef *host, uint32_t baudrate)
{
    if (host != UART_0 && host != UART_1) return;
    if (!mik32_stdout_uart_init(host, baudrate)) return;
    
    stdout->_p = (uint8_t)mik32_stdout_buffer;       //< current position in (some) buffer
    stdout->_r = 0;                         //< read space left for getc()
    stdout->_w = PRINTF_BUFFER_SIZE;        //< write space left for putc()
    /* the buffer (at least 1 byte, if !NULL) */
    stdout->_bf._base = (unsigned char*)mik32_stdout_buffer;
    stdout->_bf._size = PRINTF_BUFFER_SIZE;
    stdout->_lbfsize = -PRINTF_BUFFER_SIZE; //< 0 or -_bf._size, for inline putc
    stdout->_cookie = NULL;                 //< cookie passed to io functions
    stdout->_read = NULL;
    stdout->_write = mik32_stdout_write;
    stdout->_seek = NULL;
    stdout->_close = NULL;
    /* separate buffer for long sequences of ungetc() */
    stdout->_ub._base = NULL;	    //< ungetc buffer
    stdout->_ub._size = 0;
    stdout->_up = 0;                //< saved _p when _p is doing ungetc data
    stdout->_ur = 0;	            //< saved _r when _r is counting ungetc data
    // /* tricks to meet minimum requirements even when malloc() fails */
    // stdout->_ubuf[3];	            //< guarantee an ungetc() buffer
    // stdout->_nbuf[1];	            //< guarantee a getc() buffer
    /* separate buffer for fgetline() when line crosses buffer boundary */
    stdout->_lb._base = NULL;	    //< buffer for fgetline()
    stdout->_lb._size = 0;
    /* Unix stdio files get aligned to block boundaries on fseek() */
    stdout->_blksize = 0;	        //< stat.st_blksize (may be != _bf._size)
    // stdout->_offset;	            //< current lseek offset
    // stdout->_mbstate.__count = 0;	//< for wide char stdio functions.
    // stdout->_mbstate.__value = 0;
    stdout->_flags2 = 0;            //< for future use */


    usart_transaction_cfg_t cfg = {
        .host = host,
        .dma_channel = DMA_CH_AUTO,
        .dma_priority = 0,
        .direction = USART_TRANSACTION_TRANSMIT
    };
    usart_transaction_init(&mik32_stdout_trans, &cfg);
}

void mik32_stdout_enable_blocking(void)
{
    mik32_stdout_blocking_transmit = true;
}

void mik32_stdout_disable_blocking(void)
{
    mik32_stdout_blocking_transmit = false;
}

void __attribute__((weak)) mik32_stdout_flush(void)
{
    if (mik32_stdout_cnt == 0) return;
    if (mik32_stdout_blocking_transmit)
    {
        usart_transmit(
            &mik32_stdout_trans,
            mik32_stdout_buffer,
            mik32_stdout_cnt,
            DMA_TIMEOUT_AUTO
        );
    }
    else
    {
        usart_transaction_wait(
            &mik32_stdout_trans,
            DMA_NO_TIMEOUT
        );
        usart_transmit_start(
            &mik32_stdout_trans,
            mik32_stdout_buffer,
            mik32_stdout_cnt
        );
        // usart_transmit(
        //     &mik32_stdout_trans,
        //     mik32_stdout_buffer,
        //     mik32_stdout_cnt,
        //     10000
        // );
    }
    mik32_stdout_cnt = 0;
}

int mik32_stdout_write(void *__reent, void *, const char *src, int len)
{
    int cnt = 0;
    if (!mik32_stdout_blocking_transmit)
    {
        dma_status_t res = usart_transaction_wait(
            &mik32_stdout_trans,
            DMA_NO_TIMEOUT
        );
        if (res == DMA_STATUS_OK)
        {
            cnt = len;
        }
        else return usart_transaction_done_bytes(&mik32_stdout_trans);
    }
    for (int i=0; i<len; i++)
    {
        mik32_stdout_buffer[mik32_stdout_cnt++] = src[i];
        if (src[i] == PRINTF_FLUSHING_SYMBOL || mik32_stdout_cnt >= PRINTF_BUFFER_SIZE || mik32_stdout_blocking_transmit)
        {
            mik32_stdout_flush();
        }
        cnt += 1;
    }
    return cnt;
}

void mik32_stdout_putc(char symbol)
{
    mik32_stdout_write(NULL, NULL, &symbol, 1);
}