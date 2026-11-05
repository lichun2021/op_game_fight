//
//  AIContinuousMoveCaster.h
//  OnePiece
//
//  Created by Beileng Shan on 14/5/4.
//
//

#ifndef __OnePiece__AIContinuousMoveCaster__
#define __OnePiece__AIContinuousMoveCaster__

#include <vector>
#include <list>
#include "CommonType.h"
#include "AINode.h"

namespace OnePiece {
	class CharacterData;
	class AIContext;

	class AIContinuousMoveCaster : public AINode
	{
    public:
        enum class MoveType {
            kNormalHasTarget = 0,  // 普通有目标模式
            kNormalNoTarget = 1,   // 普通无目标模式
			kCrossBattleField = 2,  // 穿墙模式
			kBlackHoleField, // = 3  黑洞模式  (将所有目标拉向一个坐标点位置，该坐标点位置相对于施法者（即施法者位置算0,0点），且一旦释放不随施法者转向而变化)

			kSpecialHasTarget,// = 4 特殊有目标模式（将移动执行moveCasts加在target身上,其它默认加在source身上）
			kSpecialNoTarget,// = 5 特殊无目标模式（将移动执行moveCasts加在target身上,其它默认加在source身上）

			kRandomPointField,// = 6 朝随机地点移动，作用范围填移动随机的范围，每次随机移动时注意人物的面朝向

			kTrackTargetMove, // = 7 追踪目标移动，直到目标停止，移动执行结束。
			 
			kCrossBattlefieldNew, // = 8  类似穿墙，但不一定回到原位，而是根据该类型的持续时间和移动速度决定终点位置。

			KAbsolutePostionMove, // = 9, 令目标移动到绝对坐标位置。

			kSpecialSummonBackMove, // = 10 召唤物以一定速度移动到召唤者身边（将移动执行moveCasts加在target身上,其它默认加在source身上）
        };

        struct MoverInfo {
            CharacterData* mover = nullptr;
            CharacterData* moversTarget = nullptr;
            int initMoverX = 0;
            int initMoverY = 0;
            int initTargetX = 0;
            int initTargetY = 0;
            int tickAtReturnPointX = 0;
            int initFaceDirectionX = 0;
            int initFaceDirectionY = 0;
			int continualFlutterTick = 0;
			int tickAtReturnPointY = 0;
        };

	public:
        AIContext& aiContext;
		int moveID;
        int skillID;
		unsigned short skillLevel;
		MoveExec* staticData;
		CharacterData* origin;
		CharacterData* target;
        MoveType moveType;

		int maxMoveCnt;
		int BlackHoleAngleA;
        bool shouldDeleteSelf = false;

		int speedTickParam = 100;

		int realDuration = 0;
		int realDurationX = 0;
		int realDurationY = 0;
		int realReturnPause = 0;
		int realActionFreq = 0;
		int realSpeedX = 0;
		int realSpeedY = 0;
		int realAccX = 0; 
		int realAccY = 0;

	public:
		AIContinuousMoveCaster(int moveID, int skillID, unsigned short skillLevel, CharacterData* origin, CharacterData* target,AIContext& aiContext);
		~AIContinuousMoveCaster();
        void initCaster(AIContext& aiContext);
		void Excute(AIContext& aiContext);

		bool shouldKeepCaster();

	private:
        void exertMove(MoverInfo& mover, int playedTick);
        std::unordered_map<int, MoverInfo*> movers;
		void TodoAfterEndFun(CharacterData* theMover = nullptr);
		// 在移动过程中持续执行，主要是动画执行。
		void TodoMovingExecFunc(int thePlayTick, CharacterData* theMover = nullptr);
		void OldInitCaster(AIContext& aiContext);
		void NewInitCaster(AIContext& aiContext);
		void ToArrangeMoveTypeData();

    private:
        void updateNormalNoTarget(AIContinuousMoveCaster::MoverInfo& mover, int playedTick);
        void updateNormalHasTarget(AIContinuousMoveCaster::MoverInfo& mover, int playedTick);
        void updateCrossBattleFiled(AIContinuousMoveCaster::MoverInfo& mover, int playedTick);
		void updateBlackHoleField(AIContinuousMoveCaster::MoverInfo& mover, int playedTick);
		void updateRandomPointField(AIContinuousMoveCaster::MoverInfo& mover, int playedTick);
		void updateTrackTargetField(AIContinuousMoveCaster::MoverInfo& mover, int playedTick);
		void updateCrossBattleFieldNew(AIContinuousMoveCaster::MoverInfo& mover, int playedTick);
		void updateAbsolutePositionField(AIContinuousMoveCaster::MoverInfo& mover, int playedTick);
		void updateSpecialSummonBackMove(AIContinuousMoveCaster::MoverInfo& mover, int playedTick);
	};

}

#endif /* defined(__OnePiece__AIContinuousMoveCaster__) */
