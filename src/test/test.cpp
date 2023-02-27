//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "core/Circuit.h"
#include "core/Evaluator.h"

void transistorGateTest() {
	// AND Gate
	Circuit andCircuit;
	{
		auto builder = andCircuit.builder();

		auto a = builder.constant(5);
		auto b = builder.input().transistor(a);
		auto c = builder.constant(0);
		builder.input().transistor(b).connect(c).output();

		andCircuit.prepare();
	}

	// OR Gate
	Circuit orCircuit;
	{
		auto builder = orCircuit.builder();
		
		auto a = builder.constant(5);
		auto b = builder.input().transistor(a);
		auto c = builder.input().transistor(a);
		auto d = builder.constant(0);
		d.connect(b).connect(c).output();

		orCircuit.prepare();
	}

	// NAND Gate
	Circuit nandCircuit;
	{
		auto builder = nandCircuit.builder();
		
		auto a = builder.constant(5);
		auto b = a.connect(builder.output());
		auto c = builder.input().transistor(b);
		builder.input().transistor(c).constant(0);

		nandCircuit.prepare();
	}

	// NOR Gate
	Circuit norCircuit;
	{
		auto builder = norCircuit.builder();
		
		auto a = builder.constant(5);
		auto b = builder.input().transistor(a);
		auto c = builder.input().transistor(a);
		auto d = builder.constant(0);
		b.connect(c).connect(d);
		a.connect(builder.output());

		norCircuit.prepare();
	}

	// NOT Gate
	Circuit notCircuit;
	{
		auto builder = notCircuit.builder();
		
		auto a = builder.constant(5);
		auto b = builder.output().connect(a);
		auto c = builder.input().transistor(b).constant(0);

		notCircuit.prepare();
	}


	Evaluator eval;
	//eval.simulationType = SimulationType::ANALOG;

	printf("AND:\n");
	eval.evaluate(&andCircuit);
	eval.printTable();
	printf("\n");

	printf("OR:\n");
	eval.evaluate(&orCircuit);
	eval.printTable();
	printf("\n");

	printf("NAND:\n");
	eval.evaluate(&nandCircuit);
	eval.printTable();
	printf("\n");

	printf("NOR:\n");
	eval.evaluate(&norCircuit);
	eval.printTable();
	printf("\n");

	printf("NOT:\n");
	eval.evaluate(&notCircuit);
	eval.printTable();
	printf("\n");
}

void gateTest() {
	Circuit circuit;
	auto builder = circuit.builder();

	auto a = builder.input();
	auto b = builder.input();
	auto c = builder.input();

	auto ab = a.AND(b);
	ab.output();
	ab.OR(c).output();

	circuit.prepare();

	printf("gate:\n");
	Evaluator eval;
	eval.evaluate(&circuit);

	bool valid = eval.checkTable({
		{0, 0},
		{0, 0},
		{0, 0},
		{1, 1},
		{0, 1},
		{0, 1},
		{0, 1},
		{1, 1},
	});

	eval.printTable();
	if (!valid) {
		printf("not valid\n");
	}
	printf("\n");
}

void rsLatchTest() {
	Circuit circuit;
	auto builder = circuit.builder();

	//RS flip flop / latch
	auto s = builder.input();
	auto r = builder.input();

	auto sp = builder.connector();
	auto rp = builder.connector();

	s.NOR(sp).connect(rp);
	r.NOR(rp).connect(sp).output();


	circuit.prepare();

	Evaluator eval;
	//eval.evaluate(&circuit, { {0, 1} });

	eval.evaluate(&circuit, {
		{0, 0},
		{1, 0},
		{0, 0},
		{0, 1},
		{0, 0},
		{1, 0},
		{0, 0},
	});
	bool valid = eval.checkTable({
		{0},
		{1},
		{1},
		{0},
		{0},
		{1},
		{1},
	});

	printf("RS latch:\n");
	eval.printTable();
	if (!valid) {
		printf("not valid!\n");
	}
	printf("\n");
}

void dLatchTest() {
	Circuit circuit;
	auto builder = circuit.builder();

	//D flip flop / latch
	auto d = builder.input();
	auto c = builder.input();

	auto s = d.NAND(c);
	auto r = d.NOT().NAND(c);

	auto sp = builder.connector();
	auto rp = builder.connector();

	r.NAND(rp).connect(sp);
	s.NAND(sp).connect(rp).output();

	circuit.prepare();

	Evaluator eval;

	eval.evaluate(&circuit, {
		{0, 0},
		{1, 0},
		{1, 1},
		{0, 0},
		{1, 0},
		{0, 1},
		{1, 0},
		{0, 0},
	});
	bool valid = eval.checkTable({
		{0},
		{0},
		{1},
		{1},
		{1},
		{0},
		{0},
		{0},
	});

	printf("D latch:\n");
	eval.printTable();
	if (!valid) {
		printf("not valid!\n");
	}
	printf("\n");
}

int main(int argc, char* argv[]) {
	transistorGateTest();
	gateTest();
	rsLatchTest();
	dLatchTest();
	return 0;
}
