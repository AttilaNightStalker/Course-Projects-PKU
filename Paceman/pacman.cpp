/*
 * Pacman ��������
 * ���ߣ�zhouhy
 * ʱ�䣺2016/3/22 15:32:51
 * �����£�2016/4/24 17:18
 * ����������2��
 * ȡ���˶Զ��Ӳ��������Ϸ��ƶ����жϣ��� ActionValid ������
 * ���������ݡ�
 * �޸����ļ�������ʱ����ͨ������̨�����Bug����
 * �޸ĵĲ�λ��������fstream�⡢ReadInput�ĺ�������ǰ���з����˱仯����ʹ��freopen�ˡ�
 *
 * ������������
 *  r/R/y/Y��Row���У�������
 *  c/C/x/X��Column���У�������
 *  ������±궼��[y][x]��[r][c]��˳��
 *  ��ұ��0123
 *
 * ������ϵ��
 *   0 1 2 3 4 5 6 7 8
 * 0 +----------------> x
 * 1 |
 * 2 |
 * 3 |
 * 4 |
 * 5 |
 * 6 |
 * 7 |
 * 8 |
 *   v y
 *
 * ����ʾ�������ʹ��
 * #ifndef _BOTZONE_ONLINE
 * ������Ԥ����ָ����������������ͱ�������
 *
 * ����ʾ��һ����ı��༭������֧�ֽ�������۵�����
 * ���������Դ�����̫���߳������Կ��ǽ�����namespace�۵�
 */
 
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <stack>
#include <stdexcept>
#include "jsoncpp/json.h"
 
#define FIELD_MAX_HEIGHT 20
#define FIELD_MAX_WIDTH 20
#define MAX_GENERATOR_COUNT 4 // ÿ������1
#define MAX_PLAYER_COUNT 4
#define MAX_TURN 100
 
// ��Ҳ����ѡ�� using namespace std; ���ǻ���Ⱦ�����ռ�
using std::string;
using std::swap;
using std::cin;
using std::cout;
using std::endl;
using std::getline;
using std::runtime_error;
 
// ƽ̨�ṩ�ĳԶ�������߼��������
namespace Pacman
{
	const time_t seed = time(0);
	const int dx[] = { 0, 1, 0, -1, 1, 1, -1, -1 }, dy[] = { -1, 0, 1, 0, -1, 1, 1, -1 };
 
	// ö�ٶ��壻ʹ��ö����Ȼ���˷ѿռ䣨sizeof(GridContentType) == 4�������Ǽ��������32λ������Ч�ʸ���
 
	// ÿ�����ӿ��ܱ仯�����ݣ�����á����߼��������
	enum GridContentType
	{
		empty = 0, // ��ʵ�����õ�
		player1 = 1, // 1�����
		player2 = 2, // 2�����
		player3 = 4, // 3�����
		player4 = 8, // 4�����
		playerMask = 1 | 2 | 4 | 8, // ���ڼ����û����ҵ�
		smallFruit = 16, // С����
		largeFruit = 32 // ����
	};
 
	// �����ID��ȡ��������ҵĶ�����λ
	GridContentType playerID2Mask[] = { player1, player2, player3, player4 };
	string playerID2str[] = { "0", "1", "2", "3" };
 
	// ��ö��Ҳ��������Щ�����ˣ����ӻ�������
	template<typename T>
	inline T operator |=(T &a, const T &b)
	{
		return a = static_cast<T>(static_cast<int>(a) | static_cast<int>(b));
	}
	template<typename T>
	inline T operator |(const T &a, const T &b)
	{
		return static_cast<T>(static_cast<int>(a) | static_cast<int>(b));
	}
	template<typename T>
	inline T operator &=(T &a, const T &b)
	{
		return a = static_cast<T>(static_cast<int>(a) & static_cast<int>(b));
	}
	template<typename T>
	inline T operator &(const T &a, const T &b)
	{
		return static_cast<T>(static_cast<int>(a) & static_cast<int>(b));
	}
	template<typename T>
	inline T operator ++(T &a)
	{
		return a = static_cast<T>(static_cast<int>(a) + 1);
	}
	template<typename T>
	inline T operator ~(const T &a)
	{
		return static_cast<T>(~static_cast<int>(a));
	}
 
	// ÿ�����ӹ̶��Ķ���������á����߼��������
	enum GridStaticType
	{
		emptyWall = 0, // ��ʵ�����õ�
		wallNorth = 1, // ��ǽ����������ٵķ���
		wallEast = 2, // ��ǽ�����������ӵķ���
		wallSouth = 4, // ��ǽ�����������ӵķ���
		wallWest = 8, // ��ǽ����������ٵķ���
		generator = 16 // ���Ӳ�����
	};
 
	// ���ƶ�����ȡ����������赲�ŵ�ǽ�Ķ�����λ
	GridStaticType direction2OpposingWall[] = { wallNorth, wallEast, wallSouth, wallWest };
 
	// ���򣬿��Դ���dx��dy���飬ͬʱҲ������Ϊ��ҵĶ���
	enum Direction
	{
		stay = -1,
		up = 0,
		right = 1,
		down = 2,
		left = 3,
		// ������⼸��ֻ��Ϊ�˲��������򷽱㣬����ʵ���õ�
		ur = 4, // ����
		dr = 5, // ����
		dl = 6, // ����
		ul = 7 // ����
	};
 
	// �����ϴ�����������
	struct FieldProp
	{
		int row, col;
	};
 
	// �����ϵ����
	struct Player : FieldProp
	{
		int strength;
		int powerUpLeft;
		bool dead;
	};
 
	// �غ��²����Ķ��ӵ�����
	struct NewFruits
	{
		FieldProp newFruits[MAX_GENERATOR_COUNT * 8];
		int newFruitCount;
	} newFruits[MAX_TURN];
	int newFruitsCount = 0;
 
	// ״̬ת�Ƽ�¼�ṹ
	struct TurnStateTransfer
	{
		enum StatusChange // �����
		{
			none = 0,
			ateSmall = 1,
			ateLarge = 2,
			powerUpCancel = 4,
			die = 8,
			error = 16
		};
 
		// ���ѡ���Ķ���
		Direction actions[MAX_PLAYER_COUNT];
 
		// �˻غϸ���ҵ�״̬�仯
		StatusChange change[MAX_PLAYER_COUNT];
 
		// �˻غϸ���ҵ������仯
		int strengthDelta[MAX_PLAYER_COUNT];
	};
 
	// ��Ϸ��Ҫ�߼������࣬��������������غ����㡢״̬ת�ƣ�ȫ��Ψһ
	class GameField
	{
	private:
		// Ϊ�˷��㣬��������Զ�����private��
 
		// ��¼ÿ�غϵı仯��ջ��
		TurnStateTransfer backtrack[MAX_TURN];
 
		// ��������Ƿ��Ѿ�����
		static bool constructed;
 
	public:
		// ���صĳ��Ϳ�
		int height, width;
		int generatorCount;
		int GENERATOR_INTERVAL, LARGE_FRUIT_DURATION, LARGE_FRUIT_ENHANCEMENT;
 
		// ���ظ��ӹ̶�������
		GridStaticType fieldStatic[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
 
		// ���ظ��ӻ�仯������
		GridContentType fieldContent[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
		int generatorTurnLeft; // ���ٻغϺ��������
		int aliveCount; // �ж�����Ҵ��
		int smallFruitCount;
		int turnID;
		FieldProp generators[MAX_GENERATOR_COUNT]; // ����Щ���Ӳ�����
		Player players[MAX_PLAYER_COUNT]; // ����Щ���
 
		// ���ѡ���Ķ���
		Direction actions[MAX_PLAYER_COUNT];
 
		// �ָ����ϴγ���״̬������һ·�ָ����ʼ��
		// �ָ�ʧ�ܣ�û��״̬�ɻָ�������false
		bool PopState()
		{
			if (turnID <= 0)
				return false;
 
			const TurnStateTransfer &bt = backtrack[--turnID];
			int i, _;
 
			// �������ָ�״̬
 
			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player &_p = players[_];
				GridContentType &content = fieldContent[_p.row][_p.col];
				TurnStateTransfer::StatusChange change = bt.change[_];
 
				if (!_p.dead)
				{
					// 5. �󶹻غϻָ�
					if (_p.powerUpLeft || change & TurnStateTransfer::powerUpCancel)
						_p.powerUpLeft++;
 
					// 4. �³�����
					if (change & TurnStateTransfer::ateSmall)
					{
						content |= smallFruit;
						smallFruitCount++;
					}
					else if (change & TurnStateTransfer::ateLarge)
					{
						content |= largeFruit;
						_p.powerUpLeft -= LARGE_FRUIT_DURATION;
					}
				}
 
				// 2. �������
				if (change & TurnStateTransfer::die)
				{
					_p.dead = false;
					aliveCount++;
					content |= playerID2Mask[_];
				}
 
				// 1. ���λ�Ӱ
				if (!_p.dead && bt.actions[_] != stay)
				{
					fieldContent[_p.row][_p.col] &= ~playerID2Mask[_];
					_p.row = (_p.row - dy[bt.actions[_]] + height) % height;
					_p.col = (_p.col - dx[bt.actions[_]] + width) % width;
					fieldContent[_p.row][_p.col] |= playerID2Mask[_];
				}
 
				// 0. ���겻�Ϸ������
				if (change & TurnStateTransfer::error)
				{
					_p.dead = false;
					aliveCount++;
					content |= playerID2Mask[_];
				}
 
				// *. �ָ�����
				if (!_p.dead)
					_p.strength -= bt.strengthDelta[_];
			}
 
			// 3. �ջض���
			if (generatorTurnLeft == GENERATOR_INTERVAL)
			{
				generatorTurnLeft = 1;
				NewFruits &fruits = newFruits[--newFruitsCount];
				for (i = 0; i < fruits.newFruitCount; i++)
				{
					fieldContent[fruits.newFruits[i].row][fruits.newFruits[i].col] &= ~smallFruit;
					smallFruitCount--;
				}
			}
			else
				generatorTurnLeft++;
 
			return true;
		}
 
		// �ж�ָ�������ָ�������ƶ��ǲ��ǺϷ��ģ�û��ײǽ��
		inline bool ActionValid(int playerID, Direction &dir) const
		{
			if (dir == stay)
				return true;
			const Player &p = players[playerID];
			const GridStaticType &s = fieldStatic[p.row][p.col];
			return dir >= -1 && dir < 4 && !(s & direction2OpposingWall[dir]);
		}
 
		// ����actionsд����Ҷ�����������һ�غϾ��棬����¼֮ǰ���еĳ���״̬���ɹ��պ�ָ���
		// ���վֵĻ��ͷ���false
		bool NextTurn()
		{
			int _, i, j;
 
			TurnStateTransfer &bt = backtrack[turnID];
			memset(&bt, 0, sizeof(bt));
 
			// 0. ɱ�����Ϸ�����
			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player &p = players[_];
				if (!p.dead)
				{
					Direction &action = actions[_];
					if (action == stay)
						continue;
 
					if (!ActionValid(_, action))
					{
						bt.strengthDelta[_] += -p.strength;
						bt.change[_] = TurnStateTransfer::error;
						fieldContent[p.row][p.col] &= ~playerID2Mask[_];
						p.strength = 0;
						p.dead = true;
						aliveCount--;
					}
					else
					{
						// �������Լ�ǿ��׳������ǲ���ǰ����
						GridContentType target = fieldContent
							[(p.row + dy[action] + height) % height]
							[(p.col + dx[action] + width) % width];
						if (target & playerMask)
							for (i = 0; i < MAX_PLAYER_COUNT; i++)
								if (target & playerID2Mask[i] && players[i].strength > p.strength)
									action = stay;
					}
				}
			}
 
			// 1. λ�ñ仯
			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player &_p = players[_];
				if (_p.dead)
					continue;
 
				bt.actions[_] = actions[_];
 
				if (actions[_] == stay)
					continue;
 
				// �ƶ�
				fieldContent[_p.row][_p.col] &= ~playerID2Mask[_];
				_p.row = (_p.row + dy[actions[_]] + height) % height;
				_p.col = (_p.col + dx[actions[_]] + width) % width;
				fieldContent[_p.row][_p.col] |= playerID2Mask[_];
			}
 
			// 2. ��һ�Ź
			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player &_p = players[_];
				if (_p.dead)
					continue;
 
				// �ж��Ƿ��������һ��
				int player, containedCount = 0;
				int containedPlayers[MAX_PLAYER_COUNT];
				for (player = 0; player < MAX_PLAYER_COUNT; player++)
					if (fieldContent[_p.row][_p.col] & playerID2Mask[player])
						containedPlayers[containedCount++] = player;
 
				if (containedCount > 1)
				{
					// NAIVE
					for (i = 0; i < containedCount; i++)
						for (j = 0; j < containedCount - i - 1; j++)
							if (players[containedPlayers[j]].strength < players[containedPlayers[j + 1]].strength)
								swap(containedPlayers[j], containedPlayers[j + 1]);
 
					int begin;
					for (begin = 1; begin < containedCount; begin++)
						if (players[containedPlayers[begin - 1]].strength > players[containedPlayers[begin]].strength)
							break;
 
					// ��Щ��ҽ��ᱻɱ��
					int lootedStrength = 0;
					for (i = begin; i < containedCount; i++)
					{
						int id = containedPlayers[i];
						Player &p = players[id];
 
						// �Ӹ���������
						fieldContent[p.row][p.col] &= ~playerID2Mask[id];
						p.dead = true;
						int drop = p.strength / 2;
						bt.strengthDelta[id] += -drop;
						bt.change[id] |= TurnStateTransfer::die;
						lootedStrength += drop;
						p.strength -= drop;
						aliveCount--;
					}
 
					// ������������
					int inc = lootedStrength / begin;
					for (i = 0; i < begin; i++)
					{
						int id = containedPlayers[i];
						Player &p = players[id];
						bt.strengthDelta[id] += inc;
						p.strength += inc;
					}
				}
			}
 
			// 3. ��������
			if (--generatorTurnLeft == 0)
			{
				generatorTurnLeft = GENERATOR_INTERVAL;
				NewFruits &fruits = newFruits[newFruitsCount++];
				fruits.newFruitCount = 0;
				for (i = 0; i < generatorCount; i++)
					for (Direction d = up; d < 8; ++d)
					{
						// ȡ�࣬�������ر߽�
						int r = (generators[i].row + dy[d] + height) % height, c = (generators[i].col + dx[d] + width) % width;
						if (fieldStatic[r][c] & generator || fieldContent[r][c] & (smallFruit | largeFruit))
							continue;
						fieldContent[r][c] |= smallFruit;
						fruits.newFruits[fruits.newFruitCount].row = r;
						fruits.newFruits[fruits.newFruitCount++].col = c;
						smallFruitCount++;
					}
			}
 
			// 4. �Ե�����
			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player &_p = players[_];
				if (_p.dead)
					continue;
 
				GridContentType &content = fieldContent[_p.row][_p.col];
 
				// ֻ���ڸ�����ֻ���Լ���ʱ����ܳԵ�����
				if (content & playerMask & ~playerID2Mask[_])
					continue;
 
				if (content & smallFruit)
				{
					content &= ~smallFruit;
					_p.strength++;
					bt.strengthDelta[_]++;
					smallFruitCount--;
					bt.change[_] |= TurnStateTransfer::ateSmall;
				}
				else if (content & largeFruit)
				{
					content &= ~largeFruit;
					if (_p.powerUpLeft == 0)
					{
						_p.strength += LARGE_FRUIT_ENHANCEMENT;
						bt.strengthDelta[_] += LARGE_FRUIT_ENHANCEMENT;
					}
					_p.powerUpLeft += LARGE_FRUIT_DURATION;
					bt.change[_] |= TurnStateTransfer::ateLarge;
				}
			}
 
			// 5. �󶹻غϼ���
			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player &_p = players[_];
				if (_p.dead)
					continue;
 
				if (_p.powerUpLeft > 0 && --_p.powerUpLeft == 0)
				{
					_p.strength -= LARGE_FRUIT_ENHANCEMENT;
					bt.change[_] |= TurnStateTransfer::powerUpCancel;
					bt.strengthDelta[_] += -LARGE_FRUIT_ENHANCEMENT;
				}
			}
 
			++turnID;
 
			// �Ƿ�ֻʣһ�ˣ�
			if (aliveCount <= 1)
			{
				for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
					if (!players[_].dead)
					{
						bt.strengthDelta[_] += smallFruitCount;
						players[_].strength += smallFruitCount;
					}
				return false;
			}
 
			// �Ƿ�غϳ��ޣ�
			if (turnID >= 100)
				return false;
 
			return true;
		}
 
		// ��ȡ�������������룬���ص��Ի��ύƽ̨ʹ�ö����ԡ�
		// ����ڱ��ص��ԣ�����������Ŷ�ȡ������ָ�����ļ���Ϊ�����ļ���ʧ�ܺ���ѡ��ȴ��û�ֱ�����롣
		// ���ص���ʱ���Խ��ܶ����Ա������Windows�¿����� Ctrl-Z ��һ��������+�س�����ʾ���������������������ֻ����ܵ��м��ɡ�
		// localFileName ����ΪNULL
		// obtainedData ������Լ��ϻغϴ洢�����غ�ʹ�õ�����
		// obtainedGlobalData ������Լ��� Bot ����ǰ�洢������
		// ����ֵ���Լ��� playerID
		int ReadInput(const char *localFileName, string &obtainedData, string &obtainedGlobalData)
		{
			string str, chunk;
#ifdef _BOTZONE_ONLINE
			std::ios::sync_with_stdio(false); //��\\)
			getline(cin, str);
#else
			if (localFileName)
			{
				std::ifstream fin(localFileName);
				if (fin)
					while (getline(fin, chunk) && chunk != "")
						str += chunk;
				else
					while (getline(cin, chunk) && chunk != "")
						str += chunk;
			}
			else
				while (getline(cin, chunk) && chunk != "")
					str += chunk;
#endif
			Json::Reader reader;
			Json::Value input;
			reader.parse(str, input);
 
			int len = input["requests"].size();
 
			// ��ȡ���ؾ�̬״��
			Json::Value field = input["requests"][(Json::Value::UInt) 0],
				staticField = field["static"], // ǽ��Ͳ�����
				contentField = field["content"]; // ���Ӻ����
			height = field["height"].asInt();
			width = field["width"].asInt();
			LARGE_FRUIT_DURATION = field["LARGE_FRUIT_DURATION"].asInt();
			LARGE_FRUIT_ENHANCEMENT = field["LARGE_FRUIT_ENHANCEMENT"].asInt();
			generatorTurnLeft = GENERATOR_INTERVAL = field["GENERATOR_INTERVAL"].asInt();
 
			PrepareInitialField(staticField, contentField);
 
			// ������ʷ�ָ�����
			for (int i = 1; i < len; i++)
			{
				Json::Value req = input["requests"][i];
				for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
					if (!players[_].dead)
						actions[_] = (Direction)req[playerID2str[_]]["action"].asInt();
				NextTurn();
			}
 
			obtainedData = input["data"].asString();
			obtainedGlobalData = input["globaldata"].asString();
 
			return field["id"].asInt();
		}
 
		// ���� static �� content ����׼�����صĳ�ʼ״��
		void PrepareInitialField(const Json::Value &staticField, const Json::Value &contentField)
		{
			int r, c, gid = 0;
			generatorCount = 0;
			aliveCount = 0;
			smallFruitCount = 0;
			generatorTurnLeft = GENERATOR_INTERVAL;
			for (r = 0; r < height; r++)
				for (c = 0; c < width; c++)
				{
					GridContentType &content = fieldContent[r][c] = (GridContentType)contentField[r][c].asInt();
					GridStaticType &s = fieldStatic[r][c] = (GridStaticType)staticField[r][c].asInt();
					if (s & generator)
					{
						generators[gid].row = r;
						generators[gid++].col = c;
						generatorCount++;
					}
					if (content & smallFruit)
						smallFruitCount++;
					for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
						if (content & playerID2Mask[_])
						{
							Player &p = players[_];
							p.col = c;
							p.row = r;
							p.powerUpLeft = 0;
							p.strength = 1;
							p.dead = false;
							aliveCount++;
						}
				}
		}
 
		// ��ɾ��ߣ���������
		// action ��ʾ���غϵ��ƶ�����stay Ϊ���ƶ�
		// tauntText ��ʾ��Ҫ��������������������ַ�����������ʾ����Ļ�ϲ������κ����ã����ձ�ʾ������
		// data ��ʾ�Լ���洢����һ�غ�ʹ�õ����ݣ����ձ�ʾɾ��
		// globalData ��ʾ�Լ���洢���Ժ�ʹ�õ����ݣ��滻����������ݿ��Կ�Ծ�ʹ�ã���һֱ������� Bot �ϣ����ձ�ʾɾ��
		void WriteOutput(Direction action, string tauntText = "", string data = "", string globalData = "") const
		{
			Json::Value ret;
			ret["response"]["action"] = action;
			ret["response"]["tauntText"] = tauntText;
			ret["data"] = data;
			ret["globaldata"] = globalData;
			ret["debug"] = (Json::Int)seed;
 
#ifdef _BOTZONE_ONLINE
			Json::FastWriter writer; // ��������Ļ����þ��С���
#else
			Json::StyledWriter writer; // ���ص��������ÿ� > <
#endif
			cout << writer.write(ret) << endl;
		}
 
		// ������ʾ��ǰ��Ϸ״̬�������á�
		// �ύ��ƽ̨��ᱻ�Ż�����
		inline void DebugPrint() const
		{
#ifndef _BOTZONE_ONLINE
			printf("�غϺš�%d�����������%d��| ͼ�� ������[G] �����[0/1/2/3] ������[*] ��[o] С��[.]\n", turnID, aliveCount);
			for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				const Player &p = players[_];
				printf("[���%d(%d, %d)|����%d|�ӳ�ʣ��غ�%d|%s]\n",
					_, p.row, p.col, p.strength, p.powerUpLeft, p.dead ? "����" : "���");
			}
			putchar(' ');
			putchar(' ');
			for (int c = 0; c < width; c++)
				printf("  %d ", c);
			putchar('\n');
			for (int r = 0; r < height; r++)
			{
				putchar(' ');
				putchar(' ');
				for (int c = 0; c < width; c++)
				{
					putchar(' ');
					printf((fieldStatic[r][c] & wallNorth) ? "---" : "   ");
				}
				printf("\n%d ", r);
				for (int c = 0; c < width; c++)
				{
					putchar((fieldStatic[r][c] & wallWest) ? '|' : ' ');
					putchar(' ');
					int hasPlayer = -1;
					for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
						if (fieldContent[r][c] & playerID2Mask[_])
							if (hasPlayer == -1)
								hasPlayer = _;
							else
								hasPlayer = 4;
					if (hasPlayer == 4)
						putchar('*');
					else if (hasPlayer != -1)
						putchar('0' + hasPlayer);
					else if (fieldStatic[r][c] & generator)
						putchar('G');
					else if (fieldContent[r][c] & playerMask)
						putchar('*');
					else if (fieldContent[r][c] & smallFruit)
						putchar('.');
					else if (fieldContent[r][c] & largeFruit)
						putchar('o');
					else
						putchar(' ');
					putchar(' ');
				}
				putchar((fieldStatic[r][width - 1] & wallEast) ? '|' : ' ');
				putchar('\n');
			}
			putchar(' ');
			putchar(' ');
			for (int c = 0; c < width; c++)
			{
				putchar(' ');
				printf((fieldStatic[height - 1][c] & wallSouth) ? "---" : "   ");
			}
			putchar('\n');
#endif
		}
 
		Json::Value SerializeCurrentTurnChange()
		{
			Json::Value result;
			TurnStateTransfer &bt = backtrack[turnID - 1];
			for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				result["actions"][_] = bt.actions[_];
				result["strengthDelta"][_] = bt.strengthDelta[_];
				result["change"][_] = bt.change[_];
			}
			return result;
		}
 
		// ��ʼ����Ϸ������
		GameField()
		{
			if (constructed)
				throw runtime_error("�벻Ҫ�ٴ��� GameField �����ˣ�����������ֻӦ����һ������");
			constructed = true;
 
			turnID = 0;
		}
 
		GameField(const GameField &b) : GameField() { }
	};
 
	bool GameField::constructed = false;
}
 
 
// һЩ��������
namespace Helpers
{
 
	int actionScore[5] = {};
 
	inline int RandBetween(int a, int b)
	{
		if (a > b)
			swap(a, b);
		return rand() % (b - a) + a;
	}
 
	void RandomPlay(Pacman::GameField &gameField, int myID)
	{
		int count = 0, myAct = -1;
		while (true)
		{
			// ��ÿ�������������ĺϷ�����
			for (int i = 0; i < MAX_PLAYER_COUNT; i++)
			{
				if (gameField.players[i].dead)
					continue;
				Pacman::Direction valid[5];
				int vCount = 0;
				for (Pacman::Direction d = Pacman::stay; d < 4; ++d)
					if (gameField.ActionValid(i, d))
						valid[vCount++] = d;
				gameField.actions[i] = valid[RandBetween(0, vCount)];
			}
 
			if (count == 0)
				myAct = gameField.actions[myID];
 
			// ����һ������仯
			// NextTurn����true��ʾ��Ϸû�н���
			bool hasNext = gameField.NextTurn();
			count++;
 
			if (!hasNext)
				break;
		}
 
		// �������
		int rank2player[] = { 0, 1, 2, 3 };
		for (int j = 0; j < MAX_PLAYER_COUNT; j++)
			for (int k = 0; k < MAX_PLAYER_COUNT - j - 1; k++)
				if (gameField.players[rank2player[k]].strength > gameField.players[rank2player[k + 1]].strength)
					swap(rank2player[k], rank2player[k + 1]);
 
		int scorebase = 1;
		if (rank2player[0] == myID)
			actionScore[myAct + 1]++;
		else
			for (int j = 1; j < MAX_PLAYER_COUNT; j++)
			{
				if (gameField.players[rank2player[j - 1]].strength < gameField.players[rank2player[j]].strength)
					scorebase = j + 1;
				if (rank2player[j] == myID)
				{
					actionScore[myAct + 1] += scorebase;
					break;
				}
			}
 
		// �ָ���Ϸ״̬����������Ǳ��غϣ�
		while (count-- > 0)
			gameField.PopState();
	}
}
//�ҵĳ���   

namespace MyCode{
    int x_,y_;
    
	inline void Change_Pos(const int t,const int X,const int Y){
    	if(t==0){
    		x_=(x_+X-1)%X;
		}
		if(t==1){
    		y_=(y_+1)%Y;
		}
		if(t==2){
    		x_=(x_+1)%X;
		}
		if(t==3){
    		y_=(y_+Y-1)%Y;
		}			
	}
	int Steps[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH]={0};
	    
    int Opos_Steps[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH]={0};
    
    int DeadWay[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
    
    inline void Set_OpStep(Pacman::GameField &gameField,int ID){
    	const int X=gameField.height,Y=gameField.width;
    	const int x=gameField.players[ID].row,y=gameField.players[ID].col;
    	int temp[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH]={0};
    	for(int i=0;i<4;i++){
		    x_=x,y_=y;		
			if(((int)gameField.fieldStatic[x_][y_])&(1<<i)){
				continue;
			}	
			Change_Pos(i,X,Y);
			if((int)gameField.fieldStatic[x_][y_]&(1<<4)){
				continue;
			}		
			temp[x_][y_]=1;
	    }
		bool finished=false;
		while(!finished){	
			finished=true;
			for(int i=0;i<X;i++){
				for(int j=0;j<Y;j++){
					if((int)gameField.fieldStatic[i][j]&(1<<4)){
						continue;
					}
					if(!(i==x&&j==y)){
					    if(temp[i][j]==0){
					    	finished=false;
						}					    
					    for(int k=0;k<4;k++){						    				    	
					    	if(((int)gameField.fieldStatic[i][j])&(1<<k)){
					    		continue;
							}
							x_=i;y_=j;	
							Change_Pos(k,X,Y);
							if((int)gameField.fieldStatic[x_][y_]&(1<<4)){
								continue;
							}
							if(temp[x_][y_]>0){
								if(temp[i][j]==0){
								    temp[i][j]=temp[x_][y_]+1;
									finished=false;
								}
							else if(temp[x_][y_]+1<temp[i][j]){
									temp[i][j]=temp[x_][y_]+1;
									finished=false;
								}																
							}
						}
					}												
				}
			}
			
	    }
		for(int i=0;i<X;i++){
			for(int j=0;j<Y;j++){
				if(temp[i][j]<Opos_Steps[i][j]||Opos_Steps[i][j]==-1){
					Opos_Steps[i][j]=temp[i][j];
				}
			}
		}
	}
	
    inline void Set_Step(Pacman::GameField &gameField,int x,int y){
    	const int X=gameField.height,Y=gameField.width;
    	memset(Steps,0,sizeof(Steps));
		for(int i=0;i<4;i++){
		    x_=x,y_=y;		
			if(((int)gameField.fieldStatic[x_][y_])&(1<<i)){
				continue;
			}	
			Change_Pos(i,X,Y);
			if((int)gameField.fieldStatic[x_][y_]&(1<<4)){
				continue;
			}		
			Steps[x_][y_]=1;
	    }
		bool finished=false;
		while(!finished){	
			finished=true;
			for(int i=0;i<X;i++){
				for(int j=0;j<Y;j++){
					if((int)gameField.fieldStatic[i][j]&(1<<4)){
						continue;
					}
					if(!(i==x&&j==y)){
					    if(Steps[i][j]==0){
					    	finished=false;
						}					    
					    for(int k=0;k<4;k++){						    				    	
					    	if(((int)gameField.fieldStatic[i][j])&(1<<k)){
					    		continue;
							}
							x_=i;y_=j;	
							Change_Pos(k,X,Y);
							if((int)gameField.fieldStatic[x_][y_]&(1<<4)){
								continue;
							}
							if(Steps[x_][y_]>0){
								if(Steps[i][j]==0){
								    Steps[i][j]=Steps[x_][y_]+1;
									finished=false;
								}
								else if(Steps[x_][y_]+1<Steps[i][j]){
									Steps[i][j]=Steps[x_][y_]+1;
									finished=false;
								}																
							}
						}
					}												
				}
			}			
		}
	}
	
    int Min_Steps(Pacman::GameField &gameField,int x,int y,int ID){/* */
		Set_Step(gameField,x,y);
		memset(Opos_Steps,-1,sizeof(Opos_Steps));
		for(int i=0;i<4;i++){
			if(i==ID){
				continue;
			}
			Set_OpStep(gameField,i);			
		}
		/*
		    for(int i=0;i<X;i++){
			    for(int j=0;j<Y;j++){
				    cout<<Steps[i][j]<<"  ";
			    }
			    cout<<endl;
		    }	
			cout<<endl;
		*/		
		int Min=1000;
		const int X=gameField.height,Y=gameField.width;
		for(int i=0;i<X;i++){
			for(int j=0;j<Y;j++){
				if(((int)gameField.fieldContent[i][j]&((1<<5)+(1<<4)))&&Min>Steps[i][j]&&Steps[i][j]<Opos_Steps[i][j]){
					Min=Steps[i][j];
				}
			}
		}
		
		if(Min<1000){
			return Min;			
		}
		else{
			for(int i=0;i<X;i++){
			    for(int j=0;j<Y;j++){
				    if(((int)gameField.fieldContent[i][j]&((1<<5)+(1<<4)))&&Min>Steps[i][j]){
					    Min=Steps[i][j];
				    }
			    }
		    }
		    if(Min<1000){
		    	return Min+200;
			}
			else{
			    return -1;
			}			
		}		
	}
	
	int Min_Generator(Pacman::GameField &gameField,int x,int y){
		const int X=gameField.height,Y=gameField.width;
		Set_Step(gameField,x,y);	
		int Min=1000;	
		for(int i=0;i<X;i++){
			for(int j=0;j<Y;j++){
				if(gameField.fieldStatic[i][j]&16){
				    for(int k=0;k<4;k++){
				    	x_=i;y_=j;
				    	Change_Pos(k,X,Y);
				    	if(Steps[x_][y_]<Min){
				    		Min=Steps[x_][y_];
						}
					}
				}				
			}
		}
		return Min;
	}
	
	bool Persued(Pacman::GameField &gameField,int ID,int dir){
		x_=gameField.players[ID].row;
		y_=gameField.players[ID].col;
	    if((int)gameField.fieldStatic[x_][y_]&(1<<dir)){
				return false;
		}
		Change_Pos(dir,gameField.height,gameField.width);
		if((int)gameField.fieldStatic[x_][y_]&(1<<4)){
			return false;
		}
		for(int i=0;i<4;i++){
			if(i==ID){
				continue;
			}
			if(((int)gameField.fieldContent[x_][y_]&(1<<i))&&gameField.players[i].strength
			>gameField.players[ID].strength){
				return true;
			}
		}
		int tempx=x_,tempy=y_;
		for(int i=0;i<4;i++){
			if((i+2)%4==dir){
				continue;
			}
			x_=tempx;
			y_=tempy;
			if((int)gameField.fieldStatic[x_][y_]&(1<<i)){
				continue;
			}
			Change_Pos(i,gameField.height,gameField.width);
			if((int)gameField.fieldStatic[x_][y_]&(1<<4)){
				continue;
			}
			for(int j=0;j<4;j++){
				if(j==ID){
					continue;
				}
				if(((int)gameField.fieldContent[x_][y_]&(1<<j))&&gameField.players[j].strength
				>gameField.players[ID].strength){
					return true;;
				}
			}
		}
		return false;
	}
	
	bool To_Persue(Pacman::GameField &gameField,int ID,int dir){
		x_=gameField.players[ID].row;
		y_=gameField.players[ID].col;
		if((int)gameField.fieldStatic[x_][y_]&(1<<dir)){
			return false;
		}
		Change_Pos(dir,gameField.height,gameField.width);
		if((int)gameField.fieldStatic[x_][y_]&(1<<4)){
			return false;
		}
		for(int i=0;i<4;i++){
			if(i==ID){
				continue;
			}
			if(((int)gameField.fieldContent[x_][y_]&(1<<i))&&gameField.players[i].strength
			<gameField.players[ID].strength){
				if((int)gameField.fieldStatic[x_][y_]&15==15-(1<<((dir+2)%4))){
				    return true;
				}				
			}
		}
		return false;
	}
	
	inline void Find_Way(int depth,int i,int j,int dir,Pacman::GameField &gameField){
		const int X=gameField.height,Y=gameField.width;
		bool way=true;					
		x_=i,y_=j;
		DeadWay[x_][y_]=depth;
		while(way){						
			depth++;
			Change_Pos(dir,X,Y);
			if(((int)gameField.fieldStatic[x_][y_]&15)==0){
				way=false;
				break;
			}
			for(int t=0;t<4;t++){
				if(((int)gameField.fieldStatic[x_][y_]&15)==(1<<t)){
					way=false;
					break;
				}
			}
			if(!way){
				break;
			}
			DeadWay[x_][y_]=depth;
			for(int r=0;r<4;r++){
				if(r==(dir+2)%4){
					continue;
				}
				if(((int)gameField.fieldStatic[x_][y_]&(1<<r))==0){
					dir=r;
					break;
				}
			}						
		}
	}
	
	void Analyze_Map(Pacman::GameField &gameField){
		memset(DeadWay,-1,sizeof(DeadWay));
		const int X=gameField.height,Y=gameField.width;
		for(int i=0;i<X;i++){
			for(int j=0;j<Y;j++){
				if(gameField.fieldStatic[i][j]&(1<<4)){
					continue;
				}
				for(int k=0;k<4;k++){
					if(((int)gameField.fieldStatic[i][j]&15)!=(15-(1<<k))){
						continue;
					}
					bool way=true;
					Find_Way(0,i,j,k,gameField);
					break;
				}
			}
		}
		
		bool finished=false;
		while(!finished){
			finished=true;
			for(int i=0;i<X;i++){
				for(int j=0;j<Y;j++){
					if(DeadWay[i][j]>=0){
						continue;
					}
					int temp=0,dir,depth=-1;
					for(int k=0;k<4;k++){
					    x_=i,y_=j;
						if((int)gameField.fieldStatic[x_][y_]&(1<<k)){
							dir=k;
							temp++;
							continue;
						}						
						Change_Pos(k,X,Y);
						if(DeadWay[x_][y_]>=0){
							if(depth<DeadWay[x_][y_]){
								depth=DeadWay[x_][y_];
							}
							temp++;
							continue;
						}
					}
					if(temp!=3){
						continue;
					}
					finished=false;
					Find_Way(depth,i,j,(dir+2)%4,gameField);
				}
				
			}
			
		}
		/*
		for(int i=0;i<X;i++){
			for(int j=0;j<Y;j++){
				cout<<DeadWay[i][j]<<"  ";
			}
			cout<<endl;
		}
		cout<<endl;
		*/
	}
}

int main()
{
	Pacman::GameField gameField;
	string data, globalData; // ���ǻغ�֮����Դ��ݵ���Ϣ
 
	// ����ڱ��ص��ԣ���input.txt����ȡ�ļ�������Ϊ����
	// �����ƽ̨�ϣ��򲻻�ȥ�������input.txt
	int myID = gameField.ReadInput("input.txt", data, globalData); // ���룬������Լ�ID
	srand(Pacman::seed + myID);
	
	bool persued=false,to_persue=false,persuetemp[4]={0},nothing=false;
    int movement=-1,min=MyCode::Min_Steps(gameField,gameField.players[myID].row,gameField.players[myID].col,myID);
	const int X=gameField.height,Y=gameField.width;	
	int backup_position1=-1,backup_position2=-1,deadwaytemp=-1;
			   	
	MyCode::Analyze_Map(gameField);
	
	for(int i=0;i<4;i++){
		if(MyCode::Persued(gameField,myID,i)){
			persuetemp[i]=true;
			persued=true;
		}
		
    }
    
    if(gameField.fieldContent[gameField.players[myID].row][gameField.players[myID].col]&((1<<4)+(1<<5))&&persued==0){
		gameField.DebugPrint();
		gameField.WriteOutput((Pacman::Direction)(-1), "It's mine!", data, globalData);
		return 0;
	}
	
	if(min!=-1){
		for(int i=0;i<4;i++){
    	    if(persuetemp[i]){
			    
    		    continue;
		    }
		    if(MyCode::To_Persue(gameField,myID,i)){
			    movement=i;
			    to_persue=true;
			    break;
		    }
		    MyCode::x_=gameField.players[myID].row;
		    MyCode::y_=gameField.players[myID].col;
		    if((gameField.fieldStatic[MyCode::x_][MyCode::y_])&(1<<i)){
			    continue;
	        }
	        MyCode::Change_Pos(i,X,Y);
		    if(gameField.fieldStatic[MyCode::x_][MyCode::y_]&(1<<4)){
	    	    continue;
		    }
	        if(persued&&MyCode::DeadWay[MyCode::x_][MyCode::y_]>=0){
	        	if(MyCode::DeadWay[MyCode::x_][MyCode::y_]>deadwaytemp){
				    backup_position2=i;
				    deadwaytemp=MyCode::DeadWay[MyCode::x_][MyCode::y_];
				}
	    	    
	    	    continue;
		    }
		    backup_position1=i;
	        int temp=MyCode::Min_Steps(gameField,MyCode::x_,MyCode::y_,myID);
	        if(temp>=0&&(temp<min||i==0)){
	    	    min=temp;
	    	    movement=i;	    	
		    }
	    }
	    
	    if(movement==-1){
	    	movement=backup_position1;
		}
		if(movement==-1){
			movement=backup_position2;
		}
	}
	
	else{
		int min_genr;
		for(int i=0;i<4;i++){
    	    if(persuetemp[i]){			    
    		    continue;
		    }
		    if(MyCode::To_Persue(gameField,myID,i)){
			    movement=i;
			    to_persue=true;
			    break;
		    }
		    MyCode::x_=gameField.players[myID].row;
		    MyCode::y_=gameField.players[myID].col;
		    if((gameField.fieldStatic[MyCode::x_][MyCode::y_])&(1<<i)){
			    continue;
	        }
	        MyCode::Change_Pos(i,X,Y);
		    if(gameField.fieldStatic[MyCode::x_][MyCode::y_]&(1<<4)){
	    	    continue;
		    }
	        if(persued&&MyCode::DeadWay[MyCode::x_][MyCode::y_]>=0){
	        	if(MyCode::DeadWay[MyCode::x_][MyCode::y_]>deadwaytemp){
				    backup_position2=i;
				    deadwaytemp=MyCode::DeadWay[MyCode::x_][MyCode::y_];
				}	    	    
	    	    continue;
		    }
	        backup_position1=i;
	        if(MyCode::DeadWay[MyCode::x_][MyCode::y_]==-1||MyCode::DeadWay[MyCode::x_][MyCode::y_]
			>MyCode::DeadWay[gameField.players[myID].row][gameField.players[myID].col]&&
			MyCode::DeadWay[gameField.players[myID].row][gameField.players[myID].col]>=0){
				int temp=MyCode::Min_Generator(gameField,MyCode::x_,MyCode::y_);
				if(i==0){
					min_genr=temp;
					movement=i;	 
				}
				else if(temp<min_genr){
					min_genr=temp;
					movement=i;
				}	    	      	
		    }
	    }
	    if(movement==-1){
	    	movement=backup_position1;
		}
		if(movement==-1){
			movement=backup_position2;
		}
	}
		
	if(persued&&movement==-1){
		for(int i=0;i<4;i++){
			if(gameField.fieldStatic[gameField.players[myID].row][gameField.players[myID].col]&(1<<i)==0){
					movement=i;
			}
		}
	}
	
    
	// �����ǰ��Ϸ����״̬�Թ����ص��ԡ�ע���ύ��ƽ̨�ϻ��Զ��Ż��������ص��ġ�
	gameField.DebugPrint();
 
	// ��������Ƿ����
	if(to_persue){
		gameField.WriteOutput((Pacman::Direction)(movement), "Yummy!", data, globalData);
	}
	else if(persued){
		gameField.WriteOutput((Pacman::Direction)(movement), "Leave me alone!", data, globalData);
	}
	else if (rand() % 5){
	    gameField.WriteOutput((Pacman::Direction)(movement), "Hey!", data, globalData);
	}		
	else{
	    gameField.WriteOutput((Pacman::Direction)(movement), "Hello, cruel world!", data, globalData);
	}		
	return 0;
}
