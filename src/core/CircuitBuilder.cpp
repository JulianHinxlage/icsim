//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "CircuitBuilder.h"
#include "CircuitStructure.h"
#include <algorithm>

CircuitBuilder::CircuitBuilder(CircuitStructure* circuitStructure, Index socketIndex) {
	structure = circuitStructure;
	this->socketIndex = socketIndex;
}

CircuitBuilder CircuitBuilder::input(const std::string& name) const {
	CircuitBuilder builder = *this;
	builder.pin(PinType::IN, 0);
	builder.structure->inputElements.push_back(NamedPin(builder.structure->elements.size() - 1, name));
	return builder;
}

CircuitBuilder CircuitBuilder::output(const std::string& name) const {
	CircuitBuilder builder = *this;
	builder.pin(PinType::OUT, 0);
	builder.structure->outputElements.push_back(NamedPin(builder.structure->elements.size() - 1, name));
	return builder;
}

CircuitBuilder CircuitBuilder::constant(float value) const {
	CircuitBuilder builder = *this;
	builder.pin(PinType::CONSTANT, value);
	return builder;
}

CircuitBuilder CircuitBuilder::AND(const CircuitBuilder& other) const {
	CircuitBuilder builder = *this;
	builder.gate(GateType::AND, other);
	return builder;
}

CircuitBuilder CircuitBuilder::OR(const CircuitBuilder& other) const {
	CircuitBuilder builder = *this;
	builder.gate(GateType::OR, other);
	return builder;
}

CircuitBuilder CircuitBuilder::NOT() const {
	CircuitBuilder builder = *this;
	builder.gate(GateType::NOT, *this);
	return builder;
}

CircuitBuilder CircuitBuilder::NAND(const CircuitBuilder& other) const {
	CircuitBuilder builder = *this;
	builder.gate(GateType::NAND, other);
	return builder;
}

CircuitBuilder CircuitBuilder::NOR(const CircuitBuilder& other) const {
	CircuitBuilder builder = *this;
	builder.gate(GateType::NOR, other);
	return builder;
}

CircuitBuilder CircuitBuilder::XOR(const CircuitBuilder& other) const {
	CircuitBuilder builder = *this;
	builder.gate(GateType::XOR, other);
	return builder;
}

CircuitBuilder CircuitBuilder::resistor(float resistance) const {
	CircuitBuilder builder = *this;

	Element e(ElementType::RESISTOR);
	e.resistor.resistance = resistance;
	Index elementIndex = builder.structure->addElement(e);

	Index aSocketIndex = builder.structure->getSocketIndex(elementIndex, SocketSlot::RESISTOR_A);
	Index bSocketIndex = builder.structure->getSocketIndex(elementIndex, SocketSlot::RESISTOR_B);

	builder.structure->addConnection(builder.socketIndex, aSocketIndex);
	builder.socketIndex = bSocketIndex;

	return builder;
}

CircuitBuilder CircuitBuilder::transistor(const CircuitBuilder& collector) const {
	CircuitBuilder builder = *this;

	Element e(ElementType::TRANSISTOR);
	e.transistor.transistorType = TransistorType::NONE;
	Index elementIndex = builder.structure->addElement(e);

	Index collectorSocketIndex = builder.structure->getSocketIndex(elementIndex, SocketSlot::COLLECTOR);
	Index baseSocketIndex = builder.structure->getSocketIndex(elementIndex, SocketSlot::BASE);
	Index emitterSocketIndex = builder.structure->getSocketIndex(elementIndex, SocketSlot::EMITTER);

	builder.structure->addConnection(builder.socketIndex, baseSocketIndex);
	builder.structure->addConnection(collector.socketIndex, collectorSocketIndex);
	builder.socketIndex = emitterSocketIndex;

	return builder;
}

CircuitBuilder CircuitBuilder::connect(const CircuitBuilder& other) const {
	CircuitBuilder builder = *this;
	builder.structure->addConnection(builder.socketIndex, other.socketIndex);
	return builder;
}

CircuitBuilder CircuitBuilder::connector() const {
	CircuitBuilder builder = *this;
	builder.pin(PinType::CONNECTOR, 0);
	return builder;
}

CircuitBuilder CircuitBuilder::unconnect() {
	CircuitBuilder builder = *this;
	builder.socketIndex = -1;
	return builder;
}

Index CircuitBuilder::getSocketIndex() {
	return socketIndex;
}

void CircuitBuilder::pin(PinType type, float voltage) {
	Element e(ElementType::PIN);
	e.pin.pinType = type;
	e.pin.voltage = voltage;
	Index elementIndex = structure->addElement(e);
	
	Index outSocketIndex = structure->getSocketIndex(elementIndex, SocketSlot::PIN);

	if (socketIndex == -1) {
		socketIndex = outSocketIndex;
	}
	else {
		structure->addConnection(socketIndex, outSocketIndex);
		socketIndex = outSocketIndex;
	}
}

void CircuitBuilder::gate(GateType type, const CircuitBuilder& other) {
	Element e(ElementType::GATE);
	e.gate.gateType = type;
	Index elementIndex = structure->addElement(e);

	Index aSocketIndex = structure->getSocketIndex(elementIndex, SocketSlot::GATE_A);
	Index bSocketIndex = structure->getSocketIndex(elementIndex, SocketSlot::GATE_B);
	Index outSocketIndex = structure->getSocketIndex(elementIndex, SocketSlot::GATE_OUT);
	
	if (type != GateType::NOT) {
		structure->addConnection(other.socketIndex, bSocketIndex);
	}

	structure->addConnection(socketIndex, aSocketIndex);
	socketIndex = outSocketIndex;
}

void CircuitBuilder::reduceConnectors() {
	std::vector<Index> removedElements;
	std::vector<Index> removedSockets;
	std::vector<Index> removedConnections;

	for (Index i = 0; i < structure->elements.size(); i++) {
		auto& e = structure->elements[i];
		if (e.elementType == ElementType::PIN) {
			//for every connector
			if (e.pin.pinType == PinType::CONNECTOR) {
				Index socketIndex = e.socketIndises[(int)SocketSlot::PIN];
				std::vector<Index> sockets;
				std::vector<Index> connections;

				//get connected sockets and connections
				for (Index j = 0; j < structure->connections.size(); j++) {
					auto& c = structure->connections[j];
					if (c.socket1Index == socketIndex) {
						sockets.push_back(c.socket2Index);
						connections.push_back(j);
					}
					else if (c.socket2Index == socketIndex) {
						sockets.push_back(c.socket1Index);
						connections.push_back(j);
					}
				}

				//add new connections
				for (Index k = 0; k < sockets.size(); k++) {
					for (Index l = k + 1; l < sockets.size(); l++) {
						structure->addConnection(sockets[k], sockets[l]);
					}
				}

				removedElements.push_back(i);
				removedSockets.push_back(socketIndex);
				for (auto& i : connections) {
					removedConnections.push_back(i);
				}
			}
		}
	}

	
	std::sort(removedElements.rbegin(), removedElements.rend());
	std::sort(removedSockets.rbegin(), removedSockets.rend());
	std::sort(removedConnections.rbegin(), removedConnections.rend());

	for (auto& i : removedElements) {
		structure->removeElement(i);
	}

	for (auto& i : removedSockets) {
		structure->removeSocket(i);
	}

	Index last = -1;
	for (auto& i : removedConnections) {
		if (i != last) {
			structure->removeConnection(i);
		}
		last = i;
	}
}
