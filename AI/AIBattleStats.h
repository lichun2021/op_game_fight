//
//  AIBattleStats.h
//  op_game
//
//  Created by h7ing on 8/10/15.
//
//

#ifndef __op_game__AIBattleStats__
#define __op_game__AIBattleStats__

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "AICharacterData.h"
#include "HeroHurtcalculate.h"

namespace OnePiece {

    class CharacterData;

    typedef int CharacterUID;
    typedef int CharacterID;

    struct AIBattleStats;
    typedef std::shared_ptr<AIBattleStats> AIBattleStatsPtr;
    typedef std::unordered_map<CharacterUID, AIBattleStatsPtr> StatsContainer;

    struct AIBattleStats {
        CharacterID heroID = 0;
        TeamSide teamSide = TeamSide::kInvalid;
        int clusterNum = 0;

        int64_t totalHurt = 0;
        int64_t totalHeal = 0;
    };

    namespace StatsFunc {

        StatsContainer& GetAllStatsSingletonInstance();
        void ResetStats();
        void AppendHeroHurt(CharacterData& hero, HurtType hurtType, int value, CharacterData& target);
    } // namespace StatsFunc

} // namespace OnePiece

#endif /* defined(__op_game__AIBattleStats__) */
