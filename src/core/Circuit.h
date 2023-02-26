//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "Element.h"
#include "CircuitBuilder.h"
#include "CircuitStructure.h"
#include <vector>
#include <deque>
#include <set>

class SocketState {
public:
	void set(int socketIndex, float voltage);
	float get();
	void reset();

	//unit: volt
	float voltage = 0;

	//unit: ampere
	float current = 0;

	//inbound volgates from source socket
	std::map<int, float> inboundVoltages;
};

class Circuit {
public:
	CircuitStructure structure;
	std::vector<SocketState> socketStates;
	std::vector<std::vector<int>> socketConnections;

	CircuitBuilder builder() {
		return CircuitBuilder(&structure);
	}

	void setInput(int index, float value);
	float getOutput(int index);
	float getInput(int index);
	int getInputCount();
	int getOutputCount();
	void reset();
	void prepare(bool prepareSimulation = true);
};
