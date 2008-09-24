#include <quail_queue.h>

static VALUE create_queue(  VALUE self,
                            VALUE rb_h,
                            VALUE name,
                            VALUE scope,
                            VALUE address)
{
  void * handle;
  Data_Get_Struct(rb_h, void, handle);

  int native_scope = NUM2INT(scope);
  char * chr_address = native_scope == CZMQ_SCOPE_LOCAL ?
    NULL : StringValuePtr(address);

  czmq_create_queue(  handle, 
                      StringValuePtr(name),
                      native_scope,
                      chr_address
  );
  return self;
}

void Init_Quail_Queue(VALUE mQuail)
{
  VALUE cQuailHandle = rb_define_class_under(mQuail, "Queue", rb_cObject);
  rb_define_private_method(cQuailHandle, "create_queue", create_queue, 4);
}
