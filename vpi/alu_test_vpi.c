#include <stdlib.h>
#include <assert.h>
#include <vpi_user.h> /* the VPI library */
#include "common.h"

/*
  //6'b000000: // sll - shift left logical
  //6'b000010: // srl - shift right logical
  //6'b000011: // sra - shift right arithmetic
  //6'b000100: // sllv
  //6'b000110: // srlv
  //6'b000111: // srav
  //6'b001000: // jr
  //6'b001001: // jalr
  //6'b001100: // syscall
  //6'b001101: // break
  6'b100000: alucontrol = 6'b000010; // ADD
  6'b100001: alucontrol = 6'b000010; // ADDU
  6'b100010: alucontrol = 6'b100010; // SUB
  6'b100011: alucontrol = 6'b100010; // SUBU
  6'b100100: alucontrol = 6'b000000; // AND
  6'b100101: alucontrol = 6'b000001; // OR
  6'b100110: alucontrol = 6'b000011; // XOR
  6'b100111: alucontrol = 6'b000100; // NOR
  6'b101010: alucontrol = 6'b100011; // SLT
  //6'b101011: // SLTU
 */

typedef enum ALU_FUNC {
  AND = 0x00,
  OR  = 0x01,
  ADD = 0x02,
  SUB = 0x22,
  SLT = 0x23,
  XOR = 0x04,
  NOR = 0x05
} ALU_FUNC;

#define ARGS_NR 5
#define TESTS_NR 10
static s_riscyArg  args[ARGS_NR];
static s_riscyTest tests[TESTS_NR];
static double g_time;

static void update_time(void);
static void bomb(void);
static void init(void);
static PLI_INT32 cleanup(t_cb_data*);

static PLI_INT32 aluTestCompileTf(PLI_BYTE8*)
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
              tests[i].a = 0x7FFFFFFF;
              tests[i].b = 0x7FFFFFFF;
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
      case 4: tests[i].op = XOR;
              tests[i].a = 0xF;
              tests[i].b = 0xFF;
              tests[i].res = tests[i].a ^ tests[i].b;
        break;
      case 5: tests[i].op = NOR;
              tests[i].a = 0xF;
              tests[i].b = 0xFF;
              tests[i].res = ~(tests[i].a | tests[i].b);
        break;
      case 6: tests[i].op = SLT;
              tests[i].a = 0xF;
              tests[i].b = 0xFF;
              tests[i].res = (tests[i].a < tests[i].b);
        break;
      case 7: tests[i].op = ADD;  // make an overflow
              tests[i].a = 0xFFFFFFFF;
              tests[i].b = 0x1;
              tests[i].res = tests[i].a + tests[i].b;
        break;
      case 8: tests[i].op = SUB;  // make an overflow
              tests[i].a = 0xFFFFFFFF;
              tests[i].b = -0x1;  // the same as 0xFFFFFFFF
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

static PLI_INT32 aluTestCallTf(PLI_BYTE8*)
{
  static int counter = 0; // countes the clock cycles
  update_time();

  /* (arg, value, offset, zero_at) */
  set_arg_int(&args[0], tests[counter].a, 0, 40);  // a
  set_arg_int(&args[1], tests[counter].b, 0, 40);  // b
  set_arg_int(&args[2], tests[counter].op, 0, 40); // alucont

  ++counter;
  return 0;
}

static PLI_INT32 aluCheckCallTf(PLI_BYTE8*)
{
  s_vpi_value value;       // structure holds argument value
  static int counter = 0;  // countes the clock cycles
  static int success = 1;  // success flag
  int passed = 0;

  update_time();

  value.format = vpiIntVal;
  for (int i = 0; i < ARGS_NR; ++i) {
    vpi_get_value(args[i].handle, &value);
    args[i].value = value.value.integer;
  }
  if (args[3].value == tests[counter].res)  // compare w_data with r_data
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

void update_time(void)
{
  s_vpi_time time_s;  // time structure
  vpiHandle sys = vpi_handle(vpiSysTfCall, 0);  // get vpi sys handle

  time_s.type = vpiScaledRealTime;  // set type
  vpi_get_time(sys, &time_s);       // get current simulation time
  g_time = time_s.real;             // set global variable
}

static void bomb(void)
{
  cleanup(0);
  exit(1);
}

static void init(void)
{
  for (int i = 0; i < ARGS_NR; i++) {
    args[i].name = "\n";
    args[i].handle = NULL;
    args[i].value = 0;
  }
}

static PLI_INT32 cleanup(t_cb_data*)
{
  vpi_printf("...%s, cleanup()\n", __FILE__);
  for (int i = 0; i < ARGS_NR; i++) {
    if(args[i].name)
      free(args[i].name);
  }
  return 0;
}

// -- CHECK
static PLI_INT32 aluCheckCompileTf(PLI_BYTE8*)
{
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
