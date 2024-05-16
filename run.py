import subprocess
import random
from tqdm import tqdm

# 设置训练次数
NUM_EXPERIMENTS = 20
LOCAL_SEARCH_ITER = 20
STEP_SIZE = 0.2

# 生成不同参数组合
def generate_random_parameters(best_parameters=None, step_size=STEP_SIZE):
    if best_parameters:
        rand_factor1 = max(0.5, min(2.0, best_parameters[0] + random.uniform(-step_size, step_size)))
        rand_factor2 = max(0.5, min(2.0, best_parameters[1] + random.uniform(-step_size, step_size)))
        gas_weight = max(0.5, min(2.0, best_parameters[2] + random.uniform(-step_size, step_size)))
        c_weight = max(0.5, min(2.0, best_parameters[3] + random.uniform(-step_size, step_size)))
        red_base_value_weight = max(0.5, min(2.0, best_parameters[4] + random.uniform(-step_size, step_size)))
    else:
        rand_factor1 = random.uniform(0.5, 2.0)
        rand_factor2 = random.uniform(0.5, 2.0)
        gas_weight = random.uniform(0.5, 2.0)
        c_weight = random.uniform(0.5, 2.0)
        red_base_value_weight = random.uniform(0.5, 2.0)

    return rand_factor1, rand_factor2, gas_weight, c_weight, red_base_value_weight

# 更新C++代码中的参数
def update_cpp_code(rand_factor1, rand_factor2, gas_weight, c_weight, red_base_value_weight):
    with open("params.cpp", "w") as file:
        file.write(f"""
#include "params.h"

double rand_factor1 = {rand_factor1};
double rand_factor2 = {rand_factor2};
double gas_weight = {gas_weight};
double c_weight = {c_weight};
double red_base_value_weight = {red_base_value_weight};
""")

# 运行C++程序并获取结果
def run_cpp_program(mode=0):
    # 编译C++代码
    compile_result = subprocess.run(["g++", "-std=c++11", "-o", "main", "main.cpp", "params.cpp"], capture_output=True, text=True)
    if compile_result.returncode != 0:
        print("Error in compilation:")
        print(compile_result.stderr)
        return None

    # 运行编译后的程序
    run_result = subprocess.run(["./main", str(mode)], capture_output=True, text=True)
    if run_result.returncode != 0:
        print("Error in execution:")
        print(run_result.stderr)
        return None

    # 解析结果中的Expected Score
    output = run_result.stderr
    for line in output.split('\n'):
        if "Expected Score is:" in line:
            score = int(line.split()[-1])
            return score
    return None

# 训练过程
def train():
    best_score = -1
    best_parameters = None

    for i in tqdm(range(NUM_EXPERIMENTS)):
        if best_parameters and i > LOCAL_SEARCH_ITER:
            parameters = generate_random_parameters(best_parameters)
        else:
            parameters = generate_random_parameters()

        rand_factor1, rand_factor2, gas_weight, c_weight, red_base_value_weight = parameters

        print(f"Experiment {i + 1}: rand_factor1={rand_factor1}, rand_factor2={rand_factor2}, gas_weight={gas_weight}, c_weight={c_weight}, red_base_value_weight={red_base_value_weight}")

        update_cpp_code(rand_factor1, rand_factor2, gas_weight, c_weight, red_base_value_weight)

        score = run_cpp_program(mode=0)
        if score is None:
            continue

        print(f"Score: {score} with parameters: {parameters}")

        if score > best_score:
            best_score = score
            best_parameters = parameters

    print(f"Best score: {best_score} with parameters: {best_parameters}")
    return best_parameters

# 评估过程
def evaluate(parameters):
    rand_factor1, rand_factor2, gas_weight, c_weight, red_base_value_weight = parameters
    update_cpp_code(rand_factor1, rand_factor2, gas_weight, c_weight, red_base_value_weight)
    score = run_cpp_program(mode=1)
    print(f"Evaluation Score: {score} with parameters: {parameters}")
    return score

if __name__ == "__main__":
    best_parameters = train()
    if best_parameters:
        evaluate(best_parameters)
