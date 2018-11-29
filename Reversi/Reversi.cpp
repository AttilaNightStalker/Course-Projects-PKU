#include<stdio.h>
#include<iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include "jsoncpp/json.h" 
using namespace std;

bool acc[8][8],saveacc[8][8],savevalue[8][8],haveputs,corner;
int qi[8][8]={0},saveqi[8][8],value[8][8]={0};
int resultx,resulty,x,y,AI;

int cor(int i)
{
	if(i==1||i==0)
	    return 0;
	else
	    return 7;
}

void move(int dir)
{
	if(dir==0||dir==1||dir==2)
	    x--;
	if(dir==4||dir==5||dir==6)
	    x++;
	if(dir==6||dir==7||dir==0)
	    y--;
	if(dir==2||dir==3||dir==4)
	    y++;
}

bool wrongput(int qx,int qy)
{
	if(qx<0||qx>7||qy<0||qy>7)
	    return true;
	return false;
}

bool accesbl(int xpos,int ypos,int color)
{
	int opcolor=-color; 
	if(qi[xpos][ypos]!=0)
	    return false;
	for(int i=0;i<8;i++)
	{
		x=xpos;y=ypos;
		move(i);
		if(wrongput(x,y)||qi[x][y]!=opcolor)
		    continue;
		while(1)
		{
		    move(i);
			if(qi[x][y]==0||wrongput(x,y))
			    break;
			if(qi[x][y]==color)
			    return true;			
		}
	}
	return false;
}

bool put_where(int color)
{
	bool puts=false;
	for(int i=0;i<8;i++)
	    for(int j=0;j<8;j++)
	    {
		    acc[i][j]=accesbl(i,j,color);
		    if(acc[i][j]==true)
		        puts=true;
		}
	return puts;        	        
}

int put_qi(int qx,int qy,int color)
{
    int res=0;
	int opcolor=-color;	
    qi[qx][qy]=color; 
	for(int i=0;i<8;i++)
	{
	    x=qx;y=qy;
		move(i);
		if(wrongput(x,y)||qi[x][y]!=opcolor)
		    continue;
		while(1)
		{
			move(i);
			if(qi[x][y]==0||wrongput(x,y))
			    break;
			if(qi[x][y]==color)
			{
				x=qx;y=qy;
				move(i);
				while(qi[x][y]==opcolor)
				{
				    res++;
                    if(x==0||x==7||y==0||y==7)
                        res+=2;
					qi[x][y]=color;
					move(i);					
				}
				break;
			}
		}
	}
	return res;
}

void backup()
{
	for(int i=0;i<8;i++)
	    for(int j=0;j<8;j++)
	       saveqi[i][j]=qi[i][j];
	for(int i=0;i<8;i++)
	    for(int j=0;j<8;j++)
	       saveacc[i][j]=acc[i][j];
}

void back()
{
	for(int i=0;i<8;i++)
	    for(int j=0;j<8;j++)
	       qi[i][j]=saveqi[i][j];
	for(int i=0;i<8;i++)
	    for(int j=0;j<8;j++)
	       acc[i][j]=saveacc[i][j];
}

void backup_value()
{
	for(int i=0;i<8;i++)
	    for(int j=0;j<8;j++)
	       savevalue[i][j]=value[i][j];
}

void back_value()
{
	for(int i=0;i<8;i++)
	    for(int j=0;j<8;j++)
	       value[i][j]=savevalue[i][j];
}

void AI_put()
{
	corner=false;
	for(int i=0;i<8;i=i+7)
	    for(int j=0;j<8;j=j+7)
	        if(acc[i][j])
	        {
	        	resultx=i;
	        	resulty=j;
	        	corner=true;
	        	return;
			}			
	backup();
	for(int i=0;i<8;i++)
	    for(int j=0;j<8;j++)
		{		    
			if(acc[i][j])
			{
				value[i][j]=10;
				value[i][j]=value[i][j]+put_qi(i,j,AI);
				if((i==1||i==6)&&(j==1||j==6)&&qi[cor(i)][cor(j)]==0)
				    value[i][j]=value[i][j]/2;
				if(i==0||i==7)
				{
					if((j==1||j==6)&&qi[cor(i)][cor(j)]==0)
					    value[i][j]=value[i][j]/2;
					else
					    value[i][j]=value[i][j]*2;
				}
				if(j==0||j==7)
				{
					if(i==1||i==6&&qi[cor(i)][cor(j)]==0)
					    value[i][j]=value[i][j]/2;
					else
					    value[i][j]=value[i][j]*2;
				}						
				if(put_where(-AI))
				{					
				    if(acc[0][0]||acc[7][0]||acc[0][7]||acc[7][7])
				        value[i][j]=1;	
				}							
				else
				{
				    value[i][j]=value[i][j]*2;
				    backup_value();
				    AI_put();
				    if(corner)
				    {
						resultx=i;
						resulty=j;
						return;
					}
					back_value();									            				        
				}										
				back();				    				
			}
		}
	int maxvalue=0;
	for(int i=0;i<8;i++)
	    for(int j=0;j<8;j++)   
		    if(value[i][j]>maxvalue) 
		        maxvalue=value[i][j];
	for(int i=0;i<8;i++)
	    for(int j=0;j<8;j++)   
		    if(value[i][j]==maxvalue) 
		    {
		    	resultx=i;
		    	resulty=j;
		    	return;
			}
}

int main()
{
	int x0,y0;
	qi[3][4]=qi[4][3]=1;
	qi[3][3]=qi[4][4]=-1;
	
	string str;
 	getline(cin, str);
 	Json::Reader reader;
 	Json::Value input;
 	reader.parse(str, input);
 	
 	int turnID=input["responses"].size();
 	AI=input["requests"][(Json::Value::UInt)0]["x"].asInt()<0?1:-1;
 	
 	for(int i=0;i<turnID;i++)
 	{
 
 		x0=input["requests"][i]["x"].asInt();
 		y0=input["requests"][i]["y"].asInt();
 		if(x0>=0)
 			put_qi(x0,y0,-AI); // 模拟对方落子
 		x0=input["responses"][i]["x"].asInt();
 		y0=input["responses"][i]["y"].asInt();
 		if (x0>=0)
 			put_qi(x0,y0,AI); // 模拟己方落子
 	}
 	
 	x0=input["requests"][turnID]["x"].asInt();
 	y0=input["requests"][turnID]["y"].asInt();
 	if (x0>=0)
 		put_qi(x0,y0,-AI); // 模拟对方落子
 		
 	if(!put_where(AI))
    {
 		resultx=-1;
 		resulty=-1;
	}
	else
		AI_put();	
	Json::Value ret;
 	ret["response"]["x"]=resultx;
 	ret["response"]["y"]=resulty;
 	Json::FastWriter writer;
 	cout<<writer.write(ret)<<endl;
	return 0;
}





















