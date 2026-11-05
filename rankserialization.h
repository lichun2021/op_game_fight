#ifndef RANKSERIALIZATION_HHHHHHHHH
#define RANKSERIALIZATION_HHHHHHHHH
#include <fstream>
#include <string>
#include <iostream>
#include <stdint.h>
using namespace std;
class readException:public exception  
{  
public:  
	readException():exception()  
	{  
	}  
	const char* what() const throw ()
	{
		return "bad pkg";
	}
}; 
class rankserialization:public fstream
{
public:
	rankserialization(void);
	rankserialization(string path);
	~rankserialization(void);
	bool write_open(string path);
	int write_uint32(unsigned int data);
	int write_uint64(uint64_t data);
	int write_uint8(unsigned char data);
	int write_string(char *data,int size);
	bool read_open(string path);
	unsigned int read_uint32();
	uint64_t read_uint64();
	unsigned char read_uint8();
	int read_string(char *buff,int size);
};

#endif
