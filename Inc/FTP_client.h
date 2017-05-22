//////////////////////////////////////////////////////////
//
//  Name:
//      FTP_client.h
//
//  Purpose:
//      Read file from remote FTP server
//////////////////////////////////////////////////////////

#ifndef __FTP_CLIENT_H
#define __FTP_CLIENT_H

// --- INCLUDES ---
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// --- DEFINES ---
// Comment below for embedded MCU compile
//#define LINUX_ENV		1

#define FTP_BUFF_LEN	512

// --- DATA ---
struct FTP_sockaddr_in{
	char addr[64];
    unsigned short port;
};


// --- FUNCTIONS PROTOTYPES ---
int FTP_sockCMD_init(void);
void FTP_readServ(int s) ;
int FTP_sockDATA_init(int s);
int FTP_login(int s);
int FTP_getfile(int s, int ds, const char *file, char *data_out, int max_len);

int FTP_test(void);

#endif // __FTP_CLIENT_H
