#ifndef CONFIG_HHHHHHHHH
#define CONFIG_HHHHHHHHH
#include <map>
#include <stdint.h>
#define WINDOWS 0
#define LINUX 1
#define OSFLAG LINUX
#define HEAD_SIZE 4
#define HEAD_TYPE int
#define HEAD_TRANS ntohl
#define USER_BUFF_SIZE 255
//typedef unsigned long int uint64_t;
const int handle_count=2;
const int rankhandles[][3]={{100,200,500},{200,50,1}};//businessKey,每块内玩家数量，多少块
struct BlockConfig
{
	BlockConfig():businessKey(0), perblock(0), totalblock(0){}
        BlockConfig(uint64_t bkey, int perb, int totalb):businessKey(bkey), perblock(perb), totalblock(totalb){}

	uint64_t businessKey;
	int perblock;
	int totalblock;
};
typedef std::map<int, BlockConfig> ConfigMap;
#endif
