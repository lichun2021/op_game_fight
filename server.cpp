#include "server.h"
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include "stdarg.h"
#include <string.h>
#include <assert.h>
#include <execinfo.h>
#include "AI/CommonType.h"
#include "AI/AIContext.h"
#include "AI/AIWinLoseCheck.h"
#include "AI/AICallback.h"
#include "AI/AIDataManager.h"
#include "AI/BattleLog.h"
#include "AI/AICharacterData.h"
#include "AI/BattleAttributeSchema.h"
#include "AI/action_hero/HeroActionController.h"
#include "jsonx/reader.h"
#include "ServerVerifyHelper.h"

class AIContext;
extern int Dir_index;
int serializIndex=0;
uint64_t totaltime;
unsigned int totalcount;
unsigned int maxtime=0;
uint64_t ntohl64(uint64_t arg64)
{
	uint64_t res64;
#ifdef __BYTE_ORDER
	#if __BYTE_ORDER==__LITTLE_ENDIAN
		uint32_t low = (uint32_t) (arg64 & 0x00000000FFFFFFFFLL);
		uint32_t high = (uint32_t) ((arg64 & 0xFFFFFFFF00000000LL) >> 32);
		low = ntohl(low);
		high = ntohl(high);
		res64 = (uint64_t) high + (((uint64_t) low) << 32);
	#else
		res64 = arg64;
	#endif
#endif
	return res64;
} 


uint64_t htonl64(uint64_t arg64)
{
	uint64_t res64;
#ifdef __BYTE_ORDER
	#if __BYTE_ORDER==__LITTLE_ENDIAN
		uint32_t low = (uint32_t) (arg64 & 0x00000000FFFFFFFFLL);
		uint32_t high = (uint32_t) ((arg64 & 0xFFFFFFFF00000000LL) >> 32);
		//cout<<"htonl64:"<<arg64<<" low:"<<low<<" high:"<<high<<endl;
		low = htonl(low);
		high = htonl(high);
		res64 = (uint64_t) high + (((uint64_t) low) << 32);
	#else
		res64 = arg64;
	#endif
#endif
	return res64;
}

msg_pkg::msg_pkg()
{
	buf=NULL;
	len=0;
	used=0;
}

msg_pkg::msg_pkg(char *src,int size)
{
	used=0;
	if(src !=NULL && size>0 && size<=BUFF_MAXSIZE)
	{
		len=size;
		this->buf=src;
	}
	else
	{
		len=0;
		this->buf=NULL;
	}
}

msg_pkg::msg_pkg(const msg_pkg& other)
{
	buf=other.buf;
	len=other.len;
	used=other.used;
}

msg_pkg &msg_pkg::operator =(const msg_pkg &other)
{
	buf=other.buf;
	len=other.len;
	used=other.used;
}

unsigned int msg_pkg::read_uint32()
{
	int leftlen=len-used;
	if(leftlen<4)
	{
		throw readException();
	}
	unsigned int value=*((unsigned int *)(buf+used));
	//cout<<"read_uint32:"<<value<<endl;
	value=ntohl(value);
	//cout<<"read_uint32:"<<value<<":"<<used<<endl;
	used+=4;
	return value;
}

uint64_t msg_pkg::read_uint64()
{
	int leftlen=len-used;
	if(leftlen<8)
	{
		throw readException();
	}
	uint64_t value=*((uint64_t *)(buf+used));
	//cout<<"read_uint64:"<<value<<endl;
	value=ntohl64(value);
	//cout<<"read_uint64:"<<value<<":"<<used<<endl;
	used+=8;
	return value;
}

unsigned char msg_pkg::read_uint8()
{
	int leftlen=len-used;
	if(leftlen<1)
	{
		throw readException();
	}
	unsigned char value=*((unsigned char *)(buf+used));
	//cout<<"read_uint8:"<<int(value)<<endl;
	//cout<<"read_uint8:"<<int(value)<<":"<<used<<endl;
	used+=1;
	return value;
}

char* msg_pkg::read_string(unsigned int size)
{
	int leftlen=len-used;
	if(leftlen<size)
	{
		throw readException();
	}
	//cout<<"read_string:"<<size<<endl;
	char *ptr=buf+used;
	//cout<<"read_string:"<<used<<endl;
	used+=size;
	return ptr;
}

unsigned int msg_pkg::size()
{
	return len;
}

unsigned int msg_pkg::offset()
{
	return used;
}

int msg_pkg::write_uint32(unsigned int data)
{
	int leftlen=len-used;
	if(leftlen<4)
	{
		throw writeException();
	}
	unsigned int temp=htonl(data);
	memcpy((void *)(this->buf+this->used),(void *)(&temp),4);
	used+=4;
	return used;
}

int msg_pkg::write_uint64(uint64_t data)
{
	int leftlen=len-used;
	if(leftlen<8)
	{
		throw writeException();
	}
	uint64_t temp=htonl64(data);
	memcpy((void *)(this->buf+this->used),(void *)(&temp),8);
	used+=8;
	return used;
}

int msg_pkg::write_uint8(unsigned char data)
{
	int leftlen=len-used;
	if(leftlen<1)
	{
		throw writeException();
	}
	memcpy((void *)(this->buf+this->used),(void *)(&data),1);
	used+=1;
	return used;
}

int msg_pkg::write_string(char *data,unsigned int size)
{
	int leftlen=len-used;
	if(leftlen<size)
	{
		throw writeException();
	}
	memcpy((void *)(this->buf+this->used),(void *)(data),size);
	used+=size;
	return used;
}

int msg_pkg::seek(int offset)
{
	if(offset>this->len)
	{
		offset=this->len;
	}
	if(offset<0)
		offset=0;
	used=offset;
	return used;
}

bool msg_pkg::rebind(char *buff,int size)
{
	if(buff !=NULL && size>0 && size<=BUFF_MAXSIZE)
	{
		len=size;
		this->buf=buff;
		used=0;
		return true;
	}
	return false;
}

socketbuff::socketbuff():
out_msg()
{
	memset((void *)in_buf,0,2*BUFF_MAXSIZE);
	in_len=0;
	in_used=0;
	memset((void *)out_buf,0,BUFF_MAXSIZE);
	out_len=0;
	out_used=0;
	socket=0;
}

socketbuff::socketbuff(int socket):
out_msg()
{
	memset((void *)in_buf,0,2*BUFF_MAXSIZE);
	in_len=0;
	in_used=0;
	memset((void *)out_buf,0,BUFF_MAXSIZE);
	out_len=0;
	out_used=0;
	this->socket=socket;
}

bool socketbuff::fill_inbuff(char *in,int size)
{
	if(in==NULL || size<=0 || size>BUFF_MAXSIZE || in_len+size>2*BUFF_MAXSIZE)
		return false;
	memcpy(in_buf+in_len,in,size);
	in_len+=size;
	return true;
}

void socketbuff::reflash_inbuff()
{
	if(in_used<=0)
		return;
	if(in_used>=in_len)
	{
		in_len=0;
		in_used=0;
		return;
	}
	memcpy(in_buf,in_buf+in_used,in_len-in_used);
	in_len-=in_used;
	in_used=0;
}

int socketbuff::readpkg(msg_pkg& ret)
{
	int leftlen=in_len-in_used;
	cout<<"readpkg1:"<<in_len<<":"<<in_used<<":"<<leftlen<<endl;
	if(leftlen<HEAD_SIZE)
	{
		if(in_used>0)
		{
			reflash_inbuff();
			return 0;

		}
		else
		{
			in_len=0;
			in_used=0;
			return -1;
		}
	}
	cout<<"readpkg2:"<<in_len<<":"<<in_used<<":"<<leftlen<<endl;
	HEAD_TYPE strlen=*((HEAD_TYPE *)(in_buf+in_used));
	cout<<"readpkg3:"<<strlen<<endl;
	strlen=HEAD_TRANS(strlen);
	cout<<"readpkg4:"<<strlen<<endl;
	if(strlen>BUFF_MAXSIZE-4 || strlen<=0)
	{
		in_len=0;
		in_used=0;
		return -1;
	}
	cout<<"readpkg5:"<<strlen<<endl;
	int leftstrlen=leftlen-HEAD_SIZE;
	cout<<"readpkg6:"<<leftstrlen<<endl;
	if(leftstrlen>=strlen)
	{
		ret.rebind(in_buf+in_used+HEAD_SIZE,strlen);
		in_used+=(strlen+HEAD_SIZE);
		cout<<"readpkg8:"<<":"<<in_used<<":"<<strlen<<endl;
	}
	else
	{
		reflash_inbuff();
		cout<<"readpkg9:"<<":"<<in_len<<":"<<in_used<<endl;
		return 0;
	}
	cout<<"readpkg10:"<<":"<<in_len<<":"<<in_used<<endl;
	return 1;
}

msg_pkg &socketbuff::send_begin()
{
	out_used=0;
	out_len=BUFF_MAXSIZE;
	out_msg.rebind(out_buf,out_len);
	return out_msg;
}
bool socketbuff::send_end()
{
	int send_size=out_msg.offset();
	if(send_size<=4)
		return false;
	int body_size=send_size-4;
	out_msg.seek(0);
	out_msg.write_uint32(body_size);
	out_msg.seek(send_size);
	int ret=write(this->socket,this->out_buf,send_size);
	if(ret<=0)
	{
		return false;
	}
	return true;
}

static void print_trace (int thesig)
{
	void *array[10];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace (array, 10);
	strings = backtrace_symbols (array, size);

	std::cout << "on signal:" << thesig << "\n";
	std::cout << size << " stack frames:\n";

	for (i = 0; i < size; i++)
		std::cout << strings[i] << "\n";

	free (strings);
}

static void op_signal_handler(int thesig)
{
	print_trace(thesig);
	exit(0);
}

server::server(void):
	_rankhandles(),_socketbuff()
{
	memset((void *)buf,0,BUFF_MAXSIZE);
	pretime=time(NULL);
}

server::~server(void)
{

}

void server::setnoblock(int sock)
{
	int opt;
	opt=fcntl(sock,F_GETFL);
	if(opt<0)
	{
		return;
	}
	opt=opt|O_NONBLOCK;
	if(fcntl(sock,F_SETFL,opt)<0)
	{
		return;
	}
	return;
}

bool server::init(void)
{
	cout<<"rankserver init......."<<endl;
	epfd=epoll_create(FD_SET_SIZE);
	if(epfd<0)
	{
		cout<<"create epoll failed"<<endl;
		return false;
	}
	serveraddr.sin_port=htons(SERV_PORT);
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_addr.s_addr=INADDR_ANY;
	//´´½¨handle
	int i=0;
	while(i<handle_count)
	{
		
		_rankhandles[rankhandles[i][0]]=rankhandle(rankhandles[i][0],rankhandles[i][1],rankhandles[i][2]);
		++i;
	}
	this->readobj();
	return true;
}

bool server::init(const char* port, ConfigMap& config_map, const std::string& config_data_path)
{
	cout<<"rankserver init............."<<endl;
	epfd = epoll_create(FD_SET_SIZE);
	if (epfd < 0)
	{
		cout<<"create epoll faild"<<endl;
		return false;
	}
	int port_ = atoi(port);
	serveraddr.sin_port = htons(port_);
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	int i = 0;
	while (i < handle_count)
	{
		ConfigMap::const_iterator iter = config_map.find(i);
		if (iter != config_map.end())
		{
			_rankhandles[rankhandles[i][0]] = rankhandle(iter->second.businessKey, 
				iter->second.perblock, iter->second.totalblock);
		}
		++i;
	}
	this->readobj();

	if (config_data_path.length() > 0)
	{
		OnePiece::AIDataManager::setConfigDataPath(config_data_path);
		std::cout << "config data path:" << config_data_path << std::endl;
	}

	return true;
	
}

/*
  added by dy
*/
//add new Business
int server::addBusiness (map<uint64_t, rankhandle>& _rankhandles, BlockConfig nbus) {
  map<uint64_t, rankhandle>::iterator busit = _rankhandles.find(nbus.businessKey);
  if (busit != _rankhandles.end()) {
    cout << "business " << nbus.businessKey << " already existed!" << endl;
    return 1;
  } else {
    _rankhandles[nbus.businessKey] = rankhandle(nbus.businessKey, nbus.perblock, nbus.totalblock);
  }
  return 0;
}

//delete some one Business

int server::removeBusiness (map<uint64_t, rankhandle>& _rankhandles, uint64_t rmv_bkey) {
  map<uint64_t, rankhandle>::iterator busit = _rankhandles.find(rmv_bkey);
  if (busit != _rankhandles.end()) {
    //remove files
    busit->second.managerRemoveFile(serveraddr.sin_port, rmv_bkey);
    //remove mem-data
    _rankhandles.erase(busit);
    /*
    string opdir = string(getenv("PWD"));
    opdir += "/" + server_port;
    rmdir(opdir + "/data1/" + rmv_bkey);
    rmdir(opdir + "/data2/" + rmv_bkey);
    //    "rmv -R opdir + data1 + rmv bkey";
    //    "rmv -R opdir + data2 + rmv_bkey";
    */
  } else {
    cout << "business " << rmv_bkey << " doesnt exist!" << endl; 
    return 1;
  }
  return 0;
}
  

void server::fin(void)
{
	cout<<"rankserver fin......."<<endl;
	map<int,socketbuff *>::iterator beg=_socketbuff.begin();
	while(beg!=_socketbuff.end())
	{
		delete beg->second;
		++beg;
	}
	close(epfd);
}

vector<uint64_t> server::analyse(string& instr)
{
	vector<uint64_t> result;
	if(instr.empty()|| instr.size()<3)
		return result;
	int first=instr.find_first_of("[");
	int last=instr.size()-1;
	if(first!=0 || last<first)
		return result;
	int douhao=0;
	int index=first+1;
	string the_value;
	stringstream ss_stream;
	uint64_t value;
	while(true)
	{
		douhao=instr.find_first_of(",",index);
		if(douhao<0)
		{
			the_value=instr.substr(index,last-index+1);
			ss_stream.clear();
			ss_stream<<the_value;
			if(!(ss_stream>>value))
			{
				result.clear();
				return result;
			}
			result.push_back(value);
			break;
		}
		else
		{
			the_value=instr.substr(index,douhao-index);
			index=douhao+1;
			ss_stream.clear();
			ss_stream<<the_value;
			if(!(ss_stream>>value))
			{
				result.clear();
				return result;
			}
			result.push_back(value);
		}
	}
	return result;
}

bool server::writeresult(vector<userdata>* ret,msg_pkg& pkg)
{
	try
	{
		if(ret==NULL)
		{
			cout<<"get error"<<endl;
			pkg.write_uint32(0);
			return true;
		}
		pkg.write_uint32(ret->size());
		vector<userdata>::iterator beg=ret->begin();
		vector<userdata>::iterator end=ret->end();
		while(beg!=end)
		{
			cout<<beg->userid<<":"<<beg->uservalue<<":"<<beg->pos<<":"<<beg->len<<endl;
			pkg.write_uint64(beg->userid);
			pkg.write_uint64(beg->uservalue);
			pkg.write_uint32(beg->pos);
			pkg.write_uint8(beg->len);
			pkg.write_string(beg->buf,beg->len);
			++beg;
		}
		delete ret;
		return true;
	}
	catch (readException& e)
	{
		return false;
	}
	
}

bool server::distribute(int sock,socketbuff *the_buff)
{
	int ret=0;
	do 
	{
		
		msg_pkg pkg;
		ret=the_buff->readpkg(pkg);
		cout<<"distribute:"<<ret<<endl;
		if(ret==-1)
			return false;
		else if(ret==0)
			return true;
		timeval tv_begin, tv_end;
		gettimeofday(&tv_begin, NULL);
		try
		{
#if BATTLE_LOG_TEST
            OPBattleLogInit();
#endif
			unsigned int msg_type=pkg.read_uint32();
			unsigned int transid=pkg.read_uint32();
			uint64_t businessKey=pkg.read_uint64();
			cout<<"distribute:"<<msg_type<<":"<<transid<<":"<<businessKey<<endl;
			map<uint64_t,rankhandle>::iterator it= _rankhandles.find(businessKey);
			switch(msg_type)
			{
			case FIGHT_MSG_FIGHT_CHECK_REQ:
			  {
				// int total = it->second.get_rank_total();

				//
				unsigned int str_size=pkg.read_uint32();
				char *data=pkg.read_string((unsigned int)str_size);
				cout<<" FIGHT_MSG_FIGHT_CHECK_REQ json"<<":"<<data<<endl;
				
				//
                OPBattleLog("start", transid);
				std::pair<int, std::map<int, int>* > callback = fight(data);
				unsigned int fightTotalTick = OnePiece::AIContext::getInstance()->currentTick;
                OPBattleLog("end", transid, callback.first);
			
				// int callback = 1;
				msg_pkg& outpkg = the_buff->send_begin();
				cout<<"win?"<<callback.first<<endl;
				outpkg.write_uint32(0);
				outpkg.write_uint32(FIGHT_MSG_FIGHT_CHECK_RESP);
				outpkg.write_uint32(transid);
				outpkg.write_uint32(callback.first);
				
				if (callback.second->size() > 0){
					outpkg.write_uint32(callback.second->size());
					for (std::map<int, int>::iterator iter = callback.second->begin();iter != callback.second->end();iter++){
						outpkg.write_uint32(iter->first);
						outpkg.write_uint32(iter->second);
						cout<<"item:"<<iter->first<<" ,amount:"<<iter->second<<endl;
					}
				} else {
					outpkg.write_uint32(0);
				}

                auto friends = OnePiece::AIContext::getInstance()->getClusterOrigin(OnePiece::TeamSide::kOur, 1);
                if (friends->size() > 0) {
                    int count = 0;
                    for (std::list<OnePiece::CharacterData*>::iterator iter = friends->begin(); iter != friends->end(); ++iter) {
                        OnePiece::CharacterData *c = *iter;
                        if (!c->isCannon() && !c->isSummon())
                            count++;
                    }
					outpkg.write_uint32(count);
					for (std::list<OnePiece::CharacterData*>::iterator iter = friends->begin(); iter != friends->end(); ++iter) {
                        OnePiece::CharacterData *c = *iter;
						if (!c->isCannon() && !c->isSummon()) {
							outpkg.write_uint32(c->getID());
							outpkg.write_uint32(c->getHP());
							outpkg.write_uint32(c->getEnergy());
                            std::cout<<"myId"<<c->myID<<"id"<<c->getID()<<"hp"<<c->getHP()<<"mp"<<c->getEnergy()<<std::endl;
						}
					}
				} else {
					outpkg.write_uint32(0);
				}

                auto enemies = OnePiece::AIContext::getInstance()->getClusterOrigin(OnePiece::TeamSide::kEnemy, 1);
                if (enemies->size() > 0) {
                    int count = 0;
                    for (std::list<OnePiece::CharacterData*>::iterator iter = enemies->begin(); iter != enemies->end(); iter++) {
                        OnePiece::CharacterData *c = *iter;
                        if (!c->isCannon() && !c->isSummon())
                            count++;
                    }
                    outpkg.write_uint32(count);
                    for (std::list<OnePiece::CharacterData*>::iterator iter = enemies->begin(); iter != enemies->end(); ++iter) {
                        OnePiece::CharacterData *c = *iter;
						if (!c->isCannon() && !c->isSummon()) {
							outpkg.write_uint32(c->getID());
							outpkg.write_uint32(c->getHP());
							outpkg.write_uint32(c->getEnergy());
                            std::cout<<"monsterId"<<c->myID<<"id"<<c->getID()<<"hp"<<c->getHP()<<"mp"<<c->getEnergy()<<std::endl;
						}
					}
				} else {
					outpkg.write_uint32(0);
				}
				std::cout << "\nfightTotalTick:" << fightTotalTick << std::endl;
				outpkg.write_uint32(fightTotalTick);


				if(!the_buff->send_end())
				{
					return false;
				}
			  	break;
			  }
			case FIGHT_MSG_FIGHT_DOUJI_REQ:
			  {
				// int total = it->second.get_rank_total();

				//
				unsigned int str_size=pkg.read_uint32();
				char *data=pkg.read_string((unsigned int)str_size);
				cout<<"json"<<":"<<data<<endl;
				
				//
                OPBattleLog("start", transid);
				int win[3];
				for (int i = 0; i < 3; i++) {
					std::pair<int, std::map<int, int>* > callback = fightDouji(data, i, nullptr);
					
					win[i] = callback.first;
					if (callback.first == 0) {
						int my_count = 0;
                        auto friends = OnePiece::AIContext::getInstance()->getClusterOrigin(OnePiece::TeamSide::kOur, 1);
						for (std::list<OnePiece::CharacterData*>::iterator iter = friends->begin(); iter != friends->end(); ++iter) {
                            OnePiece::CharacterData *c = *iter;
                            if (!c->isCannon() && !OnePiece::HeroActionController::isDeadAction(c->currentAction)) {
                                if (c->isSummon())
                                    continue;
								my_count++;
	                            std::cout<<"myId"<<c->myID<<"id"<<c->getID()<<"hp"<<c->getHP()<<"mp"<<c->getEnergy()<<std::endl;
							}
						}
						int rival_count = 0;
                        auto enemies = OnePiece::AIContext::getInstance()->getClusterOrigin(OnePiece::TeamSide::kEnemy, 1);
                        for (std::list<OnePiece::CharacterData*>::iterator iter = enemies->begin(); iter != enemies->end(); ++iter) {
                            OnePiece::CharacterData *c = *iter;
                            if (!c->isCannon() && !OnePiece::HeroActionController::isDeadAction(c->currentAction)) {
                                if (c->isSummon())
                                    continue;
								rival_count++;
	                            std::cout<<"myId"<<c->myID<<"id"<<c->getID()<<"hp"<<c->getHP()<<"mp"<<c->getEnergy()<<std::endl;
							}
						}
						if (my_count > rival_count) {
							win[i] = 1;
						} else if (my_count < rival_count) {
							win[i] = 2;
						} else {
							win[i] = 0;
						}
						
					}
				}
				
				// int callback = 1;
				msg_pkg& outpkg = the_buff->send_begin();
				outpkg.write_uint32(0);
				outpkg.write_uint32(FIGHT_MSG_FIGHT_DOUJI_RESP);
				outpkg.write_uint32(transid);
				outpkg.write_uint32(win[0]);
				outpkg.write_uint32(win[1]);
				outpkg.write_uint32(win[2]);
                OPBattleLog("end", transid, win[0], win[1], win[2]);
				
// 				outpkg.write_uint32(0);
				

// 				if (OnePiece::AIContext::getInstance()->friends->size() > 1) {
// 					outpkg.write_uint32((OnePiece::AIContext::getInstance()->friends->size() - 1));
// 					for (std::list<OnePiece::CharacterData*>::iterator iter = OnePiece::AIContext::getInstance()->friends->begin(); iter != OnePiece::AIContext::getInstance()->friends->end(); iter++) {
// 						if ((*iter)->myID > 0) {
// 							outpkg.write_uint32((*iter)->getID());
// 							outpkg.write_uint32((*iter)->getHP());
// 							outpkg.write_uint32((*iter)->getEnergy());
// std::cout<<"myId"<<(*iter)->myID<<"id"<<(*iter)->getID()<<"hp"<<(*iter)->getHP()<<"mp"<<(*iter)->getEnergy()<<std::endl;
// 						}
// 					}
// 				} else {
// 					outpkg.write_uint32(0);
// 				}

				if(!the_buff->send_end())
				{
					return false;
				}
			  	break;
			  }
			  case FIGHT_MSG_FIGHT_DIANFENG_REQ:
			  {
				// int total = it->second.get_rank_total();

				//
				unsigned int str_size=pkg.read_uint32();
				char *data=pkg.read_string((unsigned int)str_size);
				cout<<"json"<<":"<<data<<endl;
				
				//
                OPBattleLog("start", transid);
				int win[3];
				std::list<std::pair<int, std::pair<int, int> > > * hplist  = new std::list<std::pair<int, std::pair<int, int> > >;
				std::map<int, int>* HPMAP = new std::map<int, int>();

				for (int i = 0; i < 3; i++) {
					std::pair<int, std::map<int, int>* > callback = fightDouji(data, i, HPMAP);
					
					win[i] = (callback.first == 1)?1:0;
                    auto enemies = OnePiece::AIContext::getInstance()->getClusterOrigin(OnePiece::TeamSide::kEnemy, 1);
                    for (std::list<OnePiece::CharacterData*>::iterator iter = enemies->begin(); iter != enemies->end(); ++iter) {
                        OnePiece::CharacterData *c = *iter;
						if (!c->isCannon()) {
                            if (c->isSummon())
                                continue;
							std::cout<<"myId"<<c->myID<<"id"<<c->getID()<<"hp"<<c->getHP()<<"mp"<<c->getEnergy()<<std::endl;
							std::map<int, int>::iterator hpit = HPMAP->find(c->getID());
							if (hpit == HPMAP->end()) {
								HPMAP->insert(std::pair<int, int>(c->getID(), c->getHP()));
							}
							else {
								hpit->second = c->getHP();
							}
							hplist->push_back(std::pair<int, std::pair<int, int> >(i, std::pair<int, int>(c->getID(), c->getHP())));
							
						}
					}
					
				}
				delete HPMAP;
				// int callback = 1;
				msg_pkg& outpkg = the_buff->send_begin();
				outpkg.write_uint32(0);
				outpkg.write_uint32(FIGHT_MSG_FIGHT_DIANFENG_RESP);
				outpkg.write_uint32(transid);
				outpkg.write_uint32(win[0]);
				outpkg.write_uint32(win[1]);
				outpkg.write_uint32(win[2]);
                OPBattleLog("end", transid, win[0], win[1], win[2]);

				outpkg.write_uint32(hplist->size());
				for (std::list<std::pair<int, std::pair<int, int> > >::iterator iter = hplist->begin(); iter != hplist->end(); ++iter) {
					std::cout<<"cluster"<<(*iter).first<<"id"<<(*iter).second.first<<"hp"<<(*iter).second.second<<std::endl;
					outpkg.write_uint32((*iter).first);
					outpkg.write_uint32((*iter).second.first);
					outpkg.write_uint32((*iter).second.second);

				}
				delete hplist;
				
// 				outpkg.write_uint32(0);
				

// 				if (OnePiece::AIContext::getInstance()->friends->size() > 1) {
// 					outpkg.write_uint32((OnePiece::AIContext::getInstance()->friends->size() - 1));
// 					for (std::list<OnePiece::CharacterData*>::iterator iter = OnePiece::AIContext::getInstance()->friends->begin(); iter != OnePiece::AIContext::getInstance()->friends->end(); iter++) {
// 						if ((*iter)->myID > 0) {
// 							outpkg.write_uint32((*iter)->getID());
// 							outpkg.write_uint32((*iter)->getHP());
// 							outpkg.write_uint32((*iter)->getEnergy());
// std::cout<<"myId"<<(*iter)->myID<<"id"<<(*iter)->getID()<<"hp"<<(*iter)->getHP()<<"mp"<<(*iter)->getEnergy()<<std::endl;
// 						}
// 					}
// 				} else {
// 					outpkg.write_uint32(0);
// 				}

				if(!the_buff->send_end())
				{
					return false;
				}
			  	break;
			  }
			  case FIGHT_MSG_FIGHT_CARWHEEL_REQ:
			  {
			  	unsigned int str_size=pkg.read_uint32();
				char *data=pkg.read_string((unsigned int)str_size);
				cout<<"FIGHT_MSG_FIGHT_CARWHEEL_REQ json"<<":"<<data<<endl;

                OPBattleLog("start", transid);
				std::pair<int, std::vector<std::vector<std::vector<std::vector<int> > > > > callback = fightCarWheel(data);
                OPBattleLog("end", transid, callback.first);
			
				msg_pkg& outpkg = the_buff->send_begin();
				cout<<"FIGHT_MSG_FIGHT_CARWHEEL_REQ: win?"<<callback.first<<endl;
				outpkg.write_uint32(0);
				outpkg.write_uint32(FIGHT_MSG_FIGHT_CARWHEEL_RESP);
				outpkg.write_uint32(transid);
				outpkg.write_uint32(callback.first);

				std::vector<std::vector<std::vector<std::vector<int> > > > vec0 = callback.second;
				std::vector<int>::size_type vec0_sz = vec0.size();
				outpkg.write_uint32(vec0_sz);
				for(int i = 0; i < vec0_sz; ++i){
					std::vector<std::vector<std::vector<int> > > vec1 = vec0[i];
					std::vector<int>::size_type vec1_sz = vec1.size();
					outpkg.write_uint32(vec1_sz);
					for(int j = 0; j< vec1_sz; ++j){
						std::vector<std::vector<int> > vec2 = vec1[j];
						std::vector<int>::size_type vec2_sz = vec2.size();
						outpkg.write_uint32(vec2_sz);
						for(int k = 0; k< vec2_sz; ++k){
							std::vector<int> vec3 = vec2[k];
							std::vector<int>::size_type vec3_sz = vec3.size();
							outpkg.write_uint32(vec3_sz);
							for(int m = 0; m< vec3_sz; ++m){
								outpkg.write_uint32(vec3[m]);
								cout<<i<<":"<<j<<":"<<k<<":"<<vec3[m]<<endl;
							}
							
						}
					}
				}

				if(!the_buff->send_end())
				{
					return false;
				}
			  	break;
			  }
              case FIGHT_MSG_FIGHT_HUIYI_REQ:
              {
                unsigned int str_size=pkg.read_uint32();
				char *data=pkg.read_string((unsigned int)str_size);
				cout<<"FIGHT_MSG_FIGHT_HUIYI_REQ json"<<":"<<data<<endl;
                
                OPBattleLog("start", transid);
                auto callback = fightWWTD(data); //typedef std::pair<int, WWFightRecord> WWFightResult;
                OPBattleLog("end", transid, callback.first);
			
				msg_pkg& outpkg = the_buff->send_begin();
				cout<<"FIGHT_MSG_FIGHT_HUIYI_REQ: win?"<<callback.first<<endl;
				outpkg.write_uint32(0);
                outpkg.write_uint32(FIGHT_MSG_FIGHT_HUIYI_RESP);
				outpkg.write_uint32(transid);
				outpkg.write_uint32(callback.first);
                
                auto& record = callback.second; //typedef std::vector<WWTeamInfo> WWFightRecord;
                auto recordSize = record.size();
                outpkg.write_uint32(recordSize);
                for (int iRecord = 0; iRecord < recordSize; iRecord++) {

                    auto& teamInfo = record[iRecord]; //typedef std::vector<WWHeroInfo> WWTeamInfo;
                    auto teamSize = teamInfo.size();
                    outpkg.write_uint32(teamSize);
                    for (int iTeam = 0; iTeam < teamSize; iTeam++) {

                        auto& heroInfo = teamInfo[iTeam]; //typedef std::vector<int> WWHeroInfo;
                        auto heroSize = heroInfo.size();
                        outpkg.write_uint32(heroSize);
                        for (int iHero = 0; iHero < heroSize; iHero++) {
                            outpkg.write_uint32(heroInfo[iHero]);
                            cout << iRecord << ":" << iTeam << ":" << iHero << ":" << heroInfo[iHero] << endl;
                        } // END hero

                    } // END team

                } // END record

				if(!the_buff->send_end())
				{
					return false;
				}
			  	break;
              }
              case FIGHT_MSG_FIGHT_ALLIANCE_PVE_REQ:
              {
                unsigned int str_size=pkg.read_uint32();
                char *data=pkg.read_string((unsigned int)str_size);
                cout<<"FIGHT_MSG_FIGHT_ALLIANCE_PVE_REQ json"<<":"<<data<<endl;

                OPBattleLog("start", transid);
                // std::pair<int, std::vector<std::vector<std::vector<std::vector<int> > > > > callback = fightAlliancePve(data);
                // std::pair<int, std::pair<std::map<int, int>*, std::vector<std::vector<std::vector<std::vector<int> > > > >> callback = fightAlliancePveNew(data);
                std::pair<int, std::pair<std::map<int, int>*, std::vector<std::vector<std::vector<std::vector<int> > > >* > > callback = fightAlliancePveNew(data);
                OPBattleLog("end", transid, callback.first);
            
                msg_pkg& outpkg = the_buff->send_begin();
                cout<<"FIGHT_MSG_FIGHT_ALLIANCE_PVE_REQ: win?"<<callback.first<<endl;
                outpkg.write_uint32(0);
                outpkg.write_uint32(FIGHT_MSG_FIGHT_ALLIANCE_PVE_RESP);
                outpkg.write_uint32(transid);
                outpkg.write_uint32(callback.first);

                // std::pair<std::map<int, int>*, std::vector<std::vector<std::vector<std::vector<int> > > > > reward_hp = callback.second;
                std::pair<std::map<int, int>*, std::vector<std::vector<std::vector<std::vector<int> > > >* > reward_hp = callback.second;

                if (reward_hp.first->size() > 0){
                    outpkg.write_uint32(reward_hp.first->size());
                    for (std::map<int, int>::iterator iter = reward_hp.first->begin();iter != reward_hp.first->end();iter++){
                        outpkg.write_uint32(iter->first);
                        outpkg.write_uint32(iter->second);
                        cout<<"item:"<<iter->first<<" ,amount:"<<iter->second<<endl;
                    }
                } else {
                    outpkg.write_uint32(0);
                }

                std::vector<std::vector<std::vector<std::vector<int> > > > vec0 = *reward_hp.second;
                std::vector<int>::size_type vec0_sz = vec0.size();
                outpkg.write_uint32(vec0_sz);
                for(int i = 0; i < vec0_sz; ++i){
                    std::vector<std::vector<std::vector<int> > > vec1 = vec0[i];
                    std::vector<int>::size_type vec1_sz = vec1.size();
                    outpkg.write_uint32(vec1_sz);
                    for(int j = 0; j< vec1_sz; ++j){
                        std::vector<std::vector<int> > vec2 = vec1[j];
                        std::vector<int>::size_type vec2_sz = vec2.size();
                        outpkg.write_uint32(vec2_sz);
                        for(int k = 0; k< vec2_sz; ++k){
                            std::vector<int> vec3 = vec2[k];
                            std::vector<int>::size_type vec3_sz = vec3.size();
                            outpkg.write_uint32(vec3_sz);
                            for(int m = 0; m< vec3_sz; ++m){
                                outpkg.write_uint32(vec3[m]);
                                cout<<i<<":"<<j<<":"<<k<<":"<<vec3[m]<<endl;
                            }
                            
                        }
                    }
                }

                if(!the_buff->send_end())
                {
                    return false;
                }
                break;
              }
              case FIGHT_MSG_FIGHT_SHIP_KING_PVE_REQ:
              {
                unsigned int str_size=pkg.read_uint32();
                char *data=pkg.read_string((unsigned int)str_size);
                cout<<"FIGHT_MSG_FIGHT_SHIP_KING_PVE_REQ json"<<":"<<data<<endl;

                OPBattleLog("start", transid);

                int score = fightShipKingPveVerify(data);
                OPBattleLog("end", transid, score);
            
                msg_pkg& outpkg = the_buff->send_begin();
                cout<<"FIGHT_MSG_FIGHT_SHIP_KING_PVE_REQ: score-> "<<score<<endl;
                outpkg.write_uint32(0);
                outpkg.write_uint32(FIGHT_MSG_FIGHT_SHIP_KING_PVE_RESP);
                outpkg.write_uint32(transid);
                outpkg.write_uint32(score);

                if(!the_buff->send_end())
                {
                    return false;
                }
                break;
              }
			default:
				cout<<"err parm"<<endl;
				return false;
			}
			gettimeofday(&tv_end, NULL);
			unsigned int pertime=(int)(1000000 * (tv_end.tv_sec - tv_begin.tv_sec) + tv_end.tv_usec - tv_begin.tv_usec);
			if(pertime>maxtime)
			{
				maxtime=pertime;
			}
			totaltime+=pertime;
			++totalcount;
			cout<<"perhandle time:"<<totaltime<<":"<<totalcount<<":"<<totaltime/totalcount<<":"<<pertime<<endl;

#if BATTLE_LOG_TEST
            OPBattleLogReset();
#endif
		}
		catch (const OnePiece::BattleAttributeSchema::Error& e)
		{
			cout << "attribute schema rejected: " << e.what() << endl;
			shutdown(sock, SHUT_RDWR);
			return false;
		}
		catch (readException& e)
		{
			cout<<"read err"<<endl;
			return false;
		}
		catch(writeException& e)
		{
			cout<<"write err"<<endl;
			return true;
		}
	} while (ret==1);
	return true;
}


/*void server::init_daemon()
{
	 int pid;
  int i;

  if(pid=fork())
    exit(0);//æ˜¯çˆ¶è¿›ç¨‹ï¼Œç»“æŸçˆ¶è¿›ç¨‹
  else if(pid< 0)
    exit(1);//forkå¤±è´¥ï¼Œé€€å‡?

  setsid();//ç¬¬ä¸€å­è¿›ç¨‹æˆä¸ºæ–°çš„ä¼šè¯ç»„é•¿å’Œè¿›ç¨‹ç»„é•¿
  if(pid=fork())
    exit(0);//æ˜¯ç¬¬ä¸€å­è¿›ç¨‹ï¼Œç»“æŸç¬¬ä¸€å­è¿›ç¨?
  else if(pid< 0)
    exit(1);//forkå¤±è´¥ï¼Œé€€å‡?

  chdir("/tmp");//æ”¹å˜å·¥ä½œç›®å½•åˆ?tmp
  umask(0);//é‡è®¾æ–‡ä»¶åˆ›å»ºæŽ©æ¨¡
  return;
}*/

bool server::run(void)
{
	#if OSFLAG==WINDOWS
	
	#else
		signal(SIGPIPE,SIG_IGN);
		signal(SIGABRT, op_signal_handler);
		signal(SIGSEGV, op_signal_handler);
		signal(SIGBUS, op_signal_handler);
		signal(SIGFPE, op_signal_handler);
		signal(SIGHUP, op_signal_handler);
		signal(SIGILL, op_signal_handler);
		signal(SIGINT, op_signal_handler);
		signal(SIGQUIT, op_signal_handler);
		signal(SIGTERM, op_signal_handler);
		signal(SIGTRAP, op_signal_handler);
		cout<<"rankserver run......."<<endl;
		
		listenfd= socket(AF_INET,SOCK_STREAM,0);
		//µØÖ·ÖØÓÃ
		int value = 1;
		if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(void *)&value,sizeof(value))==-1)
		{
			perror("REUSEADDR");
			return false;
		}
		setnoblock(listenfd);
		//ÉèÖÃ¼àÌý¶Ë¿ÚÊÂ¼þ
		ev.events=EPOLLIN|EPOLLET;
		ev.data.fd=listenfd;
		epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
	
		if (::bind(listenfd,(sockaddr *)&serveraddr,sizeof(serveraddr)) < 0)
		{
			perror("bind error");
			return false;
		}
		if (listen(listenfd,LISTEN) < 0)
		{
			perror("listen error");
			return false;
		}
	
		cout<<"listen completed"<<endl;
		
		while(true)
		{
			int nfds=0;
			do
			{
				
				nfds=epoll_wait(epfd,events,EVENT_MAXSIZE,10*60*1000);
			} while(nfds <0 && errno == EINTR);
	
			if(nfds<0)
			{
				
				cout<<"epoll_wait failed,errno:"<<errno<<endl;
				return false;
			}
			for(int i=0;i<nfds;++i)
			{
				
				if(events[i].data.fd==listenfd)//ÓÐÐÂÁ¬½Ó
				{
					int connfd=0;
					while(true)
					{
						
						connfd=accept(listenfd,(sockaddr *)&clientaddr,&client);
						if(connfd <0)
						{
							if(errno == EINTR)
							{
								continue;
							}
							else if(errno == EWOULDBLOCK || errno == EAGAIN)
							{
								cout<<"accept full:"<<errno<<endl;
								break;
							}
							else
							{
								cout<<"accept failed,errno:"<<errno<<endl;
								return false;
							}
						}
						setnoblock(connfd);
						int  param=1; 
						int keepIdle = 60; // Èç¸ÃÁ¬½ÓÔÚ60ÃëÄÚÃ»ÓÐÈÎºÎÊý¾ÝÍùÀ´,Ôò½øÐÐÌ½²â 
						int keepInterval = 5; // Ì½²âÊ±·¢°üµÄÊ±¼ä¼ä¸ôÎª5 Ãë
						int keepCount = 3; //Ì½²â³¢ÊÔµÄ´ÎÊý.Èç¹ûµÚ1´ÎÌ½²â°ü¾ÍÊÕµ½ÏìÓ¦ÁË,Ôòºó2´ÎµÄ²»ÔÙ·¢.
						setsockopt(connfd,SOL_SOCKET,SO_KEEPALIVE,(char*)&param,sizeof(param));
						setsockopt(connfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
						setsockopt(connfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
						setsockopt(connfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
						char *connaddr=inet_ntoa(clientaddr.sin_addr);
						cout<<"connect from:"<<connaddr<<"port:"<<ntohs(clientaddr.sin_port)<<endl;
						//fputs(connaddr,fp);
						ev.data.fd=connfd;
						ev.events=EPOLLIN|EPOLLET;
						epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
						socketbuff *temp=new socketbuff(connfd);
						if(temp==NULL)
							return false;
						_socketbuff[connfd]=temp; 
					}
				}
				else if(events[i].events & EPOLLIN)
				{
					int socketfd=0;
					if((socketfd=events[i].data.fd)<0)
					{
						cout<<"epoll in error"<<endl;
						continue;
					}
					//ÖØÖÃ
					int n=0;
					bool cancel=false;
					map<int,socketbuff *>::iterator ret=_socketbuff.find(socketfd);
					if(ret==_socketbuff.end())
					{
						return false;
					}
					do
					{
						memset(buf,0,BUFF_MAXSIZE);
						n=read(socketfd,buf,BUFF_MAXSIZE);
						cout<<"data1:"<<n<<":"<<errno<<endl;
						if(n<0)
						{
							if(errno==ECONNRESET)
							{
								close(socketfd);
								delete ret->second;
								_socketbuff.erase(ret);
								events[i].data.fd=-1;
								cout<<"socket closed 1:"<<errno<<endl;
							}
							else if(!(errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR))
							{
								close(socketfd);
								delete ret->second;
								_socketbuff.erase(ret);
								events[i].data.fd=-1;
								cout<<"socket closed 2:"<<errno<<endl;
							}
						}
						else if(n==0)
						{
							close(socketfd);
							delete ret->second;
							_socketbuff.erase(ret);
							events[i].data.fd=-1;
							cout<<"socket closed 3"<<endl;
						}
						else
						{   
							if(!cancel)
							{
								cout<<"data2:"<<n<<endl;
								if(ret->second->fill_inbuff(buf,n))
								{
									cout<<"data3:"<<endl;
									bool result=distribute(socketfd,ret->second);
									if(!result)
									{
										cancel=true;
									}
								}
								else
									cancel=true;
							}
						}
					}while((n <0 && errno == EINTR)|| n==BUFF_MAXSIZE);
	
				}
			}
			time_t now=time(NULL);
			time_t span=now-pretime;
			cout<<"log log\t"<<span<<endl;
			if(span>=10*60)
			{
				timeval tv_begin, tv_end;
				gettimeofday(&tv_begin, NULL);
				this->writeobj();
				gettimeofday(&tv_end, NULL);
				unsigned int pertime=(int)(1000000 * (tv_end.tv_sec - tv_begin.tv_sec) + tv_end.tv_usec - tv_begin.tv_usec);
				cout<<"perwrite time:"<<pertime<<endl;
				pretime=now;
			}
		}
	#endif
		return true;
}

#define RANKVERTION 10000
void server::readobj()
{
	stringstream sstr;
	std::string path;
	sstr << ntohs(serveraddr.sin_port);
	sstr >> path;

	rankserialization filestream;
	char filename[256];
	
	try
	{
		/*string dirname="./data1";
		if(Dir_index==2)
		{
			dirname="./data2";
		}*/
		std::string dirname = "/data1";
		if (2 == Dir_index){
			dirname = "/data2";		
		}
		dirname = "./"+path+dirname;

		if(!filestream.read_open(dirname+string("/rankdataindex.in")))
		{
			return;
		}
		cout<<"readobjbegin"<<endl;
		int vertion=filestream.read_uint32();
		if(vertion!=RANKVERTION)
			return;
		cout<<"readobjbegin1:"<<vertion<<endl;
		int buss_len=filestream.read_uint32();
		int i=0;
		cout<<"readobjbegin2:"<<buss_len<<endl;
		while(i<buss_len)
		{
			uint64_t busi=filestream.read_uint64();
			int namelen=filestream.read_uint8();
			cout<<"readobjbegin6:"<<namelen<<endl;
			int str_len=filestream.read_string(filename,namelen);
			filename[namelen]=0;
			cout<<"readobjbegin3:"<<str_len<<":"<<busi<<":"<<filename[0]<<endl;
			map<uint64_t,rankhandle>::iterator bit=_rankhandles.find(busi);
			if(bit!=_rankhandles.end())
			{
				cout<<"readobjbegin4:"<<busi<<endl;
				bit->second.read(dirname,string(filename));
				cout<<"readobjbegin5:"<<bit->second.get_data().size()<<endl;
			}
			++i;
		}

	}
	catch (readException& e)
	{
	}
}

void server::writeobj()
{
	/*string dirname="./data1";
	if(serializIndex%2==1)
	{
		dirname="./data2";
	}
	if(NULL==opendir(dirname.c_str()))
	{
		mkdir(dirname.c_str(),0775);
	}*/
	string dirname = getDataDir(ntohs(serveraddr.sin_port), serializIndex);	
	//cout<<"dirname is: "<<dirname<<endl;
	++serializIndex;
	rankserialization filestream;
	if(!filestream.write_open(dirname+string("/rankdataindex.in")))
	{
		return;
	}
	cout<<"log begin 0"<<endl;
	int i=0;
	int &t=i;
	cout<<"log begin 1"<<endl;
	filestream.write_uint32(RANKVERTION);
	cout<<"log begin 2"<<endl;
	filestream.write_uint32(_rankhandles.size());
	cout<<"log begin 3"<<endl;
	map<uint64_t,rankhandle>::iterator beg=_rankhandles.begin();
	map<uint64_t,rankhandle>::iterator end=_rankhandles.end();
	cout<<"log begin"<<endl;

	while(beg!=end)
	{
		filestream.write_uint64(beg->first);
		stringstream ss;
		ss<<beg->first<<"index"<<".in";
		string name=ss.str();
		//cout<<"log begin 4"<<name<<endl;
		filestream.write_uint8(name.size());
		//cout<<"log begin 5"<<name.size()<<endl;
		filestream.write_string((char *)(name.c_str()),name.size());
		beg->second.write(dirname,name);
		++beg;
	}
	filestream.close();
}

std::string server::getDataDir(const short port, int index)
{
	std::string dir;
	std::string path;
	stringstream sstr;
	std::string dir_name = "data1";

	sstr << port;
	sstr >> path;
	
	if (index%2 == 1)
	{
		dir_name = "data2";
	}
	if (-1 == chdir(path.c_str())){
		if (0 == mkdir(path.c_str(), 0755)){
			dir = "./"+path;    
		}   
		if (0 == chdir(path.c_str())){
			if (0 == mkdir(dir_name.c_str(),0755)){
				chdir("../");
				return dir+"/"+dir_name;
			}   
		}   
	}
	else{
		dir = "./"+path;
		if (-1 == chdir(dir_name.c_str())){
			if (0 == mkdir(dir_name.c_str(), 0755))
			{   
				chdir("../");
				return dir+"/"+dir_name;    
			}   
		}   
		else{
			chdir("../../");
			return dir+"/"+dir_name;
		}   
	}
	return dir;
}
