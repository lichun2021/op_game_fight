//
//  AIActionController.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/28/14.
//
//

#include "AIActionController.h"
#include <algorithm>
#include "AIContext.h"
#include "AIContinuousTransformCaster.h"
#include "AIContinuousEffectCaster.h"
#include "action_hero/HeroEffectAction.h"
#include "action_hero/HeroActionController.h"
#include "action_hero/HeroSkillController.h"
#include "AIScriptBinder.hpp"
#include "scriptbind/AIScriptBindActionController.hpp"
#include "scriptbind/AIScriptBindEffectCaster.hpp"
#include "AITargetHelper.h"

#ifndef SERVER_VERIFYING
#include "../DataManager/DataManager.h"
#endif

namespace OnePiece {

    ExecExcuteInfo::ExecExcuteInfo(
        int theExecID,
        int theSkillID,
        int theSkillLevel,
        CharacterData* theSource,
        AIProjectile* theProjectile,
        CharacterData* theTarget,
        int theParentTicklen,
        bool theUseLogicPos,
        int theWaitTickLen)
        : execID(theExecID), skillID(theSkillID), skillLv(theSkillLevel), pSource(theSource), pProjectile(theProjectile), pTarget(theTarget), parentTickLen(theParentTicklen), useLogicPos(theUseLogicPos), waitTickLen(theWaitTickLen)
    {

    }

    class AIActionController::SearchParam {
    public:
        int searchType = 0;
        CharacterDataList *filterList = nullptr;
        CharacterDataList *friendList = nullptr;
        CharacterDataList *rivalList = nullptr;
        CharacterDataList *targetList = nullptr;
        CharacterData *source = nullptr;
        CharacterData *target = nullptr;
        CharacterData *skillCaster = nullptr;
        AIContext *aiContext = nullptr;
        AIProjectile *projectile = nullptr;
        std::vector<int> *searchParams = nullptr;
        bool ignoreCanbeattackEffect = false;
        int x1 = 0;
        int x2 = 0;
        int y1 = 0;
        int y2 = 0;
        int z1 = 0;
        int z2 = 0;
        int hurtID = 0;
        int skillLevel = 0;
    };

    namespace AIActionInnerHelper {

        void InvalidateHurtCaster(CharacterData *c, int casterID)
        {
            for (auto iter = c->hurtCasts.rbegin(); iter != c->hurtCasts.rend(); ++iter)
            {
                AIContinuousHurtCaster *caster = *iter;
                if (caster->hurtID == casterID && caster->isValid)
                {
                    caster->isValid = false;
                }
            }
        }

    }

    AIActionController::AIActionController(){}
    AIActionController::~AIActionController(){}
    
    //执行总入口    
    void AIActionController::executeExec(AIContext& aiContext, int execID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool useLogicPos, int markID, bool isproj12reachexec)
    {

        /// 12.0
        if (aiContext.isBeingFinalClear() && aiContext.shouldStopExecWhenBeingFinalClear())
            return;
#ifdef OPTOOLS
        // 实际执行时此处不一定调用
        int tmpCurrentSkillID = aiContext.currentSkillID;
        aiContext.currentSkillID = skillID;
        int tmpCurrentExecID = aiContext.currentExecID;
        aiContext.currentExecID = execID;
#endif
        //if (execID == 599999) {
        //    CCLOG("find excid");
        //}

       

        if (IsHurtExec(execID))
            executeHurt(aiContext, execID, skillID, skillLevel, source, projectile, target, parentTicklen, markID, isproj12reachexec);
        else if (IsProjectileExec(execID))
            executeProjectile(aiContext, execID, skillID, skillLevel, source, projectile, target, parentTicklen);
        else if (IsEffectExec(execID))
            executeEffect(aiContext, execID, skillID, skillLevel, source, projectile, target, parentTicklen, markID, isproj12reachexec);
        else if (IsSoundExec(execID))
            executeAudio(aiContext, execID, skillLevel, source, projectile, target, parentTicklen);
        else if (IsSpecialUIEffectExec(execID))
            executeSpecialEffect(aiContext, 0, execID, skillLevel, skillID, source, projectile, target, parentTicklen,useLogicPos);
        else if (IsSpecialExec(execID))
            executeSpecialExec(aiContext, execID, skillID, source, projectile, target, parentTicklen);
        else if (IsAnimationExec(execID))
            executeAnimation(aiContext, execID, skillLevel, source, projectile, target, parentTicklen);
        else if (IsMoveExec(execID))
            executeMovement(aiContext, execID, skillID, skillLevel, source, projectile, target, parentTicklen);
        else if (IsMarkExec(execID))
            executeMark(aiContext, execID, skillID, skillLevel, source, projectile, target, parentTicklen, isproj12reachexec);
        else if (IsSummonExec(execID))
            executeSummon(aiContext, execID, skillLevel, source, projectile, target, parentTicklen);
        else if (IsTransformExec(execID))
            executeTransformExec(aiContext, execID, skillID, skillLevel, source, projectile, target, parentTicklen, useLogicPos);
        else if (IsBodyUIEffectExec(execID))
            executeBodyEffect(aiContext, 0, execID, skillLevel, source, projectile, target, parentTicklen);
#ifdef OPTOOLS
        // 实际执行时此处不一定调用
        aiContext.currentSkillID = tmpCurrentSkillID;
        aiContext.currentExecID = tmpCurrentExecID;
#endif
    }

    //新增延时执行，主要针对特效 声音 文字 等表现入口 cui 2020_2_23  op3  delayAIAction
    void AIActionController::executeDelayExec(AIContext& aiContext, int execID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool useLogicPos, int markID, bool isproj12reachexec, int delayTick) {
        DelayAIActionData* param = new DelayAIActionData();
        param->delayTick = delayTick;
        param->execID = execID;
        param->skillID = skillID;
        param->skillLevel = skillLevel;
        param->source = source;
        param->target = target;
        param->parentTicklen = parentTicklen;
        param->useLogicPos = useLogicPos;
        param->markID = markID;
        param->isproj12reachexec = isproj12reachexec;
        aiContext.dalayAIActionList.push_back(param);

    }

    
    void AIActionController::executeAudio(AIContext& aiContext, int audioID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen)
    {
        if (aiContext.audioProxy!=nullptr)
            aiContext.audioProxy->executeExec(aiContext, audioID, skillLevel, source, projectile, target, parentTicklen);
    }
    
    void AIActionController::executeSpecialEffect(AIContext& aiContext, int parentExecID, int seffID, int skillLevel, int skillID, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool useLogicPos, int altLinkEffLinkTick)
    {
        if (aiContext.seffProxy != nullptr) {
#ifndef SERVER_VERIFYING
            int rank = OnePiece::DataManager::getInstance()->getMemRank();


            switch (rank)
            {
            case 0:     // a、所有特效均展示
                break;
            case 1:     // b、策划配置展示部分角色的技能特效
                {
                    SpecialUIEffectExec* effExec = AIDataHelper::getSpecialUIEffectExec(seffID);
                    if (effExec != nullptr && effExec->unShow == 1) { // 判断是否配置 不显示
                        return;
                    }
                }
                break;
            case 2:     // c、allUnShow 字段控制不播放
                // c、除角色大招外的所有技能特效均不展示，只展示角色大招技能特效
                //if (skillID >= 0) { // 判断是否大招 只显示大招
                //    AISkill* skill = AIDataHelper::getSkillData(skillID);
                //    if (skill != nullptr && skill->skillType != 0) // 0 大招  1 小招
                //        return;
                //}
                {
                    SpecialUIEffectExec* effExec = AIDataHelper::getSpecialUIEffectExec(seffID);
                    if (effExec != nullptr && effExec->allUnShow == 1) { // 判断是否配置 不显示
                        return;
                    }
                }
                break;
            default:
                break;
            }
        
#endif // Presentation quality policy is client-only.
            aiContext.seffProxy->executeExec(aiContext, parentExecID, seffID, skillLevel, source, projectile, target, parentTicklen, useLogicPos, altLinkEffLinkTick);
        }

    }

    void AIActionController::executeSpecialEffect(AIContext& aiContext, int parentExecID, int seffID, int skillLevel, int skillID, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool useLogicPos)
    {
        executeSpecialEffect(aiContext, parentExecID, seffID, skillLevel, skillID, source, projectile, target, parentTicklen, useLogicPos, 0);
    }
    
    void AIActionController::executeSpecialExec(AIContext& aiContext, int seffID, int skillID, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen)
    {
        if (aiContext.sexecProxy != nullptr)
            aiContext.sexecProxy->executeExec(aiContext, seffID, skillID, source, projectile, target, parentTicklen);
    }
    
    void AIActionController::executeAnimation(AIContext& aiContext, int animationID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen)
    {
        if (aiContext.animationProxy != nullptr)
            aiContext.animationProxy->executeExec(aiContext, animationID, skillLevel, source, projectile, target, parentTicklen);
    }
    
    void AIActionController::executeMovement(AIContext& aiContext, int movementID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen)
    {
        if (target == nullptr)
            return;

        MoveExec* moveData = AIDataHelper::getMoveExec(movementID);
        if (moveData == nullptr)
            return;
        
		if (moveData->targetTracing == (int)AIContinuousMoveCaster::MoveType::kSpecialHasTarget || moveData->targetTracing == (int)AIContinuousMoveCaster::MoveType::kSpecialNoTarget || moveData->targetTracing == (int)AIContinuousMoveCaster::MoveType::kSpecialSummonBackMove)
		{
            if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_DEBUF_MOVECASTER, 0) > 0)
                return;

            if (moveData->targetTracing == (int)AIContinuousMoveCaster::MoveType::kSpecialSummonBackMove && (!target->isSummon() || HeroActionController::isDeadAction(target->getSummonSource()->currentAction)))
                return;

			AIContinuousMoveCaster* aic = new AIContinuousMoveCaster(movementID, skillID, skillLevel, target, source, aiContext);
			aic->initCaster(aiContext);
			aic->Excute(aiContext);
		}
		else
		{
            if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *source, EffectType::eEFFECT_DEBUF_MOVECASTER, 0) > 0)
                return;

			AIContinuousMoveCaster* aic = new AIContinuousMoveCaster(movementID, skillID, skillLevel, source, target, aiContext);
			aic->initCaster(aiContext);
			aic->Excute(aiContext);
		}
    }
    
    void AIActionController::executeHurt(AIContext& aiContext, int hurtID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, int markID, bool isproj12reachexec)
    {
        if (source == nullptr) return;
        HurtExec* hurtData = AIDataHelper::getHurtExec(hurtID);
        if (hurtData == nullptr) return;
        
        if (target != nullptr)
        {
            AIContinuousHurtCaster* aic = new AIContinuousHurtCaster(hurtID, skillID, skillLevel, source, projectile, target, aiContext, markID);
            aic->Excute(aiContext);
        }
        else if (projectile!=nullptr)
        {
            if (AITargetHelper::isUseNewHelper())
            {
                AITargetContext ctx;
                ctx.source = source;
                ctx.target = target;
                ctx.projectile = projectile;
				if (isproj12reachexec)
				{
					ctx.search_type = projectile->projectileExec->throwParams[4];
					int radius = projectile->projectileExec->throwParams[3];
					ctx.range.x1 = -1 * radius;
					ctx.range.x2 = radius;
					ctx.range.y1 = -1 * radius;
					ctx.range.y2 = radius;
				}
				else
				{
					ctx.search_type = projectile->projectileExec->target;
					ctx.range.x1 = -1 * projectile->projectileExec->redius;
					ctx.range.x2 = projectile->projectileExec->redius;
					ctx.range.y1 = -1 * projectile->projectileExec->redius;
					ctx.range.y2 = projectile->projectileExec->redius;
				}
                ctx.skill_caster = source;
                ctx.skill_id = skillID;
                std::list<CharacterData *> currentTargets;
                AITargetHelper::searchTarget(&ctx, &currentTargets);
                for (auto c : currentTargets)
                {
                    if (c)
                    {
                        AIContinuousHurtCaster* aic = new AIContinuousHurtCaster(hurtID, skillID, skillLevel, source, projectile, c, aiContext, markID);
                        aic->Excute(aiContext);
                    }
                }
            }
            else
            {
				int searchtype = isproj12reachexec ? projectile->projectileExec->throwParams[4] : projectile->projectileExec->target;
				int radius = isproj12reachexec ? projectile->projectileExec->throwParams[3] : projectile->projectileExec->redius;
                auto currentTargets = AIActionController::searchTarget(*source, target, projectile, aiContext, searchtype, -1 * radius, radius, -1 * radius, radius, -1 * radius, radius, nullptr, *source, false, skillID);
                for (std::list<CharacterData*>::iterator iter = currentTargets->begin(); iter != currentTargets->end(); ++iter)
                {
                    if (*iter == nullptr) continue;
                    AIContinuousHurtCaster* aic = new AIContinuousHurtCaster(hurtID, skillID, skillLevel, source, projectile, *iter, aiContext, markID);
                    aic->Excute(aiContext);
                }
            }
        }
    }
    
    void AIActionController::executeProjectile(AIContext& aiContext, int projectileID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen)
    {
        if (source == nullptr) return;
        ProjectileExec* pdata = AIDataHelper::getProjectileExec(projectileID);
        if (pdata == nullptr) return;
        
        AIContinuousProjectileCaster* aic = new AIContinuousProjectileCaster(projectileID, source, aiContext, skillID, skillLevel);
        
        aic->Excute(aiContext);
    }
    
    void AIActionController::executeEffect(AIContext& aiContext, int effectID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, int markID, bool isproj12reachexec)
    {
        if (target == nullptr) return;
        EffectExec* pdata = AIDataHelper::getEffectExec(effectID);
        if (pdata == nullptr) return;
		
		// clair_wang 11.0 add
		if (pdata->optionLevelUp == EEffectOptionLvUpType::eOptionLvUpBySkillID)
		{
			for (auto iter = source->skillList.begin(); iter != source->skillList.end(); ++iter)
			{
				if (pdata->optionLevelUpSkillID == iter->first)
				{
					skillLevel = iter->second;
					break;
				}
			}
		}

        if (target != nullptr)
        {
            bool isCovered = false;
            if (pdata->groupID > 0)
                isCovered = luahelper_effectcaster_is_group_covered(aiContext.getLuaState(), source, target, effectID);
            if (!isCovered) {
                if (pdata->groupID > 0)
                    luahelper_effectcaster_check_cover_group_casters(aiContext.getLuaState(), target, effectID);
                AIContinuousEffectCaster* aic = new AIContinuousEffectCaster(aiContext, effectID, skillID, skillLevel, source, projectile, target, markID);
                aic->Excute(aiContext);
            }
        }
        else if (projectile != nullptr)
        {
            if (AITargetHelper::isUseNewHelper())
            {
                AITargetContext ctx;
                ctx.source = source;
                ctx.target = target;
                ctx.projectile = projectile;
                ctx.skill_caster = source;
                ctx.skill_id = skillID;

				if (isproj12reachexec)
				{
					ctx.search_type = projectile->projectileExec->throwParams[4];
					int radius = projectile->projectileExec->throwParams[3];
					ctx.range.x1 = -1 * radius;
					ctx.range.x2 = radius;
					ctx.range.y1 = -1 * radius;
					ctx.range.y2 = radius;
				}
				else
				{
					ctx.search_type = projectile->projectileExec->target;
					ctx.range.x1 = -1 * projectile->projectileExec->redius;
					ctx.range.x2 = projectile->projectileExec->redius;
					ctx.range.y1 = -1 * projectile->projectileExec->redius;
					ctx.range.y2 = projectile->projectileExec->redius;
				}

                std::list<CharacterData *> currentTargets;
                AITargetHelper::searchTarget(&ctx, &currentTargets);
                for (CharacterData *c : currentTargets)
                {
                    if (!c) continue;
                    bool isCovered = false;
                    if (pdata->groupID > 0)
                        isCovered = luahelper_effectcaster_is_group_covered(aiContext.getLuaState(), source, c, effectID);
                    if (!isCovered) {
                        if (pdata->groupID > 0)
                            luahelper_effectcaster_check_cover_group_casters(aiContext.getLuaState(), c, effectID);
                        AIContinuousEffectCaster* aic = new AIContinuousEffectCaster(aiContext, effectID, skillID, skillLevel, source, projectile, c, markID);
                        aic->Excute(aiContext);
                    }
                }
            }
            else
            {
				int searchtype = isproj12reachexec ? projectile->projectileExec->throwParams[4] : projectile->projectileExec->target;
				int radius = isproj12reachexec ? projectile->projectileExec->throwParams[3] : projectile->projectileExec->redius;
                auto currentTargets = AIActionController::searchTarget(*source, target, projectile, aiContext, searchtype, -1 * radius, radius, -1 * radius, radius, -1 * radius, radius, nullptr, *source, false, skillID);
                for (std::list<CharacterData*>::iterator iter = currentTargets->begin(); iter != currentTargets->end(); iter++)
                {
                    CharacterData *c = *iter;
                    if (!c) continue;
                    bool isCovered = false;
                    if (pdata->groupID > 0)
                        isCovered = luahelper_effectcaster_is_group_covered(aiContext.getLuaState(), source, c, effectID);
                    if (!isCovered) {
                        if (pdata->groupID > 0)
                            luahelper_effectcaster_check_cover_group_casters(aiContext.getLuaState(), c, effectID);
                        AIContinuousEffectCaster* aic = new AIContinuousEffectCaster(aiContext, effectID, skillID, skillLevel, source, projectile, c, markID);
                        aic->Excute(aiContext);
                    }
                }
            }
        }
    }
    
    void AIActionController::executeMark(AIContext& aiContext, int markID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool isproj12reachexec)
    {
        if (!target && !isproj12reachexec) return;
        MarkExec* pdata = AIDataHelper::getMarkExec(markID);
        if (!pdata) return;
        
		if (isproj12reachexec && projectile)
		{
			if (AITargetHelper::isUseNewHelper())
			{
				AITargetContext ctx;
				ctx.source = source;
				ctx.target = target;
				ctx.projectile = projectile;
				ctx.skill_caster = source;
				ctx.skill_id = skillID;

				if (isproj12reachexec)
				{
					ctx.search_type = projectile->projectileExec->throwParams[4];
					int radius = projectile->projectileExec->throwParams[3];
					ctx.range.x1 = -1 * radius;
					ctx.range.x2 = radius;
					ctx.range.y1 = -1 * radius;
					ctx.range.y2 = radius;
				}
				else
				{
					ctx.search_type = projectile->projectileExec->target;
					ctx.range.x1 = -1 * projectile->projectileExec->redius;
					ctx.range.x2 = projectile->projectileExec->redius;
					ctx.range.y1 = -1 * projectile->projectileExec->redius;
					ctx.range.y2 = projectile->projectileExec->redius;
				}

				std::list<CharacterData *> currentTargets;
				AITargetHelper::searchTarget(&ctx, &currentTargets);
				for (auto c : currentTargets)
				{
					if (!c) continue;
					AIContinuousMarkCaster::checkAddMark(aiContext, skillID, skillLevel, source, projectile, c, pdata);
				}
			}
			else
			{
				int searchtype = isproj12reachexec ? projectile->projectileExec->throwParams[4] : projectile->projectileExec->target;
				int radius = isproj12reachexec ? projectile->projectileExec->throwParams[3] : projectile->projectileExec->redius;
				auto currentTargets = AIActionController::searchTarget(*source, target, projectile, aiContext, searchtype, -1 * radius, radius, -1 * radius, radius, -1 * radius, radius, nullptr, *source, false, skillID);
				for (std::list<CharacterData*>::iterator iter = currentTargets->begin(); iter != currentTargets->end(); iter++)
				{
					if (*iter == nullptr) continue;
					AIContinuousMarkCaster::checkAddMark(aiContext, skillID, skillLevel, source, projectile, *iter, pdata);
				}
			}
		}
		else
		{
			if (AITargetHelper::isUseNewHelper())
			{
				AITargetContext ctx;
				ctx.source = target;
				if (target)
					ctx.target = target->target;
				ctx.skill_caster = source;
				ctx.skill_id = skillID;
				ctx.search_type = pdata->target;
				ctx.range.x1 = pdata->range[0];
				ctx.range.x2 = pdata->range[1];
				ctx.range.y1 = pdata->range[2];
				ctx.range.y2 = pdata->range[3];
				std::list<CharacterData *> currentTargets;
				AITargetHelper::searchTarget(&ctx, &currentTargets);
				for (auto c : currentTargets)
				{
					if (!c) continue;
					AIContinuousMarkCaster::checkAddMark(aiContext, skillID, skillLevel, source, projectile, c, pdata);
				}
			}
			else
			{
				auto currentTargets = AIActionController::searchTarget(*target, target->target, nullptr, aiContext, pdata->target, pdata->range[0], pdata->range[1], pdata->range[2], pdata->range[3], pdata->range[2], pdata->range[3], nullptr, *source, false, skillID);
				for (std::list<CharacterData*>::iterator iter = currentTargets->begin(); iter != currentTargets->end(); iter++)
				{
					if (*iter == nullptr) continue;
					AIContinuousMarkCaster::checkAddMark(aiContext, skillID, skillLevel, source, projectile, *iter, pdata);
				}
			}
		}
    }
    
    void AIActionController::executeSummon(AIContext& aiContext, int execID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen)
    {
        if (source) {
			SummonExec *pSummonExec = AIDataHelper::getSummonExec(execID);
			if (pSummonExec)
			{
                /*
				int index = 0;
				for (std::vector<SummonInnerExec>::iterator ite = pSummonExec->execs.begin(); ite != pSummonExec->execs.end(); ++ite)
				{
					SummonProperty *pSummonProperty = AIDataHelper::getSummonProperty((*ite).summonId);
					if (pSummonProperty)
					{
						AIContinuousSummonCaster *summonCaster = new AIContinuousSummonCaster(aiContext, source, target, pSummonExec, skillLevel, index, pSummonProperty);
						summonCaster->Excute(aiContext);
					}


					++index;
				}
                */

                for (int i = 0; i < pSummonExec->execs.size(); i++)
                {
                    if (pSummonExec->execs[i].summonId > 0)
                    {
                        AIContinuousSummonCaster *summon = new AIContinuousSummonCaster(aiContext, source, target, pSummonExec, skillLevel, i);
                        summon->Excute(aiContext);
                    }
                }
			}

        }
    }

    void AIActionController::executeBodyEffect(AIContext& aiContext, int parentExecID, int bodyEffectID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen)
    {
        if (aiContext.bodyEffectProxy)
        {
            if (AIDebufClearBodyExecEffect::checkExcute(target, bodyEffectID))
                return;
            aiContext.bodyEffectProxy->executeExec(aiContext, parentExecID, bodyEffectID, target);
        }
    }

    void AIActionController::executeTransformExec(AIContext& aiContext, int execID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool useLogicPos)
    {
        TransformExec* pExec = AIDataHelper::getTransformExec(execID);
        if (!pExec)
            return;

        auto caster = AIContinuousTransformCaster::create(execID, aiContext, source, target, skillID, skillLevel);
        caster->update(aiContext);
    }
    
    bool AIActionController::isTargetTypeRandom(int searchType)
    {
        auto realSearchType = HeroSkillController::getRealSearchInfo(searchType).realSearchType;

        switch (realSearchType) {
        case TargetEnemyRandom:
        case TargetFriendRandom:
        case TargetRandomAll:
        case TargetRangeRandomFriendOne:
        case TargetRangeRandomEnemyOne:
        case TargetFriendRandomWithoutSearcher:
        case TargetEnemyRandomWithoutSearcher:
        case TargetOneEnemyRandomContinueHurtNotRandomTarget:
		case TargetSelfAttackRangeRandomEnemy:
        case TargetRandomDeadFriend:
        case TargetRandomDeadEnemy:
        case TargetRandomDeadAll:
            return true;
        }

        return false;
    }
    
    std::unique_ptr<std::list<CharacterData*> > AIActionController::searchTarget(CharacterData& source, CharacterData* target, AIProjectile* projectile, AIContext& aiContext, int searchType, int x1, int x2, int y1, int y2, int z1, int z2, std::vector<int> *searchParams, CharacterData& skillCaster, bool isUsingSkill, int theSkillId /*= 0*/, int hurtID/*=0*/, int skillLevel/*=0*/, CharacterDataList *filterList/*= nullptr*/, CharacterData *excludeCharacter/* = nullptr*/)
    {
        return searchTarget(source, target, projectile, aiContext, searchType, x1, x2, y1, y2, z1, z2, searchParams, skillCaster, isUsingSkill, theSkillId, hurtID, skillLevel, filterList, excludeCharacter, false);
    }
    
    std::unique_ptr<std::list<CharacterData*> > AIActionController::searchTarget(CharacterData& source, CharacterData* target, AIProjectile* projectile, AIContext& aiContext, int searchType, int x1, int x2, int y1, int y2, int z1, int z2, std::vector<int> *searchParams, CharacterData& skillCaster, bool isUsingSkill, int theSkillId, int hurtID, int skillLevel, CharacterDataList *filterList, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter)
    {
        if (AIScriptBinder::getInstance()->isSearchTargetByLua(aiContext.getLuaState(), searchType))
        {
            auto targetList = std::unique_ptr<std::list<CharacterData*> >(new std::list<CharacterData*>);
            luahelper_searchtargetwith(aiContext.getLuaState(), source, target, projectile, aiContext, searchType, x1, x2, y1, y2, z1, z2, searchParams, skillCaster, isUsingSkill, theSkillId, hurtID, skillLevel, filterList, excludeCharacter, shouldHpRatioHigherThanExcludeCharacter, targetList.get());
            return targetList;
        }
        else if (AITargetHelper::isUseNewHelper())
        {
            AITargetContext ctx;
            ctx.source = &source;
            ctx.target = target;
            ctx.projectile = projectile;
            ctx.skill_caster = &skillCaster;
            ctx.search_type = searchType;
            ctx.range.x1 = x1;
            ctx.range.x2 = x2;
            ctx.range.y1 = y1;
            ctx.range.y2 = y2;
            ctx.search_params = searchParams;
            ctx.is_using_skill = isUsingSkill;
            ctx.skill_id = theSkillId;
            ctx.hurt_id = hurtID;
            ctx.skill_level = skillLevel;
            ctx.filter_list = filterList;
            ctx.exclude_character = excludeCharacter;
            ctx.should_hp_ratio_higher_than_exclude_character = shouldHpRatioHigherThanExcludeCharacter;
            
            auto targetList = std::unique_ptr<std::list<CharacterData*> >(new std::list<CharacterData*>);
            AITargetHelper::searchTarget(&ctx, targetList.get());
            return targetList;
        }
        
        return searchTargetNative(source, target, projectile, aiContext, searchType, x1, x2, y1, y2, z1, z2, searchParams, skillCaster, isUsingSkill, theSkillId, hurtID, skillLevel, filterList, excludeCharacter, shouldHpRatioHigherThanExcludeCharacter);
    }

    std::unique_ptr<std::list<CharacterData*> > AIActionController::searchTargetNative(CharacterData& source, CharacterData* target, AIProjectile* projectile, AIContext& aiContext, int searchType, int x1, int x2, int y1, int y2, int z1, int z2, std::vector<int> *searchParams, CharacterData& skillCaster, bool isUsingSkill, int theSkillId, int hurtID, int skillLevel, CharacterDataList *filterList, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter)
    {
        auto targetList = std::unique_ptr<std::list<CharacterData*> >(new std::list<CharacterData*>);

        auto realSearchInfo = HeroSkillController::getRealSearchInfo(searchType);
        bool excludeSummon = realSearchInfo.excludeSummon;
        bool ignoreCanbeattackEffect = realSearchInfo.ignoreCanbeattackEffect;

        if (realSearchInfo.realSearchType == TargetCaster)
        {
            targetList->push_back(&source);
            return targetList;
        }
        //else if (realSearchInfo.realSearchType == TargetCurrent)
        //{
        //    if (target && target->canBeAttacked(&source, ignoreCanbeattackEffect))
        //        targetList->push_back(target);
        //    return targetList;
        //}
        else if (realSearchInfo.realSearchType == TargetSkillCaster)
        {
            if (!HeroActionController::isDeadAction(skillCaster.currentAction))
                targetList->push_back(&skillCaster);
            return targetList;
        }

        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, source, eEFFECT_FORCE_ATTACK_SELF, 0) > 0)
        {
            if (theSkillId == source.normalSkill.first)
            {
                targetList->push_back(&source);
                return targetList;
            }
        }

        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, skillCaster, eEFFECT_FORCE_ATTACK_SELF, 0) > 0)
        {
            if (theSkillId == skillCaster.normalSkill.first)
            {
                targetList->push_back(&skillCaster);
                return targetList;
            }
        }

        if (realSearchInfo.realSearchType == TargetCurrent)
        {
            if (target && target->canBeAttacked(&source, ignoreCanbeattackEffect))
                targetList->push_back(target);
            return targetList;
        }

        //auto friendList = source.getCurrentFriendsList(excludeSummon);
        //auto rivalList = source.getCurrentRivalsList(excludeSummon);

        /*
        auto friends = getFilteredOriginList(source.getCurrentFriendsList(), skillCaster, source, realSearchInfo.excludeSummon);
        auto rivals = getFilteredOriginList(source.getCurrentRivalsList(), skillCaster, source, realSearchInfo.excludeSummon);

        auto friendList = friends.get();
        auto rivalList = rivals.get();
        */

        /*
        std::list<CharacterData*> tmpFriends;
        std::list<CharacterData*> tmpRivals;
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, source, eEFFECT_TAUNT_EXTRA, 0) > 0 && source.myID == skillCaster.myID)
        {
        tmpFriends.assign(rivalList->begin(), rivalList->end());
        tmpFriends.push_back(&source);

        for (auto tmpIter = friendList->begin(); tmpIter != friendList->end(); ++tmpIter)
        {
        auto theFriend = *tmpIter;
        if (theFriend->myID == source.myID)
        continue;
        tmpRivals.push_back(theFriend);
        }
        friendList = &tmpFriends;
        rivalList = &tmpRivals;
        }
        */

        /*
        if (source.hasEffect(eEFFECT_TAUNT_EXTRA) && source.myID == skillCaster.myID)
        {
        friends->remove(&source);
        rivals->remove(&source);
        rivals->push_back(&source);

        friendList = rivals.get();
        rivalList = friends.get();
        }
        */

        CharacterData *hpLowerLimitCharacter = nullptr;
        if (excludeCharacter && shouldHpRatioHigherThanExcludeCharacter)
            hpLowerLimitCharacter = excludeCharacter;

        auto friends = getFilteredFriendList(skillCaster, source, realSearchInfo.excludeSummon, realSearchInfo.isonlySummon, ignoreCanbeattackEffect, hpLowerLimitCharacter, realSearchInfo.realSearchType);
        auto rivals = getFilteredRivalList(skillCaster, source, realSearchInfo.excludeSummon, realSearchInfo.isonlySummon, ignoreCanbeattackEffect, hpLowerLimitCharacter, realSearchInfo.realSearchType);
        auto friendList = friends.get();
        auto rivalList = rivals.get();

        //CharacterData* ptarget = NULL;
        //std::list<CharacterData*>::iterator iter;
        //CharacterDataListPtr characterList = NULL;

        if (excludeCharacter)
        {
            friendList->remove(excludeCharacter);
            rivalList->remove(excludeCharacter);
        }

        if (source.isInDuel())
        {
            filterList = nullptr;
        }

        SearchParam param;
        param.searchType = realSearchInfo.realSearchType;
        param.filterList = filterList;
        param.friendList = friendList;
        param.rivalList = rivalList;
        param.targetList = targetList.get();
        param.source = &source;
        param.target = target;
        param.skillCaster = &skillCaster;
        param.aiContext = &aiContext;
        param.projectile = projectile;
        param.searchParams = searchParams;
        param.ignoreCanbeattackEffect = ignoreCanbeattackEffect;
        param.x1 = x1;
        param.x2 = x2;
        param.y1 = y1;
        param.y2 = y2;
        param.z1 = z1;
        param.z2 = z2;
        param.hurtID = hurtID;
        param.skillLevel = skillLevel;

        implSearchTargets(param);


        if (excludeSummon && targetList->size() <= 0 && realSearchInfo.searchAgainIfExcludeSummon)
        {
            auto secondFriends = getFilteredOriginList(source.getCurrentFriendsList(), skillCaster, source, false, realSearchInfo.isonlySummon, ignoreCanbeattackEffect, hpLowerLimitCharacter, realSearchInfo.realSearchType);
            auto secondRivals = getFilteredOriginList(source.getCurrentRivalsList(), skillCaster, source, false, realSearchInfo.isonlySummon, ignoreCanbeattackEffect, hpLowerLimitCharacter, realSearchInfo.realSearchType);
            friendList = secondFriends.get();
            rivalList = secondRivals.get();

            if ((source.hasEffect(eEFFECT_TAUNT_EXTRA) || source.hasTag("summonincharm253")) && source.myID == skillCaster.myID)
            {
                secondFriends->remove(&source);
                secondRivals->remove(&source);
                secondRivals->push_back(&source);

                friendList = secondRivals.get();
                rivalList = secondRivals.get();
            }

            if (excludeCharacter)
            {
                friendList->remove(excludeCharacter);
                rivalList->remove(excludeCharacter);
            }

            param.friendList = friendList;
            param.rivalList = rivalList;
            implSearchTargets(param);
        }

        return targetList;
    }

    void AIActionController::implSearchTargets(const SearchParam &param)
    {
        CharacterDataList *characterList = nullptr;
        CharacterData *ptarget = nullptr;
        CharacterDataList::iterator iter;

        auto filterList = param.filterList;
        auto friendList = param.friendList;
        auto rivalList = param.rivalList;
        auto targetList = param.targetList;
        auto &&source = *param.source;
        auto target = param.target;
        auto &&aiContext = *param.aiContext;
        auto projectile = param.projectile;
        auto ignoreCanbeattackEffect = param.ignoreCanbeattackEffect;
        auto searchParams = param.searchParams;

        switch (param.searchType)
        {
            /*
            case TargetCaster:
            targetList->push_back(&source);
            break;
            case TargetCurrent:
            //if (projectile != nullptr) targetList->push_back(&target);
            //else if (target.getX() > source.getX() + x1 && target.getX() < source.getX() + x2) targetList->push_back(&target);
            if (target && target->canBeAttacked(&source, ignoreCanbeattackEffect))
            targetList->push_back(target);
            break;
            */
        case TargetFriendAll:
            characterList = pickFilterList(filterList, friendList);
            if (characterList)
            {
                for (iter = characterList->begin(); iter != characterList->end(); ++iter)
                {
                    if ((*iter)->canBeAttacked(&source, ignoreCanbeattackEffect))
                        targetList->push_back(*iter);
                }
            }
            break;
        case TargetFriendHPLowest:
            ptarget = getLowestHpCharacter(source, pickFilterList(filterList, friendList), ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
        case TargetFriendRandom:
            ptarget = getRandomCharacter(aiContext, source, pickFilterList(filterList, friendList), ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
        case TargetEnemyAll:
            characterList = pickFilterList(filterList, rivalList);
            if (characterList != NULL)
            {
                for (iter = characterList->begin(); iter != characterList->end(); ++iter)
                {
                    if ((*iter)->canBeAttacked(&source, ignoreCanbeattackEffect))
                        targetList->push_back(*iter);
                }
            }
            break;
        case TargetEnemyHPLowest:
            ptarget = getLowestHpCharacter(source, pickFilterList(filterList, rivalList), ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
        case TargetEnemyRandom:
            ptarget = getRandomCharacter(aiContext, source, pickFilterList(filterList, rivalList), ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
        case TargetCasterEnemyRange:
            addCharactInRange(source, pickFilterList(filterList, rivalList), targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
            break;
        case TargetCasterFriendRange:
            addCharactInRange(source, pickFilterList(filterList, friendList), targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
            break;
        case TargetCurrentEnemyRange:
            addCharactInRange(source, pickFilterList(filterList, rivalList), targetList, target, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
            break;
        case TargetCurrentFriendRange:
            addCharactInRange(source, pickFilterList(filterList, friendList), targetList, target, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
            break;
        case TargetCasterSummon:
            if (&source && !source.isSummon()) {
                characterList = pickFilterList(filterList, source.getCurrentFriendsList());
                if (characterList) {
                    for (iter = characterList->begin(); iter != characterList->end(); ++iter) {
                        auto c = *iter;
                        if (c->canBeAttacked(&source, ignoreCanbeattackEffect) && c->isSummon() && c->getFakeSummonType() == 0 /*12.0*/ && c->getSummonSource()->myID == source.myID)
                            targetList->push_back(c);
                    }
                }
            }
            break;
        case TargetAll:
            if (filterList) {
                addCharactInRange(source, filterList, targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
            }
            else {
                addCharactInRange(source, friendList, targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
                addCharactInRange(source, rivalList, targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
            }
            break;
        case TargetFurthest:
            characterList = pickFilterList(filterList, rivalList);
            ptarget = getFarawayCharacter(source, characterList, ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
            /*
            case TargetSkillCaster:
            if (!HeroActionController::isDeadAction(skillCaster.currentAction)) {
            targetList->push_back(&skillCaster);
            }
            break;
            */
        case TargetRandomAll:
            if (filterList)
                ptarget = getAllRandomCharacter(aiContext, source, characterList, nullptr, ignoreCanbeattackEffect);
            else
                ptarget = getAllRandomCharacter(aiContext, source, rivalList, friendList, ignoreCanbeattackEffect);
            if (ptarget != NULL)
                targetList->push_back(ptarget);
            break;
        case TargetFurthestFriend:
            ptarget = getFarawayCharacter(source, pickFilterList(filterList, friendList), ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
        case TargetNearestFriend:
            ptarget = getNearestCharacter(source, pickFilterList(filterList, friendList), ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
        case TargetNearestEnemy:
            ptarget = getNearestCharacter(source, pickFilterList(filterList, rivalList), ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
        case TargetFriendMale:
            characterList = pickFilterList(filterList, friendList);
            if (characterList != NULL) {
                for (iter = characterList->begin(); iter != characterList->end(); ++iter) {
                    if ((*iter)->getSex() == CharData::Sex_Male && (*iter)->canBeAttacked(&source, ignoreCanbeattackEffect))
                        targetList->push_back(*iter);
                }
            }
            break;
        case TargetEnemyMale:
            characterList = pickFilterList(filterList, rivalList);
            if (characterList != NULL) {
                for (iter = characterList->begin(); iter != characterList->end(); ++iter) {
                    if ((*iter)->getSex() == CharData::Sex_Male && (*iter)->canBeAttacked(&source, ignoreCanbeattackEffect))
                        targetList->push_back(*iter);
                }
            }
            break;
        case TargetRangeRandomFriendOne:
            getRandomCharacterInRange(aiContext, source, pickFilterList(filterList, friendList), targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
            break;
        case TargetRangeRandomEnemyOne:
            getRandomCharacterInRange(aiContext, source, pickFilterList(filterList, rivalList), targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
            break;
        case TargetFriendRandomWithoutSearcher:
            ptarget = getRandomCharacterWithoutTarget(aiContext, source, pickFilterList(filterList, friendList), &source, ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
        case TargetEnemyRandomWithoutSearcher:
            ptarget = getRandomCharacterWithoutTarget(aiContext, source, pickFilterList(filterList, rivalList), &source, ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
        case TargetOneEnemyRandomContinueHurtNotRandomTarget:
            ptarget = getTargetOneEnemyRandomContinueHurtNotRandomTarget(aiContext, &source, pickFilterList(filterList, rivalList), ignoreCanbeattackEffect);
            if (ptarget != NULL)
                targetList->push_back(ptarget);
            break;
        case TargetAllFriendSummon:
            addSummonCharacterInList(source, pickFilterList(filterList, friendList), targetList, ignoreCanbeattackEffect, param.searchType);
            break;
        case TargetAllEnemySummon:
            addSummonCharacterInList(source, pickFilterList(filterList, rivalList), targetList, ignoreCanbeattackEffect, param.searchType);
            break;
        case TargetAllSummon:
            if (filterList) {
                addSummonCharacterInList(source, filterList, targetList, ignoreCanbeattackEffect, param.searchType);
            }
            else {
                addSummonCharacterInList(source, friendList, targetList, ignoreCanbeattackEffect, param.searchType);
                addSummonCharacterInList(source, rivalList, targetList, ignoreCanbeattackEffect, param.searchType);
            }
            break;
        case TargetAllAttrHighest:// = 30 所有敌/友方，某项属性最高的目标
            if (searchParams)
            {
                if (filterList)
                    ptarget = getAttrEstCharacter(source, filterList, nullptr, *searchParams, COMP_HIGHEST, ignoreCanbeattackEffect);
                else
                    ptarget = getAttrEstCharacter(source, friendList, rivalList, *searchParams, COMP_HIGHEST, ignoreCanbeattackEffect);
                if (ptarget)
                    targetList->push_back(ptarget);
            }
            break;
        case TargetFriendAttrHighest:// = 31 友方，某项属性最高的目标
            if (searchParams)
            {
                ptarget = getAttrEstCharacter(source, pickFilterList(filterList, friendList), nullptr, *searchParams, COMP_HIGHEST, ignoreCanbeattackEffect);
                if (ptarget)
                    targetList->push_back(ptarget);
            }
            break;
        case TargetEnemyAttrHighest:// = 32 敌方，某项属性最高的目标
            if (searchParams)
            {
                ptarget = getAttrEstCharacter(source, pickFilterList(filterList, rivalList), nullptr, *searchParams, COMP_HIGHEST, ignoreCanbeattackEffect);
                if (ptarget)
                    targetList->push_back(ptarget);
            }
            break;
        case TargetFriendFemale://= 33  友方的女性
            addCharacterBySex(source, pickFilterList(filterList, friendList), targetList, CharData::Sex_Female, ignoreCanbeattackEffect);
            break;
        case TargetEnemyFemale://= 34 敌方的女性
            addCharacterBySex(source, pickFilterList(filterList, rivalList), targetList, CharData::Sex_Female, ignoreCanbeattackEffect);
            break;
        case TargetAllFemale://= 35 敌我双方的女性
            if (filterList) {
                addCharacterBySex(source, filterList, targetList, CharData::Sex_Female, ignoreCanbeattackEffect);
            }
            else {
                addCharacterBySex(source, pickFilterList(filterList, friendList), targetList, CharData::Sex_Female, ignoreCanbeattackEffect);
                addCharacterBySex(source, pickFilterList(filterList, rivalList), targetList, CharData::Sex_Female, ignoreCanbeattackEffect);
            }
            break;
        case TargetAbsoluteRangeEnemy:// = 36  定点范围内的所有敌军，不随角色转向而转向
            addCharactInAbsoluteRange(source, pickFilterList(filterList, rivalList), targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, false, ignoreCanbeattackEffect);
            break;
        case TargetAbsoluteRangeFriend:// = 37  定点范围内的所有友军，不随角色转向而转向
            addCharactInAbsoluteRange(source, pickFilterList(filterList, friendList), targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, false, ignoreCanbeattackEffect);
            break;
        case TargetAbsoluteRangeAll:// = 38  定点范围内的所有敌/友方，不随角色转向而转向
            if (filterList) {
                addCharactInAbsoluteRange(source, filterList, targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, false, ignoreCanbeattackEffect);
            }
            else {
                addCharactInAbsoluteRange(source, rivalList, targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, false, ignoreCanbeattackEffect);
                addCharactInAbsoluteRange(source, friendList, targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, false, ignoreCanbeattackEffect);
            }
            break;
        case TargetAbsoluteRangeAllWithoutSearcher:// = 39  定点范围内的所有敌/友方(不包含自己），不随角色转向而转向
            if (filterList) {
                addCharactInAbsoluteRange(source, filterList, targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, true, ignoreCanbeattackEffect);
            }
            else {
                addCharactInAbsoluteRange(source, rivalList, targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, true, ignoreCanbeattackEffect);
                addCharactInAbsoluteRange(source, friendList, targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, true, ignoreCanbeattackEffect);
            }
            break;
        case TargetSomeoneCampAndRoleWithId:
            if (searchParams)
                addSomeoneCampAndRoleWithId(filterList, &source, *searchParams, targetList, friendList, rivalList, ignoreCanbeattackEffect);
            break;
        case TargetSelfAttackRangeRandomEnemy:
            if (source.mAttackConfirmedTarget && source.target && source.target->canBeAttacked(&source, ignoreCanbeattackEffect))
            {
                targetList->push_back(source.target);
            }
            else
            {
                ptarget = getRandomCharacter(aiContext, source, pickFilterList(filterList, rivalList), ignoreCanbeattackEffect);
                if (ptarget != NULL)
                {
                    targetList->push_back(ptarget);
                }
            }
            break;
        case TargetByAttacker:
            if (source.attacker && source.attacker->canBeAttacked(&source, ignoreCanbeattackEffect))
                targetList->push_back(source.attacker);
            else if (target && target->canBeAttacked(&source, ignoreCanbeattackEffect))
                targetList->push_back(target);
            break;
        case TargetScreenAbsolutePointEnemy:
            getScreenAbsolutePointTargetLists(pickFilterList(filterList, rivalList), targetList, source, aiContext, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
            break;
        case TargetScreenAbsolutePointFriend:
            getScreenAbsolutePointTargetLists(pickFilterList(filterList, friendList), targetList, source, aiContext, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
            break;
        case TargetAllAttrLowest:
            if (searchParams)
            {
                if (filterList)
                    ptarget = getAttrEstCharacter(source, filterList, nullptr, *searchParams, COMP_LOWEST, ignoreCanbeattackEffect);
                else
                    ptarget = getAttrEstCharacter(source, friendList, rivalList, *searchParams, COMP_LOWEST, ignoreCanbeattackEffect);
                if (ptarget)
                    targetList->push_back(ptarget);
            }

            break;
        case TargetFriendAttrLowest:
            if (searchParams)
            {
                ptarget = getAttrEstCharacter(source, pickFilterList(filterList, friendList), nullptr, *searchParams, COMP_LOWEST, ignoreCanbeattackEffect);
                if (ptarget)
                    targetList->push_back(ptarget);
            }
            break;
        case TargetEnemyAttrLowest:
            if (searchParams)
            {
                ptarget = getAttrEstCharacter(source, pickFilterList(filterList, rivalList), nullptr, *searchParams, COMP_LOWEST, ignoreCanbeattackEffect);
                if (ptarget)
                    targetList->push_back(ptarget);
            }
            break;
        case TargetSummonBySummoner:
            if (source.isSummon() && source.getSummonSource() && source.getSummonSource()->canBeAttacked(&source, ignoreCanbeattackEffect))
            {
                targetList->push_back(source.getSummonSource());
            }
            break;
        case TargetAccordRingofLightFriend:
            if (searchParams)
            {
                addCharacterHasLabels(source, pickFilterList(filterList, friendList), targetList, *searchParams, ignoreCanbeattackEffect);
            }
            break;
        case TargetEnemyMaxHurt:
            addMaxHurtTarget(aiContext, param.skillCaster, projectile, pickFilterList(filterList, rivalList), targetList, param.hurtID, param.skillLevel, ignoreCanbeattackEffect);
            break;
        case TargetAllEnemyInHPRange:
            addAllCharacterInHPRange(source, pickFilterList(filterList, rivalList), targetList, param.x1, param.x2, ignoreCanbeattackEffect);
            break;
        case TargetTraversal:
            // MARK: 遍历目标的参数
            // param.x1 - 换目标的时间间隔, param.x2 - 重置遍历的时间
            addCharacterTraversal(source, pickFilterList(filterList, rivalList), targetList, param.x1, param.x2, ignoreCanbeattackEffect);
            break;
        case TargetHasMark:
            if (searchParams && searchParams->size() >= 3)
            {
                //参数[a, b, c]
                //    a--标记ID
                //    b--我方还是敌方 0-所有 1-我方 2-敌方
                //    c--没有目标时是否使用当前目标（0-否，1 - 是，目标选为当前目标）
                int theSide = (*searchParams)[1];
                if (filterList) {
                    addAllCharacterHasMark(source, filterList, targetList, (*searchParams)[0], ignoreCanbeattackEffect);
                }
                else {
                    if (theSide == 0 || theSide == 1)
                        addAllCharacterHasMark(source, friendList, targetList, (*searchParams)[0], ignoreCanbeattackEffect);

                    if (theSide == 0 || theSide == 2)
                        addAllCharacterHasMark(source, rivalList, targetList, (*searchParams)[0], ignoreCanbeattackEffect);
                }

                if (targetList->size() <= 0 && (*searchParams)[2] == 1 && source.target && source.target->canBeAttacked(&source, ignoreCanbeattackEffect))
                    targetList->push_back(source.target);
            }
            break;
        case TargetSelfSummonInRange:
            addSelfSummonInRange(aiContext, *param.skillCaster, targetList, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
            break;
        case TargetHasLabelEnemy:
            if (searchParams)
            {
                addCharacterHasLabels(source, pickFilterList(filterList, rivalList), targetList, *searchParams, ignoreCanbeattackEffect);
            }
            break;
        case TargetHasLabelAll:
            if (searchParams)
            {
                addCharacterHasLabels(source, pickFilterList(filterList, friendList), targetList, *searchParams, ignoreCanbeattackEffect);
                addCharacterHasLabels(source, pickFilterList(filterList, rivalList), targetList, *searchParams, ignoreCanbeattackEffect);
            }
            break;
        case TargetRandomDeadFriend:
            ptarget = getRandomDeadCharacter(aiContext, pickFilterList(filterList, friendList));
            if (ptarget)
            {
                targetList->push_back(ptarget);
            }
            break;
        case TargetRandomDeadEnemy:
            ptarget = getRandomDeadCharacter(aiContext, pickFilterList(filterList, rivalList));
            if (ptarget)
            {
                targetList->push_back(ptarget);
            }
            break;
        case TargetRandomDeadAll:
            if (filterList)
                ptarget = getRandomDeadCharacter(aiContext, filterList);
            else
                ptarget = getAllRandomDeadCharacter(aiContext, friendList, rivalList);
            if (ptarget)
                targetList->push_back(ptarget);
            break;
        case TargetDeadAllFriend:
            addAllDeadCharacter(pickFilterList(filterList, friendList), targetList);
            break;
        case TargetDeadAllEnemy:
            addAllDeadCharacter(pickFilterList(filterList, rivalList), targetList);
            break;
        case TargetDeadAll:
            if (filterList)
            {
                addAllDeadCharacter(filterList, targetList);
            }
            else
            {
                addAllDeadCharacter(friendList, targetList);
                addAllDeadCharacter(rivalList, targetList);
            }
            break;
        case TargetFriendHPHighest:
            ptarget = getHighestHpCharacter(source, pickFilterList(filterList, friendList), ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
        case TargetEnemyHPHighest:
            ptarget = getHighestHpCharacter(source, pickFilterList(filterList, rivalList), ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
        case 64: // 敌军之中战力最高的目标
            ptarget = getHighestCombatPowerCharacter(source, pickFilterList(filterList, rivalList), nullptr, ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
        case 65: // 友军之中战力最高的目标
            ptarget = getHighestCombatPowerCharacter(source, pickFilterList(filterList, friendList), nullptr, ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
        case 66: // 所有人中战力最高的目标
            ptarget = getHighestCombatPowerCharacter(source, pickFilterList(filterList, friendList), pickFilterList(filterList, rivalList), ignoreCanbeattackEffect);
            if (ptarget != NULL) targetList->push_back(ptarget);
            break;
        case 67: // 效果286中记录的对自身造成伤害最高的敌方单位
            ptarget = luahelper_innereffect_288_get_first_mosthpdelta_attacker(AIContext::getInstance()->getLuaState(), &source);
            if (ptarget != NULL)
                targetList->push_back(ptarget);
            else if (source.target)
                targetList->push_back(source.target);
            break;
        default:
            break;
        }
    }
	
    CharacterData* AIActionController::getFarawayCharacter(CharacterData& source, std::list<CharacterData*> *list, bool ignoreCanbeattackEffect)
    {
        if (list == NULL) {
            return NULL;
        }
        else
        {
            std::list<CharacterData*>::iterator iter;
            CharacterData *target = NULL;
            int distance = -1;
            for (iter = list->begin(); iter != list->end(); ++iter)
            {
                if (!(*iter)->canBeAttacked(&source, ignoreCanbeattackEffect))
                    continue;

                int dist = HeroActionController::distanceToCharacter(&source, *iter);
                if (dist > distance) {
                    distance = dist;
                    target = (*iter);
                }
            }
            return target;
        }
    }
    
    CharacterData* AIActionController::getNearestCharacter(CharacterData& source, std::list<CharacterData*> *list, bool ignoreCanbeattackEffect)
    {
        if (list == NULL) {
            return NULL;
        }
        else
        {
            std::list<CharacterData*>::iterator iter;
            CharacterData *target = NULL;
            int distance = 100000000;
            for (iter = list->begin(); iter != list->end(); ++iter)
            {
                if (!(*iter)->canBeAttacked(&source, ignoreCanbeattackEffect))
                    continue;
                
                int dist = HeroActionController::distanceToCharacter(&source, *iter);
                if (dist < distance) {
                    distance = dist;
                    target = (*iter);
                }
            }
            return target;
        }

    }
    
    CharacterData* AIActionController::getLowestHpCharacter(CharacterData& source, std::list<CharacterData*> *list, bool ignoreCanbeattackEffect)
    {
        if (list == NULL) {
            return NULL;
        }
        else
        {
            std::list<CharacterData*>::iterator iter;
            float HP = 100000, hptop=1;
            CharacterData *target = NULL;
            for (iter = list->begin(); iter != list->end(); ++iter)
            {
                hptop = (*iter)->getHPTop();
                if (hptop < 1) hptop = 1;
                if ((*iter)->getHP() / hptop < HP && (*iter)->canBeAttacked(&source, ignoreCanbeattackEffect))
                {
                    target = (*iter);
                    HP = (*iter)->getHP() / hptop;
                }
            }
            return target;
        }
    }

    CharacterData* AIActionController::getHighestHpCharacter(CharacterData& source, std::list<CharacterData*> *list, bool ignoreCanbeattackEffect)
    {
        if (list != nullptr)
        {
            std::list<CharacterData*>::iterator iter;
            float HP = 0, hptop = 1, tmpHP = 0;
            CharacterData *target = nullptr;
            for (auto aCharacter : *list)
            {
                hptop = aCharacter->getHPTop();
                if (hptop < 1)
                    hptop = 1;
                tmpHP = aCharacter->getHP() / hptop;
                if (tmpHP > HP && aCharacter->canBeAttacked(&source, ignoreCanbeattackEffect))
                {
                    target = aCharacter;
                    HP = tmpHP;
                }
            }
            return target;
        }

        return nullptr;
    }
    
    CharacterData* AIActionController::getRandomCharacter(AIContext& aiContext, CharacterData& source, std::list<CharacterData*> *list, bool ignoreCanbeattackEffect)
    {
        if (list == NULL) {
            return NULL;
        }
        else
        {
            CharacterData *target = NULL;
            std::vector<CharacterData*> tmpVec;
            for (std::list<CharacterData*>::iterator iter = list->begin(); iter != list->end(); iter++)
            {
                if ((*iter)->canBeAttacked(&source, ignoreCanbeattackEffect))
                    tmpVec.push_back(*iter);
            }
            if (tmpVec.size() > 0)
            {
                int inde = aiContext.localRandom->nextInt(0, tmpVec.size());
                target = tmpVec.at(inde);
            }
            return target;
        }
    }

    CharacterData* AIActionController::getRandomCharacter(AIContext& aiContext, CharacterData& source, std::list<CharacterData*> *list, bool ignoreCanbeattackEffect, bool excludeSummon)
    {
        if (list == nullptr)
        {
            return nullptr;
        }
        else
        {
            CharacterData *target = nullptr;
            std::vector<CharacterData*> tmpVec;
            for (std::list<CharacterData*>::iterator iter = list->begin(); iter != list->end(); iter++)
            {
                auto aCharacter = *iter;
                if (aCharacter->canBeAttacked(&source, ignoreCanbeattackEffect))
                {
                    if (excludeSummon && aCharacter->isSummon())
                    {
                    }
                    else
                    {
                        tmpVec.push_back(*iter);
                    }
                }
            }
            if (tmpVec.size() > 0)
            {
                int inde = aiContext.localRandom->nextInt(0, tmpVec.size());
                target = tmpVec.at(inde);
            }
            return target;
        }
    }
    
    CharacterData* AIActionController::getAllRandomCharacter(AIContext& aiContext, CharacterData& source, std::list<CharacterData*> *list1, std::list<CharacterData*> *list2, bool ignoreCanbeattackEffect)
    {
        if (list1 == NULL && list2 == NULL) {
            return NULL;
        }
        else
        {
            CharacterData *target = NULL;
            std::vector<CharacterData*> tmpVec;
            
            if(list1 != NULL)
            {
                for (std::list<CharacterData*>::iterator iter = list1->begin(); iter != list1->end(); ++iter)
                {
                    if ((*iter)->canBeAttacked(&source, ignoreCanbeattackEffect))
                        tmpVec.push_back(*iter);
                }
            }
            
            if (list2 != NULL) {
                for (std::list<CharacterData*>::iterator iter = list2->begin(); iter != list2->end(); ++iter)
                {
                    if ((*iter)->canBeAttacked(&source, ignoreCanbeattackEffect))
                        tmpVec.push_back(*iter);
                }
            }
            
            
            
            if (tmpVec.size() > 0)
            {
                int inde = aiContext.localRandom->nextInt(0, tmpVec.size());
                target = tmpVec.at(inde);
            }
            return target;
        }
    }
    
    void AIActionController::addCharactInRange(CharacterData& source, std::list<CharacterData*> *characterList, CharacterDataList *outPutList, CharacterData* origin, AIProjectile* projectile, short x1, short x2, short y1, short y2, short z1, short z2, bool ignoreCanbeattackEffect)
    {
        if (!origin && !projectile)
            return;

        if (characterList != NULL) {
            int x11 = projectile == nullptr ? origin->getX() + x1 : projectile->currentX + x1;
            int x22 = projectile == nullptr ? origin->getX() + x2 : projectile->currentX + x2;
            if (origin->direction == -1)
            {
                x11 = projectile == nullptr ? origin->getX() - x2 : projectile->currentX - x2;
                x22 = projectile == nullptr ? origin->getX() - x1 : projectile->currentX - x1;
            }
            int y11 = projectile == nullptr ? origin->getY() + y1 : projectile->currentY + y1;
            int y22 = projectile == nullptr ? origin->getY() + y2 : projectile->currentY + y2;
            std::list<CharacterData*>::iterator iter = characterList->begin();
            for (; iter != characterList->end(); iter++)
            {
                if (!(*iter)->canBeAttacked(&source, ignoreCanbeattackEffect))
                    continue;
                
                if (((*iter)->getX() >= x11) && ((*iter)->getX() <= x22) &&
                    ((*iter)->getY() >= y11) && ((*iter)->getY() <= y22)
                    )
                {
                    outPutList->push_back(*iter);
                }
            }
        }
    }

    void AIActionController::getRandomCharacterInRange(AIContext& aiContext, CharacterData& source, std::list<CharacterData*> *characterList, CharacterDataList *outPutList, CharacterData* origin, AIProjectile* projectile, short x1, short x2, short y1, short y2, short z1, short z2, bool ignoreCanbeattackEffect)
	{
		if (characterList != NULL) {
			int x11 = projectile == nullptr ? origin->getX() + x1 : projectile->currentX + x1;
			int x22 = projectile == nullptr ? origin->getX() + x2 : projectile->currentX + x2;
			if (origin->direction == -1)
			{
				x11 = projectile == nullptr ? origin->getX() - x2 : projectile->currentX - x2;
				x22 = projectile == nullptr ? origin->getX() - x1 : projectile->currentX - x1;
			}
			int y11 = projectile == nullptr ? origin->getY() + y1 : projectile->currentY + y1;
			int y22 = projectile == nullptr ? origin->getY() + y2 : projectile->currentY + y2;

			std::vector<CharacterData*> tmpVec;


			std::list<CharacterData*>::iterator iter = characterList->begin();
			for (; iter != characterList->end(); iter++)
			{
                if (!(*iter)->canBeAttacked(&source, ignoreCanbeattackEffect))
                    continue;

				if (((*iter)->getX() >= x11) && ((*iter)->getX() <= x22) &&
					((*iter)->getY() >= y11) && ((*iter)->getY() <= y22)
					)
				{
					tmpVec.push_back(*iter);
				}
			}


			if (tmpVec.size() > 0)
			{
				int inde = aiContext.localRandom->nextInt(0, tmpVec.size());
				outPutList->push_back( tmpVec.at(inde) );
			}

		}
	}
    
    CharacterData* AIActionController::getRandomCharacterAboveHPLowerLimit(AIContext& aiContext, CharacterData& source, std::list<CharacterData*> *list, float lowerHPRatioLimit, bool ignoreCanbeattackEffect)
    {
        if (!list)
            return nullptr;
        
        std::list<CharacterData*> filteredList;
        for (auto iter = list->begin(); iter != list->end(); ++iter)
        {
            auto hero = *iter;
            if (hero->canBeAttacked(&source, ignoreCanbeattackEffect) && CharacterFunc::GetCharacterHPRatio(*hero) > lowerHPRatioLimit)
            {
                filteredList.push_back(hero);
            }
        }

        return getRandomCharacter(aiContext, source, &filteredList, ignoreCanbeattackEffect);
    }

    CharacterData* AIActionController::getRandomCharacterWithoutTarget(AIContext& aiContext, CharacterData& source, std::list<CharacterData*> *list, CharacterData* target, bool ignoreCanbeattackEffect)
    {
        if (!list)
            return nullptr;

        std::list<CharacterData*> filteredList;
        for (auto iter = list->begin(); iter != list->end(); ++iter)
        {
            auto hero = *iter;
            if (hero->myID != target->myID)
                filteredList.push_back(hero);
        }

        return getRandomCharacter(aiContext, source, &filteredList, ignoreCanbeattackEffect);
    }
    
    CharacterData* AIActionController::getHighestCombatPowerCharacter(CharacterData& source, std::list<CharacterData* > *list1, std::list<CharacterData* > *list2, bool ignoreCanbeattackEffect)
    {
        CharacterData *t = nullptr;
        if (list1) {
            for (auto c : *list1) {
                if (!HeroActionController::isDeadAction(c->currentAction) && c->canBeAttacked(&source, ignoreCanbeattackEffect)) {
                    if (!t || c->combatPower > t->combatPower)
                        t = c;
                }
            }
        }

        if (list2) {
            for (auto c : *list2) {
                if (!HeroActionController::isDeadAction(c->currentAction) && c->canBeAttacked(&source, ignoreCanbeattackEffect)) {
                    if (!t || c->combatPower > t->combatPower)
                        t = c;
                }
            }
        }

        return t;
    }

    void AIActionController::addSummonCharacterInList(CharacterData& source, std::list<CharacterData*> *characterList, CharacterDataList *outPutList, bool ignoreCanbeattackEffect, int searchType)
    {
        if (!characterList)
            return;

        for (auto iter = characterList->begin(); iter != characterList->end(); ++iter)
        {
            auto c = *iter;
            //if (c->isSummon() && c->getFakeSummonType() == 0 /*12.0*/ && c->canBeAttacked(&source, ignoreCanbeattackEffect))
            //    outPutList->push_back(c);
            
            if (searchType == TargetAllFriendSummon) {
                if (c->isSummon() && c->getFakeSummonType() == 0 /*12.0*/ && c->canBeAttacked(&source, ignoreCanbeattackEffect))
                    outPutList->push_back(c);
            } else {
                if (c->hasTag("fakesummoneff_summon"))
                    outPutList->push_back(c);
                else if (c->isSummon()) {
                    if (c->hasTag("fakesummoneff_notsummon"))
                        continue;
                    if (c->getFakeSummonType() == 0 /*12.0*/ && c->canBeAttacked(&source, ignoreCanbeattackEffect))
                        outPutList->push_back(c);
                }
            }
        }
    }

    //void AIActionController::addAllAttrHighestCharacterInList(CharacterData& source, std::list<CharacterData*>* theInFriendList, std::list<CharacterData*>* theInRivalList, int theAttrIndex, CharacterDataListUniPtr &theOutList, bool ignoreCanbeattackEffect)
    //{
    //    if (theInFriendList == nullptr && theInRivalList == nullptr)
    //    {
    //        return;
    //    }

    //    if (theAttrIndex <= 0 || theAttrIndex >= ATT_NUM)
    //        return;

    //    CharacterData* pCharacterData = nullptr;
    //    auto findHighest = [&pCharacterData, theAttrIndex, &source, ignoreCanbeattackEffect](std::list<CharacterData*>* characters){
    //        if (!characters)
    //            return;
    //        for (auto aCharacter : *characters) {
    //            if (!aCharacter->canBeAttacked(&source, ignoreCanbeattackEffect))
    //                continue;
    //            if (!pCharacterData || pCharacterData->attributes[theAttrIndex] < aCharacter->attributes[theAttrIndex])
    //                pCharacterData = aCharacter;
    //        }
    //    };

    //    findHighest(theInFriendList);
    //    findHighest(theInRivalList);

    //    if (pCharacterData != nullptr)
    //        theOutList->push_back(pCharacterData);
    //}

    //void AIActionController::addAllAttrHighestCharacterInList(CharacterData& source, std::list<CharacterData*>* theInFriendList, std::list<CharacterData*>* theInRivalList, std::vector<int> &theAttrs, CharacterDataListUniPtr &theOutList, bool ignoreCanbeattackEffect)
    //{
    //    if (!theInFriendList && !theInRivalList)
    //        return;

    //    int theHighestAttrValue = 0;
    //    CharacterData *theHighestCharacter = nullptr;
    //    auto findHighestFunc = [&theHighestCharacter, &theHighestAttrValue, &source, ignoreCanbeattackEffect](std::list<CharacterData*>* characters, int theAttr){
    //        if (!characters)
    //            return;
    //        for (auto pCharacter : *characters) {
    //            if (!pCharacter->canBeAttacked(&source, ignoreCanbeattackEffect))
    //                continue;
    //            if (!theHighestCharacter || theHighestAttrValue < pCharacter->attributes[theAttr]){
    //                theHighestCharacter = pCharacter;
    //                theHighestAttrValue = pCharacter->attributes[theAttr];
    //            }
    //        }
    //    };

    //    for (int i = 0; i < theAttrs.size(); i++)
    //    {
    //        auto attr = theAttrs[i];
    //        if (attr <= 0 || attr >= ATT_NUM)
    //            continue;

    //        findHighestFunc(theInFriendList, attr);
    //        findHighestFunc(theInRivalList, attr);
    //    }

    //    if (theHighestCharacter)
    //        theOutList->push_back(theHighestCharacter);
    //}

    void AIActionController::addCharactInAbsoluteRange(CharacterData& source, std::list<CharacterData*> *characterList, CharacterDataList *outPutList, CharacterData* target, AIProjectile* projectile, short x1, short x2, short y1, short y2, short z1, short z2, bool bIncludeOwn /*= false*/, bool ignoreCanbeattackEffect)
	{
		if (characterList != NULL) {
			
			int x11 = 0;
			int x22 = 0;
			int y11 = 0;
			int y22 = 0;

			if (target != nullptr)
			{
#if BATTLE_LOG_TEST
				CCLOG("clair_wang target[%d]=(%d, %d) ", target->attributes[0], target->getX(), target->getY());
#endif
				if (target->mStationaryDirection != 0 && target->mStationaryX != 0 && target->mStationaryY != 0)
				{
					if (target->mStationaryDirection == -1)
					{
						x11 = projectile == nullptr ? target->mStationaryX - x2 : projectile->currentX - x2;
						x22 = projectile == nullptr ? target->mStationaryX - x1 : projectile->currentX - x1;
					}
					else if (target->mStationaryDirection == 1)
					{
						x11 = projectile == nullptr ? target->mStationaryX + x1 : projectile->currentX + x1;
						x22 = projectile == nullptr ? target->mStationaryX + x2 : projectile->currentX + x2;
					}
					
					y11 = projectile == nullptr ? target->mStationaryY + y1 : projectile->currentY + y1;
					y22 = projectile == nullptr ? target->mStationaryY + y2 : projectile->currentY + y2;
				}
			}
#if BATTLE_LOG_TEST
			CCLOG("clair_wang addCharactInAbsoluteRange x(%d, %d)-- y(%d, %d)", x11, x22, y11, y22);
#endif
			std::list<CharacterData*>::iterator iter = characterList->begin();
			for (; iter != characterList->end(); iter++)
			{
                if (!(*iter)->canBeAttacked(&source, ignoreCanbeattackEffect))
                    continue;
#if BATTLE_LOG_TEST
				CCLOG("clair_wang addCharactInAbsoluteRange iterID[%d]=(%d, %d)", (*iter)->attributes[0], (*iter)->getX(), (*iter)->getY());
#endif
				if (((*iter)->getX() >= x11) && ((*iter)->getX() <= x22) &&
					((*iter)->getY() >= y11) && ((*iter)->getY() <= y22)
					)
				{
					if (!bIncludeOwn)
					{
						outPutList->push_back(*iter);
					}
					else
					{
						if ((*iter)->myID != target->myID)
						{
							outPutList->push_back(*iter);
						}
					}
				}
			}
		}
	}

	/** clair_wang 获取屏幕绝对坐标范围内的目标 **/
    void AIActionController::getScreenAbsolutePointTargetLists(std::list<CharacterData*> *characterList, CharacterDataList *outPutList, CharacterData& source, AIContext& aiContext, short x1, short x2, short y1, short y2, short z1, short z2, bool ignoreCanbeattackEffect)
	{
		int mDirection = 0;

		float mScreenCenterX;
		float mScreenCenterY;

		float x11, x22;
		float y11, y22;

		std::list<CharacterData*>::iterator iter;

		if (source.teamSide == TeamSide::kOur)
		{
			mDirection = 1;
		}
		else if (source.teamSide == TeamSide::kEnemy)
		{
			mDirection = -1;
		}

		mScreenCenterX = aiContext.battleFieldCenterX;
		mScreenCenterY = (AIContext::getInstance()->getDeltaMaxY() - AIContext::getInstance()->getDeltaMinY()) / 2.0f;

		x11 = mScreenCenterX + (mDirection * x1);
		x22 = mScreenCenterX + (mDirection * x2);

		y11 = mScreenCenterY + y1;
		y22 = mScreenCenterY + y2;

		float sX = x11, sY = y11;
		if (x11 > x22)
		{
			x11 = x22;
			x22 = sX;
		}
		if (y11 > y22)
		{
			y11 = y22;
			y22 = sY;
		}

		if (characterList != NULL)
		{
			for (iter = characterList->begin(); iter != characterList->end(); ++iter)
			{
                if (!(*iter)->canBeAttacked(&source, ignoreCanbeattackEffect)) continue;

				if (((*iter)->getX() >= x11) && ((*iter)->getX() <= x22) &&
					((*iter)->getY() >= y11) && ((*iter)->getY() <= y22)
					)
				{
					outPutList->push_back(*iter);
				}
			}
		}

	}

    //CharacterData *AIActionController::getAttrCompareCharacter(CharacterData& source, std::list<CharacterData*> *list, int attrIndex, CompareType compType, bool ignoreCanbeattackEffect) {
    //    if (!list || attrIndex <= 0 || attrIndex >= ATT_NUM)
    //        return nullptr;

    //    CharacterData *pFoundCharacter = nullptr;
    //    for (auto iter = list->begin(); iter != list->end(); ++iter) {
    //        auto pCharacter = *iter;
    //        if (!pCharacter->canBeAttacked(&source, ignoreCanbeattackEffect))
    //            continue;
    //        if (!pFoundCharacter) {
    //            pFoundCharacter = pCharacter;
    //        }
    //        else {
    //            auto originValue = pFoundCharacter->attributes[attrIndex];
    //            auto compareValue = pCharacter->attributes[attrIndex];
    //            if (COMP_HIGHEST == compType) {
    //                if (compareValue > originValue)
    //                    pFoundCharacter = pCharacter;
    //            }
    //            else if (COMP_LOWEST == compType) {
    //                if (compareValue < originValue)
    //                    pFoundCharacter = pCharacter;
    //            }
    //        }
    //    }

    //    return pFoundCharacter;
    //}

    //CharacterData *AIActionController::getAttrCompareCharacter(CharacterData& source, std::list<CharacterData*> *list1, std::list<CharacterData*> *list2, int attrIndex, CompareType compType, bool ignoreCanbeattackEffect)
    //{
    //    if (attrIndex <= 0 || attrIndex >= ATT_NUM)
    //        return nullptr;

    //    CharacterData *pFoundCharacter = getAttrCompareCharacter(source, list1, attrIndex, compType, ignoreCanbeattackEffect);
    //    auto pFound2 = getAttrCompareCharacter(source, list2, attrIndex, compType, ignoreCanbeattackEffect);
    //    if (!pFoundCharacter) {
    //        pFoundCharacter = pFound2;
    //    }
    //    else if (pFound2) {
    //        auto originValue = pFoundCharacter->attributes[attrIndex];
    //        auto compareValue = pFound2->attributes[attrIndex];
    //        if (COMP_HIGHEST == compType) {
    //            if (compareValue > originValue)
    //                pFoundCharacter = pFound2;
    //        }
    //        else if (COMP_LOWEST == compType) {
    //            if (compareValue < originValue)
    //                pFoundCharacter = pFound2;
    //        }
    //    }
    //    return pFoundCharacter;
    //}

    CharacterData *AIActionController::getAttrEstCharacter(CharacterData& source, std::list<CharacterData*> *list1, std::list<CharacterData*> *list2, std::vector<int> &theAttrs, CompareType compType, bool ignoreCanbeattackEffect)
    {
        int theAttrValue = 0;
        CharacterData *theCharacter = nullptr;
        auto findHighestFunc = [&theCharacter, &theAttrValue, &source, compType, ignoreCanbeattackEffect](std::list<CharacterData*>* characters, int theAttr){
            if (!characters)
                return;
            for (auto pCharacter : *characters) {
                if (!pCharacter->canBeAttacked(&source, ignoreCanbeattackEffect))
                    continue;
                bool isMatch = false;
                switch (compType) {
                case COMP_HIGHEST:
                    isMatch = theAttrValue < pCharacter->attributes[theAttr];
                    break;
                case COMP_LOWEST:
                    isMatch = theAttrValue > pCharacter->attributes[theAttr];
                    break;
                default:
                    break;
                }
                if (!theCharacter || isMatch) {
                    theCharacter = pCharacter;
                    theAttrValue = pCharacter->attributes[theAttr];
                }
            }
        };

        for (int i = 0; i < theAttrs.size(); i++)
        {
            auto attr = theAttrs[i];
            if (attr <= 0 || attr >= ATT_NUM)
                continue;

            findHighestFunc(list1, attr);
            findHighestFunc(list2, attr);
        }

        return theCharacter;
    }

    std::unique_ptr<std::list<CharacterData*>> AIActionController::getFilteredFriendList(CharacterData &skillCaster, CharacterData &source, bool excludeSummon, bool onlySummon, bool ignoreCanbeattackEffect, CharacterData *lowerHpLimitCharacter, int realSearchType)
    {
        std::unique_ptr<std::list<CharacterData*>> retList = nullptr;

        if ((source.hasEffect(eEFFECT_TAUNT_EXTRA) || source.hasTag("summonincharm253")) && source.myID == skillCaster.myID)
        {
            retList = getFilteredOriginList(source.getCurrentRivalsList(), skillCaster, source, excludeSummon, onlySummon, ignoreCanbeattackEffect, lowerHpLimitCharacter, realSearchType);
            retList->remove(&source);
            retList->push_back(&source);
        }
        else
        {
            retList = getFilteredOriginList(source.getCurrentFriendsList(), skillCaster, source, excludeSummon, onlySummon, ignoreCanbeattackEffect, lowerHpLimitCharacter, realSearchType);
        }

        return retList;
    }

    std::unique_ptr<std::list<CharacterData*>> AIActionController::getFilteredRivalList(CharacterData &skillCaster, CharacterData &source, bool excludeSummon, bool onlySummon, bool ignoreCanbeattackEffect, CharacterData *lowerHpLimitCharacter, int realSearchType)
    {
        std::unique_ptr<std::list<CharacterData*>> retList = nullptr;

        if ((source.hasEffect(eEFFECT_TAUNT_EXTRA) || source.hasTag("summonincharm253")) && source.myID == skillCaster.myID)
        {
            retList = getFilteredOriginList(source.getCurrentFriendsList(), skillCaster, source, excludeSummon, onlySummon, ignoreCanbeattackEffect, lowerHpLimitCharacter, realSearchType);
            retList->remove(&source);
        }
        else
        {
            retList = getFilteredOriginList(source.getCurrentRivalsList(), skillCaster, source, excludeSummon, onlySummon, ignoreCanbeattackEffect, lowerHpLimitCharacter, realSearchType);
        }

        return retList;
    }

    CharacterData* AIActionController::getAllRandomDeadCharacter(AIContext& aiContext, std::list<CharacterData* > *list1, std::list<CharacterData* > *list2)
    {
        std::list<CharacterData*> listAll;
        if (list1)
            listAll.insert(listAll.end(), list1->begin(), list1->end());
        if (list2)
            listAll.insert(listAll.end(), list2->begin(), list2->end());
        return getRandomDeadCharacter(aiContext, &listAll);
    }

    CharacterData* AIActionController::getRandomDeadCharacter(AIContext& aiContext, std::list<CharacterData*> *list)
    {
        if (list)
        {
            CharacterData *target = nullptr;
            std::vector<CharacterData*> tmpVec;
            for (auto iter = list->begin(); iter != list->end(); ++iter)
            {
                auto pCharacter = *iter;
                if (HeroActionController::isDeadAction(pCharacter->currentAction) && !pCharacter->isSummon())
                    tmpVec.push_back(pCharacter);
            }
            if (tmpVec.size() > 0)
            {
                int index = aiContext.localRandom->nextInt(0, tmpVec.size());
                target = tmpVec.at(index);
            }
            return target;
        }

        return nullptr;
    }

    void AIActionController::addAllDeadCharacter(CharacterDataList *characterList, CharacterDataList *outPutList)
    {
        if (characterList)
        {
            for (auto iter = characterList->begin(); iter != characterList->end(); ++iter)
            {
                auto pCharacter = *iter;
                if (HeroActionController::isDeadAction(pCharacter->currentAction) && !pCharacter->isSummon())
                    outPutList->push_back(pCharacter);
            }
        }
    }

    void AIActionController::addCharacterBySex(CharacterData& source, CharacterDataList *characterList, CharacterDataList *targetList, int sex, bool ignoreCanbeattackEffect)
    {
        if (!characterList)
            return;
        for (auto iter = characterList->begin(); iter != characterList->end(); ++iter) {
            if ((*iter)->getSex() == sex && (*iter)->canBeAttacked(&source, ignoreCanbeattackEffect))
                targetList->push_back(*iter);
        }
    }

    void AIActionController::addMaxHurtTarget(AIContext &aiContext, CharacterData *source, AIProjectile *projectile, CharacterDataList *filterList, CharacterDataList *targetList, int hurtID, int skillLevel, bool ignoreCanbeattackEffect)
    {
        if (hurtID > 0)
        {
            int maxHurt = 0;
            CharacterData *maxHurtTarget = nullptr;
            for (auto iter = filterList->begin(); iter != filterList->end(); ++iter)
            {
                if (!(*iter)->canBeAttacked(source, ignoreCanbeattackEffect))
                    continue;
                int hurtDelta = HeroHurtcalculate::simulateCalcHurt(aiContext, source, projectile, hurtID, 0, skillLevel, *iter);
                if (hurtDelta > maxHurt)
                {
                    maxHurtTarget = *iter;
                    maxHurt = hurtDelta;
                }
            }
            if (maxHurtTarget)
                targetList->push_back(maxHurtTarget);
        }
    }

    void AIActionController::addSomeoneCampAndRoleWithIdOld(CharacterDataList *filterList, CharacterData *source, AISkill *skillData, CharacterDataList *targetList, CharacterDataList *friendList, CharacterDataList *rivalList, bool ignoreCanbeattackEffect)
    {
        std::vector<int> vOurs;
        std::vector<int> vEnemys;

        if (skillData)
        {
            for (int i = 0; i < skillData->vTargetParms.size(); i++)
            {
                if (i % 2 == 0)
                {
                    if (skillData->vTargetParms[i] == TeamSide::kOur)
                    {
                        if ((i + 1) < skillData->vTargetParms.size())
                        {
                            vOurs.push_back(skillData->vTargetParms[++i]);
                        }
                    }
                    else if (skillData->vTargetParms[i] == TeamSide::kEnemy)
                    {
                        if ((i + 1) < skillData->vTargetParms.size())
                        {
                            vEnemys.push_back(skillData->vTargetParms[++i]);
                        }
                    }
                }
            }
        }

        auto characterList = friendList;
        if (characterList != NULL)
        {
            for (auto iter = characterList->begin(); iter != characterList->end(); ++iter)
            {
                for (int i = 0; i < vOurs.size(); i++)
                {
                    if ((*iter)->staticData->id == vOurs[i] && (*iter)->canBeAttacked(source, ignoreCanbeattackEffect))
                    {
                        targetList->push_back(*iter);
                    }
                }
            }
        }

        characterList = rivalList;
        if (characterList != NULL)
        {
            for (auto iter = characterList->begin(); iter != characterList->end(); ++iter)
            {
                for (int i = 0; i < vEnemys.size(); i++)
                {
                    if ((*iter)->staticData->id == vEnemys[i] && (*iter)->canBeAttacked(source, ignoreCanbeattackEffect))
                    {
                        targetList->push_back(*iter);
                    }
                }
            }
        }
    }

    void AIActionController::addSomeoneCampAndRoleWithId(CharacterDataList *filterList, CharacterData *source, std::vector<int> &targetParams, CharacterDataList *targetList, CharacterDataList *friendList, CharacterDataList *rivalList, bool ignoreCanbeattackEffect)
    {
        std::map<int, bool> ourCompare;
        std::map<int, bool> enemyCompare;
        for (int i = 0; i < targetParams.size(); i++)
        {
            if (i % 2 == 0)
            {
                int side = targetParams[i];
                if ((i + 1) < targetParams.size())
                {
                    int val = targetParams[++i];
                    if (side == TeamSide::kOur)
                        ourCompare[val] = true;
                    else if (side == TeamSide::kEnemy)
                        enemyCompare[val] = true;
                }
            }
        }

        auto compareFunc = [&targetList, source, ignoreCanbeattackEffect](TeamSide side, CharacterDataList *characterList, std::map<int, bool> &compareMap){
            if (characterList != NULL)
            {
                for (auto iter = characterList->begin(); iter != characterList->end(); ++iter)
                {
                    auto pCharacter = *iter;
                    if (pCharacter->teamSide != side)
                        continue;
                    if (compareMap.find(pCharacter->staticData->id) != compareMap.end())
                    {
                        if (pCharacter->canBeAttacked(source, ignoreCanbeattackEffect))
                        {
                            targetList->push_back(pCharacter);
                        }
                    }
                }
            }
        };

        if (filterList) {
            compareFunc(TeamSide::kOur, filterList, ourCompare);
            compareFunc(TeamSide::kEnemy, filterList, enemyCompare);
        }
        else {
            compareFunc(TeamSide::kOur, friendList, ourCompare);
            compareFunc(TeamSide::kEnemy, rivalList, enemyCompare);
        }
    }

    void AIActionController::addAllCharacterInHPRange(CharacterData& source, CharacterDataList *characterList, CharacterDataList *targetList, int rangeLow, int rangeHigh, bool ignoreCanbeattackEffect) {
        if (!characterList)
            return;
        for (auto pCharacter : *characterList) {
            auto hpRatio = CharacterFunc::getSimplifiedHPRatio(pCharacter);
            if (!pCharacter->canBeAttacked(&source, ignoreCanbeattackEffect))
                continue;
            if (hpRatio >= rangeLow && hpRatio <= rangeHigh)
                targetList->push_back(pCharacter);
        }
    }

    CharacterData *AIActionController::getTargetOneEnemyRandomContinueHurtNotRandomTarget(AIContext &aiContext, CharacterData *source, CharacterDataList *characterList, bool ignoreCanbeattackEffect) {
        if (source->GetFlagFirstFound26Target() && source->GetFirstRoundConfirm26Target())
            return source->GetFirstRoundConfirm26Target();

        return getRandomCharacter(aiContext, *source, characterList, ignoreCanbeattackEffect);
    }

    void AIActionController::addCharacterTraversal(CharacterData& source, CharacterDataList *characterList, CharacterDataList *targetList, int intervalTick, int totalTick, bool ignoreCanbeattackEffect)
    {
        if (source.getTraversalStartTick() <= 0 || source.localTick > source.getTraversalStartTick() + totalTick)
        {
            source.resetTraversalTargetRecord();
            source.setTraversalStartTick(source.localTick);
        }

        if (source.getTraversalLastPickTick() > 0 && source.localTick < source.getTraversalLastPickTick() + intervalTick)
            return;

        if (!characterList)
            return;

        for (auto pCharacter : *characterList) {
            if (pCharacter->canBeAttacked(&source, ignoreCanbeattackEffect) && !source.isTargetLockedByTraversal(pCharacter->myID)) {
                targetList->push_back(pCharacter);
                source.pickTraversalTarget(pCharacter->myID, source.localTick);
                return;
            }
        }
    }

    void AIActionController::addAllCharacterHasMark(CharacterData &source, CharacterDataList *characterList, CharacterDataList *targetList, int markID, bool ignoreCanbeattackEffect) {
        if (!characterList)
            return;

        for (auto iter = characterList->begin(); iter != characterList->end(); ++iter) {
            auto pCharacter = *iter;
            
            for (auto markIter = pCharacter->markCasts.begin(); markIter != pCharacter->markCasts.end(); ++markIter) {
                auto pMark = *markIter;
                if (pMark->markID == markID) {
                    if (pCharacter->canBeAttacked(&source, ignoreCanbeattackEffect))
                        targetList->push_back(pCharacter);

                    break;
                }
            }
        }
    }

    void AIActionController::addSelfSummonInRange(AIContext& aiContext, CharacterData& source, CharacterDataList *outPutList, short x1, short x2, short y1, short y2, short z1, short z2, bool ignoreCanbeattackEffect) {
        int x11 = source.getX() + x1;
        int x22 = source.getX() + x2;
        if (source.direction == -1)
        {
            x11 = source.getX() - x2;
            x22 = source.getX() - x1;
        }
        int y11 = source.getY() + y1;
        int y22 = source.getY() + y2;
        for (auto iter = source.summonCasts.begin(); iter != source.summonCasts.end(); ++iter)
        {
            auto pSummonCaster = *iter;
            for (auto summonIter = pSummonCaster->summonUIDList.begin(); summonIter != pSummonCaster->summonUIDList.end(); ++summonIter)
            {
                auto summonUID = *summonIter;
                auto c = aiContext.getCharacterByID(summonUID);
                if (c && c->isSummon() && c->getFakeSummonType() == 0 /*12.0*/ && c->getSummonSource()->myID == source.myID && c->canBeAttacked(&source, ignoreCanbeattackEffect)) {
                    if (c->getX() >= x11 && c->getX() <= x22 && c->getY() >= y11 && c->getY() <= y22) {
                        outPutList->push_back(c);
                    }
                }
            }
        }
    }

    void AIActionController::addCharacterHasLabels(CharacterData& source, CharacterDataList *characterList, CharacterDataList *outPutList, std::vector<int> &labels, bool ignoreCanbeattackEffect)
    {
        if (!characterList)
            return;

        for (auto iter = characterList->begin(); iter != characterList->end(); ++iter) {
            auto pCharacter = *iter;
            if (!pCharacter->canBeAttacked(&source, ignoreCanbeattackEffect))
                continue;
            bool isMatch = true;
            auto &&characterLabels = pCharacter->staticData->vLabels;
            for (int i = 0; i < labels.size(); i++) {
                auto theLabel = labels[i];
                if (std::find(characterLabels.begin(), characterLabels.end(), theLabel) == characterLabels.end()) {
                    isMatch = false;
                    break;
                }
            }

            if (isMatch)
                outPutList->push_back(pCharacter);
        }
    }

    CharacterDataList *AIActionController::pickFilterList(CharacterDataList *filterList, CharacterDataList *anotherList) {
        return filterList ? filterList : anotherList;
    }

    std::unique_ptr<std::list<CharacterData*>> AIActionController::getFilteredOriginList(std::list<CharacterData*> *theList, CharacterData &skillCaster, CharacterData &source, bool excludeSummon, bool onlySummon, bool ignoreCanbeattackEffect, CharacterData *lowerHpLimitCharacter, int realSearchType)
    {
        auto outList = std::unique_ptr<std::list<CharacterData*>>(new std::list<CharacterData*>);
        if (!theList)
            return outList;

        for (auto aCharacter : *theList)
        {
            if (excludeSummon && aCharacter->seemsSummon())
            {
                continue;
            }

            if (onlySummon && !aCharacter->seemsSummon())
            {
                continue;
            }

//            if (skillCaster.cannotFindFriendTargets.size() > 0 || skillCaster.cannotFindRivalTargets.size() > 0)
//            {
//                if (!skillCaster.canFindTargetOrigin(aCharacter))
//                    continue;
//            }
//
//            if (aCharacter->cannotBeFoundConfig.size() > 0)
//            {
//                if (!aCharacter->canBeFoundByRelativeSide(aCharacter->teamSide == skillCaster.teamSide ? 1 : 0))
//                    continue;
//            }
//
//            if (source.isInDuel() || aCharacter->isInDuel())
//            {
//                if (!source.isDuelWithTarget(aCharacter))
//                    continue;
//            }
            
            //if (!aCharacter->canBeAttacked(&source, ignoreCanbeattackEffect))
            if (!aCharacter->isFindableToAttacker(&source, ignoreCanbeattackEffect))
            {
                continue;
            }

            if (aCharacter->hasTag("misseff263withorigin") && !aCharacter->isFindableToAttacker(&skillCaster, ignoreCanbeattackEffect))
            {
                continue;
            }

            if (lowerHpLimitCharacter && CharacterFunc::GetCharacterHPRatio(*aCharacter) <= CharacterFunc::GetCharacterHPRatio(*lowerHpLimitCharacter))
            {
                continue;
            }

            if (aCharacter->hasEffect(297) && !luahelper_targethelper_is_target297findabletosearcher(AIContext::getInstance()->getLuaState(), aCharacter, &source, realSearchType))
            {
                continue;
            }
            
            // 12.0
            if (source.isInDuel()) {
                if (source.myID != aCharacter->myID && source.getDuelTarget()->myID != aCharacter->myID)
                    continue;
            }

            outList->push_back(aCharacter);
        }

        return outList;
    }

    void AIActionController::invalidateCaster(CharacterData *c, int casterType, int casterID)
    {
        switch (casterType) {
        case eHurtExec:
            AIActionInnerHelper::InvalidateHurtCaster(c, casterID);
            break;
        }
    }

    void AIActionController::addToRemoveCaster(CharacterData *c, int casterType, int casterID)
    {
        AIContext *ai = AIContext::getInstance();
        if (casterType == eHurtExec)
        {
            auto &casters = ai->hurtCastersToRemove[c->myID];
            casters[casterID] = 1;
        }
    }

    void AIActionController::removeInvalidCasters(AIContext &aiContext)
    {
        if (aiContext.hurtCastersToRemove.size() > 0)
        {
            for (auto iter = aiContext.hurtCastersToRemove.begin(); iter != aiContext.hurtCastersToRemove.end(); ++iter)
            {
                CharacterData *c = aiContext.getCharacterByID(iter->first);
                if (!c)
                {
                    continue;
                }

                auto &casters = iter->second;

                for (auto it = c->hurtCasts.begin(); it != c->hurtCasts.end(); )
                {
                    AIContinuousHurtCaster *caster = *it;
                    if (casters.find(caster->hurtID) != casters.end() && !caster->isValid)
                    {
                        it = c->hurtCasts.erase(it);
                        delete caster;
                    }
                    else
                    {
                        ++it;
                    }
                }
            }

            aiContext.hurtCastersToRemove.clear();
        }
    }

} // namespace OnePiece

