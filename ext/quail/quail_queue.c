#include <quail_queue.h>

static VALUE create_queue(  VALUE self,
                            VALUE rb_h,
                            VALUE name,
                            VALUE scope,
                            VALUE address)
{
  void * handle;
  Data_Get_Struct(rb_h, void, handle);
  czmq_create_queue(  handle, 
                      StringValuePtr(name),
                      NUM2INT(scope),
                      StringValuePtr(address)
  );
}

void Init_Quail_Queue(VALUE mQuail)
{
  VALUE cQuailHandle = rb_define_class_under(mQuail, "Queue", rb_cObject);
  rb_define_private_method(cQuailHandle, "create_queue", create_queue, 4);
}
