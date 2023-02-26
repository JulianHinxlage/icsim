//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "CircuitBuilder.h"
#include "CircuitStructure.h"

CircuitBuilder::CircuitBuilder(CircuitStructure* circuitStructure) {
	structure = circuitStructure;
	socketIndex = -1;
}

CircuitBuilder CircuitBuilder::input() const {
	CircuitBuilder builder = *this;
	builder.pin(PinType::IN, 0);
	builder.structure->inputElements.push_back(builder.structure->elements.size() - 1);
	return builder;
}

CircuitBuilder CircuitBuilder::output() const {
	CircuitBuilder builder = *this;
	builder.pin(PinType::OUT, 0);
	builder.structure->outputElements.push_back(builder.structure->elements.size() - 1);
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
	int elementIndex = builder.structure->addElement(e);

	int aSocketIndex = builder.structure->getSocketIndex(elementIndex, SocketSlot::RESISTOR_A);
	int bSocketIndex = builder.structure->getSocketIndex(elementIndex, SocketSlot::RESISTOR_B);

	builder.structure->addConnection(builder.socketIndex, aSocketIndex);
	builder.socketIndex = bSocketIndex;

	return builder;
}

CircuitBuilder CircuitBuilder::transistor(const CircuitBuilder& collector) const {
	CircuitBuilder builder = *this;

	Element e(ElementType::TRANSISTOR);
	e.transistor.transistorType = TransistorType::NONE;
	int elementIndex = builder.structure->addElement(e);

	int collectorSocketIndex = builder.structure->getSocketIndex(elementIndex, SocketSlot::COLLECTOR);
	int baseSocketIndex = builder.structure->getSocketIndex(elementIndex, SocketSlot::BASE);
	int emitterSocketIndex = builder.structure->getSocketIndex(elementIndex, SocketSlot::EMITTER);

	builder.structure->addConnection(builder.socketIndex, baseSocketIndex);
	builder.structure->addConnection(collector.socketIndex, collectorSocketIndex);
	builder.socketIndex = emitterSocketIndex;

	return builder;
}

CircuitBuilder CircuitBuilder::connection(const CircuitBuilder& other) const {
	CircuitBuilder builder = *this;
	builder.pin(PinType::CONNECTOR, 0);
	builder.structure->addConnection(builder.socketIndex, other.socketIndex);
	return builder;
}

CircuitBuilder CircuitBuilder::connection() const {
	CircuitBuilder builder = *this;
	builder.pin(PinType::CONNECTOR, 0);
	return builder;
}

CircuitBuilder CircuitBuilder::endConnection(const CircuitBuilder& other) const {
	CircuitBuilder builder = *this;
	builder.structure->addConnection(builder.socketIndex, other.socketIndex);
	return builder.unconnect();
}

CircuitBuilder CircuitBuilder::unconnect() {
	CircuitBuilder builder = *this;
	builder.socketIndex = -1;
	return builder;
}

void CircuitBuilder::pin(PinType type, float voltage) {
	Element e(ElementType::PIN);
	e.pin.pinType = type;
	e.pin.voltage = voltage;
	int elementIndex = structure->addElement(e);
	
	int outSocketIndex = structure->getSocketIndex(elementIndex, SocketSlot::PIN);

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
	int elementIndex = structure->addElement(e);

	int aSocketIndex = structure->getSocketIndex(elementIndex, SocketSlot::GATE_A);
	int bSocketIndex = structure->getSocketIndex(elementIndex, SocketSlot::GATE_B);
	int outSocketIndex = structure->getSocketIndex(elementIndex, SocketSlot::GATE_OUT);
	
	if (type != GateType::NOT) {
		structure->addConnection(other.socketIndex, bSocketIndex);
	}

	structure->addConnection(socketIndex, aSocketIndex);
	socketIndex = outSocketIndex;
}
