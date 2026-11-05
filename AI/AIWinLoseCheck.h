//
//  AIWinLoseCheck.h
//  OnePiece
//
//  Created by Beileng Shan on 4/11/14.
//
//


#ifndef __OnePiece__AIWinLoseCheck__
#define __OnePiece__AIWinLoseCheck__

namespace OnePiece {

	class AIContext;

    enum WinLoseType {
        kOnGoing = 0,
        kWin = 1,
        kLose = 2,
        kDeterminedByServer = 3,
        kDeuce = 4  // 平局(AI内部使用)
    };

    extern void CheckFuncGetClusterAliveInfo(AIContext& aiContext, int side, int clusterNum, int& outAliveCnt, int& outDeathTriggerCnt);

	class AIWinLoseCheck
	{

	public:
		static AIWinLoseCheck* getInstance();
		~AIWinLoseCheck();
		int check(AIContext& aiContext, float dt);
		void resetCluster(float *conditionList);
        void resetBattleTimeElapse();
        
        float getBattleTimeElapse() const { return timeElapseBattle; }
        float getTimeElapseConditon() const;
        float getClusterTimeElapse() const { return timeElapseCluster; }

	private:
		float *conditionList;
		float timeElapseCluster; // 单个节点
        float timeElapseBattle; // 整场战斗

		AIWinLoseCheck();
		AIWinLoseCheck(const AIWinLoseCheck&);
		AIWinLoseCheck(AIWinLoseCheck&&);
		AIWinLoseCheck& operator=(const AIWinLoseCheck&);
	};
}

#endif /* defined(__OnePiece__AIWinLoseCheck__) */
