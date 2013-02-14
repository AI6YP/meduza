/*
 * Platform configuration file
 */

#ifndef _PLATFORM_CONFIG_H
#define _PLATFORM_CONFIG_H

#define		TMR1_PRE		3
#define		TMR1_PERIOD		15625
#define		TMR3_PRE		3
#define		ADC_CONV_PERIOD		15

// remote control
#define		RC_ZERO_MIN		45
#define		RC_ZERO_MAX		84
#define		RC_ONE_MIN		98
#define		RC_ONE_MAX		181
#define		RC_START_MIN		300
#define		RC_START_MAX		1000

#define		U1_BRG		0
#define		U1_BRGH		1
// rate		[bit/s]		4,000,000

// MMA8452QL -- Digital Accelerometer
// R8 = short
// #define		I2C_ACC			0x38
// R8 = open
#define		I2C_ACC			0x3A

// MAG3110 -- Three-Axis, Digital Magnetometer
#define		I2C_MAG			0x1C

// MCP9800 -- High-Accuracy Temperature Sensor
#define		I2C_TMP			0x90

// L3GD20 -- three-axis digital output gyroscope
#define		I2C_GYR			0xD4


// MMA8452QL registers and constants
#define		ACC_STATUS			0x00
#define		ACC_OUT_X_MSB		0x01
#define		ACC_F_SETUP			0x09
#define		ACC_TRIG_CFG		0x0A
// Device ID (0x1A)
#define		ACC_WHO_AM_I		0x0D
#define		ACC_XYZ_DATA_CFG	0x0E
#define		ACC_CTRL_REG1		0x2A
#define		ACC_CTRL_REG2		0x2B
#define		ACC_CTRL_REG3		0x2C
#define		ACC_CTRL_REG4		0x2D
#define		ACC_CTRL_REG5		0x2E

#endif /* _PLATFORM_CONFIG_H */
