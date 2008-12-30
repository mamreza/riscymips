#include <stdlib.h>
#include <assert.h>
#include "vpi_user.h" /* the VPI library */
#include "common.h"

#define ARGS_NR 7
#define REGISTERS_NR 32

s_riscyArg args[ARGS_NR];
static double g_time;

static void setup_register(const int index, const int value);
static void update_time(void);
static void bomb(void);
static void init(void);
static PLI_INT32 cleanup();

static PLI_INT32 regfileTestCompileTf()
{
  vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
  vpiHandle argv = vpi_iterate(vpiArgument, sys);
  vpiHandle arg = NULL;

  init();
  /* Check for arguments number */
  for (int i=0; i < ARGS_NR; i++) {
    arg = vpi_scan(argv);
    assert(arg);
    args[i].handle = arg;
    char* name = vpi_get_str(vpiName, arg);
    if (!(args[i].name = copy_string(args[i].name, name))) bomb();
  }
  return 0;
}

static PLI_INT32 regfileTestCallTf()
{
  static int counter = 0; // countes the clock cycles
  update_time();

  if (counter < REGISTERS_NR) {
    switch (counter) {
      case 0: setup_register(0, 0); break;
      case 1: setup_register(1, 402); break;
      default: setup_register(counter, g_time); break;
    }
  } else {
    vpi_control(vpiFinish);
  }
  ++counter;
  return 0;
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
  for (int i = 0; i < ARGS_NR; i++) {
    args[i].name = "\n";
    args[i].handle = NULL;
    args[i].value = 0;
  }
}

static PLI_INT32 cleanup()
{
  vpi_printf("...%s, cleanup()\n", __FILE__);
  for (int i = 0; i < ARGS_NR; i++) {
    if(args[i].name)
      free(args[i].name);
  }
  return 0;
}

// -- CHECK
static PLI_INT32 regfileCheckCompileTf()
{
  return 0;
}

static PLI_INT32 regfileCheckCallTf()
{
  s_vpi_value value;
  static int counter = 0; // countes the clock cycles
  int passed = 0;

  update_time();

  value.format = vpiIntVal;
  for (int i = 0; i < ARGS_NR; ++i) {
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

  print_table(args, ARGS_NR, &g_time, &counter, &passed);

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
