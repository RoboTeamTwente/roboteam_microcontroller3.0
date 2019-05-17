/*
 * FT812Q.c
 *
 *  Created on: May 15, 2019
 *      Author: selina
 */

#include "FT812Q.h"
#include "string.h"

uint8_t data[4]; // used by functions

/* FUNCTIONS */
void writeDisplay(uint32_t address, uint32_t size, uint8_t* data){

	// For writing, the address must start with '01' and then the address
	if (address != 0x000000) {
		address |= 0x800000;
	}

	// Create write command
	QSPI_CommandTypeDef WRITE_COMMAND = {
		.InstructionMode 	= QSPI_INSTRUCTION_NONE,
		.Instruction 		= 0,
		.AddressMode 		= QSPI_ADDRESS_1_LINE,
		.AddressSize 		= QSPI_ADDRESS_24_BITS,
		.Address 			= address,
		.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE,
		.AlternateBytes 	= 0,
		.AlternateBytesSize = 0,
		.DataMode 			= QSPI_DATA_1_LINE,
		.DummyCycles 		= 0,
		.NbData 			= size,
		.DdrMode 			= QSPI_DDR_MODE_DISABLE,
		.DdrHoldHalfCycle 	= QSPI_DDR_HHC_ANALOG_DELAY,
		.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD
	};

	// Send data
	HAL_QSPI_Command(&hqspi, &WRITE_COMMAND, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);
	HAL_QSPI_Transmit(&hqspi, data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);
}

uint8_t* readDisplay(uint32_t address, uint32_t size, uint8_t* data){

	// Create read command
	QSPI_CommandTypeDef READ_COMMAND = {
		.InstructionMode 	= QSPI_INSTRUCTION_NONE,
		.Instruction 		= 0,
		.AddressMode 		= QSPI_ADDRESS_1_LINE,
		.AddressSize 		= QSPI_ADDRESS_24_BITS,
		.Address 			= address,
		.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE,
		.AlternateBytes 	= 0,
		.AlternateBytesSize = 0,
		.DataMode 			= QSPI_DATA_1_LINE,
		.DummyCycles 		= 8,
		.NbData 			= size,
		.DdrMode 			= QSPI_DDR_MODE_DISABLE,
		.DdrHoldHalfCycle 	= QSPI_DDR_HHC_ANALOG_DELAY,
		.SIOOMode 			= QSPI_SIOO_INST_EVERY_CMD
	};

	// Receive data
	HAL_QSPI_Command(&hqspi, &READ_COMMAND, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);
	HAL_QSPI_Receive(&hqspi, data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);

	// Print data
//	for (int i = 0; i < size; i++) {
//		Putty_printf("Data[%d]: %X \n\r", i, data[i]);
//	}

	return data;
}

void display_Init(){

	writeDisplay(DISPLAY_SET_POWERDOWN, 0x3, POWERDOWN); // Reset
	writeDisplay(DISPLAY_SET_ACTIVE, 0x3, ACTIVE); // Set to ACTIVE
	HAL_Delay(300); // Needs up to 300ms to start up

	/* Configure display registers */ // From NHD-4.3-480272FT-CTXL-T Datasheet
	writeDisplay	(REG_HCYCLE, 	0x2, 	HCYCLE); 	// 548
	writeDisplay	(REG_HOFFSET, 	0x2, 	HOFFSET); 	// 43
	writeDisplay	(REG_HSYNC0, 	0x2, 	HSYNC0); 	// 0
	writeDisplay	(REG_HSYNC1, 	0x2, 	HSYNC1); 	// 41
	writeDisplay	(REG_VCYCLE, 	0x2, 	VCYCLE); 	// 292
	writeDisplay	(REG_VOFFSET, 	0x2, 	V0FFSET); 	// 12
	writeDisplay	(REG_VSYNC0, 	0x2, 	VSYNC0); 	// 0
	writeDisplay	(REG_VSYNC1, 	0x2, 	VSYNC1); 	// 10
	writeDisplay	(REG_SWIZZLE, 	0x1, 	SWIZZLE); 	// 0
	writeDisplay	(REG_PCLK_POL, 	0x1, 	PCLK_POL); 	// 1
	writeDisplay	(REG_CSPREAD, 	0x1, 	CSPREAD); 	// 1
	writeDisplay	(REG_DITHER, 	0x1, 	DITHER); 	// 1
	writeDisplay	(REG_HSIZE, 	0x2, 	HSIZE); 	// 480
	writeDisplay	(REG_VSIZE, 	0x2, 	VSIZE); 	// 272

	/* Write first display list */
	writeDisplay	(RAM_DL + 0x0, 	0x4, 	CLEAR_COLOR_RGB(0, 0, 0)); // RoboTeam Red
	writeDisplay	(RAM_DL + 0x4, 	0x4, 	CLEAR(1, 1, 1));
	writeDisplay	(RAM_DL + 0x8, 	0x4, 	DISPLAY);

	writeDisplay	(REG_DLSWAP, 	0x1, 	DLSWAP); // Display list swap
	writeDisplay	(REG_GPIO_DIR, 	0x1, 	GPIO_DIR);
	writeDisplay	(REG_GPIO, 		0x1, 	GPIO); // Enable display bit
	writeDisplay	(REG_PCLK, 		0x1, 	PCLK); // After this display is visible on the LCD
}

/* SETTINGS */
uint8_t* CLEAR(uint8_t c, uint8_t s, uint8_t t){
	// c = Clear color buffer
	// s = Clear stencil buffer
	// t = Clear tag buffer

	uint32_t temp = (0x26 << 24) | (0x0 << 16) | (0x0 << 8) | ((c << 2) + (s << 1) + t);
	memcpy(data, (uint8_t*)&temp, 4);

	return data;
}

uint8_t* CLEAR_COLOR_RGB(uint8_t red, uint8_t green, uint8_t blue){
	uint32_t temp = 0x02 << 24 | red << 16 | green << 8 | blue;
	memcpy(data, (uint8_t*)&temp, 4);

	return data;
}

uint8_t* COLOR_RGB(uint8_t red, uint8_t green, uint8_t blue){
	uint32_t temp = 0x04 << 24 | red << 16 | green << 8 | blue;
	memcpy(data, (uint8_t*)&temp, 4);

	return data;
}

uint8_t* POINT_SIZE(uint16_t size){
	uint32_t temp = 0x0D << 24 | 0x0 << 13 | (size & 0xFFF);
	memcpy(data, (uint8_t*)&temp, 4);
	return data;
}

uint8_t* LINE_WIDTH(uint16_t width){
	uint32_t temp = ((width & 0x0E) << 24) | 0x0 << 12 | (width & 0x7FF);
	memcpy(data, (uint8_t*)&temp, 4);
	return data;
}

uint8_t* VERTEX_FORMAT(uint8_t size){
	uint32_t temp = 0x27 << 24 | 0x0 << 3 | 0x0;
	memcpy(data, (uint8_t*)&temp, 4);
	return data;
}

/* OBJECTS */
uint8_t* POINT_DATA(uint16_t x, uint16_t y){
	uint32_t temp = (0x1 << 30) | (x & 0x7FFF) << 15 | (y & 0x7FFF);
	memcpy(data, (uint8_t*)&temp, 4);
	return data;
}

uint8_t* LETTER_DATA(uint16_t x, uint16_t y, uint8_t font, uint8_t letter){
	uint32_t temp = 0x2 << 30 | (x & 0x1FF) << 21 | (y & 0x1FF) << 12 | font << 7 | letter;
	memcpy(data, (uint8_t*)&temp, 4);
	return data;
}

