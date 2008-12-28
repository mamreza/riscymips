#include <stdlib.h>
#include <assert.h>
#include <vpi_user.h> /* the VPI library */
#include "common.h"

/*
  6'b100000: alucontrol = 6'b000010; // ADD
  6'b100010: alucontrol = 6'b100010; // SUB
  6'b100100: alucontrol = 6'b000000; // AND
  6'b100101: alucontrol = 6'b000001; // OR
  6'b101010: alucontrol = 6'b100011; // SLT
 */

typedef enum ALU_FUNC {
  AND = 0x00,
  OR  = 0x01,
  ADD = 0x02,
  SUB = 0x22,
  SLT = 0x23
} ALU_FUNC;

#define ARGS_NR 5
#define TESTS_NR 10
static s_riscyArg  args[ARGS_NR];
static s_riscyTest tests[TESTS_NR];
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
  for (int i = 0; i < ARGS_NR; ++i) {
    arg = vpi_scan(argv);
    assert(arg);
    args[i].handle = arg;
    char* name = vpi_get_str(vpiName, arg);
    if (!(args[i].name = copy_string(args[i].name, name))) bomb();
  }
  /* initialize the tests */
  for (int i = 0; i < TESTS_NR; ++i) {
    switch (i) {
      case 0: tests[i].op = ADD;
              tests[i].a = 0xF;
              tests[i].b = 0xFF;
              tests[i].res = tests[i].a + tests[i].b;
        break;
      case 1: tests[i].op = SUB;
              tests[i].a = 0xF;
              tests[i].b = 0xFF;
              tests[i].res = tests[i].a - tests[i].b;
        break;
      case 2: tests[i].op = AND;
              tests[i].a = 0xF;
              tests[i].b = 0xFF;
              tests[i].res = tests[i].a & tests[i].b;
        break;
      case 3: tests[i].op = OR;
              tests[i].a = 0xF;
              tests[i].b = 0xFF;
              tests[i].res = tests[i].a | tests[i].b;
        break;
      case 4: tests[i].op = SLT;
              tests[i].a = 0xF;
              tests[i].b = 0xFF;
              tests[i].res = (tests[i].a < tests[i].b);
        break;
      case 5: tests[i].op = ADD;
              tests[i].a = 0xFFFFFFFF;
              tests[i].b = 0x1;
              tests[i].res = tests[i].a + tests[i].b;
        break;
      case 6: tests[i].op = SUB;
              tests[i].a = 0x0;
              tests[i].b = 0xFFFFFFFF;
              tests[i].res = tests[i].a - tests[i].b;
        break;
      default:tests[i].op = -1;
              tests[i].a = 0x0;
              tests[i].b = 0x0;
              tests[i].res = 0;
        break;
    }
  }
  return 0;
}

static int aluTestCallTf()
{
  static int counter = 0; // countes the clock cycles
  update_time();

  /* (arg, value, offset, zero_at) */
  set_arg_int(&args[0], tests[counter].a, 0, 40);  // a
  set_arg_int(&args[1], tests[counter].b, 0, 40); // b
  set_arg_int(&args[2], tests[counter].op, 0, 40); // alucont

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

  static int success = 1;

  // compare w_data with r_data
  if (args[3].value == tests[counter].res)
    passed = 1;
  else {
    passed = -1;
    success = 0;
  }

  print_table(args, ARGS_NR, &g_time, &counter, &passed);

  ++counter;

  if (tests[counter].op == -1 || counter >= TESTS_NR) {
    if (success)
      vpi_control(vpiFinish);
    else
      vpi_control(vpiStop);
  }

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
