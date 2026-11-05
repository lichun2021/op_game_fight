-- hitlabel
local rethelper = {}

local theaihitlabel = aihitlabel

rethelper.execute = function(labelid, labelnum, character)
    theaihitlabel.execute(labelid, labelnum, character)
end

-- 0-直线飘字 1-贝塞尔1 2-贝塞尔2
rethelper.get_hitlabeldisplaytype = function(labeltype)
    -- 类型(0-伤害，1-暴击，2-加血，3-加血暴击，4-抵抗，5-法术免疫，6-格挡，7-闪避，8-伤害吸收，9-未命中，10-物理免疫)
    if labeltype == 0 or labeltype == 2 then
        return 0
    elseif labeltype == 1 or labeltype == 3 then
        return 0
    end

    return 0
end

return rethelper