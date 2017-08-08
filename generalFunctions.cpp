#include "generalFunctions.h"

bool generalFunctions::stringsAreEqual(char* A, char*B){
	//check if two strings, char arrays terminated with a '/0' are equal



//	bool test;
	//char text [2] = {'b','\0'};
	//char lode [2] = {'a','\0'};

//	test = stringsAreEqual(text, lode);
//	printf ("testresult: %d",test);


	uint32_t i = 0;
	while (A[i] == B[i]){
		if (A[i] == '\0'){
			return true;
		}
		i++;
	}
	return false;
}


bool generalFunctions::stringsAreEqual(char* A, char*B,uint16_t length){
	//check if two string are equal for the defined length
	bool equal = true;
	for (uint16_t i = 0; i<length;i++){
		if (A[i] != B[i]){
			equal = false;
		}
	}
	return equal;
}

bool generalFunctions::byteArraysAreEqual(uint8_t* A, uint8_t *B,uint16_t length){
	//check if two string are equal for the defined length
	bool equal = true;
	for (uint16_t i = 0; i<length;i++){
		if (A[i] != B[i]){
			equal = false;
		}
	}
	return equal;
}

void generalFunctions::copyByteArray(uint8_t* original, uint8_t* copy,uint16_t length){
	//check if two string are equal for the defined length
	for (uint16_t i = 0; i<length;i++){
		copy[i] = original[i];
	}
}
