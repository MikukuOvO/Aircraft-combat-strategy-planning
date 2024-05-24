#!/bin/bash

# 获取测试用例编号和其它参数
testcase_number=$1
dir=$2

# 将参数传递给 Python 脚本
python main.py "$testcase_number" "$dir"
