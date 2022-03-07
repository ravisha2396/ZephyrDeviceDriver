/*
 * Copyright (c) 2017 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 * Copyright (c) 2019 Nordic Semiconductor ASA
 * Copyright (c) 2019 Marc Reilly
 * Copyright (c) 2019 PHYTEC Messtechnik GmbH
 * Copyright (c) 2020 Endian Technologies AB
 * Copyright (c) 2020 Kim Bøndergaard <kim@fam-boendergaard.dk>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT maxim_max7219


#include <device.h>
#include <drivers/spi.h>
#include <sys/byteorder.h>
#include <drivers/display.h>
#include <logging/log.h>
#include <sys/printk.h>
LOG_MODULE_REGISTER(display_max7219, CONFIG_DISPLAY_LOG_LEVEL);

#define DISPLAY_TEST_REGISTER	0x0F
#define SHUTDOWN_REGISTER	0x0C
#define SCAN_LIMIT_REGISTER	0x0B
#define INTENSITY_REGISTER	0x0A
#define DECODE_MODE_REGISTER	0x09

#define MAX_COUNT DT_PROP(DT_NODELABEL(disp_node),width)

struct max7219_config {
	const char *spi_name;
	struct spi_config spi_config;
	uint16_t height;
	uint16_t width;
};

struct max7219_data {
	const struct max7219_config *config;
	const struct device *spi_dev;
	uint16_t width;
	uint16_t height;
	uint8_t BUFFER[MAX_COUNT];
};

static int max7219_transmit(struct max7219_data *data, uint8_t address, uint8_t tx_data)
{	

	uint16_t packet = (address<<8)|tx_data;

	struct spi_buf tx_buf = { .buf = &packet, .len = 2 };
	struct spi_buf_set tx_bufs = { .buffers = &tx_buf, .count = 1 };
	int ret;

	ret = spi_write(data->spi_dev, &data->config->spi_config, &tx_bufs);
	if (ret < 0) {
		return ret;
	}
	
	return 0;
}


static int max7219_blanking_on(const struct device *dev)
{
	struct max7219_data *data = (struct max7219_data *)dev->data;

	return max7219_transmit(data, SHUTDOWN_REGISTER, 0x00);
}

static int max7219_blanking_off(const struct device *dev)
{
	struct max7219_data *data = (struct max7219_data *)dev->data;

	return max7219_transmit(data, SHUTDOWN_REGISTER, 0x01);
}

static int max7219_read(const struct device *dev,
			const uint16_t x,
			const uint16_t y,
			const struct display_buffer_descriptor *desc,
			void *buf)
{
	return -ENOTSUP;
}

static int max7219_buffer_clear(struct max7219_data *data){
	
	for(int i=MAX_COUNT-1; i>=0;i--){
		max7219_transmit(data, MAX_COUNT-i, 0x00);
		//BUFFER[MAX_COUNT-i-1]=0;
		data->BUFFER[i]=0;
	}
	return 0;
}

static int max7219_write(const struct device *dev,
			 const uint16_t x,
			 const uint16_t y,
			 const struct display_buffer_descriptor *desc,
			 const void *buf)
{	
	uint8_t height = desc->height;
	uint8_t width = desc->width;
	uint8_t *data = ((uint8_t*)buf);
 
	for(int i=y; i<y+height;i++){
		for(int j=width-1; j>=0;j--){
			if(((data[i]>>j) & 1)>0){
				(((struct max7219_data*)(dev->data))->BUFFER[width-j-1])|= (1<<(i));
				}
			else{
				(((struct max7219_data*)(dev->data))->BUFFER[width-j-1])&= ~(1<<(i));
				}
		}
	}
	for(int k=x; k<width; k++){
		max7219_transmit((struct max7219_data*)dev->data, k+1, ((struct max7219_data*)dev->data)->BUFFER[k]);
		((struct max7219_data*)dev->data)->BUFFER[k]=0;
	}
	return 0;
}

static void *max7219_get_framebuffer(const struct device *dev)
{
	return NULL;
}

static int max7219_set_brightness(const struct device *dev,
				  const uint8_t brightness)
{
	return -ENOTSUP;
}

static int max7219_set_contrast(const struct device *dev,
				const uint8_t contrast)
{
	return -ENOTSUP;
}

static void max7219_get_capabilities(const struct device *dev,
				     struct display_capabilities *capabilities)
{
	return;
}

static int max7219_set_pixel_format(const struct device *dev,
				    const enum display_pixel_format pixel_format)
{
	return -ENOTSUP;
}

static int max7219_set_orientation(const struct device *dev,
				   const enum display_orientation orientation)
{

	return -ENOTSUP;
}

static int max7219_lcd_init(struct max7219_data *data)
{
	// const struct max7219_config *config = data->config;
	int ret;

	//max7219_set_lcd_margins(data, data->x_offset, data->y_offset);

	ret = max7219_transmit(data, SHUTDOWN_REGISTER, 0x01);
	if (ret < 0) {
		return ret;
	}
	
	ret = max7219_transmit(data, INTENSITY_REGISTER, 0x08);
	if (ret < 0) {
		return ret;
	}

	ret = max7219_transmit(data, SCAN_LIMIT_REGISTER, 0x07);
	if (ret < 0) {
		return ret;
	}

	ret = max7219_transmit(data, DECODE_MODE_REGISTER, 0x00);
	if (ret < 0) {
		return ret;
	}

	ret = max7219_transmit(data, DISPLAY_TEST_REGISTER, 0x00);
	if (ret < 0) {
		return ret;
	}

	ret = max7219_buffer_clear(data);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

static int max7219_init(const struct device *dev)
{
	struct max7219_data *data = (struct max7219_data *)dev->data;
	struct max7219_config *config = (struct max7219_config *)dev->config;
	int ret;

	data->spi_dev = device_get_binding(config->spi_name);
	if (data->spi_dev == NULL) {
		LOG_ERR("Could not get SPI device for LCD");
		return -ENODEV;
	}
	ret = max7219_lcd_init(data);

	if(ret!=0){
		LOG_ERR("Could not initialize LED Panel");
	}

	return 0;
}

static const struct display_driver_api max7219_api = {
	.blanking_on = max7219_blanking_on,
	.blanking_off = max7219_blanking_off,
	.write = max7219_write,
	.read = max7219_read,
	.get_framebuffer = max7219_get_framebuffer,
	.set_brightness = max7219_set_brightness,
	.set_contrast = max7219_set_contrast,
	.get_capabilities = max7219_get_capabilities,
	.set_pixel_format = max7219_set_pixel_format,
	.set_orientation = max7219_set_orientation,
};


#define max7219_INIT(inst)							\
	static struct max7219_data max7219_data_ ## inst;			\
										\
	const static struct max7219_config max7219_config_ ## inst = {	\
		.spi_name = DT_INST_BUS_LABEL(inst),				\
		.spi_config.slave = DT_INST_REG_ADDR(inst),			\
		.spi_config.frequency = DT_INST_PROP(inst, spi_max_frequency), 				\
		.spi_config.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(16) | SPI_TRANSFER_MSB,\
		.spi_config.cs = NULL,			               	\
		.width = DT_INST_PROP(inst, width),				\
		.height = DT_INST_PROP(inst, height),				\
										\
	};									\
										\
	static struct max7219_data max7219_data_ ## inst = {			\
		.config = &max7219_config_ ## inst,				\
										\
	};									\
	DEVICE_DT_INST_DEFINE(inst, max7219_init, NULL,		\
			      &max7219_data_ ## inst, &max7219_config_ ## inst,	\
			      APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY,	\
			      &max7219_api);

DT_INST_FOREACH_STATUS_OKAY(max7219_INIT)
