// #include <stdlib.h>
// #include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include "vpi_user.h" /* the VPI library */
#include "common.h"

void set_arg_int(const s_riscyArg *const arg,
                 const int value,
                 const int offset,
                 const int zero_at)
{
  s_vpi_value value_s;
  s_vpi_time time_s;

  value_s.format = vpiIntVal;
  time_s.type = vpiScaledRealTime;
  value_s.value.integer = value;
  if (!offset) {
    vpi_put_value(arg->handle, &value_s, NULL, vpiNoDelay);
  } else {
    time_s.real = offset;
    vpi_put_value(arg->handle, &value_s, &time_s, vpiInertialDelay);
  }
  if (zero_at) {
    time_s.real = zero_at;
    value_s.value.integer = 0;
    vpi_put_value(arg->handle, &value_s, &time_s, vpiInertialDelay);
  }
}

void print_table(const s_riscyArg*const args,
                 const int args_nr,
                 const double* const time,
                 const int* const counter,
                 const int* const pass)
{
  static int go = 1;
  char t[7] = "\0";
  char ok[] = "OK";
  char fail[] = "fail";
  char none[] = "--";
  char* p = "\0";
  switch (*pass) {
    case 0: p = none; break;
    case 1: p = ok; break;
    case -1: p = fail; break;
    default: break;
  }
  int result = sprintf(t, "%7g", *time);
  assert(result);
  if(go) {
    vpi_printf(" [nr]  [nsec]| ");
    for (int i = 0; i < args_nr; ++i) {
      vpi_printf("%8s ", args[i].name);
    }
    vpi_printf("| [result]\n");
    go = 0;
  }
  vpi_printf(".:%2d: %s| ", *counter, t);
  for (int i = 0; i < args_nr; ++i) {
    vpi_printf("%08X ", args[i].value);
  }
  vpi_printf("| %4s\n", p);
}

char* copy_string(char* dest, const char* src)
{
  int len = strlen(src);
  dest = NULL;
  dest = (char*)malloc(len+1);
  if(!dest) return NULL;
  strncpy(dest, src, len+1);
  return dest;
}

const s_riscyArg* get_arg_by_name(const s_riscyArg*const args,
                                  const int args_nr, const char* name)
{
  for (int i = 0; i < args_nr; ++i) {
    if (args[i].name == name)
      return &args[i];
  }
  vpi_printf("...FATAL_ERROR: %s, %d, %s()", __FILE__, __LINE__, __FUNCTION__);
  return NULL;
}