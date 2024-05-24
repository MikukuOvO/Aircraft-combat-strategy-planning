import math

def extract_base_features(plane, bases):
    num_bases = len(bases)
    num_features_per_base = 6  # 基地的内容，剩余油量，剩余弹量，到目前飞机位置的距离，防御值，价值
    total_features = num_bases * num_features_per_base
    features = [0] * total_features

    for i, base in enumerate(bases):
        idx = i * num_features_per_base
        features[idx] = 0 if base['content'] == '*' else 1  # 基地的内容
        features[idx + 1] = base['gas']  # 剩余油量
        features[idx + 2] = base['c']  # 剩余弹量
        dist = abs(plane['x'] - base['x']) + abs(plane['y'] - base['y'])
        features[idx + 3] = dist  # 到当前飞机位置的距离
        features[idx + 4] = base['def']  # 防御值
        features[idx + 5] = base['val']  # 价值

    return features
