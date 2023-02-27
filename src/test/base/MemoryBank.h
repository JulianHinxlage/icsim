//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "Bus.h"
#include <cmath>

class MemoryBank {
public:
	Circuit* circuit;
	int addressBusSize = 16;
	int dataBusSize = 8;
	int wordCount = 1024;

	Bus dataBus;
	Bus addressBus;

	CircuitBuilder clock;
	CircuitBuilder read;
	CircuitBuilder write;

	std::vector<Bus> cells;

	void build() {
		buildBase();
		buildCells();
	}

	void buildBase() {
		auto builder = circuit->builder();

		clock = builder.connector();
		read = builder.connector();
		write = builder.connector();

		dataBus.create(circuit, dataBusSize);
		addressBus.create(circuit, addressBusSize);
	}

	void buildCells() {
		buildCells_v3();
	}

	void addBank_v3(int level, CircuitBuilder read, CircuitBuilder write, Bus inBus, Bus outBus) {
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
			
			addBank_v3(level - 1, ar, aw, inBus.connect(aw), busA);
			addBank_v3(level - 1, br, bw, inBus.connect(bw), busB);
			
			busA.connect(outBus, ar);
			busB.connect(outBus, br);
		}
	}

	void buildCells_v3() {
		auto builder = circuit->builder();
		cells.clear();

		int level = std::max((int)std::log2(wordCount) + 1, addressBusSize - 1);
		addBank_v3(level, clock.AND(read), clock.AND(write), dataBus, dataBus);
	}

	void addBank_v2(int level, CircuitBuilder active) {
		if (cells.size() >= wordCount) {
			return;
		}

		if (level < 0) {
			cells.emplace_back();
			Bus& cell = cells.back();
			cell.circuit = circuit;

			for (int i = 0; i < dataBusSize; i++) {
				auto pin = dataBus.getPin(i);
				auto out = dLatch(pin, write.AND(clock).AND(active));
				out.AND(read).AND(clock).AND(active).connect(pin);
				cell.addPin(out);
			}
		}
		else {
			auto pin = addressBus.getPin(level);
			addBank_v2(level - 1, active.AND(pin.NOT()));
			addBank_v2(level - 1, active.AND(pin));
		}
	}

	void buildCells_v2() {
		auto builder = circuit->builder();
		cells.clear();

		auto edge = clock.AND(read.OR(write));

		int level = std::max((int)std::log2(wordCount) + 1, addressBusSize - 1);
		addBank_v2(level, edge);
	}

	void buildCells_v1() {
		auto builder = circuit->builder();

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
				auto pin = dataBus.getPin(j);

				auto out = dLatch(pin, write.AND(clock).AND(active));
				out.AND(read).AND(clock).AND(active).connect(pin);

				cell.addPin(out);
			}
		}
	}
};
