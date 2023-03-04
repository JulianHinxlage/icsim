//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "core/Circuit.h"
#include "core/Bus.h"
#include "MemoryBank.h"
#include "Register.h"

// Instruction Set
/*
NOOP 0000 0000 (0x00)
HALT 0000 0001 (0x01)
LDL  0001 xxxx (0x10) # xxxx -> ACC[0-3]
LDH  0010 xxxx (0x20) # xxxx -> ACC[4-7]
LD   0011 0000 (0x30) # [ADDR] -> ACC
ST   0100 0000 (0x40) # ACC -> [ADDR]

MV ACC ADDRL  0101 0000 (0x54) # ACC -> ADDR_L
MV ACC ADDRH  0101 0001 (0x55) # ACC -> ADDR_H
MV ACC A      0101 0010 (0x56) # ACC -> A
MV ACC B      0101 0011 (0x57) # ACC -> B

MV ADDRL ACC 0110 0000 (0x64) # ADDR_L -> ACC
MV ADDRH ACC 0110 0001 (0x65) # ADDR_H -> ACC
MV A ACC     0110 0010 (0x66) # A -> ACC
MC B ACC     0110 0011 (0x67) # B -> ACC

ADD	A		0111 0010 (0x76) # ACC + A -> ACC
SUB	A		0111 0010 (0x86) # ACC + A -> ACC
AND	A		0111 0010 (0x96) # ACC + A -> ACC
OR	A		0111 0010 (0xa6) # ACC + A -> ACC
NOT	A		0111 0010 (0xb6) # ACC + A -> ACC
XOR	A		0111 0010 (0xc6) # ACC + A -> ACC
*/

class CPU8Bit {
public:
	Circuit* circuit;
	const int addressBusSize = 16;
	const int dataBusSize = 8;
	int wordCount = 128;

	MemoryBank memory;
	Pin clock;

	Register pc;
	Register inst;
	Register flag;
	Register acc;
	Register addrL;
	Register addrH;
	Register A;
	Register B;
	Register C;
	Register D;
	Register E;
	Register F;

	Bus dataBus;
	Bus addressBus;
	Bus instBus;

	Bus accWriteBus;
	Bus pcWriteBus;

	static const int registerCount = 12;
	Register *registerByIndex[registerCount];


	Bus aluInA;
	Bus aluInB;
	Bus aluOut;
	Pin aluOpAdd;
	Pin aluOpSub;
	Pin aluOpAnd;
	Pin aluOpOr;
	Pin aluOpNot;
	Pin aluOpXor;

	void build() {
		auto builder = Pin(circuit);
		clock = builder.connector();

		//memory bank
		memory.circuit = circuit;
		memory.addressBusSize = addressBusSize;
		memory.dataBusSize = dataBusSize;
		memory.wordCount = wordCount;
		memory.build();

		//buses
		dataBus.create(circuit, dataBusSize);
		addressBus.create(circuit, addressBusSize);
		instBus.create(circuit, dataBusSize);
		accWriteBus.create(circuit, dataBusSize);
		pcWriteBus.create(circuit, addressBusSize);

		aluInA.create(circuit, dataBusSize);
		aluInB.create(circuit, dataBusSize);
		aluOut.create(circuit, dataBusSize);
		aluOpAdd = builder.connector();
		aluOpSub = builder.connector();
		aluOpAnd = builder.connector();
		aluOpOr = builder.connector();
		aluOpNot = builder.connector();
		aluOpXor = builder.connector();

		buildRegisters();
		buildControlUnit();
		buildALU();
	}

	void buildRegisters() {
		registerByIndex[0] = &pc;
		registerByIndex[1] = &inst;
		registerByIndex[2] = &flag;
		registerByIndex[3] = &acc;
		registerByIndex[4] = &addrL;
		registerByIndex[5] = &addrH;
		registerByIndex[6] = &A;
		registerByIndex[7] = &B;
		registerByIndex[8] = &C;
		registerByIndex[9] = &D;
		registerByIndex[10] = &E;
		registerByIndex[11] = &F;


		pc.init(circuit, clock, pcWriteBus, addressBus);
		pc.buildBuffered();

		inst.init(circuit, clock, dataBus, dataBus);
		inst.build();
		flag.init(circuit, clock, dataBus, dataBus);
		flag.build();
		acc.init(circuit, clock, accWriteBus, dataBus);
		acc.buildBuffered();
		addrL.init(circuit, clock, dataBus, dataBus);
		addrL.build();
		addrH.init(circuit, clock, dataBus, dataBus);
		addrH.build();
		A.init(circuit, clock, dataBus, dataBus);
		A.build();
		B.init(circuit, clock, dataBus, dataBus);
		B.build();
		C.init(circuit, clock, dataBus, dataBus);
		C.build();
		D.init(circuit, clock, dataBus, dataBus);
		D.build();
		E.init(circuit, clock, dataBus, dataBus);
		E.build();
		F.init(circuit, clock, dataBus, dataBus);
		F.build();

		pc.name = "pc";
		inst.name = "inst";
		flag.name = "flag";
		acc.name = "acc";
		addrL.name = "addrL";
		addrH.name = "addrH";
		A.name = "A";
		B.name = "B";
		C.name = "C";
		D.name = "D";
		E.name = "E";
		F.name = "F";
	}

	void buildControlUnit() {
		auto builder = Pin(circuit);
		Bus instBusL = instBus.split(0, 2);
		Bus instBusH = instBus.split(1, 2);
		Bus dataBusL = dataBus.split(0, 2);
		Bus dataBusH = dataBus.split(1, 2);
		Bus accBusL = acc.cell.split(0, 2);
		Bus accBusH = acc.cell.split(1, 2);

		//toggle fetch/execute cycle on clock
		auto executeCycle = builder.connector();
		auto fetchCycle = executeCycle.NOT();
		fetchCycle.dLatch(clock).dLatch(clock.NOT()).connect(executeCycle);

		Pin halt_signal = builder.connector();
		Pin fetch = fetchCycle.AND(halt_signal.NOT());
		auto execute = executeCycle;


		//fetch instruction
		fetch.AND(fetch).connect(memory.read);
		addressBus.AND(fetch).connect(memory.addressBus);
		memory.dataBus.AND(fetch).connect(dataBus);
		fetch.AND(fetch).AND(memory.clock).connect(inst.write);
		fetch.AND(fetch).connect(pc.read);



		//increment PC
		Bus zero;
		zero.create(circuit, addressBusSize);
		Bus incOut;
		incOut.create(circuit, addressBusSize);
		fullAdder(addressBus, zero, incOut, builder.one());
		incOut.AND(fetch).connect(pcWriteBus);
		fetch.AND(fetch).connect(pc.write);



		//decode instruction
		inst.cell.AND(execute).connect(instBus);
		Bus registerSelection = multiplexer(instBusL);
		Bus opcodeSelection = multiplexer(instBusH);
		Pin writeToSelectedRegister = builder.connector();
		Pin readFromSelectedRegister = builder.connector();

		for (int i = 0; i < 12; i++) {
			registerSelection.getPin(i).AND(readFromSelectedRegister).connect(registerByIndex[i]->read);
			registerSelection.getPin(i).AND(writeToSelectedRegister).connect(registerByIndex[i]->write);
		}

		Pin op_ldl = opcodeSelection.getPin(1).AND(clock).AND(execute);
		Pin op_ldh = opcodeSelection.getPin(2).AND(clock).AND(execute);
		Pin op_ld = opcodeSelection.getPin(3).AND(clock).AND(execute);
		Pin op_st = opcodeSelection.getPin(4).AND(clock).AND(execute);
		Pin op_mv_acc = opcodeSelection.getPin(5).AND(clock).AND(execute);
		Pin op_mv_x = opcodeSelection.getPin(6).AND(clock).AND(execute);

		Pin op_add = opcodeSelection.getPin(7).AND(clock).AND(execute);
		Pin op_sub = opcodeSelection.getPin(8).AND(clock).AND(execute);
		Pin op_and = opcodeSelection.getPin(9).AND(clock).AND(execute);
		Pin op_or = opcodeSelection.getPin(10).AND(clock).AND(execute);
		Pin op_not = opcodeSelection.getPin(11).AND(clock).AND(execute);
		Pin op_xor = opcodeSelection.getPin(12).AND(clock).AND(execute);

		instBusL.AND(op_ldl).connect(dataBusL);
		accBusH.AND(op_ldl).connect(dataBusH);

		instBusL.AND(op_ldh).connect(dataBusH);
		accBusL.AND(op_ldh).connect(dataBusL);

		op_ldl.OR(op_ldh).OR(op_ld.AND(memory.clock)).OR(op_mv_x).connect(acc.write);
		op_st.OR(op_mv_acc).connect(acc.read);
		op_mv_x.AND(op_mv_x).connect(readFromSelectedRegister);
		op_mv_acc.AND(op_mv_acc).connect(writeToSelectedRegister);

		//special case: read and write PC
		addressBus.split(0, 2).AND(pc.read.AND(execute)).connect(dataBus);
		dataBus.AND(pc.write.AND(execute)).connect(pcWriteBus.split(0, 2));

		op_ld.AND(op_ld).connect(memory.read);
		addressBus.AND(op_ld).connect(memory.addressBus);
		memory.dataBus.AND(op_ld).connect(dataBus);

		op_st.AND(op_st).connect(memory.write);
		addressBus.AND(op_st).connect(memory.addressBus);
		dataBus.AND(op_st).connect(memory.dataBus);

		addrL.cell.AND(op_ld.OR(op_st)).connect(addressBus.split(0, 2));
		addrH.cell.AND(op_ld.OR(op_st)).connect(addressBus.split(1, 2));


		//arithmetic
		Pin accWriteFromAlu = builder.connector();
		dataBus.AND(accWriteFromAlu.NOT()).connect(accWriteBus);
		aluOut.AND(accWriteFromAlu).connect(accWriteBus);

		op_add.AND(op_add).connect(aluOpAdd);
		op_sub.AND(op_sub).connect(aluOpSub);
		op_and.AND(op_and).connect(aluOpAnd);
		op_or.AND(op_or).connect(aluOpOr);
		op_not.AND(op_not).connect(aluOpNot);
		op_xor.AND(op_xor).connect(aluOpXor);
		auto any_alu = op_add.OR(op_sub).OR(op_and).OR(op_or).OR(op_not).OR(op_xor);
		any_alu.AND(any_alu).connect(accWriteFromAlu);
		any_alu.AND(any_alu).connect(acc.write);
		any_alu.AND(any_alu).connect(readFromSelectedRegister);

		acc.cell.AND(any_alu.AND(any_alu)).connect(aluInA);
		dataBus.AND(any_alu.AND(any_alu)).connect(aluInB);

		auto op_halt = opcodeSelection.getPin(0).AND(clock).AND(execute).AND(registerSelection.getPin(1));
		dLatch(op_halt, clock.AND(execute)).connect(halt_signal);
	}

	void buildALU() {
		Pin builder = Pin(circuit);
		//add
		Bus addOut;
		addOut.create(circuit, dataBusSize);
		fullAdder(aluInA, aluInB, addOut, builder.zero());
		addOut.AND(aluOpAdd).connect(aluOut);

		//sub
		Bus subOut;
		subOut.create(circuit, dataBusSize);
		fullAdder(aluInA, aluInB, subOut, builder.zero());
		subOut.AND(aluOpSub).connect(aluOut);

		//AND
		Bus andOut;
		andOut.create(circuit, dataBusSize);
		for (int i = 0; i < aluInA.size(); i++) {
			aluInA.getPin(i).AND(aluInB.getPin(i)).connect(andOut.getPin(i));
		}
		andOut.AND(aluOpAnd).connect(aluOut);

		//OR
		Bus orOut;
		orOut.create(circuit, dataBusSize);
		for (int i = 0; i < aluInA.size(); i++) {
			aluInA.getPin(i).OR(aluInB.getPin(i)).connect(orOut.getPin(i));
		}
		orOut.AND(aluOpOr).connect(aluOut);

		//NOT
		Bus notOut;
		notOut.create(circuit, dataBusSize);
		for (int i = 0; i < aluInA.size(); i++) {
			aluInA.getPin(i).NOT().connect(notOut.getPin(i));
		}
		notOut.AND(aluOpNot).connect(aluOut);

		//XOR
		Bus xorOut;
		xorOut.create(circuit, dataBusSize);
		for (int i = 0; i < aluInA.size(); i++) {
			aluInA.getPin(i).XOR(aluInB.getPin(i)).connect(xorOut.getPin(i));
		}
		xorOut.AND(aluOpXor).connect(aluOut);
	}

	Pin fullAdder(Bus& aBus, Bus& bBus, Bus& outBus, Pin carry) {
		for (int i = 0; i < aBus.size(); i++) {
			auto a = aBus.getPin(i);
			auto b = bBus.getPin(i);
			auto o = outBus.getPin(i);

			a.XOR(b).XOR(carry).connect(o);
			carry = a.AND(b).OR(carry.AND(a.XOR(b)));
		}
		return carry;
	}

};
