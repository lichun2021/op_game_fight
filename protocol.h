#ifndef PROTOCOL_HHHHHHHHH
#define PROTOCOL_HHHHHHHHH
//struct define begin

//userrespdata
//{
//	userid:uint64,
//	uservalue:uint64,
//	pos:uint32,
//  str_size:uint8,
//  str:char[str_size]
//} 
//userreqdata
//{
//	userid:uint64,
//	uservalue:uint64,
//  str_size:uint8,
//  str:char[str_size]
//} 
//struct define end
//----------------------------------------------------------------------------------
// msg define begin

// {msg_size:uint32,msg_type:uint32,transid:uint32,businessKey:uint64,pos:uint32,size:uint32}
#define RANK_MSG_RANK_RANGE_REQ  10000
// {msg_size:uint32,msg_type:uint32,transid:uint32,len:uint32,data:userrespdata[len]}
#define RANK_MSG_RANK_RANGE_RESP  10001

// {msg_size:uint32,msg_type:uint32,transid:uint32,businessKey:uint64,userid:uint64,size:uint32}
#define RANK_MSG_USER_AROUND_REQ  10002
// {msg_size:uint32,msg_type:uint32,transid:uint32,len:uint32,data:userrespdata[len]}
#define RANK_MSG_USER_AROUND_RESP  10003

// {msg_size:uint32,msg_type:uint32,transid:uint32,businessKey:uint64,data:userreqdata,size:uint32}
#define RANK_MSG_SET_USER_REQ  10004
// {msg_size:uint32,msg_type:uint32,transid:uint32,len:uint32,data:userrespdata[len]}
#define RANK_MSG_SET_USER_RESP  10005

// {msg_size:uint32,msg_type:uint32,transid:uint32,businessKey:uint64,len:uint32,data:userreqdata[len]}
#define RANK_MSG_SET_MULTI_USER_REQ  10006
// {msg_size:uint32,msg_type:uint32,transid:uint32,len:uint32,data:userrespdata[len]}
#define RANK_MSG_SET_MULTI_USER_RESP  10007
// {msg_size:uint32,msg_type:uint32, transid:uint32}
#define RANK_MSG_GET_TOTAL_USERS_REQ 10008
// {msg_size:uint32, msg_type:uint32, transid:uint32, number:uint32}
#define RANK_MSG_GET_TOTAL_USERS_RESP 10009
// {msg_size:uint32,msg_type:uint32, transid:uint32}
#define RANK_MSG_MOVE_FILE_REQ 10010
// {msg_size:uint32, msg_type:uint32, transid:uint32, status:uint32}
#define RANK_MSG_MOVE_FILE_RESP 10011
// {msg_size:uint32,msg_type:uint32, transid:uint32, b_key:uint32}
#define RANK_MSG_REMOVE_FILE_REQ 10012
// {msg_size:uint32, msg_type:uint32, transid:uint32, status:uint32}
#define RANK_MSG_REMOVE_FILE_RESP 10013

/*
  added by dy
 */
//{msg_size:uint32, msg_type:uint32, transid:uint32, businesskey:uint64, cnt_per_block:uint32, cnt_block:uint32}
#define RANK_MSG_ADD_BUSINESS_REQ 10014

//{msg_size:uint32, msg_type:uint32, transid:uint32, businesskey:uint6}
#define RANK_MSG_REMOVE_BUSINESS_REQ 10015

  //{msg_size:uint32, msg_type:uint32, transid:uint32, fight:json}
#define FIGHT_MSG_FIGHT_CHECK_REQ 10016

//{msg_size:uint32, msg_type:uint32, transid:uint32, resp:json}
#define FIGHT_MSG_FIGHT_CHECK_RESP 10017

#define FIGHT_MSG_FIGHT_DOUJI_REQ 10018

#define FIGHT_MSG_FIGHT_DOUJI_RESP 10019

#define FIGHT_MSG_FIGHT_DIANFENG_REQ 10020

#define FIGHT_MSG_FIGHT_DIANFENG_RESP 10021

#define FIGHT_MSG_FIGHT_CARWHEEL_REQ 10022

#define FIGHT_MSG_FIGHT_CARWHEEL_RESP 10023

#define FIGHT_MSG_FIGHT_HUIYI_REQ 10024

#define FIGHT_MSG_FIGHT_HUIYI_RESP 10025

#define FIGHT_MSG_FIGHT_ALLIANCE_PVE_REQ 10026

#define FIGHT_MSG_FIGHT_ALLIANCE_PVE_RESP 10027

#define FIGHT_MSG_FIGHT_SHIP_KING_PVE_REQ 10028

#define FIGHT_MSG_FIGHT_SHIP_KING_PVE_RESP 10029

// msg define end
#endif
