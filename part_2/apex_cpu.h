/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_

#include "apex_macros.h"
// added for BTB
#define BTB_adding_4_buffer 4
/* Format of an APEX instruction  */


// adding struct for BTB entry 
typedef struct BTB_entry
{
    // will need - address , calculated target address , valid bit , outcome bit, completion status - 
    int address;
    int calc_target_address;
    int valid_bit;
    int outcome_bit;
    int completion_status;
      // prediction bit?? - didnt end up using this
    // int prediction_bit;

    // to check if the branch is taken or not 
    int branch_taken;

} BTB_entry;




typedef struct APEX_Instruction
{
    char opcode_str[128];
    int opcode;
    int rd;
    int rs1;
    int rs2;
    int rs3; //added for store
    int imm;
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
    int pc;
    char opcode_str[128];
    int opcode;
    int rs1;
    int rs2;
    int rs3; //added for store 
    int rd;
    int imm;
    int rs1_value;
    int rs2_value;
    int result_buffer;
    int memory_address;
    int has_insn;

    int buff_temp; //added for STORE P AND LOAD P

    int stalling_value; //added for stalling
} CPU_Stage;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                        /* Current program counter */
    int clock;                     /* Clock cycles elapsed */
    int insn_completed;            /* Instructions retired */
    int regs[REG_FILE_SIZE];       /* Integer register file */
    int code_memory_size;          /* Number of instruction in the input file */
    APEX_Instruction *code_memory; /* Code Memory */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;               /* Wait for user input after every cycle */
    
    int zero_flag;                 /* {TRUE, FALSE} Used by BZ and BNZ to branch */
    int pos_flag;                  /* {TRUE, FALSE} */
    int neg_flag;                  /* {TRUE, FALSE} */

    int fetch_from_next_cycle;

    int flags_for_regs[REG_FILE_SIZE]; //added for flags

    //for forwarding
    // track pending result in execute stage
    int regs_status_pending[REG_FILE_SIZE];

    // track pending result in memory stage
    int regs_value_pending[REG_FILE_SIZE];

    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode;
    CPU_Stage execute;
    CPU_Stage memory;
    CPU_Stage writeback;

    // for BTB - head and array for BTB entries which will include BTB size for each entry

    // int BTB_size;
    // BTB_entry *BTB_array;
    int head_of_BTB;
    BTB_entry BTB_array[BTB_adding_4_buffer];

} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);

void APEX_cpu_simulate(APEX_CPU *cpu, int cycles,const char *filename)  ; //added for simulate

//btb functions
// int search_entry_in_BTB(APEX_CPU *cpu, int pc);

int search_entry_in_btb(APEX_CPU *cpu, int instruction_addr);
void initialize_btb(APEX_CPU *cpu);

int change_btb_11(int current_bits);
int change_btb_00(int current_bits);

void update_branch_BP_BNZ(APEX_CPU* cpu);
void update_branch_BZ_BNP(APEX_CPU* cpu);

void update_btb_entry(APEX_CPU *cpu, int instruction_addr, int calc_target_address);

#endif

