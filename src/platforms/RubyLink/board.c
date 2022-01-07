/*
 * Copyright (C) 2021 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         boards_rpi_pico
 * @{
 *
 * @file
 * @brief           Board specific implementations for the Raspberry Pi Pico
 *
 * @author          Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 * @}
 */

#include "board.h"

/*#ifdef LOGGER_RTT

// Logging with RTT
// - If RTT Control Block is not found by 'Auto Detection` try to use 'Search Range` with '0x20000000 0x10000'
// - SWD speed is rather slow around 1000Khz

#include "pico/stdio/driver.h"
#include "SEGGER_RTT.h"

static void stdio_rtt_write (const char *buf, int length)
{
  SEGGER_RTT_Write(0, buf, length);
}

static int stdio_rtt_read (char *buf, int len)
{
  return SEGGER_RTT_Read(0, buf, len);
}

static stdio_driver_t stdio_rtt =
{
  .out_chars = stdio_rtt_write,
  .out_flush = NULL,
  .in_chars = stdio_rtt_read
};

void stdio_rtt_init(void)
{
  stdio_set_driver_enabled(&stdio_rtt, true);
}

#endif*/

/*void board_init(void)
{
    //stdio_rtt_init();
}*/
