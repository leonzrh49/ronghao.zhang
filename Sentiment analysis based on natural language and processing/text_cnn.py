import tensorflow as tf
import numpy as np


class TextCNN(object):
    """
    A CNN for text classification.
    Uses an embedding layer, followed by a convolutional, max-pooling and softmax layer.
    """
    def __init__(
      self, sequence_length, num_classes, vocab_size,
      embedding_size, filter_sizes, num_filters, l2_reg_lambda=0.0):    # filters_sizes是指filter每次处理几个单词，num_filters是指每个尺寸的处理包含几个filter

        # Placeholders for input, output and dropout
        self.input_x = tf.placeholder(tf.int32, [None, sequence_length], name="input_x")  # 此函数可以理解为形参，用于定义过程，在执行的时候再赋具体的值
        self.input_y = tf.placeholder(tf.float32, [None, num_classes], name="input_y")
        self.dropout_keep_prob = tf.placeholder(tf.float32, name="dropout_keep_prob")
        # Keeping track of l2 regularization loss (optional)
        l2_loss = tf.constant(0.0)     # l2正则化的loss先定义为0.0

        # Embedding layer  将词语转化为向量表示
        with tf.device('/cpu:0'), tf.name_scope("embedding"):   # 在CPU下，命名空间为embedding
            self.W = tf.Variable(
                tf.random_uniform([vocab_size, embedding_size], -1.0, 1.0),    # 从均匀分布中返回随机值，-1到1
                name="W")
            self.embedded_chars = tf.nn.embedding_lookup(self.W, self.input_x)     # 在W中根据输入的x找到对应的元素
            self.embedded_chars_expanded = tf.expand_dims(self.embedded_chars, -1)  # 增加维度

        # Create a convolution + maxpool layer for each filter size  一共有三种类型的filter，每种类型有两个
            # 需要迭代每个filter去处理输入矩阵，将最终得到的所有结果合并为一个大的特征向量
        pooled_outputs = []
        for i, filter_size in enumerate(filter_sizes):
            with tf.name_scope("conv-maxpool-%s" % filter_size):
                # Convolution Layer   卷积层
                filter_shape = [filter_size, embedding_size, 1, num_filters]
                W = tf.Variable(tf.truncated_normal(filter_shape, stddev=0.1), name="W")    # filter 矩阵 tf.truncated_normal在正态分布中生成随机值
                b = tf.Variable(tf.constant(0.1, shape=[num_filters]), name="b")
                conv = tf.nn.conv2d(
                    self.embedded_chars_expanded,
                    W,
                    strides=[1, 1, 1, 1],      # 滑动的距离 [1, width, height, 1]
                    padding="VALID",           # 使用的是‘VALID’，所以output的尺寸为[1, sequence_length - filter_size + 1, 1, 1]
                    name="conv")           # tf.nn.conv2d 是卷及操作函数
                # Apply nonlinearity
                h = tf.nn.relu(tf.nn.bias_add(conv, b), name="relu")        # 激活函数 relu：f(x) = max(0, x) 大于0为原值，小于0变0   激活函数relu,sigmod,tanh
                # Maxpooling over the outputs   选出最大的一个，经过这一层的output尺寸为 [batch_size, 1, 1, num_filters]
                pooled = tf.nn.max_pool(       # 实现最大池化
                    h,
                    ksize=[1, sequence_length - filter_size + 1, 1, 1],
                    strides=[1, 1, 1, 1],
                    padding='VALID',
                    name="pool")
                pooled_outputs.append(pooled)

        # Combine all the pooled features
        num_filters_total = num_filters * len(filter_sizes)
        self.h_pool = tf.concat(pooled_outputs, 3)  # tf.concat拼接张量
        self.h_pool_flat = tf.reshape(self.h_pool, [-1, num_filters_total])  # tf.reshape 函数的作用是将tensor变换为参数shape形式，其中的shape为一个列表形式

        # Add dropout
        with tf.name_scope("dropout"):
            self.h_drop = tf.nn.dropout(self.h_pool_flat, self.dropout_keep_prob)

        # Final (unnormalized) scores and predictions    # 分数和预测值
        with tf.name_scope("output"):
            W = tf.get_variable(
                "W",
                shape=[num_filters_total, num_classes],
                initializer=tf.contrib.layers.xavier_initializer())      # 初始化权重，该函数返回一个用于初始化权重的初始化程序 “Xavier”，
            b = tf.Variable(tf.constant(0.1, shape=[num_classes]), name="b")
            l2_loss += tf.nn.l2_loss(W)
            l2_loss += tf.nn.l2_loss(b)
            self.scores = tf.nn.xw_plus_b(self.h_drop, W, b, name="scores")   # xw+b
            self.predictions = tf.argmax(self.scores, 1, name="predictions")    # 取最大的作为预测

        # Calculate mean cross-entropy loss  交叉熵，衡量不确定度  信息熵，定义一个信息的不确定度
        with tf.name_scope("loss"):
            losses = tf.nn.softmax_cross_entropy_with_logits(logits=self.scores, labels=self.input_y)    # 返回一个向量
            self.loss = tf.reduce_mean(losses) + l2_reg_lambda * l2_loss    # 计算所有得到的元素之合作为损失值

        # Accuracy
        with tf.name_scope("accuracy"):
            correct_predictions = tf.equal(self.predictions, tf.argmax(self.input_y, 1))   # 比较预测值和结果是否相等
            self.accuracy = tf.reduce_mean(tf.cast(correct_predictions, "float"), name="accuracy")    # 准确度计算
