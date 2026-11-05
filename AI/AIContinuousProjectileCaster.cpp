//
//  AIContinuousProjectileCaster.cpp
//  OnePiece
//
//  Created by Beileng Shan on 14/5/4.
//
//

#include "AIContinuousProjectileCaster.h"
#include "AIContext.h"
#include "projectile/ProjectileForCharacter.h"
#include "projectile/ProjectileForLanding.h"
#include "projectile/ProjectileForBouncing.h"
#include "action_hero/HeroActionController.h"
#include "action_hero/HeroSkillController.h"
#include "AIScriptBinder.hpp"
#include "scriptbind/AIScriptBindProjectile.hpp"
#include "AITargetHelper.h"


namespace OnePiece{
    
    static void initProjectileTargetContext(AITargetContext &ctx, AIContinuousProjectileCaster *caster)
    {
        ctx.source = caster->origin;
        ctx.target = caster->origin->target;
        ctx.skill_caster = caster->origin;
        ctx.projectile = nullptr;
        ctx.skill_id = caster->skillID;
        ctx.skill_level = caster->skillLevel;
        ctx.hurt_id = 0;
        
        auto caster_data = caster->staticData;
        ctx.search_type = caster_data->searchTarget;
        ctx.range.x1 = caster_data->searchX1;
        ctx.range.x2 = caster_data->searchX2;
        ctx.range.y1 = caster_data->searchZ1;
        ctx.range.y2 = caster_data->searchZ2;
        ctx.search_params = &caster_data->searchParams;
        ctx.filter_list = nullptr;
    }
    
    static void initProjectileFilterTargetContext(AITargetContext &ctx, AIContinuousProjectileCaster *caster, std::list<CharacterData *> *filterlist)
    {
        ctx.source = caster->origin;
        ctx.target = caster->origin->target;
        ctx.skill_caster = caster->origin;
        ctx.projectile = nullptr;
        ctx.skill_id = caster->skillID;
        ctx.skill_level = caster->skillLevel;
        
        auto caster_data = caster->staticData;
        ctx.hurt_id = caster_data->hitExecVec[0]; // 99 类型约定只取列表中第一个ID，作参数  来查找目标
        ctx.search_type = caster_data->filterTarget;
        ctx.range.x1 = caster_data->filterX1;
        ctx.range.x2 = caster_data->filterX2;
        ctx.range.y1 = caster_data->filterZ1;
        ctx.range.y2 = caster_data->filterZ2;
        ctx.search_params = &caster_data->filterParams;
        ctx.filter_list = filterlist;
    }
    
    static void initProjectileExcludeTargetContext(AITargetContext &ctx, AIContinuousProjectileCaster *caster)
    {
        ctx.source = caster->origin;
        ctx.target = caster->origin->target;
        ctx.skill_caster = caster->origin;
        ctx.projectile = nullptr;
        ctx.skill_id = caster->skillID;
        ctx.skill_level = caster->skillLevel;
        
        auto caster_data = caster->staticData;
        ctx.hurt_id = caster_data->hitExecVec[0]; // 99 类型约定只取列表中第一个ID，作参数  来查找目标
        ctx.search_type = caster_data->targetExclude;
        ctx.range.x1 = caster_data->excludeX1;
        ctx.range.x2 = caster_data->excludeX2;
        ctx.range.y1 = caster_data->excludeZ1;
        ctx.range.y2 = caster_data->excludeZ2;
        ctx.search_params = &caster_data->targetExcludeParams;
        ctx.filter_list = nullptr;
    }

	AIContinuousProjectileCaster::AIContinuousProjectileCaster(int projectileID, CharacterData* origin, AIContext& aiContext, int skillID, int skillLevel) 
		: AINode(origin->myID, origin->clusterNum)
	{
		myID = aiContext.local_incremential++;

		this->projectileID = projectileID;
		staticData = AIDataHelper::getProjectileExec(projectileID);
		this->origin = origin;

        totalTick = staticData->throwDuration;
        if (staticData->lazyTick > 0)
            totalTick += staticData->lazyTick;
		localTick = aiContext.currentTick;//external tick num of context
		startTick = localTick;
        _currentLazyTick = staticData->lazyTick;
        this->skillID = skillID;
		this->skillLevel = skillLevel;

        ignoreFreeze = origin->isCaptain();

		origin->projectileCasts.push_back(this);
	}

	AIContinuousProjectileCaster::~AIContinuousProjectileCaster()
	{
		for (std::list<AIContinuousProjectileCaster*>::iterator iter = origin->projectileCasts.begin(); iter != origin->projectileCasts.end();++iter)
		{
			if (*iter == this)
			{
				origin->projectileCasts.erase(iter++);
				break;
			}
		}
	}

	void AIContinuousProjectileCaster::Excute(AIContext& aiContext)
	{
		AINode::update(aiContext);
		if (isFrozen) return;

		if ((totalTick == 0 && (localTick - startTick - 1) > totalTick)
			|| (totalTick != 0 && (localTick - startTick - 1) >= totalTick)
			|| (staticData->throwInterval == 0 && localTick - startTick - 1 > 0)
			)
		{
			delete this;
			return;
		}
        
        if (_currentLazyTick > 0)
        {
            if (_currentLazyTick == staticData->lazyTick)
            {
                implThrowProjectile(aiContext, true, false);
            }
            
            _currentLazyTick -= 1;
            
            return;
        }

		if (staticData->throwInterval > 0 && (localTick - startTick -1) % staticData->throwInterval!=0)
		{
			return;
		}
        
        // 9.0 new impl
        bool shouldTurn = (staticData->lazyTick >= 0);
        implThrowProjectile(aiContext, shouldTurn, true);
        
        /*
		int size = 0;
		std::vector<CharacterData*> tmpVec;
		std::list<CharacterData*>::iterator iter;
        CharacterData* ptarget = nullptr;
        bool isAttackSelf = false;
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *origin, eEFFECT_FORCE_ATTACK_SELF, 0) > 0)
        {
            if (skillID == origin->normalSkill.first)
                isAttackSelf = true;
        }
         */

        /*
        auto friendList = origin->getCurrentFriendsList();
        auto rivalList = origin->getCurrentRivalsList();

        std::list<CharacterData*> tmpFriends;
        std::list<CharacterData*> tmpRivals;
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *origin, eEFFECT_TAUNT_EXTRA, 0) > 0)
        {
            tmpFriends.assign(rivalList->begin(), rivalList->end());
            tmpFriends.push_back(origin);

            for (auto tmpIter = friendList->begin(); tmpIter != friendList->end(); ++tmpIter)
            {
                auto theFriend = *tmpIter;
                if (theFriend->myID == origin->myID)
                    continue;
                tmpRivals.push_back(theFriend);
            }
            friendList = &tmpFriends;
            rivalList = &tmpRivals;
        }
        */

        /*
        auto friends = AIActionController::getFilteredFriendList(*origin, *origin, false);
        auto rivals = AIActionController::getFilteredRivalList(*origin, *origin, false);
        auto friendList = friends.get();
        auto rivalList = rivals.get();

		switch (staticData->throwType)
		{
		case EProjectileTargetType::eProjectileTargetCurrent:
            ptarget = origin->target;
            if (isAttackSelf)
            {
                ptarget = origin;
            }
            if (ptarget)
            {
                new ProjectileForLanding(aiContext, projectileID, origin, ptarget, ptarget->getX(), ptarget->getY(), ptarget->getZ(), skillID, skillLevel);
            }
			break;
		case EProjectileTargetType::eProjectileTargetNone:
            new ProjectileForLanding(aiContext, projectileID, origin, nullptr, origin->direction * 100000, origin->getY(), origin->getZ(), skillID, skillLevel);
			break;
		case EProjectileTargetType::eProjectileTargetDistance:
			if (staticData->deadType == 0)//time
                new ProjectileForLanding(aiContext, projectileID, origin, nullptr, origin->getX() + origin->direction * origin->getSpeed() * staticData->deadParam, origin->getY(), origin->getZ(), skillID, skillLevel);
			else if (staticData->deadType==1)//distanceDead
                new ProjectileForLanding(aiContext, projectileID, origin, nullptr, origin->getX() + origin->direction * staticData->deadParam, origin->getY(), origin->getZ(), skillID, skillLevel);
			break;
		case EProjectileTargetType::eProjectileTargetAllFriend:
            if (isAttackSelf)
            {
                new ProjectileForLanding(aiContext, projectileID, origin, origin, origin->getX(), origin->getY(), origin->getZ(), skillID, skillLevel);
            }
            else
            {
                iter = friendList->begin();
                while (iter != friendList->end())
                {
                    if (!HeroActionController::isDeadAction((*iter)->currentAction))
                        new ProjectileForLanding(aiContext, projectileID, origin, (*iter), (*iter)->getX(), (*iter)->getY(), (*iter)->getZ(), skillID, skillLevel);
                    ++iter;
                }
            }
			break;
		case EProjectileTargetType::eProjectileTargetAllEnemy:
            if (isAttackSelf)
            {
                new ProjectileForLanding(aiContext, projectileID, origin, origin, origin->getX(), origin->getY(), origin->getZ(), skillID, skillLevel);
            }
            else
            {
                iter = rivalList->begin();
                while (iter != rivalList->end())
                {
                    if (!HeroActionController::isDeadAction((*iter)->currentAction))
                        new ProjectileForLanding(aiContext, projectileID, origin, (*iter), (*iter)->getX(), (*iter)->getY(), (*iter)->getZ(), skillID, skillLevel);
                    ++iter;
                }
            }
			break;
		case EProjectileTargetType::eProjectileTargetRandomFriend:
            if (isAttackSelf)
            {
                new ProjectileForLanding(aiContext, projectileID, origin, origin, origin->getX(), origin->getY(), origin->getZ(), skillID, skillLevel);
            }
            else
            {
                size = friendList->size();
                iter = friendList->begin();
                tmpVec.clear();
                while (iter != friendList->end())
                {
                    if (!HeroActionController::isDeadAction((*iter)->currentAction)) tmpVec.push_back(*iter);
                    iter++;
                }
                if (tmpVec.size() > 0)
                {
                    for (int i = 0; i < staticData->throwNum; i++)
                    {
                        OutPutFileLine;
                        int inde = aiContext.localRandom->nextInt(0, tmpVec.size());
                        new ProjectileForLanding(aiContext, projectileID, origin, tmpVec[inde], tmpVec[inde]->getX(), tmpVec[inde]->getY(), tmpVec[inde]->getZ(), skillID, skillLevel);
                    }
                }
            }
			break;
		case EProjectileTargetType::eProjectileTargetRandomEnemy:
            if (isAttackSelf)
            {
                new ProjectileForLanding(aiContext, projectileID, origin, origin, origin->getX(), origin->getY(), origin->getZ(), skillID, skillLevel);
            }
            else
            {
                size = rivalList->size();
                iter = rivalList->begin();
                tmpVec.clear();
                while (iter != rivalList->end())
                {
                    if (!HeroActionController::isDeadAction((*iter)->currentAction)) tmpVec.push_back(*iter);
                    iter++;
                }
                if (tmpVec.size() > 0)
                {
                    for (int i = 0; i < staticData->throwNum; i++)
                    {
                        OutPutFileLine;
                        int inde = aiContext.localRandom->nextInt(0, tmpVec.size());
                        new ProjectileForLanding(aiContext, projectileID, origin, tmpVec[inde], tmpVec[inde]->getX(), tmpVec[inde]->getY(), tmpVec[inde]->getZ(), skillID, skillLevel);
                    }
                }
            }
			break;
        case EProjectileTargetType::eProjectileTarget_6:
        case EProjectileTargetType::eProjectileTarget_7:
        case EProjectileTargetType::eProjectileTarget_8:
        case EProjectileTargetType::eProjectileTarget_9:
            ptarget = origin->target;
            if (isAttackSelf)
                ptarget = origin;
            if (ptarget)
                new ProjectileForBouncing(aiContext, projectileID, origin, ptarget, skillID, skillLevel);
            break;
        case EProjectileTargetType::eProjectileTargetFollowCaster:
            ptarget = origin->target;
            if (isAttackSelf)
                ptarget = origin;
            new ProjectileForCharacter(aiContext, projectileID, origin, ptarget, skillID, skillLevel);
            break;
		case EProjectileTargetType::eProjectileTargetAllAttrHighest:
            if (isAttackSelf)
                ptarget = origin;
            else
                ptarget = AIActionController::getAttrEstCharacter(*origin, friendList, rivalList, staticData->throwParams, AIActionController::COMP_HIGHEST, false);
            if (ptarget && !HeroActionController::isDeadAction(ptarget->currentAction))
            {
                new ProjectileForLanding(aiContext, projectileID, origin, ptarget, ptarget->getX(), ptarget->getY(), ptarget->getZ(), skillID, skillLevel);
            }
			break;
		case EProjectileTargetType::eProjectileTargetFriendAttrHighest:
            if (isAttackSelf)
                ptarget = origin;
            else
                ptarget = AIActionController::getAttrEstCharacter(*origin, friendList, nullptr, staticData->throwParams, AIActionController::COMP_HIGHEST, false);
            if (ptarget && !HeroActionController::isDeadAction((*iter)->currentAction))
            {
                new ProjectileForLanding(aiContext, projectileID, origin, ptarget, ptarget->getX(), ptarget->getY(), ptarget->getZ(), skillID, skillLevel);
            }
			break;
		case EProjectileTargetType::eProjectileTargetEnemyAttrHighest:
            if (isAttackSelf)
                ptarget = origin;
            else
                ptarget = AIActionController::getAttrEstCharacter(*origin, rivalList, nullptr, staticData->throwParams, AIActionController::COMP_HIGHEST, false);
            if (ptarget && !HeroActionController::isDeadAction(ptarget->currentAction))
            {
                new ProjectileForLanding(aiContext, projectileID, origin, ptarget, ptarget->getX(), ptarget->getY(), ptarget->getZ(), skillID, skillLevel);
            }
			break;
		case EProjectileTargetType::eProjectileTargetAllAttrLowest:
            if (isAttackSelf)
                ptarget = origin;
            else
                ptarget = AIActionController::getAttrEstCharacter(*origin, friendList, rivalList, staticData->throwParams, AIActionController::COMP_LOWEST, false);
            if (ptarget && !HeroActionController::isDeadAction(ptarget->currentAction))
			{
                new ProjectileForLanding(aiContext, projectileID, origin, ptarget, ptarget->getX(), ptarget->getY(), ptarget->getZ(), skillID, skillLevel);
			}
			break;
		case EProjectileTargetType::eProjectileTargetFriendAttrLowest:
            if (isAttackSelf)
                ptarget = origin;
            else
                ptarget = AIActionController::getAttrEstCharacter(*origin, friendList, nullptr, staticData->throwParams, AIActionController::COMP_LOWEST, false);
            if (ptarget && !HeroActionController::isDeadAction((*iter)->currentAction))
			{
                new ProjectileForLanding(aiContext, projectileID, origin, ptarget, ptarget->getX(), ptarget->getY(), ptarget->getZ(), skillID, skillLevel);
			}
			break;
		case EProjectileTargetType::eProjectileTargetEnemyAttrLowest:
            if (isAttackSelf)
                ptarget = origin;
            else
                ptarget = AIActionController::getAttrEstCharacter(*origin, rivalList, nullptr, staticData->throwParams, AIActionController::COMP_LOWEST, false);
            if (ptarget && !HeroActionController::isDeadAction((*iter)->currentAction))
			{
                new ProjectileForLanding(aiContext, projectileID, origin, ptarget, ptarget->getX(), ptarget->getY(), ptarget->getZ(), skillID, skillLevel);
			}
			break;
        case EProjectileTargetType::eProjectileTargetOnSearch:
        {
            std::shared_ptr<std::list<CharacterData*>> targets = AIActionController::searchTarget(*origin, origin->target, nullptr, aiContext, staticData->searchTarget, staticData->searchX1, staticData->searchX2, staticData->searchZ1, staticData->searchZ2, staticData->searchZ1, staticData->searchZ2, &staticData->searchParams, *origin, false, skillID);
            if (staticData->filterTarget >= 0)
            {
                auto hurtID = staticData->hitExec;
                auto realTargetType = HeroSkillController::getRealSearchInfo(staticData->filterTarget).realSearchType;
                if ((realTargetType == TargetEnemyMaxHurt || realTargetType == TargetTraversal) && IsHurtExec(staticData->hitExec))
                {
                    targets = AIActionController::searchTarget(*origin, origin->target, nullptr, aiContext, staticData->filterTarget, staticData->filterX1, staticData->filterX2, staticData->filterZ1, staticData->filterZ2, staticData->filterZ1, staticData->filterZ2, &staticData->filterParams, *this->origin, false, skillID, hurtID, skillLevel, targets.get());
                }
                else
                {
                    auto secondTargets = AIActionController::searchTarget(*origin, origin->target, nullptr, aiContext, staticData->filterTarget, staticData->filterX1, staticData->filterX2, staticData->filterZ1, staticData->filterZ2, staticData->filterZ1, staticData->filterZ2, &staticData->filterParams, *this->origin, false, skillID, hurtID, skillLevel);

                    auto pFinalTargets = std::make_shared<std::list<CharacterData*> >();
                    for (auto iter = targets->begin(); iter != targets->end(); ++iter)
                    {
                        for (auto it = secondTargets->begin(); it != secondTargets->end(); ++it)
                        {
                            if ((*it)->myID == (*iter)->myID)
                                pFinalTargets->push_back((*it));
                        }
                    }

                    targets = pFinalTargets;
                }
            }

            iter = targets->begin();
            while (iter != targets->end())
            {
                if (!HeroActionController::isDeadAction((*iter)->currentAction)) new ProjectileForLanding(aiContext, projectileID, origin, (*iter), (*iter)->getX(), (*iter)->getY(), (*iter)->getZ(), skillID, skillLevel);
                ++iter;
            }
        }
            break;
		default:
			break;
		}
         */

		/*
		if (staticData->throwType == EProjectileTargetType::eProjectileEjectNormal //普通释放
			|| staticData->throwType == EProjectileTargetType::eProjectileEjectRandom
			)				  
		{
			if (staticData->hurtTrigger == 0 || staticData->throwType == EProjectileTargetType::eProjectileEjectRandom)
			{//0-无目标，随机找n个人的位置，来初始化projectile
				std::list<CharacterData*>* currentRivals = origin->rivals;
				int size = currentRivals->size();

				std::list<CharacterData*>::iterator iter = currentRivals->begin();
				std::vector<CharacterData*> tmpVec;
				while (iter != currentRivals->end())
				{
					if (!HeroActionController::isDeadAction((*iter)->currentAction)) tmpVec.push_back(*iter);
					iter++;
				}
				if (tmpVec.size() > 0)
				{
					for (int i = 0; i < staticData->throwNum; i++)
					{
						int inde = aiContext.localRandom->nextInt(0, tmpVec.size());
						new ProjectileForLanding(aiContext, projectileID, origin, tmpVec[inde]->getX(), tmpVec[inde]->getY(), 0, skillLevel);
					}
				}
			}
			else
			{//1-有目标,暂时忽略数目staticData->throwNum；每个目标一个projectile
				std::list<CharacterData*>* targetlist = HeroSkillController::getSkillTarget(aiContext, origin, (ESkillTargetType)staticData->throwType);
				std::list<CharacterData*>::iterator iter = targetlist->begin();
				while (iter != targetlist->end())
				{
					new ProjectileForCharacter(aiContext, projectileID, origin, *iter, skillLevel);
					iter++;
				}
				delete targetlist;
			}
		}
		else if (staticData->throwType == EProjectileTargetType::eProjectileEjectAll)              //全体释放
		{
			std::list<CharacterData*>* currentRivals = origin->rivals;
			int size = currentRivals->size();
			std::list<CharacterData*>::iterator iter = currentRivals->begin();
			while (iter!=currentRivals->end())
			{
				new ProjectileForCharacter(aiContext, projectileID, origin, *iter, skillLevel);
				iter++;
			}
		}
		else if (staticData->throwType == EProjectileTargetType::eProjectileEjectSameTarget		//弹射释放-可相同目标
			|| staticData->throwType == EProjectileTargetType::eProjectileEjectDifferentTarget	//弹射释放-不可相同目标
			)	
		{//TODO: check if there are cases
			std::list<CharacterData*>* currentRivals = origin->rivals;
			int size = currentRivals->size();
			std::list<CharacterData*>::iterator iter = currentRivals->begin();
			for (int i = 0; i < staticData->throwNum; i++)
			{
				new ProjectileForCharacter(aiContext, projectileID, origin, *iter, skillLevel);
			}
		}*/
    }
    
    void AIContinuousProjectileCaster::implThrowProjectile(AIContext& aiContext, bool shouldTurn, bool shouldThrow)
    {
        if (AIScriptBinder::getInstance()->isProjectileCasterThrowProjectileByLua(aiContext.getLuaState()))
            luahelper_projectilecaster_throw_projectile(aiContext.getLuaState(), this, shouldTurn, shouldThrow);
        else
            implThrowProjectileNative(aiContext, shouldTurn, shouldThrow);
    }
    
    void AIContinuousProjectileCaster::implThrowProjectileNative(AIContext& aiContext, bool shouldTurn, bool shouldThrow)
    {
        CharacterData* ptarget = nullptr;
        CharacterData* specialTarget = nullptr;
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *origin, eEFFECT_FORCE_ATTACK_SELF, 0) > 0)
        {
            if (skillID == origin->normalSkill.first)
            {
                specialTarget = origin;
            }
        }
        if (!specialTarget && origin->hasEffect(eEFFECT_FOCUS_TARGET) && origin->targetForFocus)
        {
            specialTarget = origin->targetForFocus;
        }
        
        if (EProjectileTargetType::eProjectileTargetCurrent == staticData->throwType)
        {
            ptarget = origin->target;
            if (specialTarget)
                ptarget = specialTarget;
            if (ptarget)
            {
                if (shouldTurn)
                {
                    origin->forceTurnToTarget(ptarget);
                }
                
                if (shouldThrow)
                {
                    //28.7 cui
                   // new ProjectileForLanding(aiContext, projectileID, origin, ptarget, ptarget->getX(), ptarget->getY(), ptarget->getZ(), skillID, skillLevel);
                    new ProjectileForLanding(aiContext, projectileID, origin, ptarget, this->getRealEndPos(staticData, ptarget->getX(),false), 
                        this->getRealEndPos(staticData, ptarget->getY(), true), ptarget->getZ(), skillID, skillLevel);
                }
            }
        }
        else if (EProjectileTargetType::eProjectileTargetNone == staticData->throwType)
        {
            if (shouldThrow)
            {
                new ProjectileForLanding(aiContext, projectileID, origin, nullptr, origin->direction * 100000, origin->getY(), origin->getZ(), skillID, skillLevel);
            }
        }
        else if (EProjectileTargetType::eProjectileTargetDistance == staticData->throwType)
        {
            if (shouldThrow)
            {
                if (staticData->deadType == 0)//time
                    new ProjectileForLanding(aiContext, projectileID, origin, nullptr, origin->getX() + origin->direction * origin->getSpeed() * staticData->deadParam, origin->getY(), origin->getZ(), skillID, skillLevel);
                else if (staticData->deadType==1)//distanceDead
                    new ProjectileForLanding(aiContext, projectileID, origin, nullptr, origin->getX() + origin->direction * staticData->deadParam, origin->getY(), origin->getZ(), skillID, skillLevel);
            }
        }
        else if (EProjectileTargetType::eProjectileTargetAllFriend == staticData->throwType ||
                 EProjectileTargetType::eProjectileTargetAllEnemy == staticData->throwType)
        {
            if (shouldThrow)
            {
                if (specialTarget)
                {
                    //28.7 cui
                    //new ProjectileForLanding(aiContext, projectileID, origin, specialTarget, origin->getX(), origin->getY(), origin->getZ(), skillID, skillLevel);
                    new ProjectileForLanding(aiContext, projectileID, origin, specialTarget, this->getRealEndPos(staticData, origin->getX(),false),
                        this->getRealEndPos(staticData, origin->getY(), true), origin->getZ(), skillID, skillLevel);
                }
                else
                {
                    std::unique_ptr<CharacterDataList> targetList = nullptr;
                    if (EProjectileTargetType::eProjectileTargetAllFriend == staticData->throwType)
                        targetList = AIActionController::getFilteredFriendList(*origin, *origin, false, false, false, nullptr, TargetFriendAll);
                    else if (EProjectileTargetType::eProjectileTargetAllEnemy == staticData->throwType)
                        targetList = AIActionController::getFilteredRivalList(*origin, *origin, false, false, false, nullptr, TargetEnemyAll);
                    
                    for (auto aTarget : *targetList)
                    {
                        if (!HeroActionController::isDeadAction(aTarget->currentAction))
                        {
                            if (shouldTurn && targetList->size() == 1)
                            {
                                origin->forceTurnToTarget(aTarget);
                            }
                            //28.7 cui
                            //new ProjectileForLanding(aiContext, projectileID, origin, aTarget, aTarget->getX(), aTarget->getY(), aTarget->getZ(), skillID, skillLevel);
                            new ProjectileForLanding(aiContext, projectileID, origin, aTarget, this->getRealEndPos(staticData, aTarget->getX(),false), 
                                this->getRealEndPos(staticData, aTarget->getY(), true), aTarget->getZ(), skillID, skillLevel);
                        }
                    }
                }
            }
        }
        else if (EProjectileTargetType::eProjectileTargetRandomFriend == staticData->throwType ||
                 EProjectileTargetType::eProjectileTargetRandomEnemy == staticData->throwType)
        {
            if (shouldThrow)
            {
                if (specialTarget)
                {
                    //28.7 cui
                   // new ProjectileForLanding(aiContext, projectileID, origin, specialTarget, origin->getX(), origin->getY(), origin->getZ(), skillID, skillLevel); 
                    new ProjectileForLanding(aiContext, projectileID, origin, specialTarget,this->getRealEndPos(staticData, origin->getX(),false), 
                        this->getRealEndPos(staticData, origin->getY(), true), origin->getZ(), skillID, skillLevel);
                }
                else
                {
                    std::unique_ptr<CharacterDataList> targetList = nullptr;
                    if (EProjectileTargetType::eProjectileTargetRandomFriend == staticData->throwType)
                        targetList = AIActionController::getFilteredFriendList(*origin, *origin, false, false, false, nullptr, TargetFriendRandom);
                    else if (EProjectileTargetType::eProjectileTargetRandomEnemy == staticData->throwType)
                        targetList = AIActionController::getFilteredRivalList(*origin, *origin, false, false, false, nullptr, TargetEnemyRandom);
                    
                    for (int i = 0; i < staticData->throwNum; i++)
                    {
                        auto aCharacter = AIActionController::getRandomCharacter(aiContext, *origin, targetList.get(), false);
                        if (aCharacter)
                        {
                            if (shouldTurn && staticData->throwNum == 1)
                            {
                                origin->forceTurnToTarget(aCharacter);
                            }
                            //28.7 cui
                            //new ProjectileForLanding(aiContext, projectileID, origin, aCharacter, aCharacter->getX(), aCharacter->getY(), aCharacter->getZ(), skillID, skillLevel);
                            new ProjectileForLanding(aiContext, projectileID, origin, aCharacter,this->getRealEndPos(staticData, aCharacter->getX(),false),
                                this->getRealEndPos(staticData, aCharacter->getY(), true), aCharacter->getZ(), skillID, skillLevel);
                        }
                    }
                }
            }
        }
        else if (EProjectileTargetType::eProjectileTarget_6 == staticData->throwType ||
                 /*EProjectileTargetType::eProjectileTarget_7 == staticData->throwType ||*/
                 EProjectileTargetType::eProjectileTarget_8 == staticData->throwType /*||
                 EProjectileTargetType::eProjectileTarget_9 == staticData->throwType*/)
        {
            ptarget = origin->target;
            if (specialTarget)
                ptarget = specialTarget;
            if (ptarget)
            {
                if (shouldTurn)
                {
                    origin->forceTurnToTarget(ptarget);
                }
                
                if (shouldThrow)
                {
                    new ProjectileForBouncing(aiContext, projectileID, origin, ptarget, skillID, skillLevel);
                }
            }
        }
		else if (EProjectileTargetType::eProjectileTarget_7 == staticData->throwType ||
			EProjectileTargetType::eProjectileTarget_9 == staticData->throwType)
		{
			if (AITargetHelper::isUseNewHelper())
				throwBySearchNew(aiContext, shouldTurn, shouldThrow);
			else
				throwBySearchOld(aiContext, shouldTurn, shouldThrow);
		}
        else if (EProjectileTargetType::eProjectileTargetFollowCaster == staticData->throwType)
        {
            ptarget = origin->target;
            if (specialTarget)
                ptarget = specialTarget;
            
            // MARK: no turn
            
            if (shouldThrow && ptarget)
            {
                new ProjectileForCharacter(aiContext, projectileID, origin, ptarget, skillID, skillLevel);
            }
        }
        else if (EProjectileTargetType::eProjectileTargetAllAttrHighest == staticData->throwType ||
                 EProjectileTargetType::eProjectileTargetFriendAttrHighest == staticData->throwType ||
                 EProjectileTargetType::eProjectileTargetEnemyAttrHighest == staticData->throwType)
        {
            std::unique_ptr<CharacterDataList> friends = nullptr;
            std::unique_ptr<CharacterDataList> rivals = nullptr;
            
            if (EProjectileTargetType::eProjectileTargetAllAttrHighest == staticData->throwType)
                friends = AIActionController::getFilteredFriendList(*origin, *origin, false, false, false, nullptr, TargetAllAttrHighest);
            else if (EProjectileTargetType::eProjectileTargetFriendAttrHighest == staticData->throwType)
                friends = AIActionController::getFilteredFriendList(*origin, *origin, false, false, false, nullptr, TargetFriendAttrHighest);
            
            if (EProjectileTargetType::eProjectileTargetAllAttrHighest == staticData->throwType)
                rivals = AIActionController::getFilteredRivalList(*origin, *origin, false, false, false, nullptr, TargetAllAttrHighest);
            else if (EProjectileTargetType::eProjectileTargetEnemyAttrHighest == staticData->throwType)
                rivals = AIActionController::getFilteredRivalList(*origin, *origin, false, false, false, nullptr, TargetEnemyAttrHighest);
            
            if (specialTarget)
                ptarget = specialTarget;
            else
                ptarget = AIActionController::getAttrEstCharacter(*origin, friends.get(), rivals.get(), staticData->throwParams, AIActionController::COMP_HIGHEST, false);
            
            if (shouldTurn && ptarget)
            {
                origin->forceTurnToTarget(ptarget);
            }
            
            if (shouldThrow && ptarget && !HeroActionController::isDeadAction(ptarget->currentAction))
            {
                new ProjectileForLanding(aiContext, projectileID, origin, ptarget, ptarget->getX(), ptarget->getY(), ptarget->getZ(), skillID, skillLevel);
            }
        }
        else if (EProjectileTargetType::eProjectileTargetAllAttrLowest == staticData->throwType ||
                 EProjectileTargetType::eProjectileTargetFriendAttrLowest == staticData->throwType ||
                 EProjectileTargetType::eProjectileTargetEnemyAttrLowest == staticData->throwType)
        {
            std::unique_ptr<CharacterDataList> friends = nullptr;
            std::unique_ptr<CharacterDataList> rivals = nullptr;
            
            if (EProjectileTargetType::eProjectileTargetAllAttrLowest == staticData->throwType)
                friends = AIActionController::getFilteredFriendList(*origin, *origin, false, false, false, nullptr, TargetAllAttrLowest);
            else if (EProjectileTargetType::eProjectileTargetFriendAttrLowest == staticData->throwType)
                friends = AIActionController::getFilteredFriendList(*origin, *origin, false, false, false, nullptr, TargetFriendAttrLowest);
            
            if (EProjectileTargetType::eProjectileTargetAllAttrLowest == staticData->throwType)
                rivals = AIActionController::getFilteredRivalList(*origin, *origin, false, false, false, nullptr, TargetAllAttrLowest);
            else if (EProjectileTargetType::eProjectileTargetEnemyAttrLowest == staticData->throwType)
                rivals = AIActionController::getFilteredRivalList(*origin, *origin, false, false, false, nullptr, TargetEnemyAttrLowest);
            
            if (specialTarget)
                ptarget = specialTarget;
            else
                ptarget = AIActionController::getAttrEstCharacter(*origin, friends.get(), rivals.get(), staticData->throwParams, AIActionController::COMP_LOWEST, false);
            
            if (shouldTurn && ptarget)
            {
                origin->forceTurnToTarget(ptarget);
            }
            
            if (shouldThrow && ptarget && !HeroActionController::isDeadAction(ptarget->currentAction))
            {
                new ProjectileForLanding(aiContext, projectileID, origin, ptarget, ptarget->getX(), ptarget->getY(), ptarget->getZ(), skillID, skillLevel);
            }
        }
        else if (EProjectileTargetType::eProjectileTargetOnSearch == staticData->throwType)
        {
            if (AITargetHelper::isUseNewHelper())
                throwBySearchNew(aiContext, shouldTurn, shouldThrow);
            else
                throwBySearchOld(aiContext, shouldTurn, shouldThrow);
        }
		else if (EProjectileTargetType::eProjectileTargetPosition == staticData->throwType)
		{
			if (shouldThrow)
			{
				int postype = staticData->throwParams[0];
				int posx = staticData->throwParams[1];

                //新增加y方向偏移 cui_2023_0208 28.7
                int posy = origin->getY();
                if (staticData->throwParams[2] != NULL) {
                    posy += staticData->throwParams[2];
                }
				if (postype == 1)
					posx = aiContext.getBattleFieldCenterX() + posx;
				else if (postype == 2)
					posx = aiContext.getBattleFieldCenterX() + posx * origin->direction;
				else if (postype == 3)
					posx = aiContext.getBattleFieldCenterX() + posx * (origin->teamSide == TeamSide::kOur ? 1 : -1);
				else if (postype == 4)
					posx = origin->getX() + origin->direction * posx;

                

                new ProjectileForLanding(aiContext, projectileID, origin, nullptr, posx, posy, origin->getZ(), skillID, skillLevel);
				//new ProjectileForLanding(aiContext, projectileID, origin, nullptr, posx, origin->getY(), origin->getZ(), skillID, skillLevel);
			}
		}
    }
    
    void AIContinuousProjectileCaster::throwBySearchOld(AIContext &aiContext, bool shouldTurn, bool shouldThrow)
    {
        std::shared_ptr<std::list<CharacterData*>> targets = AIActionController::searchTarget(*origin, origin->target, nullptr, aiContext, staticData->searchTarget, staticData->searchX1, staticData->searchX2, staticData->searchZ1, staticData->searchZ2, staticData->searchZ1, staticData->searchZ2, &staticData->searchParams, *origin, false, skillID);
        if (staticData->filterTarget >= 0)
        {
			auto hurtID = staticData->hitExecVec[0]; // hitExec;  99 类型约定只取列表中第一个ID，作参数  来查找目标 
            auto realTargetType = HeroSkillController::getRealSearchInfo(staticData->filterTarget).realSearchType;
			if ((realTargetType == TargetEnemyMaxHurt || realTargetType == TargetTraversal) && IsHurtExec(hurtID))
            {
                targets = AIActionController::searchTarget(*origin, origin->target, nullptr, aiContext, staticData->filterTarget, staticData->filterX1, staticData->filterX2, staticData->filterZ1, staticData->filterZ2, staticData->filterZ1, staticData->filterZ2, &staticData->filterParams, *this->origin, false, skillID, hurtID, skillLevel, targets.get());
            }
            else
            {
                auto secondTargets = AIActionController::searchTarget(*origin, origin->target, nullptr, aiContext, staticData->filterTarget, staticData->filterX1, staticData->filterX2, staticData->filterZ1, staticData->filterZ2, staticData->filterZ1, staticData->filterZ2, &staticData->filterParams, *this->origin, false, skillID, hurtID, skillLevel);
                auto pFinalTargets = std::make_shared<std::list<CharacterData*> >();
                for (auto iter = targets->begin(); iter != targets->end(); ++iter)
                {
                    for (auto it = secondTargets->begin(); it != secondTargets->end(); ++it)
                    {
                        if ((*it)->myID == (*iter)->myID)
                            pFinalTargets->push_back((*it));
                    }
                }
                
                targets = pFinalTargets;
            }
        }
        
        if (staticData->targetExclude >= 0)
        {
            auto hurtID = staticData->hitExecVec[0]; // hitExec;  99 类型约定只取列表中第一个ID，作参数  来查找目标
            auto excludeTargets = AIActionController::searchTarget(*origin, origin->target, nullptr, aiContext, staticData->targetExclude, staticData->excludeX1, staticData->excludeX2, staticData->excludeZ1, staticData->excludeZ2, staticData->excludeZ1, staticData->excludeZ2, &staticData->targetExcludeParams, *this->origin, false, skillID, hurtID, skillLevel);
            if (excludeTargets)
            {
                for (auto iter = excludeTargets->begin(); iter != excludeTargets->end(); ++iter)
                    targets->remove(*iter);
            }
        }
        
        if (targets->size() == 1 && shouldTurn)
        {
            origin->forceTurnToTarget(targets->front());
        }
        
        if (shouldThrow)
		{
			if (staticData->throwType == EProjectileTargetType::eProjectileTarget_7 || staticData->throwType == EProjectileTargetType::eProjectileTarget_9)
			{
				for (auto aTarget : *targets)
				{
					if (!HeroActionController::isDeadAction(aTarget->currentAction))
						new ProjectileForBouncing(aiContext, projectileID, origin, aTarget, skillID, skillLevel);
				}
			}
			else
			{
				for (auto aTarget : *targets)
				{
					if (!HeroActionController::isDeadAction(aTarget->currentAction))
						new ProjectileForLanding(aiContext, projectileID, origin, aTarget, aTarget->getX(), aTarget->getY(), aTarget->getZ(), skillID, skillLevel);
				}
			}
        }
    }
    //28.7 cui 2023
    int AIContinuousProjectileCaster::getRealEndPos(ProjectileExec* def, int initPos, bool isY) {
        if (def->orbitType == eProjectileOrbitLinear || def->orbitType == eProjectileOrbitLowParabola || def->orbitType == eProjectileOrbitMediumParabola || def->orbitType == eProjectileOrbitHighParabola) {
            return isY ? (initPos + def->offsetY) : (initPos + def->offsetX);
        }
        return initPos;
    }
    
    void AIContinuousProjectileCaster::throwBySearchNew(AIContext &aiContext, bool shouldTurn, bool shouldThrow)
    {
        AITargetContext ctx;
        initProjectileTargetContext(ctx, this);
        std::list<CharacterData *> targets;
        AITargetHelper::searchTarget(&ctx, &targets);
        
        if (staticData->filterTarget >= 0)
        {
			auto hurtID = staticData->hitExecVec[0];// hitExec;  99 类型约定只取列表中第一个ID，作参数  来查找目标 
            auto realTargetType = HeroSkillController::getRealSearchInfo(staticData->filterTarget).realSearchType;
			if ((realTargetType == TargetEnemyMaxHurt || realTargetType == TargetTraversal) && IsHurtExec(hurtID))
            {
                std::list<CharacterData *> filterlist = targets;
                targets.clear();
                initProjectileFilterTargetContext(ctx, this, &filterlist);
                AITargetHelper::searchTarget(&ctx, &targets);
            }
            else
            {
                std::list<CharacterData *> secondTargets;
                initProjectileFilterTargetContext(ctx, this, nullptr);
                AITargetHelper::searchTarget(&ctx, &secondTargets);
                //auto pFinalTargets = std::make_shared<std::list<CharacterData*> >();
                //for (auto iter = targets->begin(); iter != targets->end(); ++iter)
                std::list<CharacterData *> finalTargets;
                for (auto c1 : targets)
                {
                    //for (auto it = secondTargets->begin(); it != secondTargets->end(); ++it)
                    for (auto c2 : secondTargets)
                    {
                        //if ((*it)->myID == (*iter)->myID)
                        //    pFinalTargets->push_back((*it));
                        if (c1->myID == c2->myID)
                            finalTargets.push_back(c1);
                    }
                }
                
                //targets = pFinalTargets;
                targets = finalTargets;
            }
        }
        
        if (staticData->targetExclude >= 0)
        {
            std::list<CharacterData *> excludeTargets;
            initProjectileExcludeTargetContext(ctx, this);
            AITargetHelper::searchTarget(&ctx, &excludeTargets);
            for (auto c : excludeTargets) {
                targets.remove(c);
            }
        }
        
        if (targets.size() == 1 && shouldTurn)
        {
            origin->forceTurnToTarget(targets.front());
        }
        

        if (shouldThrow)
        {
			if (staticData->throwType == EProjectileTargetType::eProjectileTarget_7 || staticData->throwType == EProjectileTargetType::eProjectileTarget_9)
			{
				for (auto aTarget : targets)
				{
					if (!HeroActionController::isDeadAction(aTarget->currentAction))
						new ProjectileForBouncing(aiContext, projectileID, origin, aTarget, skillID, skillLevel);
				}
			}
			else
			{
				for (auto aTarget : targets)
				{
					if (!HeroActionController::isDeadAction(aTarget->currentAction))
						new ProjectileForLanding(aiContext, projectileID, origin, aTarget, aTarget->getX(), aTarget->getY(), aTarget->getZ(), skillID, skillLevel);
				}
			}
        }
    }
    
}
