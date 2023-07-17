#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>


#define TRIG_PIN PD0
#define ECHO_PIN PD1
#define TX_PIN PD2
#define RX_PIN PD3
#define LCD_RS PD4
#define LCD_E PD5
#define LCD_DATA_PORT PORTB

volatile unsigned long pulseStart = 0;

void lcdCommand(uint8_t cmd);

unsigned long micros() {
	unsigned long us = 0;
	us = (TCNT1 * 4) / (F_CPU / 1000000UL);
	return us;
}

void setup() {
	// Initialize HC-SR04 pins
	DDRD |= (1 << TRIG_PIN);
	DDRD &= ~(1 << ECHO_PIN);

	// Initialize 433MHz transmitter pin
	DDRD |= (1 << TX_PIN);

	// Initialize receiver pin
	DDRD &= ~(1 << RX_PIN);

	// Initialize LCD pins as output
	DDRD |= (1 << LCD_RS) | (1 << LCD_E);
	DDRB |= 0xFF; // Assuming D4-D7 are connected to PB0-PB3

	// Initialize LCD
	_delay_ms(50);
	lcdCommand(0x38); // Initialize 8-bit mode
	lcdCommand(0x0C); // Display on, cursor off
	lcdCommand(0x01); // Clear display
	_delay_ms(2);

	// Configure Timer/Counter1
	TCCR1B |= (1 << ICES1);    // Set input capture on rising edge
	TIMSK |= (1 << TICIE1);    // Enable input capture interrupt
	TCCR1B |= (1 << CS10);     // Set prescaler to 1
}

void lcdCommand(uint8_t cmd) {
	LCD_DATA_PORT = cmd; // Send command to data port
	PORTD &= ~(1 << LCD_RS); // RS = 0 for command mode
	PORTD |= (1 << LCD_E); // Enable high
	_delay_us(1);
	PORTD &= ~(1 << LCD_E); // Enable low
	_delay_us(50); // Wait for command to complete
}

void lcdWriteChar(uint8_t data) {
	LCD_DATA_PORT = data; // Send data to data port
	PORTD |= (1 << LCD_RS); // RS = 1 for data mode
	PORTD |= (1 << LCD_E); // Enable high
	_delay_us(1);
	PORTD &= ~(1 << LCD_E); // Enable low
	_delay_us(50); // Wait for data to be written
}

void lcdWriteString(const char* str) {
	while (*str != '\0') {
		lcdWriteChar(*str++);
	}
}

unsigned long pulseIn(uint8_t pin, uint8_t state) {
	unsigned long pulseStart = micros();
	while ((PIND & (1 << pin)) != state) {
		if (micros() - pulseStart > 30000) { // Timeout after 30ms
			return 0;
		}
	}
	pulseStart = micros();
	while ((PIND & (1 << pin)) == state) {
		if (micros() - pulseStart > 30000) { // Timeout after 30ms
			return 0;
		}
	}
	return micros() - pulseStart;
}

float receiveDistance() {
	uint16_t distanceInt = 0;

	// Receive the distance wirelessly
	for (int i = 0; i < 16; i++) {
		if (PIND & (1 << RX_PIN)) {
			distanceInt |= (1 << i);
		}
		_delay_us(200); // Adjust the delay as per your requirements
	}

	float distance = (float)distanceInt / 100.0; // Divide by 100 to restore two decimal places

	return distance;
}

void transmitDistance(float distance) {
	// Convert the distance to a binary representation
	uint16_t distanceInt = (uint16_t)(distance * 100); // Multiply by 100 to preserve two decimal places

	// Transmit the distance wirelessly
	for (int i = 0; i < 16; i++) {
		if (distanceInt & (1 << i)) {
			PORTD |= (1 << TX_PIN); // Set the TX_PIN HIGH
			} else {
			PORTD &= ~(1 << TX_PIN); // Set the TX_PIN LOW
		}
		_delay_us(200); // Adjust the delay as per your requirements
	}
}

void displayDistance(float distance) {
	lcdCommand(0x80); // Set cursor to the beginning of the first line
	lcdWriteString("Water Level:");
	lcdCommand(0xC0); // Set cursor to the beginning of the second line
	lcdWriteString("Level: ");
	char buffer[16];
	snprintf(buffer, sizeof(buffer), "%.2f cm", distance);
	lcdWriteString(buffer);
}

void loop() {
	// Trigger HC-SR04 to start distance measurement
	PORTD |= (1 << TRIG_PIN);
	_delay_us(10);
	PORTD &= ~(1 << TRIG_PIN);

	// Wait for the pulseStart to be updated by the interrupt handler
	while (pulseStart == 0);

	unsigned long pulseDuration = TCNT1 - pulseStart;
	float distance = pulseDuration * 0.0343 / 2;

	// Transmit the distance wirelessly
	transmitDistance(distance);

	pulseStart = 0; // Reset pulseStart for the next measurement

	float receivedDistance = receiveDistance();
	displayDistance(receivedDistance);

	_delay_ms(1000); // Delay between measurements
}

int main() {
	setup();

	sei(); // Enable global interrupts

	while (1) {
		loop();
	}

	return 0;
}

ISR(TIMER1_CAPT_vect) {
	if (PIND & (1 << ECHO_PIN)) {
		pulseStart = TCNT1; // Save the current Timer/Counter1 value on rising edge
	}
}