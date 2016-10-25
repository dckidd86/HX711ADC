#include "application.h"
#include "HX711ADC/HX711ADC.h"

HX711ADC::HX711ADC(uint8_t dout, uint8_t pd_sck, uint8_t gain) {
	PD_SCK 	= pd_sck;
	DOUT 	= dout;
	
	pinMode(PD_SCK, OUTPUT);
	pinMode(DOUT, INPUT);

	set_gain(gain);
}

HX711ADC::~HX711ADC() {

}

bool HX711ADC::is_ready() {
	return digitalRead(DOUT) == LOW;
}

void HX711ADC::set_gain(uint8_t gain) {
	switch (gain) {
		case 128:		// channel A, gain factor 128
			GAIN = 1;
			break;
		case 64:		// channel A, gain factor 64
			GAIN = 3;
			break;
		case 32:		// channel B, gain factor 32
			GAIN = 2;
			break;
	}

	digitalWrite(PD_SCK, LOW);
	read();
}

long HX711ADC::read() {
	// wait for the chip to become ready
	while (!is_ready());

	int data[3];

	// pulse the clock pin 24 times to read the data
	for (int j = 3; j--;) {
		for (int i = 8; i--;) {
			digitalWrite(PD_SCK, HIGH);
			bitWrite(data[j], i, digitalRead(DOUT));
			digitalWrite(PD_SCK, LOW);
		}
	}

	// set the channel and the gain factor for the next reading using the clock pin
	for (int i = 0; i < GAIN; i++) {
		digitalWrite(PD_SCK, HIGH);
		digitalWrite(PD_SCK, LOW);
	}

	data[2] ^= 0x80;

	return ((uint32_t) data[2] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[0];
}

long HX711ADC::read_average(uint8_t times) {
	long sum = 0;
	for (int i = 0; i < times; i++) {
		sum += read();
	}
	return sum / times;
}

double HX711ADC::get_value(uint8_t times) {
	return read_average(times) - OFFSET;
}

float HX711ADC::get_units(uint8_t times) {
	return get_value(times) / SCALE;
}

void HX711ADC::tare(uint8_t times) {
	double sum = read_average(times);
	set_offset(sum);
}

void HX711ADC::set_scale(float scale) {
	SCALE = scale;
}

void HX711ADC::set_offset(long offset) {
	OFFSET = offset;
}

void HX711ADC::power_down() {
	digitalWrite(PD_SCK, LOW);
	digitalWrite(PD_SCK, HIGH);	
}

void HX711ADC::power_up() {
	digitalWrite(PD_SCK, LOW);	
}
