//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "Bus.h"

class Register {
public:
	Circuit* circuit;
	CircuitBuilder clock;

	Bus bufferCell;
	Bus cell;
	Bus inBus;
	Bus outBus;
	CircuitBuilder read;
	CircuitBuilder write;

	Register() {

	}

	void init(Circuit* circuit, CircuitBuilder clock) {
		this->circuit = circuit;
		this->clock = clock;
	}

	void init(Circuit* circuit, CircuitBuilder clock, Bus &inBus, Bus &outBus) {
		this->circuit = circuit;
		this->clock = clock;
		this->inBus = inBus;
		this->outBus = outBus;
	}

	void build() {
		CircuitBuilder builder = circuit->builder();

		read = builder.connector();
		write = builder.connector();
		cell.circuit = circuit;

		for (int i = 0; i < inBus.size(); i++) {
			auto inPin = inBus.getPin(i);
			auto outPin = outBus.getPin(i);

			auto output = dLatch(inPin, write.AND(clock));
			output.AND(read).AND(clock).connect(outPin);

			cell.addPin(output);
		}
	}

	void buildBuffered() {
		CircuitBuilder builder = circuit->builder();

		read = builder.connector();
		write = builder.connector();
		cell.circuit = circuit;
		bufferCell.circuit = circuit;

		for (int i = 0; i < inBus.size(); i++) {
			auto inPin = inBus.getPin(i);
			auto outPin = outBus.getPin(i);

			auto bufferOutput = dLatch(inPin, write.AND(clock));
			auto output = dLatch(bufferOutput, clock.NOT());
			output.AND(read).AND(clock).connect(outPin);

			cell.addPin(output);
			bufferCell.addPin(bufferOutput);
		}
	}

	Bus connect(CircuitBuilder active) {
		Bus bus(circuit);
		for (int i = 0; i < cell.size(); i++) {
			auto output = cell.getPin(i);
			bus.addPin(output.AND(active).AND(clock));
		}
		return bus;
	}

};
