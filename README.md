# sentence_trunk_exteaction
---
句子主干提取
URL: http://zhangxiaoming.com.cn/2016/07/05/%E5%88%A9%E7%94%A8%E6%A0%87%E6%B3%A8%E8%AF%AD%E6%96%99%E7%94%9F%E6%88%90%E5%86%B3%E7%AD%96weights/
---
# get_data
获取需要训练语料中的特征数据比如词性、聚类关系、句法树依存关系及词与词之间的位置关系
# train
1.利用逻辑回归分类模型使用主动学习的思想标注语料
2.训练获取特征权重。
# test
1.对新输入的句子提取每个词的特征利用权重weight决定是否保留
2.预测结果为正的词为主干词
