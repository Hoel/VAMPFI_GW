/*
Copyright (C) 2018  GridRF Radio Team(tech@gridrf.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "gpio.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define MMAP_PATH	"/dev/mem"


#define RALINK_GPIO_DIR_IN		0
#define RALINK_GPIO_DIR_OUT		1

#define RALINK_REG_PIOINT		0x690
#define RALINK_REG_PIOEDGE		0x6A0
#define RALINK_REG_PIORENA		0x650
#define RALINK_REG_PIOFENA		0x660
#define RALINK_REG_PIODATA		0x620
#define RALINK_REG_PIODIR		0x600
#define RALINK_REG_PIOSET		0x630
#define RALINK_REG_PIORESET		0x640

#define RALINK_REG_PIO6332INT		0x694
#define RALINK_REG_PIO6332EDGE		0x6A4
#define RALINK_REG_PIO6332RENA		0x654
#define RALINK_REG_PIO6332FENA		0x664
#define RALINK_REG_PIO6332DATA		0x624
#define RALINK_REG_PIO6332DIR		0x604
#define RALINK_REG_PIO6332SET		0x634
#define RALINK_REG_PIO6332RESET		0x644

#define RALINK_REG_PIO9564INT		0x698
#define RALINK_REG_PIO9564EDGE		0x6A8
#define RALINK_REG_PIO9564RENA		0x658
#define RALINK_REG_PIO9564FENA		0x668
#define RALINK_REG_PIO9564DATA		0x628
#define RALINK_REG_PIO9564DIR		0x608
#define RALINK_REG_PIO9564SET		0x638
#define RALINK_REG_PIO9564RESET		0x648


static uint8_t* gpio_mmap_reg = NULL;
static int gpio_mmap_fd = 0;

static int gpio_mmap(void)
{
	if ((gpio_mmap_fd = open(MMAP_PATH, O_RDWR)) < 0) {
		fprintf(stderr, "unable to open mmap file");
		return -1;
	}

	gpio_mmap_reg = (uint8_t*) mmap(NULL, 1024, PROT_READ | PROT_WRITE,
		MAP_FILE | MAP_SHARED, gpio_mmap_fd, 0x10000000);
	if (gpio_mmap_reg == MAP_FAILED) {
		perror("foo");
		fprintf(stderr, "failed to mmap");
		gpio_mmap_reg = NULL;
		close(gpio_mmap_fd);
		return -1;
	}

	return 0;
}

int mt76x8_gpio_get_pin(int pin)
{
	uint32_t tmp = 0;

	/* MT7621, MT7628 */
	if (pin <= 31) {
		tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIODATA);
		tmp = (tmp >> pin) & 1u;
	} else if (pin <= 63) {
		tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332DATA);
		tmp = (tmp >> (pin-32)) & 1u;
	} else if (pin <= 95) {
		tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564DATA);
		tmp = (tmp >> (pin-64)) & 1u;
		tmp = (tmp >> (pin-24)) & 1u;
	}
	return tmp;

}

void mt76x8_gpio_set_pin_direction(int pin, int is_output)
{
	uint32_t tmp;

	/* MT7621, MT7628 */
	if (pin <= 31) {
		tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIODIR);
		if (is_output)
			tmp |=  (1u << pin);
		else
			tmp &= ~(1u << pin);
		*(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIODIR) = tmp;
	} else if (pin <= 63) {
		tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332DIR);
		if (is_output)
			tmp |=  (1u << (pin-32));
		else
			tmp &= ~(1u << (pin-32));
		*(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332DIR) = tmp;
	} else if (pin <= 95) {
		tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564DIR);
		if (is_output)
			tmp |=  (1u << (pin-64));
		else
			tmp &= ~(1u << (pin-64));
		*(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564DIR) = tmp;
	}
}

void mt76x8_gpio_set_pin_value(int pin, int value)
{
	uint32_t tmp;

	/* MT7621, MT7628 */
	if (pin <= 31) {
		tmp = (1u << pin);
		if (value)
			*(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIOSET) = tmp;
		else
			*(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIORESET) = tmp;
	} else if (pin <= 63) {
		tmp = (1u << (pin-32));
		if (value)
			*(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332SET) = tmp;
		else
			*(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332RESET) = tmp;
	} else if (pin <= 95) {
		tmp = (1u << (pin-64));
		if (value)
			*(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564SET) = tmp;
		else
			*(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564RESET) = tmp;
	}
}


int gpio_init(void) { 
	return gpio_mmap(); 
};


void gpio_setdirecton(int pin, int dir){ 
	mt76x8_gpio_set_pin_direction(pin, dir);
};

int gpio_read(int pin) { 
	return mt76x8_gpio_get_pin(pin);
};

void gpio_set(int pin, int value) { 
	return mt76x8_gpio_set_pin_value(pin, value); 
};

void gpio_free(void){ 
	close(gpio_mmap_fd); 
};



