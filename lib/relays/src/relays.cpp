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

#include "relays.hpp"

//**Relay**************************************************************************************************************************************************************************

relays::Relay::Relay(pin_size_t pin)
	: relay_pin(pin), relay_onState(LOW) {
	pinMode(relay_pin, OUTPUT);
	digitalWrite(relay_pin, !relay_onState);
}

relays::Relay::Relay(pin_size_t pin, bool onState)
	: relay_pin(pin), relay_onState(onState) {
	pinMode(relay_pin, OUTPUT);
	digitalWrite(relay_pin, !relay_onState);
}

relays::Relay::Relay(pin_size_t pin, bool onState, bool state)
	: relay_pin(pin), relay_onState(onState) {
	pinMode(relay_pin, OUTPUT);
	digitalWrite(relay_pin, state);
}

void relays::Relay::setState(bool state) {
	relay_state = state == relay_onState;
	relay_update();
}

void relays::Relay::turnOn() {
	setState(true);
}

void relays::Relay::turnOff() {
	setState(false);
}

bool relays::Relay::getState() {
	return relay_state == relay_onState;
}

void relays::Relay::changeState() {
	setState(!getState());
}

void relays::Relay::relay_update() {
	digitalWrite(relay_pin, relay_state);
}

//**Heater*************************************************************************************************************************************************************************

relays::Heater::Heater(pin_size_t pin, bool onState)
	: Relay(pin, onState), heater_offState(!relay_onState) {
	heater_enabled = true;
}

void relays::Heater::setEnabled(bool enabled) {
	heater_enabled = enabled;
	relay_update();
}

void relays::Heater::enable() {
	setEnabled(true);
}

void relays::Heater::disable() {
	setEnabled(false);
}

bool relays::Heater::getEnabled() {
	return heater_enabled.load();
}

void relays::Heater::relay_update() {
	digitalWrite(relay_pin, heater_enabled ? relay_state.load() : heater_offState);
}
