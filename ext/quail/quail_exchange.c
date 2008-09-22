#include <quail_exchange.h>

static VALUE create_exchange( VALUE self,
                              VALUE rb_h,
                              VALUE name,
                              VALUE scope,
                              VALUE address)
{
  void * handle;
  int eid;

  Data_Get_Struct(rb_h, void, handle);
  eid = czmq_create_exchange( handle, 
                              StringValuePtr(name),
                              NUM2INT(scope),
                              StringValuePtr(address) );
  return INT2NUM(eid);
}

void Init_Quail_Exchange(VALUE mQuail)
{
  VALUE cQuailHandle = rb_define_class_under(mQuail, "Exchange", rb_cObject);
  rb_define_private_method(cQuailHandle, "create_exchange", create_exchange, 4);
}
