接口定义如下：

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


//获取一定范围内用户
// {msg_size:uint32,msg_type:uint32,transid:uint32,businessKey:uint64,pos:uint32,size:uint32}
#define RANK_MSG_RANK_RANGE_REQ  10000
// {msg_size:uint32,msg_type:uint32,transid:uint32,len:uint32,data:userrespdata[len]}
#define RANK_MSG_RANK_RANGE_RESP  10001


//获取某一个用户周围用户
// {msg_size:uint32,msg_type:uint32,transid:uint32,businessKey:uint64,userid:uint64,size:uint32}
#define RANK_MSG_USER_AROUND_REQ  10002
// {msg_size:uint32,msg_type:uint32,transid:uint32,len:uint32,data:userrespdata[len]}
#define RANK_MSG_USER_AROUND_RESP  10003


//插入一个用户
// {msg_size:uint32,msg_type:uint32,transid:uint32,businessKey:uint64,data:userreqdata,size:uint32}
#define RANK_MSG_SET_USER_REQ  10004
// {msg_size:uint32,msg_type:uint32,transid:uint32,len:uint32,data:userrespdata[len]}
#define RANK_MSG_SET_USER_RESP  10005


//插入一堆用户
// {msg_size:uint32,msg_type:uint32,transid:uint32,businessKey:uint64,len:uint32,data:userreqdata[len]}
#define RANK_MSG_SET_MULTI_USER_REQ  10006
// {msg_size:uint32,msg_type:uint32,transid:uint32,len:uint32,data:userrespdata[len]}
#define RANK_MSG_SET_MULTI_USER_RESP  10007