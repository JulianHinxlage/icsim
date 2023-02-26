//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "Circuit.h"

class DigitalCircuitSimulator {
public:
	Circuit* circuit;

	DigitalCircuitSimulator(Circuit *circuit = nullptr);

	//simulates the circuit with set inputs
	void simulate();

	//settle the circuit into an idle stable state
	void prepare();

private:
	std::deque<int> elementUpdateQueue;
	std::set<int> elementUpdateSet;

	void processQueue();
	void propergateSignal(int sourceSocketIndex, std::set<int>& visitedSocketIndices, int socketIndex, float sourceVoltage);
	void propergateSignal(int socketIndex);
};
