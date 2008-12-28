#include "assert.h"
#include "vpi_user.h" /* the VPI library */

static int listNetCompileF(void)
{
  vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
  vpiHandle argv = vpi_iterate(vpiArgument, sys);
  vpiHandle arg = NULL;
  int i = 0;

  /* Need five arguments */
  /*
  for (i=0; i<2; i++) {
    arg = vpi_scan(argv);
    assert(arg);
  }*/
  while ((arg = vpi_scan(argv))) {
    ++i;
  }
  //vpi_printf("Debug: Number of arguments: %d\n", i);
  //init_sysmodel();
  return 0;
}

static int listNetCallF(void)
{
  vpiHandle systf_handle = NULL,
            argv_handle = NULL,
            module_handle = NULL,
            net_iterator = NULL,
            net_handle = NULL;

  /* get a handle to the first argument to $list_nets */
  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  argv_handle = vpi_iterate(vpiArgument, systf_handle);
  assert(argv_handle);
  while ((module_handle = vpi_scan(argv_handle))) {
    /* get the info of the module */
    vpi_printf("\n  Module %s info:\n", vpi_get_str(vpiDefName, module_handle));
    vpi_printf("    type      : %s\n", vpi_get_str(vpiType, module_handle));
    vpi_printf("    name      : %s\n", vpi_get_str(vpiName, module_handle));
    vpi_printf("    full name : %s\n", vpi_get_str(vpiFullName, module_handle));
    vpi_printf("    file      : %s\n", vpi_get_str(vpiFile, module_handle));
    vpi_printf("    handle ptr: %p\n", (void*)module_handle);

    /* get the name of the module represented by that first argument, and print it */
    vpi_printf("\n  Module %s contains nets:\n", vpi_get_str(vpiDefName, module_handle));

    /* search for all nets within the module, and print the name of each net found */
    net_iterator = vpi_iterate(vpiNet, module_handle);
    vpi_printf("    ");
    if (!net_iterator) {
      vpi_printf("no nets found.\n");
    } else {
      while ( (net_handle = vpi_scan(net_iterator)) ) {
        vpi_printf("%s, ", vpi_get_str(vpiName, net_handle));
      }
    }
    vpi_printf("\n\n");
  }
  return 0;
}

/* Associate C function with new system tesk */
void registerListNetSystfs()
{
  /* To associate your C function with a system task,
   * create a data structure of type s_vpi_systf_data
   * and a pointer to that structure.
   */
  s_vpi_systf_data task_data_s;

  /* task - does not return a value; function - can return a value.
   *  vpiSysTask, vpiSysFunc
   */
  task_data_s.type = vpiSysTask;
  /* If type is function, sysfunctype indicates
   * the type of the value that the calltf function returns.
   *  vpiSysTask, vpi[Int,Real,Time,Sized, SizedSigned]Func
   */
  task_data_s.sysfunctype = vpiSysTask;
  /* This quoted literal string defines the name of the system task or function.
   * The first character must be $.
   */
  task_data_s.tfname = "$list_net";
  /* This field is a pointer to your application routine
   */
  task_data_s.calltf = listNetCallF;
  /* This field is a pointer to a routine that the simulator calls once
   * each time it compiles an instance of the task or function.
   * Enter NULL if you have no such routine.
   */
  task_data_s.compiletf = listNetCompileF;

  vpi_register_systf(&task_data_s);
}

/* Register new system task */
void (*vlog_startup_routines[])() = {
  registerListNetSystfs,
  0
};
