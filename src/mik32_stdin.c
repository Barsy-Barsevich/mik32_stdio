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

mik32_stdio_status_t mik32_stdin_uart_init(UART_TypeDef *host, uint32_t baudrate)
{
    switch ((uint32_t)host)
    {
        case (uint32_t)UART_0:
            __HAL_PCC_UART_0_CLK_ENABLE();
            uint8_t uart0_rxd_pin = 5;
            PAD_CONFIG->PORT_0_CFG &= ~(0b11 << (2 * uart0_rxd_pin));
            PAD_CONFIG->PORT_0_CFG |= (0b01 << (2 * uart0_rxd_pin));
            break;
        case (uint32_t)UART_1:
            __HAL_PCC_UART_1_CLK_ENABLE();
            uint8_t uart1_rxd_pin = 8;
            PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * uart1_rxd_pin));
            PAD_CONFIG->PORT_1_CFG |= (0b01 << (2 * uart1_rxd_pin));
            break;
        default: return MIK32STDIO_INCORRECT_ARGUMENT;
    }
    /* UART init */
    host->CONTROL3 |= UART_CONTROL3_DMAT_M | UART_CONTROL3_DMAR_M;
    host->CONTROL1 |= UART_CONTROL1_UE_M | UART_CONTROL1_RE_M | UART_CONTROL1_M_8BIT_M;    /* Baudrate */
    uint32_t apbp_clk = HAL_PCC_GetSysClockFreq() / ((PM->DIV_AHB+1)*(PM->DIV_APB_P+1));
    uint32_t divider = apbp_clk / baudrate;
    if (divider < 16) return MIK32STDIO_INCORRECT_ARGUMENT;
    host->DIVIDER = divider;
    
    bool host_ready = false;
    uint32_t timeout_us = 100000;
    uint32_t init_start_time = HAL_Micros();
    while (!host_ready && (HAL_Micros() - init_start_time < timeout_us))
    {
        host_ready = (host->FLAGS & UART_FLAGS_REACK_M) != 0;
    }
    if (HAL_Micros() - init_start_time < timeout_us) return MIK32STDIO_OK;
    else return MIK32STDIO_TIMEOUT_ERROR;
}

mik32_stdio_status_t mik32_stdin_init(UART_TypeDef *host, uint32_t baudrate)
{
    if (host != UART_0 && host != UART_1) return MIK32STDIO_INCORRECT_ARGUMENT;
    mik32_stdio_status_t res = mik32_stdin_uart_init(host, baudrate);
    if (res != MIK32STDIO_OK) return res;

#if defined (MIK32STDIN_USE_MALLOC)
    __buffer = malloc(__buffer_size);
    if (__buffer == NULL) return MIK32STDIO_MALLOC_FAIL;
#endif
    
    stdin->_p = (unsigned char*)__buffer; //< current position in (some) buffer
    stdin->_r = __buffer_size;          //< read space left for getc()
    stdin->_w = 0;                      //< write space left for putc()
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

    usart_transaction_cfg_t cfg = {
        .host = host,
        .dma_channel = DMA_CH_AUTO,
        .dma_priority = 0,
        .direction = USART_TRANSACTION_RECEIVE
    };
    dma_status_t dma_st = usart_transaction_init(&__trans, &cfg);
    if (dma_st != DMA_STATUS_OK) return MIK32STDIO_DMA_ERROR;

    return MIK32STDIO_OK;
}

void mik32_stdin_enable_blocking(void)
{
    __blocking_receive = true;
}

mik32_stdio_status_t mik32_stdin_disable_blocking(void)
{
    __blocking_receive = false;
    dma_status_t dma_st = usart_receive_start(
        &__trans,
        __buffer,
        __buffer_size
    );
    if (dma_st != DMA_STATUS_OK) return MIK32STDIO_DMA_ERROR;
    return MIK32STDIO_OK;
}

uint32_t mik32_stdin_get_buffer_size(void)
{
    return __buffer_size;
}

mik32_stdio_status_t mik32_stdin_set_buffer_size(uint32_t size)
{
#if defined (MIK32STDIN_USE_MALLOC)
    if (size <= 1) return MIK32STDIO_INCORRECT_ARGUMENT;
    free(__buffer);
    __buffer = malloc(size);
    if (__buffer == NULL) return MIK32STDIO_MALLOC_FAIL;
    __buffer_size = size;
    if (!__blocking_receive)
    {
        dma_status_t dma_st = usart_receive_start(
            &__trans,
            __buffer,
            __buffer_size
        );
        if (dma_st != DMA_STATUS_OK) return MIK32STDIO_DMA_ERROR;
    }
    stdin->_p = (unsigned char*)__buffer;
    stdin->_bf._base = (unsigned char*)__buffer;
    stdin->_r = __buffer_size;
    stdin->_bf._size = __buffer_size;
    stdin->_lbfsize = -__buffer_size;
#endif
    return MIK32STDIO_OK;
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