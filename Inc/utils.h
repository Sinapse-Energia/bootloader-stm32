/*
 * utils.h
 *
 *  Created on: Jun 19, 2017
 *      Author: External
 */

#ifndef UTILS_H_
#define UTILS_H_

#include	<time.h>

#ifdef __cplusplus
extern "C" {
#endif


// Functions to handle timestamp
//	get the scalar timestamp
//extern	long 	GetTimeStamp(void);
// get the stringized timestamp
//extern	char	*strDateTime();
// Update timestamp adding N seconds
//extern	void 	AddSeconds(int num);
//extern	void 	AddMinute(int num);
//extern	void 	UpdateTimeStamp(int num);

// Handlers for AT-COMMANDS whose output deserves to be managed
extern	int		SetDateTime(const char *NTP);
extern	int		SetLocalIP(const char *iptx);
extern	int		SetIdSIM(const char *simtx);
extern	int		SetIMEI(const char *txt);

extern	int		ValidateReg(const char *regreply);


extern	int		SetGeneric(const char *reply);
extern	int		SetState(const char *reply);

//extern	char	*GetLocalMessage(int h, char *buffer, int maxsize);
//extern	int		pretrace(char *texto,...);

//extern void		WriteData(char *p); // this function is in Context.cpp but has to be moved to utils


//extern	unsigned char 	*base64_decode(const char *data, size_t input_length, uint8_t *output, size_t *output_length);
//extern	uint8_t			*getCertificate(size_t *lcert);

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
}
#endif


#endif /* UTILS_H_ */
