//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "type.h"

PinBaseType getPinBaseType(PinType type) {
	switch (type)
	{
	case PinType::CONNECTOR:
		return PinBaseType::CONNECTOR;
	case PinType::OUTPUT:
		return PinBaseType::CONNECTOR;
	case PinType::BUF_IN:
		return PinBaseType::INPUT;
	case PinType::BUF_OUT:
		return PinBaseType::OUTPUT;
	case PinType::NOT_IN:
		return PinBaseType::INPUT;
	case PinType::NOT_OUT:
		return PinBaseType::OUTPUT;
	case PinType::OR_A:
		return PinBaseType::INPUT;
	case PinType::OR_B:
		return PinBaseType::INPUT;
	case PinType::OR_OUT:
		return PinBaseType::OUTPUT;
	case PinType::AND_A:
		return PinBaseType::INPUT;
	case PinType::AND_B:
		return PinBaseType::INPUT;
	case PinType::AND_OUT:
		return PinBaseType::OUTPUT;
	case PinType::NOR_A:
		return PinBaseType::INPUT;
	case PinType::NOR_B:
		return PinBaseType::INPUT;
	case PinType::NOR_OUT:
		return PinBaseType::OUTPUT;
	case PinType::NAND_A:
		return PinBaseType::INPUT;
	case PinType::NAND_B:
		return PinBaseType::INPUT;
	case PinType::NAND_OUT:
		return PinBaseType::OUTPUT;
	case PinType::XOR_A:
		return PinBaseType::INPUT;
	case PinType::XOR_B:
		return PinBaseType::INPUT;
	case PinType::XOR_OUT:
		return PinBaseType::OUTPUT;
	case PinType::D_LATCH_DATA:
		return PinBaseType::INPUT;
	case PinType::D_LATCH_ENABLE:
		return PinBaseType::INPUT;
	case PinType::D_LATCH_OUT:
		return PinBaseType::OUTPUT;
	case PinType::DISABLED:
		return PinBaseType::CONNECTOR;
	default:
		return PinBaseType::CONNECTOR;
	}
}
