#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <vector>
#include <chrono>
#include <random>
#include <stdlib.h>

const int N = 5005;
const int K = 205;
const int MaxT = 15000;

int n, m;
int NumBaseBlue, NumBaseRed;
int NumPlane;
std::string s[N];

struct Pos
{
    int x, y;
    bool operator==(const Pos& other) const
    {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Pos& other) const
    {
        return x != other.x || y != other.y;
    }
    bool operator<(const Pos& other) const
    {
        if (x == other.x) return y < other.y;
        return x < other.x;
    }
};

struct Base
{
    int x, y;
    int gas, c, def, val;
    int maxgas, maxc;
};

Base b[N], r[N];

struct Plane
{
    int id;
    int x, y;
    int gas, c;
    int maxgas, maxc;
};

Plane pe[K];

struct Consume
{
    int gas, c;
};

std::queue<Plane> PlaneQueue, tmpQueue;
std::queue<Pos> DisQueue;

int dx[] = {-1, 1, 0, 0};
int dy[] = {0, 0, -1, 1};

struct Move
{
    int id, dir;
};
struct Attack
{
    int id, dir, count;
};
struct Fuel
{
    int id, count;
};
struct Missile
{
    int id, count;
};

std::vector<Move> MoveAction[MaxT + 5];
std::vector<Attack> AttackAction[MaxT + 5];
std::vector<Fuel> FuelAction[MaxT + 5];
std::vector<Missile> MissileAction[MaxT + 5];
std::vector<Plane> BackPlane[MaxT + 5];

// std::map<Pos, int> Planedis[K];
std::map<Pos, Pos> Planepre[N];

int Planedis[N][K][K];

std::vector<Pos> refe[MaxT + 5];

void GetBaseDis(int id, Base srcbase)
{
    Pos src = {srcbase.x, srcbase.y};
    DisQueue.push(src);
    Planedis[id][src.x][src.y] = 0;
    while (!DisQueue.empty())
    {
        Pos cur = DisQueue.front();
        DisQueue.pop();
        for (int i = 0; i < 4; ++i)
        {
            Pos ncur = {cur.x + dx[i], cur.y + dy[i]};
            if (ncur.x < 0 || ncur.x >= n || ncur.y < 0 || ncur.y >= m) continue;
            if (Planedis[id][ncur.x][ncur.y] >= 0) continue;
            Planedis[id][ncur.x][ncur.y] = Planedis[id][cur.x][cur.y] + 1;
            Planepre[id][ncur] = cur;
            if (s[ncur.x][ncur.y] != '#') DisQueue.push(ncur);
        }
    }
    while (!DisQueue.empty()) DisQueue.pop();
}
int GetBaseId(Plane cur)
{
    for (int i = 0; i < NumBaseBlue; ++i)
    {
        if (b[i].x == cur.x && b[i].y == cur.y) return i;
    }
    return -1;
}
int GetBaseId(Pos cur)
{
    for (int i = 0; i < NumBaseBlue; ++i)
    {
        if (b[i].x == cur.x && b[i].y == cur.y) return i;
    }
    return -1;
}
int GetDis(Pos src, Pos dst)
{
    int id = GetBaseId(src);
    if (Planedis[id][dst.x][dst.y] == -1) return 1e9;
    return Planedis[id][dst.x][dst.y];
}
bool CheckPlaneCanGo(Plane p, Base gd, Base gs, Base ge)
{
    Pos src = {p.x, p.y};
    Pos dst = {gd.x, gd.y};
    Pos eds = {ge.x, ge.y};
    int id = GetBaseId(src);
    int dis1 = GetDis(src, dst) - 1, dis2 = GetDis(eds, Planepre[id][dst]);
    if (dis1 + dis2 + 1 <= gs.gas && dis1 + dis2 + 1 <= p.maxgas) return true;
    return false;
}
Consume GetConsume(Plane p, Base gd, Base gs, Base ge)
{
    Pos src = {p.x, p.y};
    Pos dst = {gd.x, gd.y};
    Pos eds = {ge.x, ge.y};
    int id = GetBaseId(src);
    int dis1 = GetDis(src, dst) - 1, dis2 = GetDis(eds, Planepre[id][dst]);
    int cc = std::max(std::min({p.maxc, gs.c, gd.def}) - p.c, 0);
    int cg = std::max(std::min(dis1 + dis2 - p.gas, p.maxgas - p.gas),0);
    return {cg, cc};
}
Pos RemoveGraphIcon(Base cur)
{
    return {cur.x, cur.y};
}
void ActionOnBase(Consume cs, Base &gs)
{
    gs.gas -= cs.gas;
    gs.c -= cs.c;
}
int GetDir(Pos lst, Pos nxt)
{
    if (lst.x - 1 == nxt.x && lst.y == nxt.y) return 0;
    if (lst.x + 1 == nxt.x && lst.y == nxt.y) return 1;
    if (lst.x == nxt.x && lst.y - 1 == nxt.y) return 2;
    if (lst.x == nxt.x && lst.y + 1 == nxt.y) return 3;
    return -1;
}
void SetMoveAction(int t, Plane p, Base gd, Base gs, Base ge, Consume cs, Pos NeedRefersh)
{
    Pos src = {p.x, p.y};
    Pos dst = {gd.x, gd.y};
    Pos eds = {ge.x, ge.y};
    Pos cur = dst;
    int id = GetBaseId(src);
    // std::cerr << id<<"\n";
    while (cur != src)
    {
        if (cur != dst)
        {
            MoveAction[t + Planedis[id][Planepre[id][cur].x][Planepre[id][cur].y]].push_back({p.id, GetDir(Planepre[id][cur], cur)});
            // MoveAction[t + 2 * (Planedis[id][dst] - 1) - Planedis[id][Planepre[id][cur]]].push_back({p.id, GetDir(cur, Planepre[id][cur])});
        }
        cur = Planepre[id][cur];
        // if (cur.x != 0 || cur.y != 0)std::cerr << cur.x <<" "<< cur.y <<"\n";
    }

    cur = Planepre[id][dst];
    int newid = GetBaseId(eds);
    while (cur != eds)
    {
        MoveAction[t + Planedis[id][Planepre[id][dst].x][Planepre[id][dst].y] + Planedis[newid][Planepre[id][dst].x][Planepre[id][dst].y] - Planedis[newid][cur.x][cur.y] + 1].push_back({p.id, GetDir(cur, Planepre[newid][cur])});
        // MoveAction[t + 2 * (Planedis[id][dst] - 1) - Planedis[id][Planepre[id][cur]]].push_back({p.id, GetDir(cur, Planepre[id][cur])});
        cur = Planepre[newid][cur];
        // if (cur.x != 0 || cur.y != 0)std::cerr << cur.x <<" "<< cur.y <<"\n";
    }
    AttackAction[t + Planedis[id][dst.x][dst.y] - 1].push_back({p.id, GetDir(Planepre[id][dst], dst), cs.c});
    FuelAction[t].push_back({p.id, cs.gas});
    MissileAction[t].push_back({p.id, cs.c});
    p.x = ge.x, p.y = ge.y;
    BackPlane[t + Planedis[id][dst.x][dst.y]+ Planedis[newid][dst.x][dst.y]].push_back(p);
    if (NeedRefersh.x >= 0 && NeedRefersh.y >= 0)
    {
        refe[t + Planedis[id][dst.x][dst.y]].push_back({NeedRefersh.x, NeedRefersh.y});
    }
}
int main()
{
    freopen("../testcase2.in", "r", stdin);
    freopen("../testcase2.out", "w", stdout);
    srand(time(0));
    auto start = std::chrono::high_resolution_clock::now();
    std::cin >> n >> m;
    for (int i = 0; i < n; ++i) std::cin >> s[i];
    std::cin >> NumBaseBlue;
    for (int i = 0; i < NumBaseBlue; ++i)
    {
        std::cin >> b[i].x >> b[i].y;
        std::cin >> b[i].gas >> b[i].c >> b[i].def >> b[i].val;
        b[i].maxgas = b[i].gas, b[i].maxc = b[i].c;
    }
    std::cin >> NumBaseRed;
    int TotalScore = 0;
    for (int i = 0; i < NumBaseRed; ++i)
    {
        std::cin >> r[i].x >> r[i].y;
        std::cin >> r[i].gas >> r[i].c >> r[i].def >> r[i].val;
        TotalScore += r[i].val;
    }
    std::sort(b, b + NumBaseBlue, [&](Base b1, Base b2)
    {
        return b1.val / b1.def > b2.val / b2.def;
    });
    std::cerr << "Total Score is: " << TotalScore << "\n";
    std::cin >> NumPlane;
    for (int i = 0; i < NumPlane; ++i)
    {
        Plane cur = {i,0, 0, 0, 0, 0,0};
        std::cin >> cur.x >> cur.y;
        std::cin >> cur.maxgas >> cur.maxc;
        pe[i] = cur;
        PlaneQueue.push(cur);
    }
    memset(Planedis, -1, sizeof(Planedis));
    for (int k = 0; k < NumBaseBlue; ++k) GetBaseDis(k, b[k]);
    int BreakTime = MaxT;
    int ExpectedScore = 0;
    for (int t = 0; t < MaxT; ++t)
    {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = now - start;
        if (elapsed.count() >= 300) {
            BreakTime = std::min(BreakTime, t);
            std::cout << "OK\n";
            continue;
        }
        std::cerr << "Running on Epochs: " << t << ". The time now is: " << elapsed.count() << "seconds" << ". ";
        std::cerr << "The Expected Score now is: " << ExpectedScore << "\n";
        for (auto ret : refe[t])  s[ret.x][ret.y] = '.';
        if (refe[t].size() > 0)
        {
            memset(Planedis, -1, sizeof(Planedis));
            for (int k = 0; k < NumBaseBlue; ++k) GetBaseDis(k, b[k]);
        }
        for (auto pl:BackPlane[t]) PlaneQueue.push(pl);

        for (int i = 0; i < NumBaseRed; ++i)
        {
            // std::cerr << "Planning on Base: " << i << "\n";
            while (r[i].def > 0 && !PlaneQueue.empty())
            {
                Plane cur = PlaneQueue.front();
                PlaneQueue.pop();
                // std::cerr<<"QWQ\n";
                // std::cerr << 1.0 * rand() / RAND_MAX << " "<< std::max(1.0 * r[i].gas / r[i].maxgas, 1.0 * r[i].c / r[i].maxc) << "\n";
                // std::cerr << std::min(1.0 * b[GetBaseId(cur)].gas / b[GetBaseId(cur)].maxgas, 1.0 * b[GetBaseId(cur)].c / b[GetBaseId(cur)].maxc);
                if (1.0 * rand() / RAND_MAX > std::min(1.0 * b[GetBaseId(cur)].gas / b[GetBaseId(cur)].maxgas, 1.0 * b[GetBaseId(cur)].c / b[GetBaseId(cur)].maxc))
                {
                    int newid = -1;
                    int maxval = 0;
                    for (int j = 0; j < NumBaseRed; ++j)
                    {
                        if (CheckPlaneCanGo(cur, r[i], b[GetBaseId(cur)], b[j]))
                        {
                            int curval = b[j].gas * b[j].c;
                            if (curval > maxval)
                            {
                                newid = j;
                                maxval = curval;
                            }
                        }
                    }
                    // std::cerr << newid << "\n";
                    if (newid >= 0)
                    {
                        std::cerr << newid << "\n";
                        Pos NeedRefersh = {-1, -1};
                        Consume cs = GetConsume(cur, r[i], b[GetBaseId(cur)], b[newid]);
                        r[i].def -= cs.c;
                        if (r[i].def <= 0)
                        {
                            NeedRefersh = RemoveGraphIcon(r[i]);
                            ExpectedScore += r[i].val;
                        }
                        ActionOnBase(cs, b[GetBaseId(cur)]);
                        SetMoveAction(t, cur, r[i], b[GetBaseId(cur)], b[newid], cs, NeedRefersh);
                    }
                    else tmpQueue.push(cur);
                }
                else
                {
                    if (CheckPlaneCanGo(cur, r[i], b[GetBaseId(cur)], b[GetBaseId(cur)]))
                    {
                        // std::cerr << "TWT\n";
                        Pos NeedRefersh = {-1, -1};
                        Consume cs = GetConsume(cur, r[i], b[GetBaseId(cur)], b[GetBaseId(cur)]);
                        r[i].def -= cs.c;
                        if (r[i].def <= 0)
                        {
                            NeedRefersh = RemoveGraphIcon(r[i]);
                            ExpectedScore += r[i].val;
                        }
                        ActionOnBase(cs, b[GetBaseId(cur)]);
                        SetMoveAction(t, cur, r[i], b[GetBaseId(cur)], b[GetBaseId(cur)], cs, NeedRefersh);
                    }
                    else tmpQueue.push(cur);
                }
            }
            while (!tmpQueue.empty())
            {
                PlaneQueue.push(tmpQueue.front());
                tmpQueue.pop();
            }
        }
        for (auto fe:FuelAction[t]) std::cout << "fuel " << fe.id << " " << fe.count << "\n";
        for (auto mis:MissileAction[t]) std::cout << "missile " << mis.id << " " << mis.count << "\n";
        for (auto mv:MoveAction[t]) std::cout << "move " << mv.id << " " << mv.dir << "\n";
        for (auto atk:AttackAction[t]) std::cout << "attack " << atk.id << " " << atk.dir << " " << atk.count << "\n";
        std::cout<<"OK\n";
    }
    std::cerr << "Break Program at: " << BreakTime << "\n";
    std::cerr << "Expected Score is: " << ExpectedScore << "\n";
    return 0;
}

// 下一步改进方向：飞机返航时判断哪个基地最合适，挑选最合适的基地进行降落：
// 最合适的定义：剩余油量 / 距离 * (0.5 ^ 基地内飞机数)