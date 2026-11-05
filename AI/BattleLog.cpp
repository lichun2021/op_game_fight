#include "BattleLog.h"

#if BATTLE_LOG_TEST

void OPBattleLogInit() {
    OPBattleLogInitWithPath("./data/log/fs_battle.log");
}

void OPBattleLogInitWithPath(const std::string& logPath) {
    std::remove(logPath.c_str());
    OFStreamPtr outfream_ai = OFStreamPtr(new std::ofstream);
    outfream_ai->open(logPath, std::ios::out | std::ios::app | std::ios::ate);
    setBattleLogFileStream(outfream_ai);
}

void OPBattleLogReset() {
    getBattleLogFileStream().reset();
}

void setBattleLogFileStream(const OFStreamPtr& ofs) {
    auto& s_ptr = getBattleLogFileStream();
    s_ptr = ofs;
}

OFStreamPtr& getBattleLogFileStream() {
    static OFStreamPtr ptr = nullptr;
    return ptr;
}

void OPBattleLog() {
    auto& pStream = getBattleLogFileStream();
    if (pStream)
        *pStream << std::endl;
}

#endif
