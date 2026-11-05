//
//  AIContinuousHurtCaster.cpp
//  OnePiece
//
//  Created by Beileng Shan on 14/5/4.
//
//
#include "AIContinuousHurtCaster.h"
#include <iostream>
#include "AIContext.h"
#include "AITargetHelper.h"
#include "projectile/ProjectileForCharacter.h"
#include "projectile/ProjectileForLanding.h"
#include "action_hero/HeroSkillController.h"
#include "action_hero/HeroActionController.h"

namespace OnePiece{
    
    static std::map<int, AIContinuousHurtCaster *> s_remainCasters;
    
    static void initHurtCasterTargetContext(AITargetContext &ctx, AIContinuousHurtCaster *caster)
    {
        ctx.source = caster->target;
        if (caster->target)
            ctx.target = caster->target->target;
        ctx.skill_caster = caster->origin;
        ctx.projectile = nullptr;
        ctx.skill_id = caster->skillID;
        ctx.skill_level = caster->skillLevel;
        ctx.hurt_id = caster->hurtID;
        
        auto hurt_data = caster->staticData;
        ctx.search_type = hurt_data->target;
        ctx.range.x1 = hurt_data->x1;
        ctx.range.x2 = hurt_data->x2;
        ctx.range.y1 = hurt_data->z1;
        ctx.range.y2 = hurt_data->z2;
        ctx.search_params = &hurt_data->targetParams;
        ctx.filter_list = nullptr;
    }
    
    static void initHurtCasterFilterTargetContext(AITargetContext &ctx, AIContinuousHurtCaster *caster, std::list<CharacterData *> *filterlist)
    {
        ctx.source = caster->target;
        if (caster->target)
            ctx.target = caster->target->target;
        ctx.skill_caster = caster->origin;
        ctx.projectile = nullptr;
        ctx.skill_id = caster->skillID;
        ctx.skill_level = caster->skillLevel;
        ctx.hurt_id = caster->hurtID;
        
        auto hurt_data = caster->staticData;
        ctx.search_type = hurt_data->filterTarget;
        ctx.range.x1 = hurt_data->filterX1;
        ctx.range.x2 = hurt_data->filterX2;
        ctx.range.y1 = hurt_data->filterZ1;
        ctx.range.y2 = hurt_data->filterZ2;
        ctx.search_params = &hurt_data->filterParams;
        ctx.filter_list = filterlist;
    }
    
    static void initHurtCasterExcludeTargetContext(AITargetContext &ctx, AIContinuousHurtCaster *caster)
    {
        ctx.source = caster->target;
        if (caster->target)
            ctx.target = caster->target->target;
        ctx.skill_caster = caster->origin;
        ctx.projectile = nullptr;
        ctx.skill_id = caster->skillID;
        ctx.skill_level = caster->skillLevel;
        ctx.hurt_id = caster->hurtID;
        
        auto hurt_data = caster->staticData;
        ctx.search_type = hurt_data->targetExclude;
        ctx.range.x1 = hurt_data->excludeX1;
        ctx.range.x2 = hurt_data->excludeX2;
        ctx.range.y1 = hurt_data->excludeZ1;
        ctx.range.y2 = hurt_data->excludeZ2;
        ctx.search_params = &hurt_data->targetExcludeParams;
        ctx.filter_list = nullptr;
    }
    
    static void initHurtCasterSimpleTargetContext(AITargetContext &ctx, CharacterData *s, CharacterData *t, CharacterData *sc, int skillId, int skillLevel, HurtExec *staticData, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter, bool isUseFilterParam, std::list<CharacterData *> *filterList)
    {
        ctx.source = s;
        ctx.target = t;
        ctx.skill_caster = sc;
        ctx.projectile = nullptr;
        ctx.skill_id = skillId;
        ctx.skill_level = skillLevel;
        ctx.hurt_id = staticData->ID;
        ctx.exclude_character = excludeCharacter;
        ctx.should_hp_ratio_higher_than_exclude_character = shouldHpRatioHigherThanExcludeCharacter;
		if (shouldHpRatioHigherThanExcludeCharacter)
			ctx.lowerhplimit_character = excludeCharacter;
        
        if (!isUseFilterParam) {
            ctx.search_type = staticData->target;
            ctx.range.x1 = staticData->x1;
            ctx.range.x2 = staticData->x2;
            ctx.range.y1 = staticData->z1;
            ctx.range.y2 = staticData->z2;
            ctx.search_params = &staticData->targetParams;
            ctx.filter_list = nullptr;
        } else {
            ctx.search_type = staticData->filterTarget;
            ctx.range.x1 = staticData->filterX1;
            ctx.range.x2 = staticData->filterX2;
            ctx.range.y1 = staticData->filterZ1;
            ctx.range.y2 = staticData->filterZ2;
            ctx.search_params = &staticData->filterParams;
            ctx.filter_list = filterList;
        }
    }
    
    static void initHurtCasterSimpleExcludeTargetContext(AITargetContext &ctx, CharacterData *s, CharacterData *t, CharacterData *sc, int skillId, int skillLevel, HurtExec *staticData, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter)
    {
        ctx.source = s;
        ctx.target = t;
        ctx.skill_caster = sc;
        ctx.projectile = nullptr;
        ctx.skill_id = skillId;
        ctx.skill_level = skillLevel;
        ctx.hurt_id = staticData->ID;
        ctx.exclude_character = excludeCharacter;
        ctx.should_hp_ratio_higher_than_exclude_character = shouldHpRatioHigherThanExcludeCharacter;
        
        ctx.search_type = staticData->targetExclude;
        ctx.range.x1 = staticData->excludeX1;
        ctx.range.x2 = staticData->excludeX2;
        ctx.range.y1 = staticData->excludeZ1;
        ctx.range.y2 = staticData->excludeZ2;
        ctx.search_params = &staticData->targetExcludeParams;
        ctx.filter_list = nullptr;
    }
    
    void AIContinuousHurtCaster::addRemainCaster(AIContinuousHurtCaster *aCaster)
    {
        s_remainCasters[aCaster->myID] = aCaster;
    }
    
    void AIContinuousHurtCaster::cleanRemainCaster()
    {
        for (auto iter = s_remainCasters.begin(); iter != s_remainCasters.end(); ++iter) {
            AIContinuousHurtCaster *pCaster = iter->second;
            if (pCaster)
                delete pCaster;
            else {
                std::cout << "ai:WARNING! AIContinuousHurtCaster pointer double freed!\n";
                //assert(0);
            }
        }
        
        s_remainCasters.clear();
    }

	AIContinuousHurtCaster::AIContinuousHurtCaster(int hurtID, int skillID, int skillLevel, CharacterData* origin, AIProjectile* projectile, CharacterData* target, AIContext& aiContext, int theMarkID)
        : AINode(target->myID, target->clusterNum)
	{
		myID = aiContext.local_incremential++;

		this->hurtID = hurtID;
		staticData = AIDataHelper::getHurtExec(hurtID);
		this->origin = origin;
		this->target = target;
		this->projectile = projectile;
        this->skillID = skillID;
		this->skillLevel = skillLevel;
        this->markID = theMarkID;

        if (!staticData)
            totalTick = 0;
        else
            totalTick = staticData->duration;//from 0
		localTick = aiContext.currentTick;//external tick num of context
        freezeTick = localTick;
		startTick = localTick;
        
        if (origin->attackSpeed != 100 && origin->attackSpeed > 0 && origin->myID == target->myID)
        {
            totalTick = totalTick * origin->attackSpeed / 100;
        }
        
		m_bContinueHurt = false;
	
		mStationaryDrt = 0;
		mStationaryPosX = 0;
		mStationaryPosY = 0;


		if (staticData != nullptr && target != nullptr)
		{
			target->mStationaryDirection = 0;
			target->mStationaryX = 0;
			target->mStationaryY = 0;

            switch (HeroSkillController::getRealSearchInfo(staticData->target).realSearchType) {
            case TargetAbsoluteRangeEnemy:
            case TargetAbsoluteRangeFriend:
            case TargetAbsoluteRangeAll:
            case TargetAbsoluteRangeAllWithoutSearcher:
                mStationaryDrt = target->direction;
                mStationaryPosX = target->getX();
                mStationaryPosY = target->getY();
                break;
            default:
                break;
            }
		}

		if (staticData->duration > 0 && staticData->freq > 0)
			m_bContinueHurt = staticData->duration / staticData->freq > 1;

        if (m_bContinueHurt)
        {
            firstRound = true;
        }
        pFirstRoundConfirmTarget = nullptr;

        ignoreFreeze = target->isCaptain();

		target->hurtCasts.push_back(this);
	}

	AIContinuousHurtCaster::~AIContinuousHurtCaster()
	{
//		for (std::list<AIContinuousHurtCaster*>::iterator iter = target->hurtCasts.begin(); iter != target->hurtCasts.end();)
//		{
//			if (*iter == this)
//			{
//				target->hurtCasts.erase(iter);
//				break;
//			}
//			iter++;
//		}
//
//		mStationaryDrt = 0;
//		mStationaryPosX = 0;
//		mStationaryPosY = 0;
//
//		if (target != nullptr)
//		{
//			target->mStationaryDirection = 0;
//			target->mStationaryX = 0;
//			target->mStationaryY = 0;
//		}
//
//		firstRound = false;
//		pFirstRoundConfirmTarget = nullptr;
        
        if (s_remainCasters.find(myID) != s_remainCasters.end())
            s_remainCasters[myID] = nullptr;
	}
    
    void AIContinuousHurtCaster::onFree()
    {
        for (std::list<AIContinuousHurtCaster*>::iterator iter = target->hurtCasts.begin(); iter != target->hurtCasts.end();)
        {
            if (*iter == this)
            {
                target->hurtCasts.erase(iter);
                break;
            }
            iter++;
        }
        
        mStationaryDrt = 0;
        mStationaryPosX = 0;
        mStationaryPosY = 0;
        
        if (target != nullptr)
        {
            target->mStationaryDirection = 0;
            target->mStationaryX = 0;
            target->mStationaryY = 0;
        }
        
        firstRound = false;
        pFirstRoundConfirmTarget = nullptr;
    }

	void AIContinuousHurtCaster::ExcuteOld(AIContext& aiContext)
	{
		AINode::update(aiContext);
		//if (isFrozen) return;
        if (freezeTick > localTick)
            localTick = freezeTick;
        int myTick = localTick;

        if (localTick < startTick + 1)
            return;

        int delayTick = 0;
        if (staticData)
            delayTick = staticData->delayFrameTick;
        int freqTick = 0;
        if (staticData)
            freqTick = staticData->freq;
        if (origin->attackSpeed != 100 && origin->attackSpeed > 0 && origin->myID == target->myID)
        {
            delayTick = delayTick * origin->attackSpeed / 100;
            freqTick = freqTick * origin->attackSpeed / 100;

            if (staticData)
            {
                if (staticData->delayFrameTick > 0 && delayTick <= 0)
                    delayTick = 1;
                if (staticData->freq > 0 && freqTick <= 0)
                    freqTick = 1;
            }
        }

        /*
        if (delayTick > 0)
        {// 延迟执行帧数
            if ((myTick - startTick - 1) < delayTick)
            {
                return;
            }

            if ((totalTick == 0 && (myTick - (startTick + delayTick) - 1) > totalTick)
                || (totalTick != 0 && (myTick - (startTick + delayTick) - 1) >= totalTick)
                )
            {
                delete this;
                return;
            }



            if (freqTick > 0 && (int)((myTick - (startTick + delayTick) - 1) / freqTick)*freqTick != (myTick - (startTick + delayTick) - 1))
            {
                return;
            }
        }
        else
        {
            if ((totalTick == 0 && (myTick - startTick - 1) > totalTick)
                || (totalTick != 0 && (myTick - startTick - 1) >= totalTick)
                )
            {
                delete this;
                return;
            }

            if (freqTick > 0 && (int)((myTick - startTick - 1) / freqTick)*freqTick != (myTick - startTick - 1))
            {
                return;
            }
        }
        */

        int deltaTick = myTick - startTick - 1;
        if (delayTick > 0)
        {
            if (deltaTick < delayTick)
                return;

            deltaTick -= delayTick;
        }

        if ((totalTick == 0 && deltaTick > totalTick) || (totalTick != 0 && deltaTick >= totalTick))
        {
            onFree();
            delete this;
            return;
        }

        if (staticData && staticData->isDeadClear && HeroActionController::isDeadAction(origin->currentAction))
        {
            onFree();
            delete this;
            return;
        }

        if (freqTick > 0 && (int)(deltaTick / freqTick) * freqTick != deltaTick)
        {
            return;
        }

        if (!staticData)
        {
            return;
        }

        // HQ: delete from 8.0
        // 防止出现开场没有设置敌人目标，导致目标即使是自己也不执行的问题
		//if (target->target!=nullptr)
		{
            std::shared_ptr<std::list<CharacterData*> > currentTargets = nullptr;
            //std::shared_ptr<std::list<CharacterData*> > secondTargets = nullptr;
            bool hasComfirmedTarget = false;
            auto realTargetType = HeroSkillController::getRealSearchInfo(staticData->target).realSearchType;
            if (realTargetType == TargetOneEnemyRandomContinueHurtNotRandomTarget)
			{
				if (m_bContinueHurt)
				{
					if (firstRound)
					{
						//currentTargets = AIActionController::searchTarget(*target, *target->target, nullptr, aiContext, staticData->target, staticData->x1, staticData->x2, staticData->z1, staticData->z2, staticData->z1, staticData->z2, *this->origin);
					}
					else
					{
						if (pFirstRoundConfirmTarget != nullptr)
						{
                            currentTargets = std::make_shared<std::list<CharacterData*> >();
							currentTargets->push_back(pFirstRoundConfirmTarget);
                            hasComfirmedTarget = true;
						}
					}
				}
				else
				{
					//currentTargets = AIActionController::searchTarget(*target, *target->target, nullptr, aiContext, staticData->target, staticData->x1, staticData->x2, staticData->z1, staticData->z2, staticData->z1, staticData->z2, *this->origin);
				}
			}
			else
			{
                if (staticData != nullptr && realTargetType == TargetAbsoluteRangeEnemy || realTargetType == TargetAbsoluteRangeFriend || realTargetType == TargetAbsoluteRangeAll || realTargetType == TargetAbsoluteRangeAllWithoutSearcher)
				{
					target->mStationaryDirection = mStationaryDrt;
					target->mStationaryX = mStationaryPosX;
					target->mStationaryY = mStationaryPosY;
				}
				//currentTargets = AIActionController::searchTarget(*target, *target->target, nullptr, aiContext, staticData->target, staticData->x1, staticData->x2, staticData->z1, staticData->z2, staticData->z1, staticData->z2, *this->origin);
			}

            if (!hasComfirmedTarget)
                currentTargets = AIActionController::searchTarget(*target, target->target, nullptr, aiContext, staticData->target, staticData->x1, staticData->x2, staticData->z1, staticData->z2, staticData->z1, staticData->z2, &staticData->targetParams, *this->origin, false, skillID, hurtID, skillLevel);

			/** clair_wang
			二次筛选
			**/
            if (staticData->filterTarget >= 0)
            {
                auto realFilterType = HeroSkillController::getRealSearchInfo(staticData->filterTarget).realSearchType;
                if (realFilterType == TargetEnemyMaxHurt || realFilterType == TargetTraversal)
                {
                    currentTargets = AIActionController::searchTarget(*target, target->target, nullptr, aiContext, staticData->filterTarget, staticData->filterX1, staticData->filterX2, staticData->filterZ1, staticData->filterZ2, staticData->filterZ1, staticData->filterZ2, &staticData->filterParams, *this->origin, false, skillID, hurtID, skillLevel, currentTargets.get());
                }
                else
                {
                    auto secondTargets = AIActionController::searchTarget(*target, target->target, nullptr, aiContext, staticData->filterTarget, staticData->filterX1, staticData->filterX2, staticData->filterZ1, staticData->filterZ2, staticData->filterZ1, staticData->filterZ2, &staticData->filterParams, *this->origin, false, skillID, hurtID, skillLevel);

                    auto pFinalTargets = std::make_shared<std::list<CharacterData*> >();

                    if (currentTargets != nullptr && secondTargets != nullptr)
                    {
                        for (auto iter = currentTargets->begin(); iter != currentTargets->end(); ++iter)
                        {
                            for (auto it = secondTargets->begin(); it != secondTargets->end(); ++it)
                            {
                                if ((*it)->myID == (*iter)->myID)
                                {
                                    // find
                                    pFinalTargets->push_back((*it));
                                }
                            }
                        }

                        currentTargets = pFinalTargets;
                    }
                }
            }
            
            if (staticData->targetExclude >= 0)
            {
                auto excludeTargets = AIActionController::searchTarget(*target, target->target, nullptr, aiContext, staticData->targetExclude, staticData->excludeX1, staticData->excludeX2, staticData->excludeZ1, staticData->excludeZ2, staticData->excludeZ1, staticData->excludeZ2, &staticData->targetExcludeParams, *this->origin, false, skillID, hurtID, skillLevel);
                if (excludeTargets)
                {
                    for (auto iter = excludeTargets->begin(); iter != excludeTargets->end(); ++iter)
                        currentTargets->remove(*iter);
                }
            }

			if (currentTargets != nullptr)
			{
                if (realTargetType == TargetOneEnemyRandomContinueHurtNotRandomTarget)
				{
					if (m_bContinueHurt && firstRound && currentTargets->size() > 0)
					{
						pFirstRoundConfirmTarget = (*currentTargets->begin());
					}
				}

				for (std::list<CharacterData*>::iterator iter = currentTargets->begin(); iter != currentTargets->end(); iter++)
				{
					CharacterData* tmpTarget = *iter;
					if (HeroActionController::isDeadAction(tmpTarget->currentAction)) continue;

					if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *tmpTarget, eEFFECT_ONLY_HURT_BY_CONTINUE, 0) > 0)
					{
						if (m_bContinueHurt)
						{
                            CharacterData *theTransTarget = nullptr;
                            if (tmpTarget->hasHurtTransFunc())
                            {
                                int theSimuHpDelta = HeroHurtcalculate::simulateCalcHurt(aiContext, origin, projectile, hurtID, skillID, skillLevel, tmpTarget);
                                theTransTarget = tmpTarget->getHurtTransTarget(theSimuHpDelta, hurtID);
                            }
                            if (theTransTarget)
                            {
                                HeroHurtcalculate::ExertHurt(aiContext, origin, projectile, hurtID, skillID, skillLevel, theTransTarget, markID);
                            }
                            else
                            {
                                HeroHurtcalculate::ExertHurt(aiContext, origin, projectile, hurtID, skillID, skillLevel, tmpTarget, markID);
                            }
						}//是否要执行免疫效果
					}
					else
					{
                        CharacterData *theTransTarget = nullptr;
                        if (tmpTarget->hasHurtTransFunc())
                        {
                            int theSimuHpDelta = HeroHurtcalculate::simulateCalcHurt(aiContext, origin, projectile, hurtID, skillID, skillLevel, tmpTarget);
                            theTransTarget = tmpTarget->getHurtTransTarget(theSimuHpDelta, hurtID);
                        }
                        if (theTransTarget)
                        {
                            HeroHurtcalculate::ExertHurt(aiContext, origin, projectile, hurtID, skillID, skillLevel, theTransTarget, markID);
                        }
                        else
                        {
                            HeroHurtcalculate::ExertHurt(aiContext, origin, projectile, hurtID, skillID, skillLevel, tmpTarget, markID);
                        }
					}

				}
			}
		}

		firstRound = false;

	}
    
    void AIContinuousHurtCaster::Excute(AIContext &aiContext)
    {
        if (AITargetHelper::isUseNewHelper())
            ExcuteNew(aiContext);
        else
            ExcuteOld(aiContext);
    }
    
    void AIContinuousHurtCaster::ExcuteNew(AIContext &aiContext)
    {
        AINode::update(aiContext);
        //if (isFrozen) return;
        if (freezeTick > localTick)
            localTick = freezeTick;
        int myTick = localTick;
        
        if (shouldDeleteSelf)
        {
            onFree();
            delete this;
            return;
        }
        if (localTick < startTick + 1)
            return;
        
        int delayTick = 0;
        if (staticData)
            delayTick = staticData->delayFrameTick;
        int freqTick = 0;
        if (staticData)
            freqTick = staticData->freq;
        if (origin->attackSpeed != 100 && origin->attackSpeed > 0 && origin->myID == target->myID)
        {
            delayTick = delayTick * origin->attackSpeed / 100;
            freqTick = freqTick * origin->attackSpeed / 100;
            
            if (staticData)
            {
                if (staticData->delayFrameTick > 0 && delayTick <= 0)
                    delayTick = 1;
                if (staticData->freq > 0 && freqTick <= 0)
                    freqTick = 1;
            }
        }
        
        int deltaTick = myTick - startTick - 1;
        if (delayTick > 0)
        {
            if (deltaTick < delayTick)
                return;
            
            deltaTick -= delayTick;
        }
        
        if ((totalTick == 0 && deltaTick > totalTick) || (totalTick != 0 && deltaTick >= totalTick))
        {
            onFree();
            delete this;
            return;
        }
        
        if (staticData && staticData->isDeadClear && HeroActionController::isDeadAction(origin->currentAction))
        {
            onFree();
            delete this;
            return;
        }

        if (!isValid)
            return;
        
        if (freqTick > 0 && (int)(deltaTick / freqTick) * freqTick != deltaTick)
            return;
        
        if (!staticData)
            return;
        
        // HQ: delete from 8.0
        // 防止出现开场没有设置敌人目标，导致目标即使是自己也不执行的问题
//        if (!target->target) {
//            firstRound = false;
//            return;
//        }
        
        std::list<CharacterData*> currentTargets;
        bool hasComfirmedTarget = false;
        auto realTargetType = AITargetHelper::parseTargetType(staticData->target, nullptr, nullptr, nullptr, nullptr);
        if (realTargetType == TargetOneEnemyRandomContinueHurtNotRandomTarget) {
            if (m_bContinueHurt && !firstRound) {
                if (pFirstRoundConfirmTarget != nullptr) {
                    currentTargets.push_back(pFirstRoundConfirmTarget);
                    hasComfirmedTarget = true;
                }
            }
        } else {
            if (realTargetType == TargetAbsoluteRangeEnemy || realTargetType == TargetAbsoluteRangeFriend || realTargetType == TargetAbsoluteRangeAll || realTargetType == TargetAbsoluteRangeAllWithoutSearcher) {
                target->mStationaryDirection = mStationaryDrt;
                target->mStationaryX = mStationaryPosX;
                target->mStationaryY = mStationaryPosY;
            }
        }
        
        AITargetContext ctx;
        //ctx.initWithHurtExec(staticData, target, target->target, origin, nullptr, skillID, skillLevel);
        initHurtCasterTargetContext(ctx, this);
        
        if (!hasComfirmedTarget)
            AITargetHelper::searchTarget(&ctx, &currentTargets);
        
        if (staticData->filterTarget >= 0) {
            auto realFilterType = AITargetHelper::parseTargetType(staticData->filterTarget, nullptr, nullptr, nullptr, nullptr);
            if (realFilterType == TargetEnemyMaxHurt || realFilterType == TargetTraversal) {
                std::list<CharacterData*> filterTargets = currentTargets;
                //ctx.initWithFilterHurtExec(staticData, target, target->target, origin, nullptr, skillID, skillLevel, &filterTargets);
                initHurtCasterFilterTargetContext(ctx, this, &filterTargets);
                currentTargets.clear();
                AITargetHelper::searchTarget(&ctx, &currentTargets);
            } else {
                std::list<CharacterData*> secondTargets;
                //ctx.initWithFilterHurtExec(staticData, target, target->target, origin, nullptr, skillID, skillLevel, nullptr);
                initHurtCasterFilterTargetContext(ctx, this, nullptr);
                AITargetHelper::searchTarget(&ctx, &secondTargets);
                
                std::list<CharacterData*> finalTargets;
                
                if (currentTargets.size() && secondTargets.size()) {
                    for (auto c1 : currentTargets) {
                        for (auto c2 : secondTargets) {
                            if (c1->myID == c2->myID) {
                                finalTargets.push_back(c1);
                            }
                        }
                    }
                }
                
                currentTargets = finalTargets;
            }
        }
        
        if (staticData->targetExclude >= 0) {
            std::list<CharacterData *> excludeTargets;
            AITargetContext ctxExclude;
            initHurtCasterExcludeTargetContext(ctxExclude, this);
            AITargetHelper::searchTarget(&ctxExclude, &excludeTargets);
            for (auto c : excludeTargets) {
                currentTargets.remove(c);
            }
        }

        if (realTargetType == TargetOneEnemyRandomContinueHurtNotRandomTarget) {
            if (m_bContinueHurt && firstRound && currentTargets.size() > 0)
                pFirstRoundConfirmTarget = (*currentTargets.begin());
        }
                
        for (auto c : currentTargets) {
            CharacterData* tmpTarget = c;
            
            if (HeroActionController::isDeadAction(tmpTarget->currentAction))
                continue;

            if (!isValid)
                break;
            
            if (tmpTarget->hasEffect(eEFFECT_ONLY_HURT_BY_CONTINUE)) {
                if (m_bContinueHurt) {
                    CharacterData *theTransTarget = nullptr;
                    if (tmpTarget->hasHurtTransFunc()) {
                        int theSimuHpDelta = HeroHurtcalculate::simulateCalcHurt(aiContext, origin, projectile, hurtID, skillID, skillLevel, tmpTarget);
                        theTransTarget = tmpTarget->getHurtTransTarget(theSimuHpDelta, hurtID);
                    }
                    if (theTransTarget)
                        HeroHurtcalculate::ExertHurt(aiContext, origin, projectile, hurtID, skillID, skillLevel, theTransTarget, markID);
                    else
                        HeroHurtcalculate::ExertHurt(aiContext, origin, projectile, hurtID, skillID, skillLevel, tmpTarget, markID);
                }
            } else {
                CharacterData *theTransTarget = nullptr;
                if (tmpTarget->hasHurtTransFunc()) {
                    int theSimuHpDelta = HeroHurtcalculate::simulateCalcHurt(aiContext, origin, projectile, hurtID, skillID, skillLevel, tmpTarget);
                    theTransTarget = tmpTarget->getHurtTransTarget(theSimuHpDelta, hurtID);
                }
                if (theTransTarget)
                    HeroHurtcalculate::ExertHurt(aiContext, origin, projectile, hurtID, skillID, skillLevel, theTransTarget, markID);
                else
                    HeroHurtcalculate::ExertHurt(aiContext, origin, projectile, hurtID, skillID, skillLevel, tmpTarget, markID);
            }
            
        }
        
        firstRound = false;
    }
    
    std::shared_ptr<std::list<CharacterData*> > AIContinuousHurtCaster::SimpleSearchHurtCasterTarget(AIContext& aiContext, int hurtID, CharacterData *target, CharacterData *origin, int skillID, int skillLevel, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter)
    {
        auto staticData = AIDataHelper::getHurtExec(hurtID);
        if (!staticData)
            return nullptr;
        
        std::shared_ptr<std::list<CharacterData*> > retTargets = AIActionController::searchTarget(*target, target->target, nullptr, aiContext, staticData->target, staticData->x1, staticData->x2, staticData->z1, staticData->z2, staticData->z1, staticData->z2, &staticData->targetParams, *origin, false, skillID, hurtID, skillLevel, nullptr, excludeCharacter, shouldHpRatioHigherThanExcludeCharacter);
        
        if (staticData->filterTarget >= 0)
        {
            auto realFilterType = HeroSkillController::getRealSearchInfo(staticData->filterTarget).realSearchType;
            if (realFilterType == TargetEnemyMaxHurt || realFilterType == TargetTraversal)
            {
                retTargets = AIActionController::searchTarget(*target, target->target, nullptr, aiContext, staticData->filterTarget, staticData->filterX1, staticData->filterX2, staticData->filterZ1, staticData->filterZ2, staticData->filterZ1, staticData->filterZ2, &staticData->filterParams, *origin, false, skillID, hurtID, skillLevel, retTargets.get(), excludeCharacter);
            }
            else
            {
                auto secondTargets = AIActionController::searchTarget(*target, target->target, nullptr, aiContext, staticData->filterTarget, staticData->filterX1, staticData->filterX2, staticData->filterZ1, staticData->filterZ2, staticData->filterZ1, staticData->filterZ2, &staticData->filterParams, *origin, false, skillID, hurtID, skillLevel, nullptr, excludeCharacter, shouldHpRatioHigherThanExcludeCharacter);
                
                auto pFinalTargets = std::make_shared<std::list<CharacterData*> >();
                
                if (retTargets != nullptr && secondTargets != nullptr)
                {
                    for (auto iter = retTargets->begin(); iter != retTargets->end(); ++iter)
                    {
                        for (auto it = secondTargets->begin(); it != secondTargets->end(); ++it)
                        {
                            if ((*it)->myID == (*iter)->myID)
                            {
                                // find
                                pFinalTargets->push_back((*it));
                            }
                        }
                    }
                    
                    retTargets = pFinalTargets;
                }
            }
        }
        
        if (staticData->targetExclude >= 0)
        {
            auto excludeTargets = AIActionController::searchTarget(*target, target->target, nullptr, aiContext, staticData->targetExclude, staticData->excludeX1, staticData->excludeX2, staticData->excludeZ1, staticData->excludeZ2, staticData->excludeZ1, staticData->excludeZ2, &staticData->targetExcludeParams, *origin, false, skillID, hurtID, skillLevel, nullptr, excludeCharacter, shouldHpRatioHigherThanExcludeCharacter);
            if (excludeTargets)
            {
                for (auto iter = excludeTargets->begin(); iter != excludeTargets->end(); ++iter)
                    retTargets->remove(*iter);
            }
        }
        
        return retTargets;
    }
    
    void AIContinuousHurtCaster::SimpleSearchHurtCasterTargetNew(AIContext& aiContext, int hurtID, CharacterData *target, CharacterData *origin, int skillID, int skillLevel, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter, std::list<CharacterData *> *outlist)
    {
        if (!outlist)
            return;
        auto staticData = AIDataHelper::getHurtExec(hurtID);
        if (!staticData)
            return;
        
        AITargetContext ctx;
        initHurtCasterSimpleTargetContext(ctx, target, target->target, origin, skillID, skillLevel, staticData, excludeCharacter, shouldHpRatioHigherThanExcludeCharacter, false, nullptr);
        AITargetHelper::searchTarget(&ctx, outlist);
        
        if (staticData->filterTarget >= 0) {
            int realFilterType = AITargetHelper::parseTargetType(staticData->filterTarget, nullptr, nullptr, nullptr, nullptr);
            if (realFilterType == TargetEnemyMaxHurt || realFilterType == TargetTraversal) {
                std::list<CharacterData *> filterlist = *outlist;
                outlist->clear();
                initHurtCasterSimpleTargetContext(ctx, target, target->target, origin, skillID, skillLevel, staticData, excludeCharacter, shouldHpRatioHigherThanExcludeCharacter, true, &filterlist);
                AITargetHelper::searchTarget(&ctx, outlist);
            } else {
                std::list<CharacterData *> secondTargets;
                initHurtCasterSimpleTargetContext(ctx, target, target->target, origin, skillID, skillLevel, staticData, excludeCharacter, shouldHpRatioHigherThanExcludeCharacter, true, nullptr);
                AITargetHelper::searchTarget(&ctx, &secondTargets);
                
                std::list<CharacterData *> finalTargets;
                
                if (outlist->size() && secondTargets.size()) {
                    for (auto c1 : *outlist) {
                        for (auto c2 : secondTargets) {
                            if (c1->myID == c2->myID)
                                finalTargets.push_back(c1);
                        }
                    }
                }
                
                *outlist = finalTargets;
            }
        }
        
        if (staticData->targetExclude >= 0) {
            std::list<CharacterData *> excludeTargets;
            AITargetContext ctxExclude;
            initHurtCasterSimpleExcludeTargetContext(ctxExclude, target, target->target, origin, skillID, skillLevel, staticData, excludeCharacter, shouldHpRatioHigherThanExcludeCharacter);
            AITargetHelper::searchTarget(&ctxExclude, &excludeTargets);
            for (auto c : excludeTargets) {
                outlist->remove(c);
            }
        }
    }

}
