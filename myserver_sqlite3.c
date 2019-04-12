/********************************************************
*   Copyright (C) 2016 All rights reserved.
*   
*   Filename:myserver_sqlite3.c
*   Author  :Lematin
*   Date    :2016-12-06
*   Describe:
*
********************************************************/
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sqlite3.h>
#include "myserver_sqlite3.h"


int callback_1(void *para,int f_num,char **f_value,char **f_name)
{
#ifdef DEBUG	
	printf("callback:%s\n",(char *)para);
#endif
	/*判断是否与查询结果一样*/
	if(!strcmp((char *)para,f_value[0])){
		strcpy((char *)para,"SAME");
	}else{
		strcpy((char *)para,f_value[0]);
	}
	return 0;

}
int callback_2(void *para,int f_num,char **f_value,char **f_name)
{
#ifdef DEBUG	
	printf("callback:%s\n",(char *)para);
#endif
	int i;
	for(i=0;i<f_num;i++){
		strcat((char *)para,f_value[i]);
		strcat((char *)para,"\n");

	}	
	return 0;

}


int sqlite3_insert(char *name,char *pwd)
{

	/*打开数据库*/
	sqlite3 *db = NULL;
	if (0 != sqlite3_open("my.db", &db)) {
		fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errmsg(db));
		return -1;
	}
#ifdef DEBUG	
	printf("sqlite3_open success!\n");
#endif
	char *errmsg = NULL;

	/*判断该用户名是否已经被注册*/
	char name_temp[50];
	strcpy(name_temp,name);
	char *zSQL1=sqlite3_mprintf("select name from user where name=('%q')",name);
	if (0 > sqlite3_exec(db,zSQL1, callback_1,name_temp, &errmsg)) {
		fprintf(stderr, "sqlite3_exec: %s\n", errmsg);
		return -1;
	}
#ifdef DEBUG	
	printf("name_temp:%s\n",name_temp);
#endif
	
	/*如果被注册则返回-1,终止注册*/
	if(!strncmp(name_temp,"SAME",4)){
		return -1;
	}

	char *zSQL2=sqlite3_mprintf("insert into user values('%q','%q')",name,pwd);
	if (0 > sqlite3_exec(db,zSQL2, NULL, NULL, &errmsg)) {
		fprintf(stderr, "sqlite3_exec: %s\n", errmsg);
		return -1;
	}
	/*关闭数据库*/
	if (0 != sqlite3_close(db)) {
		fprintf(stderr, "sqlite3_close: %s\n", sqlite3_errmsg(db));
		return -1;
	}
#ifdef DEBUG	
	printf("sqlite3_close success!\n");
#endif
	
	return 0;
}


int sqlite3_query_user(char *name,char *pwd)
{
	/*打开数据库*/
	sqlite3 *db = NULL;
	if (0 != sqlite3_open("my.db", &db)) {
		fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errmsg(db));
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG	
	printf("sqlite3_open success!\n");
#endif
	char *errmsg = NULL;

	/*判断该用户名和密码是否正确*/
	char pwd_temp[50];
	strcpy(pwd_temp,pwd);	
	char *zSQL1=sqlite3_mprintf("select pwd from user where name=('%q')",name);
	if (0 > sqlite3_exec(db,zSQL1, callback_1,pwd_temp, &errmsg)) {
		fprintf(stderr, "sqlite3_exec: %s\n", errmsg);
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG	
	printf("pwd_temp:%s\n",pwd_temp);
#endif
	
	/*如果密码不正确,返回-1*/
	if(strncmp(pwd_temp,"SAME",4)){
		return -1;
	}

	/*关闭数据库*/
	if (0 != sqlite3_close(db)) {
		fprintf(stderr, "sqlite3_close: %s\n", sqlite3_errmsg(db));
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG	
	printf("sqlite3_close success!\n");
#endif
	return 0;
}



int sqlite3_chgpwd(char *name,char *npwd,char *opwd)
{
	/*打开数据库*/
	sqlite3 *db = NULL;
	if (0 != sqlite3_open("my.db", &db)) {
		fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errmsg(db));
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG	
	printf("sqlite3_open success!\n");
#endif
	char *errmsg = NULL;

	/*更改用户密码*/
	char *zSQL1=sqlite3_mprintf("update user set pwd=('%q') where name=('%q') and pwd=('%q')",npwd,name,opwd);
	if (0 > sqlite3_exec(db,zSQL1, NULL,NULL, &errmsg)) {
		fprintf(stderr, "sqlite3_exec: %s\n", errmsg);
		exit(EXIT_FAILURE);
	}
	/*判断该用户的密码是否已经更改成功*/
	char pwd_temp[50];
	strcpy(pwd_temp,npwd);	
	char *zSQL2=sqlite3_mprintf("select pwd from user where name=('%q')",name);
	if (0 > sqlite3_exec(db,zSQL2, callback_1,pwd_temp, &errmsg)) {
		fprintf(stderr, "sqlite3_exec: %s\n", errmsg);
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG	
	printf("pwd_temp:%s\n",pwd_temp);
#endif
	
	/*如果密码没有修改成功,返回-1*/
	if(strncmp(pwd_temp,"SAME",4)){
		return -1;
	}

	/*关闭数据库*/
	if (0 != sqlite3_close(db)) {
		fprintf(stderr, "sqlite3_close: %s\n", sqlite3_errmsg(db));
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG	
	printf("sqlite3_close success!\n");
#endif
	return 0;
}
