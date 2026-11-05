//
//  CommonType.cpp
//  OnePiece
//
//  Created by Jianguo Yang on 14-4-15.
//
//

#include "CommonType.h"
#include "AIDataManager.h"
#include "stdarg.h"
#include "json/stringbuffer.h"
#include "json/writer.h"


namespace OnePiece
{
#ifdef DENA_OP3
    const std::string AIDataHelper::system_config = "_system_config";
    const std::string AIDataHelper::skill_list = "_skill_list";
    const std::string AIDataHelper::skill_animation = "_skill_animation";
    const std::string AIDataHelper::skill_ui_effect = "_skill_ui_effect";
    const std::string AIDataHelper::skill_ui_effect_res = "_skill_ui_effect_res";
    const std::string AIDataHelper::skill_move = "_skill_move";
    const std::string AIDataHelper::skill_hurt = "_skill_hurt";
    const std::string AIDataHelper::skill_label_effect = "_skill_label_effect";
    const std::string AIDataHelper::skill_audio = "_skill_audio";
    const std::string AIDataHelper::character_position = "_character_position";
    const std::string AIDataHelper::fighting_parameter = "_fighting_parameter";
    const std::string AIDataHelper::skill_composite_animation = "_skill_composite_animation";
    const std::string AIDataHelper::skill_composite_ui_effect = "_skill_composite_ui_effect";
    const std::string AIDataHelper::skill_projectile = "_skill_projectile";
    const std::string AIDataHelper::domineer_site_list_hash = "_domineer_site_list_hash";
    const std::string AIDataHelper::character_list = "_character_list";
    const std::string AIDataHelper::pve_guanqia_characters = "_pve_guanqia_characters";
    const std::string AIDataHelper::skill_special = "_skill_special";
    const std::string AIDataHelper::skill_body_effect = "_skill_body_effect";
    const std::string AIDataHelper::pve_guanqia_hash = "_pve_guanqia_hash";
    const std::string AIDataHelper::mark_list = "_mark_list";
    const std::string AIDataHelper::mark_effect = "_mark_effect";
    const std::string AIDataHelper::skill_summon = "_skill_summon";
    const std::string AIDataHelper::skill_summon_attr = "_skill_summon_attr";
    const std::string AIDataHelper::skill_transform = "_skill_transform";
    const std::string AIDataHelper::skill_transform_attr = "_skill_transform_attr";
    const std::string AIDataHelper::skill_effect = "_skill_effect";
    const std::string AIDataHelper::combo_animation = "_combo_animation";
    const std::string AIDataHelper::combo_audio = "_combo_audio";
    const std::string AIDataHelper::combo_hit_num = "_combo_hit_num";
    const std::string AIDataHelper::combo_trigger = "_combo_trigger";
    const std::string AIDataHelper::convert_attr_list = "_convert_attr_list";
#endif // DENA_OP3

	int AIDataHelper::s_aiUpdateTimePerSecond = 20;
	int* AIDataHelper::_character_pos = nullptr;
    std::unordered_map<int, MoveExec*> AIDataHelper::_moveExec_cache;
    std::unordered_map<int, HurtExec*> AIDataHelper::_hurtExec_cache;
    std::unordered_map<int, ProjectileExec*> AIDataHelper::_projectileExec_cache;
    std::unordered_map<int, AISkill*> AIDataHelper::_aiSkill_cache;
    std::unordered_map<int, CharData*> AIDataHelper::_characterDataMap;
    std::unordered_map<int, SpecialUIEffectExec*> AIDataHelper::_specialEffExecMap;
    std::unordered_map<int, AnimationExec*> AIDataHelper::_animationExecMap;
    std::unordered_map<std::string, SpecialUIEffectData*> AIDataHelper::_effDataMap;
    std::unordered_map<int, LabelEffectExec*> AIDataHelper::_labelEffExecMap;
    std::unordered_map<int, std::vector<SkillExec>*> AIDataHelper::_skillExecVecMap;
    std::unordered_map<int, EffectExec*> AIDataHelper::_effectExecMap;
    std::unordered_map<int, MarkExec*> AIDataHelper::_markExecMap;
    std::unordered_map<int, MarkEffectExec*> AIDataHelper::_markEffectExecMap;
    std::unordered_map<int, std::unordered_map<int, MarkEffectExec*> > AIDataHelper::_markLevelMap; // 改进的标记层数表 {_type:{level:markeffect}}
    std::unordered_map<int, SummonExec*> AIDataHelper::_summonExecMap;
    std::unordered_map<int, SummonProperty*> AIDataHelper::_summonPropertyMap;
    std::unordered_map<int, TransformExec*> AIDataHelper::_transformExecMap;
    std::unordered_map<int, BodyUIEffectExec*> AIDataHelper::_bodyEffectExecMap;
    
    
    std::unordered_map<std::string, BaqiSpecialExec*> AIDataHelper::_baqiSpecialExecMap;
    std::unordered_map<int, SoundExec*> AIDataHelper::_soundExecMap;
    std::unordered_map<int, LogicAnimationExec*> AIDataHelper::_logicAniMap;
    std::unordered_map<int, LogicSpecialUIEffectExec*> AIDataHelper::_logicEffMap;
    std::unordered_map<int, SpecialExec*> AIDataHelper::_specialExecMap;
    std::unordered_map<int, ComboTrigger*> AIDataHelper::_comboTriggerMap;
    std::unordered_map<int, ComboAnimationExec*> AIDataHelper::_comboAniMap;
    std::unordered_map<int, ComboAudioExec*> AIDataHelper::_comboAudioMap;
    std::unordered_map<int, ComboHitExec*> AIDataHelper::_comboHitMap;
    std::unordered_map<int, std::pair<int, int>> AIDataHelper::_beHittedAddEnergyMap;

    std::unordered_map<int, pve_guanqia_character*> AIDataHelper::_pve_guanqia_character_Map;
    std::unordered_map<int, std::vector<RewardCandidateItem*>*> AIDataHelper::_rewardCandidateMap;
    std::unordered_map<int, std::vector<RewardCandidateItem*>*> AIDataHelper::_pve_guanqia_character_reward_Map;
    
    std::vector<ConvertAttrParams> AIDataHelper::_convertAttrVec;
    

	int AIDataHelper::energyRecover[2] = { 0,0 };
	int AIDataHelper::evenPosition[5] = { -1, -1, -1, -1, -1 };
	int AIDataHelper::oddPosition[5] = { -1, -1, -1, -1, -1 };

	float* AIDataHelper::fightingParameter[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int AIDataHelper::battleFieldRangeToCenterX = -1;
    int AIDataHelper::aiUpdateModeAfterWinlose = -1;
    int AIDataHelper::aiCenterXMode = -1;
    int AIDataHelper::aiUltraBrokenMode = -1;
    int AIDataHelper::aiUiEffectMode = -1;
    std::vector<int> *AIDataHelper::aiCenterXCache = nullptr;

	std::string stringWithFormat(const char *pFormat, ...)
	{
		va_list argptr;
		va_start(argptr, pFormat);

		char buffer[256] = { 0 };
		vsprintf(buffer, pFormat, argptr);
		va_end(argptr);

		return std::string(buffer);
	}

	std::string AIDataHelper::scriptGetConfigData(int data_type, int data_id)
	{
		if (CONFIGDATA_CHARACTERDATA == data_type) {
			return AIDataHelper::getCharacterDataJsonStr(data_id);
		}
		else if (CONFIGDATA_SKILLDATA == data_type) {
			return AIDataHelper::getSkillDataJsonStr(data_id);
		}
		else if (CONFIGDATA_EFFECTEXEC == data_type) {
			return AIDataHelper::getEffectExecJsonStr(data_id);
		}
		else if (CONFIGDATA_HURTEXEC == data_type) {
			return AIDataHelper::getHurtExecJsonStr(data_id);
		}
		else if (CONFIGDATA_PROJECTILEEXEC == data_type) {
			return AIDataHelper::getProjectileExecJsonStr(data_id);
		}
		else if (CONFIGDATA_BAQISPECIALEXEC == data_type) {
			return AIDataHelper::getBaqiSpecialExecJsonStr(data_id);
		}
		else if (CONFIGDATA_SUMMONPROPERTY == data_type) {
			return AIDataHelper::getSummonPropertyJsonStr(data_id);
		}
        else if (CONFIGDATA_SUMMONEXEC == data_type) {
            return AIDataHelper::getSummonExecJsonStr(data_id);
        }

		return "";
	}

	std::vector<SkillExec>* AIDataHelper::getSkillExecVec(int skillId)
	{
		if (_skillExecVecMap.find(skillId) != _skillExecVecMap.end())
		{
			return _skillExecVecMap.find(skillId)->second;
		}
		
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_list.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_list");
#endif // DENA_OP3

		if (!doc)
			return nullptr;

		std::string idStr = stringWithFormat("%d", skillId);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
		rapidjson::Value& skillData = doc->operator[](idStr.c_str());
        if (skillData.IsNull())
        {
#ifdef WIN32
            idStr = "skill " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
		}
		std::vector<SkillExec> *execVec = new std::vector<SkillExec>();
		rapidjson::Value& array = skillData["exe"];

		int count = array.Size();
		for (int idx = 0; idx < count / 4; ++idx)
		{
			int i = idx * 4;
			SkillExec se;
			se.execId = array[i++].GetInt();
			se.execType = array[i++].GetInt();
			se.param0 = array[i++].GetInt();
			se.param1 = array[i++].GetInt();
			execVec->push_back(se);
		}

		_skillExecVecMap[skillId] = execVec;
		return execVec;
	}

	AnimationExec* AIDataHelper::getSkillAnimationExec(int execId)
	{
		if (_animationExecMap.find(execId) != _animationExecMap.end())
		{
			return (AnimationExec*)(_animationExecMap.find(execId)->second);
		}
		
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_animation.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_animation");
#endif // DENA_OP3	

		if (!doc)
			return nullptr;
		
		std::string idStr = stringWithFormat("%d", execId);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
		rapidjson::Value& data = doc->operator[](idStr.c_str());
        if (data.IsNull())
        {
#ifdef WIN32
            idStr = "skill_animation " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
		}
		AnimationExec* exec = new AnimationExec;
		exec->execId = execId;
		exec->name = data["n"].GetString();

		rapidjson::Value& resData = data["res"];
		exec->characterName = resData[0u].GetString();
		exec->aniName = resData[1].GetString();

		rapidjson::Value& charData = data["conf"];		
		exec->playSpeed = (int)charData[0u].GetInt();
		exec->playTime = (int)charData[1].GetInt();
		exec->duration = (int)charData[2].GetInt();
		exec->hardFrameTime = (int)charData[3].GetInt();
		exec->hardTime = (int)charData[4].GetInt();

		_animationExecMap[execId] = exec;

		return exec;
	}

	SpecialUIEffectExec* AIDataHelper::getSpecialUIEffectExec(int execId)
	{
		if (_specialEffExecMap.find(execId) != _specialEffExecMap.end())
		{
			return (SpecialUIEffectExec*)(_specialEffExecMap.find(execId)->second);
		}

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_ui_effect.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_ui_effect");
#endif // DENA_OP3	
		if (!doc)
			return nullptr;

		std::string idStr = stringWithFormat("%d", execId);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
		rapidjson::Value& data = doc->operator[](idStr.c_str());
        if (data.IsNull())
        {
#ifdef WIN32
            idStr = "skill_ui_effect " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
		}
		SpecialUIEffectExec* exec = new SpecialUIEffectExec;
		exec->execId = execId;
		exec->name = data["n"].GetString();

		rapidjson::Value& resData = data["res"];
		exec->specID = resData[0u].GetString();
        exec->spAnimName = resData[1].GetString();

		rapidjson::Value& charData = data["conf"];		
		exec->deltaX = (float)charData[0u].GetDouble();
		exec->deltaY = (float)charData[1].GetDouble();
		exec->deltaZ = (float)charData[2].GetDouble();
		exec->bFollow = (charData[3].GetInt());
        /// 12.0
        if (exec->bFollow == 3)
        {
            exec->bFollow = 0;
            exec->followUnitTurnType = 1;
        }
        ///
		exec->playTime = (int)charData[4].GetInt();
		exec->duration = (int)charData[5].GetInt();
		exec->bSync = charData[6].GetInt();
		exec->target = charData[7].GetInt();
		exec->scale = (charData[8].GetInt()) / 100.0f;
		exec->opacity = (unsigned char)(charData[9].GetInt());
		exec->rotation = charData[10].GetInt();
		if (charData[12].IsInt() && charData[13].IsInt() && charData[14].IsInt())
		{
			exec->r = charData[12].GetInt();
			exec->g = charData[13].GetInt();
			exec->b = charData[14].GetInt();
		}

		exec->duration2 = charData[15].GetInt();
        exec->flipX = charData[16].GetInt();
		
		exec->mStationaryMode = charData[17].GetInt();

		exec->absoluteX = charData[18].GetDouble();
		exec->absoluteY = charData[19].GetDouble();
		exec->absoluteZ = charData[20].GetDouble();

        // 8.0 add
        exec->canSpeedup = charData[21].GetInt();

        // 9.0 add
        if (charData.Size() > 22)
        {
            exec->keepWhenDead = charData[22].GetInt();
        }

		_specialEffExecMap[execId] = exec;

        //cui 2023_2_24
        rapidjson::Value& sounds = data["sounds"];
        if (sounds.IsArray()) {
            exec->delaySound.resize(sounds.Size());
            for (int idx = 0; idx < sounds.Size(); ++idx)
            {
                exec->delaySound[idx] = sounds[idx].GetInt();
            }
        }

		return exec;
	}

	SpecialUIEffectData* AIDataHelper::getSpecialUIEffectData(const std::string& effName)
	{
		if (_effDataMap.find(effName) != _effDataMap.end())
		{
			return (SpecialUIEffectData*)(_effDataMap.find(effName)->second);
		}
				

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_ui_effect_res.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_ui_effect_res");
#endif // DENA_OP3	

		if (!doc)
			return nullptr;
        if (!doc->HasMember(effName.c_str()))
            return nullptr;
		rapidjson::Value& data = doc->operator[](effName.c_str());
        if (data.IsNull())
        {
#ifdef WIN32
            std::string theLog = "skill_ui_effect_res " + effName + " not found!";
            //CCASSERT(0, theLog.c_str());
#endif
            return nullptr;
		}
		SpecialUIEffectData *effData = new SpecialUIEffectData;
		effData->name = effName;

		rapidjson::Value& resData = data["res"];
		effData->spriteName = resData[0u].GetString();
		effData->plistName = resData[1].GetString();
		effData->type = (SpecialEffectType)data["t"].GetInt();
		effData->blendType = (EBlendType)data["b"].GetInt();

		_effDataMap[effName] = effData;

		return effData;
	}
	
	MoveExec* AIDataHelper::getMoveExec(int execID)
	{
		if (_moveExec_cache.find(execID) != _moveExec_cache.end())
		{
			return (MoveExec*)(_moveExec_cache.find(execID)->second);
		}
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_move.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_move");
#endif // DENA_OP3	
		if (!doc)
			return nullptr;

		std::string idStr = stringWithFormat("%d", execID);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
		rapidjson::Value& data = doc->operator[](idStr.c_str());
        if (data.IsNull())
        {
#ifdef WIN32
            idStr = "move_exec " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
		}
		MoveExec *exec = new MoveExec();
		exec->execId = execID;
		exec->name = data["n"].GetString();
		rapidjson::Value& charData = data["conf"];
		exec->range.resize(4);
		exec->targetTracing = charData[0u].GetInt();
		exec->target = charData[1].GetInt();
		exec->range[0] = charData[2].GetInt();
		exec->range[1] = charData[3].GetInt();
		exec->range[2] = charData[4].GetInt();
		exec->range[3] = charData[5].GetInt();
		exec->duration = charData[6].GetInt();
		exec->speedX = charData[7].GetInt();
		exec->accX = charData[8].GetInt();
		exec->durationX = charData[9].GetInt();
		exec->targetDistanceX = charData[10].GetInt();
		exec->speedY = charData[11].GetInt();
		exec->accY = charData[12].GetInt();
		exec->durationY = charData[13].GetInt();
		exec->targetDistanceY = charData[14].GetInt();
		exec->needReturn = charData[15].GetInt();
		exec->returnPause = charData[16].GetInt();
		exec->keepWhenChangeAction = charData[17].GetInt();
				
		if (charData.Size() > 18)
		{
			exec->bIsUseactionFreq = charData[18].GetInt();
		}
		if (charData.Size() > 19)
		{
			exec->actionFreq = charData[19].GetInt();
		}

		rapidjson::Value& execs = data["exec"];
		exec->vAfterMoveExecs.resize(execs.Size());
		for (int idx = 0; idx < execs.Size(); ++idx)
		{
			exec->vAfterMoveExecs[idx] = execs[idx].GetInt();
		}
		
		// clair_wang
		rapidjson::Value& moveExeIds = data["movingExecuteID"];
		if (moveExeIds.IsArray())
		{
			exec->vMovingExecuteIds.resize(moveExeIds.Size());
			for (int idx = 0; idx < moveExeIds.Size(); ++idx)
			{
				exec->vMovingExecuteIds[idx] = moveExeIds[idx].GetInt();
			}
		}
		else
		{
			exec->vMovingExecuteIds.push_back(moveExeIds.GetInt());
		}
		// clair_wang
		if (data.HasMember("syncTargetMech"))
		{
			exec->syncTargetMech = data["syncTargetMech"].GetInt();
		}

		_moveExec_cache[execID] = exec;
		return exec;
	}

	int MoveExec::getRepeatCasterCnt()
	{
		int nTotalTIck = 0;
		int nMoveCnt = 1;
		if (targetTracing == 3)// (int)MoveType::kBlackHoleField)
		{
			nTotalTIck = duration + durationX;//from 0
		}
		else
		{
			nTotalTIck = duration;//from 0
		}

		if (bIsUseactionFreq > 0 && actionFreq > 0)
		{
			nMoveCnt = nTotalTIck / actionFreq;
		}

		return nMoveCnt;
	}

	std::string AIDataHelper::getHurtExecJsonStr(int execId)
	{
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_hurt.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_hurt");
#endif // DENA_OP3

		if (!doc)
			return "";
		
		std::string idStr = stringWithFormat("%d", execId);
        if (!doc->HasMember(idStr.c_str()))
            return "";
		rapidjson::Value& data = doc->operator[](idStr.c_str());
		if (data.IsNull())
			return "";

		rapidjson::StringBuffer buf;
        buf.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        
        data.Accept(writer);
        return buf.GetString();
	}

	HurtExec* AIDataHelper::getHurtExec(int execID)
	{
		if (_hurtExec_cache.find(execID) != _hurtExec_cache.end())
		{
			return (HurtExec*)(_hurtExec_cache.find(execID)->second);
		}
		
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_hurt.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_hurt");
#endif // DENA_OP3

		if (!doc)
			return nullptr;
        
		std::string idStr = stringWithFormat("%d", execID);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
		rapidjson::Value& data = doc->operator[](idStr.c_str());
        if (data.IsNull())
        {
#ifdef WIN32
            idStr = "hurt_exec " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
        }
        HurtExec* exec = new HurtExec();
		exec->ID = execID;
		exec->name = data["n"].GetString();
		rapidjson::Value& charData = data["conf"];
		exec->target = charData[0u].GetInt();
		exec->x1 = charData[1].GetInt();
		exec->x2 = charData[2].GetInt();
		exec->z1 = charData[3].GetInt();
		exec->z2 = charData[4].GetInt();
		exec->duration = charData[5].GetInt();
		exec->freq = charData[6].GetInt();
		exec->type = charData[7].GetInt();
		exec->property = charData[8].GetInt();
		exec->baseAPFactor = charData[9].GetInt();
		exec->APBonus = charData[10].GetInt();
		exec->baseExtraHurt = charData[11].GetInt();
		exec->extraHurtBonus = charData[12].GetInt();
		exec->hitExecCondition = charData[13].GetInt();
		exec->hitExecParam = charData[14].GetInt();
		//exec->hitExec1 = charData[15].GetInt();
		//exec->hitExec2 = charData[16].GetInt();
		//exec->hitExec3 = charData[17].GetInt();
        exec->hitExecs.push_back(charData[15].GetInt());
        exec->hitExecs.push_back(charData[16].GetInt());
        exec->hitExecs.push_back(charData[17].GetInt());
		exec->hitStunP = charData[18].GetInt();
		exec->hitSpecialEffect = charData[19].GetInt();
		exec->hitAudio = charData[20].GetInt();
		exec->stunP = charData[21].GetInt();
		exec->result = charData[22].GetInt();
		exec->resultParam1 = charData[23].GetInt();
		exec->resultParam2 = charData[24].GetInt();

		exec->attackType = charData[25].GetInt();
		exec->sourceEnergyAdd = charData[26].GetInt();
		exec->targetEnergyAdd = charData[27].GetInt();

		exec->hard = charData[28].GetInt();
		exec->deadType = charData[29].GetInt();
		exec->dtrate = charData[30].GetInt();

        exec->accuInterval = charData[31].GetInt();
        exec->accuAttAddPerInterval = charData[32].GetInt();
        
		exec->filterTarget = charData[33].GetInt();
		exec->filterX1 = charData[34].GetInt();
		exec->filterX2 = charData[35].GetInt();
		exec->filterZ1 = charData[36].GetInt();
		exec->filterZ2 = charData[37].GetInt();
		
		exec->delayFrameTick = charData[38].GetInt();
		
		rapidjson::Value& vHitDeadExecs = charData[39];// clair_wang
		if (vHitDeadExecs.IsArray())
		{
			for (int paramIdx = 0; paramIdx < vHitDeadExecs.Size(); paramIdx++)
			{
				exec->vHitDeadExecIDs.push_back(vHitDeadExecs[paramIdx].GetInt());
			}
		}
		
		exec->hitLevelUp = charData[40].GetInt();
        auto &extraHitExec = charData[41];
        if (extraHitExec.IsArray())
        {
            for (int extraIdx = 0; extraIdx < extraHitExec.Size(); ++extraIdx)
            {
                exec->hitExecs.push_back(extraHitExec[extraIdx].GetInt());
            }
        }

        // 8.0 add
        // TODO: remove check in 9.0
        exec->resultStack = charData[42].GetInt();
        auto &targetparams = charData[43];
        if (targetparams.IsArray())
        {
            for (int idx = 0; idx < targetparams.Size(); idx++)
                exec->targetParams.push_back(targetparams[idx].GetInt());
        }

        auto &filterparams = charData[44];
        if (filterparams.IsArray())
        {
            for (int idx = 0; idx < filterparams.Size(); idx++)
                exec->filterParams.push_back(filterparams[idx].GetInt());
        }

        // 9.0 add
        // TODO: remove check in 10.0
        if (charData.Size() > 45)
        {
            exec->isDeadClear = (charData[45].GetInt() != 0);
        }
        //cui delayAIAction 2023_2_24
        if (charData.Size() > 46)
        {
            exec->delayTick = charData[46].GetInt();
        }
        //cui realHurtDisplayId 2023_2_27
        if (charData.Size() > 47)
        {
            exec->realHurtDisplayId = charData[47].GetInt();
        }

        // 
        
        /// 12.0
        if (data.HasMember("et"))
        {
            auto &theExcludeVal = data["et"];
            if (!theExcludeVal.IsNull())
            {
                exec->targetExclude = theExcludeVal[0u].GetInt();
                exec->excludeX1 = theExcludeVal[1].GetInt();
                exec->excludeX2 = theExcludeVal[2].GetInt();
                exec->excludeZ1 = theExcludeVal[3].GetInt();
                exec->excludeZ2 = theExcludeVal[4].GetInt();
                auto &theExcludeParamVal = theExcludeVal[5];
                if (theExcludeParamVal.IsArray())
                {
                    for (int idx = 0; idx < theExcludeParamVal.Size(); idx++)
                        exec->targetExcludeParams.push_back(theExcludeParamVal[idx].GetInt());
                }
            }
        }
        ///

		_hurtExec_cache[execID] = exec;
		return exec;

	}

	LabelEffectExec* AIDataHelper::getLabelEffectExec(int labelId)
	{
		if (_labelEffExecMap.find(labelId) != _labelEffExecMap.end())
		{
			return (LabelEffectExec*)(_labelEffExecMap.find(labelId)->second);
		}

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_label_effect.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_label_effect");
#endif // DENA_OP3	
		if (!doc)
            return nullptr;
		
        std::string idStr = stringWithFormat("%d", labelId);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
        rapidjson::Value& labelData = doc->operator[](idStr.c_str());
        if (labelData.IsNull())
        {
#ifdef WIN32
            idStr = "skill_label_effect " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
        }

        LabelEffectExec *label = new LabelEffectExec;
        label->execId = labelId;
        label->name = labelData["n"].GetString();
        label->res = labelData["res"].GetString();
        label->res2 = labelData["res2"].GetString();
        label->res3 = labelData["res3"].GetString();
        rapidjson::Value& array = labelData["conf"];
        label->type = array[0u].GetInt();
        label->pos = array[1].GetInt();
        label->onlyOneSameType = array[2].GetInt();
        label->backgroundParticle = array[3].GetInt();
        label->period = array[4].GetInt();
        label->deadType = array[5].GetInt();
        label->deltaX = array[6].GetInt();
        label->deltaY = array[7].GetInt();


        
        if (array.Size() > 8)
        {
            label->OurColor = array[8].GetInt();  //   
        }

        if (array.Size() > 9)
        {
            label->EnemyColor = array[9].GetInt();
        }
       

        // add 9.0 clair_wang
        if (array.Size() > 10)
        {
            label->upSpeed = (float)array[10].GetInt() / 10000;
        }

        if (array.Size() > 11)
        {
            label->initScale = (float)array[11].GetInt() / 10000;
        }

        if (array.Size() > 12)
        {
            rapidjson::Value& actionScale = array[12];
            if (actionScale.IsArray() && actionScale.Size() > 0)
            {
                for (int idx = 0; idx < actionScale.Size() - 1; idx += 2)
                {
                    LabelEffectExec::LabelScale tmScale;
                    tmScale.Reset();
                    tmScale.duration = (float)actionScale[idx].GetInt() / 10000;
                    tmScale.scaleNum = (float)actionScale[idx + 1].GetInt() / 10000;

                    label->actionScale = tmScale;
                }
            }
        }

        //飘字功能优化 cui
        if (array.Size() > 13)
        {
            label->canMerge = (array[13].GetInt() != 0);
        }
        if (array.Size() > 14)
        {
            label->mergeTick = array[14].GetInt();
        }


        rapidjson::Value& actTypeArry = labelData["art_type"];
        label->actType.resize(actTypeArry.Size());
        for (int idx = 0; idx < actTypeArry.Size(); ++idx)
        {
            label->actType[idx] = actTypeArry[idx].GetInt();
        }


        _labelEffExecMap[labelId] = label;
		
		return label;
	}

	SoundExec* AIDataHelper::getSoundExec(int id)
	{
		if (_soundExecMap.find(id) != _soundExecMap.end())
		{
			return _soundExecMap.find(id)->second;
		}
		
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_audio.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_audio");
#endif // DENA_OP3	
		if (!doc)
			return nullptr;

		std::string idStr = stringWithFormat("%d", id);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
		rapidjson::Value& soundData = doc->operator[](idStr.c_str());
        if (soundData.IsNull())
        {
#ifdef WIN32
            idStr = "skill_audio " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
        }

		SoundExec *sound = new SoundExec;
		sound->name = soundData["n"].GetString();
		sound->soundId = soundData["f"].GetString();

		rapidjson::Value& array = soundData["conf"];
		sound->duration = array[0u].GetInt();
		sound->bSync = (bool)array[1].GetInt();
		sound->playtime = array[2].GetInt();

        if (array.Size() > 4)
        {
            sound->type = array[3].GetInt();
            sound->cueId = array[4].GetInt();
        }

		_soundExecMap[id] = sound;

		return sound;
	}

	LogicFrameData* LogicAnimationExec::getLogicFrameData(int frameIdx, int &aniIdx)
	{
		int totalFrame = 0;
		for (std::vector<LogicFrameData>::iterator iter = frameVec.begin(); iter != frameVec.end(); ++iter)
		{
			int count = totalFrame + iter->frameDuration;
			if (frameIdx < count)
			{
				aniIdx = iter->frameIdx + (frameIdx - totalFrame);
				return &(*iter);
			}
			totalFrame = count;
		}

		return nullptr;
	}

	int LogicAnimationExec::getTotalFrameCount()
	{
		int duration = 0;
		for (std::vector<LogicFrameData>::iterator iter = this->frameVec.begin(); iter != this->frameVec.end(); ++iter)
		{
			duration += iter->frameDuration;
		}
		return duration;
	}

	LogicEffectData* LogicSpecialUIEffectExec::getLogicFrameData(int frameIdx, int &aniIdx)
	{
        /*
        int totalFrame = 0;
        for (std::vector<LogicEffectData>::iterator iter = frameVec.begin(); iter != frameVec.end(); ++iter)
        {
            int count = totalFrame + iter->frameDuration;
            if (frameIdx < count)
            {
                aniIdx = iter->frameIdx + (frameIdx - totalFrame);
                return &(*iter);
            }
            totalFrame = count;
        }

        return nullptr;
        */
		int totalFrame = 0;
		for (std::vector<LogicEffectData>::iterator iter = frameVec.begin(); iter != frameVec.end(); ++iter)
		{
            LogicEffectData &innerData = *iter;
            int count = totalFrame + innerData.frameDuration * innerData.playTime;
            if (frameIdx < count && innerData.frameDuration > 0)
			{
                int theFrameIdx = (frameIdx - totalFrame) % innerData.frameDuration;
                aniIdx = innerData.frameIdx + theFrameIdx;
                return &innerData;
			}
			totalFrame = count;
		}

		return nullptr;
	}

	int LogicSpecialUIEffectExec::getTotalFrameCount()
	{
        /*
		int duration = 0;
		for (std::vector<LogicEffectData>::iterator iter = this->frameVec.begin(); iter != this->frameVec.end(); ++iter)
		{
			duration += iter->frameDuration * iter->playTime;
		}
		return duration;
        */
        return totalFrameCount;
	}
	
	int* AIDataHelper::getPositionArray(bool isEven)
	{
		if (isEven && evenPosition[0]!=-1)
		{
			return evenPosition;
		}
		else if (!isEven && oddPosition[0]!=-1)
		{
			return oddPosition;
		}

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::character_position.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_character_position");
#endif // DENA_OP3	

		if (!doc)
			return nullptr;
		
		rapidjson::Value& array =  doc->operator[]("0")["y"];
		oddPosition[0] = array[0u].GetInt();
		evenPosition[0] = array[1u].GetInt(); 
		
		array = doc->operator[]("1")["y"];
		oddPosition[1] = array[0u].GetInt();
		evenPosition[1] = array[1u].GetInt();

		array = doc->operator[]("2")["y"];
		oddPosition[2] = array[0u].GetInt();
		evenPosition[2] = array[1u].GetInt();

		array = doc->operator[]("3")["y"];
		oddPosition[3] = array[0u].GetInt();
		evenPosition[3] = array[1u].GetInt();

		array = doc->operator[]("4")["y"];
		oddPosition[4] = array[0u].GetInt();
		evenPosition[4] = array[1u].GetInt();

		return isEven?evenPosition:oddPosition;
	}

	int* AIDataHelper::getEnergyRecover()
	{
		if (energyRecover[0] != 0)
		{
			return energyRecover;
		}

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::system_config.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_system_config");
#endif // DENA_OP3

		if (!doc)
			return energyRecover;

		energyRecover[0] = doc->operator[]("max_energy").GetInt();
		energyRecover[1] = doc->operator[]("get_energy").GetInt();

		return energyRecover;
	}

	float* AIDataHelper::getFightingParameter(int index)
	{
		if (fightingParameter[index] != 0)
		{
			return fightingParameter[index];
		}

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::fighting_parameter.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_fighting_parameter");
#endif // DENA_OP3	

		if (!doc)
			return nullptr;

		for (int i = 0; i < 10; i++)
		{
			fightingParameter[i] = new float[8];
			rapidjson::Value& array = doc->operator[](i)["p"];
			fightingParameter[i][0] = (float)array[0u].GetInt() / 100;
			fightingParameter[i][1] = (float)array[1u].GetInt() / 100;
			fightingParameter[i][2] = (float)array[2u].GetInt() / 100;
			fightingParameter[i][3] = (float)array[3u].GetInt() / 100;
			fightingParameter[i][4] = (float)array[4u].GetInt() / 100;
			fightingParameter[i][5] = (float)array[5u].GetInt() / 100;
			fightingParameter[i][6] = (float)array[6u].GetInt() / 100;
			fightingParameter[i][7] = (float)array[7u].GetInt() / 100;
		}

		return fightingParameter[index];
	}

    int AIDataHelper::getBattleFieldRangeToCenterX()
    {
        if (battleFieldRangeToCenterX < 0)
        {
            battleFieldRangeToCenterX = 0;

#ifdef DENA_OP3
            rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::system_config.c_str());
#else
            rapidjson::Document* doc = AIDataManager::getJsonFromFile("_system_config");
#endif // DENA_OP3

            if (doc && doc->HasMember("battlefield_range_x"))
            {
                rapidjson::Value& rangeValue = (*doc)["battlefield_range_x"];
                if (!rangeValue.IsNull())
                    battleFieldRangeToCenterX = rangeValue.GetInt();
            }
        }
        return battleFieldRangeToCenterX;
    }

    int AIDataHelper::getAiUpdateModeAfterWinlose()
    {
        if (aiUpdateModeAfterWinlose < 0)
        {
            aiUpdateModeAfterWinlose = AIUPDATEMODE_CONTINUE_FIGHT_AFTER_WINLOSE;
#ifdef DENA_OP3
            rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::system_config.c_str());
#else
            rapidjson::Document* doc = AIDataManager::getJsonFromFile("_system_config");
#endif // DENA_OP3

            if (doc && doc->HasMember("aimode"))
            {
                rapidjson::Value& theJsonValue = (*doc)["aimode"];
                if (!theJsonValue.IsNull())
                    aiUpdateModeAfterWinlose = theJsonValue.GetInt();
            }
        }
        return aiUpdateModeAfterWinlose;
    }

    int AIDataHelper::getAiCenterXMode()
    {
        if (aiCenterXMode < 0)
        {
            aiCenterXMode = AICENTERX_SELF_CALCULATE;
#ifdef DENA_OP3
            rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::system_config.c_str());
#else
            rapidjson::Document* doc = AIDataManager::getJsonFromFile("_system_config");
#endif // DENA_OP3
            if (doc && doc->HasMember("cxmode"))
            {
                rapidjson::Value& theJsonValue = (*doc)["cxmode"];
                if (!theJsonValue.IsNull())
                    aiCenterXMode = theJsonValue.GetInt();
            }
        }
        return aiCenterXMode;
    }

    int AIDataHelper::getAiUltraBrokenMode()
    {
        if (aiUltraBrokenMode < 0)
        {
            aiUltraBrokenMode = AIULTRABROKEN_DO_NOTHING;
#ifdef DENA_OP3
            rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::system_config.c_str());
#else
            rapidjson::Document* doc = AIDataManager::getJsonFromFile("_system_config");
#endif // DENA_OP3
            if (doc && doc->HasMember("ubmode"))
            {
                rapidjson::Value& theJsonValue = (*doc)["ubmode"];
                if (!theJsonValue.IsNull())
                    aiUltraBrokenMode = theJsonValue.GetInt();
            }
        }
        return aiUltraBrokenMode;
    }

    int AIDataHelper::getAiUiEffectMode()
    {
        if (aiUiEffectMode < 0)
        {
            aiUiEffectMode = AIUIEFFECTMODE_DEFAULT;

#ifdef DENA_OP3
            rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::system_config.c_str());
#else
            rapidjson::Document* doc = AIDataManager::getJsonFromFile("_system_config");
#endif // DENA_OP3
            if (doc && doc->HasMember("uieffmode"))
            {
                rapidjson::Value& theJsonValue = (*doc)["uieffmode"];
                if (!theJsonValue.IsNull())
                    aiUiEffectMode = theJsonValue.GetInt();
            }
        }
        return aiUiEffectMode;
    }

    int AIDataHelper::getAiCenterXForCluster(int theClusterCnt)
    {
        if (aiCenterXCache == nullptr)
        {
            aiCenterXCache = new std::vector<int>;
#ifdef DENA_OP3
            rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::system_config.c_str());
#else
            rapidjson::Document* doc = AIDataManager::getJsonFromFile("_system_config");
#endif // DENA_OP3
            if (doc)
            {
                rapidjson::Value& theJsonValue = (*doc)["cx"];
                if (theJsonValue.IsArray())
                {
                    for (int i = 0; i < theJsonValue.Size(); i++)
                    {
                        aiCenterXCache->push_back(theJsonValue[i].GetInt());
                    }
                }
            }
        }

        if (theClusterCnt > 0 && aiCenterXCache && theClusterCnt <= aiCenterXCache->size())
        {
            return (*aiCenterXCache)[theClusterCnt - 1];
        }

        return 0;
    }

	LogicAnimationExec* AIDataHelper::getLogicAnimationExec(int id)
	{
		if (_logicAniMap.find(id) != _logicAniMap.end())
		{
			return (_logicAniMap.find(id)->second);
		}
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_composite_animation.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_composite_animation");
#endif // DENA_OP3	

		if (!doc)
			return nullptr; 

		std::string idStr = stringWithFormat("%d", id);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
		rapidjson::Value& aniData = doc->operator[](idStr.c_str());
        if (aniData.IsNull())
        {
#ifdef WIN32
            idStr = "skill_composite_animation " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
        }
		rapidjson::Value& array = aniData["conf"];

		LogicAnimationExec *pLogicAni = new LogicAnimationExec;
		pLogicAni->execId = id;
		pLogicAni->aniName = aniData["n"].GetString();
		pLogicAni->charName = aniData["c"].GetString();
		pLogicAni->duration = aniData["d"].GetInt();
		
		for (int idx = 0; idx < array.Size() / 3; ++idx)
		{
			LogicFrameData ani1;
			ani1.aniName = array[3*idx + 0].GetString();
			ani1.frameIdx = (char)array[3 * idx + 1].GetInt();
			ani1.frameDuration = (char)array[3 * idx + 2].GetInt();
			if (ani1.aniName.empty())
				continue;
			pLogicAni->frameVec.push_back(ani1);
		}

		_logicAniMap[id] = pLogicAni;
		return pLogicAni;
	}

	LogicSpecialUIEffectExec* AIDataHelper::getLogicSpecialUIEffectExec(int id)
	{
		if (_logicEffMap.find(id) != _logicEffMap.end())
		{
			return (_logicEffMap.find(id)->second);
		}

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_composite_ui_effect.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_composite_ui_effect");
#endif // DENA_OP3	

		if (!doc)
			return nullptr;

		std::string idStr = stringWithFormat("%d", id);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
		rapidjson::Value& effData = doc->operator[](idStr.c_str());
        if (effData.IsNull())
        {
#ifdef WIN32
            idStr = "skill_composite_ui_effect " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
        }
		rapidjson::Value& array = effData["conf"];

		LogicSpecialUIEffectExec *pLogicEffect = new LogicSpecialUIEffectExec;

		pLogicEffect->execId = id;
		pLogicEffect->bFollow = effData["follow"].GetInt();
        /// 12.0
        if (pLogicEffect->bFollow == 3)
        {
            pLogicEffect->bFollow = 0;
            pLogicEffect->followUnitTurnType = 1;
        }
        ///
		pLogicEffect->name = effData["n"].GetString();
		pLogicEffect->duration = effData["d"].GetInt();

        if (array.Size() <= 24) 
        {
            for (int idx = 0; idx < array.Size() / 3; ++idx)
            {
                LogicEffectData ani1;
                ani1.effectId = array[3 * idx + 0].GetInt();
                ani1.frameIdx = array[3 * idx + 1].GetInt();
                ani1.frameDuration = array[3 * idx + 2].GetInt();
                if (ani1.effectId == 0)
                    continue;
                pLogicEffect->frameVec.push_back(ani1);

                pLogicEffect->totalFrameCount += ani1.frameDuration;
            }
        }
        else 
        {
            for (int idx = 0; idx < array.Size() / 4; ++idx)
            {
                LogicEffectData ani1;
                ani1.effectId = array[4 * idx + 0].GetInt();
                ani1.frameIdx = array[4 * idx + 1].GetInt();
                ani1.frameDuration = array[4 * idx + 2].GetInt();
                ani1.playTime = array[4 * idx + 3].GetInt();
                if (ani1.effectId == 0)
                    continue;
                pLogicEffect->frameVec.push_back(ani1);

                pLogicEffect->totalFrameCount += ani1.frameDuration * ani1.playTime;
            }
        }

	
		_logicEffMap[id] = pLogicEffect;
		return pLogicEffect;
	}

	std::string AIDataHelper::getSkillDataJsonStr(int skid)
	{
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_list.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_list");
#endif // DENA_OP3

		if (!doc)
			return "";
			
		std::string idStr = stringWithFormat("%d", skid);
        if (!doc->HasMember(idStr.c_str()))
            return "";
		rapidjson::Value& data = doc->operator[](idStr.c_str());
		if (data.IsNull())
			return "";
		
        rapidjson::StringBuffer buf;
        buf.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        
        data.Accept(writer);
        return buf.GetString();
	}

	AISkill* AIDataHelper::getSkillData(int skillId)
	{
		if (_aiSkill_cache.find(skillId) != _aiSkill_cache.end())
		{
			return (AISkill*)(_aiSkill_cache.find(skillId)->second);
		}

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_list.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_list");
#endif // DENA_OP3
		if (doc)
		{
			std::string idStr = stringWithFormat("%d", skillId);
            if (!doc->HasMember(idStr.c_str()))
                return nullptr;
			rapidjson::Value& skillData = doc->operator[](idStr.c_str());
            if (skillData.IsNull())
            {
#ifdef WIN32
                idStr = "skill " + idStr + " not found!";
                //CCASSERT(0, idStr.c_str());
#endif
                return nullptr;
            }
			rapidjson::Value& array = skillData["conf"];
			if (array.IsNull()) return nullptr;

			AISkill *skill = new AISkill;

			skill->upperBound = array[0u].GetInt();
			skill->startLevel = array[1].GetInt();
			skill->skillType = array[2].GetInt();
			skill->requireEn = array[3].GetInt();
			skill->consumeEn = array[4].GetInt();
			skill->interruptRate = array[5].GetInt();
			skill->freezeScreen = array[6].GetInt();
			skill->freezeEnemy = array[7].GetInt();
			skill->freezeTime = array[8].GetInt();
            skill->castCondition = ESkillCastCondition(array[9].GetInt());
			//skill->castParam = array[10].GetInt();
            rapidjson::Value& castParamValue = array[10];
            if (castParamValue.IsArray())
            {
                for (int paramIdx = 0; paramIdx < castParamValue.Size(); paramIdx++)
                    skill->castParams.push_back(castParamValue[paramIdx].GetInt());
            }
            else if (castParamValue.IsInt() && castParamValue.GetInt())
            {
                skill->castParams.push_back(castParamValue.GetInt());
            }
			skill->targetType = array[11].GetInt();
			
			rapidjson::Value& targetParamValue = array[12];
			if (targetParamValue.IsArray())
			{
				for (int paramIdx = 0; paramIdx < targetParamValue.Size(); paramIdx++)
					skill->vTargetParms.push_back(targetParamValue[paramIdx].GetInt());
			}

			//	skill->targetParm = array[12].GetInt();
			skill->range = array[13].GetInt();
			skill->icon = array[14].GetInt();

            skill->interruptByUltra = array[15].GetInt();
            rapidjson::Value &ignoreeffs = array[16];
            if (ignoreeffs.IsArray())
            {
                for (int idx = 0; idx < ignoreeffs.Size(); idx++)
                    skill->ignoreEffects[ignoreeffs[idx].GetInt()] = true;
            }

            // 9.0 add
            // TODO:remove check in 10.0
            if (array.Size() > 17)
            {
                skill->canBrokenWhenEnter = (array[17].GetInt() != 0);
            }

            //cui 2023_7_27
            if (array.Size() > 20)
            {
                skill->darkTime = array[18].GetInt();
                skill->virtualTime = array[19].GetInt();
                skill->skillEftId = array[20].GetInt() || 0;
            }

			rapidjson::Value& execArray = skillData["exe"];
			int count = execArray.Size();
			for (int idx = 0; idx < count / 4; ++idx)
			{
				int i = idx * 4;
				SkillExec se;
				se.execId = execArray[i++].GetInt();
				se.execType = (SkillExecType)execArray[i++].GetInt();
				se.param0 = execArray[i++].GetInt();
				se.param1 = execArray[i++].GetInt();
                if (se.execId > 0)
                    skill->skillExec.push_back(se);
			}
            
            // 11.0 add
            auto &val_t = skillData["t"];
            if (val_t.IsInt())
                skill->rangeType = val_t.GetInt();
			_aiSkill_cache[skillId] = skill;
			return skill;
		}

		return nullptr;
	}

	std::string AIDataHelper::getProjectileExecJsonStr(int execId)
	{
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_projectile.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_projectile");
#endif // DENA_OP3

		if (!doc)
			return "";

		std::string idStr = stringWithFormat("%d", execId);
        if (!doc->HasMember(idStr.c_str()))
            return "";
		rapidjson::Value& data = doc->operator[](idStr.c_str());
        if (data.IsNull())
			return "";
			
		rapidjson::StringBuffer buf;
		buf.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
		
		data.Accept(writer);
		return buf.GetString();
	}

	ProjectileExec* AIDataHelper::getProjectileExec(int execID)
	{
		if (_projectileExec_cache.find(execID) != _projectileExec_cache.end())
		{
			return (ProjectileExec*)(_projectileExec_cache.find(execID)->second);
		}
		
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_projectile.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_projectile");
#endif // DENA_OP3

		if (!doc)
			return nullptr;

		std::string idStr = stringWithFormat("%d", execID);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
		rapidjson::Value& data = doc->operator[](idStr.c_str());
        if (data.IsNull())
        {
#ifdef WIN32
            idStr = "skill_projectile " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
		}
		ProjectileExec* exec = new ProjectileExec();
		exec->ID = execID;
		exec->name = data["n"].GetString();

		rapidjson::Value& resData = data["res"];
		exec->resVec.resize(resData.Size());
		for (int idx = 0; idx < resData.Size(); ++idx)
		{
			exec->resVec[idx] = resData[idx].GetString();
		}

		rapidjson::Value& hitValue = data["hit"];
		exec->hitVec.resize(hitValue.Size());
		for (int idx = 0; idx < hitValue.Size(); ++idx)
		{
			exec->hitVec[idx] = hitValue[idx].GetInt();
		}
		rapidjson::Value& charData = data["conf"];
		exec->throwDuration = charData[0u].GetInt();
		exec->throwType = (EProjectileTargetType)charData[1].GetInt();
		exec->hurtTrigger = (EProjectileHurtTriggerType)charData[2].GetInt();
		exec->throwInterval = charData[3].GetInt();
		exec->throwNum = charData[4].GetInt();
		exec->orbitType = (EProjectileOrbitType)charData[5].GetInt();
		exec->x = charData[6].GetInt();
		exec->y = charData[7].GetInt();
		exec->z = charData[8].GetInt();
		exec->dx = charData[9].GetInt();
		exec->dy = charData[10].GetInt();
		exec->followingExec1 = charData[11].GetInt();
		exec->followingExec2 = charData[12].GetInt();
//		exec->hitExec = charData[13].GetInt();
		
		rapidjson::Value& hitExecValue = charData[13];
		if (hitExecValue.IsArray())
		{
			exec->hitExecVec.resize(hitExecValue.Size());
			for (int idx = 0; idx < hitExecValue.Size(); ++idx)
			{
				exec->hitExecVec[idx] = hitExecValue[idx].GetInt();
			}
		}
		else if (hitExecValue.IsInt() && hitExecValue.GetInt())
		{
			exec->hitExecVec.clear();
			exec->hitExecVec.push_back(hitExecValue.GetInt());
		}

		exec->deadType = (EProjectileDeadType)charData[14].GetInt();
		exec->deadParam = charData[15].GetInt();
		exec->deadExec = charData[16].GetInt();
		exec->deadSpecialEffect1 = charData[17].GetInt();
		exec->deadSpecialEffect2 = charData[18].GetInt();
		exec->deadSpecialEffect3 = charData[19].GetInt();
		exec->deadSpecialEffect4 = charData[20].GetInt();
		exec->deadSpecialEffect5 = charData[21].GetInt();

		exec->deadAudio = charData[22].GetInt();
		exec->rotation = charData[23].GetInt();
		exec->scale = charData[24].GetInt();
		exec->shadowScale = charData[25].GetInt();
		exec->opacity = charData[26].GetInt();
		exec->redius = charData[27].GetInt();
		exec->target = charData[28].GetInt();

		rapidjson::Value& castParamValue = charData[29];
		if (castParamValue.IsArray())
		{
			for (int paramIdx = 0; paramIdx < castParamValue.Size(); paramIdx++)
			{
				exec->throwParams.push_back(castParamValue[paramIdx].GetInt());
			}
		}
        else if (castParamValue.IsInt() && castParamValue.GetInt())
        {
            exec->throwParams.push_back(castParamValue.GetInt());
        }

		exec->levelup = charData[30].GetDouble();
		exec->limitOnOff = charData[31].GetInt();
		exec->limitValLowerBase = charData[32].GetInt();
		exec->limitValLowerGrowth = charData[33].GetInt();
		exec->limitValUpperBase = charData[34].GetInt();
		exec->limitValUpperGrowth = charData[35].GetInt();
        exec->throwDirectionType = charData[36].GetInt();
        exec->searchTarget = charData[37].GetInt();
        exec->searchX1 = charData[38].GetInt();
        exec->searchX2 = charData[39].GetInt();
        exec->searchZ1 = charData[40].GetInt();
        exec->searchZ2 = charData[41].GetInt();
        exec->filterTarget = charData[42].GetInt();
        exec->filterX1 = charData[43].GetInt();
        exec->filterX2 = charData[44].GetInt();
        exec->filterZ1 = charData[45].GetInt();
        exec->filterZ2 = charData[46].GetInt();

        // 8.0 add
        //if (charData.Size() > 48)
        //{
        auto &var47 = charData[47];
        if (var47.IsArray())
        {
            for (int i = 0; i < var47.Size(); i++)
            {
                exec->searchParams.push_back(var47[i].GetInt());
            }
        }

        auto &var48 = charData[48];
        if (var48.IsArray())
        {
            for (int i = 0; i < var48.Size(); i++)
            {
                exec->filterParams.push_back(var48[i].GetInt());
            }
        }
        //}

        // 9.0 add
        //if (charData.Size() > 49)
        //{
            exec->lazyTick = charData[49].GetInt();
        //}

        // 10.0
        if (charData.Size() > 50)
        {
            auto &theBouncingJsonData = charData[50];
            if (theBouncingJsonData.IsArray())
            {
                for (int i = 0; i < theBouncingJsonData.Size(); i++)
                {
                    exec->bouncingExecs.push_back(theBouncingJsonData[i].GetInt());
                }
            }
            else if (theBouncingJsonData.IsInt())
            {
                if (theBouncingJsonData.GetInt() != 0)
                    exec->bouncingExecs.push_back(theBouncingJsonData.GetInt());
            }
        }

        //28.0 add
        if (charData.Size() > 53)
        {
            if (charData[51].IsInt() && charData[52].IsInt() && charData[53].IsInt())
            {
                exec->r = charData[51].GetInt();
                exec->g = charData[52].GetInt();
                exec->b = charData[53].GetInt();
            }
        }

        // 28.7 add 2023_02_10 
        if (charData.Size() > 54)
        {
            auto& var54 = charData[54];
            if (var54.IsArray())
            {
                exec->offsetX = var54[0u].GetInt();
                exec->offsetY = var54[1].GetInt();
            }
        }
        
        /// 12.0
        if (data.HasMember("et"))
        {
            auto &theExcludeVal = data["et"];
            if (!theExcludeVal.IsNull())
            {
                exec->targetExclude = theExcludeVal[0u].GetInt();
                exec->excludeX1 = theExcludeVal[1].GetInt();
                exec->excludeX2 = theExcludeVal[2].GetInt();
                exec->excludeZ1 = theExcludeVal[3].GetInt();
                exec->excludeZ2 = theExcludeVal[4].GetInt();
                auto &theExcludeParamVal = theExcludeVal[5];
                if (theExcludeParamVal.IsArray())
                {
                    for (int idx = 0; idx < theExcludeParamVal.Size(); idx++)
                        exec->targetExcludeParams.push_back(theExcludeParamVal[idx].GetInt());
                }
            }
        }
        ///

		_projectileExec_cache[execID] = exec;
		return exec;
	}

    std::string AIDataHelper::getCharacterDataJsonStr(int cid)
	{
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::character_list.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_character_list");
#endif // DENA_OP3
		if (!doc)
			return "";
		
		std::string idStr = stringWithFormat("%d", cid);
        if (!doc->HasMember(idStr.c_str()))
            return "";
		rapidjson::Value& data = doc->operator[](idStr.c_str());
		if (data.IsNull())
			return "";
        
        rapidjson::StringBuffer buf;
        buf.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        
        data.Accept(writer);
        return buf.GetString();
	}

	CharData* AIDataHelper::getCharacterData(int id)
	{
        std::unordered_map<int, CharData*>::iterator iter = _characterDataMap.find(id);
		if (iter != _characterDataMap.end())
			return iter->second;
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::character_list.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_character_list");
#endif // DENA_OP3						

		if (!doc)
			return nullptr;

		std::string idStr = stringWithFormat("%d", id);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
		rapidjson::Value& charValue = doc->operator[](idStr.c_str());
        if (charValue.IsNull())
        {
#if BATTLE_LOG_TEST
        idStr = "character " + idStr + " not found!";
        OPBattleLog(idStr);
#endif
            return nullptr;
        }

		CharData *charData = new CharData;
		charData->id = id;
		charData->name = charValue["n"].GetString();
		
		rapidjson::Value& labels = charValue["label"];
		charData->vLabels.resize(labels.Size());
		for (int idx = 0; idx < labels.Size(); ++idx)
		{
			charData->vLabels[idx] = labels[idx].GetInt();
		}

        // OP2: 新增额外标签,不会在面板显示(无论该标签是否有对应资源)
        if (charValue.HasMember("label1"))
        {
            auto& label1 = charValue["label1"];
            for (int idx = 0; idx < label1.Size(); idx++)
            {
                int lab = label1[idx].GetInt();
                if (lab > 0)
                {
                    charData->vLabels.push_back(lab);
                }
            }
        }

		charData->sex = charValue["sex"].GetInt();
		charData->comment = charValue["sc"].GetString();
		charData->damping = ((float)charValue["f"].GetInt()) / 100;
		charData->hard = charValue["hard"].GetInt();

		rapidjson::Value& resData = charValue["res"];
		charData->aniName = resData[0u].GetString();
		charData->icon = resData[1].GetString();
        int resOffset = 2;
        if (resData.Size() % 3 == 0)
        {
            resOffset = 3;
        }
        for (int i = resOffset; i < resData.Size() - 1; i += resOffset)
        {
            std::string alternateAniName = resData[i].GetString();
            std::string alternateIcon = resData[i + 1].GetString();
            charData->alternateAniNameNIcon.push_back(std::make_pair(alternateAniName, alternateIcon));
        }

		charData->bMirror = (charValue["jx"].GetInt());

		rapidjson::Value& seq = charValue["seq"];
		charData->attackSeq.resize(seq.Size());
		for (int idx = 0; idx < seq.Size(); ++idx)
		{
			charData->attackSeq[idx] = seq[idx].GetInt();
		}
        
        rapidjson::Value& seq2 = charValue["seq2"];
        charData->attackSeq2.resize(seq2.Size());
        for (int idx = 0; idx < seq2.Size(); ++idx)
        {
            charData->attackSeq2[idx] = seq2[idx].GetInt();
        }

		rapidjson::Value& hpPos = charValue["pos"];
		charData->hpPosX = hpPos[0u].GetInt();
		charData->hpPosY = hpPos[1].GetInt();

		charData->fragmentID = charValue["sp"].GetInt();

		rapidjson::Value& config = charValue["conf"];
		charData->confg.sellPrice = config[0u].GetInt();
		charData->confg.height = config[1].GetInt();
		charData->confg.quality = config[2].GetInt();
		charData->confg.line = config[3].GetInt();
		charData->confg.positionPriority = config[4].GetInt();

		rapidjson::Value& sk = charValue["sk"];
		//charData->sk[0] = sk[0u].GetInt();
		//charData->sk[1] = sk[1].GetInt();
		//charData->sk[2] = sk[2].GetInt();
		//charData->sk[3] = sk[3].GetInt();
		//charData->sk[4] = sk[4].GetInt();
        charData->sk.resize(sk.Size());
        for (int idx = 0; idx < sk.Size(); ++idx)
        {
            charData->sk[idx] = sk[idx].GetInt();
        }

		rapidjson::Value& att = charValue["att"];
		charData->att.strenght = att[0u].GetInt();
		charData->att.agile = att[1].GetInt();
		charData->att.intelligence = att[2].GetInt();
		charData->att.precision = att[3].GetInt();
		charData->att.sprit = att[4].GetInt();
		charData->att.life = att[5].GetInt();
		charData->att.energy = att[6].GetInt();
		charData->att.moveSpeed = att[7].GetInt();
		charData->att.melee = att[8].GetInt();
		charData->att.spell = att[9].GetInt();
		charData->att.meleeCrit = att[10].GetInt();
		charData->att.spellCrit = att[11].GetInt();
		charData->att.meleeCritTime = att[12].GetInt();
		charData->att.spellCritTime = att[13].GetInt();
		charData->att.meleeHit = att[14].GetInt();
		charData->att.spellHit = att[15].GetInt();
		charData->att.meleeArmor = att[16].GetInt();
		charData->att.spellAntiBody = att[17].GetInt();
		charData->att.meleePenetrate = att[18].GetInt();
		charData->att.spellPenetrate = att[19].GetInt();
		charData->att.dodgePercentage = att[20].GetInt();
		charData->att.defencePercentage = att[21].GetInt();
		charData->att.vampireLevel = att[22].GetInt();
		charData->att.lifeRecover = att[23].GetInt();
		charData->att.energyRecover = att[24].GetInt();
		charData->att.healEffect = att[25].GetInt();
		charData->att.spellInterval = att[26].GetInt();
		charData->att.attackDistance = att[27].GetInt();
		charData->att.funcHurtAdd = att[32].GetInt();
		charData->att.funcHurtSub = att[33].GetInt();
		charData->att.meleeCritAdd = att[34].GetInt();
		charData->att.spellCritAdd = att[35].GetInt();
		
		rapidjson::Value& juexing = charValue["juexing"];
        
        for (int i = 0; i < 30 ; ++i) {
            charData->juexing[i] = juexing[i].GetInt();
        }
        
//		charData->juexing.zeroJueStrenghtGrowth = juexing[0u].GetInt();
//		charData->juexing.zeroJueAgileGrowth = juexing[1].GetInt();
//		charData->juexing.zeroJueIntelligenceGrowth = juexing[2].GetInt();
//		charData->juexing.zeroJuePrecisionGrowth = juexing[3].GetInt();
//		charData->juexing.zeroJueSpiritGrowth = juexing[4].GetInt();
//		charData->juexing.oneJueStrenghtGrowth = juexing[5].GetInt();
//		charData->juexing.oneJueAgileGrowth = juexing[6].GetInt();
//		charData->juexing.oneJueIntelligenceGrowth = juexing[7].GetInt();
//		charData->juexing.oneJuePrecisionGrowth = juexing[8].GetInt();
//		charData->juexing.oneJueSpiritGrowth = juexing[9].GetInt();
//		charData->juexing.twoJueStrenghtGrowth = juexing[10].GetInt();
//		charData->juexing.twoJueAgileGrowth = juexing[11].GetInt();
//		charData->juexing.twoJueIntelligenceGrowth = juexing[12].GetInt();
//		charData->juexing.twoJuePrecisionGrowth = juexing[13].GetInt();
//		charData->juexing.twoJueSpiritGrowth = juexing[14].GetInt();
//		charData->juexing.threeJueStrenghtGrowth = juexing[15].GetInt();
//		charData->juexing.threeJueAgileGrowth = juexing[16].GetInt();
//		charData->juexing.threeJueIntelligenceGrowth = juexing[17].GetInt();
//		charData->juexing.threeJuePrecisionGrowth = juexing[18].GetInt();
//		charData->juexing.threeJueSpiritGrowth = juexing[19].GetInt();
//		charData->juexing.fourJueStrenghtGrowth = juexing[20].GetInt();
//		charData->juexing.fourJueAgileGrowth = juexing[21].GetInt();
//		charData->juexing.fourJueIntelligenceGrowth = juexing[22].GetInt();
//		charData->juexing.fourJuePrecisionGrowth = juexing[23].GetInt();
//		charData->juexing.fourJueSpiritGrowth = juexing[24].GetInt();
//		charData->juexing.fiveJueStrenghtGrowth = juexing[25].GetInt();
//		charData->juexing.fiveJueAgileGrowth = juexing[26].GetInt();
//		charData->juexing.fiveJueIntelligenceGrowth = juexing[27].GetInt();
//		charData->juexing.fiveJuePrecisionGrowth = juexing[28].GetInt();
//		charData->juexing.fiveJueSpiritGrowth = juexing[29].GetInt();

		rapidjson::Value& levelup = charValue["levelup"];
        //总共19阶 每阶21个属性成长
        for (int i = 0; i < 399; ++i) {
            charData->levelup[i] = levelup[i].GetInt();
        }
        
//		charData->levelup.oneLevelLife = levelup[0u].GetInt();
//		charData->levelup.oneLevelMelee = levelup[1].GetInt();
//		charData->levelup.oneLevelSpell = levelup[1].GetInt();
//		charData->levelup.oneLevelMellDefense = levelup[1].GetInt();
//		charData->levelup.oneLevelSpellDefense = levelup[1].GetInt();
//		charData->levelup.twoLevelLife = levelup[1].GetInt();
//		charData->levelup.twoLevelMelee = levelup[1].GetInt();
//		charData->levelup.twoLevelSpell = levelup[1].GetInt();
//		charData->levelup.twoLevelMellDefense = levelup[1].GetInt();
//		charData->levelup.twoLevelSpellDefense = levelup[1].GetInt();
//		charData->levelup.threeLevelLife = levelup[1].GetInt();
//		charData->levelup.threeLevelMelee = levelup[1].GetInt();
//		charData->levelup.threeLevelSpell = levelup[1].GetInt();
//		charData->levelup.threeLevelMellDefense = levelup[1].GetInt();
//		charData->levelup.threeLevelSpellDefense = levelup[1].GetInt();
//		charData->levelup.fourLevelLife = levelup[1].GetInt();
//		charData->levelup.fourLevelMelee = levelup[1].GetInt();
//		charData->levelup.fourLevelSpell = levelup[1].GetInt();
//		charData->levelup.fourLevelMellDefense = levelup[1].GetInt();
//		charData->levelup.fourLevelSpellDefense = levelup[1].GetInt();
//		charData->levelup.fiveLevelLife = levelup[1].GetInt();
//		charData->levelup.fiveLevelMelee = levelup[1].GetInt();
//		charData->levelup.fiveLevelSpell = levelup[1].GetInt();
//		charData->levelup.fiveLevelMellDefense = levelup[1].GetInt();
//		charData->levelup.fiveLevelSpellDefense = levelup[1].GetInt();
//		charData->levelup.sixLevelLife = levelup[1].GetInt();
//		charData->levelup.sixLevelMelee = levelup[1].GetInt();
//		charData->levelup.sixLevelSpell = levelup[1].GetInt();
//		charData->levelup.sixLevelMellDefense = levelup[1].GetInt();
//		charData->levelup.sixLevelSpellDefense = levelup[1].GetInt();
//		charData->levelup.sevenLevelLife = levelup[1].GetInt();
//		charData->levelup.sevenLevelMelee = levelup[1].GetInt();
//		charData->levelup.sevenLevelSpell = levelup[1].GetInt();
//		charData->levelup.sevenLevelMellDefense = levelup[1].GetInt();
//		charData->levelup.sevenLevelSpellDefense = levelup[1].GetInt();
//		charData->levelup.eightLevelLife = levelup[1].GetInt();
//		charData->levelup.eightLevelMelee = levelup[1].GetInt();
//		charData->levelup.eightLevelSpell = levelup[1].GetInt();
//		charData->levelup.eightLevelMellDefense = levelup[1].GetInt();
//		charData->levelup.eightLevelSpellDefense = levelup[1].GetInt();
//		charData->levelup.nineLevelLife = levelup[1].GetInt();
//		charData->levelup.nineLevelMelee = levelup[1].GetInt();
//		charData->levelup.nineLevelSpell = levelup[1].GetInt();
//		charData->levelup.nineLevelMellDefense = levelup[1].GetInt();
//		charData->levelup.nineLevelSpellDefense = levelup[1].GetInt();


		rapidjson::Value& jxj = charValue["jxj"];
		for (int i = 0; i < 18; i++)
			charData->jxj[i] = jxj[i].GetInt();

		_characterDataMap[id] = charData;
		return charData;
	}

	pve_guanqia_character* AIDataHelper::get_pve_guanqia_character(int guanqiaID, int seq)
	{
		int tmpID = guanqiaID * 100 + seq;

        std::unordered_map<int, pve_guanqia_character*>::iterator iter = _pve_guanqia_character_Map.find(tmpID);
		if (iter != _pve_guanqia_character_Map.end())
			return iter->second;
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::pve_guanqia_characters.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_pve_guanqia_characters");
#endif // DENA_OP3	

		if (!doc)
			return nullptr;

		std::string idStr = stringWithFormat("%d", guanqiaID);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
		rapidjson::Value& theValue = doc->operator[](idStr.c_str());
        if (theValue.IsNull())
        {
#ifdef WIN32
            idStr = "pve_guanqia_character " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
        }

		pve_guanqia_character* thecharacter = new pve_guanqia_character();
		thecharacter->guqnqiaID = guanqiaID;
		thecharacter->seq = seq;
		rapidjson::Value& posarray = theValue[seq-1]["pos"];
		thecharacter->pos.resize(posarray.Size());
		for (int idx = 0; idx < posarray.Size(); ++idx)
		{
			thecharacter->pos[idx] = posarray[idx].GetInt();
		}

		_pve_guanqia_character_Map[tmpID] = thecharacter;
		return thecharacter;
	}

	SpecialExec* AIDataHelper::getSpecialExec(int id)
	{
        std::unordered_map<int, SpecialExec*>::iterator iter = _specialExecMap.find(id);
		if (iter != _specialExecMap.end())
			return iter->second;

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_special.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_special");
#endif // DENA_OP3	
		if (!doc) return nullptr;

		std::string idStr = stringWithFormat("%d", id);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
		rapidjson::Value& execValue = doc->operator[](idStr.c_str());
        if (execValue.IsNull())
        {
#ifdef WIN32
            idStr = "skill_special " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
        }

		if (!execValue.IsNull())
		{
			SpecialExec *execData = new SpecialExec;
			execData->execId = id;
			execData->name = execValue["n"].GetString();
			execData->textureName = execValue["tn"].GetString();
			rapidjson::Value& confData = execValue["conf"];

			execData->duration = confData[0u].GetInt();
			execData->affectTarget = confData[1].GetInt();
			execData->affectRangeX1 = confData[2].GetInt();
			execData->affectRangeX2 = confData[3].GetInt();
			execData->affectRangeY1 = confData[4].GetInt();
			execData->affectRangeY2 = confData[5].GetInt();
			execData->actionType = confData[6].GetInt();
			execData->param0 = confData[7].GetInt();
			execData->param1 = confData[8].GetInt();
			execData->param2 = confData[9].GetInt();
			execData->param3 = confData[10].GetInt();
			execData->param4 = confData[11].GetInt();

			_specialExecMap[id] = execData;
			return execData;
		}
		return nullptr;
	}

    BodyUIEffectExec* AIDataHelper::getBodyUIEffectExec(int id)
    {
        auto iter = _bodyEffectExecMap.find(id);
        if (iter != _bodyEffectExecMap.end())
            return iter->second;

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_body_effect.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_body_effect");
#endif // DENA_OP3	
        if (!doc)
            return nullptr;

        std::string idStr = stringWithFormat("%d", id);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
        rapidjson::Value& execValue = doc->operator[](idStr.c_str());

        if (execValue.IsNull())
        {
#ifdef WIN32
            idStr = "skill_body_effect " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
        }
        
        BodyUIEffectExec *execData = new BodyUIEffectExec;
        execData->execId = id;
        execData->name = execValue["n"].GetString();
        rapidjson::Value& confData = execValue["conf"];

        execData->duration = confData[0u].GetInt();
        execData->affectTarget = confData[1].GetInt();
        execData->affectRangeX1 = confData[2].GetInt();
        execData->affectRangeX2 = confData[3].GetInt();
        execData->affectRangeY1 = confData[4].GetInt();
        execData->affectRangeY2 = confData[5].GetInt();
        execData->actionType = confData[6].GetInt();
        execData->param0 = confData[7].GetInt();
        execData->param1 = confData[8].GetInt();
        execData->param2 = confData[9].GetInt();
        execData->param3 = confData[10].GetInt();
        execData->param4 = confData[11].GetInt();
        execData->param5 = confData[12].GetInt();

        _bodyEffectExecMap[id] = execData;
        return execData;
    }

	std::vector<RewardCandidateItem*>* AIDataHelper::getRewardCandidate(int guanqiaID)
	{
        std::unordered_map<int, std::vector<RewardCandidateItem*>*>::iterator iter = _rewardCandidateMap.find(guanqiaID);
		if (iter != _rewardCandidateMap.end())
			return iter->second;

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::pve_guanqia_hash.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_pve_guanqia_hash");
#endif // DENA_OP3	
		if (!doc)
			return nullptr;

		std::string idStr = stringWithFormat("%d", guanqiaID);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
		rapidjson::Value& theValue = doc->operator[](idStr.c_str());
        if (theValue.IsNull())
        {
#ifdef WIN32
            idStr = "pve_guanqia_hash " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
        }
		rapidjson::Value& rewards = theValue["reward"];

		std::vector<RewardCandidateItem*>* rewardcandidates = new std::vector<RewardCandidateItem*>(rewards.Size());
		for (int idx = 0; idx < rewards.Size(); ++idx)
		{
			rapidjson::Value& rewarditem = rewards[idx];
			if (rewarditem["i"].Size() == 0 || rewarditem["a"].GetInt() == 0 || rewarditem["r"].GetInt() == 0) continue;

			RewardCandidateItem* rci = new RewardCandidateItem();
			rci->iarlist.resize(rewarditem["i"].Size());
			for (int idx = 0; idx < rewarditem["i"].Size(); ++idx)
			{
				rci->iarlist[idx] = new IARitem(rewarditem["i"][idx]["i"].GetInt(), rewarditem["i"][idx]["a"].GetInt(), rewarditem["i"][idx]["r"].GetInt());
			}
			rci->a = rewarditem["a"].GetInt();
			rci->r = rewarditem["r"].GetInt();

			rapidjson::Value& icon = rewarditem["icon"];
			rci->icon.resize(icon.Size());
			for (int idx = 0; idx < icon.Size(); ++idx)
			{
				rci->icon[idx] = icon[idx].GetInt();
			}

			rci->q = rewarditem["q"].GetInt();
			rci->n = rewarditem["n"].GetString();
			rci->d = rewarditem["d"].GetString();
			rci->t = rewarditem["t"].GetInt();

			(*rewardcandidates)[idx] = rci;
		}

		_rewardCandidateMap[guanqiaID] = rewardcandidates;
		return rewardcandidates;
	}

	std::string AIDataHelper::getBaqiSpecialExecJsonStr(int talentID)
	{
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::domineer_site_list_hash.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_domineer_site_list_hash");
#endif // DENA_OP3

		if (!doc)
			return "";

		std::string talentIDStr = stringWithFormat("%d", talentID);
        if (!doc->HasMember(talentIDStr.c_str()))
            return "";
		rapidjson::Value& data = doc->operator[](talentIDStr.c_str());
		if (data.IsNull())
			return "";

		rapidjson::StringBuffer buf;
		buf.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
		
		data.Accept(writer);
		return buf.GetString();
	}
	
	BaqiSpecialExec* AIDataHelper::getBaqiSpecialExec(unsigned short talentID, unsigned short talentLV)
	{
		std::string idStr = stringWithFormat("%d-%d", talentID, talentLV);

		if (_baqiSpecialExecMap.find(idStr) != _baqiSpecialExecMap.end())
		{
			return (BaqiSpecialExec*)(_baqiSpecialExecMap.find(idStr)->second);
		}
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::domineer_site_list_hash.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_domineer_site_list_hash");
#endif // DENA_OP3

		if (doc)
		{

			std::string talentIDStr = stringWithFormat("%d", talentID);
            if (!doc->HasMember(talentIDStr.c_str()))
                return nullptr;
			rapidjson::Value& lvdataArray = doc->operator[](talentIDStr.c_str());
            if (lvdataArray.IsNull())
            {
#ifdef WIN32
                idStr = "domineer_site_list_hash " + talentIDStr + " not found!";
                //CCASSERT(0, idStr.c_str());
#endif
                return nullptr;
            }
			rapidjson::Value& lvdata = lvdataArray.operator[](talentLV);

			BaqiSpecialExec *effectExec = new BaqiSpecialExec();

			effectExec->talentID = talentID;
			effectExec->talentLV = talentLV;
			
			rapidjson::Value& special_condition = lvdata["special_condition"];
			effectExec->trigger = special_condition[0u].GetInt();
			effectExec->target = special_condition[1u].GetInt();
			effectExec->effectExecID = special_condition[2u].GetInt();

			_baqiSpecialExecMap[idStr] = effectExec;
			return effectExec;
		}

		return nullptr;
	}

    MarkExec* AIDataHelper::getMarkExec(int execId)
    {
        auto iterFound = _markExecMap.find(execId);
        if (iterFound != _markExecMap.end())
        {
            return iterFound->second;
        }

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::mark_list.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_mark_list");
#endif // DENA_OP3	
        if (doc)
        {
            std::string idStr = stringWithFormat("%d", execId);
            if (!doc->HasMember(idStr.c_str()))
                return nullptr;
            rapidjson::Value& MarkExecData = doc->operator[](idStr.c_str());
            if (MarkExecData.IsNull())
            {
#ifdef WIN32
                idStr = "mark_list " + idStr + " not found!";
                //CCASSERT(0, idStr.c_str());
#endif
                return nullptr;
            }

            MarkExec *markExec = new MarkExec;

            markExec->ID = execId;
            markExec->name = MarkExecData["n"].GetString();

            rapidjson::Value& array = MarkExecData["conf"];
            markExec->target = array[0u].GetInt();
            markExec->range.resize(4);
            markExec->range[0u] = array[1].GetInt();
            markExec->range[1] = array[2].GetInt();
            markExec->range[2] = array[3].GetInt();
            markExec->range[3] = array[4].GetInt();
            markExec->type = array[5].GetInt();
            markExec->duration = array[6].GetInt();
			// clair_wang begin
			if (MarkExecData.HasMember("timeAdd"))
				markExec->durationTimeAddUp = MarkExecData["timeAdd"].GetInt();
			if (MarkExecData.HasMember("ifRelateSkill"))
				markExec->durationPromoteType = MarkExecData["ifRelateSkill"].GetInt();
			if (MarkExecData.HasMember("relateSkillID"))
				markExec->durationPromoteRelateSkillID = MarkExecData["relateSkillID"].GetInt();
			// clair_wang end
            markExec->initLevel = array[7].GetInt();
			markExec->maxLevelBase = array[8].GetInt();
            markExec->increaseLevel = array[9].GetInt();
			markExec->resetDurationType = array[10].GetInt();
			// clair_wang begin
			if (array.Size() > 11)
			{
				markExec->maxLevelUp = array[11].GetInt();
			}
			if (array.Size() > 12)
			{
				markExec->maxLevelPromoteCond = array[12].GetInt();
			}

			if (array.Size() > 13)
			{
				rapidjson::Value& condtions = array[13];
                if (condtions.IsArray() && condtions.Size() > 0)
				{
					for (int idx = 0; idx < condtions.Size()-1; idx+=3)
					{
						MarkExec::MaxLevelUpParam tmDur;
						tmDur.camp = condtions[idx].GetInt();
						tmDur.characterId = condtions[idx+1].GetInt();
						tmDur.upLevel = condtions[idx+2].GetInt();
						markExec->maxLevelUpParam = tmDur;
					}
				}
			}

			if (array.Size() > 14)
			{
				markExec->maxLevelValue = array[14].GetInt();
			}
			// clair_wang end

			if (MarkExecData.HasMember("counteractSwitch"))
				markExec->counteractSwitch = MarkExecData["counteractSwitch"].GetInt();

            rapidjson::Value& counteractTypes = MarkExecData["counteract"];
            markExec->counteractTypes.resize(counteractTypes.Size());
            for (int idx = 0; idx < counteractTypes.Size(); idx++)
            {
                markExec->counteractTypes.push_back(counteractTypes[idx].GetInt());
            }

            rapidjson::Value& coverTypes = MarkExecData["cover"];
            markExec->coverTypes.resize(coverTypes.Size());
            for (int idx = 0; idx < coverTypes.Size(); idx++)
            {
                markExec->coverTypes.push_back(coverTypes[idx].GetInt());
            }

            _markExecMap[execId] = markExec;
            return markExec;
        }

        return nullptr;
    }

    MarkEffectExec* AIDataHelper::getMarkEffectExec(int markType, int markLevel)
    {
        //const int markEffectKey = (int(markType) << 8) + markLevel;
        //auto iterFound = _markEffectExecMap.find(markEffectKey);
        //if (iterFound != _markEffectExecMap.end())
        //{
        //    return iterFound->second;
        //}

        auto &marks = _markLevelMap[markType];
        auto iterFound = marks.find(markLevel);
        if (iterFound != marks.end())
        {
            return iterFound->second;
        }


#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::mark_effect.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_mark_effect");
#endif // DENA_OP3
        if (doc)
        {
            std::string idStr = stringWithFormat("%d", (int)markType);
            if (!doc->HasMember(idStr.c_str()))
                return nullptr;
            rapidjson::Value& MarkEffectExecData = doc->operator[](idStr.c_str());
            if (MarkEffectExecData.IsNull())
            {
#ifdef WIN32
                idStr = "mark_effect " + idStr + " not found!";
                //CCASSERT(0, idStr.c_str());
#endif
                return nullptr;
            }
            if (MarkEffectExecData.IsNull()) return nullptr;

            int idxFound = -1;
            for (int iEff = 0; iEff < MarkEffectExecData.Size(); ++iEff)
            {
                rapidjson::Value& effExecData = MarkEffectExecData[iEff];
                if (effExecData["level"].GetInt() == markLevel)
                {
                    idxFound = iEff;
                    break;
                }
            }
            if (idxFound < 0)
                return nullptr;

            MarkEffectExec* markEffectExec = new MarkEffectExec;
            markEffectExec->type = markType;
            markEffectExec->level = markLevel;

            rapidjson::Value& execData = MarkEffectExecData[idxFound];
            markEffectExec->triggerType = (EMarkEffectTriggerType)execData["t"].GetInt();

            rapidjson::Value& triggerParams = execData["t_param"];
            markEffectExec->triggers.resize(triggerParams.Size());
            for (int idx = 0; idx < triggerParams.Size(); idx++)
            {
                markEffectExec->triggers[idx] = triggerParams[idx].GetInt();
            }

            rapidjson::Value& array = execData["conf"];

            // markEffectExec->triggerEffectID = array[0u].GetInt();
            if (array[0u].IsArray())
            {
                rapidjson::Value& triggerIDs = array[0u];
                markEffectExec->triggerEffectIDs.resize(triggerIDs.Size());
                for (int i = 0; i < triggerIDs.Size(); i++)
                {
                    markEffectExec->triggerEffectIDs[i] = triggerIDs[i].GetInt();
                }
            }
            else
            {
                markEffectExec->triggerEffectIDs.push_back(array[0u].GetInt());
            }

            markEffectExec->iconID = array[1].GetInt();
            markEffectExec->specialEffectID = array[2].GetInt();
            markEffectExec->soundID = array[3].GetInt();
            markEffectExec->deadType = (EMarkEffectDeadType)array[4].GetInt();

            rapidjson::Value& deadParams = execData["d_param"];
            markEffectExec->deadParams.resize(deadParams.Size());
            for (int idx = 0; idx < deadParams.Size(); idx++)
            {
                markEffectExec->deadParams[idx] = deadParams[idx].GetInt();
            }

            rapidjson::Value& exec = execData["exec"];
            markEffectExec->decreaseLevel = exec[0u].GetInt();
            markEffectExec->deadExecID = exec[1].GetInt();
            markEffectExec->deadEffectID = exec[2].GetInt();
            markEffectExec->deadSoundID = exec[3].GetInt();
            markEffectExec->notAffactCDTick = exec[4].GetInt();

            if (execData.HasMember("trigger_t"))
            {
                markEffectExec->triggerEftSelectType = execData["trigger_t"].GetInt();
            }

            if (execData.HasMember("if_deadcleared"))
            {
                markEffectExec->afterTriggerDeadIfCleared = execData["if_deadcleared"].GetInt();
            }

            if (execData.HasMember("if_damageclear"))
            {
                markEffectExec->ifClearHurtAfterMarkDead = execData["if_damageclear"].GetInt();
            }

            // OP2
            if (execData.HasMember("cd"))
            {
                markEffectExec->triggerCD = execData["cd"].GetInt();
            }

            //_markEffectExecMap[markEffectKey] = markEffectExec;
            marks[markLevel] = markEffectExec;
            return markEffectExec;
        }

        return nullptr;
    }

    std::string AIDataHelper::getSummonExecJsonStr(int execId)
    {
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_summon.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_summon");
#endif // DENA_OP3

        if (!doc)
            return "";
        std::string idStr = stringWithFormat("%d", execId);
        if (!doc->HasMember(idStr.c_str()))
            return "";
        rapidjson::Value& data = doc->operator[](idStr.c_str());
        if (data.IsNull())
            return "";

        rapidjson::StringBuffer buf;
        buf.Clear();
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);

        data.Accept(writer);
        return buf.GetString();
    }
    
    SummonExec* AIDataHelper::getSummonExec(int execId)
    {
        if (_summonExecMap.find(execId) != _summonExecMap.end())
        {
            return (SummonExec*)(_summonExecMap.find(execId)->second);
        }
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_summon.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_summon");
#endif // DENA_OP3

        if (doc) {
            std::string idStr = stringWithFormat("%d", execId);
            if (!doc->HasMember(idStr.c_str()))
                return nullptr;
            rapidjson::Value& SummonExecData = doc->operator[](idStr.c_str());
            if (SummonExecData.IsNull())
            {
#ifdef WIN32
                idStr = "skill_summon " + idStr + " not found!";
                //CCASSERT(0, idStr.c_str());
#endif
                return nullptr;
            }
            
            SummonExec *pSummonExec = new SummonExec;
            pSummonExec->ID = execId;
            pSummonExec->name = SummonExecData["n"].GetString();
            
            rapidjson::Value& array = SummonExecData["conf"];
            pSummonExec->target = array[0u].GetInt();
            pSummonExec->range.resize(4);
            pSummonExec->range[0u] = array[1].GetInt();
            pSummonExec->range[1] = array[2].GetInt();
            pSummonExec->range[2] = array[3].GetInt();
            pSummonExec->range[3] = array[4].GetInt();

            // 9.0 add
            if (array.Size() > 5)
            {
                pSummonExec->fakeSummon = array[5].GetInt();
            }

            if (array.Size() > 8)
            {
                pSummonExec->groupID = array[6].GetInt();
                pSummonExec->groupWeight = array[7].GetInt();
                pSummonExec->groupMaxNum = array[8].GetInt();
            }
            
            rapidjson::Value& execs = SummonExecData["exec"];
            int exec_len = 14;
            int count = execs.Size()/exec_len;
            pSummonExec->execs.resize(count);
            
            rapidjson::Value& extraExecs = SummonExecData["exec2"];
            
            for (int idx = 0; idx < count; idx++)
            {
                pSummonExec->execs[idx].type = execs[exec_len * idx].GetInt();
                pSummonExec->execs[idx].rate = execs[exec_len * idx + 1].GetInt();
                pSummonExec->execs[idx].interval = execs[exec_len * idx + 2].GetInt();
                pSummonExec->execs[idx].summonNum = execs[exec_len * idx + 3].GetInt();
                pSummonExec->execs[idx].maxSummonNum = execs[exec_len * idx + 4].GetInt();
                pSummonExec->execs[idx].oneSample = execs[exec_len * idx + 5].GetInt();
                pSummonExec->execs[idx].summonId = execs[exec_len * idx + 6].GetInt();
				pSummonExec->execs[idx].showSummonInterval = execs[exec_len * idx + 7].GetInt();

                /*
				pSummonExec->execs[idx].specialExec[0] = execs[exec_len * idx + 8].GetInt();
				pSummonExec->execs[idx].specialExecInterval[0] = execs[exec_len * idx + 9].GetInt();

				pSummonExec->execs[idx].specialExec[1] = execs[exec_len * idx + 10].GetInt();
				pSummonExec->execs[idx].specialExecInterval[1] = execs[exec_len * idx + 11].GetInt();

				pSummonExec->execs[idx].specialExec[2] = execs[exec_len * idx + 12].GetInt();
				pSummonExec->execs[idx].specialExecInterval[2] = execs[exec_len * idx + 13].GetInt();
                */

                pSummonExec->execs[idx].execs.push_back(std::make_pair(execs[exec_len*idx + 8].GetInt(), execs[exec_len*idx + 9].GetInt()));
                pSummonExec->execs[idx].execs.push_back(std::make_pair(execs[exec_len*idx + 10].GetInt(), execs[exec_len*idx + 11].GetInt()));
                pSummonExec->execs[idx].execs.push_back(std::make_pair(execs[exec_len*idx + 12].GetInt(), execs[exec_len*idx + 13].GetInt()));

                if (extraExecs.IsArray() && idx < extraExecs.Size())
                {
                    rapidjson::Value& extraExec = extraExecs[idx];
                    if (!extraExec.IsArray())
                        continue;
                    for (int ei = 0; ei < extraExec.Size(); ei++)
                    {
                        rapidjson::Value& exexec = extraExec[ei];
                        if (!exexec.IsArray() || exexec.Size() < 2)
                            continue;
                        int exexecID = exexec[0u].GetInt();
                        int exexecInterval = exexec[1].GetInt();
                        /*
                        pSummonExec->execs[idx].extraExecs.push_back(std::make_pair(exexecID, exexecInterval));
                        */
                        pSummonExec->execs[idx].execs.push_back(std::make_pair(exexecID, exexecInterval));
                    }
                }
            }

            _summonExecMap[execId] = pSummonExec;
            return pSummonExec;
            
        }
        
        return nullptr;
	}
	
	std::string AIDataHelper::getSummonPropertyJsonStr(int summonId)
	{
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_summon_attr.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_summon_attr");
#endif // DENA_OP3
		if (!doc)
			return "";
		std::string idStr = stringWithFormat("%d", summonId);
        if (!doc->HasMember(idStr.c_str()))
            return "";
		rapidjson::Value& data = doc->operator[](idStr.c_str());
		if (data.IsNull())
			return "";

		rapidjson::StringBuffer buf;
		buf.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
		
		data.Accept(writer);
		return buf.GetString();
	}
    
    SummonProperty* AIDataHelper::getSummonProperty(int summonId)
    {
        if (_summonPropertyMap.find(summonId) != _summonPropertyMap.end())
        {
            return (SummonProperty*)(_summonPropertyMap.find(summonId)->second);
        }
        
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_summon_attr.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_summon_attr");
#endif // DENA_OP3

        if (doc) {
            std::string idStr = stringWithFormat("%d", summonId);
            if (!doc->HasMember(idStr.c_str()))
                return nullptr;
            rapidjson::Value& SummonPropertyData = doc->operator[](idStr.c_str());
            if (SummonPropertyData.IsNull())
            {
#ifdef WIN32
                idStr = "skill_summon_attr " + idStr + " not found!";
                //CCASSERT(0, idStr.c_str());
#endif
                return nullptr;
            }
            
            SummonProperty *pSummonProperty = new SummonProperty;
            pSummonProperty->summonId = summonId;
            
            rapidjson::Value& associationIds = SummonPropertyData["n"];
            
            pSummonProperty->associationIdVec.resize(associationIds.Size());
            for (int idx = 0; idx < associationIds.Size(); ++idx)
            {
                pSummonProperty->associationIdVec[idx] = associationIds[idx].GetInt();
            }

            
            rapidjson::Value& array = SummonPropertyData["conf"];
            pSummonProperty->target = array[0u].GetInt();
            pSummonProperty->targetId = array[1].GetInt();
            pSummonProperty->lx = array[2].GetInt();
            pSummonProperty->rx = array[3].GetInt();
            pSummonProperty->ly = array[4].GetInt();
            pSummonProperty->ry = array[5].GetInt();
            
            pSummonProperty->summonSkillInfos.resize(5);
            for (int i = 0; i < 5; ++i) {
                pSummonProperty->summonSkillInfos[i].skillId = array[6 + i*3].GetInt();
                pSummonProperty->summonSkillInfos[i].param0 = array[7 + i*3].GetInt();
                pSummonProperty->summonSkillInfos[i].param1 = array[8 + i*3].GetInt();
            }

            pSummonProperty->beAttackRate = array[21].GetInt();
            pSummonProperty->attackRate = array[22].GetInt();
            pSummonProperty->treatRate = array[23].GetInt();
            pSummonProperty->durationTick = array[24].GetInt();
            pSummonProperty->deadType = array[25].GetInt();
            if (array.Size() > 26)
            {
                pSummonProperty->posType = array[26].GetInt();
            }
            if (array.Size() > 27)
                pSummonProperty->teamsideType = array[27].GetInt();
            if (array.Size() > 28)
                pSummonProperty->affectWinlose = array[28].GetInt();
            
            auto&& exconf = SummonPropertyData["arr"];
            auto&& extSkId = exconf[0u];
            auto&& extSkP0 = exconf[1];
            auto&& extSkP1 = exconf[2];
            for (int iSk = 0; iSk < extSkId.Size(); iSk++) {
                SummonSkillDetail sk;
                sk.skillId = extSkId[iSk].GetInt();
                sk.param0 = 0;
                sk.param1 = 0;
                if (extSkP0.IsArray() && iSk < extSkP0.Size())
                    sk.param0 = extSkP0[iSk].GetInt();
                if (extSkP1.IsArray() && iSk < extSkP1.Size())
                    sk.param1 = extSkP1[iSk].GetInt();

                pSummonProperty->summonSkillInfos.push_back(sk);
            }
            if (exconf.Size() > 3) {
                auto&& exinherits = exconf[3];
                if (exinherits.IsArray() && exinherits.Size() > 0) {
                    for (int iexin = 0; iexin < exinherits.Size(); iexin++) {
                        auto &&anExinherit = exinherits[iexin];
                        SummonProperty::AttrInheritConf anAttrInherit;
                        anAttrInherit.attr = anExinherit[0u].GetInt();
                        anAttrInherit.inheritBase = anExinherit[1].GetInt();
                        anAttrInherit.inheritGrow = anExinherit[2].GetInt();

						pSummonProperty->specialAttrInherit.push_back(anAttrInherit);
                    }
                }
            }
 
            rapidjson::Value& deadParams = SummonPropertyData["param"];
            pSummonProperty->deadParams.resize(deadParams.Size());
            for (int idx = 0; idx < deadParams.Size(); idx++)
            {
                pSummonProperty->deadParams[idx] = deadParams[idx].GetInt();
            }
            
            
            rapidjson::Value& execs = SummonPropertyData["exec"];
            int exec_len = 4;
            int count = execs.Size()/exec_len;
            
            pSummonProperty->deadRunVec.resize(count);
            
            for (int idx = 0; idx < count; idx++)
            {
                pSummonProperty->deadRunVec[idx].deadRunCondition = execs[exec_len * idx].GetInt();
                pSummonProperty->deadRunVec[idx].deadRunParam = execs[exec_len * idx + 1].GetInt();
                pSummonProperty->deadRunVec[idx].run1 = execs[exec_len * idx + 2].GetInt();
                pSummonProperty->deadRunVec[idx].run2 = execs[exec_len * idx + 3].GetInt();
                
            }

            auto& color = SummonPropertyData["color"];
            if (color.IsArray())
            {
                for (int i = 0; i < color.Size(); i += 5)
                {
                    if (i < 5)
                    {
                        pSummonProperty->shouldColorFriend.shouldColor = color[0u].GetInt();
                        pSummonProperty->shouldColorFriend.opacity = color[1].GetInt();
                        pSummonProperty->shouldColorFriend.r = color[2].GetInt();
                        pSummonProperty->shouldColorFriend.g = color[3].GetInt();
                        pSummonProperty->shouldColorFriend.b = color[4].GetInt();
                    }
                    else
                    {
                        pSummonProperty->shouldColorEnemy.shouldColor = color[5].GetInt();
                        pSummonProperty->shouldColorEnemy.opacity = color[6].GetInt();
                        pSummonProperty->shouldColorEnemy.r = color[7].GetInt();
                        pSummonProperty->shouldColorEnemy.g = color[8].GetInt();
                        pSummonProperty->shouldColorEnemy.b = color[9].GetInt();
                    }
                }
            }
            else
            {
                pSummonProperty->shouldColorFriend.clear();
                pSummonProperty->shouldColorEnemy.clear();
            }

            // TODO: remove compatible check after 4.0
            pSummonProperty->attrInheritBase = 0;
            pSummonProperty->attrInheritGrow = 0;
            auto& inherit = SummonPropertyData["inherit"];
            if (inherit.IsInt())
                pSummonProperty->attrInheritBase = inherit.GetInt();
            auto& inheriGrow = SummonPropertyData["inheritP"];
            if (inheriGrow.IsInt())
                pSummonProperty->attrInheritGrow = inheriGrow.GetInt();

            pSummonProperty->deadClear = 0;
            auto& deadClear = SummonPropertyData["dead"];
            if (deadClear.IsInt())
                pSummonProperty->deadClear = deadClear.GetInt();

            _summonPropertyMap[summonId] = pSummonProperty;
            return pSummonProperty;
            
        }
        
        
        return nullptr;
    }

    TransformExec* AIDataHelper::getTransformExec(int execId)
    {
        if (_transformExecMap.find(execId) != _transformExecMap.end())
            return (TransformExec*)(_transformExecMap.find(execId)->second);

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_transform.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_transform");
#endif // DENA_OP3	

        if (!doc)
            return nullptr;

#ifdef DENA_OP3
        rapidjson::Document* attrDoc = AIDataManager::getJsonFromFile(AIDataHelper::skill_transform_attr.c_str());
#else
        rapidjson::Document* attrDoc = AIDataManager::getJsonFromFile("_skill_transform_attr");
#endif // DENA_OP3	
        if (!attrDoc)
            return nullptr;

        std::string idStr = stringWithFormat("%d", execId);
        if (!doc->HasMember(idStr.c_str()))
            return nullptr;
        auto& record = (*doc)[idStr.c_str()];
        if (record.IsNull())
        {
#ifdef WIN32
            idStr = "skill_transform " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
        }

        int transformID = record["attrId"].GetInt();
        idStr = stringWithFormat("%d", transformID);
        if (!attrDoc->HasMember(idStr.c_str()))
            return nullptr;
        auto& attr = (*attrDoc)[idStr.c_str()];
        if (attr.IsNull())
        {
#ifdef WIN32
            idStr = "skill_transform_attr " + idStr + " not found!";
            //CCASSERT(0, idStr.c_str());
#endif
            return nullptr;
        }

        auto pTransformExec = new TransformExec;
        pTransformExec->ID = execId;
        pTransformExec->name = record["n"].GetString();

        auto& range = record["conf"];
        pTransformExec->target = range[0u].GetInt();
        pTransformExec->range.resize(4);
        pTransformExec->range[0u] = range[1].GetInt();
        pTransformExec->range[1] = range[2].GetInt();
        pTransformExec->range[2] = range[3].GetInt();
        pTransformExec->range[3] = range[4].GetInt();

        pTransformExec->transformType = record["type"].GetInt();
        pTransformExec->rate = record["chance"].GetInt();
        pTransformExec->interval = record["interval"].GetInt();

        pTransformExec->transform.ID = transformID;
        pTransformExec->transform.animation = attr["n"].GetInt();
        pTransformExec->transform.keepEffects = attr["keepEffect"].GetInt();
        pTransformExec->transform.keepMarks = attr["keepMark"].GetInt();
        pTransformExec->transform.keepHP = attr["keepHp"].GetInt();
        pTransformExec->transform.duration = attr["time"].GetInt();

        auto& array1 = attr["trigger1"];
        for (int i = 0; i < array1.Size(); i++)
            pTransformExec->transform.execIDVecWhenTrigger.push_back(array1[i].GetInt());

        auto& array2 = attr["trigger2"];
        for (int i = 0; i < array2.Size(); i++)
            pTransformExec->transform.execIDVecWhenEnd.push_back(array2[i].GetInt());
        
        _transformExecMap[execId] = pTransformExec;

        return pTransformExec;
	}
	
	std::string AIDataHelper::getEffectExecJsonStr(int id)
	{
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_effect.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_effect");
#endif // DENA_OP3	

		if (!doc)
			return "";
		std::string idStr = stringWithFormat("%d", id);
        if (!doc->HasMember(idStr.c_str()))
            return "";
		rapidjson::Value& data = doc->operator[](idStr.c_str());
		if (data.IsNull())
			return "";
			
        rapidjson::StringBuffer buf;
        buf.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        
        data.Accept(writer);
        return buf.GetString();
	}

	EffectExec* AIDataHelper::getEffectExec(int effectId)
	{
		if (_effectExecMap.find(effectId) != _effectExecMap.end())
		{
			return (EffectExec*)(_effectExecMap.find(effectId)->second);
		}
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::skill_effect.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_skill_effect");
#endif // DENA_OP3	

		if (doc)
		{
			std::string idStr = stringWithFormat("%d", effectId);
            if (!doc->HasMember(idStr.c_str()))
                return nullptr;
			rapidjson::Value& effectData = doc->operator[](idStr.c_str());
            if (effectData.IsNull())
            {
#ifdef WIN32
                idStr = "effect_exec " + idStr + " not found!";
                //CCASSERT(0, idStr.c_str());
#endif
                return nullptr;
            }

			EffectExec *effectExec = new EffectExec();

			effectExec->ID = effectId;
			effectExec->name = effectData["n"].GetString();
			
			rapidjson::Value& del = effectData["del"];
			//effectExec->delExecs.resize(del.Size());
			for (int idx = 0; idx < del.Size(); idx++)
			{
				effectExec->delExecs.push_back(del[idx].GetInt());
			}

			auto& execs = effectData["exec"];
            auto& triggerCDs = effectData["triggerCD"];
			auto& hitDeadArray = effectData["tok"];
            auto& ui_special_effects = effectData["ui_effect"];
            auto& bodyEffects = effectData["body_effect"];
            auto& params = effectData["param"];
            auto& uiStartEndEffects = effectData["uise"];

            const int exec_len = 19;
            const int exec_count = execs.Size() / exec_len;
            effectExec->execs.resize(exec_count);
            for (int idx = 0; idx < exec_count; idx++)
			{
                effectExec->execs[idx].idx = idx;
				effectExec->execs[idx].ID = execs[exec_len * idx].GetInt();
				effectExec->execs[idx].type = execs[exec_len * idx + 1].GetInt();
				effectExec->execs[idx].label = execs[exec_len * idx + 2].GetInt();
				effectExec->execs[idx].rate = execs[exec_len * idx + 3].GetInt();
				effectExec->execs[idx].changeType = execs[exec_len * idx + 4].GetInt();
				effectExec->execs[idx].targetAttribute = execs[exec_len * idx + 5].GetInt();
				effectExec->execs[idx].valueBase = execs[exec_len * idx + 6].GetInt();
				effectExec->execs[idx].changeValue = execs[exec_len * idx + 7].GetInt();
				effectExec->execs[idx].levelup = execs[exec_len * idx + 8].GetInt();
                //效果执行的上下限
                //effectExec->execs[idx].lowerLimitVal = execs[exec_len * idx + 9].GetInt();
                //effectExec->execs[idx].upperLimitVal = execs[exec_len * idx + 10].GetInt();
                effectExec->execs[idx].limitOnOff = execs[exec_len * idx + 9].GetInt();
                effectExec->execs[idx].limitValLowerBase = execs[exec_len * idx + 10].GetInt();
                effectExec->execs[idx].limitValLowerGrowth = execs[exec_len * idx + 11].GetInt();
                effectExec->execs[idx].limitValUpperBase = execs[exec_len * idx + 12].GetInt();
                effectExec->execs[idx].limitValUpperGrowth = execs[exec_len * idx + 13].GetInt();
                
				effectExec->execs[idx].onlySameType = execs[exec_len * idx + 14].GetInt();
				effectExec->execs[idx].exitClear = execs[exec_len * idx + 15].GetInt();
				effectExec->execs[idx].interval = execs[exec_len * idx + 16].GetInt();
				effectExec->execs[idx].canBeAttack = execs[exec_len * idx + 17].GetInt();
				effectExec->execs[idx].hurtType = execs[exec_len * idx + 18].GetInt();

                effectExec->execs[idx].triggerCD = 0;
                if (triggerCDs.IsArray())
                    effectExec->execs[idx].triggerCD = triggerCDs[idx].GetInt();

				if (hitDeadArray.IsArray())
				{
					for (int i = 0; i < hitDeadArray[idx].Size(); i++)
					{
						effectExec->execs[idx].vExecDeadExecIDs.push_back(hitDeadArray[idx][i].GetInt());
					}
				}

                if (idx < ui_special_effects.Size())
                {
                    auto& uieffs = ui_special_effects[idx];
                    effectExec->execs[idx].uiSpecialEffects.resize(uieffs.Size());
                    for (int uiidx = 0; uiidx < uieffs.Size(); ++uiidx)
                    {
                        effectExec->execs[idx].uiSpecialEffects[uiidx] = uieffs[uiidx].GetInt();
                    }
                }

                if (bodyEffects.IsArray() && idx < bodyEffects.Size())
                {
                    auto& bodyeffs = bodyEffects[idx];
                    if (!bodyeffs.IsArray())
                        continue;
                    effectExec->execs[idx].bodyEffects.resize(bodyeffs.Size());
                    for (int bodyidx = 0; bodyidx < bodyeffs.Size(); ++bodyidx)
                    {
                        effectExec->execs[idx].bodyEffects[bodyidx] = bodyeffs[bodyidx].GetInt();
                    }
                }

                if (params.IsArray() && idx < params.Size())
                {
                    auto& param = params[idx];
                    if (!param.IsArray())
                        continue;
                    effectExec->execs[idx].params.resize(param.Size());
                    for (int paramIdx = 0; paramIdx < param.Size(); ++paramIdx)
                    {
                        effectExec->execs[idx].params[paramIdx] = param[paramIdx].GetInt();
                    }
                }

                /// 12.0
                if (uiStartEndEffects.IsArray() && idx < uiStartEndEffects.Size())
                {
                    auto& anSe = uiStartEndEffects[idx];
                    if (anSe.IsArray() && anSe.Size() >= 2)
                    {
                        effectExec->execs[idx].uiStartEffect = anSe[0u].GetInt();
                        effectExec->execs[idx].uiEndEffect = anSe[1].GetInt();
                    }
                }
                ///
			}

			rapidjson::Value& array = effectData["conf"];
			effectExec->target = array[0u].GetInt();
			effectExec->range.resize(4);
			effectExec->range[0u] = array[1].GetInt();
			effectExec->range[1] = array[2].GetInt();
			effectExec->range[2] = array[3].GetInt();
			effectExec->range[3] = array[4].GetInt();
			effectExec->period = array[5].GetInt();
			effectExec->isBuff = array[6].GetInt();
			effectExec->deadClear = array[7].GetInt();
			effectExec->stateAnimation = array[8].GetInt();
			effectExec->resumeToOrig = array[9].GetInt();
			effectExec->ifReact = array[10].GetInt();
			effectExec->recurrenceGrowTimeValue = array[11].GetInt();

            // 8.0 add
            auto &targetparam = array[12];
            if (targetparam.IsArray())
            {
                for (int idx = 0; idx < targetparam.Size(); idx++)
                    effectExec->targetParams.push_back(targetparam[idx].GetInt());
            }

            // 9.0 add
            // TODO:remove in 10.0
            if (array.Size() > 13)
            {
                effectExec->coverBySameType = array[13].GetInt();
            }
            
            // MARK: 9.0 add
            if (array.Size() > 14)
            {
                // TODO: impl haha
                effectExec->stateAnimationParam = array[14].GetInt();
            }

			// MARK: 11.0 add
			if (array.Size() > 15)
			{
				// TODO: impl haha heheda
				rapidjson::Value& optionparam = array[15];
				if (optionparam.IsArray())
				{
					effectExec->optionLevelUp = 0;
					effectExec->optionLevelUpSkillID = 0;

					for (int idx = 0; idx < optionparam.Size(); ++idx)
					{
						if (idx == 0)
						{
							effectExec->optionLevelUp = (EEffectOptionLvUpType)optionparam[idx].GetInt();
						}
						else if (idx == 1)
						{
							effectExec->optionLevelUpSkillID = optionparam[idx].GetInt();
							break;
						}
					}
				}
			}
            
            /// 12.0
            if (array.Size() > 16)
            {
                effectExec->followSkillTick = array[16].GetInt();
            }
            ///

			/// OP20.1
			if (array.Size() > 17)
			{
				effectExec->autoClear = array[17].GetInt();
			}
			///
            
            /// 12.0
            //auto &theExcludeVal = effectData["et"];
            //if (!theExcludeVal.IsNull())
            //{
            //    effectExec->targetExclude = theExcludeVal[0u].GetInt();
            //    effectExec->excludeX1 = theExcludeVal[1].GetInt();
            //    effectExec->excludeX2 = theExcludeVal[2].GetInt();
            //    effectExec->excludeZ1 = theExcludeVal[3].GetInt();
            //    effectExec->excludeZ2 = theExcludeVal[4].GetInt();
            //    auto &theExcludeParamVal = theExcludeVal[5];
            //    if (theExcludeParamVal.IsArray())
            //    {
            //        for (int idx = 0; idx < theExcludeParamVal.Size(); idx++)
            //            effectExec->targetExcludeParams.push_back(theExcludeParamVal[idx].GetInt());
            //    }
            //}
            ///

            if (effectExec->stateAnimation > 1000)
            {
                effectExec->stateAnimation -= 1000;
                effectExec->canStateAnimationBeDebuffed = false;
            }
		
			rapidjson::Value& colorArray = effectData["color"];
			effectExec->red = (unsigned char)colorArray[0u].GetInt();
			effectExec->green = (unsigned char)colorArray[1].GetInt();
			effectExec->blue = (unsigned char)colorArray[2].GetInt();

            /// 14.0
            if (effectData.HasMember("group"))
            {
                rapidjson::Value &groupData = effectData["group"];
                if (groupData.IsArray() && groupData.Size() >= 3) {
                    effectExec->groupID = groupData[0u].GetInt();
                    effectExec->groupCoverRule = groupData[1].GetInt();
                    effectExec->groupWeight = groupData[2].GetInt();
                }
            }
            ///

			_effectExecMap[effectId] = effectExec;
			return effectExec;
		}

		return nullptr;
	}

	ComboAnimationExec* AIDataHelper::getComboAnimationExec(int id)
	{
        std::unordered_map<int, ComboAnimationExec*>::iterator iter = _comboAniMap.find(id);
		if (iter != _comboAniMap.end())
		{
			return (ComboAnimationExec*)(iter->second);
		}

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::combo_animation.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_combo_animation");
#endif // DENA_OP3

		if (doc)
		{
			std::string idStr = stringWithFormat("%d", id);
            if (!doc->HasMember(idStr.c_str()))
                return nullptr;
			rapidjson::Value& comboData = doc->operator[](idStr.c_str());
            if (comboData.IsNull())
            {
#ifdef WIN32
                idStr = "combo_animation " + idStr + " not found!";
                //CCASSERT(0, idStr.c_str());
#endif
                return nullptr;
            }

			ComboAnimationExec *comboAni = new ComboAnimationExec();

			comboAni->execId = id;

			comboAni->comboAniVec.resize(comboData.Size());
			for (int idx = 0; idx < comboData.Size(); idx++)
			{
				rapidjson::Value& data = comboData[idx];
				comboAni->comboAniVec[idx].startTick = data["t1"].GetInt();
				comboAni->comboAniVec[idx].duration = data["t2"].GetInt();
				comboAni->comboAniVec[idx].type = data["type"].GetInt();
				comboAni->comboAniVec[idx].targetState = data["s"].GetString();
				comboAni->comboAniVec[idx].targetDirection = data["d"].GetInt();
				comboAni->comboAniVec[idx].roleid = data["r"].GetInt();
				comboAni->comboAniVec[idx].aniExecId = data["f"].GetInt();
				comboAni->comboAniVec[idx].posX = data["x"].GetInt();
				comboAni->comboAniVec[idx].posY = data["y"].GetInt();
				comboAni->comboAniVec[idx].selfDirection = data["d2"].GetInt();
			}

			return comboAni;
		}

		return nullptr;
	}

	ComboAudioExec* AIDataHelper::getComboAudioExec(int id)
	{
        std::unordered_map<int, ComboAudioExec*>::iterator iter = _comboAudioMap.find(id);
		if (iter != _comboAudioMap.end())
		{
			return (ComboAudioExec*)(iter->second);
		}

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::combo_audio.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_combo_audio");
#endif // DENA_OP3
		if (doc)
		{
			std::string idStr = stringWithFormat("%d", id);
            if (!doc->HasMember(idStr.c_str()))
                return nullptr;
			rapidjson::Value& comboData = doc->operator[](idStr.c_str());
            if (comboData.IsNull())
            {
#ifdef WIN32
                idStr = "combo_audio " + idStr + " not found!";
                //CCASSERT(0, idStr.c_str());
#endif
                return nullptr;
            }

			ComboAudioExec *comboAni = new ComboAudioExec();

			comboAni->execId = id;

			comboAni->comboAudioVec.resize(comboData.Size());
			for (int idx = 0; idx < comboData.Size(); idx++)
			{
				rapidjson::Value& data = comboData[idx]["conf"];
				comboAni->comboAudioVec[idx].startTick = data[0u].GetInt();
				comboAni->comboAudioVec[idx].targetSpecialEffectId = data[1].GetInt();
				comboAni->comboAudioVec[idx].targetAudioEffectId = data[2].GetInt();
				comboAni->comboAudioVec[idx].duration = data[3].GetInt();
				comboAni->comboAudioVec[idx].frequency = data[4].GetInt();
				//comboAni->comboAudioVec[idx].roleid = data[5].GetInt();
			}

			return comboAni;
		}

		return nullptr;
	}

	ComboHitExec* AIDataHelper::getComboHitExec(int id)
	{
        std::unordered_map<int, ComboAudioExec*>::iterator iter = _comboAudioMap.find(id);
		if (iter != _comboAudioMap.end())
		{
			return (ComboHitExec*)(iter->second);
		}

#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::combo_hit_num.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_combo_hit_num");
#endif // DENA_OP3	
		if (doc)
		{
			std::string idStr = stringWithFormat("%d", id);
            if (!doc->HasMember(idStr.c_str()))
                return nullptr;
			rapidjson::Value& comboData = doc->operator[](idStr.c_str());
            if (comboData.IsNull())
            {
#ifdef WIN32
                idStr = "combo_hit_exec " + idStr + " not found!";
                //CCASSERT(0, idStr.c_str());
#endif
                return nullptr;
            }

			ComboHitExec *comboAni = new ComboHitExec();

			comboAni->execId = id;

			comboAni->comboHitVec.resize(comboData.Size());
			for (int idx = 0; idx < comboData.Size(); idx++)
			{
				rapidjson::Value& data = comboData[idx]["conf"];
				comboAni->comboHitVec[idx].startTick = data[0u].GetInt();
				comboAni->comboHitVec[idx].duration = data[1].GetInt();
				comboAni->comboHitVec[idx].increaseFrequency = data[2].GetInt();

				rapidjson::Value& zbData = comboData[idx]["zb"];
				comboAni->comboHitVec[idx].posX = zbData[0u].GetInt();
				comboAni->comboHitVec[idx].posY = zbData[1].GetInt();
			}

			return comboAni;
		}

		return nullptr;
	}
	
    std::unordered_map<int, ComboTrigger*>& AIDataHelper::checkComboTriggerMap()
	{
		if (_comboTriggerMap.size() == 0)
		{
#ifdef DENA_OP3
            rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::combo_trigger.c_str());
#else
            rapidjson::Document* doc = AIDataManager::getJsonFromFile("_combo_trigger");
#endif // DENA_OP3	
			if (doc)
			{
				for (int idx = 0; idx < doc->Size(); ++idx)
				{
					ComboTrigger *comboTrigger = new ComboTrigger();
					comboTrigger->ID = doc->operator[](idx)["_id"].GetInt();
					comboTrigger->triggerSkill = doc->operator[](idx)["s"].GetInt();
					comboTrigger->file = doc->operator[](idx)["f"].GetString();

					rapidjson::Value& p = doc->operator[](idx)["p"];
					comboTrigger->team.resize(p.Size());
					for (int i = 0; i < p.Size(); i++)
					{
						comboTrigger->team[i] = p[i].GetInt();
					}

					rapidjson::Value& conf = doc->operator[](idx)["conf"];
					comboTrigger->rate = conf[0u].GetInt();
					comboTrigger->totalPeriod = conf[1u].GetInt();
					comboTrigger->hurtPeriod = conf[2u].GetInt();
					comboTrigger->hurtID = conf[3u].GetInt();
					comboTrigger->juexing = conf[4u].GetInt();
					comboTrigger->levlup = conf[5u].GetInt();
					
					_comboTriggerMap[comboTrigger->triggerSkill] = comboTrigger;
				}
			}
		}
		return _comboTriggerMap;
	}

	ComboTrigger* AIDataHelper::getComboTrigger(int comboID)
	{
		checkComboTriggerMap();

		if (_comboTriggerMap.find(comboID) != _comboTriggerMap.end())
		{
			return (ComboTrigger*)(_comboTriggerMap.find(comboID)->second);
		}

		return nullptr;
	}

	SkillExecIDType AIDataHelper::getSkillExecType(int execId)
	{
		if (execId <= 0)
			return eSkillExecIDTypeEnd;

		if (IsAnimationExec(execId))
			return eAnimationExec;
		else if (IsHurtExec(execId))
			return eHurtExec;
		else if (IsMoveExec(execId))
			return eMoveExec;
		else if (IsProjectileExec(execId))
			return eProjectTileExec;
		else if (IsEffectExec(execId))
			return eEffectExec;
		else if (IsSpecialUIEffectExec(execId))
			return eSpecialUIEffectExec;
		else if (IsSoundExec(execId))
			return eSoundExec;
		else if (IsSpecialExec(execId))
			return eSpecialExec;
		else
			return eSkillExecIDTypeEnd;
	}
    
    void AIDataHelper::getBeHittedAddEnergy(int skillId,int &useSkillAdd,int &beHittedAdd)
    {
        std::unordered_map<int, std::pair<int, int>>::iterator ite = _beHittedAddEnergyMap.find(skillId);
        if (ite != _beHittedAddEnergyMap.end())
        {
            useSkillAdd = ite->second.first;
            beHittedAdd = ite->second.second;
            return;
        }
        
        AISkill * skill = AIDataHelper::getSkillData(skillId);
        int hurtExecId = 0;
        int projectileExecId = 0;
        if(skill && skill->skillExec.size() > 0)
        {
            std::vector<SkillExec>::iterator iter;
            for (iter = skill->skillExec.begin(); iter != skill->skillExec.end(); ++iter)
            {
                 if (IsHurtExec(iter->execId))
                 {
                     hurtExecId = iter->execId;
                     break;
                 }
      
                 if (IsProjectileExec(iter->execId))
                 {
                     projectileExecId = iter->execId;
                     break;
                 }
                 
            }
            
            if(projectileExecId > 0)
            {
                ProjectileExec* pProjectileExec =  AIDataHelper::getProjectileExec(projectileExecId);
                if(pProjectileExec)
				{
					// hurtExecId = pProjectileExec->hitExec;
					for (int indx = 0; indx < pProjectileExec->hitExecVec.size(); indx++)
					{
						auto hurtId = pProjectileExec->hitExecVec[indx];
						if (hurtId > 0 && IsHurtExec(hurtId))
						{
							HurtExec* hurtData = AIDataHelper::getHurtExec(hurtId);
							if (hurtData)
							{
								_beHittedAddEnergyMap.insert(std::make_pair(skillId, std::make_pair(hurtData->sourceEnergyAdd, hurtData->targetEnergyAdd)));
								useSkillAdd += hurtData->sourceEnergyAdd;
								beHittedAdd += hurtData->targetEnergyAdd;
							}
						}
					}
				}
			}

			else if (hurtExecId > 0 && IsHurtExec(hurtExecId))
            {
                HurtExec* hurtExec = AIDataHelper::getHurtExec(hurtExecId);
                if(hurtExec)
                {
                    _beHittedAddEnergyMap.insert(std::make_pair(skillId, std::make_pair(hurtExec->sourceEnergyAdd, hurtExec->targetEnergyAdd) ));
                    useSkillAdd = hurtExec->sourceEnergyAdd;
                    beHittedAdd = hurtExec->targetEnergyAdd;
                }
                
            }

			/*if(hurtExecId > 0)
			{
			HurtExec* hurtExec = AIDataHelper::getHurtExec(hurtExecId);
			if(hurtExec)
			{
			_beHittedAddEnergyMap.insert(std::make_pair(skillId, std::make_pair(hurtExec->sourceEnergyAdd, hurtExec->targetEnergyAdd) ));
			useSkillAdd = hurtExec->sourceEnergyAdd;
			beHittedAdd = hurtExec->targetEnergyAdd;
			}

			}*/
        }
        
    }
    
    void AIDataHelper::loadConverAttrListTable()
    {
        if (_convertAttrVec.size() > 0) {
            return;
        }
#ifdef DENA_OP3
        rapidjson::Document* doc = AIDataManager::getJsonFromFile(AIDataHelper::convert_attr_list.c_str());
#else
        rapidjson::Document* doc = AIDataManager::getJsonFromFile("_convert_attr_list");
#endif // DENA_OP3

        if (doc) {
            for (int idx = 0; idx < doc->Size(); ++idx)
            {
                rapidjson::Value& array = doc->operator[](idx);
                
                ConvertAttrParams _convertParams;
                _convertParams.param0 = array[0u].GetInt();
                _convertParams.param1 = array[1].GetInt();
                _convertParams.param2 = (float)(array[2].GetDouble());
                
                _convertAttrVec.push_back(_convertParams);
                
            }
        }

    }
    
    std::vector<ConvertAttrParams>& AIDataHelper::getConvertAttrVec()
    {
        return AIDataHelper::_convertAttrVec;
    }

    void AIDataHelper::parseMeleeCombatBattleLog(const std::string &battle_log_str, MeleeCombatBattleInfo *out_info)
    {
        if (!out_info)
            return;

        rapidjson::Document doc;
        //doc.ParseStream(battle_log_str);
        doc.Parse<0>(battle_log_str.c_str());
        if (doc.HasParseError())
        {
            return;
        }

        if (doc.HasMember("buildcharacters"))
        {
            rapidjson::Value &buildcharacters = doc["buildcharacters"];
            for (int i = 0; i < buildcharacters.Size(); i++)
            {
                rapidjson::Value &characterjson = buildcharacters[i];

                MeleeCombatCharacterInfo c_info;

                /// attr
                rapidjson::Value &attrjson = characterjson["attr"];
                for (int j = 0; j < attrjson.Size(); j++)
                {
                    c_info.attrs.push_back(attrjson[j].GetInt());
                }

                /// skill
                rapidjson::Value &skilljson = characterjson["j"];
                for (int j = 0; j < skilljson.Size(); j++)
                {
                    rapidjson::Value &a_skilljson = skilljson[j];

                    int skid = a_skilljson["i"].GetInt();
                    int sklv = a_skilljson["lv"].GetInt();
                    
                    c_info.skills.push_back(std::make_pair(skid, sklv));
                }

                /// bqsl
                if (characterjson.HasMember("bqsl"))
                {
                    rapidjson::Value &bqsl = characterjson["bqsl"];
                    for (int j = 0; j < bqsl.Size(); j++)
                    {
                        rapidjson::Value &a_bqsl = bqsl[j];
                        for (int l = 0; l < a_bqsl.Size(); l++)
                        {
                            rapidjson::Value &a_bqslvar = a_bqsl[l];
                            int bqi = a_bqslvar["i"].GetInt();
                            int bqlv = a_bqslvar["lv"].GetInt();
                            c_info.bqsls.push_back(std::make_pair(bqi, bqlv));
                        }
                    }
                }

                /// other
                c_info.juexing = characterjson["x"].GetInt();
                if (characterjson.HasMember("k"))
                    c_info.k = characterjson["k"].GetInt();
                c_info.teamSide = characterjson["side"].GetInt();
                if (characterjson.HasMember("fp"))
                    c_info.fp = characterjson["fp"].GetInt();

                /// builddata
                rapidjson::Value &builddatajson = characterjson["builddata"];
                c_info.ifUltraLock = builddatajson["ifUltraLock"].GetInt();
                c_info.clusterNum = builddatajson["clusterNum"].GetInt();
                c_info.posX = builddatajson["px"].GetInt();
                c_info.posY = builddatajson["py"].GetInt();

                /// 
                out_info->allcharacters.push_back(c_info);
            }
        }

        if (doc.HasMember("battlerect"))
        {
            rapidjson::Value &battlerect = doc["battlerect"];
            out_info->battleRectX = battlerect["x"].GetInt();
            out_info->battleRectY = battlerect["y"].GetInt();
            out_info->battleRectWidth = battlerect["width"].GetInt();
            out_info->battleRectHeight = battlerect["height"].GetInt();
        }

        if (doc.HasMember("deltay"))
        {
            rapidjson::Value &deltay = doc["deltay"];
            out_info->battleMinY = deltay["min"].GetInt();
            out_info->battleMaxY = deltay["max"].GetInt();
        }

        if (doc.HasMember("winlosecheck"))
        {
            rapidjson::Value &winlosecheck = doc["winlosecheck"];
            for (int i = 0; i < winlosecheck.Size(); i++)
            {
                rapidjson::Value &a_winlosejson = winlosecheck[i];
                int clusternum = a_winlosejson["clusterNum"].GetInt();
                out_info->conditions[clusternum] = a_winlosejson["t"].GetInt();
            }
        }

        if (doc.HasMember("seed"))
        {
            out_info->seed = doc["seed"].GetInt();
        }

        if (doc.HasMember("bkey"))
        {
            out_info->businesstype = doc["bkey"].GetInt();
        }

        if (doc.HasMember("maxtick"))
        {
            out_info->maxticklimit = doc["maxtick"].GetInt();
        }
    }

	template<typename KEY, typename VALUE>
    void deleteAllMapItem(std::unordered_map<KEY, VALUE> &map)
	{
        for (typename std::unordered_map<KEY, VALUE>::iterator iter = map.begin(); iter != map.end(); ++iter)
		{
			VALUE pV = iter->second;
			delete pV;
		}
		map.clear();
	}

	void AIDataHelper::unload()
	{
		deleteAllMapItem(_moveExec_cache);
		deleteAllMapItem(_hurtExec_cache);
		deleteAllMapItem(_projectileExec_cache);
		deleteAllMapItem(_aiSkill_cache);
		deleteAllMapItem(_characterDataMap);
		deleteAllMapItem(_specialEffExecMap);
		deleteAllMapItem(_animationExecMap);
		deleteAllMapItem(_effDataMap);
		deleteAllMapItem(_labelEffExecMap);
		deleteAllMapItem(_skillExecVecMap);
		deleteAllMapItem(_effectExecMap);
        deleteAllMapItem(_markExecMap);
        deleteAllMapItem(_markEffectExecMap);
		deleteAllMapItem(_baqiSpecialExecMap);
		deleteAllMapItem(_soundExecMap);
		deleteAllMapItem(_logicAniMap);
		deleteAllMapItem(_logicEffMap);
		deleteAllMapItem(_specialExecMap);
        deleteAllMapItem(_bodyEffectExecMap);

		deleteAllMapItem(_pve_guanqia_character_Map);
		deleteAllMapItem(_rewardCandidateMap);
		deleteAllMapItem(_pve_guanqia_character_reward_Map);
		deleteAllMapItem(_comboTriggerMap);
		deleteAllMapItem(_comboAniMap);
		deleteAllMapItem(_comboAudioMap);
		deleteAllMapItem(_comboHitMap);
        deleteAllMapItem(_summonExecMap);
        deleteAllMapItem(_summonPropertyMap);
        deleteAllMapItem(_transformExecMap);
        
        for (auto iter = _markLevelMap.begin(); iter != _markLevelMap.end(); ++iter)
        {
            deleteAllMapItem(iter->second);
        }
        _markLevelMap.clear();
        
        _beHittedAddEnergyMap.clear();

        battleFieldRangeToCenterX = -1;
        aiUpdateModeAfterWinlose = -1;
        aiCenterXMode = -1;
        aiUltraBrokenMode = -1;
        aiUiEffectMode = -1;
        delete aiCenterXCache;
        aiCenterXCache = nullptr;
	}
}
