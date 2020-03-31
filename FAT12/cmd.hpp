#ifndef CMD_HPP
#define CMD_HPP
#include "func.hpp"
using namespace std;

//�鿴�ļ��� 
void tree(unsigned int pos=0x2600,int level=1){
	if(pos==0x2600){
		//�ȼ����ļ� 
		for(unsigned int j=pos; j<0x4200; j+=32){
			if(FAT[j+0xb]==0)break;
			if(FAT[j]==(char)0xe5)continue;
			if(FAT[j+0xb]==0x20){
				for(int i=0; i<level-1; i++)cout<<"  |\t";
				cout<<"  |----\t";
				//����ļ��� 
				for(unsigned i=j; i<j+8; i++){
					if(FAT[i]==0x20)break;
					if((unsigned) FAT[i]>126)cout<<'*';
					else cout<<FAT[i];
				}
				cout<<'.';
				//����ļ����� 
				for(unsigned i=j+8; i<j+11; i++){
					if(FAT[i]==0x20)break;
					if((unsigned) FAT[i]>126)cout<<'*';
					else cout<<FAT[i];
				}
				cout<<endl;
			}	
		}
		//�������Ŀ¼ 
		for(unsigned int j=pos; j<0x4200; j+=32){
			if(FAT[j+0xb]==0)break;	
			if(FAT[j]==(char)0xe5)continue;
			if(FAT[j+0xb]==0x10){
				//�����Ŀ¼�� 
				string tmp;
				for(unsigned i=j; i<j+8; i++){
					if(FAT[i]==0x20)break;
					if((unsigned) FAT[i]>126)tmp+='*';
					else tmp+=FAT[i];
				}
				if(tmp=="."||tmp=="..")continue;
				for(int i=0; i<level-1; i++)cout<<"  |\t";
				cout<<"  |----\t";
				cout<<tmp<<endl;
				unsigned int cluster = FAT[j+0x1a]&0xff + (FAT[j+0x1b]&0xff)*0x100;
				tree((cluster+31)*0x200,level+1);//�����һ�����ļ��� 
				for(int i=0; i<level; i++)cout<<"  |\t";
				cout<<endl;
			}
		}
	}
	else{
		unsigned int originPos = pos;
		//�ȼ����ļ� 
		for(unsigned int j=pos; j<pos+0x200; j+=32){
			if(FAT[j]==(char)0xe5)continue;
			if(FAT[j+0xb]==0x20){
				for(int i=0; i<level-1; i++)cout<<"  |\t";
				cout<<"  |----\t";
				//����ļ���
				for(unsigned i=j; i<j+8; i++){
					if(FAT[i]==0x20)break;
					if((unsigned) FAT[i]>126)cout<<'*';
					else cout<<FAT[i];
				}
				cout<<'.';
				//����ļ����� 
				for(unsigned i=j+8; i<j+11; i++){
					if(FAT[i]==0x20)break;
					if((unsigned) FAT[i]>126)cout<<'*';
					else cout<<FAT[i];
				}
				cout<<endl;
			}
			if(FAT[j+0xb]==0 || j+32==pos+0x200){
				unsigned int cluster = getNextCluster(pos);
				if(cluster==0xfff)break;
				pos = (cluster+31)*0x200;
				j = pos - 0x20;
			}	
		}
		//�������Ŀ¼ 
		pos = originPos;
		for(unsigned int j=pos; j<pos+0x200; j+=32){
			if(FAT[j]==(char)0xe5)continue;
			if(FAT[j+0xb]==0x10){
				//�����Ŀ¼�� 
				string tmp;
				for(unsigned i=j; i<j+8; i++){
					if(FAT[i]==0x20)break;
					if((unsigned) FAT[i]>126)tmp+='*';
					else tmp+=FAT[i];
				}
				if(tmp=="."||tmp=="..")continue;
				for(int i=0; i<level-1; i++)cout<<"  |\t";
				cout<<"  |----\t";
				cout<<tmp<<endl;
				unsigned int cluster = FAT[j+0x1a]&0xff + (FAT[j+0x1b]&0xff)*0x100;
				tree((cluster+31)*0x200,level+1);//�����һ�����ļ��� 
				for(int i=0; i<level; i++)cout<<"  |\t";
				cout<<endl;
			}
			if(FAT[j+0xb]==0 || j+32==pos+0x200){
				unsigned int cluster = getNextCluster(pos);
				if(cluster==0xfff)break;
				pos = (cluster+31)*0x200;
				j = pos - 0x20;
			}
		}
	}
}

void dir(string* cmdParts,unsigned int pos){
	if(cmdParts[1]!=""&&!getPath(cmdParts,pos,false)){
		cout<<"File not found\n";
		return;
	}
	int count = 0;
	long long int totalSize = 0;
	if(pos==0x2600){
		for(unsigned int i=pos; i!=0x4200; i+=0x200)dirCluster(i,count,totalSize);
	}
	else{
		while(1){
			dirCluster(pos,count,totalSize);
			int nextClusterNum = getNextCluster(pos);
			if(nextClusterNum == 0xfff)break;
			else pos = (nextClusterNum+31)*0x200;
		}
	}
	//���ͳ����Ϣ
	cout<<'\t'<<count<<" file(s)\t\t" ;
	cout<<strNum(totalSize)<<" bytes\n\n";
}

void cd(string* cmdParts,unsigned int &pos,bool changePath){
	if(cmdParts[1]=="")return;
	if(!getPath(cmdParts,pos,changePath))cout<<"Invalid directory\n";
}

void type(string* cmdParts,unsigned int pos){
	string filename = path_file(cmdParts);
	if(filename=="")return;
	cd(cmdParts,pos,false);
	unsigned int filePos = findFile(pos,false,filename);
	if(filePos == 0)cout<<"File not found\t-"<<cmdParts[1]<<'\\'<<filename<<endl;
	else if(filePos == 1)cout<<"Access denied\t-"<<filename<<endl;
	else{
		unsigned int size =  getFileSize(filePos);
		filePos = entry2pos(filePos);
		display(filePos,size);
	}
}

void rd(string* cmdParts,unsigned int pos){
	unsigned int originPos = pos;
	string filename = path_file(cmdParts);
	if(filename=="")return;
	cd(cmdParts,pos,false);
	pos = findFile(pos,true,filename);
	if(pos==0||pos==1){
		cout<<"Invalid path, not directory,\nor directory not empty\n\n";
		return;
	}
	unsigned int filePos = entry2pos(pos);
	if(!dirempty(filePos)){
		cout<<"Invalid path, not directory,\nor directory not empty\n\n";
		return;
	}
	if(filePos == originPos){
		cout<<"Attempt to remove current directory\t-"<<cmdParts[1]<<'\\'<<filename<<endl<<endl;
	}
	FAT[pos] = 0xe5;
	freeCluster.push_back(FAT[pos+0x1a]&0xff + (FAT[pos+0x1b]&0xff)*0x100);
}

void md(string *cmdParts,unsigned int pos){
	string name = path_file(cmdParts);
	if(name=="")return;
	cd(cmdParts,pos,false);
	if(count(name.begin(),name.end(),'.')>1 || findFile(pos,true,name)!=0){
		cout<<"Unable to creat directory\n";
		return;
	}
	pos = findFreeEntry(pos);
	//��Ŀ¼entry�ﵽ���ֵ�����ܴ��� 
	if(pos == 1){
		cout<<"No space left in root directory\n";
		return;
	}
	//��ǰĿ¼���ñ�ռ�� 
	if(pos == 0){
		if(freeCluster.size()){
			
			
			
			
		}
		else{
			unsigned int freePos = findFreePos();
			if(freePos==0){
				cout<<"No space left\n";
				return;
			}
			int cluster = freePos/0x200-31;
			endCluster(cluster);
			//��ǰĿ¼�Ĵغ�Ҫ�ĳ�cluster 
			
			
			
		}
	}
	else{
		unsigned int cluster = FAT[pos+0x1a]&0xff + (FAT[pos+0x1b]&0xff)*0x100;
		if(freeCluster.size()){
			vector<int>::iterator iter = find(freeCluster.begin(),freeCluster.end(),cluster);
			//��ǰentry���״ز��ڿ����ļ��صĶ�����,���ö���ͷ�Ĵ���Ϊ���ļ��е��״� 
			if(iter==freeCluster.end())iter = freeCluster.begin();
			makedir(name,pos,*iter);
			unsigned int tmp = getNextCluster(((*iter)+31)*0x200);
			if(tmp == 0xfff)freeCluster.erase(iter);//����ͷ�Ĵز�������һ���أ������
			//�����ڣ����ôص���һ���غŸ�Ϊ0xfff�������и�λ���滻Ϊԭ�����´صĴغ� 
			else{
				int cluster = (*iter)*3;
				endCluster(cluster);
				(*iter) = tmp;
			}
		}
		else{
			unsigned int freePos = findFreePos();
			if(freePos==0){
				cout<<"No space left\n";
				return;
			}
			int cluster = freePos/0x200-31;
			endCluster(cluster);
			makedir(name,pos,cluster);
		}
	}
}



#endif
