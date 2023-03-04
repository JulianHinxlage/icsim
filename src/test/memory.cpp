//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "util/Clock.h"
#include "core/Circuit.h"
#include "core/Pin.h"
#include "core/Bus.h"
#include "cpu/MemoryBank.h"
#include <iostream>

class SubCircuit {
public:
	Circuit* circuit;
	std::vector<Index> internalPins;
	std::string name;

	std::map<std::string, Bus> buses;
	std::map<std::string, Pin> pins;
};

void testMemory() {
	Circuit circuit;
	MemoryBank memory;
	memory.circuit = &circuit;
	memory.addressBusSize = 16;
	memory.dataBusSize = 8;
	memory.wordCount = 1 << 8;
	int testCount = 1 << 8;

	Clock totalClock;
	Clock clock;

	memory.buildBase(false);
	memory.buildCells();

	printf("build took %fs\n", clock.round());

	circuit.prepare();

	printf("prepare took %fs\n", clock.round());

	//info
	printf("gates: %i\n", circuit.getGateCount());
	printf("lines: %i\n", circuit.getLineCount());
	printf("pins:  %i\n", circuit.getPinCount());



	srand(time(nullptr));

	std::vector<int> testValues;
	for (int i = 0; i < testCount; i++) {
		testValues.push_back(rand() % testCount);
	}

	for (int i = 0; i < testCount; i++) {
		memory.addressBus.setValue(i);
		memory.dataBus.setValue(testValues[i]);
		memory.write.setValue(true);
		memory.read.setValue(false);

		memory.clock.setValue(false);
		memory.circuit->simulate();
		memory.clock.setValue(true);
		memory.circuit->simulate();
		printf("%i: %i\n", i, (int)memory.dataBus.getValue());
		memory.clock.setValue(false);
		memory.circuit->simulate();
	}
	printf("\n");

	bool valid = true;
	memory.dataBus.setValue(0);
	for (int i = 0; i < testCount; i++) {
		memory.addressBus.setValue(i);
		memory.write.setValue(false);
		memory.read.setValue(true);


		memory.clock.setValue(false);
		memory.circuit->simulate();
		memory.clock.setValue(true);
		memory.circuit->simulate();
		printf("%i: %i\n", i, (int)memory.dataBus.getValue());

		if (testValues[i] != memory.dataBus.getValue()) {
			valid = false;
		}

		memory.clock.setValue(false);
		memory.circuit->simulate();
	}

	printf("\n");

	if (valid) {
		printf("result: OK\n");
	}
	else {
		printf("result: FAIL\n");
	}

	printf("\n");
	printf("sim took %fs\n", clock.round());
	printf("total took %fs\n", totalClock.round());
}

int main() {
	testMemory();
	return 0;
}
