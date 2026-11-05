//
//  AICharacterData.cpp
//  OnePiece
//
//
//

#include "AICharacterData.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <stdlib.h>
#include "cocos2d.h"

#include "AIDataManager.h"
#include "AIContext.h"
#include "AIProjectile.h"
#include "AIActionController.h"
#include "AIContinuousTransformCaster.h"
#include "AISpecialEffectLabel.h"
#include "action_hero/HeroActionController.h"
#include "action_hero/HeroDeadAction.h"
#include "action_hero/HeroCelebrateAction.h"
#include "action_hero/HeroIdleAction.h"
#include "action_hero/HeroAttackAction.h"
#include "action_hero/HeroSkillController.h"
#include "action_hero/HeroTurnAction.h"
#include "projectile/ProjectileForWarship.h"
#include "BattleLog.h"
#include "scriptbind/AIScriptBindCharacterData.hpp"
#include "scriptbind/AIScriptBindActionController.hpp"
#include "scriptbind/AIScriptBindEffectCaster.hpp"
#include "AIScriptBinder.hpp"
#include "AITargetHelper.h"

using namespace std;

namespace OnePiece {

    const int kInvalidPos = 10000000;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CharacterData
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CharacterData::CharacterData() : AINode(-1, -1), teamSide(TeamSide::kInvalid),
        attributesOriginalBackup(nullptr), _originX(kInvalidPos), _originY(kInvalidPos), roleType(RoleType::kNormalCharacter)
	{
		memset(actionAtt, 0, sizeof(actionAtt));
		attributesOrigin = std::unique_ptr<CharacterAttr>(new CharacterAttr);
		attributesAfterBegin = std::unique_ptr<CharacterAttr>(new CharacterAttr);
	}

    CharacterData::CharacterData(int* attList, std::list<std::pair<int, int>>& skillList, int juexing, AIContext& aiContext, int type, short k, short ifUltraLock, int clusterNum, CharacterData* pSummonSource, TeamSide side, RoleType role, int fp)
        : AINode(-1, clusterNum), friends(nullptr), m_pTmpFriends(nullptr), teamSide(side), attributesOriginalBackup(nullptr),
        _originX(kInvalidPos), _originY(kInvalidPos), roleType(role), combatPower(fp)
	{
		attributesOrigin = std::unique_ptr<CharacterAttr>(new CharacterAttr);
		attributesAfterBegin = std::unique_ptr<CharacterAttr>(new CharacterAttr);

		myID = aiContext.local_incremential++;
		frozenCompareID = myID;
		needAdjusting = false;
		juexingLevel = juexing;
		this->k = k;
		this->ifUltraLock = ifUltraLock;

		canUltraSkill = 0;
		targetStuckBackward = false;

		m_bInCharmEffect = false;
		m_bInTauntEffect = false;
        _isInHalfCharmEffect = false;
        
        m_iSkillSeq2Index = 0;
        
        summoner = pSummonSource;
        isSummonActive = false;
        summonProperty = NULL;

        direction = CharacterFunc::IsEnemyTeam(teamSide) ? -1 : 1;
        //lastDirection = direction;

		this->aiContext = &aiContext;
		aiContext.allCharacters[myID] = this;

		attributes = attList;

		staticData = AIDataHelper::getCharacterData(*attList);

		CharacterFunc::CopyOldFashionAttributes(attributes, attributesOrigin.get());
#if BATTLE_LOG_TEST
        OPBattleLog("CharacterData 4 start");
#endif
		for (auto iter = skillList.begin(); iter != skillList.end(); ++iter)
		{
			if (iter->first < 1 || iter->second < 1) continue;


            #if BATTLE_LOG_TEST
                    OPBattleLog("CharacterData 4 1");

            #endif
			if (iter->first == staticData->sk[0])
			{
				normalSkill.first = iter->first;
				normalSkill.second = iter->second;
                #if BATTLE_LOG_TEST
                    OPBattleLog("normalSkillID ",normalSkill.first);
                #endif
                int useSkillAdd = 0;
                AIDataHelper::getBeHittedAddEnergy(normalSkill.first, useSkillAdd, _beHittedAddEnergy);
            }
            //else
            //{
            //    AIDataHelper::getBeHittedAddEnergy(normalSkill.first,useSkillAdd,beHittedAdd);
            //}
            

			this->skillList.push_back(std::pair<int, int>(iter->first, iter->second));
			int jlevel = juexingLevel;
			if (jlevel > 5)jlevel = 5;//max juexing=5 now
			if (iter->first == staticData->sk[1] && staticData->jxj[3 * jlevel]>0){
				this->skillList.push_back(std::pair<int, int>(staticData->jxj[3 * jlevel], iter->second));
			}

            auto skillID = iter->first;
            auto skillLevel = iter->second;
             #if BATTLE_LOG_TEST
                    OPBattleLog("CharacterData 4 2");
            #endif
            auto skillData = AIDataHelper::getSkillData(skillID);
            if (skillData)
            {
                #if BATTLE_LOG_TEST
                    OPBattleLog("CharacterData 4 3");
                #endif
                if (HeroSkillController::isSkillDataIgnoreSilence(skillData))
                {
                    ignoreSilenceSkill[skillID] = true;
                }
                #if BATTLE_LOG_TEST
                    OPBattleLog("CharacterData 4 4");
                #endif
                auto pSkill = SpecialSkillInfo::create(staticData, skillID, skillLevel, skillData);
                if (pSkill)
                {
                    if (skillData->castCondition == eSkillCastConditionAnyEnemyHP)
                        aiContext.addEventListener("hpchange", this);
                    _specialSkills[skillData->castCondition].push_back(std::move(pSkill));
                }
                #if BATTLE_LOG_TEST
                    OPBattleLog("CharacterData 4 5");
                #endif
                if (skillData->castCondition == eSkillCastConditionOnFightBegin)
                {
                    skillsOnBegin.push_back(std::pair<int, int>(skillID, skillLevel));
                }
            }
		} // END for skilllist
        OPBattleLog("CharacterData 4 end");
        // HQ:type完全没有用到
		//if (type == 1)
	    actionController = HeroActionController::getInstance();

		//friends = aiContext.getRivalCluster(clusterNum);
		//m_pTmpFriends = aiContext.getTmpRivalCluster(clusterNum);
        
        currentAction = (summoner != NULL) ? DEAD_ACTION1 : IDLE_ACTION;
        actionCauserMyID = -1;
#if BATTLE_LOG_TEST
        OPBattleLog("CharacterData 5");
#endif
		memset(actionAtt, 0, sizeof(actionAtt));
		localTick = aiContext.currentTick;
		startTick = localTick+1;
		totalTick = 1;
		HeroActionController::setSkillActionCD(actionAtt, 0);
		HeroActionController::setSkillSeq(actionAtt, -1);
		HeroActionController::setSkillID(actionAtt, HeroSkillController::calculateSkillID(aiContext,*this, 0));
#if BATTLE_LOG_TEST
        OPBattleLog("CharacterData 6");
#endif
        m_bEnterEffectAction = false;
        
		mStationaryDirection = 0;
		mStationaryX = 0;
		mStationaryY = 0;

        ignoreFreeze = (role == RoleType::kCaptain);
	}

    void CharacterData::initCaptainSeq(std::list<CaptainSeqSkillInfo> &seqList)
    {
        for (auto iter = seqList.begin(); iter != seqList.end(); ++iter)
        {
            CaptainSeqSkillInfo &skinfo = *iter;
            originCaptainSkills.push_back(skinfo);
            if (skinfo.seqType != CaptainSeqType::kSeqNone)
            {
                remainCaptainSkills.push_back(skinfo);
            }
        }
    }

    AIContinuousEffectCaster *CharacterData::getEffectCaster(int casterMyId)
    {
		if (IsEffectExec(casterMyId))
		{
			for (auto anEffect : effectCasts)
			{
				if (anEffect && anEffect->effectID == casterMyId)
					return anEffect;
			}
		}
		else
		{
			for (auto anEffect : effectCasts)
			{
				if (anEffect && anEffect->myID == casterMyId)
					return anEffect;
			}
		}
        
        return nullptr;
    }


	AIContinuousMarkCaster *CharacterData::getMarkCaster(int casterMyId)
	{
		if (IsMarkExec(casterMyId))
		{
			for (auto anEffect : markCasts)
			{
				if (anEffect && anEffect->markID == casterMyId)
					return anEffect;
			}
		}
		else
		{
			for (auto anEffect : markCasts)
			{
				if (anEffect && anEffect->myID == casterMyId)
					return anEffect;
			}
		}

		return nullptr;
	}


    bool CharacterData::hasTag(const std::string &tag)
    {
        return _tags.find(tag) != _tags.end();
    }

    void CharacterData::addTag(const std::string &tag)
    {
        _tags[tag] = 0;
    }

    void CharacterData::eraseTag(const std::string &tag)
    {
        _tags.erase(tag);
    }

    int CharacterData::getTagValue(const std::string &tag)
    {
        auto iter = _tags.find(tag);
        if (iter != _tags.end())
            return iter->second;
        return 0;
    }

    void CharacterData::setTagValue(const std::string &tag, int value)
    {
        _tags[tag] = value;
    }

    int CharacterData::getCannotBeFoundByTeamRelativeTypeCnt(int type)
    {
        if (type == 0)
            return _cannotBeFoundByRealRivalCnt;
        return _cannotBeFoundByRealFriendCnt;
    }

    void CharacterData::setCannotBeFoundByTeamRelativeTypeCnt(int type, int cnt)
    {
        if (type == 0)
            _cannotBeFoundByRealRivalCnt = cnt;
        _cannotBeFoundByRealFriendCnt = cnt;
    }

    bool CharacterData::hasEffect(int effectID)
    {
        auto iter = m_aiEffectMap.find(effectID);
        if (iter != m_aiEffectMap.end())
        {
            for (auto pEff : iter->second)
            {
                if (pEff && !pEff->isErased() && !pEff->isTempInvalid())
                    return true;
            }
        }
        return false;
    }

    int CharacterData::getEffectRemainCD(int effectID)
    {
        auto iter = innerEffectIntervals.find(effectID);
        if (iter != innerEffectIntervals.end())
            return iter->second;
        return 0;
    }
    
    bool CharacterData::hasMark(int markID)
    {
        for (auto pMark : markCasts)
        {
            if (pMark && pMark->isValid() && pMark->markID == markID)
                return true;
        }
        return false;
    }
    
    bool CharacterData::isEnterEffectAction()
    {
        return currentAction == ActionType::EFFECT_ACTION && m_bEnterEffectAction;
    }
    
    void CharacterData::addSelfInToTmpRivalList(CharacterDataListPtr tmpList)
	{
		tmpList->push_back(this);
	}

    void CharacterData::removeSelfFromRivalList(CharacterDataListPtr tmpList)
	{
        CharacterDataList::iterator ite = tmpList->begin();
		while (ite != tmpList->end())
		{
			CharacterData *pCharacter = *ite;
			if (pCharacter && pCharacter->myID == this->myID)
			{
				tmpList->erase(ite);
				break;
			}

			++ite;
		}
	}

    void CharacterData::changeFriendRivalSideForCluster(TeamSide clusterSide, int clusterNum, bool isFromFriendToRival, bool excludeSummon)
    {
        auto theFriends = aiContext->getFriendsForCluster(clusterSide, clusterNum, excludeSummon);
        auto theRivals = aiContext->getRivalsForCluster(clusterSide, clusterNum, excludeSummon);

        removeSelfFromRivalList(isFromFriendToRival ? theFriends : theRivals);
        addSelfInToTmpRivalList(isFromFriendToRival ? theRivals : theFriends);
    }

	void CharacterData::inCharmEffect()
	{
		if (!m_bInCharmEffect)
		{
            TeamSide fromSide = teamSide;
            int fromClusterNum = clusterNum;
            TeamSide toSide = teamSide == TeamSide::kOur ? TeamSide::kEnemy : TeamSide::kOur;
            int toClusterNum = teamSide == TeamSide::kOur ? aiContext->current_ClusterNumEnemy : aiContext->current_ClusterNumOur;

            changeFriendRivalSideForCluster(fromSide, fromClusterNum, true, false);
            changeFriendRivalSideForCluster(toSide, toClusterNum, false, false);
            if (!isSummon())
            {
                changeFriendRivalSideForCluster(fromSide, fromClusterNum, true, true);
                changeFriendRivalSideForCluster(toSide, toClusterNum, false, true);
            }

            auto friendsTo = aiContext->getFriendsForCluster(toSide, toClusterNum);
            auto rivalsTo = aiContext->getRivalsForCluster(toSide, toClusterNum);
            this->m_pTmpFriends = friendsTo;
            this->m_pTmpRivals = rivalsTo;

			m_bInCharmEffect = true;

			lockTarget(*aiContext);
		}
	}


	void CharacterData::outCharmEffect()
	{
		if (m_bInCharmEffect)
		{
            TeamSide fromSide = teamSide == TeamSide::kOur ? TeamSide::kEnemy : TeamSide::kOur;
            int fromClusterNum = teamSide == TeamSide::kOur ? aiContext->current_ClusterNumEnemy : aiContext->current_ClusterNumOur;
            TeamSide toSide = teamSide;
            int toClusterNum = clusterNum;

            changeFriendRivalSideForCluster(fromSide, fromClusterNum, true, false);
            changeFriendRivalSideForCluster(toSide, toClusterNum, false, false);
            if (!isSummon())
            {
                changeFriendRivalSideForCluster(fromSide, fromClusterNum, true, true);
                changeFriendRivalSideForCluster(toSide, toClusterNum, false, true);
            }

            auto friendsTo = aiContext->getFriendsForCluster(toSide, toClusterNum);
            auto rivalsTo = aiContext->getRivalsForCluster(toSide, toClusterNum);
            this->m_pTmpFriends = friendsTo;
            this->m_pTmpRivals = rivalsTo;

			m_bInCharmEffect = false;

			lockTarget(*aiContext);
		}
	}

    void CharacterData::inHalfCharmEffect()
    {
        if (_isInHalfCharmEffect)
            return;

        TeamSide fromSide = teamSide;
        int fromClusterNum = clusterNum;
        TeamSide toSide = teamSide == TeamSide::kOur ? TeamSide::kEnemy : TeamSide::kOur;
        int toClusterNum = teamSide == TeamSide::kOur ? aiContext->current_ClusterNumEnemy : aiContext->current_ClusterNumOur;

        changeFriendRivalSideForCluster(fromSide, fromClusterNum, true, false);
        if (!isSummon())
            changeFriendRivalSideForCluster(fromSide, fromClusterNum, true, true);

        _tmpFriends.clear();
        _tmpRivals.clear();

        auto friendsTo = aiContext->getFriendsForCluster(toSide, toClusterNum);
        auto rivalsTo = aiContext->getRivalsForCluster(toSide, toClusterNum);

        for (auto iter = friendsTo->begin(); iter != friendsTo->end(); ++iter)
            _tmpFriends.push_back(*iter);
        _tmpFriends.remove(this);
        _tmpFriends.push_back(this);

        for (auto iter = rivalsTo->begin(); iter != rivalsTo->end(); ++iter)
        {
            if (*iter != this)
                _tmpRivals.push_back(*iter);
        }

        this->m_pTmpFriends = &_tmpFriends;
        this->m_pTmpRivals = &_tmpRivals;

        _isInHalfCharmEffect = true;

        lockTarget(*aiContext);
    }

    void CharacterData::outHalfCharmEffect()
    {
        if (!_isInHalfCharmEffect)
            return;
        
        TeamSide fromSide = teamSide;
        int fromClusterNum = clusterNum;
        TeamSide toSide = teamSide;
        int toClusterNum = clusterNum;

        changeFriendRivalSideForCluster(fromSide, fromClusterNum, false, false);
        if (!isSummon())
            changeFriendRivalSideForCluster(fromSide, fromClusterNum, false, true);

        auto friendsTo = aiContext->getFriendsForCluster(toSide, toClusterNum);
        auto rivalsTo = aiContext->getRivalsForCluster(toSide, toClusterNum);
        this->m_pTmpFriends = friendsTo;
        this->m_pTmpRivals = rivalsTo;

        _tmpFriends.clear();
        _tmpRivals.clear();

        _isInHalfCharmEffect = false;

        lockTarget(*aiContext);
    }

	void CharacterData::inTauntEffect()
	{
		if (!m_bInTauntEffect)
		{
			CharacterData *pTmpData = this->target;
            if (!hasEffect(248))
            {
                this->target = this->tauntTarget;
            }
			this->m_pTmpTarget = pTmpData;
            this->specialFoundSkillTarget = nullptr;
			m_bInTauntEffect = true;
		}
	}
	void CharacterData::outTauntEffect()
	{
		if (m_bInTauntEffect)
		{
			CharacterData *pTmpData = this->target;
            if (!hasEffect(248))
            {
                this->target = this->m_pTmpTarget;
            }
			this->m_pTmpTarget = pTmpData;
            this->specialFoundSkillTarget = nullptr;
            this->tauntTarget = nullptr;
			m_bInTauntEffect = false;
		}
	}

    void CharacterData::onEnterSilence()
    {
        if (isInAccumulateState()) {
            //releaseAccumulateSkill(false);
            HeroAttackAction::ForceEndAccumulate(*this, *aiContext);
        }
    }

    void CharacterData::resetSummonData()
    {
        currentAction = ActionType::DEAD_ACTION1;
        actionCauserMyID = -1;
        HeroActionController::cleanup(*this, *aiContext, true, false);
		targetStuckBackward = false;
		oldY = getY();
		target = nullptr;
		m_pTmpTarget = nullptr;
        specialFoundSkillTarget = nullptr;
        attacker = nullptr;
		setZ(getY());
		adjustingX = 100000000;
        mAttackConfirmedTarget = false;
        moveCasterAffectedCount = 0;
        _tags.clear();
        setFakeSummonType(0);
        cannotBeAttackedBuffCnt = 0;

		memset(actionAtt, 0, sizeof(actionAtt));
        
		HeroActionController::setSkillActionCD(actionAtt, 0);
		HeroActionController::setSkillSeq(actionAtt, -1);
		HeroActionController::setSkillID(actionAtt, HeroSkillController::calculateSkillID(*aiContext, *this, 0));

        /*
        for (auto loseHpAddAttrIte = _loseHpAddAttrMap.begin();loseHpAddAttrIte!= _loseHpAddAttrMap.end(); ++loseHpAddAttrIte)
        {
            delete [] loseHpAddAttrIte->second;
        }
        
        _loseHpAddAttrMap.clear();
        */
        loseHpAddAttrMap.clear();
        
        resetEffectExec();
        removeMarks();
        resetSpecialSkillConditon();
        resetTraversalTargetRecord();
        resetFlags();
        
        //要将数据恢复 主要是血 怒气
        updateHP(getHPTop());
        updateEnergy(0);
    }
    
    void CharacterData::onSummoned()
    {
        hasBeenSummoned = true;

        // TODO:判断特殊情况下的友军
        auto theFriends = getCurrentFriendsList();
        if (theFriends)
        {
            for (auto aFriend : *theFriends)
            {
                if (aFriend->canBeAttacked(summoner, true))
                    AICastWhenFriendSummonEffect::checkExecute(*aiContext, aFriend, this);
            }
        }
    }

    /*
    void CharacterData::removeTargetSummonInSummonCaster(int characterID,int num, int excludeCasterUID)
    {
        int count = 0;
		for (std::list<AIContinuousSummonCaster*>::iterator iter = summonCasts.begin(); iter != summonCasts.end();)
        {
            auto pCaster = *iter;
            if (pCaster && pCaster->myID != excludeCasterUID)
            {
                if(count >= num)
                    break;
                
                //for (auto summonIter = (*iter)->summonUIDList.begin(); summonIter != (*iter)->summonUIDList.end();)
                //{
                //    auto heroUID = *summonIter;
                //    CharacterData *hero = aiContext->getCharacterByID(heroUID);
                //    if (hero && hero->getID() == characterID)
                //    {
                //        CCLOG("summon is active ? %d", hero->m_bSummonActive);
                //        hero->m_bSummonActive = false;
                //        targetSummonList.push_front(hero);

                //        summonIter = (*iter)->summonUIDList.erase(summonIter);
                //        ++count;

                //        if (count == num)
                //            break;
                //    }
                //    else
                //    {
                //        ++summonIter;
                //    }
                //}
                count += pCaster->removeSummonByID(characterID, num);
                
                //if (pCaster->summonUIDList.size() == 0)
				//{
				//	delete *iter++;
				//}
				//else
				//{
				//	++iter;
				//}
                ++iter;
            }
            else
            {
                if (!pCaster)
                    CCLOG("Error data!!! CharacterID:%d", characterID);
				++iter;
            }
        }
    }
    */
    
	void CharacterData::update(AIContext& aiContext)
	{
		//CCLOG("SHANBEILENG:CharacterData::update:%d--%d--%d--%d", aiContext.currentTick, myID, staticData->id, currentAction);
		AINode::update(aiContext);

        if (AIDataHelper::getAiUpdateModeAfterWinlose() == AIUPDATEMODE_STOP_FIGHT_AFTER_WINLOSE && aiContext.hasWinlose())
        {
            if (!HeroActionController::isDeadAction(currentAction))
            {
                removeSpecialMoveCast((int)AIContinuousMoveCaster::MoveType::kSpecialSummonBackMove);
                actionController->update(*this, aiContext);
            }
            return;
        }
		
		// clair_wang test
//#if BATTLE_LOG_TEST
//		CCLOG("clair_wang CharacterData::update characterId= %d currentTick= %d, isFrozen= %d", attributes[0], aiContext.currentTick, isFrozen);
//#endif

        //延迟入场:整场战斗只触发一次
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *this, eEFFECT_LAZY_ENTER_BATTLE, 0) > 0) {
            //if (lazyEnterWave < 0)
            //    lazyEnterWave = aiContext.currentWaveCount;
            
            //if (lazyEnterWave == aiContext.currentWaveCount) {
                for (auto iter = effectCasts.begin(); iter != effectCasts.end();) {
                    auto effectCaster = *iter++;
                    if (isFrozen && !effectCaster->hasInnerEffect(eEFFECT_LAZY_ENTER_BATTLE))
                        continue;
                    effectCaster->Excute(aiContext);
                }
                return;
            //}
            //else {
            //    AIContinuousEffectCaster::zero_Special_Effect(aiContext, *this, eEFFECT_LAZY_ENTER_BATTLE);
            //}
        }

        for (std::list<AIContinuousHurtCaster*>::iterator iter = hurtCasts.begin(); iter != hurtCasts.end(); )
        {
            (*iter++)->Excute(aiContext);
        }
        
        if (isFrozen)
        {
            //判断是否是友方放大
            //若是则可以放大招
            

            bool isFriend = false;
            
            if (aiContext.ultraFreezeList.size() > 0)
            {
                std::list<int*>::iterator thefirst = aiContext.ultraFreezeList.begin();
                for (; thefirst != aiContext.ultraFreezeList.end(); thefirst++)
                {
                    //锁屏倒计时
                    if ((*thefirst)[0] >0) break;
                }
                if (thefirst != aiContext.ultraFreezeList.end())
                {

                    //OpCountdown, EffCountdown, myID, myCluster, startGlobalTick, freezetime
                    for (std::list<int*>::iterator iter = aiContext.ultraFreezeList.begin(); iter != aiContext.ultraFreezeList.end(); iter++)
                    {
                        if ((*iter)[4] - (*thefirst)[4]<(*iter)[5] && (*iter)[3] == clusterNum && (*iter)[6] == teamSide)
                        {
                            isFriend = true;
                            break;
                        }
                    }

                }
            }
            if (isFriend) {
                if ((/*clusterNum>0*/CharacterFunc::IsEnemyTeam(teamSide) || aiContext.autoFight>0 || canSummonCastUltra()) && canCastUltraSkill()>0) {
                    castUltraSkill(false);
                    
                    actionController->update(*this, aiContext);
                }
                else
                {
                    checkCastUltraSkill();
                }
            }
                 
            
        }else
        {
            if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *this, eEFFECT_FREEZE_IN_EFFECT_ACTION, 0) > 0)
            {
                implUpdateCastsInEffect281Action(aiContext);
            }
            else
            {
                implUpdateCasts(aiContext);

                AIHpLostEffect::checkLostExcute(aiContext, this);

                if ((/*clusterNum>0*/CharacterFunc::IsEnemyTeam(teamSide) || aiContext.autoFight>0 || canSummonCastUltra()) && canCastUltraSkill()>0) {
                    castUltraSkill(false);
                }
                else
                {
                    checkCastUltraSkill();
                }


                actionController->update(*this, aiContext);
            }

            //if ((/*clusterNum>0*/CharacterFunc::IsEnemyTeam(teamSide) || aiContext.autoFight>0 || canSummonCastUltra()) && canCastUltraSkill()>0) {
            //    castUltraSkill(false);
            //}
            //else
            //{
            //    checkCastUltraSkill();
            //}
            //
            //
            //actionController->update(*this, aiContext);
        }
	}

    void CharacterData::implUpdateCasts(AIContext& aiContext)
    {
        for (std::list<AIContinuousSummonCaster*>::iterator iter = summonCasts.begin(); iter != summonCasts.end();)
        {
            (*iter++)->Excute(aiContext);
        }

        for (std::list<AIContinuousMoveCaster*>::iterator iter = moveCasts.begin(); iter != moveCasts.end();)
        {
            (*iter++)->Excute(aiContext);
        }

        for (std::list<AIContinuousProjectileCaster*>::iterator iter = projectileCasts.begin(); iter != projectileCasts.end();)
        {
            (*iter++)->Excute(aiContext);
        }

        for (std::list<AIContinuousEffectCaster*>::iterator iter = effectCasts.begin(); iter != effectCasts.end();)
        {
            (*iter++)->Excute(aiContext);
        }

        for (auto iter = markCasts.begin(); iter != markCasts.end();)
        {
            auto pMark = *iter;
            if (pMark->isValid()) {
                pMark->Excute(aiContext);
                ++iter;
            }
            else {
                delete pMark;
                iter = markCasts.erase(iter);
            }
            //(*iter++)->Excute(aiContext);
        }

        for (auto iter = markDeadImmunes.begin(); iter != markDeadImmunes.end();)
        {
            if (iter->second > 0)
                iter->second--;

            if (iter->second <= 0)
                markDeadImmunes.erase(iter++);
            else
                ++iter;
        }

        for (auto iter = innerEffectIntervals.begin(); iter != innerEffectIntervals.end(); ++iter)
        {
            if (iter->second > 0)
                iter->second--;
        }

        for (auto iter = _transformCasts.begin(); iter != _transformCasts.end();)
        {
            (*iter++)->update(aiContext);
        }

        for (auto iter = _extraUpdateFuncs.begin(); iter != _extraUpdateFuncs.end(); ++iter)
        {
            auto &aFunc = iter->second;
            if (aFunc != nullptr)
            {
                aFunc(aiContext);
            }
        }
        for (auto iter = _extraUpdateFuncs.begin(); iter != _extraUpdateFuncs.end();)
        {
            if (iter->second == nullptr)
                iter = _extraUpdateFuncs.erase(iter);
            else
                ++iter;
        }
    }

    void CharacterData::implUpdateCastsInEffect281Action(AIContext& aiContext)
    {
        for (std::list<AIContinuousSummonCaster*>::iterator iter = summonCasts.begin(); iter != summonCasts.end();)
        {
            auto aCast = (*iter++);
            if (canCastInEffect281Action(aCast->myID))
                aCast->Excute(aiContext);
        }

        for (std::list<AIContinuousMoveCaster*>::iterator iter = moveCasts.begin(); iter != moveCasts.end();)
        {
            auto aCast = (*iter++);
            if (canCastInEffect281Action(aCast->myID))
                aCast->Excute(aiContext);
        }

        for (std::list<AIContinuousProjectileCaster*>::iterator iter = projectileCasts.begin(); iter != projectileCasts.end();)
        {
            auto aCast = (*iter++);
            if (canCastInEffect281Action(aCast->myID))
                aCast->Excute(aiContext);
        }

        for (std::list<AIContinuousEffectCaster*>::iterator iter = effectCasts.begin(); iter != effectCasts.end();)
        {
            auto aCast = (*iter++);
            if (canCastInEffect281Action(aCast->myID) || !aCast->isFollowSkillTick())
                aCast->Excute(aiContext);
        }

        for (auto iter = markCasts.begin(); iter != markCasts.end();)
        {
            auto pMark = *iter;
            if (canCastInEffect281Action(pMark->myID))
            {
                if (pMark->isValid()) {
                    pMark->Excute(aiContext);
                    ++iter;
                }
                else {
                    delete pMark;
                    iter = markCasts.erase(iter);
                }
                //(*iter++)->Excute(aiContext);
            }
            else
            {
                ++iter;
            }
        }

        for (auto iter = markDeadImmunes.begin(); iter != markDeadImmunes.end();)
        {
            if (iter->second > 0)
                iter->second--;

            if (iter->second <= 0)
                markDeadImmunes.erase(iter++);
            else
                ++iter;
        }

        for (auto iter = innerEffectIntervals.begin(); iter != innerEffectIntervals.end(); ++iter)
        {
            if (iter->second > 0)
                iter->second--;
        }

        for (auto iter = _transformCasts.begin(); iter != _transformCasts.end();)
        {
            auto aCast = (*iter++);
            if (canCastInEffect281Action(aCast->myID))
                aCast->update(aiContext);
        }

        //for (auto iter = _extraUpdateFuncs.begin(); iter != _extraUpdateFuncs.end(); ++iter)
        //{
        //    auto &aFunc = iter->second;
        //    if (aFunc != nullptr)
        //    {
        //        aFunc(aiContext);
        //    }
        //}
        //for (auto iter = _extraUpdateFuncs.begin(); iter != _extraUpdateFuncs.end();)
        //{
        //    if (iter->second == nullptr)
        //        iter = _extraUpdateFuncs.erase(iter);
        //    else
        //        ++iter;
        //}
    }

    void CharacterData::removeSpecialMoveCast(int moveType)
    {
        for (std::list<AIContinuousMoveCaster*>::iterator iter = moveCasts.begin(); iter != moveCasts.end();)
        {
            auto aCast = (*iter++);
            if ((int)aCast->moveType == moveType)
                delete aCast;
        }
    }

    bool CharacterData::canCastInEffect281Action(int theCastMyId)
    {
        return _canUpdateCastMyIdsInEffect281Action.find(theCastMyId) != _canUpdateCastMyIdsInEffect281Action.end();
    }
    
    bool CharacterData::canSummonCastUltra()
    {
        return isSummon() && isSummonActive && !HeroActionController::isDeadAction(currentAction);
    }
    
    int CharacterData::getFakeSummonType()
    {
        return _fakeSummonType;
    }

    void CharacterData::setFakeSummonType(int theType)
    {
        _fakeSummonType = theType;
    }

    bool CharacterData::seemsSummon()
    {
        if (hasTag("fakesummoneff_summon") || (isSummon() && !hasTag("fakesummoneff_notsummon") && getFakeSummonType() == 0)) {
            return true;
        }
        
        return false;
    }

	CharacterData::~CharacterData()
	{
		HeroActionController::cleanup(*this, *aiContext, true);

//        for (auto pHurt : hurtCastsToDelete)
//        {
//            delete pHurt;
//        }
//        hurtCastsToDelete.clear();

		aiContext->allCharacters[myID] = nullptr;
		delete[] attributes;

        if (attributesOriginalBackup)
            delete attributesOriginalBackup;
        attributesOriginalBackup = nullptr;

		//resetEffectExec();
        
        /*
        for (std::unordered_map<int,int*>::iterator loseHpAddAttrIte = _loseHpAddAttrMap.begin();loseHpAddAttrIte!= _loseHpAddAttrMap.end(); ++loseHpAddAttrIte)
        {
            delete [] loseHpAddAttrIte->second;
        }
    
        _loseHpAddAttrMap.clear();
        */
        loseHpAddAttrMap.clear();

        removeMarks();
        
        _specialSkills.clear();
        //元素之力标记清除 cui_2023_7_5
        atkLabelVec.clear();
	}

	void CharacterData::addTalent(int talentID, int talentLV)
	{
		talentList.push_back(std::pair<int, int>(talentID, talentLV));
	}

	int CharacterData::checkIfHit(int x, int y, int dx, int dy,int sourceDirection)
	{
        //CCLOG("show person x = %d\n",getX());
        
        if(currentAction == ActionType::MOVE_ACTION)
        {
            dx = std::abs(dx * sourceDirection - getSpeed() * direction);
        }
        
        if (std::abs(getX() - x) <= dx * 0.5) {
			//if (y >= getY() - dy){
			//	if (y <= (getY() + staticData->confg.height + dy))
			//		return 0;
			//}
			return 0;
		}

		return 10000;
	}

    bool CharacterData::canBeAttacked(CharacterData *attacker, bool ignoreCanbeattackEffect, CharacterData *skillCaster)
	{
        if (HeroActionController::isDeadAction(currentAction))
            return false;

		if (HeroActionController::isDeadAttacking(this))
			return false;

        if (attacker)
        {
            // 12.0
            if (/*isInDuel() || */attacker->isInDuel())
            {
                if (this->myID == attacker->myID)
                    return true;
                return isDuelWithTarget(attacker);
            }
        }

        /*
        if (ignoreCanbeattackEffect)
            return true;
        
        // MARK: 9.0 add
        if (attacker)
        {
            if (attacker->cannotFindFriendTargets.size() > 0 || attacker->cannotFindRivalTargets.size() > 0)
            {
                if (!attacker->canFindTargetOrigin(this))
                    return false;
            }
            
            if (this->cannotBeFoundConfig.size() > 0)
            {
                if (!this->canBeFoundByRelativeSide(this->teamSide == attacker->teamSide ? 1 : 0))
                    return false;
            }
        }

		for (auto iter = m_aiEffectMap.begin(); iter != m_aiEffectMap.end(); ++iter)
		{
            //for (auto eIter = iter->second.begin(); eIter != iter->second.end(); ++eIter) 
            //{
            //    if ((*eIter)->innerExec->canBeAttack == 1)
            //        return false;
            //}

            auto &effs = iter->second;
            for (int i = 0; i < effs.size(); i++)
            {
                auto &pEff = effs[i];
                if (pEff->innerExec->canBeAttack == 1)
                    return false;
            }
		}
        return true;
        */

        if (!isFindableToAttacker(attacker, ignoreCanbeattackEffect))
            return false;

        if (skillCaster && !isFindableToSkillCaster(skillCaster, ignoreCanbeattackEffect))
            return false;
		
        return true;
	}

    bool CharacterData::isFindableToAttacker(CharacterData *attacker, bool ignoreMissingTargetEffect)
    {
        if (ignoreMissingTargetEffect)
            return true;

        if (attacker)
        {
            if (attacker->cannotFindFriendTargets.size() > 0 || attacker->cannotFindRivalTargets.size() > 0)
            {
                if (!attacker->canFindTargetOrigin(this))
                    return false;
            }

            if (this->cannotBeFoundConfig.size() > 0)
            {
                if (!this->canBeFoundByRelativeSide(this->teamSide == attacker->teamSide ? 1 : 0))
                    return false;
            }
        }

        if (AIScriptBinder::getInstance()->isUseCannotBeAttackedBuffCnt(aiContext->getLuaState()))
        {
            if (cannotBeAttackedBuffCnt > 0)
                return false;
        }
        else
        {
            for (auto iter = m_aiEffectMap.begin(); iter != m_aiEffectMap.end(); ++iter)
            {
                std::vector<AIInnerEffect *> &effs = iter->second;
                for (int i = 0; i < effs.size(); i++)
                {
                    AIInnerEffect *pEff = effs[i];
                    if (pEff && pEff->innerExec->canBeAttack == 1)
                        return false;
                }
            }
        }

        return true;
    }

    bool CharacterData::isFindableToSkillCaster(CharacterData *skillCaster, bool ignoreMissingTargetEffect)
    {
        if (ignoreMissingTargetEffect)
            return true;

        if (skillCaster)
        {
            if (skillCaster->cannotFindFriendTargets.size() > 0 || skillCaster->cannotFindRivalTargets.size() > 0)
            {
                if (!skillCaster->canFindTargetOrigin(this))
                    return false;
            }

            if (getCannotBeFoundByTeamRelativeTypeCnt(teamSide == skillCaster->teamSide ? 1 : 0) > 0)
                return false;
        }

        if (AIScriptBinder::getInstance()->isUseCannotBeAttackedBuffCnt(aiContext->getLuaState()))
        {
            if (cannotBeAttackedBuffCnt > 0)
                return false;
        }
        else
        {
            for (auto iter = m_aiEffectMap.begin(); iter != m_aiEffectMap.end(); ++iter)
            {
                std::vector<AIInnerEffect *> &effs = iter->second;
                for (int i = 0; i < effs.size(); i++)
                {
                    AIInnerEffect *pEff = effs[i];
                    if (pEff && pEff->innerExec->canBeAttack == 1)
                        return false;
                }
            }
        }

        return true;
    }

	void CharacterData::resetEffectExec()
	{
		//m_aiEffectMap.clear();
        for (auto iter = m_aiEffectMap.begin(); iter != m_aiEffectMap.end(); ++iter)
        {
            while (iter->second.size() > 0)
            {
                auto pEff = *(iter->second.rbegin());
                pEff->onEraseEffect();
                iter->second.pop_back();
            }
        }
        m_aiEffectMap.clear();
        
        attrEffectResults.clear();

        m_bEnterEffectAction = false;
		m_bInCharmEffect = false;
		m_bInTauntEffect = false;

        //outCharmEffect();
        //outTauntEffect();

        alternateAccuSkillMaxTicks.clear();
        _extraUpdateFuncs.clear();
        _hurtTransFuncs.clear();

        _canUpdateCastMyIdsInEffect281Action.clear();

        targetForFocus = nullptr;
	}

    void CharacterData::removeMarks()
    {
        for (std::list<AIContinuousMarkCaster*>::iterator iter = this->markCasts.begin(); iter != this->markCasts.end(); ++iter)
        {
            //(*iter)->isRemoved = true;
            delete *iter;
        }
        this->markCasts.clear();
    }

    void CharacterData::invalidateMarks()
    {
        for (auto pMark : markCasts)
        {
            pMark->invalidate();
        }
    }

    void CharacterData::marksOnSelfEnterDead()
    {
        for (auto markCast : markCasts)
            markCast->onCharacterDead();
    }

    void CharacterData::executeSpecialEffectLabel(int labelID, int num)
    {
        AISpecialEffectLabel::excute(labelID, num, *aiContext, this);
    }
    
    void CharacterData::forceTurnToTarget(CharacterData *theTarget)
    {
        bool cannotTurn = (AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, eEFFECT_NO_TURN_ACTION, 0) > 0);
        if (theTarget && !cannotTurn && (theTarget->getX() - this->getX())*this->direction < 0)
        {
            this->direction = -this->direction;
            if (aiContext->animationProxy)
                aiContext->animationProxy->forceTurnCharacter(*aiContext, this);
        }
    }

    void CharacterData::onEscape()
    {
        AIContinuousEffectCaster::excertOnEscapeEffects(*aiContext, *this);

        if (_specialSkills.find(ESkillCastCondition::eSkillCastConditionOnAvoidNoBreakAction) != _specialSkills.end())
            _specialSkills[ESkillCastCondition::eSkillCastConditionOnAvoidNoBreakAction].front()->onConditon();

        if (_specialSkills.find(ESkillCastCondition::eSkillCastConditionOnAvoidAndBreakAction) != _specialSkills.end())
        {
            auto skillInfo = _specialSkills[ESkillCastCondition::eSkillCastConditionOnAvoidAndBreakAction].front().get();
            if (!HeroSkillController::isTargetIgnoreSilenceBySkill(*aiContext, target, skillInfo->skillID))
            {
                if (AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, EffectType::eEFFECT_NO_SKILL_MARK, 0) > 0)
                    return;
            }
            skillInfo->onConditon();
            HeroAttackAction::ForceEnter(*this, *aiContext, -1, skillInfo->skillID, skillInfo->skillLevel, true, 0, skillInfo->skillID == staticData->sk[1]);
        }
    }

    void CharacterData::onHurtHit(int id)
    {
        if (aiContext->aiCallback && aiContext->businessType == BusinessType::CANNON_KING && teamSide == TeamSide::kEnemy)
            aiContext->aiCallback->monsterOnHit(myID, aiContext->getCharacterWaveNum(myID));
    }

	// clair_wang
	// 造成暴击后触发
	void CharacterData::onCruelHitSkillRelease()
	{
		if (_specialSkills.find(ESkillCastCondition::eSkillCastConditionAfterCruelHit) != _specialSkills.end())
		{
			_specialSkills[ESkillCastCondition::eSkillCastConditionAfterCruelHit].front()->onConditon();
		}
	}

    void CharacterData::onCastCritHurt()
    {
        if (currentAction == ATTACK_ACTION && hasTag("repeatskill") && hasTag("extendrepeatoncrit"))
        {
            currentRepeatSkillMaxRepeatTick += HeroSkillController::getRepeatSkillExtendRepeatTickForCrit(currentRepeatSkillId);
            int theRepeatMaxTick = HeroSkillController::getRepeatSkillMaxRepeatTick(currentRepeatSkillId);
            if (currentRepeatSkillMaxRepeatTick > theRepeatMaxTick)
                currentRepeatSkillMaxRepeatTick = theRepeatMaxTick;
        }
    }

    void CharacterData::updateHP(int hp)
    {
        if (hp < 0)
            setHP(0);
        else
            setHP(std::min(getHPTop(), hp));

        if (AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, EffectType::eEFFECT_ATTR_WITH_LOSEHP_ADD, 0))
            AIContinuousEffectCaster::change_Special_Effect(*aiContext, *this, EffectType::eEFFECT_ATTR_WITH_LOSEHP_ADD, 0);

        AIRepelEffect::checkCleanEffectOnCharacterHpChange(*aiContext, this);

        aiContext->onEvent("hpchange", this);
        aiContext->onEvent("hpchange249", this);

        AIExcuteOnAttrChangeEffect::checkExcute(this, ECharacterAttr::HP); // 13.0
    }
    
    void CharacterData::updateEnergy(int energy)
    {
        if (energy > AIDataHelper::getEnergyRecover()[0])
            energy = AIDataHelper::getEnergyRecover()[0];
        else if (energy < 0)
            energy = 0;
        setEnergy(energy);

        AIExcuteOnAttrChangeEffect::checkExcute(this, ECharacterAttr::ENG);
    }

    void CharacterData::enhanceAttribute(int attribute, int enhanceRatio, bool isAdd)
    {
        if (!attributesOriginalBackup)
        {
            attributesOriginalBackup = new std::vector<int>;
            for (int i = 0; i < ECharacterAttr::ATT_LEN; i++)
                attributesOriginalBackup->push_back(attributes[i]);
        }

        std::vector<int> attrs;
        if (-1 == attribute)
        {
            attrs = { ECharacterAttr::MAX_HP, 
                      //ECharacterAttr::HP,
                      ECharacterAttr::PHY_AT,
                      ECharacterAttr::MAG_AT,
                      ECharacterAttr::PHY_CRIT,
                      ECharacterAttr::MAG_CRIT,
                      //ECharacterAttr::PHY_HIT,
                      //ECharacterAttr::MAG_HIT,
                      ECharacterAttr::PHY_DE,
                      ECharacterAttr::MAG_DE,
                      ECharacterAttr::PHY_PE,
                      ECharacterAttr::MAG_PE,
                      ECharacterAttr::CURE };
        }
        else
            attrs.push_back(attribute);

        for (int i = 0; i < attrs.size(); i++)
        {
            int attr = attrs[i];
            int baseValue = attributesOriginalBackup->at(attr);
            int enhanceValue = baseValue * enhanceRatio / 100;
            if (isAdd)
                attributes[attr] = baseValue + enhanceValue;
            else
                attributes[attr] = enhanceValue;
        }

        updateHP(getHPTop());
    }

    CharacterData* CharacterData::lockTarget(AIContext &aiContext, bool isJudgeBy297)
    {
        if (AIScriptBinder::getInstance()->isLockTargetByLua(aiContext.getLuaState()))
            return luahelper_characterdata_locktarget(aiContext.getLuaState(), this, isJudgeBy297);
        return lockTargetNative(aiContext, isJudgeBy297);
    }

	CharacterData* CharacterData::lockTargetNative(AIContext &aiContext, bool isJudgeBy297)
    {
        // 248
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *this, eEFFECT_FORCE_ATTACK_SELF, 0) > 0)
            return this;

        /// 12.0
        //if (_duelTarget)
        //    return _duelTarget;
        ///

		if (this->mAttackConfirmedTarget)
		{
            /// 12.0
            if (_duelTarget && this->target && this->target->myID != _duelTarget->myID)
                return nullptr;
            ///
			return this->target;
		}

        if (isInTauntEffect())
        {
            /// 12.0
            if (_duelTarget && this->target && this->target->myID != _duelTarget->myID)
                return nullptr;
            ///
            //return this->target;
            return this->tauntTarget; // OP2
        }

        auto tmpTargets = m_pTmpRivals;
        bool isTauntExtra = false;
        // 253
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *this, eEFFECT_TAUNT_EXTRA, 0) > 0 || hasTag("summonincharm253"))
        {
            isTauntExtra = true;
            tmpTargets = m_pTmpFriends;
        }
        else
        {
            if (isInCharmEffect() || isInHalfCharmEffect())
            {
            }
            else
            {
                // 280
                if (hasEffect(eEFFECT_FOCUS_TARGET) && targetForFocus && !HeroActionController::isDeadAction(targetForFocus->currentAction))
                {
                    return targetForFocus;
                }
            }
        }
        
        if (tmpTargets == NULL) {
			return NULL;
		}
		else
		{
			CharacterDataList::iterator iter;
            //uint32_t distanceSquare = std::numeric_limits<uint32_t>::max();
            int distance = 100000000;
            int distnormal = distance;
			CharacterData *t = NULL;
            CharacterData *tnormal = NULL; // 最近的非幻象目标
            bool has313 = hasEffect(313); // 优先搜索最近的非幻象目标
            for (iter = tmpTargets->begin(); iter != tmpTargets->end(); ++iter)
			{
                auto pRival = *iter;
                if (pRival->myID == this->myID && isTauntExtra)
                    continue;
                if (!pRival->canBeAttacked(this))
                    continue;
                if (isJudgeBy297 && hasEffect(297) && !luahelper_innereffect_297_is_findable(aiContext.getLuaState(), pRival, this))
                    continue;
                /// 12.0
                if (_duelTarget && pRival->myID != _duelTarget->myID)
                    continue;
                ///
                if (!hasEffect(293) || CharacterFunc::IsAttackerFaceTarget(this, pRival))
                {
                    auto dist = HeroActionController::distanceToCharacter(this, pRival);
                    if (dist < distance)
                    {
                        t = pRival;
                        distance = dist;
                    }
                    if (has313 && !pRival->isSummon() && dist < distnormal)
                    {
                        tnormal = pRival;
                        distnormal = dist;
                    }
                }
			}

            if (has313 && tnormal)
            {
                return tnormal;
            }
			return t;
		}
	}

    
    
	void CharacterData::printCharacter(){
		std::cout << myID << std::endl;
		std::cout << getX() << std::endl;
		std::cout << getY() << std::endl;
		std::cout << getHP() << std::endl;
		std::cout << currentAction << std::endl;
		std::cout << std::endl;
    }
    
    short CharacterData::checkTarget(short ifCanUltraSkill)
    {
        if (AIScriptBinder::getInstance()->isCheckTargetByLua(aiContext->getLuaState()))
            return luahelper_attackaction_check_target(aiContext->getLuaState(), this, ifCanUltraSkill);
        return checkTargetNative(ifCanUltraSkill);
    }

	short CharacterData::checkTargetNative(short ifCanUltraSkill)
	{
		if (this->target == nullptr) return 0;

		int ultraSkill = staticData->sk[1];
		if (ifCanUltraSkill == 2)
		{
			ultraSkill = staticData->jxj[3 * juexingLevel];
		}
		AISkill* skillData = AIDataHelper::getSkillData(ultraSkill);
        if (!skillData)
            return 0;
        
        if (hasEffect(294) && luahelper_innereffect_294_is_skillidblocked(aiContext->getLuaState(), this, ultraSkill))
            return -1;
        
        if (HeroSkillController::isSkillTargetForSingle(skillData) && hasEffect(297) && !luahelper_innereffect_297_is_findable(aiContext->getLuaState(), target, this))
        {
            auto anotherTarget = lockTarget(*aiContext, true);
            if (anotherTarget)
                this->target = anotherTarget;
            else
                return -1;
        }

//        auto realTarget = HeroSkillController::getRealSearchInfo(skillData->targetType).realSearchType;
        int realTarget = AITargetHelper::parseTargetType(skillData->targetType, nullptr, nullptr, nullptr, nullptr);
        // clair_wang 20150827
        if (realTarget == TargetCurrent && HeroActionController::isDeadAction(this->target->currentAction))
        {
            CharacterData* theTarget = lockTarget(*aiContext);
            if (theTarget && theTarget->myID != this->target->myID)
            {
                HeroActionController::setCurrentTarget(*this, theTarget);
            }
            else
            {
                // 大招所释放的对象为空
                HeroActionController::setCurrentTarget(*this, nullptr);
                this->target = nullptr;

                return 0;
            }
        }

        if (!HeroActionController::isTargetTypeRandom(realTarget))
        {
            if (AITargetHelper::isUseNewHelper())
            {
                AITargetContext ctx;
                ctx.source = this;
                ctx.target = this->target;
                ctx.skill_caster = this;
                ctx.skill_id = ultraSkill;
                ctx.search_type = skillData->targetType;
                ctx.range.x1 = -1 * skillData->range;
                ctx.range.x2 = skillData->range;
                ctx.range.y1 = -1 * skillData->range;
                ctx.range.y2 = skillData->range;
                ctx.search_params = &skillData->vTargetParms;
                ctx.is_skip_random = true;
                std::list<CharacterData *> currentTargets;
                AITargetHelper::searchTarget(&ctx, &currentTargets);
                if (currentTargets.size() < 1)
                    ifCanUltraSkill = 0;
            }
            else
            {
                auto currentTargets = AIActionController::searchTarget(*this, this->target, nullptr, *aiContext, skillData->targetType, -1 * skillData->range, skillData->range, -1 * skillData->range, skillData->range, -1 * skillData->range, skillData->range, &skillData->vTargetParms, *this, false, ultraSkill);
                if (currentTargets->size() == 0)
                    ifCanUltraSkill = 0;
            }
        }
        else
        {
            bool allEnemyDie = false;
            if (realTarget == TargetEnemyRandom ||
                realTarget == TargetRangeRandomEnemyOne ||
                realTarget == TargetEnemyRandomWithoutSearcher ||
                realTarget == TargetOneEnemyRandomContinueHurtNotRandomTarget ||
                realTarget == TargetSelfAttackRangeRandomEnemy)
            {

                allEnemyDie = true;
                // the all enemy must not die
                for (CharacterDataList::iterator ite = m_pTmpRivals->begin(); ite != m_pTmpRivals->end(); ++ite)
                {
                    CharacterData* tmpTarget = *ite;
                    if (tmpTarget && !HeroActionController::isDeadAction(tmpTarget->currentAction)) {
                        allEnemyDie = false;
                        break;
                    }
                }
            }
            //else if (skillData->targetType == TargetOneEnemyRandomContinueHurtNotRandomTarget)
            //{
            //	// if first round then random a new enemy
            //	// .to be continue
            //}

            if (!allEnemyDie && this->aiContext->adjustingDistance <= 0) {
                return ifCanUltraSkill;
            }
            else
            {
                return 0;
            }
        }

		return ifCanUltraSkill;
    }
    
    int CharacterData::canCastUltraSkill(bool isExternal)
    {
        if (AIScriptBinder::getInstance()->isCanCastUltraSkillByLua(aiContext->getLuaState()))
            return luahelper_attackaction_can_cast_ultra(aiContext->getLuaState(), this, isExternal);
        
        return canCastUltraSkillNative(isExternal);
    }

    int CharacterData::canCastUltraSkillNative(bool isExternal)
	{
        if (canUltraSkill <= 0)
            return 0;
        
        if (HeroActionController::isDeadAction(currentAction) || currentAction == EFFECT_ACTION || currentAction == CONTROLLED_EFFECT_ACTION || !aiContext->isFighting || getHP() <= 0)
            return -1;
        /*
        if (CharacterFunc::IsEnemyTeam(teamSide) && (aiContext->friend_num[0] + aiContext->friend_num[1] + aiContext->friend_num[2]) < 1)
            return -1;
        if (CharacterFunc::IsOurTeam(teamSide) && (aiContext->rival_num[0] + aiContext->rival_num[1] + aiContext->rival_num[2]) < 1)
            return -1;
        */
        if (CharacterFunc::IsRivalTeamAllDeadForCheckUltra(this))
            return -1;
        if (skillList.size() < 2 || skillList[1].first<1)
            return -1;

        if (hasTag("energyskill"))
            return -1;

        if (hasTag("repeatskill"))
            return -1;

		if (hasTag("besuppressed"))
		{
			return -1;
		}

        if (currentAction == ATTACK_ACTION && hasTag("nointerruptbyultra"))
            return -1;

        // 137
        if (AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, EffectType::eEFFECT_NO_UltraSkill, 0) > 0)
            return -1;

        // 248
        if (AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, EffectType::eEFFECT_FORCE_ATTACK_SELF, 0) > 0)
            return -1;

        if (!HeroSkillController::isTargetIgnoreSilenceBySkill(*aiContext, this, staticData->sk[1]))
        {
            // 114
            if (AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, EffectType::eEFFECT_NO_SKILL_MARK, 0) > 0)
                return -1;
        }
        
        int effectID = HeroActionController::getEffectID(this->actionAtt);
        if(effectID > 0)
        {
            EffectExec* pEffectExec = AIDataHelper::getEffectExec(effectID);
            if(pEffectExec && pEffectExec->stateAnimation > 0)
            {
                return -1;
            }
        }
        
        // 11.0
        if (hasTag("unactionable"))
            return -1;

        if (!aiContext->getOwnerIsInVaildBarrier(this))
            return -1;

        int returnType = checkTarget(canUltraSkill);
        if (returnType > 0)
        {
            int ultraSkill = staticData->sk[1];
            if (returnType == 2)
                ultraSkill = staticData->jxj[3 * juexingLevel];

            if (this->target && !HeroSkillController::isTargetInSkilllRange(this, ultraSkill, this->target))
                return -1;

            auto skillData = AIDataHelper::getSkillData(ultraSkill);
            if (skillData && getEnergy() < skillData->requireEn)
            {
                //canUltraSkill = 0; // 12.0:sync from lua:客户端UI显示会额外调用本接口，不应修改
                return -1;
            }
            if (skillData && IsAccumulateSkill(skillData->castCondition))
            {
                if (ECanCastUltraSkill::kCanCastNormalUltraSkill == returnType)
                    returnType = ECanCastUltraSkill::kCanCastAccumulateNormalUltraSkill;
                else if (ECanCastUltraSkill::kCanCastJueXingSkill == returnType)
                    returnType = ECanCastUltraSkill::kCanCastAccumulateJueXingSkill;
            }
        }


        return returnType;
	}

	int CharacterData::checkCastUltraSkill()
	{
        if (HeroActionController::isDeadAction(currentAction) || currentAction == EFFECT_ACTION || currentAction == CONTROLLED_EFFECT_ACTION || !aiContext->isFighting || getHP() <= 0)
            return -1;
        /*
		if (CharacterFunc::IsEnemyTeam(teamSide) && (aiContext->friend_num[0] + aiContext->friend_num[1] + aiContext->friend_num[2]) < 1) 
            return -1;
		if (CharacterFunc::IsOurTeam(teamSide) && (aiContext->rival_num[0] + aiContext->rival_num[1] + aiContext->rival_num[2]) < 1) 
            return -1;
        */
        if (CharacterFunc::IsRivalTeamAllDeadForCheckUltra(this))
            return -1;
		if (skillList.size() < 2 || skillList[1].first<1) return -1;

        if (hasTag("energyskill"))
        {
            return -1;
        }

        if (hasTag("repeatskill"))
            return -1;
		
		if (hasTag("besuppressed"))
		{
			return -1;
		}

		std::list<int*>::iterator thefirst = aiContext->ultraFreezeList.begin();
		for (; thefirst != aiContext->ultraFreezeList.end(); thefirst++)
		{
			if ((*thefirst)[0] > 0)
			{
				break;
			}
		}

		if (thefirst != aiContext->ultraFreezeList.end())
		{
			//OpCountdown, EffCountdown, myID, myCluster
//			int a = (*thefirst)[0];
//			int b = (*thefirst)[1];
//			int c = (*thefirst)[2];
//			int d = (*thefirst)[3];
			if (/*(*thefirst)[3] != clusterNum*/(*thefirst)[6] != teamSide && (*thefirst)[0]>0) //己方,时间内不被影响大招,只在操作时间内影响对方
			{
				return -1;
			}
		}

        if (!HeroSkillController::isTargetIgnoreSilenceBySkill(*aiContext, this, staticData->sk[1]))
        {
            if (AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, EffectType::eEFFECT_NO_SKILL_MARK, 0) > 0)
                return -1;
        }

        if (currentAction == ATTACK_ACTION && hasTag("nointerruptbyultra"))
            return -1;

		switch (currentAction)
		{
		case IDLE_ACTION:
		case ATTACK_ACTION:
		case MOVE_ACTION:
		case RUN_ACTION:
		case TURN_ACTION:
		case DEFENSE_ACTION:
		case IN_HURT_ACTION:
		case STANDUP_ACTION:
			if (canUltraSkill > 0) return canUltraSkill;
            if (staticData->sk[1] > 0 && AIDataHelper::getSkillData(staticData->sk[1]) == nullptr) return -1;
			//CCLOG("ULTRA:%d--%d--%d", myID, getEnergy(), AIDataHelper::getSkillData(staticData->sk[1])->requireEn);
			if (staticData->sk[1] > 0 && getEnergy() >= AIDataHelper::getSkillData(staticData->sk[1])->requireEn)
			{
				if (staticData->jxj[3 * juexingLevel]>0 && OutPutFileLine && aiContext->localRandom->nextInt(0, 100)<staticData->jxj[3 * juexingLevel + 2])
				{
					canUltraSkill = 2;
				}else{
					canUltraSkill = 1;
				}

				return canUltraSkill;
			}
			else
			{
				return 0;
			}
			break;
		case STRUCK_FLY_AWAY_ACTION:
		case STRUCK_BACKWORD_ACTION:
        case STRUCK_HOVER_ACTION:
		case FALL_DOWN_ACTION:
		case DEAD_ACTION1:
		case DEAD_ACTION2:
		case DEAD_ACTION3:
			return 0;
		default:
			return 0;
			break;
		}
    }
    
    int CharacterData::castUltraSkill(bool ifExternal)
    {
        return luahelper_attackaction_cast_ultra(aiContext->getLuaState(), this, ifExternal);
    }

	int CharacterData::castUltraSkillNative(bool ifExternal)
	{
		CCLOG("SHANBEILENG-castUltraSkill: 1, %ld", aiContext->currentTick);
        if (HeroActionController::isDeadAction(currentAction) || currentAction == EFFECT_ACTION || currentAction == CONTROLLED_EFFECT_ACTION || !aiContext->isFighting || getHP() <= 0)
            return -1;
        /*
		if (CharacterFunc::IsEnemyTeam(teamSide) && (aiContext->friend_num[0] + aiContext->friend_num[1] + aiContext->friend_num[2]) < 1) 
            return -1;
		if (CharacterFunc::IsOurTeam(teamSide) && (aiContext->rival_num[0] + aiContext->rival_num[1] + aiContext->rival_num[2]) < 1) 
            return -1;
        */
        if (CharacterFunc::IsRivalTeamAllDeadForCheckUltra(this))
            return -1;

        if (!HeroSkillController::isTargetIgnoreSilenceBySkill(*aiContext, this, staticData->sk[1]))
        {
            if (AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, eEFFECT_NO_SKILL_MARK, 0) > 0)
                return -1;
        }

        if (AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, eEFFECT_NO_UltraSkill, 0) > 0)
            return -1;

        if (AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, EffectType::eEFFECT_FORCE_ATTACK_SELF, 0) > 0)
            return -1;

        if (skillList.size() <= 0)
            return -1;

        if (currentAction == ATTACK_ACTION && hasTag("nointerruptbyultra"))
            return -1;

		std::vector<std::pair<int, int>>::iterator iter = skillList.begin();
		int ultraSkill = staticData->sk[1];
		if (canUltraSkill==2)
		{
			ultraSkill = staticData->jxj[3 * juexingLevel];
		}
		while (++iter != skillList.end())
		{
			if (ultraSkill == iter->first)
			{
				AISkill* skillData = AIDataHelper::getSkillData(ultraSkill);
				// clair_wang
				
				if (skillData != nullptr)
				{
                    // MARK:fix bug
                    // 大招释放时目标距离判断放在canCastUltraSkill中，防止出现canCast返回能释放(UI显示用)，实际释放时没有释放成功的问题
                    //if (!aiContext->getOwnerIsInVaildBarrier(this))
                    //    break;
                    //if (skillData->skillType != AISkill::SkillType_unactiveSkill && 
                    //    !HeroSkillController::isTargetInSkilllRange(*this, iter->first, iter->second, *this->target))
                    //    break;

					if (skillData->freezeTime > 0 && ifUltraLock>0)
					{
						//OpCountdown, EffCountdown, myID, myCluster, startGlobalTick, freezeTime, teamSide
						//大招锁屏,每个人的动作都静止,但是考虑到伤害执行,动作可以进入,但不执行。
						int* ultraFreeze = new int[10]();
						ultraFreeze[0] = skillData->freezeTime;
						ultraFreeze[1] = HeroSkillController::calculateSkillLength(ultraSkill);
						ultraFreeze[2] = myID;
						ultraFreeze[3] = clusterNum;
						ultraFreeze[4] = aiContext->currentTick;
						ultraFreeze[5] = skillData->freezeTime;
                        ultraFreeze[6] = teamSide;

                        //cui_2023_7_27
                        ultraFreeze[7] = skillData->darkTime;
                        ultraFreeze[8] = skillData->virtualTime;
                        ultraFreeze[9] = skillData->skillEftId;
						aiContext->ultraFreezeList.push_front(ultraFreeze);//后发优势
					}

					int returnType = canUltraSkill;
                    if (IsAccumulateSkill(skillData->castCondition))
                    {
                        if (ECanCastUltraSkill::kCanCastNormalUltraSkill == returnType)
                            returnType = ECanCastUltraSkill::kCanCastAccumulateNormalUltraSkill;
                        else if (ECanCastUltraSkill::kCanCastJueXingSkill == returnType)
                            returnType = ECanCastUltraSkill::kCanCastAccumulateJueXingSkill;
                    }
                    else if (skillData->castCondition == eSkillCastConditionEnergySkill)
                    {
                        if (ECanCastUltraSkill::kCanCastNormalUltraSkill == returnType)
                            returnType = kCanCastEnergyUltraSkill;
                        else if (ECanCastUltraSkill::kCanCastJueXingSkill == returnType)
                            returnType = kCanCastEnergyJueXingSkill;
                    }
					canUltraSkill = 0;//reset to zero

                    if (!IsEnergySkill(skillData->castCondition))
                    {
                        int energy = getEnergy();
                        if (energy > AIDataHelper::getEnergyRecover()[0])
                        {
                            energy = AIDataHelper::getEnergyRecover()[0];
                        }

                        CharacterFunc::ApplyEnergyDelta(this, -skillData->consumeEn);
                    }
					//int e2 = getEnergy();

                    // MARK: 6.0后,技能目标是寻找最高最低属性的,需要先面向目标,再释放技能,防止技能中的飞行道具永远打不到目标
                    bool isSpecialSkillTarget = false;
                    auto realSearchType = HeroSkillController::getRealSearchInfo(skillData->targetType).realSearchType;
                    switch (realSearchType) {
                    case TargetAllAttrHighest:
                    case TargetFriendAttrHighest:
                    case TargetEnemyAttrHighest:
                    case TargetAllAttrLowest:
                    case TargetFriendAttrLowest:
                    case TargetEnemyAttrLowest:
                        isSpecialSkillTarget = true;
                        break;
                    default:break;
                    }
                    if (isSpecialSkillTarget) {
                        auto targets = AIActionController::searchTarget(*this, target, nullptr, *aiContext, skillData->targetType, -1 * skillData->range, skillData->range, -1 * skillData->range, skillData->range, -1 * skillData->range, skillData->range, &skillData->vTargetParms, *this, true, ultraSkill);
                        if (targets->size() > 0) {
                            auto ultraTarget = targets->front();
                            /*
                            bool cannotTurn = (AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, eEFFECT_NO_TURN_ACTION, 0) > 0);
                            if ((ultraTarget->getX() - this->getX())*this->direction < 0 && !cannotTurn) {
                                this->direction = -this->direction;
                                if (aiContext->animationProxy)
                                    aiContext->animationProxy->forceTurnCharacter(*aiContext, this);
                            }
                             */
                            this->forceTurnToTarget(ultraTarget);
                        }
                    }

                    HeroAttackAction::Enter(*this, *aiContext, -1, iter->first, iter->second, true, 0, true);

					if (ifExternal)
                        aiContext->trackUltra.push_back(std::pair<int, int>(aiContext->currentTick, myID));

                    if (aiContext->aiCallback)
                    {
                        int postertype = 0;
                        if (getTagValue("posternone") > 0)
                        {
                            postertype = 2;
                        }
                        else if (getTagValue("posterall") > 0)
                        {
                            postertype = 1;
                        }
                        int posternum = getTagValue("posternum");
                        aiContext->aiCallback->characterDidCastUltraSkill(myID, getID(), teamSide, returnType, postertype, posternum);
                    }

                    aiContext->onEvent("castultra", this);

					return returnType;
				}
				
				break; 
			}
		}

		return 0;
	}

    void CharacterData::checkForceCastSkill(int skillID)
    {
        bool hasSkill = false;
        int skillLv = 0;
        if (HeroSkillController::isSkillSwapWithNormal(skillID)) {
            skillID = normalSkill.first;
        }

        for (auto iter = skillList.begin(); iter != skillList.end(); ++iter) {
            if (iter->first == skillID) {
                hasSkill = true;
                skillLv = iter->second;
                break;
            }
        }

        if (!hasSkill)
            return;

        auto skillData = AIDataHelper::getSkillData(skillID);
        if (!skillData)
            return;

        switch (skillData->castCondition) {
        case eSkillCastConditionOnFightBegin:
        case eSkillCastConditionOnDeath:
        case eSkillCastConditionOnSelfHP:
        case eSkillCastConditionOnTargetHP:
        case eSkillCastConditionOnHit:
        case eSkillCastConditionAfterDeath:
        case eSkillCastConditionOnMultipleHurt:
        case eSkillCastConditionOnAvoidAndBreakAction:
        case eSkillCastConditionOnAvoidNoBreakAction:
        case eSkillCastConditionAfterCruelHit:
        case eSkillCastConditionOnSelfHPBreakAction:
        case eSkillCastConditionOnTargetHPBreakAction:
            return;
        default:break;
        }

        CCLOG("HQ-forceCastSkill:%d, %d", skillID, aiContext->currentTick);
        if (HeroActionController::isDeadAction(currentAction) || currentAction == EFFECT_ACTION || currentAction == CONTROLLED_EFFECT_ACTION || !aiContext->isFighting || getHP() <= 0)
            return;
        /*
        if (CharacterFunc::IsEnemyTeam(teamSide) && (aiContext->friend_num[0] + aiContext->friend_num[1] + aiContext->friend_num[2]) < 1)
            return;
        if (CharacterFunc::IsOurTeam(teamSide) && (aiContext->rival_num[0] + aiContext->rival_num[1] + aiContext->rival_num[2]) < 1)
            return;
        */
        if (CharacterFunc::IsRivalTeamAllDeadForCheckUltra(this))
            return;

        if (!HeroSkillController::isTargetIgnoreSilenceBySkill(*aiContext, this, skillID))
        {
            if (AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, eEFFECT_NO_SKILL_MARK, 0) > 0)
                return;
        }

        if (AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, eEFFECT_NO_UltraSkill, 0) > 0)
        {
            if (staticData->sk.size() > 1 && skillID == staticData->sk[1])
                return;
            if (juexingLevel < 6 && staticData->jxj[3 * juexingLevel] == skillID)
                return;
        }

        if (AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, EffectType::eEFFECT_FORCE_ATTACK_SELF, 0) > 0)
        {
            if (skillID != normalSkill.first)
                return;
        }

        if (!aiContext->getOwnerIsInVaildBarrier(this))
            return;
        
        if (HeroSkillController::getRealSearchInfo(skillData->targetType).realSearchType == TargetSelfAttackRangeRandomEnemy) {
            bool shouldRetarget = true;
            if (mAttackConfirmedTarget) {
                shouldRetarget = (!target || HeroActionController::isDeadAction(target->currentAction));
            }

            if (shouldRetarget) {
                auto currentTargets = AIActionController::searchTarget(*this, target, nullptr, *aiContext, skillData->targetType, -1 * skillData->range, skillData->range, -1 * skillData->range, skillData->range, -1 * skillData->range, skillData->range, &skillData->vTargetParms, *this, true, skillID);
                for (auto iter = currentTargets->begin(); iter != currentTargets->end(); ++iter){
                    CharacterData* theTarget = *iter;
                    HeroActionController::setCurrentTarget(*this, theTarget);
                    this->mAttackConfirmedTarget = true;
                    break;
                }
            }
        }

        if (skillData->skillType != AISkill::SkillType_unactiveSkill && !HeroSkillController::isTargetInSkilllRange(this, skillID, this->target)) {
            return;
        }

        if (skillData->freezeTime > 0 && ifUltraLock > 0)
        {
            //OpCountdown, EffCountdown, myID, myCluster, startGlobalTick, freezeTime, teamSide
            int* ultraFreeze = new int[10]();
            ultraFreeze[0] = skillData->freezeTime;
            ultraFreeze[1] = HeroSkillController::calculateSkillLength(skillID);
            ultraFreeze[2] = myID;
            ultraFreeze[3] = clusterNum;
            ultraFreeze[4] = aiContext->currentTick;
            ultraFreeze[5] = skillData->freezeTime;
            ultraFreeze[6] = teamSide;

            //cui_2023_7_27
            ultraFreeze[7] = skillData->darkTime;
            ultraFreeze[8] = skillData->virtualTime;
            ultraFreeze[9] = skillData->skillEftId;
            aiContext->ultraFreezeList.push_front(ultraFreeze);//后发优势
        }

        if (skillData->consumeEn) {
            int energy = getEnergy();
            if (energy > AIDataHelper::getEnergyRecover()[0])
                energy = AIDataHelper::getEnergyRecover()[0];
            CharacterFunc::ApplyEnergyDelta(this, -skillData->consumeEn);
        }

        HeroAttackAction::ForceEnter(*this, *aiContext, -1, skillID, skillLv, true, 0, false);
    }

    int CharacterData::forceCastUltraSkill(int juexing)
    {
        return luahelper_attackaction_force_cast_ultra(aiContext->getLuaState(), this, juexing);
    }

    // 蓄力技能蓄力结束释放
    int CharacterData::releaseAccumulateSkill(bool ifExternal)
    {
        auto releaseSuccess = HeroActionController::releaseAccumulateSkill(*this, *aiContext);
        
        if (ifExternal)
            aiContext->trackAccumulate.push_back(std::pair<int, int>(aiContext->currentTick, myID));

        return releaseSuccess ? 1 : 0;
    }

    void CharacterData::onStartCastSkill(int skillID)
    {
        for (auto& value : _specialSkills)
        {
            for (auto&& pSkill : value.second)
            {
                if (pSkill->skillID == skillID)
                    pSkill->onCast();
            }
        }

        auto skillData = AIDataHelper::getSkillData(skillID);
        if (skillData && IsAccumulateSkill(skillData->castCondition))
            enterAccumulateState(skillID);

        //if (skillID == normalSkill.first && AIContinuousEffectCaster::check_exert_Special_Effect(*aiContext, *this, eEFFECT_FORCE_ATTACK_SELF, 0) > 0)
        //{
        //    AIContinuousEffectCaster::change_Special_Effect(*aiContext, *this, eEFFECT_FORCE_ATTACK_SELF, 1);
        //}
        
        if (isCaptain() && aiContext->aiCallback)
            aiContext->aiCallback->captainCastSkill(myID, getID(), teamSide, skillID);
    }

    void CharacterData::onHurt(bool isEffect, int id /*hurtID or effectExecID*/, int hpDelta, int hurtType, CharacterData *attacker, int casterId)
    {
        auto pSkillInfo = getSpecialSkill(ESkillCastCondition::eSkillCastConditionOnMultipleHurt);
        if (pSkillInfo)
            pSkillInfo->onConditon();

        if (!isEffect && hpDelta > 0) {
            auto pSkillHurtPlus = getSpecialSkill(ESkillCastCondition::eSkillCastConditionOnHurtPlus);
            if (pSkillHurtPlus) {
                pSkillHurtPlus->onConditon();
                if (currentAction != ATTACK_ACTION && pSkillHurtPlus->canCast(this)) {
                    HeroActionController::actionSelector(*this, *aiContext);
                }
            }
        }

        if (hpDelta) {
            if (hurtType == PHYSICAL_HURT_TYPE || hurtType == MAGICAL_HURT_TYPE) {
                AIKOByCountEffect::checkExcute(*aiContext, this, hurtType, hpDelta);
            }

            if (hpDelta > 0)
            {
                AICastBySpecLabelHurtEffect::checkExecute(*aiContext, this, attacker, casterId);
            }
        }
    }

    SpecialSkillInfo *CharacterData::getSpecialSkill(ESkillCastCondition key)
    {
        if (eSkillCastConditionOnSelfHP == key || eSkillCastConditionOnSelfHPBreakAction == key)
        {
            auto iter = _specialSkills.find(key);
            if (iter != _specialSkills.end())
            {
                auto &skills = iter->second;
                SpecialSkillInfo *pHighestSkill = nullptr;
                for (int i = 0; i < skills.size(); i++)
                {
                    auto askill = skills[i].get();
                    if (pHighestSkill == nullptr)
                    {
                        if (askill->canCast(this))
                            pHighestSkill = skills[i].get();
                    }
                    else if (askill->canCast(this))
                    {
                        OnHPLimitSkillInfo *pskill = (OnHPLimitSkillInfo*)pHighestSkill;
                        OnHPLimitSkillInfo *panother = (OnHPLimitSkillInfo*)askill;
                        if (panother->isLimitHigher(this, pskill, this))
                            pHighestSkill = askill;
                    }
                }
                return pHighestSkill;
            }
            return nullptr;
        }

        auto foundIter = _specialSkills.find(key);
        if (foundIter != _specialSkills.end())
            return foundIter->second.front().get();
        return nullptr;
    }

    CharacterDataListPtr CharacterData::getCurrentFriendsList(bool excludeSummon)
    {
        if (excludeSummon)
        {
            //_tmpFriends.clear();
            //for (auto iter = m_pTmpFriends->begin(); iter != m_pTmpFriends->end(); ++iter)
            //{
            //    if (!(*iter)->isSummon())
            //        _tmpFriends.push_back(*iter);
            //}
            //return &_tmpFriends;
            return aiContext->getFriendsForCluster(teamSide, clusterNum, excludeSummon);
        }
        return m_pTmpFriends;
    }

    CharacterDataListPtr CharacterData::getCurrentRivalsList(bool excludeSummon)
    {
        if (excludeSummon)
        {
            //_tmpRivals.clear();
            //for (auto iter = m_pTmpRivals->begin(); iter != m_pTmpRivals->end(); ++iter)
            //{
            //    if (!(*iter)->isSummon())
            //        _tmpRivals.push_back(*iter);
            //}
            //return &_tmpRivals;
            return aiContext->getRivalsForCluster(teamSide, clusterNum, excludeSummon);
        }
        return m_pTmpRivals;
    }

    CharacterDataListPtr CharacterData::getSummonList()
    {
        return &_summons;
    }

    void CharacterData::resetSpecialSkillConditon()
    {
        for (auto& value : _specialSkills)
        {
            SpecialSkillInfo *skinfo = value.second.front().get();
            if (skinfo)
                skinfo->resetCondition();
        }
    }

    void CharacterData::resetClusterSpecialSkillCondition()
    {
        for (auto& value : _specialSkills)
        {
            SpecialSkillInfo *skinfo = value.second.front().get();
            if (skinfo)
                skinfo->resetClusterCondition();
        }
    }

    void CharacterData::onEffectRestore()
    {
        _restoredByEffectCount++;
    }

    void CharacterData::onScriptSetX(int x)
    {
        if (kInvalidPos == _originX)
            _originX = x;
    }

    void CharacterData::onScriptSetY(int y)
    {
        if (kInvalidPos == _originY)
            _originY = y;
    }

    void CharacterData::resetPos()
    {
        setX(_originX);
        setY(_originY);
    }

    int CharacterData::getScriptOriginX()
    {
        return _originX;
    }

    int CharacterData::getScriptOriginY()
    {
        return _originY;
    }

    void CharacterData::enterAccumulateState(int skillID)
    {
        _accumulateState = EAccumulateState::kInAccumulate;
        if (aiContext->aiCallback)
            aiContext->aiCallback->characterEnterState(myID, AICallbackCharacterState::kAccumulate, skillID);
    }

    void CharacterData::endAccumulateState()
    {
        if (aiContext->aiCallback)
            aiContext->aiCallback->characterExitState(myID, AICallbackCharacterState::kAccumulate);
        _accumulateState = EAccumulateState::kAfterAccumulate;
    }

    void CharacterData::exitAccumulateState()
    {
        _accumulateState = EAccumulateState::kNotAccumulate;

        HeroActionController::setAccumulateTick(actionAtt, 0);
        setAltAccuTick(0);
    }

    bool CharacterData::isInAccumulateState() const
    {
        return EAccumulateState::kInAccumulate == _accumulateState;
    }

    bool CharacterData::isAfterAccumulateState() const
    {
        return EAccumulateState::kAfterAccumulate == _accumulateState;
    }

    //cui_获取元素之力增伤减伤
    int CharacterData::getElementAtkAdd()
    {
        if (atkLabelVec.size() == 0) {
            return  CharacterFunc::GetCharacterAttrByLabel(this, -1, true);
        }
        return CharacterFunc::GetCharacterAttrByLabel(this, atkLabelVec.at(0), true);
    }
    int CharacterData::getElementDefAdd(CharacterData* source) {
        if (source->atkLabelVec.size() == 0) {
            return  CharacterFunc::GetCharacterAttrByLabel(this, -1, false);
        }
        int allnum = 0;
        for (size_t i = 0; i < source->atkLabelVec.size(); i++)
        {
            allnum += CharacterFunc::GetCharacterAttrByLabel(this, source->atkLabelVec.at(i), false); ;
        }
        return allnum / (source->atkLabelVec.size());
    }

    int CharacterData::checkIgnoreSilenceSkill(int skillID)
    {
        return ignoreSilenceSkill.find(skillID) != ignoreSilenceSkill.end() ? 1 : 0;
    }

    void CharacterData::appendTransformCast(const AIContinuousTransformCasterPtr& caster)
    {
        if (caster)
        {
            _transformCasts.push_back(caster);
        }
    }

    void CharacterData::removeTransformCast(int casterUID)
    {
        for (auto iter = _transformCasts.begin(); iter != _transformCasts.end(); ++iter)
        {
            if ((*iter)->myID == casterUID)
            {
                _transformCasts.erase(iter);
                return;
            }
        }
    }

    void CharacterData::clearTransformCast()
    {
        for (auto iter = _transformCasts.begin(); iter != _transformCasts.end(); ++iter)
        {
            (*iter)->stop();
        }
        _transformCasts.clear();
    }

    void CharacterData::enterTransformState()
    {
        // TODO: enter transform
        _isInTransformState = true;
    }

    void CharacterData::exitTransformState()
    {
        // TODO: exit transform
        _isInTransformState = false;
    }

    bool CharacterData::isInTransformState() const 
    {
        return _isInTransformState;
    }


	// clair_wang
	void CharacterData::SetFlagFirstFound26Target(bool flag)
	{
		b_isFirstFound26Target = flag;
	}
	bool CharacterData::GetFlagFirstFound26Target()
	{
		return b_isFirstFound26Target;
	}

	CharacterData* CharacterData::GetFirstRoundConfirm26Target()
	{
		return pFirstRoundConfirm26Target;
	}

	void CharacterData::SetFirstRoundConfirm26Target(CharacterData* theTarget /*= nullptr*/)
	{
		pFirstRoundConfirm26Target = theTarget;
	}

    void CharacterData::onEvent(const std::string &name, CharacterData *pusher)
    {
        if (name == "hpchange")
        {
            if (pusher->teamSide != this->teamSide)
            {
                auto pSkill = getSpecialSkill(ESkillCastCondition::eSkillCastConditionAnyEnemyHP);
                if (pSkill)
                {
                    auto pHPSkill = (OnEnemyHPSkillInfo *)pSkill;
                    if (pHPSkill->isTargetMeetCondition(pusher, this) && this->canBeAttacked(pusher, true))
                    {
                        pHPSkill->onConditon();
                        pHPSkill->addCheckTarget(pusher);
                        if (!pHPSkill->isInCD() && currentAction != ATTACK_ACTION)
                        {
                            HeroActionController::actionSelector(*this, *aiContext);
                        }
                    }
                }
            }
        }
        else if (name == "hpchange249")
        {
            if (pusher->teamSide == this->teamSide && !pusher->isSummon() && hasEffect(eEFFECT_CAST_BY_TEAM_HP_RATIO))
            {
                AICastByTeamHPRatioEffect::checkExcute(*aiContext, this);
            }
        }
        else if (name == "herodead250")
        {
            if (pusher->teamSide == this->teamSide && !pusher->isSummon() && hasEffect(eEFFECT_CAST_BY_TEAM_DEAD_CNT))
            {
                AICastByTeamDeadCntEffect::checkExcute(*aiContext, this);
            }
        }
        else if (name == "castultra")
        {
            if (pusher->teamSide != this->teamSide && hasTag("repeatskill") && !hasTag("repeatskillfirstround") &&!hasTag("repeatskillfirstbasicround") && !hasTag("repeatskillnointerrupt"))
            {
                addTag("repeatskillinterrupt");
            }
        }
    }

    void CharacterData::onFriendDead(CharacterData *theFriend)
    {
        if (hasEffect(eEFFECT_CAST_WHEN_FRIEND_DIE) && canBeAttacked(theFriend, true))
        {
            AICastWhenFriendDownEffect::checkExecute(*aiContext, this, theFriend);
        }
    }

    void CharacterData::onFriendSummonTimedOut(CharacterData *theFriendSummon)
    {
        if (hasEffect(eEFFECT_CAST_WHEN_FRIEND_DIE) && canBeAttacked(theFriendSummon, true))
        {
            AICastWhenFriendDownEffect::checkExecute(*aiContext, this, theFriendSummon);
        }
    }

    void CharacterData::onAttrChange(int attr)
    {
        //AIExcuteOnAttrChangeEffect::checkExcute(this, attr);
    }

    void CharacterData::onHPDelta(int hpdelta)
    {
        if (hpdelta < 0)
        {
            aiContext->onCharacterLoseHP(-hpdelta, this);

            if (hasEffect(317))
            {
                luahelper_innereffect_317_checkdo(aiContext->getLuaState(), this, hpdelta);
            }
        }
    }

    void CharacterData::onENDelta(int endelta)
    {
        if (endelta < 0)
        {
            if (hasEffect(318))
            {
                luahelper_innereffect_318_checkdo(aiContext->getLuaState(), this, endelta);
            }
        }
    }

    void CharacterData::onEnterDuel(CharacterData *duelTarget)
    {
        _isInDuel = true;
        _duelTarget = duelTarget;
    }

    void CharacterData::onExitDuel()
    {
        _isInDuel = false;
        _duelTarget = nullptr;
    }

    bool CharacterData::isInDuel()
    {
        return _duelTarget != nullptr;
    }

    bool CharacterData::isDuelWithTarget(CharacterData *duelTarget)
    {
        if (!isInDuel() || !duelTarget->isInDuel())
            return false;
        return _duelTarget->myID == duelTarget->myID;
    }

    void CharacterData::exitDuel()
    {
        if (isInDuel())
        {
            auto duelTarget = getDuelTarget();
            AIContinuousEffectCaster::zero_Special_Effect(*aiContext, *this, eEFFECT_DUEL);
            if (duelTarget)
                AIContinuousEffectCaster::zero_Special_Effect(*aiContext, *duelTarget, eEFFECT_DUEL);
        }
    }

    void CharacterData::resetTraversalTargetRecord()
    {
        _traversalRecord.clear();
        _traversalStartTick = 0;
        _traversalLastPickTick = 0;
    }

    void CharacterData::setTraversalStartTick(int tick)
    {
        _traversalStartTick = tick;
    }

    void CharacterData::pickTraversalTarget(int targetMyID, int tick)
    {
        _traversalRecord[targetMyID] = tick;
        _traversalLastPickTick = tick;
    }

    bool CharacterData::isTargetLockedByTraversal(int targetMyID)
    {
        return _traversalRecord.find(targetMyID) != _traversalRecord.end();
    }

    void CharacterData::resetFlags()
    {
        hitFlyExtendTick = 0;
        tempSex = -1;
        hasBeenSummoned = false;
        eff249cnt = 0;
        eff250cnt = 0;
        attackSpeed = 100;
        actionCauserMyID = -1;
    }

    int CharacterData::getSex()
    {
        if (tempSex >= 0)
            return tempSex;
        return staticData->sex;
    }

    void CharacterData::setTempSex(int theTempSex)
    {
        tempSex = theTempSex;
    }

    bool CharacterData::canFindTargetOrigin(CharacterData *theTarget)
    {
        if (theTarget->teamSide == teamSide)
        {
            return (cannotFindFriendTargets.find(theTarget->getID()) == cannotFindFriendTargets.end());
        }

        return (cannotFindRivalTargets.find(theTarget->getID()) == cannotFindRivalTargets.end());
    }

    bool CharacterData::canBeFoundByRelativeSide(int theSide)
    {
        return (cannotBeFoundConfig.find(theSide) == cannotBeFoundConfig.end());
    }

    int CharacterData::getNextAlternateAccuMaxTick(int theSkillId)
    {
        if (alternateAccuSkillMaxTicks.find(theSkillId) != alternateAccuSkillMaxTicks.end() && alternateAccuSkillMaxTicks[theSkillId] != nullptr)
        {
            auto theAltTicks = alternateAccuSkillMaxTicks[theSkillId].get();
            int theTotalTicks = 0;
            for (auto iter = theAltTicks->altAccuMaxTickChangeValues.begin(); iter != theAltTicks->altAccuMaxTickChangeValues.end(); ++iter)
            {
                theTotalTicks += iter->second;
            }
            if (theTotalTicks == 0)
                return -1;

            auto theSkillData = AIDataHelper::getSkillData(theSkillId);
            if (theSkillData)
            {
                int theRetAltMaxTick = theSkillData->castParams[0] + theTotalTicks;
                if (theRetAltMaxTick < 0)
                    theRetAltMaxTick = 0;
                return theRetAltMaxTick;
            }
        }
        return -1;
    }

    void CharacterData::setCurrentAlternateAccuMaxTick(int theSkillId, int theAltTick)
    {
        if (alternateAccuSkillMaxTicks[theSkillId] == nullptr)
            alternateAccuSkillMaxTicks[theSkillId] = std::unique_ptr<AccuSkillMaxTickInfo>(new AccuSkillMaxTickInfo);
        alternateAccuSkillMaxTicks[theSkillId]->currentAlternateAccuMaxTick = theAltTick;
    }

    int CharacterData::getCurrentAlternateAccuMaxTick(int theSkillId)
    {
        if (alternateAccuSkillMaxTicks.find(theSkillId) != alternateAccuSkillMaxTicks.end() && alternateAccuSkillMaxTicks[theSkillId] != nullptr)
        {
            return alternateAccuSkillMaxTicks[theSkillId]->currentAlternateAccuMaxTick;
        }
        return -1;
    }

    void CharacterData::addAlternateAccuMaxTick(int theSkillId, int theInnerEffUid, int theChangevalue)
    {
        if (alternateAccuSkillMaxTicks[theSkillId] == nullptr)
            alternateAccuSkillMaxTicks[theSkillId] = std::unique_ptr<AccuSkillMaxTickInfo>(new AccuSkillMaxTickInfo);
        alternateAccuSkillMaxTicks[theSkillId]->altAccuMaxTickChangeValues.push_back(std::make_pair(theInnerEffUid, theChangevalue));
    }

    void CharacterData::invalidateAlternateAccuMaxTick(int theSkillId, int theInnerEffUid)
    {
        if (alternateAccuSkillMaxTicks.find(theSkillId) != alternateAccuSkillMaxTicks.end() && alternateAccuSkillMaxTicks[theSkillId] != nullptr)
        {
            auto theAltTicks = alternateAccuSkillMaxTicks[theSkillId].get();
            for (auto iter = theAltTicks->altAccuMaxTickChangeValues.begin(); iter != theAltTicks->altAccuMaxTickChangeValues.end(); ++iter)
            {
                if (iter->first == theInnerEffUid)
                    iter->second = 0;
            }
        }
    }

    void CharacterData::addExtraUpdateFunc(int theUid, const std::function<void(AIContext&)> &theFunc)
    {
        _extraUpdateFuncs.push_back(std::make_pair(theUid, theFunc));
    }

    void CharacterData::invalidateExtraUpdateFunc(int theUid)
    {
        for (auto &aPair : _extraUpdateFuncs)
        {
            if (aPair.first == theUid)
                aPair.second = nullptr;
        }
    }

    void CharacterData::addHurtTransFunc(int theUid, const std::function<CharacterData *(AIContext &, CharacterData *, int, int)> &theFunc)
    {
        _hurtTransFuncs.push_back(std::make_pair(theUid, theFunc));
    }

    void CharacterData::invalidateHurtTransFunc(int theUid)
    {
        for (auto &aPair : _hurtTransFuncs)
        {
            if (aPair.first == theUid)
                aPair.second = nullptr;
        }
    }

    bool CharacterData::hasHurtTransFunc()
    {
        return (_hurtTransFuncs.size() > 0);
    }

    CharacterData *CharacterData::getHurtTransTarget(int theHpDelta, int theExecId)
    {
        for (auto iter = _hurtTransFuncs.begin(); iter != _hurtTransFuncs.end(); ++iter)
        {
            auto &aFunc = iter->second;
            if (aFunc == nullptr)
                continue;
            auto aTarget = aFunc(*aiContext, this, theHpDelta, theExecId);
            if (aTarget)
            {
                return aTarget;
            }
        }
        return nullptr;
    }

    bool CharacterData::hasSkill(int theSkillId)
    {
        for (auto iter = skillList.begin(); iter != skillList.end(); ++iter)
        {
            if (iter->first == theSkillId)
                return true;
        }
        return false;
    }

    int CharacterData::getSkillLevel(int theSkillId)
    {
        for (auto iter = skillList.begin(); iter != skillList.end(); ++iter)
        {
            if (iter->first == theSkillId)
                return iter->second;
        }
        return 0;
    }

    void CharacterData::addCanCastMyIdInEffect281Action(int theCasterMyId)
    {
        _canUpdateCastMyIdsInEffect281Action[theCasterMyId] = true;
    }

    void CharacterData::removeCanCastMyIdInEffect281Action(int theCasterMyId)
    {
        _canUpdateCastMyIdsInEffect281Action.erase(theCasterMyId);
    }

    void CharacterData::checkOutSummonCharm253()
    {
        if (isSummon())
            return;

        for (auto iter = summonCasts.begin(); iter != summonCasts.end(); ++iter)
        {
            auto aSummonCast = *iter;
            aSummonCast->outCharm253();
        }
    }

    void CharacterData::setAltAccuTick(int theAltAccuTick)
    {
        _altAccuTick = theAltAccuTick;
    }

    void CharacterData::updateSkillLevel(int theSkillId, int theSkillLevel)
    {
        for (auto iter = skillList.begin(); iter != skillList.end(); ++iter)
        {
            if (theSkillId == iter->first)
            {
                iter->second = theSkillLevel;
                return;
            }
        }
    }

    void CharacterData::backupSkillInfo(int theSkillId, int theSkillLevel)
    {
        skillInfoBackup[theSkillId] = theSkillLevel;
    }

    bool CharacterData::hasSkillInfoBackup(int theSkillId)
    {
        return skillInfoBackup.find(theSkillId) != skillInfoBackup.end();
    }

    int CharacterData::getSkillInfoBackup(int theSkillId)
    {
        if (hasSkillInfoBackup(theSkillId)) return skillInfoBackup[theSkillId];
        return 0;
    }
    
    void CharacterData::increaseCannotBeAttackedBuffCnt()
    {
        cannotBeAttackedBuffCnt += 1;
        //if (cannotBeAttackedBuffCnt == 1 && hasEffect(302)) {
        //    luahelper_innereffect_302_check_do(aiContext->getLuaState(), this);
        //}
    }
    
    void CharacterData::decreaseCannotBeAttackedBuffCnt()
    {
        cannotBeAttackedBuffCnt -= 1;
        if (cannotBeAttackedBuffCnt < 0)
            cannotBeAttackedBuffCnt = 0;
    }

    void CharacterData::increaseAllCannotBeFoundCnt()
    {
        _allCannotBeFoundCnt++;
        if (_allCannotBeFoundCnt == 1 && hasEffect(302))
            luahelper_innereffect_302_check_do(aiContext->getLuaState(), this);
    }

    void CharacterData::decreaseAllCannotBeFoundCnt()
    {
        _allCannotBeFoundCnt--;
        if (_allCannotBeFoundCnt < 0)
            _allCannotBeFoundCnt = 0;
    }

    void CharacterData::initPenaltyHpTop(int penalty)
    {
        penaltyHpTop = penalty;
        penaltyHpTopPercentage = (float)penaltyHpTop / ((float)penaltyHpTop + (float)getHPTop());
    }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    namespace CharacterFunc {

        int GetSkillIDByIndex(CharacterData& hero, int index)
        {
            if (index < hero.staticData->sk.size())
                return hero.staticData->sk[index];

            return hero.normalSkill.first;
        }

        int GetSkillIndexByID(CharacterData *target, int skillID) {
            for (int i = 0; i < target->staticData->sk.size(); i++) {
                if (target->staticData->sk[i] == skillID)
                    return i;
            }
            return -1;
        }

        bool IsOurTeam(TeamSide side) {
            return (TeamSide::kOur == side);
        }

        bool IsOurHero(CharacterData& hero) {
            return IsOurTeam(hero.teamSide);
        }

        bool IsEnemyTeam(TeamSide side) {
            return TeamSide::kEnemy == side;
        }

        bool IsEnemyHero(CharacterData& hero) {
            return IsEnemyTeam(hero.teamSide);
        }

        float GetCharacterHPRatio(CharacterData& hero) {
            float hpTop = std::max(float(hero.getHPTop()), 1.0f);
            return float(hero.getHP()) / hpTop;
        }

        int getSimplifiedHPRatio(CharacterData *target) {
            return target->getHP() * 100 / target->getHPTop();
        }

        CharacterAttrData& GetAttrDataCache() {
            static CharacterAttrData cacheData;
            return cacheData;
        }

        void ClearAttrDataCache() {
            GetAttrDataCache().clear();
        }

        void SetAttrValue(CharacterID chID, ECharacterAttr attr, int value) {
            auto& cache = GetAttrDataCache();
            cache[chID][attr] = value;
        }

        void SetAttrValueWhenNotExisted(CharacterID chID, ECharacterAttr attr, int value) {
            if (CheckHasAttrCache(chID, attr))
                return;

            SetAttrValue(chID, attr, value);
        }

        CharacterAttrCacheFoundResult GetAttrValue(CharacterID chID, ECharacterAttr attr) {
            CharacterAttrCacheFoundResult result(false, 0);
            if (CheckHasAttrCache(chID, attr))
            {
                auto& cache = GetAttrDataCache();
                result.value = cache[chID][attr];
                result.found = true;
            }
            return result;
        }

        bool CheckHasAttrCache(CharacterID chID, ECharacterAttr attr) {
            auto& cache = GetAttrDataCache();
            auto iter = cache.find(chID);
            if (iter != cache.end())
            {
                if (iter->second.find(attr) != iter->second.end())
                    return true;
            }
            return false;
        }

        void CheckInheritSummonAttr(SummonProperty *summonConf, int summonLv, int *summonAttributes, CharacterData *summoner) {
            if (summonConf->attrInheritBase > 0)
            {
                std::vector<int> inheritAttrs = {
                    ECharacterAttr::MAX_HP,
                    ECharacterAttr::PHY,
                    ECharacterAttr::AGILE,
                    ECharacterAttr::INT,
                    ECharacterAttr::ACCURATE,
                    ECharacterAttr::SPIRIT,
                    ECharacterAttr::PHY_AT,
                    ECharacterAttr::MAG_AT,
                    ECharacterAttr::PHY_CRIT,
                    ECharacterAttr::MAG_CRIT,
                    ECharacterAttr::PHY_CRIT_MUL,
                    ECharacterAttr::MAG_CRIT_MUL,
                    ECharacterAttr::PHY_HIT,
                    ECharacterAttr::MAG_HIT,
                    ECharacterAttr::PHY_DE,
                    ECharacterAttr::MAG_DE,
                    ECharacterAttr::PHY_PE,
                    ECharacterAttr::MAG_PE,
                    ECharacterAttr::DODGE_RATE,
                    ECharacterAttr::RESIST_RATE,
                    ECharacterAttr::BLOOD_SUCK,
					ECharacterAttr::CURE,
					ECharacterAttr::PHY_CRIT_RISIST,
					ECharacterAttr::MAG_CRIT_RISIST,
					ECharacterAttr::PHY_CRIT_MUL_ADD,
					ECharacterAttr::MAG_CRIT_MUL_ADD,
					ECharacterAttr::FUNC_HURT_ADD,
					ECharacterAttr::FUNC_HURT_SUB,
					ECharacterAttr::PHY_CRIT_ADD,
					ECharacterAttr::MAG_CRIT_ADD,

                    ECharacterAttr::SLASH_ADD,
                    ECharacterAttr::SLASH_SUB,
                    ECharacterAttr::LIGHT_ADD,
                    ECharacterAttr::LIGHT_SUB,
                    ECharacterAttr::DARK_ADD,
                    ECharacterAttr::DARK_SUB,
                    ECharacterAttr::WATER_ADD,
                    ECharacterAttr::WATER_SUB,
                    ECharacterAttr::POISON_ADD,
                    ECharacterAttr::POISON_SUB,
                    ECharacterAttr::ELECTRIC_ADD,
                    ECharacterAttr::ELECTRIC_SUB,
                    ECharacterAttr::FIRE_ADD,
                    ECharacterAttr::FIRE_SUB,
                    ECharacterAttr::NONE_ADD,
                    ECharacterAttr::NONE_SUB

                };
                int inheritRatio = summonConf->attrInheritBase;
                if (summonConf->attrInheritGrow > 0)
                    inheritRatio += summonConf->attrInheritGrow * summonLv;
                float ratio = (float)inheritRatio / 10000.0f;
                for (auto attr : inheritAttrs)
                    summonAttributes[attr] = (float)summoner->attributes[attr] * ratio;
            }

            for (auto &&inherit : summonConf->specialAttrInherit)
            {
                int inheritRatio = inherit.inheritBase;
                if (inherit.inheritGrow > 0)
                    inheritRatio += inherit.inheritGrow * summonLv;
                float ratio = (float)inheritRatio / 10000.0f;
                summonAttributes[inherit.attr] = (float)summoner->attributes[inherit.attr] * ratio;
            }

            summonAttributes[ECharacterAttr::HP] = summonAttributes[ECharacterAttr::MAX_HP];
        }
        
        bool IsAttackerFaceTarget(CharacterData *attacker, CharacterData *t)
        {
            if (attacker->direction == 1 && attacker->getX() <= t->getX())
                return true;
            else if (attacker->direction == -1 && attacker->getX() >= t->getX())
                return true;
            
            return false;
        }

        int GetPositionX(CharacterData *c, bool ismirrored)
        {
            if (ismirrored)
                return 2 * c->aiContext->getBattleFieldCenterX() - c->getX();
            return c->getX();
        }

        int GetDirection(CharacterData *c, bool ismirrored)
        {
            if (ismirrored)
                return -c->direction;
            return c->direction;
        }

        int GetInnerEffectProtectCount(CharacterData *c, int parentEffectCasterId, int innerEffectId)
        {
            if (c->innerEffectProtectCntMap.size() <= 0)
                return 0;
            std::map<int, std::map<int, int>> &protectinfo = c->innerEffectProtectCntMap;
            auto iter = protectinfo.find(parentEffectCasterId);
            if (iter != protectinfo.end())
            {
                std::map<int, int> &innerinfo = iter->second;
                auto inneriter = innerinfo.find(innerEffectId);
                if (inneriter != innerinfo.end())
                {
                    return inneriter->second;
                }
            }
            return 0;
        }

        void SetInnerEffectProtectCount(CharacterData *c, int parentEffectCasterId, int innerEffectId, int count)
        {
            c->innerEffectProtectCntMap[parentEffectCasterId][innerEffectId] = count;
        }

        int GetInnerEffectDebuffCount(CharacterData *c, int parentEffectCasterId, int innerEffectId)
        {
            if (c->innerEffectDebuffCntMap.size() <= 0)
                return 0;
            std::map<int, std::map<int, int>> &debuffinfo = c->innerEffectDebuffCntMap;
            auto iter = debuffinfo.find(parentEffectCasterId);
            if (iter != debuffinfo.end())
            {
                std::map<int, int> &innerinfo = iter->second;
                auto inneriter = innerinfo.find(innerEffectId);
                if (inneriter != innerinfo.end())
                {
                    return inneriter->second;
                }
            }
            return 0;
        }

        void SetInnerEffectDebuffCount(CharacterData *c, int parentEffectCasterId, int innerEffectId, int count)
        {
            c->innerEffectDebuffCntMap[parentEffectCasterId][innerEffectId] = count;
        }

        void SetTeamListInfo(CharacterData *c, int characterTeamListType, int aiTeamListType, int aiTeamside, int aiClusternum, bool aiIsExcludeSummon)
        {
            if (c) {
                CharacterDataListPtr list = c->aiContext->getTeamList(aiTeamListType, aiTeamside, aiClusternum, aiIsExcludeSummon);
                switch (characterTeamListType) {
                case CTEAM_FRIENDS:
                    c->friends = list;
                    break;
                case CTEAM_RIVALS:
                    c->rivals = list;
                    break;
                case CTEAM_TMP_FRIENDS:
                    c->m_pTmpFriends = list;
                    break;
                case CTEAM_TMP_RIVALS:
                    c->m_pTmpRivals = list;
                    break;
                default:
                    break;
                }
            }
        }

        bool CheckHasCaster(CharacterData *c, int casterType, int casterId)
        {
            if (c) {
                switch (casterType) {
                case eAnimationExec:
                    break;
                case eHurtExec:
                    for (AIContinuousHurtCaster *a_hurt_caster : c->hurtCasts) {
                        if (a_hurt_caster && a_hurt_caster->hurtID == casterId)
                            return true;
                    }
                    break;
                case eMoveExec:
                    for (AIContinuousMoveCaster *a_move_caster : c->moveCasts) {
                        if (a_move_caster && a_move_caster->moveID == casterId)
                            return true;
                    }
                    break;
                case eProjectTileExec:
                    for (AIContinuousProjectileCaster *a_projectile_caster : c->projectileCasts) {
                        if (a_projectile_caster && a_projectile_caster->projectileID == casterId)
                            return true;
                    }
                    break;
                case eEffectExec:
                    for (AIContinuousEffectCaster *an_effect_caster : c->effectCasts) {
                        if (an_effect_caster && an_effect_caster->effectID == casterId)
                            return true;
                    }
                    break;
                case eSpecialUIEffectExec:
                    break;
                case eSoundExec:
                    break;
                case eSpecialExec:
                    break;
                case eMarkExec:
                    for (AIContinuousMarkCaster *a_mark_caster : c->markCasts) {
                        if (a_mark_caster && a_mark_caster->markID == casterId)
                            return true;
                    }
                    break;
                case eSummonExec:
                    for (AIContinuousSummonCaster *a_summon_caster : c->summonCasts) {
                        if (a_summon_caster && a_summon_caster->summonExec->ID == casterId)
                            return true;
                    }
                    break;
                case eTransformExec:
                    break;
                case eBodyUIEffectExec:
                    break;
                default:
                    break;
                }

            }

            return false;
        }

        bool IsRivalTeamAllDeadForCheckUltra(CharacterData *c)
        {
            int rivalSide = TeamSide::kEnemy;
            if (CharacterFunc::IsEnemyTeam(c->teamSide))
                rivalSide = TeamSide::kOur;

            int check_alive_for_ultra_type = AIScriptBinder::getInstance()->getCommonConfigInteger(c->aiContext->getLuaState(), "cafut");
            if (check_alive_for_ultra_type == 0)
                return (c->aiContext->getTeamAliveNumExcludeAllSummon(rivalSide) < 1);
            else if (check_alive_for_ultra_type == 1) {
                int rivalClusterNum = c->aiContext->current_ClusterNumEnemy;
                if (CharacterFunc::IsEnemyTeam(c->teamSide))
                    rivalClusterNum = c->aiContext->current_ClusterNumOur;

                int aliveCnt = 0;
                int deathTriggeredCnt = 0;
                c->aiContext->getTeamAliveInfo(rivalSide, rivalClusterNum, aliveCnt, deathTriggeredCnt);
                return (aliveCnt < 1);
            }
            else if (check_alive_for_ultra_type == 2) {
                return luahelper_characterdata_is_rival_team_all_dead_for_check_ultra(c->aiContext->getLuaState(), c);
            }

            return false;
        }

        int64_t CalcTeamHPTopFunc(CharacterDataListPtr pCharacterList)
        {
            int64_t totalhptop = 0;
            for (auto iter = pCharacterList->begin(); iter != pCharacterList->end(); ++iter) {
                CharacterData *character = *iter;
                if (character->isSummon())
                    continue;
                totalhptop += character->getHPTop();
            }
            return totalhptop;
        }

        void ApplyEnergyDelta(CharacterData *c, int endelta)
        {
            if (c->hasEffect(315))
            {
                int absorben = luahelper_innereffect_315_trigger(AIContext::getInstance()->getLuaState(), c, endelta);
                if (endelta > 0)
                {
                    endelta -= absorben;
                    if (endelta < 0)
                    {
                        endelta = 0;
                    }
                }
                else
                {
                    endelta -= absorben;
                    if (endelta > 0)
                    {
                        endelta = 0;
                    }
                }
            }
            int olden = c->getEnergy();
            c->updateEnergy(c->getEnergy() + endelta);
            c->onENDelta(c->getEnergy() - olden);
        }

        bool HasLabel(CharacterData *c, int label)
        {
            auto &&labels = c->staticData->vLabels;
            if (std::find(labels.begin(), labels.end(), label) != labels.end())
            {
                return true;
            }
            return false;
        }

        //cui_2023_7_3 根据元素之力标签获取属性 {501 斩击 ,601 火 ,602 水,603 雷,604 毒 ,605 光 ,606 暗};
        extern int GetCharacterAttrByLabel(CharacterData* c, int label, bool isAtk)
        {
            switch (label)
            {
            case 501:
                return  isAtk ? c->getSlashAdd(): c->getSlashSub();
            case 601:
                return  isAtk ? c->getFireAdd() : c->getFireSub();
            case 602:
                return  isAtk ? c->getWaterAdd() : c->getWaterSub();
            case 603:
                return  isAtk ? c->getElectticAdd() : c->getElectticSub();
            case 604:
                return  isAtk ? c->getPosionAdd() : c->getPosionSub();
            case 605:
                return  isAtk ? c->getLightAdd() : c->getLightSub();
            case 606:
                return  isAtk ? c->getDarkAdd() : c->getDarkSub();

            default: //-1 
                return  isAtk ? c->getNoneAdd() : c->getNoneSub();
            }
        }

		void SetCharacterAttr(CharacterAttr *dst, int idx, int val)
		{
			switch (idx) {
			case ID: dst->c_id = val; break;
			case MAX_HP: dst->max_hp = val; break;
			case LV: dst->level = val; break;
			case X: dst->x = val; break;
			case Y: dst->y = val; break;
			case Z: dst->z = val; break;
			case PHY: dst->phy = val; break;
			case AGILE: dst->agile = val; break;
			case INT: dst->intelligence = val; break;
			case ACCURATE: dst->accurate = val; break;
			case SPIRIT: dst->spirit = val; break;
			case HP: dst->hp = val; break;
			case ENG: dst->eng = val; break;
			case SPEED: dst->speed = val; break;
			case PHY_AT: dst->physics_attack = val; break;
			case MAG_AT: dst->magic_attack = val; break;
			case PHY_CRIT: dst->physics_crit = val; break;
			case MAG_CRIT: dst->magic_crit = val; break;
			case PHY_CRIT_MUL: dst->physics_crit_mul = val; break;
			case MAG_CRIT_MUL: dst->magic_crit_mul = val; break;
			case PHY_HIT: dst->physics_hit = val; break;
			case MAG_HIT: dst->magic_hit = val; break;
			case PHY_DE: dst->physics_de = val; break;
			case MAG_DE: dst->magic_de = val; break;
			case PHY_PE: dst->physics_pe = val; break;
			case MAG_PE: dst->magic_pe = val; break;
			case DODGE_RATE: dst->dodge_rate = val; break;
			case RESIST_RATE: dst->resist_rate = val; break;
			case BLOOD_SUCK: dst->blood_suck = val; break;
			case HP_RECOVER: dst->hp_recover = val; break;
			case ENG_RECOVER: dst->eng_recover = val; break;
			case CURE: dst->cure = val; break;
			case ATT_INTER: dst->attack_interval = val; break;
			case AT_DIST: dst->attack_dist = val; break;
			case PHY_CRIT_RISIST: dst->physics_crit_resist = val; break;
			case MAG_CRIT_RISIST: dst->magic_crit_resist = val; break;
			case PHY_CRIT_MUL_ADD: dst->physics_crit_mul_add = val; break;
			case MAG_CRIT_MUL_ADD: dst->magic_crit_mul_add = val; break;
			case FUNC_HURT_ADD: dst->func_hurt_add = val; break;
			case FUNC_HURT_SUB: dst->func_hurt_sub = val; break;
			case PHY_CRIT_ADD: dst->physics_crit_add = val; break;
			case MAG_CRIT_ADD: dst->magic_crit_add = val; break;

                //cui_2023_3_60 yunsushuxi
            case SLASH_ADD: dst->slash_add = val; break;
            case SLASH_SUB: dst->slash_sub = val; break;
            case LIGHT_ADD: dst->light_add = val; break;
            case LIGHT_SUB: dst->light_sub = val; break;
            case DARK_ADD: dst->dark_add = val; break;
            case DARK_SUB: dst->dark_sub = val; break;
            case WATER_ADD: dst->water_add = val; break;
            case WATER_SUB: dst->water_sub = val; break;
            case POISON_ADD: dst->poison_add = val; break;
            case POISON_SUB: dst->poison_sub = val; break;
            case ELECTRIC_ADD: dst->electric_add = val; break;
            case ELECTRIC_SUB: dst->electric_sub = val; break;
            case FIRE_ADD: dst->fire_add = val; break;
            case FIRE_SUB: dst->fire_sub = val; break;
            case NONE_ADD: dst->none_add = val; break;
            case NONE_SUB: dst->none_sub = val; break;

			default:
				break;
			}
		}


		int GetCharacterAttr(CharacterAttr *dst, int idx)
		{
			int ret = 0;
			switch (idx) {
			case ID: ret = dst->c_id; break;
			case MAX_HP: ret = dst->max_hp; break;
			case LV: ret = dst->level; break;
			case X: ret = dst->x; break;
			case Y: ret = dst->y; break;
			case Z: ret = dst->z; break;
			case PHY: ret = dst->phy; break;
			case AGILE: ret = dst->agile; break;
			case INT: ret = dst->intelligence; break;
			case ACCURATE: ret = dst->accurate; break;
			case SPIRIT: ret = dst->spirit; break;
			case HP: ret = dst->hp; break;
			case ENG: ret = dst->eng; break;
			case SPEED: ret = dst->speed; break;
			case PHY_AT: ret = dst->physics_attack; break;
			case MAG_AT: ret = dst->magic_attack; break;
			case PHY_CRIT: ret = dst->physics_crit; break;
			case MAG_CRIT: ret = dst->magic_crit; break;
			case PHY_CRIT_MUL: ret = dst->physics_crit_mul; break;
			case MAG_CRIT_MUL: ret = dst->magic_crit_mul; break;
			case PHY_HIT: ret = dst->physics_hit; break;
			case MAG_HIT: ret = dst->magic_hit; break;
			case PHY_DE: ret = dst->physics_de; break;
			case MAG_DE: ret = dst->magic_de; break;
			case PHY_PE: ret = dst->physics_pe; break;
			case MAG_PE: ret = dst->magic_pe; break;
			case DODGE_RATE: ret = dst->dodge_rate; break;
			case RESIST_RATE: ret = dst->resist_rate; break;
			case BLOOD_SUCK: ret = dst->blood_suck; break;
			case HP_RECOVER: ret = dst->hp_recover; break;
			case ENG_RECOVER: ret = dst->eng_recover; break;
			case CURE: ret = dst->cure; break;
			case ATT_INTER: ret = dst->attack_interval; break;
			case AT_DIST: ret = dst->attack_dist; break;
			case PHY_CRIT_RISIST: ret = dst->physics_crit_resist; break;
			case MAG_CRIT_RISIST: ret = dst->magic_crit_resist; break;
			case PHY_CRIT_MUL_ADD: ret = dst->physics_crit_mul_add; break;
			case MAG_CRIT_MUL_ADD: ret = dst->magic_crit_mul_add; break;
			case FUNC_HURT_ADD: ret = dst->func_hurt_add; break;
			case FUNC_HURT_SUB: ret = dst->func_hurt_sub; break;
			case PHY_CRIT_ADD: ret = dst->physics_crit_add; break;
			case MAG_CRIT_ADD: ret = dst->magic_crit_add; break;

                //cui_2023_3_60 yunsushuxi
            case SLASH_ADD: ret = dst->slash_add; break;
            case SLASH_SUB: ret = dst->slash_sub; break;
            case LIGHT_ADD: ret = dst->light_add; break;
            case LIGHT_SUB: ret = dst->light_sub; break;
            case DARK_ADD: ret = dst->dark_add; break;
            case DARK_SUB: ret = dst->dark_sub; break;
            case WATER_ADD: ret = dst->water_add; break;
            case WATER_SUB: ret = dst->water_sub; break;
            case POISON_ADD: ret = dst->poison_add; break;
            case POISON_SUB: ret = dst->poison_sub; break;
            case ELECTRIC_ADD: ret = dst->electric_add; break;
            case ELECTRIC_SUB: ret = dst->electric_sub; break;
            case FIRE_ADD: ret = dst->fire_add; break;
            case FIRE_SUB: ret = dst->fire_sub; break;
            case NONE_ADD: ret = dst->none_add; break;
            case NONE_SUB: ret = dst->none_sub; break;
			default:
				break;
			}
			return ret;
		}

		void CopyOldFashionAttributes(int *attlist, CharacterAttr *dst)
		{
			for (int i = 0; i < ATT_LEN; i++)
			{
				SetCharacterAttr(dst, i, attlist[i]);
			}
			//dst->c_id = attlist[ID];
			//dst->max_hp = attlist[MAX_HP];
			//dst->level = attlist[LV];
			//dst->x = attlist[X];
			//dst->y = attlist[Y];
			//dst->z = attlist[Z];
			//dst->phy = attlist[PHY];
			//dst->agile = attlist[AGILE];
			//dst->intelligence = attlist[INT];
			//dst->accurate = attlist[ACCURATE];
			//dst->spirit = attlist[SPIRIT];
			//dst->hp = attlist[HP];
			//dst->eng = attlist[ENG];
			//dst->speed = attlist[SPEED];
			//dst->physics_attack = attlist[PHY_AT];
			//dst->magic_attack = attlist[MAG_AT];
			//dst->physics_crit = attlist[PHY_CRIT];
			//dst->magic_crit = attlist[MAG_CRIT];
		}

    } // namespace CharacterFunc

} // namespace OnePiece

