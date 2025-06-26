# mik32_stdio
*Поддежка стандартного ввода-вывода для MIK32*

\[RU/[EN](./README.md)]

---
## Поддержка динамически лоцируемых буферов
Если Вы хотитеиспользовать динамически лоцируемые  буферы для неблокирующего приема и/или передачи, добавьте следующую строку в файл `platformio.ini` в Вашем проекте.
```
build_flags = -DMIK32STDOUT_USE_MALLOC -DMIK32STDIN_USE_MALLOC
```

---
## Заголовочные файлы

### ⚡️`mik32_stdio_status.h`
Содержит объявление типа `mik32_stdio_status_t`.
#### Enums
##### `mik32_stdio_status_t`
- `MIK32STDIO_OK` (=0);
- `MIK32STDIO_INCORRECT_ARGUMENT` (=1);
- `MIK32STDIO_DMA_ERROR` (=2): ошибка ПДП транзации;
- `MIK32STDIO_TIMEOUT_ERROR` (=3);
- `MIK32STDIO_MALLOC_FAIL` (=4).

---
### ⚡️`mik32_stdio.h`
Общий заголовочный файл для стандартного ввода-вывода.
#### Методы
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdio_init(UART_TypeDef *host, uint32_t baudrate)`
Инициализация стандартного ввода и вывода. Вызов функции эквивалентен последовательному вызову функций `mik32_stdout_init` и `mik32_stdin_init`.

---
### ⚡️`mik32_stdout.h`
Заголовочный файл для обслуживающих функций стандартного вывода.
#### Методы
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdout_uart_init(UART_TypeDef *host, uint32_t baudrate`
Инициацизация модуля UART для вывода, ширина символа 8 бит, без бита четности, одинарная длина стоп-бита. Возращает `MIK32STDIO_INCORRECT_ARGUMENT`, если `host` не корректен илиесли значение `baudrate` больше, чем `apbp_clk` / 16.
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdout_init(UART_TypeDef *host, uint32_t baudrate)`
Инициализация стандартного вывода и модуля UART для вывода. Значение `host` должно быть `UART_0` или `UART_1`. Возращает `MIK32STDIO_INCORRECT_ARGUMENT`, если `host` не корректен илиесли значение `baudrate` больше, чем `apbp_clk` / 16.
##### `void mik32_stdout_enable_blocking(void)`
Включить режим блокирующего вывода. В режиме блокирующего вывода процессор будет занят при передаче, пока последний байт не будет передан.
##### `void mik32_stdout_disable_blocking(void)`
Включить режим неблокирующего вывода. В режиме неблокирующего вывода процессор загружает данные в буффер (по-умолчанию имеет объем 50), и когда буфер переполняется, или встручается символ `\n`, или была вызвана специальная функция (`mik32_stdout_flush`), процессор запускает DMA-транзакцию на отправку содержимого буфера. Сразу после этого управление возращается пользователю, и процессор не тратит времени, пока данные передаются.
##### `uint32_t mik32_stdout_get_buffer_size(void)`
Возращает размер буфера на вывод в байтах.
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdout_set_buffer_size(uint32_t size)`
Устанавливает размер буфера на вывод (для неблокирующего вывода). Имеет смысл только в случае, если выражение `MIK32STDOUT_USE_MALLOC` объявлено. Возращает `MIK32STDIO_OK`, `MIK32STDIO_DMA_ERROR`, `MIK32STDIO_MALLOC_FAIL` или `MIK32STDIO_INCORRECT_ARGUMENT`.
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdout_flush(void)`
Отправляет данные буфера в порт немедленно. Имеет смысл в неблокирующем режиме.
##### `int mik32_stdout_write(void *__reent, void *dummy, const char *src, int len)`
Записывает данные в стандартный вывод.
##### `void mik32_stdout_putc(char symbol)`
Аналог функции `putc`. Отправляет 1 байт в стандартный вывод.

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