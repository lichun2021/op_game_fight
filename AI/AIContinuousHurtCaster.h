//
//  AIContinuousHurtCaster.h
//  OnePiece
//
//  Created by Beileng Shan on 14/5/4.
//
//

#ifndef __OnePiece__AIContinuousHurtCaster__
#define __OnePiece__AIContinuousHurtCaster__

#include <vector>
#include <list>
#include "HeroHurtcalculate.h"
#include "CommonType.h"
#include "AINode.h"

namespace OnePiece {
	class CharacterData;
	class AIContext;

	class AIContinuousHurtCaster : public AINode
	{
	public:
		int hurtID;
        int skillID;
		int skillLevel;
        int markID;
		HurtExec* staticData;
		CharacterData* origin;
		CharacterData* target;
		AIProjectile* projectile;
		bool firstRound;// clair_wang
		int mStationaryDrt;// clair_wang 记录定点范围内查找目标时，施法者Direction （有效值为-1和1， 0为无效值）
		int mStationaryPosX;// clair_wang 记录定点范围内查找目标时，施法者X
		int mStationaryPosY;// clair_wang 记录定点范围内查找目标时，施法者Y

		CharacterData* pFirstRoundConfirmTarget;// 技能列表查找目标类型：26 随机一个敌人，并且若是持续伤害，每次伤害不再重新随机目标。
        bool isValid = true;
		bool shouldDeleteSelf = false;

	public:
		AIContinuousHurtCaster(int hurtID, int skillID, int skillLevel, CharacterData* origin, AIProjectile* projectile, CharacterData* target, AIContext& aiContext, int theMarkID);
		~AIContinuousHurtCaster();
		void ExcuteOld(AIContext& aiContext);
        void ExcuteNew(AIContext &aiContext);
        void Excute(AIContext &aiContext);
        static std::shared_ptr<std::list<CharacterData*> > SimpleSearchHurtCasterTarget(AIContext& aiContext, int hurtID, CharacterData *target, CharacterData *origin, int skillID, int skillLevel, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter);
        static void SimpleSearchHurtCasterTargetNew(AIContext& aiContext, int hurtID, CharacterData *target, CharacterData *origin, int skillID, int skillLevel, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter, std::list<CharacterData *> *outlist);
        void onFree();
        
        static void addRemainCaster(AIContinuousHurtCaster *aCaster);
        static void cleanRemainCaster();

	private:
		//AIContinuousProjectileCaster(const AIContinuousProjectileCaster&);
		//AIContinuousProjectileCaster(AIContinuousProjectileCaster&&);
		//AIContinuousProjectileCaster& operator=(const AIContinuousProjectileCaster&);
		bool m_bContinueHurt;
	};

}

#endif /* defined(__OnePiece__AIContinuousProjectileCaster__) */
