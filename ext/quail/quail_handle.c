#include <quail_handle.h>

static VALUE native_bind(VALUE self, VALUE exchange_name, VALUE queue_name)
{
  void * handle;

  Data_Get_Struct(rb_iv_get(self, "@handle"), void, handle);
  czmq_bind(handle, StringValuePtr(exchange_name), StringValuePtr(queue_name));
}

static VALUE destroy_handle(VALUE self)
{
  void * handle;

  Data_Get_Struct(rb_iv_get(self, "@handle"), void, handle);
  czmq_destroy(handle);
}

static VALUE create_handle(VALUE self, VALUE host)
{
  void * handle = czmq_create(StringValuePtr(host));
  rb_iv_set(self, "@handle", Data_Wrap_Struct(rb_cObject, NULL, NULL, handle));
  return self;
}

static VALUE receive(VALUE self)
{
  void * handle;
  void * buf;
  size_t size;
  czmq_free_fn *ffn;

  Data_Get_Struct(rb_iv_get(self, "@handle"), void, handle);
  czmq_receive(handle, &buf, &size, &ffn);
  if(ffn) ffn(buf);

  return rb_str_new((char *)buf, size);
}

void Init_Quail_Handle(VALUE mQuail)
{
  VALUE cQuailHandle = rb_define_class_under(mQuail, "Handle", rb_cObject);
  rb_define_private_method(cQuailHandle, "create_handle", create_handle, 1);
  rb_define_private_method(cQuailHandle, "destroy_handle", destroy_handle, 0);
  rb_define_private_method(cQuailHandle, "native_bind", native_bind, 2);
  rb_define_method(cQuailHandle, "receive", receive, 0);
}
