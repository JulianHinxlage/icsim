//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "core/Circuit.h"
#include "core/DigitalCircuitSimulator.h"
#include "util/Clock.h"

#include "base/Bus.h"
#include "base/CPU8Bit.h"

#include <string>
#include <iostream>
#include <cassert>

class CPUTester {
public:
	Circuit circuit;
	CPU8Bit cpu;
	DigitalCircuitSimulator sim;
	
	std::map<std::string, int> instructionMap;


	void build() {
		cpu.circuit = &circuit;
		sim.circuit = &circuit;

		cpu.build();

		auto builder = circuit.builder();
		builder.input("clock").connect(cpu.clock);
		builder.input("mem clock").connect(cpu.memory.clock);

		circuit.prepare();


		instructionMap["NOOP"] = 0x00;
		instructionMap["HALT"] = 0x01;

		for (int i = 0; i < 16; i++) {
			instructionMap[std::string("LDL ") + std::to_string(i)] = 0x10 + i;
			instructionMap[std::string("LDH ") + std::to_string(i)] = 0x20 + i;
		}

		instructionMap["LD"] = 0x30;
		instructionMap["ST"] = 0x40;

		std::vector<std::string> registerNames = {
			"PC", "INST", "FLAG", "ACC", "ADDRL", "ADDRH", "A", "B", "C", "D", "E", "F",
		};
		for (int i = 0; i < registerNames.size(); i++) {
			auto name = registerNames[i];
			instructionMap[std::string("MV ACC ") + name] = 0x50 + i;
			instructionMap[std::string("MV ") + name + " ACC"] = 0x60 + i;
			instructionMap[std::string("ADD ") + name] = 0x70 + i;
			instructionMap[std::string("SUB ") + name] = 0x80 + i;
			instructionMap[std::string("AND ") + name] = 0x90 + i;
			instructionMap[std::string("OR ") + name] = 0xa0 + i;
			instructionMap[std::string("XOR ") + name] = 0xc0 + i;
		}

		instructionMap["NOT"] = 0xb3;
		instructionMap["NOT ACC"] = 0xb3;
	}

	void tick(bool print = false) {
		circuit.setInput("clock", 0);
		sim.simulate();

		//fetch
		circuit.setInput("clock", 1);
		sim.simulate();
		
		circuit.setInput("mem clock", 1);
		sim.simulate();
		
		circuit.setInput("mem clock", 0);
		sim.simulate();

		circuit.setInput("clock", 0);
		sim.simulate();

		//execute
		circuit.setInput("clock", 1);
		sim.simulate();

		circuit.setInput("mem clock", 1);
		sim.simulate();

		if (print) {
			printState();
		}

		circuit.setInput("mem clock", 0);
		sim.simulate();

		circuit.setInput("clock", 0);
		sim.simulate();
	}

	void printBus(Bus &bus, const std::string &name) {
		printf("%s:\t  %s (0x%X)\n", name.c_str(), bus.getValue().c_str(), bus.getIntValue());
	}

	void printState(int memoryCellCount = 8) {
		printBus(cpu.memory.dataBus, "mem data bus");
		printBus(cpu.memory.addressBus, "mem addr bus");
		printBus(cpu.dataBus, "data bus");
		printBus(cpu.addressBus, "addr bus");

		for (int i = 0; i < cpu.registerCount; i++) {
			auto &reg = cpu.registerByIndex[i]->cell;

			if (reg.name == "inst") {
				printf("%s:\t  %s (0x%X) %s\n", reg.name.c_str(), reg.getValue().c_str(), reg.getIntValue(), instructionFromCode(reg.getIntValue()).c_str());
			}
			else if (reg.name == "acc" || reg.name == "pc") {
				auto& buffer = cpu.registerByIndex[i]->bufferCell;
				printf("%s:\t  %s (0x%X)\n", reg.name.c_str(), buffer.getValue().c_str(), buffer.getIntValue());
			}
			else {
				printf("%s:\t  %s (0x%X)\n", reg.name.c_str(), reg.getValue().c_str(), reg.getIntValue());
			}
		}

		printBus(cpu.aluInA, "alu A");
		printBus(cpu.aluInB, "alu B");
		printBus(cpu.aluOut, "alu O");

		for (int i = 0; i < memoryCellCount; i++) {
			printf("mem[%i]:   %s (0x%X)\n", i, cpu.memory.cells[i].getValue().c_str(), cpu.memory.cells[i].getIntValue());
		}
		
		printf("\n");
	}

	std::vector<std::string> strSplit(const std::string& string, const std::string& delimiter, bool includeEmpty = false) {
		std::vector<std::string> parts;
		std::string token;
		int delimiterIndex = 0;
		for (char c : string) {
			if ((int)delimiter.size() == 0) {
				parts.push_back({ c, 1 });
			}
			else if (c == delimiter[delimiterIndex]) {
				delimiterIndex++;
				if (delimiterIndex == delimiter.size()) {
					if (includeEmpty || (int)token.size() != 0) {
						parts.push_back(token);
					}
					token.clear();
					delimiterIndex = 0;
				}
			}
			else {
				token += delimiter.substr(0, delimiterIndex);
				token.push_back(c);
				delimiterIndex = 0;
			}
		}
		token += delimiter.substr(0, delimiterIndex);
		if (includeEmpty || (int)token.size() != 0) {
			parts.push_back(token);
		}
		return parts;
	}

	std::string instructionFromCode(int code) {
		for (auto& i : instructionMap) {
			if (i.second == code) {
				return i.first;
			}
		}
		return "";
	}

	int codeFomrInstruction(const std::string& str) {
		if (instructionMap.contains(str)) {
			return instructionMap[str];
		}
		return 0x00;
	}

	void loadProgram(const std::string& code, int memoryOffset) {
		cpu.pc.cell.setIntValue(memoryOffset);
		auto lines = strSplit(code, "\n", false);
		for (auto& line : lines) {
			int byte = codeFomrInstruction(line);
			if (byte != 0 || line == "NOOP") {
				cpu.memory.cells[memoryOffset++].setIntValue(byte);
			}
		}
	}

	void run(bool print, int maxCycles = 1024) {
		for (int i = 0; i < maxCycles; i++) {
			tick(print);
			if (cpu.inst.cell.getIntValue() == 0x1) {
				//HALT
				break;
			}
		}
	}

	void printInfo() {
		printf("memory: %i bit (%i byte)\n", cpu.memory.wordCount * cpu.memory.dataBusSize, (cpu.memory.wordCount * cpu.memory.dataBusSize) / 8);
		printf("data bus: %i bit\n", cpu.dataBusSize);
		printf("address bus: %i bit\n", cpu.addressBusSize);

		printf("elements: %i\n", (int)circuit.structure.elements.size());
		printf("sockets: %i\n", (int)circuit.structure.sockets.size());
		printf("connections: %i\n", (int)circuit.structure.connections.size());

		int gateCount = 0;
		for (auto& e : circuit.structure.elements) {
			if (e.elementType == ElementType::GATE) {
				gateCount++;
			}
		}
		printf("gates: %i\n", gateCount);
		printf("transistors: %i\n", gateCount * 2);
		printf("\n");
	}
};

void testJmp(CPUTester&cpu) {
	std::string code = R"(
LDL 5
MV ACC A
LDL 2
MV ACC ADDRL

LD
ADD A
ST

# jump back by 4 
LDL 10
LDH 15
ADD PC
MV ACC PC
HALT
)";
	cpu.loadProgram(code, 32);
	cpu.run(false, 128);
	cpu.printState();
}

void testMem(CPUTester& cpu) {
	std::string code = R"(
LDL 3
LDH 6
MV ACC A
LDL 1
LDH 1
MV ACC B
LDH 2
LDL 0
MV ACC ADDRL

LDL 1
LDH 0
ADD ADDRL
MV ACC ADDRL
MV C ACC
ADD A
MV ACC C
ST
LDL 1
LDH 0
ADD ADDRL
MV ACC ADDRL
MV C ACC
ADD B
MV ACC C
ST

# jump
LDL 13
LDH 14
ADD PC
MV ACC PC
HALT
)";
	cpu.loadProgram(code, 0);
	cpu.run(false, 1024);
	cpu.printState(128);
}

void cpuTest() {
	CPUTester cpu;
	cpu.cpu.wordCount = 1024;
	cpu.build();
	cpu.printInfo();

	Clock clock;
	testMem(cpu);
	printf("took %fs\n", clock.elapsed());
}

int main(int argc, char* argv[]) {
	cpuTest();
	return 0;
}
