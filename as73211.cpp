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

#include "./as73211.h"

namespace lightkraken {

constexpr uint8_t AS73211_REG_OPERATIONAL_STATE        = 0x00;
constexpr uint8_t AS73211_REG_API_GENERATION           = 0x02;
constexpr uint8_t AS73211_REG_CONFIGURATION_REGISTER_1 = 0x06;
constexpr uint8_t AS73211_REG_CONFIGURATION_REGISTER_2 = 0x07;
constexpr uint8_t AS73211_REG_CONFIGURATION_REGISTER_3 = 0x08;
constexpr uint8_t AS73211_REG_BREAK_TIME               = 0x09;
constexpr uint8_t AS73211_REG_EDGE_COUNT_VALUE         = 0x0A;
constexpr uint8_t AS73211_REG_OPTIONS_REGISTER         = 0x0B;

constexpr uint8_t AS73211_OSR_STOP_MEASUREMENT        = 0x00;
constexpr uint8_t AS73211_OSR_START_MEASUREMENT       = 0x80;
constexpr uint8_t AS73211_OSR_POWER_DOWN_SWITCHED_OFF = 0x00;
constexpr uint8_t AS73211_OSR_POWER_DOWN_SWITCHED_ON  = 0x40;
constexpr uint8_t AS73211_OSR_SOFTWARE_RESET          = 0x08;
constexpr uint8_t AS73211_OSR_DOS_CONFIGURATION       = 0x02;
constexpr uint8_t AS73211_OSR_DOS_MEASUREMENT         = 0x03;

constexpr uint8_t AS73211_AGEN_DEVICE_ID       = 0x01;
constexpr uint8_t AS73211_AGEN_MUTATION_NUMBER = 0x02;

constexpr uint8_t AS73211_CREG1_GAIN_XYZ_2048 = 0x00;
constexpr uint8_t AS73211_CREG1_GAIN_XYZ_1024 = 0x10;
constexpr uint8_t AS73211_CREG1_GAIN_XYZ_512  = 0x20;
constexpr uint8_t AS73211_CREG1_GAIN_XYZ_256  = 0x30;
constexpr uint8_t AS73211_CREG1_GAIN_XYZ_128  = 0x40;
constexpr uint8_t AS73211_CREG1_GAIN_XYZ_64   = 0x50;
constexpr uint8_t AS73211_CREG1_GAIN_XYZ_32   = 0x60;
constexpr uint8_t AS73211_CREG1_GAIN_XYZ_16   = 0x70;
constexpr uint8_t AS73211_CREG1_GAIN_XYZ_8    = 0x80;
constexpr uint8_t AS73211_CREG1_GAIN_XYZ_4    = 0x90;
constexpr uint8_t AS73211_CREG1_GAIN_XYZ_2    = 0xA0;
constexpr uint8_t AS73211_CREG1_GAIN_XYZ_1    = 0xB0;

constexpr uint8_t AS73211_CREG1_TIME_1ms      = 0x00;
constexpr uint8_t AS73211_CREG1_TIME_2ms      = 0x01;
constexpr uint8_t AS73211_CREG1_TIME_4ms      = 0x02;
constexpr uint8_t AS73211_CREG1_TIME_8ms      = 0x03;
constexpr uint8_t AS73211_CREG1_TIME_16ms     = 0x04;
constexpr uint8_t AS73211_CREG1_TIME_32ms     = 0x05;
constexpr uint8_t AS73211_CREG1_TIME_64ms     = 0x06;
constexpr uint8_t AS73211_CREG1_TIME_128ms    = 0x07;
constexpr uint8_t AS73211_CREG1_TIME_256ms    = 0x08;
constexpr uint8_t AS73211_CREG1_TIME_512ms    = 0x09;
constexpr uint8_t AS73211_CREG1_TIME_1024ms   = 0x0A;
constexpr uint8_t AS73211_CREG1_TIME_2048ms   = 0x0B;
constexpr uint8_t AS73211_CREG1_TIME_4096ms   = 0x0C;
constexpr uint8_t AS73211_CREG1_TIME_8192ms   = 0x0D;
constexpr uint8_t AS73211_CREG1_TIME_16384ms  = 0x0E;

constexpr uint8_t AS73211_CREG2_EN_TM_DISABLE           = 0x00;
constexpr uint8_t AS73211_CREG2_EN_TM_ENABLE            = 0x40;
constexpr uint8_t AS73211_CREG2_DIGITAL_DIVIDER_DISABLE = 0x00;
constexpr uint8_t AS73211_CREG2_DIGITAL_DIVIDER_ENABLE  = 0x08;
constexpr uint8_t AS73211_CREG2_DIVIDER_2               = 0x00;
constexpr uint8_t AS73211_CREG2_DIVIDER_4               = 0x01;
constexpr uint8_t AS73211_CREG2_DIVIDER_8               = 0x02;
constexpr uint8_t AS73211_CREG2_DIVIDER_16              = 0x03;
constexpr uint8_t AS73211_CREG2_DIVIDER_32              = 0x04;
constexpr uint8_t AS73211_CREG2_DIVIDER_64              = 0x05;
constexpr uint8_t AS73211_CREG2_DIVIDER_128             = 0x06;
constexpr uint8_t AS73211_CREG2_DIVIDER_256             = 0x07;

constexpr uint8_t AS73211_CREG3_MMODE_CONT_MODE         = 0x00;
constexpr uint8_t AS73211_CREG3_MMODE_CMD_MODE          = 0x40;
constexpr uint8_t AS73211_CREG3_MMODE_SYNS_MODE         = 0x80;
constexpr uint8_t AS73211_CREG3_MMODE_SIND_MODE         = 0xC0;
constexpr uint8_t AS73211_CREG3_SB_STANDBY_SWITCHED_OFF = 0x00;
constexpr uint8_t AS73211_CREG3_SB_STANDBY_SWITCHED_ON  = 0x10;
constexpr uint8_t AS73211_CREG3_READY_PUSH_PULL_OUT     = 0x00;
constexpr uint8_t AS73211_CREG3_READY_OPEN_DRAIN_OUT    = 0x08;
constexpr uint8_t AS73211_CREG3_INTERNAL_CLOCK_1024MHZ  = 0x00;
constexpr uint8_t AS73211_CREG3_INTERNAL_CLOCK_2048MHZ  = 0x01;
constexpr uint8_t AS73211_CREG3_INTERNAL_CLOCK_4096MHZ  = 0x02;
constexpr uint8_t AS73211_CREG3_INTERNAL_CLOCK_8192MHZ  = 0x03;

constexpr uint8_t AS73211_MREG_STATUS_REGISTER         = 0x00;
constexpr uint8_t AS73211_MREG_TEMPERATURE_MEASUREMENT = 0x01;
constexpr uint8_t AS73211_MREG_MEASUREMENT_X_CHANNEL   = 0x02;
constexpr uint8_t AS73211_MREG_MEASUREMENT_Y_CHANNEL   = 0x03;
constexpr uint8_t AS73211_MREG_MEASUREMENT_Z_CHANNEL   = 0x04;
constexpr uint8_t AS73211_MREG_OUT_CONVERSION_LSB      = 0x05;
constexpr uint8_t AS73211_MREG_OUT_CONVERSION_MSB      = 0x06;

constexpr uint8_t AS73211_STATUS_OUTCONVOF = 0x80;
constexpr uint8_t AS73211_STATUS_MRESOF    = 0x40;
constexpr uint8_t AS73211_STATUS_ADCOF     = 0x20;
constexpr uint8_t AS73211_STATUS_LDATA     = 0x10;
constexpr uint8_t AS73211_STATUS_NDATA     = 0x08;
constexpr uint8_t AS73211_STATUS_NOTREADY  = 0x04;
constexpr uint8_t AS73211_STATUS_STANDBY   = 0x02;
constexpr uint8_t AS73211_STATUS_POWER     = 0x01;

constexpr float AS73211_X_FSR_OF_GAIN_2048 = 0.866f;
constexpr float AS73211_Y_FSR_OF_GAIN_2048 = 0.932f;
constexpr float AS73211_Z_FSR_OF_GAIN_2048 = 0.501f;
constexpr float AS73211_X_FSR_OF_GAIN_1024 = 1.732f;
constexpr float AS73211_Y_FSR_OF_GAIN_1024 = 1.865f;
constexpr float AS73211_Z_FSR_OF_GAIN_1024 = 1.002f;
constexpr float AS73211_X_FSR_OF_GAIN_512  = 3.463f;
constexpr float AS73211_Y_FSR_OF_GAIN_512  = 3.730f;
constexpr float AS73211_Z_FSR_OF_GAIN_512  = 2.003f;
constexpr float AS73211_X_FSR_OF_GAIN_256  = 6.927f;
constexpr float AS73211_Y_FSR_OF_GAIN_256  = 7.460f;
constexpr float AS73211_Z_FSR_OF_GAIN_256  = 4.006f;
constexpr float AS73211_X_FSR_OF_GAIN_128  = 13.854f;
constexpr float AS73211_Y_FSR_OF_GAIN_128  = 14.919f;
constexpr float AS73211_Z_FSR_OF_GAIN_128  = 8.012f;
constexpr float AS73211_X_FSR_OF_GAIN_64   = 27.707f;
constexpr float AS73211_Y_FSR_OF_GAIN_64   = 29.838f;
constexpr float AS73211_Z_FSR_OF_GAIN_64   = 16.024f;
constexpr float AS73211_X_FSR_OF_GAIN_32   = 55.414f;
constexpr float AS73211_Y_FSR_OF_GAIN_32   = 59.677f;
constexpr float AS73211_Z_FSR_OF_GAIN_32   = 32.048f;
constexpr float AS73211_X_FSR_OF_GAIN_16   = 110.828f;
constexpr float AS73211_Y_FSR_OF_GAIN_16   = 119.354f;
constexpr float AS73211_Z_FSR_OF_GAIN_16   = 64.097f;
constexpr float AS73211_X_FSR_OF_GAIN_8    = 221.657f;
constexpr float AS73211_Y_FSR_OF_GAIN_8    = 238.707f;
constexpr float AS73211_Z_FSR_OF_GAIN_8    = 128.194f;
constexpr float AS73211_X_FSR_OF_GAIN_4    = 443.314f;
constexpr float AS73211_Y_FSR_OF_GAIN_4    = 477.415f;
constexpr float AS73211_Z_FSR_OF_GAIN_4    = 256.387f;
constexpr float AS73211_X_FSR_OF_GAIN_2    = 886.628f;
constexpr float AS73211_Y_FSR_OF_GAIN_2    = 954.830f;
constexpr float AS73211_Z_FSR_OF_GAIN_2    = 512.774f;
constexpr float AS73211_X_FSR_OF_GAIN_1    = 1773.255f;
constexpr float AS73211_Y_FSR_OF_GAIN_1    = 1909.659f;
constexpr float AS73211_Z_FSR_OF_GAIN_1    = 1025.548f;

constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_1ms     = 1024;
constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_2ms     = 2048;
constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_4ms     = 4096;
constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_8ms     = 8192;
constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_16ms    = 16384;
constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_32ms    = 32768;
constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_64ms    = 65536;
constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_128ms   = 131072;
constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_256ms   = 262144;
constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_512ms   = 524288;
constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_1024ms  = 1048576;
constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_2048ms  = 2097152;
constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_4096ms  = 4194304;
constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_8192ms  = 8388608;
constexpr uint32_t AS73211_NUMBER_OF_CLK_TIME_16384ms = 16777216;

constexpr uint8_t as73211_channel_time[15] = {
    AS73211_CREG1_TIME_1ms,
    AS73211_CREG1_TIME_2ms,
    AS73211_CREG1_TIME_4ms,
    AS73211_CREG1_TIME_8ms,
    AS73211_CREG1_TIME_16ms,
    AS73211_CREG1_TIME_32ms,
    AS73211_CREG1_TIME_64ms,
    AS73211_CREG1_TIME_128ms,
    AS73211_CREG1_TIME_256ms,
    AS73211_CREG1_TIME_512ms,
    AS73211_CREG1_TIME_1024ms,
    AS73211_CREG1_TIME_2048ms,
    AS73211_CREG1_TIME_4096ms,
    AS73211_CREG1_TIME_8192ms,
    AS73211_CREG1_TIME_16384ms
};

constexpr uint32_t as73211_number_of_clock[15] = {
    AS73211_NUMBER_OF_CLK_TIME_1ms,
    AS73211_NUMBER_OF_CLK_TIME_2ms,
    AS73211_NUMBER_OF_CLK_TIME_4ms,
    AS73211_NUMBER_OF_CLK_TIME_8ms,
    AS73211_NUMBER_OF_CLK_TIME_16ms,
    AS73211_NUMBER_OF_CLK_TIME_32ms,
    AS73211_NUMBER_OF_CLK_TIME_64ms,
    AS73211_NUMBER_OF_CLK_TIME_128ms,
    AS73211_NUMBER_OF_CLK_TIME_256ms,
    AS73211_NUMBER_OF_CLK_TIME_512ms,
    AS73211_NUMBER_OF_CLK_TIME_1024ms,
    AS73211_NUMBER_OF_CLK_TIME_2048ms,
    AS73211_NUMBER_OF_CLK_TIME_4096ms,
    AS73211_NUMBER_OF_CLK_TIME_8192ms,
    AS73211_NUMBER_OF_CLK_TIME_16384ms
};

constexpr uint8_t as73211_channel_gain[12] = {
    AS73211_CREG1_GAIN_XYZ_2048,
    AS73211_CREG1_GAIN_XYZ_1024,
    AS73211_CREG1_GAIN_XYZ_512,
    AS73211_CREG1_GAIN_XYZ_256,
    AS73211_CREG1_GAIN_XYZ_128,
    AS73211_CREG1_GAIN_XYZ_64,
    AS73211_CREG1_GAIN_XYZ_32,
    AS73211_CREG1_GAIN_XYZ_16,
    AS73211_CREG1_GAIN_XYZ_8,
    AS73211_CREG1_GAIN_XYZ_4,
    AS73211_CREG1_GAIN_XYZ_2,
    AS73211_CREG1_GAIN_XYZ_1
};

constexpr float as73211_X_channel_FSR[12] = {
    AS73211_X_FSR_OF_GAIN_2048,
    AS73211_X_FSR_OF_GAIN_1024,
    AS73211_X_FSR_OF_GAIN_512,
    AS73211_X_FSR_OF_GAIN_256,
    AS73211_X_FSR_OF_GAIN_128,
    AS73211_X_FSR_OF_GAIN_64,
    AS73211_X_FSR_OF_GAIN_32,
    AS73211_X_FSR_OF_GAIN_16,
    AS73211_X_FSR_OF_GAIN_8,
    AS73211_X_FSR_OF_GAIN_4,
    AS73211_X_FSR_OF_GAIN_2,
    AS73211_X_FSR_OF_GAIN_1
};

constexpr float as73211_Y_channel_FSR[12] = {
    AS73211_Y_FSR_OF_GAIN_2048,
    AS73211_Y_FSR_OF_GAIN_1024,
    AS73211_Y_FSR_OF_GAIN_512,
    AS73211_Y_FSR_OF_GAIN_256,
    AS73211_Y_FSR_OF_GAIN_128,
    AS73211_Y_FSR_OF_GAIN_64,
    AS73211_Y_FSR_OF_GAIN_32,
    AS73211_Y_FSR_OF_GAIN_16,
    AS73211_Y_FSR_OF_GAIN_8,
    AS73211_Y_FSR_OF_GAIN_4,
    AS73211_Y_FSR_OF_GAIN_2,
    AS73211_Y_FSR_OF_GAIN_1
};

constexpr float as73211_Z_channel_FSR[12] = {
    AS73211_Z_FSR_OF_GAIN_2048,
    AS73211_Z_FSR_OF_GAIN_1024,
    AS73211_Z_FSR_OF_GAIN_512,
    AS73211_Z_FSR_OF_GAIN_256,
    AS73211_Z_FSR_OF_GAIN_128,
    AS73211_Z_FSR_OF_GAIN_64,
    AS73211_Z_FSR_OF_GAIN_32,
    AS73211_Z_FSR_OF_GAIN_16,
    AS73211_Z_FSR_OF_GAIN_8,
    AS73211_Z_FSR_OF_GAIN_4,
    AS73211_Z_FSR_OF_GAIN_2,
    AS73211_Z_FSR_OF_GAIN_1
};

AS73211 &AS73211::instance() {
	static bool init = false;
	static AS73211 as73211;
	if (!init) {
		init = true;
		as73211.init();
	}
	return as73211;
}

void AS73211::init() {

	const uint8_t A1 = 0;
	const uint8_t A0 = 0;
	_slaveAddress = 0b1110100 | (A1<<1) | A0;

	softwareReset();
	setGainAndTime(Gain1, Time256ms);
	setConfiguration(
		true,
		Divider1,
		ModeContinous,
		false,
		ModePushPull,
		Clock1024MHz);
	writeByte(AS73211_REG_BREAK_TIME, 0x52);
	setState(Measurement_state_Start_measurement);
}

void AS73211::writeByte(uint8_t reg, uint8_t _data) {
    uint8_t writeReg[2];

    writeReg[0] = reg;
    writeReg[1] = _data;
    
    i2cStart();
    i2cWrite(_slaveAddress, writeReg, 2);
    
    if (reg == AS73211_REG_CONFIGURATION_REGISTER_1) {
        _setGain = (_data & 0xF0);
        _setTime = (_data & 0x0F);
    }
}

uint8_t AS73211::readByte(uint8_t reg) {
    uint8_t writeReg[1];
    uint8_t readReg[1] = {0};
   
    writeReg[0] = reg;
    i2cStart();
    i2cWrite(_slaveAddress, writeReg, 1);
    i2cRead(_slaveAddress, readReg, 1);

    return readReg[ 0 ];
}

uint16_t AS73211::readData(uint8_t reg) {
    uint8_t writeReg[1];
    uint8_t readReg[2] = {0, 0};
    uint16_t readData = 0;

    writeReg[0] = reg;

    i2cStart();
    i2cWrite(_slaveAddress, writeReg, 1);
    i2cRead(_slaveAddress, readReg, 2);

    readData = readReg[1];
    readData = readData << 8;
    readData = readData | readReg[0];
    
    return readData;
}

bool AS73211::setState(OperationalState state){
	writeByte(AS73211_REG_OPERATIONAL_STATE,(uint8_t)state);
	return getState()==state;
}

AS73211::OperationalState AS73211::getState(){
	return (OperationalState)readByte(AS73211_REG_OPERATIONAL_STATE);
}

uint8_t AS73211::getDeviceID(){
	return readByte(AS73211_REG_API_GENERATION);
}

bool AS73211::setGainAndTime(IntegrationGain gain,IntegrationTime time){
	uint8_t value=((uint8_t)gain)|((uint8_t)time);
	writeByte(AS73211_REG_CONFIGURATION_REGISTER_1,value);
	return getGainAndTime()==value;
}

bool AS73211::setGain(IntegrationGain gain){
	uint8_t value=((uint8_t)gain)|((uint8_t)getTime());
	writeByte(AS73211_REG_CONFIGURATION_REGISTER_1,value);
	return getGainAndTime()==value;
}

bool AS73211::setTime(IntegrationTime time){
	uint8_t value=((uint8_t)getGain())|((uint8_t)time);
	writeByte(AS73211_REG_CONFIGURATION_REGISTER_1,value);
	return getGainAndTime()==value;
}

uint8_t AS73211::getGainAndTime(){
	return readByte(AS73211_REG_CONFIGURATION_REGISTER_1);
}

AS73211::IntegrationGain AS73211::getGain(){
	return (IntegrationGain)(getGainAndTime()&0xF0);
}

uint16_t AS73211::getGainValue(){
	switch(getGain()){
		case Gain2048 : return 2048;
		case Gain1024 : return 1024;
		case Gain512  : return 512;
		case Gain256  : return 256;
		case Gain128  : return 128;
		case Gain64   : return 64;
		case Gain32   : return 32;
		case Gain16   : return 16;
		case Gain8    : return 8;
		case Gain4    : return 4;
		case Gain2    : return 2;
		case Gain1    : return 1;
		default: return 0;
	}
}

AS73211::IntegrationTime AS73211::getTime(){
	return (IntegrationTime)(getGainAndTime()&0x0F);
}

uint16_t AS73211::getTimeValue(){
	switch(getTime()){
		case Time1ms     : return 1;
		case Time2ms     : return 2;
		case Time4ms     : return 4;
		case Time8ms     : return 8;
		case Time16ms    : return 16;
		case Time32ms    : return 32;
		case Time64ms    : return 64;
		case Time128ms   : return 128;
		case Time256ms   : return 256;
		case Time512ms   : return 512;
		case Time1024ms  : return 1024;
		case Time2048ms  : return 2048;
		case Time4096ms  : return 4096;
		case Time8192ms  : return 8192;
		case Time16384ms : return 16384;
		default: return 0;
	}
}

uint16_t AS73211::getTimeValueBuffer(){
	switch((IntegrationTime)_setTime){
		case Time1ms     : return 1;
		case Time2ms     : return 2;
		case Time4ms     : return 4;
		case Time8ms     : return 8;
		case Time16ms    : return 16;
		case Time32ms    : return 32;
		case Time64ms    : return 64;
		case Time128ms   : return 128;
		case Time256ms   : return 256;
		case Time512ms   : return 512;
		case Time1024ms  : return 1024;
		case Time2048ms  : return 2048;
		case Time4096ms  : return 4096;
		case Time8192ms  : return 8192;
		case Time16384ms : return 16384;
		default: return 0;
	}
}

bool AS73211::setConfiguration(
		bool measureTemperature,
		MeasurementDivider divider,
		MeasurementMode mmode,
		bool standby,
		ReadyPinMode readyMode,
		InternalClockFrequency freq){

	uint8_t conf2 = (measureTemperature?0x40:0x00)|(uint8_t)divider;
	uint8_t conf3 = (uint8_t)mmode|(standby?0x10:0x00)|(uint8_t)readyMode|(uint8_t)freq;
	writeByte(AS73211_REG_CONFIGURATION_REGISTER_2, conf2);
	writeByte(AS73211_REG_CONFIGURATION_REGISTER_3, conf3);
	return  (0b01001111 & readByte(AS73211_REG_CONFIGURATION_REGISTER_2)) == conf2 &&
			(0b11011011 & readByte(AS73211_REG_CONFIGURATION_REGISTER_3)) == conf3;
			
}

AS73211::MeasurementDivider AS73211::getDivider(){
	uint8_t value=readByte(AS73211_REG_CONFIGURATION_REGISTER_2)&0x0F;
	if(value<0x8) {
		value=0;
	}
	return (MeasurementDivider)value;
}

bool AS73211::setDivider(MeasurementDivider divider){
	uint8_t value=(readByte(AS73211_REG_CONFIGURATION_REGISTER_2)&0xF0)|((uint8_t)divider);
	writeByte(AS73211_REG_CONFIGURATION_REGISTER_2,value);
	return readByte(AS73211_REG_CONFIGURATION_REGISTER_2)==value;
}

uint16_t AS73211::getDividerValue(){
	switch (getDivider()){
		case Divider1   : return 1;
		case Divider2   : return 2;
		case Divider4   : return 4;
		case Divider8   : return 8;
		case Divider16  : return 16;
		case Divider32  : return 32;
		case Divider64  : return 64;
		case Divider128 : return 128;
		case Divider256 : return 256;
		default: return 0;
	}
}

AS73211::InternalClockFrequency AS73211::getClock(){
	uint8_t value=readByte(AS73211_REG_CONFIGURATION_REGISTER_3)&0x03;
	return (InternalClockFrequency)value;
}

uint16_t AS73211::getClockValue(){
	switch(getClock()){
		case Clock1024MHz : return 1024;
		case Clock2048MHz : return 2048;
		case Clock4096MHz : return 4096;
		case Clock8192MHz : return 8192;
		default: return 0;
	}
}

bool AS73211::setClock(InternalClockFrequency freq){
	uint8_t value=(readByte(AS73211_REG_CONFIGURATION_REGISTER_3)&0xFC)|((uint8_t)freq);
	writeByte(AS73211_REG_CONFIGURATION_REGISTER_3,value);
	return readByte(AS73211_REG_CONFIGURATION_REGISTER_3)==value;
}

bool AS73211::newDataAvailable(){
	return readData(AS73211_MREG_STATUS_REGISTER)&AS73211_STATUS_NDATA;
}

float AS73211::getTemperature() {
    uint16_t channelData = AS73211::readData(AS73211_MREG_TEMPERATURE_MEASUREMENT) & 0x0FFF;
    return (channelData * 0.05f) - 66.9f;
}

void AS73211::softwareReset() {
    AS73211::writeByte(AS73211_REG_OPERATIONAL_STATE, 0x4A);
}

void AS73211::goToMeasurementMode() {
    delay(1000);
    AS73211::writeByte(AS73211_REG_OPERATIONAL_STATE, 0x83);
    delay(1000);
}

float AS73211::getX() {
	return convertingToEe(AS73211_MREG_MEASUREMENT_X_CHANNEL, readData(AS73211_MREG_MEASUREMENT_X_CHANNEL));
}

float AS73211::getY() {
	return convertingToEe(AS73211_MREG_MEASUREMENT_Y_CHANNEL, readData(AS73211_MREG_MEASUREMENT_Y_CHANNEL));
}

float AS73211::getZ() {
	return convertingToEe(AS73211_MREG_MEASUREMENT_Z_CHANNEL, readData(AS73211_MREG_MEASUREMENT_Z_CHANNEL));
}

float AS73211::convertingToEe(uint8_t channel, uint16_t MRES_data) {
	float FSR = 0.0f;
	float numOfClk = 0.0f;
    for (uint8_t cnt = 0; cnt < 12; cnt++) {
        if (as73211_channel_gain[ cnt ] == _setGain) {
             if (channel == AS73211_MREG_MEASUREMENT_X_CHANNEL) {
                 FSR = as73211_X_channel_FSR[ cnt ];
             }
             else if (channel == AS73211_MREG_MEASUREMENT_Y_CHANNEL) {
                 FSR = as73211_Y_channel_FSR[ cnt ];
             }
             else if (channel == AS73211_MREG_MEASUREMENT_Z_CHANNEL) {
                 FSR = as73211_Z_channel_FSR[ cnt ];
             }
        }
        if (as73211_channel_time[ cnt ] == _setTime) {
             numOfClk = float(as73211_number_of_clock[ cnt ]);
        }
    }
    return (FSR / numOfClk) * (float)MRES_data;
}

void AS73211::delay(int32_t ms) {
	(void)ms;
}

int AS73211::i2cStart() {
	return 0;
}

int AS73211::i2cWrite(uint8_t slaveAddr, uint8_t *pBuff, uint16_t nBytes) {
	(void)slaveAddr;
	(void)pBuff;
	(void)nBytes;
	return 0;
}

int AS73211::i2cRead(uint8_t slaveAddr, uint8_t *pBuff, uint16_t nBytes) {
	(void)slaveAddr;
	(void)pBuff;
	(void)nBytes;
	return 0;
}

} /// namespace lightkraken {

