//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "Element.h"
#include <vector>

class CircuitStructure {
public:
	std::vector<Element> elements;
	std::vector<Connection> connections;
	std::vector<Socket> sockets;
	std::vector<int> inputElements;
	std::vector<int> outputElements;

	int addElement(const Element &element);
	int addSocket(int elementIndex, SocketType socketType, SocketSlot socketSlot);
	int addConnection(int socket1Index, int socket2Index);

	Socket& getSocket(int elementIndex, SocketSlot socketSlot);
	int getSocketIndex(int elementIndex, SocketSlot socketSlot);
};
