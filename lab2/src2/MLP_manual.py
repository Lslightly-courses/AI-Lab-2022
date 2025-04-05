import torch
import numpy as np
from matplotlib import pyplot as plt

def softmax(x):
    return np.exp(x) / np.sum(np.exp(x))

def crossEntropyLoss(y_hat, y):
    return -np.log(y_hat[getCategory(y)][0])

def getCategory(y):
    '''
        获得类别下标
    '''
    for i, y_elem in enumerate(y):
        if y_elem == 1.0:
            return i
    print('no 1', y)
    exit(-1)

def printShape(x, string):
    print(string, x.shape)

class MLP:
    def __init__(self):
        # layer size = [10, 8, 8, 4]
        # 初始化所需参数   
        self.layer_size = [10, 10, 8, 8, 4]
        self.h = [np.ones((self.layer_size[i], 1)) for i in range(0, 5)]    # h[0]为x,h[4]为y_hat
        self.delta = [np.ones((self.layer_size[i], 1)) for i in range(0, 5)]    # 对b的偏导
        self.delta_W = [np.ones((self.layer_size[i], self.layer_size[i-1])) for i in range(1, 5)]   # 对W的偏导
        self.delta_W.insert(0, 0)
    
    def init_w_b(self, init_w_np, init_b_np):
        self.w = [np.copy(init_w) for init_w in init_w_np]
        self.w.insert(0, 0)
        self.b = [np.copy(init_b) for init_b in init_b_np]
        self.b.insert(0, 0)
    
    def get_y_hat(self):
        return self.h[4]

    def forward(self, x):
        # 前向传播
        self.h[0] = x.reshape(len(x), 1)
        # printShape(self.h[0], 'self.h[0]')
        for i in range(1, 4):
            self.h[i] = np.tanh(self.w[i]@self.h[i-1]+self.b[i])
            # printShape(self.h[i], f'self.h[{i}]')
        self.h[4] = softmax(self.w[4]@self.h[3]+self.b[4])
        # printShape(self.h[4], 'self.h[4]')
        return self.h[4]

    def backward(self, y, lr): # 自行确定参数表
        # 反向传播
        self.generate_delta(y)
        for i in range(1, 5):
            self.w[i] -= lr*self.delta_W[i]
            self.b[i] -= lr*self.delta[i]
    
    def generate_delta(self, y):
        self.delta[4] = np.copy(self.get_y_hat())
        self.delta[4][getCategory(y)] -= 1
        # print(self.get_y_hat())
        # print(self.delta[4])
        self.delta_W[4] = self.delta[4]@self.h[3].T
        # printShape(self.delta[4], f'self.delta[4]')
        # printShape(self.delta_W[4], f'self.delta_W[4]')
        for i in range(3, 0, -1):
            self.delta[i] = (self.w[i+1].T@self.delta[i+1])*self.tanh_prime(i)
            self.delta_W[i] = self.delta[i]@self.h[i-1].T
            # printShape(self.delta[i], f'self.delta[{i}]')
            # printShape(self.delta_W[i], f'self.delta_W[{i}]')
    
    def tanh_prime(self, index):
        '''
            tanh' = 1-tanh^2
        '''
        return 1-self.h[index]**2
    
    def get_w_grad_in_layer(self, layer):
        return self.delta_W[layer]
    
    def get_b_grad_in_layer(self, layer):
        return self.delta[layer]
    
    def print_w_b(self):
        with open('mlp_w_b.out', 'w') as f:
            for layer in range(1, 5):
                f.write(f'w[{layer}]=\n')
                f.write(f'{self.w[layer]}\n')
                f.write(f'b[{layer}]=\n')
                f.write(f'{self.b[layer]}\n')

class TorchTrainModel:
    """
    使用torch autograd自动计算梯度
    """

    def __init__(self, lr, init_w_np, init_b_np):
        self.lr = lr
        self.layer_size = [10, 10, 8, 8, 4]
        self.w = [torch.from_numpy(np.copy(init_w)) for init_w in init_w_np]
        for w in self.w:
            w.requires_grad_(True)
        self.b = [torch.from_numpy(np.copy(init_b)) for init_b in init_b_np]
        for b in self.b:
            b.requires_grad_(True)
        self.w.insert(0, 0)
        self.b.insert(0, 0)
        self.h = [torch.ones(self.layer_size[i], 1, dtype=torch.double) for i in range(0, 5)]
        self.w_grad = [torch.ones(self.layer_size[i], 1, dtype=torch.double) for i in range(1, 5)]
        self.w_grad.insert(0, 0)
        self.b_grad = [torch.ones(self.layer_size[i], 1, dtype=torch.double) for i in range(1, 5)]
        self.b_grad.insert(0, 0)
    
    def train(self, x, y):
        self.h[0] = torch.from_numpy(x).reshape((10, 1))
        for i in range(1, 4):
            self.h[i] = torch.tanh(self.w[i]@self.h[i-1]+self.b[i])
        self.h[4] = torch.softmax(self.w[4]@self.h[3]+self.b[4], dim=0, dtype=torch.double)
        log_y_hat = torch.log(self.h[4].T)  # log(y_hat^T)
        # print(self.h[4], log_y_hat)
        y_torch = torch.tensor([getCategory(y)])    # one-hot
        nllloss_func = torch.nn.NLLLoss()
        loss = nllloss_func(log_y_hat, y_torch) # cross entropy loss
        # print(loss)
        loss.backward() # autograd
        self.update_param() # 
        return loss.item()
    
    def update_param(self):
        for i in range(1, 5):
            self.w_grad[i] = self.w[i].grad
            self.b_grad[i] = self.b[i].grad
            self.w[i] = torch.tensor((self.w[i]-self.lr*self.w[i].grad).tolist(), dtype=torch.double, requires_grad=True)
            self.b[i] = torch.tensor((self.b[i]-self.lr*self.b[i].grad).tolist(), dtype=torch.double, requires_grad=True)
    
    def get_w_grad_in_layer(self, layer):
        return self.w_grad[layer]
    
    def get_b_grad_in_layer(self, layer):
        return self.b_grad[layer]

def train(mlp: MLP, epochs, lr, inputs, labels):
    '''
        mlp: 传入实例化的MLP模型
        epochs: 训练轮数
        lr: 学习率
        inputs: 生成的随机数据
        labels: 生成的one-hot标签
    '''
    layer_size = [10, 10, 8, 8, 4]

    # 使用相同的初始参数
    init_w = [np.random.randn(layer_size[i], layer_size[i-1]) for i in range(1, 5)]
    init_b = [np.random.randn(layer_size[i], 1) for i in range(1, 5)]
    
    mlp.init_w_b(init_w, init_b)
    mlp_loss = mlpTrain(mlp, epochs, lr, inputs, labels)
    torch_train_model = TorchTrainModel(lr, init_w, init_b)
    torch_loss = torchTrain(torch_train_model, epochs, inputs, labels)
    outputGrad(mlp, torch_train_model)
    mlp.print_w_b()
    showLossImg(mlp_loss, torch_loss, epochs)


def mlpTrain(mlp: MLP, epochs, lr, inputs, labels):
    '''
        手动求导mlp训练
    '''
    num_data = len(labels)
    mlp_loss = []
    for epoch in range(epochs):
        loss = 0
        for x, y in zip(inputs, labels):
            y_hat = mlp.forward(x)
            # print(y_hat.T)
            loss += crossEntropyLoss(y_hat, y)
            mlp.backward(y, lr)
        loss /= num_data
        mlp_loss.append(loss)
        print('epoch {},\tloss {}'.format(epoch, loss))
    return mlp_loss

def torchTrain(torch_train_model: TorchTrainModel, epochs, inputs, labels):
    '''
        自动求导mlp训练
    '''
    torch_loss = []
    num_data = len(labels)
    for epoch in range(epochs):
        loss = 0
        for x, y in zip(inputs, labels):
            loss += torch_train_model.train(x, y)
        loss /= num_data
        torch_loss.append(loss)
        print('epoch {},\tloss {}'.format(epoch, loss))
    return torch_loss

def outputGrad(mlp: MLP, torch_train_model: TorchTrainModel):
    with open('mlp_grad.out', 'w') as f:
        for layer in range(1, 5):
            mlp_w_grad = torch.from_numpy(mlp.get_w_grad_in_layer(layer))
            torch_w_grad = torch_train_model.get_w_grad_in_layer(layer)
            mlp_b_grad = torch.from_numpy(mlp.get_b_grad_in_layer(layer))
            torch_b_grad = torch_train_model.get_b_grad_in_layer(layer)
            if not torch.sum(mlp_w_grad-torch_w_grad) < 0.001:
                print(f'layer{layer} diff')
            if not torch.sum(mlp_b_grad-torch_b_grad) < 0.001:
                print(f'layer{layer} diff')
            f.write(f'layer {layer}\nmlp grad for w\n{mlp_w_grad}\ntorch auto grad for w\n{torch_w_grad}\n')
            f.write(f'mlp grad for b\n{mlp_b_grad}\ntorch auto grad for b\n{torch_b_grad}\n')

def showLossImg(mlp_loss: list, torch_loss: list, epochs):
    epoch_list = [epoch+1 for epoch in range(epochs)]
    plt.plot(epoch_list, mlp_loss, c='green', label='hand write grad')
    plt.plot(epoch_list, torch_loss, c='blue', label='torch auto grad')
    plt.legend(loc='best', fontsize=10)
    plt.show()


if __name__ == '__main__':
    # 设置随机种子,保证结果的可复现性
    np.random.seed(1)
    mlp = MLP()
    # 生成数据
    inputs = np.random.randn(100, 10)

    # 生成one-hot标签
    labels = np.eye(4)[np.random.randint(0, 4, size=(1, 100))].reshape(100, 4)

    # 训练
    epochs = 3000
    lr = 0.01
    train(mlp, epochs, lr, inputs, labels)
    