/*
 * RM08.h
 *
 *  Created on: 26 abr. 2018
 *      Author: juanra
 */

#ifndef RM08_H_
#define RM08_H_

#include "Transceiver.h"



class HilinkRM08 : public Transceiver {
public:
	// Factory
	static HilinkRM08	*TRANSCEIVER	(DEV_HANDLER, st_CB *);
	// Restore initial state
	static void	Set					();
	static void	ReSet					();

			HilinkRM08				(void *hc);
	int		Configure				();
	int		ConfigureLAN			();
	int		ConfigureWifi			(const char *ssid, const char *pwd);
	int		ConnectTCP				(const char *apn, const char *host, int port);
	int		ConnectUDP				(const char *apn, const char *host, int port);
	int		DisConnect				();
	int		ExecuteCommand			(const char *cmd, char *destination, int (* hfun ) (const char *)) { return 0; }

	int		SynchronizeTime					();

private:
	int		Connect					(const char *host, int port, int protocol);
//	void	*handler;				// the communication channel
};






#endif /* RM08_H_ */
