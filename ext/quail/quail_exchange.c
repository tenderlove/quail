#include <quail_exchange.h>

static VALUE create_exchange( VALUE self,
                              VALUE rb_h,
                              VALUE name,
                              VALUE scope,
                              VALUE address)
{
  void * handle;
  int eid;
  int native_scope = NUM2INT(scope);

  char * chr_address = native_scope == CZMQ_SCOPE_LOCAL ?
    NULL : StringValuePtr(address);

  Data_Get_Struct(rb_h, void, handle);
  eid = czmq_create_exchange( handle, 
                              StringValuePtr(name),
                              native_scope,
                              chr_address );
  return INT2NUM(eid);
}

static VALUE send(VALUE self, VALUE message)
{
  void * handle;

  VALUE length  = rb_funcall(message, rb_intern("length"), 0);
  VALUE rb_h    = rb_iv_get(self, "@handle");
  VALUE eid     = rb_iv_get(self, "@eid");
  Data_Get_Struct(rb_h, void, handle);
  czmq_send(handle, NUM2INT(eid), StringValuePtr(message), NUM2INT(length), 0);
  return message;
}

void Init_Quail_Exchange(VALUE mQuail)
{
  VALUE cQuailHandle = rb_define_class_under(mQuail, "Exchange", rb_cObject);
  rb_define_private_method(cQuailHandle, "create_exchange", create_exchange, 4);
  rb_define_method(cQuailHandle, "send", send, 1);
}
