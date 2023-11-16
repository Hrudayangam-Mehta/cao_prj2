/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"
#include "apex_macros.h"

CPU_Stage outputDisplay[5];

char stages[5][20] = { "FETCH_ ","DECODE_RF_","EX_","MEMORY_","WRITEBACK_"};



/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }


        case OPCODE_SUB:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_SUBL:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_ADDL:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_MUL:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_DIV:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_AND:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_OR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }

        case OPCODE_LOAD:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_STORE:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }

        case OPCODE_BZ:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_BNZ:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        
        case OPCODE_BP:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_BNP:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_BN:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_BNN:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }



        case OPCODE_HALT:
        {
            printf("%s", stage->opcode_str);
            break;
        }

        case OPCODE_CMP:
        {
            printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2);
            break;
        }

        case OPCODE_CML:
        {
            printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->imm);
            break;
        }

        case OPCODE_NOP:
        {
            printf("%s", stage->opcode_str);
            break;
        }
        
        case OPCODE_JUMP:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rs1, stage->imm);
            break;
        }

        case OPCODE_JALR:
        {
            printf("%s,R%d, R%d, #%d ", stage->opcode_str, stage->rd, stage->rs1, stage->imm);
            break;
        }

        case OPCODE_LOADP:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1, stage->imm);
            break;
        }

        case OPCODE_STOREP:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2, stage->imm);
            break;
        }



        
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if (cpu->fetch.has_insn && !cpu->fetch.stalling_value)
    {
        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            outputDisplay[0].has_insn = 0; //fetch

            /* Skip this cycle*/
            return;
        }

        /* Store current PC in fetch latch */
        cpu->fetch.pc = cpu->pc;

        /* Index into code memory using this pc and copy all instruction fields
         * into fetch latch  */
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
        cpu->fetch.opcode = current_ins->opcode;
        cpu->fetch.rd = current_ins->rd;
        cpu->fetch.rs1 = current_ins->rs1;
        cpu->fetch.rs2 = current_ins->rs2;
        cpu->fetch.rs3 = current_ins->rs3; //added for store
        cpu->fetch.imm = current_ins->imm;

        /* Update PC for next instruction */

        // cpu->pc += 4;

        // /* Copy data from fetch latch to decode latch*/
        // cpu->decode = cpu->fetch;

        // fetch code for BTB 
        // 4 cases for BTB - 1. BTB miss, 2. BTB hit, 3. BTB miss and branch taken, 4. BTB hit and branch taken
        // BTB will be used for - BNZ, BZ, BNP and BP
        
        // BNZ and BP will have the same implementation
        // BZ and BNP will have the same implementation




            if(cpu->fetch.opcode == OPCODE_BNZ || cpu->fetch.opcode == OPCODE_BZ || cpu->fetch.opcode == OPCODE_BNP || cpu->fetch.opcode == OPCODE_BP){

    // Search for the entry in the BTB
    // print btb_index, outcome_bit, calc_target_address
    // printf("BTB INDEX: %d\n", search_entry_in_btb(cpu, cpu->fetch.pc));
   

            int btb_index = search_entry_in_btb(cpu, cpu->pc);
            if(btb_index >= 0){
                // Check if the opcode is BNZ or BP and the outcome bit is not 0
                if( (cpu->fetch.opcode == OPCODE_BNZ || cpu->fetch.opcode == OPCODE_BP) && cpu->BTB_array[btb_index].outcome_bit != 0 ){
                    cpu->pc = cpu->BTB_array[btb_index].calc_target_address;
                }
                // Check if the opcode is BZ or BNP and the outcome bit is 2
                else if((cpu->fetch.opcode == OPCODE_BZ || cpu->fetch.opcode == OPCODE_BNP) && cpu->BTB_array[btb_index].outcome_bit == 2 ){
                    cpu->pc = cpu->BTB_array[btb_index].calc_target_address;
                }
                else{
                    cpu->pc += 4;
                }
                        }
                                    
            else{
                // Add the entry in the BTB
                // add_entry_in_BTB(cpu, cpu->pc);

                cpu->pc += 4;
            }
        }
        else{
            cpu->pc += 4;
        }
        

        
        if(!cpu->decode.stalling_value)
        {
            // cpu->pc += 4;
            cpu->decode = cpu->fetch;
        }

        else
        {
            cpu->fetch.stalling_value = 1;
        }
        outputDisplay[0] = cpu->fetch;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Fetch", &cpu->fetch);
        }

        /* Stop fetching new instructions if HALT is fetched */
        if (cpu->fetch.opcode == OPCODE_HALT && !cpu->decode.stalling_value)
        {
            cpu->decode = cpu->fetch; 
            // whaa??
            cpu->fetch.has_insn = FALSE;
        }
    }
    else{
       outputDisplay[0] = cpu->fetch; //fetch
    }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{


    if (cpu->decode.has_insn)
    {
        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {

            case OPCODE_BNZ:
            {
                int btb_index = search_entry_in_btb(cpu, cpu->decode.pc);
                if (btb_index==-1)
                {
                    update_btb_entry(cpu, cpu->decode.pc,-1);
                    btb_index = search_entry_in_btb(cpu, cpu->decode.pc);
                    // cpu->BTB_array[btb_index].outcome_bit = 0;
                    cpu->BTB_array[btb_index].outcome_bit = 2;
                    
                    cpu->BTB_array[btb_index].completion_status = 0;
                    // cpu->BTB_array[btb_index].calc_target_address = cpu->decode.pc + cpu->decode.imm;

                }
                break;
                
            }

            case OPCODE_BP:
            {
                 int btb_index = search_entry_in_btb(cpu, cpu->decode.pc);
                if (btb_index==-1)
                {
                    update_btb_entry(cpu, cpu->decode.pc,-1);
                    btb_index = search_entry_in_btb(cpu, cpu->decode.pc);
                    // cpu->BTB_array[btb_index].outcome_bit = 0;
                    cpu->BTB_array[btb_index].outcome_bit = 2;
                    
                    cpu->BTB_array[btb_index].completion_status = 0;
                    // cpu->BTB_array[btb_index].calc_target_address = cpu->decode.pc + cpu->decode.imm;

                }
                break;
                
            }
            
            
            case OPCODE_BNP:
            {
                int btb_index = search_entry_in_btb(cpu, cpu->decode.pc);
                if (btb_index==-1)
                {
                    update_btb_entry(cpu, cpu->decode.pc,-1);
                    btb_index = search_entry_in_btb(cpu, cpu->decode.pc);
                    // cpu->BTB_array[btb_index].outcome_bit = 0;
                    cpu->BTB_array[btb_index].outcome_bit = 0;
                    
                    cpu->BTB_array[btb_index].completion_status = 0;
                    // cpu->BTB_array[btb_index].calc_target_address = cpu->decode.pc + cpu->decode.imm;

                }
                break;
            }
            case OPCODE_BZ:
            {
                int btb_index = search_entry_in_btb(cpu, cpu->decode.pc);
                if (btb_index==-1)
                {
                    update_btb_entry(cpu, cpu->decode.pc,-1);
                    btb_index = search_entry_in_btb(cpu, cpu->decode.pc);
                    // cpu->BTB_array[btb_index].outcome_bit = 0;
                    cpu->BTB_array[btb_index].outcome_bit = 0;
                    
                    cpu->BTB_array[btb_index].completion_status = 0;
                    // cpu->BTB_array[btb_index].calc_target_address = cpu->decode.pc + cpu->decode.imm;

                }
                break;
                
            }
            
            
            case OPCODE_BN:
            {
                break;
            }
            case OPCODE_BNN:
            {
                break;
            }
           

            case OPCODE_ADD:
           {

            if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rs2] && !cpu->flags_for_regs[cpu->decode.rd])
            {
                cpu->decode.stalling_value = 0;
                //update flags
                cpu->flags_for_regs[cpu->decode.rd] = 1;

                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
            }
            else
            {
                cpu->decode.stalling_value = 1;
                cpu->fetch_from_next_cycle = TRUE;
            }
            break;
            

           }

            case OPCODE_SUB:
            {
                if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rs2] && !cpu->flags_for_regs[cpu->decode.rd])
                {
                    cpu->decode.stalling_value = 0;
                    //update flags
                    cpu->flags_for_regs[cpu->decode.rd] = 1;

                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }
                break;
            }
            
            case OPCODE_MUL:
            {
                if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rs2] && !cpu->flags_for_regs[cpu->decode.rd])
                {
                    cpu->decode.stalling_value = 0;
                    //update flags
                    cpu->flags_for_regs[cpu->decode.rd] = 1;

                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }
                break;
            }
            

            case OPCODE_DIV:
            {
                if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rs2] && !cpu->flags_for_regs[cpu->decode.rd])
                {
                    cpu->decode.stalling_value = 0;
                    //update flags
                    cpu->flags_for_regs[cpu->decode.rd] = 1;

                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }
                break;
            }

            case OPCODE_SUBL:
            {
                if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rd])
                {
                    cpu->decode.stalling_value = 0;
                    //update flags
                    cpu->flags_for_regs[cpu->decode.rd] = 1;

                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    //will get from immediate
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }
                break;
            }

            case OPCODE_ADDL:
            {
                if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rd])
                {
                    cpu->decode.stalling_value = 0;
                    //update flags
                    cpu->flags_for_regs[cpu->decode.rd] = 1;

                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    //will get from immediate
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }
                break;
            }

            case OPCODE_AND:
            {
                if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rs2] && !cpu->flags_for_regs[cpu->decode.rd])
                {
                    cpu->decode.stalling_value = 0;
                    //update flags
                    cpu->flags_for_regs[cpu->decode.rd] = 1;

                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }
                break;
            }

            case OPCODE_OR:
            {
                if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rs2] && !cpu->flags_for_regs[cpu->decode.rd])
                {
                    cpu->decode.stalling_value = 0;
                    //update flags
                    cpu->flags_for_regs[cpu->decode.rd] = 1;

                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }
                break;
            }

            case OPCODE_XOR:
            {
                if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rs2] && !cpu->flags_for_regs[cpu->decode.rd])
                {
                    cpu->decode.stalling_value = 0;
                    //update flags
                    cpu->flags_for_regs[cpu->decode.rd] = 1;

                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }
                break;
            }

           
        

            case OPCODE_LOAD:
            {
                if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rd])
                {
                    cpu->decode.stalling_value = 0;
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

                    //update flags
                    cpu->flags_for_regs[cpu->decode.rd] = 1;
                    //will get 1 VAL from immediate
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }
                break;
            }

            case OPCODE_STORE:
            {
                if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rs2])
                {
                    cpu->decode.stalling_value = 0;
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    
                    //update flags
                    cpu->flags_for_regs[cpu->decode.rs1] = 1;

                    //will get 1 VAL from immediate
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }
                break;
            }

            case OPCODE_LOADP:
            {
                if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rd] && !cpu->flags_for_regs[cpu->decode.rs2])
                {
                    cpu->decode.stalling_value = 0;
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

                    //update flags
                    cpu->flags_for_regs[cpu->decode.rd] = 1;
                    //will get 1 VAL from immediate
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }

                break;
            }

            case OPCODE_STOREP:
            {
                //add condition for storep, if the next instruction is using rs1 or rs2, then lock rs1 and rs2
              
                if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rs2])
                {
                    cpu->decode.stalling_value = 0;
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    
                    //update flags
                    cpu->flags_for_regs[cpu->decode.rs1] = 1;
                    cpu->flags_for_regs[cpu->decode.rs2] = 1;

                    //will get 1 VAL from immediate
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }

                break;
            }
            

            case OPCODE_MOVC:
            {
                /* MOVC doesn't have register operands */
                if (!cpu->flags_for_regs[cpu->decode.rd])
                {
                    cpu->flags_for_regs[cpu->decode.rd] = 1;
                    cpu->decode.stalling_value = 0;
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                }

                break;
            }

            case OPCODE_CMP:
            {
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                // cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                if(!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rs2])
                {
                    cpu->decode.stalling_value = 0;
                    
                    cpu->flags_for_regs[cpu->decode.rd] = 1;

                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    //update flags
                  
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }

                break;
            }

            case OPCODE_CML:
            {
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                //will get from immediate

                if(!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rd])
                {
                    cpu->decode.stalling_value = 0;
                    cpu->flags_for_regs[cpu->decode.rd] = 1;

                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    //update flags
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }


                break;
            }

            case OPCODE_NOP:
            {
                /* NOP doesn't have register operands */
                break;
            }

            case OPCODE_JUMP:
            {
                // print to say we have reached execute stage
                // printf("Reached DECLDE stage\n");
                if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rd])
                {
                    cpu->decode.stalling_value = 0;
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->flags_for_regs[cpu->decode.rd] = 1;
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }
                break;
            }

            case OPCODE_JALR:
            {
                // print to say we have reached execute stage
                printf("Reached DECLDE stage\n");
                if (!cpu->flags_for_regs[cpu->decode.rs1] && !cpu->flags_for_regs[cpu->decode.rd] && !cpu->flags_for_regs[cpu->decode.rs2])
                {
                    cpu->decode.stalling_value = 0;
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->flags_for_regs[cpu->decode.rd] = 1;
                }
                else
                {
                    cpu->decode.stalling_value = 1;
                    cpu->fetch_from_next_cycle = TRUE;
                }
                break;
            }

           






        }

        outputDisplay[1] = cpu->decode; //decode

        /* Copy data from decode latch to execute latch*/
        // only execute when the register is not busy
        
        // cpu->execute = cpu->decode;
        // cpu->decode.has_insn = FALSE;

     // if decode.stalling_value is 0 then only copy the data from decode to execute
     if(!cpu->decode.stalling_value)
        {
            cpu->execute = cpu->decode;
            cpu->decode.has_insn = FALSE;
        }
        
        
          

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Decode/RF", &cpu->decode);
        }
    }
    else{
        outputDisplay[1] = cpu->decode; //decode
    }
}



/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_execute(APEX_CPU *cpu)
{
    if (cpu->execute.has_insn)
    {
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {
            
            case OPCODE_BP:
            {
                int btb_index = search_entry_in_btb(cpu, cpu->execute.pc);

                //PRINT POS FLAG VALUE 
                // printf("------------------------------------");
                // printf("POS FLAG VALUE: %d\n", cpu->pos_flag);
                

                if(cpu->pos_flag == 1)
                {
                    update_branch_BP_BNZ(cpu);
                }

                else if( cpu->BTB_array[btb_index].completion_status == 1 && cpu->BTB_array[btb_index].outcome_bit == 0)
                {

                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
            
            case OPCODE_BZ:
            {
                int btb_index = search_entry_in_btb(cpu, cpu->execute.pc);

                if (cpu->zero_flag == TRUE)
                {

                    update_branch_BZ_BNP(cpu);
                }

                else if( cpu->BTB_array[btb_index].completion_status == 1 && cpu->BTB_array[btb_index].outcome_bit == 2)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNZ:
            
            {

             int btb_index = search_entry_in_btb(cpu, cpu->execute.pc);
                if(cpu->zero_flag ==0)
                {
                    update_branch_BP_BNZ(cpu);
                }


                //   or flush
            // else if( cpu->BTB_array[btb_index].completion_status == 1 && cpu->BTB_array[btb_index].outcome_bit == 2)
            
            else if( cpu->BTB_array[btb_index].completion_status == 1 && cpu->BTB_array[btb_index].outcome_bit != 0)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc +4;
                    // cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNP:
            {
                int btb_index = search_entry_in_btb(cpu, cpu->execute.pc);

                if(cpu->pos_flag == FALSE)
                {
                    update_branch_BZ_BNP(cpu);
                }

                else if( cpu->BTB_array[btb_index].completion_status == 1 && cpu->BTB_array[btb_index].outcome_bit == 2){

                


                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BN:
            {
                if(cpu->neg_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNN:
            {
                if(cpu->neg_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_ADD:
            {

                
              
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value + cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                // Setzero_Flag(cpu);


                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = FALSE;
                } 
                
                else if (cpu->execute.result_buffer > 0) 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = TRUE;
                    cpu -> neg_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = TRUE;
                }
                break;
                }
            

            case OPCODE_SUB:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value - cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                // Setzero_Flag(cpu);
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = FALSE;
                } 
                
                else if (cpu->execute.result_buffer > 0) 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = TRUE;
                    cpu -> neg_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = TRUE;
                }
                
                break;
            }

            case OPCODE_MUL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value * cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                // Setzero_Flag(cpu);
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = FALSE;
                } 
                
                else if (cpu->execute.result_buffer > 0) 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = TRUE;
                    cpu -> neg_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = TRUE;
                }
                break;
            }

            case OPCODE_DIV:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value / cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                // Setzero_Flag(cpu);
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = FALSE;
                } 
                
                else if (cpu->execute.result_buffer > 0) 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = TRUE;
                    cpu -> neg_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = TRUE;
                }
                break;
            }

            case OPCODE_AND:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value & cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                // Setzero_Flag(cpu);
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = FALSE;
                } 
                
                else if (cpu->execute.result_buffer > 0) 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = TRUE;
                    cpu -> neg_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = TRUE;
                }
                break;
            }

            case OPCODE_OR:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value | cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                // Setzero_Flag(cpu);
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = FALSE;
                } 
                
                else if (cpu->execute.result_buffer > 0) 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = TRUE;
                    cpu -> neg_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = TRUE;
                }
                break;
            }

            case OPCODE_XOR:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value ^ cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                // Setzero_Flag(cpu);
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = FALSE;
                } 
                
                else if (cpu->execute.result_buffer > 0) 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = TRUE;
                    cpu -> neg_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = TRUE;
                }
                break;
            }

            case OPCODE_SUBL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value - cpu->execute.imm;

                /* Set the zero flag based on the result buffer */
                // Setzero_Flag(cpu);
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = FALSE;
                } 
                
                else if (cpu->execute.result_buffer > 0) 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = TRUE;
                    cpu -> neg_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = TRUE;
                }
                break;
            }

            case OPCODE_ADDL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value + cpu->execute.imm;

                /* Set the zero flag based on the result buffer */
                // Setzero_Flag(cpu);

                // print result buffer
                printf("Result buffer is %d\n", cpu->execute.result_buffer);

                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = FALSE;
                } 
                
                else if (cpu->execute.result_buffer > 0) 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = TRUE;
                    cpu -> neg_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = TRUE;
                }

                break;
            }

            case OPCODE_CMP:
            {
                //update flags based on comparison
                if (cpu->execute.rs1_value == cpu->execute.rs2_value)
                {
                    cpu->zero_flag = TRUE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = FALSE;
                } 
                else if (cpu->execute.rs1_value > cpu->execute.rs2_value) 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = TRUE;
                    cpu -> neg_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = TRUE;
                }
                break;
            }

            case OPCODE_CML:
            {
                //update flags based on comparison
                if (cpu->execute.rs1_value == cpu->execute.imm)
                {
                    cpu->zero_flag = TRUE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = FALSE;
                } 
                else if (cpu->execute.rs1_value > cpu->execute.imm) 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = TRUE;
                    cpu -> neg_flag = FALSE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                    cpu -> pos_flag = FALSE;
                    cpu -> neg_flag = TRUE;
                }
                break;
            }

           

            case OPCODE_JUMP:
            {
                cpu->pc = cpu->execute.rs1_value + cpu->execute.imm;
                    
                /* Since we are using reverse callbacks for pipeline stages, 
                 * this will prevent the new instruction from being fetched in the current cycle*/
                cpu->fetch_from_next_cycle = TRUE;

                /* Flush previous stages */
                cpu->decode.has_insn = FALSE;

                /* Make sure fetch stage is enabled to start fetching from new PC */
                cpu->fetch.has_insn = TRUE;
                break;
            }

            case OPCODE_JALR:
            {
                cpu->execute.result_buffer = cpu->execute.pc + 4;
                cpu->pc = cpu->execute.rs1_value + cpu->execute.imm;
                    
                /* Since we are using reverse callbacks for pipeline stages, 
                 * this will prevent the new instruction from being fetched in the current cycle*/
                cpu->fetch_from_next_cycle = TRUE;

                /* Flush previous stages */
                cpu->decode.has_insn = FALSE;

                /* Make sure fetch stage is enabled to start fetching from new PC */
                cpu->fetch.has_insn = TRUE;
                break;
            }

            case OPCODE_NOP:
            {
                /* NOP doesn't have any operation */
                break;
            }

            case OPCODE_STORE:
            {
                // print to check if it is working
                printf("STORE result buffer: %d\n", cpu->execute.result_buffer);

                cpu->execute.result_buffer = cpu->execute.rs1_value;
                
                cpu->execute.memory_address
                    = cpu->execute.rs2_value + cpu->execute.imm;
            
                break;
            }



            case OPCODE_LOAD:
            { 
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.imm;
                break;
            }

            
            case OPCODE_MOVC: 
            {
                cpu->execute.result_buffer = cpu->execute.imm;

                // no flags for MOVC, LOAD, LOADP, STORE, STOREP
                break;
                
            }
        

        case OPCODE_LOADP:
        {

            cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.imm;
            


           cpu->execute.buff_temp = cpu->execute.rs1_value+4;
            
            break;
        }

        case OPCODE_STOREP:
        {
            // cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.imm;
            
            cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.imm;

           cpu->execute.buff_temp = cpu->execute.rs2_value+4;
            
            break;
        }

        }

        outputDisplay[2] = cpu->execute;

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Execute", &cpu->execute);
        }
    }
    else{
        outputDisplay[2] = cpu->execute;
    }
}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory(APEX_CPU *cpu)
{
    if (cpu->memory.has_insn)
    {
        switch (cpu->memory.opcode)
        {
           
            case OPCODE_STORE:
            {
                /* Store */
                cpu->data_memory[cpu->memory.memory_address]
                    = cpu->memory.rs1_value;
                    //rs1
                break;
            }

            case OPCODE_LOAD:
            {
                /* Read from data memory */
                cpu->memory.result_buffer = cpu->data_memory[cpu->memory.memory_address];
                break;
            }

            case OPCODE_LOADP:
            {
                /* Read from data memory */
                cpu->memory.result_buffer= cpu->data_memory[cpu->memory.memory_address];
                break;
            }

            case OPCODE_STOREP:
            {
                /* Store */
                /*Store data from destination register to data memory */
                 cpu->data_memory[cpu->memory.memory_address]  = cpu->memory.rs1_value;
                
                // cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs2_value;
                break;
            }
        }

        outputDisplay[3] = cpu->memory;

        /* Copy data from memory latch to writeback latch*/
        cpu->writeback = cpu->memory;
        cpu->memory.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Memory", &cpu->memory);
        }
    }
    else{
        outputDisplay[3] = cpu->memory;
    }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    if (cpu->writeback.has_insn)
    {
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                //update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                break;
            }
            
            case OPCODE_SUB:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                //update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;

                break;
            }
            
            case OPCODE_MUL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                //update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                break;
            }
            case OPCODE_SUBL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                break;
            }
            case OPCODE_ADDL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                break;
            }

            case OPCODE_DIV:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                //update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                break;
            }
            case OPCODE_AND:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                break;
            }

            case OPCODE_OR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                break;
            }
            case OPCODE_XOR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                
                break;
            }
            
            case OPCODE_CMP:
            {
                //FOR TESTING PRINT STATEMENT
                // printf("CMP result buffer: %d\n", cpu->writeback.result_buffer);
                // cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // update flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                break;
            }
            case OPCODE_CML:
            {
                //FOR TESTING PRINT STATEMENT
                // printf("CML result buffer: %d\n", cpu->writeback.result_buffer);
                // cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // update flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0; 
                break;
            }
            
            
            case OPCODE_JUMP:
            {
                // print to say we have reached writeback stage
                printf("Reached writeback stage bbbbbbbbbbbbbbbb\n");
                // update flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                break;
            }
            case OPCODE_JALR:
            {
                // print to say we have reached writeback stage
                printf("Reached writeback stage\n");

                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                
                // update flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;

                break;
            }
            case OPCODE_NOP:
            {
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                //print to check if this is working
                printf("LOAD result buffer: %d\n", cpu->writeback.result_buffer);
                
                // update flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                break;
            }

            case OPCODE_STORE:
            {
                // cpu->regs[cpu->writeback.rd] = cpu->writeback.rs1_value;
                //print to check if this is working
                // printf("STORE result buffer: %d\n", cpu->writeback.result_buffer);
                // update flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                // cpu->fetch.stalling_value = 0;
                // cpu->decode.stalling_value = 0;
                break;
            }

            case OPCODE_MOVC: 
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                //update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                break;
            }

            case OPCODE_LOADP:
            {

                // //print value of memory address
                // printf("LOADP memory address: %d\n", cpu->writeback.memory_address);
                // // print valye at memory address
                // printf("LOADP value at memory address: %d\n", cpu->data_memory[cpu->writeback.memory_address]);

                
                //update value of rd with the value at memory address
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;

                
                cpu->regs[cpu->writeback.rs1] = cpu->writeback.buff_temp;

                // //update the value of rd with the value in memory buffer
                // cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;



                //update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                cpu->flags_for_regs[cpu->writeback.rs1] = 0;
                break;
            }

            case OPCODE_STOREP:
            {
                
                cpu->regs[cpu->writeback.rs2] = cpu->writeback.buff_temp;
                //update the value of rd with the value in memory buffer
                // cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                

                
                //update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;


             
                //unlock the registers
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                cpu->flags_for_regs[cpu->writeback.rs1] = 0;
                cpu->flags_for_regs[cpu->writeback.rs2] = 0;

                break;
            }   



        }

        outputDisplay[4] = cpu->writeback;

        cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Writeback", &cpu->writeback);
        }

        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }
    }
    else{
        outputDisplay[4] = cpu->writeback;
    }

    /* Default */
    return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    cpu->single_step = ENABLE_SINGLE_STEP;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}


void displaySequence()
{ printf("\n");
    for(int i = 0; i < 5; i++)
    {
        if(outputDisplay[i].has_insn){
            printf("\n");
            printf("%d.  %-15s--->: (l%d: %d) ",i+1, stages[i], (outputDisplay[i].rd>0 ? outputDisplay[i].rd:0),outputDisplay[i].pc);
            print_instruction(&outputDisplay[i]);
            printf("\n");
        } else{
            printf("%d.   %-15s--->:   NA \n",i+1, stages[i]);
        }
    }
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;

    while (TRUE)
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("-------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock+1);
           
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
            break;
        }

        APEX_memory(cpu);
        APEX_execute(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        displaySequence();

        print_reg_file(cpu);

        // print the value of flags 
        printf("\n");
        printf("Zero flag: %d\n", cpu->zero_flag);
        printf("Positive flag: %d\n", cpu->pos_flag);
        printf("Negative flag: %d\n", cpu->neg_flag);
printf("\n");

        if (cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
                break;
            }
        }

        cpu->clock++;
    }
}



void Registers_state(APEX_CPU* cpu) {
  printf("\n== STATE REGISTER FILE ====\n");
  for(int i = 0; i < REG_FILE_SIZE; ++i) 
  {
    printf("|\tREG[%d]\t|\tValue = %d\t|\tStatus = %s\t|\n", i, cpu->regs[i], (cpu->flags_for_regs[i] ? "INVALID" : "VALID"));
  }
}

void State_data_memory(APEX_CPU* cpu) {
  printf("\n=TATE OF DATA MEMORY ==\n");
  int index;
  for(index = 0; index < 100; ++index) {
    printf("|\tMEM[%d]\t|\tData Value = %d\t|\n", index, cpu->data_memory[index]);
  }
}


void APEX_cpu_simulate(APEX_CPU *cpu, int cycles,const char *filename)
{

    while (cycles != 0)
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("---Clock Cycle #:%d------\n", cpu->clock+1);
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
            break;
        }

        APEX_memory(cpu);
        APEX_execute(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);
        
        displaySequence();

        cpu->clock++;
        cycles -= 1;
    }
    if(strcmp(filename, "simulate") == 0){
        APEX_cpu_run(cpu);
    }
    Registers_state(cpu);
    State_data_memory(cpu);
}


/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}


//HELPER FUNCTIONS FOR BTB

// search for the branch in the BTB to check for entry in BTB
// search_entry_in_btb - inputs apex cpu, instruction address

// search_entry_in_btb:


// int search_entry_in_btb(APEX_CPU *cpu, int instruction_addr, int calc_target_address)
// {
    // printf("search_entry_in_btb\n");

    // for (int i = 0; i < BTB_adding_4_buffer; i++)
    // {
    //     if (cpu->BTB_array[i].calc_target_address == instruction_addr)
    //     {
    //         // printf("search_entry_in_btb - found entry in BTB\n");

    //         return address[i];
    //     }
    // }

//     
    //printing for testing 
//     printf("search_entry_in_btb - entry not found in BTB\n");
//     return instruction_addr;
// }

int search_entry_in_btb(APEX_CPU *cpu, int instruction_addr)
{
    // printf("search_entry_in_btb\n");

    for (int i = 0; i < BTB_adding_4_buffer; i++)
    {
        if (cpu->BTB_array[i].address == instruction_addr)
        {
            // printf("search_entry_in_btb - found entry in BTB\n");
            return i;
        }
    }
    //printing for testing 
    printf("search_entry_in_btb - entry not found in BTB\n");
    return -1;
}

// initialize_btb - inputs apex cpu
void initialize_btb(APEX_CPU *cpu)
{
    // printf("initialize_btb test AAAAAAAAAAAAAAAAAAAAAAAAA \n");
    for (int i = 0; i < BTB_adding_4_buffer; i++)
    {

         // cpu->BTB_array[i].address = -1;
        // cpu->BTB_array[i].calc_target_address = -1;
        // cpu->BTB_array[i].prediction = -1;
        // cpu->BTB_array[i].valid = 0;

        cpu->BTB_array[i].outcome_bit=0;
        cpu->BTB_array[i].completion_status=0;
        cpu->BTB_array[i].branch_taken=0;
       
    }
}


// update_btb_entry - inputs apex cpu, instruction address, calc target address
// void update_btb_entry(APEX_CPU *cpu, int instruction_addr, int calc_target_address)
// {
//     // printf("update_btb_entry\n");
//     int index = search_entry_in_btb(cpu, instruction_addr);
//     if (index != -1)
//     {
//         // printf("update_btb_entry - updating entry in BTB\n");
//         cpu->BTB_array[index].address = instruction_addr;
//         cpu->BTB_array[index].calc_target_address = calc_target_address;
//         cpu->BTB_array[index].prediction = 1;
//         cpu->BTB_array[index].valid = 1;
//     }
//     else
//     {
//         // printf("update_btb_entry - adding entry in BTB\n");
//         for (int i = 0; i < BTB_adding_4_buffer; i++)
//         {
//             if (cpu->BTB_array[i].valid == 0)
//             {
//                 cpu->BTB_array[i].address = instruction_addr;
//                 cpu->BTB_array[i].calc_target_address = calc_target_address;
//                 cpu->BTB_array[i].prediction = 1;
//                 cpu->BTB_array[i].valid = 1;
//                 break;
//             }
//         }
//     }
// }



// update_btb_entry - inputs apex cpu, instruction address, calc target address
void update_btb_entry(APEX_CPU *cpu, int instruction_addr, int calc_target_address)
{
    // printf("update_btb_entry AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA \n");
    cpu->BTB_array[cpu->head_of_BTB].address = instruction_addr;
    cpu->BTB_array[cpu->head_of_BTB].calc_target_address = calc_target_address;
    // cpu->BTB_array[cpu->head_of_BTB].prediction = 1;
    cpu->BTB_array[cpu->head_of_BTB].completion_status = 0  ;
    cpu->BTB_array[cpu->head_of_BTB].branch_taken = 0;

    //adddng entry to BTB 

    cpu->head_of_BTB = (cpu->head_of_BTB + 1) % BTB_adding_4_buffer;

    // printf("update_btb_entry - adding entry in BTB\n");
    // uhhhhhhhhhhhhhh moving to the next entry in BTB somehow?????

}


// int change_btb_2(int outcome_bit)
// {
//     if(outcome_bit == 0){
//         printf(" --------------------- btb update to 2 ka loop --------------");
//         return 1;
//     }

//     return 2;
// }

// increment/change btb entry to 2 
int change_btb_2(int current_bits)
{
    if(current_bits == 0){
        printf(" --------------------- btb update to 2 ka loop --------------");
        return 1;
    }
    // baaki cases ke liye update
    return 2;
}

// increment/change btb entry to 0
int change_btb_0(int current_bits)
{
    if(current_bits == 2){
        return 1;
    }
    //printf("change_btb_0 - changing to 0\n");
    //printf("reached change btbt 0\n");
    // return 0;
    return 0;

}

// code for BP_BNZ

//logic 
/*

 from updated P2 pdf - using Mintify (commenting ai)

agar branch hua:

- pehle dekh ki complete hua ki nahi, and then flush it 
- agar bits are 0 to change that, so you need to manually change that entry

- and if entry is right, toh 2 karna padega and voh 3 line copy paste for flushing it out - from prj1

- agar bits are 1 or 2 and entry is wrong, toh 0 update aur flush cause calculation is wrorng

    agar entry is correct, toh keep it as it is to 2

- agar decode mein pata chala it is not a branch

2 1 ke liye - complete karke flush karna padega - pc value update karni padegi

all the rest cases - nahi samaj aaraha cause infinite loop pred mein atak raha hai :(

*/

// update_branch_BP_BNZ - inputs apex cpu, instruction address, calc target address

// void update_branch_BP_BNZ(APEX_CPU *cpu, int instruction_addr, int calc_target_address)
// {
//     // printf("update_branch_BP_BNZ\n");
//     int index = search_entry_in_btb(cpu, instruction_addr);
//     if (index != -1)
//     {
//         // printf("update_branch_BP_BNZ - updating entry in BTB\n");
//         cpu->BTB_array[index].address = instruction_addr;
//         cpu->BTB_array[index].calc_target_address = calc_target_address;
//         cpu->BTB_array[index].prediction = 1;
//         cpu->BTB_array[index].valid = 1;
//     }
//     else
//     {
//         // printf("update_branch_BP_BNZ - adding entry in BTB\n");
//         for (int i = 0; i < BTB_adding_4_buffer; i++)
//         {
//             if (cpu->BTB_array[i].valid == 0)
//             {
//                 cpu->BTB_array[i].address = instruction_addr;
//                 cpu->BTB_array[i].calc_target_address = calc_target_address;
//                 cpu->BTB_array[i].prediction = 1;
//                 cpu->BTB_array[i].valid = 1;
//                 break;
//             }
//         }
//     }
// }





void update_branch_BP_BNZ(APEX_CPU* cpu)
{
    // Calculate the target address
    int target_address = cpu->execute.pc + cpu->execute.imm;
    // Search for the entry in the BTB
    int btb_index = search_entry_in_btb(cpu, cpu->execute.pc);

    // Check if the BTB entry is unresolved or if it's resolved but the calculated address doesn't match and the outcome bit is 0
    if (cpu->BTB_array[btb_index].completion_status == 0 || (cpu->BTB_array[btb_index].completion_status == 1  && cpu->BTB_array[btb_index].calc_target_address != target_address && cpu->BTB_array[btb_index].outcome_bit == 0)){
        // Update the BTB entry
        cpu->BTB_array[btb_index].completion_status = 1;
        cpu->BTB_array[btb_index].calc_target_address = target_address;
        cpu->BTB_array[btb_index].outcome_bit = 2;

        // cpu->BTB_array[btb_index].outcome_bit = change_btb_2(cpu->BTB_array[btb_index].outcome_bit);

        cpu->pc = target_address;

        // this will remain the same for all the cases
        cpu->fetch_from_next_cycle = TRUE;

        cpu->decode.has_insn = FALSE;

        cpu->fetch.has_insn = TRUE;
    }
    // Check if the BTB entry is resolved, the calculated address matches and the outcome bit is 0
    else if (cpu->BTB_array[btb_index].completion_status == 1  && cpu->BTB_array[btb_index].calc_target_address == target_address && cpu->BTB_array[btb_index].outcome_bit == 0  ){
        // Increment the outcome bit
        cpu->BTB_array[btb_index].outcome_bit = change_btb_2(cpu->BTB_array[btb_index].outcome_bit);
        
        cpu->pc = target_address;

        // this will remain the same for all the cases
        cpu->fetch_from_next_cycle = TRUE;

        cpu->decode.has_insn = FALSE;

        cpu->fetch.has_insn = TRUE;
    }
    // Check if the BTB entry is resolved, the calculated address doesn't match and the outcome bit is not 0
    else if( cpu->BTB_array[btb_index].completion_status == 1  && cpu->BTB_array[btb_index].calc_target_address != target_address && cpu->BTB_array[btb_index].outcome_bit != 0 ){
        // Decrement the outcome bit
        cpu->BTB_array[btb_index].outcome_bit = change_btb_0(cpu->BTB_array[btb_index].outcome_bit);
        
        cpu->pc = target_address;

        // this will remain the same for all the cases
        cpu->fetch_from_next_cycle = TRUE;

        cpu->decode.has_insn = FALSE;

        cpu->fetch.has_insn = TRUE;
    }
    // Check if the BTB entry is resolved, the calculated address matches and the outcome bit is not 0
    else if(cpu->BTB_array[btb_index].completion_status == 1  && cpu->BTB_array[btb_index].calc_target_address == target_address && cpu->BTB_array[btb_index].outcome_bit != 0) {
        // Increment the outcome bit
        cpu->BTB_array[btb_index].outcome_bit = change_btb_2(cpu->BTB_array[btb_index].outcome_bit);
    }
}



// logic change 
/*

same rahega sab scenarios but "NOT" initialize the entry in BTB - according to p2 project 

so 1 and 0 entry wrong, toh edit the entry fir flush kar



*/


void update_branch_BZ_BNP(APEX_CPU* cpu){
    // Calculate the target address
    int target_address = cpu->execute.pc + cpu->execute.imm;
    // Search for the entry in the BTB
    int btb_index = search_entry_in_btb(cpu, cpu->execute.pc);

    // Check if the BTB entry is unresolved or if it's resolved but the calculated address doesn't match and the outcome bit is 0
    if (cpu->BTB_array[btb_index].completion_status == 0 || (cpu->BTB_array[btb_index].completion_status == 1  && cpu->BTB_array[btb_index].calc_target_address != target_address && cpu->BTB_array[btb_index].outcome_bit == 0)){
        // Update the BTB entry
        cpu->BTB_array[btb_index].completion_status = 1;
        cpu->BTB_array[btb_index].calc_target_address = target_address;

        // cpu->BTB_array[btb_index].outcome_bit= 0;
        // this is giving some error that i am not able to figure out... changibg this to 1 for now --- CHANGE THIS BACK :( ????????

        cpu->BTB_array[btb_index].outcome_bit= 1;
        cpu->pc = target_address;

        cpu->fetch_from_next_cycle = TRUE;

        cpu->decode.has_insn = FALSE;

        cpu->fetch.has_insn = TRUE;
    }
    // Check if the BTB entry is resolved, the calculated address doesn't match and the outcome bit is 2
    else if(cpu->BTB_array[btb_index].completion_status == 1 && cpu->BTB_array[btb_index].calc_target_address != target_address && cpu->BTB_array[btb_index].outcome_bit == 2  ){
        // Decrement the outcome bit
        cpu->BTB_array[btb_index].outcome_bit = change_btb_0(cpu->BTB_array[btb_index].outcome_bit);
        cpu->pc = target_address;
        cpu->fetch_from_next_cycle = TRUE;

        cpu->decode.has_insn = FALSE;

        cpu->fetch.has_insn = TRUE; 
    }
    // Check if the BTB entry is resolved, the calculated address doesn't match and the outcome bit is not 2
    else if (cpu->BTB_array[btb_index].outcome_bit != 2 && cpu->BTB_array[btb_index].completion_status == 1 && cpu->BTB_array[btb_index].calc_target_address != target_address ){
        // Decrement the outcome bit
        cpu->BTB_array[btb_index].outcome_bit = change_btb_0(cpu->BTB_array[btb_index].outcome_bit);
        cpu->pc = target_address;
        cpu->fetch_from_next_cycle = TRUE;

        cpu->decode.has_insn = FALSE;

        cpu->fetch.has_insn = TRUE; 
    }
    // Check if the BTB entry is resolved, the calculated address matches and the outcome bit is not 2
    else if(cpu->BTB_array[btb_index].outcome_bit != 2 && cpu->BTB_array[btb_index].completion_status == 1 && cpu->BTB_array[btb_index].calc_target_address == target_address){
        // Increment the outcome bit
        cpu->BTB_array[btb_index].outcome_bit = change_btb_2(cpu->BTB_array[btb_index].outcome_bit);
        cpu->pc = target_address;
        cpu->fetch_from_next_cycle = TRUE;

        cpu->decode.has_insn = FALSE;

        cpu->fetch.has_insn = TRUE; 
    }
}
