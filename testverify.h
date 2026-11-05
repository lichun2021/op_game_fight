#ifndef __testverify_h__
#define __testverify_h__

namespace OnePiece {
    enum TestVerifyType {
        FIGHT_GENERIC = 0,
        FIGHT_ALLIANCE_PVE_NEW, //同盟副本
        FIGHT_WHEELWAR_OR_TOWERDEFENCE, //回忆之战车轮战或守波数
        FIGHT_CARWHEEL, //珍宝巡航
        FIGHT_SHIPKING, //船炮之王
    };

    extern int test_verify(int verify_type, const char *verify_input, bool is_use_alt_seed, int loop_cnt);
    extern int test_verify_batch(const char *input_file_name);
}

#endif