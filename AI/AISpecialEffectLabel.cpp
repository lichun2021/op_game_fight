//
//  AISpecialEffectLabel.cpp
//  OnePiece
//
//  Created by Beileng Shan on 14/5/4.
//
//

#include "AISpecialEffectLabel.h"
#include "AIContext.h"

namespace OnePiece{
    // 飘子传参调整 cui 2023_2_27
    void AISpecialEffectLabel::excute(int labelID, int num, AIContext& aiContext, CharacterData* target, bool isCrit)
    {

#ifdef SERVER_VERIFYING
        return;
#endif
		LabelEffectExec* staticData = AIDataHelper::getLabelEffectExec(labelID);
		if (staticData==nullptr) return;

		//TODO check
		//if (staticData->onlyOneSameType > 0 && target->labelEffects.find(labelID) != target->labelEffects.end()) return;

        //	target->labelEffects.push_back(std::pair<int, int>(labelID, num));
        // 飘子传参调整 cui 2023_2_27batt
        auto labels = &target->specialEffectLabels[LabelEffectExec::LabelActionType::SCALE_OUT];
        if (staticData->upSpeed > 0)
        {
            //target->specialEffectLabels[LabelEffectExec::LabelActionType::MOVEUP_OUT].push_back(std::tuple<int, int,bool,long, long,bool>(labelID, num, isCrit, aiContext.currentTick,staticData->mergeTick,false));
            labels = &target->specialEffectLabels[LabelEffectExec::LabelActionType::MOVEUP_OUT];
        }
        //else
        //{
            //target->specialEffectLabels[LabelEffectExec::LabelActionType::SCALE_OUT].push_back(std::tuple<int, int, bool,long,long, bool>(labelID, num, isCrit, aiContext.currentTick,staticData->mergeTick,false));
        //}

        /*(*labels).push_back(std::tuple<int, int, bool, long, long, bool>(labelID, num, isCrit, aiContext.currentTick, staticData->mergeTick, false));
        if (1) {
            return;
        }*/

        //id 0， 伤害值 1 ，是否暴击 2，插入的时间 3，可合并时间 4，是否执行过 5 
        if (!staticData->canMerge) {
            (*labels).push_back(std::tuple<int, int, bool, long, long, bool>(labelID, num, isCrit, aiContext.currentTick, staticData->mergeTick, false));
            return;
        }
        bool canInsert = true;
        for (auto iter = (*labels).begin(); iter != (*labels).end(); iter++) {
            if ((std::get<0>(*iter) == labelID) && (aiContext.currentTick - std::get<3>(*iter) < staticData->mergeTick)) {
                canInsert = false;
                break;
            }
        }
        if (canInsert) {
            (*labels).push_back(std::tuple<int, int, bool, long, long, bool>(labelID, num, isCrit, aiContext.currentTick, staticData->mergeTick, false));
        }

	}



     bool AISpecialEffectLabel::checkShowCrit(int hitId)
    {
        #ifdef SERVER_VERIFYING
                return false;
        #endif
                return (hitId == 100001 || hitId == 100003);
    }


}
