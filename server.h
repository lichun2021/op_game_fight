#ifndef SERVER_HHHHHHHHH
#define SERVER_HHHHHHHHH
#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include <string>
#include <sstream>
#include <cstring>
#include <exception>
#include <sys/socket.h>
//#include <winsock2.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "signal.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <memory.h>
#include <sys/time.h>
#include <dirent.h>
#include <cstddef>
#include "time.h"
#include "rankhandle.h"
#include "config.h"
#include "protocol.h"
#include "rankserialization.h"

#define BUFF_MAXSIZE 2000000
#define EVENT_MAXSIZE 40
#define FD_SET_SIZE 256
#define SERV_PORT 8500
#define LISTEN 40
using namespace std;

class writeException:public exception  
{  
public:  
	writeException():exception()  
	{  
	}  
	const char* what() const throw ()
	{
		return "bad pkg";
	}
};
class msg_pkg
{
public:
	msg_pkg();
	msg_pkg(char *src,int size);
	msg_pkg(const msg_pkg& other);
	msg_pkg &operator =(const msg_pkg &other);
	unsigned int read_uint32();
	uint64_t read_uint64();
	unsigned char read_uint8();
	unsigned int size();
	unsigned int offset();
	char* read_string(unsigned int size);
	int write_uint32(unsigned int data);
	int write_uint64(uint64_t data);
	int write_uint8(unsigned char data);
	int write_string(char *data,unsigned int size);
	int seek(int offset);
	bool rebind(char *buff,int size);
private:
	char *buf;
	int len;
	int used;
};

class socketbuff
{
public:
	socketbuff();
	socketbuff(int socket);
	bool fill_inbuff(char *in,int size);
	int readpkg(msg_pkg& ret);
	void reflash_inbuff();
	void reflash_outbuff();
	msg_pkg &send_begin();
	bool send_end();

private:
	int socket;
private:
	char in_buf[2*BUFF_MAXSIZE];
	int in_len;
	int in_used;
private:
	char out_buf[BUFF_MAXSIZE];
	int out_len;
	int out_used;
private:
	msg_pkg out_msg;
};

class server
{
public:
	server(void);
	~server(void);
	bool init(void);
	bool init(const char* port, ConfigMap& config_map, const std::string& config_data_path);
	void fin(void);
	bool run(void);
	//void init_daemon();

private:
	int addBusiness (map<uint64_t, rankhandle>& _rankhandles, BlockConfig nbus);
	int removeBusiness (map<uint64_t, rankhandle>& _rankhandles, uint64_t rmv_bkey);
	bool distribute(int sock,socketbuff *the_buff);
	vector<uint64_t> analyse(string& instr);
	bool writeresult(vector<userdata>* pRet,msg_pkg& ret);
	void setnoblock(int sock);
	void readobj();
	void writeobj();
	std::string getDataDir(const short port, int index);	
private:
	int listenfd;
	int epfd;
	epoll_event ev;
	epoll_event events[EVENT_MAXSIZE];
	sockaddr_in clientaddr;
	sockaddr_in serveraddr;
	socklen_t client;
	char buf[BUFF_MAXSIZE];
	map<int,socketbuff *> _socketbuff;
	map<uint64_t,rankhandle> _rankhandles;
	time_t pretime;


};
#endif
