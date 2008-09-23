#include <native.h>

static bool awesome (const char *foo)
{
  return false;
}

void Init_native()
{
  VALUE mQuail = rb_define_module("Quail");
  rb_const_set(mQuail, rb_intern("LOCAL"), INT2NUM(CZMQ_SCOPE_LOCAL));
  rb_const_set(mQuail, rb_intern("GLOBAL"), INT2NUM(CZMQ_SCOPE_GLOBAL));
  czmq_set_error_handler(awesome);

  Init_Quail_Handle(mQuail);
  Init_Quail_Exchange(mQuail);
  Init_Quail_Queue(mQuail);
}
