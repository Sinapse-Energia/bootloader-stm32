/*
 * NBinterface.h
 *
 *  Created on: May 26, 2017
 *
 *      Author: External
 *
 *
 *		This file declares the C++ functions with C storage
 *		providing entry points to the Nothbound subsystem
 *		form de main program and/or all the remainder C stuff
 *
 */

#ifndef NBINTERFACE_H_
#define NBINTERFACE_H_

#include "Definitions.h"


#ifdef __cplusplus
extern "C" {
#endif



	int		CreateContext	();
	int		ReadMetadata	(char *domainIn, char *domainOut);

	// this function returns the payload in the arrived subscription (on any topic)
	char	*ProcessMessage	(char *message);

	//		Helper functions for reading/writing variables forn C
	char	*GetVariable	(const char *name);
	int		SetVariable		(const char *name, char *value);


	// Actions called at pre or post-process
	// Save & Restore Connection Params
	int		SaveConnParams	(void);
	int 	RecConnParams	(void);

	// Save & Restore App Params
	int		SaveAppParams	(void);
	int 	RecAppParams	(void);

	// Save & Restore BL Params
	int		SaveBLParams	(void);
	int 	RecBLParams	(void);


	int		Flush			(void);

	int		SHORTNEWCONN		(void);
	int		FULLNEWCONN			(void);

	//	C linkage call to the Scheduler
	int		Schedule			(void);

	// make publication, transport agnostic
	int		Publish				(char *message);

	// send message log transport agnostic
	int		Log				(char *message);


	void	*GetSubspaceElement	(const char *prefix, const char *id);


	void	*MODEMFACTORY		(void *, st_CB *);
	void	*ETHFACTORY			(void *, st_CB *);


	int		BuildALL 				(void);
	int		BuildINs 				(void);
	int		BuildOUTs 				(void);

#ifdef DEBUG
	char	*Set_TimeOffset(const char *);
#endif


#ifdef __cplusplus
}
#endif









#endif /* NBINTERFACE_H_ */
