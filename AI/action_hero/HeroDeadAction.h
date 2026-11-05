//
//  HeroDeadAction.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__HeroDeadAction__
#define __OnePiece__HeroDeadAction__

namespace OnePiece {

	class HeroDeadAction
	{
		friend class HeroActionController;
	public:
		static void Excute(CharacterData& aiCharacter, AIContext& aiContext);
		static void Enter(CharacterData&aiCharacter, AIContext& aiContext, short deadType, bool isFatal = false);
		static void EnterNative(CharacterData&aiCharacter, AIContext& aiContext, short deadType, bool isFatal = false);
		static void ForceEnter(CharacterData& aiCharacter, AIContext& aiContext);
		static void ForceEnterNative(CharacterData& aiCharacter, AIContext& aiContext);
		static void AdjustTeamPos(CharacterData& aiCharacter, AIContext& aiContext, bool isRevive);
		static void ClaimMonsterDeadReward(CharacterData& aiCharacter, AIContext& aiContext);
		static void DoHeroDead(CharacterData& aiCharacter, AIContext& aiContext);
	private:
		HeroDeadAction(const HeroDeadAction&);
		HeroDeadAction(HeroDeadAction&&);
		HeroDeadAction& operator=(const HeroDeadAction&);
	};

}

#endif /* defined(__OnePiece__HeroAttackAction__) */
