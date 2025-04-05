import numpy as np
import cvxpy

class SupportVectorMachine:
    def __init__(self, C=1, kernel='Linear', epsilon=1e-4):
        self.C = C
        self.epsilon = epsilon
        self.kernel = kernel

        # Hint: 你可以在训练后保存这些参数用于预测
        # SV即Support Vector，表示支持向量，SV_alpha为优化问题解出的alpha值，
        # SV_label表示支持向量样本的标签。
        self.SV = None
        self.SV_alpha = None
        self.SV_label = None
        self.b = None

    def KERNEL(self, x1, x2, d=2, sigma=1):
        #d for Poly, sigma for Gauss
        if self.kernel == 'Gauss':
            K = np.exp(-(np.sum((x1 - x2) ** 2)) / (2 * sigma ** 2))
        elif self.kernel == 'Linear':
            K = np.dot(x1,x2)
        elif self.kernel == 'Poly':
            K = (np.dot(x1,x2) + 1) ** d
        else:
            raise NotImplementedError()
        return K
    
    def fit(self, train_data, train_label):
        '''
        TODO：实现软间隔SVM训练算法
        train_data：训练数据，是(N, 7)的numpy二维数组，每一行为一个样本
        train_label：训练数据标签，是(N,)的numpy数组，和train_data按行对应
        '''
        n = len(train_label)
        alpha_vec = cvxpy.Variable(n)
        NPKernelA = np.array([[train_label[i]*train_label[j]*self.KERNEL(train_data[i], train_data[j]) for j in range(0, n)] for i in range(0, n)])
        constraints = [0 <= alpha_vec, alpha_vec <= self.C, alpha_vec@train_label == 0]
        obj = cvxpy.Maximize(cvxpy.sum(alpha_vec)-1.0/2*cvxpy.quad_form(alpha_vec, NPKernelA))
        prob = cvxpy.Problem(obj, constraints)
        prob.solve(solver=cvxpy.ECOS_BB)
        # print('status:', prob.status)
        self.SV_alpha = [alpha for alpha in alpha_vec.value if alpha > self.epsilon]
        self.SV = [train_data[i] for i, alpha in enumerate(alpha_vec.value) if alpha > self.epsilon]
        self.SV_label = [train_label[i] for i, alpha in enumerate(alpha_vec.value) if alpha > self.epsilon]
        self.calc_b()
    
    def calc_b(self):
        for i, alpha in enumerate(self.SV_alpha):
            if alpha < self.C-self.epsilon:
                self.b = self.SV_label[i] - self.wx(self.SV[i])
                break
        # for i, alpha in enumerate(self.SV_alpha):
        #     if 0 < alpha and alpha < self.C:
        #         if abs(self.wx_add_b(self.SV[i]) - self.SV_label[i]) > 0.01:
        #             print("label {}, wx+b: {}".format(self.SV_label[i], self.wx_add_b(self.SV[i])))


    def predict(self, test_data):
        '''
        TODO：实现软间隔SVM预测算法
        train_data：测试数据，是(M, 7)的numpy二维数组，每一行为一个样本
        必须返回一个(M,)的numpy数组，对应每个输入预测的标签，取值为1或-1表示正负例
        '''
        test_lables = np.ones((len(test_data),))
        for i, data in enumerate(test_data):
            res = self.wx_add_b(data)
            test_lables[i] = 1 if res > 0 else -1
        return test_lables


    def wx(self, x):
        return np.sum([self.SV_alpha[j]*self.SV_label[j]*self.KERNEL(self.SV[j], x) for j in range(0, len(self.SV_alpha))])

    def wx_add_b(self, x):
        return self.wx(x)+self.b
