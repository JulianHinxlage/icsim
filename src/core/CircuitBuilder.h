//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "Element.h"
#include <string>

class CircuitStructure;

class CircuitBuilder {
public:
	CircuitBuilder(CircuitStructure* circuitStructure = nullptr, Index socketIndex = -1);

	CircuitBuilder input(const std::string &name = "") const;
	CircuitBuilder output(const std::string& name = "") const;
	CircuitBuilder constant(float value) const;

	CircuitBuilder AND(const CircuitBuilder& other) const;
	CircuitBuilder OR(const CircuitBuilder& other) const;
	CircuitBuilder NOT() const;
	CircuitBuilder NAND(const CircuitBuilder& other) const;
	CircuitBuilder NOR(const CircuitBuilder& other) const;
	CircuitBuilder XOR(const CircuitBuilder& other) const;

	CircuitBuilder resistor(float resistance) const;
	CircuitBuilder transistor(const CircuitBuilder& collector) const;
	CircuitBuilder connect(const CircuitBuilder& other) const;
	CircuitBuilder connector() const;

	//unconnect the builder form the last socket
	CircuitBuilder unconnect();
	Index getSocketIndex();

	void reduceConnectors();

private:
	CircuitStructure* structure;
	Index socketIndex;

	void pin(PinType type, float voltage);
	void gate(GateType type, const CircuitBuilder& other);
};