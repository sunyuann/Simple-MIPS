// smips program by LUM SUN YUAN, z5289055, 31/7/2020
// COMP1521 Assignment 2 (smips, Simple MIPS)

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INSTRUCTION_CODES 1000
#define MAX_REGISTERS 32
#define MAX_OUTPUT_VALUES 1000

#define OPCODE1 0
#define OPCODE2 1
#define S_VALUE 2
#define T_VALUE 3
#define D_VALUE 4
#define I_VALUE 5


void separate_bit_pattern(int code, int separated_bit[6]);
void print_instruction(int code, int instruction_number);
int execute_instruction(int code, int registers[MAX_REGISTERS]);

int main(int argc, char *argv[]) {
    
    if (argc != 2) { // check have command line argument
        printf("Invalid number of arguments\n");
        return 1;
    }
    
    FILE *input_file = fopen(argv[1], "r");
    
    if (input_file == NULL) { // checks if open suceeded
        perror(argv[1]);
        return 1;
    }
    
    // array for hex codes
    int codes[MAX_INSTRUCTION_CODES];
    
    int num_instructions = 0;
    
    // while loop runs as long as the number of lines of code does not 
    // exceed MAX_INSTRUCTION_CODES.
    // puts every line of code into the codes array as a hexadecimal value. 
    while (num_instructions < MAX_INSTRUCTION_CODES && 
        fscanf(input_file, "%x", &codes[num_instructions]) == 1) {
        // adds 1 to num_instructions every time the while loop runs
        num_instructions++;
    }
        
    // print "Program\n" before printing interpreted instructions
    printf("Program\n");
    int i = 0;
    while (i < num_instructions) { 
        // while loop runs through the number of lines of code and 
        // prints out all the instructions using print_instruction
        print_instruction(codes[i], i);
        i++;
    }
    
    // print "Output\n" before printing interpreted output
    printf("Output\n");
    
    // array for register values
    // initialize all registers array values to 0
    int registers[MAX_REGISTERS] = {0};
    
    i = 0;
    while (i < num_instructions) { // loops through all lines of code 
        int offset = execute_instruction(codes[i], registers);
        i = i + offset;
        if (i >= MAX_INSTRUCTION_CODES) {
            // if i >= MAX_INSTRUCTION_CODES, it means that offset = 1000
            // (meaning that the exit instruction has been issued or an error
            // has occurred while executing syscall instruction)
            // thus, break and exit the loop
            break;
        }
        if (offset == 0) {
            // if no offset, add 1 to i and read the next line as 
            // the order of lines is unchanged
            i++;
        } 
    }
    
    // print "Registers After Execution\n" before printing non-zero registers
    printf("Registers After Execution\n");
    i = 0;
    while (i < MAX_REGISTERS) { // while loop to loop through registers array
        if (registers[i] != 0) { // checks if registers[i] is non-zero
            if (i < 10) { // if i < 10, print an extra space for padding
                printf("$%d  = %d\n", i, registers[i]);
            } else if (i >= 10) { // i >= 10, print without extra space
                printf("$%d = %d\n", i, registers[i]);
            }     
        } 
        i++;
    }
             
    return 0;    
}

// Function to separate bit patterns into opcode 1, opcode 2, s, t, d and
// I. These separated bit pattern values are put into the separated_bit 
// array.
// The array is for separated bit patterns, given by:
// array for separated bit patterns
// separated_bit[OPCODE1] = opcode 1 (bits 26-31) (left hand side)
// separated_bit[OPCODE2] = opcode 2 (bits 0-10) (right hand side)
// separated_bit[S_VALUE] = s (bits 21-25) (5-bit register number)
// separated_bit[T_VALUE] = t (bits 16-20) (5-bit register number)
// separated_bit[D_VALUE] = d (bits 11-15) (5-bit register number)
// separated_bit[I_VALUE] = I (bits 0-15) (16-bit signed number)
void separate_bit_pattern(int code, int separated_bit[6]) {
    // obtain opcode 1 and put into separated_bit[OPCODE1]
    separated_bit[OPCODE1] = code >> 26;
    
    // create mask for opcode 2
    int opcode2_mask = (1 << 11) - 1;
    // obtain opcode 2 and put into separated_bit[OPCODE2]
    separated_bit[OPCODE2] = code & opcode2_mask;
    
    // create mask for all register values (5-bits)
    int register_mask = (1 << 5) - 1;
    // obtain s and put into separated_bit[S_VALUE]
    separated_bit[S_VALUE] = (code >> 21) & register_mask;
    // obtain t and put into separated_bit[T_VALUE]
    separated_bit[T_VALUE] = (code >> 16) & register_mask;
    // obtain t and put into separated_bit[D_VALUE]
    separated_bit[D_VALUE] = (code >> 11) & register_mask;
    
    // create mask for immediate value, I (without sign)
    int immediate_value_mask = (1 << 15) - 1;
    
    // obtain I and put into separated_bit[I_VALUE]
    int immediate_without_sign = code & immediate_value_mask;
    
    // obtain sign value for immediate value, I 
    int immediate_sign_bit = (code >> 15) & 1;
    
    if (immediate_sign_bit == 0) { // immediate value is positive
        // put positive I into separated_bit[I_VALUE]
        separated_bit[I_VALUE] = immediate_without_sign;
    } else if (immediate_sign_bit == 1) { // immediate value is negative
        
        // reverse bits (use ~ bitwise operator and add 1 to reversed
        // bits)
        immediate_without_sign = (~immediate_without_sign) 
        & immediate_value_mask;
        immediate_without_sign++;
        
        // put negative I into separated_bit[I_VALUE]
        separated_bit[I_VALUE] = (-1) * immediate_without_sign;
    }
    return;
}

// Program to print instruction of given code by analyzing bit patterns
// and determining the instruction type and bit patterns for individual
// instructions.
void print_instruction(int code, int instruction_number) {
    
    printf("%3d: ", instruction_number);
    
    int separated_bit[6];
    // use separate_bit_pattern function to separate bit patterns and put
    // into separated_bit array
    separate_bit_pattern(code, separated_bit);
    
    // put separated_bit array values into int for easier method of inputting
    int s = separated_bit[S_VALUE];
    int t = separated_bit[T_VALUE];
    int d = separated_bit[D_VALUE];
    int immediate = separated_bit[I_VALUE];
    
    // checks opcode 1 to see if it matches 0 or 28, which indicates that
    // the instructions involves 3 registers (or syscall)
    if ((separated_bit[OPCODE1] == 0) || (separated_bit[OPCODE1] == 28)) {

        // checks and matches opcode 2 to determine the unique instruction
        // given by the code
        
        if (separated_bit[OPCODE2] == 12) { // syscall
            printf("syscall\n"); // prints syscall and ends function
            return;
        } else if (separated_bit[OPCODE2] == 32) { // add instruction 
            printf("add  "); // prints add 
        } else if (separated_bit[OPCODE2] == 34) { // sub instruction 
            printf("sub  "); // prints sub
        } else if (separated_bit[OPCODE2] == 36) { // and instruction 
            printf("and  "); // prints and
        } else if (separated_bit[OPCODE2] == 37) { // or instruction
            printf("or   "); // prints or
        } else if (separated_bit[OPCODE2] == 42) { // slt instruction
            printf("slt  "); // prints slt
        } else if (separated_bit[OPCODE2] == 2) { // mul instruction
            printf("mul  "); // prints mul
        }
        // prints the three registers in order of d, s, t 
        printf("$%d, $%d, $%d\n", d, s, t);
        return; // ends function
        
    } else if (separated_bit[OPCODE1] == 4) { // beq instruction
        // checks opcode 1 to see if it matches 4 (beq instruction)
        // if true, print beq function
        printf("beq  $%d, $%d, %d\n", s, t, immediate);
            
    } else if (separated_bit[OPCODE1] == 5) { // bne instruction
        // checks opcode 1 to see if it matches 5 (bne instruction)
        // if true, print bne function
        printf("bne  $%d, $%d, %d\n", s, t, immediate);
            
    } else if (separated_bit[OPCODE1] == 8) { // addi instruction
        // checks opcode 1 to see if it matches 8 (addi instruction)
        // if true, print addi function
        printf("addi $%d, $%d, %d\n", t, s, immediate);
            
    } else if (separated_bit[OPCODE1] == 10) { // slti instruction
        // checks opcode 1 to see if it matches 10 (slti instruction)
        // if true, print slti function
        printf("slti $%d, $%d, %d\n", t, s, immediate);
            
    } else if (separated_bit[OPCODE1] == 12) { // andi instruction
        // checks opcode 1 to see if it matches 12 (andi instruction)
        // if true, print andi function
        printf("andi $%d, $%d, %d\n", t, s, immediate);
            
    } else if (separated_bit[OPCODE1] == 13) { // ori instruction
        // checks opcode 1 to see if it matches 13 (ori instruction)
        // if true, print ori function
        printf("ori  $%d, $%d, %d\n", t, s, immediate);
            
    } else if (separated_bit[OPCODE1] == 15) { // lui instruction
        // checks opcode 1 to see if it matches 15 (lui instruction)
        // if true, print lui function
        printf("lui  $%d, %d\n", t, immediate);
    } 
    
    return;
}

// Program to execute instruction of given code by analyzing bit patterns
// and determining the instruction type and bit patterns for individual
// instructions. The return_value is the offset value.
int execute_instruction(int code, int registers[MAX_REGISTERS]) {
 
    // initialise return_value to be 0
    int return_value = 0;   
    
    int separated_bit[6];
    // use separate_bit_pattern function to separate bit patterns and put
    // into separated_bit array
    separate_bit_pattern(code, separated_bit);
    
    // put separated_bit array values into int for easier method of inputting
    int s = separated_bit[S_VALUE];
    int t = separated_bit[T_VALUE];
    int d = separated_bit[D_VALUE];
    int immediate = separated_bit[I_VALUE];
    
    // checks opcode 1 to see if it matches 0 or 28, which indicates that
    // the instructions involves 3 registers (or syscall)
    if ((separated_bit[OPCODE1] == 0) || (separated_bit[OPCODE1] == 28)) {

        // checks and matches opcode 2 to determine the unique instruction
        // given by the code
        
        if (separated_bit[OPCODE2] == 12) { // syscall
            // check request type by checking value in $v0 ($2)
            if (registers[2] == 1) { 
                // if $2 is equal to 1, then print integer in $a0 ($4)
                printf("%d", registers[4]);
            } else if (registers[2] == 11) {
                // if $2 is equal to 11, then print char in $a0 ($4)
                printf("%c", registers[4]);
            } else if (registers[2] == 10) {
                // if $2 is equal to 10, then exit program by adding 1000 to 
                // the return_value, which is checked everytime the main loop
                // runs and ends if i is bigger than 1000
                return_value = 1000;
            } else { 
                // if syscall is not recognised (unknown syscall),
                // supply an error message
                printf("Unknown system call: %d\n", registers[2]);
                return_value = 1000;
            }  
            
        } else if (separated_bit[OPCODE2] == 32) { // add instruction
            if (d != 0) { // execute instruction if $d is not $0
                // $d = $s + $t; 
                registers[d] = registers[s] + registers[t];
            }
        } else if (separated_bit[OPCODE2] == 34) { // sub instruction 
            if (d != 0) { // execute instruction if $d is not $0
                // $d = $s - $t; 
                registers[d] = registers[s] - registers[t];
            }
        } else if (separated_bit[OPCODE2] == 36) { // and instruction 
            if (d != 0) { // execute instruction if $d is not $0
                // $d = $s & $t; 
                registers[d] = registers[s] & registers[t];
            }
        } else if (separated_bit[OPCODE2] == 37) { // or instruction
            if (d != 0) { // execute instruction if $d is not $0
                // $d = $s | $t; 
                registers[d] = registers[s] | registers[t];
            }
        } else if (separated_bit[OPCODE2] == 42) { // slt instruction
            if (d != 0) { // execute instruction if $d is not $0
                // $d = 1 if $s < $t else 0
                if (registers[s] < registers[t]) { // if $s < $t
                    // set $d to 1
                    registers[d] = 1;
                } else { // if $s >= $t
                    // set $d to 0
                    registers[d] = 0;
                }
            }
        } else if (separated_bit[OPCODE2] == 2) { // mul instruction
            if (d != 0) { // execute instruction if $d is not $0
                // $d = $s * $t; 
                registers[d] = registers[s] * registers[t];
            }
        }

    } else if (separated_bit[OPCODE1] == 4) { // beq instruction
        // checks opcode 1 to see if it matches 4 (beq instruction)
        // if true, execute beq function
        if (registers[s] == registers[t]) { 
            // checks if value in registers[s] is equal to registers[t]
            // if equal, set return_value (offset) to immediate, I
            // else do nothing
            return_value = immediate;
        }
            
    } else if (separated_bit[OPCODE1] == 5) { // bne instruction
        // checks opcode 1 to see if it matches 5 (bne instruction)
        // if true, execute bne function
        if (registers[s] != registers[t]) {
            // checks if value in registers[s] is equal to registers[t]
            // if not equal, set return value (offset) to immediate, I
            // else do nothing
            return_value = immediate;
        }
            
    } else if (separated_bit[OPCODE1] == 8) { // addi instruction
        // checks opcode 1 to see if it matches 8 (addi instruction)
        // if true, execute addi function
        if (t != 0) { // execute instruction if $t is not $0
            // $t = $s + I
            registers[t] = registers[s] + immediate;
        }
            
    } else if (separated_bit[OPCODE1] == 10) { // slti instruction
        // checks opcode 1 to see if it matches 10 (slti instruction)
        // if true, execute slti function
        if (t != 0) { // execute instruction if $t is not $0
            // $t = ($s < I)
            // $t = 1 if $s < immediate, I else 0
            if (registers[s] < immediate) { // is $s < I
                // set $t to 1
                registers[t] = 1;
            } else { // if $s >= I
                // set $t to 0
                registers[t] = 0;
            }
        }
            
    } else if (separated_bit[OPCODE1] == 12) { // andi instruction
        // checks opcode 1 to see if it matches 12 (andi instruction)
        // if true, execute andi function
        if (t != 0) { // execute instruction if $t is not $0
            // $t = $s & I
            registers[t] = registers[s] & immediate;
        }
            
    } else if (separated_bit[OPCODE1] == 13) { // ori instruction
        // checks opcode 1 to see if it matches 13 (ori instruction)
        // if true, execute ori function
        if (t != 0) { // execute instruction if $t is not $0
            // $t = $s | I
            registers[t] = registers[s] | immediate;
        }
            
    } else if (separated_bit[OPCODE1] == 15) { // lui instruction
        // checks opcode 1 to see if it matches 15 (lui instruction)
        // if true, execute lui function 
        if (t != 0) { // execute instruction if $t is not $0
            // $t = I << 16
            registers[t] = immediate << 16;   
        }
    } 
    
    return return_value;            
}
