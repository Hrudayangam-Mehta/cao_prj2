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

        if(!cpu->decode.stalling_value)
        {
            cpu->pc += 4;
            cpu->decode = cpu->fetch;
        }
        else
        {
            cpu->fetch.stalling_value = 1;
        }
        

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
            

            case OPCODE_LOADP:
            {
                // implement forwarding
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                //will get 1 VAL from immediate

                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }

                break;
            }

           

            case OPCODE_STOREP:
            {
                //implement forwarding
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                //will get 1 VAL from immediate

                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }
                if(cpu->regs_status_pending[cpu->decode.rs2] == 1)
                {
                    cpu->decode.rs2_value = cpu->regs_value_pending[cpu->decode.rs2];
                }
                

                break;
            }




            case OPCODE_ADD:
           {

            // implement forwarding 
            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

            if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
            {
                cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
            }
            if (cpu->regs_status_pending[cpu->decode.rs2] == 1)
            {
                cpu->decode.rs2_value = cpu->regs_value_pending[cpu->decode.rs2];
            }
            break;

           }

            case OPCODE_SUB:
            {
                //implement forwarding 
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }
                if (cpu->regs_status_pending[cpu->decode.rs2] == 1)
                {
                    cpu->decode.rs2_value = cpu->regs_value_pending[cpu->decode.rs2];
                }
                break;
            
            }
            
            case OPCODE_MUL:
            {
                //implement forwarding 
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }
                if (cpu->regs_status_pending[cpu->decode.rs2] == 1)
                {
                    cpu->decode.rs2_value = cpu->regs_value_pending[cpu->decode.rs2];
                }
                break;
            
            }
            

            case OPCODE_DIV:
            {
                //implement forwarding 
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }
                if (cpu->regs_status_pending[cpu->decode.rs2] == 1)
                {
                    cpu->decode.rs2_value = cpu->regs_value_pending[cpu->decode.rs2];
                }
                break;
            
            }

            case OPCODE_SUBL:
            {
                //implement forwarding 
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                //will get from immediate

                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }
                break;
            
            }

            case OPCODE_ADDL:
            {
                //implement forwarding 
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                //will get from immediate

                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }
                break;
            
            }

            case OPCODE_AND:
            {
                //implement forwarding 
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }
                if (cpu->regs_status_pending[cpu->decode.rs2] == 1)
                {
                    cpu->decode.rs2_value = cpu->regs_value_pending[cpu->decode.rs2];
                }
                break;
            
            }

            case OPCODE_OR:
            {
                //implement forwarding 
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }
                if (cpu->regs_status_pending[cpu->decode.rs2] == 1)
                {
                    cpu->decode.rs2_value = cpu->regs_value_pending[cpu->decode.rs2];
                }
                break;
            
            }

            case OPCODE_XOR:
            {
                //implement forwarding 
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }
                if (cpu->regs_status_pending[cpu->decode.rs2] == 1)
                {
                    cpu->decode.rs2_value = cpu->regs_value_pending[cpu->decode.rs2];
                }
                break;
                
            }

           
        

            case OPCODE_LOAD:
            {
                //implement forwarding
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                //will get from immediate
                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }
                break;
            }

            case OPCODE_STORE:
            {
                //implement forwarding
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                //will get from immediate
                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }
                if(cpu->regs_status_pending[cpu->decode.rs2] == 1)
                {
                    cpu->decode.rs2_value = cpu->regs_value_pending[cpu->decode.rs2];
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
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }
                if (cpu->regs_status_pending[cpu->decode.rs2] == 1)
                {
                    cpu->decode.rs2_value = cpu->regs_value_pending[cpu->decode.rs2];
                }
                break;

                
            }

            case OPCODE_CML:
            {
                //implement forwarding
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                //will get from immediate
                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
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
                //implement forwarding
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                //will get from immediate
                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }
                break;
            }

            case OPCODE_JALR:
            {
                //implement forwarding
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                //will get from immediate
                if(cpu->regs_status_pending[cpu->decode.rs1] == 1)
                {
                    cpu->decode.rs1_value = cpu->regs_value_pending[cpu->decode.rs1];
                }
                break;
            }

            




        }

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

                cpu->regs_status_pending[cpu->execute.rd] = 1;
                cpu->regs_value_pending[cpu->execute.rd] = cpu->execute.result_buffer;
                break;
                }

                // the below part is not required as we are setting the zero flag in the setZeroFlag function
                // if (cpu->execute.result_buffer == 0)
                // {
                //     cpu->zero_flag = TRUE;
                // } 
                // else 
                // {
                //     cpu->zero_flag = FALSE;
                // }
                // break;
            // }
            

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

                cpu->regs_status_pending[cpu->execute.rd] = 1;
                cpu->regs_value_pending[cpu->execute.rd] = cpu->execute.result_buffer;
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

                cpu->regs_status_pending[cpu->execute.rd] = 1;
                cpu->regs_value_pending[cpu->execute.rd] = cpu->execute.result_buffer;

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

                cpu->regs_status_pending[cpu->execute.rd] = 1;
                  cpu->regs_value_pending[cpu->execute.rd] = cpu->execute.result_buffer;
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
                

                cpu->regs_status_pending[cpu->execute.rd] = 1;
                cpu->regs_value_pending[cpu->execute.rd] = cpu->execute.result_buffer;
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
                

                cpu->regs_status_pending[cpu->execute.rd] = 1;
                cpu->regs_value_pending[cpu->execute.rd] = cpu->execute.result_buffer;
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
                

                cpu->regs_status_pending[cpu->execute.rd] = 1;
                cpu->regs_value_pending[cpu->execute.rd] = cpu->execute.result_buffer;
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
                

                cpu->regs_status_pending[cpu->execute.rd] = 1;
                cpu->regs_value_pending[cpu->execute.rd] = cpu->execute.result_buffer;
                break;
            }

            case OPCODE_ADDL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value + cpu->execute.imm;

                /* Set the zero flag based on the result buffer */
                // Setzero_Flag(cpu);

                // print result buffer
                // printf("Result buffer is %d\n", cpu->execute.result_buffer);
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


                cpu->regs_status_pending[cpu->execute.rd] = 1;
                cpu->regs_value_pending[cpu->execute.rd] = cpu->execute.result_buffer;
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

                cpu->regs_status_pending[cpu->execute.rd] = 1;
                cpu->regs_value_pending[cpu->execute.rd] = cpu->execute.result_buffer;
                
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
                cpu->regs_status_pending[cpu->execute.rd] = 1;
                cpu->regs_value_pending[cpu->execute.rd] = cpu->execute.result_buffer;

                break;
            }

            case OPCODE_BZ:
            {
                if (cpu->zero_flag == TRUE)
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
                if (cpu->zero_flag == FALSE)
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


            case OPCODE_BP:
            {
                if(cpu->pos_flag == TRUE)
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

            case OPCODE_BNP:
            {
                if(cpu->pos_flag == FALSE)
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
                // cpu->regs_status_pending[cpu->execute.rd] = 1;
                // cpu->regs_value_pending[cpu->execute.rd] = cpu->execute.result_buffer;

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

                // cpu->regs_status_pending[cpu->execute.rd] = 1;
                // cpu->regs_value_pending[cpu->execute.rd] = cpu->execute.result_buffer;

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

                cpu->regs_status_pending[cpu->execute.rd] = 1;
                cpu->regs_value_pending[cpu->execute.rd] = cpu->execute.result_buffer;

                break;
                
            }
        

        

        

        }

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Execute", &cpu->execute);
        }
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

        /* Copy data from memory latch to writeback latch*/
        cpu->writeback = cpu->memory;
        cpu->memory.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Memory", &cpu->memory);
        }
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
                cpu->regs_status_pending[cpu->writeback.rd] = 0;
                break;
            }
            
            case OPCODE_SUB:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                //update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                     cpu->regs_status_pending[cpu->writeback.rd] = 0;

                break;
            }
            
            case OPCODE_MUL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                //update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                     cpu->regs_status_pending[cpu->writeback.rd] = 0;
                break;
            }
            case OPCODE_SUBL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                     cpu->regs_status_pending[cpu->writeback.rd] = 0;
                break;
            }
            case OPCODE_ADDL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // update the flags

                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                cpu->regs_status_pending[cpu->writeback.rd] = 0;
                break;
            }

            case OPCODE_DIV:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                //update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                     cpu->regs_status_pending[cpu->writeback.rd] = 0;
                break;
            }
            case OPCODE_AND:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                     cpu->regs_status_pending[cpu->writeback.rd] = 0;
                break;
            }

            case OPCODE_OR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                     cpu->regs_status_pending[cpu->writeback.rd] = 0;
                break;
            }
            case OPCODE_XOR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                     cpu->regs_status_pending[cpu->writeback.rd] = 0;
                break;
            }
            
            case OPCODE_CMP:
            {
                //FOR TESTING PRINT STATEMENT
                // printf("CMP result buffer: %d\n", cpu->writeback.result_buffer);
                // cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // update flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                cpu->regs_status_pending[cpu->writeback.rd] = 0;

                break;
            }
            case OPCODE_CML:
            {
                //FOR TESTING PRINT STATEMENT
                // printf("CML result buffer: %d\n", cpu->writeback.result_buffer);
                // cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // update flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0; 
                cpu->regs_status_pending[cpu->writeback.rd] = 0;

                break;
            }
            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_BP:
            case OPCODE_BNP:
            case OPCODE_BN:
            case OPCODE_BNN:
            {
                break;
            }
            case OPCODE_JUMP:
            {
                // print to say we have reached writeback stage
                printf("Reached writeback stage\n");
                // update flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                cpu->regs_status_pending[cpu->writeback.rd] = 0;
                break;
            }
            case OPCODE_JALR:
            {
                // print to say we have reached writeback stage
                printf("Reached writeback stage\n");

                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                
                // update flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;

                cpu->regs_status_pending[cpu->writeback.rd] = 0;

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
                cpu->regs_status_pending[cpu->writeback.rd] = 0;

                
                break;
            }

            case OPCODE_STORE:
            {
                // cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                // //print to check if this is working
                // printf("STORE result buffer: %d\n", cpu->writeback.result_buffer);
                // update flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                // cpu->fetch.stalling_value = 0;
                // cpu->decode.stalling_value = 0;
                cpu->regs_status_pending[cpu->writeback.rd] = 0;

                break;
            }

            case OPCODE_MOVC: 
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                //update the flags
                cpu->flags_for_regs[cpu->writeback.rd] = 0;
                cpu->regs_status_pending[cpu->writeback.rd] = 0;

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
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock+1);
            printf("--------------------------------------------\n");
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