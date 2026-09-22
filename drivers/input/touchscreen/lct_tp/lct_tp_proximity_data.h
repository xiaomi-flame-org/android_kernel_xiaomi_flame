#ifndef __LCT_TP_PROXIMITY_DATA_H__
#define __LCT_TP_PROXIMITY_DATA_H__

typedef int (*tp_proximity_status_cb_t)(int tp_status);

extern int init_lct_tp_proximity_data(void);
extern void uninit_lct_tp_proximity_data(void);
void set_lct_tp_proximity_data(int value);
extern bool get_lct_tp_proximity_data(void);

#endif //__LCT_TP_PROXIMITY_DATA_H__
