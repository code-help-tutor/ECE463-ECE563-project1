WeChat: cstutorcs
QQ: 749389476
Email: tutorcs@163.com
#include "sim_pipe.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <iomanip>
#include <map>

//#define DEBUG

using namespace std;

//used for debugging purposes
static const char *reg_names[NUM_SP_REGISTERS] = {"PC", "NPC", "IR", "A", "B", "IMM", "COND", "ALU_OUTPUT", "LMD"};
static const char *stage_names[NUM_STAGES] = {"IF", "ID", "EX", "MEM", "WB"};
static const char *instr_names[NUM_OPCODES] = {"LW", "SW", "ADD", "ADDI", "SUB", "SUBI", "XOR", "BEQZ", "BNEZ", "BLTZ", "BGTZ", "BLEZ", "BGEZ", "JUMP", "EOP", "NOP"};

/* =============================================================

   HELPER FUNCTIONS

   ============================================================= */


/* converts integer into array of unsigned char - little indian */
inline void int2char(unsigned value, unsigned char *buffer){
	memcpy(buffer, &value, sizeof value);
}

/* converts array of char into integer - little indian */
inline unsigned char2int(unsigned char *buffer){
	unsigned d;
	memcpy(&d, buffer, sizeof d);
	return d;
}

/* implements the ALU operations */
unsigned alu(unsigned opcode, unsigned a, unsigned b, unsigned imm, unsigned npc){
	switch(opcode){
			case ADD:
				return (a+b);
			case ADDI:
				return(a+imm);
			case SUB:
				return(a-b);
			case SUBI:
				return(a-imm);
			case XOR:
				return(a ^ b);
			case LW:
			case SW:
				return(a + imm);
			case BEQZ:
			case BNEZ:
			case BGTZ:
			case BGEZ:
			case BLTZ:
			case BLEZ:
			case JUMP:
				return(npc+imm);
			default:	
				return (-1);
	}
}

/* returns true if the instruction is a taken branch/jump */
bool taken_branch(opcode_t opcode, unsigned a){
        switch(opcode){
                case BEQZ:
                        if (a==0) return true;
                        break;
                case BNEZ:
                        if (a!=0) return true;
                        break;
                case BGTZ:
                        if ((int)a>0)  return true;
                        break;
                case BGEZ:
                        if ((int)a>=0) return true;
                        break;
                case BLTZ:
                        if ((int)a<0)  return true;
                        break;
                case BLEZ:
                        if ((int)a<=0) return true;
                        break;
                case JUMP:
                        return true;
                default:
                        return false;
        }
        return false;
}

/* return the kind of instruction encoded */

bool is_branch(opcode_t opcode){
        return (opcode == BEQZ || opcode == BNEZ || opcode == BLTZ || opcode == BLEZ || opcode == BGTZ || opcode == BGEZ || opcode == JUMP);
}

bool is_memory(opcode_t opcode){
        return (opcode == LW || opcode == SW);
}

bool is_int_r(opcode_t opcode){
        return (opcode == ADD || opcode == SUB || opcode == XOR);
}

bool is_int_imm(opcode_t opcode){
        return (opcode == ADDI || opcode == SUBI);
}


/* =============================================================

   CODE PROVIDED - NO NEED TO MODIFY FUNCTIONS BELOW

   ============================================================= */

/* loads the assembly program in file "filename" in instruction memory at the specified address */
void load_program(const char *filename, unsigned base_address){

   /* initializing the base instruction address */
   mips.instr_base_address = base_address;

   /* creating a map with the valid opcodes and with the valid labels */
   map<string, opcode_t> opcodes; //for opcodes
   map<string, unsigned> labels;  //for branches
   for (int i=0; i<NUM_OPCODES; i++)
	 opcodes[string(instr_names[i])]=(opcode_t)i;

   /* opening the assembly file */
   ifstream fin(filename, ios::in | ios::binary);
   if (!fin.is_open()) {
      cerr << "error: open file " << filename << " failed!" << endl;
      exit(-1);
   }

   /* parsing the assembly file line by line */
   string line;
   unsigned instruction_nr = 0;
   while (getline(fin,line)){
	// set the instruction field
	char *str = const_cast<char*>(line.c_str());

  	// tokenize the instruction
	char *token = strtok (str," \t");
	map<string, opcode_t>::iterator search = opcodes.find(token);
        if (search == opcodes.end()){
		// this is a label for a branch - extract it and save it in the labels map
		string label = string(token).substr(0, string(token).length() - 1);
		labels[label]=instruction_nr;
                // move to next token, which must be the instruction opcode
		token = strtok (NULL, " \t");
		search = opcodes.find(token);
		if (search == opcodes.end()) cout << "ERROR: invalid opcode: " << token << " !" << endl;
	}
	mips.instr_memory[instruction_nr].opcode = search->second;

	//reading remaining parameters
	char *par1;
	char *par2;
	char *par3;
	switch(mips.instr_memory[instruction_nr].opcode){
		case ADD:
		case SUB:
		case XOR:
			par1 = strtok (NULL, " \t");
			par2 = strtok (NULL, " \t");
			par3 = strtok (NULL, " \t");
			mips.instr_memory[instruction_nr].dest = atoi(strtok(par1, "R"));
			mips.instr_memory[instruction_nr].src1 = atoi(strtok(par2, "R"));
			mips.instr_memory[instruction_nr].src2 = atoi(strtok(par3, "R"));
			break;
		case ADDI:
		case SUBI:
			par1 = strtok (NULL, " \t");
			par2 = strtok (NULL, " \t");
			par3 = strtok (NULL, " \t");
			mips.instr_memory[instruction_nr].dest = atoi(strtok(par1, "R"));
			mips.instr_memory[instruction_nr].src1 = atoi(strtok(par2, "R"));
			mips.instr_memory[instruction_nr].immediate = strtoul (par3, NULL, 0); 
			break;
		case LW:
			par1 = strtok (NULL, " \t");
			par2 = strtok (NULL, " \t");
			mips.instr_memory[instruction_nr].dest = atoi(strtok(par1, "R"));
			mips.instr_memory[instruction_nr].immediate = strtoul(strtok(par2, "()"), NULL, 0);
			mips.instr_memory[instruction_nr].src1 = atoi(strtok(NULL, "R"));
			break;
		case SW:
			par1 = strtok (NULL, " \t");
			par2 = strtok (NULL, " \t");
			mips.instr_memory[instruction_nr].src2 = atoi(strtok(par1, "R"));
			mips.instr_memory[instruction_nr].immediate = strtoul(strtok(par2, "()"), NULL, 0);
			mips.instr_memory[instruction_nr].src1 = atoi(strtok(NULL, "R"));
			break;
		case BEQZ:
		case BNEZ:
		case BLTZ:
		case BGTZ:
		case BLEZ:
		case BGEZ:
			par1 = strtok (NULL, " \t");
			par2 = strtok (NULL, " \t");
			mips.instr_memory[instruction_nr].src1 = atoi(strtok(par1, "R"));
			mips.instr_memory[instruction_nr].label = par2;
			break;
		case JUMP:
			par2 = strtok (NULL, " \t");
			mips.instr_memory[instruction_nr].label = par2;
		default:
			break;

	} 

	/* increment instruction number before moving to next line */
	instruction_nr++;
   }
   //reconstructing the labels of the branch operations
   int i = 0;
   while(true){
   	instruction_t instr = mips.instr_memory[i];
	if (instr.opcode == EOP) break;
	if (instr.opcode == BLTZ || instr.opcode == BNEZ ||
            instr.opcode == BGTZ || instr.opcode == BEQZ ||
            instr.opcode == BGEZ || instr.opcode == BLEZ ||
            instr.opcode == JUMP
	 ){
		mips.instr_memory[i].immediate = (labels[instr.label] - i - 1) << 2;
	}
        i++;
   }

}

/* writes an integer value to data memory at the specified address (use little-endian format: https://en.wikipedia.org/wiki/Endianness) */
void write_memory(unsigned address, unsigned value){
	int2char(value,mips.data_memory+address);
}

/* prints the content of the data memory within the specified address range */
void print_memory(unsigned start_address, unsigned end_address){
	cout << "data_memory[0x" << hex << setw(8) << setfill('0') << start_address << ":0x" << hex << setw(8) << setfill('0') <<  end_address << "]" << endl;
	for (unsigned i=start_address; i<end_address; i++){
		if (i%4 == 0) cout << "0x" << hex << setw(8) << setfill('0') << i << ": "; 
		cout << hex << setw(2) << setfill('0') << int(mips.data_memory[i]) << " ";
		if (i%4 == 3) cout << endl;
	} 
}

/* prints the values of the registers */
void print_registers(){
        cout << "Special purpose registers:" << endl;
        unsigned i, s;
        for (s=0; s<NUM_STAGES; s++){
                cout << "Stage: " << stage_names[s] << endl;
                for (i=0; i< NUM_SP_REGISTERS; i++)
                        if ((sp_register_t)i != IR && (sp_register_t)i != COND && get_sp_register((sp_register_t)i, (stage_t)s)!=UNDEFINED) cout << reg_names[i] << " = " << dec <<  get_sp_register((sp_register_t)i, (stage_t)s) << hex << " / 0x" << get_sp_register((sp_register_t)i, (stage_t)s) << endl;
        }
        cout << "General purpose registers:" << endl;
        for (i=0; i< NUM_GP_REGISTERS; i++)
                if (get_gp_register(i)!=(int)UNDEFINED) cout << "R" << dec << i << " = " << get_gp_register(i) << hex << " / 0x" << get_gp_register(i) << endl;
}

/* initializes the pipeline simulator */
void sim_pipe_init(unsigned mem_size, unsigned mem_latency){
	mips.data_memory_size = mem_size;
	mips.data_memory_latency = mem_latency;
	mips.data_memory = new unsigned char[mips.data_memory_size];
	reset();
}
	
/* deallocates the pipeline simulator */
void sim_pipe_terminate(){
	delete [] mips.data_memory;
}

/* execution statistics */
unsigned get_clock_cycles(){return mips.clock_cycles;}

unsigned get_instructions_executed(){return mips.instructions_executed;}

unsigned get_stalls(){return mips.stalls;}

float get_IPC(){return (float)mips.instructions_executed/mips.clock_cycles;}

/* =============================================================

   CODE TO BE COMPLETED

   ============================================================= */

/* reset the state of the pipeline simulator */
void reset(){

        // initializing data memory to all 0xFF
        for (unsigned i=0; i<mips.data_memory_size; i++) mips.data_memory[i]=0xFF;

        // initializing instuction memory
        for (int i=0; i<PROGRAM_SIZE;i++){
                mips.instr_memory[i].opcode=(opcode_t)NOP;
                mips.instr_memory[i].src1=UNDEFINED;
                mips.instr_memory[i].src2=UNDEFINED;
                mips.instr_memory[i].dest=UNDEFINED;
                mips.instr_memory[i].immediate=UNDEFINED;
        }
        mips.instr_base_address = UNDEFINED;

        // general purpose registers initialization
        // <TODO: add initialization code here>

        // pipeline registers initialization
        // <TODO: add initialization code here>

        // IR initialization
        for (int i=0; i<NUM_STAGES-1; i++){
                mips.ir[i].opcode=(opcode_t)NOP;
                mips.ir[i].src1=UNDEFINED;
                mips.ir[i].src2=UNDEFINED;
                mips.ir[i].dest=UNDEFINED;
                mips.ir[i].immediate=UNDEFINED;
        }

        // other required initializations (statistics, etc.)
        mips.clock_cycles = 0; //clock cycles
        mips.stalls = 0; //stalls
        mips.instructions_executed = 0; //instruction count
}

//returns value of special purpose register (see sim_pipe.h for more details)
unsigned get_sp_register(sp_register_t reg, stage_t s){
        return 0; // <TODO: please modify> 
}

//returns value of general purpose register
int get_gp_register(unsigned reg){
        return 0; // <TODO: please modify>
}

//sets the value of referenced general purpose register
void set_gp_register(unsigned reg, int value){
         // <TODO: please modify>
}

/* <TODO: BODY OF THE SIMULATOR */
// Note: processing the stages in reverse order simplifies the data propagation through pipeline registers
void run(unsigned cycles){

        unsigned start_cycles = mips.clock_cycles;
        
        /* initialization at the beginning of simulation */
        if (mips.clock_cycles == 0){
                // <set PC register to instr_base_address>
        }

        /* ====== MAIN SIMULATION LOOP (one iteration per clock cycle)  ========= */
        while(cycles==0 || mips.clock_cycles-start_cycles!=cycles){
                        
                /* =============== */
                /* PIPELINE STAGES */
                /* =============== */
                        
                /* ============   WB stage   ============  */
                        
                        // <hint: the simulation loop should be exited when the instruction processed is EOP>

                /* ============   MEM stage   ===========  */

                /* ============   EXE stage   ===========  */

                        // <suggestion: use "alu" and "taken_branch" helper functions above to update ALU_OUTPUT and COND registers>

                /* ============   ID stage   ============  */

                        // <suggestion: use the helper functions "is_branch", "is_int_r", ..., above to improve code readability/compactness>

                /* ============   IF stage   ============  */

                        // <hint: when accessing the instruction memory, you will need to scale PC to an integer index as in the following pseudocode:>
                        // ir[IF/ID] = instr_memory[(PC-instr_base_address)>>2] 

                /* =============== */
                /* END STAGES      */
                /* =============== */

                /* Other bookkeeping code */
                /* ====================== */

                mips.clock_cycles++; // increase clock cycles count
        }
}


