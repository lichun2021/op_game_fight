// rankserver.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include <iostream>
#include <string>
#include <random>
#include <stdlib.h>
#include "server.h"
#include "config.h"
#include "ServerVerifyHelper.h"
#include "AI/AIContext.h"
#include "AI/AIPlatformHelper.hpp"
#include "json/rapidjson.h"
#include "json/filestream.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"

using namespace std;
int Dir_index=1;

//#define TEST_VERIFY

static void test_serverinput(int fightType)
{
	std::cout << "test server input\n";
    std::ifstream input;
    std::string inputName = OnePiece::AIPlatformHelper::getExecutablePath() + "/../test/data/server_input.txt";
    input.open(inputName.c_str(), std::ios::in);
    std::string verifyJson((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

	int fightCount = 1;
	// int fightType = 0;
    // if (argc > 1) {
    //     std::string fightCountStr(argv[1]);
    //     fightType = std::stoi(fightCountStr);
    //     if (fightType < 0)
	// 		fightType = 0;
    // }

    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<int> distribution;

    bool useTestSeed = false;
    for (int i = 0; i < fightCount; i++) {
        int dice_roll = distribution(generator);
		std::cout << "start" << i << std::endl;
		if (fightType == 0) {
			auto winlose = fight(verifyJson.c_str(), useTestSeed, dice_roll);
			std::cout << "end normal fight" << i << winlose.first << std::endl;
		} else if (fightType == 1) {
			auto winlose = fightAlliancePveNew(verifyJson.c_str());
			std::cout << "end fightAlliancePveNew" << i << winlose.first << std::endl;
		} else if (fightType == 2) {
			auto winlose = fightWWTD(verifyJson);
			std::cout << "end fightWWTD" << i << winlose.first << std::endl;
		} else if (fightType == 3) {
			auto winlose = fightCarWheel(verifyJson);
			std::cout << "end fightCarWheel" << i << winlose.first << std::endl;
		} else if (fightType == 4) {
			auto score = fightShipKingPveVerify(verifyJson);
			std::cout << "end fightShipKingPveVerify" << i << "," << score << std::endl;
		}
        
        //OnePiece::AIContext::getInstance()->clear();
		// std::cout << "end" << i << winlose.first << std::endl;
		std::cout << "finish" << std::endl;
	}
}

#ifndef TEST_VERIFY
int main(int argc, char* argv[])
{
	// if (argc < 3)
	// {
	// 	if (argc == 1)
	// 	{
	// 		test_serverinput(0);
	// 		return 0;
	// 	}
	// 	cout<<"argument is: name dir_index port businessKey1 perBlock1 totalBlock1 ..."<<endl;
	// 	exit(0);
	// }

	// if (*(argv[1]) == '1')
	// {
	// 	Dir_index = 1;
	// }
	// else
	// {
	// 	Dir_index = 2;
	// }

	// if((argc % 3) != 0)
	// {
	// 	cout<<"argument is: name dir_index port businessKey1 perBlock1 totalBlock1 ..."<<endl;
	// }
	// else
	// {
	// 	ConfigMap config_map;
	// 	int arg_index = 2;
	// 	while (arg_index < (argc-1) )
	// 	{
	// 		static int key = 0;
	// 		BlockConfig config;
	// 		config.businessKey = atol(argv[++arg_index]);
	// 		config.perblock = atoi(argv[++arg_index]);
	// 		config.totalblock = atoi(argv[++arg_index]);
	// 		config_map.insert(std::pair<int, BlockConfig>(key, config));
	// 		++key;
	// 	}

	// 	server myserver;
	// 	myserver.init(argv[2], config_map);
	// 	bool result=myserver.run();
	// 	myserver.fin();
	// 	cout<<"result:"<<result<<endl;
	// }

	ConfigMap config_map;
	std::string port_str = "8400";
	std::string configdata_path;

	for (int i = 0; i < argc; i++) 
	{
		std::string arg = argv[i];
		if (arg == "-test" || arg == "-t")
		{
			int test_bkey = 0;
			if (i + 1 < argc)
				test_bkey = atoi(argv[i+1]);
			test_serverinput(test_bkey);
			break;
		}
		else if (i == 1)
		{
			if (arg == "1")
				Dir_index = 1;
			else
				Dir_index = 2;
		}
		else if (i == 2)
		{
			port_str = argv[i];
		}
		else if (arg == "-extra" || arg == "-e")
		{
			int extra_args = argc - i - 1;
			if(extra_args > 0 && (extra_args % 3) == 0)
			{
				int arg_index = i + 1;
				while (arg_index < (argc-1))
				{
					static int key = 0;
					BlockConfig config;
					config.businessKey = atol(argv[++arg_index]);
					config.perblock = atoi(argv[++arg_index]);
					config.totalblock = atoi(argv[++arg_index]);
					config_map.insert(std::pair<int, BlockConfig>(key, config));
					++key;
				}
			}
		}
		else if (arg == "-c" || arg == "-configdata")
		{
			if (i + 1 < argc)
			{
				configdata_path = argv[i+1];
				if (configdata_path.length() > 0)
				{
					auto last_pos = configdata_path.length() - 1;
					auto last_char = configdata_path[last_pos];
					if (last_char != '/')
						configdata_path.push_back('/');
				}
				i++;
			}
		}
	}

	server myserver;
	myserver.init(port_str.c_str(), config_map, configdata_path);
	bool result=myserver.run();
	myserver.fin();
	cout<<"result:"<<result<<endl;

	return 0;
}
#else

enum TestVerifyType {
	FIGHT_GENERIC = 0,
	FIGHT_ALLIANCE_PVE_NEW, //同盟副本
	FIGHT_WHEELWAR_OR_TOWERDEFENCE, //回忆之战车轮战或守波数
	FIGHT_CARWHEEL, //珍宝巡航
	FIGHT_SHIPKING, //船炮之王
};

static int test_verify(int verify_type, const char *verify_input, bool is_use_alt_seed, int loop_cnt)
{
	(void)loop_cnt;
	int alt_seed = 0;
	
	std::cout << "start\n";

	if (verify_type == FIGHT_GENERIC) {
		if (is_use_alt_seed) {
			std::random_device rd;
			std::default_random_engine generator(rd());
			std::uniform_int_distribution<int> distribution(1, 100000);
			alt_seed = distribution(generator);
		}
		auto winlose = fight(verify_input, is_use_alt_seed, alt_seed);
		return winlose.first;
	} else if (verify_type == FIGHT_ALLIANCE_PVE_NEW) {
		auto winlose = fightAlliancePveNew(verify_input);
		return winlose.first;
	} else if (verify_type == FIGHT_WHEELWAR_OR_TOWERDEFENCE) {
		auto winlose = fightWWTD(verify_input);
		return winlose.first;
	} else if (verify_type == FIGHT_CARWHEEL) {
		auto winlose = fightCarWheel(verify_input);
		return winlose.first;
	} else if (verify_type == FIGHT_SHIPKING) {
		auto score = fightShipKingPveVerify(verify_input);
		return score;
	}
	
	std::cout << "end\n";

	return 0;
}

static void test_qiwuhai()
{
	std::string input_file_name = OnePiece::AIPlatformHelper::getExecutablePath() + "/../test/data/qiwuhai.log";
	FILE *file = fopen(input_file_name.c_str(), "r");
	rapidjson::FileStream f(file);
	rapidjson::Document *doc = new rapidjson::Document();
	doc->ParseStream<0>(f);
	fclose(file);
	
	auto logcnt = doc->Size();
	for (int i = 0; i < logcnt; ++i) {
		auto &a_battle_log = (*doc)[i];
		rapidjson::StringBuffer buf;
		buf.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
		
		a_battle_log.Accept(writer);
		
		std::cout << "log:\n" << buf.GetString() << "\n";
		test_verify(FIGHT_GENERIC, buf.GetString(), true, 0);
	}
	
	delete doc;
}

int main(int argc, const char * argv[]) 
{
	std::cout << "test verify\n";

	if (argc > 1) {
		std::string verify_type_str(argv[1]);
		int verify_type = std::stoi(verify_type_str);
		if (verify_type == 1) {
			while (true)
				test_qiwuhai();
		}
		else 
			test_serverinput(0);
	}
	else
		test_serverinput(0);
    //std::cin.ignore();
    return 0;
}
#endif

