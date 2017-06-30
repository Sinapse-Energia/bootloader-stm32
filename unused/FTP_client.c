#include "FTP_client.h"

#ifdef LINUX_ENV
#include <arpa/inet.h>
#include <select.h>

//#include <netinet/in.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <sys/time.h>
#else
#include "stm32f4xx_hal.h"

#endif //LINUX_ENV


// Input/Output buffer
char ftp_buff[FTP_BUFF_LEN];

// Current socket source selector
//SOCKETS_SOURCE ftp_socket = SOCKET_SRC_NONE;

//**************************************************************************
// Portable functions start
//**************************************************************************
int _sock_init(int s)
{
#ifdef LINUX_ENV
	int s = -1;
    s = socket(AF_INET, SOCK_STREAM, 0);
#else
    // MCU socket init here
    Socket_Init(s);// == SOCKET_OK) {
#endif //#ifdef LINUX_ENV

    return s;
}

int _sock_connect(int s, const char *addr, unsigned int port)
{

#ifdef LINUX_ENV
	struct sockaddr_in address;
    int len;
    int result;

    // Domine
    address.sin_family = AF_INET;
    // Server address
    address.sin_addr.s_addr = inet_addr(addr);
    // Port (21)
    address.sin_port = htons(port);
    len = sizeof(address);
    // Connect to socket (almost 25 seconds timeout)
    return connect(s, (const struct sockaddr *)&address, len);
#else

    if (Socket_Connect(s) != SOCKET_OK) {
    	return SOCKET_SRC_NONE;
    }

    return s;
#endif //#ifdef LINUX_ENV
}



int _sock_read(int s, char *data_out, int data_len)
{
#ifdef LINUX_ENV
	return recv(s, data_out, data_max, 0);
#else
	return Socket_Read(s, data_out, data_len);
#endif //#ifdef LINUX_ENV

	return  0;
}


int _sock_write(int s, const char *data_in, int data_len)
{
#ifdef LINUX_ENV
	return send(s, data_in, data_len, 0);
#else
    // MCU socket write here
	Socket_Write(s, data_in, data_len);
#endif //#ifdef LINUX_ENV

	return  data_len;
}
//**************************************************************************
// Portable functions stop
//**************************************************************************



//**************************************************************************
// Init CMD Socket and return its descriptor
//**************************************************************************
int FTP_initCMD(int s, const char* ftp_addr, int ftp_port)
{
	s = _sock_init(s);
	int result = _sock_connect(s, ftp_addr, ftp_port);
    if (result <= 0) {
        perror("Socket connection Error!");
        return 0;
    }

    return s;
}

//**************************************************************************
// Readback and output server answer
//**************************************************************************
void FTP_readServ(int s) 
{
//    fd_set fdr;
    int rc;    
//    struct timeval timeout;

//    FD_ZERO(&fdr);
//    FD_SET(s, &fdr);
    
    // Set server answer timeout (1 sec)
//    timeout.tv_sec = 1;
//    timeout.tv_usec = 0;
    
    do {
        // Get data from stream
    	rc = _sock_read(s, ftp_buff, FTP_BUFF_LEN);
        printf("%s", ftp_buff);        
        // Wait for data with 1-sec timeout
        //rc = select(s + 1, &fdr, NULL, NULL, &timeout);
    } while(rc);     
}

//**************************************************************************
// Init DATA Socket and return its descriptor
//**************************************************************************
int FTP_initDAT(int s, const char* ftp_addr, int ftp_port)
{
    char *tmp_char;
    int a,b;    
    int c,d,e,f;
    int port;
    int ds;
    int result;

    _sock_write(s, "PASV\r\n", strlen("PASV\r\n"));
    
    _sock_read(s, ftp_buff, FTP_BUFF_LEN);
    // Output server string to screen
    printf("%s", ftp_buff);

    tmp_char = strtok(ftp_buff,"(");
    tmp_char = strtok(NULL,"(");
    tmp_char = strtok(tmp_char, ")");
    
    sscanf(tmp_char, "%d,%d,%d,%d,%d,%d",&c,&d,&e,&f,&a,&b);
    
    port = a * 256 + b;
    ds = _sock_init(s);
    result = _sock_connect(ds, ftp_addr, port);
    if (result <= 0) {
        perror("Data socket connection Error!");
        return 0;
    }
    return ds;
}

//**************************************************************************
// Login / passw on a server
//**************************************************************************
int FTP_login(int s, const char* ftp_login, const char* ftp_pass)
{
    sprintf(ftp_buff,"USER %s\r\n", ftp_login);
    _sock_write(s, ftp_buff, strlen(ftp_buff));
    FTP_readServ(s);
    sprintf(ftp_buff,"PASS %s\r\n", ftp_pass);
    _sock_write(s, ftp_buff, strlen(ftp_buff));
    //FTP_readServ(s); 
    
    // Check Login result ('OK' string)
    _sock_read(s, ftp_buff, FTP_BUFF_LEN);
    // Output server string to screen
    printf("%s\r\n", ftp_buff);

    if (strstr(ftp_buff, "230")) return 1; // OK
    
    perror("Login/Password Failed!\r\n");
    return 0; // Err
}

//**************************************************************************
// Read desire file into a memory
//**************************************************************************
int FTP_getfile(int s, int ds, const char *file, char *data_out, int max_len) 
{
    int file_size;   
    int read;
    int readed;
    char *tmp_size;
    FILE *f;
    
    // Get file size
    sprintf(ftp_buff, "SIZE %s\r\n", file);
    _sock_write(s, ftp_buff, strlen(ftp_buff));

    // Read answer	 
    file_size = _sock_read(s, ftp_buff, FTP_BUFF_LEN);
//    printf("ANS:%s\r\n", ftp_buff);
    // Check file existance
    if (!strstr(ftp_buff, "213")) {
        perror("No file found!\r\n");
        return 0; // Err
    }

    // Output file size
    tmp_size = strtok(ftp_buff," ");
    tmp_size = strtok(NULL," ");
    sscanf(tmp_size,"%d", &file_size);
    printf("fileseize=%i\r\n", file_size);

    // Read file data
    sprintf(ftp_buff, "RETR %s\r\n", file);
    _sock_write(s, ftp_buff, strlen(ftp_buff));

    // Open file in binary! mode
    f = fopen(file, "wb");   
    // Start with read 0 bytes
    read = 0;  
    do {            
            // Read data from 'data socket'
            readed = _sock_read(ds, ftp_buff, sizeof(ftp_buff));
            // Check max length
            if (max_len < readed) {
                readed = max_len;
            } 
            // Copy data to output memory location
            memcpy(data_out, ftp_buff, readed);
            // Write data to file
            fwrite(ftp_buff, 1, readed, f);   
            // add data
            read += readed;  
            data_out += readed; 
            max_len -= readed;
    } while ((read < file_size) && (max_len));
    
    fclose(f);    

    return 0; // Ok
}

//**************************************************************************
// Read desire file into a memory
//**************************************************************************
int FTP_putfile(int s, int ds, const char *file, const char *data_in, int data_len)
{
    // Sent filename
    sprintf(ftp_buff, "STOR %s\r\n", file);
    _sock_write(s, ftp_buff, strlen(ftp_buff));

    // Read server answer
    /*readed =*/ _sock_read(s, ftp_buff, sizeof(ftp_buff));

    // Sent file content
    _sock_write(ds, data_in, data_len);

    return 0; // Ok
}


//**************************************************************************
// Test FTP connection
//**************************************************************************
int FTP_test(void)
{
    int s = 0;
    int ds = 0;
    char output[100];

     // Create Command socket
     s = FTP_initCMD(0, FTP_SERVER_IP, FTP_SERVER_PORT);
     if (s >= 0)	{
         FTP_readServ(s);

         if (FTP_login(s, FTP_SERVER_USERNAME, FTP_SERVER_PASSWORD)) {
             // Create Data socket
             ds = FTP_initDAT(s, FTP_SERVER_IP, FTP_SERVER_PORT);
             if (ds >= 0) {
                 FTP_getfile(s, ds, FTP_SERVER_FW_FILENAME, output, 100);
                 FTP_putfile(s, ds, "mylog2.log", "test content", 12);
                 // Close DATA connection
                 close(ds);
             }
         }

        // Close CMD connection 
        close(s);  
    } else {
     printf("Err s=%i\r\n", s);

    }

    return 0;  
}
