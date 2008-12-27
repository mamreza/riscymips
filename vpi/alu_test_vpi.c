#include <stdlib.h>
#include <assert.h>
#include "vpi_user.h" /* the VPI library */
#include "common.h"

#define ARGS_NR 4

s_riscyArg args[ARGS_NR];
static double g_time;

static void update_time(void);
static void bomb(void);
static void init(void);
static void cleanup(void);

static int aluTestCompileTf()
{
  vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
  vpiHandle argv = vpi_iterate(vpiArgument, sys);
  vpiHandle arg = NULL;

  init();
  /* Check for arguments number */
  for (int i = 0; i < ARGS_NR; i++) {
    arg = vpi_scan(argv);
    assert(arg);
    args[i].handle = arg;
    char* name = vpi_get_str(vpiName, arg);
    if (!(args[i].name = copy_string(args[i].name, name))) bomb();
  }
  return 0;
}

static int aluTestCallTf()
{
  static int counter = 0; // countes the clock cycles
  update_time();
/*
      6'b100000: alucontrol = 3'b010; // ADD
      6'b100010: alucontrol = 3'b110; // SUB
      6'b100100: alucontrol = 3'b000; // AND
      6'b100101: alucontrol = 3'b001; // OR
      6'b101010: alucontrol = 3'b111; // SLT */

  switch (counter) {
    case 0: // add
      /* (arg, value, offset, zero_at) */
      set_arg_int(&args[0], 0xF, 0, 40);  // a
      set_arg_int(&args[1], 0xFF, 0, 40); // b
      set_arg_int(&args[2], 2, 0, 40); // alucont
      break;
    case 1: // sub
      set_arg_int(&args[0], 0xF, 0, 40);  // a
      set_arg_int(&args[1], 0xFF, 0, 40); // b
      set_arg_int(&args[2], 6, 0, 40); // alucont
      break;
    case 2: // and
      set_arg_int(&args[0], 0xF, 0, 40);  // a
      set_arg_int(&args[1], 0xFF, 0, 40); // b
      set_arg_int(&args[2], 0, 0, 40); // alucont
      break;
    case 3: // or
      set_arg_int(&args[0], 0xF, 0, 40);  // a
      set_arg_int(&args[1], 0xFF, 0, 40); // b
      set_arg_int(&args[2], 1, 0, 40); // alucont
      break;
    case 4: // slt
      set_arg_int(&args[0], 0xF, 0, 40);  // a
      set_arg_int(&args[1], 0xFF, 0, 40); // b
      set_arg_int(&args[2], 7, 0, 40); // alucont
      break;
    default: vpi_control(vpiFinish); break;
  }
  ++counter;
  return 0;
}

void update_time(void)
{
  s_vpi_time time_s;  // time structure
  vpiHandle sys = vpi_handle(vpiSysTfCall, 0);  // get vpi sys handle

  time_s.type = vpiScaledRealTime;  // set type
  vpi_get_time(sys, &time_s);       // get current simulation time
  g_time = time_s.real;             // set global variable
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

void cleanup(void)
{
  vpi_printf("...%s, cleanup()\n", __FILE__);
  for (int i = 0; i < ARGS_NR; i++) {
    if(args[i].name)
      free(args[i].name);
  }
}

// -- CHECK
static int aluCheckCompileTf()
{
  return 0;
}

static int aluCheckCallTf()
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

  int result = 0;
  switch (counter) {
    case 0: result = args[0].value + args[1].value; break;
    case 1: result = args[0].value - args[1].value; break;
    case 2: result = args[0].value & args[1].value; break;
    case 3: result = args[0].value | args[1].value; break;
    case 4: result = (args[0].value < args[1].value); break;
    default: break;
  }
  // compare w_data with r_data
  if (args[3].value == result)
    passed = 1;
  else
    passed = -1;

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
  task_data_s.tfname = "$alu_test";
  task_data_s.calltf = aluTestCallTf;
  task_data_s.compiletf = aluTestCompileTf;
  task_data_s.sizetf = NULL;
  task_data_s.user_data = "$alu_test";

  vpi_register_systf(&task_data_s);

  task_data_s.type = vpiSysTask;
  task_data_s.sysfunctype = vpiSysTask;
  task_data_s.tfname = "$alu_check";
  task_data_s.calltf = aluCheckCallTf;
  task_data_s.compiletf = aluCheckCompileTf;
  task_data_s.sizetf = NULL;
  task_data_s.user_data = "$alu_check";

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
