/*
 * This file is part of the Miyako-Link project.
 *
 * Copyright (C) 2020 Dmitry Rezvanov <dmitry.rezvanov@yandex.ru>
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
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

static TaskHandle_t pid_list[MAX_GDB_NUMBER] = { 0 };
static mutex_t		pid_list_mutex;

void add_pid_to_list(void)
{
    TaskHandle_t pid = xTaskGetCurrentTaskHandle();

    if (pid == NULL)
    {
        return;
    }

	mutex_lock(&pid_list_mutex);
    int insertIndex = -1;
	for(int i = 0; i < MAX_GDB_NUMBER; ++i)
    {
		if(pid_list[i] == NULL)
        {
			pid_list[i] = pid;
            continue;
		}
        if (pid_list[i] == pid)
        {
            if (insertIndex != -1)
            {
                pid_list[insertIndex] = NULL;
            }

            break;
        }
	}
	mutex_unlock(&pid_list_mutex);
}

int get_thread_number(void)
{
	int retVal = -1;
	kernel_pid_t pid = xTaskGetCurrentTaskHandle();

	mutex_lock(&pid_list_mutex);
	for(int i = 0; i < MAX_GDB_NUMBER; ++i)
    {
		if(pid_list[i] == pid)
        {
			retVal = i;
			break;
		}
	}
	mutex_unlock(&pid_list_mutex);
	return retVal;
}

static int interface_number_list[MAX_GDB_NUMBER] = {0};
static bool interface_busy_list[INTERFACE_NUMBER] = {0};
static mutex_t interface_busy_mutex;

void interface_numbers_init(void)
{
	for(int i = 0; i < INTERFACE_NUMBER; ++i)
    {
		interface_number_list[i] = -1;
    }
}

int get_interface_number(void)
{
    const int thread_number = get_thread_number();
    if (thread_number < 0)
    {
        return -1;
    }
	return interface_number_list[thread_number];
}

bool set_interface_number(int number)
{
    const int thread_number = get_thread_number();
    if (thread_number < 0)
    {
        return -1;
    }

	mutex_lock(&interface_busy_mutex);
	if(interface_busy_list[number])
    {
		mutex_unlock(&interface_busy_mutex);
		return false;
	}
    else
    {
		interface_busy_list[number] = true;
		interface_number_list[thread_number] = number;
		mutex_unlock(&interface_busy_mutex);
		return true;
	}
}

bool free_interface(int number)
{
    const int thread_number = get_thread_number();
    if (thread_number < 0)
    {
        return -1;
    }

	mutex_lock(&interface_busy_mutex);
	if(!interface_busy_list[number])
    {
		mutex_unlock(&interface_busy_mutex);
		return false;
	}
    else
    {
		interface_busy_list[number] = false;
		interface_number_list[thread_number] = -1;
		mutex_unlock(&interface_busy_mutex);
		return true;
	}
}

void mutex_lock(mutex_t *mutex)
{

}

void mutex_unlock(mutex_t *mutex)
{

}