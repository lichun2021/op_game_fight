
local TalentTrigger = require "hurtfuncs/talenttrigger"

return function (source, target, triggertype)
    if source ~= nil then 
        local talentlist = source:get_talentlist()
        for i=1,#talentlist do 
            local a_talent = talentlist[i]
            local talentid = a_talent[1]
            local talentlv = a_talent[2]
            local baqiexec = g_datahelper.get_baqispecialexec(talentid, talentlv)
            if baqiexec ~= nil and talentid > 0 then 
                if triggertype == TalentTrigger.IN_EFFECT_ACTION or triggertype == TalentTrigger.ALWAYS or triggertype == TalentTrigger.HIT then 
                    if baqiexec.special_condition[1] == triggertype and baqiexec.special_condition[3] > 0 then 
                        g_exechelper.execute(baqiexec.special_condition[3], talentid, talentlv, source, nil, source, 0)
                    end 
                end 
            end 
        end 
    end 

    if target ~= nil then 
        local talentlist = target:get_talentlist()
        for i=1,#talentlist do 
            local a_talent = talentlist[i]
            local talentid = a_talent[1]
            local talentlv = a_talent[2]
            local baqiexec = g_datahelper.get_baqispecialexec(talentid, talentlv)
            if baqiexec ~= nil and talentid > 0 then 
                if triggertype == TalentTrigger.BY_HURL or triggertype == TalentTrigger.DODGE or triggertype == TalentTrigger.HIT then 
                    if baqiexec.special_condition[1] == triggertype and baqiexec.special_condition[3] > 0 then 
                        g_exechelper.execute(baqiexec.special_condition[3], talentid, talentlv, target, nil, target, 0)
                    end 
                end 
            end 
        end 
    end 
end
