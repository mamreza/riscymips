#ifndef COMMON_H
#define COMMON_H

typedef struct t_riscyArg {
  char* name;
  vpiHandle handle;
  int value;
} s_riscyArg;

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