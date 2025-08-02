/*
 * Copyright (c) 2024-2025 Dalen Hardy
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include "lights.hpp"

lights::Light::Light(uint8_t pin, uint8_t speed, bool onState, bool state, std::atomic<bool> *PSUVar)
	: light_pin(pin), light_speed(speed), light_onState(onState), light_state(state), light_PSUVar(PSUVar) {
	pinMode(light_pin, OUTPUT);
	digitalWrite(light_pin, light_state);
	light_time = millis();
}

uint8_t lights::Light::getPin() {
	return light_pin;
}

bool lights::Light::needsPower() {
	if (light_onState) {
		return (light_state || light_changing);

	} else {
		return (!light_state || light_changing);
	}
}

void lights::Light::setSpeed(uint8_t speed) {
  	light_speed = speed;
}

uint8_t lights::Light::getSpeed() {
  	return light_speed;
}

void lights::Light::setState(bool state) {
	//  this if statemet is to reverse the set state if the lights are on when LOW, as the internal state is equal to HIGH or LOW not on and off
	if (light_onState) {
		if (state != light_state) {
			light_toChange = true;
			light_changing = true;
		}

		light_state = state;

	} else {
		if (state == light_state) {
			light_toChange = true;
			light_changing = true;
		}

		light_state = !state;
	}
}

void lights::Light::changeState() {
	light_state = !light_state;
	light_toChange = true;
	light_changing = true;
}

bool lights::Light::getState() {
	//  this if statement flips the returned state if the lights are on when LOW, as the internal state is equal to HIGH or LOW not on and off
	if (light_onState) {
		return light_state;

	} else {
		return !light_state;
	}
}

void lights::Light::tick() {
	if (!light_changing) { // If we are not in the process of changing the light state
		return; // return without doing anything
	}

	if (light_toChange) { // initialization for if we haven't started changing yet
		light_toChange = false;

		if (light_state) { // if we are going to HIGH
			light_i = 0; // Start from fully LOW

		} else { // if we are going to LOW
			light_i = 255; // Start from fully HIGH
		}

		while (!light_PSUVar->load()); // wait for the PSU to be on

		light_time = millis(); // reset the last updtate time so that the lights don't turn on all at once
	}

	uint32_t currentTime = millis();
	uint32_t elapsedTime = currentTime - light_time;

	if (!(elapsedTime >= light_speed)) { // if it's not time to update the light brightness
		return; // return without doing anything
	}

	light_time = currentTime; // Update the last update time

	if (light_state) { // Turning the light HIGH
		// Calculate the increment to apply this tick
		uint8_t increment = min(uint8_t(elapsedTime / light_speed), uint8_t(255 - light_i));
		light_i += increment;
		analogWrite(light_pin, light_i);

		if (light_i >= 255) { // If we've reached fully HIGH
			light_i = 255; // Clamp to max value
			light_changing = false; // Stop changing the light
		}

	} else { // Turning the light to LOW
		// Calculate the decrement to apply this tick
		uint8_t decrement = min(uint8_t(elapsedTime / light_speed), light_i);
		light_i -= decrement;
		analogWrite(light_pin, light_i);

		if (light_i <= 0) { // If we've reached fully LOW
			light_i = 0; // Clamp to min value
			light_changing = false; // Stop changing the light

		} // if (light_i <= 0)
	} // else (  if (light_state)  )
} // lights::Light::tick()

void lights::Light::blip(uint32_t microseconds) {
	analogWrite(light_pin, (light_state * 255) ^ 4); // set the lights to 4/255 of set to LOW, 251/255 if set to HIGH
	delayMicroseconds(microseconds);
	analogWrite(light_pin, (light_state * 255));
}
