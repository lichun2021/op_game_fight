#ifndef __OnePiece__BattleLog__
#define __OnePiece__BattleLog__

#include <fstream>
#include <memory>
#include <string>

#define BATTLE_LOG_TEST 0

#if BATTLE_LOG_TEST

typedef std::shared_ptr<std::ofstream> OFStreamPtr;

void OPBattleLogInit();
void OPBattleLogInitWithPath(const std::string& logPath);
void OPBattleLogReset();

void setBattleLogFileStream(const OFStreamPtr& ofs);
OFStreamPtr& getBattleLogFileStream();

void OPBattleLog();

template<typename _T>
void OPBattleLog(_T log) {
    auto& pStream = getBattleLogFileStream();
    if (pStream)
        *pStream << log << std::endl;
}

template<typename Head, typename... Tail>
void OPBattleLog(const Head& h, const Tail&... tail) {
    auto& pStream = getBattleLogFileStream();
    if (pStream) {
        *pStream << h << ",";
        OPBattleLog(tail...);
    }
}

#else
#define OPBattleLog(...) do {} while (0)
#endif // BATTLE_LOG_TEST

#endif /* defined(__OnePiece__BattleLog__) */
