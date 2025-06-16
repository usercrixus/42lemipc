#include "game.h"

static int countCardinalThreats(t_player *p)
{
	int count = 0;

	if (p->y > 0 &&
		shared->map[p->y - 1][p->x] != EMPTY_TILE &&
		shared->map[p->y - 1][p->x] != p->symbole)
		count++;

	if (p->y < MAP_HEIGHT - 1 &&
		shared->map[p->y + 1][p->x] != EMPTY_TILE &&
		shared->map[p->y + 1][p->x] != p->symbole)
		count++;

	if (p->x > 0 &&
		shared->map[p->y][p->x - 1] != EMPTY_TILE &&
		shared->map[p->y][p->x - 1] != p->symbole)
		count++;

	if (p->x < MAP_WIDTH - 1 &&
		shared->map[p->y][p->x + 1] != EMPTY_TILE &&
		shared->map[p->y][p->x + 1] != p->symbole)
		count++;

	return count;
}

static int countDiagonalThreats(t_player *p)
{
	int count = 0;

	if (p->y > 0 && p->x > 0 &&
		shared->map[p->y - 1][p->x - 1] != EMPTY_TILE &&
		shared->map[p->y - 1][p->x - 1] != p->symbole)
		count++;

	if (p->y > 0 && p->x < MAP_WIDTH - 1 &&
		shared->map[p->y - 1][p->x + 1] != EMPTY_TILE &&
		shared->map[p->y - 1][p->x + 1] != p->symbole)
		count++;

	if (p->y < MAP_HEIGHT - 1 && p->x > 0 &&
		shared->map[p->y + 1][p->x - 1] != EMPTY_TILE &&
		shared->map[p->y + 1][p->x - 1] != p->symbole)
		count++;

	if (p->y < MAP_HEIGHT - 1 && p->x < MAP_WIDTH - 1 &&
		shared->map[p->y + 1][p->x + 1] != EMPTY_TILE &&
		shared->map[p->y + 1][p->x + 1] != p->symbole)
		count++;

	return count;
}

int isAlive(t_player *p)
{
	return (countCardinalThreats(p) + countDiagonalThreats(p) < 2);
}

void manageDeath(t_player *p)
{
	shared->map[p->y][p->x] = EMPTY_TILE;
	p->isAlive = 0;
}

void setIsAlive()
{
	for (size_t i = 0; i < MAX_PLAYER; i++)
	{
		t_player *p = &shared->players[i];
		if (p->pid != -1 && !isAlive(p))
			manageDeath(p);
	}
}

static void effectiveMove(int dx, int dy)
{
	t_player *player = &shared->players[playerId];
	int new_x = player->x + dx;
	int new_y = player->y + dy;
	if (!(new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT))
	{
		if (shared->map[new_y][new_x] == EMPTY_TILE)
		{
			shared->map[player->y][player->x] = EMPTY_TILE;
			shared->map[new_y][new_x] = player->symbole;
			player->x = new_x;
			player->y = new_y;
		}
	}
}

void move(t_move move)
{
	if (shared->players[playerId].isAlive)
	{
		if (move == TOP)
			effectiveMove(0, -1);
		else if (move == BOT)
			effectiveMove(0, 1);
		else if (move == LEFT)
			effectiveMove(-1, 0);
		else if (move == RIGHT)
			effectiveMove(1, 0);
		for (int i = 0; i < MAX_PLAYER; i++)
		{
			if (shared->players[i].pid != -1)
				kill(shared->players[i].pid, SIGUSR1);
		}
	}
}

void drawMap()
{
	for (size_t y = 0; y < MAP_HEIGHT; y++)
	{
		for (size_t x = 0; x < MAP_WIDTH; x++)
		{
			ft_printf(" %c ", shared->map[y][x]);
		}
		ft_printf("\n");
	}
	ft_printf("\n");
}

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