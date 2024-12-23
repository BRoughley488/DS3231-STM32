/*
 * DS3231.c
 *
 *  Created on: Dec 19, 2024
 *      Author: Brandon
 */
#include "DS3231.h"

uint8_t debuggingVariable1, debuggingVariable2;

DS3231 DSRTCpriv;


void DS3231_Init(DS3231 *dev, I2C_HandleTypeDef *handle){

	//the struct created in DS3231.h is passed into this as a pointer, along with i2c handle

	dev->i2cHandle = handle; //set the struct handle parameter to the handle i am using

	DSRTCpriv.i2cHandle = handle;

	/*
	 * set all registers to 24h mode where applicable
	 */

	DS3231_LL_Change_bit(&DSRTCpriv, reg_hours, 6, 0);
	DS3231_LL_Change_bit(&DSRTCpriv, reg_alarm1_hours, 6, 0);
	DS3231_LL_Change_bit(&DSRTCpriv, reg_alarm2_hours, 6, 0);

	//EOSC enable (0)

	DS3231_LL_Change_bit(&DSRTCpriv, 0x0e, 7, 0); //EOSC

	DS3231_LL_Change_bit(&DSRTCpriv, 0x0e, 2, 1); //INTCN

	DS3231_LL_Change_bit(&DSRTCpriv, 0x0e, 5, 0); //CONV

}


void DS3231_Get_Time(DS3231 *dev){

	uint8_t buff = 0;

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_seconds, 1, &buff, 1, HAL_MAX_DELAY); //get seconds
	dev->seconds = (buff & 0x0f) + (((buff & 0b01110000) >> 4) * 10);

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_minutes, 1, &buff, 1, HAL_MAX_DELAY); //get minutes
	dev->minutes = (buff & 0x0f) + (((buff & 0b01110000) >> 4) * 10);

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_hours, 1, &buff, 1, HAL_MAX_DELAY);
	if ((buff & 0b01000000) == 0b01000000){ //if bit 6 of the hours address = HIGH, clock is in 12hr mode, LOW is 24h mode
		dev->hours = (buff & 0x0f) + (((buff & 0b00010000) >> 4) * 10); // set number of hrs in 12h mode

		if((buff & 0b00100000) == 0b00100000){ //if AM/PM flag is HIGH (PM)
			dev -> AMPM = 1;
		}
		else{
			dev -> AMPM = 0;
		}
	}
	else{ //clock is in 24h mode
		dev -> hours = (buff & 0x0f) + (((buff & 0b00110000) >> 4) * 10);
	}

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_day, 1, &buff, 1, HAL_MAX_DELAY);
	dev->dayOfWeek = buff & 0b00000111;

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_date, 1, &buff, 1, HAL_MAX_DELAY);
	dev->date = (buff & 0x0f) + (((buff & 0b00110000) >> 4) * 10);

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_month_century, 1, &buff, 1, HAL_MAX_DELAY);
	dev->month = (buff & 0x0f) + (((buff & 0b00010000) >> 4) * 10);

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_year, 1, &buff, 1, HAL_MAX_DELAY);
	dev -> year = (buff & 0x0f) + (((buff & 0xf0) >> 4) * 10);
}

void DS3231_Set_Time(DS3231 *dev, uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t date, uint8_t month, uint8_t year){

	uint8_t buffTX;

	/*
	 * Write seconds register
	 */

	if (seconds >= 10){
		buffTX = floor(seconds / 10); //get the 10 second portion
		buffTX = buffTX << 4; // bit shift 4 bits left e.g. 00000011 = 00110000

		if (seconds % 10 != 0){
			buffTX = buffTX | (seconds % 10); //insert least significant digit using logic AND, if it is not a multiple of 10
		}
	}

	else{
		buffTX = seconds;
	}

	HAL_I2C_Mem_Write(dev ->i2cHandle, addr, reg_seconds, 1, &buffTX, 1, HAL_MAX_DELAY);

	/*
	 * Write minutes register
	 */

	if (minutes >= 10){
		buffTX = floor(minutes / 10); //get the 10 second portion
		buffTX = buffTX << 4; // bit shift 4 bits left e.g. 00000011 = 00110000

		if (minutes % 10 != 0){
			buffTX = buffTX | (minutes % 10); //insert least significant digit using logic AND, if it is not a multiple of 10
		}
	}

	else{
		buffTX = minutes;
	}

	HAL_I2C_Mem_Write(dev ->i2cHandle, addr, reg_minutes, 1, &buffTX, 1, HAL_MAX_DELAY);

	/*
	 * Write hour register
	 */

	if (hours >= 10){

		buffTX = floor(hours / 10); //get the 10 second portion
		buffTX = buffTX << 4; // bit shift 4 bits left e.g. 00000011 = 00110000

		if (hours % 10 != 0){
			buffTX = buffTX | (hours % 10); //insert least significant digit using logic AND, if it is not a multiple of 10
		}
	}

	else{
		buffTX = hours;
	}


	HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_hours, 1, &buffTX, 1, HAL_MAX_DELAY);

	/*
	 * Write Day Register
	 */

	HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_day, 1, &day, 1, HAL_MAX_DELAY);

	/*
	 * Write Date Register
	 */

	if (date >= 10){
		buffTX = floor(date/10);
		buffTX = buffTX << 4;

		if ((date % 10) != 0){

			buffTX = buffTX | (date % 10);

		}
	}

	else{
		buffTX = date;
	}

	HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_date, 1, &buffTX, 1, HAL_MAX_DELAY);

	/*
	 * Write Month Register i am not doing century i cannot be fucked
	 */

	if (month >= 10){
		buffTX = 1 << 4;

		buffTX = buffTX | (month % 10);

	}

	else{
		buffTX = month;
	}

	HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_month_century, 1, &buffTX, 1, HAL_MAX_DELAY);

	/*
	 * Write Year Register
	 */
	if (year >= 10){

		buffTX = (year / 10) << 4;
		buffTX = buffTX | (year % 10);

	}

	else{
		buffTX = year;
	}

	HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_year, 1, &buffTX, 1, HAL_MAX_DELAY);

}

void DS3231_Get_Temperature(DS3231 *dev){

	uint8_t buff[2];

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_temperature, 1, &buff[0], 1, HAL_MAX_DELAY);
	HAL_I2C_Mem_Read(dev->i2cHandle, addr, 0x12, 1, &buff[1], 1, HAL_MAX_DELAY);

	dev -> temp = (int8_t)buff[0];

	//giving up with the decimal point for now, fucked if i know whey it's not working


	//DOES NOT WORK YET ____________________________________________
}

void DS3231_alarm1_enable(DS3231 *dev){

	uint8_t buff = 0;
	uint8_t buffTX = 0;

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_control_register, 1, &buff, 1, HAL_MAX_DELAY);

	buffTX = buff | 0b00000001; //change A1IE bit to 1

	HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_control_register, 1, &buffTX, 1, HAL_MAX_DELAY); // write to register

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_control_register, 1, &buff, 1, HAL_MAX_DELAY);

	dev -> controlRegister = buff; //verify register changed

}

void DS3231_alarm2_enable(DS3231 *dev){

	uint8_t buff = 0;
	uint8_t buffTX = 0;

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_control_register, 1, &buff, 1, HAL_MAX_DELAY);

	buffTX = buff | 0b00000010; //change A2IE bit to 1

	HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_control_register, 1, &buffTX, 1, HAL_MAX_DELAY); // write to register

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_control_register, 1, &buff, 1, HAL_MAX_DELAY);

	dev -> controlRegister = buff; //verify register changed

}

void DS3231_alarm1_disable(DS3231 *dev){

	uint8_t buff = 0;
	uint8_t buffTX = 0;

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_control_register, 1, &buff, 1, HAL_MAX_DELAY);

	buffTX = buff & 0b11111110; //change A1IE bit to 0

	HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_control_register, 1, &buffTX, 1, HAL_MAX_DELAY); // write to register

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_control_register, 1, &buff, 1, HAL_MAX_DELAY);

	dev -> controlRegister = buff; //verify register changed

}

void DS3231_alarm2_disable(DS3231 *dev){

	uint8_t buff = 0;
	uint8_t buffTX = 0;

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_control_register, 1, &buff, 1, HAL_MAX_DELAY);

	buffTX = buff & 0b11111101; //change A2IE bit to 0

	HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_control_register, 1, &buffTX, 1, HAL_MAX_DELAY); // write to register

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_control_register, 1, &buff, 1, HAL_MAX_DELAY);

	dev -> controlRegister = buff; //verify register changed (they have i promise)

}

void DS3231_set_alarm1(DS3231 *dev, uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t date, uint8_t DYDT, uint16_t mask){

	uint8_t buffRX;
	uint8_t buffTX;

	/*
	 * Write seconds register
	 */

	if (seconds >= 10){
		buffTX = floor(seconds / 10); //get the 10 second portion
		buffTX = buffTX << 4; // bit shift 4 bits left e.g. 00000011 = 00110000

		if (seconds % 10 != 0){
			buffTX = buffTX | (seconds % 10); //insert least significant digit using logic AND, if it is not a multiple of 10
		}
	}

	else{
		buffTX = seconds;
	}

	/*
	 * make sure bit 7 (A1M1) does not get changed
	 */

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm1_seconds, 1, &buffRX, 1, HAL_MAX_DELAY);
	buffRX = buffRX & 0b10000000; // preserve only bit7 MSB of the register
	buffTX = buffTX | buffRX;

	HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_alarm1_seconds, 1, &buffTX, 1, HAL_MAX_DELAY); //write 10 seconds bits

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm1_seconds, 1, &debuggingVariable1, 1, HAL_MAX_DELAY);

	/*
	 * write minutes register
	 */

	if (minutes >= 10){
		buffTX = floor(minutes / 10); //get the 10 second portion
		buffTX = buffTX << 4; // bit shift 4 bits left e.g. 00000011 = 00110000

		if (minutes % 10 != 0){
			buffTX = buffTX | (minutes % 10); //insert least significant digit using logic AND, if it is not a multiple of 10
		}
	}

	else{
		buffTX = minutes;
	}

	/*
	 * make sure bit 7 (A2M2) does not get changed
	 */

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm1_minutes, 1, &buffRX, 1, HAL_MAX_DELAY);
	buffRX = buffRX & 0b10000000; // preserve only bit7 MSB of the register
	buffTX = buffTX | buffRX;

	HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_alarm1_minutes, 1, &buffTX, 1, HAL_MAX_DELAY); //write 10 minutes bits
	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm1_minutes, 1, &debuggingVariable2, 1, HAL_MAX_DELAY);

	/*
	 * Write hours register
	 */

	if (hours >= 10){
			buffTX = floor(hours / 10); //get the 10 second portion
			buffTX = buffTX << 4; // bit shift 4 bits left e.g. 00000011 = 00110000

			if (hours % 10 != 0){
				buffTX = buffTX | (hours % 10); //insert least significant digit using logic AND, if it is not a multiple of 10
			}
		}

		else{
			buffTX = hours;
		}

		/*
		 * make sure bit 7,6,5 dont get changed
		 */

		HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm1_hours, 1, &buffRX, 1, HAL_MAX_DELAY);
		buffRX = buffRX & 0b11100000;
		buffTX = buffTX | buffRX;

		HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_alarm1_hours, 1, &buffTX, 1, HAL_MAX_DELAY); //write 20 hour bits
		HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm1_hours, 1, &debuggingVariable2, 1, HAL_MAX_DELAY);

		/*
		 * Write day / date register
		 */

		if (DYDT){
				DS3231_LL_Change_bit(&DSRTCpriv, reg_alarm1_day_date, 6, 1); //change DYDT bit
			}

			else {
				DS3231_LL_Change_bit(&DSRTCpriv, reg_alarm1_day_date, 6, 0);
			}

		HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm1_day_date, 1, &buffRX, 1, HAL_MAX_DELAY);

		if ((buffRX & 0b01000000) == 0b01000000){ //if DY/DT bit = 1 (day)

			buffTX = (day & 0b00001111);

			HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm1_day_date, 1, &buffRX, 1, HAL_MAX_DELAY);

			buffRX = buffRX & 0b11110000;
			buffTX = buffTX | buffRX;

		}

		else {  //if DY/DT bit = 0 (date)

			buffTX = floor(date / 10); //get the 10 second portion
			buffTX = buffTX << 4; // bit shift 4 bits left e.g. 00000011 = 00110000

			if ((date % 10) != 0){
				buffTX = buffTX | (date % 10);
			}

		}

		/*
		 * make sure first 2 bits dont get changed
		 */

		HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm1_day_date, 1, &buffRX, 1, HAL_MAX_DELAY);

		buffRX = buffRX & 0b11000000;
		buffTX = buffTX | buffRX;

		HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_alarm1_day_date, 1, &buffTX, 1, HAL_MAX_DELAY); //write day / date

		/*
		 * set mask bits
		 */

		switch (mask) {
		case 1111:
			DS3231_LL_Change_bit(&DSRTCpriv, 0x07, 7, 1);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x08, 7, 1);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x09, 7, 1);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x0A, 7, 1);
			break;
		case 1110:

			DS3231_LL_Change_bit(&DSRTCpriv, 0x07, 7, 0);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x08, 7, 1);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x09, 7, 1);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x0A, 7, 1);

			break;
		case 1100:

			DS3231_LL_Change_bit(&DSRTCpriv, 0x07, 7, 0);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x08, 7, 0);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x09, 7, 1);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x0A, 7, 1);

			break;
		case 1000:

			DS3231_LL_Change_bit(&DSRTCpriv, 0x07, 7, 0);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x08, 7, 0);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x09, 7, 0);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x0A, 7, 1);

			break;
		case 0000:

			DS3231_LL_Change_bit(&DSRTCpriv, 0x07, 7, 0);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x08, 7, 0);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x09, 7, 0);
			DS3231_LL_Change_bit(&DSRTCpriv, 0x0A, 7, 0);

			break;

		}
}

void DS3231_set_alarm2(DS3231 *dev, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t date, uint8_t DYDT, uint16_t mask){


	uint8_t buffRX;
	uint8_t buffTX;

	/*
	 * write minutes register
	 */

	if (minutes >= 10){
		buffTX = floor(minutes / 10); //get the 10 second portion
		buffTX = buffTX << 4; // bit shift 4 bits left e.g. 00000011 = 00110000

		if (minutes % 10 != 0){
			buffTX = buffTX | (minutes % 10); //insert least significant digit using logic AND, if it is not a multiple of 10
		}
	}

	else{
		buffTX = minutes;
	}

	/*
	 * make sure bit 7 (A2M2) does not get changed
	 */

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm2_minutes, 1, &buffRX, 1, HAL_MAX_DELAY);
	buffRX = buffRX & 0b10000000; // preserve only bit7 MSB of the register
	buffTX = buffTX | buffRX;

	HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_alarm2_minutes, 1, &buffTX, 1, HAL_MAX_DELAY); //write 10 minutes bits
	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm2_minutes, 1, &debuggingVariable2, 1, HAL_MAX_DELAY);

	/*
	 * Write hours register
	 */

	if (hours >= 10){
			buffTX = floor(hours / 10); //get the 10 second portion
			buffTX = buffTX << 4; // bit shift 4 bits left e.g. 00000011 = 00110000

			if (hours % 10 != 0){
				buffTX = buffTX | (hours % 10); //insert least significant digit using logic AND, if it is not a multiple of 10
			}
		}

		else{
			buffTX = hours;
		}

		/*
		 * make sure bit 7,6,5 dont get changed
		 */

		HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm2_hours, 1, &buffRX, 1, HAL_MAX_DELAY);
		buffRX = buffRX & 0b11100000;
		buffTX = buffTX | buffRX;

		HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_alarm2_hours, 1, &buffTX, 1, HAL_MAX_DELAY); //write 20 hour bits
		HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm2_hours, 1, &debuggingVariable2, 1, HAL_MAX_DELAY);

		/*
		 * Write day / date register
		 *
		 * DY = DYDT 1
		 *
		 * DT = DYDT 0
		 *
		 */

		if (DYDT){
			DS3231_LL_Change_bit(&DSRTCpriv, reg_alarm2_day_date, 6, 1); //change DYDT bit
		}

		else {
			DS3231_LL_Change_bit(&DSRTCpriv, reg_alarm2_day_date, 6, 0);
		}

		HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm2_day_date, 1, &buffRX, 1, HAL_MAX_DELAY);

		if ((buffRX & 0b01000000) == 0b01000000){ //if DY/DT bit = 1 (day)

			buffTX = (day & 0b00001111);

			HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm2_day_date, 1, &buffRX, 1, HAL_MAX_DELAY);

			buffRX = buffRX & 0b11110000;
			buffTX = buffTX | buffRX;

		}

		else {  //if DY/DT bit = 0 (date)

			buffTX = floor(date / 10); //get the 10 second portion
			buffTX = buffTX << 4; // bit shift 4 bits left e.g. 00000011 = 00110000

			if ((date % 10) != 0){
				buffTX = buffTX | (date % 10);
			}

		}

		/*
		 * make sure first 2 bits dont get changed
		 */

		HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_alarm2_day_date, 1, &buffRX, 1, HAL_MAX_DELAY);

		buffRX = buffRX & 0b11000000;
		buffTX = buffTX | buffRX;

		HAL_I2C_Mem_Write(dev -> i2cHandle, addr, reg_alarm2_day_date, 1, &buffTX, 1, HAL_MAX_DELAY); //write day / date

		switch (mask) {
			case 111:
				DS3231_LL_Change_bit(&DSRTCpriv, 0x0b, 7, 1);
				DS3231_LL_Change_bit(&DSRTCpriv, 0x0c, 7, 1);
				DS3231_LL_Change_bit(&DSRTCpriv, 0x0d, 7, 1);
				break;
			case 110:

				DS3231_LL_Change_bit(&DSRTCpriv, 0x0b, 7, 0);
				DS3231_LL_Change_bit(&DSRTCpriv, 0x0c, 7, 1);
				DS3231_LL_Change_bit(&DSRTCpriv, 0x0d, 7, 1);

				break;
			case 100:

				DS3231_LL_Change_bit(&DSRTCpriv, 0x0b, 7, 0);
				DS3231_LL_Change_bit(&DSRTCpriv, 0x0c, 7, 0);
				DS3231_LL_Change_bit(&DSRTCpriv, 0x0d, 7, 1);

				break;
			case 000:

				DS3231_LL_Change_bit(&DSRTCpriv, 0x0b, 7, 0);
				DS3231_LL_Change_bit(&DSRTCpriv, 0x0c, 7, 0);
				DS3231_LL_Change_bit(&DSRTCpriv, 0x0d, 7, 0);

				break;

			}
}

void DS3231_check_alarms(DS3231 *dev){

	uint8_t buff;

	/*
	 * CHECK ALARM 1 REGISTERS
	 */

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_alarm1_seconds, 1, &buff, 1, HAL_MAX_DELAY); //get alarm 1 seconds
	dev->alarm1Seconds = (buff & 0x0f) + (((buff & 0b01110000) >> 4) * 10);

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_alarm1_minutes, 1, &buff, 1, HAL_MAX_DELAY); //get alarm 1minutes
	dev->alarm1Minutes = (buff & 0x0f) + (((buff & 0b01110000) >> 4) * 10);

	/*
	 * get alarm 1 hours
	 */

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_alarm1_hours, 1, &buff, 1, HAL_MAX_DELAY);
	if ((buff & 0b01000000) == 0b01000000){ //if bit 6 of the hours address = HIGH, clock is in 12hr mode, LOW is 24h mode

		dev->alarm1Hours = (buff & 0x0f) + (((buff & 0b00010000) >> 4) * 10); // set number of hrs in 12h mode

		if((buff & 0b00100000) == 0b00100000){ //if AM/PM flag is HIGH (PM)
			dev -> alarm1AMPM = 1;
		}
		else{
			dev -> alarm1AMPM = 0;
		}
	}
	else{ //clock is in 24h mode
		dev -> alarm1Hours = (buff & 0x0f) + (((buff & 0b00110000) >> 4) * 10);
	}

	/*
	 * CHECK ALARM 1 DAY / DATE
	 */

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_alarm1_day_date, 1, &buff, 1, HAL_MAX_DELAY);

	if ((buff & 0b01000000) == 0b01000000){ //if DY/DT is HIGH
		dev->alarm1Day = buff & 0b00001111;
	}

	else{
		dev->alarm1Date = (buff & 0x0f) + (((buff & 0b00110000) >> 4) * 10);
	}

	/*
	 * CHECK ALARM 2 REGISTERS
	 */

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_alarm2_minutes, 1, &buff, 1, HAL_MAX_DELAY); //get alarm 1minutes
	dev->alarm2Minutes = (buff & 0x0f) + (((buff & 0b01110000) >> 4) * 10);

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_alarm2_hours, 1, &buff, 1, HAL_MAX_DELAY);
		if ((buff & 0b01000000) == 0b01000000){ //if bit 6 of the hours address = HIGH, clock is in 12hr mode, LOW is 24h mode

			dev->alarm2Hours = (buff & 0x0f) + (((buff & 0b00010000) >> 4) * 10); // set number of hrs in 12h mode

			if((buff & 0b00100000) == 0b00100000){ //if AM/PM flag is HIGH (PM)
				dev -> alarm2AMPM = 1;
			}
			else{
				dev -> alarm2AMPM = 0;
			}
		}
		else{ //clock is in 24h mode
			dev -> alarm2Hours = (buff & 0x0f) + (((buff & 0b00110000) >> 4) * 10);
		}

	/*
	 * CHECK ALARM 2 DAY / DATE
	 */

	HAL_I2C_Mem_Read(dev->i2cHandle, addr, reg_alarm2_day_date, 1, &buff, 1, HAL_MAX_DELAY);

	if ((buff & 0b01000000) == 0b01000000){ //if DY/DT is HIGH
		dev->alarm2Day = buff & 0b00001111;
	}

	else{
		dev->alarm2Date = (buff & 0x0f) + (((buff & 0b00110000) >> 4) * 10);
	}

}

void DS3231_LL_Change_bit(DS3231 *dev, uint8_t regAddress, uint8_t bitPos, uint8_t bitValue){

	uint8_t reg = 0;

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, regAddress, 1, &reg, 1, HAL_MAX_DELAY);

	if(bitValue){
		reg |= (1 << bitPos);
	}
	else{
		reg &= ~(1 << bitPos);
	}

	HAL_I2C_Mem_Write(dev -> i2cHandle, addr, regAddress, 1, &reg, 1, HAL_MAX_DELAY);


}

void DS3231_enable_sq(DS3231 *dev){

	DS3231_LL_Change_bit(&DSRTCpriv, 0x0e, 6, 1);

}

void DS3231_check_alarm_flags(DS3231 *dev){

	uint8_t buff;

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_controlStatus, 1, &buff, 1, HAL_MAX_DELAY);

	if(buff & 0b00000001){

		dev -> alarm1flag = 1;
	}

	if((buff & 0b00000001) == 0){

		dev -> alarm1flag = 0;
	}

	if(buff & 0b00000010){

		dev -> alarm2flag = 1;
	}

	if((buff & 0b00000010) == 0){

		dev -> alarm2flag = 0;
	}
}

void DS3231_reset_alarm_flags(DS3231 *dev){

	DS3231_LL_Change_bit(&DSRTCpriv, reg_controlStatus, 0, 0);
	DS3231_LL_Change_bit(&DSRTCpriv, reg_controlStatus, 1, 0);
}

void DS3231_check_status_register(DS3231 *dev){
	uint8_t buff;

	HAL_I2C_Mem_Read(dev -> i2cHandle, addr, reg_controlStatus, 1, &buff, 1, HAL_MAX_DELAY);

	dev -> statusRegister = buff;
}

void DS3231_reset_OSF_flag(DS3231 *dev){
	DS3231_LL_Change_bit(&DSRTCpriv, reg_controlStatus, 7, 0);
}
