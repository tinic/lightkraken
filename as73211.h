/*
Copyright 2019 Tinic Uro

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef _AS73211_H_
#define _AS73211_H_

#include <stdint.h>
#include <string.h>

#include "./color.h"

namespace lightkraken {

class AS73211 {
public:
	static AS73211 &instance();

	enum OperationalState {
		Software_reset                          = 0x0A,
		Configuration_state_Power_Down_state_on = 0x42,
		Configuration_state                     = 0x02,
		Measurement_state                       = 0x03,
		Measurement_state_Start_measurement     = 0x83,
		If_Measurement_state_Start_measurement  = 0x80,
		Measurement_state_Power_Down_state_on   = 0x43,
		Measurement_state_Start_measurement_and_internal_startup_Power_Down_state_on = 0xC3,
		If_Measurement_state_Start_measurement_and_internal_startup_Power_Down_state_on = 0xC0,
	};

	enum IntegrationGain {
		Gain2048 = 0x00,
		Gain1024 = 0x10,
		Gain512  = 0x20,
		Gain256  = 0x30,
		Gain128  = 0x40,
		Gain64   = 0x50,
		Gain32   = 0x60,
		Gain16   = 0x70,
		Gain8    = 0x80,
		Gain4    = 0x90,
		Gain2    = 0xA0,
		Gain1    = 0xB0,
	};

	enum IntegrationTime {
		Time1ms     = 0x00,
		Time2ms     = 0x01,
		Time4ms     = 0x02,
		Time8ms     = 0x03,
		Time16ms    = 0x04,
		Time32ms    = 0x05,
		Time64ms    = 0x06,
		Time128ms   = 0x07,
		Time256ms   = 0x08,
		Time512ms   = 0x09,
		Time1024ms  = 0x0A,
		Time2048ms  = 0x0B,
		Time4096ms  = 0x0C,
		Time8192ms  = 0x0D,
		Time16384ms = 0x0E
	};

	enum MeasurementDivider {
		Divider1   = 0x00,
		Divider2   = 0x08,
		Divider4   = 0x09,
		Divider8   = 0x0A,
		Divider16  = 0x0B,
		Divider32  = 0x0C,
		Divider64  = 0x0D,
		Divider128 = 0x0E,
		Divider256 = 0x0F
	};

	enum MeasurementMode {
		ModeContinous             = 0x00,
		ModeCommand               = 0x40,
		ModeSynchronizedStart     = 0x80,
		ModeSynchronizedStartStop = 0xC0
	};

	enum ReadyPinMode {
		ModePushPull  = 0x00,
		ModeOpenDrain = 0x08
	};

	enum InternalClockFrequency {
		Clock1024MHz = 0x00,
		Clock2048MHz = 0x01,
		Clock4096MHz = 0x02,
		Clock8192MHz = 0x03
	};

	float getTemperature();
	float getX();
	float getY();
	float getZ();

	bool newDataAvailable();

	void softwareReset();
	bool setState(OperationalState state);
	OperationalState getState();

	uint8_t getDeviceID();

	bool setGain(IntegrationGain gain);
	IntegrationGain getGain();
	uint16_t getGainValue();

	bool setGainAndTime(IntegrationGain gain, IntegrationTime time);
	uint8_t getGainAndTime();

	bool setTime(IntegrationTime time);
	IntegrationTime getTime();
	uint16_t getTimeValue();
	uint16_t getTimeValueBuffer();

	bool setConfiguration(
		bool measureTemperature, 
		MeasurementDivider divider, 
		MeasurementMode mmode,
		bool standby,
		ReadyPinMode readyMode,
		InternalClockFrequency freq);

	MeasurementDivider getDivider();
	uint16_t getDividerValue();
	bool setDivider(MeasurementDivider divider);

	InternalClockFrequency getClock();
	uint16_t getClockValue();
	bool setClock(InternalClockFrequency freq);

	void goToMeasurementMode();

private:
	void delay(int32_t ms);

	void writeByte(uint8_t reg, uint8_t _data);
	uint8_t readByte(uint8_t reg);
	uint16_t readData(uint8_t reg);

	float convertingToEe(uint8_t channel, uint16_t MRES_data);

	void init();

 	int i2cStart();
	int i2cWrite(uint8_t slaveAddr, uint8_t *pBuff, uint16_t nBytes); 
	int i2cRead(uint8_t slaveAddr, uint8_t *pBuff, uint16_t nBytes);	

	uint8_t _slaveAddress;
	uint8_t _setGain;
	uint8_t _setTime;
};

} // namespace lightkraken {


#endif  // #ifndef _AS73211_H_
