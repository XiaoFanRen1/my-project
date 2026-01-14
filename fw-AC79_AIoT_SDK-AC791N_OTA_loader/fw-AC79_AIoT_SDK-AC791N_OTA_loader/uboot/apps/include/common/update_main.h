#ifndef _UPDATE_MAIN_H_
#define _UPDATE_MAIN_H_

#define     LMP_STA_UPDATA_ACTIVE              0x0800
#define     LMP_STA_UPDATA_REC_DATA            0x1000
#define     LMP_STA_UPDATA_ACTIVE_SCUSS        0x2000
#define     LMP_STA_UPDATA_ACCEPT              0x4000
enum {
    UPDATA_STOP = 0,
    UPDATA_OPEN,
    UPDATA_SEEK,
    UPDATA_READ,
    UPDATA_READ_OFFSIZE,
    UPDATA_STOP_KEYERR,
    UPDATA_ACCEPT_OK,
    UPDATA_WAIT_REBOOT,
};

typedef struct __notif_info {
    u8 btupdata_sw;
    u8 seek_type;
    u8 reserve[2];
    u32 len;
    u32 seek_offsize;
} notif_info;

typedef struct __ble_control_info {
    volatile u32 ble_update_state;
    volatile u8 btupdata_sw;
    u32 date_len;
    u32 need_rx_len;
    u32 flash_write_addr;
    u16 once_write_len;
    u16 once_rx_len;
    u8 *read_buf;
    u32 wait_timeout_instant;       //等待超时时记录的时刻
} ble_control_info_t;

u32 get_flash_write_size(void);
void set_flash_write_size(u32 len);
int btstack_main(void);
void ble_task_register(void(*init), void(*run));
UPDATA_RESULT ble_updata_run(void);

enum {
    APP_BLE_NO_ERROR = 0,
    APP_BLE_BUFF_ERROR = 1,
    APP_BLE_BUFF_FULL = 2,
    APP_BLE_OPERATION_ERROR = 3,
    APP_BLE_IS_DISCONN =  4,
};

enum {
    DEV_CONN_STATE_DISCONNECTED = 0,
    DEV_CONN_STATE_CONNECTED,
    DEV_CONN_STATE_READY_UPDATE,
    DEV_CONN_STATE_START_UPDATE,
};

struct ble_operation_t {
    int(*adv_enable)(void *priv, u32 enable);
    bool(*is_connected)(void *priv);
    int(*disconnect)(void *priv);
    int(*get_buffer_vaild)(void *priv);
    int(*send_data)(void *priv, void *buf, u16 len);
    int(*regist_wakeup_send)(void *priv, void *cbk);
    int(*regist_recieve_cbk)(void *priv, void *cbk);
};

void ble_get_operation_table(struct ble_operation_t **interface_pt);


#endif
