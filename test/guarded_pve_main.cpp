// Isolated adapter. Algorithm source provenance is recorded in this candidate source_manifest.json.
#include "local_transport.hpp"
#include "ServerVerifyHelper.h"
#include "AI/AIContext.h"
#include "AI/AICharacterData.h"
#include "AI/AIDataManager.h"
#include "json/document.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <limits.h>
#include <set>
#include <algorithm>
#ifdef __linux__
#include <sys/prctl.h>
#endif
int Dir_index=1; // Link-only old rank serializer symbol; rankserver's main is excluded.
namespace {
void setParentDeathSignal(int signalNumber) {
#ifdef __linux__
 prctl(PR_SET_PDEATHSIG,signalNumber);
#else
 throw std::runtime_error("guarded PVE serving requires Linux");
#endif
}
struct Settings {bool enabled;unsigned port,request_max,response_max,timeout;std::string host,config;std::vector<std::string> files;std::vector<unsigned> stages;};
std::string read(const std::string& name){std::ifstream f(name);if(!f)throw std::runtime_error("unreadable input");std::ostringstream s;s<<f.rdbuf();return s.str();}
std::string canonical(const std::string& name){char p[PATH_MAX];if(!realpath(name.c_str(),p))throw std::runtime_error("missing path");return p;}
Settings settings(const char* file){rapidjson::Document d;d.Parse<0>(read(file).c_str());if(d.HasParseError()||!d.IsObject())throw std::runtime_error("settings JSON");
 auto integer=[&](const char* key,unsigned low,unsigned high){if(!d.HasMember(key)||!d[key].IsUint()||d[key].GetUint()<low||d[key].GetUint()>high)throw std::runtime_error("settings integer");return d[key].GetUint();};
 auto str=[&](const char* key){if(!d.HasMember(key)||!d[key].IsString())throw std::runtime_error("settings string");return std::string(d[key].GetString());};
 if(!d.HasMember("enabled")||!d["enabled"].IsBool()||str("protocol")!="card-pve-length32be-json-v1"||str("configuration_profile")!="card-baseline")throw std::runtime_error("settings profile");
 if(!d.HasMember("allowed_commands")||!d["allowed_commands"].IsArray()||d["allowed_commands"].Size()!=1||!d["allowed_commands"][0u].IsInt()||d["allowed_commands"][0u].GetInt()!=10016)throw std::runtime_error("command whitelist");
 Settings s{d["enabled"].GetBool(),integer("port",1025,65535),integer("max_request_bytes",24,65535),integer("max_response_bytes",32,655350),integer("request_timeout_ms",1,16000),str("host"),str("configuration_path"),{}, {}};
 if(s.host!="127.0.0.1")throw std::runtime_error("loopback required");std::string root=canonical(str("guest_root"));std::string config=canonical(s.config);
 if(config!=root+"/configuration")throw std::runtime_error("configuration escapes isolated root");s.config=config+"/";
 if(!d.HasMember("configuration_files")||!d["configuration_files"].IsArray()||d["configuration_files"].Size()!=34)throw std::runtime_error("configuration list");
 std::set<std::string> names;
 for(unsigned ix=0;ix<d["configuration_files"].Size();++ix){const auto& item=d["configuration_files"][ix];if(!item.IsString())throw std::runtime_error("configuration filename");std::string name=item.GetString();
 if(name.size()<6||name.front()!='_'||name.substr(name.size()-4)!=".txt"||name.find_first_not_of("_abcdefghijklmnopqrstuvwxyz0123456789.")!=std::string::npos)throw std::runtime_error("unsafe table");
 if(!names.insert(name).second)throw std::runtime_error("duplicate configuration");
 if(canonical(s.config+name)!=config+"/"+name)throw std::runtime_error("table symlink");rapidjson::Document table;table.Parse<0>(read(s.config+name).c_str());if(table.HasParseError()||(!table.IsObject()&&!table.IsArray()))throw std::runtime_error("invalid table JSON: "+name);s.files.push_back(name);}
 if(!d.HasMember("allowed_stage_ids")||!d["allowed_stage_ids"].IsArray())throw std::runtime_error("stage scope required");
 std::set<unsigned> stages;
 for(unsigned ix=0;ix<d["allowed_stage_ids"].Size();++ix){const auto& item=d["allowed_stage_ids"][ix];if(!item.IsUint()||item.GetUint()==0||!stages.insert(item.GetUint()).second)throw std::runtime_error("invalid stage scope");s.stages.push_back(item.GetUint());}
 if(s.enabled&&s.stages.empty())throw std::runtime_error("enabled stage scope empty");
 return s;
}
void details(std::vector<unsigned char>& b,OnePiece::TeamSide side){auto list=OnePiece::AIContext::getInstance()->getClusterOrigin(side,1);unsigned n=0;for(auto c:*list)if(!c->isCannon()&&!c->isSummon())++n;op38::put(b,n);for(auto c:*list)if(!c->isCannon()&&!c->isSummon()){op38::put(b,c->getID());op38::put(b,c->getHP());op38::put(b,c->getEnergy());}}
void validatePost(const Settings& s,const op38::Request& request){rapidjson::Document post;post.Parse<0>(request.json.c_str());
 if(post.HasParseError()||!post.IsObject()||!post.HasMember("teams")||!post["teams"].IsArray()||post["teams"].Size()<2||!post.HasMember("operation")||!post["operation"].IsObject()||!post.HasMember("battleRect")||!post["battleRect"].IsObject())throw std::runtime_error("invalid ordinary PVE JSON");
 if(!post.HasMember("gkid")||!post["gkid"].IsUint()||std::find(s.stages.begin(),s.stages.end(),post["gkid"].GetUint())==s.stages.end())throw std::runtime_error("stage outside XLSX scope");
 if(!post.HasMember("k")||!post["k"].IsInt()||post["k"].GetInt()!=0||!post.HasMember("bkey")||!post["bkey"].IsInt()||post["bkey"].GetInt()!=11)throw std::runtime_error("not an ordinary PVE post");
}
void battle(int fd,const Settings& s,const std::vector<unsigned char>& frame){auto request=op38::decode(frame,s.request_max);validatePost(s,request);
 OnePiece::AIDataManager::setConfigDataPath(s.config);
 auto result=fight(request.json);std::vector<unsigned char> b;op38::put(b,0);op38::put(b,10017);op38::put(b,request.transaction);op38::put(b,result.first);
 if(!result.second)throw std::runtime_error("missing engine rewards");op38::put(b,result.second->size());for(auto item:*result.second){op38::put(b,item.first);op38::put(b,item.second);}
 details(b,OnePiece::TeamSide::kOur);details(b,OnePiece::TeamSide::kEnemy);op38::put(b,OnePiece::AIContext::getInstance()->currentTick);op38::transmit(fd,b,s.response_max,s.timeout);
}
void serve(const Settings& s){
#ifndef __linux__
 throw std::runtime_error("guarded PVE serving requires Linux");
#endif
if(!s.enabled)throw std::runtime_error("XLSX backend disabled");
 // Host supervisor must verify gate evidence and install dedicated-user egress restrictions first.
 const char* gate=getenv("OP38_FIGHT_GATES_VERIFIED");if(!gate||std::string(gate)!="1"||geteuid()==0)throw std::runtime_error("dedicated guarded launcher required");
 signal(SIGPIPE,SIG_IGN);signal(SIGCHLD,SIG_IGN);int sock=socket(AF_INET,SOCK_STREAM,0);if(sock<0)throw std::runtime_error("socket failed");int yes=1;setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);
 sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_port=htons(s.port);inet_pton(AF_INET,s.host.c_str(),&addr.sin_addr);
 if(bind(sock,reinterpret_cast<sockaddr*>(&addr),sizeof addr)||listen(sock,1))throw std::runtime_error("bind/listen failed");
 std::cout<<"op38 ordinary PVE loopback ready"<<std::endl;
 for(;;){sockaddr_in peer{};socklen_t n=sizeof peer;int fd=accept(sock,reinterpret_cast<sockaddr*>(&peer),&n);if(fd<0){if(errno==EINTR)continue;throw std::runtime_error("accept failed");}
 if(ntohl(peer.sin_addr.s_addr)!=INADDR_LOOPBACK){close(fd);continue;}
 // Each Node pool connection remains usable after a response. Isolate every engine request
 // in its own bounded-lifetime worker, so a stalled battle cannot hang the listener.
 pid_t listener_pid=getpid();pid_t connection=fork();if(connection<0){close(fd);continue;}
 if(connection>0){close(fd);continue;}
 close(sock);setParentDeathSignal(SIGTERM);if(getppid()!=listener_pid)_exit(1);signal(SIGCHLD,SIG_DFL);
 try{for(;;){auto frame=op38::receive(fd,s.request_max,s.timeout);auto request=op38::decode(frame,s.request_max);validatePost(s,request);
 std::cout<<"op38 engine worker requested"<<std::endl;pid_t manager_pid=getpid();pid_t worker=fork();if(worker<0)throw std::runtime_error("fork failed");
 if(worker==0){setParentDeathSignal(SIGKILL);if(getppid()!=manager_pid)_exit(1);int nullfd=open("/dev/null",O_WRONLY);if(nullfd>=0){dup2(nullfd,STDOUT_FILENO);dup2(nullfd,STDERR_FILENO);close(nullfd);}try{battle(fd,s,frame);_exit(0);}catch(...){_exit(1);}}
 auto deadline=std::chrono::steady_clock::now()+std::chrono::milliseconds(s.timeout);int status=0;
 for(;;){pid_t result=waitpid(worker,&status,WNOHANG);if(result==worker)break;if(result<0&&errno!=EINTR)throw std::runtime_error("wait failed");
 if(std::chrono::steady_clock::now()>=deadline){kill(worker,SIGKILL);waitpid(worker,&status,0);throw std::runtime_error("engine deadline");}poll(nullptr,0,10);}
 std::cout<<"op38 request worker status="<<status<<std::endl;if(status!=0)throw std::runtime_error("engine worker failed");
 }}catch(const std::exception& e){std::cout<<"op38 connection rejected/closed: "<<e.what()<<std::endl;close(fd);_exit(0);}
 }

}
}
int main(int argc,char** argv){try{if(argc!=3||(std::string(argv[1])!="--validate-only"&&std::string(argv[1])!="--serve"))throw std::runtime_error("explicit mode and settings required");auto s=settings(argv[2]);if(std::string(argv[1])=="--validate-only"){std::cout<<"validated 34 original configuration files; no socket or battle execution"<<std::endl;return 0;}serve(s);return 0;}catch(const std::exception& e){std::cerr<<"op38 rejected: "<<e.what()<<std::endl;return 2;}}
