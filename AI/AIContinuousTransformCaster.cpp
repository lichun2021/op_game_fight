//
//  AIContinuousTransformCaster.cpp
//  op_game
//
//  Created by h7ing on 15/9/25.
//
//

#include "AIContinuousTransformCaster.h"
#include "CommonType.h"
#include "AICharacterData.h"
#include "AIContext.h"
#include "AIActionController.h"
#include "action_hero/HeroActionController.h"
#include "AITargetHelper.h"
#include "AIScriptBinder.hpp"

namespace OnePiece {
    
void AIContinuousTransformCaster::initTransformTargetContext(AITargetContext &ctx, AIContinuousTransformCaster *caster)
{
    ctx.source = caster->_pTarget;
    if (caster->_pTarget)
        ctx.target = caster->_pTarget->target;
    ctx.skill_caster = caster->_pSource;
    ctx.projectile = nullptr;
    ctx.skill_id = caster->_skillID;
    ctx.skill_level = caster->_skillLevel;
    ctx.hurt_id = 0;
    
    auto caster_data = caster->_staticData;
    ctx.search_type = caster_data->target;
    ctx.range.x1 = caster_data->range[0];
    ctx.range.x2 = caster_data->range[1];
    ctx.range.y1 = caster_data->range[2];
    ctx.range.y2 = caster_data->range[3];
    ctx.search_params = nullptr;
    ctx.filter_list = nullptr;
}

AIContinuousTransformCasterPtr AIContinuousTransformCaster::create(int transformID, AIContext& aiContext, CharacterData* source, CharacterData* target, int skillID, int skillLevel)
{
    auto ret = AIContinuousTransformCasterPtr(new AIContinuousTransformCaster(transformID, aiContext, source, target, skillID, skillLevel));
    target->appendTransformCast(ret);
    return ret;
}

AIContinuousTransformCaster::~AIContinuousTransformCaster()
{
}

AIContinuousTransformCaster::AIContinuousTransformCaster(int transformID, AIContext& aiContext, CharacterData* source, CharacterData* target, int skillID, int skillLevel)
: AINode(source->myID, source->clusterNum), _aiContext(aiContext), _transformID(transformID),
  _pSource(source), _pTarget(target), _skillID(skillID), _skillLevel(skillLevel)
{
    myID = aiContext.local_incremential++;

    _staticData = AIDataHelper::getTransformExec(transformID);

    localTick = aiContext.currentTick;
    startTick = localTick;
    totalTick = _staticData->transform.duration;

    if (source->attackSpeed != 100 && source->attackSpeed > 0 && target && source->myID == target->myID && AIScriptBinder::getInstance()->isCasterSyncAttackSpeed(aiContext.getLuaState(), transformID))
    {
        totalTick = totalTick * source->attackSpeed / 100;
    }

    _pOwner = target;
    if (target && target->isCaptain())
        ignoreFreeze = true;

   
}

void AIContinuousTransformCaster::stop()
{

    if (_isStopped) return;
    _isStopped = true;

    for (auto aTarget : _affectedTargets)
    {
        aTarget->staticData = AIDataHelper::getCharacterData(aTarget->initCharacterID);
        aTarget->setID(aTarget->initCharacterID);
        if (aTarget->staticData->sk[0])
        {
            aTarget->normalSkill.first = aTarget->staticData->sk[0];
        }

        aTarget->exitTransformState();
        if (_aiContext.transformProxy && !HeroActionController::isDeadAction(aTarget->currentAction))
        {
            if (_staticData->transformType == 1)
            {
                _aiContext.transformProxy->transformToCreature(_aiContext, aTarget, aTarget->getID());
            }
            else if (_staticData->transformType == 2)
            {
                _aiContext.transformProxy->exitTransform(aTarget);
            }
        }
        //if (_aiContext.transformProxy && !HeroActionController::isDeadAction(aTarget->currentAction))
        //    _aiContext.transformProxy->transformToCreature(_aiContext, aTarget, aTarget->getID());


        for (auto execID : _staticData->transform.execIDVecWhenEnd)
            AIActionController::executeExec(_aiContext, execID, _skillID, _skillLevel, _pSource, nullptr, aTarget, 0);
    }
}

void AIContinuousTransformCaster::update(AIContext& aiContext)
{
    AINode::update(aiContext);
    if (isFrozen)
        return;

    int deltaTick = localTick - startTick - 1;

    if ((totalTick == 0 && deltaTick > totalTick) || (totalTick != 0 && deltaTick >= totalTick))
    {
        stop();
        if (_pOwner)
            _pOwner->removeTransformCast(myID);
        
        return;
    }

    if (deltaTick != _staticData->interval)
        return;
    
    if (AITargetHelper::isUseNewHelper()) {
        AITargetContext ctx;
        initTransformTargetContext(ctx, this);
        std::list<CharacterData *> targets;
        AITargetHelper::searchTarget(&ctx, &targets);
        
        for (auto aTarget : targets) {
            castOnTarget(aiContext, aTarget);
        }
            
    } else {
        auto targets = AIActionController::searchTarget(*_pTarget, _pTarget->target, nullptr, aiContext, _staticData->target, _staticData->range[0], _staticData->range[1], _staticData->range[2], _staticData->range[3], _staticData->range[2], _staticData->range[3], nullptr, *_pSource);
        if (targets == nullptr)
            return;
        
        for (auto aTarget : *targets)
        {
            castOnTarget(aiContext, aTarget);
        }
    }
}
    
void AIContinuousTransformCaster::castOnTarget(AIContext &aiContext, CharacterData *aTarget)
{
    if (aiContext.localRandom->nextInt(0, 100) >= _staticData->rate)
        return;
    
    _affectedTargets.insert(aTarget);
    aTarget->enterTransformState();
    if (aiContext.transformProxy)
    {
        if (_staticData->transformType == 1)
        {
            aiContext.transformProxy->createTransformCreature(aiContext, aTarget, _staticData->transform.animation);
            aiContext.transformProxy->transformToCreature(aiContext, aTarget, _staticData->transform.animation);
        }
        else if (_staticData->transformType == 2)
        {
            aiContext.transformProxy->enterTransformToCharacter(aiContext, aTarget, _staticData->transform.animation);
        }
    }
    
    if (_staticData->newTransform) {
        aTarget->staticData = AIDataHelper::getCharacterData(_staticData->transform.animation);
        aTarget->setID(aTarget->staticData->id);
        if (aTarget->staticData->sk[0])
        {
            aTarget->normalSkill.first = aTarget->staticData->sk[0];
        }
    }


    if (!_staticData->transform.keepMarks)
        //aTarget->removeMarks();
        aTarget->invalidateMarks();
    
    if (!_staticData->transform.keepEffects)
        aTarget->resetEffectExec();
    
    if (!_staticData->transform.keepHP)
        aTarget->updateHP(aTarget->getHPTop());
    
    for (auto execID : _staticData->transform.execIDVecWhenTrigger)
        AIActionController::executeExec(_aiContext, execID, _skillID, _skillLevel, _pSource, nullptr, aTarget, 0);

}

} // namespace OnePiece
