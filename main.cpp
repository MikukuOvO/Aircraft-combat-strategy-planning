#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <vector>
#include <string.h>
#include <chrono>
#include <random>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>

const int N = 4005;
const int M = 205;
const int K = 15;
const int MaxT = 15000;

int n, m;
int NumBaseBlue, NumBaseRed;
int NumPlane;
int TotalScore, ExpectedScore;
int parameter;
std::string s[M];

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
    int x, y;
    int gas, c;
    int maxgas, maxc;
};
Plane p[K];
int dis[M][M], BaseId[M][M];
Pos pre[M][M];

struct Feature
{
    int gas, c, dis;
};

int dx[] = {-1, 1, 0, 0};
int dy[] = {0, 0, -1, 1};

int GetDirect(Pos st, Pos ed){
    if (ed.x == st.x - 1) return 0;
    else if (ed.x == st.x + 1) return 1;
    else if (ed.y == st.y - 1) return 2;
    else if (ed.y == st.y + 1) return 3;
    return -1;
}
Pos Get2Enemy(Pos startpos)
{
    std::queue<Pos>PosQ;
    PosQ.push(startpos);
    Pos endpos = {-1, -1};
    memset(dis, -1, sizeof(dis));
    while (!PosQ.empty())
    {
        Pos cur = PosQ.front();
        PosQ.pop();
        if (s[cur.x][cur.y] == '#')
        {
            endpos = cur;
            break;
        }
        for (int i = 0; i < 4; ++i) {
            Pos ncur = {cur.x + dx[i], cur.y + dy[i]};
            if (ncur.x < 0 || ncur.x >= n || ncur.y < 0 || ncur.y >= m) continue;
            if (dis[ncur.x][ncur.y] < 0)
            {
                dis[ncur.x][ncur.y] = dis[cur.x][cur.y] + 1;
                pre[ncur.x][ncur.y] = cur;
                PosQ.push(ncur);
            }
        }
    }
    return endpos;
}
Pos Get2Supply(Pos startpos, Plane pl, Pos targetpos)
{
    // 先计算需要多少油或者多少导弹
    int delta = pl.gas - dis[targetpos.x][targetpos.y];
    std::queue<Pos>PosQ;
    PosQ.push(startpos);
    Pos endpos = {-1, -1};
    double MaxScore = -1e9;
    memset(dis, -1, sizeof(dis));
    int FindSupplyCount = 0;
    std::vector<Feature> feat;
    int maxval = -1;
    while (!PosQ.empty())
    {
        Pos cur = PosQ.front();
        PosQ.pop();
        if (s[cur.x][cur.y] == '*')
        {
            int bid = BaseId[cur.x][cur.y];
            ++FindSupplyCount;
            if (b[bid].gas * b[bid].c > maxval) endpos = cur, maxval = b[bid].gas * b[bid].c;
            if (FindSupplyCount >= 20) break;
        }
        for (int i = 0; i < 4; ++i) {
            Pos ncur = {cur.x + dx[i], cur.y + dy[i]};
            if (ncur.x < 0 || ncur.x >= n || ncur.y < 0 || ncur.y >= m) continue;
            if (dis[ncur.x][ncur.y] < 0)
            {
                dis[ncur.x][ncur.y] = dis[cur.x][cur.y] + 1;
                pre[ncur.x][ncur.y] = cur;
                if (dis[ncur.x][ncur.y] <= pl.gas) PosQ.push(ncur);
            }
        }
    }
    return endpos;
}
void GetMoveAction(int id, Plane &pl)
{
    if (pl.gas <= 0) return;
    Pos startpos = {pl.x, pl.y};
    Pos endpos = Get2Enemy(startpos);
    if (endpos.x < 0 || endpos.y < 0) return;
    if (pl.gas < dis[endpos.x][endpos.y]|| pl.c < r[BaseId[endpos.x][endpos.y]].def) // Considering modify
    {
        Pos curpos = Get2Supply(startpos, pl, endpos);
        if (curpos.x >= 0 && curpos.y >= 0) endpos = curpos;
    }
    Pos cur = endpos;
    while (pre[cur.x][cur.y] != startpos) cur = pre[cur.x][cur.y];
    if (s[cur.x][cur.y] == '#') return;
    int dir = GetDirect(startpos, cur);
    --pl.gas;
    pl.x = cur.x, pl.y = cur.y;
    std::cout << "move " << id << " " << dir << "\n";
}
void AddFuel(int pid, Plane &pl, int bid)
{
    int add_count = std::min(b[bid].gas, pl.maxgas - pl.gas);
    std::cout << "fuel " << pid << " " << add_count << "\n";
    pl.gas += add_count;
    b[bid].gas -= add_count;
    if (b[bid].gas <= 0 && b[bid].c <= 0) s[b[bid].x][b[bid].y] = '.';
}
void AddMissile(int pid, Plane &pl, int bid)
{
    int add_count = std::min(b[bid].c, pl.maxc - pl.c);
    std::cout << "missile " << pid << " " << add_count << "\n";
    pl.c += add_count;
    b[bid].c -= add_count;
    if (b[bid].gas <= 0 && b[bid].c <= 0) s[b[bid].x][b[bid].y] = '.';
}
void GetAttackAction(int pid, Plane &pl)
{
    Pos cur = {pl.x, pl.y};
    for (int i = 0; i < 4; ++i) {
        Pos ncur = {cur.x + dx[i], cur.y + dy[i]};
        if (ncur.x < 0 || ncur.x >= n || ncur.y < 0 || ncur.y >= m) continue;
        if (s[ncur.x][ncur.y] == '#')
        {
            int bid = BaseId[ncur.x][ncur.y];
            int attack_count = std::min(r[bid].def, pl.c);
            std::cout << "attack " << pid << " " << i << " " << attack_count << "\n";
            r[bid].def -= attack_count;
            pl.c -= attack_count;
            if (r[bid].def <= 0)
            {
                s[ncur.x][ncur.y] = '.';
                ExpectedScore += r[bid].val;
            }
        }
    }
}
int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    std::cin >> n >> m;
    for (int i = 0; i < n; ++i) std::cin >> s[i];
    std::cin >> NumBaseBlue;
    memset(BaseId, -1, sizeof(BaseId));
    for (int i = 0; i < NumBaseBlue; ++i)
    {
        std::cin >> b[i].x >> b[i].y;
        std::cin >> b[i].gas >> b[i].c >> b[i].def >> b[i].val;
        BaseId[b[i].x][b[i].y] = i;
    }
    std::cin >> NumBaseRed;
    for (int i = 0; i < NumBaseRed; ++i)
    {
        std::cin >> r[i].x >> r[i].y;
        std::cin >> r[i].gas >> r[i].c >> r[i].def >> r[i].val;
        BaseId[r[i].x][r[i].y] = i;
        TotalScore += r[i].val;
    }
    std::cin >> NumPlane;
    for (int i = 0; i < NumPlane; ++i)
    {
        std::cin >> p[i].x >> p[i].y;
        std::cin >> p[i].maxgas >> p[i].maxc;
        p[i].gas = 0, p[i].c = 0;
    }

    int BreakTime = MaxT;
    for (int t = 0; t < MaxT; ++t)
    {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = now - start;
        if (t % 1000 == 0) {
            std::cerr << "Running on Epochs: " << t << ". The time now is: " << elapsed.count() << "seconds" << ". ";
            std::cerr << "The Expected Score now is: " << ExpectedScore << "\n";
        }
        for (int k = 0; k < NumPlane; ++k) {
            int bid = BaseId[p[k].x][p[k].y];
            if (s[p[k].x][p[k].y] == '*'){
                AddFuel(k, p[k], bid);
                AddMissile(k, p[k], bid);
            }
            GetMoveAction(k, p[k]);
            GetAttackAction(k ,p[k]);
        }
        std::cout<<"OK\n";
    }
    std::cerr << "Break Program at: " << BreakTime << "\n";
    std::cerr << "Expected Score is: " << ExpectedScore << "\n";
    std::cerr << "Total Score is: " << TotalScore << "\n";
    return 0;
}