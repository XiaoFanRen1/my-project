#ifndef __EX_API_CODE_H__
#define __EX_API_CODE_H__

typedef enum {
    UPDATE_START = 0,
    UPDATE_STOP,
    UPDATE_PROCESS,
} UPDATA_STATUS_FOR_USER;

u8 ex_api_code_load(void);
void updata_status_to_user(UPDATA_STATUS_FOR_USER st, u32 param);
#endif
