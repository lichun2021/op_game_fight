//
//  AIContinuousMarkCaster.cpp
//  OnePiece
//
//  Created by h7ing on Mar/16/2015.
//
//

#include "AIContinuousMarkCaster.h"
#include <algorithm>
#include "AIContext.h"
#include "action_hero/HeroActionController.h"
#include "AIScriptBinder.hpp"

namespace OnePiece {

    namespace MarkInnerHelper {

        int GetMaxLevel(MarkExec* exec, CharacterData* c, int skilllv)
        {
            int maxlv = exec->maxLevelBase + exec->maxLevelUp * skilllv / 100;

            // 指定角色在场上,最高叠加层数提升
            if (exec->maxLevelPromoteCond == 1)
            {
                CharacterDataListPtr characterList = nullptr;
                if (exec->maxLevelUpParam.camp == 1)
                    characterList = c->getCurrentFriendsList();
                else if (exec->maxLevelUpParam.camp == 2)
                    characterList = c->getCurrentRivalsList();

                if (characterList)
                {
                    for (auto iter = characterList->begin(); iter != characterList->end(); ++iter)
                    {
                        CharacterData *ac = *iter;
                        if (ac->getID() == exec->maxLevelUpParam.characterId && !HeroActionController::isDeadAction(ac->currentAction))
                        {
                            maxlv += exec->maxLevelUpParam.upLevel;
                            break;
                        }
                    }
                }
            }

            if (maxlv > exec->maxLevelValue)
                maxlv = exec->maxLevelValue;

            return maxlv;
        }

        int GetDuration(MarkExec* exec, int skilllv, CharacterData *c)
        {
            int d = exec->duration;

            if (exec->durationTimeAddUp != 0)
            {
                if (exec->durationPromoteType == 0)
                {
                    d += skilllv * exec->durationTimeAddUp;
                }
                else if (exec->durationPromoteType == 1)
                {
                    // 使用角色拥有的指定技能等级计算持续时间提升值
                    for (auto iter = c->skillList.begin(); iter != c->skillList.end(); ++iter)
                    {
                        if (exec->durationPromoteRelateSkillID == iter->first)
                        {
                            d += iter->second * exec->durationTimeAddUp;
                            break;
                        }
                    }
                }
            }

            return d;
        }

        // 达到层数,有人死亡触发
        void CheckTriggerType5(AIContinuousMarkCaster *caster, CharacterData *trigger)
        {
            if (caster->innerMarkData && EMarkEffectTriggerType::eMarkEffectTriggerByReachLevelAndDead == caster->innerMarkData->triggerType)
            {
                if (caster->isTriggerParam(AIContinuousMarkCaster::TriggerType::kEffect, AIContinuousMarkCaster::kInvalidParamID, AIContinuousMarkCaster::kInvalidParamID))
                {
                    int result = caster->excuteEffect(caster->skillLevel, trigger);
                    if (MarkExcuteResult::EXCUTE_RESULT_REMOVE_MARK == result)
                    {
                        caster->invalidate();
                    }
                }
            }
        }

        void CheckTriggerCharacterDead(CharacterDataListPtr cs, CharacterData *c)
        {
            for (CharacterDataList::iterator iter = cs->begin(); iter != cs->end(); ++iter)
            {
                CharacterData *ac = *iter;
                if (!ac->isSummon() && !HeroActionController::isDeadAction(ac->currentAction))
                {
                    for (auto it = ac->markCasts.begin(); it != ac->markCasts.end(); ++it)
                    {
                        AIContinuousMarkCaster *caster = *it;

if (caster->isValid())
{
    CheckTriggerType5(caster, ac);
}
                    }
                }
            }
        }

        void RespondToCharacterDead(CharacterDataListPtr cs, CharacterData *c)
        {
            for (CharacterDataList::iterator iter = cs->begin(); iter != cs->end(); ++iter)
            {
                CharacterData *ac = *iter;
                for (auto it = ac->markCasts.begin(); it != ac->markCasts.end(); ++it)
                {
                    AIContinuousMarkCaster *caster = *it;

                    if (caster->isValid())
                    {
                        if (c->myID == ac->myID)
                        {
                            caster->onCharacterDead();
                        }
                        else if (!ac->isSummon() && !HeroActionController::isDeadAction(ac->currentAction))
                        {
                            CheckTriggerType5(caster, ac);
                        }
                    }

                    if (!caster->isValid())
                        continue;

                    // 再清除标记:标记施法者死亡后清除
                    // OP2 fix:使用原始施法者
                    if (!HeroActionController::isDeadAction(ac->currentAction) || ac->myID == c->myID)
                    {
                        if (caster->initOrigin && caster->initOrigin->myID == c->myID && caster->innerMarkData && caster->innerMarkData->afterTriggerDeadIfCleared == EMarkTriggerCharDeadClearType::eMarkTriggerCharDead_Cleared)
                        {
                            caster->invalidate();
                        }
                    }
                }
            }
        }
    }

    AIContinuousMarkCaster::AIContinuousMarkCaster(AIContext& aiContext, int skID, int skLv, int mkID, int mkLv, CharacterData* origin, AIProjectile* projectile, CharacterData* target)
        : aiContext(aiContext), markID(mkID), _markLevel(mkLv), AINode(target->myID, target->clusterNum), skillID(skID), skillLevel(skLv)
    {
        myID = aiContext.local_incremential++;

        staticData = AIDataHelper::getMarkExec(markID);
        innerMarkData = nullptr;

        initSkillLevel = skLv;
        _initLevel = mkLv;

        setEffectExecData(AIDataHelper::getMarkEffectExec(staticData->type, _initLevel));//mkLv));

        this->projectile = projectile;
        this->origin = origin;
        this->target = target;
        initOrigin = origin;

        localTick = aiContext.currentTick;
        startTick = localTick;
        totalTick = MarkInnerHelper::GetDuration(staticData, skLv, origin);

        hitContinualNum = 0;

        _maxLevel = MarkInnerHelper::GetMaxLevel(staticData, origin, skLv);

        ignoreFreeze = target->isCaptain();

        switch (innerMarkData->triggerType)
        {
        case EMarkEffectTriggerType::eMarkEffectTriggerByOwnerCastHurt:
            target->markType11Cnt += 1;
            break;
        case EMarkEffectTriggerType::eMarkEffectTriggerByFriendHurtType:
            target->setTagValue("markt8", target->getTagValue("markt8") + 1);
            break;
        default: break;
        }

        target->markCasts.push_back(this);

#ifdef OPTOOLS
        // quanhao.liu 2023/8/22
        MarkInfoLog* markInfoLog = new MarkInfoLog();
        markInfoLog->markID = markID;
        markInfoLog->myID = myID;
        markInfoLog->currentTick = aiContext.currentTick;
        markInfoLog->originID = origin->getID();
        markInfoLog->originName = origin->staticData->name;
        markInfoLog->targetID = target->getID();
        markInfoLog->targetName = target->staticData->name;
        markInfoLog->totalTick = totalTick;
        markInfoLog->maxLevel = _maxLevel;

        aiContext.incSkillTriggerCount(skillID);
        SkillInfoLog* skillInfoLog = aiContext.getSkillInfoLog(skillID, skillLevel, myID);
        skillInfoLog->markInfoLogs.push_back(markInfoLog);
        aiContext.markInfoLogsMap.insert({ myID, markInfoLog });
        aiContext.caseterFindIndexMap.insert({ myID, skillInfoLog->markInfoLogs.size() - 1 });
        aiContext.casterFindSkillIndexMap.insert({ myID, aiContext.getSkillInfoLogsIndex(skillID) });
#endif
    }
    
    AIContinuousMarkCaster::~AIContinuousMarkCaster()
    {
        removeUIEffect();

        removeTriggeredEffectCasts();

        removeTriggeredHurtCasts();

        if (innerMarkData && innerMarkData->notAffactCDTick > 0)
            target->markDeadImmunes[staticData->type] = innerMarkData->notAffactCDTick;

        if (EMarkEffectTriggerType::eMarkEffectTriggerByOwnerCastHurt == innerMarkData->triggerType)
        {
            target->markType11Cnt -= 1;
            if (target->markType11Cnt < 0)
                target->markType11Cnt = 0;
        }
        else if (EMarkEffectTriggerType::eMarkEffectTriggerByFriendHurtType == innerMarkData->triggerType)
        {
            int val = target->getTagValue("markt8") - 1;
            if (val < 0)
                val = 0;
            target->setTagValue("markt8", val);
        }

        //if (!isRemoved)
        //{
        //    for (auto iter = target->markCasts.begin(); iter != target->markCasts.end(); ++iter)
        //    {
        //        if (*iter == this)
        //        {
        //            target->markCasts.erase(iter);
        //            break;
        //        }
        //    }
        //}
    }

    void AIContinuousMarkCaster::checkAddMark(AIContext& aiContext, int skillID, int skillLevel, CharacterData* origin, AIProjectile* projectile, CharacterData* target, MarkExec* markToBeAdd)
    {
        // ways to create mark:
        // a. a fresh new mark
        // b. increase existed mark level, reset mark duration
        // c. conteract some mark, clear those marks, do not create new mark
        // d. cover some mark, clear those marks, create new mark
        // e. composite(can conteract and cover), clear those marks, do not create new mark
        // f. if target has immunse for the mark, do nothing

        if (!target || !markToBeAdd)
            return;

        if (target->markDeadImmunes.find(markToBeAdd->type) != target->markDeadImmunes.end())
            return;

        bool hasCounteractMark = false;
        bool hasSameMark = false;
		bool hasCoverMark = false;// 覆盖双向

        std::unordered_map<int, int> deleteTypes;
        for (auto iter = markToBeAdd->coverTypes.begin(); iter != markToBeAdd->coverTypes.end(); ++iter)
            deleteTypes[*iter] = 0;
		
		// clair_wang start
		// Function reverse search cover
		if (deleteTypes.size() < 1)
		{
			hasCoverMark = isHasCoverMarkFlag(target, markToBeAdd->type);
		}
		// clair_wang end

		
		// one by one counteract Flag
		for (auto iter = markToBeAdd->counteractTypes.begin(); iter != markToBeAdd->counteractTypes.end(); ++iter)
			deleteTypes[*iter] = 1;
		
        auto iterFound = deleteTypes.end();

        bool hasRemovedSameMark = false;
        bool shouldRemoveMark = false;
		AIContinuousMarkCaster* pTmp = nullptr;
        for (auto iter = target->markCasts.begin(); iter != target->markCasts.end(); ++iter)
        {
            auto pMark = *iter;
            if (!pMark->isValid())
                continue;
            shouldRemoveMark = false;
            iterFound = deleteTypes.find(pMark->staticData->type);
            if (iterFound != deleteTypes.end())
            {
                if (iterFound->second)
                    hasCounteractMark = true;

                shouldRemoveMark = true;
            }
            else
            {
                if (markToBeAdd->type == (*iter)->staticData->type)
                {
                    hasSameMark = true;
                    pTmp = pMark;
                    auto result = pMark->updateMarkLevelOffset(markToBeAdd->increaseLevel, skillLevel, origin);
                    if (MarkExcuteResult::EXCUTE_RESULT_REMOVE_MARK == result)
                    {
                        shouldRemoveMark = true;
                        hasRemovedSameMark = true;
                    }
                }
            }

            if (shouldRemoveMark)
            {
                //pMark->isRemoved = true;
                //delete *iter;
                //iter = target->markCasts.erase(iter);
				if (hasCounteractMark)
				{
					if (markToBeAdd->counteractSwitch == MarkExec::Enum_Counteact_Direct)
					{
						// 直接抵消
						pMark->invalidate();
					}
					else if (markToBeAdd->counteractSwitch == MarkExec::Enum_Counteact_OneByOne)
					{
						// 一层层抵消
						int nOffset = -1;
						auto result = pMark->updateMarkLevelOffset(nOffset, skillLevel, origin);
						if (MarkExcuteResult::EXCUTE_RESULT_REMOVE_MARK == result)
						{
							pMark->invalidate();
						}
					}
				}
				else
				{
					pMark->invalidate();
				}
            }
            //else
            //{
            //    ++iter;
            //}
        } // END for

        if (hasCounteractMark)
        {
        }
        else if (hasSameMark)
        {
			if (!hasRemovedSameMark)
			{
				checkExcuteAllEffectTriggeredByMark(target, markToBeAdd->ID, pTmp);
				checkExcuteEffectTriggerdByReachLevelMark(target, markToBeAdd->ID, pTmp);
			}
        }
		else if (hasCoverMark)
		{
		}
        else
        {
            AIContinuousMarkCaster *mark = new AIContinuousMarkCaster(aiContext, skillID, skillLevel, markToBeAdd->ID, markToBeAdd->initLevel, origin, projectile, target);
            auto result = mark->Excute(aiContext);
            if (MarkExcuteResult::EXCUTE_RESULT_HAS_REMOVED_MARK != result)
            {
                result = mark->checkExcuteForTriggeredByMark();
                if (MarkExcuteResult::EXCUTE_RESULT_REMOVE_MARK == result)
                {
                    //delete mark;
                    mark->invalidate();
                }
                else
                {
					if (mark->innerMarkData/* && (!mark->innerMarkData->ifShowOnTrigger)*/)
					{
						mark->updateUIEffect();
					}

                    checkExcuteEffectTriggeredByReachLevel(target, mark);
                    checkExcuteAllEffectTriggeredByMark(target, markToBeAdd->ID, mark);
					checkExcuteEffectTriggerdByReachLevelMark(target, markToBeAdd->ID, mark);
                }
            }
        }
    } // END checkAddMark

    void AIContinuousMarkCaster::checkExcuteEffectTriggeredByReachLevel(CharacterData* target, AIContinuousMarkCaster* mark)
    {
        if (mark->innerMarkData && EMarkEffectTriggerType::eMarkEffectTriggerReachLevel == mark->innerMarkData->triggerType)
        {
			CharacterData *pTrigger = nullptr;

			pTrigger = (mark->innerMarkData->triggerEftSelectType == EMarkTriggerEffectSelectType::eMarkTriggerEffectByMarkCaster) ? mark->origin : mark->origin;

            auto result = mark->excuteEffect(mark->skillLevel, pTrigger);
            if (MarkExcuteResult::EXCUTE_RESULT_REMOVE_MARK == result)
            {
                //for (auto iter = target->markCasts.begin(); iter != target->markCasts.end(); ++iter)
                //{
                //    if (mark == *iter)
                //    {
                //        (*iter)->isRemoved = true;
                //        delete (*iter);
                //        target->markCasts.erase(iter);
                //        break;
                //    }
                //} // END for

                mark->invalidate();
            } // END if remove
        } // END if reach level
    }


    void AIContinuousMarkCaster::checkExcuteAllEffectTriggeredByMark(CharacterData* target, int markID, AIContinuousMarkCaster* markInst/* = nullptr*/)
    {
        if (markInst && markInst->innerMarkData && EMarkEffectTriggerType::eMarkEffectTriggerBySomeoneMark == markInst->innerMarkData->triggerType)
        {
            for (auto itt = target->markCasts.begin(); itt != target->markCasts.end(); ++itt)
            {
                auto tmpTt = *itt;
                if (tmpTt->myID == markInst->myID)
                    continue;

                if (markInst->isTriggerParam(TriggerType::kEffect, tmpTt->markID, kInvalidParamID))
                {
					markInst->setEffectTriggedMarkLevel(tmpTt->innerMarkData->level);

					CharacterData* pTrigger = nullptr;					

					pTrigger = (markInst->innerMarkData->triggerEftSelectType == EMarkTriggerEffectSelectType::eMarkTriggerEffectByMarkCaster) ? markInst->target : tmpTt->origin;

                    auto result = markInst->excuteEffect(markInst->skillLevel, pTrigger);
                    if (MarkExcuteResult::EXCUTE_RESULT_REMOVE_MARK == result)
                    {
                        markInst->invalidate();
                    }
                    break;
                }
            }
        }

        for (auto iter = target->markCasts.begin(); iter != target->markCasts.end(); ++iter)
        {
            auto caster = *iter;
            if (!caster->isValid())
                continue;

            if (markInst && markInst->myID == caster->myID)
            {
				continue;
            }

            int result = MarkExcuteResult::EXCUTE_RESULT_NONE;

            if (caster->innerMarkData && EMarkEffectTriggerType::eMarkEffectTriggerByMark == caster->innerMarkData->triggerType)
            {
                //for (auto paramIter = target->markCasts.begin(); paramIter != target->markCasts.end(); ++paramIter)
                //{
                //    if (*paramIter == *iter)
                //        continue;
                //    if (caster->isTriggerParam(TriggerType::kEffect, (*paramIter)->staticData->ID, kInvalidParamID))
                //    {
                //        result = caster->excuteEffect(caster->skillLevel, caster->origin);
                //        break;
                //    }
                //}

				if (caster->isTriggerParam(TriggerType::kEffect, markID, kInvalidParamID))
				{
					CharacterData* pTrigger = nullptr;

					pTrigger = (caster->innerMarkData->triggerEftSelectType == EMarkTriggerEffectSelectType::eMarkTriggerEffectByMarkCaster) ? caster->origin : markInst->origin;

					result = caster->excuteEffect(caster->skillLevel, pTrigger);
				}
            }

            if (MarkExcuteResult::EXCUTE_RESULT_REMOVE_MARK == result)
            {
                //(*iter)->isRemoved = true;
                //delete *iter;
                //iter = target->markCasts.erase(iter);

                caster->invalidate();
            }
            //else
            //{
            //    ++iter;
            //}
        } // END for
    }

	static int OldExecuteMarkEffect(AIContext& aiContext, CharacterData& target, int skillID, int theSkillLevel, int theCallType, CharacterData* trigger/* = nullptr*/, AIContinuousMarkCaster *caster)
	{
		int result = MarkExcuteResult::EXCUTE_RESULT_NONE;

		if (EMarkEffectTriggerType::eMarkEffectTriggerByMarkedSomeoneHurted == caster->innerMarkData->triggerType ||
			EMarkEffectTriggerType::eMarkEffectTriggerByOwnerCastHurt == caster->innerMarkData->triggerType)
		{
			CharacterData *theEffectOrigin = caster->origin;
			if (caster->innerMarkData->triggerEftSelectType == EMarkTriggerEffectSelectType::eMarkTriggerEffectByTrigerMarkCaster)
				theEffectOrigin = trigger ? trigger : caster->origin;
			else if (caster->innerMarkData->triggerEftSelectType == EMarkTriggerEffectSelectType::eMarkTriggerEffectByCreatedMarkCaster)
				theEffectOrigin = caster->initOrigin;
			result = caster->excuteEffect(theSkillLevel, theEffectOrigin);
		}
		else if (caster->innerMarkData->triggerEftSelectType == EMarkTriggerEffectSelectType::eMarkTriggerEffectByMarkCaster)
		{
			if (theCallType == AIContinuousMarkCaster::kCheckMarkCallBySkill)
			{
				result = caster->excuteEffect(caster->skillLevel, &target);
			}
			else if (theCallType == AIContinuousMarkCaster::kCheckMarkCallByHurt)
			{
				if (!trigger)
					trigger = caster->origin;
				result = caster->excuteEffect(theSkillLevel, trigger);
			}
			else if (theCallType == AIContinuousMarkCaster::kCheckMarkCallByFriendHurt)
			{
				result = caster->excuteEffect(theSkillLevel, caster->origin);
			}
		}
		else if (caster->innerMarkData->triggerEftSelectType == EMarkTriggerEffectSelectType::eMarkTriggerEffectByCreatedMarkCaster)
		{
			if (theCallType == AIContinuousMarkCaster::kCheckMarkCallByHurt)
			{
				if (EMarkEffectTriggerType::eMarkEffectTriggerAttackByHurt == caster->innerMarkData->triggerType)
				{
					result = caster->excuteEffect(theSkillLevel, caster->origin);
				}
			}
		}
		else
		{
			if (theCallType == AIContinuousMarkCaster::kCheckMarkCallBySkill)
			{// 1\3
				if (target.hasSkill(skillID))
				{
					result = caster->excuteEffect(caster->skillLevel, &target);
				}
			}
			else if (theCallType == AIContinuousMarkCaster::kCheckMarkCallByHurt)
			{// 2
				result = caster->excuteEffect(theSkillLevel, trigger);
			}
			else if (theCallType == AIContinuousMarkCaster::kCheckMarkCallByFriendHurt)
			{ // 8
				result = caster->excuteEffect(theSkillLevel, trigger);
			}
		}

		return result;
	}

	void AIContinuousMarkCaster::checkExcuteEffect(AIContext& aiContext, CharacterData& target, int skillID, int theSkillLevel, int hurtType, int hurtProperty, int theCallType, int HPDelta /* = 0*/, int hitType /* = 0*/, CharacterData* trigger/* = nullptr*/)
    {
      //  if (!target.markCasts.size())
     //       return;

		
        // 受到特定技能或伤害类型，触发标记效果
        bool shouldExcuteEffect = false;
        bool shouldRemoveMark = false;
        for (auto ite = target.markCasts.begin(); ite != target.markCasts.end(); ++ite)
        {
            AIContinuousMarkCaster *caster = *ite;
            shouldExcuteEffect = false;
            shouldRemoveMark = false;

            if (EMarkEffectTriggerType::eMarkEffectTriggerAttackBySkill == caster->innerMarkData->triggerType)
            {
                shouldExcuteEffect = caster->isTriggerParam(TriggerType::kEffect, skillID, kInvalidParamID);
            }
            else if (EMarkEffectTriggerType::eMarkEffectTriggerAttackByHurt == caster->innerMarkData->triggerType)
            {
                shouldExcuteEffect = caster->isTriggerParam(TriggerType::kEffect, hurtType, hurtProperty);
            }
			else if (EMarkEffectTriggerType::eMarkEffectTriggerBySingleHurtNum == caster->innerMarkData->triggerType)
			{
				// clair_wang 20160225
				shouldExcuteEffect = caster->isTriggerParam(TriggerType::kEffect, HPDelta, kInvalidParamID);
			}
			else if (EMarkEffectTriggerType::eMarkEffectTriggerByFriendHurtType == caster->innerMarkData->triggerType)
			{
				// clair_wang 20160503
				shouldExcuteEffect = caster->isTriggerParam(TriggerType::kEffect, hurtType, hurtProperty);
			}
			else if (EMarkEffectTriggerType::eMarkEffectTriggerUseSkill == caster->innerMarkData->triggerType)
			{
				if (theCallType == E_CheckExcuteEffectMarkCallType::kCheckMarkCallBySkill)
				{
					shouldExcuteEffect = caster->isTriggerParam(TriggerType::kEffect, skillID, kInvalidParamID);
				}
			}
			else if (EMarkEffectTriggerType::eMarkEffectTriggerByMarkedSomeoneHurted == caster->innerMarkData->triggerType)
			{
				if (trigger == nullptr)
					return;
				shouldExcuteEffect = caster->isTriggerParam(TriggerType::kEffect, hurtType, hurtProperty, trigger->myID);
			}
            else if (EMarkEffectTriggerType::eMarkEffectTriggerByOwnerCastHurt == caster->innerMarkData->triggerType)
            {
                if (theCallType == kCheckMarkCallByOwnerCastHurt)
                {
                    shouldExcuteEffect = caster->isTriggerParam(TriggerType::kEffect, hurtType, hurtProperty);
                }
            }


            if (shouldExcuteEffect)
            {
                int result = MarkExcuteResult::EXCUTE_RESULT_NONE;

				if (caster->innerMarkData->triggerEftSelectType <= 2)
					result = OldExecuteMarkEffect(aiContext, target, skillID, theSkillLevel, theCallType, trigger, caster);
				else
				{
					CharacterData *t = nullptr;
					int sklv = 0;
					switch (caster->innerMarkData->triggerEftSelectType)
					{
					case eMarkTriggerEffectByMarkCurrentOrigin:
						t = caster->origin;
						sklv = caster->skillLevel;
						break;
					case eMarkTriggerEffectByMarkCurrentTarget:
						t = caster->target;
						sklv = caster->skillLevel;
						break;
					case eMarkTriggerEffectByMarkCurrentTrigger:
						t = trigger;
						sklv = theSkillLevel;
						break;
					case eMarkTriggerEffectByMarkInitOrigin:
						t = caster->initOrigin;
						sklv = caster->initSkillLevel;
						break;
					}
					result = caster->excuteEffect(theSkillLevel, t);
				}

                shouldRemoveMark = (MarkExcuteResult::EXCUTE_RESULT_REMOVE_MARK == result);
            }

            if (shouldRemoveMark)
            {
                //(*iter)->isRemoved = true;
                //delete (*iter);
                //iter = target.markCasts.erase(iter);
                caster->invalidate();
            }
            //else
            //{
            //    ++iter;
            //}
        }
    }

	void AIContinuousMarkCaster::checkExcuteDead(AIContext& aiContext, CharacterData& target, int skillID, int skillLevel, int hurtType, int hurtProperty, CharacterData* trigger/* = nullptr*/, int hurtID /* = 0*/)
    {
        // 受到特定技能或伤害类型，消耗标记
        if (!target.markCasts.size())
            return;

        bool shouldExcuteMarkDead = false;
        bool shouldRemoveMark = false;
        for (auto iter = target.markCasts.begin(); iter != target.markCasts.end(); ++iter)
        {
            shouldExcuteMarkDead = false;
            shouldRemoveMark = false;

            if (EMarkEffectDeadType::eMarkEffectDeadBySkill == (*iter)->innerMarkData->deadType)
            {
				bool bFlag = false;
				AISkill* skillData = AIDataHelper::getSkillData(skillID);

				if (skillData != nullptr && hurtID > 0)
				{
					for (auto itt = skillData->skillExec.begin(); itt != skillData->skillExec.end(); ++itt)
					{
						if (itt->execId > 0 && itt->execId == hurtID)
						{
							bFlag = true;
							break;
						}
					}
				}

				if (bFlag)
				{
					shouldExcuteMarkDead = (*iter)->isTriggerParam(TriggerType::kDeath, skillID, kInvalidParamID);
				}
				else
				{
					shouldExcuteMarkDead = false;
				}
            }
            else if (EMarkEffectDeadType::eMarkEffectDeadByHurt == (*iter)->innerMarkData->deadType)
            {
                shouldExcuteMarkDead = (*iter)->isTriggerParam(TriggerType::kDeath, hurtType, hurtProperty);
            }
                
            if (shouldExcuteMarkDead)
            {
                if (!trigger)
                    trigger = (*iter)->origin;
                (*iter)->excuteDeath(skillLevel, trigger);
                auto result = (*iter)->updateMarkLevelOffset(-(*iter)->innerMarkData->decreaseLevel, skillLevel, trigger);
                shouldRemoveMark = (MarkExcuteResult::EXCUTE_RESULT_REMOVE_MARK == result);
            }

            if (shouldRemoveMark)
            {
                //(*iter)->isRemoved = true;
                //delete (*iter);
                //iter = target.markCasts.erase(iter);
                (*iter)->invalidate();
            }
            //else
            //{
            //    ++iter;
            //}
        }
    }

    void AIContinuousMarkCaster::reshowUIEffects(AIContext &aiContext, CharacterData &target)
    {
        for (auto iter = target.markCasts.begin(); iter != target.markCasts.end(); ++iter)
        {
            auto mark = *iter;
            mark->removeUIEffect();
            mark->reshowUIEffect();
        }
    }

    int AIContinuousMarkCaster::checkExcuteForTriggeredByMark()
    {
		if (EMarkEffectTriggerType::eMarkEffectTriggerByMark != this->innerMarkData->triggerType
			|| EMarkEffectTriggerType::eMarkEffectTriggerBySomeoneMark != this->innerMarkData->triggerType
			)
		{
			return MarkExcuteResult::EXCUTE_RESULT_NONE;
		}

        for (auto iter = target->markCasts.begin(); iter != target->markCasts.end(); ++iter)
        {
            if ((*iter)->myID == this->myID)
                continue;
            if (this->isTriggerParam(TriggerType::kEffect, (*iter)->staticData->ID, kInvalidParamID))
            {
                return this->excuteEffect(this->skillLevel, this->origin);
            }
        }

        return MarkExcuteResult::EXCUTE_RESULT_NONE;
    }

    int AIContinuousMarkCaster::Excute(AIContext& aiContext)
    {
        AINode::update(aiContext);
        if (isFrozen)
            return MarkExcuteResult::EXCUTE_RESULT_NONE;

        if ((totalTick == 0 && (localTick - startTick - 1) > totalTick)
            || (totalTick != 0 && (localTick - startTick - 1) >= totalTick)
            )
        {
            if (innerMarkData && (EMarkEffectDeadType::eMarkEffectDeadByTime == innerMarkData->deadType
				|| EMarkEffectDeadType::eMarkEffectDeadByAllReplaceDeadTime == innerMarkData->deadType))
                excuteDeath(this->skillLevel, origin);
            //delete this;
            invalidate();
            return MarkExcuteResult::EXCUTE_RESULT_HAS_REMOVED_MARK;
        }

        return MarkExcuteResult::EXCUTE_RESULT_NONE;
    }

    void AIContinuousMarkCaster::invalidate()
    {
        _isValid = false;
    }

    int AIContinuousMarkCaster::updateMarkLevelOffset(int offset, int skLv, CharacterData* origin)
    {
        if (EMarkResetDurationType::eMarkResetDurationRefresh == staticData->resetDurationType)
            localTick = startTick;
        else if (EMarkResetDurationType::eMarkResetDurationAddon == staticData->resetDurationType)
            totalTick += MarkInnerHelper::GetDuration(staticData, this->skillLevel, origin);

        this->skillLevel = skLv;
        this->origin = origin;
        
        if (!offset)
            return MarkExcuteResult::EXCUTE_RESULT_NONE;

        int currMaxLevel = MarkInnerHelper::GetMaxLevel(staticData, origin, initSkillLevel);
        if (currMaxLevel < _maxLevel && _markLevel >= _maxLevel)
        {
            // 当前标记层数已达最高层数,且原标记最高层数高于新最高层数,则不会降低最高层数
            currMaxLevel = _maxLevel;
        }
        else
        {
            _maxLevel = currMaxLevel;
        }

        int tmpLevel = _markLevel + offset;
        if (tmpLevel > currMaxLevel)
            tmpLevel = currMaxLevel;
        else if (tmpLevel < 0)
            tmpLevel = 0;


        return updateMarkCountForce(tmpLevel, origin, tmpLevel);
    /*
        _markLevel = tmpLevel;

        if (_markLevel <= 0)
        {
            return MarkExcuteResult::EXCUTE_RESULT_REMOVE_MARK;
        }
        else
        {
            setEffectExecData(AIDataHelper::getMarkEffectExec(staticData->type, _markLevel));
			
			if (innerMarkData)
			{
				updateUIEffect();
			}
			
            removeTriggeredEffectCasts();

            removeTriggeredHurtCasts();

            if (EMarkEffectTriggerType::eMarkEffectTriggerReachLevel == innerMarkData->triggerType)
                return excuteEffect(skillLevel, origin);
        }

        return MarkExcuteResult::EXCUTE_RESULT_NONE;
     */
    }

    int AIContinuousMarkCaster::updateMarkCountForce(int tmpLevel, CharacterData* origin, int staticMarkLv) {
        this->origin = origin;
        _markLevel = tmpLevel;

        if (_markLevel <= 0)
        {
            return MarkExcuteResult::EXCUTE_RESULT_REMOVE_MARK;
        }
        else
        {
            setEffectExecData(AIDataHelper::getMarkEffectExec(staticData->type, staticMarkLv));

            if (innerMarkData)
            {
                updateUIEffect();
            }

            removeTriggeredEffectCasts();

            removeTriggeredHurtCasts();

            if (innerMarkData && (EMarkEffectTriggerType::eMarkEffectTriggerReachLevel == innerMarkData->triggerType))
                return excuteEffect(skillLevel, origin);
        }

        return MarkExcuteResult::EXCUTE_RESULT_NONE;
    }

    void AIContinuousMarkCaster::updateUIEffect()
    {
        if (aiContext.markProxy && innerMarkData && innerMarkData->iconID)
            aiContext.markProxy->executeExec(aiContext, target, innerMarkData->iconID, _markLevel);

        if (innerMarkData && innerMarkData->specialEffectID > 0)
            AIActionController::executeExec(aiContext, innerMarkData->specialEffectID, skillID, _markLevel, origin, projectile, target, 0, false, markID);
        if (innerMarkData && innerMarkData->soundID > 0)
            AIActionController::executeExec(aiContext, innerMarkData->soundID, skillID, _markLevel, origin, projectile, target, 0, false, markID);
    }

    void AIContinuousMarkCaster::removeUIEffect()
    {
        if (aiContext.markProxy && innerMarkData && innerMarkData->iconID)
        {
            aiContext.markProxy->notify(aiContext, target, innerMarkData->iconID);
        }
    }

 

	int AIContinuousMarkCaster::excuteEffect(int skillLevel, CharacterData* trigger)
	{ 
		//  令触发的效果类型高层覆盖低层，其它触发的类型各自独立。 11.0 modify

		if (!isValid())
			return MarkExcuteResult::EXCUTE_RESULT_NONE;

		if (!trigger)
			return MarkExcuteResult::EXCUTE_RESULT_NONE;

        // OP2 触发间隔
        if (innerMarkData->triggerCD && localTick < _nextTriggerTick)
        {
            return MarkExcuteResult::EXCUTE_RESULT_NONE;
        }

		int nTriggerEffectID = 0;
		std::vector<int> vTriggerEft;

		if (innerMarkData->triggerType == EMarkEffectTriggerType::eMarkEffectTriggerBySomeoneMark)
		{
			int nIdx = getEffectTriggedMarkLevel();

			nIdx--;

			if (nIdx >= 0 && nIdx < innerMarkData->triggerEffectIDs.size())
			{
				nTriggerEffectID = innerMarkData->triggerEffectIDs[nIdx];
			}
			else
			{
				nTriggerEffectID = innerMarkData->triggerEffectIDs[0];
			}
		}
		else
		{
			if (innerMarkData->triggerEffectIDs.size() > 1)
			{
				vTriggerEft.assign(innerMarkData->triggerEffectIDs.begin(), innerMarkData->triggerEffectIDs.end());
			}
			else
			{
				nTriggerEffectID = innerMarkData->triggerEffectIDs[0];
			}
		}

		if (innerMarkData && (nTriggerEffectID > 0 || vTriggerEft.size() > 0))
		{
			// clair added by 11.0 version

			if (nTriggerEffectID > 0)
			{
				AIActionController::executeExec(aiContext, nTriggerEffectID, skillID, skillLevel, trigger, projectile, target, 0, false, markID);
			}
			else if (vTriggerEft.size() > 0)
			{
				for (auto tmpIter = vTriggerEft.begin(); tmpIter != vTriggerEft.end(); ++tmpIter)
				{
					AIActionController::executeExec(aiContext, *tmpIter, skillID, skillLevel, trigger, projectile, target, 0, false, markID);
				}
			}

			//	AIActionController::executeExec(aiContext, nTriggerEffectID, skillID, skillLevel, /*origin*/trigger, projectile, target, 0, false, markID);

			if (target->effectCasts.size())
			{
				for (auto iter = target->effectCasts.rbegin(); iter != target->effectCasts.rend(); ++iter)
				{
					auto pEff = *iter;

					if (pEff)
					{
						if (nTriggerEffectID > 0)
						{
							if (nTriggerEffectID == pEff->effectID)
							{
								_triggeredEffects.insert(pEff->myID);
								break;
							}
						}
						else if (vTriggerEft.size() > 0)
						{
							for (auto tmpIter = vTriggerEft.begin(); tmpIter != vTriggerEft.end(); ++tmpIter)
							{
								if (pEff && *tmpIter == pEff->effectID)
								{
									_triggeredEffects.insert(pEff->myID);

									break;
								}
							}
						}
					}
				}

			}

            if (innerMarkData->ifClearHurtAfterMarkDead && target->hurtCasts.size())
            {
                for (auto iter = target->hurtCasts.rbegin(); iter != target->hurtCasts.rend(); ++iter)
                {
                    auto pEff = *iter;

                    if (pEff)
                    {
                        if (nTriggerEffectID > 0)
                        {
                            if (nTriggerEffectID == pEff->hurtID)
                            {
                                _triggeredHurts.insert(pEff->myID);
                                break;
                            }
                        }
                        else if (vTriggerEft.size() > 0)
                        {
                            for (auto tmpIter = vTriggerEft.begin(); tmpIter != vTriggerEft.end(); ++tmpIter)
                            {
                                if (pEff && *tmpIter == pEff->hurtID)
                                {
                                    _triggeredHurts.insert(pEff->myID);
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            _triggeredNum += 1;
            if (innerMarkData->triggerCD > 0)
            {
                _nextTriggerTick = localTick + innerMarkData->triggerCD;
            }

			if (EMarkEffectDeadType::eMarkEffectDeadByTriggered == innerMarkData->deadType || (eMarkEffectDeadByTriggerCnt == innerMarkData->deadType && innerMarkData->deadParams.size() > 0 && _triggeredNum >= innerMarkData->deadParams[0]))
			{
				excuteDeath(skillLevel, trigger);
				auto result = updateMarkLevelOffset(-innerMarkData->decreaseLevel, skillLevel, trigger);
				if (MarkExcuteResult::EXCUTE_RESULT_REMOVE_MARK == result)
				{
					_triggeredEffects.clear();
				}

				return result;
			}
		}

		return MarkExcuteResult::EXCUTE_RESULT_NONE;
	}

    void AIContinuousMarkCaster::excuteDeath(int skillLevel, CharacterData* trigger)
    {
        if (!isValid())
            return;

        if (innerMarkData && innerMarkData->deadExecID > 0)
            AIActionController::executeExec(aiContext, innerMarkData->deadExecID, skillID, skillLevel, trigger, projectile, target, 0, false, markID);
        if (innerMarkData && innerMarkData->deadEffectID > 0)
            AIActionController::executeExec(aiContext, innerMarkData->deadEffectID, skillID, skillLevel, trigger, projectile, target, 0, false, markID);
        if (innerMarkData && innerMarkData->deadSoundID > 0)
            AIActionController::executeExec(aiContext, innerMarkData->deadSoundID, skillID, skillLevel, trigger, projectile, target, 0, false, markID);
    }

    bool AIContinuousMarkCaster::isParamExistInParams(const Params& theParams, ParamType theType, ParamID theKey)
    {
        auto iterValues = theParams.find(theType);
        if (iterValues != theParams.end())
        {
            auto& theValues = iterValues->second;
            auto iterFound = std::find(theValues.begin(), theValues.end(), theKey);
            return (iterFound != theValues.end());
        }
        return false;
    }
	
	bool AIContinuousMarkCaster::isDeadSomeOneParamExist(const Params& theParams, ParamType theType)
	{
		bool hasFound = false;
		

		auto iterValues = theParams.find(theType);
		if (iterValues != theParams.end())
		{
			std::list<CharacterData*> *targetList = nullptr;
			auto& theValues = iterValues->second;

			if (theValues.size() > 1)
			{
				int nCharacterType = theValues[0];
				bool bIsIncludeApport = (bool)theValues[1];

				if (!bIsIncludeApport)
				{
					if (nCharacterType == kTriggerFirendType)
					{
						targetList = target->getCurrentFriendsList();
					}
					else if (nCharacterType == kTriggerEnemyType)
					{
						targetList = target->getCurrentRivalsList();
					}
					else if (nCharacterType == kTriggerAllType)
					{
						targetList = target->getCurrentFriendsList();
						for (auto iter = target->getCurrentRivalsList()->begin(); iter != target->getCurrentRivalsList()->end(); ++iter)
						{
							if ((!(*iter)->isSummon()) && HeroActionController::isDeadAction((*iter)->currentAction))
							{
								return true;
							}
						}
					}

					for (auto iter = targetList->begin(); iter != targetList->end(); ++iter)
					{
						if ((!(*iter)->isSummon()) && HeroActionController::isDeadAction((*iter)->currentAction))
						{
							return true;
						}
					}
				}
				else
				{
					if (nCharacterType == kTriggerFirendType)
					{
						targetList = target->getCurrentFriendsList();
					}
					else if (nCharacterType == kTriggerEnemyType)
					{
						targetList = target->getCurrentRivalsList();
					}
					else if (nCharacterType == kTriggerAllType)
					{
						targetList = target->getCurrentFriendsList();
						for (auto iter = target->getCurrentRivalsList()->begin(); iter != target->getCurrentRivalsList()->end(); ++iter)
						{
							if (HeroActionController::isDeadAction((*iter)->currentAction))
							{
								return true;
							}
						}
					}

					for (auto iter = targetList->begin(); iter != targetList->end(); ++iter)
					{
						if (HeroActionController::isDeadAction((*iter)->currentAction))
						{
							return true;
						}
					}
				}
			}

		}

		return hasFound;
	}

	bool AIContinuousMarkCaster::isTriggerParam(TriggerType triggerType, int paramA, int paramB, int paramC)
    {
        if (!innerMarkData)
            return false;

        const auto& params = (TriggerType::kEffect == triggerType) ? _triggerEffectParams : _triggerDeathParams;
        if (!params.size())
            return false;

        if ((TriggerType::kEffect == triggerType && EMarkEffectTriggerType::eMarkEffectTriggerAttackByHurt == innerMarkData->triggerType) ||
			(TriggerType::kEffect == triggerType && EMarkEffectTriggerType::eMarkEffectTriggerByFriendHurtType == innerMarkData->triggerType) ||
			(TriggerType::kDeath == triggerType && EMarkEffectDeadType::eMarkEffectDeadByHurt == innerMarkData->deadType) ||
            (TriggerType::kEffect == triggerType && EMarkEffectTriggerType::eMarkEffectTriggerByOwnerCastHurt == innerMarkData->triggerType)
			)
        {
            bool hasFound = false;
            if (/*paramA >= 0 && */params.find(ParamType::kHurtType) != params.end())
            {
                hasFound = isParamExistInParams(params, ParamType::kHurtType, paramA);
                if (!hasFound)
                    return false;
            }

            if (paramB >= 0)
            {
                hasFound = isParamExistInParams(params, ParamType::kHurtProperty, paramB);
            }

            return hasFound;
        }
		else if (TriggerType::kEffect == triggerType && EMarkEffectTriggerType::eMarkEffectTriggerByReachLevelAndDead == innerMarkData->triggerType)
		{
			bool hasFound = false; 

			hasFound = isDeadSomeOneParamExist(params, ParamType::kNormalParam);

			return hasFound;
		}
		else if (TriggerType::kEffect == triggerType && EMarkEffectTriggerType::eMarkEffectTriggerBySingleHurtNum == innerMarkData->triggerType)
		{
			bool hasFound = false;
			
			hasFound = isSingleHurtParamTrigger(params, ParamType::kNormalParam, paramA, paramB);
			
			return hasFound;
		}
		else if (TriggerType::kEffect == triggerType && EMarkEffectTriggerType::eMarkEffectTriggerByReachLevelMark == innerMarkData->triggerType)
		{
			bool hasFound = false;

			hasFound = isTypeLevelMarkParamExistInParams(params, ParamType::kNormalParam, paramA, paramB);

			return hasFound;
		}
		else if (TriggerType::kEffect == triggerType && EMarkEffectTriggerType::eMarkEffectTriggerByMarkedSomeoneHurted == innerMarkData->triggerType)
		{
			bool hasFound = false;

			if (paramC != kInvalidParamID)
			{
				auto theCharacter = aiContext.getCharacterByID(paramC);

				auto iterValues = params.find(ParamType::kNormalParam);
				if (iterValues != params.end())
				{
					auto& theMarkids = iterValues->second;
					for (int i = 0; i < theMarkids.size(); i++)
					{
						int markID = theMarkids[i];
						if (theCharacter->hasMark(markID))
						{
							hasFound = true;
							break;
						}
					}
				}

                if (!hasFound)
                    return false;

				if (params.find(ParamType::kHurtType) != params.end())
				{
					hasFound = isParamExistInParams(params, ParamType::kHurtType, paramA);
					if (!hasFound)
						return false;
				}

				if (paramB >= 0)
				{
					hasFound = isParamExistInParams(params, ParamType::kHurtProperty, paramB);
				}
			}

			return hasFound;
		}
        else
        {
            return isParamExistInParams(params, ParamType::kNormalParam, paramA);
        }

        return false;
    }

    void AIContinuousMarkCaster::removeTriggeredEffectCasts()
    {
        if (_triggeredEffects.size() && target->effectCasts.size())
        {
            std::vector<AIContinuousEffectCaster*> effectsToBeDeleted;
            for (auto iter = target->effectCasts.begin(); iter != target->effectCasts.end(); ++iter)
            {
                auto effectCast = *iter;
                if (effectCast && _triggeredEffects.find(effectCast->myID) != _triggeredEffects.end())
                    effectsToBeDeleted.push_back(effectCast);
            }

            for (auto iter = effectsToBeDeleted.begin(); iter != effectsToBeDeleted.end(); ++iter)
                //delete *iter;
                (*iter)->shouldDeleteSelf = true;

            _triggeredEffects.clear();
        }
    }

    void AIContinuousMarkCaster::removeTriggeredHurtCasts()
    {
        if (_triggeredHurts.size() && target->hurtCasts.size())
        {
            for (auto iter = target->hurtCasts.begin(); iter != target->hurtCasts.end(); ++iter)
            {
                auto pCast = *iter;
                if (pCast && _triggeredHurts.find(pCast->myID) != _triggeredHurts.end())
                    pCast->shouldDeleteSelf = true;
            }

            _triggeredHurts.clear();
        }
    }

    void AIContinuousMarkCaster::setEffectExecData(MarkEffectExec *data)
    {

        innerMarkData = data;

        _triggerEffectParams.clear();
        _triggerDeathParams.clear();


        if (!innerMarkData)
            return;

        if (EMarkEffectTriggerType::eMarkEffectTriggerAttackBySkill == innerMarkData->triggerType ||
            EMarkEffectTriggerType::eMarkEffectTriggerUseSkill == innerMarkData->triggerType ||
			EMarkEffectTriggerType::eMarkEffectTriggerByMark == innerMarkData->triggerType ||
			EMarkEffectTriggerType::eMarkEffectTriggerByReachLevelAndDead == innerMarkData->triggerType ||
			EMarkEffectTriggerType::eMarkEffectTriggerBySomeoneMark == innerMarkData->triggerType ||
			EMarkEffectTriggerType::eMarkEffectTriggerByReachLevelMark == innerMarkData->triggerType
			)
        {
            for (auto iter = innerMarkData->triggers.begin(); iter != innerMarkData->triggers.end(); ++iter)
            {
                auto param = *iter;
                if (param > 0)
                    _triggerEffectParams[ParamType::kNormalParam].push_back(param);
            }
        }
		else if (EMarkEffectTriggerType::eMarkEffectTriggerBySingleHurtNum == innerMarkData->triggerType)
		{
			for (auto iter = innerMarkData->triggers.begin(); iter != innerMarkData->triggers.end(); ++iter)
			{
				auto param = *iter;
				if (param >= 0)
					_triggerEffectParams[ParamType::kNormalParam].push_back(param);
			}
		}
        else if (EMarkEffectTriggerType::eMarkEffectTriggerAttackByHurt == innerMarkData->triggerType ||
                 EMarkEffectTriggerType::eMarkEffectTriggerByOwnerCastHurt == innerMarkData->triggerType)
        {
            // param:[a,b,c...]
            // a:伤害类型 -1-所有 0-物理 1-法术 3-纯粹
            // b,c,...:伤害属性
            ParamType type = ParamType::kHurtType;
            for (int i = 0; i < innerMarkData->triggers.size(); i++)
            {
                type = i ? ParamType::kHurtProperty : ParamType::kHurtType;
				if (type == ParamType::kHurtType && innerMarkData->triggers[i] == -1)
				{
					_triggerEffectParams[type].push_back(HurtType::PHYSICAL_HURT_TYPE);
					_triggerEffectParams[type].push_back(HurtType::MAGICAL_HURT_TYPE);
					_triggerEffectParams[type].push_back(3);
				}
				else if (innerMarkData->triggers[i] >= 0)
				{
					_triggerEffectParams[type].push_back(innerMarkData->triggers[i]);
				}
            }
        }
		else if (EMarkEffectTriggerType::eMarkEffectTriggerByFriendHurtType == innerMarkData->triggerType)
		{
			ParamType type = ParamType::kHurtType;

			for (int i = 0; i < innerMarkData->triggers.size(); i++)
			{
				type = i ? ParamType::kHurtProperty : ParamType::kHurtType;
				if (innerMarkData->triggers[i] >= 0)
					_triggerEffectParams[type].push_back(innerMarkData->triggers[i]);
			}
		}
		else if (EMarkEffectTriggerType::eMarkEffectTriggerByMarkedSomeoneHurted == innerMarkData->triggerType)
		{
			ParamType type = ParamType::kHurtType;
            const auto paramSize = innerMarkData->triggers.size();
            if (paramSize > 1)
            {
                for (int i = 0; i < paramSize - 1; i++)
                {
                    // 参数最后一位作为标记ID参数，不与其他配置重合
                    type = i ? ParamType::kHurtProperty : ParamType::kHurtType;
                    if (type == ParamType::kHurtType && innerMarkData->triggers[i] == -1)
                    {
                        _triggerEffectParams[type].push_back(HurtType::PHYSICAL_HURT_TYPE);
                        _triggerEffectParams[type].push_back(HurtType::MAGICAL_HURT_TYPE);
                        _triggerEffectParams[type].push_back(3);
                    }
                    else if (innerMarkData->triggers[i] >= 0)
                    {
                        _triggerEffectParams[type].push_back(innerMarkData->triggers[i]);
                    }
                }
            }
            
            if (paramSize > 0)
            {
                int theMarkId = innerMarkData->triggers[paramSize - 1];
                _triggerEffectParams[ParamType::kNormalParam].push_back(theMarkId);
            }
		}


        if (EMarkEffectDeadType::eMarkEffectDeadBySkill == innerMarkData->deadType)
        {
            for (auto iter = innerMarkData->deadParams.begin(); iter != innerMarkData->deadParams.end(); ++iter)
            {
                auto param = *iter;
                if (param > 0)
                    _triggerDeathParams[ParamType::kNormalParam].push_back(param);
            }
        }
        else if (EMarkEffectDeadType::eMarkEffectDeadByHurt == innerMarkData->deadType)
        {
            ParamType type = ParamType::kHurtType;
            for (int i = 0; i < innerMarkData->deadParams.size(); i++)
            {
                type = i ? ParamType::kHurtProperty : ParamType::kHurtType;
                if (type == ParamType::kHurtType && innerMarkData->deadParams[i] == -1)
                {
                    _triggerDeathParams[type].push_back(HurtType::PHYSICAL_HURT_TYPE);
                    _triggerDeathParams[type].push_back(HurtType::MAGICAL_HURT_TYPE);
					_triggerDeathParams[type].push_back(3);
                }
                else if (innerMarkData->deadParams[i] >= 0)
                {
                    _triggerDeathParams[type].push_back(innerMarkData->deadParams[i]);
                }
            }
        }
		
		
    }

    void AIContinuousMarkCaster::onCharacterDead()
    {
		if (this && innerMarkData)
		{
			if (EMarkEffectDeadType::eMarkEffectDeadWhenHeroDead == innerMarkData->deadType
				|| EMarkEffectDeadType::eMarkEffectDeadByAllReplaceDeadTime == innerMarkData->deadType)
			{
				excuteDeath(this->skillLevel, origin);
				invalidate();
			}
		}
    }

    // 角色死亡后,触发其身上的标记执行,条件:标记触发条件为达到层数,有人死亡触发
	void AIContinuousMarkCaster::checkExcuteAfterCharcterDeadReleaseTrigger(CharacterData* theCharacterData)
	{
		for (auto iter = theCharacterData->markCasts.begin(); iter != theCharacterData->markCasts.end(); ++iter)
		{
			int result = MarkExcuteResult::EXCUTE_RESULT_NONE;
			bool shouldRemoveMark = false;
			auto caster = *iter;

			if (caster->innerMarkData && EMarkEffectTriggerType::eMarkEffectTriggerByReachLevelAndDead == caster->innerMarkData->triggerType)
			{
				if (caster->isTriggerParam(TriggerType::kEffect, kInvalidParamID, kInvalidParamID))
				{ 
					CharacterData *pTrigger = nullptr;

					pTrigger = (caster->innerMarkData->triggerEftSelectType == EMarkTriggerEffectSelectType::eMarkTriggerEffectByMarkCaster) ? theCharacterData : theCharacterData;

					result = caster->excuteEffect(caster->skillLevel, pTrigger);
				}

				if (MarkExcuteResult::EXCUTE_RESULT_REMOVE_MARK == result)
				{
					shouldRemoveMark = true;
				}

				if (shouldRemoveMark)
				{
                    (*iter)->invalidate();
				}
			}
		}
	}

    void AIContinuousMarkCaster::checkDoCharacterDead(AIContext &aiContext, CharacterData *c)
    {
        c->marksOnSelfEnterDead();

        CharacterDataListPtr currentOurCluster = aiContext.getClusterOrigin(TeamSide::kOur, aiContext.current_ClusterNumOur);
        CharacterDataListPtr currentRivals = aiContext.getClusterOrigin(TeamSide::kEnemy, aiContext.current_ClusterNumEnemy);

        MarkInnerHelper::RespondToCharacterDead(currentOurCluster, c);
        MarkInnerHelper::RespondToCharacterDead(currentRivals, c);
    }

    void AIContinuousMarkCaster::checkTriggerOnCharacterDead(AIContext &aiContext, CharacterData *c)
    {
        CharacterDataListPtr currentOurCluster = aiContext.getClusterOrigin(TeamSide::kOur, aiContext.current_ClusterNumOur);
        CharacterDataListPtr currentRivals = aiContext.getClusterOrigin(TeamSide::kEnemy, aiContext.current_ClusterNumEnemy);

        MarkInnerHelper::CheckTriggerCharacterDead(currentOurCluster, c);
        MarkInnerHelper::CheckTriggerCharacterDead(currentRivals, c);
    }
    
	bool AIContinuousMarkCaster::isSingleHurtParamTrigger(const Params& theParams, ParamType theType, ParamID theKeyA, ParamID theKeyB)
	{
		bool bReturn = false;

/*		if (theKeyA <= 0)
		{
			return false;
		}*/

		auto iterValues = theParams.find(theType);
		if (iterValues != theParams.end())
		{
			auto& theValues = iterValues->second;

			if (theValues.size() > 1)
			{
				int bIsPct = theValues[0];
				float nNum = theValues[1];
				nNum /= 100;

				if (bIsPct == 0)
				{
					if (theKeyA >= nNum)
					{
						return true;
					}
				}
				else
				{
					if (theKeyA >= (target->getHPTop() * (nNum / 100)))
					{
						return true;
					}
				}
			}
		}

		return bReturn;
	}

	bool MathEqualFunc(int CompareNum, int LogicSymbol, int Num)
	{
		switch (LogicSymbol)
		{
		case AIContinuousMarkCaster::E_MathLogicSymbolType::eMathGatherThan:
		{
			return (CompareNum > Num);
		}
			break;
		case AIContinuousMarkCaster::E_MathLogicSymbolType::eMathGatherOrEqualTo:
		{
			return (CompareNum >= Num);
		}
			break;
		case AIContinuousMarkCaster::E_MathLogicSymbolType::eMathEqualTo:
		{
			return (CompareNum == Num);
		}
			break;
		case AIContinuousMarkCaster::E_MathLogicSymbolType::eMathLessOrEqualTo:
		{
			return (CompareNum <= Num);
		}
			break;
		case AIContinuousMarkCaster::E_MathLogicSymbolType::eMathLessThan:
		{
			return (CompareNum < Num);
		}
			break;
		default:
			break;
		}

		return false;
	}

	bool AIContinuousMarkCaster::isTypeLevelMarkParamExistInParams(const Params& theParams, ParamType theType, ParamID theKeyA, ParamID theKeyB)
	{
		bool bReturn = false;

		if (theKeyA <= 0)
		{
			return false;
		}

		auto iterValues = theParams.find(theType);
		if (iterValues != theParams.end())
		{
			auto& theValues = iterValues->second;

			if (theValues.size() > 2)
			{
				int ComPareID = theValues[0];
				int nLogic1 = theValues[1];
				int nNum1 = theValues[2];
				int nLogic2 = -1, nNum2 = -1;

				if (theValues.size() > 4)
				{
					nLogic2 = theValues[3];
					nNum2 = theValues[4];
				}
				
				// 标记ID
				if (ComPareID == theKeyA)
				{
					if (theValues.size() > 4)
					{
						bReturn = MathEqualFunc(theKeyB, nLogic1, nNum1) && MathEqualFunc(theKeyB, nLogic2, nNum2);
					}
					else if (theValues.size() > 2)
					{
						bReturn = MathEqualFunc(theKeyB, nLogic1, nNum1);
					}
				}	
			}
		}

		return bReturn;
	}

    void AIContinuousMarkCaster::reshowUIEffect()
    {
        if (aiContext.markProxy && innerMarkData && innerMarkData->iconID)
            aiContext.markProxy->executeExec(aiContext, target, innerMarkData->iconID, _markLevel);
    }

	bool AIContinuousMarkCaster::isHasCoverMarkFlag(CharacterData* target, int theType)
	{
		bool bHasCoverFlag = false;
		for (auto iter = target->markCasts.begin(); iter != target->markCasts.end(); ++iter)
		{
			auto pMark = *iter;
			for (auto itt = pMark->staticData->coverTypes.begin(); itt != pMark->staticData->coverTypes.end(); ++itt)
			{
				if ((*itt) == theType)
				{
					bHasCoverFlag = true;
					break;
				}
			}

			if (bHasCoverFlag)
			{
				break;
			}
		}

		return bHasCoverFlag;
	}


	void AIContinuousMarkCaster::checkExcuteEffectTriggerdByReachLevelMark(CharacterData* target, int markID, AIContinuousMarkCaster* markInst /*= nullptr*/)
	{
		for (auto iter = target->markCasts.begin(); iter != target->markCasts.end(); ++iter)
		{
			auto caster = *iter;

			if (!caster->isValid())
				continue;

			if (markInst && markInst->myID == caster->myID)
			{
				continue;
			}

			int result = MarkExcuteResult::EXCUTE_RESULT_NONE;

			if (caster->innerMarkData && EMarkEffectTriggerType::eMarkEffectTriggerByReachLevelMark == caster->innerMarkData->triggerType)
			{
				if (caster->isTriggerParam(TriggerType::kEffect, markInst->markID, markInst->getMarkLevel()))
				{
					CharacterData* pTrigger = nullptr;

					pTrigger = (caster->innerMarkData->triggerEftSelectType == EMarkTriggerEffectSelectType::eMarkTriggerEffectByMarkCaster) ? caster->origin : markInst->origin;

					result = caster->excuteEffect(caster->skillLevel, pTrigger);
				}
			}

			if (MarkExcuteResult::EXCUTE_RESULT_REMOVE_MARK == result)
			{
				caster->invalidate();
			}

		} // END for
	}
} // namespace OnePiece
