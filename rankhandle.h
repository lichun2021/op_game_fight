#ifndef RANKHANDLE_HHHHHHHHH
#define RANKHANDLE_HHHHHHHHH
#include "config.h"
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <utility>
#include <cstring>
#include <algorithm>
#include <sstream>
#include "rankserialization.h"
using namespace std;

#define all_data_it map<uint64_t,vector<blockindexdata> >::iterator
#define all_user_index_it map<uint64_t,map<uint64_t,int> >::iterator
#define user_index map<uint64_t,int>
#define user_index_it map<uint64_t,int>::iterator
#define block_index_it vector<blockindexdata>::iterator
#define block_index_rit vector<blockindexdata>::reverse_iterator
#define block_index vector<blockindexdata>
#define node_data_it list<nodedata>::iterator
#define node_data_rit list<nodedata>::reverse_iterator

#define NODEDATAVERTION 10000
typedef struct nodedata
{
	uint64_t userid;
	uint64_t uservalue;
	unsigned char len;
	char *buf;
	nodedata()
	{
		userid=0;
		uservalue=0;
		len=0;
		buf=NULL;
	};
	nodedata(uint64_t a,uint64_t b,unsigned char len,char *buff)
	{
		userid=a;
		uservalue=b;
		if(len>0 && buff!=NULL)
		{
			this->buf=new char[len];
			if(this->buf==NULL)
			{
				this->len=0;
				this->buf=NULL;
			}
			else
			{
				memcpy(this->buf,buff,len);
				this->len=len;
			}
		}
		else
		{
			this->len=0;
			this->buf=NULL;
		}
	};
	~nodedata()
	{
		if(len>0 && buf!=NULL)
		{
			delete [] buf;
			len=0;
			buf=NULL;
		}
	}
	nodedata(const nodedata& bd)
	{
		userid=bd.userid;
		uservalue=bd.uservalue;
		if(bd.len>0 && bd.buf!=NULL)
		{
			this->buf=new char[bd.len];
			if(this->buf==NULL)
			{
				this->len=0;
				this->buf=NULL;
			}
			else
			{
				memcpy(this->buf,bd.buf,bd.len);
				this->len=bd.len;
			}
		}
		else
		{
			this->len=0;
			this->buf=NULL;
		}
	};

	nodedata &operator =(const nodedata &other)
	{
		userid=other.userid;
		uservalue=other.uservalue;
		if(len>0 && buf!=NULL)
		{
			delete [] buf;
			len=0;
			buf=NULL;
		}
		if(other.len>0 && other.buf!=NULL)
		{
			this->buf=new char[other.len];
			if(this->buf==NULL)
			{
				this->len=0;
				this->buf=NULL;
			}
			else
			{
				memcpy(this->buf,other.buf,other.len);
				this->len=other.len;
			}
		}
		else
		{
			this->len=0;
			this->buf=NULL;
		}
		return *this;
	}

	bool read(int version,rankserialization& filesteam)
	{
		if(version!=NODEDATAVERTION)
		{
			return false;
		}
		if(this->len>0 && buf!=NULL)
		{
			delete [] this->buf;
			this->len=0;
			this->buf=NULL;
		}
		this->userid=filesteam.read_uint64();
		this->uservalue=filesteam.read_uint64();
		this->len=filesteam.read_uint8();
		this->buf=new char[this->len];
		filesteam.read_string(this->buf,this->len);
		return true;
	}

	bool write(rankserialization& filesteam)
	{
		filesteam.write_uint64(this->userid);
		filesteam.write_uint64(this->uservalue);
		filesteam.write_uint8(this->len);
		filesteam.write_string(this->buf,this->len);
		return true;
	}

} nodedata;

typedef struct userdata
{
	uint64_t userid;
	uint64_t uservalue;
	unsigned char len;
	char buf[USER_BUFF_SIZE];
	int pos;
	userdata()
	{
		userid=0;
		uservalue=0;
		len=0;
		pos=0;
	};

	userdata(nodedata &nd,int pos)
	{
		userid=nd.userid;
		uservalue=nd.uservalue;
		if(nd.len>0 && nd.buf!=NULL)
		{
			memcpy(buf,nd.buf,nd.len);
			len=nd.len;
		}
		else
			len=0;
		this->pos=pos;
	};

	userdata(const userdata& bd)
	{
		userid=bd.userid;
		uservalue=bd.uservalue;
		this->pos=bd.pos;
		if(bd.len>0 && bd.buf!=NULL)
		{
			memcpy(this->buf,bd.buf,bd.len);
			this->len=bd.len;
		}
		else
		{
			this->len=0;
		}
		
	};

	userdata &operator =(const userdata &other)
	{
		userid=other.userid;
		uservalue=other.uservalue;
		if(other.len>0 && other.buf!=NULL)
		{
			memcpy(this->buf,other.buf,other.len);
			this->len=other.len;
		}
		else
		{
			this->len=0;
		}
		this->pos=other.pos;
		return *this;
	}
} userdata;


typedef struct blockindexdata
{
	int beginindex;
	list<nodedata> nodelist;
	blockindexdata():nodelist()
	{
		beginindex=0;
		cout<<"blockindexdata"<<beginindex<<endl;
	};
	~blockindexdata()
	{
	};
	blockindexdata(const blockindexdata& bd):nodelist(bd.nodelist)
	{
		beginindex=bd.beginindex;
		cout<<"blockindexdata copy1"<<beginindex<<endl;
	};

	blockindexdata &operator =(const blockindexdata &other)
	{
		beginindex=other.beginindex;
		nodelist=other.nodelist;
		cout<<"blockindexdata copy1"<<beginindex<<endl;
		return *this;
	}

	uint64_t maxvalue()
	{
		if(nodelist.empty())
			return 0;
		return nodelist.front().uservalue;
	};

	uint64_t minvalue()
	{
		if (nodelist.empty())
		{
			return 0;
		}
		return nodelist.back().uservalue;
	}

	int size()
	{
		return nodelist.size();
	};

	bool read(rankserialization& filesteam,int size,int version,map<uint64_t,int>& userindex,int index)
	{
		cout<<"block read begin:"<<size<<":"<<version<<":"<<index<<endl;
		int i=0;
		while(i<size)
		{
			nodedata data;
			if(!data.read(version,filesteam))
			{
				return false;
			}
			if(userindex.count(data.userid)<=0)
			{
				this->nodelist.push_back(data);
				userindex[data.userid]=index;
			}
			++i;
		}
		cout<<"block read end:"<<i<<endl;
		return true;
	};

	bool write(rankserialization& filesteam)
	{
		list<nodedata>::iterator beg=this->nodelist.begin();
		list<nodedata>::iterator end=this->nodelist.end();
		while(beg!=end)
		{
			if(!(*beg).write(filesteam))
				return false;
			++beg;
		}
		return true;
	};

} blockindexdata;

class rankhandle
{
public:
	rankhandle(void);
	rankhandle(const rankhandle &other);
	rankhandle(uint64_t businessKey,int perblock,int totalblock);
	~rankhandle(void);
	rankhandle &operator =(const rankhandle &other);
	bool init();
	void fin();
	vector<userdata>* get_rank_range(int pos,int size);
	vector<userdata>* get_user_around(uint64_t userid,int size);
	vector<userdata>* set_s_value(nodedata& data,int size);
	void set_m_value(nodedata *data,int size);
	bool read(string path,string indexname);
	bool write(string path,string indexname);
	vector<blockindexdata>& get_data();
	int total_count();
	int get_rank_total();
	int copyFile(std::string read_dir_path, std::string write_dir_path);
	int copyFileContent(std::string original, std::string target);
	//int removeFile(short _port, const char* filename);
	int removeFile(std::string path, uint64_t businessKey);
	int managerRemoveFile(short _port, uint64_t businessKey);
	int isdir(const std::string path);
	int managerMoveFile(short _port);

private:
	bool isfull();
	bool get_node_info_by_pos(block_index_it bit,int pos,node_data_it &result);
	bool get_node_info_by_uid(block_index_it bit,uint64_t userid,node_data_it &result,int &pos);
	bool get_block_node_info_by_pos(block_index& blockdatacontain,int pos,block_index_it &bit,node_data_it &nit);
	block_index_it get_block_info_by_value(block_index_it beg,block_index_it last,uint64_t value);
	vector<userdata>* get_range(block_index_it begin,block_index_it end,node_data_it nbeg,int pos,int size);
	vector<userdata>* get_range_around(block_index& blockdatacontain,int blockindex,int pos,int size);
	bool inserttorank(user_index& the_index,block_index& the_data,nodedata &data,block_index_it &newbit,node_data_it &newnit,int &pos);
	bool adjustblock_one(int begin,user_index& the_index,block_index& the_data,uint64_t userid,bool& flag,int &newbit,node_data_it &newnit);

private:
	uint64_t businessKey;
	int perblock;
	int totalblock;
private:
	vector<blockindexdata> all_data;
	map<uint64_t,int> all_user_index;
};

#endif
