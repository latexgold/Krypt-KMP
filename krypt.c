#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/mm.h>

#define DEVICE_NAME "krypt"
#define KRYPT_READ_MEM   _IOWR('K', 1, struct krypt_rw)
#define KRYPT_WRITE_MEM  _IOWR('K', 2, struct krypt_rw)
#define KRYPT_TOGGLE_ESP _IO('K', 3)
#define KRYPT_TOGGLE_AIM _IO('K', 4)

struct krypt_rw {
    pid_t pid;
    uint64_t addr;
    void *data;
    size_t size;
};

static int major_num;
static struct class* krypt_class = NULL;
static struct device* krypt_device = NULL;

static bool esp_enabled = true;
static bool aim_enabled = true;

static long krypt_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct krypt_rw rw;
    struct task_struct *task;
    struct mm_struct *mm;
    int ret = 0;

    if (cmd == KRYPT_TOGGLE_ESP) { esp_enabled = !esp_enabled; return 0; }
    if (cmd == KRYPT_TOGGLE_AIM) { aim_enabled = !aim_enabled; return 0; }

    if (copy_from_user(&rw, (void __user *)arg, sizeof(rw))) return -EFAULT;

    task = pid_task(find_vpid(rw.pid), PIDTYPE_PID);
    if (!task) return -ESRCH;

    mm = get_task_mm(task);
    if (!mm) return -EINVAL;

    mmap_read_lock(mm);

    if (cmd == KRYPT_READ_MEM) {
        if (access_process_vm(task, rw.addr, rw.data, rw.size, 0) != rw.size) ret = -EIO;
    } else if (cmd == KRYPT_WRITE_MEM) {
        if (aim_enabled) {
            if (access_process_vm(task, rw.addr, rw.data, rw.size, FOLL_WRITE) != rw.size) ret = -EIO;
        } else { ret = -EPERM; }
    }

    mmap_read_unlock(mm);
    mmput(mm);
    return ret;
}

static struct file_operations fops = { .unlocked_ioctl = krypt_ioctl };

static int __init krypt_init(void) {
    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    krypt_class = class_create(THIS_MODULE, DEVICE_NAME);
    krypt_device = device_create(krypt_class, NULL, MKDEV(major_num, 0), NULL, DEVICE_NAME);
    return 0;
}

static void __exit krypt_exit(void) {
    device_destroy(krypt_class, MKDEV(major_num, 0));
    class_unregister(krypt_class);
    class_destroy(krypt_class);
    unregister_chrdev(major_num, DEVICE_NAME);
}

module_init(krypt_init);
module_exit(krypt_exit);
MODULE_LICENSE("GPL");
