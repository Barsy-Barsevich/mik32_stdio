# mik32_stdio
*Supporting standard I/O on the MIK32 platform*

\[[RU](README.md)/EN]

---
## Dynamic buffer support
If you want to use dynamic buffers for non-blocking receive and/or transmit, enter following equation to `platformio.ini` file in your project.
```
build_flags = -DMIK32STDOUT_USE_MALLOC -DMIK32STDIN_USE_MALLOC
```

---
## Headers

### ⚡️`mik32_stdio_status.h`
Contains `mik32_stdio_status_t` type definition.
#### Enums
##### `mik32_stdio_status_t`
- `MIK32STDIO_OK` (=0);
- `MIK32STDIO_INCORRECT_ARGUMENT` (=1);
- `MIK32STDIO_DMA_ERROR` (=2): dma transaction error;
- `MIK32STDIO_TIMEOUT_ERROR` (=3);
- `MIK32STDIO_MALLOC_FAIL` (=4).

---
### ⚡️`mik32_stdio.h`
Common stdio header.
#### Methods
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdio_init(UART_TypeDef *host, uint32_t baudrate)`
Standard input and output initializing. Is equivalent to call both of`mik32_stdout_init` and `mik32_stdin_init` subroutines.

---
### ⚡️`mik32_stdout.h`
Standard output.
#### Methods
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdout_uart_init(UART_TypeDef *host, uint32_t baudrate`
UART initializing for output, frame 8bit, no parity bit, 1-period size stop bit. Returns `MIK32STDIO_INCORRECT_ARGUMENT` if `host` is incorrect or `baudrate` is more than `apbp_clk` / 16.
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdout_init(UART_TypeDef *host, uint32_t baudrate)`
Standard output and UART initialization. `host` must be `UART_0` or `UART_1`. Returns `MIK32STDIO_INCORRECT_ARGUMENT` if `host` is incorrect or `baudrate` is more than `apbp_clk` / 16.
##### `void mik32_stdout_enable_blocking(void)`
Enable blocking transmission. Blocking transmission means the system will hold the processor while not all the bytes are sent.
##### `void mik32_stdout_disable_blocking(void)`
Disable blocking transmission. Non-blocking mode means that the processor loads data to the buffer and when the buffer is overloaded or the `\n` symbol was sent or the special function was called processor starts the sending DMA-transaction. Then the processor is ready to work on other tasks while DMA sends the data. Non-blocking mode is enabled by default.
##### `uint32_t mik32_stdout_get_buffer_size(void)`
Returns input buffer size.
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdout_set_buffer_size(uint32_t size)`
Sets output buffer (for non-blocking mode) size and allocates new buffer. Only effective if `MIK32STDOUT_USE_MALLOC` is defined. Returns `MIK32STDIO_OK`, `MIK32STDIO_DMA_ERROR`, `MIK32STDIO_MALLOC_FAIL` or `MIK32STDIO_INCORRECT_ARGUMENT`.
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdout_flush(void)`
Send the buffer data immediately. Only effective in non-blocking mode.
##### `int mik32_stdout_write(void *__reent, void *dummy, const char *src, int len)`
Write some data to the standard output.
##### `void mik32_stdout_putc(char symbol)`
Putc function equivalent, write 1 byte to the buffer.

---
### ⚡️`mik32_stdin.h`
Standard input.
#### Methods
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdin_uart_init(UART_TypeDef *host, uint32_t baudrate`
UART initializing for input, frame 8bit, no parity bit, 1-period size stop bit. Returns `false` if `host` is incorrect or baudrate is more than apbp_clk / 16.
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdin_init(UART_TypeDef *host)`
Standard input initialization. `host` must be `UART0` or `UART1`. Returns `false` if `host` is incorrect or baudrate is more than apbp_clk / 16.
##### `void mik32_stdin_enable_blocking(void)`
Enables blocking input mode. In this mode when any of input functions is called (for example, `getc`) the system waits for the bytes entered.
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdin_disable_blocking(void)`
Enables non-blocking input mode. In this mode entered data are permanently loaded into the buffer and when the user calls `getc` or other input function data are read from buffer, that increases UART receiving speed capability because the microcontroller can receive data while the processor works on other tasks. This method returns `MIK32STDIO_OK` or `MIK32STDIO_DMA_ERROR`.
##### `uint32_t mik32_stdin_get_buffer_size(void)`
Returns input buffer size.
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdin_set_buffer_size(uint32_t size)`
Sets input buffer (for non-blocking mode) size and allocates new buffer. Only effective if `MIK32STDIN_USE_MALLOC` is defined. Returns `MIK32STDIO_OK`, `MIK32STDIO_DMA_ERROR`, `MIK32STDIO_MALLOC_FAIL` or `MIK32STDIO_INCORRECT_ARGUMENT`.
##### `int mik32_stdin_read(void *__reent, void *dummy, char *dst, int len)`
Receive some data from standard input.
##### `char mik32_stdin_getc(void)`
Getc function equivalent, receive 1 byte of data.