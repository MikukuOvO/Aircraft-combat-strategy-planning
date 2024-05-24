import os

# 编译C++代码
os.system("g++ -std=c++17 main.cpp -o main")

# 调用编译后的C++程序，并传入参数
os.system("./main 1")  # 这里传入参数 1，你可以根据需要传入 0，1，2，3

print("Data generation completed.")
