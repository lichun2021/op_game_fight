// Offline transport boundary: original decode/fight/battle writer, file I/O only.
#include "verify/fight.h"
#include "AI/AIDataManager.h"
#include "AI/AIContext.h"
#include "AI/AICharacterData.h"
#include "local_transport.hpp"
#include "jsonx/json.h"
#include <fstream>
#include <iterator>
int Dir_index=1;
// Exact details() body from the source-pinned op38_local_main.cpp.
void details(std::vector<unsigned char>& b,OnePiece::TeamSide side){auto list=OnePiece::AIContext::getInstance()->getClusterOrigin(side,1);unsigned n=0;for(auto c:*list)if(!c->isCannon()&&!c->isSummon())++n;op38::put(b,n);for(auto c:*list)if(!c->isCannon()&&!c->isSummon()){op38::put(b,c->getID());op38::put(b,c->getHP());op38::put(b,c->getEnergy());}}
int main(int argc,char**argv){try{
 if(argc!=4)throw std::runtime_error("config encoded-request output-frame required");
 std::ifstream file(argv[2],std::ios::binary);std::vector<unsigned char> frame((std::istreambuf_iterator<char>(file)),{});
 auto request=op38::decode(frame,65535);
 OnePiece::AIDataManager::setConfigDataPath(std::string(argv[1])+"/");
 // Exact battle() writer sequence. No listener, socket transfer, database, or grant.
 auto result=fight(request.json);std::vector<unsigned char> b;op38::put(b,0);op38::put(b,10017);op38::put(b,request.transaction);op38::put(b,result.first);
 if(!result.second)throw std::runtime_error("missing engine rewards");op38::put(b,result.second->size());for(auto item:*result.second){op38::put(b,item.first);op38::put(b,item.second);}
 details(b,OnePiece::TeamSide::kOur);details(b,OnePiece::TeamSide::kEnemy);op38::put(b,OnePiece::AIContext::getInstance()->currentTick);
 // Exact prefix update performed by transmit(), followed by local file output.
 uint32_t n=uint32_t(b.size()-4);b[0]=n>>24;b[1]=n>>16;b[2]=n>>8;b[3]=n;
 std::ofstream output(argv[3],std::ios::binary);if(!output)throw std::runtime_error("output unavailable");output.write(reinterpret_cast<const char*>(b.data()),b.size());
 Json::Value input,state;Json::Reader reader;if(!reader.parse(request.json,input))throw std::runtime_error("invalid original input");
 auto* ctx=OnePiece::AIContext::getInstance();state["win"]=result.first;state["tick"]=int(ctx->currentTick);state["characters"]=Json::Value(Json::arrayValue);state["rewards"]=Json::Value(Json::arrayValue);
 for(auto pair:*result.second){Json::Value row;row["i"]=pair.first;row["a"]=pair.second;state["rewards"].append(row);}
 for(unsigned team=0;team<input["teams"].size();++team){auto side=team==0?OnePiece::TeamSide::kOur:OnePiece::TeamSide::kEnemy;auto* cluster=ctx->getClusterOrigin(side,team==0?1:team);if(!cluster)continue;for(auto* c:*cluster){if(c->isCannon()||c->isSummon())continue;Json::Value row;row["team"]=team;row["id"]=c->getID();row["hp"]=c->getHP();row["mp"]=c->getEnergy();row["origin"]=Json::Value(Json::arrayValue);for(int i=0;i<ATT_NUM;++i)row["origin"].append(OnePiece::CharacterFunc::GetCharacterAttr(c->attributesOrigin.get(),i));state["characters"].append(row);}}
 std::ofstream sidecar(std::string(argv[3])+".json");sidecar<<Json::FastWriter().write(state);
 return 0;
 }catch(const std::exception& e){fprintf(stderr,"offline frame failed: %s\n",e.what());return 2;}}
