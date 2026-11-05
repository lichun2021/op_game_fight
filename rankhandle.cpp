#include "rankhandle.h"
#include <ctime>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sstream>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <string>

rankhandle::rankhandle(void):
all_data(),all_user_index()
{
}

rankhandle::rankhandle(uint64_t businessKey,int perblock,int totalblock):
all_data(),all_user_index()
{
	this->businessKey=businessKey;
	this->perblock=perblock;
	this->totalblock=totalblock;
	all_data.reserve(totalblock);
}
rankhandle &rankhandle::operator =(const rankhandle &other)
{
	all_data=other.all_data;
	all_user_index=other.all_user_index;
	businessKey=other.businessKey;
	perblock=other.perblock;
	totalblock=other.totalblock;
	return *this;
}
rankhandle::rankhandle(const rankhandle &other):
all_data(other.all_data),all_user_index(other.all_user_index)
{
	businessKey=other.businessKey;
	perblock=other.perblock;
	totalblock=other.totalblock;
	all_data.reserve(totalblock);
}

rankhandle::~rankhandle(void)
{
}

bool rankhandle::init()
{
	//临时方案
	return true;
}

void rankhandle::fin()
{

}

bool rankhandle::get_node_info_by_uid(block_index_it bit,uint64_t userid,node_data_it &result,int &pos)
{
	blockindexdata &block=*bit;
	if(block.size()<=0)
	{
		return false;
	}
	result=block.nodelist.begin();
	pos=block.beginindex;
	for(;result!=block.nodelist.end();++result)
	{
		if(result->userid==userid)
		{
			break;
		}
		++pos;
	}
	if(result==block.nodelist.end())
		return false;
	return true;
}

bool rankhandle::get_node_info_by_pos(block_index_it bit,int pos,node_data_it &result)
{
	cout<<"get_node_info_by_pos 1"<<" "<<pos<<" "<<endl;
	blockindexdata &block=*bit;
	if(block.size()<=0)
	{
		return false;
	}
	result=block.nodelist.begin();
	int index=block.beginindex;
	cout<<"get_node_info_by_pos 3"<<" "<<index<<endl;
	while(result!=block.nodelist.end())
	{
		if(index>=pos)
		{
			break;
		}
		++index;
		++result;
	}
	cout<<"get_node_info_by_pos 2"<<" "<<index<<endl;
	/*for(;result!=block.nodelist.end();++result)
	{
	if(index>=pos)
	{
	break;
	}
	++index;
	}*/
	//cout<<"get_node_info_by_pos 2"<<" "<<pos<<" "<<index<<endl;
	if( (result==block.nodelist.end()) || (index != pos) )
	{	
		cout<<"here...."<<endl;
		return false;
	}
	else
	{
		return true;
	}
}

bool rankhandle::get_block_node_info_by_pos(block_index& blockdatacontain,int pos,block_index_it &bit,node_data_it &nit)
{
	cout<<"get_block_node_info_by_pos"<<" "<<pos<<" "<<endl;
	if(pos<0 || blockdatacontain.empty() || pos>=blockdatacontain.back().beginindex+blockdatacontain.back().size()){
		return false;
	}
	
	block_index_it beg=blockdatacontain.begin();
	block_index_it last=blockdatacontain.end()-1;
	block_index_it mid;
	while(beg <= last)  
    	{
		mid = beg+(last-beg)/2;
		if (pos >= mid->beginindex && pos<mid->beginindex+mid->size())  
		{
			break;
		}
		else if (mid->beginindex < pos)
		{
			beg = mid+1;
		}
		else
		{
			last = mid-1;
		}
    	}
	 
	if(beg<=last)
	{
		bit=mid;
		return get_node_info_by_pos(bit, pos, nit);
	}
	return false;
}

block_index_it rankhandle::get_block_info_by_value(block_index_it beg,block_index_it last,uint64_t value)
{
	block_index_it low=beg;
	block_index_it high=last + 1;//??
	block_index_it mid;
	/*
	while(low <= high)  
	{
		mid = low+(high-low)/2;
   	 	if (value <= mid->maxvalue() && value >= mid->minvalue()){
      	 	 	return mid;
     		}
	        if (value > mid->maxvalue()){
   		     	high = mid - 1;
  	   	   	if (value < high->maxvalue()){
        			return high;
 		       	}
	        }
	        else {
    		    	low = mid + 1;
  		      	if (value > low->minvalue()){
				return low;
	        	}
	        }	
	}  
	
	return low;
	*/
	/*
	  edited by dy
	 */
	while (low < high) {
	  mid = low + (high - low) / 2;
	  if (value > mid->maxvalue())
	    high = mid;
	  else 
	    low = mid + 1;
	}
	return std::max(low - 1, beg);
}

vector<userdata>* rankhandle::get_range(block_index_it begin,block_index_it end,node_data_it nbeg,int pos,int size)
{
	vector<userdata>* result=new vector<userdata>();
	int count=size;
	node_data_it node_beg=nbeg;
	node_data_it node_end=begin->nodelist.end();
	int indexbeg=pos;
	while(true)
	{
		if(count<=0)
			break;
		if(node_beg==node_end)
		{
			if(++begin==end)
			{
				break;
			}
			else
			{
				node_beg=begin->nodelist.begin();
				node_end=begin->nodelist.end();
			}
		}
		////cout<<"get_range"<<indexbeg<<endl;
		userdata u(*node_beg,indexbeg);
		result->push_back(u);
		--count;
		++node_beg;
		++indexbeg;
	}
	return result;
}

vector<userdata>* rankhandle::get_range_around(block_index& blockdatacontain,int blockindex,int pos,int size)
{
	vector<userdata>* result=NULL;
	if(size<=0)
		return result;
	cout<<"get_range_around begin---------------"<<blockindex<<" "<<pos<<" "<<size<<" "<<endl;
	blockindexdata &block=blockdatacontain[blockindex];
	int beg=blockindex-1;
	int blocksize=pos-block.beginindex;
	while(beg>=0 && blocksize<size)
	{
		blocksize+=blockdatacontain[beg].size();
		--beg;
	}
	int tempsize=size;
	beg+=1;
	if(blocksize<size)
	{
		tempsize=blocksize;
	}
	cout<<"front info"<<beg<<" "<<tempsize<<" "<<pos<<endl;
	node_data_it nit;
	bool ret=get_node_info_by_pos(blockdatacontain.begin()+beg,pos-tempsize,nit);
	cout<<"front detail"<<ret<<":"<<nit->userid<<":"<<nit->uservalue<<endl;
	if(ret)
		return get_range(blockdatacontain.begin()+beg,blockdatacontain.end(),nit,pos-tempsize,tempsize+1+size);
	return result;
}

vector<userdata>* rankhandle::get_rank_range(int pos,int size)
{
	vector<userdata>* result=NULL;
	if(size<=0)
	{
		return result;
	}

	block_index& the_data=all_data;
	block_index_it bit;
	node_data_it nit;
	bool ret=get_block_node_info_by_pos(the_data,pos,bit,nit);
	cout<<"------ret: "<<ret<<endl;
	if(ret)
	{
		return get_range(bit,the_data.end(),nit,pos,size);
	}
	return result;
}

int rankhandle::get_rank_total()
{
	return this->total_count();
}

vector<userdata>* rankhandle::get_user_around(uint64_t userid,int size)
{
	vector<userdata>* result=NULL;
	if(size<=0)
		return result;

	user_index &the_index=all_user_index;
	user_index_it userindex=the_index.find(userid);
	if(the_index.end()!=userindex)
	{
		int blockindex=userindex->second;
		block_index& blockdatacontain=all_data;
		if(blockindex>=0 && blockindex< blockdatacontain.size())
		{
			node_data_it nit;
			int indexpos=0;
			bool ret=get_node_info_by_uid(blockdatacontain.begin()+blockindex,userid,nit,indexpos);
			if(ret)
			{
				result= get_range_around(blockdatacontain,blockindex,indexpos,size);
			}
		}
	}
	return result;
}

bool rankhandle::adjustblock_one(int begin,user_index& the_index,block_index& the_data,uint64_t userid,bool& flag,int &newbit,node_data_it &newnit)
{
	int end=the_data.size()-1;
	if(begin>end)
		return false;
	cout<<"adjust begin----------------------------"<<begin<<":"<<userid<<endl;
	int first=0;
	int adjust_count=0;
	while(begin+1<=end)
	{
		//cout<<"adjusting:"<<begin<<" cursize:"<<the_data[begin].size()<<" perblock:"<<perblock<<endl;
		if(the_data[begin].size()>perblock)
		{
			int next=(begin+1);
			nodedata &thepop=the_data[begin].nodelist.back();
			//cout<<"tui hou:"<<the_data[next].size()<< " max:"<<the_data[next].maxvalue()<<endl;
			the_data[next].nodelist.push_front(thepop);
			the_index[thepop.userid]=begin-first+1;
			if(thepop.userid==userid)
			{
				flag=true;
				newbit=next;
				newnit=the_data[next].nodelist.begin();
			}
			//cout<<"tui hou result:"<<the_data[next].size()<< "max:"<<the_data[next].maxvalue()<<" old value:"<<thepop.uservalue<<endl;
			the_data[begin].nodelist.pop_back();
		}
		else if(the_data[begin].size()<perblock)
		{
			int next=(begin+1);
			nodedata &thepop=the_data[next].nodelist.front();
			//cout<<"la qian:"<<the_data[next].size()<< " max:"<<the_data[next].maxvalue()<<endl;
			the_data[begin].nodelist.push_back(thepop);
			the_index[thepop.userid]=begin-first;
			if(thepop.userid==userid)
			{
				flag=true;
				newbit=begin;
				node_data_it temp=the_data[begin].nodelist.end();
				newnit=--temp;
			}
			the_data[next].nodelist.pop_front();
			//cout<<"la qian result:"<<the_data[next].size()<< " max:"<<the_data[next].maxvalue()<<endl;
		}
		else
			break;
		++begin;
		++adjust_count;
	}
	cout<<"adjust_count:"<<adjust_count<<endl;
	cout<<"last one"<<endl;
	//最后,追加
	if(the_data[begin].size()>perblock)
	{
		cout<<"last one info:"<<the_data[begin].size()<<" max:"<<the_data[begin].maxvalue()<<endl;
		nodedata thepop=the_data[begin].nodelist.back();
		the_data[begin].nodelist.pop_back();
		cout<<"last one new info:"<<the_data[begin].size()<<endl;
		if(begin>=totalblock-1)
		{
			cout<<"reach max:"<<thepop.userid<<endl;
			the_index.erase(thepop.userid);
			if(thepop.userid==userid)
			{
				flag=true;
				newbit=-1;
			}
		}
		else
		{
			cout<<"create new:"<<all_data.size()<<endl;
			blockindexdata bd;
			bd.beginindex=the_data[begin].beginindex+perblock;
			bd.nodelist.push_front(thepop);
			the_data.push_back(bd);
			the_index[thepop.userid]=the_data.size()-1;
			if(thepop.userid==userid)
			{
				flag=true;
				newbit=the_data.size()-1;
				newnit=the_data.back().nodelist.begin();
			}
			cout<<"create new result:"<<all_data.size()<<" new size:"<<the_data.back().size()<<endl;
		}
	}
	return true;
}

bool rankhandle::isfull()
{
	if(perblock*totalblock<=0)
		return true;
	if(all_data.size()>0 && all_data.back().beginindex+all_data.back().size()>=perblock*totalblock)
		return true;
	return false;
}

bool rankhandle::inserttorank(user_index& the_index,block_index& the_data,nodedata &data,block_index_it &newbit,node_data_it &newnit,int &pos)
{
	int index=0;
	block_index_it blockinfo;
	block_index_it adjust_beg;
	block_index_it adjust_end;
	uint64_t userid=data.userid;
	uint64_t value=data.uservalue;
	cout<<"inserttorank begin------------------------"<<userid<<" "<<value<<" "<<the_index.size()<<" "<<pos<<endl;
	user_index_it userindex=the_index.find(userid);

	if(userindex!=the_index.end())//find index
	{
		cout<<"old in:"<<userindex->second<<endl;
		if(userindex->second<0 || userindex->second>=the_data.size())
		{
			return false;
		}
		node_data_it result;
		int indexpos=0;
		block_index_it preblock=the_data.begin()+userindex->second;
		blockindexdata& userblock=*preblock;
		bool ret=get_node_info_by_uid(preblock,userid,result,indexpos);
		cout<<"old in block:"<<indexpos<<endl;
		if(!ret)
			return false;
		//删除原信息
		uint64_t prevalue=result->uservalue;
		cout<<"old value:"<<prevalue<<endl;
		if(prevalue==value)
		{
			newbit=preblock;
			newnit=result;
			pos=indexpos;
			return true;
		}
		cout<<"check if has been here1"<<endl;
		userblock.nodelist.erase(result++);
		cout<<"check if has been here2"<<endl;
		//cout<<"max: "<<userblock.maxvalue()<<" sec: "<<userindex->second<<" size: "<<the_data.size()-1<<" value: "<<value<<" next: "<<the_data[userindex->second+1].maxvalue()<<endl;
		//暂时不需要调整块信息
		if((value>prevalue && (value<=userblock.maxvalue() || userindex->second==0)) || (value<prevalue && (userindex->second==(the_data.size()-1) || value>=the_data[userindex->second+1].maxvalue())))//还在本快
		{
			cout<<"check if has been here3"<<endl;

			cout<<"stall in old block:"<<userblock.maxvalue()<<endl;
			cout<<":"<<userindex->second<<endl;
			cout<<":"<<the_data.size()-1<<endl;
			//cout<<":"<<the_data[userindex->second+1].maxvalue()<<endl;
			cout<<"check if has been here3.1"<<endl;
			blockinfo=preblock;
			adjust_beg=preblock;
			adjust_end=preblock;
			pos=0;
		}
		else
		{
			cout<<"check if has been here4"<<endl;
			if(value>prevalue)
			{
				cout<<"front:"<<endl;
				blockinfo=get_block_info_by_value(the_data.begin(),the_data.begin()+(userindex->second-1),value);
				cout<<"front info:"<<blockinfo-all_data.begin()<<" beginpos:"<<blockinfo->beginindex<<" maxvalue"<<blockinfo->maxvalue()<<endl;
				adjust_beg=blockinfo;
				adjust_end=preblock;
				pos=0;
			}
			else
			{
				cout<<"back:"<<endl;
				blockinfo=get_block_info_by_value(the_data.begin()+userindex->second+1,the_data.end()-1,value);
				cout<<"back info:"<<blockinfo-all_data.begin()<<" beginpos:"<<blockinfo->beginindex<<" maxvalue"<<blockinfo->maxvalue()<<endl;
				adjust_beg=preblock;
				adjust_end=blockinfo;
				pos=-1;
			}
		}
	}
	else
	{
		pos=0;
		cout<<"new:"<<endl;
		if(the_data.size()<=0)
		{
			cout<<"the first one:"<<endl;
			if(isfull())
			{
				return false;
			}
			the_data.push_back(blockindexdata());
			blockinfo=the_data.begin();
			adjust_beg=blockinfo;
			adjust_end=blockinfo;
		}
		else
		{
			cout<<"not the first:"<<endl;
			if(value<=all_data.back().nodelist.back().uservalue && isfull())
				return false;
			cout<<"here"<<endl;
			blockinfo=get_block_info_by_value(the_data.begin(),the_data.end()-1,value);
			cout<<"there"<<endl;
			//			cout<<blockinfo<<endl;
			//			cout<<all_data.begin()<<endl;
			cout<<"blockinfo info:"<<blockinfo-the_data.begin()<<endl;
			cout<<"the_data.size(): "<<the_data.size()<<endl;
			//			cout<<" beginpos:"<<blockinfo->beginindex<<endl;
			//			cout<<" maxvalue"<<blockinfo->maxvalue()<<endl;
			adjust_beg=blockinfo;
			adjust_end=the_data.end()-1;
		}
	}
	cout<<"insert into the block:"<<blockinfo-all_data.begin()<<endl;
	node_data_it curuser_beg=blockinfo->nodelist.begin();
	node_data_it curuser_end=blockinfo->nodelist.end();
	cout<<"blockinfo nodelist size:"<<blockinfo->nodelist.size()<<endl;
	index=0;
	while(curuser_beg!=curuser_end)
	{
		cout<<curuser_beg->uservalue<<":\t"<<value<<endl;
		if(curuser_beg->uservalue<value)
		{
			break;
		}
		++curuser_beg;
		++index;
	}
	cout<<"new index:"<<index<<endl;
	blockinfo->nodelist.insert(curuser_beg,data);
	int tempindex=blockinfo-the_data.begin();
	if(userindex!=the_index.end())
	{
		userindex->second=tempindex;
	}
	else
	{
		the_index[userid]=tempindex;
	}
	cout<<"update user index:"<<userid<<" index:"<<the_index[userid]<<endl;
	pos+=blockinfo->beginindex+index;
	cout<<"new pos:"<<pos<<endl;
	bool flag=false;
	int newbindex=0;
	int adjust_index=adjust_beg-the_data.begin();
	cout<<"adjust info:"<<adjust_index<<" "<<adjust_end-the_data.begin()<<endl;
	adjustblock_one(adjust_index,the_index,the_data,userid,flag,newbindex,newnit);
	cout<<"adjustblock result:"<<flag<<endl;
	if(!flag)
	{
		newbit=the_data.begin()+tempindex;
		newnit=--curuser_beg;
		cout<<"check result:"<<newnit->userid<<endl;
	}
	else
	{
		if(newbindex==-1)
			return false;
		newbit=the_data.begin()+newbindex;
	}
	return true;
}

vector<userdata>* rankhandle::set_s_value(nodedata& data,int size)
{
	vector<userdata>* result=NULL;
	cout<<"set_s_value begin------------------"<<" "<<data.userid<<" "<<data.uservalue<<" "<<size<<endl;
	cout<<"all_user_index size:"<<all_user_index.size()<<endl;
	user_index &the_index=all_user_index;
	block_index &the_data=all_data;
	block_index_it newbit;
	node_data_it newnit;
	int pos=0;
	bool ret=inserttorank(the_index,the_data,data,newbit,newnit,pos);
	cout<<"return value:"<<ret<<endl;
	if(ret && size>0)
	{
		result=get_range_around(the_data,newbit-the_data.begin(),pos,size);
	}
	cout<<"set_s_value end------------------"<<endl;
	return result;
}

void rankhandle::set_m_value(nodedata *data,int size)
{
	if(size<=0)
		return;
	user_index &the_index=all_user_index;
	block_index &the_data=all_data;
	block_index_it newbit;
	node_data_it newnit;
	int pos;
	while(size>0)
	{
		inserttorank(the_index,the_data,data[size-1],newbit,newnit,pos);
		--size;
	}
}

#define RANKINDEXVERTION 10000
bool rankhandle::read(string path,string indexname)
{
	//10000 版本号
	char name[255];
	rankserialization filesteam;
	rankserialization blocksteam;
	try
	{
		cout<<"read begin--------------------------------"<<indexname<<endl;
		if(!filesteam.read_open(path+"/"+indexname))
		{
			return false;
		}
		int vertion=filesteam.read_uint32();
		cout<<"vertion:"<<vertion<<endl;
		if(vertion!=RANKINDEXVERTION)
		{
			filesteam.close();
			return false;
		}
		int blocklen=filesteam.read_uint32();
		cout<<"blocklen:"<<blocklen<<endl;
		int i=0;
		blockindexdata *curblock=NULL;
		int temp_count=0;
		while(i<blocklen && all_data.size()<this->totalblock)
		{
			cout<<"block read begin----------------------------"<<endl;
			int nodevertion=filesteam.read_uint32();
			cout<<"node vertion:"<<nodevertion<<endl;
			if(nodevertion!=NODEDATAVERTION)
			{
				filesteam.close();
				return false;
			}
			int begpos=filesteam.read_uint32();
			int nodelen=filesteam.read_uint32();
			temp_count+=nodelen;
			int name_size=filesteam.read_uint8();
			filesteam.read_string(name,name_size);
			name[name_size]=0;
			cout<<"old block info:"<<begpos<<" nodelength:"<<nodelen<<" name:"<<name<<endl;
			if(!blocksteam.read_open(path+"/"+string(name)))
			{
				filesteam.close();
				return false;
			}
			cout<<"new block info:"<<this->perblock<<endl;
			int leftsize=nodelen;
			do 
			{
				if(curblock==NULL)
				{
					curblock=new blockindexdata();
					curblock->beginindex=this->total_count();//begpos+(nodelen-leftsize);
				}
				int left=this->perblock-curblock->size();
				cout<<"new block left:"<<left<<endl;
				cout<<"old block left:"<<leftsize<<endl;
				if(left>leftsize)
				{
					if(!curblock->read(blocksteam,leftsize,nodevertion,this->all_user_index,this->all_data.size()))
					{
						filesteam.close();
						blocksteam.close();
						return false;
					}
					leftsize=0;
				}
				else if(left==leftsize)
				{
					if(!curblock->read(blocksteam,leftsize,nodevertion,this->all_user_index,this->all_data.size()))
					{
						filesteam.close();
						blocksteam.close();
						return false;
					}
					leftsize=0;
					this->all_data.push_back(*curblock);
					delete curblock;
					curblock=NULL;
				}
				else
				{
					if(!curblock->read(blocksteam,left,nodevertion,this->all_user_index,this->all_data.size()))
					{
						filesteam.close();
						blocksteam.close();
						return false;
					}
					this->all_data.push_back(*curblock);
					delete curblock;
					curblock=NULL;
					leftsize-=left;
				}

			} while (leftsize>0 && all_data.size()<this->totalblock);
			blocksteam.close();
			blocksteam.clear();
			++i;
			cout<<"print i"<<i<<endl;
		}
		if(curblock!=NULL)
		{
			this->all_data.push_back(*curblock);
			delete curblock;
			curblock=NULL;
		}
		blocksteam.close();
		filesteam.close();
		cout<<"read end--------------------------------"<<this->all_data.size()<<":"<<all_user_index.size()<<"yuanyou num:"<<temp_count<<" total num:"<<total_count()<<endl;
	}
	catch(readException& e)
	{
		filesteam.close();
		blocksteam.close();
		return false;
	}
	return true;
}

bool rankhandle::write(string path,string indexname)
{
	rankserialization filesteam;
	if(!filesteam.write_open(path+"/"+indexname))
	{
		return false;
	}
	cout<<"rankhandle log begin"<<endl;
	filesteam.write_uint32(RANKINDEXVERTION);
	filesteam.write_uint32(all_data.size());
	block_index_it block_beg=all_data.begin();
	block_index_it block_end=all_data.end();
	int i=0;
	rankserialization blocksteam;
	while(block_beg!=block_end)
	{
		filesteam.write_uint32(NODEDATAVERTION);
		filesteam.write_uint32((*block_beg).beginindex);
		filesteam.write_uint32((*block_beg).size());
		stringstream ss;
		ss<<this->businessKey<<i<<".data";
		string name=ss.str();
		filesteam.write_uint8(name.size());
		filesteam.write_string((char *)name.c_str(),name.size());
		cout<<"block log begin:"<<name<<endl;
		if(!blocksteam.write_open(path+"/"+name))
		{
			filesteam.close();
			return false;
		}
		node_data_it beg=block_beg->nodelist.begin();
		node_data_it end=block_beg->nodelist.end();
		cout<<"node log begin:"<<name<<endl;
		while(beg!=end)
		{
			if(!beg->write(blocksteam))
			{
				blocksteam.close();
				filesteam.close();
				return false;
			}
			++beg;
		}
		blocksteam.close();
		++i;
		++block_beg;
	}
	filesteam.close();
	return true;

}

vector<blockindexdata>& rankhandle::get_data()
{
	return this->all_data;
}

int rankhandle::total_count()
{
	if(all_data.empty())
		return 0;
	return all_data.back().beginindex+all_data.back().nodelist.size();
}

int rankhandle::copyFile(std::string read_dir_path, std::string write_dir_path) {  
    DIR * p_dir;  
    struct dirent * p_dirent;  
  
    read_dir_path += "/";
    write_dir_path += "/";
 	 
    if( isdir(read_dir_path) ) {  
        //mkdir   
        if(mkdir(write_dir_path.c_str(), 0755) < 0) {    
            return -1;  
        }
  
        //opendir and readdir   
        if((p_dir = opendir(read_dir_path.c_str())) == NULL ) {  
            return -2;  
        }  
  
        while((p_dirent=readdir(p_dir)) != NULL) {  
            std::string read_buffer = read_dir_path+p_dirent->d_name;
            std::string write_buffer = write_dir_path+p_dirent->d_name;
  
            //it is a directory   
            if( isdir(read_buffer) && 0 != strcmp(p_dirent->d_name, ".") && 0 != strcmp(p_dirent->d_name, "..")) { 
                copyFile(read_buffer, write_buffer);
            }
            else if(0 != strcmp(p_dirent->d_name, ".") && 0 != strcmp(p_dirent->d_name, "..")) {  
                if( copyFileContent(read_buffer, write_buffer) > 0 ) { return 0; } 
            } 
        }  
        closedir(p_dir); 
        return 0;
    }
    else{
    	return -4;
    }   
} 

int rankhandle::copyFileContent(std::string original, std::string target){
    FILE *in_file, *out_file;
    char data[256];
    size_t bytes_in, bytes_out;
    long len = 0;

    if ( (in_file = fopen(original.c_str(), "rb")) == NULL ) { return 2; }
    if ( (out_file = fopen(target.c_str(), "wb")) == NULL ) { return 3; }

    while ( (bytes_in = fread(data, 1, 256, in_file)) > 0 ) {
        bytes_out = fwrite(data, 1, bytes_in, out_file);
        if ( bytes_in != bytes_out ) {
            return 4;
        }
        len += bytes_out;
    }

    fclose(in_file);
    fclose(out_file);

    return 0;
}

int rankhandle::isdir(const std::string path)
{
    struct stat buf;
    int status=stat(path.c_str(),&buf);
    if(!status && (buf.st_mode & S_IFDIR)) return 1;
    return status;
}

int rankhandle::removeFile(std::string path, uint64_t businessKey){
	int len = all_data.size();
	while(len>=0){
		char* pPath = getenv("PWD");
		chdir(pPath);
		stringstream ss;
		ss<<this->businessKey<<len<<".data";
		std::string name=ss.str();
		std::string file_path = path+"/"+name;
		remove(file_path.c_str());
		--len;
	}
	stringstream sstr;
	sstr<<businessKey<<"index"<<".in";
	string index_name=sstr.str();
	std::string file_path = path+"/"+index_name;
	char* pPath = getenv("PWD");
	chdir(pPath);
	return remove(file_path.c_str());
}

int rankhandle::managerRemoveFile(short _port, uint64_t businessKey){
	int status = managerMoveFile(_port);
	if (status != 0){
		std::cout<<"----error: "<<status<<std::endl;
		return -1;
	}else{
		stringstream sstr;
		sstr<<_port;
		std::string name = sstr.str();
		std::string data1 = "./"+name+"/data1";
		std::string data2 = "./"+name+"/data2";
		status = removeFile(data1, businessKey);
		status = removeFile(data2, businessKey);
		all_user_index.clear();
		all_data.clear();
		return status;
	}	
}

int rankhandle::managerMoveFile(short _port){
	char time_path[80];
	time_t nowtime = time(NULL);
	strftime(time_path, 80, "%Y%m%d%H%M%S", localtime(&nowtime));
	std::string port_;
	stringstream sstr;
	sstr << _port;
	sstr >> port_;
	return copyFile(port_, time_path);
}

