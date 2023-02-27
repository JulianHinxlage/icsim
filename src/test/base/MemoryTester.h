//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "core/Circuit.h"
#include "core/DigitalCircuitSimulator.h"
#include "MemoryBank.h"
#include <string>

class MemoryTester {
public:
	Circuit circuit;
	DigitalCircuitSimulator simulator;
	MemoryBank bank;

	int dataBusSize = 8;
	int addressBusSize = 4;
	int wordCount = 16;

	int dataBitOffset;
	int addressBitOffset;

	void build() {
		bank.circuit = &circuit;
		bank.addressBusSize = addressBusSize;
		bank.dataBusSize = dataBusSize;
		bank.wordCount = wordCount;

		bank.buildBase();
		bank.buildCells_v1();

		auto builder = circuit.builder();
		auto clock = builder.input("clock");
		auto write = builder.input("write");
		auto read = builder.input("read");

		clock.connect(bank.clock);
		read.connect(bank.read);
		write.connect(bank.write);


		dataBitOffset = circuit.getInputCount();
		for (int i = 0; i < dataBusSize; i++) {
			builder.input().connect(bank.dataBus.getPin(i));
		}

		addressBitOffset = circuit.getInputCount();
		for (int i = 0; i < addressBusSize; i++) {
			builder.input().connect(bank.addressBus.getPin(i));

		}

		for (int i = 0; i < dataBusSize; i++) {
			builder.output().connect(bank.dataBus.getPin(i).AND(read));
		}
	}

	void prepare() {
		circuit.prepare(true);
		simulator.circuit = &circuit;
	}

	void setData(int value) {
		for (int i = 0; i < dataBusSize; i++) {
			circuit.setInput(i + dataBitOffset, (float)(bool)(value & (1 << i)));
		}
	}

	void setAddress(int value) {
		for (int i = 0; i < addressBusSize; i++) {
			circuit.setInput(i + addressBitOffset, (float)(bool)(value & (1 << i)));
		}
	}

	int getOutput() {
		int value = 0;
		for (int i = 0; i < dataBusSize; i++) {
			bool v = circuit.getOutput(i);
			if (v > 0) {
				value |= (1 << i);
			}
		}
		return value;
	}

	void setClock(bool value) {
		circuit.setInput("clock", value);
	}

	void setRead(bool value) {
		circuit.setInput("read", value);
	}

	void setWrite(bool value) {
		circuit.setInput("write", value);
	}

	void tick() {
		setClock(0);
		simulator.simulate();
		setClock(1);
		simulator.simulate();
	}

	void tickEnd() {
		setClock(0);
		simulator.simulate();
	}

	std::string toString(const std::vector<float>& values) {
		std::string str;
		for (int i = 0; i < values.size(); i++) {
			if (!str.empty()) {
				str += ", ";
			}
			str += std::to_string((int)values[i]);
		}
		return str;
	}

	void printOutput() {
		std::vector<float> outputValues;
		for (int i = 0; i < circuit.getOutputCount(); i++) {
			float v = circuit.getOutput(i);
			outputValues.push_back(v);
		}

		int intOutputValue = 0;
		for (int i = 0; i < outputValues.size(); i++) {
			if (outputValues[i] > 0) {
				intOutputValue |= (1 << i);
			}
		}

		std::vector<float> inputValues;
		for (int i = 0; i < circuit.getInputCount(); i++) {
			float v = circuit.getInput(i);
			inputValues.push_back(v);
		}

		printf("in = %s ### out = %s (%i)\n", toString(inputValues).c_str(), toString(outputValues).c_str(), intOutputValue);
	}

	void printInfo() {
		printf("words: %i\n", wordCount);
		printf("word size: %i bit\n", dataBusSize);
		printf("total: %i bit (%i byte)\n", wordCount * dataBusSize, (wordCount * dataBusSize) / 8);
		printf("data bus: %i bit\n", dataBusSize);
		printf("address bus: %i bit\n", addressBusSize);

		printf("elements: %i\n", (int)circuit.structure.elements.size());
		printf("sockets: %i\n", (int)circuit.structure.sockets.size());
		printf("connections: %i\n", (int)circuit.structure.connections.size());

		int gateCount = 0;
		for (auto& e : circuit.structure.elements) {
			if (e.elementType == ElementType::GATE) {
				gateCount++;
			}
		}
		printf("gates: %i\n", gateCount);
		printf("transistors: %i\n", gateCount * 2);
	}

	void printMemUsage() {
		int elementBytes = circuit.structure.elements.capacity() * sizeof(Element);
		int socketBytes = circuit.structure.sockets.capacity() * sizeof(Socket);
		int connectionBytes = circuit.structure.connections.capacity() * sizeof(Connection);
		int totalBytes = elementBytes + socketBytes + connectionBytes;

		printf("elements: %i kb\n", elementBytes / 1024);
		printf("sockets: %i kb\n", socketBytes / 1024);
		printf("connections: %i kb\n", connectionBytes / 1024);
		printf("structure total: %i kb\n", totalBytes / 1024);

		int stateSocketsBytes = 0;
		for (auto& s : circuit.socketStates) {
			stateSocketsBytes += sizeof(s);
			stateSocketsBytes += s.inboundVoltages.size() * (sizeof(Index) + sizeof(float));
		}
		printf("state sockets: %i kb\n", stateSocketsBytes / 1024);
		int stateConnrctionsBytes = circuit.socketConnections.capacity() * sizeof(std::vector<Index>);
		for (auto& s : circuit.socketConnections) {
			stateConnrctionsBytes += s.capacity() * sizeof(Index);
		}
		printf("state connections: %i kb\n", stateConnrctionsBytes / 1024);
		totalBytes += stateSocketsBytes + stateConnrctionsBytes;
		printf("state total: %i kb\n", (stateSocketsBytes + stateConnrctionsBytes) / 1024);
		printf("total: %i kb\n", totalBytes / 1024);
	}
};
