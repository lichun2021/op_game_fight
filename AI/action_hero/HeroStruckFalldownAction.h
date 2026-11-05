
#ifndef __OnePiece__HeroStruckFalldownAction__
#define __OnePiece__HeroStruckFalldownAction__

#include "HeroOutofControlAction.h"

namespace OnePiece {

	class HeroStruckFalldownAction : public HeroOutofControlAction
	{
		friend class HeroActionController;
	public:
		static void Excute(CharacterData& aiCharacter, AIContext& aiContext);
	};

}

#endif /* defined(__OnePiece__HeroMoveAction__) */