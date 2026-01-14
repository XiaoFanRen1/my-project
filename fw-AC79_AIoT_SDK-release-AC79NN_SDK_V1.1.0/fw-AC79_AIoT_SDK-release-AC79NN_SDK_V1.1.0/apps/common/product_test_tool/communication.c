#include "product_main.h"

#ifdef PRODUCT_TEST_ENABLE

static OS_SEM cdc_sem;
static u8 product_usb_id, online_flag = 0;


static int product_device_event_handler(struct sys_event *e)
{
    struct device_event *event = (struct device_event *)e->payload;
    const char *usb_msg = (const char *)event->value;

    if (e->from == DEVICE_EVENT_FROM_OTG) {
        if (usb_msg[0] == 's') {
            if (event->event == DEVICE_EVENT_IN) {
                log_info("usb %c online", usb_msg[2]);
                online_flag = 1;
                product_usb_id = usb_msg[2] - '0';
            } else if (event->event == DEVICE_EVENT_OUT) {
                log_info("usb %c offline", usb_msg[2]);
                online_flag = 0;
            }
        }
    }
    return 0;
}


SYS_EVENT_STATIC_HANDLER_REGISTER(product_device_event) = {
    .event_type     = SYS_DEVICE_EVENT,
    .prob_handler   = product_device_event_handler,
    .post_handler   = NULL,
};


u8 product_tool_cdc_post(void)
{
    if (is_product_mode()) {
        os_sem_post(&cdc_sem);
    }
    return is_product_mode();
}


static s8 comm_dev_init(void)
{
    os_sem_create(&cdc_sem, 0);
    return 0;
}


static s8 comm_dev_online(void)
{
    if (!online_flag) {
        log_err("communication devices no online\n");
    }
    online_flag = 1;
    return online_flag;
}


static s32 comm_dev_read(u8 *data, u32 size)
{
    //os_sem_set(&cdc_sem, 0);
    os_sem_pend(&cdc_sem, 0);
    return cdc_read_data(product_usb_id, data, size);
}


static s32 comm_dev_write(u8 *data, u32 size)
{
    return cdc_write_data(product_usb_id, data, size);
}


static prod_comm_type comm = {
    .init   = comm_dev_init,
    .online = comm_dev_online,
    .read   = comm_dev_read,
    .write  = comm_dev_write,
};



prod_comm_type *comm_ops(void)
{
    return &comm;
}


#endif


