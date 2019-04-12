/********************************************************
*   Copyright (C) 2016 All rights reserved.
*   
*   Filename:serv_sqlite3.h
*   Author  :Lematin
*   Date    :2016-12-06
*   Describe:
*
********************************************************/
#ifndef _SERV_SQLITE3_H
#define _SERV_SQLITE3_H
#include <sqlite3.h>


int sqlite3_insert(char *name,char *pwd);
int sqlite3_query_user(char *name,char *pwd);
int sqlite3_chgpwd(char *name,char *npwd,char *opwd);


#endif
