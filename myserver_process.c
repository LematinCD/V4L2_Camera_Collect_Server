/********************************************************
*   Copyright (C) 2016 All rights reserved.
*   
*   Filename:myserver_process.c
*   Author  :Lematin
*   Date    :2017-2-17
*   Describe:
*
********************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "myserver_cam.h"
#include "myserver_uart.h"
#include "myserver_sqlite3.h"
#include "myserver_process.h"

#ifdef YUYV
#include "myserver_convert.h"
#endif
#include "myconfig.h"

#ifdef M0
static unsigned char Open_LED1_ZIGBEE[36]={0xDD,0x5,0x24,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
static unsigned char Close_LED1_ZIGBEE[36]={0xDD,0x5,0x24,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

static unsigned char Open_Beep_ZIGBEE[36]={0xDD,0x5,0x24,0x0,0x2,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
static unsigned char Close_Beep_ZIGBEE[36]={0xDD,0x5,0x24,0x0,0x3,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

static unsigned char Open_Fan_ZIGBEE[36]={0xDD,0x5,0x24,0x0,0x4,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
static unsigned char Close_Fan_ZIGBEE[36]={0xDD,0x5,0x24,0x0,0x8,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};


#elif MY_ZIGBEE
static unsigned char Open_LED1_ZIGBEE[64]="$SRS--------------------01----------------------------JD-------*";
static unsigned char Close_LED1_ZIGBEE[64]="$SRS--------------------00----------------------------JD-------*";
static unsigned char Open_Fan_ZIGBEE[64]="$SRS--------------------01----------------------------BJ-------*";
static unsigned char Close_Fan_ZIGBEE[64]="$SRS--------------------00----------------------------BJ-------*";
static unsigned char Open_Beep_ZIGBEE[64]="$SRS--------------------01----------------------------BP-------*";
static unsigned char Close_Beep_ZIGBEE[64]="$SRS--------------------00----------------------------BP-------*";


#endif


static unsigned int cmd = 4;
static unsigned int Length = 0;
static unsigned char dev_name[12] ="/dev/video0\n";
static unsigned int dev_name_length = 12;

enum my_net_cmd{
      	Login = 0x1,
        LoginSuccess,
        LoginFailed,

        Register,
        RegisterSuccess,
        RegisterFailed,

        
        Change_Password,
        Change_Password_Success,
        Change_Password_Failed,

        Dev_list,

        Open_Camera = 0x20,
        Open_Camera_Success,
        Open_Camera_Failed,
        
        Close_Camera,
        Close_Camera_Success,
        Close_Camera_Failed,
        
        /*灯 控制命令组 开 关 亮度*/
        Open_Led = 0x30,
        Open_Led_Success,
        Open_Led_Failed,
        Close_Led,
        Close_Led_Success,
        Close_Led_Failed,


        Open_Fan = 0x40,
        Open_Fan_Success,
        Open_Fan_Failed,
        Close_Fan,
        Close_Fan_Success,
        Close_Fan_Failed,
        
       	Open_Beep,
        Open_Beep_Success,
        Open_Beep_Failed,
        Close_Beep,
        Close_Beep_Success,
        Close_Beep_Failed,
        

        Open_Sensor = 0x60,
        Open_Sensor_Success,
        Open_Seneor_Failed,
        Close_Sensor,
        Close_Sensor_Success,
        Close_Sensor_Faile,

		Quit
}my_cmd;


static unsigned int uchar_to_uint(const unsigned char *bufdata)
{
	int i = 0;
	unsigned int intdata = 0;
	for(i=3;i>=0;i--){
		intdata+=intdata*10+bufdata[i];
	}
	return  intdata;        
}

static void toString(char *dstbuf,char *srcbuf,unsigned int size)
{
	int i = 0;
	for(i=0;i<size/2;i++){
		dstbuf[i] = srcbuf[2*i];
	}
	dstbuf[i]='\0';
}

void send_video_data_handle(void *args)
{
	int ret = 0,i=0;
	
#ifdef YUYV
	if(0 != pthread_mutex_lock(&convert_mutex)){
		printf("write cam_mutex locl fail!\n");
		return;
	}	
	Length = my_jpeg.jpeg_size+5;
	memcpy(my_jpeg.jpeg_buf,(char *)&Length,4);
	my_jpeg.jpeg_buf[4] = (char)Open_Camera_Success;
	ret = write(*(int *)args,my_jpeg.jpeg_buf,Length+4);
	if(0 != pthread_mutex_unlock(&convert_mutex)){
		printf("write cam_mutex locl fail!\n");
		return;
	}
#elif	JPEG
	
	if(0 != pthread_mutex_lock(&camera_mutex)){
		printf("write cam_mutex locl fail!\n");
		return;
	}
	Length = my_image.image_size+5;
	memcpy(my_image.image_buf,(char *)&Length,4);
	my_image.image_buf[4] = (char)Open_Camera_Success;
	ret = write(*(int *)args,my_image.image_buf,Length+4);
	if(0 != pthread_mutex_unlock(&camera_mutex)){
		printf("write cam_mutex locl fail!\n");
		return;
	}	
#endif	
		
}


void uartdata_send_handle(void *args)
{
      int ret = 0,i=0;
	if(0 != pthread_mutex_lock(&uart_read_mutex)){
	        printf("uart_read_mutex lock fail!\n");
		return;
        }
	Length = uart_read.uart_size+5;
	memcpy(uart_read.uart_buf,(char *)&Length,4);
	uart_read.uart_buf[MY_NET_CMD_POS] = (char)Open_Sensor_Success;
	ret = write(*(int *)args,uart_read.uart_buf,Length+4);
#ifdef	DEBUG
	printf("uart send over! ret:%d\n",ret);
#endif
	if(0 != pthread_mutex_unlock(&uart_read_mutex)){
	        printf("uart_read_mutex lock fail!\n");
		return;
        }	
	
	
	  
}

void uartdata_recv_handle(void *args,char flag)
{
      int ret = 0,i=0;
	if (0 > sem_wait(&sem_uart_write_empty)) {
		perror("sem_wait");
		pthread_exit("pthread_exit unnormally!");
	}
	memset(&uart_write.uart_buf[MY_UART_BUF_SIZE_POS],0,MY_UART_BUF_SIZE);
	switch(flag){
		case Open_Led:
			printf("Open_Led!\n");
			memcpy(&uart_write.uart_buf[MY_UART_BUF_DATA_POS],Open_LED1_ZIGBEE,MY_UART_BUF_SIZE);
			break;
		case Close_Led:
			printf("Close_Led!\n");
			memcpy(&uart_write.uart_buf[MY_UART_BUF_DATA_POS],Close_LED1_ZIGBEE,MY_UART_BUF_SIZE);
			break;
		case Open_Fan:
			printf("Open_Fan!\n");
			memcpy(&uart_write.uart_buf[MY_UART_BUF_DATA_POS],Open_Fan_ZIGBEE,MY_UART_BUF_SIZE);
			break;
		case Close_Fan:
			printf("Close_Fan!\n");
			memcpy(&uart_write.uart_buf[MY_UART_BUF_DATA_POS],Close_Fan_ZIGBEE,MY_UART_BUF_SIZE);
			break;
		case Open_Beep:
			memcpy(&uart_write.uart_buf[MY_UART_BUF_DATA_POS],Open_Beep_ZIGBEE,MY_UART_BUF_SIZE);
			break;
		case Close_Beep:
			memcpy(&uart_write.uart_buf[MY_UART_BUF_DATA_POS],Close_Beep_ZIGBEE,MY_UART_BUF_SIZE);
			break;	
	
	}
	if (0 > sem_post(&sem_uart_write_full)) {
		perror("sem_wait");
		pthread_exit("pthread_exit unnormally!");
	}
	
	printf("uart recv over!\n");
	  
}


void login_regster_changepwd_handle(int *conn_fd,char *recv_buf,char flag,char *send_buf)
{
	char name[255];
	char passwd[255];
	char new_passwd[255];   
	int name_size =  uchar_to_uint(&recv_buf[5]);
	int passwd_size = uchar_to_uint(&recv_buf[9+name_size]);         
	toString(name,&recv_buf[9],name_size);//获取name
	toString(passwd,&recv_buf[13+name_size],passwd_size);//获取passwd 
#ifdef  DEBUG	
	printf("name:%s\n",name);
	printf("passwd:%s\n",passwd);
#endif	
	Length =1;
	bzero(send_buf,5);
	memcpy(send_buf,(char *)&Length,4);
	switch(flag){
		case Login: 
			if(0>sqlite3_query_user(name,passwd)){
				send_buf[MY_NET_CMD_POS] = (char)LoginFailed;
			}else{
				send_buf[MY_NET_CMD_POS] = (char)LoginSuccess;
			}
			break;
		case Register:
			if(0>sqlite3_insert(name,passwd)){
				send_buf[MY_NET_CMD_POS] = (char)RegisterFailed;
			}else{
				send_buf[MY_NET_CMD_POS] = (char)RegisterSuccess;
			}
			break;
		case Change_Password:
			toString(new_passwd,&recv_buf[17+passwd_size],uchar_to_uint(&recv_buf[13+passwd_size]));
			if(0>sqlite3_chgpwd(name,new_passwd,passwd)){
				send_buf[MY_NET_CMD_POS] = (char)Change_Password_Failed;
			}else{
				send_buf[MY_NET_CMD_POS] = (char)Change_Password_Success;
			}
			break;
	}  			         
	write(*conn_fd,send_buf,Length+4);       

}

/*重写处理函数*/
void *my_net_process(void *args)
{
	int *conn_fd = (int *)args;
#ifdef DEBUG	
	printf("conn_fd:%d\n",*conn_fd);
#endif	
	int ret = 0;
	int i = 0;
        char send_buf[255];
        char recv_buf[255];
	int net_data_size =0;        
	while(1){
                memset(recv_buf,0,255);
		if(0>read(*conn_fd,recv_buf,sizeof(recv_buf)))	
			return NULL;
		switch(recv_buf[4]){
			case Login:  
			case Register:
			case Change_Password:
				login_regster_changepwd_handle(conn_fd,recv_buf,recv_buf[4],send_buf);  
				break;

			case Dev_list:
				Length =21;
				bzero(send_buf,25);
				memcpy(send_buf,(char *)&Length,4);
				send_buf[MY_NET_CMD_POS] = (char)Dev_list;
				memcpy(&send_buf[5],(char *)&cmd,4);
				memcpy(&send_buf[9],(char *)&dev_name_length,4);
				memcpy(&send_buf[13],dev_name,12);
				write(*conn_fd,send_buf,25);
                                break;

			case Open_Camera:
				send_video_data_handle(conn_fd);
				break; 
			case Close_Camera:
				close(*conn_fd);
				free(conn_fd);
				return;
				break;


			case Open_Beep:
			case Close_Beep:
			case Open_Led:
			case Close_Led:
			case Open_Fan:
			case Close_Fan:
				uartdata_recv_handle(conn_fd,recv_buf[4]);
				break;

			case Open_Sensor:
                                uartdata_send_handle(conn_fd);
				break;
			case Close_Sensor:
				close(*conn_fd);
				free(conn_fd);
				return;
				break;
			case Quit:
				close(*conn_fd);
				free(conn_fd);
				return;
				break;	

			default:
				break;        

		}
                usleep(10000);
	}
	
}
