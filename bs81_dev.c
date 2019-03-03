/**
 * 功能：BS8116A-3芯片的I2C驱动程序
 * 日期：2019.3.3
 */
 
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/module.h>

static struct i2c_client *bs81xx_client;
static const unsigned short addr_list[] = { 0x50, I2C_CLIENT_END };

static int bs81xx_dev_init(void)
{
	struct i2c_adapter *i2c_adap;
	struct i2c_board_info bs81xx_info;
	
	memset(&bs81xx_info, 0, sizeof(struct i2c_board_info));
	strlcpy(bs81xx_info.type, "BS8116", I2C_NAME_SIZE);
	
	i2c_adap = i2c_get_adapter(0);
	bs81xx_client = i2c_new_probed_device(i2c_adap, &bs81xx_info, addr_list, NULL);
	i2c_put_adapter(i2c_adap);
	
	if(bs81xx_client){
		printk("has bs81xx_client\n");
		return 0;
	}
	else{
		printk("no bs81xx_client\n");
		return -ENODEV;
	}	
}

static void bs81xx_dev_exit(void)
{
	printk("bs81xx_dev_exit\n");
	i2c_unregister_device(bs81xx_client);
}

module_init(bs81xx_dev_init);
module_exit(bs81xx_dev_exit);

MODULE_LICENSE("GPL");