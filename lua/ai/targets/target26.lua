-- 26 随机一个敌人，并且若是持续伤害，每次伤害不再重新随机目标
--[[
    CharacterData *AIActionController::getTargetOneEnemyRandomContinueHurtNotRandomTarget(AIContext &aiContext, CharacterData *source, CharacterDataList *characterList, bool ignoreCanbeattackEffect) {
        if (source->GetFlagFirstFound26Target() && source->GetFirstRoundConfirm26Target())
            return source->GetFirstRoundConfirm26Target();

        return getRandomCharacter(aiContext, *source, characterList, ignoreCanbeattackEffect);
    }
]]

return function(params)
    if params.source:is_target26foundfirst() then
        local thetarget = params.source:get_target26firstfoundtarget()
        if thetarget ~= nil then return {thetarget} end
    end

    local characterlist = g_targethelper.pick_filteredrivallist(params)
    return g_targethelper.get_randomcharacter(characterlist, params)
end