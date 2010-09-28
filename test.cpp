#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <map>
#include <vector>
using namespace std;

#define MAX_CHILD_NUMS 26
int icount = 0;
const char *valid_params[]={"host","hot","port","filter","por","hospital"};
//const char *valid_params[]={"host","ho"};
struct Node
{
	char c ;
	int ichild ;
	bool bend ;
	Node* children[MAX_CHILD_NUMS];
	Node():ichild(0),bend(false){}
};

void insert(Node *root,char *sequence,int len);// a chars sequence ,len : the length of sequence
map<string,string> parseopts(int argc, char *argv[]);
Node *build_tree(const char* params[],int size);
void destory_tree(Node *root);
void travel_tree(Node *root,string &s,vector<string> &v);
void print_tree(Node *root);
void sub_search(Node *root,string query,vector<string> &result);
vector<string> prefix_search(Node *root,string query);


map<string,string> parseopts(int argc, char *argv[])
{
	map<string,string> opts;
	if(argc==1)return opts;
	assert(argv&&"argv is NULL");


}

Node *build_tree(const char* params[],int size)
{
	Node *root = new Node();
	for(int i = 0 ;i < size; ++i)
	{
		const char *param = params[i];
		int len = strlen(param);
		insert(root,(char*)param,len);
		
	}
	return root;
}
void insert(Node *root,char *sequence,int len)// a chars sequence ,len : the length of sequence
{
	if(root ==NULL)return;
	if(sequence ==NULL)return;
	if(len == 0) return ;

/*	++icount;
	cout<<icount<<endl;*/
	//cout<<sequence<<endl;
	bool has = false;
	for(int i = 0 ;i < root->ichild; ++i)
	{
		Node *child = root->children[i];
		if(child->c == sequence[0] )
		{
			has = true;	
			if(len == 1){child->bend=true;}
			insert(child,++sequence,--len);
		}
	}
	if(!has)
	{
		//cout<<sequence<<endl;
		char ch = sequence[0];
		Node *node = new Node();
		node->c =ch;
		node->ichild = 0;
		node->bend = (len == 1)?true:false;
		root->children[root->ichild++] = node;
		Node *head = node;

		for(int j = 1 ;j < len; ++j)  // notice : here we start from 1 
		{
			Node *node = new Node();
			node->c =sequence[j];
			node->ichild = 0;
			node->bend = (len == j+1)?true:false;
			head->children[head->ichild++] = node;
			head = node;
		}
	}
}
void destory_tree(Node *root)
{
	for(int i = 0 ;i < root->ichild; ++i)
	{
		Node *child = root->children[i];
		destory_tree(child);
	}
	delete root;
}
void print_tree(Node *root)
{
	string s;
	vector<string> v;
	travel_tree(root,s,v);
	for(int i = 0 ; i != v.size() ; ++i)
	{
		cout<<v.at(i)<<endl;
	}

}
void travel_tree(Node *root,string &s,vector<string> &v)
{
	for(int i = 0 ;i < root->ichild ; ++i)
	{
		Node *child = root->children[i];
		s.push_back(child->c);
		if(child->bend)
		{
			v.push_back(s)	;
		}
		travel_tree(child,s,v);
		s.erase(s.begin()+s.length()-1);
	}

}
void sub_search(Node *root,string query,vector<string> &result)
{
	if(query.size()==0) return;
	Node *child = NULL;
	int i = 0;
	//for(int i = 0; i < query.size() ; ++i)
	{
		if(root->ichild ==0) return ;
		char c = query[i];
		bool match = false;
		for(int j = 0; j < root->ichild; ++j)
		{
			child = root->children[j];
			if (c==child->c)
			{
				sub_search(child,string(query.begin()+1,query.end()),result);
				match = true;
			}
		}

		if(match && i == query.size()-1)
		{
			string s;
			travel_tree(child,s,result);
		}
	}

}

vector<string> prefix_search(Node *root,string query)
{
	vector<string> result;
	sub_search(root,query,result);
	for(int i = 0 ;i < result.size() ; ++i)
	{
		result[i] = query+result[i];
	}
	return result;
}

int main(int argc,char *argv[])
{
	Node * root = build_tree(valid_params,sizeof(valid_params)/sizeof(valid_params[0]));

	print_tree(root);
	cout<<"*************"<<endl;
	vector<string> result;
	string query = "ho";
	result = prefix_search(root,query);

	for(int i = 0 ;i < result.size() ; ++i)
	{
		cout<<result[i]<<endl;
	}
	destory_tree(root);

	return 0;
}
