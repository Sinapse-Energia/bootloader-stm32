/*
 * utils.c
 *
 *  Created on: Jun 19, 2017
 *      Author: External
 */


#include 	<stdio.h>
#include 	<string.h>
#include 	<stdarg.h>
#include	"utils.h"


struct tm	ECdatetime;

long		ECtimestamp;


// The function now can deal with the reply oof CCLK command (NTP server), and QLTS command (Network time)

int SetDateTime(const char *reply){
	/**
	int			utcoffset = 0;
	char		Ttype[16];
	int n = sscanf (reply+2, "+%s \"%d/%d/%d,%d:%d:%d+%d", Ttype, &ECdatetime.tm_year, &ECdatetime.tm_mon, &ECdatetime.tm_mday, &ECdatetime.tm_hour, &ECdatetime.tm_min, &ECdatetime.tm_sec, &utcoffset);
	if ( n ==8 ){
		if (!strcmp (Ttype, "CCLK:"))
			utcoffset = 1;
		if (!strcmp (Ttype, "QLTS:"))
			utcoffset = utcoffset/4;

		ECdatetime.tm_year += 2000;
		ECdatetime.tm_hour += utcoffset;

		if (ECdatetime.tm_sec > 5) {
			ECdatetime.tm_sec -= 5;
		}
		else {
			ECdatetime.tm_sec = 0;
		}
	//	sscanf (NTP+2, "%d/%d/%d", &ECdatetime.tm_year, &ECdatetime.tm_mon, &ECdatetime.tm_mday);
		ECtimestamp = ((ECdatetime.tm_hour * 60 + ECdatetime.tm_min) * 60 ) + ECdatetime.tm_sec;
		return 1;
	}
	else
		return 0;
		**/
	return 1;
};


/**
long GetTimeStamp(){
	return ECtimestamp;
}
**/

/**
void AddSeconds(int n){
	static int counter = 0;
	UpdateTimeStamp(10);
	counter += n;
	if (counter > 60 ){
		AddMinute(1);
		counter = counter - 60;
	}
}**/

/**
void AddMinute(int n){
}
**/


/**
void UpdateTimeStamp(int n){
	ECtimestamp += n;
	ECdatetime.tm_sec += n;
	if (ECdatetime.tm_sec > 59) {
		ECdatetime.tm_min += ECdatetime.tm_sec / 60;
		ECdatetime.tm_sec = ECdatetime.tm_sec % 60;
		if (ECdatetime.tm_min > 59) {
			ECdatetime.tm_hour += ECdatetime.tm_min / 60;
			ECdatetime.tm_min = ECdatetime.tm_min % 60;
			if (ECdatetime.tm_hour > 23) {
				ECdatetime.tm_mday += ECdatetime.tm_hour / 24;
				ECdatetime.tm_hour = ECdatetime.tm_hour % 24;
			}
		}

	}

}
**/

/**
char	*strDateTime() {
	static	char	DT[64];
	sprintf (DT, "%d-%d-%d %02d:%02d:%02d",
					ECdatetime.tm_mday, ECdatetime.tm_mon, ECdatetime.tm_year,
					ECdatetime.tm_hour, ECdatetime.tm_min, ECdatetime.tm_sec);
	return DT;
}
**/

// Pending : to validate the IP format is correct

int	 SetLocalIP(const char *txt){
	/**
	char	myIP[20];
	strcpy (myIP, txt+2);
	char *p = strchr(myIP, '\r');
	if (p)
		*p = 0;
	SetVariable ("LOCALIP", myIP);
	**/
	return 1;
}


// Pending : to validate the SIM format is correct

int	SetIdSIM(const char *txt){
	/**
	char	SIM[40];
	strcpy (SIM, txt+2);
	char *p = strchr(SIM, '\r');
	if (p)
		*p = 0;
	SetVariable ("IDSIM", SIM);
	**/
	return 1;
}


int	SetIMEI(const char *txt){
	/**
	char	IMEI[32];  // supossed to have 16 chars
	strncpy (IMEI, txt+2, 20);  // provisional.... sometimes comes longer to null terminator
	char *p = strchr(IMEI, '\r');
	if (p)
		*p = 0;
	SetVariable ("IMEI", IMEI);
	**/
	return 1;
}


int		ValidateReg(const char *reply){

	int	n;
	int stat;
	char	cmd[10];
	int x = sscanf (reply, "\r\n+%s %d,%d\r\n\r\nOK\r\n", cmd, &n, &stat );
	printf ("tengo %s, %d, %d", reply, n, stat);
	if (x == 3) {
		if ( (n == 0) && ((stat == 1) || (stat == 5)))
				return 1;
		if ( (n == 0) && (stat == 2))
			return -1; //it lasts.... retry again
		else
			return 0;
	}
	else
		return 0;

}



int		SetGeneric(const char *txt){

	return 1;
}

int		SetState(const char *txt){
	//const char *tmp = txt;

	return 1;
}


// Helper function to accumulate trace until it can be shown
// Small buffer allocation until verify how much memory is available
/**
char	pretrbuf[500] = "";
int		pretrace(char *texto,...) {
	va_list	  ap;

	va_start	  (ap, texto);
	vsprintf (pretrbuf+ strlen(pretrbuf), texto, ap);
	return 1;
}
**/
//
//
// 	This is a PROVISIONAL function intended to verify the message manipulation, without having to depend on message reception
//	Uses a arbitrary sequence of commands in a circular list....
//	Each item de function is called, returns the next message form the list

/**
char	*GetLocalMessage(int h, char *buffer, int maxsize){
	static  	char 	*messages[] = {
	  			"1;",
				"3;20;",
				"1;",
				"3;60;",
				"1;",
				"3;90;",
				"1;",
				"3;0;"
	  	};
	static int i = 0;
	char	*result = messages[i];
	i = (i+1)%(sizeof(messages)/sizeof(*messages));
	if (strlen(result) < maxsize)
		strcpy (buffer, result);
	else
		strncpy(buffer, result, maxsize);
	// HAL_Delay(1000);
	return buffer;
}
**/



/**
#define STORESIZE 512
char	store[STORESIZE];
**/

/**
int	SaveALL(){
//	const char	*listofvars[] = { "IP","PORT", "USER", "PSWD", "SEC", "LIP", "LPORT","LUSER", "LPSWD","LSEC", "DSTAT", "APN", "LAPN", NULL};
	const char	*listofvars[] = {
			"IP","PORT", "USER", "PSWD", "SEC", "LIP", "LPORT","LUSER", "LPSWD","LSEC", "APN", "LAPN",
			"MPERIOD", "MPTIMES", "MPBEGIN", "MPEND", "DSTAT",
			NULL
	};
	int i = 0;
	store[0] = 0;
	while (listofvars[i]) {
		char item[128];
		const char *name = listofvars[i];
		char *value = GetVariable(name);
		if (value){
			sprintf (item, "%s=%s;",  name, value );
			if (strlen(store) + strlen(item) < STORESIZE){
				strcat(store, item);
				i++;
			}
			else {
				break;
			}
		}
		else
			i++;
	}
	MIC_Flash_Memory_Write((const uint8_t *) store, (uint32_t)strlen(store)+ 1);
	return 1;
}
**/

/**
int	SaveConnParams(){
	return SaveALL();
}
**/

/**
int	SaveAppParams(){
	return SaveALL();
}
**/

/**
int RecConnParams(){
	MIC_Flash_Memory_Read( (const uint8_t *) store, sizeof(store));
	WriteData((char *) store);
}
**/
/**
int RecAppParams(){
	MIC_Flash_Memory_Read( (const uint8_t *) store, sizeof(store));
	WriteData((char *) store);
}
**/
/**
#ifdef  TLS
// Array of bytes to hold the BINARY representation of the CA
static	uint8_t	certificate[2000];
// String with the hardcoded base64 representation of the CA
const char	*textCert= "MIIEATCCAumgAwIBAgIJALFjqVBg1LHlMA0GCSqGSIb3DQEBCwUAMIGWMQswCQYD"
				"VQQGEwJFUzEPMA0GA1UECAwGTWFkcmlkMQ8wDQYDVQQHDAZNYWRyaWQxEDAOBgNV"
				"BAoMB1NpbmFwc2UxFDASBgNVBAsMC0RldmVsb3BtZW50MQ0wCwYDVQQDDARNUVRU"
				"MS4wLAYJKoZIhvcNAQkBFh9yYWZhLmFsY2FpZGVAc2luYXBzZWVuZXJnaWEuY29t"
				"MB4XDTE3MDkxNDExMjY0MloXDTIyMDkxNDExMjY0MlowgZYxCzAJBgNVBAYTAkVT"
				"MQ8wDQYDVQQIDAZNYWRyaWQxDzANBgNVBAcMBk1hZHJpZDEQMA4GA1UECgwHU2lu"
				"YXBzZTEUMBIGA1UECwwLRGV2ZWxvcG1lbnQxDTALBgNVBAMMBE1RVFQxLjAsBgkq"
				"hkiG9w0BCQEWH3JhZmEuYWxjYWlkZUBzaW5hcHNlZW5lcmdpYS5jb20wggEiMA0G"
				"CSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDCfuQkv6LrQoyynZMi4jsmGgQ32geK"
				"5v7fYWCH3dB7f6KVDozRraFW4a9vqkhQEpwmMcXZ6+aI/NaFW0ifc16BKdXXMslw"
				"aJmQIBbBgrpXCYCmED7v4h8bBYNoA+/yIqo+EAYfLSYwMvM/9D7n2x28LiytuNsf"
				"Nsga6tToN2rIIfsMrCRxBT4Ex8NKlpyRF0EO29jbZFrlUXp0wqowZFobgM5mgNjG"
				"MSYzPJFvlF/Hlp5XL2MM5nbgWWYgGk6w2Ep79gR4a32Np0Gq8C3r7avv29num6Mu"
				"3pKp+wiBUtZzafNHGqbRClcQEUZk7E6K/yXRqEfOdBO1RZE99/orO7vTAgMBAAGj"
				"UDBOMB0GA1UdDgQWBBT4A15SVpWhKsWWWD9nWnn91bPVDDAfBgNVHSMEGDAWgBT4"
				"A15SVpWhKsWWWD9nWnn91bPVDDAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBCwUA"
				"A4IBAQBdkrJkQ5f6wA+FoofKy19GewXWtn6UTnE1U/dVehJmB8WXxnLZAqH7aPq8"
				"kFN+MsAXa6qaLe9C52ne75bop8rzLE73fcWjRtfbJr5fovk32jF20VL0QhjRfwko"
				"pRqqrYBoDMzpvyzPaJnPEcqyVf+MHVPqaRdnXz/9Qqumu9yz09XVBPL6KkskpQgj"
				"oZs2jpCPSDhAavzNTJzQuJmVYj5eYTBBMmg8y9UENqvmQsUPFx8lAcWF/BcKLBH5"
				"BMCnZ9MYAwA4kwMpv4/yeKfDShF9RbG2U6rSbW0Rv4bpS5IuKBgIyWdfJf3Qcb1c"
				"IsqfjZ5GlW+Gih34ZxlzNYEAjm/j";


// function to get the raw certificate (converting it from text, if necesary..)
uint8_t	*getCertificate(size_t *lcert){
	size_t ltext = strlen(textCert);
	if (!*certificate) {
		base64_decode(textCert, ltext, certificate, lcert);
	}
	return certificate;
}


// the base64 direct (bin to text) conversion base table
static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};


// Placeholder to build the base64 inverse (text to bin) conversion base table
static char decoding_table[256] = {255};


void build_decoding_table() {
	size_t i;

    for (i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}


unsigned char *base64_decode(const char *data, size_t input_length, uint8_t *output, size_t *output_length) {

    uint8_t *decoded_data = output;
	size_t i, j;

    if (decoding_table[0] == 255)
    	build_decoding_table();

    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;


    for (i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
        + (sextet_b << 2 * 6)
        + (sextet_c << 1 * 6)
        + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}

#endif

**/
