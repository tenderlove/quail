#include <native.h>

void Init_native()
{
  VALUE mQuail = rb_define_module("Quail");
  rb_const_set(mQuail, rb_intern("LOCAL"), INT2NUM(CZMQ_SCOPE_LOCAL));
  rb_const_set(mQuail, rb_intern("GLOBAL"), INT2NUM(CZMQ_SCOPE_GLOBAL));

  Init_Quail_Handle(mQuail);
}
