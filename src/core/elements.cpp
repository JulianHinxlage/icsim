//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "elements.h"

Pin dLatch(Pin data, Pin enable) {
	auto s = data.NAND(enable);
	auto r = data.NOT().NAND(enable);

	auto notq = data.connector();
	auto q = data.connector();

	r.NAND(q).connect(notq);
	return s.NAND(notq).connect(q);
}

Pin busMatch(Bus& bus, int value) {
	Pin active;
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

Bus multiplexer(Bus input) {
	Bus output;
	output.circuit = input.circuit;
	for (int i = 0; i < (1 << input.size()); i++) {
		output.addPin(busMatch(input, i));
	}
	return output;
}

Pin tLatch(Pin toggle) {
	auto q = toggle.connector();
	dLatch(dLatch(q, toggle.NOT()), toggle).NOT().connect(q);
	return q.NOT();
}
