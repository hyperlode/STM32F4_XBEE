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
