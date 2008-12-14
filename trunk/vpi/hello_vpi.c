#include <vpi_user.h>

static int hello(void)
{
  vpi_printf("\nHello :D\n");
  return 0;
}

/* Associate C function with new system tesk */
void registerHelloSystfs()
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
  task_data_s.tfname = "$hello";
  /* This field is a pointer to your application routine
   */
  task_data_s.calltf = hello;
  /* This field is a pointer to a routine that the simulator calls once
   * each time it compiles an instance of the task or function.
   * Enter NULL if you have no such routine.
   */
  task_data_s.compiletf = 0;

  vpi_register_systf(&task_data_s);
}

/* Register new system task */
void (*vlog_startup_routines[])() = {
  registerHelloSystfs,
  0
};
