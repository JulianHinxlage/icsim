//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "core/Circuit.h"
#include <string>
#include <cassert>

class Bus {
public:
	Circuit* circuit;
	std::vector<Index> sockets;
	std::string name;

	Bus(Circuit* circuit = nullptr) {
		this->circuit = circuit;
	}

	void addPin(CircuitBuilder builder) {
		sockets.push_back(builder.getSocketIndex());
	}

	CircuitBuilder getPin(int index) const {
		return CircuitBuilder(&circuit->structure, sockets[index]);
	}

	void create(Circuit* circuit, int size) {
		this->circuit = circuit;
		for (int i = 0; i < size; i++) {
			addPin(circuit->builder().connector());
		}
	}

	Bus split(int index, int segments) {
		Bus result;
		result.circuit = circuit;

		int s = size() / segments;
		for (int i = index * s; i < ((index + 1) * s); i++) {
			result.addPin(getPin(i));
		}
		return result;
	}

	int size() {
		return sockets.size();
	}

	void connect(const Bus& other) {
		assert(sockets.size() == other.sockets.size());
		for (int i = 0; i < sockets.size(); i++) {
			getPin(i).connect(other.getPin(i));
		}
	}

	void connect(const Bus& other, CircuitBuilder active) {
		assert(sockets.size() == other.sockets.size());
		for (int i = 0; i < sockets.size(); i++) {
			getPin(i).AND(active).connect(other.getPin(i));
		}
	}

	Bus connect(CircuitBuilder active) {
		Bus bus;
		bus.circuit = circuit;
		for (int i = 0; i < sockets.size(); i++) {
			bus.addPin(getPin(i).AND(active));
		}
		return bus;
	}

	void setIntValue(int value) {
		for (int i = 0; i < sockets.size(); i++) {
			if (value & (1 << i)) {
				circuit->setSocketValue(sockets[i], 1);
			}
			else {
				circuit->setSocketValue(sockets[i], 0);
			}
		}
	}

	std::string getValue() {
		std::string str;
		for (int i = 0; i < sockets.size(); i++) {
			bool v = circuit->getSocketValue(sockets[sockets.size() - 1 - i]) > 0;
			str += "01"[v];
		}
		return str;
	}

	int getIntValue() {
		int value = 0;
		for (int i = 0; i < sockets.size(); i++) {
			bool v = circuit->getSocketValue(sockets[i]) > 0;
			if (v) {
				value |= (1 << i);
			}
		}
		return value;
	}
};

CircuitBuilder dLatch(CircuitBuilder data, CircuitBuilder set) {
	auto s = data.NAND(set);
	auto r = data.NOT().NAND(set);

	CircuitBuilder builder = data.unconnect();
	auto sTmp = builder.connector();
	auto rTmp = builder.connector();

	r.NAND(rTmp).connect(sTmp);
	return s.NAND(sTmp).connect(rTmp);
}

CircuitBuilder busMatch(Bus &bus, int value) {
	CircuitBuilder active;
	for (int i = 0; i < bus.size(); i++) {
		auto pin = bus.getPin(i);
		if (i == 0) {
			if (value & (1 << i)) {
				active = pin;
			}
			else {
				active = pin.NOT();
			}
		}
		else {
			if (value & (1 << i)) {
				active = active.AND(pin);
			}
			else {
				active = active.AND(pin.NOT());
			}
		}
	}
	return active;
}

Bus multiplexer(Bus &input) {
	Bus output;
	output.circuit = input.circuit;
	for (int i = 0; i < (1 << input.size()); i++) {
		output.addPin(busMatch(input, i));
	}
	return output;
}
