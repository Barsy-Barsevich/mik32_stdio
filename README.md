# mik32_stdio
*Поддежка стандартного ввода-вывода для MIK32*

\[RU/[EN](README_EN.md)]

---
## Требуемые нестандартные модули
`mik32_transaction` (https://github.com/Barsy-Barsevich/mik32_transaction)

---
## Философия
Набор библиотек `mik32_stdio` был создан для упрощения ввода-вывода при использовании MIK32. Цель -- добавить поддержку методов стандартной библиотеки `stdio.h`. В настоящее время работают функции: `putc`, `putchar`, `puts`, `printf`, `sprintf`, `fprintf`, `getc`, `getchar` , `scanf`. Данные функции уже реализованы в стандартной библиотеке компилятора `riscv-none-elf-gcc`, но стандартные реализации не работают на MIK32, так как заточены под выполнение в составе ОС. Однако эти функции объявлены как "слабые" (`weak`), что дает возможность их переопределять, что и было сделано.

Данная библиотека позволяет вести прием и передачу данных как через `UART_0`, так и через `UART_1`, используя вызовы стандартной Си-библиотеки `stdio.h`. Ввод и вывод может быть буферизирован; поддерживаются как статические, так и динамические буферы.

---
## Поддержка динамически лоцируемых буферов
Если Вы хотите использовать динамически лоцируемые  буферы для неблокирующего приема и/или передачи, добавьте следующую строку в файл `platformio.ini` в Вашем проекте.
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
Удобная нициацизация модуля UART для вывода, ширина символа 8 бит, без бита четности, одинарная длина стоп-бита. Возращает `MIK32STDIO_INCORRECT_ARGUMENT`, если `host` не корректен илиесли значение `baudrate` больше, чем `apbp_clk` / 16.
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
Аналог функции `putchar`. Отправляет 1 байт в стандартный вывод.

---
### ⚡️`mik32_stdin.h`
Заголовочный файл для обслуживающих функций стандартного ввода.
#### Methods
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdin_uart_init(UART_TypeDef *host, uint32_t baudrate`
Удобная инициализация UART для ввода, ширина символа 8 бит, без бита четности, одинарная длина стоп-бита. Функция возвращает `false`, если `host` некорректен или `baudrate` больше, чем `apbp_clk / 16`.
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdin_init(UART_TypeDef *host, uint32_t baudrate)`
Инициализация стандартного ввода. `host` должен быть `UART0` or `UART1`. Функция возвращает `false`, если `host` некорректен или `baudrate` больше, чем `apbp_clk / 16`.
##### `void mik32_stdin_enable_blocking(void)`
Разрешает блокирующий режим ввода. В этом режиме, при вызове любой функции ввода, например, `getc`, процессор будет ждать каждого байта в цикле.
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdin_disable_blocking(void)`
Разрешает неблокирующий режим ввода. В этом режиме поступившие в UART данные сразу загружаются в буфер посредством DMA, не тревожа процессор. Чтение буфера доступно через функции `getc` и пр. функции ввода. Данный подход позволяет ускорить прием данных, потому что процессор не вынужден специально ждать каждого байта. Функция возвращает `MIK32STDIO_OK` или `MIK32STDIO_DMA_ERROR`.
##### `uint32_t mik32_stdin_get_buffer_size(void)`
Возвращает текущий размер буфера для приема данных.
##### [`mik32_stdio_status_t`](#mik32_stdio_status_t)` mik32_stdin_set_buffer_size(uint32_t size)`
Устанавливает размер буфера для приема (для неблокирующего режима) и аллоцирует новый буфер нового размера. Имеет смысл только если выражение `MIK32STDIN_USE_MALLOC` объявлено, иначе пустое действие. Возвращает `MIK32STDIO_OK`, `MIK32STDIO_DMA_ERROR`, `MIK32STDIO_MALLOC_FAIL` или `MIK32STDIO_INCORRECT_ARGUMENT`.
##### `int mik32_stdin_read(void *__reent, void *dummy, char *dst, int len)`
Принять данные из стандартного ввода.
##### `char mik32_stdin_getc(void)`
Аналог функции `getchar`. Принимает 1 байт из стандартного ввода.