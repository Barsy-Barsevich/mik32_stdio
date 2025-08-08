#include "mik32_stdout.h"
#include <sys/reent.h>

#if defined(MIK32STDOUT_USE_MALLOC)
static char *__buffer;
#else
static char __buffer[MIK32STDOUT_BUFSIZE_DEFAULT];
#endif

static uint32_t __buffer_size = MIK32STDOUT_BUFSIZE_DEFAULT;
static uint32_t __cnt = 0;
static char __flushing_symbol = MIK32STDOUT_FLUSHING_SYMBOL_DEFAULT;
static usart_transaction_t __trans;
static bool __blocking_transmit = true;

mik32_stdio_status_t mik32_stdout_uart_init(UART_TypeDef *host, uint32_t baudrate)
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
        default: return MIK32STDIO_INCORRECT_ARGUMENT;
    }
    /* UART init */
    host->CONTROL3 |= UART_CONTROL3_DMAT_M | UART_CONTROL3_DMAR_M;
    host->CONTROL1 |= UART_CONTROL1_UE_M | UART_CONTROL1_TE_M | UART_CONTROL1_M_8BIT_M;    /* Baudrate */
    uint32_t apbp_clk = HAL_PCC_GetSysClockFreq() / ((PM->DIV_AHB+1)*(PM->DIV_APB_P+1));
    uint32_t divider = apbp_clk / baudrate;
    if (divider < 16) return MIK32STDIO_INCORRECT_ARGUMENT;
    host->DIVIDER = divider;
    
    bool host_ready = false;
    uint32_t timeout_us = 100000;
    uint32_t init_start_time = HAL_Micros();
    while (!host_ready && (HAL_Micros() - init_start_time < timeout_us))
    {
        host_ready = (host->FLAGS & UART_FLAGS_TEACK_M) != 0;
    }
    if (HAL_Micros() - init_start_time < timeout_us) return MIK32STDIO_OK;
    else return MIK32STDIO_TIMEOUT_ERROR;
}

mik32_stdio_status_t mik32_stdout_init(UART_TypeDef *host, uint32_t baudrate)
{
    if (host != UART_0 && host != UART_1) return MIK32STDIO_INCORRECT_ARGUMENT;
    mik32_stdio_status_t res = mik32_stdout_uart_init(host, baudrate);
    if (res != MIK32STDIO_OK) return res;

#if defined(MIK32STDOUT_USE_MALLOC)
    __buffer = (char*)malloc(__buffer_size);
    if (__buffer == NULL) return MIK32STDIO_MALLOC_FAIL;
#endif
    
    stdout->_p = (unsigned char*)__buffer;       //< current position in (some) buffer
    stdout->_r = 0;                         //< read space left for getc()
    stdout->_w = __buffer_size;        //< write space left for putc()
    /* the buffer (at least 1 byte, if !NULL) */
    stdout->_bf._base = (unsigned char*)__buffer;
    stdout->_bf._size = __buffer_size;
    stdout->_lbfsize = -__buffer_size; //< 0 or -_bf._size, for inline putc
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
    dma_status_t dma_st = usart_transaction_init(&__trans, &cfg);
    if (dma_st != DMA_STATUS_OK) return MIK32STDIO_DMA_ERROR;

    return MIK32STDIO_OK;
}

void mik32_stdout_enable_blocking(void)
{
    __blocking_transmit = true;
}

void mik32_stdout_disable_blocking(void)
{
    __blocking_transmit = false;
}

uint32_t mik32_stdout_get_buffer_size(void)
{
    return __buffer_size;
}

mik32_stdio_status_t mik32_stdout_set_buffer_size(uint32_t size)
{
#if defined (MIK32STDOUT_USE_MALLOC)
    if (size <= 1) MIK32STDIO_INCORRECT_ARGUMENT;
    free(__buffer);
    __buffer = (char*)malloc(__buffer_size);
    if (__buffer == NULL) return MIK32STDIO_MALLOC_FAIL;
    __buffer_size = size;
    __cnt = 0;
    stdin->_p = (unsigned char*)__buffer;
    stdin->_bf._base = (unsigned char*)__buffer;
    stdin->_w = __buffer_size;
    stdin->_bf._size = __buffer_size;
    stdin->_lbfsize = -__buffer_size;
#endif
    return MIK32STDIO_OK;
}

char mik32_stdout_get_flushing_symbol(void)
{
    return __flushing_symbol;
}

void mik32_stdout_set_flushing_symbol(char sym)
{
    __flushing_symbol = sym;
}

void __attribute__((weak)) mik32_stdout_flush(void)
{
    if (__cnt == 0) return;
    if (__blocking_transmit)
    {
        usart_transmit(
            &__trans,
            __buffer,
            __cnt,
            DMA_TIMEOUT_AUTO
        );
    }
    else
    {
        usart_transaction_wait(
            &__trans,
            DMA_NO_TIMEOUT
        );
        usart_transmit_start(
            &__trans,
            __buffer,
            __cnt
        );
        // usart_transmit(
        //     &__trans,
        //     __buffer,
        //     __cnt,
        //     10000
        // );
    }
    __cnt = 0;
}

int mik32_stdout_write(void *__reent, void *, const char *src, int len)
{
    int cnt = 0;
    if (!__blocking_transmit)
    {
        dma_status_t res = usart_transaction_wait(
            &__trans,
            DMA_NO_TIMEOUT
        );
        if (res == DMA_STATUS_OK)
        {
            cnt = len;
        }
        else return usart_transaction_done_bytes(&__trans);
    }
    for (int i=0; i<len; i++)
    {
        __buffer[__cnt++] = src[i];
        if (src[i] == __flushing_symbol || __cnt >= __buffer_size || __blocking_transmit)
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