//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "Circuit.h"

enum class SimulationType {
	DIGITAL,
	ANALOG,
};

class Evaluator {
public:
	class Table {
	public:
		class Entry {
		public:
			std::vector<float> inputs;
			std::vector<float> outputs;
		};
		std::vector<Entry> entries;
	};
	Table table;
	SimulationType simulationType = SimulationType::DIGITAL;

	void evaluate(Circuit *circuit);
	void evaluate(Circuit *circuit, const std::vector<std::vector<float>> &inputs);
	bool checkTable(const std::vector<std::vector<float>>& outputs);
	void printTable();

private:
	void evaluateTable(Circuit* circuit);
};