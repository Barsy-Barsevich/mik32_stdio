#pragma once

#include <stdio.h>
#include "usart_transaction.h"

void mik32_stdout_init(void);
void mik32_stdout_enable_blocking(void);
void mik32_stdout_disable_blocking(void);
void mik32_stdout_flush(void);
void mik32_stdout_putc(char symbol);