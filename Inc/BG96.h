/*
 * BG96.h
 *
 *  Created on: 7 feb. 2018
 *      Author: juanra
 */

#ifndef BG96_H_
#define BG96_H_
#include "Transceiver.h"


class QuectelBG96: public Transceiver {
public:
	int		ConnectTCP				(const char *apn, const char *host, int port);
	int		ConnectTLS				(const char *apn, const char *host, int port);


//	int		SendData				(int sock, const char *data, int length);
//	int		GetData					(int count, char *dest);

			QuectelBG96();
private:

};





#endif /* BG96_H_ */
