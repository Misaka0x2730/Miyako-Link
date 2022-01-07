/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2015  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "general.h"
#include "morse.h"

/* Morse code patterns and lengths */
static const struct {
    uint16_t code;
    uint8_t bits;
} morse_letter[] = {
        {        0b00011101,  8}, // 'A' .-
        {    0b000101010111, 12}, // 'B' -...
        {  0b00010111010111, 14}, // 'C' -.-.
        {      0b0001010111, 10}, // 'D' -..
        {            0b0001,  4}, // 'E' .
        {    0b000101110101, 12}, // 'F' ..-.
        {    0b000101110111, 12}, // 'G' --.
        {      0b0001010101, 10}, // 'H' ....
        {          0b000101,  6}, // 'I' ..
        {0b0001110111011101, 16}, // 'J' .---
        {    0b000111010111, 12}, // 'K' -.-
        {    0b000101011101, 12}, // 'L' .-..
        {      0b0001110111, 10}, // 'M' --
        {        0b00010111,  8}, // 'N' -.
        {  0b00011101110111, 14}, // 'O' ---
        {  0b00010111011101, 14}, // 'P' .--.
        {0b0001110101110111, 16}, // 'Q' --.-
        {      0b0001011101, 10}, // 'R' .-.
        {        0b00010101,  8}, // 'S' ...
        {          0b000111,  6}, // 'T' -
        {      0b0001110101, 10}, // 'U' ..-
        {    0b000111010101, 12}, // 'V' ...-
        {    0b000111011101, 12}, // 'W' .--
        {  0b00011101010111, 14}, // 'X' -..-
        {0b0001110111010111, 16}, // 'Y' -.--
        {  0b00010101110111, 14}, // 'Z' --..
};

static char* morse_msg_list[MAX_GDB_NUMBER] = {NULL};
static char* morse_ptr_list[MAX_GDB_NUMBER] = {NULL};
static char morse_repeat_list[MAX_GDB_NUMBER] = {0};
static uint16_t morse_code_list[MAX_GDB_NUMBER] = {0};
static uint8_t morse_bits_list[MAX_GDB_NUMBER] = {0};
static mutex_t morse_mutex[MAX_GDB_NUMBER] = {0};

void morse_lock(void)
{
    const int number = get_interface_number();
    mutex_lock(&morse_mutex[number]);
}

void morse_unlock(void)
{
    const int number = get_interface_number();
    mutex_unlock(&morse_mutex[number]);
}

char* get_morse_msg(void)
{
    char *msg = NULL;
    const int number = get_interface_number();

    mutex_lock(&morse_mutex[number]);
    msg = morse_msg_list[number];
    mutex_unlock(&morse_mutex[number]);

    return msg;
}

void morse(char *msg, char repeat)
{
    const int number = get_interface_number();

    mutex_lock(&morse_mutex[number]);
    morse_msg_list[number] = msg;
    morse_ptr_list[number] = msg;
    morse_repeat_list[number] = repeat;
    mutex_unlock(&morse_mutex[number]);
}

bool morse_update(void)
{
    const int number = get_interface_number();
    if(number == -1)
    {
        return false;
    }

    mutex_lock(&morse_mutex[number]);

    uint16_t code = morse_code_list[number];
    uint8_t bits = morse_bits_list[number];
    char *morse_ptr = morse_ptr_list[number];
    char *morse_msg = morse_msg_list[number];

    if (!morse_ptr_list[number])
    {
        mutex_unlock(&morse_mutex[number]);
        return false;
    }

    if (!bits)
    {
        char c = *morse_ptr_list[number]++;
        if (!c)
        {
            if(morse_repeat_list[number])
            {
                morse_ptr_list[number] = morse_msg_list[number];
                c = *morse_ptr_list[number]++;
            }
            else
            {
                morse_ptr_list[number] = NULL;
                mutex_unlock(&morse_mutex[number]);
                return false;
            }
        }
        if ((c >= 'A') && (c <= 'Z'))
        {
            c -= 'A';
            code = morse_letter[c].code;
            bits = morse_letter[c].bits;
        }
        else
        {
            code = 0;
            bits = 4;
        }
    }

    bool ret = code & 1;
    code >>= 1;
    bits--;

    morse_code_list[number] = code;
    morse_bits_list[number] = bits;
    mutex_unlock(&morse_mutex[number]);
    return ret;
}

