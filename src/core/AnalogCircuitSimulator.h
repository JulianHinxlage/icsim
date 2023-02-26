//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "Circuit.h"

class AnalogCircuitSimulator {
public:
	Circuit* circuit;

	AnalogCircuitSimulator(Circuit* circuit = nullptr);

	//simulates the circuit with set inputs
	void simulate();
};
