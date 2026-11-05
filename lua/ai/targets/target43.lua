-- 43 绝对坐标内的所有敌军
-- 绝对坐标是指，根据本阵营的出场方向，面朝敌人出场方向为x轴正方向，屏幕上方为y轴正方向，以屏幕正中心为坐标原点
return function(params)
    return g_targethelper.get_character_in_battlefield_absolute_range(g_targethelper.pick_filteredrivallist(params), params)
end