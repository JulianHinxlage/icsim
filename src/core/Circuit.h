//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "type.h"
#include "EventQueue.h"
#include "Pin.h"
#include "Bus.h"

#include <vector>
#include <map>

class Circuit {
public:
	Index addGate(GateType type);
	void addLine(Index pinA, Index pinB);

	void prepare();
	int simulate(int timeUnits = -1);

	int getGateCount();
	int getPinCount();
	int getLineCount();
	int64_t getSimulationTime();
	void setGateDelay(GateType type, int delay);
	void setSimulationMode(bool sortQueue);

	Pin pin() {
		return Pin(this);
	}

	Bus bus(int size) {
		Bus bus;
		bus.create(this, size);
		return bus;
	}

private:
	friend class Pin;
	friend class CircuitSimulator;

	//circuit definition
	std::vector<PinType> pins;
	std::vector<bool> pinStates;
	std::vector<Index> changedPins;
	std::vector<std::pair<Index, Index>> lines;
	int gateCount = 0;

	//propergation groups
	std::vector<Index> inboundPin;
	std::map<Index, std::set<Index>> inboundPins;
	std::vector<Index> outboundPin;
	std::map<Index, std::set<Index>> outboundPins;

	std::vector<std::set<Index>> groups;
	std::vector<Index> groupByPin;
	std::vector<bool> groupUpToDate;
	std::vector<bool> groupValues;

	//simulation
	EventQueue queue;
	int64_t simulationTime = 0;
	std::vector<int> gateDelays;

	Index addPin(PinType type);
	void addPinConnection(Index pinA, Index pinB);
	void initPinConnections();
	bool getInboundSignal(Index pin);
	void addPinToQueue(Index pin, int delay = 0, bool external = false);
	void addOutboundPinsToQueue(Index pin);
	int processQueue(int timeUnits = -1);
};
