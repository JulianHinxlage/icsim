//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "core/Circuit.h"
#include "core/Evaluator.h"
#include "core/DigitalCircuitSimulator.h"
#include "base/MemoryTester.h"
#include "util/Clock.h"
#include <string>
#include <iostream>

void memoryTest() {
	Clock totalClock;
	Clock clock;

	MemoryTester memory;

	memory.dataBusSize = 8;
	memory.addressBusSize = 10;
	memory.wordCount = 1 << memory.addressBusSize;
	int testCount = 1 << 4;

	memory.build();
	printf("build took %fs\n", clock.round());
	printf("reduction took %fs\n", clock.round());
	memory.prepare();
	printf("prepare took %fs\n", clock.round());
	printf("\n");

	memory.printInfo();
	printf("\n");
	memory.printMemUsage();
	printf("\n");


	srand(time(nullptr));

	int maxValue = 1 << memory.dataBusSize;
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
		memory.tickEnd();
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
		memory.tickEnd();
	}

	printf("\n");
	if (valid) {
		printf("result: OK\n");
	}
	else {
		printf("result: FAIL\n");
	}

	printf("\n");
	printf("simulation took %fs\n", clock.round());
	printf("total %fs\n", totalClock.round());
}

int main(int argc, char* argv[]) {
	memoryTest();
	return 0;
}
