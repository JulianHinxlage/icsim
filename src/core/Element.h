//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include <memory>
#include <map>

typedef int32_t Index;

enum class ElementType : uint8_t {
	NONE,
	PIN,
	RESISTOR,
	TRANSISTOR,
	GATE,
};

enum class PinType : uint8_t {
	NONE,
	IN,
	OUT,
	CONSTANT,
	CONNECTOR,
};

enum class TransistorType : uint8_t {
	NONE,
	MOSFET,
	BJT,
	JFET,
	IGBT
};

enum class GateType : uint8_t {
	NONE,
	AND,
	OR,
	NOT,
	NAND,
	NOR,
	XOR,
};

enum class SocketSlot : uint8_t {
	PIN = 0,
	
	RESISTOR_A = 0,
	RESISTOR_B = 1,
	
	COLLECTOR = 0,
	BASE = 1,
	EMITTER = 2,

	GATE_A = 0,
	GATE_B = 1,
	GATE_OUT = 2,
};

enum class SocketType : uint8_t {
	NONE = 0,
	IN = 1,
	OUT = 2,
	IN_AND_OUT = 3,
};

struct Pin {
	PinType pinType = PinType::NONE;
	float voltage = 0;
};

struct Resistor {
	//unit: ohm
	float resistance = 100;
};

struct Transistor {
	TransistorType transistorType = TransistorType::NONE;
};

struct Gate {
	GateType gateType = GateType::NONE;
};

struct Element {
	ElementType elementType;
	Index socketIndises[3];

	Element(ElementType elementType = ElementType::NONE) {
		memset(this, 0, sizeof(*this));
		socketIndises[0] = -1;
		socketIndises[1] = -1;
		socketIndises[2] = -1;
		this->elementType = elementType;
		switch (elementType)
		{
		case ElementType::NONE:
			break;
		case ElementType::PIN:
			pin = Pin();
			break;
		case ElementType::RESISTOR:
			resistor = Resistor();
			break;
		case ElementType::TRANSISTOR:
			transistor = Transistor();
			break;
		case ElementType::GATE:
			gate = Gate();
			break;
		default:
			break;
		}
	}

	union {
		Pin pin;
		Resistor resistor;
		Transistor transistor;
		Gate gate;
	};
};

class Socket {
public:
	SocketType socketType = SocketType::NONE;
	SocketSlot socketSlot = SocketSlot::PIN;
	Index elementIndex = -1;
};

class Connection {
public:
	Index socket1Index = -1;
	Index socket2Index = -1;
};
