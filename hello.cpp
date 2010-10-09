#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <map>
#include <cassert>
#include <string>
#include <vector>
#include <sphinxclient.h>
using namespace std;
void usage()
{
	cout<<"Usage:"<<"params:"<<"[--host=][--port=][--index=][--query=][--limit=][--offset=][--filter=]"<<endl;
	cout<<" --host= tell me the sphinx server ip,default is localhost"<<endl;
	cout<<" --port= tell me the sphinx server port,defalult is 9312"<<endl;
	cout<<" --index= the index name to query, must be seted"<<endl;
	cout<<" --query= the sphinx query language ,we use extend2 match mode"<<endl;
	cout<<" --filter= the sphinx filters you set ,just for int values now,the pattern is: key1=value1&key2=value2"<<endl;
	cout<<" --range= filter by range ,can be int or float , you must input dot('.') if using float"<<endl;
	cout<<" --offset= where to start,default value is 0"<<endl;
	cout<<" --limit= how many to return,cooperates with offset, default value is 10"<<endl;
	cout<<"for example:"<<"./hellosphinx --host=10.10.1.1 --port=9312 --index=product_index_main_5 --query=\"租房\" --filter=\"cateID=43&cityID=188\" --limit=10 --offset=0"<<endl;
}
struct range_t{
	typedef union {
		double value_f;
		int value_i;
	} RangeType;
	RangeType min ;
	RangeType max ;
	bool bFloat;
	range_t():bFloat(false)
	{
		min.value_f =0;
		max.value_f = 0;
	}
	
};
map<string,string> parsefilters(string filter)
{
	map<string,string> filters;
	int len = filter.length();

	if(len<2){return filters;}  // check for at least "q="
	
	char next = '=';
	string name,value;
	int i = 0;
	int j = i ;

	while(true)
	{
		while(filter[++i]!=next && i < len);
		if(next == '=')
		{
			name = filter.substr(j,i-j);
			next = '&';
		}
		else if(next=='&')
		{
			value = filter.substr(j,i-j);
			filters[name] = value;
			next = '=';
		}
		if(i==len) break;
		++i;
		j = i ;

	}
	return filters;

}
vector<string> split(const string &src , const string &split)
{
	vector<string> res;
	string::size_type prepos = 0;
	string::size_type pos = 0;
	while((pos = src.find(split,prepos))!=string::npos)
	{
		string s = src.substr(prepos,pos-prepos);
		res.push_back(s);
		prepos = pos+1;
	}
	if(prepos < src.size()-1)res.push_back(src.substr(prepos));
	return res;
}

map<string,range_t> parseranges(string range)
{
	// example :range="rent(300,500)&price(33.1,51.3)"	
	map<string,range_t> res;	
	vector<string> attrs = split(range,"&");
	for(size_t i = 0 ;i < attrs.size(); ++i)
	{
		string attr = attrs[i];
		int j = 0;
		int len = attr.size();
		string::size_type cur = 0;
		while(attr[j++]!='(' && j < len);
		string name = attr.substr(cur,j-1-cur);
		if(name.length()==0 || j>=len){cerr<<"parse error near:"<<attr<<endl;exit(2);}
		assert(j<len);
		cur = j;

		while(attr[j++]!=',' && j < len);
		string value = attr.substr(cur,j-1-cur);
		if(value.length()==0 || j>=len){cerr<<"parse error near:"<<attr<<endl;exit(2);}
		cur = j;
		range_t range;
		range_t::RangeType min,max;
		if(value.find(".")!=string::npos)
			min.value_f = atof(value.c_str());
		else
			min.value_i = atoi(value.c_str());
		assert(j<len);

		while(attr[j++]!=')' && j < len);
		value = attr.substr(cur,j-1-cur);
		if(value.length()==0 || j>len){cerr<<"parse error near:"<<attr<<endl;exit(2);}
		cur = j;
		if(value.find(".")!=string::npos)
		{
			max.value_f = atof(value.c_str());
			range.bFloat = true;
		}
		else
			max.value_i = atoi(value.c_str());
		range.min = min;
		range.max = max;
		res[name] = range;
	}
	return res;
}
map<string,string> parseopts(int argc,char *argv[])
{
	map<string,string> opts;
	for(int i = 1; i < argc; i++)
	{
		int j = 0;
		int len = strlen(argv[i]);
		char *arg = argv[i];
		if(arg[j] == '-'&&j+1<len)
		{
			while(arg[++j]=='-' && j+1<len); // skip '-'     so you can write any consequent '-'s in fact
			switch (arg[j])
			{
				case 'h':while(arg[++j]!='='&&j<len);if(j<len)(opts["host"] = arg+j+1);
						 break;
				case 'p':while(arg[++j]!='='&&j<len);if(j<len)(opts["port"] = arg+j+1);
						 break;
				case 'i':while(arg[++j]!='='&&j<len);if(j<len)(opts["index"] = arg+j+1);
						 break;
				case 'q':while(arg[++j]!='='&&j<len);if(j<len)(opts["query"] = arg+j+1);
						 break;
				case 'l':while(arg[++j]!='='&&j<len);if(j<len)opts["limit"] = arg+j+1;
						 break;
				case 'o':while(arg[++j]!='='&&j<len);if(j<len)opts["offset"] = arg+j+1;
						 break;
				case 'f':while(arg[++j]!='='&&j<len);if(j<len)opts["filter"] = arg+j+1;
						 break;
				case 'r':while(arg[++j]!='='&&j<len);if(j<len)opts["range"] = arg+j+1;
						 break;
				default:
					 cout<<"unkown param :"<<arg<<endl;
					 usage();exit(1);
					 break;
			}

		}
		else
		{
			usage(); exit(1);
		}
	}
	return opts;

}

int main(int argc,char **argv)
{
	string host= "localhost";
	int port = 9312;
	string indexName;
	string query ;
	int limit = 10;
	int offset = 0;

	if(argc==1)
	{
		usage();
		return 1;
	}
	map<string,string> opts = parseopts(argc,argv);

	host = opts["host"].length()>0?opts["host"]:"localhost";
	port = opts["port"].length()>0?atoi(opts["port"].c_str()):9312;
	limit = opts["limit"].length()>0?atoi(opts["limit"].c_str()):10;
	offset = atoi(opts["offset"].c_str());
	query = opts["query"];
	indexName = opts["index"];
	if(indexName.length()==0){cout<<"must set indexname with --index= "<<endl;exit(2);}
	map<string,string> filters = parsefilters(opts["filter"]);

	map<string,range_t> ranges;
	if(opts["range"].length()>0)
		ranges= parseranges(opts["range"]);

	//init sphinx
	sphinx_client *client;
	client = sphinx_create(SPH_TRUE);
	if(!sphinx_set_server(client,host.c_str(),port))
	{
		cout<<"set sphinx server error"<<endl;
		exit(2);
	}

	// add filters
	for(map<string,string>::iterator iter = filters.begin();iter!=filters.end(); ++iter)
	{
		sphinx_int64_t value = atoi(iter->second.c_str());
		sphinx_add_filter(client,iter->first.c_str(),1,&value,false);
	}

	// add range filters
	for(map<string,range_t>::iterator iter_range = ranges.begin(); iter_range!=ranges.end(); ++iter_range) 
	{
		range_t range = iter_range->second;
		if(!range.bFloat)
		{
			sphinx_add_filter_range(client,iter_range->first.c_str(),(sphinx_int64_t)range.min.value_i,(sphinx_int64_t)range.max.value_i,false);
		}
		else
		{
			sphinx_add_filter_float_range(client,iter_range->first.c_str(),(float)range.min.value_f,(float)range.max.value_f,false);
		}

	}

	sphinx_set_limits(client,offset,limit,0,0);
	sphinx_set_match_mode(client,SPH_MATCH_EXTENDED2);

	sphinx_result *res = NULL;
	res = sphinx_query(client,query.c_str(),indexName.c_str(),NULL);
	if(res==NULL)
	{
		cout<<"sphinx error:"<<sphinx_error(client)<<endl;
		exit(3);
	}
	int count = res->total_found;
	cout<<"total records:"<<count<<endl;
	for(size_t i = 0 ; i < res->num_matches; ++i)
	{
		cout<<i+1<<". document="<<sphinx_get_id(res,i);
		for(int j = 0 ; j < res->num_attrs; ++j)
		{
			cout<<","<<res->attr_names[j]<<"=";
			switch (res->attr_types[j])
			{
				unsigned int *mva;
				case SPH_ATTR_MULTI | SPH_ATTR_INTEGER:
				mva = sphinx_get_mva ( res, i, j );
				printf ( "(" );
				for ( int k=0; k<(int)mva[0]; k++ )
					printf ( k ? ",%u" : "%u", mva[1+k] );
				printf ( ")" );
				break;
				case SPH_ATTR_FLOAT:    printf ( "%f", sphinx_get_float ( res, i, j ) ); break;
							//case SPH_ATTR_STRING:   printf ( "%s", sphinx_get_string ( res, i, j ) ); break;
				default:                                printf ( "%u", (unsigned int)sphinx_get_int ( res, i, j ) ); break;
			}
		}
		cout<<endl;
	}

	//build keywords
	if(query.length()>0)
	{
		int num = 0;
		sphinx_keyword_info * kinfo = sphinx_build_keywords(client,query.c_str(),indexName.c_str(),true,&num);
		if (kinfo!=NULL) 
		{		
			cout<<"words:"<<endl;
			for (int i = 0 ;i < num ; ++i)
			{
				cout<<"'"<<kinfo[i].tokenized<<"' : "<<kinfo[i].num_docs<<" documents, "<<kinfo[i].num_hits<<" hits"<<endl;
			}
		}
		else
		{
			cout<<sphinx_error(client)<<endl;
		}
	}
	sphinx_destroy(client);
	return 0;
}
