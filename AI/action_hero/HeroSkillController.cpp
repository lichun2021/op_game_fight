//
//  HeroSkillAction.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "HeroSkillController.h"
#include "HeroActionController.h"
#include "../AITargetHelper.h"
#include "../HeroHurtcalculate.h"
#include "vector"
#include "../CommonType.h"
#include "HeroAttackAction.h"
#include "../BattleLog.h"
#include "../scriptbind/AIScriptBindEffectCaster.hpp"

namespace OnePiece {

    SkillSeqInfo::SkillSeqInfo(int idIndex, int seqIndex, int theSpecialSkillID, int theSpecialSkillCastCondition) :
        skillIDIndex(idIndex), 
        attackSeqIndex(seqIndex),
        specialSkillID(theSpecialSkillID),
        specialSkillCastContiond(theSpecialSkillCastCondition)
    {

    }
    
    void SkillSeqInfo::setInfo(int idIndex, int seqIndex, int theSpecialSkillID, int theSpecialSkillCastCondition)
    {
        skillIDIndex = idIndex;
        attackSeqIndex = seqIndex;
        specialSkillID = theSpecialSkillID;
        specialSkillCastContiond = theSpecialSkillCastCondition;
    }
    
    static int pickNormalSkill(CharacterData *c, std::vector<int> *attackSeq, int currentSeqIndex)
    {
        int nextSeqIndex = currentSeqIndex + 1;
        if (currentSeqIndex < 0 || nextSeqIndex >= attackSeq->size())
            nextSeqIndex = 0;
        
        // 231
        return AISkipSkillSeqEffect::checkExcute(c, *attackSeq, nextSeqIndex);
    }
    
    static int pickSkillWithEff294(AIContext &aiContext, CharacterData *c, std::vector<int> *attackSeq, int currentSeqIndex)
    {
        int nextSeqIndex = -1;
        int pickcnt = 0;
        while (true) {
            nextSeqIndex = pickNormalSkill(c, attackSeq, currentSeqIndex);
            if (nextSeqIndex >= 0 && nextSeqIndex < attackSeq->size()) {
                int skillidindex = (*attackSeq)[nextSeqIndex];
                if (luahelper_innereffect_294_is_skillidblocked(aiContext.getLuaState(), c, CharacterFunc::GetSkillIDByIndex(*c, skillidindex))) {
                    currentSeqIndex = nextSeqIndex;
                }
                else {
                    return nextSeqIndex;
                }
            }
            else {
                currentSeqIndex = nextSeqIndex;
            }
            
            pickcnt += 1;
            if (pickcnt >= attackSeq->size())
                return -1;
        }
        return nextSeqIndex;
    }

    void HeroSkillController::pickSkill(CharacterData& aiCharacter, AIContext& aiContext, SkillSeqInfo *outresult)
    {
        if (!outresult)
            return;
        
//        if ((CharacterFunc::IsEnemyHero(aiCharacter) || aiContext.autoFight>0) && aiCharacter.canCastUltraSkill()>0)
//        {
//            outresult->setInfo(1, -1, -1, -1);
//            return;
//        }

        if (aiCharacter.skillList.empty())
        {
            outresult->setInfo(-1, -1, -1, -1);
            return;
        }
        
        bool isHasEff294 = aiCharacter.hasEffect(294);

        if (aiContext.monsterCanMove) {
            // 16,17,13,3,11,9,12
            std::vector<ESkillCastCondition> specialConditions = {
                ESkillCastCondition::eSkillCastConditionOnHurtPlus,
                ESkillCastCondition::eSkillCastConditionAnyEnemyHP,
                ESkillCastCondition::eSkillCastConditionOnSelfHPBreakAction,
                ESkillCastCondition::eSkillCastConditionOnSelfHP,
                ESkillCastCondition::eSkillCastConditionOnAvoidNoBreakAction,
                ESkillCastCondition::eSkillCastConditionOnMultipleHurt,
                ESkillCastCondition::eSkillCastConditionAfterCruelHit,
            };
            for (const auto& condition : specialConditions)
            {
                auto pSpecialSkill = aiCharacter.getSpecialSkill(condition);
                if (pSpecialSkill && pSpecialSkill->canCast(&aiCharacter) && pSpecialSkill->skillID > 0)
                {
                    // 294
                    if (!isHasEff294 || !luahelper_innereffect_294_is_skillidblocked(aiContext.getLuaState(), &aiCharacter, pSpecialSkill->skillID))
                    {
                        outresult->setInfo(pSpecialSkill->skillIDIndex, -1, pSpecialSkill->skillID, condition);
                        return;
                    }
                }
            }
        }

        // 126
        bool useSeq2 = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_OTHER_SKLIST, 0);

        // 183
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, eEFFECT_PICK_RANGE_SKILL, 0) > 0) {
            int rangeSkillIndex = AIPickRangeSkillEffect::getRangeSkillIndex(&aiCharacter);
            if (rangeSkillIndex < 0) {
                outresult->setInfo(-1, -1, -1, -1);
                return;
            }
            
            if (!isHasEff294 || !luahelper_innereffect_294_is_skillidblocked(aiContext.getLuaState(), &aiCharacter, CharacterFunc::GetSkillIDByIndex(aiCharacter, rangeSkillIndex)))
            {
                int seqIndex = getAttackSeqFirstMatchIndex(&aiCharacter, rangeSkillIndex, useSeq2);
                if (useSeq2)
                    aiCharacter.m_iSkillSeq2Index = seqIndex;
                else
                    HeroActionController::setSkillSeq(aiCharacter.actionAtt, seqIndex);
                outresult->setInfo(rangeSkillIndex, seqIndex, -1, -1);
            }
            else
            {
                outresult->setInfo(-1, -1, -1, -1);
            }

            return;
        }

        // 214
        auto pAttackSeq = AIAttackListAddSomeOneEffect::getAlternateAttackSeq(&aiCharacter, useSeq2);
        if (!pAttackSeq)
            pAttackSeq = useSeq2 ? &aiCharacter.staticData->attackSeq2 : &aiCharacter.staticData->attackSeq;
        int currentSeqIndex = useSeq2 ? aiCharacter.m_iSkillSeq2Index : HeroActionController::getSkillSeq(aiCharacter.actionAtt);
        int nextSeqIndex = 0;
        if (isHasEff294)
            nextSeqIndex = pickSkillWithEff294(aiContext, &aiCharacter, pAttackSeq, currentSeqIndex);
        else
            nextSeqIndex = pickNormalSkill(&aiCharacter, pAttackSeq, currentSeqIndex);

        if (nextSeqIndex < 0)
            outresult->setInfo(-1, -1, -1, -1);
        else
            outresult->setInfo((*pAttackSeq)[nextSeqIndex], nextSeqIndex, -1, -1);
    }

    CaptainSeqSkillInfo::CaptainSeqSkillInfo(int theseqidx, int theseqtype, int theseqvalue, int theskillid, int theskilllevel)
        : seqIdx(theseqidx), seqType(theseqtype), seqValue(theseqvalue), skillID(theskillid), skillLevel(theskilllevel)
    {
    }
	
	int HeroSkillController::calculateSkillID(AIContext& aiContext,CharacterData& aiCharacter, int skillSeq)
	{
        auto calcFunc = [&skillSeq, &aiCharacter](std::vector<int> &attackSeq)->int{
            if (skillSeq < 0 || skillSeq >= attackSeq.size())
                skillSeq = 0;
            int skillIndex = attackSeq[skillSeq];
            int skillID = CharacterFunc::GetSkillIDByIndex(aiCharacter, skillIndex);
            for (auto iter = aiCharacter.skillList.begin(); iter != aiCharacter.skillList.end(); ++iter) {
                if (iter->first == skillID)
                    return skillID;
            }
            return aiCharacter.staticData->sk[0];
        };

        int bInSpecialEffect = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_OTHER_SKLIST, 0);
        if (bInSpecialEffect == 1)
            return calcFunc(aiCharacter.staticData->attackSeq2);
        else
            return calcFunc(aiCharacter.staticData->attackSeq);

        return aiCharacter.staticData->sk[0];
	}

	int HeroSkillController::calculateSkillID(AIContext& aiContext,CharacterData& aiCharacter)
	{
        
        int bInSpecialEffect = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_OTHER_SKLIST, 0);
        
        int skillSeq = 0;
        if(bInSpecialEffect == 1)
        {
            skillSeq = aiCharacter.m_iSkillSeq2Index;
        }else
        {
            skillSeq = HeroActionController::getSkillSeq(aiCharacter.actionAtt);
        }
		
		return calculateSkillID(aiContext,aiCharacter, skillSeq);
	}

	bool HeroSkillController::isTargetInSkilllRange(CharacterData* owner, int skillId, CharacterData* target)
	{
        //return (HeroActionController::distanceSquare(owner, target) <= calculateSkillRangeSquare(skillId));
        AISkill* skill = AIDataHelper::getSkillData(skillId);
        if (!skill)
            return false;
        return (HeroActionController::distanceToCharacter(owner, target) <= skill->range);
	}

	std::pair<int, int> HeroSkillController::getUltraSkill(CharacterData& aiCharacter)
	{
		if (aiCharacter.skillList.empty())
		{
			//if skill not set   should not be executed
			return std::pair<int, int>(-1, 0);
		}
		std::vector<std::pair<int, int>>::iterator iter = aiCharacter.skillList.begin();
		advance(iter,1);
		return *iter;
	}

    void HeroSkillController::executeExecID(int eid, AISkill* skillData, AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& source, CharacterData& target)
    {
        if (eid < 1)
            return;

        int realTarget = 0;
		if (skillData != nullptr)
		{
            if (AITargetHelper::isUseNewHelper())
                realTarget = AITargetHelper::parseTargetType(skillData->targetType, nullptr, nullptr, nullptr, nullptr);
            else
                realTarget = HeroSkillController::getRealSearchInfo(skillData->targetType).realSearchType;
            if (realTarget == TargetAbsoluteRangeEnemy || realTarget == TargetAbsoluteRangeFriend || realTarget == TargetAbsoluteRangeAll || realTarget == TargetAbsoluteRangeAllWithoutSearcher)
			{
				source.mStationaryDirection = source.direction;
				source.mStationaryX = source.getX();
				source.mStationaryY = source.getY();
			}
			else
			{
				source.mStationaryDirection = 0;
				source.mStationaryX = 0;
				source.mStationaryY = 0;
			}
		}
        
        if (AITargetHelper::isUseNewHelper())
        {
            AITargetContext ctx;
            ctx.source = &source;
            ctx.target = &target;
            ctx.skill_caster = &source;
            ctx.projectile = nullptr;
            ctx.search_type = skillData->targetType;
            ctx.range.x1 = -1 * skillData->range;
            ctx.range.x2 = skillData->range;
            ctx.range.y1 = -1 * skillData->range;
            ctx.range.y2 = skillData->range;
            ctx.search_params = &skillData->vTargetParms;
            ctx.skill_id = skillId;
            ctx.is_using_skill = true;
            
            std::list<CharacterData *> targets;
            AITargetHelper::searchTarget(&ctx, &targets);
            if (targets.empty())
            {
                CharacterData *anotherTarget = source.lockTarget(aiContext);
                if (anotherTarget)
                {
                    HeroActionController::setCurrentTarget(source, anotherTarget);
                    AITargetHelper::searchTarget(&ctx, &targets);
                }
            }
            
            for (auto c : targets)
            {
                AIActionController::executeExec(aiContext, eid, skillId, skillLv, &source, nullptr, c, 0);
            }
            
            if (realTarget == TargetOneEnemyRandomContinueHurtNotRandomTarget && source.GetFlagFirstFound26Target() && !targets.empty())
            {
                source.SetFirstRoundConfirm26Target(targets.front());
            }
        }
        else
        {
            auto currentTargets = AIActionController::searchTarget(source, &target, nullptr, aiContext, skillData->targetType, -1 * skillData->range, skillData->range, -1 * skillData->range, skillData->range, -1 * skillData->range, skillData->range, &skillData->vTargetParms, source, true, skillId);
            if (currentTargets->empty())
            {
                // clair_wang 解决某一角色要放技能（eg:15801，道具ID:41580)，currentTargets却找不到目标（目标被其它人弄死了），重新锁定一目标
                CharacterData* pTarget = source.lockTarget(aiContext);
                if (pTarget != nullptr)
                {
                    HeroActionController::setCurrentTarget(source, pTarget);
                    currentTargets = AIActionController::searchTarget(source, &target, nullptr, aiContext, skillData->targetType, -1 * skillData->range, skillData->range, -1 * skillData->range, skillData->range, -1 * skillData->range, skillData->range, &skillData->vTargetParms, source, true, skillId);
                }
            }
            
            if (currentTargets->size() > 0)
            {
                for (auto iter = currentTargets->begin(); iter != currentTargets->end(); ++iter)
                {
                    AIActionController::executeExec(aiContext, eid, skillId, skillLv, &source, nullptr, *iter, 0);
                }
                
                int skillSearchType = HeroSkillController::getRealSearchInfo(skillData->targetType).realSearchType;
                if (skillSearchType == TargetOneEnemyRandomContinueHurtNotRandomTarget && source.GetFlagFirstFound26Target())
                {
                    source.SetFirstRoundConfirm26Target(currentTargets->front());
                }
            }
        }

        if (source.hasTag("energyskill"))
            return;

        // 使用相应的伤害执行/飞行道具的击中伤害执行对应的能量增加
		// 飞行道具的击中伤害执行数组对应的能量增加总和
        int energyID = eid;
		int useSkillAdd = 0;
        if (IsProjectileExec(eid))
		{
            auto projectileData = AIDataHelper::getProjectileExec(eid);
			if (projectileData)
			{
				//energyID = projectileData->hitExec;
				for (int idx = 0; idx < projectileData->hitExecVec.size(); idx++)
				{
					int hurtID = projectileData->hitExecVec[idx];
					if (IsHurtExec(hurtID))
					{
						auto hurtData = AIDataHelper::getHurtExec(hurtID);
						if (hurtData)
						{
							useSkillAdd += hurtData->sourceEnergyAdd;
						}
					}
				}
			}
        }
		else if (IsHurtExec(energyID))
		{
			auto hurtData = AIDataHelper::getHurtExec(energyID);
			if (hurtData)
			{
				useSkillAdd = hurtData->sourceEnergyAdd;
			}
		}
        
        #if BATTLE_LOG_TEST
            OPBattleLog("gainenergy", source.teamSide, source.clusterNum, source.attributes[0], useSkillAdd);
        #endif
                
            if (useSkillAdd)
            {
                //useSkillAdd += AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, source, eEFFECT_ATTACK_GAIN_ENERGY_ADD, useSkillAdd);
                useSkillAdd += EffectHelper::checkExcuteMixChangeTypeEff(&source, eEFFECT_ATTACK_GAIN_ENERGY_ADD, useSkillAdd);
            }

            CharacterFunc::ApplyEnergyDelta(&source, useSkillAdd);

            if (useSkillAdd > 0)
            {
                if (skillId == source.normalSkill.first && AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, source, eEFFECT_FORCE_ATTACK_SELF, 0) > 0)
                {
                    AIContinuousEffectCaster::change_Special_Effect(aiContext, source, eEFFECT_FORCE_ATTACK_SELF, 1);
                }
            }
    }

	// clair_wang 此函数用于目标类型：26  查找所用
    std::unique_ptr<std::list<CharacterData*> > HeroSkillController::GetSkillTargetListsByType(AISkill* skillData, AIContext& aiContext, int skillId, CharacterData& source, CharacterData& target)
	{

        std::unique_ptr<std::list<CharacterData*> > currentTargets = nullptr;
        
        if (skillData && HeroSkillController::getRealSearchInfo(skillData->targetType).realSearchType == TargetOneEnemyRandomContinueHurtNotRandomTarget)
		{
			if (source.GetFlagFirstFound26Target())
			{
				if (source.GetFirstRoundConfirm26Target() != nullptr)
				{
                    currentTargets = std::unique_ptr<std::list<CharacterData*> >(new std::list<CharacterData*>);
					currentTargets->push_back(source.GetFirstRoundConfirm26Target());
				}
				else
				{
                    currentTargets = AIActionController::searchTarget(source, &target, nullptr, aiContext, skillData->targetType, -1 * skillData->range, skillData->range, -1 * skillData->range, skillData->range, -1 * skillData->range, skillData->range, &skillData->vTargetParms, source, true, skillId);
				}
			}
		}
		else
		{
            currentTargets = AIActionController::searchTarget(source, &target, nullptr, aiContext, skillData->targetType, -1 * skillData->range, skillData->range, -1 * skillData->range, skillData->range, -1 * skillData->range, skillData->range, &skillData->vTargetParms, source, true, skillId);
		}

		return currentTargets;
	}

    void HeroSkillController::forceReleaseAccumulateSkill(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& source, CharacterData& target)
    {
        AISkill* skillData = AIDataHelper::getSkillData(skillId);
        if (!skillData)
            return;
        if (!IsAccumulateSkill(skillData->castCondition))
            return;
        if (skillData->skillExec.empty())
            return;
        if (!source.isInAccumulateState())
            return;

        int theAccuTick = tick;
        int theReadyTick = getAccuSkillReadyTick(skillId);
        if (theReadyTick > 0)
            theAccuTick -= theReadyTick;
        int theCurrentAltTick = source.getCurrentAlternateAccuMaxTick(skillId);
        if (theCurrentAltTick >= 0)
        {
            int theOriginAccuMaxTick = getAccuSkillMaxAccuTick(skillId);
            if (theCurrentAltTick == 0)
            {
                source.setAltAccuTick(theOriginAccuMaxTick);
            }
            else
            {
                source.setAltAccuTick(theAccuTick * theOriginAccuMaxTick / theCurrentAltTick);
            }
        }
        else
        {
            source.setAltAccuTick(0);
        }
        if (theAccuTick < 0)
            theAccuTick = 0;
        HeroActionController::setAccumulateTick(source.actionAtt, theAccuTick);

        for (auto iter = skillData->skillExec.begin(); iter != skillData->skillExec.end(); ++iter)
        {
            // 蓄力大招参数0: 执行延迟tick
            // 参数1: 0 - 蓄力时执行, 1 - 蓄力结束时执行(忽略参数0)
            if (iter->param1 != 1)
                continue;

            executeExecID(iter->execId, skillData, aiContext, skillId, skillLv, tick, source, target);
        }

        source.endAccumulateState();
        source.exitAccumulateState();
    }

    void HeroSkillController::checkReleaseHPSkill(AIContext& aiContext, CharacterData* target)
    {
        auto pHPSkill = target->getSpecialSkill(ESkillCastCondition::eSkillCastConditionOnSelfHPBreakAction);
        if (pHPSkill && pHPSkill->canCast(target)) {
            if (!HeroSkillController::isTargetIgnoreSilenceBySkill(aiContext, target, pHPSkill->skillID))
            {
                if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_NO_SKILL_MARK, 0) > 0)
                    return;
            }

            if (target->hasTag("ultraskill"))
                return;
            if (target->canCastUltraSkill())
                return;

            pHPSkill->onConditon();
            bool isUltra = (pHPSkill->skillID == target->staticData->sk[1]);
            HeroAttackAction::ForceEnter(*target, aiContext, -1, pHPSkill->skillID, pHPSkill->skillLevel, true, 0, isUltra);
        }
    }

    void HeroSkillController::executeExecOnExitEnergySkill(AIContext& aiContext, int skillId, int skillLv, CharacterData& caster, CharacterData& target)
    {
        auto theSkill = AIDataHelper::getSkillData(skillId);
        if (!theSkill)
            return;
        
        if (theSkill->castParams.size() > 4)
        {
            for (int i = 5; i < theSkill->castParams.size() && i < 10; i++)
            {
                auto anExecId = theSkill->castParams[i];
                if (anExecId > 0)
                {
                    AIActionController::executeExec(aiContext, anExecId, skillId, skillLv, &caster, nullptr, &target, 0, false, 0);
                }
            }
        }
    }

    void HeroSkillController::endAccumulateSkill(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& source, CharacterData& target)
    {
        AISkill* skillData = AIDataHelper::getSkillData(skillId);
        if (!skillData)
            return;
        if (!IsAccumulateSkill(skillData->castCondition))
            return;
        if (skillData->skillExec.empty())
            return;
        if (!source.isInAccumulateState())
            return;

        if (eSkillCastConditionAccuRepeatAttack != skillData->castCondition)
        {
            int theAccuTick = tick;
            int theReadyTick = getAccuSkillReadyTick(skillId);
            if (theReadyTick > 0)
                theAccuTick -= theReadyTick;
            int theCurrentAltTick = source.getCurrentAlternateAccuMaxTick(skillId);
            if (theCurrentAltTick >= 0)
            {
                int theOriginAccuMaxTick = getAccuSkillMaxAccuTick(skillId);
                if (theCurrentAltTick == 0)
                {
                    source.setAltAccuTick(theOriginAccuMaxTick);
                }
                else
                {
                    source.setAltAccuTick(theAccuTick * theOriginAccuMaxTick / theCurrentAltTick);
                }
            }
            else
            {
                source.setAltAccuTick(0);
            }
                
            if (theAccuTick < 0)
                theAccuTick = 0;
            HeroActionController::setAccumulateTick(source.actionAtt, theAccuTick);
        }
        else
        {
            auto repeatInterval = skillData->castParams[1];
            int repeatCnt = tick / repeatInterval;
            if (repeatCnt > 0)
            {
                HeroActionController::setAccuAttackRepeat(source.actionAtt, repeatCnt);
                source.startTick = source.localTick + 1;
                source.totalTick = calculateAccumulateSkillReleaseLength(skillId);
                source.addTag("repeatattacking");
            }
            else
            {
                source.totalTick = 0;
            }
        }
        source.endAccumulateState();
    }

	void HeroSkillController::executeExecIDInTick(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& source, CharacterData& target)
	{
		AISkill* skillData = AIDataHelper::getSkillData(skillId);
        if (!skillData)
            return;
		if (skillData->skillExec.empty())
            return;
        
        /*
        int executeState = 0; // 0 - 普通执行, 1 - 仅蓄力中执行, 2 - 蓄力结束执行
        if (IsAccumulateSkill(skillData->castCondition))
        {
            executeState = 1;
            if (!skillData->castParams.empty())
            {
                // MARK: to be optimised
                // @author: h7ing
                // 因本接口主要在AttackAction中调用，结束后会切换状态，为避免在一次更新中无意的多次切状态引起一些问题，
                // 在这里单独处理蓄力释放的逻辑，而没有统一调CharacterData的蓄力释放接口
                if (source.getAccumulateState() == EAccumulateState::kAfterAccumulate)
                    executeState = 3;
                else 
                {
                    int theParamTick = skillData->castParams[0];
                    if (source.attackSpeed > 0 && source.attackSpeed != 100)
                        theParamTick = theParamTick * source.attackSpeed / 100;
                    if (tick == theParamTick)
                        executeState = 2;
                } 
            }
        }

        if (2 == executeState)
        {
            endAccumulateSkill(aiContext, skillId, skillLv, tick, source, target);
            executeState = 3;
        }


		for (auto iter = (skillData->skillExec).begin(); iter != (skillData->skillExec).end(); ++iter)
		{
            int compareTick = iter->param0;
			int theExecId = iter->execId;
            if (theExecId < 1)
                continue;

            if (source.attackSpeed > 0 && source.attackSpeed != 100)
                compareTick = compareTick * source.attackSpeed / 100;

            if (0 == executeState)
            {
                if (compareTick != tick)
                    continue;

                // HQ:5.0起,开场释放技能,执行ID的参数1为非0的,客户端不执行
                if (eSkillCastConditionOnFightBegin == skillData->castCondition && iter->param1)
                    continue;
            }
            else if (1 == executeState)
            {
                // 蓄力大招参数0: 执行延迟tick
                // 参数1: 0 - 蓄力时执行, 1 - 蓄力结束时执行(忽略参数0)
                if (compareTick != tick || iter->param1 != 0)
                    continue;
            }
            else if (3 == executeState)
            {
                auto afterTick = tick - HeroActionController::getAccumulateTick(source.actionAtt);
                if (compareTick != afterTick || iter->param1 != 1)
                    continue;
            }

            executeExecID(theExecId, skillData, aiContext, skillId, skillLv, tick, source, target);
		}

        // HQ: 7.0退出时重置状态放到攻击状态处理
        //if (3 == executeState && tick == source.totalTick)
        //    source.exitAccumulateState();
         
         */
        
        
        // MARK: 9.0 new impl
        if (IsAccumulateSkill(skillData->castCondition))
        {
            implExecuteExecIDInAccuSkillTick(aiContext, skillId, skillLv, tick, source, target, skillData);
        }
        else if (IsEnergySkill(skillData->castCondition))
        {
            implExecuteExecIDInEnergySkillTick(aiContext, skillId, skillLv, tick, source, target, skillData);
        }
        else if (IsRepeatSkill(skillData->castCondition))
        {
            implExecuteExecIDInRepeatSkillTick(aiContext, skillId, skillLv, tick, source, target, skillData);
        }
        else
        {
            implExecuteExecIDInNormalSkillTick(aiContext, skillId, skillLv, tick, source, target, skillData);
        }
	}
    
    void HeroSkillController::implExecuteExecIDInNormalSkillTick(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& source, CharacterData& target, AISkill *skillData)
    {
        for (auto iter = (skillData->skillExec).begin(); iter != (skillData->skillExec).end(); ++iter)
        {
            int compareTick = iter->param0;
            int theExecId = iter->execId;
            if (theExecId < 1)
                continue;
            
            if (source.attackSpeed > 0 && source.attackSpeed != 100)
                compareTick = compareTick * source.attackSpeed / 100;
            
            if (compareTick != tick)
                continue;
            
            // HQ:5.0起,开场释放技能,执行ID的参数1为非0的,客户端不执行
            if (eSkillCastConditionOnFightBegin == skillData->castCondition && iter->param1 != 0)
                continue;
            
            executeExecID(theExecId, skillData, aiContext, skillId, skillLv, tick, source, target);
        }
    }
    
    void HeroSkillController::implExecuteExecIDInAccuSkillTick(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& source, CharacterData& target, AISkill *skillData)
    {
        int executeState = 1; // 0 - 普通执行, 1 - 仅蓄力中执行, 2 - 蓄力结束执行
        if (!skillData->castParams.empty())
        {
            // MARK: to be optimised
            // @author: h7ing
            // 因本接口主要在AttackAction中调用，结束后会切换状态，为避免在一次更新中无意的多次切状态引起一些问题，
            // 在这里单独处理蓄力释放的逻辑，而没有统一调CharacterData的蓄力释放接口
            if (source.getAccumulateState() == EAccumulateState::kAfterAccumulate)
            {
                executeState = 3;
            }
            else
            {
                int theParamTick = skillData->castParams[0];
                //if (source.currentAlternateAccuMaxTick >= 0)
                //    theParamTick = source.currentAlternateAccuMaxTick;
                int theCurrentAltTick = source.getCurrentAlternateAccuMaxTick(skillId);
                if (theCurrentAltTick >= 0)
                    theParamTick = theCurrentAltTick;
                int theReadyTick = getAccuSkillReadyTick(skillId);
                if (theReadyTick > 0)
                    theParamTick += theReadyTick;
                if (source.attackSpeed > 0 && source.attackSpeed != 100)
                    theParamTick = theParamTick * source.attackSpeed / 100;
                if (tick >= theParamTick)
                    executeState = 2;
            }
        }
        
        if (2 == executeState)
        {
            endAccumulateSkill(aiContext, skillId, skillLv, tick, source, target);
            executeState = 3;
        }
        
        
        for (auto iter = (skillData->skillExec).begin(); iter != (skillData->skillExec).end(); ++iter)
        {
            int compareTick = iter->param0;
            int theExecId = iter->execId;
            if (theExecId < 1)
                continue;
            
            if (source.attackSpeed > 0 && source.attackSpeed != 100)
                compareTick = compareTick * source.attackSpeed / 100;
            
            if (0 == executeState)
            {
                if (compareTick != tick)
                    continue;
                
                // HQ:5.0起,开场释放技能,执行ID的参数1为非0的,客户端不执行
                if (eSkillCastConditionOnFightBegin == skillData->castCondition && iter->param1)
                    continue;
            }
            else if (1 == executeState)
            {
                // 蓄力大招参数0: 执行延迟tick
                // 参数1: 0 - 蓄力时执行, 1 - 蓄力结束时执行(忽略参数0)
                if (compareTick != tick || iter->param1 != 0)
                    continue;
            }
            else if (3 == executeState)
            {
                auto afterTick = tick - HeroActionController::getAccumulateTick(source.actionAtt);
                int theReadyTick = getAccuSkillReadyTick(skillId);
                if (theReadyTick > 0)
                    afterTick -= theReadyTick;
                if (afterTick < 0)
                    afterTick = 0;
                if (compareTick != afterTick || iter->param1 != 1)
                    continue;
            }
            
            executeExecID(theExecId, skillData, aiContext, skillId, skillLv, tick, source, target);
        }
    }
    
    void HeroSkillController::implExecuteExecIDInEnergySkillTick(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& source, CharacterData& target, AISkill *skillData)
    {
        for (auto iter = (skillData->skillExec).begin(); iter != (skillData->skillExec).end(); ++iter)
        {
            int theExecId = iter->execId;
            if (theExecId < 1)
                continue;
            
            int compareTick = iter->param0;
            if (source.attackSpeed > 0 && source.attackSpeed != 100)
                compareTick = compareTick * source.attackSpeed / 100;
            
            if (compareTick != tick)
                continue;
            
            bool isEnergySkillFirstRound = source.hasTag("energyskillfirstround");
            // 能量消耗型大招参数1: 0-只在第一遍执行一次 1-第一遍不执行，之后循环执行
            int compareMode = iter->param1;
            if (compareMode == 0 && !isEnergySkillFirstRound)
                continue;
            else if (compareMode == 1 && isEnergySkillFirstRound)
                continue;
            
            executeExecID(theExecId, skillData, aiContext, skillId, skillLv, tick, source, target);
        }
    }

    void HeroSkillController::implExecuteExecIDInRepeatSkillTick(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData& source, CharacterData& target, AISkill *skillData)
    {
        for (auto iter = (skillData->skillExec).begin(); iter != (skillData->skillExec).end(); ++iter)
        {
            int theExecId = iter->execId;
            if (theExecId < 1)
                continue;

            int compareTick = iter->param0;
            if (source.attackSpeed > 0 && source.attackSpeed != 100)
                compareTick = compareTick * source.attackSpeed / 100;

            if (compareTick != tick)
                continue;

            bool isSkillFirstRound = source.hasTag("repeatskillfirstround");
            // 参数1: 0-只在第一遍执行一次 1-第一遍不执行，之后循环执行
            int compareMode = iter->param1;
            if (compareMode == 0)
            {
                if (!isSkillFirstRound) continue;
            }
            else if (compareMode == 1)
            {
                if (isSkillFirstRound) continue;
            }
            else
            {
                continue;
            }

            executeExecID(theExecId, skillData, aiContext, skillId, skillLv, tick, source, target);
        }
    }
    
    void HeroSkillController::executeBeginSkillExecIDInTick(AIContext& aiContext, int skillId, int skillLv, int tick, CharacterData *source, CharacterData *target, AISkill *skillData, bool isProtective, int lazyTick)
    {
        for (auto iter = (skillData->skillExec).begin(); iter != (skillData->skillExec).end(); ++iter) {
            // 5.0起,开场释放技能,执行ID的参数1为非0的,客户端不执行
            if (iter->param1 != 0)
                continue;
            int compareTick = iter->param0;
            int theExecId = iter->execId;
            if (theExecId < 1)
                continue;
            
            if (source->attackSpeed > 0 && source->attackSpeed != 100)
                compareTick = compareTick * source->attackSpeed / 100;
            
            if (compareTick != tick)
                continue;
            
            // 12.0: 开场释放技能 执行播放类型：0-非保护性执行 1-保护性执行
            if (isProtective) {
                if (iter->execType != 1)
                    continue;
            } else {
                if (iter->execType != 0)
                    continue;
            }
            
            executeExecID(theExecId, skillData, aiContext, skillId, skillLv, tick, *source, *target);
        }
    }

    int HeroSkillController::calculateAccumulateSkillReleaseLength(int skillId)
    {
        auto skill = AIDataHelper::getSkillData(skillId);
        if (!skill)
            return 0;

        int theMax = 0;
        for (auto iter = skill->skillExec.begin(); iter != skill->skillExec.end(); ++iter)
        {
            if (IsEffectExec(iter->execId) || iter->param1 != 1)
                continue;
            int execLen = HeroSkillController::calculateExecLength(iter->execId);
            if (theMax < iter->param0 + execLen)
                theMax = iter->param0 + execLen;
        }
        return theMax;
    }

    int HeroSkillController::calculateAccuSkillLength(int skillId, int theAlternateMaxAccuTick)
    {
        int theMax0 = 0;
        AISkill *skill = AIDataHelper::getSkillData(skillId);
        if (!skill)
            return 0;
        if (!skill->castParams.empty())
            theMax0 = skill->castParams[0];
        if (theAlternateMaxAccuTick >= 0)
            theMax0 = theAlternateMaxAccuTick;
        int theMax1 = calculateAccumulateSkillReleaseLength(skillId);
        return theMax0 + theMax1;
    }

    int HeroSkillController::calculateRepeatSkillFirstRoundTickLength(int skillId)
    {
        auto skill = AIDataHelper::getSkillData(skillId);
        if (!skill)
            return 0;

        int theMax = 0;
        if (skill->castParams.size() > 2)
        {
            theMax = skill->castParams[2];
        }
        else
        {
            for (auto iter = skill->skillExec.begin(); iter != skill->skillExec.end(); ++iter)
            {
                if (IsEffectExec(iter->execId) || iter->param1 != 0)
                    continue;
                int execLen = HeroSkillController::calculateExecLength(iter->execId);
                if (theMax < iter->param0 + execLen)
                    theMax = iter->param0 + execLen;
            }
        }
        return theMax;
    }

    int HeroSkillController::calculateEnergySkillFirstRoundTickLength(int skillId)
    {
        auto skill = AIDataHelper::getSkillData(skillId);
        if (!skill)
            return 0;

        int theMax = 0;
        if (skill->castParams.size() > 1)
        {
            theMax = skill->castParams[1];
        }
        else
        {
            for (auto iter = skill->skillExec.begin(); iter != skill->skillExec.end(); ++iter)
            {
                if (IsEffectExec(iter->execId) || iter->param1 != 0)
                    continue;
                int execLen = HeroSkillController::calculateExecLength(iter->execId);
                if (theMax < iter->param0 + execLen)
                    theMax = iter->param0 + execLen;
            }
        }
        return theMax;
    }

	int HeroSkillController::calculateSkillLength(int skillId)
	{
		AISkill * skill = AIDataHelper::getSkillData(skillId);
        if (!skill)
            return 0;

        if (IsAccumulateSkill(skill->castCondition) && !skill->castParams.empty())
        {
            int theMax0 = skill->castParams[0]; // 蓄力时间
            int theMax1 = calculateAccumulateSkillReleaseLength(skillId); // 蓄力后释放时间
            int theMax2 = 0;
            if (skill->castParams.size() > 1)
                theMax2 = skill->castParams[1]; // 起手时间
            return  theMax0 + theMax1 + theMax2;
        }

		if (skill->skillExec.size() > 0) {
			std::vector<SkillExec>::iterator iter;
			int theMax=0;
			for (iter = skill->skillExec.begin(); iter != skill->skillExec.end(); ++iter)
			{
				if (IsEffectExec(iter->execId))
					continue;
				int execLen = HeroSkillController::calculateExecLength(iter->execId);
				if (theMax < iter->param0 + execLen) theMax = iter->param0 + execLen;
			}
			return theMax;
		}
		else{
			return 0;
		}
	}

	int HeroSkillController::calculateExecLength(int execID)
	{
		if (IsHurtExec(execID))
		{
            auto theHurtExec = AIDataHelper::getHurtExec(execID);
            if (!theHurtExec) return 0;
            return theHurtExec->duration;
		}
		else if (IsProjectileExec(execID))
		{
            auto theProjectileExec = AIDataHelper::getProjectileExec(execID);
            if (!theProjectileExec) return 0;
            return theProjectileExec->throwDuration;
		}
		else if (IsEffectExec(execID))
		{
            auto theEffectExec = AIDataHelper::getEffectExec(execID);
            if (!theEffectExec) return 0;
            return theEffectExec->period;
		}
		else if (IsSoundExec(execID))
		{
			//return AIDataHelper::getSound(execID)->duration;
			return 10;
		}
		else if (IsSpecialUIEffectExec(execID))
		{
			if (IsLogicSpecialUIEffectExec(execID))
			{
				LogicSpecialUIEffectExec* logicEffExec = AIDataHelper::getLogicSpecialUIEffectExec(execID);
				return logicEffExec->getDuration();
			}
            else
            {
                auto theSpecialUiEffectExec = AIDataHelper::getSpecialUIEffectExec(execID);
                if (!theSpecialUiEffectExec) return 0;
                return theSpecialUiEffectExec->duration;
            }
		}
		else if (IsAnimationExec(execID))
		{
			if (IsLogicAnimationExec(execID))
			{
				LogicAnimationExec* aniExec = AIDataHelper::getLogicAnimationExec(execID);
				return aniExec->getDuration();
			}
            else
            {
                auto theAnimationExec = AIDataHelper::getSkillAnimationExec(execID);
                if (!theAnimationExec) return 0;
                return theAnimationExec->duration;
            }
		}
        else if (IsMoveExec(execID))
        {
			// 把移动执行中，结束后执行的动画执行的长度添加进skill列表中，防止移动执行结束后，动画播放即被打断
            auto exec = AIDataHelper::getMoveExec(execID);

			int nTmp = 0;
			for (int i = 0; i < exec->vAfterMoveExecs.size(); i++)
			{
				int nAID = exec->vAfterMoveExecs[i];
				if (IsAnimationExec(nAID))
				{
					if (IsLogicAnimationExec(nAID))
					{
						LogicAnimationExec* aniExec = AIDataHelper::getLogicAnimationExec(nAID);
						nTmp += (exec->getRepeatCasterCnt() * aniExec->getDuration());
					}
					else
					{
                        if (AIDataHelper::getSkillAnimationExec(nAID))
						    nTmp += (exec->getRepeatCasterCnt() * AIDataHelper::getSkillAnimationExec(nAID)->duration);
					}
				}
				else if (IsHurtExec(nAID))
				{
                    if (AIDataHelper::getHurtExec(nAID))
                    {
                        nTmp += (exec->getRepeatCasterCnt() * AIDataHelper::getHurtExec(nAID)->duration);
                    }
				}
			}

		/*	int moveExecId = exec->movingExecuteID;
			if (moveExecId > 0)
			{
				if (IsAnimationExec(moveExecId))
				{
					int nDur = 0;
					if (IsLogicAnimationExec(moveExecId))
					{
						LogicAnimationExec* aniExec = AIDataHelper::getLogicAnimationExec(moveExecId);
						nDur = aniExec->getDuration();
					}
					else
					{
						nDur = AIDataHelper::getSkillAnimationExec(moveExecId)->duration;
					}

					nTmp += ((exec->duration / nDur) * nDur);
				}
			}*/

			if (exec->targetTracing == 2) // 穿墙 (int)MoveType::kTrackTargetMove
            {
				return (nTmp + AIContext::getInstance()->clusterRangeToCenterX * 2 / exec->speedX);
            }
			else
			{
				return (nTmp + 10);
			}
        }
		else
		{
			return 10;
		}
	}

	//for skill, its target is not the source->target, it is calculated for each skill specifically.
	std::list<CharacterData*>* HeroSkillController::getSkillTarget(AIContext& aiContext, CharacterData* caster, ESkillTargetType targetType)
	{
		//TODO to be implemented
		std::list<CharacterData*>* targets = new std::list<CharacterData*>();
		targets->push_back(caster->target);

		return targets;
	}

    bool HeroSkillController::isSkillSwapWithNormal(int skillID)
    {
        auto *pSkill = AIDataHelper::getSkillData(skillID);
        if (pSkill && ESkillCastCondition::eSkillCastConditionRandomSwapNormal == pSkill->castCondition
            && pSkill->castParams.size() > 0)
        {
            return (AIContext::getInstance()->localRandom->nextInt(0, 100) < pSkill->castParams[0]);
        }
        return false;
    }

    int HeroSkillController::getAttackSeqFirstMatchIndex(CharacterData* character, int skillIndex, bool useSeq2)
    {
        auto matchFunc = [skillIndex](std::vector<int> &seq)->int{
            for (int i = 0; i < seq.size(); i++) {
                if (seq[i] == skillIndex)
                    return i;
            }
            return -1;
        };

        return matchFunc(useSeq2 ? character->staticData->attackSeq2 : character->staticData->attackSeq);
    }

    uint32_t HeroSkillController::calculateSkillRangeSquare(int skillID)
    {
        auto pSkill = AIDataHelper::getSkillData(skillID);
        if (!pSkill)
            return 0;
        auto range = pSkill->range;
        return range * range;
    }

    bool HeroSkillController::isSkillIgnoreCD(int skillID)
    {
        auto pSkill = AIDataHelper::getSkillData(skillID);
        if (!pSkill)
            return false;
        switch (pSkill->castCondition) {
        case ESkillCastCondition::eSkillCastConditionOnHurtPlus:
        case ESkillCastCondition::eSkillCastConditionAnyEnemyHP:
        case ESkillCastCondition::eSkillCastConditionOnSelfHPBreakAction: // HQ:8.0增加
            return true;
            break;
        default:break;
        }
        return false;
    }

    void HeroSkillController::updateSpecialSkillCD(CharacterData *character)
    {
        auto pSkill = character->getSpecialSkill(ESkillCastCondition::eSkillCastConditionAnyEnemyHP);
        if (pSkill)
        {
            OnEnemyHPSkillInfo *pHPSkill = (OnEnemyHPSkillInfo *)pSkill;
            pHPSkill->tick();
        }
    }

    SkillTargetInfo HeroSkillController::getRealSearchInfo(int originSearchType)
    {
        SkillTargetInfo info;
        info.realSearchType = originSearchType;

        // +10000:无视不能成为目标的效果
        if (info.realSearchType >= 10000)
        {
            info.ignoreCanbeattackEffect = true;
            info.realSearchType -= 10000;
        }

        // +3000:仅从召唤物中找
        if (info.realSearchType >= 3000)
        {
            info.isonlySummon = true;
            info.realSearchType -= 3000;
        }

        // +2000:先排除召唤物，如果没有找到，再从召唤物中找
        if (info.realSearchType >= 2000)
        {
            info.searchAgainIfExcludeSummon = true;
            info.realSearchType -= 1000;
        }

        // +1000:目标类型为不含召唤物的原目标类型
        if (info.realSearchType >= 1000)
        {
            info.excludeSummon = true;
            info.realSearchType -= 1000;
        }

        return info;
    }

    bool HeroSkillController::isTargetSameNoSummonOption(int type, int comp)
    {
        return getRealSearchInfo(type).realSearchType == comp;
    }

    bool HeroSkillController::isSkillIgnoreSilence(int skillID)
    {
        auto skillData = AIDataHelper::getSkillData(skillID);
        if (!skillData)
            return false;
        return isSkillDataIgnoreSilence(skillData);
    }

    bool HeroSkillController::isSkillDataIgnoreSilence(AISkill *skillData)
    {
        for (auto &exec : skillData->skillExec)
        {
            if (IsEffectExec(exec.execId))
            {
                auto effectData = AIDataHelper::getEffectExec(exec.execId);
                if (!effectData)
                    continue;
                for (auto &&innerEff : effectData->execs)
                {
                    if (innerEff.ID == eEFFECT_IGNORE_SILENCE)
                        return true;
                }
            }
        }
        return false;
    }

    bool HeroSkillController::isTargetIgnoreSilenceBySkill(AIContext &aiContext, CharacterData *target, int skillID)
    {
        bool ignore_silence = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_IGNORE_SILENCE, 0) > 0;
        if (!ignore_silence)
        {
            ignore_silence = target->checkIgnoreSilenceSkill(skillID);
        }
        return ignore_silence;
    }

    bool HeroSkillController::isTargetIgnoreSilenceBySkillIndex(AIContext &aiContext, CharacterData *target, int index)
    {
        return isTargetIgnoreSilenceBySkill(aiContext, target, CharacterFunc::GetSkillIDByIndex(*target, index));
    }

    int HeroSkillController::getRepeatSkillMaxRepeatTick(CharacterData *character, int skillId)
    {
        auto skillData = AIDataHelper::getSkillData(skillId);
        if (!skillData || !IsRepeatSkill(skillData->castCondition))
            return 0;

        // 技能释放参数：
        // a - 默认最大重复持续时间（不含起手时非重复的时间）
        // b - 符合延长条件时的默认基础重复持续时间（不含起手时非重复的时间）
        // c - 重复开始时刻（释放完起手非重复的开始时刻，即非重复的持续时间）
        // d - 使用延长时间的条件类型 0 - 不延长，使用默认最大重复持续时间 1 - 施法者造成暴击 2 - 施法者拥有技能（列表，只要拥有一个就算符合）
        // e...延长条件参数：指定拥有的技能列表，预留5位
        if (skillData->castParams.size() > 3)
        {
            int theRepeatTickType = skillData->castParams[3];
            if (theRepeatTickType == AIREPEATSKILL_DEFAULT_MAX_TICK)
            {
                return skillData->castParams[1];
            }
            else if (theRepeatTickType == AIREPEATSKILL_EXTEND_ON_CRIT)
            {
                return skillData->castParams[1];
            }
            else if (theRepeatTickType == AIREPEATSKILL_EXTEND_ON_HAS_SKILL)
            {
                if (skillData->castParams.size() > 4)
                {
                    // 预留5位参数，作为拥有指定技能列表
                    for (int i = 4; i < skillData->castParams.size() && i < 9; i++)
                    {
                        if (character->hasSkill(skillData->castParams[i]))
                        {
                            return skillData->castParams[0];
                        }
                    }
                }

                return skillData->castParams[1];
            }
        }

        return 0;
    }

    int HeroSkillController::getRepeatSkillRepeatType(int skillId)
    {
        auto skillData = AIDataHelper::getSkillData(skillId);
        if (!skillData || !IsRepeatSkill(skillData->castCondition))
            return AIREPEATSKILL_DEFAULT_MAX_TICK;
        if (skillData->castParams.size() > 3)
        {
            return skillData->castParams[3];
        }
        return AIREPEATSKILL_DEFAULT_MAX_TICK;
    }

    int HeroSkillController::getRepeatSkillExtendRepeatTickForCrit(int skillId)
    {
        auto skillData = AIDataHelper::getSkillData(skillId);
        if (!skillData || !IsRepeatSkill(skillData->castCondition))
            return 0;

        if (skillData->castParams.size() > 4)
        {
            int theRepeatType = skillData->castParams[3];
            if (theRepeatType == AIREPEATSKILL_EXTEND_ON_CRIT)
                return skillData->castParams[4];
        }

        return 0;
    }

    int HeroSkillController::getRepeatSkillMaxRepeatTick(int skillId)
    {
        auto skillData = AIDataHelper::getSkillData(skillId);
        if (!skillData || !IsRepeatSkill(skillData->castCondition))
            return 0;

        if (skillData->castParams.size() > 0)
        {
            return skillData->castParams[0];
        }

        return 0;
    }

    int HeroSkillController::getRepeatSkillReadyTick(int skillId)
    {
        auto skillData = AIDataHelper::getSkillData(skillId);
        if (!skillData || !IsRepeatSkill(skillData->castCondition))
            return 0;

        if (skillData->castParams.size() > 2)
            return skillData->castParams[2];

        return 0;
    }

    int HeroSkillController::getRepeatSkillBasicTick(int skillId)
    {
        auto skillData = AIDataHelper::getSkillData(skillId);
        if (!skillData || !IsRepeatSkill(skillData->castCondition))
            return 0;

        if (skillData->castParams.size() > 1)
        {
            return skillData->castParams[1];
        }

        return 0;
    }

    int HeroSkillController::getAccuSkillReadyTick(int skillId)
    {
        AISkill * skill = AIDataHelper::getSkillData(skillId);
        if (!skill)
            return 0;

        if (IsAccumulateSkill(skill->castCondition) && skill->castParams.size() > 1)
        {
            return skill->castParams[1];
        }

        return 0;
    }

    int HeroSkillController::getAccuSkillMaxAccuTick(int skillId)
    {
        AISkill * skill = AIDataHelper::getSkillData(skillId);
        if (!skill)
            return 0;

        if (skill->castParams.size() > 0)
        {
            return skill->castParams[0];
        }

        return 0;
    }
    
    bool HeroSkillController::isSkillTargetForSingle(AISkill *skillData)
    {
        return skillData->rangeType == 1;
    }

    /////////////////////////////////////////////////////////
    // SpecialSkillInfo
    /////////////////////////////////////////////////////////

    std::unique_ptr<SpecialSkillInfo> SpecialSkillInfo::create(CharData *charData, int skillID, int skillLevel, AISkill *skillData)
    {
        std::unique_ptr<SpecialSkillInfo> pSkill;
        //auto skillData = AIDataHelper::getSkillData(skillID);
        if (!skillData)
            return nullptr;
        const auto castCondition = skillData->castCondition;
        auto getSkillIdx = [charData, skillID]()->int{
            for (int iSk = 0; iSk < charData->sk.size(); iSk++)
            {
                if (skillID == charData->sk[iSk])
                    return iSk;
            }
            return -1;
        };
        
        if (eSkillCastConditionOnMultipleHurt == castCondition || eSkillCastConditionOnHurtPlus == castCondition)
        {
            pSkill = std::unique_ptr<SpecialSkillInfo>(new OnHurtSkillInfo(castCondition, skillID, getSkillIdx(), skillLevel, skillData->castParams[0]));
        }
        else if (eSkillCastConditionOnSelfHP == castCondition || eSkillCastConditionOnSelfHPBreakAction == castCondition)
        {
            // 通过HP判定释放的技能
            //3 - 自身HP判定，4 - 目标HP判定
            //    参数为数组［a，b，c］
            //    a：0－大于等于；1－小于等于
            //    b：数值
            //    c：0 - 固定值；1 - 百分比
            auto paramA = OnHPLimitSkillInfo::CompareType(skillData->castParams[0]);
            auto paramB = skillData->castParams[1];
            auto paramC = OnHPLimitSkillInfo::LimitParamType(skillData->castParams[2]);

            pSkill = std::unique_ptr<SpecialSkillInfo>(new OnHPLimitSkillInfo(castCondition, skillID, getSkillIdx(), skillLevel, paramB, paramA, paramC));
        }
        else if (eSkillCastConditionOnAvoidAndBreakAction == castCondition || eSkillCastConditionOnAvoidNoBreakAction == castCondition)
        {
            pSkill = std::unique_ptr<SpecialSkillInfo>(new OnEscapeSkillInfo(castCondition, skillID, getSkillIdx(), skillLevel));
        }
        else if (eSkillCastConditionAfterCruelHit == castCondition)
        {//clair_wang 造成暴击后触发
            pSkill = std::unique_ptr<SpecialSkillInfo>(new OnAfterCruelHitSkillInfo(castCondition, skillID, getSkillIdx(), skillLevel));
        }
        else if (eSkillCastConditionOnDeath == castCondition)
        {
            int limit = 0;
            if (skillData->castParams.size() > 0)
                limit = skillData->castParams[0];
            pSkill = std::unique_ptr<SpecialSkillInfo>(new OnDeadSkillInfo(castCondition, skillID, getSkillIdx(), skillLevel, limit));
        }
        else if (eSkillCastConditionAnyEnemyHP == castCondition)
        {
            //    参数为数组［a，b，c, d］
            //    a：CD
            //    b：下限
            //    c：上限
			//    d: 是否包含召唤物 0：不包含  1：包含
            auto paramA = skillData->castParams[0];
            auto paramB = skillData->castParams[1];
            auto paramC = skillData->castParams[2];
            int paramD = 1;
            if (skillData->castParams.size() > 3)
			    paramD = skillData->castParams[3];
            pSkill = std::unique_ptr<SpecialSkillInfo>(new OnEnemyHPSkillInfo(castCondition, skillID, getSkillIdx(), skillLevel, paramA, paramB, paramC, paramD));
        }
        return pSkill;
    }

    SpecialSkillInfo::SpecialSkillInfo(ESkillCastCondition condition, int skillID, int skillIDIndex, int skillLevel) :
        castCondition(condition), skillID(skillID), skillIDIndex(skillIDIndex), skillLevel(skillLevel)
    {
    }


    /////////////////////////////////////////////////////////
    // OnDeadSkillInfo
    /////////////////////////////////////////////////////////

    OnDeadSkillInfo::OnDeadSkillInfo(ESkillCastCondition condition, int skillID, int skillIDIndex, int skillLevel, int limitValue)
        :SpecialSkillInfo(condition, skillID, skillIDIndex, skillLevel), _limit(limitValue)
    {
    }

    bool OnDeadSkillInfo::canCast(CharacterData *caster)
    {
        // 22.0
        // 增加了死亡触发技的释放次数参数,兼容原来参数为0的老配置:
        // 限制次数为0时,整场只能释放1次
        if (_limit == -1)
        {
            return true;
        }
        else if (_limit == 0)
        {
            return releaseCount <= _limit;
        }
        return releaseCount < _limit;
    }

    void OnDeadSkillInfo::onConditon() {}

    void OnDeadSkillInfo::onCast()
    {
        releaseCount++;
    }

    void OnDeadSkillInfo::resetCondition() {}
    
    void OnDeadSkillInfo::resetClusterCondition()
    {
        // 22.0新增
        // 当限制次数>0时,过节点重置释放次数
        if (_limit > 0)
        {
            releaseCount = 0;
        }
    }

    /////////////////////////////////////////////////////////
    // OnHPLimitSkillInfo
    /////////////////////////////////////////////////////////

    OnHPLimitSkillInfo::OnHPLimitSkillInfo(ESkillCastCondition condition, int skillID, int skillIDIndex, int skillLevel, int limitValue, CompareType compType, LimitParamType limitType)
        : SpecialSkillInfo(condition, skillID, skillIDIndex, skillLevel), _limit(limitValue), _limitType(limitType)
    {
        if (CompareType::kGreaterOrEqual == compType)
            _compFunc = &GreaterOrEqual;
        else if (CompareType::kLessOrEqual == compType)
            _compFunc = &LessOrEqual;
    }

    bool OnHPLimitSkillInfo::canCast(CharacterData *caster)
    {
        if (releaseCount > 0)
            return false;

        if (LimitParamType::kLiteral == _limitType)
            return _compFunc(caster->getHP(), _limit);
        else if (LimitParamType::kPercent == _limitType)
            return _compFunc(CharacterFunc::GetCharacterHPRatio(*caster) * 100.0f, _limit);

        return false;
    }

    void OnHPLimitSkillInfo::onConditon()
    {
    }

    void OnHPLimitSkillInfo::onCast()
    {
        releaseCount++;
    }

    void OnHPLimitSkillInfo::resetCondition()
    {
    }

    bool OnHPLimitSkillInfo::isLimitHigher(CharacterData *selfCaster, OnHPLimitSkillInfo *another, CharacterData *anotherCaster)
    {
        if (!another)
            return false;

        int mylimit = 0;
        if (LimitParamType::kLiteral == _limitType)
            mylimit = _limit;
        else if (LimitParamType::kPercent == _limitType) {
            float val = selfCaster->getHPTop();
            float ratio = (float)_limit / 10000.0;
            mylimit = val * ratio;
        }

        int anotherlimit = 0;
        if (LimitParamType::kLiteral == another->_limitType)
            anotherlimit = another->_limit;
        else if (LimitParamType::kPercent == another->_limitType) {
            float val = anotherCaster->getHPTop();
            float ratio = (float)another->_limit / 10000.0;
            anotherlimit = val * ratio;
        }

        return (mylimit > anotherlimit);
    }

    /////////////////////////////////////////////////////////
    // OnEscapeSkillInfo
    /////////////////////////////////////////////////////////

    OnEscapeSkillInfo::OnEscapeSkillInfo(ESkillCastCondition condition, int skillID, int skillIDIndex, int skillLevel)
        : SpecialSkillInfo(condition, skillID, skillIDIndex, skillLevel)
    {
    }

    bool OnEscapeSkillInfo::canCast(CharacterData *caster)
    {
        return _escapeCount > 0;
    }

    void OnEscapeSkillInfo::onConditon()
    {
        _escapeCount++;
    }

    void OnEscapeSkillInfo::onCast()
    {
        releaseCount++;
        _escapeCount = 0;
    }

    void OnEscapeSkillInfo::resetCondition()
    {
        _escapeCount = 0;
    }


    /////////////////////////////////////////////////////////
    // 造成暴击后触发
    // OnAfterCruelHitSkillInfo
    /////////////////////////////////////////////////////////

    OnAfterCruelHitSkillInfo::OnAfterCruelHitSkillInfo(ESkillCastCondition condition, int skillID, int skillIDIndex, int skillLevel)
        : SpecialSkillInfo(condition, skillID, skillIDIndex, skillLevel)
    {
    }

    bool OnAfterCruelHitSkillInfo::canCast(CharacterData *caster)
    {
        return _nAfterCruelHitCount > 0;
    }

    void OnAfterCruelHitSkillInfo::onConditon()
    {
        _nAfterCruelHitCount++;
    }

    void OnAfterCruelHitSkillInfo::onCast()
    {
        releaseCount++;
        _nAfterCruelHitCount = 0;
    }

    void OnAfterCruelHitSkillInfo::resetCondition()
    {
        _nAfterCruelHitCount = 0;
    }

    /////////////////////////////////////////////////////////
    // OnHurtSkillInfo
    /////////////////////////////////////////////////////////

    OnHurtSkillInfo::OnHurtSkillInfo(ESkillCastCondition condition, int skillID, int skillIDIndex, int skillLevel, int limitValue)
        : SpecialSkillInfo(condition, skillID, skillIDIndex, skillLevel), _limit(limitValue)
    {
    }

    bool OnHurtSkillInfo::canCast(CharacterData *caster)
    {
        return _onHurtCount >= _limit;
    }

    void OnHurtSkillInfo::onConditon()
    {
        _onHurtCount++;
    }

    void OnHurtSkillInfo::onCast()
    {
        releaseCount++;
        _onHurtCount = 0;
    }

    void OnHurtSkillInfo::resetCondition()
    {
        _onHurtCount = 0;
    }

    /////////////////////////////////////////////////////////
    // OnEnemyHPSkillInfo
    /////////////////////////////////////////////////////////

	OnEnemyHPSkillInfo::OnEnemyHPSkillInfo(ESkillCastCondition condition, int skillID, int skillIDIndex, int skillLevel, int interval, int boundsLow, int boundsHigh, int isIncludeSummon)
		: SpecialSkillInfo(condition, skillID, skillIDIndex, skillLevel), _interval(interval), _boundsLow(boundsLow), _boundsHigh(boundsHigh), _isIncludeSummon(isIncludeSummon)
    {
    }

    bool OnEnemyHPSkillInfo::canCast(CharacterData *caster)
    {
        if (isInCD() || _onConditionCount <= 0)
            return false;
        for (auto iter = _checkTargets.begin(); iter != _checkTargets.end();)
        {
            auto pTarget = AIContext::getInstance()->getCharacterByID(iter->first);
            if (pTarget && isTargetMeetCondition(pTarget, caster))
                return true;
            
            _checkTargets.erase(iter++);
        }
        return false;
    }

    void OnEnemyHPSkillInfo::onConditon()
    {
        _onConditionCount++;
    }

    void OnEnemyHPSkillInfo::onCast()
    {
        _onConditionCount = 0;
        _localInterval = _interval;
        _checkTargets.clear();
    }

    void OnEnemyHPSkillInfo::resetCondition()
    {
        _onConditionCount = 0;
        _localInterval = 0;
        _checkTargets.clear();
    }

    bool OnEnemyHPSkillInfo::isInCD()
    {
        return _localInterval > 0;
    }

	bool OnEnemyHPSkillInfo::isTargetMeetCondition(CharacterData *target, CharacterData *source)
    {
        if (_localInterval > 0)
            return false;
		
		if (_isIncludeSummon == 0 && target->isSummon() && target->getFakeSummonType() == 0 /*12.0*/)
		{
			return false;
		}

		if (!target->canBeAttacked(source, false, source))//HeroActionController::isDeadAction(target->currentAction))
            return false;

        //    参数为数组［a，b，c, d］
        //    a：技能再次触发CD，如配40，表示CD为40 tick
        //    b：百分比下限，如配50，表示下限为50%
        //    c：百分比上限
		//    d: 是否包含召唤物 0：不包含  1：包含
        int ratio = CharacterFunc::getSimplifiedHPRatio(target);
        return ratio >= _boundsLow && ratio <= _boundsHigh;
    }

    void OnEnemyHPSkillInfo::addCheckTarget(CharacterData *target)
    {
        _checkTargets[target->myID] = true;
    }

    void OnEnemyHPSkillInfo::tick()
    {
        if (_localInterval > 0)
            _localInterval -= 1;
    }

} // namespace OnePiece
