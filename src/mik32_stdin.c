#include "mik32_stdin.h"

static char mik32_stdin_buffer[MIK32STDIN_BUFFER_SIZE];
static size_t mik32_stdin_cnt;
static usart_transaction_t mik32_stdin_trans;
static bool mik32_stdio_blocking_receive = true;

void mik32_stdin_init(void)
{
    stdin->_p = mik32_stdin_buffer;       //< current position in (some) buffer
    stdin->_r = 0;                         //< read space left for getc()
    stdin->_w = MIK32STDIN_BUFFER_SIZE;        //< write space left for putc()
    stdin->_bf._base = (unsigned char*)mik32_stdin_buffer;
    stdin->_bf._size = MIK32STDIN_BUFFER_SIZE; //< the buffer (at least 1 byte, if !NULL)
    stdin->_lbfsize = -MIK32STDIN_BUFFER_SIZE; //< 0 or -_bf._size, for inline putc
    stdin->_cookie = NULL;                 //< cookie passed to io functions
    stdin->_read = mik32_stdin_read;
    stdin->_write = NULL;
    stdin->_seek = NULL;
    stdin->_close = NULL;
    stdin->_blksize = 0;	        //< stat.st_blksize (may be != _bf._size)
    stdin->_flags2 = 0;            //< for future use */

    usart_transaction_cfg_t cfg = {
        .host = UART_0,
        .dma_channel = DMA_CH_AUTO,
        .dma_priority = 0,
        .direction = USART_TRANSACTION_RECEIVE
    };
    usart_transaction_init(&mik32_stdin_trans, &cfg);
}

int mik32_stdin_read(void *__reent, void *dummy, char *dst, int len)
{
    if (mik32_stdio_blocking_receive)
    {
        dma_status_t res = usart_receive(
            &mik32_stdin_trans,
            dst,
            len,
            0xFFFFFFFE//DMA_TIMEOUT_AUTO
        );
        if (res == DMA_STATUS_OK)
        {
            return len;
        }
        else return usart_transaction_done_bytes(&mik32_stdin_trans);
    }
    else //< non-blocking receive
    {
        // bool trans_ready = usart_transaction_ready(&mik32_stdin_trans);
        // if (trans_ready)
        // {
        //     dummy = mik32_stdin_buffer[mik32_stdin_cnt];
        //     usart_receive_start(&mik32_stdin_trans, mik32_stdin_buffer, mik32_stdin_cnt+1);
        //     mik32_stdin_cnt += 1;
        // }
        // else //< transaction is not ready
        // {
        //     while
        //     (
        //         mik32_stdin_cnt >= MIK32STDIN_BUFFER_SIZE-usart_transaction_left_bytes(&mik32_stdin_trans) ||
        //         !usart_transaction_ready(&mik32_stdin_trans)
        //     )
        //     {
        //         // probably timeout
        //     }
        //     dummy = mik32_stdin_buffer[mik32_stdin_cnt++];
        // }
        // usart_transaction_left_bytes(&mik32_stdin_trans);
    }
}

char mik32_stdin_getc(void)
{
    // char dummy;
    // if (mik32_stdio_blocking_receive)
    // {
    //     usart_receive(
    //         &mik32_stdin_trans,
    //         &dummy,
    //         1,
    //         1000 // AUTO_TIMEOUT
    //     );
    // }
    // else //< non-blocking receive
    // {
    //     bool trans_ready = usart_transaction_ready(&mik32_stdin_trans);
    //     if (trans_ready)
    //     {
    //         dummy = mik32_stdin_buffer[mik32_stdin_cnt];
    //         usart_receive_start(&mik32_stdin_trans, mik32_stdin_buffer, mik32_stdin_cnt+1);
    //         mik32_stdin_cnt += 1;
    //     }
    //     else //< transaction is not ready
    //     {
    //         while
    //         (
    //             mik32_stdin_cnt >= MIK32STDIN_BUFFER_SIZE-usart_transaction_left_bytes(&mik32_stdin_trans) ||
    //             !usart_transaction_ready(&mik32_stdin_trans)
    //         )
    //         {
    //             // probably timeout
    //         }
    //         dummy = mik32_stdin_buffer[mik32_stdin_cnt++];
    //     }
    //     usart_transaction_left_bytes(&mik32_stdin_trans);
    // }
    // return dummy;

    char dummy;
    mik32_stdin_read(NULL, NULL, &dummy, 1);
}