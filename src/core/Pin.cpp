//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "Pin.h"
#include "Circuit.h"

Pin::Pin(Circuit* circuit, Index index) {
	this->circuit = circuit;
	this->index = index;
}

Pin Pin::connector() {
	Index out = circuit->addGate(GateType::CONNECTOR);
	return Pin(circuit, out);
}

Pin Pin::input() {
	Index out = circuit->addGate(GateType::OUTPUT);
	return Pin(circuit, out);
}

Pin Pin::connect(Pin rhs) {
	circuit->addLine(index, rhs.index);
	return Pin(circuit, index);
}

Pin Pin::BUF() {
	Index out = circuit->addGate(GateType::BUF);
	circuit->addLine(index, out - 1);
	return Pin(circuit, out);
}

Pin Pin::AND(Pin rhs) {
	Index out = circuit->addGate(GateType::AND);
	circuit->addLine(index, out - 2);
	circuit->addLine(rhs.index, out - 1);
	return Pin(circuit, out);
}

Pin Pin::OR(Pin rhs) {
	Index out = circuit->addGate(GateType::OR);
	circuit->addLine(index, out - 2);
	circuit->addLine(rhs.index, out - 1);
	return Pin(circuit, out);
}

Pin Pin::NOT() {
	Index out = circuit->addGate(GateType::NOT);
	circuit->addLine(index, out - 1);
	return Pin(circuit, out);
}

Pin Pin::NAND(Pin rhs) {
	Index out = circuit->addGate(GateType::NAND);
	circuit->addLine(index, out - 2);
	circuit->addLine(rhs.index, out - 1);
	return Pin(circuit, out);
}

Pin Pin::NOR(Pin rhs) {
	Index out = circuit->addGate(GateType::NOR);
	circuit->addLine(index, out - 2);
	circuit->addLine(rhs.index, out - 1);
	return Pin(circuit, out);
}

Pin Pin::XOR(Pin rhs) {
	Index out = circuit->addGate(GateType::XOR);
	circuit->addLine(index, out - 2);
	circuit->addLine(rhs.index, out - 1);
	return Pin(circuit, out);
}

Pin Pin::dLatch(Pin enable) {
	Index out = circuit->addGate(GateType::D_LATCH);
	circuit->addLine(index, out - 2);
	circuit->addLine(enable.index, out - 1);
	return Pin(circuit, out);
}

Pin Pin::zero() {
	return connector();
}

Pin Pin::one() {
	return connector().NOT();
}

bool Pin::getValue() {
	return circuit->pinStates[index];
}

void Pin::setValue(bool value) {
	if (circuit->pinStates[index] != value) {
		circuit->pinStates[index] = value;
		circuit->changedPins.push_back(index);
	}
}
