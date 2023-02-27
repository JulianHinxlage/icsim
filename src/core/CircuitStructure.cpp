//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "CircuitStructure.h"

Index CircuitStructure::addElement(const Element& element) {
	Index index = elements.size();
	elements.push_back(element);

	//add sockets
	switch (element.elementType)
	{
	case ElementType::NONE:
		break;
	case ElementType::PIN:
		switch (element.pin.pinType) {
		case PinType::IN:
			addSocket(index, SocketType::OUT, SocketSlot::PIN);
			break;
		case PinType::OUT:
			addSocket(index, SocketType::IN, SocketSlot::PIN);
			break;
		case PinType::CONSTANT:
			addSocket(index, SocketType::OUT, SocketSlot::PIN);
			break;
		case PinType::CONNECTOR:
			addSocket(index, SocketType::IN_AND_OUT, SocketSlot::PIN);
			break;
		}
		break;
	case ElementType::RESISTOR:
		addSocket(index, SocketType::IN_AND_OUT, SocketSlot::RESISTOR_A);
		addSocket(index, SocketType::IN_AND_OUT, SocketSlot::RESISTOR_B);
		break;
	case ElementType::TRANSISTOR:
		addSocket(index, SocketType::IN, SocketSlot::COLLECTOR);
		addSocket(index, SocketType::IN, SocketSlot::BASE);
		addSocket(index, SocketType::OUT, SocketSlot::EMITTER);
		break;
	case ElementType::GATE:
		addSocket(index, SocketType::IN, SocketSlot::GATE_A);
		addSocket(index, SocketType::IN, SocketSlot::GATE_B);
		addSocket(index, SocketType::OUT, SocketSlot::GATE_OUT);
		break;
	default:
		break;
	}

	return index;
}

Index CircuitStructure::addSocket(Index elementIndex, SocketType socketType, SocketSlot socketSlot) {
	Index index = sockets.size();
	sockets.emplace_back();
	Socket& socket = sockets.back();

	socket.elementIndex = elementIndex;
	socket.socketSlot = socketSlot;
	socket.socketType = socketType;

	elements[elementIndex].socketIndises[(int)socketSlot] = index;
	return index;
}

Index CircuitStructure::addConnection(Index socket1Index, Index socket2Index) {
	Index index = connections.size();
	connections.emplace_back();
	Connection& connection = connections.back();

	connection.socket1Index = socket1Index;
	connection.socket2Index = socket2Index;

	return index;
}

void CircuitStructure::removeElement(Index elementIndex) {
	elements.erase(elements.begin() + elementIndex);
	for (auto& s : sockets) {
		if (s.elementIndex >= elementIndex) {
			s.elementIndex--;
		}
	}
}

void CircuitStructure::removeSocket(Index socketIndex) {
	sockets.erase(sockets.begin() + socketIndex);
	for (auto& e : elements) {
		for (int i = 0; i < 3; i++) {
			if (e.socketIndises[i] >= socketIndex) {
				e.socketIndises[i]--;
			}
		}
	}

	for (auto& c : connections) {
		if (c.socket1Index >= socketIndex) {
			c.socket1Index--;
		}
		if (c.socket2Index >= socketIndex) {
			c.socket2Index--;
		}
	}
}

void CircuitStructure::removeConnection(Index connectionIndex) {
	connections.erase(connections.begin() + connectionIndex);
}

Socket& CircuitStructure::getSocket(Index elementIndex, SocketSlot socketSlot) {
	Index socketIndex = elements[elementIndex].socketIndises[(int)socketSlot];
	return sockets[socketIndex];
}

Index CircuitStructure::getSocketIndex(Index elementIndex, SocketSlot socketSlot) {
	Index socketIndex = elements[elementIndex].socketIndises[(int)socketSlot];
	return socketIndex;
}
