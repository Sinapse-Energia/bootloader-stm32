
//#include "stm32f2xx_hal.h"
#include "GPRS_transport.h"


int 	gsocket;   			// temporary trick to discriminate secure/clear
void	*gtransceiver;		// PROVISIONAL


int transport_open(const char* host, int port, int security, char *apn) {
	int handle = CONNECT((DEV_HANDLER) gtransceiver, apn, host, port, security);
	gsocket = handle;
	return handle;
}


int transport_close(int sock) {
	return DISCONNECT(gtransceiver);
}

int transport_sendPacketBuffer(int sock, unsigned char* buffer, int lengthBuffer) {
	return SENDDATA(gtransceiver, sock, buffer, lengthBuffer);
}


int transport_getdata(unsigned char* buffer, int count) {
	return GETDATA(gtransceiver, count, buffer);
}

int transport_getdatanb(void *sck, unsigned char *buffer, int count) {
	return transport_getdata(buffer, count);
}





