#include "rankserialization.h"


rankserialization::rankserialization(void):
fstream()
{
}

rankserialization::rankserialization(string path):
fstream()
{

}

bool rankserialization::write_open(string path)
{
	this->close();
	this->clear();
	fstream::open(path.c_str(),fstream::out|fstream::binary);
	return this->good();
}

int rankserialization::write_uint32(unsigned int data)
{
	//cout<<"write_uint32 1"<<data<<endl;
	this->write((char *)&data,4);
	//cout<<"write_uint32 2"<<endl;
	return this->tellp();
}

int rankserialization::write_uint64(uint64_t data)
{
	this->write((char *)&data,8);
	return this->tellp();
}

int rankserialization::write_uint8(unsigned char data)
{
	this->write((char *)&data,1);
	return this->tellp();
}

int rankserialization::write_string(char *data,int size)
{
	this->write((char *)data,size);
	return this->tellp();
}

bool rankserialization::read_open(string path)
{
	this->close();
	this->clear();
	fstream::open(path.c_str(),fstream::in|fstream::binary);
	return this->good();
}

unsigned int rankserialization::read_uint32()
{
	unsigned int value=0;
	this->read((char*)&value,4);
	if(this->eof())
	{
		throw readException();
	}
	return value;
}

uint64_t rankserialization::read_uint64()
{
	uint64_t value=0;
	this->read((char*)&value,8);
	if(this->eof())
	{
		throw readException();
	}
	return value;
}

unsigned char rankserialization::read_uint8()
{
	unsigned char value=0;
	this->read((char*)&value,1);
	if(this->eof())
	{
		throw readException();
	}
	return value;
}

int rankserialization::read_string(char *buff,int size)
{
	int prepos=this->tellg();
	this->read((char*)buff,size);
	if(this->eof())
	{
		throw readException();
	}
	int curpos=this->tellg();
	return curpos-prepos;
}

rankserialization::~rankserialization(void)
{
	this->close();
	this->clear();
}
