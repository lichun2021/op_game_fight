local HurtParamHelper = {}

--[[
        AIProjectile *projectile = nullptr;
        int skillID = 0;
        unsigned short skillLevel = 0;
        int hitType = NORMAL_HIT;
        int HPDelta = 0;
        int hurtProperty = HurtPropertyType::kNoProperty;
        int hurtType = PHYSICAL_HURT_TYPE;
        HurtExec *hurtExec = nullptr;
        bool shouldSkipRebound = false;
        bool ignoreHurtLabel = false;
        bool isFromEffectCaster = false;
        bool isRawHurt = false;
        bool isSimulateCalc = false;
        bool isFromSacrificeRecv = false;
        CharacterData *statHurtSourceAlt = nullptr; // 指定伤害统计角色
        int markID = 0;//伤害来源是标记的则有标记id
        int casterID = 0;//伤害的直接来源执行id
        int alternateHitLabel = 0;

        HurtCalcParam(AIProjectile *proj, int skId, unsigned short skLv, int hpdel, int hurtTp, int theCasterId)
            : projectile(proj), skillID(skId), skillLevel(skLv), HPDelta(hpdel), hurtType(hurtTp), casterID(theCasterId) {}

        HurtCalcParam(AIProjectile *proj, int skId, unsigned short skLv, int hpdel, int hurtTp, int hurtProp, int hitTp, HurtExec *hurtExec, bool skipRebound, int theCasterId)
            : projectile(proj), skillID(skId), skillLevel(skLv), HPDelta(hpdel), hurtType(hurtTp), hurtProperty(hurtProp), hitType(hitTp), hurtExec(hurtExec), shouldSkipRebound(skipRebound), casterID(theCasterId) {}
]]
HurtParamHelper.new = function(projectile, skillid, skill_level, hpdelta, hurttype, hurtprop, hittype, hurtid, is_skiprebound, casterid)
    local hurtparam = {}
    hurtparam.projectile = projectile
    hurtparam.skillid = skillid
    hurtparam.skill_level = skill_level
    hurtparam.hpdelta = hpdelta
    hurtparam.inithpdelta = hpdelta
    hurtparam.hurttype = hurttype
    hurtparam.hurtprop = hurtprop
    hurtparam.hittype = hittype
    hurtparam.hurtid = hurtid
    hurtparam.is_skiprebound = is_skiprebound
    hurtparam.is_ignorehitlabel = false
    hurtparam.casterid = casterid
    hurtparam.is_simu = false
    hurtparam.is_fromsacrificerecv = false
    hurtparam.is_fromeffect = false
    hurtparam.is_rawhurt = false
    hurtparam.alternatehitlabel = 0
    return hurtparam
end

return HurtParamHelper