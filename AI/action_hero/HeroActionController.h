//
//  HeroActionController.h
//  OnePiece
//
//  Created by Beileng Shan on 3/29/14.
//
//

#ifndef __OnePiece__HeroActionController__
#define __OnePiece__HeroActionController__

#include <cstdint>
#include "../AIContext.h"
#include "../AIActionController.h"

namespace OnePiece {
	
	class HeroActionController : public AIActionController
	{
	public:
		static HeroActionController* getInstance();
		virtual void update(CharacterData& aiCharacter, AIContext& aiContext);
		static void actionSelector(CharacterData& aiCharacter, AIContext& aiContext);
		static void actionSelectorNative(CharacterData& aiCharacter, AIContext& aiContext);
		static void captainActionSelector(CharacterData& aiCharacter, AIContext& aiContext);
		static void captainActionSelectorNative(CharacterData& aiCharacter, AIContext& aiContext);
		static void cleanup(CharacterData& aiCharacter, AIContext& aiContext, bool ifHurtEffect=false, bool isCleanTransform = true);
        static void safeCleanupCasters(CharacterData& aiCharacter, AIContext& aiContext); // 12.0
        static void exitAction(CharacterData& aiCharacter, AIContext& aiContext);
        static void didEnterAction(CharacterData& aiCharacter, AIContext& aiContext);
        static void exitActionNative(CharacterData& aiCharacter, AIContext& aiContext);
        static void didEnterActionNative(CharacterData& aiCharacter, AIContext& aiContext);
		~HeroActionController();

		STATIC_SETGET_ATT(getSkillSeq, setSkillSeq, 0);
		STATIC_SETGET_ATT(getSkillID, setSkillID, 1);
		STATIC_SETGET_ATT(getSkillLv, setSkillLv, 2);
		STATIC_SETGET_ATT(getOldTotalTick, setOldTotalTick, 3);
		STATIC_SETGET_ATT(getOldTick, setOldTick, 4);
		STATIC_SETGET_ATT(getOldAction, setOldAction, 5);
		STATIC_SETGET_ATT(getOldSkillID, setOldSkillID, 6);
        STATIC_SETGET_ATT(getNextSkillID, setNextSkillID, 7);
		STATIC_SETGET_ATT(getYingzhi, setYingzhi, 9);
		STATIC_SETGET_ATT(getSkillActionCD, setSkillActionCD, 10);
		STATIC_SETGET_ATT(getFlySpeedX, setFlySpeedX, 11);
		STATIC_SETGET_ATT(getFlySpeedY, setFlySpeedY, 12);
		STATIC_SETGET_ATT(getEffectID, setEffectID, 13);
        STATIC_SETGET_ATT(getAccumulateTick, setAccumulateTick, 14);
        STATIC_SETGET_ATT(getAccuAttackRepeat, setAccuAttackRepeat, 15);

		static void setCurrentTarget(CharacterData& aiCharacter, CharacterData* target);
		static int distanceToCharacter(CharacterData* chara1, CharacterData* chara2);
        // max dist: 65536
        //static uint32_t distanceSquare(CharacterData* chara1, CharacterData* chara2);

        static bool isDeadAction(ActionType action);
        static bool isDeadAttacking(CharacterData *aiCharacter);

        static bool releaseAccumulateSkill(CharacterData& aiCharacter, AIContext& aiContext);
        static void releaseSkillOnBegin(AIContext& aiContext, CharacterData *c, bool isProtective, int lazyTick);
        static void teamReleaseSkillOnBegin(AIContext& aiContext, CharacterDataListPtr pCharacterList, bool isProtective, int lazyTick);
		
	private:
		HeroActionController();
		HeroActionController(const HeroActionController&);
		//HeroActionController(HeroActionController&);
		HeroActionController& operator=(const HeroActionController&);
	};

}

#endif /* defined(__OnePiece__HeroActionController__) */
