//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "CircuitStructure.h"

int CircuitStructure::addElement(const Element& element) {
	int index = elements.size();
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

int CircuitStructure::addSocket(int elementIndex, SocketType socketType, SocketSlot socketSlot) {
	int index = sockets.size();
	sockets.emplace_back();
	Socket& socket = sockets.back();

	socket.elementIndex = elementIndex;
	socket.socketSlot = socketSlot;
	socket.socketType = socketType;

	elements[elementIndex].socketIndises[(int)socketSlot] = index;
	return index;
}

int CircuitStructure::addConnection(int socket1Index, int socket2Index) {
	int index = connections.size();
	connections.emplace_back();
	Connection& connection = connections.back();

	connection.socket1Index = socket1Index;
	connection.socket2Index = socket2Index;

	return index;
}

Socket& CircuitStructure::getSocket(int elementIndex, SocketSlot socketSlot) {
	int socketIndex = elements[elementIndex].socketIndises[(int)socketSlot];
	return sockets[socketIndex];
}

int CircuitStructure::getSocketIndex(int elementIndex, SocketSlot socketSlot) {
	int socketIndex = elements[elementIndex].socketIndises[(int)socketSlot];
	return socketIndex;
}
