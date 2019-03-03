/**
 * 功能：BS8116A-3芯片的应用程序
 * 日期：2019.3.3
 */
 
#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

int main(void)
{
	int fd;
	int j;
    char buf[2];
	unsigned int value;
	buf[0] = 0x08;
	
	fd = open("/dev/bs81xx", O_RDWR);
	if(fd < 0){
		printf("can't open /dev/bs81xx\n");
		return -1;
	}
	
	while(1){
		buf[0] = 0x08;
		read(fd, buf, 1);									/** 读取触摸芯片0x08寄存器的数据 */
		printf("0x08, buf[0]:%d, 0x%2X\n", buf[0], buf[0]);
		if(!buf[0]){
			goto REGISTER;									/** 如果全是0，表示key1到key8没有按键按下，去查看key9到key16 */
		}			
        for(j=0; j<8; j++){                          		/** 查看读取到的数据，看key1到key8哪个按键按下了 */
            value = buf[0]>>j;                        		/** 移位 */
            value &= 0x01;                           		/** 与1相与 */
            if(value){
                switch(j){
                    case 0:                          		/** 按键1按下 */
						printf("key 1, number 2\n"); break;
                    case 1:                          		/** 按键2按下 */
						printf("key 2, number 1\n"); break;
                    case 2:                          		/** 按键3按下 */
						printf("key 3, number 7\n"); break;
                    case 3:                          		/** 按键4按下 */
						printf("key 4, number *\n"); break;
                    case 4:                          		/** 按键5按下 */
						printf("key 5, number 4\n"); break;
                    case 5:                          		/** 按键6按下 */
						printf("key 6, number 5\n"); break;
                    case 6:                          		/** 按键7按下 */
						printf("key 7, number 8\n"); break;
                    case 7:                          		/** 按键8按下 */
						printf("key 8, number 0\n"); break;
                    default : 						 break;
                }
            }
        }
		
		REGISTER:
		usleep(300 * 1000);
		buf[0] = 0x09;
		read(fd, buf, 1);									/** 读取触摸芯片0x09寄存器的数据 */
		printf("0x09, buf[0]:%d, 0x%2X\n", buf[0], buf[0]);
		if(!buf[0])                                     		
			continue;										/** 如果全是0，表示key9到key16没有按键按下，则不执行下面语句 */
        for(j=0; j<8; j++){                          		/** 查看读取到的数据，看key9到key16是哪个按键按下了 */
            value = buf[0]>>j;                        		/** 移位 */
            value &= 0x01;                           		/** 与1相与 */
            if(value){
                switch(j){
                    case 0:                          		/** 按键9按下 */
						printf("key 9, number #\n");  break;
                    case 1:                          		/** 按键10按下 */
						printf("key 10, number 9\n"); break;
                    case 2:                          		/** 按键11按下 */
						printf("key 11, number h\n"); break;
                    case 3:                          		/** 按键12按下 */
						printf("key 12, number s\n"); break;
                    case 4:                          		/** 按键13按下 */
						printf("key 13, number >\n"); break;
                    case 5:                          		/** 按键14按下 */
						printf("key 14, number <\n"); break;
                    case 6:                          		/** 按键15按下 */
						printf("key 15, number 6\n"); break;
                    case 7:                          		/** 按键16按下 */
						printf("key 16, number 3\n"); break;
                    default :						  break;
                }
            }
        }
		usleep(300 * 1000);
	}
    
	close(fd);
	printf("end\n");
	
	return 0;
}
