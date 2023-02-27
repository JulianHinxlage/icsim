//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "Circuit.h"

class DigitalCircuitSimulator {
public:
	Circuit* circuit;
	bool recursiveSignalPropergation = true;

	DigitalCircuitSimulator(Circuit *circuit = nullptr);

	//simulates the circuit with set inputs
	void simulate();

	//settle the circuit into an idle stable state
	void prepare();

private:
	std::deque<Index> elementUpdateQueue;
	std::set<Index> elementUpdateSet;

	void processQueue();
	void propergateSignal(Index sourceSocketIndex, std::set<Index>& visitedSocketIndices, Index socketIndex, float sourceVoltage);
	void propergateSignal(Index socketIndex);
};
