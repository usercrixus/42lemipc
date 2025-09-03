#include "ai.h"
#include <sys/msg.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

static int getMsgid()
{
	static int msgid = -1;
	if (msgid == -1)
		msgid = msgget(MSGQ_KEY, IPC_CREAT | 0666);
	return msgid;
}

static int getCountEnemyThreatsAt(int x, int y, char me)
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

static int isNeighbor(int ax, int ay, int bx, int by)
{
    return (abs(ax - bx) <= 1 && abs(ay - by) <= 1 && !(ax == bx && ay == by));
}

static int getCountAlliedThreatsAround(int tx, int ty, char team)
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

static int isEnemyAt(int x, int y, char me)
{
    if (x < 0 || x >= MAX_MAP_WIDTH || y < 0 || y >= MAX_MAP_HEIGHT)
        return 0;
    char c = shared->map[y][x];
    return (c != EMPTY_TILE && c != me);
}

static int hasEmptyAdjacent(int x, int y)
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

static int setNearestEnemy(int *tx, int *ty)
{
    t_player *me = &shared->players[playerId];
    int bestD = INT_MAX;
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
	if (bestD == INT_MAX)
		return 0;
	*tx = bx;
	*ty = by;
	return 1;
}

static int setNearestKillableEnemy(int *tx, int *ty)
{
	t_player *me = &shared->players[playerId];
	int bestD = INT_MAX;
	int bx = me->x;
	int by = me->y;
    for (int y = 0; y < MAX_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAX_MAP_WIDTH; x++)
        {
            if (isEnemyAt(x, y, me->symbole) && hasEmptyAdjacent(x, y))
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
	if (bestD == INT_MAX)
		return 0;
	*tx = bx;
	*ty = by;
	return 1;
}

// Same as above, but compute distance from an arbitrary reference point (rx, ry)
static int setNearestEnemyFrom(int rx, int ry, int *tx, int *ty)
{
    int bestD = INT_MAX;
    int bx = rx;
    int by = ry;

    for (int y = 0; y < MAX_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAX_MAP_WIDTH; x++)
        {
            char c = shared->map[y][x];
            if (c != EMPTY_TILE)
            {
                // Any non-empty cell that is not the same team as the ref target is a candidate.
                // We can’t know the ref team reliably here; use current player team for exclusion.
                if (c == shared->players[playerId].symbole)
                    continue;
                int d = abs(x - rx) + abs(y - ry);
                if (d < bestD)
                {
                    bestD = d;
                    bx = x;
                    by = y;
                }
            }
        }
    }
    if (bestD == INT_MAX)
        return 0;
    *tx = bx;
    *ty = by;
    return 1;
}

static int setNearestKillableEnemyFrom(int rx, int ry, int *tx, int *ty)
{
    int bestD = INT_MAX;
    int bx = rx;
    int by = ry;
    char myTeam = shared->players[playerId].symbole;
    for (int y = 0; y < MAX_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAX_MAP_WIDTH; x++)
        {
            char c = shared->map[y][x];
            if (c == EMPTY_TILE || c == myTeam)
                continue;
            if (!hasEmptyAdjacent(x, y))
                continue;
            int d = abs(x - rx) + abs(y - ry);
            if (d < bestD)
            {
                bestD = d;
                bx = x;
                by = y;
            }
        }
    }
    if (bestD == INT_MAX)
        return 0;
    *tx = bx;
    *ty = by;
    return 1;
}

static t_move stepToward(int tx, int ty)
{
    t_player *me = &shared->players[playerId];
    int dirs[4][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} };
    int bestScore = INT_MIN;
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
        int risk = getCountEnemyThreatsAt(nx, ny, me->symbole);
        int score = -dist;
        if (risk >= 2)
            score = INT_MIN + 1;
        else if (risk == 1)
            score -= risk;
        int base = getCountAlliedThreatsAround(tx, ty, me->symbole);
        int meAdjPre = isNeighbor(me->x, me->y, tx, ty) ? 1 : 0;
        int meAdjPost = isNeighbor(nx, ny, tx, ty) ? 1 : 0;
        int after = base - meAdjPre + meAdjPost;
        if (after >= 2)
            score /= 2;
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
    int msgid = getMsgid();
    if (msgid == -1)
        return STAY;

    t_msg_target msg = {0};
    ssize_t sz = sizeof(t_msg_target) - sizeof(long);
    int have_msg = (msgrcv(msgid, &msg, sz, (long)me->symbole, IPC_NOWAIT) >= 0);

    int tx;
    int ty;
    if (have_msg)
    {
        if (isEnemyAt(msg.targetX, msg.targetY, me->symbole) &&
            hasEmptyAdjacent(msg.targetX, msg.targetY))
        {
            msgsnd(msgid, &msg, sz, IPC_NOWAIT);
            return stepToward(msg.targetX, msg.targetY);
        }
        // Otherwise, pick the next target relative to the last target position
        int refx = msg.targetX;
        int refy = msg.targetY;
        if (!setNearestKillableEnemyFrom(refx, refy, &tx, &ty))
        {
            if (!setNearestEnemyFrom(refx, refy, &tx, &ty))
                return STAY;
        }
    }
    else
    {
        // No prior team target; pick relative to our own position as before
        if (!setNearestKillableEnemy(&tx, &ty))
        {
            if (!setNearestEnemy(&tx, &ty))
                return STAY;
        }
    }
    // Ensure no padding bytes remain uninitialized in mtext
    msg.mtype = 0;
    msg.targetX = tx;
    msg.targetY = ty;
    msg.team = me->symbole;
    msg.mtype = (long)me->symbole;
    msgsnd(msgid, &msg, sz, IPC_NOWAIT);
    return stepToward(tx, ty);
}
