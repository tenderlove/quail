#include <native.h>

VALUE mQuail;

static bool error_handler (const char *name)
{
  VALUE block = rb_funcall(mQuail, rb_intern("error_handler"), 0);
  VALUE ret = rb_funcall(block, rb_intern("call"), 1, rb_str_new2(name));

  if(ret == Qtrue) return true;
  return false;
}

void Init_native()
{
  mQuail = rb_define_module("Quail");
  rb_const_set(mQuail, rb_intern("LOCAL"), INT2NUM(CZMQ_SCOPE_LOCAL));
  rb_const_set(mQuail, rb_intern("GLOBAL"), INT2NUM(CZMQ_SCOPE_GLOBAL));
  czmq_set_error_handler(error_handler);

  Init_Quail_Handle(mQuail);
  Init_Quail_Exchange(mQuail);
  Init_Quail_Queue(mQuail);
}
