#ifndef COMMON_H
#define COMMON_H

typedef struct t_riscyArg {
  char* name;  // argument name
  vpiHandle handle;  // argument handle
  int value;   // argument value
} s_riscyArg;

typedef struct t_riscyTest {
  int op;  // tested operation code/symbol
  long long a, b;       // operands
  long long res;  // expected result
} s_riscyTest;

/* print crrent entry of the test results table */
void print_table(const s_riscyArg *const args,  // UUT arguments structure
                 const int args_nr,            // number of arguments
                 const double* const time,     // current sim time
                 const int* const counter,     // current clock count
                 const int* const pass);       // test result
/* set UUT argument to int value */
void set_arg_int(const s_riscyArg *const arg,  // UUT arguments structure
                 const int value,              // value to set
                 const int offset,             // set deleay
                 const int zero_at);           //

char* copy_string(char* dest, const char* src);

const s_riscyArg* get_arg_by_name(const s_riscyArg*const args,
                                  const int args_nr,
                                  const char* name);

#endif