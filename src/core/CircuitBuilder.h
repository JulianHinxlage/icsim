//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "Element.h"

class CircuitStructure;

class CircuitBuilder {
public:
	CircuitBuilder(CircuitStructure* circuitStructure = nullptr);

	CircuitBuilder input() const;
	CircuitBuilder output() const;
	CircuitBuilder constant(float value) const;

	CircuitBuilder AND(const CircuitBuilder& other) const;
	CircuitBuilder OR(const CircuitBuilder& other) const;
	CircuitBuilder NOT() const;
	CircuitBuilder NAND(const CircuitBuilder& other) const;
	CircuitBuilder NOR(const CircuitBuilder& other) const;
	CircuitBuilder XOR(const CircuitBuilder& other) const;

	CircuitBuilder resistor(float resistance) const;
	CircuitBuilder transistor(const CircuitBuilder& collector) const;
	CircuitBuilder connection(const CircuitBuilder& other) const;
	CircuitBuilder connection() const;
	CircuitBuilder endConnection(const CircuitBuilder& other) const;

	//unconnect the builder form the last socket
	CircuitBuilder unconnect();

private:
	CircuitStructure* structure;
	int socketIndex;

	void pin(PinType type, float voltage);
	void gate(GateType type, const CircuitBuilder& other);
};