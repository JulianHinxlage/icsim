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
		builder.input().transistor(b).connection(c).output();

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
		d.connection(b).connection(c).output();

		orCircuit.prepare();
	}

	// NAND Gate
	Circuit nandCircuit;
	{
		auto builder = nandCircuit.builder();
		
		auto a = builder.constant(5);
		auto b = a.connection(builder.output());
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
		b.connection(c).connection(d);
		a.connection(builder.output());

		norCircuit.prepare();
	}

	// NOT Gate
	Circuit notCircuit;
	{
		auto builder = notCircuit.builder();
		
		auto a = builder.constant(5);
		auto b = builder.output().connection(a);
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

	auto sp = builder.connection();
	auto rp = builder.connection();

	s.NOR(sp).connection(rp);
	r.NOR(rp).connection(sp).output();


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

	auto sp = builder.connection();
	auto rp = builder.connection();

	s.NAND(sp).connection(rp).output();
	r.NAND(rp).connection(sp);

	circuit.prepare();

	Evaluator eval;
	eval.evaluate(&circuit, { {0, 1} });

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
