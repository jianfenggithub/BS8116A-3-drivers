/**
 * 功能：BS8116A-3芯片的I2C驱动程序
 * 日期：2019.3.3
 */
 
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/mod_devicetable.h>
#include <linux/log2.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/timer.h>

static dev_t dev_num;                               /** 设备号 */
static unsigned int bs81xx_minor = 0;               /** 次设备号的开始值 */

static struct class *bs81xx_class;
static struct device *bs81xx_dev;

struct cdev bs81xx_cdev;                            /** 定义一个cdev */
struct i2c_client *bs8xx_client;

/** 一次读取触摸芯片2个按键输出寄存器下的数据，读取到2个字节的数据 */
static unsigned short read_two_byte(struct i2c_client *client, unsigned char command)
{
    short key_data;
    key_data = i2c_smbus_read_word_data(client, command);           /** 读2个字节数据 */
    if(key_data < 0){
        printk("read two byte data fail!\n");
        printk("key_data = %d\n", key_data);
        return key_data;
    }else{
        //printk("key_data = %d, 0x%4X\n", key_data, key_data);
        return key_data;
    }
}

/** 应用层执行read函数时会执行此函数,可以读取输出寄存器或者设置寄存器一个字节的数据 */
static ssize_t bs81xx_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos)
{
    unsigned char addr, data;

    copy_from_user(&addr, buf, 1);
    data = i2c_smbus_read_byte_data(bs8xx_client, addr);            /** 读一个字节数据 */
    copy_to_user(buf, &data, 1);

    return 0;
}

/** 读取设置寄存器22字节数据 */
static int read_n_byte(struct i2c_client *client)
{
    printk("read_n_byte\n");
    int ret;
    int i;
    unsigned char addr = 0xB0;
    u8 *data = kmalloc(22, GFP_KERNEL);                         	/** GFP_KERNEL:分配内存的标志 */

    msleep(1);
    ret = i2c_smbus_read_i2c_block_data(client, addr, 22, data);	/** 读取设置寄存器的数据，从0xB0开始连续读取22字节 */
    printk("ret:%d\n", ret);

    for(i=0; i<22; i++){
        printk("data[%d]:0x%2X\n", i, data[i]);
    }

    kfree(data);

    return 0;
}

/** 写设置寄存器,设置触摸芯片参数，根据芯片手册，从0xB0开始连续写入22个数据字节 */
/** 当需要写设置寄存器的时候，则要在上电8秒内完成此动作 */
static void set_bs81xx(struct i2c_client *client)
{
    printk("set_bs81xx\n");
    int ret;

    u8 *data = kmalloc(23, GFP_KERNEL);
    data[0] = 0x00;data[1] = 0x00;
    data[2] = 0x83;data[3] = 0xF3;
    data[4] = 0xD8;data[5] = 0x10;
    data[6] = 0x10;data[7] = 0x10;
    data[8] = 0x10;data[9] = 0x10;
    data[10] = 0x10;data[11] = 0x10;
    data[12] = 0x10;data[13] = 0x10;
    data[14] = 0x10;data[15] = 0x10;
    data[16] = 0x10;data[17] = 0x10;
    data[18] = 0x10;data[19] = 0x10;
    data[20] = 0x10;data[21] = 0x4E;

    ret = i2c_smbus_write_i2c_block_data(client, 0xB0, 22, data);

    kfree(data);
    printk("ret=:%d\n", ret);
}

/** 应用层执行open函数时会执行此函数 */
static int bs81xx_open(struct inode *inode, struct file *filp)
{
	printk("bs81xx_open\n");
	return 0;
}

/** 应用层执行write函数时会执行此函数 */
static ssize_t bs81xx_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{
	printk("bs81xx_write\n");
	return 0;
}

/** 应用层执行close函数时会执行此函数 */
static int bs81xx_release(struct inode *inode, struct file *filp)
{
	printk("bs81xx_release\n");
	return 0;
}

static long bs81xx_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
	printk("bs81xx_ioctl\n");
	return 0;
}

static const struct file_operations bs81xx_fops = {
	.open=bs81xx_open,
	.write=bs81xx_write,
	.read=bs81xx_read,
	.release=bs81xx_release,
    .unlocked_ioctl = bs81xx_ioctl,
};

static int bs81xx_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    printk("bs81xx_probe\n");
	int ret;
	bs8xx_client = client;
    ret = alloc_chrdev_region(&dev_num, bs81xx_minor, 1, "bs81xx");/** 申请cdev的设备号,让内核自动分配设备号 */
    if(ret < 0){									/** 申请设备号失败 */
        printk("can not get a dev_t\n");
        return ret;
    }
    cdev_init(&bs81xx_cdev, &bs81xx_fops);			/** cdev初始化 */

    ret = cdev_add(&bs81xx_cdev, dev_num, 1);		/** 将cdev加入内核 */
    if(ret < 0){									/** 如果cdev加入内核失败 */
        printk("can not add cdev to kernel\n");
        goto cdev_add_error;
    }

    bs81xx_class = class_create(THIS_MODULE, "bs81xx");/** 自动创建设备文件 */
    if(bs81xx_class == NULL){						/** 创建设备文件失败 */
        printk("class create error\n");
        ret = -EFAULT;
        goto class_create_error;
    }
    bs81xx_dev = device_create(bs81xx_class,NULL,dev_num, NULL, "bs81xx");/** 创建device */
    if(bs81xx_dev == NULL){                         /** 创建device失败 */
        printk("device create error\n");
        ret = -EFAULT;
        goto device_create_error;
    }
    printk("bs81xx driver init....\n");

    //read_n_byte(client);
    //msleep(100);
    //set_bs81xx(client);

	return ret;

device_create_error:
    class_destroy(bs81xx_class);    				/** 销毁class */
class_create_error:
    cdev_del(&bs81xx_cdev);         				/** 删除一个cdev */
cdev_add_error:
    unregister_chrdev_region(dev_num, 1);   		/** 注销设备号 */

    return ret;
}

static int __devexit bs81xx_remove(struct i2c_client *client)
{
    printk("bs81xx_remove\n");
	
	device_destroy(bs81xx_class, dev_num);			/** 销毁device */
	class_destroy(bs81xx_class);					/** 销毁class */
	unregister_chrdev_region(dev_num, 1);			/** 注销设备号 */
	
    return 0;
}

static const struct i2c_device_id bs81xx_ids[] = {	/** 该驱动所支持的 I2C 设备的 ID 表 */
        { "BS8116", 0 },							/** 匹配i2c client名为BS8116的设备 */
        { /* END OF LIST */ }
};

static struct i2c_driver bs81xx_driver = {
        .driver = {
                .name = "bs81xx",
                .owner = THIS_MODULE,
        },
        .probe = bs81xx_probe,
        .remove = __devexit_p(bs81xx_remove),
        .id_table = bs81xx_ids,
};

static int bs81xx_drv_init(void)
{
	printk("bs81xx_drv_init\n");
	return i2c_add_driver(&bs81xx_driver);
}

static void bs81xx_drv_exit(void)
{
	printk("bs81xx_exit\n");
    i2c_del_driver(&bs81xx_driver);
}
module_init(bs81xx_drv_init);
module_exit(bs81xx_drv_exit);

MODULE_DESCRIPTION("Driver for BS81xx");
MODULE_AUTHOR("Luo jian feng");
MODULE_LICENSE("GPL");	