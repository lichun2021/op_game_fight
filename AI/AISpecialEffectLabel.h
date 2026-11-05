//
//  AISpecialEffectLabel.h
//  OnePiece
//
//  Created by Beileng Shan on 14/5/4.
//
//

#ifndef __OnePiece__AISpecialEffectLabel__
#define __OnePiece__AISpecialEffectLabel__

#include <vector>
#include <list>
#include "CommonType.h"

namespace OnePiece {
	class CharacterData;
	class AIContext;

    enum SpecialHitLabel {
        MOVEUP_LABEL_CONFIG = 1, // 上升后消失文字配置
        SCALE_LABEL_CONFIG = 2, // 缩放后消失文字配置
        NORMAL_HURT_LABEL = 100000, // 普通伤害
        NORMAL_HEAL_LABEL = 100002, // 普通治疗
        MAG_IMMUNIZE_LABEL = 100005, // 法术免疫
        ABSORB_HURT_LABEL = 100008, // 伤害吸收
        PHY_IMMUNIZE_LABEL = 100010, // 物理免疫
        ENERGY_CHANGE_LABEL = 100031, // 能量变化
        MAX_HP_INCREASE = 105023, // 血量上限增加
        MAX_HP_DECREASE = 105024 // 血量上限减少
    };

	class AISpecialEffectLabel
	{

	public:
		//static void excute(int labelID, int num, AIContext& aiContext, CharacterData* target);
        // 飘子传参调整 cui 2023_2_27
        static void excute(int labelID, int num, AIContext& aiContext, CharacterData* target,bool isCrit = false);

        static bool checkShowCrit(int hitId);



    private:
		//AISpecialEffectLabel(const AISpecialEffectLabel&);
		//AISpecialEffectLabel(AISpecialEffectLabel&&);
		//AISpecialEffectLabel& operator=(const AISpecialEffectLabel&);
	};

}

#endif /* defined(__OnePiece__AISpecialEffectLabel__) */
