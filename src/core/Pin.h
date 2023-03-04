//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "type.h"

class Pin {
public:
	class Circuit* circuit;
	Index index;

	Pin(Circuit* circuit = nullptr, Index index = -1);

	Pin connector();
	Pin input();
	Pin connect(Pin rhs);

	Pin BUF();
	Pin AND(Pin rhs);
	Pin OR(Pin rhs);
	Pin NOT();
	Pin NAND(Pin rhs);
	Pin NOR(Pin rhs);
	Pin XOR(Pin rhs);
	Pin dLatch(Pin enable);

	Pin zero();
	Pin one();

	bool getValue();
	void setValue(bool value);
};
