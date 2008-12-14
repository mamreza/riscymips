#ifndef COMMON_H
#define COMMON_H

typedef struct t_riscyArg {
  char* name;
  vpiHandle handle;
  int value;
} s_riscyArg;

void print_table(const s_riscyArg*const args,
                 const int args_nr,
                 const double* const time,
                 const int* const counter,
                 const int* const pass);
void set_arg_int(const s_riscyArg *const arg,
                 const int value,
                 const int offset,
                 const int zero_at);
char* copy_string(char* dest, const char* src);
const s_riscyArg* get_arg_by_name(const s_riscyArg*const args,
                                  const int args_nr, const char* name);

#endif