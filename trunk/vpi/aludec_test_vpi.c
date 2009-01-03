#include <stdlib.h>
#include <assert.h>
#include <vpi_user.h> /* the VPI library */
#include "common.h"

#define ARGS_NR 3
#define TESTS_NR 23
static s_riscyArg  args[ARGS_NR];
static s_riscyTest tests[TESTS_NR];
static double g_time;

static void update_time(void);
static void bomb(void);
static void init(void);
static PLI_INT32 cleanup();

static PLI_INT32 aluTestCompileTf()
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
  /*
  func
  10 0000 (32) add
  10 0001 (33) addu
  10 0010 (34) sub
  10 0011 (35) subu
  10 0100 (36) and
  10 0101 (37) or
  10 0110 (38) xor
  10 0111 (39) nor
  10 1010 (42) slt
  10 1011 (43) sltu
   */
  /* initialize the tests */
  for (int i = 0; i < TESTS_NR; ++i) {
    switch (i) {
      case 0: tests[i].a = 0x20; // funct
              tests[i].b = 0xF; // aluop (0xF = R-type)
              tests[i].res = ADD; // alucontrol
        break;
      case 1: tests[i].a = 0x21; // funct
              tests[i].b = 0xF; // aluop (0xF = R-type)
              tests[i].res = ADDU; // alucontrol
        break;
      case 2: tests[i].a = 0x22; // funct
              tests[i].b = 0xF; // aluop (0xF = R-type)
              tests[i].res = SUB; // alucontrol
        break;
      case 3: tests[i].a = 0x23; // funct
              tests[i].b = 0xF; // aluop (0xF = R-type)
              tests[i].res = SUBU; // alucontrol
        break;
      case 4: tests[i].a = 0x24; // funct
              tests[i].b = 0xF; // aluop (0xF = R-type)
              tests[i].res = AND; // alucontrol
        break;
      case 5: tests[i].a = 0x25; // funct
              tests[i].b = 0xF; // aluop (0xF = R-type)
              tests[i].res = OR; // alucontrol
        break;
      case 6: tests[i].a = 0x26; // funct
              tests[i].b = 0xF; // aluop (0xF = R-type)
              tests[i].res = XOR; // alucontrol
        break;
      case 7: tests[i].a = 0x27; // funct
              tests[i].b = 0xF; // aluop (0xF = R-type)
              tests[i].res = NOR; // alucontrol
        break;
      case 8: tests[i].a = 0x2A; // funct
              tests[i].b = 0xF; // aluop (0xF = R-type)
              tests[i].res = SLT; // alucontrol
        break;
      case 9: tests[i].a = 0x2B; // funct
              tests[i].b = 0xF; // aluop (0xF = R-type)
              tests[i].res = SLTU; // alucontrol
        break;
   /*
   case(aluop)
      4'b0000: alucontrol = 6'b0_00010;  // ADDI
      4'b0001: alucontrol = 6'b1_00010;  // SUBI
      4'b0010: alucontrol = 6'b1_00011;  // SLTI
    //4'b0011: // SLTU
      4'b0100: alucontrol = 6'b0_00000;  // ANDI
      4'b0101: alucontrol = 6'b0_00001;  // ORI
      4'b0110: alucontrol = 6'b0_00100;  // XORI
      4'b0111: alucontrol = 6'b0_00110;  // LUI
    */
      case 10: tests[i].a = 0x0; // funct
               tests[i].b = 0x0; // aluop (0xF = R-type)
               tests[i].res = ADD; // alucontrol
        break;
      case 11: tests[i].a = 0x0; // funct
               tests[i].b = 0x1; // aluop (0xF = R-type)
               tests[i].res = SUB; // alucontrol
        break;
      case 12: tests[i].a = 0x0; // funct
               tests[i].b = 0x2; // aluop (0xF = R-type)
               tests[i].res = SLT; // alucontrol
        break;
      case 13: tests[i].a = 0x0; // funct
               tests[i].b = 0x3; // aluop (0xF = R-type)
               tests[i].res = SLTU; // alucontrol
        break;
      case 14: tests[i].a = 0x0; // funct
               tests[i].b = 0x4; // aluop (0xF = R-type)
               tests[i].res = AND; // alucontrol
        break;
      case 15: tests[i].a = 0x0; // funct
               tests[i].b = 0x5; // aluop (0xF = R-type)
               tests[i].res = OR; // alucontrol
        break;
      case 16: tests[i].a = 0x0; // funct
               tests[i].b = 0x6; // aluop (0xF = R-type)
               tests[i].res = XOR; // alucontrol
        break;
      case 17: tests[i].a = 0x0; // funct
               tests[i].b = 0x7; // aluop (0xF = R-type)
               tests[i].res = LUI; // alucontrol
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

static PLI_INT32 aluTestCallTf()
{
  static int counter = 0; // countes the clock cycles
  update_time();

  /* (arg, value, offset, zero_at) */
  set_arg_int(&args[0], tests[counter].a, 0, 40);  // funct
  set_arg_int(&args[1], tests[counter].b, 0, 40);  // aluopt

  ++counter;
  return 0;
}

static PLI_INT32 aluCheckCallTf()
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
  if (args[2].value == tests[counter].res)  // compare w_data with r_data
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
  cleanup();
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
static PLI_INT32 aluCheckCompileTf()
{
  return 0;
}

/* Associate C function with new system tesk */
void registerRegfileTestSystfs()
{
  s_vpi_systf_data task_data_s;

  task_data_s.type = vpiSysTask;
  task_data_s.sysfunctype = vpiSysTask;
  task_data_s.tfname = "$aludec_test";
  task_data_s.calltf = aluTestCallTf;
  task_data_s.compiletf = aluTestCompileTf;
  task_data_s.sizetf = NULL;
  task_data_s.user_data = "$aludec_test";

  vpi_register_systf(&task_data_s);

  task_data_s.type = vpiSysTask;
  task_data_s.sysfunctype = vpiSysTask;
  task_data_s.tfname = "$aludec_check";
  task_data_s.calltf = aluCheckCallTf;
  task_data_s.compiletf = aluCheckCompileTf;
  task_data_s.sizetf = NULL;
  task_data_s.user_data = "$aludec_check";

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
