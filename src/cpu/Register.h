//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "core/Bus.h"
#include "core/elements.h"

class Register {
public:
	Circuit* circuit;
	Pin clock;
	std::string name;

	Bus bufferCell;
	Bus cell;
	Bus inBus;
	Bus outBus;
	Pin read;
	Pin write;

	Register() {

	}

	void init(Circuit* circuit, Pin clock) {
		this->circuit = circuit;
		this->clock = clock;
	}

	void init(Circuit* circuit, Pin clock, Bus &inBus, Bus &outBus) {
		this->circuit = circuit;
		this->clock = clock;
		this->inBus = inBus;
		this->outBus = outBus;
	}

	void build() {
		Pin builder = Pin(circuit);

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
		Pin builder = Pin(circuit);

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
};
