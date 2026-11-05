//
//  AICannonKing.h
//  OnePiece
//
//  Created by Qing Huang on Nov/12/2015.
//
//

#ifndef __OnePiece__AICannonKing__
#define __OnePiece__AICannonKing__

#include <set>
#include <map>

namespace OnePiece {

    namespace OPFuncCannonKing {
        bool isBossWave(int passedWaveCount, int bossWaveInterval);
    }

    class AICannonKing
    {
    public:
        void onCannonExploded(int projetileUID, int endTick);

        int cannonEndTick = 0;
        int cannonRemainCount = 0;
        int lastSpawnTick = 0;
        int nextSpawnTick = 0;
        int spawnTickNormal = 0;
        int spawnTickLong = 0; // 每boss波，后一波的刷新时间
        int bossClusterNum = -1;
        int bossClusterNumInterval = 0; // boss波刷新的波数,eg:5-每5波
        std::set<int> normalWaves;
        std::map<int, char> availableWaves;
        std::set<int> bossWaves;
        std::map<int/*hero uid*/, int/*wave num*/> characterWaveNumMap;
    };

} // namespace OnePiece

#endif /* defined(__OnePiece__AICannonKing__) */
