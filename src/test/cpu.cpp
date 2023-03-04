//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#include "cpu/CPU8Bit.h"
#include "util/Clock.h"
#include <string>

class CPUTester {
public:
	Circuit circuit;
	CPU8Bit cpu;

	std::map<std::string, int> instructionMap;

	Pin clock;
	Pin memoryClock;
	//currently 22 needed (26 for sorted)
	int timeUnitsPerClockCycle = 26;
	int timeUnitsSpentTotal = 0;
	int clockCyclesTotal = 0;
	int instructionsTotal = 0;

	void build() {
		cpu.circuit = &circuit;

		cpu.build();

		auto builder = Pin(&circuit);
		clock = builder.input();
		memoryClock = builder.input();
		clock.connect(cpu.clock);
		memoryClock.connect(cpu.memory.clock);

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

	void sim() {
		timeUnitsSpentTotal += circuit.simulate(timeUnitsPerClockCycle);
		//printf("needed: %i\n", circuit.simulate(timeUnitsPerClockCycle));
	}

	void tick(bool print = false) {
		clock.setValue(0);
		sim();
		
		//fetch
		clock.setValue(1);
		sim();

		memoryClock.setValue(1);
		sim();

		memoryClock.setValue(0);
		sim();

		clock.setValue(0);
		sim();

		//execute
		clock.setValue(1);
		sim();

		memoryClock.setValue(1);
		sim();

		if (print) {
			printState();
		}

		memoryClock.setValue(0);
		sim();

		clock.setValue(0);
		sim();

		instructionsTotal++;
		clockCyclesTotal += 2;
	}

	void printBus(Bus& bus, const std::string& name) {
		printf("%s:\t  %s (0x%02X)\n", name.c_str(), bus.getStrValue().c_str(), (int)bus.getValue());
	}

	void printState(int memoryCellCount = 8) {
		printBus(cpu.memory.dataBus, "mem data bus");
		printBus(cpu.memory.addressBus, "mem addr bus");
		printBus(cpu.dataBus, "data bus");
		printBus(cpu.addressBus, "addr bus");

		printBus(cpu.instBus, "instruction bus");


		for (int i = 0; i < cpu.registerCount; i++) {
			auto& reg = *cpu.registerByIndex[i];
			auto& cell = reg.cell;
		
			if (reg.name == "inst") {
				printf("%s:\t  %s (0x%02X) %s\n", reg.name.c_str(), cell.getStrValue().c_str(), (int)cell.getValue(), instructionFromCode(cell.getValue()).c_str());
			}
			else if (reg.name == "acc" || reg.name == "pc") {
				auto& buffer = reg.bufferCell;
				printf("%s:\t  %s (0x%02X)\n", reg.name.c_str(), buffer.getStrValue().c_str(), (int)buffer.getValue());
			}
			else {
				printf("%s:\t  %s (0x%02X)\n", reg.name.c_str(), cell.getStrValue().c_str(), (int)cell.getValue());
			}
		}

		printBus(cpu.aluInA, "alu A");
		printBus(cpu.aluInB, "alu B");
		printBus(cpu.aluOut, "alu O");

		memoryCellCount = std::min(memoryCellCount, (int)cpu.memory.cells.size());
		for (int i = 0; i < memoryCellCount; i++) {
			printf("mem[0x%02X]:   %s (0x%02X)\n", i, cpu.memory.cells[i].getStrValue().c_str(), (int)cpu.memory.cells[i].getValue());
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
		cpu.pc.cell.setValue(memoryOffset);
		auto lines = strSplit(code, "\n", false);
		for (auto& line : lines) {
			int byte = codeFomrInstruction(line);
			if (byte != 0 || line == "NOOP") {
				if (cpu.memory.cells.size() > memoryOffset) {
					cpu.memory.cells[memoryOffset++].setValue(byte);
				}
			}
		}
	}

	void run(bool print, int maxCycles = 1024) {
		for (int i = 0; i < maxCycles; i++) {
			tick(print);
			if (cpu.inst.cell.getValue() == 0x1) {
				//HALT
				break;
			}
		}
	}

	void printInfo() {
		printf("memory: %i bit (%i byte)\n", cpu.memory.wordCount * cpu.memory.dataBusSize, (cpu.memory.wordCount * cpu.memory.dataBusSize) / 8);
		printf("data bus: %i bit\n", cpu.dataBusSize);
		printf("address bus: %i bit\n", cpu.addressBusSize);

		printf("gates: %i\n", circuit.getGateCount());
		printf("lines: %i\n", circuit.getLineCount());
		printf("pins:  %i\n", circuit.getPinCount());

		printf("transistors: %i\n", circuit.getGateCount() * 2);
		printf("\n");
	}
};

void testCPU() {
	CPUTester tester;
	tester.cpu.wordCount = 256;

	tester.build();
	tester.circuit.setGateDelay(GateType::D_LATCH, 3);
	tester.circuit.setSimulationMode(false);

	tester.printInfo();

	Clock clock;

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

	tester.loadProgram(code, 0);

	clock.reset();
	tester.run(false, 2000);
	tester.printState(64);
	
	float time = clock.elapsed();
	printf("took %fs\n", time);
	printf("time units spent: %i\n", tester.timeUnitsSpentTotal);
	printf("sim time: %i\n", tester.circuit.getSimulationTime());
	printf("clock cycles: %i\n", tester.clockCyclesTotal);
	printf("instructions: %i\n", tester.instructionsTotal);
	printf("speed: %.3f kH\n", (tester.clockCyclesTotal / time) / 1000);
	printf("unit took: %.0f ns\n", (time / tester.timeUnitsSpentTotal) * 1000 * 1000 * 1000);
	printf("units per instruction: %i\n", tester.timeUnitsSpentTotal / tester.instructionsTotal);
	printf("sim time units per instruction: %i\n", tester.circuit.getSimulationTime() / tester.instructionsTotal);
}

int main() {
	testCPU();
	return 0;
}
