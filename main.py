import queue
import copy
import sys
from contextlib import contextmanager
from utils import extract_base_features

@contextmanager
def open_files():
    train_outs = [open(f"./data/raw_train_data{k}.txt", "w") for k in range(0, NumPlane)]
    try:
        yield train_outs
    finally:
        for train_out in train_outs:
            train_out.close()


K = 15
MaxT = 15000

def get_direct(st, ed):
    if ed['x'] == st['x'] - 1: return 0
    elif ed['x'] == st['x'] + 1: return 1
    elif ed['y'] == st['y'] - 1: return 2
    elif ed['y'] == st['y'] + 1: return 3
    return -1

def get_2_enemy(startpos, s, n, m):
    pos_q = queue.Queue()
    pos_q.put(startpos)
    endpos = {'x': -1, 'y': -1}
    dis = [[-1] * m for _ in range(n)]
    pre = [[{'x': -1, 'y': -1} for _ in range(m)] for _ in range(n)]
    while not pos_q.empty():
        cur = pos_q.get()
        if s[cur['x']][cur['y']] == '#':
            endpos = cur
            break
        for i in range(4):
            ncur = {'x': cur['x'] + dx[i], 'y': cur['y'] + dy[i]}
            if 0 <= ncur['x'] < n and 0 <= ncur['y'] < m and dis[ncur['x']][ncur['y']] < 0:
                dis[ncur['x']][ncur['y']] = dis[cur['x']][cur['y']] + 1
                pre[ncur['x']][ncur['y']] = cur
                pos_q.put(ncur)
    return endpos, dis, pre

def get_2_supply(startpos, s, n, m, pl):
    pos_q = queue.Queue()
    pos_q.put(startpos)
    endpos = {'x': -1, 'y': -1}
    dis = [[-1] * m for _ in range(n)]
    pre = [[{'x': -1, 'y': -1} for _ in range(m)] for _ in range(n)]
    FindCount = 0
    while not pos_q.empty():
        cur = pos_q.get()
        if s[cur['x']][cur['y']] == '*':
            endpos = cur
            break
        for i in range(4):
            ncur = {'x': cur['x'] + dx[i], 'y': cur['y'] + dy[i]}
            if 0 <= ncur['x'] < n and 0 <= ncur['y'] < m and dis[ncur['x']][ncur['y']] < 0:
                dis[ncur['x']][ncur['y']] = dis[cur['x']][cur['y']] + 1
                pre[ncur['x']][ncur['y']] = cur
                pos_q.put(ncur)
    return endpos, dis, pre

def get_move_action(id, pl, s, n, m, fout, FixedDir):
    if pl['gas'] <= 0: return -1
    startpos = {'x': pl['x'], 'y': pl['y']}
    endpos, dis, pre = get_2_enemy(startpos, s, n, m)
    if endpos['x'] < 0 or endpos['y'] < 0: return -1
    if pl['gas'] < dis[endpos['x']][endpos['y']] - 1 or pl['c'] < pl['maxc'] / 2.0:
        curpos, dis, pre = get_2_supply(startpos, s, n, m, pl)
        if curpos['x'] >= 0 and curpos['y'] >= 0:
            endpos = curpos
    cur = copy.deepcopy(endpos)
    if startpos == endpos: return -1
    while pre[cur['x']][cur['y']] != startpos:
        cur = copy.deepcopy(pre[cur['x']][cur['y']])
    if s[cur['x']][cur['y']] == '#': return -1
    dir = get_direct(startpos, cur)
    pl['gas'] -= 1
    pl['x'] = cur['x']
    pl['y'] = cur['y']
    print(f"move {id} {dir}", file=fout)
    return dir

def add_fuel(pid, pl, bid, bases, s, fout):
    add_count = min(bases[bid]['gas'], pl['maxgas'] - pl['gas'])
    print(f"fuel {pid} {add_count}", file=fout)
    pl['gas'] += add_count
    bases[bid]['gas'] -= add_count
    if bases[bid]['gas'] <= 0 and bases[bid]['c'] <= 0:
        s[bases[bid]['x']] = s[bases[bid]['x']][:bases[bid]['y']] + '.' + s[bases[bid]['x']][bases[bid]['y'] + 1:]

def add_missile(pid, pl, bid, bases, s, fout):
    add_count = min(bases[bid]['c'], pl['maxc'] - pl['c'])
    print(f"missile {pid} {add_count}", file=fout)
    pl['c'] += add_count
    bases[bid]['c'] -= add_count
    if bases[bid]['gas'] <= 0 and bases[bid]['c'] <= 0:
        s[bases[bid]['x']] = s[bases[bid]['x']][:bases[bid]['y']] + '.' + s[bases[bid]['x']][bases[bid]['y'] + 1:]

def get_attack_action(pid, pl, s, base_id, bases, fout):
    cur = {'x': pl['x'], 'y': pl['y']}
    Reward = 0
    for i in range(4):
        ncur = {'x': cur['x'] + dx[i], 'y': cur['y'] + dy[i]}
        if 0 <= ncur['x'] < n and 0 <= ncur['y'] < m and s[ncur['x']][ncur['y']] == '#':
            bid = base_id[ncur['x']][ncur['y']]
            attack_count = min(bases[bid]['def'], pl['c'])
            print(f"attack {pid} {i} {attack_count}", file=fout)
            bases[bid]['def'] -= attack_count
            pl['c'] -= attack_count
            if bases[bid]['def'] <= 0:
                s[ncur['x']] = s[ncur['x']][:ncur['y']] + '.' + s[ncur['x']][ncur['y'] + 1:]
                Reward += bases[bid]['val']
    return Reward

if __name__ == "__main__":
    TotalScore = 0
    testcase_number = sys.argv[1]
    FixedDir = sys.argv[2]
    with (open(f"./input/testcase{testcase_number}.in", "r") as fin):
        n, m = map(int, fin.readline().split())
        s = [fin.readline().strip() for _ in range(n)]

        NumBaseBlue = int(fin.readline())
        bases = []
        base_id = [[-1] * m for _ in range(n)]
        for _ in range(NumBaseBlue):
            x, y = map(int, fin.readline().split())
            gas, c, defense, val = map(int, fin.readline().split())
            bases.append({'content': '*', 'x': x, 'y': y, 'gas': gas, 'c': c, 'def': defense, 'val': val})
            base_id[x][y] = len(bases) - 1

        NumBaseRed = int(fin.readline())
        for _ in range(NumBaseRed):
            x, y = map(int, fin.readline().split())
            gas, c, defense, val = map(int, fin.readline().split())
            bases.append({'content': '*', 'x': x, 'y': y, 'gas': gas, 'c': c, 'def': defense, 'val': val})
            TotalScore += val
            base_id[x][y] = len(bases) - 1

        NumPlane = int(fin.readline())
        planes = [{'x': 0, 'y': 0, 'maxgas': 0, 'maxc': 0, 'gas': 0, 'c': 0} for _ in range(K)]
        for _ in range(NumPlane):
            x, y, maxgas, maxc = map(int, fin.readline().split())
            planes[_] = {'x': x, 'y': y, 'maxgas': maxgas, 'maxc': maxc, 'gas': 0, 'c': 0}

    dx = [-1, 1, 0, 0]
    dy = [0, 0, -1, 1]

    dis = [[-1] * m for _ in range(n)]
    pre = [[{'x': -1, 'y': -1} for _ in range(m)] for _ in range(n)]
    ExpectedScore = 0

    with open(f"./output/testcase{testcase_number}.out", "w") as fout:
        with open_files() as train_outs:
            for t in range(MaxT):
                if t % 1000 == 0:
                    print(f"Run at: {t}")
                    print(f"Expected Score is: {ExpectedScore}")
                for k, train_out in enumerate(train_outs):
                    if ExpectedScore == TotalScore:
                        continue
                    if s[planes[k]['x']][planes[k]['y']] == '*':
                        bid = base_id[planes[k]['x']][planes[k]['y']]
                        add_fuel(k, planes[k], bid, bases, s, fout)
                        add_missile(k, planes[k], bid, bases, s, fout)
                    move = get_move_action(k, planes[k], s, n, m, fout, FixedDir)
                    Reward = get_attack_action(k, planes[k], s, base_id, bases, fout)
                    features = extract_base_features(planes[k], bases)
                    ExpectedScore += Reward
                    print(f"{move} {Reward} {features}", file=train_out)
                print("OK", file=fout)
            print(f"Break Program at: {MaxT}")
            print(f"Total Expected Score is: {ExpectedScore}")
            print(f"Score cleared all bases is: {TotalScore}")