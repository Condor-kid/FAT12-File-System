#include "cmd.hpp"
using namespace std;

//������ָ���ֳɸ������� 
void cmdDecode(string* cmdParts,string cmd){
	int len = cmd.length();
	int j=0;
	for(int i=0; i!=len; i++){
		if(cmd[i]==' '){
			if(cmdParts[j]!="")j++;
		}
		else cmdParts[j]+=cmd[i];
	}
}

//��freeCluster��ʼ�� 
void initFreeCluster(unsigned int pos=0x2600){
	for(unsigned int i=pos; i<pos+0x200; i+=32){
		if(FAT[i]=='.'&&FAT[i+1]==0x20)continue;
		if(FAT[i]=='.'&&FAT[i+1]=='.'&&FAT[i+2]==0x20)continue;
		if(FAT[i]==(char)0xe5){
			freeCluster.push_back(FAT[i+0x1a]&0xff + (FAT[i+0x1b]&0xff)*0x100);
			continue;
		}
		if(FAT[i+0xb]==0x10){
			int cluster = FAT[i+0x1a]&0xff + (FAT[i+0x1b]&0xff)*0x100;
			initFreeCluster((cluster+31)*0x200);
		}
		if(FAT[i+0xb]==0)return;
	}
	if(pos<0x4200)initFreeCluster(pos+0x200);
	else{
		int cluster = getNextCluster(pos);
		if(cluster==0xfff)return;
		initFreeCluster((31+cluster)*0x200);
	}
}

//����FAT����ʱ�ĳ�ʼ�� 
void FATinit(fstream &FATfile){
	FATfile.open("test.img",ios::in|ios::out|ios::binary);
	for(int i=0; i<SIZE; i++)FATfile.read((char*)&FAT[i],sizeof(char));
	PATH = "\\";
	initFreeCluster();
}

int main(){
	fstream FATfile;
	string cmd;
	unsigned int pos=0x2600;
	FATinit(FATfile);
	while(1){
		cout<<"A:"<<PATH<<'>';
		getline(cin,cmd);
		int len = cmd.length();
		for(int i=0; i!=len; i++)if(cmd[i]>='a'&&cmd[i]<='z')cmd[i]=cmd[i]-'a'+'A';//ָ��ȫ��ת��д
		string cmdParts[3];
		cmdDecode(cmdParts,cmd);
		if(cmdParts[0]=="")continue;
		//ԭ��ָ�� 
		else if(cmdParts[0]=="DIR")dir(cmdParts,pos); 
		else if(cmdParts[0]=="CD")cd(cmdParts,pos,true);
		else if(cmdParts[0]=="RD")rd(cmdParts,pos);
		else if(cmdParts[0]=="MD")md(cmdParts,pos);
		else if(cmdParts[0]=="TYPE")type(cmdParts,pos);
		else if(cmdParts[0]=="DEL")del(cmdParts,pos);
		else if(cmdParts[0]=="CLS")system("cls");
		//ԭ��ָ�� 
		else if(cmdParts[0]=="TREE"){
			tree();
			cout<<endl;
		}
		else if(cmdParts[0]=="EXIT")return 0;
		//debug�õ�ָ�� 
		else if(cmdParts[0]=="POS")printf("0x%x\n",pos);			//��ʾ��ǰ��ַ 
		else if(cmdParts[0]=="FREE"){
			for(int i=0;i<freeCluster.size();i++)cout<<freeCluster[i]<<' ';//��ʾ�����ļ��״� 
			cout<<endl<<"size: "<<freeCluster.size()<<endl;
		}
		else if(cmdParts[0]=="P2C")p2c();		//����pos���ض�Ӧ��cluster��ֵ 
		else if(cmdParts[0]=="FULL")full(cmdParts,pos);//�ڵ�ǰĿ¼����14����Ŀ¼
		else cout<<"Bad command or file name\n";
	}
	FATfile.close();
}
//dir 10
//hiden 27
//file 20
