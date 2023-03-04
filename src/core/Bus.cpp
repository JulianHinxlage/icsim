//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "Bus.h"

void Bus::create(Circuit* circuit, int size) {
	this->circuit = circuit;
	for (int i = 0; i < size; i++) {
		addPin(Pin(circuit).connector());
	}
}

void Bus::createInput(Circuit* circuit, int size) {
	this->circuit = circuit;
	for (int i = 0; i < size; i++) {
		addPin(Pin(circuit).input());
	}
}

int Bus::size() {
	return pins.size();
}

void Bus::addPin(Pin pin) {
	pins.push_back(pin.index);
}

Pin Bus::getPin(int index) {
	return Pin(circuit, pins[index]);
}

Bus Bus::BUF() {
	Bus result;
	result.circuit = circuit;
	for (int i = 0; i < pins.size(); i++) {
		result.addPin(getPin(i).BUF());
	}
	return result;
}

Bus Bus::AND(Pin rhs) {
	Bus result;
	result.circuit = circuit;
	for (int i = 0; i < pins.size(); i++) {
		result.addPin(getPin(i).AND(rhs));
	}
	return result;
}

Bus Bus::connect(Bus rhs) {
	Bus result;
	result.circuit = circuit;
	for (int i = 0; i < pins.size(); i++) {
		result.addPin(getPin(i).connect(rhs.getPin(i)));
	}
	return result;
}

Bus Bus::split(int index, int parts) {
	Bus result;
	result.circuit = circuit;

	int begin = index * (pins.size() / parts);
	int end = (index + 1) * (pins.size() / parts);

	for (int i = begin; i < end; i++) {
		result.addPin(getPin(i));
	}
	return result;
}

void Bus::setValue(uint64_t value) {
	for (int i = 0; i < pins.size(); i++) {
		if (value & (1ull << i)) {
			getPin(i).setValue(1);
		}
		else {
			getPin(i).setValue(0);
		}
	}
}

uint64_t Bus::getValue() {
	uint64_t value = 0;
	for (int i = 0; i < pins.size(); i++) {
		if (getPin(i).getValue()) {
			value |= (1ull << i);
		}
		else {
			value &= ~(1ull << i);
		}
	}
	return value;
}

std::string Bus::getStrValue() {
	std::string value;
	for (int i = pins.size() - 1; i >= 0; i--) {
		if (getPin(i).getValue()) {
			value += "1";
		}
		else {
			value += "0";
		}
	}
	return value;
}
