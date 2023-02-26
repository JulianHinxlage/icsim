//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "Circuit.h"
#include "DigitalCircuitSimulator.h"
#include <deque>

void SocketState::set(int socketIndex, float voltage) {
	inboundVoltages[socketIndex] = voltage;
	float max = 0;
	for (auto& i : inboundVoltages) {
		max = std::max(max, i.second);
	}
	this->voltage = max;
}

float SocketState::get() {
	return voltage;
}

void SocketState::reset() {
	inboundVoltages.clear();
	voltage = 0;
	current = 0;
}

void Circuit::setInput(int index, float value) {
	auto& e = structure.elements[structure.inputElements[index]];
	int sIndex = e.socketIndises[(int)SocketSlot::PIN];
	socketStates[sIndex].set(sIndex, value);
}

float Circuit::getOutput(int index) {
	auto& e = structure.elements[structure.outputElements[index]];
	int sIndex = e.socketIndises[(int)SocketSlot::PIN];
	return socketStates[sIndex].get();
}

float Circuit::getInput(int index) {
	auto& e = structure.elements[structure.inputElements[index]];
	int sIndex = e.socketIndises[(int)SocketSlot::PIN];
	return socketStates[sIndex].get();
}

int Circuit::getInputCount() {
	return structure.inputElements.size();
}

int Circuit::getOutputCount() {
	return structure.outputElements.size();
}

void Circuit::reset() {
	for (auto& s : socketStates) {
		s.reset();
	}
	for (auto& element : structure.elements) {
		if (element.elementType == ElementType::PIN) {
			if (element.pin.pinType == PinType::CONSTANT) {
				int socketIndex = element.socketIndises[(int)SocketSlot::PIN];
				socketStates[socketIndex].set(socketIndex, element.pin.voltage);
			}
		}
	}
}

void Circuit::prepare(bool prepareSimulation) {
	//init socket state
	socketStates.clear();
	socketStates.resize(structure.sockets.size());

	//init socket connections
	socketConnections.clear();
	socketConnections.resize(structure.sockets.size());
	for (int i = 0; i < structure.connections.size(); i++) {
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
