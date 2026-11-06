//
//  AIContinuousSummonCaster.cpp
//  op_game
//
//  Created by Ping Wang on 15/5/5.
//
//

#include "AIContinuousSummonCaster.h"
#include <algorithm>
#include "AIContext.h"
#include "action_hero/HeroActionController.h"
#include "action_hero/HeroSkillController.h"
#include "scriptbind/AIScriptBindSummonCaster.hpp"

namespace OnePiece{
    
    AIContinuousSummonCaster::AIContinuousSummonCaster(AIContext& aiContext, CharacterData* origin, CharacterData* target, SummonExec* pSummonExec, int skillLv, int innerExecIndex)
        : AINode(origin->myID, origin->clusterNum), aiContext(aiContext), origin(origin), target(target), summonExec(pSummonExec), skillLevel(skillLv), _innerExecIndex(innerExecIndex)
    {
        myID = aiContext.local_incremential++;
        _summonID = pSummonExec->execs[innerExecIndex].summonId;
        summonProperty = AIDataHelper::getSummonProperty(_summonID);
        
        localTick = aiContext.currentTick;
        startTick = localTick;
        totalTick = summonProperty->durationTick;
        
        ignoreFreeze = origin->isCaptain();

        // MARK: use script from 14.0
        //auto characterList = origin->getOriginFriends();
        //for (auto pCharacter : *characterList)
        //{
        //    if (pCharacter && pCharacter->isSummon() && pCharacter->summonProperty->summonId == _summonID)
        //    {
        //        _summonPool.push_back(pCharacter);
        //    }
        //}

        luahelper_summoncaster_init_summonpool(aiContext.getLuaState(), this, origin, _summonID);
        initSummonExecTodo(pSummonExec->execs[innerExecIndex]);
        
        origin->summonCasts.push_back(this);

#ifdef OPTOOLS
        // quanhao.liu 2023/8/22
        SummonInfoLog* summonInfoLog = new SummonInfoLog();
        summonInfoLog->summonID = _summonID;
        summonInfoLog->myID = myID;
        summonInfoLog->currentTick = aiContext.currentTick;
        summonInfoLog->originID = origin->getID();
        summonInfoLog->originName = origin->staticData->name;

       aiContext.incSkillTriggerCount(aiContext.currentSkillID);
       SkillInfoLog* skillInfoLog = aiContext.getSkillInfoLog(aiContext.currentSkillID, skillLevel, myID);
       skillInfoLog->summonInfoLogs.push_back(summonInfoLog);
       aiContext.summonInfoLogsMap.insert({ myID, summonInfoLog });
       aiContext.caseterFindIndexMap.insert({ myID, skillInfoLog->summonInfoLogs.size() - 1 });
       aiContext.casterFindSkillIndexMap.insert({ myID, aiContext.getSkillInfoLogsIndex(aiContext.currentSkillID) });
#endif
    }
    
    AIContinuousSummonCaster::~AIContinuousSummonCaster()
    {
        removeSummons();
        
        for (std::list<AIContinuousSummonCaster*>::iterator iter = origin->summonCasts.begin(); iter != origin->summonCasts.end();)
        {
            if (*iter == this)
            {
                origin->summonCasts.erase(iter);
                break;
            }
            iter++;
        }
        
    }

    void AIContinuousSummonCaster::initSummonPool(AIContinuousSummonCaster *caster, CharacterData *summoner, int summonId, int poolType)
    {
        CharacterDataListPtr characterList = nullptr;
        if (poolType == 0)
            characterList = summoner->getOriginFriends();
        else if (poolType == 1)
            characterList = summoner->getOriginRivals();
        else if (poolType == 2)
            characterList = summoner->getSummonList();

        if (characterList) {
            for (CharacterData *c : *characterList)
            {
                if (c && c->isSummon() && c->summonProperty->summonId == summonId)
                {
                    caster->_summonPool.push_back(c);
                }
            }
        }
    }
    
    void AIContinuousSummonCaster::Excute(AIContext& aiContext)
    {
#ifdef OPTOOLS
        int tmpCurrentHPDeltaCasterMyId = aiContext.currentHPDeltaCasterMyId;
        aiContext.currentHPDeltaCasterMyId = this->myID;
        int tmpCurrentHPDeltaCasterId = aiContext.currentHPDeltaCasterId;
        aiContext.currentHPDeltaCasterId = this->_summonID;
#endif
		AINode::update(aiContext);
		
		int deltaTick = localTick - startTick - 1;
		if ((totalTick == 0 && deltaTick > totalTick)
			|| (totalTick != 0 && deltaTick >= totalTick))
        {
            delete this;
            return;
        }

        if (summonProperty->deadClear && HeroActionController::isDeadAction(origin->currentAction))
        {
            delete this;
            return;
        }

        auto& innerExec = summonExec->execs[_innerExecIndex];

        if (deltaTick == innerExec.showSummonInterval)
        {
            int toSummonCount = innerExec.summonNum;
            if (summonExec->groupID > 0)
            {
                int groupAvailableCnt = luahelper_summoncaster_get_group_summon_available_count(aiContext.getLuaState(), this, origin);
                if (groupAvailableCnt >= 0)
                {
                    toSummonCount = std::min(toSummonCount, groupAvailableCnt);
                    if (toSummonCount <= 0)
                        return;
                }
            }

            //auto characterList = origin->getOriginFriends();
            int unactiveCnt = 0;
            for (auto ite = _summonPool.begin(); ite != _summonPool.end(); ++ite)
            {
                auto pCharacter = *ite;
                if (pCharacter && pCharacter->isSummon() && pCharacter->summonProperty->summonId == _summonID)
                {
                    if (!pCharacter->isSummonActive)
                        unactiveCnt++;
                }
            }

            if (innerExec.oneSample && unactiveCnt < toSummonCount)
            {
                /*
                int totalNum = innerExec.summonNum - unactiveCnt;
                int removedNum = 0;
                int toRemoveNum = totalNum;

                for (auto summonIter = origin->summonCasts.begin(); summonIter != origin->summonCasts.end(); ++summonIter)
                {
                    auto theSummonCaster = *summonIter;
                    if (!theSummonCaster || theSummonCaster->myID == this->myID)
                        continue;

                    removedNum += theSummonCaster->removeSummon(_summonID, toRemoveNum);
                    if (removedNum >= totalNum)
                        break;

                    toRemoveNum = totalNum - removedNum;
                }
                */
                int totalNum = toSummonCount - unactiveCnt;
                sleepCasterSummon(this->origin, _summonID, totalNum, this->myID);
            }

            int awakeNum = 0;
            for (auto pCharacter : _summonPool)
            {
                if (pCharacter->isSummonActive)
                    continue;

                pCharacter->resetSummonData();
                pCharacter->currentAction = ActionType::IDLE_ACTION;
                pCharacter->isSummonActive = true;
                pCharacter->setFakeSummonType(summonExec->fakeSummon);
                initSummonPos(pCharacter);
                summonUIDList.remove(pCharacter->myID);
                summonUIDList.push_back(pCharacter->myID);
                pCharacter->onSummoned();

                // TODO: on charmed
                //if (origin->isInCharmEffect())
                //{
                //    pCharacter->inCharmEffect();
                //}
                //else if (origin->isInHalfCharmEffect())
                //{
                //    pCharacter->inHalfCharmEffect();
                //}
                //else
                //{
                //    pCharacter->outCharmEffect();
                //    pCharacter->outHalfCharmEffect();
                //}

                if (pCharacter->hasTag("summonincharm253"))
                {
                    pCharacter->eraseTag("summonincharm253");
                    if (aiContext.seffProxy != nullptr)
                    {
                        aiContext.seffProxy->notify(aiContext, pCharacter->myID);
                    }
                }

                if (pCharacter->getFakeSummonType() != 3 && AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *origin, eEFFECT_TAUNT_EXTRA, 0) > 0 && origin->hasTag("charm253includesummon"))
                {
                    pCharacter->addTag("summonincharm253");
                    AIActionController::executeSpecialEffect(aiContext, pCharacter->myID, 61575, skillLevel, -1, pCharacter, nullptr, pCharacter, totalTick - deltaTick);
                }

                for (auto &&skillPair : pCharacter->skillList)
                {
                    auto skillID = skillPair.first;
                    auto skillLevel = skillPair.second;
                    AISkill *aSkill = AIDataHelper::getSkillData(skillID);
                    if (aSkill && aSkill->castCondition == ESkillCastCondition::eSkillCastConditionOnFightBegin && !HeroActionController::isDeadAction(pCharacter->currentAction))
                    {
                        HeroSkillController::executeExecIDInTick(aiContext, skillID, skillLevel, 0, *pCharacter, *pCharacter->target);
                    }
                }

                if (aiContext.summonProxy)
                {
                    aiContext.summonProxy->awakeSummon(aiContext, origin, pCharacter->myID);
                }

                luahelper_summoncaster_on_summon_awake(aiContext.getLuaState(), this, pCharacter, _summonID);

                awakeNum++;

                if (awakeNum >= toSummonCount)
                    break;
            }

            if (innerExec.oneSample && innerExec.maxSummonNum > 0)
            {
                int sameActiveSummonCount = 0;
                for (auto pCharacter : _summonPool)
                {
                    if (pCharacter && pCharacter->isSummonActive)
                    {
                        sameActiveSummonCount++;
                    }
                }

                if (sameActiveSummonCount > innerExec.maxSummonNum)
                {
                    /*
                    int totalNum = sameActiveSummonCount - innerExec.maxSummonNum;
                    int removedNum = 0;
                    int toRemoveNum = totalNum;

                    for (auto summonIter = origin->summonCasts.begin(); summonIter != origin->summonCasts.end(); ++summonIter)
                    {
                        auto theSummonCaster = *summonIter;
                        if (!theSummonCaster || theSummonCaster->myID == this->myID)
                            continue;

                        removedNum += theSummonCaster->removeSummon(_summonID, toRemoveNum);
                        if (removedNum >= totalNum)
                            break;

                        toRemoveNum = totalNum - removedNum;
                    }
                    */
                    int totalNum = sameActiveSummonCount - innerExec.maxSummonNum;
                    sleepCasterSummon(this->origin, _summonID, totalNum, this->myID);
                }
            }

            if (summonExec->groupID > 0)
            {
                int maxGroupSummonNum = summonExec->groupMaxNum;
                int currentGroupSummonCount = 0;
                for (AIContinuousSummonCaster *a_caster : origin->summonCasts) {
                    if (a_caster && a_caster->summonExec->groupID == summonExec->groupID)
                        currentGroupSummonCount += a_caster->getActiveCount();
                }
                luahelper_summoncaster_check_sleep_group_summon(aiContext.getLuaState(), this, origin, summonExec->ID, summonExec->groupID, summonExec->groupWeight, summonExec->groupMaxNum, currentGroupSummonCount);
            }

            luahelper_summoncaster_on_finish_awake_summon(aiContext.getLuaState(), myID, origin);
        }

        /*
        for (int i = 0; i < 3; ++i)
        {
            int execID = innerExec.specialExec[i];
            if (execID > 0 && deltaTick == innerExec.specialExecInterval[i])
                summonDoExec(execID);
        }

        for (int i = 0; i < innerExec.extraExecs.size(); ++i)
        {
            auto& execPair = innerExec.extraExecs[i];
            int execID = execPair.first;
            int interval = execPair.second;
            if (execID > 0 && deltaTick == interval)
                summonDoExec(execID);
        }
        */

        /*
        for (int i = 0; i < innerExec.execs.size(); ++i)
        {
            auto& execPair = innerExec.execs[i];
            int execID = execPair.first;
            int interval = execPair.second;
            if (execID > 0 && deltaTick == interval)
                summonDoExec(execID);
        }
        */

        if (summonUIDList.size() > 0)
        {
            auto iter = _summonExecTodo.find(deltaTick);
            if (iter != _summonExecTodo.end()) {
                std::list<int> &execs = iter->second;
                for (int execID : execs)
                    summonDoExec(execID);
            }
        }
#ifdef OPTOOLS
        aiContext.currentHPDeltaCasterMyId = tmpCurrentHPDeltaCasterMyId;
        aiContext.currentHPDeltaCasterId = tmpCurrentHPDeltaCasterId;
#endif
    }

    /*
	void AIContinuousSummonCaster::beforeShowSummon()
	{
		if (summonExec == NULL)
			return;

		if (summonProperty == NULL)
			return;

		SummonProperty* pSummonProperty = AIDataHelper::getSummonProperty(summonExec->execs[_innerExecIndex].summonId);
		if (pSummonProperty) {
            beforeAwakeSummons(pSummonProperty, summonExec->execs[_innerExecIndex]);
		}
	}
    */
    
    /*
	void AIContinuousSummonCaster::beforeAwakeSummons(const SummonProperty* pSummonProperty, const SummonInnerExec& summonInnerExec)
	{
		std::list<CharacterData*> *characterList = NULL;

        characterList = origin->getCurrentFriendsList();

		//统计现在场面上存在多少个目标镜像 多少个休眠镜像
		int targetSummonCharacterId = 0;

		if (pSummonProperty->target == 0)
		{
			targetSummonCharacterId = origin->getID();
		}
		else
		{
			targetSummonCharacterId = pSummonProperty->targetId;
		}

        unsigned short unactiveCnt = 0;
		for (std::list<CharacterData*>::iterator ite = characterList->begin(); ite != characterList->end(); ++ite) {
            auto pCharacter = *ite;
            if (pCharacter && pCharacter->isSummon() && pCharacter->getID() == targetSummonCharacterId && pCharacter->m_pSummonProperty->summonId == summonInnerExec.summonId) {
                if (!pCharacter->m_bSummonActive) {
					//CCLOG("beforeAwakeSummons ID[%d]=(%d, %d)", targetSummonCharacterId, (*ite)->getX(), (*ite)->getY());
					//m_unActiveSummonList.push_back(*ite);
                    unactiveCnt++;
				}
			}
		}

        //CCLOG("before remove _unActiveSummonList size = %u", unactiveCnt);
		//首先从剩余幻象中唤醒
		//那么只存在两种情况 够 不够

        if (summonInnerExec.oneSample && unactiveCnt < summonInnerExec.summonNum) {
			//不够
            int num = (int)(summonInnerExec.summonNum - unactiveCnt);

			origin->removeTargetSummonInSummonCaster(targetSummonCharacterId, num, myID);
		}

        m_unActiveSummonList.clear();
        for (auto pCharacter : *characterList) {
            if (pCharacter && pCharacter->isSummon() && pCharacter->getID() == targetSummonCharacterId && pCharacter->m_pSummonProperty->summonId == summonInnerExec.summonId && !pCharacter->m_bSummonActive) {
                m_unActiveSummonList.push_back(pCharacter);
            }
        }

		//CCLOG("after remove_unActiveSummonList size = %ld", m_unActiveSummonList.size());

		int awakeNum = 0;

		for (std::list<CharacterData*>::iterator ite = m_unActiveSummonList.begin(); ite != m_unActiveSummonList.end(); ++ite) {
			(*ite)->resetSummonData();
			setSummonPos(pSummonProperty, *ite);
			++awakeNum;

			if (awakeNum >= summonInnerExec.summonNum) {
				break;
			}
		}
	}
    */

    /*
    void AIContinuousSummonCaster::awakeSummons(const SummonProperty* pSummonProperty,const SummonInnerExec& summonInnerExec)
    {
        int awakeNum = 0;
        
		for (auto ite = m_unActiveSummonList.begin(); ite != m_unActiveSummonList.end(); ++ite) {
            
            auto pSummon = *ite;

            //准备唤醒
            //要设置下幻影的位置
            
            if (pSummon->m_bSummonActive)
                CCLOG("MAY BE Error~~~~~~");
            
			//pSummon->resetSummonData();
            pSummon->currentAction = ActionType::IDLE_ACTION;
            pSummon->m_bSummonActive = true;
            pSummon->onSummoned();
            //setSummonPos(pSummonProperty, pSummon);
            //m_SummonList.push_back(pSummon);
            summonUIDList.push_back(pSummon->myID);
            
			//CCLOG("clair_wang awakeSummons ID[%d] =(%d,%d)", (*ite)->attributes[0], (*ite)->getX(), (*ite)->getY());

            if(aiContext.summonProxy)
            {
                aiContext.summonProxy->awakeSummon(aiContext, origin, pSummon->myID);
            }
            
            ++awakeNum;
            
            if (awakeNum >= summonInnerExec.summonNum)
                break;
        }
    }
    */
    
    /*
    void AIContinuousSummonCaster::setSummonPos(const SummonProperty* pSummonProperty,CharacterData* pSummon)
    {
        int x = pSummonProperty->rx - pSummonProperty->lx;
        int y = pSummonProperty->ry - pSummonProperty->ly;
        
        OutPutFileLine;
        int pX = aiContext.localRandom->nextInt(0,x);
        int pY = aiContext.localRandom->nextInt(0,y);
        
		// clair_wang 召唤物无论如何不应该召到场外。超出去的话就退到边界。	
		int _stX = target->getX() + (pX + pSummonProperty->lx) * target->direction;
		int _stY = target->getY() + pY + pSummonProperty->ly;

		_stX = aiContext.getAdjustedPosXWithinRange(_stX);

		// 当目标被击飞时，取目标击飞前的Y轴坐标
		if (target->currentAction == STRUCK_FLY_AWAY_ACTION || target->currentAction == STRUCK_HOVER_ACTION)
		{
			_stY = target->oldY + pY + pSummonProperty->ly;
		}

		pSummon->setX(_stX);
        pSummon->setY(_stY);
        pSummon->setZ(pSummon->getY());

        pSummon->adjustingX = target->adjustingX;
        pSummon->adjustingY = target->adjustingY;
        pSummon->adjustingZ = target->adjustingZ;
    }
    */

    void AIContinuousSummonCaster::initSummonExecTodo(SummonInnerExec &theExec)
    {
        for (int i = 0; i < theExec.execs.size(); ++i)
        {
            std::pair<int, int> &execPair = theExec.execs[i];
            int execID = execPair.first;
            int interval = execPair.second;
            if (execID > 0)
                _summonExecTodo[interval].push_back(execID);
        }
    }
    
    void AIContinuousSummonCaster::initSummonPos(CharacterData *theSummon)
    {
        if (summonProperty->posType == 0)
        {
            auto theTargetForPos = target;
            if (!target)
                theTargetForPos = origin;

            int distx = summonProperty->rx - summonProperty->lx;
            int disty = summonProperty->ry - summonProperty->ly;

            OutPutFileLine;
            int x = 0;
            if (distx != 0)
            {
                x = aiContext.localRandom->nextInt(0, distx);
            }
            int y = 0;
            if (disty != 0)
            {
                y = aiContext.localRandom->nextInt(0, disty);
            }

            x = theTargetForPos->getX() + (x + summonProperty->lx) * theTargetForPos->direction;
            x = aiContext.getAdjustedPosXWithinRange(x);

            // 当目标被击飞时，取目标击飞前的Y轴坐标
            if (theTargetForPos->currentAction == STRUCK_FLY_AWAY_ACTION || theTargetForPos->currentAction == STRUCK_HOVER_ACTION)
                y = theTargetForPos->oldY + y + summonProperty->ly;
            else
                y = theTargetForPos->getY() + y + summonProperty->ly;

            theSummon->setX(x);
            theSummon->setY(y);
            theSummon->setZ(theSummon->getY());

            theSummon->adjustingX = theTargetForPos->adjustingX;
            theSummon->adjustingY = theTargetForPos->adjustingY;
            theSummon->adjustingZ = theTargetForPos->adjustingZ;

            theSummon->oldY = y;

            theSummon->onScriptSetX(theSummon->getSummonSource()->getScriptOriginX());
            theSummon->onScriptSetY(theSummon->getSummonSource()->getScriptOriginY());
        }
        else if (summonProperty->posType == 1)
        {
            int theDirection = origin->teamSide == TeamSide::kOur ? 1 : -1;
            int x = aiContext.getBattleFieldCenterX() + summonProperty->lx * theDirection;
            x = aiContext.getAdjustedPosXWithinRange(x);
            int y = (aiContext.getDeltaMaxY() + aiContext.getDeltaMinY()) / 2.0f + summonProperty->ly;

            theSummon->setX(x);
            theSummon->setY(y);
            theSummon->setZ(theSummon->getY());

            theSummon->adjustingX = 100000000;
            theSummon->adjustingY = 100000000;
            theSummon->adjustingZ = 100000000;

            theSummon->oldY = y;

            theSummon->onScriptSetX(theSummon->getSummonSource()->getScriptOriginX());
            theSummon->onScriptSetY(theSummon->getSummonSource()->getScriptOriginY());
        }
    }

    void AIContinuousSummonCaster::summonDoExec(int execID)
    {
        int oldx = origin->getX();
        int oldy = origin->getY();

        for (auto iter = summonUIDList.begin(); iter != summonUIDList.end(); ++iter)
        {
            CharacterData *pSummon = aiContext.getCharacterByID(*iter);
            if (!pSummon || !pSummon->isSummonActive)
                continue;

            origin->setX(pSummon->getX());
            origin->setY(pSummon->getY());

            auto summonTarget = pSummon->lockTarget(aiContext);
            HeroActionController::setCurrentTarget(*pSummon, summonTarget);

            AIActionController::executeExec(aiContext, execID, 0, skillLevel, origin, nullptr, pSummon, 0, true);
        }

        origin->setX(oldx);
        origin->setY(oldy);
    }
    
    /*
    bool AIContinuousSummonCaster::isTargetSummon(const SummonProperty* pSummonProperty,CharacterData* pCharacterData)
    {
        if( pSummonProperty->target == 0)
        {
            return pCharacterData->isSummon() && pCharacterData->getID() == origin->getID();
        }else
        {
            return pCharacterData->isSummon() && pCharacterData->getID() == pSummonProperty->targetId;
        }
    }
    */
    
    /*
    void AIContinuousSummonCaster::showSummons()
    {
        
		if (summonExec == NULL)
            return;

		if (summonProperty == NULL)
			return;

        SummonProperty* pSummonProperty = AIDataHelper::getSummonProperty(summonExec->execs[_innerExecIndex].summonId);
        if (pSummonProperty) {
            awakeSummons(pSummonProperty, summonExec->execs[_innerExecIndex]);
        }
    }
    */

    void AIContinuousSummonCaster::removeSummons()
    {
        for (auto ite = summonUIDList.begin(); ite != summonUIDList.end(); ++ite) {
            CharacterData *pData = aiContext.getCharacterByID(*ite);
            if (!pData)
                continue;

            if (pData->hasTag("summonincharm253"))
            {
                pData->eraseTag("summonincharm253");
                if (aiContext.seffProxy)
                    aiContext.seffProxy->notify(aiContext, pData->myID);
            }

            if (pData->isSummonActive && !pData->hasTag("summonkilled")) {
                /*
                //持续时间结束 还没死亡的
                if (pData->m_pSummonProperty) {
                    for (auto ite = pData->m_pSummonProperty->deadRunVec.begin(); ite != pData->m_pSummonProperty->deadRunVec.end(); ++ite) {
                        if ((*ite).deadRunCondition == 2)
                        {
                            if ((*ite).run1)
                                AIActionController::executeExec(aiContext, (*ite).run1, 0, pData->getLv(), origin, NULL, pData, 0);

                            if ((*ite).run2)
                                AIActionController::executeExec(aiContext, (*ite).run2, 0, pData->getLv(), origin, NULL, pData, 0);
                        }
                    }
                }
                */

                if (!aiContext.hasWinlose())
                {
                    checkExcuteDeadRun(aiContext, pData, 2);
                    checkExcuteDeadRun(aiContext, pData, 4);

                    auto theFriends = pData->getCurrentFriendsList();
                    for (auto aFriend : *theFriends)
                    {
                        aFriend->onFriendSummonTimedOut(pData);
                    }
                }
            }

            pData->isSummonActive = false;
            pData->currentAction = ActionType::DEAD_ACTION1;

            pData->exitDuel();
            pData->resetSummonData();

            if (aiContext.summonProxy)
                aiContext.summonProxy->sleepSummon(aiContext, origin, pData->myID);
        }
    }

    /*
    int AIContinuousSummonCaster::removeSummonByID(int characterID, int removeNum)
    {
        if (removeNum <= 0)
            return 0;

        int removedCount = 0;
        for (auto iter = summonUIDList.begin(); iter != summonUIDList.end();)
        {
            CharacterData *pData = aiContext.getCharacterByID(*iter);
            if (!pData || pData->getID() != characterID || !pData->m_bSummonActive)
            {
                ++iter;
                continue;
            }

            if (pData->m_pSummonProperty)
            {
                for (auto ite = pData->m_pSummonProperty->deadRunVec.begin(); ite != pData->m_pSummonProperty->deadRunVec.end(); ++ite)
                {
                    if ((*ite).deadRunCondition == 2)
                    {
                        if ((*ite).run1)
                            AIActionController::executeExec(aiContext, (*ite).run1, 0, pData->getLv(), origin, NULL, pData, 0);

                        if ((*ite).run2)
                            AIActionController::executeExec(aiContext, (*ite).run2, 0, pData->getLv(), origin, NULL, pData, 0);
                    }
                }
            }

            pData->m_bSummonActive = false;
            pData->currentAction = ActionType::DEAD_ACTION1;
            pData->exitDuel();

            if (aiContext.summonProxy)
                aiContext.summonProxy->sleepSummon(aiContext, origin, pData->myID);

            iter = summonUIDList.erase(iter);
            removedCount++;
            if (removedCount >= removeNum)
                break;
        }
        //if (summonUIDList.size() <= 0)
        //    _shouldDeleteSelf = true;
        return removedCount;
    }
    */
    
    int AIContinuousSummonCaster::removeSummon(int summonID, int num)
    {
        if (num <= 0)
            return 0;

        int removedCount = 0;
        for (auto iter = summonUIDList.begin(); iter != summonUIDList.end();)
        {
            CharacterData *pSummon = aiContext.getCharacterByID(*iter);
            if (!pSummon || !pSummon->isSummon() || pSummon->summonProperty->summonId != summonID || !pSummon->isSummonActive)
            {
                ++iter;
                continue;
            }

            //HQ:同类幻象持续时间未到被替换时,死亡执行条件使用2而不是0,为不影响线上角色,不修改
            if (!pSummon->hasTag("summonkilled"))
            {
                checkExcuteDeadRun(aiContext, pSummon, 2);
                checkExcuteDeadRun(aiContext, pSummon, 4);
            }

            pSummon->isSummonActive = false;
            pSummon->exitDuel();
            pSummon->resetSummonData();

            if (aiContext.summonProxy)
                aiContext.summonProxy->sleepSummon(aiContext, origin, pSummon->myID);

            if (pSummon->hasTag("summonincharm253"))
            {
                pSummon->eraseTag("summonincharm253");
                if (aiContext.seffProxy)
                    aiContext.seffProxy->notify(aiContext, pSummon->myID);
            }

            iter = summonUIDList.erase(iter);
            removedCount++;
            if (removedCount >= num)
                break;
        }

        return removedCount;
    }

    int AIContinuousSummonCaster::getActiveCount()
    {
        return summonUIDList.size();
    }

    int AIContinuousSummonCaster::getSummonID()
    {
        return _summonID;
    }

    int AIContinuousSummonCaster::getGroupSummonAvailableCount(int casterMyID, CharacterData *target, int execID)
    {
        SummonExec *exec = AIDataHelper::getSummonExec(execID);
        if (!exec || exec->groupID <= 0)
            return -1;
        int availablecnt = exec->groupMaxNum;
        for (AIContinuousSummonCaster *a_caster : target->summonCasts) {
            if (a_caster && a_caster->myID != casterMyID && a_caster->summonExec->groupID == exec->groupID && a_caster->summonExec->groupWeight > exec->groupWeight) {
                int an_active_count = a_caster->getActiveCount();
                availablecnt -= an_active_count;
                if (availablecnt <= 0)
                    return 0;
            }
        }
        return availablecnt;
    }

    void AIContinuousSummonCaster::sleepCasterSummon(CharacterData *target, int summonID, int coverCnt, int excludeCasterMyID)
    {
        int totalNum = coverCnt;
        int removedNum = 0;
        int toRemoveNum = totalNum;

        for (auto summonIter = target->summonCasts.begin(); summonIter != target->summonCasts.end(); ++summonIter)
        {
            AIContinuousSummonCaster *theSummonCaster = *summonIter;
            if (!theSummonCaster || (excludeCasterMyID >= 0 && theSummonCaster->myID == excludeCasterMyID))
                continue;

            removedNum += theSummonCaster->removeSummon(summonID, toRemoveNum);
            if (removedNum >= totalNum)
                break;

            toRemoveNum = totalNum - removedNum;
        }
    }

    void AIContinuousSummonCaster::sleepGroupSummon(int groupID, int groupWeight, CharacterData *target, int excludeCasterMyID, int sleepNum)
    {
        if (groupID <= 0 || sleepNum <= 0)
            return;
        int currentSleepNum = 0;
        int toSleepNum = sleepNum;
        for (AIContinuousSummonCaster *a_caster : target->summonCasts) {
            if (a_caster && a_caster->myID != excludeCasterMyID && a_caster->summonExec->groupID == groupID && a_caster->summonExec->groupWeight <= groupWeight) {
                currentSleepNum += a_caster->removeSummon(a_caster->_summonID, toSleepNum);
                if (currentSleepNum >= sleepNum)
                    break;
                toSleepNum = sleepNum - currentSleepNum;
            }
        }
    }
    
    bool AIContinuousSummonCaster::couldHurtSummon(CharacterData* pSource, HurtExec* pHurtExec, CharacterData* pTarget, int skillID)
    {
        if (!pSource || !pHurtExec || !pTarget)
            return false;
        
        if (!pTarget->isSummon())
            return true;

        if(pTarget->summonProperty)
        {
            //0-正常被打死亡，1-被特定技能打死，2-被某类型伤害打死
            
            if (pTarget->summonProperty->deadType == 0)
            {
                return true;
            }
            else if (pTarget->summonProperty->deadType == 1)
            {
                auto iter = std::find(pTarget->summonProperty->deadParams.begin(), pTarget->summonProperty->deadParams.end(), skillID);
                if (iter != pTarget->summonProperty->deadParams.end())
                    return true;
            }
            else if (pTarget->summonProperty->deadType == 2)
            {
                auto &&deadparams = pTarget->summonProperty->deadParams;
                if (deadparams.size() < 2)
                    return false;
                
                const int hurtProperty = HeroHurtcalculate::HurtFuncGetRealtimeHurtProperty(*pSource, pHurtExec->property);
                if (deadparams[0] == -1 || deadparams[0] == pHurtExec->type)
                {
                    auto iter = std::find(deadparams.begin() + 1, deadparams.end(), hurtProperty);
                    if (iter != deadparams.end())
                    {
                        return true;
                    }
                }
            }
        }
        
        return false;
    }

    void AIContinuousSummonCaster::checkExcuteDeadRun(AIContext &aiContext, CharacterData *pSummon, int runCondition)
    {
        for (auto &&deadRun : pSummon->summonProperty->deadRunVec)
        {
            if (deadRun.deadRunCondition != runCondition)
                continue;

            bool canRun = true;
            if (runCondition == 1)
            {
                canRun = false;
                bool isRelatedSummonAlive = false;
                auto &&relatedSummonIDs = pSummon->summonProperty->associationIdVec;

                auto characterList = pSummon->getOriginFriends();
                for (auto iter = characterList->begin(); iter != characterList->end(); ++iter)
                {
                    auto pCharacter = *iter;
                    if (!pCharacter || !pCharacter->isSummon())
                        continue;

                    if (std::find(relatedSummonIDs.begin(), relatedSummonIDs.end(), pCharacter->summonProperty->summonId) == relatedSummonIDs.end())
                        continue;

                    if (!HeroActionController::isDeadAction(pCharacter->currentAction))
                    {
                        isRelatedSummonAlive = true;
                        break;
                    }
                }

                if (!isRelatedSummonAlive)
                {
                    canRun = true;
                }
            }

            if (canRun)
            {
                if (deadRun.run1)
                {
                    AIActionController::executeExec(aiContext, deadRun.run1, 0, pSummon->getLv(), pSummon->getSummonSource(), nullptr, pSummon, 0);
                }
                if (deadRun.run2)
                {
                    AIActionController::executeExec(aiContext, deadRun.run2, 0, pSummon->getLv(), pSummon->getSummonSource(), nullptr, pSummon, 0);
                }
            }
        }
    }

    void AIContinuousSummonCaster::outCharm253()
    {
        for (auto ite = summonUIDList.begin(); ite != summonUIDList.end(); ++ite) {
            CharacterData *aSummon = aiContext.getCharacterByID(*ite);
            if (!aSummon)
                continue;
            aSummon->eraseTag("summonincharm253");
            if (aiContext.seffProxy != nullptr)
            {
                aiContext.seffProxy->notify(aiContext, aSummon->myID);
            }
        }
    }
}
