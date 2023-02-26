//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "core/Circuit.h"
#include "core/Evaluator.h"
#include "core/DigitalCircuitSimulator.h"
#include "util/Clock.h"
#include <string>
#include <iostream>

CircuitBuilder dLatch(CircuitBuilder data, CircuitBuilder set) {
	auto s = data.NAND(set);
	auto r = data.NOT().NAND(set);

	CircuitBuilder builder = data.unconnect();
	auto sTmp = builder.connection();
	auto rTmp = builder.connection();

	r.NAND(rTmp).endConnection(sTmp);
	return s.NAND(sTmp).connection(rTmp);
}

class Memory {
public:
	Circuit circuit;
	DigitalCircuitSimulator simulator;

	int dataBits = 8;
	int addressBits = 4;
	int wordCount = 16;
	int dataBitOffset;
	int addressBitOffset;

	int clockIndex;
	int writeIndex;
	int readIndex;

	void build() {
		auto builder = circuit.builder();

		clockIndex = circuit.getInputCount();
		auto clock = builder.input();
		writeIndex = circuit.getInputCount();
		auto write = builder.input();
		readIndex = circuit.getInputCount();
		auto read = builder.input();

		dataBitOffset = circuit.getInputCount();
		std::vector<CircuitBuilder> inDataBits(dataBits);
		for (int i = 0; i < dataBits; i++) {
			inDataBits[i] = builder.input();
		}

		addressBitOffset = circuit.getInputCount();
		std::vector<CircuitBuilder> inAddressBits(addressBits);
		for (int i = 0; i < addressBits; i++) {
			inAddressBits[i] = builder.input();
		}

		std::vector<CircuitBuilder> outDataBits(dataBits);
		for (int i = 0; i < dataBits; i++) {
			outDataBits[i] = builder.output();
		}

		//worlds
		for (int i = 0; i < wordCount; i++) {

			auto active = builder;
			for (int k = 0; k < addressBits; k++) {
				if (k == 0) {
					if (i & (1 << k)) {
						active = inAddressBits[k];
					}
					else {
						active = inAddressBits[k].NOT();
					}
				}
				else {
					if (i & (1 << k)) {
						active = active.AND(inAddressBits[k]);
					}
					else {
						active = active.AND(inAddressBits[k].NOT());
					}
				}
			}

			for (int j = 0; j < dataBits; j++) {
				auto out = dLatch(inDataBits[j], write.AND(clock).AND(active));
				out.AND(read).AND(active).endConnection(outDataBits[j]);
			}
		}

	}

	void prepare() {
		circuit.prepare(true);
		simulator.circuit = &circuit;
	}

	void setData(int value) {
		for (int i = 0; i < dataBits; i++) {
			circuit.setInput(i + dataBitOffset, (float)(bool)(value & (1 << i)));
		}
	}

	void setAddress(int value) {
		for (int i = 0; i < addressBits; i++) {
			circuit.setInput(i + addressBitOffset, (float)(bool)(value & (1 << i)));
		}
	}

	int getOutput() {
		int value = 0;
		for (int i = 0; i < dataBits; i++) {
			bool v = circuit.getOutput(i);
			if (v > 0) {
				value |= (1 << i);
			}
		}
		return value;
	}

	void setClock(bool value) {
		circuit.setInput(clockIndex, (float)value);
	}

	void setRead(bool value) {
		circuit.setInput(readIndex, (float)value);
	}

	void setWrite(bool value) {
		circuit.setInput(writeIndex, (float)value);
	}

	void tick() {
		setClock(0);
		simulator.simulate();
		setClock(1);
		simulator.simulate();
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
		printf("word size: %i bit\n", dataBits);
		printf("total: %i bit (%i byte)\n", wordCount * dataBits, (wordCount * dataBits) / 8);
		printf("data bus: %i bit\n", dataBits);
		printf("address bus: %i bit\n", addressBits);

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
			stateSocketsBytes += s.inboundVoltages.size() * (sizeof(int) + sizeof(float));
		}
		printf("state sockets: %i kb\n", stateSocketsBytes / 1024);
		int stateConnrctionsBytes = circuit.socketConnections.capacity() * sizeof(std::vector<int>);
		for (auto& s : circuit.socketConnections) {
			stateConnrctionsBytes += s.capacity() * sizeof(int);
		}
		printf("state connections: %i kb\n", stateConnrctionsBytes / 1024);
		totalBytes += stateSocketsBytes + stateConnrctionsBytes;
		printf("state total: %i kb\n", (stateSocketsBytes + stateConnrctionsBytes) / 1024);
		printf("total: %i kb\n", totalBytes / 1024);
	}
};

void memoryTest() {
	Clock totalClock;
	Clock clock;

	Memory memory;

	memory.dataBits = 8;
	memory.addressBits = 8;
	memory.wordCount = 1 << memory.addressBits;
	int testCount = 1 << 5;

	memory.build();
	printf("build took %fs\n", clock.round());
	memory.prepare();
	printf("prepare took %fs\n", clock.round());
	printf("\n");

	memory.printInfo();
	printf("\n");
	memory.printMemUsage();
	printf("\n");


	srand(time(nullptr));

	int maxValue = 1 << memory.dataBits;
	std::vector<int> testData;
	for (int i = 0; i < testCount; i++) {
		testData.push_back(rand() % maxValue);
	}



	//write
	memory.setWrite(1);
	memory.setRead(0);
	for (int i = 0; i < testCount; i++) {
		memory.setAddress(i);
		memory.setData(testData[i]);
		memory.tick();
		memory.printOutput();
	}

	bool valid = true;

	//read
	memory.setWrite(0);
	memory.setRead(1);
	memory.setData(0);
	for (int i = 0; i < testCount; i++) {
		memory.setAddress(i);
		int expected = testData[i];
		memory.tick();
		memory.printOutput();
		if (expected != memory.getOutput()) {
			printf("dose not match\n");
			valid = false;
		}
	}

	if (valid) {
		printf("OK\n");
	}
	else {
		printf("FAIL\n");
	}

	printf("\n");
	printf("simulation took %fs\n", clock.round());
	printf("total %fs\n", totalClock.round());
}

int main(int argc, char* argv[]) {
	memoryTest();
	return 0;
}
