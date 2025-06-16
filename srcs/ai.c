#include "ai.h"

t_move getBestMove()
{
	t_player *self = &shared->players[playerId];
	int tx = -1, ty = -1;

	// === LEADER SELECTS A TARGET ===
	int isLeader = 1;
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		t_player *p = &shared->players[i];
		if (p->isAlive && p->symbole == self->symbole && p->playerId < self->playerId)
			isLeader = 0;
	}

	int msgid = msgget(MSGQ_KEY, 0666 | IPC_CREAT);
	t_msg_target msg;

	if (isLeader)
	{
		int minDist = 9999;
		t_player *closest = NULL;

		for (int i = 0; i < MAX_PLAYER; i++)
		{
			t_player *enemy = &shared->players[i];
			if (!enemy->isAlive || enemy->symbole == self->symbole)
				continue;
			int dist = abs(enemy->x - self->x) + abs(enemy->y - self->y);
			if (dist < minDist)
			{
				minDist = dist;
				closest = enemy;
			}
		}

		if (closest)
		{
			msg.mtype = MSG_TYPE_TARGET;
			msg.targetX = closest->x;
			msg.targetY = closest->y;
			msg.team = self->symbole;
			msgsnd(msgid, &msg, sizeof(t_msg_target) - sizeof(long), 0);
			tx = closest->x;
			ty = closest->y;
		}
	}
	else
	{
		while (msgrcv(msgid, &msg, sizeof(t_msg_target) - sizeof(long), MSG_TYPE_TARGET, IPC_NOWAIT) > 0)
		{
			if (msg.team == self->symbole)
			{
				tx = msg.targetX;
				ty = msg.targetY;
			}
		}
	}

	if (tx == -1 || ty == -1)
		return STAY;

	// === SMART PATH SELECTION ===
	int dx = tx - self->x;
	int dy = ty - self->y;

	typedef struct
	{
		t_move dir;
		int offsetX;
		int offsetY;
	} MoveOption;

	MoveOption moveOptions[4];
	int count = 0;

	// Prefer axis with greater distance
	if (abs(dx) >= abs(dy))
	{
		moveOptions[count++] = (MoveOption){dx > 0 ? RIGHT : LEFT, dx > 0 ? 1 : -1, 0};
		if (dy != 0)
			moveOptions[count++] = (MoveOption){dy > 0 ? BOT : TOP, 0, dy > 0 ? 1 : -1};
	}
	else
	{
		moveOptions[count++] = (MoveOption){dy > 0 ? BOT : TOP, 0, dy > 0 ? 1 : -1};
		if (dx != 0)
			moveOptions[count++] = (MoveOption){dx > 0 ? RIGHT : LEFT, dx > 0 ? 1 : -1, 0};
	}

	// Add remaining directions to complete fallback
	t_move allDirs[4] = {TOP, BOT, LEFT, RIGHT};
	int dxs[4] = {0, 0, -1, 1};
	int dys[4] = {-1, 1, 0, 0};

	for (int i = 0; i < 4; i++)
	{
		int alreadyAdded = 0;
		for (int j = 0; j < count; j++)
		{
			if (moveOptions[j].dir == allDirs[i])
			{
				alreadyAdded = 1;
				break;
			}
		}
		if (!alreadyAdded && count < 4)
		{
			moveOptions[count++] = (MoveOption){allDirs[i], dxs[i], dys[i]};
		}
	}

	// Try each move in order of preference
	for (int i = 0; i < count; i++)
	{
		int newX = self->x + moveOptions[i].offsetX;
		int newY = self->y + moveOptions[i].offsetY;

		if (newX >= 0 && newX < MAP_WIDTH &&
			newY >= 0 && newY < MAP_HEIGHT &&
			shared->map[newY][newX] == EMPTY_TILE)
		{
			return moveOptions[i].dir;
		}
	}

	return STAY;
}

bool isGameEnd()
{
	char team;

	team = '0';
	for (size_t y = 0; y < MAP_HEIGHT; y++)
	{
		for (size_t x = 0; x < MAP_WIDTH; x++)
		{
			if (shared->map[y][x] != team)
			{
				if (team == '0')
					team = shared->map[y][x];
				else
					return (true);
			}
		}
	}
	return (false);
}