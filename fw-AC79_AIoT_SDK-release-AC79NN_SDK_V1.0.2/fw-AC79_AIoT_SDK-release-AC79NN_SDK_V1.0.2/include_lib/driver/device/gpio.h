#ifndef ASM_GPIO_H
#define ASM_GPIO_H


#include "asm/gpio.h"

#define GPIO2PORT(gpio)    (gpio / IO_GROUP_NUM)

void gpio_port_lock(unsigned int port);

void gpio_port_unlock(unsigned int port);

int __gpio_direction_input(unsigned int gpio);
int gpio_direction_input(unsigned int gpio);

int __gpio_direction_output(unsigned int gpio, int value);
int gpio_direction_output(unsigned int gpio, int value);

int __gpio_set_pull_up(unsigned int gpio, int value);
int gpio_set_pull_up(unsigned int gpio, int value);

int __gpio_set_pull_down(unsigned int gpio, int value);
int gpio_set_pull_down(unsigned int gpio, int value);

int __gpio_set_hd(unsigned int gpio, int value);
int gpio_set_hd(unsigned int gpio, int value);
int gpio_set_hd1(unsigned int gpio, int value);

int __gpio_set_die(unsigned int gpio, int value);
int gpio_set_die(unsigned int gpio, int value);

int __gpio_set_output_clk(unsigned int gpio, int clk);
int gpio_set_output_clk(unsigned int gpio, int clk);
int gpio_clear_output_clk(unsigned int gpio, int clk);

int __gpio_read(unsigned int gpio);
int gpio_read(unsigned int gpio);

int gpio_set_direction(unsigned int gpio, unsigned int dir);
void gpio_latch_en(unsigned int gpio, unsigned char en);
int gpio_set_dieh(unsigned int gpio, int value);

unsigned int gpio_output_channle(unsigned int gpio, enum gpio_out_channle clk);
unsigned int gpio_clear_output_channle(unsigned int gpio, enum gpio_out_channle clk);
unsigned int gpio_uart_rx_input(unsigned int gpio, unsigned int ut, unsigned int ch);

unsigned int gpio_close_uart0(void);

unsigned int gpio_close_uart1(void);

unsigned int gpio_close_uart2(void);

unsigned int gpio_set_uart0(unsigned int ch);

unsigned int gpio_set_uart1(unsigned int ch);

unsigned int gpio_set_uart2(unsigned int ch);

void usb_iomode(unsigned char usb_id, unsigned int enable);
unsigned int usb_set_direction(unsigned int gpio, unsigned int value);
unsigned int usb_output(unsigned int gpio, unsigned int value);
unsigned int usb_set_pull_up(unsigned int gpio, unsigned int value);
unsigned int usb_set_pull_down(unsigned int gpio, unsigned int value);
unsigned int usb_set_dieh(unsigned int gpio, unsigned int value);
unsigned int usb_set_die(unsigned int gpio, unsigned int value);
unsigned int usb_read(unsigned int gpio);

#endif
