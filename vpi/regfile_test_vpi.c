#include "stdlib.h"
#include "stdio.h"
#include "malloc.h"
#include "assert.h"
#include "string.h"
#include "vpi_user.h" /* the VPI library */

#define ARG_NR 7
#define REGISTERS_NR 32

typedef struct t_riscyArg {
  char* name;
  vpiHandle handle;
  int value;
} s_riscyArg;

/*
typedef struct t_riscyTestLine {
  s_riscyArg* arg;

} s_riscyTestLine;
*/
static s_riscyArg args[ARG_NR];
static double g_time;

void print_table(const double* const time,
                 const int* const counter,
                 const int* const pass);
void set_arg_int(const s_riscyArg *const arg,
                 const int value,
                 const int offset,
                 const int zero_at);
char* copy_string(char* dest, const char* src);
s_riscyArg* get_arg_by_name(const char* name);
void setup_register(const int index, const int value);
static void update_time(void);
static void bomb(void);
static void init(void);
static void cleanup(void);

static int regfileTestCompileTf()
{
  vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
  vpiHandle argv = vpi_iterate(vpiArgument, sys);
  vpiHandle arg = NULL;

  init();
  /* Check for arguments number */
  for (int i=0; i<7; i++) {
    arg = vpi_scan(argv);
    assert(arg);
    args[i].handle = arg;
    char* name = vpi_get_str(vpiName, arg);
    if (!(args[i].name = copy_string(args[i].name, name))) bomb();
  }
  return 0;
}

static int regfileTestCallTf()
{
  static int counter = 0; // countes the clock cycles
  update_time();

  if (counter < REGISTERS_NR) {
    switch (counter) {
      case 0: setup_register(0, 0); break;
      case 1: setup_register(1, 402); break;
      default: setup_register(counter, g_time); break;break;
    }
  } else {
    vpi_control(vpiFinish);
  }
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

void print_table(const double* const time,
                 const int* const counter,
                 const int* const pass)
{
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
  if(*time == 0) {
    vpi_printf(" [nr]  [nsec]| %8s %8s | %8s %8s | %8s %8s %8s | [result]\n",
                args[1].name, args[5].name, args[2].name, args[6].name,
                args[0].name, args[3].name, args[4].name);
  }
  vpi_printf(".:%2d: %s| %08X %08X | %08X %08X | %08X %08X %08X | %4s\n",
              *counter, t, args[1].value, args[5].value, args[2].value,
              args[6].value, args[0].value, args[3].value, args[4].value, p);
}

char* copy_string(char* dest, const char* src)
{
  int len = strlen(src);
  dest = NULL;
  dest = (char*)malloc(len+1);
  if(!dest) return NULL;
  strncpy(dest, src, len+1);
//  vpi_printf(".: dest: %s\n", dest);
  return dest;
}

s_riscyArg* get_arg_by_name(const char* name)
{
  for (int i = 0; i < ARG_NR; ++i) {
    if (args[i].name == name)
//      vpi_printf(".: > %s\n", args[i].name);
      return &args[i];
  }
  vpi_printf("...FATAL_ERROR: %s, %d, %s()", __FILE__, __LINE__, __FUNCTION__);
  return NULL;
}

void setup_register(const int index, const int value)
{
  set_arg_int(&args[0], 1, 0, 40); //set_arg_int(arg, value, offset, zero_at)
  set_arg_int(&args[3], index, 0, 40); //w_addr1 -> 3
  set_arg_int(&args[4], value, 0, 40); //w_data1 -> 4
  set_arg_int(&args[1], index, 0, 40); //r_addr1 -> 1
  set_arg_int(&args[2], index, 0, 40); //r_addr2 -> 2
}

void update_time(void)
{
  s_vpi_time time_s;
  vpiHandle sys = vpi_handle(vpiSysTfCall, 0);

  time_s.type = vpiScaledRealTime;
  vpi_get_time(sys, &time_s);
  g_time = time_s.real;
}

void bomb(void)
{
  cleanup();
  exit(1);
}

void init(void)
{
  for (int i=0; i<7; i++) {
    args[i].name = "\n";
    args[i].handle = NULL;
    args[i].value = 0;
  }
}

void cleanup(void)
{
  vpi_printf("...cleanup()\n");
  for (int i=0; i<7; i++) {
    if(args[i].name)
      free(args[i].name);
  }
}

// -- CHECK
static int regfileCheckCompileTf()
{
  return 0;
}

static int regfileCheckCallTf()
{
  s_vpi_value value;
  static int counter = 0; // countes the clock cycles
  int passed = 0;

  update_time();

  value.format = vpiIntVal;
  for (int i = 0; i < ARG_NR; ++i) {
    vpi_get_value(args[i].handle, &value);
    args[i].value = value.value.integer;
  }

  // compare w_data with r_data
  if (args[4].value == args[5].value
      && args[4].value == args[6].value
      && args[0].value != 0)
    passed = 1;
  else if (args[0].value != 0)
    passed = -1;
  else
    passed = 0;

  print_table(&g_time, &counter, &passed);

  ++counter;
  return 0;
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

  task_data_s.type = vpiSysTask;
  task_data_s.sysfunctype = vpiSysTask;
  task_data_s.tfname = "$regfile_check";
  task_data_s.calltf = regfileCheckCallTf;
  task_data_s.compiletf = regfileCheckCompileTf;
  task_data_s.sizetf = NULL;
  task_data_s.user_data = "$regfile_check";

  vpi_register_systf(&task_data_s);

  /* Arrange for a cleanup function to be called when the
     simulation is finished. */
  s_cb_data cb_data;

  cb_data.reason = cbEndOfSimulation;
  cb_data.cb_rtn = cleanup;
  cb_data.obj    = 0;
  cb_data.time   = 0;

  vpi_register_cb(&cb_data);
}

/* Register new system task */
void (*vlog_startup_routines[])() = {
  registerRegfileTestSystfs,
  0
};
