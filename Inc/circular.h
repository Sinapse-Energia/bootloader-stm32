/*
 * circular.h
 *
 *  Created on: Jun 14, 2017
 *      Author: External
 */

#ifndef CIRCULAR_H_
#define CIRCULAR_H_


#include "stm32f4xx_it.h"  // DMA or IRQ
#include "stm32f4xx_hal.h"  // uintxx_t
#include "stm32f4xx_hal_dma.h"


extern int			balold;
extern int			balnew;
extern int			allold;
extern int			allnew;

extern int			allwr;

extern	int			nfulls;





typedef	struct	st_circularbuffer {
	size_t					size;
	DMA_HandleTypeDef 		*hdma;
	unsigned int			wrindex;
	unsigned int			rindex;
	unsigned int			overruns;
	uint8_t					*buffer;
} st_CB;


extern	st_CB	*DataBuffer;

extern	st_CB	*CircularBuffer	(size_t, DMA_HandleTypeDef *);

extern 	int		IsEmpty (st_CB *);
extern	int		IsFull 	(st_CB *);
extern	int		Stock 	(st_CB *);
extern	int		Reset	(st_CB *);
extern	int		Read	(st_CB *);

extern	int		Write	(st_CB *, uint8_t );   // only when IRQ is used


// looks for a pattern any place in the buffer content
extern int	Lookup	(st_CB *, uint8_t *);
// looks for a pattern from the start of the buffer content
extern int	Match	(st_CB *, uint8_t *);

#endif /* CIRCULAR_H_ */
