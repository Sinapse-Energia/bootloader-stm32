/*
 * M95.h
 *
 *  Created on: 7 feb. 2018
 *      Author: juanra
 */

#ifndef M95_H_
#define M95_H_
#include "Transceiver.h"

class QuectelM95: public Transceiver {
public:
	int		ConnectTCP				(const char *apn, const char *host, int port);
	int		ConnectTLS				(const char *apn, const char *host, int port);


	//	int		SendData				(int sock, const char *data, int length);
	//	int		GetData					(int count, char *dest);

			QuectelM95();
private:

};






#endif /* M95_H_ */
