#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<pwd.h>
#include<termios.h>
#include<grp.h>
#include<sys/ioctl.h>
#include<bits/stdc++.h>
#include<dirent.h>
#include<math.h>
#include<fstream>
#include<filesystem>
#define cls() printf("\033[H\033[J")
#define MAX 5 //items to be displayed on the screen at a time
using namespace std;
string stringProcess(string fname);
int copyFile1(string source,string destination);
void copyall(string source,string destination);
int copyFile(vector<string> &commandSplit);
int createDir(vector<string> &commandSplit);
int createFile(vector<string> &commandSplit);
void deleteall(string rm);
vector<string> deleteDir(vector<string> &commandSplit);
int deleteFile(vector<string> &commandSplit);
int moveFile(vector<string> &commandSplit);
int rename(vector<string> &commandSplit);
void findA(string source,string key);
vector<string> search(vector<string> &commandSplit);

static struct termios initSettings,newSettings;
static int peek_char = -1;
char const *root;
struct winsize win;
vector<string> backStack;
vector<string> forwardStack;
vector<dirent*> files;
int firstIndex=0,lastIndex=firstIndex+MAX,cursor=1;
int modeline;
int output_line;
int status_line;
int input_line;

void initKeyboard(){
	tcgetattr(0,&initSettings);
	newSettings=initSettings;
	newSettings.c_lflag &= ~ICANON;
	newSettings.c_lflag &= ~ECHO;
	newSettings.c_cc[VMIN]=1;
	newSettings.c_cc[VTIME]=0;
	tcsetattr(0,TCSANOW,&newSettings);
	return;
}
void closeKeyboard(){
	tcsetattr(0,TCSANOW,&initSettings);
	return;
}
vector<string> deletefilesfolders;


vector<string> deleteDir(vector<string> &commandSplit) // It accepts arg from commandFile and call deleteall() for del.
{
	int status;
    char *remove_argument;
    deletefilesfolders.clear();
    for(unsigned i=1;i<commandSplit.size();i++)
    {
    	remove_argument= new char[commandSplit[i].length()+1];
    	strcpy(remove_argument,commandSplit[i].c_str());
    	DIR *d;
    	d=opendir(remove_argument);
    	if(d)
    	{
    		status = rmdir(remove_argument);
		    if(status!=0)
		    {
		        deletefilesfolders.push_back(commandSplit[i]);
		        deleteall(commandSplit[i]);
		    }
    	}
    	else
    		cout<<endl<<"No such directory exist"<<endl;  
    }
   
	return deletefilesfolders;
}

int kbHit(){
	char ch;
	int nread;
	if(peek_char!=-1) return 1;
	newSettings.c_cc[VMIN]=0;
	tcsetattr(0,TCSANOW,&newSettings);	
	nread=read(0,&ch,1);
	newSettings.c_cc[VMIN]=1;
	tcsetattr(0,TCSANOW,&newSettings);
	if(nread==1){
		peek_char=ch;
		return 1;
	}
	return 0;
}

int readCh(){
	char ch;
	if(peek_char!=-1){
		ch=peek_char;
		peek_char=-1;
		return ch;
	}
	read(0,&ch,1);
	return ch;
}

void Cursor(int x, int y){
    cout<<"\033["<<x<<";"<<y<<"H";
    fflush(stdout);
}

void setRoot(char const* path){
    root=path;
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&win);
    modeline=win.ws_row-7;
    output_line=modeline+1;
    status_line=output_line+1;
    input_line=status_line+1;
}

string compute_size(long long int size)
{
	
	string temp;
	if(size>1099511627776)
	{
		temp=to_string((size/float(1099511627776)));
		temp=temp+" TB";
	}
	else if(size>1073741824)
	{
		temp=to_string(size/float(1073741824));
		temp=temp+" GB";
	}
	else if(size>1048576)
	{
		temp=to_string(size/float(1048576));
		temp=temp+" MB";
	}
	else if(size>1024)
	{
		temp=to_string((size/float(1024)));
		temp=temp+" KB";
	}
	else
	{
		temp=to_string(size)+" B";
	}

	return temp;
}

void copyall(string source,string destination)
{   
    char* sourceconst=new char[source.length()+1];
    strcpy(sourceconst,source.c_str());
    struct dirent **namelist;
    int total = scandir(sourceconst,&namelist, NULL,alphasort);
    //cout<<source;
    struct stat statObj;
    for(int i=0;i<total;i++)
    {
        string ts=source+"/"+namelist[i]->d_name;
        char* ts1=new char[ts.length()+1];
        strcpy(ts1,ts.c_str());
        if(stat(ts1,&statObj) < 0)   
        {
            cout<<endl<<"No such directory exist"<<endl;
            //return 1;
        } 
        //cout<<"**"<<sourceconst<<"**";
        if(S_ISDIR(statObj.st_mode) && string(namelist[i]->d_name)!="." && string(namelist[i]->d_name)!="..")
        {
            string s=destination+"/"+namelist[i]->d_name;
            char *create_dir_argument=new char[s.length()+1];
            strcpy(create_dir_argument,s.c_str());
            mkdir(create_dir_argument, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IXOTH);

            copyall(source+"/"+namelist[i]->d_name,destination+"/"+namelist[i]->d_name);
        }
        else
        {
            //cout<<"FF"<<namelist[i]->d_name<<"FF";
            if(string(namelist[i]->d_name)!="." && string(namelist[i]->d_name)!="..")
                copyFile1(source+"/"+namelist[i]->d_name,destination+"/"+namelist[i]->d_name);
        }
    } 
}

void ls()
{
	cls();
	string filename;
	char temp[1000];
	char permi[10];
	

	struct stat fileInfo;
	
	// printf("at: %s\n",currentDir);
	for(int fileIt=firstIndex; fileIt<lastIndex && fileIt<files.size(); fileIt++){
        
		lstat(files[fileIt]->d_name,&fileInfo);
		if((S_ISDIR(fileInfo.st_mode))){
			cout<<fileIt+1<<": "<<"\033[1;31m"<<files[fileIt]->d_name<<"\033[0m"<<"\t";
			//Cursor(fileIt, 26);
            cout<<compute_size(fileInfo.st_size)<<"\t";
		}
		else{
			cout<<fileIt+1<<": "<<"\033[1;36m"<<files[fileIt]->d_name<<"\033[0m"<<"\t";
			//Cursor(fileIt, 26);
            cout<<compute_size(fileInfo.st_size)<<"\t";
		}

		//Cursor(fileIt,52);
        
		/*time_t t = fileInfo.st_mtim.tv_sec;				//last modified time
		struct tm lt;
		localtime_r(&t, &lt);
		char time[100];
		strftime(time, sizeof(time), "%c", &lt);
		cout<<time;*/
		struct passwd *psd=getpwuid(fileInfo.st_uid);
        const char *uname=psd->pw_name;
        cout<<uname<<"\t";

        struct group *grp=getgrgid(fileInfo.st_gid);
        const char *gname=grp->gr_name;
        cout<<gname<<"\t";
		
		cout<<compute_size(fileInfo.st_size)<<"\t";
		
		char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
		struct tm *tim=gmtime(&(fileInfo.st_mtime));
		cout<<months[(tim->tm_mon)]<<" "<<tim->tm_mday<<" "<<tim->tm_hour<<":"<<tim->tm_min<<"\t";

		if(fileInfo.st_mode & S_IRUSR)
	    	permi[0]='r';
	    else
	    	permi[0]='-';

	    if(fileInfo.st_mode & S_IWUSR)
	    	permi[1]='w';
	    else
	    	permi[1]='-';
	    
	    if(fileInfo.st_mode & S_IXUSR)
	    	permi[2]='x';
	    else
	    	permi[2]='-';
	    
	    if(fileInfo.st_mode & S_IRGRP)
	    	permi[3]='r';
	    else
	    	permi[3]='-';
	    
	    if(fileInfo.st_mode & S_IWGRP)
	    	permi[4]='w';
	    else
	    	permi[4]='-';
	    
	    if(fileInfo.st_mode & S_IXGRP)
	    	permi[5]='x';
	    else
	    	permi[5]='-';
	    
	    if(fileInfo.st_mode & S_IROTH)
	    	permi[6]='r';
	    else
	    	permi[6]='-';
	    
	    if(fileInfo.st_mode & S_IWOTH)
	    	permi[7]='w';
	    else
	    	permi[7]='-';
	    
	    if(fileInfo.st_mode & S_IXOTH)
			permi[8]='x';
	    else
	    	permi[8]='-';
		permi[9]=0;
	    //printf("%s\n",permi);
	    cout<<permi<<"\t";
		
        cout<<"\n";
	}
	Cursor(modeline,0);
	cout<<"Mode: Normal Mode";
}

void getSetCurrentDir(char const* dir){
	DIR* dp;
	struct dirent* entry;

	if((dp=opendir(dir))==NULL){
		fprintf(stderr, "Can't open the Directory!\n");
		return;
	}
	chdir(dir);
	//getcwd(currentDir,dirSize);
	files.clear();
	while((entry=readdir(dp))!=NULL){
		files.push_back(entry);
	}
	closedir(dp);
	firstIndex=0;
	cursor=lastIndex=min(MAX,int(files.size()));
	ls();
	Cursor(cursor,0);
}
int copyFile1(string source,string destination) // It recursively go till depth for copying all child files/dir..
{
	struct stat fileStat,fileStat1;
    char block[1024];
    int in, out;
    int nread;
    char *copy_argument;
    
    copy_argument=new char[source.length()+1];
    strcpy(copy_argument,source.c_str());
    in = open(copy_argument, O_RDONLY);
    char *paste_argument=new char[destination.length()+1];
    strcpy(paste_argument,destination.c_str());
    out = open(paste_argument, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
    while((nread = read(in,block,sizeof(block))) > 0)
        write(out,block,nread);

    if(stat(copy_argument,&fileStat) < 0) 
    {
        cout<<"Error";
        //return 1;
    }   
    if(stat(paste_argument,&fileStat1) < 0)    
    {
        cout<<"Error";
        //return 1;
    }
    int status=chown(paste_argument, fileStat.st_uid, fileStat.st_gid);
    if(status!=0)
    {
        cout<<"Error";
    }
    status=chmod(paste_argument, fileStat.st_mode);
    if(status!=0)
    {
        cout<<"Error";
    }
	return 0;
}

void scrollUp(){
	if(cursor>1){
		cursor--;
		Cursor(cursor,0);
		return;
	}
	if(firstIndex==0) return;
	firstIndex--;
	lastIndex--;
	ls();
	Cursor(cursor,0);
	return;
}

void scrollDown(){
	if(cursor<files.size() && cursor<MAX){
		cursor++;
		Cursor(cursor,0);
		return;
	}
	if(lastIndex==files.size()) return;
	firstIndex++;
	lastIndex++;
	ls();
	Cursor(cursor,0);
	return;
}
void deleteall(string rm) // It recursively go till depth and delete all files/directories
{
	struct dirent **namelist;
	int i;
	struct stat statObj;
	string path;
	char *rm1=new char[rm.length()+1];
	strcpy(rm1,rm.c_str());
	char *rm2;
	int total = scandir(rm1,&namelist, NULL,alphasort);

	for(i=0;i<total;i++)
	{
		path=rm+"/"+namelist[i]->d_name;
		rm2=new char[path.length()+1];
		strcpy(rm2,path.c_str());
		//stat(rm2,&statObj);
		if(stat(rm2,&statObj) < 0)   
	    {
	        cout<<endl<<"No such directory exist"<<endl;
	        //return 1;
	    } 
		if(S_ISDIR(statObj.st_mode) && string(namelist[i]->d_name)!="." && string(namelist[i]->d_name)!="..")
		{
			//int f=current.find_last_of("/\\");
	    	//current=current.substr(0,f);
			//path=rm+"/"+namelist[i]->d_name;
			deletefilesfolders.push_back(path);
			deleteall(path);
		}
		else
		{
			if(string(namelist[i]->d_name)!="." && string(namelist[i]->d_name)!="..")
				deletefilesfolders.push_back(path);
		}
	}
}
void pagedown(){
	if(lastIndex+MAX<files.size()){
		firstIndex=lastIndex;
		lastIndex=lastIndex+MAX;
		ls();
		Cursor(cursor,0);
		return;
	}
	else{
		firstIndex=firstIndex+files.size()-lastIndex;
		lastIndex=files.size();
		ls();
		Cursor(cursor,0);
		return;
	}
}
void pageup(){
	if(firstIndex-MAX>=0){
		firstIndex=firstIndex-MAX;
        lastIndex = firstIndex+MAX;
		ls();
		Cursor(cursor,0);
		return;
	}
	else{
		firstIndex=0;
		lastIndex=firstIndex+MAX;
		ls();
		Cursor(cursor,0);
		return;
	}
}
void Enter(){
	struct stat fileinfo;
	lstat(files[firstIndex+cursor-1]->d_name,&fileinfo);
	if((S_ISDIR(fileinfo.st_mode))){
	char p[1024];
	size_t s=1024;
	getcwd(p,s);
	string temp(p);
	backStack.push_back(temp);
	string z = files[firstIndex+cursor-1]->d_name;
	char path[1024];
	realpath(z.c_str(),path);
	getSetCurrentDir(path);
	}
    else{
		pid_t pid=fork();
		if(pid==0){
			execl("/usr/bin/xdg-open","xdg-open",files[firstIndex+cursor-1]->d_name,NULL);
		}
	}
}
void goback(){
	if(backStack.empty()){
		Cursor(26,0);
		cout<<"can't go backward!";
		Cursor(cursor,0);
		return;
	}
	char p[1024];
	size_t s=1024;
	getcwd(p,s);
	string temp(p);
	forwardStack.push_back(temp);
	string z=backStack.back();
	backStack.pop_back();
	getSetCurrentDir(z.c_str());
}
void goforward(){
	if(forwardStack.empty()){
		Cursor(26,0);
		cout<<"can't go forward! ";
		Cursor(cursor,0);
		return;
	}
	char p[1024];
	size_t s=1024;
	getcwd(p,s);
	string temp(p);
	backStack.push_back(temp);
	string z=forwardStack.back();
	forwardStack.pop_back();
	getSetCurrentDir(z.c_str());
}

int copyFile(vector<string> &commandSplit)	// It accepts arg from commandFile copy File/directories
{
    string destination=commandSplit[commandSplit.size()-1];
    struct stat statObj;
    for(unsigned i=1;i<commandSplit.size()-1;i++)
    {
        string source=commandSplit[i];
        char *ss=new char[source.length()+1];
        strcpy(ss,source.c_str());
        if(stat(ss,&statObj) < 0)   
        {
            cout<<endl<<"No such directory exist"<<endl;
            //return 1;
        }
        if(S_ISDIR(statObj.st_mode))
        {
            int f=source.find_last_of("/\\");
            string source1=source.substr(f+1,source.length());
            string s=destination+"/"+source1;
            char *create_dir_argument=new char[s.length()+1];
            strcpy(create_dir_argument,s.c_str());
            mkdir(create_dir_argument, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IXOTH);
            string destination1=destination+"/"+source1;
            copyall(source,destination1);
        }
        else
        {
            int f=commandSplit[i].find_last_of("/\\");
            commandSplit[i]=commandSplit[i].substr(f+1,commandSplit[i].length());
            string destination_file=destination+"/"+commandSplit[i];
            copyFile1(source,destination_file);   
        }
    }
    return 0;
}
void Up_one_level(){
	char p[1024];
	size_t s=1024;
	getcwd(p,s);
	string temp(p);
	backStack.push_back(temp);
	char path[1024];
	forwardStack.clear();
	realpath("..",path);
	getSetCurrentDir(path);
}
void Home(){
	char p[1024];
	size_t s=1024;
	getcwd(p,s);
	string temp(p);
	backStack.push_back(temp);
	getSetCurrentDir(root);
}

//**************************Command-Mode********************************************
string home,current; 
vector<string> searchResult;
void clrCMD(){
	Cursor(modeline,0);
	for(int i=modeline; i<win.ws_row; i++){
		for(int j=0; j<win.ws_col; j++){
			cout<<" ";
		}
	}
	Cursor(modeline,0);
	cout<<"Mode: Command Mode";
}
string stringProcess(string fname)  // To process input string for relative/absolute path
{
	if(fname[0]=='.')
	{
		fname=fname.substr(1,fname.length());
		fname=current+fname;
	}
	else if(fname[0]=='~')
	{
		fname=fname.substr(1,fname.length());
		fname=home+fname;
	}
	else if(fname[0]=='/')
	{
		if(fname=="/")
		{
			fname=fname.substr(1,fname.length());
			fname=home+fname;
		}
		else
			fname=home+fname;
	}
	else
	{
		fname=current+"/"+fname;
	}
	//cout<<"***"<<fname<<"***";
	return fname;
}





int createDir(vector<string> &commandSplit)	// It accepts arg from commandFile and create a directory
{
	//struct stat fileStat,fileStat1;
	string destination=commandSplit[commandSplit.size()-1];
	char *create_dir_argument;
    for(unsigned i=1;i<commandSplit.size()-1;i++)
    {
    	string s=destination+"/"+commandSplit[i];
    	create_dir_argument= new char[s.length()+1];
    	strcpy(create_dir_argument,s.c_str());
    	int status= mkdir(create_dir_argument, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IXOTH);
    	if(status!=0)
	    {
	        cout<<endl<<"Invalid path"<<endl;
	    }
    }
    return 0;
}
int createFile(vector<string> &commandSplit)	// It accepts arg from commandFile and create a file
{
	//struct stat fileStat,fileStat1;
	string destination=commandSplit[commandSplit.size()-1];
	char *create_argument;
    for(unsigned i=1;i<commandSplit.size()-1;i++)
    {
    	string s=destination+"/"+commandSplit[i];
    	create_argument= new char[s.length()+1];
    	strcpy(create_argument,s.c_str());
    	int status=open(create_argument, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IXOTH);
    	if(status<0)
        {
    		cout<<endl<<"Invalid Path"<<endl;
        }
    }
    return 0;
}


int deleteFile(vector<string> &commandSplit)	// It accepts arg from commandFile and delete file/files.
{
    char *remove_argument;
    for(unsigned i=1;i<commandSplit.size();i++)
    {
    	remove_argument= new char[commandSplit[i].length()+1];
    	strcpy(remove_argument,commandSplit[i].c_str());
    	int status=remove(remove_argument);
	    if(status!=0)
	    {
	        cout<<endl<<"No such file exists"<<endl;
	    }
    }
    
	return 0;
}
vector<string> findk;
vector<string> search(vector<string> &commandSplit)	// It accepts arg from commandFile and return result.
{
	string key=commandSplit[1];
	string from=commandSplit[2];
	findk.clear();
	
	findA(from,key);
	
	return findk;
}

int rename(vector<string> &commandSplit)	// It accepts arg from command and rename file/dir 
{
	char *old=new char[commandSplit[1].length()+1];
	char *_new=new char[commandSplit[2].length()+1];
	strcpy(old,commandSplit[1].c_str());
	strcpy(_new,commandSplit[2].c_str());
   	
   	int status=rename(old,_new);
   	if(status!=0)
   	{
   		cout<<endl<<"No such file exists"<<endl;
   	}
	return 0;
}

void findA(string source,string key)	// It recursively go till depth and store all matches result in vector.
{
	char *path=new char[source.length()+1];
	strcpy(path,source.c_str());
	struct dirent **namelist;
	struct stat statObj;
	int total=scandir(path,&namelist,NULL,alphasort);
	for(int i=0;i<total;i++)
	{
		string new_path=source+"/"+namelist[i]->d_name;
		char *const_new_path=new char[new_path.length()+1];
		strcpy(const_new_path,new_path.c_str());
		if(stat(const_new_path,&statObj) < 0)   
	    {
	        cout<<endl<<"No such directory exist"<<endl;
	        //return 1;
	    } 
		if(S_ISDIR(statObj.st_mode) && string(namelist[i]->d_name)!="." && string(namelist[i]->d_name)!="..")
		{
			if(string(namelist[i]->d_name)==key)
				findk.push_back(new_path);
			findA(new_path,key);
		}
		else
		{
			if(string(namelist[i]->d_name)!="." && string(namelist[i]->d_name)!="..")
			{
				if(string(namelist[i]->d_name)==key)
					findk.push_back(new_path);
			}
		}
	}
}
int moveFile(vector<string> &commandSplit)	// It accepts arg from commandFile and move file/dir
{
    string destination=commandSplit[commandSplit.size()-1];
    vector<string> v;
	for(unsigned i=1;i<commandSplit.size()-1;i++)
    {
        v.clear();
        v.push_back("copy");
        v.push_back(commandSplit[i]);
        v.push_back(destination);
        copyFile(v);
        struct stat statObj;
        char *ss=new char[commandSplit[i].length()+1];
        strcpy(ss,commandSplit[i].c_str());
        if(stat(ss,&statObj) < 0)   
        {
            cout<<endl<<"No such directory exist"<<endl;
            //return 1;
        } 
        if(S_ISDIR(statObj.st_mode))
        {
            v.clear();
            v.push_back("delete");
            v.push_back(commandSplit[i]);
            vector<string> ans=deleteDir(v);
            if(ans.size()>0)
            {
                //cout<<endl<<endl;
                for(int j=ans.size()-1;j>=0;j--)
                {
                    //cout<<vc[i]<<endl;
                    char *p=new char[ans[j].length()+1];
                    strcpy(p,ans[j].c_str());
                    struct stat statObj;
                    stat(p,&statObj);
                    if(S_ISDIR(statObj.st_mode))
                    {
                        vector<string> v;
                        v.clear();
                        v.push_back("abc");
                        v.push_back(ans[j]);
                        deleteDir(v);
                    }
                    else
                    {
                        vector<string> v;
                        v.clear();
                        v.push_back("abc");
                        v.push_back(ans[j]);
                        deleteFile(v);
                    }
                }
            }
        }
        else
        {
            v.clear();
            v.push_back("delete");
            v.push_back(commandSplit[i]);
            deleteFile(v);
        }
    }
    return 0;
}

string commandMode(int row,string currentPath,string root) // For commandMode utility
{
	clrCMD();
	string command="";
	home=root;
	current=currentPath;
	int col=0;
	char ch;
	vector<string> commandSplit;
	string s="";
	Cursor(input_line,col);
	do
	{
		command="";
		while((ch=cin.get())!=27 && (ch!=10))
		{
			cout<<ch;
			if(ch==127)
			{
				if(command.length()<=1)
				{
					command="";
					Cursor(row,col);
					cout<<command;
				}
				else
				{
					command=command.substr(0,command.length()-1);
					Cursor(row,col);
					cout<<command;
				}
			}
			else
				command+=ch;
		}
		if(ch==27)
			return "";
		else
		{
			unsigned int i=0;
			int flag=0;
			while(i<command.length())
			{
				flag=0;
				if(command[i]=='\\')
				{
					s=s+command[i+1];
					i++;
				}
				else if(command[i]==32 && commandSplit.size()==0)
				{
					//s=stringProcess(s);
					commandSplit.push_back(s);
					s="";
				}
				else if(command[i]==32 && commandSplit.size()>=1)
				{
					if(s!="" && s!=" " && (commandSplit[0]=="search" || commandSplit[0]=="create_file" || commandSplit[0]=="create_dir"))
					{
						commandSplit.push_back(s);
					}
					else if(s!="" && s!=" ")
					{
						s=stringProcess(s);
						commandSplit.push_back(s);
					}
					s="";
				}
				else
				{
					s=s+command[i];
					flag=1;
				}
				i++;
			}
			if(commandSplit.size()==0)
			{
				//cout<<endl<<"Invalid Command"<<endl;
				cout<<":";
				commandSplit.clear();
				s="";
				continue;
			}
			else if(flag==1 && commandSplit[0]!="search" && commandSplit[0]!="create_dir" && commandSplit[0]!="create_file")
			{
				s=stringProcess(s);
				commandSplit.push_back(s);
			}
			else if(flag==1)
			{
				commandSplit.push_back(s);
			}
			if(commandSplit[0]=="copy")
			{
				if(commandSplit.size()>=3)
				{
					copyFile(commandSplit);
					Cursor(row,col);
					clrCMD();
					Cursor(row,col);
				}
				else
					cout<<endl<<"Argument Missing"<<endl<<":";
			}
			else if(commandSplit[0]=="move")
			{
				if(commandSplit.size()>=3)
				{
					moveFile(commandSplit);
					Cursor(row,col);
					clrCMD();
					Cursor(row,col);
				}
				else
					cout<<endl<<"Argument Missing"<<endl<<":";
			}
			else if(commandSplit[0]=="rename")
			{
				if(commandSplit.size()==3)
				{
					rename(commandSplit);
					Cursor(row,col);
					clrCMD();
					Cursor(row,col);
				}
				else
					cout<<endl<<"Argument Missing"<<endl<<":";
			}
			else if(commandSplit[0]=="create_file")
			{
				if(commandSplit.size()>=3)
				{
					s=stringProcess(commandSplit[commandSplit.size()-1]);
					commandSplit.pop_back();
					commandSplit.push_back(s);
					createFile(commandSplit);
					Cursor(row,col);
					clrCMD();
					Cursor(row,col);
				}
				else
					cout<<endl<<"Argument Missing"<<endl<<":";
			}
			else if(commandSplit[0]=="create_dir")
			{
				if(commandSplit.size()>=3)
				{
					s=stringProcess(commandSplit[commandSplit.size()-1]);
					commandSplit.pop_back();
					commandSplit.push_back(s);
					createDir(commandSplit);
					Cursor(row,col);
					clrCMD();
					Cursor(row,col);
				}
				else
					cout<<endl<<"Argument Missing"<<endl<<":";
			}
			else if(commandSplit[0]=="delete_file")
			{
				if(commandSplit.size()>=2)
				{
					deleteFile(commandSplit);
					Cursor(row,col);
					clrCMD();
					Cursor(row,col);
				}
				else
					cout<<endl<<"Argument Missing"<<endl<<":";	
			}
			else if(commandSplit[0]=="delete_dir")
			{
				if(commandSplit.size()>=2)
				{
					vector<string> vc=deleteDir(commandSplit);
					if(vc.size()>0)
					{
						//cout<<endl<<endl;
						for(int i=vc.size()-1;i>=0;i--)
					    {
					    	//cout<<vc[i]<<endl;
					    	char *p=new char[vc[i].length()+1];
					    	strcpy(p,vc[i].c_str());
					    	struct stat statObj;
					    	stat(p,&statObj);
							if(S_ISDIR(statObj.st_mode))
							{
								vector<string> v;
								v.clear();
								v.push_back("abc");
								v.push_back(vc[i]);
								deleteDir(v);
							}
							else
							{
								vector<string> v;
								v.clear();
								v.push_back("abc");
								v.push_back(vc[i]);
								deleteFile(v);
							}
					    }
					}
					Cursor(row,col);
				}
				else
					cout<<endl<<"Argument Missing"<<endl<<":";
			}
			else if(commandSplit[0]=="goto")
			{
				if(commandSplit.size()==2)
				{
					string path=commandSplit[1];
					DIR *d;
					char *temp2=new char[path.length()+1];
					strcpy(temp2,path.c_str());
					d=opendir(temp2);
					if(!d)
					{
						cout<<endl<<"Invalid path"<<endl<<":";
					}
					else
						return path;
				}
				else
					cout<<endl<<"Argument Missing"<<endl<<":";
			}
			else if(commandSplit[0]=="search")
			{
				if(commandSplit.size()==2)
				{
					commandSplit.push_back(current);
					vector<string> ans=search(commandSplit);
					if(ans.size()>0)
					{
						searchResult=ans;
						Cursor(output_line,col);
						cout<<"True";
						Cursor(input_line,col);
						string s="$$";
						return s;
					}
					else
					{
					    Cursor(output_line,col);
						cout<<"False";
						Cursor(input_line,col);	
					}
				}
				else
					cout<<endl<<"Argument Missing"<<endl<<":";
			}
									
			else
			{
				cout<<endl<<"Invalid Command"<<endl;
				cout<<":";
			}
		}
		commandSplit.clear();
		s="";
  	}while(true);
}

int main(){
    cls();
    char path[1024];
	char curr[1024];
    size_t size=1024;
    getcwd(path,size);
    setRoot(path);
    getSetCurrentDir(path);
    initKeyboard();
	//string curr,r;
    char ch='a';
	while(ch!='q'){
		if(kbHit()){
			ch=readCh();		// take a character as user input and respond instantly
			switch(ch){
				case 65:scrollUp();		// scroll up on pressing up arrow
					break;
				case 66:scrollDown();	// scroll down on down arrow
					break;
				case 107:pagedown();
				    break;
				case 108:pageup();
				    break;
				case 10:Enter();
				    break;
				case 68:goback();
				    break;
				case 67:goforward();
					break;
				case 127:Up_one_level();
				    break;
				case 104:Home();
				    break;
				case ':':
					closeKeyboard();
					getcwd(curr,size);
					commandMode(input_line,curr,root);
					getSetCurrentDir(curr); 
					initKeyboard();
					break;
				default:
					break;
			}
		}
	}
	closeKeyboard();	// restore terminal settings
	cls();
	exit(0);
}