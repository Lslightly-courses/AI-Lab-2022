from collections import defaultdict
from logging import fatal
import math
from typing import Tuple
import numpy as np

class DecisionTree:
    class Node:
        def __init__(self, parent=None):
            self.parent = parent
            self.children = []
            self.label = 1
            self.value = -1
            self.attr = -1
        def add_child(self, node):
            self.children.append(node)
        def find_child(self, value):
            for child in self.children:
                if child.get_value() == value:
                    return child
            return None
        def is_leave(self):
            return len(self.children) == 0
        def set_label(self, label):
            self.label = label
        def get_label(self):
            return self.label
        def set_value(self, value):
            self.value = value
        def get_value(self):
            return self.value
        def set_attr(self, attr):
            self.attr = attr
        def get_attr(self):
            return self.attr
    attrs = [i for i in range(0, 9)]
    attr2value_set = defaultdict(lambda: set())
    root: Node = None

    def fit(self, train_features, train_labels):
        '''
        TODO: 实现决策树学习算法.
        train_features是维度为(训练样本数,属性数)的numpy数组
        train_labels是维度为(训练样本数, )的numpy数组
        '''
        self.getFeatureValueDict(train_features)
        self.root = self.treeGenerate(train_features, train_labels, set(self.attrs), None)
    

    def predict(self, test_features):
        '''
        TODO: 实现决策树预测.
        test_features是维度为(测试样本数,属性数)的numpy数组
        该函数需要返回预测标签，返回一个维度为(测试样本数, )的numpy数组
        '''
        test_labels = np.ones((len(test_features),))
        for i, features in enumerate(test_features):
            test_labels[i] = self.predictForFeatures(features)
        return test_labels
    
    def predictForFeatures(self, features):
        node = self.root
        while True:
            if node.is_leave():
                return node.get_label()
            attr = node.get_attr()
            value = features[attr]
            child = node.find_child(value)
            if child is None:
                fatal("impossible")
                return node.get_label()
            else:
                node = child


    def getFeatureValueDict(self, train_features):
        for features in train_features:
            for attr in self.attrs:
                self.attr2value_set[attr].add(features[attr])
    
    def treeGenerate(self, train_features, train_labels, attr_set, parent):
        node = DecisionTree.Node(parent)
        same, label = self.allSameLabel(train_labels)
        if same:
            node.set_label(label)
            node.set_value(-1)
            return node
        max_count_train_label = self.maxCountLabel(train_labels)
        if len(attr_set) == 0 or self.allSameValueInAttrSet(train_features, attr_set):
            node.set_label(max_count_train_label)
            node.set_value(-1)
            return node
        opt_attr, info_gain_ok = self.optimalAttr(train_features, train_labels, attr_set)
        if not info_gain_ok:
            node.set_label(max_count_train_label)
            node.set_value(-1)
            return node
        node.set_attr(opt_attr) # 表示子节点依据opt_attr进行划分
        value2features, value2labels = self.divideByAttr(train_features, train_labels, opt_attr)    # 按照值进行划分
        for value in self.attr2value_set[opt_attr]:
            features = value2features[value]
            labels = value2labels[value]
            if len(labels) == 0:    # 某个值在该次划分中没有出现
                branch = DecisionTree.Node(node)
                branch.set_label(max_count_train_label)
                branch.set_value(value)
                node.add_child(branch)
            else:
                new_attr_set = attr_set.copy()
                new_attr_set.remove(opt_attr)
                branch = self.treeGenerate(features, labels, new_attr_set, node)
                branch.set_value(value)
                node.add_child(branch)
        node.set_label(max_count_train_label)   # 如果测试集中有一些特征值不在训练集中出现，则按照最多的label标记该节点，预测时使用该值
        return node


        
    def divideByAttr(self, train_features, train_labels, attr) -> Tuple[dict, dict]:
        value2features = defaultdict(lambda: [])
        value2labels = defaultdict(lambda: [])
        for features, label in zip(train_features, train_labels):
            value2features[features[attr]].append(features)
            value2labels[features[attr]].append(label)
        return value2features, value2labels

    def I(p_count, n_count):
        p_prob = p_count*1.0/(p_count+n_count)
        n_prob = n_count*1.0/(p_count+n_count)
        if p_count == 0 and n_count == 0:
            fatal("p_count == 0 and n_count == 0, impossible!")
        elif p_count == 0:
            return -n_prob*math.log2(n_prob)
        elif n_count == 0:
            return -p_prob*math.log2(p_prob)
        else:
            return -p_prob*math.log2(p_prob)-n_prob*math.log2(n_prob)
    
    def printPN(self, p_count, n_count):
        print("p_count: {}, n_count: {}, total_count: {}".format(p_count, n_count, p_count+n_count))
    
    def optimalAttr(self, train_features, train_labels, attr_set) -> Tuple[int, bool]:
        """按照信息增益进行划分

        Args:
            train_features : 上一次按照某个属性的某个值划分之后得到的数据集
            train_labels (_type_): 上一次按照某个属性的某个值划分之后得到的数据标签
            attr_set (_type_): 仍未划分的属性

        Returns:
            Tuple[int, bool]: 第一个返回值为进一步划分的最优属性，第二个返回值表示是否需要进一步划分，若按照剩余属性进一步划分之后信息增益都为负数，则不需要进行进一步划分，返回False,否则返回True
        """
        max_IG = 0  # 最大信息增益
        opt_attr = -1   # 最优属性，初始化为-1,如果在attr_set划分之后信息增益都为负数，即没有信息增益，则opt_attr仍然等于-1
        label_count_dict = self.countLabel(train_labels)
        p_count = label_count_dict[1]
        n_count = label_count_dict[0]
        origin_I = DecisionTree.I(p_count, n_count) # I(p/(p+n), n/(p+n))划分前的熵
        train_set_count = len(train_labels)
        for attr in attr_set:
            value2features, value2labels = self.divideByAttr(train_features, train_labels, attr)    # 按照属性划分之后，各个值下的样本和标签字典
            I_after_divide = 0  # 划分之后的熵
            total_count = 0
            for value in value2features.keys():
                label_count_dict = self.countLabel(value2labels[value])
                p_count = label_count_dict[1]
                n_count = label_count_dict[0]
                total_count += p_count + n_count
                I_after_divide += (p_count+n_count)*1.0/train_set_count*DecisionTree.I(p_count, n_count)
            if total_count != train_set_count:
                fatal("something is miss when divide by attr")
            # print("attr {} calc IG={}".format(attr, origin_I-I_after_divide))
            if origin_I-I_after_divide > max_IG:
                opt_attr = attr
                max_IG = origin_I-I_after_divide
        if opt_attr == -1:  # 按照attr_set中属性进行划分后，信息增益均为负数，不做进一步划分
            # print('no info gain')
            return None, False
        else:
            # print('choose {}'.format(opt_attr))
            return opt_attr, True

    def allSameLabel(self, train_labels) -> Tuple[bool, int]:
        '''
            是否属于同一个样本
        '''
        init_label = train_labels[0]
        for label in train_labels:
            if label != init_label:
                return False, label
        return True, init_label
    
    def allSameValueInAttrSet(self, features, attr_set) -> bool:
        '''
            是否在特征上为相同值
        '''
        init_value_list = self.featureValueInAttrSet(features[0], attr_set)
        for feature in features:
            value_list = self.featureValueInAttrSet(feature, attr_set)
            if value_list != init_value_list:
                return False
        return True
    
    def countLabel(self, train_labels) -> dict:
        label_count_dict = defaultdict(int)
        for label in train_labels:
            label_count_dict[label] += 1
        return label_count_dict
    
    def maxCountLabel(self, train_labels) -> int:
        '''
            样本数最多的类
        '''
        label_count_dict = self.countLabel(train_labels)
        return max(label_count_dict, key=label_count_dict.get)
            

    def featureValueInAttrSet(self, features, attr_set):
        return [features[attr] for attr in attr_set]


# treenode: [attr, feat[attr] == 1, feat[attr] == 2, feat[attr] == 3]
