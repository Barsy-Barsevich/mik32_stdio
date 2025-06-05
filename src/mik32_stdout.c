#include "mik32_stdout.h"

#define PRINTF_BUFFER_SIZE          50
#define PRINTF_FLUSHING_SYMBOL      '\n'

static char mik32_stdout_buffer[PRINTF_BUFFER_SIZE];
static uint32_t mik32_stdout_cnt;
static usart_transaction_t mik32_stdout_trans;
static bool mik32_stdout_blocking_transmit = true;

void mik32_stdout_init(void)
{
    usart_transaction_cfg_t cfg = {
        .host = UART_0,
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
            10000
        );
    }
    else
    {
        // usart_transaction_wait(
        //     &mik32_stdout_trans,
        //     1000000000 // INF_TIMEOUT // AUTO_TIMEOUT
        // );
        // usart_transmit_start(
        //     &mik32_stdout_trans,
        //     mik32_stdout_buffer,
        //     mik32_stdout_cnt
        // );
        usart_transmit(
            &mik32_stdout_trans,
            mik32_stdout_buffer,
            mik32_stdout_cnt,
            10000
        );
    }
    mik32_stdout_cnt = 0;
}

void mik32_stdout_putc(char symbol)
{
    mik32_stdout_buffer[mik32_stdout_cnt++] = symbol;
    if (symbol == PRINTF_FLUSHING_SYMBOL || mik32_stdout_cnt >= PRINTF_BUFFER_SIZE || mik32_stdout_blocking_transmit)
    {
        mik32_stdout_flush();
    }
}