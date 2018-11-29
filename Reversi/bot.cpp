#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include "jsoncpp/json.h" // C++����ʱĬ�ϰ����˿�
 
 using namespace std;
 
 int AI,x,y; // ����ִ����ɫ��1Ϊ�ڣ�-1Ϊ�ף�����״̬��ͬ��
 int qi[8][8] = { 0 }; // ��x��y����¼����״̬
 int blackPieceCount = 2, whitePieceCount = 2;
 bool acc[8][8],saveacc[8][8],haveputs,corner;
 int saveqi[8][8],value[8][8]={0};
 int resultx, resulty;

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
				        res+=5;
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
				value[i][j]=1;
				value[i][j]=value[i][j]+put_qi(i,j,AI);
				if(i!=1&&i!=6&&j!=1&&j!=6)
				     value[i][j]=value[i][j]+5;					
				if(put_where(-AI))
				{					
				    if(acc[0][0]||acc[7][0]||acc[0][7]||acc[7][7])
				        value[i][j]=1;	
				}							
				else
				{
				    value[i][j]=value[i][j]+10;
				    if(i==0||i==7||j==0||j==7)
				        value[i][j]=value[i][j]+10;
				    AI_put();
				    if(corner)
				    {
						resultx=i;
						resulty=j;
						return;
					}									            				        
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
 // ��Direction����Ķ����꣬�������Ƿ�Խ��
 
 inline bool MoveStep(int &x, int &y, int Direction)
 {
 	if (Direction == 0 || Direction == 6 || Direction == 7)
 		x++;
 	if (Direction == 0 || Direction == 1 || Direction == 2)
 		y++;
 	if (Direction == 2 || Direction == 3 || Direction == 4)
 		x--;
 	if (Direction == 4 || Direction == 5 || Direction == 6)
 		y--;
 	if (x < 0 || x > 7 || y < 0 || y > 7)
 		return false;
 	return true;
 }
 
 // �����괦���ӣ�����Ƿ�Ϸ���ģ������
 bool ProcStep(int xPos, int yPos, int color, bool checkOnly = false)
 {
 	int effectivePoints[8][2];
 	int dir, x, y, currCount;
 	bool isValidMove = false;
 	if (qi[xPos][yPos] != 0)
 		return false;
 	for (dir = 0; dir < 8; dir++)
 	{
 		x = xPos;
 		y = yPos;
 		currCount = 0;
 		while (1)
 		{
 			if (!MoveStep(x, y, dir))
 			{
 				currCount = 0;
 				break;
 			}
 			if (qi[x][y] == -color)
 			{
 				currCount++;
 				effectivePoints[currCount][0] = x;
 				effectivePoints[currCount][1] = y;
 			}
 			else if (qi[x][y] == 0)
 			{
 				currCount = 0;
 				break;
 			}
 			else
 			{
 				break;
 			}
 		}
 		if (currCount != 0)
 		{
 			isValidMove = true;
 			if (checkOnly)
 				return true;
 			if (color == 1)
 			{
 				blackPieceCount += currCount;
 				whitePieceCount -= currCount;
 			}
 			else
 			{
 				whitePieceCount += currCount;
 				blackPieceCount -= currCount;
 			}
 			while (currCount > 0)
 			{
 				x = effectivePoints[currCount][0];
 				y = effectivePoints[currCount][1];
 				qi[x][y] *= -1;
 				currCount--;
 			}
 		}
 	}
 	if (isValidMove)
 	{
 		qi[xPos][yPos] = color;
 		if (color == 1)
 			blackPieceCount++;
 		else
 			whitePieceCount++;
 		return true;
 	}
 	else
 		return false;
 }
 
 // ���color�����޺Ϸ��岽
 bool CheckIfHasValidMove(int color)
 {
 	int x, y;
 	for (y = 0; y < 8; y++)
 	for (x = 0; x < 8; x++)
 	if (ProcStep(x, y, color, true))
 		return true;
 	return false;
 }
 
 int main()
 {
 	int x0, y0;
 
 	// ��ʼ������
 	qi[3][4] = qi[4][3] = 1;  //|��|��|
 	qi[3][3] = qi[4][4] = -1; //|��|��|
 
 	// ����JSON
 	string str;
 	getline(cin, str);
 	Json::Reader reader;
 	Json::Value input;
 	reader.parse(str, input);
 
 	// �����Լ��յ���������Լ���������������ָ�״̬
 	int turnID = input["responses"].size();
 	AI = input["requests"][(Json::Value::UInt) 0]["x"].asInt() < 0 ? 1 : -1; // ��һ�غ��յ�������-1, -1��˵�����Ǻڷ�
 	for (int i = 0; i < turnID; i++)
 	{
 		// ������Щ��������𽥻ָ�״̬����ǰ�غ�
 		x0 = input["requests"][i]["x"].asInt();
 		y0 = input["requests"][i]["y"].asInt();
 		if (x >= 0)
 			ProcStep(x0, y0, -AI); // ģ��Է�����
 		x0 = input["responses"][i]["x"].asInt();
 		y0 = input["responses"][i]["y"].asInt();
 		if (x >= 0)
 			ProcStep(x0, y0, AI); // ģ�⼺������
 	}
 
 	// �����Լ����غ�����
 	x0 = input["requests"][turnID]["x"].asInt();
 	y0 = input["requests"][turnID]["y"].asInt();
 	if (x >= 0)
 		ProcStep(x0, y0, -AI); // ģ��Է�����
 
 	// �ҳ��Ϸ����ӵ�
 	int possiblePos[64][2], posCount = 0, choice;
 
 	for (y = 0; y < 8; y++)
 	for (x = 0; x < 8; x++)
 	if (ProcStep(x, y, AI, true))
 	{
 		possiblePos[posCount][0] = x;
 		possiblePos[posCount++][1] = y;
 	}
 
 	// �������ߣ���ֻ���޸����²��֣�
 
 	if (posCount > 0)
 	{
 		put_where(AI);
 		AI_put();
 	}
 	else
 	{
 		resultx = -1;
 		resulty = -1;
 	}
 
 	// ���߽���������������ֻ���޸����ϲ��֣�
 
 	Json::Value ret;
 	ret["response"]["x"] = resultx;
 	ret["response"]["y"] = resulty;
 	Json::FastWriter writer;
 	cout << writer.write(ret) << endl;
 	return 0;
 }
