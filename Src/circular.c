/*
 * circular.c
 *
 *  Created on: Jun 16, 2017
 *      Author: External
 */


#include <stdlib.h>  // malloc
#include <string.h>

#include "circular.h"

  int allold = 0;
  int allnew = 0;
  int allwr = 0;

  int balold = 0;
  int balnew = 0;


  //	Constructor-like C linkage function
  //
  st_CB	*CircularBuffer(size_t size, DMA_HandleTypeDef *h){
  	st_CB *result = (st_CB *) malloc (sizeof(st_CB));
  	result->size = size;
  	result->rindex = 0;
  	result->wrindex = 0;
  	result->overruns = 0;
  	result->hdma = h;
  	result->buffer = malloc (size);
  	memset(result->buffer, 0, size);
  	return result;
  }



  // returns the number of chars available to read
  int		Stock (st_CB *cb) {
	if (cb->hdma)
		//cb->wrindex = cb->size - cb->hdma->Instance->CNDTR;
		cb->wrindex = cb->size - cb->hdma->Instance->NDTR;
  	int x = (cb->size + cb->wrindex - cb->rindex) % cb->size;
  	return x;
  }

  //	To test if the Cbuffer is full or not (fits anything more)
  int		IsFull (st_CB *cb) {
	if (cb->hdma)
		//cb->wrindex = cb->size - cb->hdma->Instance->CNDTR;
		cb->wrindex = cb->size - cb->hdma->Instance->NDTR;
  	return (cb->rindex  == (cb->wrindex + 1) % cb->size);
  }

  // sets the reader equals to the writer
  int 	Reset(st_CB *cb) {
	  if (cb->hdma)
	  	//cb->wrindex = cb->size - cb->hdma->Instance->CNDTR;
		  cb->wrindex = cb->size - cb->hdma->Instance->NDTR;
	  int stock = Stock(cb);
	  if (stock)
		  cb->rindex = cb->wrindex;
  	return stock;
  }

int		Skip	(st_CB *cb, unsigned int n){
	int z = Stock(cb);
	if (z >= n) {
		cb->rindex = (cb->rindex + n) % cb->size;
		return n;
	}
	return 0;
}

  // Reads ONE byte from the buffer...
  int		Read(st_CB *cb) {
	  int clear = 0;
	  if (cb->hdma)
	  	//cb->wrindex = cb->size - cb->hdma->Instance->CNDTR;
		  cb->wrindex = cb->size - cb->hdma->Instance->NDTR;
	  if (cb->rindex == cb->wrindex)
		  return -1;
	  else {
		  int nextr = (cb->rindex + 1) % cb->size;  // next position to be read
		  int result = cb->buffer[cb->rindex];
		  if (clear) // clear on read...
			  cb->buffer[cb->rindex] = 0;
		  cb->rindex = nextr;
		  balnew--;
		  return result;
	  }
 }



  //  Function to WRITE a char in the corresponding position
  //	If the Cbuffer is FULL, doesn´t write anything and returns -1
  //	Otherwise, writes the char , updates the writing offset and returns 1
  int		Write(st_CB *cb, uint8_t x) {
    if (cb->hdma)
	   //cb->wrindex = cb->size - cb->hdma->Instance->CNDTR;
    	cb->wrindex = cb->size - cb->hdma->Instance->NDTR;
  	if (cb->rindex == (cb->wrindex + 1) % cb->size) {
  		cb->overruns++;
  		return -1;
  	}
  	else {
  		int pre = cb->wrindex;
  		int nextw = (cb->wrindex + 1) % cb->size;  // next position to be written
  		cb->buffer[cb->wrindex] =  x;
  		cb->wrindex = nextw;
  		allwr++;
  		return (cb->size + cb->wrindex - pre) % cb->size;
  	}
  }


//  Function to MATCH the buffer with a text from the start
//  if match, returns 1, else returns 0
int	 Match	(st_CB *cb, uint8_t *pattern) {
	int x = -1;
	int l = strlen((char *)pattern);
	int s = Stock(cb);
	if (s >= l) {
		// there are enough ...
		int n1, n2;
		if (cb->rindex + l < cb->size) {
			// all are together..
			n1 = l;
			n2 = 0;
		}
		else {
			// two parts..
			n1 = cb->size - cb->rindex;
			n2 = l - n1;
		}
		if (n1) {
			x = strncmp((char *) cb->buffer + cb->rindex, (char *) pattern, n1);
		}
		else
			x = 0;
		if (n2 && !x) {
			x = strncmp((char *) cb->buffer, (char *) (pattern+n1), n2);
		}
		if (x == 0)
			return 1;
		else
			return 0;

	}
	else
		return 0;

}


//  Function to LOOKUP inside the buffer content for a text in any position
//	If the text is found returns the position,
//	else returns -1
//  @@@ So far it delegates to Match anyway
//	(the function is used in the connection phase)
int	 Lookup	(st_CB *cb, uint8_t *pattern) {
	  return Match(cb, pattern);
}



