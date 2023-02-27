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

class AnalogSocketState {
public:
	void set(Index socketIndex, float voltage);
	float get();
	void reset();

	//unit: volt
	float voltage = 0;

	//unit: ampere
	float current = 0;

	//inbound volgates from source socket
	std::map<Index, float> inboundVoltages;
};

class DigitalSocketState {
public:
	void set(Index socketIndex, bool value);
	bool get();
	void reset();

	bool value = 0;
};
 
class Circuit {
public:
	CircuitStructure structure;
	std::vector<AnalogSocketState> socketStates;
	std::vector<std::vector<Index>> socketConnections;
	std::vector<Index> changedSockets;

	CircuitBuilder builder() {
		return CircuitBuilder(&structure);
	}

	void setInput(Index index, float value);
	float getOutput(Index index);
	float getInput(Index index);
	void setInput(const std::string& name, float value);
	float getOutput(const std::string& name);
	float getInput(const std::string &name);
	int getInputCount();
	int getOutputCount();
	float getSocketValue(Index socketIndex);
	void setSocketValue(Index socketIndex, float value);

	void reset();
	void prepare(bool prepareSimulation = true);
};
