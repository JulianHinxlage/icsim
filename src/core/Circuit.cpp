//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "Circuit.h"
#include <cassert>

Index Circuit::addGate(GateType type) {
	switch (type)
	{
	case GateType::CONNECTOR:
		addPin(PinType::CONNECTOR);
		break;
	case GateType::OUTPUT:
		addPin(PinType::OUTPUT);
		break;
	case GateType::BUF:
		addPin(PinType::BUF_IN);
		addPin(PinType::BUF_OUT);
		gateCount++;
		break;
	case GateType::NOT:
		addPin(PinType::NOT_IN);
		addPin(PinType::NOT_OUT);
		gateCount++;
		break;
	case GateType::OR:
		addPin(PinType::OR_A);
		addPin(PinType::OR_B);
		addPin(PinType::OR_OUT);
		gateCount++;
		break;
	case GateType::AND:
		addPin(PinType::AND_A);
		addPin(PinType::AND_B);
		addPin(PinType::AND_OUT);
		gateCount++;
		break;
	case GateType::NOR:
		addPin(PinType::NOR_A);
		addPin(PinType::NOR_B);
		addPin(PinType::NOR_OUT);
		gateCount++;
		break;
	case GateType::NAND:
		addPin(PinType::NAND_A);
		addPin(PinType::NAND_B);
		addPin(PinType::NAND_OUT);
		gateCount++;
		break;
	case GateType::XOR:
		addPin(PinType::XOR_A);
		addPin(PinType::XOR_B);
		addPin(PinType::XOR_OUT);
		gateCount++;
		break;
	case GateType::D_LATCH:
		addPin(PinType::D_LATCH_DATA);
		addPin(PinType::D_LATCH_ENABLE);
		addPin(PinType::D_LATCH_OUT);
		gateCount++;
		break;
	default:
		break;
	}
	return pins.size() - 1;
}

void Circuit::addLine(Index pinA, Index pinB) {
	lines.push_back({ pinA, pinB });
}

Index Circuit::addPin(PinType type) {
	pins.push_back(type);
	inboundPin.push_back(-1);
	outboundPin.push_back(-1);
	pinStates.push_back(false);
	Index index = pins.size() - 1;
	return index;
}

int Circuit::getGateCount() {
	return gateCount;
}

int Circuit::getPinCount() {
	return pins.size();
}

int Circuit::getLineCount() {
	return lines.size();
}

int64_t Circuit::getSimulationTime() {
	return simulationTime;
}

void Circuit::setGateDelay(GateType type, int delay) {
	if (gateDelays.size() <= (int)type) {
		gateDelays.resize((int)type + 1, 1);
	}
	gateDelays[(int)type] = delay;
}

void Circuit::setSimulationMode(bool sortQueue) {
	queue.sortQueue = sortQueue;
}

void Circuit::prepare() {
	gateDelays.resize((int)GateType::GATE_TYPE_COUNT, 1);

	pinStates.clear();
	pinStates.resize(pins.size(), 0);

	initPinConnections();

	for (Index i = 0; i < pins.size(); i++) {
		addPinToQueue(i);
	}
	changedPins.clear();
	processQueue();
	simulationTime = 0;
}

int Circuit::simulate(int timeUnits) {
	for (auto& pin : changedPins) {
		addPinToQueue(pin, 0, true);
	}
	changedPins.clear();
	return processQueue(timeUnits);
}

void Circuit::addPinConnection(Index pinA, Index pinB) {
	PinBaseType a = getPinBaseType(pins[pinA]);
	PinBaseType b = getPinBaseType(pins[pinB]);

	if (b == PinBaseType::INPUT || b == PinBaseType::CONNECTOR) {
		if (a == PinBaseType::OUTPUT || a == PinBaseType::CONNECTOR) {

			if (inboundPin[pinB] == -1) {
				inboundPin[pinB] = pinA;
			}
			else if (inboundPin[pinB] == -2) {
				inboundPins[pinB].insert(pinA);
			}
			else {
				if (inboundPin[pinB] != pinA) {
					inboundPins[pinB].insert(inboundPin[pinB]);
					inboundPins[pinB].insert(pinA);
					inboundPin[pinB] = -2;
				}
			}

			if (outboundPin[pinA] == -1) {
				outboundPin[pinA] = pinB;
			}
			else if (outboundPin[pinA] == -2) {
				outboundPins[pinA].insert(pinB);
			}
			else {
				if (outboundPin[pinA] != pinB) {
					outboundPins[pinA].insert(outboundPin[pinA]);
					outboundPins[pinA].insert(pinB);
					outboundPin[pinA] = -2;
				}
			}

		}
	}
}

void Circuit::initPinConnections() {
	inboundPin.clear();
	inboundPins.clear();
	outboundPin.clear();
	outboundPins.clear();
	inboundPin.resize(pins.size(), -1);
	outboundPin.resize(pins.size(), -1);

	groups.clear();
	groupByPin.clear();
	groupUpToDate.clear();
	groupValues.clear();
	groupByPin.resize(pins.size(), -1);

	for (auto& line : lines) {
		Index a = line.first;
		Index b = line.second;
		Index ag = groupByPin[a];
		Index bg = groupByPin[b];

		if (ag == -1 && bg == -1) {
			groups.emplace_back();
			auto g = groups.size() - 1;
			groups[g].insert(a);
			groups[g].insert(b);
			groupByPin[a] = g;
			groupByPin[b] = g;
		}
		else if (ag == -1) {
			groups[bg].insert(a);
			groupByPin[a] = bg;
		}
		else if (bg == -1) {
			groups[ag].insert(b);
			groupByPin[b] = ag;
		}
		else {
			for (auto i : groups[bg]) {
				groups[ag].insert(i);
				groupByPin[i] = ag;
			}
			groups[bg].clear();
		}
	}

	for (auto& group : groups) {
		for (auto& a : group) {
			for (auto& b : group) {
				if (a != b) {
					addPinConnection(a, b);
				}
			}
		}
	}

	groupUpToDate.resize(groups.size(), false);
	groupValues.resize(groups.size(), false);

	for (auto& group : groups) {
		auto it = group.begin();
		while (it != group.end()) {
			auto current = it++;
			auto pin = *current;
			auto baseType = getPinBaseType(pins[pin]);
			if (baseType == PinBaseType::INPUT) {
				group.erase(current);
			}
		}
	}
}

bool Circuit::getInboundSignal(Index pin) {
	Index source = inboundPin[pin];
	if (source == -1) {
		return pinStates[pin];
	}
	else if (source == -2) {
		auto groupIndex = groupByPin[pin];
		if (groupUpToDate[groupIndex]) {
			return groupValues[groupIndex];
		}

		auto &group = groups[groupIndex];

		bool value = false;
		for (auto& i : group) {
			if (i != pin) {
				value |= pinStates[i];
				if (value) {
					break;
				}
			}
		}

		groupUpToDate[groupIndex] = true;
		groupValues[groupIndex] = value;
		return value;
	}
	else {
		return pinStates[source];
	}
}

void Circuit::addPinToQueue(Index pin, int delay, bool external) {
	queue.add(pin, simulationTime + delay, external);
}

void Circuit::addOutboundPinsToQueue(Index pin) {
	Index destination = outboundPin[pin];
	if (destination == -1) {
		return;
	}
	else if (destination == -2) {
		for (auto& p : outboundPins[pin]) {
			addPinToQueue(p);
		}

		auto groupIndex = groupByPin[pin];
		groupUpToDate[groupIndex] = false;
	}
	else {
		addPinToQueue(destination);
	}
}

int Circuit::processQueue(int timeUnits) {
	int64_t startSimulationTime = simulationTime;
	int64_t endSimulationTime = simulationTime;
	if (timeUnits != -1) {
		endSimulationTime += timeUnits;
	}

	while (!queue.empty()) {
		auto event = queue.get();

		//assert(event.time >= simulationTime && "a gate was updated to late");
		if (timeUnits != -1 && event.time > endSimulationTime) {
			break;
		}
		if (event.time > simulationTime) {
			simulationTime = event.time;
		}

		queue.pop();

		Index pin = event.pin;
		PinType type = pins[pin];
		PinBaseType baseType = getPinBaseType(type);

		if (event.external) {
			addOutboundPinsToQueue(pin);
			continue;
		}

		switch (baseType)
		{
		case PinBaseType::CONNECTOR: {
			if (type == PinType::CONNECTOR) {
				pinStates[pin] = getInboundSignal(pin);
			}
			else if (type == PinType::OUTPUT) {
				addOutboundPinsToQueue(pin);
			}
			break;
		}
		case PinBaseType::INPUT: {
			bool oldValue = pinStates[pin];
			pinStates[pin] = getInboundSignal(pin);
			if (oldValue != pinStates[pin]) {
				switch (type)
				{
				case PinType::CONNECTOR:
					break;
				case PinType::OUTPUT:
					break;
				case PinType::BUF_IN:
					addPinToQueue(pin + 1, gateDelays[(int)GateType::BUF]);
					break;
				case PinType::BUF_OUT:
					break;
				case PinType::NOT_IN:
					addPinToQueue(pin + 1, gateDelays[(int)GateType::NOT]);
					break;
				case PinType::NOT_OUT:
					break;
				case PinType::OR_A:
					addPinToQueue(pin + 2, gateDelays[(int)GateType::OR]);
					break;
				case PinType::OR_B:
					addPinToQueue(pin + 1, gateDelays[(int)GateType::OR]);
					break;
				case PinType::OR_OUT:
					break;
				case PinType::AND_A:
					addPinToQueue(pin + 2, gateDelays[(int)GateType::AND]);
					break;
				case PinType::AND_B:
					addPinToQueue(pin + 1, gateDelays[(int)GateType::AND]);
					break;
				case PinType::AND_OUT:
					break;
				case PinType::NOR_A:
					addPinToQueue(pin + 2, gateDelays[(int)GateType::NOR]);
					break;
				case PinType::NOR_B:
					addPinToQueue(pin + 1, gateDelays[(int)GateType::NOR]);
					break;
				case PinType::NOR_OUT:
					break;
				case PinType::NAND_A:
					addPinToQueue(pin + 2, gateDelays[(int)GateType::NAND]);
					break;
				case PinType::NAND_B:
					addPinToQueue(pin + 1, gateDelays[(int)GateType::NAND]);
					break;
				case PinType::NAND_OUT:
					break;
				case PinType::XOR_A:
					addPinToQueue(pin + 2, gateDelays[(int)GateType::XOR]);
					break;
				case PinType::XOR_B:
					addPinToQueue(pin + 1, gateDelays[(int)GateType::XOR]);
					break;
				case PinType::XOR_OUT:
					break;
				case PinType::D_LATCH_DATA:
					addPinToQueue(pin + 2, gateDelays[(int)GateType::D_LATCH]);
					break;
				case PinType::D_LATCH_ENABLE:
					addPinToQueue(pin + 1, gateDelays[(int)GateType::D_LATCH]);
					break;
				case PinType::D_LATCH_OUT:
					break;
				case PinType::DISABLED:
					break;
				default:
					break;
				}
			}
			break;
		}
		case PinBaseType::OUTPUT: {
			bool oldValue = pinStates[pin];
			switch (type)
			{
			case PinType::CONNECTOR:
				break;
			case PinType::OUTPUT:
				break;
			case PinType::BUF_IN:
				break;
			case PinType::BUF_OUT:
				pinStates[pin] = pinStates[pin - 1];
				break;
			case PinType::NOT_IN:
				break;
			case PinType::NOT_OUT:
				pinStates[pin] = !pinStates[pin - 1];
				break;
			case PinType::OR_A:
				break;
			case PinType::OR_B:
				break;
			case PinType::OR_OUT:
				pinStates[pin] = pinStates[pin - 2] || pinStates[pin - 1];
				break;
			case PinType::AND_A:
				break;
			case PinType::AND_B:
				break;
			case PinType::AND_OUT:
				pinStates[pin] = pinStates[pin - 2] && pinStates[pin - 1];
				break;
			case PinType::NOR_A:
				break;
			case PinType::NOR_B:
				break;
			case PinType::NOR_OUT:
				pinStates[pin] = !(pinStates[pin - 2] || pinStates[pin - 1]);
				break;
			case PinType::NAND_A:
				break;
			case PinType::NAND_B:
				break;
			case PinType::NAND_OUT:
				pinStates[pin] = !(pinStates[pin - 2] && pinStates[pin - 1]);
				break;
			case PinType::XOR_A:
				break;
			case PinType::XOR_B:
				break;
			case PinType::XOR_OUT:
				pinStates[pin] = pinStates[pin - 2] ^ pinStates[pin - 1];
				break;
			case PinType::D_LATCH_DATA:
				break;
			case PinType::D_LATCH_ENABLE:
				break;
			case PinType::D_LATCH_OUT:
				if (pinStates[pin - 1]) {
					pinStates[pin] = pinStates[pin - 2];
				}
				break;
			case PinType::DISABLED:
				break;
			default:
				break;
			}
			if (oldValue != pinStates[pin]) {
				addOutboundPinsToQueue(pin);
			}
			break;
		}
		default:
			break;
		}
	}

	int timeNeeded = simulationTime - startSimulationTime;
	if (simulationTime < endSimulationTime) {
		simulationTime = endSimulationTime;
	}
	return timeNeeded;
}
