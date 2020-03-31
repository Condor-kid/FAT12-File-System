#ifndef FUNC_HPP
#define FUNC_HPP
#include<string>
#include<algorithm>
#include<iostream>
#include<vector>
#include<string>
#include<ctime>
#include<fstream>
#define SIZE 1474560
using namespace std;
#include<stdio.h>
char FAT[SIZE]; 
string PATH; 
vector<int> freeCluster;	//��¼�����ļ����״غ� 

//���ú����ο� 
string strNum(unsigned long long num);//�������ö��ż���� 
unsigned int getFileSize(unsigned int offset);//�����entry���׵�ַ 
int getNextCluster(unsigned int pos);//��ȡ��ǰ��������һ���ؿ�� 
unsigned int findFile(unsigned pos,bool isDry,string filename);	//�ڵ�ǰĿ¼��Ѱ���ļ��������׵�ַ���Ҳ�������0���ļ����Ͳ�ƥ�䷵��1
unsigned int entry2pos(unsigned int offset);//����entry�ĵ�ַ���������״غŶ�Ӧ�ĵ�ַ
bool getPath(string*cmdParts,unsigned int &pos,bool changePATH);//�ı䵱ǰĿ¼ 
string path_file(string* cmdParts);//��·��+�ļ�����ֿ����������ļ�������������Ϣ�����
bool dirempty(unsigned int pos);//����Ŀ¼�׵�ַ�����Ŀ¼�Ƿ�Ϊ�� 
unsigned int findFreePos();//û�п����ļ������Ŀ��д�ʱ����FAT������Ѱ��һ�����дأ��������Ӧ��ַ���޿��дط���0 

//�������ö��ż���� 
string strNum(unsigned long long num){
	string res;
	int count=0;
	while(num){
		res+=('0'+num%10);
		count++;
		if(count==3){
			count=0;
			res+=',';
		}
		num/=10;
	}
	reverse(res.begin(),res.end());
	if(*(res.begin()))res.erase(res.begin());
	if(res!="")return res;
	else return "0";
}

unsigned int getFileSize(unsigned int offset){
	unsigned int fileSize;
	fileSize = ((unsigned int)FAT[offset+0x1f]&0xff)<<24;
	fileSize += ((unsigned int)FAT[offset+0x1e]&0xff)<<16;
	fileSize += ((unsigned int)FAT[offset+0x1d]&0xff)<<8;
	fileSize += ((unsigned int)FAT[offset+0x1c]&0xff);
	return fileSize;
}

void printTime(unsigned int offset){
	//4λmm
	int time = (unsigned int)FAT[offset+0x18]>>5 & 7;
	time += (((unsigned int)FAT[offset+0x19]&1)<<3);
	if(time<10)cout<<"0";
	cout<<time<<'-';								
	//5λdd
	time = ((unsigned int)FAT[offset+0x18]&0x1F);
	if(time<10)cout<<"0";
	cout<<time<<'-';	 
	//7λyy 
	time = ((unsigned int)FAT[offset+0x19]>>1) & 0x7f;
	time+=80;
	cout<<time%100<<'\t';
	//��ӡʱ��
	bool am = true;//am or pm 
	//5λСʱ
	time = ((unsigned int)FAT[offset+0x17]>>3)&0x1f;
	if(time>12){
		time-=12;
		am=false;
	}
	if(time==12)am=false;
	cout<<time<<':'; 
	//6λ���� 
	time = ((unsigned int)FAT[offset+0x17]&7)<<3;
	time += ((unsigned int)FAT[offset+0x16]>>5)&7;
	cout<<time;
	if(am)cout<<"a";
	else cout<<"p";
}


int getNextCluster(unsigned int pos){
	if(pos<0x4000)return pos/0x200 -30 ;
	if(pos < 0x4200)return 0xfff;
	int nextClusterNum = 0;
	pos /= 0x200;
	pos -= 31;
	pos*=3;
	if(pos%2){
		pos/=2;
		nextClusterNum += (((unsigned int)FAT[0x200+pos]&0xf0)>>4);
		nextClusterNum += (((unsigned int)FAT[0x200+pos+1]&0xff)<<4);
	}
	else{
		pos/=2;
		nextClusterNum += (((unsigned int)FAT[0x200+pos])&0xff);
		nextClusterNum += (((unsigned int)FAT[0x200+pos+1]&0xf)<<8);
	}
	return nextClusterNum;
}

//�ڵ�ǰĿ¼��Ѱ���ļ��������׵�ַ 
unsigned int findFile(unsigned pos,bool isDry,string filename){	//�Ҳ�������0���ļ����Ͳ�ƥ�䷵��1
	while(1){
		for(unsigned int j=pos; j!=pos+0x200; j+=0x20){
			if(FAT[j+0xb]==0x27 || FAT[j]==(char)0xe5)continue;				//�Թ������ļ�����ɾ���ļ�
			if(FAT[j+0xb]==0)return 0;		//FAT��ʣ�ಿ��Ϊ��ʱֹͣ����
			string name;					
			for(int i=0; i<8; i++){
				if(FAT[j+i]==0x20)break;
				name += FAT[j+i];
			}
			if(FAT[j+8]!=0x20){
				name+='.';
				for(int i=8; i<11; i++){
					if(FAT[j+i]==0x20)break;
					name += FAT[j+i];
				}
			}
			if(name==filename){			//�����ļ����Ƿ�ƥ�� 
				//�ж��ļ����� 
				if((FAT[j+0xb]!=0x10 && isDry) || (FAT[j+0xb]!=0x20 && !isDry))return 1;
				return j;
			}
		}
		int tmp = getNextCluster(pos);
		if(tmp == 0xfff)return 0;
		pos = (tmp+31)*0x200;
	}
}

//����Ŀ¼���ļ���Ŀ�ĵ�ַ���������״غŶ�Ӧ�ĵ�ַ 
unsigned int entry2pos(unsigned int offset){
	unsigned int cluster = FAT[offset+0x1a]&0xff + (FAT[offset+0x1b]&0xff)*0x100;
	if(cluster==0)return 0x2600;
	return (31+cluster)*0x200;
}


//��������·�� 
bool pathStep(string step,unsigned int &pos,bool changePATH){
	pos = findFile(pos,true,step);
	if(pos==0||pos==1)return false;
	pos = entry2pos(pos); 
	if(changePATH){
		if(step==".."){
			while(*(PATH.end()-1)!='\\')PATH.erase(PATH.end()-1);
			PATH.erase(PATH.end()-1);
		}
		else if(step!="."){
			if(*(PATH.end()-1)!='\\')PATH+="\\";
			PATH+=step;
		}
		if(PATH=="")PATH+="\\";
	}
	return true;
}

//��������·�� 
bool getPath(string*cmdParts,unsigned int &pos,bool changePATH){
	unsigned int originPos = pos;
	string originPATH = PATH;
	int len = cmdParts[1].length();
	//����·���Ĵ��� (·����A:/��ͷ) 
	if(len>2&&cmdParts[1][0]=='A'&&cmdParts[1][1]==':'&&cmdParts[1][2]=='\\'){
		pos=0x2600;
		cmdParts[1].erase(cmdParts[1].begin());
		cmdParts[1].erase(cmdParts[1].begin());
		cmdParts[1].erase(cmdParts[1].begin());
		len-=3;
		if(changePATH)PATH="\\";
		if(cmdParts[1]=="")return true;
	}
	int i=0;
	string step;
	while(i!=len+1){
		if(cmdParts[1][i]!='\\' && i!=len)step += cmdParts[1][i];
		else{
			if(!pathStep(step,pos,changePATH)){
				pos = originPos;
				PATH = originPATH;
				return false;
			}
			step = "";
		}
		i++;
	}
	return true;
}

//cmdParts[1]��ֳ�·��+�ļ���
string path_file(string* cmdParts){
	if(cmdParts[1]==""){
		cout<<"Required parameter missing\n";
		return "";
	}
	string filename;
	while(cmdParts[1].length()){
		if(*(cmdParts[1].end()-1)=='\\'){
			cmdParts[1].erase(cmdParts[1].end()-1);
			break;
		}
		filename += (*(cmdParts[1].end()-1));
		cmdParts[1].erase(cmdParts[1].end()-1);
	}
	reverse(filename.begin(),filename.end());
	return filename;
} 

//�Ե�����������dir���� 
void dirCluster(unsigned int pos,int &count,long long int &totalSize){
	for(int j=pos; j!=pos+0x200; j+=0x20){
		if(FAT[j+0xb]==(char)0x27 || FAT[j]==(char)0xe5)continue;		//����ʾ�����ļ�����ɾ���ļ� 
		if(FAT[j+0xb]==0)break;									//FAT��ʣ�ಿ��Ϊ��ʱֹͣ��ӡ
		//��ӡ�ļ������ļ����� 
		for(int i=0; i<11; i++){
			if((unsigned)FAT[j+i]>126)cout<<'*';
			else cout<<FAT[j+i];
			if(i==7)cout<<'\t';
		}
		if(FAT[11]=0x10)cout<<"<DIR>\t\t";
		else {
			//���ļ�����Ŀ¼���ӡ��С 
			unsigned int fileSize = getFileSize(j);
			cout<<"\t\t"<<strNum(fileSize)<<"\t";
			totalSize+=fileSize;
		}
		//��ӡ�޸�ʱ��mm-dd-yy 
		printTime(j);
		count++;
		cout<<endl;
	}
}

//չʾ�ļ�����
void display(unsigned int pos,unsigned int size){
	unsigned int num = pos;
	while(size--){
		cout<<FAT[num++];
		if(num==pos+0x200 && size){
			int cluster = getNextCluster(pos);
			if(cluster = 0xfff)return;
			pos = (cluster+31)*200;
			num = pos;
		}
	}
} 

//��鵱ǰĿ¼�Ƿ�Ϊ�� 
bool dirempty(unsigned int pos){
	for(unsigned int i=pos; i!=pos+0x200; i+=0x20){
		if(i+0x20==pos+0x200){
			unsigned int cluster = getNextCluster(pos);
			if(cluster==0xfff)return true;
			pos = (cluster+31)*0x200;
			i = pos - 0x20;
		}
		if(FAT[i]=='.'&&FAT[i+1]==0x20)continue;
		if(FAT[i]=='.'&&FAT[i+1]=='.'&&FAT[i+2]==0x20)continue;
		if(FAT[i]==(char)0xe5||FAT[i]==0)continue;
		else return false;
	}
}

//�ڵ�ǰ�ļ�����Ѱ�ҿ���λ�ò������׵�ַ,�Ҳ�������0 
unsigned int findFreeEntry(unsigned int pos){
	if(pos < 0x4200){
		while(pos != 0x4200){
			if(FAT[pos]==(char)0xe5 || FAT[pos]==0)return pos;
			pos += 0x20;
			if(pos == 0x4200)return 1;//��Ŀ¼���˷���1 
		}
	}
	else{
		for(int i=pos; i<pos+0x200; i+=0x20){
			if(FAT[i]==(char)0xe5 || FAT[i]==0)return i;
			if(i+0x20 == pos+0x200){
				unsigned int cluster = getNextCluster(pos);
				if(cluster==0xfff)return 0;
				pos = (cluster+31)*0x200;
				i = pos - 0x20;
			}
		}
	}
} 

void makeEntry(string name,unsigned int pos,int cluster,char type,unsigned int size=0){
	int len = name.length(); 
	//����+��չ�� 
	if(name=="."){
		FAT[pos]='.';
		for(int i=1; i<11; i++)FAT[pos+i]=0x20;
	}
	else if(name==".."){
		FAT[pos]=FAT[pos+1]='.';
		for(int i=2; i<11 ;i++)FAT[pos+i]=0x20;
	}
	else{
		int offset=0;
		for(int i=0; i<len&&offset<11 ;i++,offset++){
			if(name[i]=='.'){
				while(offset!=7)FAT[pos+offset++]=0x20;
				continue;
			}
			FAT[pos + offset] = name[i];
		} 
		while(offset<11){
			FAT[pos+offset]=0x20;
			offset++;
		}
	}
	//�ļ����� 
	FAT[pos+11] = type;
	//����λ 
	for(int i=12; i<22; i++) FAT[pos+i]=' ';
	//����ʱ��
	time_t now = time(0);
	tm *ltm = localtime(&now);
	unsigned int input_time= ((ltm->tm_hour&0x1f)<<11)+((ltm->tm_min&0x3f)<<5);
	FAT[pos+22] = input_time & 0xff;
	FAT[pos+23] = (input_time >> 8) &0xff;
	//��������
	unsigned int input_date = (((ltm->tm_year-80)&0x7f)<<9) + (((ltm->tm_mon+1)&0xf)<<5) + (ltm->tm_mday&0x1f);
	FAT[pos+24] = input_date & 0xff;
	FAT[pos+25] = (input_date >> 8) &0xff;
	//�״غ� 
	FAT[pos+26] = cluster & 0xff;
	FAT[pos+27] = (cluster & 0xff00)>>8;
	//�ļ���С��Ŀ¼Ϊ0�� 
	if(type==0x10)for(int i=28; i<32; i++) FAT[pos+i]=' ';
}

void makedir(string name,unsigned int pos,int cluster){
	makeEntry(name,pos,cluster,0x10);
	//д��Ŀ¼.��.. 
	unsigned int originPos = pos;
	pos = (cluster+31) * 0x200;
	makeEntry(".",pos,cluster,0x10);
	if(originPos<0x4200)makeEntry("..",pos+0x20,0,0x10);
	else makeEntry("..",pos+0x20,originPos/0x200-31,0x10);
	//���Ŀ¼��Ӧ������ȫ������
	for(unsigned int i=pos+0x40; i<pos+0x200; i+=1)FAT[i]=0;
}

//û�п����ļ������Ŀ��д�ʱ����FAT������Ѱ��һ�����дأ��������Ӧ��ַ���޿��дط���0 
unsigned int findFreePos(){
	for(unsigned int pos = 0x4200; pos<0x168000; pos+=0x200){
		if(getNextCluster(pos)==0)return pos;
	}
	return 0;
} 

//���غ���FAT���еĶ�Ӧλ�ø�Ϊ0xfff 
void endCluster(int cluster){
	if(cluster%2){
		cluster /= 2;
		FAT[0x200+cluster] |= 0xf0;
		FAT[0x200+cluster+1] = 0xff;
	}
	else{
		cluster /= 2;
		FAT[0x200+cluster] = 0xff;
		FAT[0x200+cluster+1] |= 0xf;
	}
}
#endif
