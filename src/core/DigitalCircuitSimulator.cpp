//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "DigitalCircuitSimulator.h"

DigitalCircuitSimulator::DigitalCircuitSimulator(Circuit* circuit) {
	this->circuit = circuit;
}

void DigitalCircuitSimulator::simulate() {
	elementUpdateQueue.clear();
	elementUpdateSet.clear();
	for (auto &input : circuit->structure.inputElements) {
		elementUpdateQueue.push_back(input.index);
		elementUpdateSet.insert(input.index);
	}
	for (auto& socketIndex : circuit->changedSockets) {
		propergateSignal(socketIndex);
	}
	circuit->changedSockets.clear();
	processQueue();
}

void DigitalCircuitSimulator::prepare() {
	elementUpdateQueue.clear();
	elementUpdateSet.clear();
	for (Index i = 0; i < circuit->structure.elements.size(); i++) {
		elementUpdateQueue.push_back(i);
		elementUpdateSet.insert(i);
	}
	processQueue();
}

void DigitalCircuitSimulator::processQueue() {
	while (!elementUpdateQueue.empty()) {
		Index elementIndex = elementUpdateQueue.front();
		elementUpdateQueue.pop_front();
		elementUpdateSet.erase(elementIndex);

		auto& element = circuit->structure.elements[elementIndex];

		switch (element.elementType) {
		case ElementType::PIN: {
			if (element.pin.pinType == PinType::IN || element.pin.pinType == PinType::CONSTANT) {
				Index socketIndex = element.socketIndises[(int)SocketSlot::PIN];
				propergateSignal(socketIndex);
			}
			break;
		}
		case ElementType::TRANSISTOR: {
			Index collectorIndex = element.socketIndises[(int)SocketSlot::COLLECTOR];
			Index baseIndex = element.socketIndises[(int)SocketSlot::BASE];
			Index emitterIndex = element.socketIndises[(int)SocketSlot::EMITTER];

			float collector = circuit->socketStates[collectorIndex].get();
			float base = circuit->socketStates[baseIndex].get();
			float emitter = 0;
			if (base > 0) {
				emitter = collector;
			}
			else {
				emitter = 0;
			}

			if (emitter != circuit->socketStates[emitterIndex].get()) {
				circuit->socketStates[emitterIndex].set(emitterIndex, emitter);
				propergateSignal(emitterIndex);
			}
			break;
		}
		case ElementType::GATE: {
			Index aIndex = element.socketIndises[(int)SocketSlot::GATE_A];
			Index bIndex = element.socketIndises[(int)SocketSlot::GATE_B];
			Index outIndex = element.socketIndises[(int)SocketSlot::GATE_OUT];

			float a = circuit->socketStates[aIndex].get();
			float b = circuit->socketStates[bIndex].get();
			float out = 0;

			switch (element.gate.gateType) {
			case GateType::NONE:
				break;
			case GateType::AND:
				out = (float)(a > 0 && b > 0);
				break;
			case GateType::OR:
				out = (float)(a > 0 || b > 0);
				break;
			case GateType::NOT:
				out = (float)!(a > 0);
				break;
			case GateType::NAND:
				out = (float)!(a > 0 && b > 0);
				break;
			case GateType::NOR:
				out = (float)!(a > 0 || b > 0);
				break;
			case GateType::XOR:
				out = (float)((a > 0) ^ (b > 0));
				break;
			default:
				break;
			}

			if (out != circuit->socketStates[outIndex].get()) {
				circuit->socketStates[outIndex].set(outIndex, out);
				propergateSignal(outIndex);
			}
			break;
		}

		default:
			break;
		}
	}
}

void DigitalCircuitSimulator::propergateSignal(Index sourceSocketIndex, std::set<Index>& visitedSocketIndices, Index socketIndex, float sourceVoltage) {
	//propergate the signal to all sockets connected through connections
	//if input sockets are updated the element if put into the update queue
	
	for (Index socketIndex2 : circuit->socketConnections[socketIndex]) {
		if (!visitedSocketIndices.contains(socketIndex2)) {
			auto& socket = circuit->structure.sockets[socketIndex2];
			auto& state = circuit->socketStates[socketIndex2];

			if ((int)socket.socketType & (int)SocketType::IN) {
				float preValue = state.get();
				state.set(sourceSocketIndex, sourceVoltage);
				if (preValue != state.get()) {
					if (!elementUpdateSet.contains(socket.elementIndex)) {
						elementUpdateQueue.push_back(socket.elementIndex);
						elementUpdateSet.insert(socket.elementIndex);
					}
				}
			}

			if (circuit->socketConnections[socketIndex2].size() > 1) {
				visitedSocketIndices.insert(socketIndex);
				propergateSignal(sourceSocketIndex, visitedSocketIndices, socketIndex2, sourceVoltage);
			}
		}
	}
}

void DigitalCircuitSimulator::propergateSignal(Index socketIndex) {
	if (recursiveSignalPropergation) {
		auto& sourceState = circuit->socketStates[socketIndex];
		std::set<Index> visitedSocketIndices;
		propergateSignal(socketIndex, visitedSocketIndices, socketIndex, sourceState.get());
	}
	else {
		auto& sourceState = circuit->socketStates[socketIndex];
		for (Index socketIndex2 : circuit->socketConnections[socketIndex]) {
			auto& socket = circuit->structure.sockets[socketIndex2];
			auto& state = circuit->socketStates[socketIndex2];

			if ((int)socket.socketType & (int)SocketType::IN) {
				float preValue = state.get();
				state.set(socketIndex, sourceState.get());
				if (preValue != state.get()) {
					if (!elementUpdateSet.contains(socket.elementIndex)) {
						elementUpdateQueue.push_back(socket.elementIndex);
						elementUpdateSet.insert(socket.elementIndex);
					}
				}
			}
		}
	}
}
