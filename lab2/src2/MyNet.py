import torch
import torchvision
import torch.utils.data

import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim

from torchvision import transforms


device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

class MyNet(nn.Module):
    def __init__(self):
        super(MyNet,self).__init__()
        ########################################################################
        #这里需要写MyNet的卷积层、池化层和全连接层
        self.conv_1 = nn.Conv2d(3, 12, 5, padding=2)
        self.maxpool_2 = nn.MaxPool2d(2)
        self.conv_3 = nn.Conv2d(12, 24, 3, padding=1)
        self.maxpool_4 = nn.MaxPool2d(2)
        self.conv_5 = nn.Conv2d(24, 32, 2)
        self.linear_6 = nn.Linear(32*7*7, 108)
        self.linear_7 = nn.Linear(108, 84)
        self.linear_8 = nn.Linear(84, 10)
        

    def forward(self, x):
        ########################################################################
        #这里需要写MyNet的前向传播
        x = F.relu(self.conv_1(x))
        x = self.maxpool_2(x)
        x = F.relu(self.conv_3(x))
        x = self.maxpool_4(x)
        x = F.relu(self.conv_5(x))
        x = x.view(-1, self.num_flat_features(x))
        x = F.relu(self.linear_6(x))
        x = F.relu(self.linear_7(x))
        x = F.relu(self.linear_8(x))
        return x
    
    def num_flat_features(self, x):
        size = x.size()[1:]
        num_features = 1
        for s in size:
            num_features *= s
        return num_features
       

def train(net,train_loader,optimizer,n_epochs,loss_function):
    net.train()
    for epoch in range(n_epochs):
        for step, (inputs, labels) in enumerate(train_loader, start=0):
            # get the inputs; data is a list of [inputs, labels]
            inputs, labels = inputs.to(device), labels.to(device)
            
            ########################################################################
            #计算loss并进行反向传播
            optimizer.zero_grad()
            output = net(inputs)
            loss = loss_function(output, labels)
            loss.backward()
            optimizer.step()
            
            ########################################################################

            if step % 100 ==0:
                print('Train Epoch: {}/{} [{}/{}]\tLoss: {:.6f}'.format(
                    epoch, n_epochs, step * len(inputs), len(train_loader.dataset), loss.item()))

    print('Finished Training')
    save_path = './MyNet.pth'
    torch.save(net.state_dict(), save_path)

def test(net, test_loader, loss_function):
    net.eval()
    test_loss = 0.
    num_correct = 0 #correct的个数
    with torch.no_grad():
        for inputs, labels in test_loader:
            inputs, labels = inputs.to(device), labels.to(device)
        ########################################################################
        #需要计算测试集的loss和accuracy
            output = net(inputs)
            loss = loss_function(output, labels)
            test_loss += loss
            _, predict = torch.max(output.data, 1)
            num_correct += (predict == labels).sum()
        total = sum([labels.size(0) for _, labels in test_loader])
        accuracy = num_correct / total
        test_loss /= len(test_loader)
        ########################################################################
        print("Test set: Average loss: {:.4f}\t Acc {:.2f}".format(test_loss.item(), accuracy))
    

if __name__ == '__main__':
    n_epochs =5
    train_batch_size = 128
    test_batch_size =5000 
    learning_rate = 5e-4

    transform = transforms.Compose(
        [transforms.ToTensor(),
         transforms.Normalize((0.4914, 0.4822, 0.4465), (0.247, 0.243, 0.261))])

    # 50000张训练图片
    train_set = torchvision.datasets.CIFAR10(root='./data', train=True,
                                             download=False, transform=transform)                                      
    train_loader = torch.utils.data.DataLoader(train_set, batch_size=train_batch_size,
                                               shuffle=True, num_workers=0)

    # 10000张验证图片
    test_set = torchvision.datasets.CIFAR10(root='./data', train=False,
                                           download=False, transform=transform)
    test_loader = torch.utils.data.DataLoader(test_set, batch_size=test_batch_size,
                                             shuffle=False, num_workers=0)


    net = MyNet()

    # 自己设定优化器和损失函数
    net.to(device)
    # print(net)
    optimizer = optim.SGD(net.parameters(), lr=0.1)
    loss_function = nn.CrossEntropyLoss()

   
    #######################################################################

    train(net,train_loader,optimizer,n_epochs,loss_function)
    test(net,test_loader,loss_function)


    
