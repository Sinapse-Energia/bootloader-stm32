
#ifndef __SHARINGMEMORY
#define __SHARINGMEMORY

typedef struct {

	char APN[128];
	char LAPN[128];
	char UPDFW[2];
	char UPDFW_COUNT[10];
	char UPDFW_HOST[128];
	char UPDFW_PORT[10];
	char UPDFW_NAME[128];
	char UPDFW_ROUTE[128];
	char ID[30];
	char GPIO[10];
	char PWM[10];

} _SHARING_VARIABLE;

#endif
