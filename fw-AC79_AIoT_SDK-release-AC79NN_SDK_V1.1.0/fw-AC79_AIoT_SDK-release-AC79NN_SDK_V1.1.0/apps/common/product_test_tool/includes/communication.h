#ifndef _PRODUCT_COMM_H_
#define _PRODUCT_COMM_H_


typedef struct {
    s8(*init)(void);
    s8(*online)(void);
    s8(*read)(u8 *, u32);
    s8(*write)(u8 *, u32);
} prod_comm_type;


prod_comm_type *comm_ops(void);


#endif


