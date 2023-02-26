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
	for (int inputElement : circuit->structure.inputElements) {
		elementUpdateQueue.push_back(inputElement);
		elementUpdateSet.insert(inputElement);
	}
	processQueue();
}

void DigitalCircuitSimulator::prepare() {
	elementUpdateQueue.clear();
	elementUpdateSet.clear();
	for (int i = 0; i < circuit->structure.elements.size(); i++) {
		auto& element = circuit->structure.elements[i];
		elementUpdateQueue.push_back(i);
		elementUpdateSet.insert(i);
	}
	processQueue();
}

void DigitalCircuitSimulator::processQueue() {
	while (!elementUpdateQueue.empty()) {
		int elementIndex = elementUpdateQueue.front();
		elementUpdateQueue.pop_front();
		elementUpdateSet.erase(elementIndex);

		auto& element = circuit->structure.elements[elementIndex];

		switch (element.elementType) {
		case ElementType::PIN: {
			if (element.pin.pinType == PinType::IN || element.pin.pinType == PinType::CONSTANT) {
				int socketIndex = element.socketIndises[(int)SocketSlot::PIN];
				propergateSignal(socketIndex);
			}
			break;
		}
		case ElementType::TRANSISTOR: {
			int collectorIndex = element.socketIndises[(int)SocketSlot::COLLECTOR];
			int baseIndex = element.socketIndises[(int)SocketSlot::BASE];
			int emitterIndex = element.socketIndises[(int)SocketSlot::EMITTER];

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
			int aIndex = element.socketIndises[(int)SocketSlot::GATE_A];
			int bIndex = element.socketIndises[(int)SocketSlot::GATE_B];
			int outIndex = element.socketIndises[(int)SocketSlot::GATE_OUT];

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
				out = (float)(a > 0 ^ b > 0);
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

void DigitalCircuitSimulator::propergateSignal(int sourceSocketIndex, std::set<int>& visitedSocketIndices, int socketIndex, float sourceVoltage) {
	//propergate the signal to all sockets connected through connections
	//if input sockets are updated the element if put into the update queue
	
	for (int socketIndex2 : circuit->socketConnections[socketIndex]) {
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

void DigitalCircuitSimulator::propergateSignal(int socketIndex) {
	auto& sourceState = circuit->socketStates[socketIndex];
	std::set<int> visitedSocketIndices;
	propergateSignal(socketIndex, visitedSocketIndices, socketIndex, sourceState.get());
}
