#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <vector>
#include <chrono>
#include <random>

const int N = 5005;
const int K = 15;
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

std::map<Pos, int> Planedis[K];
std::map<Pos, Pos> Planepre[K];

void GetPlaneDis(int id, Base srcbase)
{
    Pos src = {srcbase.x, srcbase.y};
    DisQueue.push(src);
    Planedis[id][src] = 0;
    while (!DisQueue.empty())
    {
        Pos cur = DisQueue.front();
        DisQueue.pop();
        for (int i = 0; i < 4; ++i)
        {
            Pos ncur = {cur.x + dx[i], cur.y + dy[i]};
            if (ncur.x < 0 || ncur.x >= n || ncur.y < 0 || ncur.y >= m) continue;
            if (Planedis[id].find(ncur) != Planedis[id].end()) continue;
            Planedis[id][ncur] = Planedis[id][cur] + 1;
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
int GetPlaneId(Pos cur)
{
    for (int i = 0; i < NumPlane; ++i)
    {
        if (pe[i].x == cur.x && pe[i].y == cur.y) return i;
    }
    return -1;
}
int GetDis(Pos src, Pos dst)
{
    int id = GetPlaneId(src);
    if (Planedis[id].find(dst) == Planedis[id].end()) return 1e9;
    return Planedis[id][dst];
}
bool CheckPlaneCanGo(Plane p, Base gd, Base gs)
{
    Pos src = {p.x, p.y};
    Pos dst = {gd.x, gd.y};
    int dis = GetDis(src, dst) - 1;
    if (dis * 2 + 1 <= gs.gas && dis * 2 + 1 <= p.maxgas) return true;
    return false;
}
Consume GetConsume(Plane p, Base gd, Base gs)
{
    Pos src = {p.x, p.y};
    Pos dst = {gd.x, gd.y};
    int dis = GetDis(src, dst) - 1;
    int cc = std::max(std::min({p.maxc, gs.c, gd.def}) - p.c, 0);
    int cg = std::max(dis * 2 + 1 - p.gas, 0);
    return {cg, cc};
}
void RemoveGraphIcon(Base cur)
{
    s[cur.x][cur.y] = '.';
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
void SetMoveAction(int t, Plane p, Base gd, Base gs, Consume cs)
{
    Pos src = {p.x, p.y};
    Pos dst = {gd.x, gd.y};
    Pos cur = dst;
    int id = GetPlaneId(src);
    // std::cerr << id<<"\n";
    while (cur != src)
    {
        if (cur != dst)
        {
            MoveAction[t + Planedis[id][Planepre[id][cur]]].push_back({p.id, GetDir(Planepre[id][cur], cur)});
            MoveAction[t + 2 * (Planedis[id][dst] - 1) - Planedis[id][Planepre[id][cur]]].push_back({p.id, GetDir(cur, Planepre[id][cur])});
        }
        cur = Planepre[id][cur];
        // if (cur.x != 0 || cur.y != 0)std::cerr << cur.x <<" "<< cur.y <<"\n";
    }
    AttackAction[t + Planedis[id][dst] - 1].push_back({p.id, GetDir(Planepre[id][dst], dst), cs.c});
    FuelAction[t].push_back({p.id, cs.gas});
    MissileAction[t].push_back({p.id, cs.c});
    BackPlane[t + (Planedis[id][dst] - 1) * 2 + 1].push_back(p);
}
int main()
{
    freopen("../testcase3.in", "r", stdin);
    freopen("../testcase3.out", "w", stdout);
    auto start = std::chrono::high_resolution_clock::now();
    std::cin >> n >> m;
    for (int i = 0; i < n; ++i) std::cin >> s[i];
    std::cin >> NumBaseBlue;
    for (int i = 0; i < NumBaseBlue; ++i)
    {
        std::cin >> b[i].x >> b[i].y;
        std::cin >> b[i].gas >> b[i].c >> b[i].def >> b[i].val;
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
    for (int k = 0; k < NumPlane; ++k) GetPlaneDis(k, b[GetBaseId(pe[k])]);
    int BreakTime = MaxT;
    int ExpectedScore = 0;
    for (int t = 0; t < MaxT; ++t)
    {
        std::cerr << "Running on Time: " << t << "\n";
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = now - start;
        if (elapsed.count() >= 120) {
            BreakTime = std::min(BreakTime, t);
            std::cout << "OK\n";
            continue;
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
                if (CheckPlaneCanGo(cur, r[i], b[GetBaseId(cur)]))
                {
                    // std::cerr << "TWT\n";
                    Consume cs = GetConsume(cur, r[i], b[GetBaseId(cur)]);
                    r[i].def -= cs.c;
                    if (r[i].def <= 0)
                    {
                        RemoveGraphIcon(r[i]);
                        ExpectedScore += r[i].val;
                    }
                    ActionOnBase(cs, b[GetBaseId(cur)]);
                    SetMoveAction(t, cur, r[i], b[GetBaseId(cur)], cs);
                }
                else tmpQueue.push(cur);
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