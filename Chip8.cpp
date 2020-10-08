#pragma warning (disable : 4996 26812 26451 26495)
#include <array>
#include <stdexcept>
#include <unordered_map>

using std::array;

class Chip8 {
private:

#pragma region Instructions
	//-------------------------------INSTRUCTIONS---------------------------------------

	void IXXXX() {
		printf("Instruction not found (%04X)\n\n", opcode);
		pc += 2;
	}

	void I00E0() { //Clear display
		display.fill(0);
		clear = true;
		pc += 2;
		printf("Executing 00E0\n");
	}

	void I00EE() { //RET: Ende einer Subroutine, der sp wird um 1 erniedrigt, sodass pc wieder zum CALL-Punkt der Subroutine kommt und weitermachen kann
		sp--;
		pc = stack[sp] + 2;
		printf("Executing 00EE\n");
	}

	void I1NNN() { //Jump to NNN
		pc = NNN;
		printf("Executing 1NNN\n");
	}

	void I2NNN() { //CALL: pc geht auf Instruction einer anderen Region udn führt diese aus
		stack[sp] = pc; //Stack speichert Ort von pc in Stack[sp] und erhöht sp um 1, sodass pc zurück kommen kann, wenn das Return vom Subroutine kommt
		sp++;
		pc = NNN; //Execute subroutine at NNN
		printf("Executing 00E0\n");
	}

	void I3XKK() { //Skip next instruction, if V[X] == KK
		V[X] == KK ? pc += 4 : pc += 2;
		printf("Executing 3XKK\n");
	}

	void I4XKK() { //Skip next instruction, if V[X] != KK
		V[X] != KK ? pc += 4 : pc += 2;
		printf("Executing 4XKK\n");
	}

	void I5XY0() { //Skip next instruction if V[X] == V[Y]
		V[X] == V[Y] ? pc += 4 : pc += 2;
		printf("Executing 5XY0\n");
	}

	void I6XKK() {
		V[X] = KK;
		pc += 2;
		printf("Executing 6XKK\n");
	}

	void I7XKK() {
		if ((V[X] + KK) > 0xFF) { //Register cant store values higher than 0xFF
			V[X] = (V[X] + KK) % 0x100; //300 would be 44 (300 % 256)
		}
		else {
			V[X] += KK; //0x7XKK: Add KK to value of register[X]
		}
		pc += 2;
		printf("Executing 7XKK\n");
	}

	void I8XY0() {
		V[X] = Y;
		pc += 2;
		printf("Executing 8XY0\n");
	}

	void I8XY1() {
		V[X] |= V[Y];
		pc += 2;
		printf("Executing 8XY1\n");
	}

	void I8XY2() {
		V[X] &= V[Y];
		pc += 2;
		printf("Executing 8XY2\n");
	}

	void I8XY3() {
		V[X] ^= V[Y];
		pc += 2;
		printf("Executing 8XY3\n");
	}

	void I8XY4() {
		if ((V[X] + V[Y]) > 0xFF) {
			V[X] = (V[X] + V[Y]) % 0x100;
			V[0xF] = 1; //if new value is bigger than 255 (0xFF) V[15] will be 1
		}
		else {
			V[X] += V[Y];
			V[0xF] = 0; //if new value is smaller than 256 (0x100) V[15] will be 0
		}
		pc += 2;
		printf("Executing 8XY4\n");
	}

	void I8XY5() {
		if ((V[X] - V[Y]) < 0) {
			V[X] = 256 + (V[X] - V[Y]);
			V[0xF] = 0; //if new value is smaller than 0 V[15] will be 0
		}
		else {
			V[X] -= V[Y];
			V[0xF] = 1; //if new value is bigger than 0 V[15] will be 1
		}
		pc += 2;
		printf("Executing 8XY5\n");
	}

	void I8XY6() {
		V[0xf] = V[Y] & 1; //0x8XY6: V[15] will be least significant bit of Register[Y]
		V[X] = (V[Y] >> 1); //V[X] gets Value from rightshifted V[Y]
		pc += 2;
		printf("Executing 8XY6\n");
	}

	void I8XY7() {
		if ((V[Y] - V[X]) < 0) {
			V[X] = 256 + (V[Y] - V[X]);
			V[0xF] = 0; //if new value is smaller than 0 V[15] will be 0
		}
		else {
			V[X] = V[Y] - V[X];
			V[0xF] = 1; //if new value is bigger than 0 V15] will be 1
		}
		pc += 2;
		printf("Executing 8XY7\n");
	}

	void I8XYE() {
		V[0xf] = (V[Y] >= 128) ? 1 : 0; //0x8XY6: V[15] will be most significant bit of V[Y]
		V[X] = V[Y] << 1; //V[X] gets Value from leftshifted V[Y]
		pc += 2;
		printf("Executing 8XYE\n");
	}

	void I9XY0() {
		V[X] != V[Y] ? pc += 4 : pc += 2;
		printf("Executing 9XY0\n");
	}

	void IANNN() {
		index = NNN;
		pc += 2;
		printf("Executing ANNN\n");
	}

	void IBNNN() {
		pc = NNN + V[0];
		printf("Executing BNNN\n");
	}

	void ICXKK() {
		V[X] = ((rand() % 0x100) & KK); //0xCXKK Set V[X] to a random number between 0 and 255 but bitwise AND this number with KK
		pc += 2;
		printf("Executing CXKK\n");
	}

	void IDXYN() {
		/*
					Draw a Sprite, set VF to 1 if active pixel become unactive, 0 if not
						X:      V[X]
						Y:      V[Y]
						Width:  8
						Height: (opcode  &0x000F)

						Sprite starts at memory[index]

				*/

		V[0xF] = 0;

		for (int height = 0; height < (opcode & 0x000F); height++) {
			int spriteRow = memory[index + height]; //Get a byte of a sprite

			for (int width = 0; width < 8; width++) {
				int pixel = spriteRow & (0x80 >> width); //0x80 = 1000 0000: Check bit for bit from byte if it's 1

				if (pixel != 0) { //TODO: Maybe add & &V[X] < 64 & &V[Y] < 32
					int pixelPos = (width + V[X]) + ((height + V[Y]) * 64);

					try {
						if (display.at(pixelPos) == 1) V[0xF] = 1;
						display.at(pixelPos) ^= 1;
					}
					catch (std::out_of_range oor) {
						printf("\nOut of range (Expected, idk how to solve, but it works like this)\n");
					}
				}
			}
		}
		needToDraw = true;
		pc += 2;
		printf("Executing DXYN\n");
	}

	void IEX9E() {
		keys[V[X]] ? pc += 4 : pc += 2;
		printf("Executing EX9E\n");
	}

	void IEXA1() {
		!keys[V[X]] ? pc += 4 : pc += 2;
		printf("Executing EXA1\n");
	}

	void IFX07() {
		V[X] = delayTimer;
		pc += 2;
		printf("Executing FX07\n");
	}

	void IFX0A() { //Wait for input and save next input to V[X]
		for (int i = 0; i < 16; i++) {
			if (keys[i]) {
				V[X] = i;
				pc += 2;
			}
		}
		printf("\nFX0A\nWaiting for input...\n");
	}

	void IFX15() {
		delayTimer = V[X];
		pc += 2;
		printf("Executing FX15\n");
	}

	void IFX18() {
		soundTimer = V[X];
		pc += 2;
		printf("Executing FX18\n");
	}

	void IFX1E() {
		index += V[X];
		pc += 2;
		printf("Executing FX1E\n");
	}

	void IFX29() {
		index = 0x050 + (5 * V[X]); //0xFX29: Stellt Index auf V[X]tes Zeichen des Fontsets
		pc += 2;
		printf("Executing FX29\n");
	}

	void IFX33() {
		//0xFX33: V[X] als BCD in memory[index], memory[index + 1] und memory[index + 2] speichern
		memory[index] = V[X] % 10;              // 00X
		memory[index + 1] = (V[X] / 10) % 10;   // 0X0
		memory[index + 2] = V[X] / 100;         // X00

		pc += 2;
		printf("Executing FX33\n");
	}

	void IFX55() { //0xFX55: Store Registers V0 - VX to in memory, starting at index
		for (int i = 0; i <= V[X]; i++) {
			memory[index + i] = V[i];
		}
		pc += 2;
		printf("Executing FX55\n");
	}

	void IFX65() { //0xFX65: Set Value of Registers V0 - VX to memory with index + X
		for (int i = 0; i <= V[X]; i++) {
			V[i] = memory[index + i];
		}
		pc += 2;
		printf("Executing FX65\n");
	}

	//-------------------------------INSTRUCTIONS---------------------------------------

#pragma endregion

public:

	array<uint8_t, 0x1000> memory;
	array<uint8_t, 16> V;           //Register: Small CPU storage for calculations
	uint16_t index;                 //Can store a memory address
	uint16_t pc = 0x200;            //points to next instruction
	array<uint16_t, 16> stack;      //Holds order of execution for subroutines. Holds pc value, so that it can come back after Subroutine
	uint8_t sp;                     //on most recent value in stack
	uint8_t delayTimer;             //decrementing in 60hz until 0
	uint8_t soundTimer;             //  ""  ""  ""
	array<bool, 16> keys;
	array<uint8_t, 64 * 32> display;
	uint16_t opcode;
	bool needToDraw;
	bool clear;

	//function pointer. Switch statement ist schneller, wollt aber was lernen
	typedef void (Chip8::* Instruction)();

	//Function pointer array für 0x0 instructions
	array<Instruction, 2> I_0 = { &Chip8::I00E0, &Chip8::I00EE };
	void DoI_0() {
		if (opcode == 0x00EE || opcode == 0x00E0) { //Muss man leider machen, weil I0NNN existiert und nicht benutzt wird
			(this->*I_0.at((KK & 0b10) >> 1))(); //0xE0 ends with 0b00, 0xEE ends with 0b10
		}
		else IXXXX();
	}

	array<Instruction, 15> I_8 =
	{
		&Chip8::I8XY0, &Chip8::I8XY1, &Chip8::I8XY2, &Chip8::I8XY3, &Chip8::I8XY4, &Chip8::I8XY5, &Chip8::I8XY6,
		&Chip8::I8XY7, &Chip8::IXXXX , &Chip8::IXXXX, &Chip8::IXXXX, &Chip8::IXXXX, &Chip8::IXXXX, &Chip8::IXXXX, &Chip8::I8XYE
	};
	void DoI_8() {
		(this->*I_8.at(KK & 0x0F))(); //8-14 würde nix auslösen
	}

	array<Instruction, 2> I_E = { &Chip8::IEX9E, &Chip8::IEXA1 };
	void DoI_E() {
		(this->*I_E.at(KK & 0b1))(); //0 -> IEX9E, 1 -> IEXA1
	}

	std::unordered_map<int, Instruction> map; //Slower than array filled with nullptr but not as big
	void DoI_F() {
		(this->*map.at(KK))();
	}

	array<Instruction, 16> I =
	{
		&Chip8::DoI_0,
		&Chip8::I1NNN, &Chip8::I2NNN, &Chip8::I3XKK, &Chip8::I4XKK, &Chip8::I5XY0, &Chip8::I6XKK, &Chip8::I7XKK,
		&Chip8::DoI_8,
		&Chip8::I9XY0, &Chip8::IANNN, &Chip8::IBNNN, &Chip8::ICXKK, &Chip8::IDXYN,
		&Chip8::DoI_E,
		&Chip8::DoI_F
	};

	array<int, 80> fontSet;

	uint8_t A, X, Y, KK;
	uint16_t NNN;

	Chip8() {
		fontSet =
		{
			0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
			0x20, 0x60, 0x20, 0x20, 0x70, // 1
			0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
			0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
			0x90, 0x90, 0xF0, 0x10, 0x10, // 4
			0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
			0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
			0xF0, 0x10, 0x20, 0x40, 0x40, // 7
			0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
			0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
			0xF0, 0x90, 0xF0, 0x90, 0x90, // A
			0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
			0xF0, 0x80, 0x80, 0x80, 0xF0, // C
			0xE0, 0x90, 0x90, 0x90, 0xE0, // D
			0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
			0xF0, 0x80, 0xF0, 0x80, 0x80  // F
		};

		//Reset values
		memory = { 0 };
		V = { 0 };
		index = 0;
		pc = 0x200;
		stack = { 0 };
		sp = 0;
		delayTimer = 0;
		soundTimer = 0;
		keys = { false };
		display = { 0 };
		opcode = 0;
		needToDraw = false;
		clear = false;

		A = 0;
		X = 0;
		Y = 0;
		KK = 0;
		NNN = 0;

		//Load font into memory
		for (int i = 0; i < 80; ++i)
		{
			memory[0x050 + i] = fontSet[i];
		}

		//fill unordered_map
		map.insert({ 0x07, &Chip8::IFX07 });
		map.insert({ 0x0A, &Chip8::IFX0A });
		map.insert({ 0x15, &Chip8::IFX15 });
		map.insert({ 0x18, &Chip8::IFX18 });
		map.insert({ 0x1E, &Chip8::IFX1E });
		map.insert({ 0x29, &Chip8::IFX29 });
		map.insert({ 0x33, &Chip8::IFX33 });
		map.insert({ 0x55, &Chip8::IFX55 });
		map.insert({ 0x65, &Chip8::IFX65 });
	}

	void LoadRom(const char* path = "rom//IBM.ch8") {
		FILE* file;
		file = fopen(path, "rb");
		fread(&memory[pc], 0xfff, 1, file);
		fclose(file);
	}

	void Cycle() {
		//get opcode
		opcode = (memory[pc] << 8) | memory[pc + 1]; //Zwei einzelne Bytes zusammenführen

		A = opcode >> 12;             //0xA000
		X = (opcode & 0x0F00) >> 8;   //0x0X00
		Y = (opcode & 0x00F0) >> 4;   //0x00Y0
		KK = opcode & 0x00FF;         //0x00KK
		NNN = opcode & 0x0FFF;        //0x0NNN

		printf("\nOpcode:   %04X \n", opcode);

		try {
			(this->*I.at(A))();
		}
		catch (std::out_of_range oorr) {
			IXXXX();
		}
	}
};