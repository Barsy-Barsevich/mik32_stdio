#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "usart_transaction.h"

#define MIK32STDIN_BUFFER_SIZE      10

void mik32_stdin_init(void);
int mik32_stdin_read(void *__reent, void *dummy, char *dst, int len);