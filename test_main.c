/********************************************************
*   Copyright (C) 2016 All rights reserved.
*   
*   Filename:main.c
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
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "myserver_thread_pool.h"
#include "myserver_net.h"
#include "myserver_process.h"



int main (int argc, char **argv) 
{ 

	/*if(argc != 2){
		printf("Usage:./xxx port!\n");
		exit(EXIT_FAILURE);
	}*/
	
	if(0>mycamera_init()){
		printf("camera init failed!\n");
		exit(EXIT_FAILURE);
	}
	printf("camera init success!\n");
#ifdef YUYV        
        if(0>convert_init()){
        	printf("convert init failed!\n");
		exit(EXIT_FAILURE);
        }
        printf("convert init success!\n");
#endif	
	
/*	if(0>uart_init()){
		printf("uart init failed!\n");
		exit(EXIT_FAILURE);
	}
	printf("uart init success!\n");
  */     	
       	//初始化线程池
	pool_init(100);
	
	
	int socket_fd,*conn_fd;
       	if(0>server_net_init(&socket_fd)){
       		printf("server net init failed!\n");
       		exit(EXIT_FAILURE);
       	}
       	printf("server net init success!\n");
       		
	int client_addr_len = sizeof(client_addr);
	memset(&client_addr,0,client_addr_len);
	while(1)
	{
		conn_fd = (int *)malloc(sizeof(int));
		if((*conn_fd = accept(socket_fd,(struct sockaddr *)(&client_addr),&client_addr_len)) == -1)
		{
			perror("accept error!");
			exit(-1);
		}
		//执行process，将process任务交给线程池
		pool_add_task(my_net_process,conn_fd);
	}

	close(socket_fd);

	return 0; 
}
