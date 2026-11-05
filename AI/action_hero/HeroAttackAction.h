//
//  HeroAttackAction.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__HeroAttackAction__
#define __OnePiece__HeroAttackAction__

#include "../AIContext.h"
#include "HeroActionController.h"

namespace OnePiece {

	class HeroAttackAction
	{
		friend class HeroActionController;
		friend class HeroSkillController;

	private:
		static void Excute(CharacterData& aiCharacter, AIContext& aiContext);
	public:
		static void Enter(CharacterData&aiCharacter, AIContext& aiContext, int skillSeq, int skillID, int skillLevel, bool resetCD, unsigned short tickOffset, bool isUltraSkill);
		static void EnterNative(CharacterData&aiCharacter, AIContext& aiContext, int skillSeq, int skillID, int skillLevel, bool resetCD, unsigned short tickOffset, bool isUltraSkill);
        static void Exit(CharacterData&aiCharacter, AIContext& aiContext);
        static void ExitNative(CharacterData&aiCharacter, AIContext& aiContext);
		
		static void UpdateNative(CharacterData& aiCharacter, AIContext& aiContext);

        // 结束蓄力
        static void ReleaseAccumulate(CharacterData& aiCharacter, AIContext& aiContext);

        // 强制结束蓄力: true - 打断了蓄力,保持攻击状态, false - 没有蓄力,不保持攻击状态
        static bool ForceEndAccumulate(CharacterData& aiCharacter, AIContext& aiContext);
        static bool ForceEndAccumulateNative(CharacterData& aiCharacter, AIContext& aiContext);

		// clair_wang
		static void ForceEnter(CharacterData&aiCharacter, AIContext& aiContext, int skillSeq, int skillID, int skillLevel, bool resetCD, unsigned short tickOffset, bool isUltraSkill);
		static void ForceEnterNative(CharacterData&aiCharacter, AIContext& aiContext, int skillSeq, int skillID, int skillLevel, bool resetCD, unsigned short tickOffset, bool isUltraSkill);

		static bool TriggerComboNative(CharacterData&aiCharacter, AIContext& aiContext, int skillID, int skillLevel);

    private:
        static void ImplEnter(CharacterData& aiCharacter, AIContext& aiContext, int skillSeq, int skillID, int skillLevel, bool resetCD, int tickOffset, bool isUltraSkill, bool isForceEnter);
	};

}

#endif /* defined(__OnePiece__HeroAttackAction__) */
