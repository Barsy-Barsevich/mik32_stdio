#include "mik32_stdin.h"

#if defined (MIK32STDIN_USE_MALLOC)
static char *__buffer;
#else
static char __buffer[MIK32STDIN_BUFSIZE_DEFAULT];
#endif

static size_t __cnt;
static usart_transaction_t __trans;
static bool __blocking_receive = true;
static uint32_t __buffer_size = MIK32STDIN_BUFSIZE_DEFAULT;

void mik32_stdin_init(UART_TypeDef *host)
{
    if (host != UART_0 && host != UART_1) return;
    
    stdin->_p = (uint8_t*)__buffer;     //< current position in (some) buffer
    stdin->_r = 0;                      //< read space left for getc()
    stdin->_w = __buffer_size;          //< write space left for putc()
    stdin->_bf._base = (unsigned char*)__buffer;
    stdin->_bf._size = __buffer_size;   //< the buffer (at least 1 byte, if !NULL)
    stdin->_lbfsize = -__buffer_size;   //< 0 or -_bf._size, for inline putc
    stdin->_cookie = NULL;              //< cookie passed to io functions
    stdin->_read = mik32_stdin_read;
    stdin->_write = NULL;
    stdin->_seek = NULL;
    stdin->_close = NULL;
    stdin->_blksize = 0;	        //< stat.st_blksize (may be != _bf._size)
    stdin->_flags2 = 0;            //< for future use */

#if defined (MIK32STDIN_USE_MALLOC)
    __buffer = malloc(__buffer_size);
#endif

    usart_transaction_cfg_t cfg = {
        .host = host,
        .dma_channel = DMA_CH_AUTO,
        .dma_priority = 0,
        .direction = USART_TRANSACTION_RECEIVE
    };
    usart_transaction_init(&__trans, &cfg);
}

void mik32_stdin_enable_blocking(void)
{
    __blocking_receive = true;
}

void mik32_stdin_disable_blocking(void)
{
    __blocking_receive = false;
    usart_receive_start(
        &__trans,
        __buffer,
        __buffer_size
    );
}

uint32_t mik32_stdin_get_buffer_size(void)
{
    return __buffer_size;
}

void mik32_stdin_set_buffer_size(uint32_t size)
{
#if defined (MIK32STDIN_USE_MALLOC)
    if (size <= 1) return;
    __buffer_size = size;
    free(__buffer);
    __buffer = malloc(__buffer_size);
    if (!__blocking_receive)
    {
        usart_receive_start(
            &__trans,
            __buffer,
            __buffer_size
        );
    }
#endif
}

int mik32_stdin_read(void *__reent, void *dummy, char *dst, int len)
{
    if (__blocking_receive)
    {
        dma_status_t res = usart_receive(
            &__trans,
            dst,
            len,
            DMA_NO_TIMEOUT
        );
        if (res == DMA_STATUS_OK)
        {
            return len;
        }
        else return usart_transaction_done_bytes(&__trans);
    }
    else //< non-blocking receive
    {
        xprintf("#%u#", usart_transaction_ready(&__trans));
        if (__cnt >= __buffer_size) __cnt = 0;
        bool trans_ready = usart_transaction_ready(&__trans);
        if (trans_ready)
        {
            // xprintf(" ENTER_TRANS_READY:\n");
            while (__cnt != __buffer_size && len > 0)
            {
                *dst++ = __buffer[__cnt++];
                len -= 1;
            }
            if (len == 0)
            {
                /* Start transaction from 0, len=cnt */
                usart_receive_start(
                    &__trans,
                    __buffer,
                    __cnt
                );
                return 0;
            }
        }
        /* Transaction has not been finished OR il faut to read more */
        do {
            while (!usart_transaction_ready(&__trans) && len > 0)
            {
                /* Delay if the last buffer symbol :( */
                if (__cnt == __buffer_size-1)
                {
                    uint32_t ticks_per_us = HAL_PCC_GetSysClockFreq() / ((PM->DIV_AHB+1) * (PM->DIV_APB_P+1) * 1000000);
                    if (ticks_per_us == 0) ticks_per_us = 1;
                    uint32_t delay_us = __trans.host->DIVIDER / ticks_per_us;
                    HAL_DelayUs(delay_us);
                }
                uint32_t done = usart_transaction_done_bytes(&__trans);
                uint32_t bytes_to_read = done - __cnt;
                while (bytes_to_read-- > 0 && len-- > 0)
                {
                    *dst++ = __buffer[__cnt++];
                }
            }
            trans_ready = usart_transaction_ready(&__trans);
            if (!trans_ready)
            {
                return 0;
            }
            __cnt = 0;
            /* Start transaction from 0, len=BUFSIZE */
            usart_receive_start(
                &__trans,
                __buffer,
                __buffer_size
            );
            return 0;
        } while (1);
    }
}

char mik32_stdin_getc(void)
{
    char dummy;
    mik32_stdin_read(NULL, NULL, &dummy, 1);
    return dummy;
}