#include "ai.h"
#include <sys/msg.h>
#include <errno.h>
#include <stdlib.h>

static int get_msgid()
{
	static int msgid = -1;
	if (msgid == -1)
		msgid = msgget(MSGQ_KEY, IPC_CREAT | 0666);
	return msgid;
}

static int find_nearest_enemy(int *tx, int *ty)
{
    t_player *me = &shared->players[playerId];
    int bestD = 1 << 30;
    int bx = me->x;
    int by = me->y;

    for (int y = 0; y < MAX_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAX_MAP_WIDTH; x++)
        {
            char c = shared->map[y][x];
            if (c != EMPTY_TILE && c != me->symbole)
            {
                int d = abs(x - me->x) + abs(y - me->y);
				if (d < bestD)
				{
					bestD = d;
					bx = x;
					by = y;
				}
			}
		}
	}
	if (bestD == (1 << 30))
		return 0;
	*tx = bx;
	*ty = by;
	return 1;
}

static int enemy_threats_at(int x, int y, char me)
{
    int cnt = 0;
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
                continue;
            int nx = x + dx;
            int ny = y + dy;
            if (nx < 0 || nx >= MAX_MAP_WIDTH || ny < 0 || ny >= MAX_MAP_HEIGHT)
                continue;
            char c = shared->map[ny][nx];
            if (c != EMPTY_TILE && c != me)
                cnt++;
        }
    }
    return cnt;
}

static int is_neighbor(int ax, int ay, int bx, int by)
{
    return (abs(ax - bx) <= 1 && abs(ay - by) <= 1 && !(ax == bx && ay == by));
}

static int allied_threats_around(int tx, int ty, char team)
{
    int cnt = 0;
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
                continue;
            int nx = tx + dx;
            int ny = ty + dy;
            if (nx < 0 || nx >= MAX_MAP_WIDTH || ny < 0 || ny >= MAX_MAP_HEIGHT)
                continue;
            if (shared->map[ny][nx] == team)
                cnt++;
        }
    }
    return cnt;
}

static int is_enemy_at(int x, int y, char me)
{
    if (x < 0 || x >= MAX_MAP_WIDTH || y < 0 || y >= MAX_MAP_HEIGHT)
        return 0;
    char c = shared->map[y][x];
    return (c != EMPTY_TILE && c != me);
}

static int has_empty_adjacent(int x, int y)
{
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
                continue;
            int nx = x + dx;
            int ny = y + dy;
            if (nx < 0 || nx >= MAX_MAP_WIDTH || ny < 0 || ny >= MAX_MAP_HEIGHT)
                continue;
            if (shared->map[ny][nx] == EMPTY_TILE)
                return 1;
        }
    }
	return 0;
}

static int find_nearest_killable_enemy(int *tx, int *ty)
{
	t_player *me = &shared->players[playerId];
	int bestD = 1 << 30;
	int bx = me->x;
	int by = me->y;
    for (int y = 0; y < MAX_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAX_MAP_WIDTH; x++)
        {
            if (is_enemy_at(x, y, me->symbole) && has_empty_adjacent(x, y))
            {
                int d = abs(x - me->x) + abs(y - me->y);
                if (d < bestD)
				{
					bestD = d;
					bx = x;
					by = y;
				}
			}
		}
	}
	if (bestD == (1 << 30))
		return 0;
	*tx = bx;
	*ty = by;
	return 1;
}

static t_move step_toward(int tx, int ty)
{
    t_player *me = &shared->players[playerId];

	// Evaluate only legal cardinal moves; never pass turn if a move exists
	int dirs[4][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} };
	int bestScore = -2147483647;
	int bestDx = 0;
	int bestDy = 0;
	int found = 0;

    for (int i = 0; i < 4; i++)
    {
        int ndx = dirs[i][0];
        int ndy = dirs[i][1];
        int nx = me->x + ndx;
        int ny = me->y + ndy;
        if (!(nx >= 0 && nx < MAX_MAP_WIDTH && ny >= 0 && ny < MAX_MAP_HEIGHT))
            continue;
        if (shared->map[ny][nx] != EMPTY_TILE)
            continue;
        found = 1;
        int dist = abs(tx - nx) + abs(ty - ny);
        int risk = enemy_threats_at(nx, ny, me->symbole);
        // Purely offensive: minimize distance, avoid suicide
        int score = -dist * 100;
        if (risk >= 2)
            score -= 100000;
        else
            score -= risk * 10;
        // If this move would create a 2+ threat around the target (including diagonal), prefer it strongly
        int base = allied_threats_around(tx, ty, me->symbole);
        int meAdjPre = is_neighbor(me->x, me->y, tx, ty) ? 1 : 0;
        int meAdjPost = is_neighbor(nx, ny, tx, ty) ? 1 : 0;
        int after = base - meAdjPre + meAdjPost;
        if (after >= 2)
            score += 100000; // take the kill setup immediately
        score += (rand() & 1);
        if (score > bestScore)
        {
            bestScore = score;
            bestDx = ndx;
			bestDy = ndy;
		}
	}
	if (!found)
		return STAY;
	if (bestDx == -1 && bestDy == 0) return LEFT;
	if (bestDx == 1 && bestDy == 0)  return RIGHT;
	if (bestDx == 0 && bestDy == -1) return TOP;
	if (bestDx == 0 && bestDy == 1)  return BOT;
	return STAY;
}

t_move getBestMove()
{
	t_player *me = &shared->players[playerId];
	int msgid = get_msgid();
	if (msgid == -1)
		return STAY;

	t_msg_target msg;
	ssize_t sz = sizeof(t_msg_target) - sizeof(long);

	int have_msg = (msgrcv(msgid, &msg, sz, (long)me->symbole, IPC_NOWAIT) >= 0);
	if (!have_msg)
		errno = 0;

	int tx;
	int ty;
	if (have_msg)
	{
		// Validate existing team target
		if (is_enemy_at(msg.targetX, msg.targetY, me->symbole) &&
			has_empty_adjacent(msg.targetX, msg.targetY))
		{
			// Re-broadcast and go
			msgsnd(msgid, &msg, sz, IPC_NOWAIT);
			return step_toward(msg.targetX, msg.targetY);
		}
	}

	// Find a new team target: prefer nearest killable, fallback to nearest enemy
	if (!find_nearest_killable_enemy(&tx, &ty))
	{
		if (!find_nearest_enemy(&tx, &ty))
			return STAY;
	}
	msg.mtype = (long)me->symbole;
	msg.targetX = tx;
	msg.targetY = ty;
	msg.team = me->symbole;
	msgsnd(msgid, &msg, sz, IPC_NOWAIT);
	return step_toward(tx, ty);
}
