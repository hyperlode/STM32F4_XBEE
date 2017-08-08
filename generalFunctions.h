
#ifndef GENERALFUNCTIONS_H
#define GENERALFUNCTIONS_H

#include <stdint.h>
#include <stdio.h>
/**/

class generalFunctions{
public:
	static bool stringsAreEqual(char* A, char*B);
	static bool stringsAreEqual(char* A, char*B,uint16_t length);
	static bool byteArraysAreEqual(uint8_t* A, uint8_t *B,uint16_t length);
	static void copyByteArray(uint8_t* original, uint8_t * copy,uint16_t length);
};

/**/
#endif
