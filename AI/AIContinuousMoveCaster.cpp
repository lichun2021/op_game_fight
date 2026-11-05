//
//  AIContinuousMoveCaster.cpp
//  OnePiece
//
//  Created by Beileng Shan on 14/5/4.
//
//

#include <functional>
#include "AIContext.h"
#include "AIContinuousMoveCaster.h"
#include "action_hero/HeroSkillController.h"
#include "action_hero/HeroActionController.h"
#include "action_hero/HeroTurnAction.h"
#include "AITargetHelper.h"
#include "scriptbind/AIScriptBindActionController.hpp"
#include "AIScriptBinder.hpp"

namespace OnePiece{
    
    static void initMoveCasterTargetContextForOld(AITargetContext &ctx, AIContinuousMoveCaster *caster)
    {
        ctx.source = caster->origin;
        ctx.target = caster->target;
		MoveExec* moveData = caster->staticData;
		if (moveData && moveData->targetTracing == (int)AIContinuousMoveCaster::MoveType::kSpecialSummonBackMove) {
			ctx.target = caster->origin->getSummonSource();
		}		
        ctx.skill_caster = caster->origin;
        ctx.projectile = nullptr;
        ctx.skill_id = caster->skillID;
        ctx.skill_level = caster->skillLevel;
        ctx.hurt_id = 0;
        
        auto caster_data = caster->staticData;
        ctx.search_type = (int)caster_data->target;
        ctx.range.x1 = caster_data->range[0];
        ctx.range.x2 = caster_data->range[1];
        ctx.range.y1 = caster_data->range[2];
        ctx.range.y2 = caster_data->range[3];
        ctx.search_params = nullptr;
        ctx.filter_list = nullptr;
    }
    
    static void initMoveCasterTargetContextForNew(AITargetContext &ctx, AIContinuousMoveCaster *caster)
    {
        ctx.source = caster->target;
        if (caster->target)
            ctx.target = caster->target->target;
        ctx.skill_caster = caster->origin;
        ctx.projectile = nullptr;
        ctx.skill_id = caster->skillID;
        ctx.skill_level = caster->skillLevel;
        ctx.hurt_id = 0;
        
        auto caster_data = caster->staticData;
        ctx.search_type = (int)caster_data->target;
        ctx.range.x1 = caster_data->range[0];
        ctx.range.x2 = caster_data->range[1];
        ctx.range.y1 = caster_data->range[2];
        ctx.range.y2 = caster_data->range[3];
        ctx.search_params = nullptr;
        ctx.filter_list = nullptr;
    }

	AIContinuousMoveCaster::AIContinuousMoveCaster(int moveID, int skillID, unsigned short skillLevel, CharacterData* origin, CharacterData* target, AIContext& aiContext)
        : AINode(origin->myID, origin->clusterNum), aiContext(aiContext)
	{
		myID = aiContext.local_incremential++;

		this->moveID = moveID;
		staticData = AIDataHelper::getMoveExec(moveID);
		this->origin = origin;
		this->target = target;
        this->skillID = skillID;
		this->skillLevel = skillLevel;
		this->maxMoveCnt = 1;

		localTick = aiContext.currentTick;//external tick num of context
		startTick = localTick;
		if (staticData->targetTracing == (int)MoveType::kBlackHoleField)
		{
			totalTick = staticData->duration + staticData->durationX;//from 0
		}
		else
		{
			totalTick = staticData->duration;//from 0
		}

		CharacterData* character = nullptr;
		if (origin->hasSkill(skillID))
		{
			character = origin;
		}
		else if (target->hasSkill(skillID))
		{
			character = target;
		}

		if (character != nullptr && character->attackSpeed != 100 && character->attackSpeed > 0 && AIScriptBinder::getInstance()->isCasterSyncAttackSpeed(aiContext.getLuaState(), moveID))
		{
			speedTickParam = character->attackSpeed;
		}

		totalTick = totalTick * speedTickParam / 100;
		
		realDuration = staticData->duration * speedTickParam / 100;
		realDurationX = staticData->durationX * speedTickParam / 100;
		realDurationY = staticData->durationY * speedTickParam / 100;
		realReturnPause = staticData->returnPause * speedTickParam / 100; 
		realActionFreq = staticData->actionFreq * speedTickParam / 100;
		realSpeedX = staticData->speedX;
		realSpeedY = staticData->speedY;
		realAccX = staticData->accX;
		realAccY = staticData->accY;

		if (staticData->bIsUseactionFreq > 0 && realActionFreq > 0)
		{
			this->maxMoveCnt = totalTick / realActionFreq;
		}


		if (staticData->syncTargetMech == 0)
			moveType = MoveType::kNormalHasTarget;
	
		BlackHoleAngleA = 0;

        ignoreFreeze = target->isCaptain();

		origin->removeSpecialMoveCast((int)MoveType::kSpecialSummonBackMove);

 		origin->moveCasts.push_back(this);
	}

	AIContinuousMoveCaster::~AIContinuousMoveCaster()
	{
        for (auto iter = movers.begin(); iter != movers.end(); ++iter)
        {
            auto pHero = iter->second->mover;
            pHero->moveCasterAffectedCount--;
            if (pHero->moveCasterAffectedCount < 0)
                pHero->moveCasterAffectedCount = 0;
            //pHero->lastDirection = pHero->direction;
            delete iter->second;
        }

        movers.clear();

		if (staticData->targetTracing == (int)MoveType::kSpecialSummonBackMove)
		{
			origin->needAdjusting = false;
			origin->adjustingSpeedX = 0;
		}

		for (auto iter = origin->moveCasts.begin(); iter != origin->moveCasts.end(); ++iter)
		{
			if ((*iter)->myID == myID)
			{
				origin->moveCasts.erase(iter);
				break;
			}
		}
	}

    void AIContinuousMoveCaster::initCaster(AIContext& aiContext)
    {
		if (staticData->syncTargetMech > 0)
		{
			NewInitCaster(aiContext);
		}
		else
		{
			OldInitCaster(aiContext);
		}
    }

    void AIContinuousMoveCaster::updateNormalHasTarget(AIContinuousMoveCaster::MoverInfo& mover, int playedTick)
    {
        CharacterData* theMover = mover.mover;
        CharacterData* theTarget = mover.moversTarget;

		
		const int flyTick = (realDuration - realReturnPause) / (staticData->needReturn ? 2 : 1);
        int targetHeroPosX = (staticData->targetTracing) ? theTarget->getX() : mover.initTargetX;
        int targetX = targetHeroPosX;
        //targetX += ((targetX > theMover->getX()) ? 1 : -1) * staticData->targetDistanceX;
        targetX += mover.initFaceDirectionX * staticData->targetDistanceX;
		targetX = aiContext.getAdjustedPosXWithinRange(targetX);
		
        int targetY = (staticData->targetTracing) ? theTarget->getY() : mover.initTargetY;
		
		// clair_wang 16.01.19 // fix bug：马尔高小招2击飞怪物，恰好鲁兹大招跳后排时，鲁兹站位y坐标不正确
		if (theTarget->currentAction == STRUCK_FLY_AWAY_ACTION || theTarget->currentAction == STRUCK_HOVER_ACTION)
		{
			targetY = (staticData->targetTracing) ? theTarget->oldY : mover.initTargetY;
		}

        //targetY += ((targetY > theMover->getY()) ? 1 : -1) * staticData->targetDistanceY;
        targetY += mover.initFaceDirectionY * staticData->targetDistanceY;

        if (staticData->needReturn)
        {
			if (staticData->bIsUseactionFreq > 0 && realActionFreq > 0)
			{
				int nTmpFlyTick = flyTick / maxMoveCnt;
				if (nTmpFlyTick < 1)
				{
					nTmpFlyTick = 1;
				}

				int nTmpPlayedTick = playedTick;
				if (nTmpPlayedTick >= realActionFreq)
				{
					nTmpPlayedTick %= realActionFreq;
				}
				nTmpPlayedTick += 1;


				if ((nTmpPlayedTick / realActionFreq) >= maxMoveCnt)
				{
					targetHeroPosX = mover.initMoverX;
					if (realActionFreq == nTmpPlayedTick)
					{
						theMover->setX(mover.initMoverX);
						theMover->setY(mover.initMoverY);

						// 有时间间隔的
						if (((playedTick + 1) % realActionFreq == 0) && ((playedTick + 1) / realActionFreq < maxMoveCnt))
						{
							TodoAfterEndFun(theMover);
						}
					}
                    else if (totalTick != playedTick)
					{
						auto nextX = (mover.initMoverX - theMover->getX()) / (totalTick - playedTick) + theMover->getX();
						nextX = aiContext.getAdjustedPosXWithinRange(nextX);

						theMover->setX(nextX);
						theMover->setY((mover.initMoverY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
					}
				}
				else
				{
					
					if (nTmpPlayedTick <= nTmpFlyTick)
					{
						if (nTmpFlyTick == nTmpPlayedTick)
						{
							theMover->setX(targetX);
							theMover->setY(targetY);
						}
                        else if (nTmpFlyTick != playedTick)
						{
							auto nextX = (targetX - theMover->getX()) / (nTmpFlyTick - playedTick) + theMover->getX();
							nextX = aiContext.getAdjustedPosXWithinRange(nextX);

							theMover->setX(nextX);
							theMover->setY((targetY - theMover->getY()) / (nTmpFlyTick - playedTick) + theMover->getY());
						}
					}
					else if (nTmpPlayedTick > (realReturnPause / maxMoveCnt))
					{
						targetHeroPosX = mover.initMoverX;
						if (realActionFreq == nTmpPlayedTick)
						{
							theMover->setX(mover.initMoverX);
							theMover->setY(mover.initMoverY);
						}
                        else if (totalTick != playedTick)
						{
							auto nextX = (mover.initMoverX - theMover->getX()) / (totalTick - playedTick) + theMover->getX();
							nextX = aiContext.getAdjustedPosXWithinRange(nextX);

							theMover->setX(nextX);
							theMover->setY((mover.initMoverY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
						}
					}
				}
			}
			else
			{
				if (playedTick <= flyTick)
				{
					if (flyTick == playedTick)
					{
						theMover->setX(targetX);
						theMover->setY(targetY);
					}
                    else
					{
						auto nextX = (targetX - theMover->getX()) / (flyTick - playedTick) + theMover->getX();
						nextX = aiContext.getAdjustedPosXWithinRange(nextX);

						theMover->setX(nextX);
						theMover->setY((targetY - theMover->getY()) / (flyTick - playedTick) + theMover->getY());
					}
				}
				else if (playedTick > flyTick + realReturnPause)
				{
					targetHeroPosX = mover.initMoverX;
					if (totalTick - 1 == playedTick)
					{
						theMover->setX(mover.initMoverX);
						theMover->setY(mover.initMoverY);
					}
                    else if (totalTick != playedTick)
					{
						auto nextX = (mover.initMoverX - theMover->getX()) / (totalTick - playedTick) + theMover->getX();
						nextX = aiContext.getAdjustedPosXWithinRange(nextX);

						theMover->setX(nextX);
						theMover->setY((mover.initMoverY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
					}
				}
				else
				{
					// MARK: 尝试修正新的可保留移动执行，在角色切换状态选择新目标后能够继续攻击，而不是不断的转向
		  //          targetHeroPosX = mover.initMoverX;
				}
			}
        }
        else
        {
			if (staticData->bIsUseactionFreq > 0 && realActionFreq > 0)
			{
				int nTmpPlayedTick = playedTick;
				if (nTmpPlayedTick >= realActionFreq)
				{
					nTmpPlayedTick %= realActionFreq;
				}
				nTmpPlayedTick += 1;


				if ((playedTick / realActionFreq) >= maxMoveCnt)
				{
					theMover->setX(targetX);
					theMover->setY(targetY);
				}
				else
				{
					if (nTmpPlayedTick % realActionFreq == 0)
					{
						theMover->setX(targetX);
						theMover->setY(targetY);
					}
					else
					{
						if (playedTick > 0 && playedTick % realActionFreq == 0)
						{
							theMover->setX(mover.initMoverX);
							theMover->setY(mover.initMoverY);
						}
                        else if (totalTick != playedTick)
						{
							auto nextX = (targetX - theMover->getX()) / (totalTick - playedTick) + theMover->getX();
							nextX = aiContext.getAdjustedPosXWithinRange(nextX);

							theMover->setX(nextX);
							theMover->setY((targetY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
						}
					}
				}

		/*		if ((playedTick+1) % staticData->actionFreq == 0)
				{
					theMover->setX(targetX);
					theMover->setY(targetY);
				}
				else
				{
					theMover->setX((targetX - theMover->getX()) / (totalTick - playedTick) + theMover->getX());
					theMover->setY((targetY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
				}
		*/
			}
			else
			{
				if (flyTick - 1 <= playedTick)
				{
					theMover->setX(targetX);
					theMover->setY(targetY);
				}
                else if (totalTick != playedTick)
				{
					auto nextX = (targetX - theMover->getX()) / (totalTick - playedTick) + theMover->getX();
					nextX = aiContext.getAdjustedPosXWithinRange(nextX);

					theMover->setX(nextX);
					theMover->setY((targetY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
				}
			}
        }

		TodoMovingExecFunc(playedTick, theMover);

        // MARK: 移动中实时改变人物朝向，未严格测试
        // 角色改变状态时不清除的移动执行，不做自动转向，防止角色释放下一个技能寻找目标时反复转向
        if ((!staticData->keepWhenChangeAction))
        {
			if (theMover->direction == -1)
			{
				int faceDirection = theMover->getX() >= targetHeroPosX ? -1 : 1;
				//theMover->lastDirection = theMover->direction;
				theMover->direction = faceDirection;
			}
			else
			{
				int faceDirection = theMover->getX() > targetHeroPosX ? -1 : 1;
				//theMover->lastDirection = theMover->direction;
				theMover->direction = faceDirection;
			}
        }
    }

    void AIContinuousMoveCaster::updateNormalNoTarget(AIContinuousMoveCaster::MoverInfo& mover, int playedTick)
    {
        CharacterData* theMover = mover.mover;
        CharacterData* theTarget = mover.moversTarget;

        if (staticData->needReturn)
        {
            do {
				if (realDurationX <= 0)
                    break;

				const int stopTick = realDurationX;
                if (playedTick >= stopTick)
                    break;

                if (playedTick == stopTick - 1)
                {
                    theMover->setX(mover.initMoverX);
                    break;
                }

                const int direction = mover.initMoverX < mover.initTargetX ? 1 : -1;
                bool shouldMove = false;
                bool isReturn = false;

                if (mover.tickAtReturnPointX <= 0)
                {
                    if ((direction > 0 && theMover->getX() < mover.initTargetX) || (direction < 0 && theMover->getX() > mover.initTargetX))
                    {
                        shouldMove = true;
                    }
                    else
                    {
                        mover.tickAtReturnPointX = playedTick;
						isReturn = (realReturnPause <= 0);
                        shouldMove = isReturn;
                    }
                }
                else
                {
                    // start return
					if (playedTick - mover.tickAtReturnPointX >= realReturnPause)
                    {
                        shouldMove = true;
                        isReturn = true;
                    }
                }

                if (shouldMove)
					theMover->setX(theMover->getX() + (realSpeedX + realAccX * playedTick) * direction * (isReturn ? -1 : 1));

            } while (false);

			do {
				if (realDurationY <= 0)
					break;

				const int stopTick = realDurationY;
				if (playedTick >= stopTick)
					break;

				if (playedTick == stopTick - 1)
				{
					theMover->setY(mover.initMoverY);
					break;
				}

				const int direction = mover.initMoverY < mover.initTargetY ? 1 : -1;
				bool shouldMove = false;
				bool isReturn = false;

				if (mover.tickAtReturnPointY <= 0)
				{
					if ((direction > 0 && theMover->getY() < mover.initTargetY) || (direction < 0 && theMover->getY() > mover.initTargetY))
					{
						shouldMove = true;
					}
					else
					{
						mover.tickAtReturnPointY = playedTick;
						isReturn = (realReturnPause <= 0);
						shouldMove = isReturn;
					}
				}
				else
				{
					// start return
					if (playedTick - mover.tickAtReturnPointY >= realReturnPause)
					{
						shouldMove = true;
						isReturn = true;
					}
				}

				if (shouldMove)
					theMover->setY(theMover->getY() + (realSpeedY + realAccY * playedTick) * direction * (isReturn ? -1 : 1));

			} while (false);
        }
        else
        {
			if (staticData->bIsUseactionFreq > 0 && realActionFreq > 0)
			{
				if (playedTick < realDurationX)
				{
					auto nextX = theMover->getX() + (realSpeedX + realAccX * playedTick) * theMover->direction;
					nextX = aiContext.getAdjustedPosXWithinRange(nextX);
					theMover->setX(nextX);
				}

				// 12.0 新增Y轴的移动
				if (playedTick < realDurationY)
				{
					auto nextY = theMover->getY() + (realSpeedY + realAccY * playedTick) * theMover->direction;
					nextY = aiContext.getAdjustedPosYWithinRange(nextY);
					theMover->setY(nextY);
				}
			}
			else
			{
				if (playedTick < realDurationX)
				{
					auto nextX = theMover->getX() + (realSpeedX + realAccX * playedTick) * theMover->direction;
					nextX = aiContext.getAdjustedPosXWithinRange(nextX);
					theMover->setX(nextX);
				}

				// 12.0 新增Y轴的移动
				if (playedTick < realDurationY)
				{
					auto nextY = theMover->getY() + (realSpeedY + realAccY * playedTick) * theMover->direction;
					nextY = aiContext.getAdjustedPosYWithinRange(nextY);
					theMover->setY(nextY);
				}
			}
        }

		TodoMovingExecFunc(playedTick, theMover);
		
    }

    void AIContinuousMoveCaster::updateCrossBattleFiled(AIContinuousMoveCaster::MoverInfo& mover, int playedTick)
    {
        // 穿墙模式，先按初始面向，移动到该方向的战场边界，
        // 再瞬间到另一个边界，重新进入战场，移动到原始位置
        auto theMover = mover.mover;
        auto direction = mover.initFaceDirectionX;

        // 首次到达边界，立即移到另一个边界，达到穿屏效果
        if (mover.tickAtReturnPointX <= 0)
        {
            auto deltaX = theMover->getX() - mover.initTargetX;
            if (deltaX * direction >= 0)
            {
                mover.tickAtReturnPointX = playedTick;
                auto startX = direction > 0 ? aiContext.getBattleFieldXMostLeft() : aiContext.getBattleFieldXMostRight();
                theMover->setX(startX);
                return;
            }
        }
        
		theMover->setX(theMover->getX() + (realSpeedX + realAccX * playedTick) * direction);
		
		auto _CmpDeltaX = theMover->getX() - mover.initMoverX;
		if (mover.tickAtReturnPointX > 0 && (_CmpDeltaX * mover.initFaceDirectionX >= 0))
		{
			theMover->setX(mover.initMoverX);
		}

		TodoMovingExecFunc(playedTick, theMover);
    }
	
	/** 黑洞模式 **/
	#define BLACKHOLD_DELTA_SPEED_Y 10
	void AIContinuousMoveCaster::updateBlackHoleField(AIContinuousMoveCaster::MoverInfo& mover, int playedTick)
	{
		CharacterData* theMover = mover.mover;
		
		const int flyTick = realDurationX;
		const int cirleTick = realDuration;

		int targetHeroPosX = mover.initTargetX;	
		int targetX = targetHeroPosX;
		int targetY = mover.initTargetY;


		if (staticData->needReturn)
		{
			// 目前黑洞模式不考虑折回
			//{
			//	if (playedTick <= (flyTick + cirleTick))
			//	{
			//		if (playedTick < flyTick)
			//		{
			//			theMover->setX((targetX - theMover->getX()) / (flyTick - playedTick) + theMover->getX());
			//			theMover->setY((targetY - theMover->getY()) / (flyTick - playedTick) + theMover->getY());
			//		}
			//		else if (flyTick == playedTick)
			//		{
			//			theMover->setX(targetX);
			//			theMover->setY(targetY);
			//		}
			//		else
			//		{
			//			mover.continualFlutterTick = aiContext.localRandom->nextInt(0, staticData->targetDistanceX);

			//			if (playedTick % 20 == 0)
			//			{// 计算
			//				int nTmpX = 0;
			//				int nTmpY = 0;
			//				int nAngelA = aiContext.localRandom->nextInt(0, 360);
			//				int nR = mover.continualFlutterTick;

			//				float cosA = std::cos(nAngelA);
			//				float sinA = std::sin(nAngelA);

			//				nTmpX = ((nR * cosA) > 0) ? (nR * cosA) : 0;
			//				nTmpY = ((nR * sinA) > 0) ? (nR * sinA) : 0;

			//				nTmpX += targetX;
			//				nTmpY += targetY;

			//				nTmpX = nTmpX < aiContext.getBattleFieldXMostLeft() ? aiContext.getBattleFieldXMostLeft() : nTmpX;
			//				nTmpX = nTmpX > aiContext.getBattleFieldXMostRight() ? aiContext.getBattleFieldXMostRight() : nTmpX;

			//				theMover->setX(nTmpX);
			//				theMover->setY(nTmpY);
			//			}
			//		}
			//	}
			//	else if (playedTick >= (flyTick + cirleTick - 1))
			//	{
			//		if (playedTick > (flyTick + cirleTick + staticData->returnPause))
			//		{
			//			targetHeroPosX = mover.initMoverX;
			//			if (totalTick - 1 == playedTick)
			//			{
			//				theMover->setX(mover.initMoverX);
			//				theMover->setY(mover.initMoverY);
			//			}
			//			else
			//			{
			//				theMover->setX((mover.initMoverX - theMover->getX()) / (totalTick - playedTick) + theMover->getX());
			//				theMover->setY((mover.initMoverY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
			//			}
			//		}
			//	}
			//	else
			//	{
			//		// MARK: 尝试修正新的可保留移动执行，在角色切换状态选择新目标后能够继续攻击，而不是不断的转向
			//		//          targetHeroPosX = mover.initMoverX;
			//	}
			//}
		}
		else
		{
			if (playedTick >= (flyTick + cirleTick - 1))
			{
				theMover->setX(theMover->getX());
				theMover->setY(mover.initMoverY);
				
				// 解决被击飞中断后，目标停留在半空不下来的问题。
				if (theMover->currentAction == STRUCK_FLY_AWAY_ACTION || theMover->currentAction == STRUCK_HOVER_ACTION)
				{
					theMover->oldY = mover.initMoverY;
				}

				HeroActionController::setFlySpeedY(theMover->actionAtt, HeroActionController::getFlySpeedY(theMover->actionAtt) - BLACKHOLD_DELTA_SPEED_Y);
			}
			else
			{
				if (playedTick < flyTick)
				{
					theMover->setX((targetX - theMover->getX()) / (flyTick - playedTick) + theMover->getX());
					theMover->setY((targetY - theMover->getY()) / (flyTick - playedTick) + theMover->getY());
				}
				else
				{
					if (playedTick == flyTick)
					{
						mover.continualFlutterTick = aiContext.localRandom->nextInt(0, staticData->targetDistanceX);
						BlackHoleAngleA = aiContext.localRandom->nextInt(0, 45);
					}
				
					if (playedTick % 20 == 0)
					{// 计算
						int nTmpX = 0;
						int nTmpY = 0;
						int mMaxRound = 360;

						int nLastAngel = BlackHoleAngleA;
						if (BlackHoleAngleA != 0)
						{
							BlackHoleAngleA = aiContext.localRandom->nextInt((nLastAngel + 1), (nLastAngel + 24));

							if ((mMaxRound - nLastAngel) < 14 /*|| (nAngelA - nLastAngel) < 14*/)
							{
								BlackHoleAngleA = aiContext.localRandom->nextInt(0, 45);
							}
						}

						int nR = mover.continualFlutterTick;

						float cosA = std::cos(BlackHoleAngleA);
						float sinA = std::sin(BlackHoleAngleA);

						nTmpX = (nR * cosA) * theMover->direction;// ((nR * cosA) > 0) ? (nR * cosA) : 0;
						nTmpY = (nR * sinA);// ((nR * sinA) > 0) ? (nR * sinA) : 0;

						nTmpX += targetX;
						nTmpY += targetY;

						nTmpX = nTmpX < aiContext.getBattleFieldXMostLeft() ? (aiContext.getBattleFieldXMostLeft() /*+ staticData->targetDistanceX*/) : nTmpX;
						nTmpX = nTmpX > aiContext.getBattleFieldXMostRight() ? (aiContext.getBattleFieldXMostRight()/* + staticData->targetDistanceX*/) : nTmpX;
						
						theMover->setX(nTmpX);
						theMover->setY(nTmpY);

						//CCLOG("clair_wang nAngleA=%d, x,y=[%d,%d]", nAngelA, theMover->getX(), theMover->getY());
					}
				}
			}

			// 目前用了鲁兹大招不折回进行了测试，晕的人也可以移动 
		}

		
		TodoMovingExecFunc(playedTick, theMover);

		// MARK: 移动中实时改变人物朝向，未严格测试
		// 角色改变状态时不清除的移动执行，不做自动转向，防止角色释放下一个技能寻找目标时反复转向
		if ((!staticData->keepWhenChangeAction))
		{
			if (theMover->direction == -1)
			{
				int faceDirection = theMover->getX() >= targetHeroPosX ? -1 : 1;
				//theMover->lastDirection = theMover->direction;
				theMover->direction = faceDirection;
			}
			else
			{
				int faceDirection = theMover->getX() > targetHeroPosX ? -1 : 1;
				//theMover->lastDirection = theMover->direction;
				theMover->direction = faceDirection;
			}
		}
	}

	void AIContinuousMoveCaster::updateRandomPointField(AIContinuousMoveCaster::MoverInfo& mover, int playedTick)
	{
		CharacterData* theMover = mover.mover;

		const int flyTick = (realDuration - realReturnPause) / (staticData->needReturn ? 2 : 1);
		
		int targetHeroPosX = mover.initTargetX;
		int targetX = targetHeroPosX;
		int targetY = mover.initTargetY;

		if (staticData->needReturn)
		{
			if (staticData->bIsUseactionFreq > 0 && realActionFreq > 0)
			{
				
				int nTmpFlyTick = flyTick / maxMoveCnt;
				if (nTmpFlyTick < 1)
				{
					nTmpFlyTick = 1;
				}

				int nTmpPlayedTick = playedTick;
				if (nTmpPlayedTick >= realActionFreq)
				{
					nTmpPlayedTick %= realActionFreq;
				}
				nTmpPlayedTick += 1;


				if ((nTmpPlayedTick / realActionFreq) >= maxMoveCnt)
				{
					targetHeroPosX = mover.initMoverX;
					if (realActionFreq == nTmpPlayedTick)
					{
						theMover->setX(mover.initMoverX);
						theMover->setY(mover.initMoverY);
					}
                    else if (totalTick != playedTick)
					{
						theMover->setX((mover.initMoverX - theMover->getX()) / (totalTick - playedTick) + theMover->getX());
						theMover->setY((mover.initMoverY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
					}
				}
				else
				{

					if (nTmpPlayedTick <= nTmpFlyTick)
					{
						if (nTmpFlyTick == nTmpPlayedTick)
						{
							theMover->setX(targetX);
							theMover->setY(targetY);
						}
                        else if (nTmpFlyTick != playedTick)
						{
							theMover->setX((targetX - theMover->getX()) / (nTmpFlyTick - playedTick) + theMover->getX());
							theMover->setY((targetY - theMover->getY()) / (nTmpFlyTick - playedTick) + theMover->getY());
						}
					}
					else if (nTmpPlayedTick > (realReturnPause / maxMoveCnt) + nTmpFlyTick)
					{
						targetHeroPosX = mover.initMoverX;
						if (realActionFreq == nTmpPlayedTick)
						{
							theMover->setX(mover.initMoverX);
							theMover->setY(mover.initMoverY);
						}
                        else if (totalTick != playedTick)
						{
							theMover->setX((mover.initMoverX - theMover->getX()) / (totalTick - playedTick) + theMover->getX());
							theMover->setY((mover.initMoverY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
						}
					}
				}
			}
			else
			{
				if (playedTick < flyTick)
				{
					if ((flyTick - 1) == playedTick)
					{
						theMover->setX(targetX);
						theMover->setY(targetY);
					}
					else
					{
						theMover->setX((targetX - theMover->getX()) / (flyTick - playedTick) + theMover->getX());
						theMover->setY((targetY - theMover->getY()) / (flyTick - playedTick) + theMover->getY());
					}
				}
				else if ((playedTick + 1) > flyTick + realReturnPause)
				{
					targetHeroPosX = mover.initMoverX;
					if (totalTick - 1 == playedTick)
					{
						theMover->setX(mover.initMoverX);
						theMover->setY(mover.initMoverY);
					}
                    else if (totalTick != playedTick)
					{
						theMover->setX((mover.initMoverX - theMover->getX()) / (totalTick - playedTick) + theMover->getX());
						theMover->setY((mover.initMoverY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
					}
				}
				else
				{
					// MARK: 尝试修正新的可保留移动执行，在角色切换状态选择新目标后能够继续攻击，而不是不断的转向
					//          targetHeroPosX = mover.initMoverX;
				}
			}
		}
		else
		{
			if (staticData->bIsUseactionFreq > 0 && realActionFreq > 0)
			{		
				int nTmpPlayedTick = playedTick;
				if (nTmpPlayedTick >= realActionFreq)
				{
					nTmpPlayedTick %= realActionFreq;
				}
				nTmpPlayedTick += 1;


				if ((playedTick / realActionFreq) >= maxMoveCnt)
				{
					theMover->setX(targetX);
					theMover->setY(targetY);
				}
				else
				{
					if (nTmpPlayedTick % realActionFreq == 0)
					{
						theMover->setX(targetX);
						theMover->setY(targetY);

						// 有时间间隔的
						if (((playedTick + 1) % realActionFreq == 0) && ((playedTick + 1) / realActionFreq < maxMoveCnt))
						{
							TodoAfterEndFun(theMover);
						}
					}
					else
					{
						if (playedTick > 0 && playedTick % realActionFreq == 0)
						{
							theMover->setX(mover.initMoverX);
							theMover->setY(mover.initMoverY);
						}
                        else if(totalTick != playedTick)
						{
							theMover->setX((targetX - theMover->getX()) / (totalTick - playedTick) + theMover->getX());
							theMover->setY((targetY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
						}
					}
				}

				/*if ((playedTick / staticData->actionFreq + 1) >= (flyTick / staticData->actionFreq))
				{
					theMover->setX(targetX);
					theMover->setY(targetY);
				}
				else
				{
					theMover->setX((targetX - theMover->getX()) / (totalTick - playedTick) + theMover->getX());
					theMover->setY((targetY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
				}*/
			}
			else
			{
				if (flyTick - 1 <= playedTick)
				{
					theMover->setX(targetX);
					theMover->setY(targetY);
				}
                else if (totalTick != playedTick)
				{
					theMover->setX((targetX - theMover->getX()) / (totalTick - playedTick) + theMover->getX());
					theMover->setY((targetY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
				}
			}
		}


		// MARK: 移动中实时改变人物朝向，未严格测试
		// 角色改变状态时不清除的移动执行，不做自动转向，防止角色释放下一个技能寻找目标时反复转向
/*		if ((!staticData->keepWhenChangeAction))
		{
			if (theMover->direction == -1)
			{
				int faceDirection = theMover->getX() >= targetHeroPosX ? -1 : 1;
				theMover->lastDirection = theMover->direction;
				theMover->direction = faceDirection;
			}
			else
			{
				int faceDirection = theMover->getX() > targetHeroPosX ? -1 : 1;
				theMover->lastDirection = theMover->direction;
				theMover->direction = faceDirection;
			}
		}*/

		if ((!staticData->keepWhenChangeAction))
		{
			bool bFlag = false;
			CharacterData* pTarget = theMover->lockTarget(aiContext);
			if (pTarget != nullptr)
			{
				HeroActionController::setCurrentTarget(*theMover, pTarget);
			}

			if (((playedTick + 2) % realActionFreq == 0) && ((playedTick + 2) / realActionFreq < maxMoveCnt))
			{
				bFlag = true;
			}

			if (!bFlag && (pTarget->getX() < targetHeroPosX && theMover->getX() < targetHeroPosX))
			{
				if (theMover->direction == -1)
				{
					int faceDirection = theMover->getX() >= targetHeroPosX ? -1 : 1;
					//theMover->lastDirection = theMover->direction;
					theMover->direction = faceDirection;
				}
				else
				{
					int faceDirection = theMover->getX() > targetHeroPosX ? -1 : 1;
					//theMover->lastDirection = theMover->direction;
					theMover->direction = faceDirection;
				}
			}
			else if (!bFlag && (pTarget->getX() > targetHeroPosX && theMover->getX() > targetHeroPosX))
			{
				if (theMover->direction == -1)
				{
					int faceDirection = theMover->getX() >= targetHeroPosX ? -1 : 1;
					//theMover->lastDirection = theMover->direction;
					theMover->direction = faceDirection;
				}
				else
				{
					int faceDirection = theMover->getX() > targetHeroPosX ? -1 : 1;
					//theMover->lastDirection = theMover->direction;
					theMover->direction = faceDirection;
				}
			}
			else
			{
				if (theMover->direction == -1)
				{
					int faceDirection = theMover->getX() >= pTarget->getX() ? -1 : 1;
					//theMover->lastDirection = theMover->direction;
					theMover->direction = faceDirection;
				}
				else
				{
					int faceDirection = theMover->getX() > pTarget->getX() ? -1 : 1;
					//theMover->lastDirection = theMover->direction;
					theMover->direction = faceDirection;
				}
			}
		}

		TodoMovingExecFunc(playedTick, theMover);
	}

    void AIContinuousMoveCaster::exertMove(MoverInfo& mover, int playedTick)
    {
		switch (moveType)
		{
		case MoveType::kNormalNoTarget:
		{
			updateNormalNoTarget(mover, playedTick);
		}
			break;
		case MoveType::kNormalHasTarget:
		{
			updateNormalHasTarget(mover, playedTick);
		}
			break;
		case MoveType::kCrossBattleField:
		{
			updateCrossBattleFiled(mover, playedTick);
		}
			break;
		case MoveType::kBlackHoleField:
		{
			updateBlackHoleField(mover, playedTick);
		}
			break;
		case MoveType::kRandomPointField:
		{
			updateRandomPointField(mover, playedTick);
		}
			break;
		case MoveType::kTrackTargetMove:
		{
			updateTrackTargetField(mover, playedTick);
		}
			break;
		case MoveType::kCrossBattlefieldNew:
		{
			updateCrossBattleFieldNew(mover, playedTick);
		}
			break;
		case MoveType::KAbsolutePostionMove:
		{
			updateAbsolutePositionField(mover, playedTick);
		}
			break;
		case MoveType::kSpecialSummonBackMove:
		{
			updateSpecialSummonBackMove(mover, playedTick);
		}
			break;
		default:
			break;
		} 
    }

	void AIContinuousMoveCaster::Excute(AIContext& aiContext)
	{
		AINode::update(aiContext);
		if (isFrozen)
			return;

        if (shouldDeleteSelf)
        {
            TodoAfterEndFun(origin);
            delete this;
            return;
        }

		const int playedTick = localTick - startTick - 1;
		
		if (MoveType::kBlackHoleField == moveType)
		{
			if ((totalTick == 0 && playedTick > totalTick) || (totalTick != 0 && playedTick >= totalTick))
			{
				TodoAfterEndFun(origin);
                delete this;
				return;
			}
		}
		else if (MoveType::kTrackTargetMove == moveType) // = 7 追踪目标移动，直到目标停止，移动执行结束。
		{
			 // totalTick; // 设置的最大移动时间，时间到后不论在何位置一律结束移动。

			if ((totalTick == 0 && playedTick > totalTick) || (totalTick != 0 && playedTick >= totalTick)
				)
			{
				TodoAfterEndFun(origin);
                delete this;
				return;
			}
			else if (origin->getX() < aiContext.getBattleFieldXMostLeft() || origin->getX() > aiContext.getBattleFieldXMostRight())
			{ 
				// 移动不会超出战场边界，若到达战场边界仍未满足1(设置的最大移动时间)，则立刻结束移动；

				TodoAfterEndFun(origin);
				delete this;
				return;
			}
		}
		else if (MoveType::kCrossBattlefieldNew == moveType)
		{
			if ((totalTick == 0 && playedTick > totalTick) || (totalTick != 0 && playedTick >= totalTick))
			{
				TodoAfterEndFun(origin);
				delete this;
				return;
			}
		}
		else if (MoveType::KAbsolutePostionMove == moveType)
		{
			if ((totalTick == 0 && playedTick > totalTick) || (totalTick != 0 && playedTick >= totalTick))
			{
				TodoAfterEndFun(origin);
				delete this;
				return;
			}
		}
		else if (MoveType::kSpecialSummonBackMove == moveType)
		{
		}
		else if(MoveType::kCrossBattleField != moveType)
        {
            if ((totalTick == 0 && playedTick > totalTick) || (totalTick != 0 && playedTick >= totalTick))
            {
				TodoAfterEndFun(origin);
                delete this;
                return;
            }			
        }
        else
        {
            auto moveInfo = movers.begin()->second;
            auto theMover = moveInfo->mover;
			
			if (realDurationX >= 0)
			{
				auto deltaX = theMover->getX() - moveInfo->initMoverX;
				if (moveInfo->tickAtReturnPointX > 0 && (deltaX * moveInfo->initFaceDirectionX >= 0))
				{
					TodoAfterEndFun(theMover);
					delete this;
					return;
				}
			}
			if (realDurationY >= 0)
			{
				auto deltaY = theMover->getY() - moveInfo->initMoverY;
				if (moveInfo->tickAtReturnPointY > 0 && (deltaY * moveInfo->initFaceDirectionY >= 0))
				{
					TodoAfterEndFun(theMover);
					delete this;
					return;
				}
			}
        }

        for (auto iter = movers.begin(); iter != movers.end(); ++iter)
        {
            exertMove(*(iter->second), playedTick);
        }
	} // END Excute

    bool AIContinuousMoveCaster::shouldKeepCaster()
    {
        return staticData->keepWhenChangeAction;
	}

	void AIContinuousMoveCaster::TodoAfterEndFun(CharacterData* theMover/* = nullptr*/)
	{
		if (staticData->vAfterMoveExecs.size() > 0)
		{
			for (int i = 0; i < staticData->vAfterMoveExecs.size(); i++)
			{
				int nExecId = staticData->vAfterMoveExecs[i];
				//if(nExecId == 12808)
				//	continue;
				AIActionController::executeExec(aiContext, nExecId, skillID, skillLevel, theMover, nullptr, target, 0);
			}
		}
	}

	// 在移动过程中持续执行，主要是动画执行。
	void AIContinuousMoveCaster::TodoMovingExecFunc(int thePlayTick, CharacterData* theMover/* = nullptr*/)
	{
		if (staticData->vMovingExecuteIds.size() > 1)
		{

			for (int i = 0; i < staticData->vMovingExecuteIds.size(); i++)
			{
				int nExecID = 0;
				nExecID = staticData->vMovingExecuteIds[i];

				if (nExecID > 0)
				{
					int _animTick = 1;
#if BATTLE_LOG_TEST
					// clair_wang test
					CCLOG("clair_wang Mover(%d, %d) Target(%d, %d) leftMax=%d, RightMax=%d", theMover->getX(), theMover->getY(), theTarget->getX(), theTarget->getY(), aiContext.getBattleFieldXMostLeft(), aiContext.getBattleFieldXMostRight());
#endif
					if (IsAnimationExec(nExecID))
					{
                        if (AIDataHelper::getSkillAnimationExec(nExecID))
						    _animTick = AIDataHelper::getSkillAnimationExec(nExecID)->duration;
					}
					else if (IsHurtExec(nExecID))
					{
                        if (AIDataHelper::getHurtExec(nExecID))
						    _animTick = AIDataHelper::getHurtExec(nExecID)->duration;
					}
					else if (IsSpecialUIEffectExec(nExecID))
					{
                        if (AIDataHelper::getSpecialUIEffectExec(nExecID))
						    _animTick = AIDataHelper::getSpecialUIEffectExec(nExecID)->duration;
					}

					if (thePlayTick % _animTick == 0)
					{
						AIActionController::executeExec(aiContext, nExecID, skillID, skillLevel, theMover, nullptr, target, 0);
					}
				}
			}
			
		}
		else
		{
			int nExecID = 0;

			if (staticData->vMovingExecuteIds.size() > 0)
			{
				nExecID = staticData->vMovingExecuteIds[0];
			}

			if (nExecID > 0)
			{
				int _animTick = 1;

#if BATTLE_LOG_TEST
				// clair_wang test
				CCLOG("clair_wang Mover(%d, %d) Target(%d, %d) leftMax=%d, RightMax=%d", theMover->getX(), theMover->getY(), theTarget->getX(), theTarget->getY(), aiContext.getBattleFieldXMostLeft(), aiContext.getBattleFieldXMostRight());
#endif

				if (IsAnimationExec(nExecID))
				{
                    if (AIDataHelper::getSkillAnimationExec(nExecID))
					    _animTick = AIDataHelper::getSkillAnimationExec(nExecID)->duration;
				}
				else if (IsHurtExec(nExecID))
				{
                    if (AIDataHelper::getHurtExec(nExecID))
					    _animTick = AIDataHelper::getHurtExec(nExecID)->duration;
				}
				else if (IsSpecialUIEffectExec(nExecID))
				{
                    if (AIDataHelper::getSpecialUIEffectExec(nExecID))
					    _animTick = AIDataHelper::getSpecialUIEffectExec(nExecID)->duration;
				}

				if (thePlayTick % _animTick == 0)
				{
					AIActionController::executeExec(aiContext, nExecID, skillID, skillLevel, theMover, nullptr, target, 0);
				}
			}
		}
		
	}


	void AIContinuousMoveCaster::updateTrackTargetField(AIContinuousMoveCaster::MoverInfo& mover, int playedTick)
	{
		CharacterData* theMover = mover.mover;
		CharacterData* theTarget = mover.moversTarget;
		

		int targetX = mover.initTargetX;
		int targetY = mover.initTargetY;

		if (targetX != theTarget->getX())
		{
			targetX = theTarget->getX();
			mover.initTargetX = targetX;
			mover.initFaceDirectionX = targetX > theMover->getX() ? 1 : -1;
		}

		if (targetY != theTarget->getY())
		{
			targetY = theTarget->getY();

			if (theTarget->currentAction == STRUCK_FLY_AWAY_ACTION || theTarget->currentAction == STRUCK_HOVER_ACTION)
			{
				targetY = theTarget->oldY;
			}

			mover.initTargetY = targetY;
			mover.initFaceDirectionY = targetY > theMover->getY() ? 1 : -1;
		}

		targetX += mover.initFaceDirectionX * staticData->targetDistanceX;
		targetY += mover.initFaceDirectionY * staticData->targetDistanceY;

		bool bIsDeleleMoveX = false;
		bool bIsDeleleMoveY = false;
		auto nextX = theMover->getX() + (realSpeedX + realAccX * playedTick) * mover.initFaceDirectionX;
		nextX = aiContext.getAdjustedPosXWithinRange(nextX);
		
		targetX = aiContext.getAdjustedPosXWithinRange(targetX);

		static int flyYTick = (targetX - theMover->getX()) / ((realSpeedX + realAccX) * mover.initFaceDirectionY);
		if (flyYTick == 0 && playedTick == 0)
		{
			flyYTick = (targetX - theMover->getX()) / ((realSpeedX + realAccX) * mover.initFaceDirectionY);
		}

		if (mover.initFaceDirectionX == 1)
		{
			if (nextX < targetX)
			{
				theMover->setX(nextX);
			}
			else
			{
				theMover->setX(targetX);

				bIsDeleleMoveX = true;
			}
		}
		else 
		{
			if (targetX < nextX)
			{
				theMover->setX(nextX);
			}
			else
			{
				theMover->setX(targetX);

				bIsDeleleMoveX = true;
			}
		}

		
		auto nextY = (flyYTick > playedTick) ? (abs(targetY - theMover->getY()) / ((flyYTick - playedTick) * mover.initFaceDirectionY) + theMover->getY()) : targetY;

		if (targetY != theMover->getY())
		{
			if (mover.initFaceDirectionY == 1)
			{
				if (nextY < targetY)
				{
					theMover->setY(nextY);
				}
				else
				{
					theMover->setY(targetY);

					bIsDeleleMoveY = true;
				}
			}
			else
			{
				if (targetY < nextY)
				{
					theMover->setY(nextY);
				}
				else
				{
					theMover->setY(targetY);

					bIsDeleleMoveY = true;
				}
			}
		}
		else
		{
			bIsDeleleMoveY = true;
		}
		

		if (bIsDeleleMoveX && bIsDeleleMoveY)
		{
			// 移到指定位置不论时间是否未达，结束移动
			shouldDeleteSelf = true;

			flyYTick = 0; // clear
		}

		TodoMovingExecFunc(playedTick, theMover);

		// MARK: 移动中实时改变人物朝向，未严格测试
		// 角色改变状态时不清除的移动执行，不做自动转向，防止角色释放下一个技能寻找目标时反复转向
		if ((!staticData->keepWhenChangeAction))
		{
			if (theMover->direction == -1)
			{
				int faceDirection = theMover->getX() >= targetX ? -1 : 1;
				//theMover->lastDirection = theMover->direction;
				theMover->direction = faceDirection;
			}
			else
			{
				int faceDirection = theMover->getX() > targetX ? -1 : 1;
				//theMover->lastDirection = theMover->direction;
				theMover->direction = faceDirection;
			}
		}

	}

	void AIContinuousMoveCaster::updateCrossBattleFieldNew(AIContinuousMoveCaster::MoverInfo& mover, int playedTick)
	{
		//// 新型穿墙模式，先按初始面向，移动到该方向的战场边界，（若时间到，还未移动到战场边界，就原地停下）
		//// 再瞬间到另一个边界，重新进入战场，移动到初始面向方向的战场边界（若时间到，还未移动到战场边界，就原地停下）
		auto theMover = mover.mover;
		auto direction = mover.initFaceDirectionX;

		if (playedTick < totalTick)
		{
			if (mover.tickAtReturnPointX <= 0)
			{
				auto deltaX = theMover->getX() - mover.initTargetX;
				if (deltaX * direction >= 0)
				{
					auto startX = direction > 0 ? aiContext.getBattleFieldXMostLeft() : aiContext.getBattleFieldXMostRight();
					theMover->setX(startX);

					return;
				}
			}

			theMover->setX(theMover->getX() + (realSpeedX + realAccX * playedTick) * direction);
		}
		
		TodoMovingExecFunc(playedTick, theMover);
	}

	void AIContinuousMoveCaster::updateAbsolutePositionField(AIContinuousMoveCaster::MoverInfo& mover, int playedTick)
	{
		// = 9, 令目标移动到绝对坐标位置。
		auto theMover = mover.mover;
		
		const int flyTick = (realDuration - realReturnPause) / (staticData->needReturn ? 2 : 1);
		int targetHeroPosX = mover.initTargetX;
		int targetX = targetHeroPosX;

		targetX = aiContext.getAdjustedPosXWithinRange(targetX);

		int targetY = mover.initTargetY;

		auto direction = theMover->getX() < targetHeroPosX ? 1 : -1;


		if (staticData->needReturn)
		{
			if (staticData->bIsUseactionFreq > 0 && realActionFreq > 0)
			{
				int nTmpFlyTick = flyTick / maxMoveCnt;
				if (nTmpFlyTick < 1)
				{
					nTmpFlyTick = 1;
				}

				int nTmpPlayedTick = playedTick;
				if (nTmpPlayedTick >= realActionFreq)
				{
					nTmpPlayedTick %= realActionFreq;
				}
				nTmpPlayedTick += 1;


				if ((nTmpPlayedTick / realActionFreq) >= maxMoveCnt)
				{
					targetHeroPosX = mover.initMoverX;

					direction = theMover->getX() > targetHeroPosX ? 1 : -1;

					if (realActionFreq == nTmpPlayedTick)
					{
						theMover->setX(mover.initMoverX);
						theMover->setY(mover.initMoverY);

						// 有时间间隔的
						if (((playedTick + 1) % realActionFreq == 0) && ((playedTick + 1) / realActionFreq < maxMoveCnt))
						{
							TodoAfterEndFun(theMover);
						}
					}
					else if (totalTick != playedTick)
					{
						auto nextX = theMover->getX() - (realSpeedX + realAccX * (totalTick - playedTick)) * direction;
						nextX = aiContext.getAdjustedPosXWithinRange(nextX);

						theMover->setX(nextX);
						theMover->setY((mover.initMoverY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
					}
				}
				else
				{

					if (nTmpPlayedTick <= nTmpFlyTick)
					{
						if (nTmpFlyTick == nTmpPlayedTick)
						{
							theMover->setX(targetX);
							theMover->setY(targetY);
						}
						else if (nTmpFlyTick != playedTick)
						{
							auto nextX = theMover->getX() + (realSpeedX + realAccX * (nTmpFlyTick - playedTick)) * direction;
							nextX = aiContext.getAdjustedPosXWithinRange(nextX);

							theMover->setX(nextX);
							theMover->setY((targetY - theMover->getY()) / (nTmpFlyTick - playedTick) + theMover->getY());
						}
					}
					else if (nTmpPlayedTick > (realReturnPause / maxMoveCnt))
					{
						targetHeroPosX = mover.initMoverX;

						direction = theMover->getX() > targetHeroPosX ? 1 : -1;

						if (realActionFreq == nTmpPlayedTick)
						{
							theMover->setX(mover.initMoverX);
							theMover->setY(mover.initMoverY);
						}
						else if (totalTick != playedTick)
						{
							auto nextX = theMover->getX() - (realSpeedX + realAccX * (totalTick - playedTick)) * direction;
							nextX = aiContext.getAdjustedPosXWithinRange(nextX);

							theMover->setX(nextX);
							theMover->setY((mover.initMoverY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
						}
					}
				}
			}
			else
			{
				if (playedTick <= flyTick)
				{
					if (flyTick == playedTick)
					{
						theMover->setX(targetX);
						theMover->setY(targetY);
					}
					else
					{
						auto nextX = theMover->getX() + (realSpeedX + realAccX * (flyTick - playedTick)) * direction;
						nextX = aiContext.getAdjustedPosXWithinRange(nextX);

						theMover->setX(nextX);
						theMover->setY((targetY - theMover->getY()) / (flyTick - playedTick) + theMover->getY());
					}
				}
				else if (playedTick > flyTick + realReturnPause)
				{
					targetHeroPosX = mover.initMoverX;

					direction = theMover->getX() > targetHeroPosX ? 1 : -1;

					if (totalTick - 1 == playedTick)
					{
						theMover->setX(mover.initMoverX);
						theMover->setY(mover.initMoverY);
					}
					else if (totalTick != playedTick)
					{
						auto nextX = theMover->getX() - (realSpeedX + realAccX * (totalTick - playedTick)) * direction;
						nextX = aiContext.getAdjustedPosXWithinRange(nextX);

						theMover->setX(nextX);
						theMover->setY((mover.initMoverY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
					}
				}
			}
		}
		else
		{
			if (staticData->bIsUseactionFreq > 0 && realActionFreq > 0)
			{
				int nTmpPlayedTick = playedTick;
				if (nTmpPlayedTick >= realActionFreq)
				{
					nTmpPlayedTick %= realActionFreq;
				}
				nTmpPlayedTick += 1;


				if ((playedTick / realActionFreq) >= maxMoveCnt)
				{
					theMover->setX(targetX);
					theMover->setY(targetY);
				}
				else
				{
					if (nTmpPlayedTick % realActionFreq == 0)
					{
						theMover->setX(targetX);
						theMover->setY(targetY);
					}
					else
					{
						if (playedTick > 0 && playedTick % realActionFreq == 0)
						{
							theMover->setX(mover.initMoverX);
							theMover->setY(mover.initMoverY);
						}
						else if (totalTick != playedTick)
						{
							auto nextX = theMover->getX() + (realSpeedX + realAccX * playedTick) * direction;
							nextX = aiContext.getAdjustedPosXWithinRange(nextX);

							theMover->setX(nextX);
							theMover->setY((targetY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
						}
					}
				}

			}
			else
			{
				if (flyTick - 1 <= playedTick)
				{
					theMover->setX(targetX);
					theMover->setY(targetY);
				}
				else if (totalTick != playedTick)
				{
					auto nextX = theMover->getX() + (realSpeedX + realAccX * playedTick) * direction;
					nextX = aiContext.getAdjustedPosXWithinRange(nextX);

					theMover->setX(nextX);
					theMover->setY((targetY - theMover->getY()) / (totalTick - playedTick) + theMover->getY());
				}
			}
		}

		TodoMovingExecFunc(playedTick, theMover);

	}

	void AIContinuousMoveCaster::updateSpecialSummonBackMove(AIContinuousMoveCaster::MoverInfo& mover, int playedTick)
	{
		// = 10 召唤物以一定速度移动到召唤者身边（将移动执行moveCasts加在target身上,其它默认加在source身上）
		CharacterData* theMover = mover.mover;

		if (!theMover->needAdjusting) {
			shouldDeleteSelf = true;
		}

		TodoMovingExecFunc(playedTick, theMover);
	}

	void AIContinuousMoveCaster::OldInitCaster(AIContext& aiContext)
	{
        std::unique_ptr<std::list<CharacterData*> > currentTargets;
        if (AITargetHelper::isUseNewHelper())
        {
            currentTargets = std::unique_ptr<std::list<CharacterData*> >(new std::list<CharacterData*>);
            AITargetContext ctx;
            initMoveCasterTargetContextForOld(ctx, this);
            AITargetHelper::searchTarget(&ctx, currentTargets.get());
        }
        else
        {
            currentTargets = AIActionController::searchTarget(*origin, target, nullptr, aiContext, staticData->target, staticData->range[0], staticData->range[1], staticData->range[2], staticData->range[3], staticData->range[2], staticData->range[3], nullptr, *origin);
        }
        
        if (currentTargets)
        {
			CharacterData* moversTarget = origin;
            for (auto iter = currentTargets->begin(); iter != currentTargets->end(); ++iter)
			{
				if ((*iter)->myID == origin->myID)
				{
					moversTarget = target;
					break;
				}
			}

            for (auto iter = currentTargets->begin(); iter != currentTargets->end(); ++iter)
			{
				auto hero = *iter;

				if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *hero, eEFFECT_DEBUF_MOVE_EX, 0) > 0)
				{
					if (hero->teamSide != origin->teamSide)
						continue;
				}
				hero->moveCasterAffectedCount++;


				if (hero->hasEffect(303))
				{
					luahelper_actioncontroller_is_move_action_effect(aiContext.getLuaState(), hero, 0);
				}

				MoverInfo* info = new MoverInfo;
				info->mover = hero;
				info->moversTarget = moversTarget;
				info->initMoverX = hero->getX();
				if (hero->currentAction == STRUCK_FLY_AWAY_ACTION || hero->currentAction == STRUCK_HOVER_ACTION)
				{
					info->initMoverY = hero->oldY;
				}
				else
				{
					info->initMoverY = hero->getY();
				}

				info->initTargetX = moversTarget->getX();

				// 召唤执行与移动执行的目标，当目标被击飞时，取目标击飞前的Y轴坐标
				if (hero->currentAction == STRUCK_FLY_AWAY_ACTION || hero->currentAction == STRUCK_HOVER_ACTION)
				{
					info->initTargetY = moversTarget->oldY;
				}
				else
				{
					info->initTargetY = moversTarget->getY();
				}

				info->initFaceDirectionX = info->initTargetX > info->initMoverX ? 1 : -1;
				info->initFaceDirectionY = info->initTargetY > info->initMoverY ? 1 : -1;
				info->tickAtReturnPointX = 0;// only used in no-target-mode
				info->tickAtReturnPointY = 0;// only used in no-target-mode


				// 特殊穿墙模式
				if (staticData->targetTracing == (int)MoveType::kCrossBattleField)
				{
					moveType = MoveType::kCrossBattleField;

					if (hero->direction > 0)
						info->initTargetX = aiContext.getBattleFieldXMostRight();
					else
						info->initTargetX = aiContext.getBattleFieldXMostLeft();

					info->initFaceDirectionX = hero->direction;

					int dist = info->initMoverX + info->initTargetX - info->initMoverX*info->initFaceDirectionX;
					float oldtick = (float)(dist) / ((float)staticData->speedX);
					float parm = oldtick * (float)speedTickParam / 100;
					realSpeedX = dist / parm;
				}
				else if (staticData->targetTracing == (int)MoveType::kBlackHoleField)
				{
					moveType = MoveType::kBlackHoleField;

				/*	auto ittff = currentTargets->begin();
					bool _bIsSameDirt = true;
					int _CompDirt = (*ittff)->direction;
					for (; ittff != currentTargets->end(); ++ittff)
					{
						if (_CompDirt != (*ittff)->direction)
						{
							_bIsSameDirt = false;
							break;
						}
					}
					
					if (_bIsSameDirt)
					{
						_CompDirt = info->mover->direction;
					}
					else
					{
						_CompDirt = info->moversTarget->direction;
					}*/

					info->initTargetX += (staticData->speedX * info->moversTarget->direction);// 黑洞模式下，X移动速度与Y移动速度列分别代表X偏移和Y偏移
					info->initTargetY += staticData->speedY;// 黑洞模式下，X移动速度与Y移动速度列分别代表X偏移和Y偏移

					info->initTargetX = info->initTargetX < aiContext.getBattleFieldXMostLeft() ? aiContext.getBattleFieldXMostLeft() : info->initTargetX;
					info->initTargetX = info->initTargetX > aiContext.getBattleFieldXMostRight() ? aiContext.getBattleFieldXMostRight() : info->initTargetX;

					info->continualFlutterTick = realDuration;
				}
				else if (staticData->targetTracing == (int)MoveType::kRandomPointField)
				{
					// 移动执行：新增目标类型-朝随机地点移动，作用范围填移动随机的范围，每次随机移动时注意人物的面朝向

					moveType = MoveType::kRandomPointField;

					info->initTargetX = info->initMoverX + aiContext.localRandom->nextInt(staticData->range[0], staticData->range[1]) * info->mover->direction;
					info->initTargetY = info->initMoverY + aiContext.localRandom->nextInt(staticData->range[2], staticData->range[3]);

					info->initTargetX = info->initTargetX < aiContext.getBattleFieldXMostLeft() ? aiContext.getBattleFieldXMostLeft() : info->initTargetX;
					info->initTargetX = info->initTargetX > aiContext.getBattleFieldXMostRight() ? aiContext.getBattleFieldXMostRight() : info->initTargetX;

					info->initFaceDirectionX = hero->direction;
				}
				else if (staticData->targetTracing == (int)MoveType::kTrackTargetMove)
				{
					// = 7 追踪目标移动，直到目标停止，移动执行结束。
					int faceDirection = info->initTargetX > info->initMoverX ? 1 : -1;

					moveType = MoveType::kTrackTargetMove;

					//hero->lastDirection = hero->direction;
					hero->direction = faceDirection;

					info->initFaceDirectionX = hero->direction;
				}
				else if (staticData->targetTracing == (int)MoveType::kCrossBattlefieldNew)
				{
					// = 8  类似穿墙，但不一定回到原位，而是根据该类型的持续时间和移动速度决定终点位置。
					moveType = MoveType::kCrossBattlefieldNew;

					if (hero->direction > 0)
						info->initTargetX = aiContext.getBattleFieldXMostRight();
					else
						info->initTargetX = aiContext.getBattleFieldXMostLeft();

					info->initFaceDirectionX = hero->direction;
				}
				else if (staticData->targetTracing == (int)MoveType::KAbsolutePostionMove)
				{
					// 9 令目标移动到绝对坐标位置。
					moveType = MoveType::KAbsolutePostionMove;
					
					float mScreenCenterX = AIContext::getInstance()->battleFieldCenterX;
					float mScreenCenterY = (AIContext::getInstance()->getDeltaMaxY() - AIContext::getInstance()->getDeltaMinY()) / 2.0f;

					info->initFaceDirectionX = (hero->teamSide == TeamSide::kOur) ? 1 : -1;

					info->initTargetX = mScreenCenterX + info->initFaceDirectionX * staticData->targetDistanceX;
					info->initTargetY = mScreenCenterY + staticData->targetDistanceY;

					info->initTargetX = info->initTargetX < aiContext.getBattleFieldXMostLeft() ? aiContext.getBattleFieldXMostLeft() : info->initTargetX;
					info->initTargetX = info->initTargetX > aiContext.getBattleFieldXMostRight() ? aiContext.getBattleFieldXMostRight() : info->initTargetX;

					if (staticData->duration != 0)
					{
						realSpeedX = (abs(info->initTargetX - info->initMoverX)) / staticData->duration;
					}
				}
				else if (staticData->targetTracing == (int)MoveType::kSpecialSummonBackMove)
				{
					moveType = MoveType::kSpecialSummonBackMove;
					info->initTargetX += info->initFaceDirectionX * staticData->targetDistanceX;
					info->initTargetX = info->initTargetX < aiContext.getBattleFieldXMostLeft() ? aiContext.getBattleFieldXMostLeft() : info->initTargetX;
					info->initTargetX = info->initTargetX > aiContext.getBattleFieldXMostRight() ? aiContext.getBattleFieldXMostRight() : info->initTargetX;
					origin->needAdjusting = true;
					origin->adjustingX = info->initTargetX;
					origin->adjustingY = 100000000;
					if (staticData->speedX == 0)
					{
						origin->adjustingSpeedX = 9999;
					}
					else if (staticData->speedX > 0)
					{
						origin->adjustingSpeedX = staticData->speedX;
					}			
				}
				else if (totalTick != 0 && moveType == MoveType::kNormalHasTarget)
				{
					// 有目标移动执行时，先要把移动的人的朝向转到要移动的目标那里

					int faceDirection = info->initMoverX >= info->initTargetX ? -1 : 1;
					//hero->lastDirection = hero->direction;
					hero->direction = faceDirection;

				}
				else if (totalTick == 0) // no-target-mode
				{
					// MARK: no-target-mode
					// use speed, acc, stop-duration to calculate the move distance
					// not support Y to avoid affacting related calculating(eg:target locking)
					moveType = MoveType::kNormalNoTarget;

					totalTick = realDurationX;
					if (realDurationX > 0)
					{
						int moveDurationX = realDurationX - realReturnPause;
						
						if (staticData->needReturn)
						{
							moveDurationX = moveDurationX < 1 ? 2 : moveDurationX; //起码有一帧的动态位移+一帧的返回
						}
						else
						{
							moveDurationX = moveDurationX < 1 ? 1 : moveDurationX; //起码有一帧的动态位移
						}

						if (speedTickParam > 0 && speedTickParam != 100)
						{
							int oldDurX = staticData->durationX - staticData->returnPause;
							int oldTargetX = staticData->speedX * oldDurX + (staticData->accX * oldDurX * oldDurX) / 2;
							if (staticData->needReturn)
								oldTargetX /= 2;
							oldTargetX = info->initMoverX + oldTargetX * info->mover->direction;

							oldTargetX = oldTargetX < aiContext.getBattleFieldXMostLeft() ? aiContext.getBattleFieldXMostLeft() : oldTargetX;
							oldTargetX = oldTargetX > aiContext.getBattleFieldXMostRight() ? aiContext.getBattleFieldXMostRight() : oldTargetX;

							realSpeedX = oldTargetX / moveDurationX;
						}
						else
						{
							realSpeedX = staticData->speedX;
						}

						info->initTargetX = realSpeedX * moveDurationX + (staticData->accX * moveDurationX * moveDurationX) / 2;
						if (staticData->needReturn)
							info->initTargetX /= 2;
						info->initTargetX = info->initMoverX + info->initTargetX * info->mover->direction;

					}

					// 12.0 新增Y轴的移动
					if (realDurationY > 0)
					{
						if (totalTick < realDurationY)
						{
							totalTick = realDurationY;
						}
						int moveDurationY = realDurationY - realReturnPause;
						
						if (staticData->needReturn)
						{
							moveDurationY = moveDurationY < 1 ? 2 : moveDurationY; //起码有一帧的动态位移+一帧的返回
						}
						else
						{
							moveDurationY = moveDurationY < 1 ? 1 : moveDurationY; //起码有一帧的动态位移
						}

						if (speedTickParam > 0 && speedTickParam != 100)
						{
							int oldDurY = staticData->durationY - staticData->returnPause;
							int oldTargetY = staticData->speedY * oldDurY + (staticData->accY * oldDurY * oldDurY) / 2;
							if (staticData->needReturn)
								oldTargetY /= 2;
							oldTargetY = info->initMoverY + oldTargetY * info->mover->direction;

							realSpeedY = oldTargetY / moveDurationY;
						}
						else
						{
							realSpeedY = staticData->speedY;
						}

						info->initTargetY = realSpeedY * moveDurationY + (staticData->accY * moveDurationY * moveDurationY) / 2;
						if (staticData->needReturn)
							info->initTargetY /= 2;
						info->initTargetY = info->initMoverY + info->initTargetY * info->mover->direction;
					}

				}

				movers[hero->myID] = info;
			}

		}
	}
	
	void AIContinuousMoveCaster::ToArrangeMoveTypeData()
	{
		switch (staticData->targetTracing)
		{
		case (int)MoveType::kNormalHasTarget:
		case (int)MoveType::kNormalNoTarget:
		case (int)MoveType::kSpecialHasTarget:
		case (int)MoveType::kSpecialNoTarget:
		{
			if (totalTick == 0)
			{
				moveType = MoveType::kNormalNoTarget;
			}
			else
			{
				moveType = MoveType::kNormalHasTarget;
			}
		}
			break;
		case (int)MoveType::kCrossBattleField:
		{
			// 穿墙模式
			moveType = MoveType::kCrossBattleField;
		}
			break;
		case (int)MoveType::kBlackHoleField:
		{
			// = 3  黑洞模式  (将所有目标拉向一个坐标点位置，该坐标点位置相对于施法者（即施法者位置算0,0点），且一旦释放不随施法者转向而变化)
			moveType = MoveType::kBlackHoleField;
		}
			break;
		case (int)MoveType::kRandomPointField:
		{
			// = 6 朝随机地点移动，作用范围填移动随机的范围，每次随机移动时注意人物的面朝向
			moveType = MoveType::kRandomPointField;
		}
			break;
		case (int)MoveType::kTrackTargetMove:
		{
			// = 7 追踪目标移动，直到目标停止，移动执行结束。
			moveType = MoveType::kTrackTargetMove;
		}
			break;
		case (int)MoveType::kCrossBattlefieldNew:
		{
			// = 8  类似穿墙，但不一定回到原位，而是根据该类型的持续时间和移动速度决定终点位置。
			moveType = MoveType::kCrossBattlefieldNew;
		}
			break;
		case (int)MoveType::KAbsolutePostionMove:
		{
			// = 9, 令目标移动到绝对坐标位置。
			moveType = MoveType::KAbsolutePostionMove;
		}
			break;
		default:
			break;
		}
	}

	void AIContinuousMoveCaster::NewInitCaster(AIContext& aiContext)
	{
		ToArrangeMoveTypeData();
        
        std::unique_ptr<std::list<CharacterData*> > currentTargets;
        if (AITargetHelper::isUseNewHelper())
        {
            currentTargets = std::unique_ptr<std::list<CharacterData*> >(new std::list<CharacterData*>);
            AITargetContext ctx;
            initMoveCasterTargetContextForNew(ctx, this);
            AITargetHelper::searchTarget(&ctx, currentTargets.get());
        }
        else
        {
            currentTargets = AIActionController::searchTarget(*target, target->target, nullptr, aiContext, staticData->target, staticData->range[0], staticData->range[1], staticData->range[2], staticData->range[3], staticData->range[2], staticData->range[3], nullptr, *origin);
        }

        if (currentTargets)
		{
			auto hero = origin;

			if (hero->hasEffect(303))
			{
				luahelper_actioncontroller_is_move_action_effect(aiContext.getLuaState(), hero, 0);
			}

            for (auto itter = currentTargets->begin(); itter != currentTargets->end(); ++itter)
			{
				auto moverTarget = *itter;

				if (moveType == MoveType::kBlackHoleField || moveType == MoveType::KAbsolutePostionMove)
				{ 
					// 黑胡子黑洞 是敌动我不动
					hero = *itter;
					moverTarget = origin;
				}

				if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *hero, eEFFECT_DEBUF_MOVE_EX, 0) > 0)
				{
					if (hero->teamSide != moverTarget->teamSide)
						continue;
				}
				hero->moveCasterAffectedCount++;

				MoverInfo* info = new MoverInfo;
				info->mover = hero;
				info->moversTarget = moverTarget;
				info->initMoverX = hero->getX();

				if (hero->currentAction == STRUCK_FLY_AWAY_ACTION || hero->currentAction == STRUCK_HOVER_ACTION)
				{
					info->initMoverY = hero->oldY;
				}
				else
				{
					info->initMoverY = hero->getY();
				}

				info->initTargetX = moverTarget->getX();

				// 召唤执行与移动执行的目标，当目标被击飞时，取目标击飞前的Y轴坐标
				if (hero->currentAction == STRUCK_FLY_AWAY_ACTION || hero->currentAction == STRUCK_HOVER_ACTION)
				{
					info->initTargetY = moverTarget->oldY;
				}
				else
				{
					info->initTargetY = moverTarget->getY();
				}

				info->initFaceDirectionX = info->initTargetX > info->initMoverX ? 1 : -1;
				info->initFaceDirectionY = info->initTargetY > info->initMoverY ? 1 : -1;
				info->tickAtReturnPointX = 0;// only used in no-target-mode
				info->tickAtReturnPointY = 0;// only used in no-target-mode

				switch (moveType)
				{
				case MoveType::kNormalHasTarget:
				{
					// 有目标移动执行时，先要把移动的人的朝向转到要移动的目标那里

					int faceDirection = info->initMoverX >= info->initTargetX ? -1 : 1;
					//hero->lastDirection = hero->direction;
					hero->direction = faceDirection;
				}
					break;
				case MoveType::kNormalNoTarget:
				{
					// 普通无目标模式
					totalTick = realDurationX;

					if (realDurationX > 0)
					{
						int moveDurationX = realDurationX - realReturnPause;

						if (staticData->needReturn)
						{
							moveDurationX = moveDurationX < 1 ? 2 : moveDurationX; //起码有一帧的动态位移+一帧的返回
						}
						else
						{
							moveDurationX = moveDurationX < 1 ? 1 : moveDurationX; //起码有一帧的动态位移
						}

						if (speedTickParam > 0 && speedTickParam != 100)
						{
							int oldDurX = staticData->durationX - staticData->returnPause;
							int oldTargetX = staticData->speedX * oldDurX + (staticData->accX * oldDurX * oldDurX) / 2;
							if (staticData->needReturn)
								oldTargetX /= 2;
							oldTargetX = info->initMoverX + oldTargetX * info->mover->direction;

							oldTargetX = oldTargetX < aiContext.getBattleFieldXMostLeft() ? aiContext.getBattleFieldXMostLeft() : oldTargetX;
							oldTargetX = oldTargetX > aiContext.getBattleFieldXMostRight() ? aiContext.getBattleFieldXMostRight() : oldTargetX;


							realSpeedX = oldTargetX / moveDurationX;
						}
						else
						{
							realSpeedX = staticData->speedX;
						}

						info->initTargetX = realSpeedX * moveDurationX + (staticData->accX * moveDurationX * moveDurationX) / 2;
						if (staticData->needReturn)
							info->initTargetX /= 2;
						info->initTargetX = info->initMoverX + info->initTargetX * info->mover->direction;
					}

					// 12.0 新增Y轴的移动
					if (realDurationY > 0)
					{
						if (totalTick < realDurationY)
						{
							totalTick = realDurationY;
						}

						int moveDurationY = realDurationY - realReturnPause;

						if (staticData->needReturn)
						{
							moveDurationY = moveDurationY < 1 ? 2 : moveDurationY; //起码有一帧的动态位移+一帧的返回
						}
						else
						{
							moveDurationY = moveDurationY < 1 ? 1 : moveDurationY; //起码有一帧的动态位移
						}

						if (speedTickParam > 0 && speedTickParam != 100)
						{
							int oldDurY = staticData->durationY - staticData->returnPause;
							int oldTargetY = staticData->speedY * oldDurY + (staticData->accY * oldDurY * oldDurY) / 2;
							if (staticData->needReturn)
								oldTargetY /= 2;
							oldTargetY = info->initMoverY + oldTargetY * info->mover->direction;

							realSpeedY = oldTargetY / moveDurationY;
						}
						else
						{
							realSpeedY = staticData->speedY;
						}

						info->initTargetY = realSpeedY * moveDurationY + (staticData->accY * moveDurationY * moveDurationY) / 2;
						if (staticData->needReturn)
							info->initTargetY /= 2;
						info->initTargetY = info->initMoverY + info->initTargetY * info->mover->direction;
					}

				}
					break;
				case MoveType::kCrossBattleField:
				{
					if (hero->direction > 0)
						info->initTargetX = aiContext.getBattleFieldXMostRight();
					else
						info->initTargetX = aiContext.getBattleFieldXMostLeft();

					info->initFaceDirectionX = hero->direction;

					int dist = info->initMoverX + info->initTargetX - info->initMoverX*info->initFaceDirectionX;
					float oldtick = (float)(dist) / ((float)staticData->speedX);
					float parm = oldtick * (float)speedTickParam / 100;
					realSpeedX = dist / parm;
				}
					break;
				case MoveType::kBlackHoleField:
				{
					// 修改后黑洞的xy位置，取的是施法者当前面朝方向的正负。
					info->initTargetX += (staticData->speedX * info->moversTarget->direction);// 黑洞模式下，X移动速度与Y移动速度列分别代表X偏移和Y偏移
					info->initTargetY += staticData->speedY;// 黑洞模式下，X移动速度与Y移动速度列分别代表X偏移和Y偏移

					info->initTargetX = info->initTargetX < aiContext.getBattleFieldXMostLeft() ? aiContext.getBattleFieldXMostLeft() : info->initTargetX;
					info->initTargetX = info->initTargetX > aiContext.getBattleFieldXMostRight() ? aiContext.getBattleFieldXMostRight() : info->initTargetX;

					info->continualFlutterTick = realDuration;
				}
					break;
				case MoveType::kRandomPointField:
				{
					// 移动执行：新增目标类型-朝随机地点移动，作用范围填移动随机的范围，每次随机移动时注意人物的面朝向

					info->initTargetX = info->initMoverX + aiContext.localRandom->nextInt(staticData->range[0], staticData->range[1]) * info->mover->direction;
					info->initTargetY = info->initMoverY + aiContext.localRandom->nextInt(staticData->range[2], staticData->range[3]);

					info->initTargetX = info->initTargetX < aiContext.getBattleFieldXMostLeft() ? aiContext.getBattleFieldXMostLeft() : info->initTargetX;
					info->initTargetX = info->initTargetX > aiContext.getBattleFieldXMostRight() ? aiContext.getBattleFieldXMostRight() : info->initTargetX;

					info->initFaceDirectionX = hero->direction;
				}
					break;
				case MoveType::kTrackTargetMove:
				{
					// = 7 追踪目标移动，直到目标停止，移动执行结束。
					int faceDirection = info->initTargetX > info->initMoverX ? 1 : -1;

					//hero->lastDirection = hero->direction;
					hero->direction = faceDirection;

					info->initFaceDirectionX = hero->direction;
				}
					break;
				case MoveType::kCrossBattlefieldNew:
				{
					// = 8  类似穿墙，但不一定回到原位，而是根据该类型的持续时间和移动速度决定终点位置。

					if (hero->direction > 0)
						info->initTargetX = aiContext.getBattleFieldXMostRight();
					else
						info->initTargetX = aiContext.getBattleFieldXMostLeft();

					info->initFaceDirectionX = hero->direction;
				}
					break;
				case MoveType::KAbsolutePostionMove:
				{
					float mScreenCenterX = AIContext::getInstance()->battleFieldCenterX;
					float mScreenCenterY = (AIContext::getInstance()->getDeltaMaxY() - AIContext::getInstance()->getDeltaMinY()) / 2.0f;

					info->initFaceDirectionX = (hero->teamSide == TeamSide::kOur) ? 1 : -1;

					info->initTargetX = mScreenCenterX + info->initFaceDirectionX*staticData->targetDistanceX;
					info->initTargetY = mScreenCenterY + staticData->targetDistanceY;

					if (staticData->duration != 0)
					{
						realSpeedX = (abs(info->initTargetX - info->initMoverX)) / staticData->duration;
					}
				}
					break;
				default:
					break;
				}

				movers[hero->myID] = info;

			}
		}

	}

} // namespace OnePiece
