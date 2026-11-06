#ifndef OP38_LOCAL_TRANSPORT_HPP
#define OP38_LOCAL_TRANSPORT_HPP
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <cerrno>
#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <chrono>
namespace op38 {
inline uint32_t u32(const unsigned char* p) {return uint32_t(p[0])<<24|uint32_t(p[1])<<16|uint32_t(p[2])<<8|uint32_t(p[3]);}
inline void put(std::vector<unsigned char>& b,uint32_t v) {b.push_back(v>>24);b.push_back(v>>16);b.push_back(v>>8);b.push_back(v);}
struct Request {uint32_t transaction;std::string json;};
inline Request decode(const std::vector<unsigned char>& frame,size_t maximum) {
 if(frame.size()<24||frame.size()>maximum||u32(frame.data())!=frame.size()-4)throw std::runtime_error("frame length");
 if(u32(frame.data()+4)!=10016)throw std::runtime_error("unapproved command");
 if(u32(frame.data()+12)!=0||u32(frame.data()+16)!=100)throw std::runtime_error("unexpected ordinary PVE business key");
 uint32_t n=u32(frame.data()+20);if(n!=frame.size()-24||n==0)throw std::runtime_error("JSON length");
 std::string json(reinterpret_cast<const char*>(frame.data()+24),n);
 if(json.find('\0')!=std::string::npos)throw std::runtime_error("embedded NUL");
 return{u32(frame.data()+8),json};
}
inline int remaining(std::chrono::steady_clock::time_point deadline) {auto n=std::chrono::duration_cast<std::chrono::milliseconds>(deadline-std::chrono::steady_clock::now()).count();if(n<=0)throw std::runtime_error("request deadline");return int(n);}
inline void transfer(int fd,unsigned char* bytes,size_t size,bool writing,std::chrono::steady_clock::time_point deadline) {
 size_t done=0;while(done<size){pollfd p{fd,short(writing?POLLOUT:POLLIN),0};int ready=poll(&p,1,remaining(deadline));
 if(ready<0&&errno==EINTR)continue;if(ready<=0)throw std::runtime_error("I/O timeout");
 ssize_t n=writing?send(fd,bytes+done,size-done,0):recv(fd,bytes+done,size-done,0);
 if(n<0&&(errno==EINTR||errno==EAGAIN||errno==EWOULDBLOCK))continue;
 if(n<=0)throw std::runtime_error("connection closed");done+=size_t(n);}
}
inline std::vector<unsigned char> receive(int fd,size_t maximum,int timeout) {
 auto deadline=std::chrono::steady_clock::now()+std::chrono::milliseconds(timeout);
 std::vector<unsigned char> b(4);transfer(fd,b.data(),4,false,deadline);uint32_t n=u32(b.data());
 if(n<20||uint64_t(n)+4>maximum)throw std::runtime_error("oversized or short frame");
 b.resize(n+4);transfer(fd,b.data()+4,n,false,deadline);return b;
}
inline void transmit(int fd,std::vector<unsigned char>& frame,size_t maximum,int timeout) {
 if(frame.size()>maximum)throw std::runtime_error("response exceeds limit");
 uint32_t n=uint32_t(frame.size()-4);frame[0]=n>>24;frame[1]=n>>16;frame[2]=n>>8;frame[3]=n;
 transfer(fd,frame.data(),frame.size(),true,std::chrono::steady_clock::now()+std::chrono::milliseconds(timeout));
}
}
#endif
