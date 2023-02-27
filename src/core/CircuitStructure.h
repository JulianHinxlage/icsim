//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "Element.h"
#include <string>
#include <vector>

class NamedPin {
public:
	Index index;
	std::string name;

	NamedPin(Index index = -1, const std::string& name = "") {
		this->index = index;
		this->name = name;
	}
};

class CircuitStructure {
public:
	std::vector<Element> elements;
	std::vector<Connection> connections;
	std::vector<Socket> sockets;



	std::vector<NamedPin> inputElements;
	std::vector<NamedPin> outputElements;

	Index addElement(const Element &element);
	Index addSocket(Index elementIndex, SocketType socketType, SocketSlot socketSlot);
	Index addConnection(Index socket1Index, Index socket2Index);

	void removeElement(Index elementIndex);
	void removeSocket(Index socketIndex);
	void removeConnection(Index connectionIndex);

	Socket& getSocket(Index elementIndex, SocketSlot socketSlot);
	Index getSocketIndex(Index elementIndex, SocketSlot socketSlot);

};
