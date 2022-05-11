/******************************************************************************
* File Name: lcd_i2c.c
*
* Version: 1.0
*
* Description: This example project demonstrates the use of the 2x20 LCD display in the parts kit
* using an I2C master in the PSoC.

******************************************************************************
* Copyright (2020), Anujan Varma
******************************************************************************
* This software, should be used only as an example for CSE 121 labs.
******************************************************************************
*
* USER-CALLABLE FUNCTIONS
* static void lcd_init(void):  Must be included in system initialization tasks. Wait for 50 ms after reset 
* before calling init.
*
* static void lcd_cursor(uint8_t row, uint8_t col): Positions cursor on a row and column to start writing.
* row must be 0 or 1, col must be between 0 and 19 (inclusive)
*
* static void lcd_write(char *data, uint8_t size): Write a character string on screen
* For writing integers, use sprintf to convert.
*
* static void lcd_clear(void): Clears screen
******************************************************************************/
#include <string.h>
#include "project.h"
#include "lcd_i2c.h"

 /***************************************
* Function prototypes
****************************************/
void lcd_HandleError(void); 
void lcd_command(uint8_t value);
void lcd_send(uint32_t i2c_addr, uint8_t *data, int length);
void lcd_init(void);
void lcd_setReg(uint8_t reg_num, uint8_t val);
void lcd_write(char *data, uint8_t size);
void lcd_cursor(uint8_t row, uint8_t col);
void lcd_clear(void);

 /***************************************************************************
 * User-callable functions
 ***************************************************************************/

 /*-------------------------------------------------------------------------
 * lcd_init: Initialize LCD display
 ---------------------------------------------------------------------------*/
 void lcd_init(void)
{
    uint32_t dataRateSet;
    cy_en_scb_i2c_status_t initStatus;
    
    __enable_irq(); /* Enable global interrupts. */

    /* Initalize the master I2C. */
    
    /* Configure component. */
    initStatus = Cy_SCB_I2C_Init(mI2C_HW, &mI2C_config, &mI2C_context);
    if(initStatus!=CY_SCB_I2C_SUCCESS)
    {
     lcd_HandleError();
    }
    /* Configure desired data rate. */
    dataRateSet = Cy_SCB_I2C_SetDataRate(mI2C_HW, mI2C_DATA_RATE_HZ, mI2C_CLK_FREQ_HZ);
    
    /* check whether data rate set is not greather then required reate. */
    if( dataRateSet > mI2C_DATA_RATE_HZ )
    {
       lcd_HandleError();
    }
	
	/* Enable I2C master hardware */
	Cy_SCB_I2C_Enable(mI2C_HW);

	// Delay 50 ms for display to come out of reset.
	Cy_SysLib_Delay(50);
	
	uint8_t cmdarg = LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;
	lcd_command(cmdarg);
	Cy_SysLib_Delay(DELAY_5MS); // 5 ms delay
	lcd_command(cmdarg);
	Cy_SysLib_Delay(DELAY_5MS);
	lcd_command(cmdarg);
	Cy_SysLib_Delay(DELAY_5MS);
    // turn the display on with no cursor or blinking default
	cmdarg = LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	lcd_command(cmdarg);
	Cy_SysLib_Delay(DELAY_5MS);

	// clear it off
	cmdarg = LCD_CLEARDISPLAY;
	lcd_command(cmdarg);
	Cy_SysLib_Delay(DELAY_2SEC);

	// Initialize to default text direction
	cmdarg = LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	//< set the entry mode
	 lcd_command(cmdarg);
	Cy_SysLib_Delay(DELAY_5MS);
	
	// backlight init
	lcd_setReg(REG_MODE1, 0);
	// set LEDs controllable by both PWM and GRPPWM registers
	lcd_setReg(REG_OUTPUT, 0xFF);
	// set MODE2 values
	// 0010 0000 -> 0x20  (DMBLNK to 1, ie blinky mode)
    lcd_setReg(REG_MODE2, 0x20);

	// Set color to white 
	lcd_setReg(REG_RED, 255);
	lcd_setReg(REG_GREEN, 255);
	lcd_setReg(REG_BLUE, 255);
}

 /*-------------------------------------------------------------------------
 * lcd_write: Display character strings on LCD
 ---------------------------------------------------------------------------*/

 void lcd_write(char *data, uint8_t size)
{
  uint8_t buffer[LCD_COLS+1];
  buffer[0] = 0x40;
  if (size > LCD_COLS) size = LCD_COLS;
  memcpy(&buffer[1], data, size);

  lcd_send(LCD_ADDR, buffer, size);
}
 /*-------------------------------------------------------------------------
 * lcd_cursor: Position cursor on a row and column
 ---------------------------------------------------------------------------*/

 void lcd_cursor(uint8_t row, uint8_t col)
{
  uint8_t buffer[2];

  buffer[0] = 0x80;
  buffer[1] = (row == 0)? 0x80 + col: 0xc0 + col;
  lcd_send(LCD_ADDR, buffer, 2);
}

 /*-------------------------------------------------------------------------
 * lcd_clear: Clear LCD display
 ---------------------------------------------------------------------------*/

 void lcd_clear(void)
{
   lcd_command(LCD_CLEARDISPLAY);
}

 void lcd_command(uint8_t value)
{
	uint8_t data[3] = {0x80, value};
	lcd_send(LCD_ADDR, data, 2);
}

 /***************************************************************************
 * Utility functions: Do not call these directly
 ***************************************************************************/

 void lcd_send(uint32_t i2c_addr, uint8_t *data, int length)
{
    uint32_t timeout = 100UL;
	cy_en_scb_i2c_status_t status;
	int cnt = 0UL;
			
/* Send Start condition, address and receive ACK/NACK response from slave */
    do 
	{
		status = Cy_SCB_I2C_MasterSendStart(mI2C_HW, i2c_addr, CY_SCB_I2C_WRITE_XFER, timeout, &mI2C_context);
		if (CY_SCB_I2C_SUCCESS != status)
		Cy_SCB_I2C_MasterSendStop(mI2C_HW, timeout, &mI2C_context);
		++cnt;
	}
	while ((CY_SCB_I2C_SUCCESS != status) && (cnt < 5));

    if (CY_SCB_I2C_SUCCESS != status){
		lcd_HandleError();
    }
    cnt = 0;
    do
	{
		/* Write byte and receive ACK/NACK response */
		status = Cy_SCB_I2C_MasterWriteByte(mI2C_HW, data[cnt], timeout, &mI2C_context);
		++cnt;
	}
	while((status == CY_SCB_I2C_SUCCESS) && (cnt < length));
	
/* Check status of transaction */
    if ((status == CY_SCB_I2C_SUCCESS)           ||
	    (status == CY_SCB_I2C_MASTER_MANUAL_NAK) ||
 	    (status == CY_SCB_I2C_MASTER_MANUAL_ADDR_NAK))
    {
     	/* Send Stop condition on the bus */
     	status = Cy_SCB_I2C_MasterSendStop(mI2C_HW, timeout, &mI2C_context);
    	if (status == CY_SCB_I2C_SUCCESS)
    	{
  	    	/* Data has been written into the slave */
	    return;
	    }
    }
    else
   	{
        lcd_HandleError();
	/* Other statuses do not require any actions */
    }
}

 void lcd_setReg(uint8_t reg_num, uint8_t val)
{
	uint8_t data[2];
   data[0] = reg_num;
   data[1] = val;
   lcd_send(RGB_ADDR, data, 2);
}

 void lcd_HandleError(void)
{   
	 /* Disable all interrupts. */
	__disable_irq();
	
	/* Infinite loop. */
	while(1u) {}
}
