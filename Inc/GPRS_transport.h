#ifndef __GPRS_TRANSPORT_H
#define __GPRS_TRANSPORT_H


#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx_hal.h"  // make visible uart types where needed

extern void	*gtransceiver;		// PROVISIONAL

typedef		void	*DEV_HANDLER;

	DEV_HANDLER		Device_Init	();
	void			Device_Reset();


	int				CONNECT		(DEV_HANDLER 	ph, const char *apn, const char *host, int port, int security);
	int				SENDDATA	(DEV_HANDLER	ph, int sock, char *data, int n);
	int				GETDATA		(DEV_HANDLER	ph, int n, char *destination);
	int				DISCONNECT	(DEV_HANDLER	ph);
	int				EXECCOMMAND	(DEV_HANDLER	ph, const char *command, char *, int (* hfun ) (const char *));






int transport_open(const char* host, int port, int security, char *apn);
int transport_close(int sock);
int transport_sendPacketBuffer(int sock, unsigned char* buf, int buflen);
int transport_getdata(unsigned char* buf, int count);
int transport_getdatanb(void *sck, unsigned char *buf, int count);

//typedef	UART_HandleTypeDef *HDEVICE;

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
}
#endif


#endif
