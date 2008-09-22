
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CZMQ_SCOPE_LOCAL 0
#define CZMQ_SCOPE_GLOBAL 1

typedef void (czmq_free_fn) (void *data_);

void *czmq_create (const char *host_);
void czmq_destroy (void *obj_);
int czmq_create_exchange (void *obj_, const char *exchange_, int scope_,
    const char *nic_);
int czmq_create_queue (void *obj_, const char *queue_, int scope_,
    const char *nic_);
void czmq_bind (void *obj_, const char *exchange_, const char *queue_);
void czmq_send (void *obj_, int eid_, void *data_, size_t size,
    czmq_free_fn *ffn_);
void czmq_receive (void *obj_, void **data_, size_t *size_,
    czmq_free_fn **ffn_);

#ifdef __cplusplus
}
#endif
