#include "stdio.h"
#include "assert.h"
#include "vpi_user.h" /* the VPI library */

#define ARG_NR 7

typedef struct t_riscyArg {
  char* name;
  vpiHandle handle;
  int value;
} s_riscyArg;

typedef struct t_riscyTestLine {
  s_riscyArg* arg;

} s_riscyTestLine;

static s_riscyArg args[ARG_NR];

void print_table(const double* const time);
void set_arg_int(const s_riscyArg *const arg,
                 const int value,
                 const int offset,
                 const int zero_at);
s_riscyArg* get_arg_by_name(const char* name);

static void init()
{
  for (int i=0; i<7; i++) {
    args[i].name = NULL;
    args[i].handle = NULL;
    args[i].value = 0;
  }
}

static int regfileTestCompileTf()
{
  init();
  vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
  vpiHandle argv = vpi_iterate(vpiArgument, sys);
  vpiHandle arg = NULL;

  /* Check for arguments number */
  for (int i=0; i<7; i++) {
    arg = vpi_scan(argv);
    assert(arg);
//    args[i].name = vpi_get_str(vpiName, arg);
//    vpi_printf(" :%d: %s", i, args[i].name);
  }
  vpi_printf("\n");
  for (int i=0; i<7; i++) {
    vpi_printf(" :%d: %s", i, args[i].name);
  }
/*
  //--test
  char text[] = "w_data1";
  s_riscyArg* result;
  if ((result = get_arg_by_name(text)))
    vpi_printf(".:test: %p : %s", (void*)result, result->name);
  else
    vpi_printf(".:test: failure.");
  //--*/

  return 0;
}

static int regfileTestCallTf()
{
  s_vpi_value value;
  s_vpi_time time_s;
  static int counter = 0;
  vpi_printf(".: %d :.", counter);

  vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
  vpiHandle argv = vpi_iterate(vpiArgument, sys);

  for (int i = 0; i < ARG_NR; ++i) {
    args[i].handle = vpi_scan(argv);
    assert(args[i].handle);
    args[i].name = vpi_get_str(vpiName, args[i].handle);
  }

  time_s.type = vpiScaledRealTime;
  vpi_get_time(sys, &time_s);
  double time = time_s.real;

  // set w_enable
  set_arg_int(&args[0], 1, 0, 0);

  value.format = vpiIntVal;
  for (int i = 0; i < ARG_NR; ++i) {
    vpi_get_value(args[i].handle, &value);
    args[i].value = value.value.integer;
  }

  print_table(&time);

  ++counter;
  return 0;
}

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

void print_table(const double* const time)
{
  char t[7] = "\0";
  int result = sprintf(t, "%7g", *time);
  assert(result);
  if(*time == 0) {
    vpi_printf(" [nsec]| %8s %8s | %8s %8s | %8s %8s %8s\n",
               args[1].name, args[5].name, args[2].name, args[6].name,
               args[0].name, args[3].name, args[4].name);
  }
  vpi_printf("%s| %08X %08X | %08X %08X | %08X %08X %08X\n",
              t, args[1].value, args[5].value, args[2].value, args[6].value,
              args[0].value, args[3].value, args[4].value);
}

s_riscyArg* get_arg_by_name(const char* name)
{
  for (int i = 0; i < ARG_NR; ++i) {
    vpi_printf(".: > %s\n", args[i].name);
    if (args[i].name == name)
      return &args[i];
  }
  return NULL;
}

/* Associate C function with new system tesk */
void registerRegfileTestSystfs()
{
  s_vpi_systf_data task_data_s;

  task_data_s.type = vpiSysTask;
  task_data_s.sysfunctype = vpiSysTask;
  task_data_s.tfname = "$regfile_test";
  task_data_s.calltf = regfileTestCallTf;
  task_data_s.compiletf = regfileTestCompileTf;
  task_data_s.sizetf = NULL;
  task_data_s.user_data = "$regfile_test";

  vpi_register_systf(&task_data_s);
}

/* Register new system task */
void (*vlog_startup_routines[])() = {
  registerRegfileTestSystfs,
  0
};
