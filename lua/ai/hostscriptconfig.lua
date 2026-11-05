-- hostscriptconfig
local is_searchtargetbylua = require "targets/targetconfig"

local innereffconfig = require "effects/innereffconfig"
local is_innereffectbylua = innereffconfig.is_innereffectbylua

local exechelper = require "exechelper"
local is_castersyncattackspeed = exechelper.is_castersyncattackspeed

local hitlabelhelper = require "hitlabelhelper"
local get_hitlabeldisplaytype = hitlabelhelper.get_hitlabeldisplaytype

return {
    beginskill_lazytick = 5, -- 开场释放非保护性技能延迟
    is_searchtargetbylua = is_searchtargetbylua,
    is_hurtcalc_exerthurtbylua = false,
    is_hurtcalc_hurttarget_bylua = false,
    is_innereffectbylua = is_innereffectbylua,
    is_inneffapplyhpchangebylua = false,
    is_projectilecaster_throwprojectile_bylua = false,
    is_attackaction_update_bylua = false,
    is_attackaction_enter_bylua = false,
    is_actionselector_bylua = true,
    is_locktarget_bylua = false,
    is_cancastultra_bylua = false,
    is_checktarget_bylua = false,
    is_pickskill_bylua = true,
    is_preload_creature = false,
    is_use_new_targethelper = true,
    is_ignore_positive_hurtvalue_hitlabel_and_stat = true,
    is_stop_exec_when_being_final_clear = true,
    is_captain_selector_bylua = false,
    is_reshow_uieffects_when_revived = true,
    is_reshow_markicons_when_revived = true,
    is_castersyncattackspeed = is_castersyncattackspeed,
    is_use_cannotbeattackbuffcnt = true,
    is_use_newdispel = true,
    -- is_effect_175_disable_limit = true,
    get_hitlabeldisplaytype = get_hitlabeldisplaytype,
    cafut = 1 -- check alive for ultra type
}
