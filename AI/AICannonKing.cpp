//
//  AICannonKing.cpp
//  OnePiece
//
//  Created by Qing Huang on Nov/12/2015.
//
//

#include "AICannonKing.h"

namespace OnePiece {
    namespace OPFuncCannonKing {
        bool isBossWave(int passedWaveCount, int bossWaveInterval)
        {
            return (passedWaveCount > 0 && bossWaveInterval > 0 && (passedWaveCount % bossWaveInterval) == 0);
        }
    }

    void AICannonKing::onCannonExploded(int projetileUID, int endTick)
    {
        if (cannonEndTick < endTick)
            cannonEndTick = endTick;
    }

} // namespace OnePiece
