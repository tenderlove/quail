#include <quail_handle.h>

static void dealloc(void * handle)
{
  czmq_destroy(handle);
}

static VALUE new(VALUE klass, VALUE host)
{
  void * handle = czmq_create(StringValuePtr(host));
  return Data_Wrap_Struct(klass, NULL, dealloc, handle);
}

void Init_Quail_Handle(VALUE mQuail)
{
  VALUE cQuailHandle = rb_define_class_under(mQuail, "Handle", rb_cObject);
  rb_define_singleton_method(cQuailHandle, "new", new, 1);
}
