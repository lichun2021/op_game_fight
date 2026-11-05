-- teamlisttype

local TeamListType = {}

TeamListType.CharacterTeamListType = {
    CTEAM_FRIENDS = 0,
    CTEAM_TMP_FRIENDS = 1,
    CTEAM_RIVALS = 2,
    CTEAM_TMP_RIVALS = 3
}

TeamListType.AIContextTeamListType = {
    AITEAM_ORIGIN = 0,
    AITEAM_FRIENDS_FOR_CLUSTER = 1,
    AITEAM_RIVALS_FOR_CLUSTER = 2
}

return TeamListType