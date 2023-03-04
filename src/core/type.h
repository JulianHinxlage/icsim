//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include <cstdint>

typedef int32_t Index;

enum class GateType : uint8_t {
	CONNECTOR,
	OUTPUT,
	BUF,
	NOT,
	OR,
	AND,
	NOR,
	NAND,
	XOR,
	D_LATCH,
	GATE_TYPE_COUNT,
};

enum class PinBaseType : uint8_t {
	CONNECTOR,
	INPUT,
	OUTPUT,
};

enum class PinType : uint8_t {
	CONNECTOR,
	OUTPUT,
	BUF_IN,
	BUF_OUT,
	NOT_IN,
	NOT_OUT,
	OR_A,
	OR_B,
	OR_OUT,
	AND_A,
	AND_B,
	AND_OUT,
	NOR_A,
	NOR_B,
	NOR_OUT,
	NAND_A,
	NAND_B,
	NAND_OUT,
	XOR_A,
	XOR_B,
	XOR_OUT,
	D_LATCH_DATA,
	D_LATCH_ENABLE,
	D_LATCH_OUT,
	DISABLED,
	PIN_TYPE_COUNT,
};

PinBaseType getPinBaseType(PinType type);
