//
//  AINode.h
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#ifndef __OnePiece__AINode__
#define __OnePiece__AINode__

#include <vector>
#include <list>
#include "CommonType.h"
#include "cocos2d.h"

namespace OnePiece {
	class CharacterData;
	class AIContext;

	class AINode : public cocos2d::Ref
	{
	public:
		virtual ~AINode();
		AINode(int frozenCompareID, unsigned short clusterN);

		/*
		int totalTick;//from 0
		int tickCurrent;//from 0
		int startTick;//external tick num of context
		*/
		unsigned short clusterNum;
		int myID;
		int frozenCompareID;
		bool isFrozen;
        bool ignoreFreeze = false;

		int totalTick = 0;//from 0
		int localTick = 0;
		int startTick = 0;//tick num based on former localTick
		int freezeTick = 0;
		
	public:
		virtual void update(AIContext& aiContext);

	private:
		//AINode(const AINode&);
		//AINode(AINode&&);
		//AINode& operator=(const AINode&);
	};

}

#endif /* defined(__OnePiece__AINode__) */
