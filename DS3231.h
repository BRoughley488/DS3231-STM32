/*
 * DS3231.h
 *
 *  Created on: Dec 19, 2024
 *      Author: Brandon
 *
 *  RTC module set to 24h clock
 */

#ifndef INC_DS3231_H_
#define INC_DS3231_H_

#include "stm32f4xx_hal.h"
#include <math.h>

/*
 * MACROS?? I THINK THATS WHAT YOU CALL THIS
 */

#define DS3231_ALARM1_ONCE_PER_SECOND 1111
#define DS3231_ALARM1_SECONDS_MATCH 1110
#define DS3231_ALARM1_MINUTES_SECONDS_MATCH 1100
#define DS3231_ALARM1_HOURS_MINUTES_SECONDS 1000
#define DS3231_ALARM1_DATEDAY_HOURS_MINUTES_SECONDS 0000

#define DS3231_ALARM2_ONCE_PER_MINUTE 111
#define DS3231_ALARM2_MINUTES_MATCH 110
#define DS3231_ALARM2_HOURS_MINUTES 100
#define DS3231_ALARM2_DATEDAY_HOURS_MINUTES 000

//defines

#define addr	(0x68 << 1)


/*
 * Registers listed on page 11 of datasheet
 */

/*
 * TIME REGISTERS
 */

#define reg_seconds 0x00
#define reg_minutes 0x01
#define reg_hours 0x02
#define reg_day 0x03
#define reg_date 0x04
#define reg_month_century 0x05
#define reg_year 0x06

/*
 * ALARM REGISTERS
 */

#define reg_alarm1_seconds 0x07
#define reg_alarm1_minutes 0x08
#define reg_alarm1_hours 0x09
#define reg_alarm1_day_date 0x0a

#define reg_alarm2_minutes 0x0b
#define reg_alarm2_hours 0x0c
#define reg_alarm2_day_date 0x0d

/*
 * TEMPERATURE REGISTERS
 */

#define reg_temperature 0x11
#define reg_temperature_fractional 0x12

/*
 * control registers
 */

#define reg_control_register 0x0e
#define reg_controlStatus 0x0f

#define reg_aging_offset 0x10

typedef struct {
	I2C_HandleTypeDef *i2cHandle;

	/*
	 * Current time variables
	 */

	int seconds;
	int minutes;
	int hours;
	int AMPM; // PM = 1, AM = 0, ignore if in 24h mode
	int dayOfWeek;
	int date;
	int month;
	int year;

	/*
	 * alarm variables
	 */

	int alarm1Seconds, alarm1Minutes, alarm1Hours, alarm1AMPM, alarm1Day, alarm1Date;
	int alarm2Minutes, alarm2Hours,alarm2AMPM, alarm2Day, alarm2Date;

	int temp; //temperature variables

	int controlRegister; //self explanatory
	int statusRegister;

	int alarm1flag;
	int alarm2flag;


} DS3231;


//functions

void DS3231_Init(DS3231 *dev, I2C_HandleTypeDef *handle);

void DS3231_Get_Time(DS3231 *dev);

void DS3231_Set_Time(DS3231 *dev, uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t date, uint8_t month, uint8_t year);

void DS3231_Get_Temperature(DS3231 *dev);

void DS3231_alarm1_enable(DS3231 *dev);

void DS3231_alarm2_enable(DS3231 *dev);

void DS3231_alarm1_disable(DS3231 *dev);

void DS3231_alarm2_disable(DS3231 *dev);

void DS3231_set_alarm1(DS3231 *dev, uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t date, uint8_t DYDT, uint16_t mask);

void DS3231_set_alarm2(DS3231 *dev, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t date, uint8_t DYDT, uint16_t mask);

void DS3231_check_alarms(DS3231 *dev);

void DS3231_enable_sq(DS3231 *dev);

void DS3231_check_alarm_flags(DS3231 *dev);

void DS3231_reset_alarm_flags(DS3231 *dev);

void DS3231_check_status_register(DS3231 *dev);

void DS3231_reset_OSF_flag(DS3231 *dev);
/*
 * LL functions
 */

void DS3231_LL_Change_bit(DS3231 *dev, uint8_t regAddress, uint8_t bitPos, uint8_t bitValue);

#endif /* INC_DS3231_H_ */
