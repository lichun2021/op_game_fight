#include "testverify.h"
#include <iostream>
#include <random>
#include "json/rapidjson.h"
#include "json/filestream.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"
#include "ServerVerifyHelper.h"

namespace OnePiece {

    int test_verify(int verify_type, const char *verify_input, bool is_use_alt_seed, int loop_cnt)
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

    int test_verify_batch(const char *input_file_name)
    {
        FILE *file = fopen(input_file_name, "r");
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
        
        return 0;
    }

}
