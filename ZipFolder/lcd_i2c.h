/* File Name: lcd_i2c.h
*
* Version: 1.0
*
* Description: This example project demonstrates the use of the 2x20 LCD display in the parts kit
* using an I2C master in the PSoC.

******************************************************************************
* Copyright (2020), Anujan Varma
******************************************************************************
* This software, should be used only as an example for CSE 121 labs.
******************************************************************************/

/***************************************
* Constants related to I2C
****************************************/


/* Delays in milliseconds */
#define CMD_TO_CMD_DELAY      (2000UL)
#define I2C_TIMEOUT           (100UL)
#define DELAY_5MS             (5UL)
#define DELAY_2SEC            (2000UL)

/* I2C slave addresses to communicate with */
#define LCD_ADDR (0x7c >> 1)
#define RGB_ADDR (0xc0 >> 1)

/***************************************
* Constants related to the LCD display
****************************************/

/* Number of columns */
#define LCD_COLS              20

/* Color Codes  */
#define COLOR_RED             (0x00UL)
#define COLOR_GREEN           (0x01UL)
#define COLOR_BLUE            (0x02UL)
#define COLOR_CYAN            (0x03UL)
#define COLOR_PURPLE          (0x04UL)
#define COLOR_YELLOW          (0x05UL)
#define COLOR_WHITE           (0x06UL)

#define REG_MODE1       0x00
#define REG_MODE2       0x01
#define REG_OUTPUT      0x08

/*!
 *  @brief commands
 */
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

/*!
 *  @brief flags for display entry mode
 */
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

/*!
 *  @brief flags for display on/off control
 */
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

/*!
 *  @brief flags for display/cursor shift
 */
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

/*!
 *  @brief flags for function set
 */
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// Backlight PWM register addresses

#define REG_RED         0x04        // pwm2
#define REG_GREEN       0x03        // pwm1
#define REG_BLUE        0x02        // pwm0

