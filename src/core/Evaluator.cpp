//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "Evaluator.h"
#include "DigitalCircuitSimulator.h"
#include "AnalogCircuitSimulator.h"
#include <string>

void Evaluator::evaluate(Circuit* circuit) {
	table.entries.clear();

	int inCount = circuit->getInputCount();
	for (int i = 0; i < (1 << inCount); i++) {
		table.entries.emplace_back();
		Table::Entry& e = table.entries.back();
		for (int j = 0; j < inCount; j++) {
			e.inputs.push_back((int)(bool)(i & (1 << j)));
		}
	}

	evaluateTable(circuit);
}

void Evaluator::evaluate(Circuit* circuit, const std::vector<std::vector<float>>& inputs) {
	table.entries.clear();

	for (auto& input : inputs) {
		table.entries.emplace_back();
		Table::Entry& e = table.entries.back();
		e.inputs = input;
	}

	evaluateTable(circuit);
}

void Evaluator::evaluateTable(Circuit* circuit) {
	for (auto& entry : table.entries) {
		for (int i = 0; i < entry.inputs.size(); i++) {
			circuit->setInput(i, entry.inputs[i]);
		}

		if (simulationType == SimulationType::DIGITAL) {
			DigitalCircuitSimulator sim(circuit);
			sim.simulate();
		}
		else if (simulationType == SimulationType::ANALOG) {
			AnalogCircuitSimulator sim(circuit);
			sim.simulate();
		}

		int outCount = circuit->getOutputCount();
		for (int i = 0; i < outCount; i++) {
			float v = circuit->getOutput(i);
			entry.outputs.push_back(v);
		}
	}
}

std::string toString(const std::vector<float> &values) {
	std::string str;
	for (int i = 0; i < values.size(); i++) {
		if (!str.empty()) {
			str += ", ";
		}
		str += std::to_string((int)values[i]);
	}
	return str;
}

void Evaluator::printTable() {
	for (int i = 0; i < table.entries.size(); i++) {
		printf("%s: %s\n", toString(table.entries[i].inputs).c_str(), toString(table.entries[i].outputs).c_str());
	}
}

bool Evaluator::checkTable(const std::vector<std::vector<float>>& outputs) {
	if (table.entries.size() != outputs.size()) {
		return false;
	}

	for (int i = 0; i < table.entries.size(); i++) {
		auto& entry = table.entries[i];
		if (entry.outputs.size() != outputs[i].size()) {
			return false;
		}
		for (auto j = 0; j < entry.outputs.size(); j++) {
			if (entry.outputs[j] != outputs[i][j]) {
				return false;
			}
		}
	}
	return true;
}
