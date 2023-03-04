//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "core/Circuit.h"
#include "core/elements.h"
#include <cmath>

class MemoryBank {
public:
	Circuit* circuit;
	int addressBusSize = 16;
	int dataBusSize = 8;
	int wordCount = 1024;

	Bus dataBus;
	Bus addressBus;

	Pin clock;
	Pin read;
	Pin write;

	std::vector<Bus> cells;
	Bus internalReadBus;
	Bus internalWriteBus;

	void build() {
		buildBase();
		buildCells();
	}

	void buildBase(bool useInternalBus = false) {
		auto builder = Pin(circuit);

		clock = builder.connector();
		read = builder.connector();
		write = builder.connector();

		if (useInternalBus) {
			addressBus.create(circuit, addressBusSize);
			dataBus.create(circuit, dataBusSize);

			internalReadBus.create(circuit, dataBusSize);
			internalWriteBus.create(circuit, dataBusSize);

			dataBus.BUF().connect(internalWriteBus);
			internalReadBus.BUF().connect(dataBus);
		}
		else {
			addressBus.create(circuit, addressBusSize);
			dataBus.create(circuit, dataBusSize);

			internalReadBus = dataBus;
			internalWriteBus = dataBus;
		}
	}

	void buildCells() {
		buildCells_v2();
	}

	void buildCells_v1() {
		auto builder = Pin(circuit);

		//word cells
		cells.resize(wordCount);
		for (int i = 0; i < wordCount; i++) {

			auto active = builder;
			for (int k = 0; k < addressBusSize; k++) {
				auto pin = addressBus.getPin(k);
				if (k == 0) {
					if (i & (1 << k)) {
						active = pin;
					}
					else {
						active = pin.NOT();
					}
				}
				else {
					if (i & (1 << k)) {
						active = active.AND(pin);
					}
					else {
						active = active.AND(pin.NOT());
					}
				}
			}

			Bus& cell = cells[i];
			cell.circuit = circuit;

			for (int j = 0; j < dataBusSize; j++) {
				auto out = dLatch(internalWriteBus.getPin(j), write.AND(clock).AND(active));
				out.AND(read).AND(clock).AND(active).connect(internalReadBus.getPin(j));
				cell.addPin(out);
			}
		}
	}

	void addBank_v2(int level, Pin read, Pin write, Bus inBus, Bus outBus) {
		if (cells.size() >= wordCount) {
			return;
		}

		bool isCell = false;
		if (level < 0) {
			isCell = true;
		}

		if (isCell) {
			cells.emplace_back();
			Bus& cell = cells.back();
			cell.circuit = circuit;

			for (int i = 0; i < dataBusSize; i++) {
				auto out = dLatch(inBus.getPin(i), write);
				out.AND(read).connect(outBus.getPin(i));
				cell.addPin(out);
			}
		}
		else {
			auto pin = addressBus.getPin(level);

			auto ar = read.AND(pin.NOT());
			auto aw = write.AND(pin.NOT());

			auto br = read.AND(pin);
			auto bw = write.AND(pin);

			Bus busA;
			Bus busB;
			busA.create(circuit, dataBusSize);
			busB.create(circuit, dataBusSize);

			addBank_v2(level - 1, ar, aw, inBus.AND(aw), busA);
			addBank_v2(level - 1, br, bw, inBus.AND(bw), busB);

			busA.AND(ar).connect(outBus);
			busB.AND(br).connect(outBus);
		}
	}

	void buildCells_v2() {
		auto builder = Pin(circuit);
		cells.clear();

		int level = std::min((int)std::log2(wordCount) + 1, addressBusSize - 1);
		addBank_v2(level, clock.AND(read), clock.AND(write), internalWriteBus, internalReadBus);
	}
};
