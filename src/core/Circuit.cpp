//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "Circuit.h"
#include "DigitalCircuitSimulator.h"
#include <deque>

void AnalogSocketState::set(Index socketIndex, float voltage) {
	inboundVoltages[socketIndex] = voltage;
	float max = 0;
	for (auto& i : inboundVoltages) {
		max = std::max(max, i.second);
	}
	this->voltage = max;
}

float AnalogSocketState::get() {
	return voltage;
}

void AnalogSocketState::reset() {
	inboundVoltages.clear();
	voltage = 0;
	current = 0;
}


void DigitalSocketState::set(Index socketIndex, bool value) {
	this->value = value;
}

bool DigitalSocketState::get() {
	return value;
}

void DigitalSocketState::reset() {
	value = 0;
}


void Circuit::setInput(Index index, float value) {
	auto& e = structure.elements[structure.inputElements[index].index];
	Index sIndex = e.socketIndises[(int)SocketSlot::PIN];
	socketStates[sIndex].set(sIndex, value);
	changedSockets.push_back(sIndex);
}

float Circuit::getOutput(Index index) {
	auto& e = structure.elements[structure.outputElements[index].index];
	Index sIndex = e.socketIndises[(int)SocketSlot::PIN];
	return socketStates[sIndex].get();
}

float Circuit::getInput(Index index) {
	auto& e = structure.elements[structure.inputElements[index].index];
	Index sIndex = e.socketIndises[(int)SocketSlot::PIN];
	return socketStates[sIndex].get();
}

void Circuit::setInput(const std::string& name, float value) {
	for (int i = 0; i < structure.inputElements.size(); i++) {
		if (structure.inputElements[i].name == name) {
			setInput(i, value);
			return;
		}
	}
}

float Circuit::getOutput(const std::string& name) {
	for (int i = 0; i < structure.outputElements.size(); i++) {
		if (structure.outputElements[i].name == name) {
			return getOutput(i);
		}
	}
}

float Circuit::getInput(const std::string& name) {
	for (int i = 0; i < structure.inputElements.size(); i++) {
		if (structure.inputElements[i].name == name) {
			return getInput(i);
		}
	}
}

int Circuit::getInputCount() {
	return structure.inputElements.size();
}

int Circuit::getOutputCount() {
	return structure.outputElements.size();
}

float Circuit::getSocketValue(Index socketIndex) {
	return socketStates[socketIndex].get();
}

void Circuit::setSocketValue(Index socketIndex, float value) {
	socketStates[socketIndex].set(socketIndex, value);
	changedSockets.push_back(socketIndex);
}

void Circuit::reset() {
	for (auto& s : socketStates) {
		s.reset();
	}
	for (auto& element : structure.elements) {
		if (element.elementType == ElementType::PIN) {
			if (element.pin.pinType == PinType::CONSTANT) {
				Index socketIndex = element.socketIndises[(int)SocketSlot::PIN];
				socketStates[socketIndex].set(socketIndex, element.pin.voltage);
			}
		}
	}
}

void Circuit::prepare(bool prepareSimulation) {
	//init socket state
	socketStates.clear();
	socketStates.resize(structure.sockets.size());

	//set socket connections
	socketConnections.clear();
	socketConnections.resize(structure.sockets.size());
	for (Index i = 0; i < structure.connections.size(); i++) {
		auto& c = structure.connections[i];
		socketConnections[c.socket1Index].push_back(c.socket2Index);
		socketConnections[c.socket2Index].push_back(c.socket1Index);
	}

	//set initial zero state
	reset();

	if (prepareSimulation) {
		DigitalCircuitSimulator sim(this);
		sim.prepare();
	}
}
