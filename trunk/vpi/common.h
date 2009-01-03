#ifndef COMMON_H
#define COMMON_H

typedef struct t_riscyArg {
  char* name;        // argument name
  vpiHandle handle;  // argument handle
  PLI_INT32 value;   // argument value
} s_riscyArg;

typedef struct t_riscyTest {
  PLI_INT32 op;    // tested operation code/symbol
  PLI_INT32 a, b;  // operands
  PLI_INT32 res;   // expected result
} s_riscyTest;

 /*  alucont
    5'b00000:   // AND, ANDI
    5'b00001:   // OR, ORI
    5'b00010:   // sum: ADD, ADDI, ADDU, ADDUI, SUB, SUBI, SUBU, SUBUI
    5'b00011:   // set if less than: SLT, SLTI
    5'b00100:   // XOR, XORI
    5'b00101:   // NOR, NORI
    5'b00110:   // LUI
  */
typedef enum ALU_CONT {
  AND  = 0x00,
  OR   = 0x01,
  ADD  = 0x02,
  ADDU = 0x02,
  SUB  = 0x22,
  SUBU = 0x22,
  SLT  = 0x23,
  SLTU = 0x23,
  XOR  = 0x04,
  NOR  = 0x05,
  LUI  = 0x06
} ALU_CONT;

/* print crrent entry of the test results table */
void print_table(const s_riscyArg *const args,  // UUT arguments structure
                 const int args_nr,             // number of arguments
                 const double* const time,      // current sim time
                 const int* const counter,      // current clock count
                 const int* const pass);        // test result
/* set UUT argument to int value */
void set_arg_int(const s_riscyArg *const arg,   // UUT arguments structure
                 const int value,               // value to set
                 const int offset,              // set deleay
                 const int zero_at);            //

char* copy_string(char* dest, const char* src);

const s_riscyArg* get_arg_by_name(const s_riscyArg*const args,
                                  const int args_nr,
                                  const char* name);

#endif