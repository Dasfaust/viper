#pragma once
#include "../Defines.hpp"
#include "../log/Logger.hpp"

class VVM
{
public:
	uint32 program_counter = 0;
	uint32 stack_pointer = 100;
	std::vector<uint32> memory;
	uint32 rType = 0;
	uint32 rData = 0;
	uint32 rRunning = 1;

	VVM()
	{
		memory.resize(4096);
	}

	uint32 getType(uint32 instruction)
	{
		uint32 type = 0xc0000000;
		type = (type & instruction) >> 30;
		return type;
	};

	uint32 getData(uint32 instruction)
	{
		uint32 data = 0x3fffffff;
		data = data & instruction;
		return data;
	};

	void fetch()
	{
		program_counter++;
	};

	void decode()
	{
		rType = getType(memory[program_counter]);
		rData = getData(memory[program_counter]);
	};

	void execute()
	{
		if (rType == 0 || rType == 2)
		{
			stack_pointer++;
			memory[stack_pointer] = rData;
		}
		else
		{
			doPrimitive();
		}
	};

	void doPrimitive()
	{
		switch(rData)
		{
		case 0: // halt
			debug("VVM: halt");
			rRunning = 0;
			break;
		case 1: // add
			debug("VVM: adding %d + %d", memory[stack_pointer - 1], memory[stack_pointer]);
			memory[stack_pointer - 1] = memory[stack_pointer - 1] + memory[stack_pointer];
			stack_pointer--;
			break;
		}
	};

	void run()
	{
		program_counter -= 1;

		while(rRunning == 1)
		{
			fetch();
			decode();
			execute();
			debug("VVM: tos: %d", memory[stack_pointer]);
		}
	};

	void loadProgram(std::vector<uint32> program)
	{
		for (uint32 i = 0; i < program.size(); i++)
		{
			memory[program_counter + i] = program[i];
		}
	};
};

namespace vvm
{
	void test()
	{
		VVM vm;
		vm.loadProgram({ 3, 4, 0x40000001, 0x40000000 });
		vm.run();
	};
};