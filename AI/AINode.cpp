//
//  AINode.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "AINode.h"
#include "AIContext.h"

namespace OnePiece{

	AINode::AINode(int frozenCompareID, unsigned short clusterN)
	{
		this->frozenCompareID = frozenCompareID;
		clusterNum = clusterN;
		isFrozen = false;
		//int totalTick = 0;//from 0
		//localTick = 0;
		//startTick = 0;//tick num based on former localTick
	}

	AINode::~AINode()
	{
	}

	void AINode::update(AIContext& aiContext)
	{
		if (localTick == 0)
		{
			localTick = aiContext.currentTick;
			startTick = localTick + 1;
		}

        if (aiContext.ultraFreezeList.size() > 0 && !ignoreFreeze)
		{
			std::list<int*>::iterator thefirst = aiContext.ultraFreezeList.begin();
			for (; thefirst != aiContext.ultraFreezeList.end(); thefirst++)
			{
				if ((*thefirst)[0] >0) break;
			}
			if (thefirst != aiContext.ultraFreezeList.end())
			{
				isFrozen = true;
				//OpCountdown, EffCountdown, myID, myCluster, startGlobalTick, freezetime
				for (std::list<int*>::iterator iter = aiContext.ultraFreezeList.begin(); iter != aiContext.ultraFreezeList.end(); iter++)
				{
					if ((*thefirst)[2] == frozenCompareID
						|| ( (*iter)[2] == frozenCompareID && (*iter)[4] - (*thefirst)[4]<(*iter)[5]) )
					{
						isFrozen = false;
						break;
					}
				}
				if (isFrozen) {
					freezeTick = localTick+1;
					return;
				}
			}
		}

		isFrozen = false;
		localTick++;
	}

}