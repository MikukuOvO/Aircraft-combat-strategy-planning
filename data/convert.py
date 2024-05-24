import csv

# 从文本文件读取输入数据
with open('raw_train_data0.txt', 'r') as file:
    lines = file.readlines()

# 处理每一行数据，并写入CSV文件
with open('train_move_0.csv', 'w', newline='') as move_file, \
     open('train_reward_0.csv', 'w', newline='') as reward_file, \
     open('train_features_0.csv', 'w', newline='') as features_file:

    move_writer = csv.writer(move_file)
    reward_writer = csv.writer(reward_file)
    features_writer = csv.writer(features_file)

    for line in lines:
        # 去除换行符并拆分每一行数据
        parts = line.strip().split()
        # 提取动作和奖励值
        move = parts[0]
        reward = parts[1]
        # 提取特征值列表并将字符串转换为整数
        features = [int(value) for value in parts[2].strip('[]').split(', ')]

        # 将动作、奖励和特征写入CSV文件
        move_writer.writerow([move])
        reward_writer.writerow([reward])
        features_writer.writerow(features)

print("CSV 文件已生成。")
