//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "Pin.h"
#include <vector>
#include <string>

class Bus {
public:
	class Circuit* circuit;
	//std::vector<Pin> pins;
	std::vector<Index> pins;

	void create(Circuit* circuit, int size);
	void createInput(Circuit* circuit, int size);
	int size();
	void addPin(Pin pin);
	Pin getPin(int index);

	Bus BUF();
	Bus AND(Pin rhs);
	Bus connect(Bus rhs);
	Bus split(int index, int parts);

	void setValue(uint64_t value);
	uint64_t getValue();
	std::string getStrValue();
};
