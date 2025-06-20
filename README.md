# mik32_stdio
Supporting standard I/O on the MIK32 platform

## Headers
### `mik32_stdout.h`
Standard output.
#### Methods
##### `void mik32_stdout_init(UART_TypeDef *host)`
Standard output initialization. `host` must be `UART0` or `UART1`.
##### `void mik32_stdout_enable_blocking(void)`
Enable blocking transmission. Blocking transmission means the system will hold the processor while not all the bytes are sent.
##### `void mik32_stdout_disable_blocking(void)`
Disable blocking transmission. Non-blocking mode means that the processor loads data to the buffer and when the buffer is overloaded or the '\n' symbol was sent or the special function was called processor starts the sending DMA-transaction. Then the processor is ready to work on other tasks while DMA sends the data. Non-blocking mode is enabled by default.
##### `void mik32_stdout_flush(void)`
Send the buffer data immediately.
##### `int mik32_stdout_write(void *__reent, void *dummy, const char *src, int len)`
Write some data to the standard output.
##### `void mik32_stdout_putc(char symbol)`
Putc function equivalent, write 1 byte to the buffer.

### `mik32_stdin.h`
Standard input.
#### Methods
##### `void mik32_stdin_init(UART_TypeDef *host)`
##### `void mik32_stdin_enable_blocking(void)`
##### `void mik32_stdin_disable_blocking(void)`
##### `uint32_t mik32_stdin_get_buffer_size(void)`
##### `void mik32_stdin_set_buffer_size(uint32_t size)`
##### `int mik32_stdin_read(void *__reent, void *dummy, char *dst, int len)`
Receive some data from standard input.
##### `char mik32_stdin_getc(void)`
Getc function equivalent, receive 1 byte of data.