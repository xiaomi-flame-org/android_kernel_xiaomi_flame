/****************************************************************************************
 *
 * @File Name   : lct_tp_proximity_data.c
 * @Author      : hongmo
 * @E-mail      : <hongmo@longcheer.com>
 * @Create Time : 2023-07-25 17:34:43
 * @Description : proximity data.
 *
 ****************************************************************************************/

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <linux/module.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

/*
 * DEFINE CONFIGURATION
 ****************************************************************************************
 */
#define TP_PROXIMITY_DATA_NAME "tp_proximity_data"
#define TP_PROXIMITY_DATA_LOG_ENABLE
#define TP_PROXIMITY_DATA_TAG "LCT_TP_PROXIMITY_DATA"

#ifdef TP_PROXIMITY_DATA_LOG_ENABLE
#define TP_LOGW(log, ...) printk(KERN_WARNING "[%s] %s (line %d): " log, TP_PROXIMITY_DATA_TAG, __func__, __LINE__, ##__VA_ARGS__)
#define TP_LOGE(log, ...) printk(KERN_ERR "[%s] %s ERROR (line %d): " log, TP_PROXIMITY_DATA_TAG, __func__, __LINE__, ##__VA_ARGS__)
#else
#define TP_LOGW(log, ...) \
    {                     \
    }
#define TP_LOGE(log, ...) \
    {                     \
    }
#endif

bool tp_proximity_data_probe = false;

/*
 * DATA STRUCTURES
 ****************************************************************************************
 */
typedef int (*tp_proximity_data_cb_t)(int tp_data);

typedef struct lct_tp
{
    int tp_proximity_data;
    struct proc_dir_entry *proc_entry_tp;
    //tp_proximity_data_cb_t pfun;
} lct_tp_t;

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static lct_tp_t *lct_tp_p = NULL;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
// --- proc ---
static int lct_creat_proc_tp_entry(void);
static ssize_t lct_proc_tp_proximity_data_read(struct file *file, char __user *buf, size_t size, loff_t *ppos);
static ssize_t lct_proc_tp_proximity_data_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos);
static const struct proc_ops lct_proc_tp_proximity_data_fops = {
    .proc_read = lct_proc_tp_proximity_data_read,
    .proc_write = lct_proc_tp_proximity_data_write,
};

int init_lct_tp_proximity_data(void)
{
    TP_LOGW("Initialization tp_proximity_data node!\n");
    lct_tp_p = kzalloc(sizeof(lct_tp_t), GFP_KERNEL);
    if (IS_ERR_OR_NULL(lct_tp_p)) {
        TP_LOGE("kzalloc() request memory failed!\n");
        return -ENOMEM;
    }
    lct_tp_p->tp_proximity_data = 0;

    if (lct_creat_proc_tp_entry() < 0) {
        kfree(lct_tp_p);
        return -1;
    }
    tp_proximity_data_probe = true;

    return 0;
}
EXPORT_SYMBOL(init_lct_tp_proximity_data);

void uninit_lct_tp_proximity_data(void)
{
    TP_LOGW("uninit /proc/%s ...\n", TP_PROXIMITY_DATA_NAME);
    if (IS_ERR_OR_NULL(lct_tp_p))
        return;
    if (lct_tp_p->proc_entry_tp != NULL) {
        remove_proc_entry(TP_PROXIMITY_DATA_NAME, NULL);
        lct_tp_p->proc_entry_tp = NULL;
        TP_LOGW("remove /proc/%s\n", TP_PROXIMITY_DATA_NAME);
    }
    kfree(lct_tp_p);
    tp_proximity_data_probe = false;
    return;
}
EXPORT_SYMBOL(uninit_lct_tp_proximity_data);

void set_lct_tp_proximity_data(int value)
{
    if (!tp_proximity_data_probe) {
        TP_LOGW("tp probe load failed!!!");
        return;
    }

    lct_tp_p->tp_proximity_data = value;
    TP_LOGW("set_lct_tp_proximity_data value=%d\n", value);
}
EXPORT_SYMBOL(set_lct_tp_proximity_data);

bool get_lct_tp_proximity_data_data(void)
{
    return lct_tp_p->tp_proximity_data;
}
EXPORT_SYMBOL(get_lct_tp_proximity_data_data);

static int lct_creat_proc_tp_entry(void)
{
    lct_tp_p->proc_entry_tp = proc_create_data(TP_PROXIMITY_DATA_NAME, 0666, NULL, &lct_proc_tp_proximity_data_fops, NULL);
    if (IS_ERR_OR_NULL(lct_tp_p->proc_entry_tp))
    {
        TP_LOGE("add /proc/tp_proximity_data error \n");
        return -1;
    }
    TP_LOGW("/proc/tp_proximity_data is okay!\n");

    return 0;
}

static ssize_t lct_proc_tp_proximity_data_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    ssize_t cnt = 0;
    char *page = NULL;

    if (*ppos)
        return 0;

    page = kzalloc(128, GFP_KERNEL);
    if (IS_ERR_OR_NULL(page))
        return -ENOMEM;

    cnt = sprintf(page, "%d\n", lct_tp_p->tp_proximity_data);

    cnt = simple_read_from_buffer(buf, size, ppos, page, cnt);
    if (*ppos != cnt)
        *ppos = cnt;

    kfree(page);
    return cnt;
}

static ssize_t lct_proc_tp_proximity_data_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
    int value = 0;
    ssize_t cnt = 0;
    char *page = NULL;
    unsigned int input = 0;

    page = kzalloc(128, GFP_KERNEL);
    if (IS_ERR_OR_NULL(page))
        return -ENOMEM;
    cnt = simple_write_to_buffer(page, 128, ppos, buf, size);
    if (cnt <= 0)
        return -EINVAL;
    if (sscanf(page, "%u", &input) != 1)
        return -EINVAL;
    if (value > 3 || value < 0){
        TP_LOGW("write tp data over range\n");
        return -EINVAL;
    }

    set_lct_tp_proximity_data(value);
    TP_LOGW("Set Touchpad proximity successfully!\n");

    kfree(page);
    return cnt;
}

MODULE_DESCRIPTION("Touchpad Work Contoller Driver");
MODULE_LICENSE("GPL");
