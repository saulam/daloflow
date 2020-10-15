import mnist
import numpy as np
import pickle as pk
import zlib
import os

dataset_name = 'dataset1/'
heigh = 50
width = 50

'''
mnist.init()

heigh = 28
width = 28
x_train, t_train, x_test, t_test = mnist.load()
y_train=t_train.reshape((60000*1)).astype(np.uint8)
y_test=t_test.reshape((10000*1)).astype(np.uint8)
'''

x_train = np.random.randn(100000,heigh,width)
x_test = np.random.randn(1000,heigh,width)
y_train= np.random.randint(2, size=100000)
y_test= np.random.randint(2, size=1000)

Y_train = {}
Y_test  = {}

counter = 0
dir_index = 0

print('Init...')
for i in range(x_train.shape[0]):
    print(i)
    x = x_train[i].reshape((heigh*width)).astype(np.uint8)
    x = zlib.compress(x.tobytes())
    ID = 'train'+str(i)
    prefix = dataset_name + str(dir_index) + '/' 
    if not os.path.exists(prefix):
        os.makedirs(prefix)
    with open(prefix+ID+'.tar.gz','wb') as fd:
        fd.write(x)
    Y_train[prefix[9:]+ID]=y_train[i]
    counter+=1
    if counter>=10000:
        counter=0
        dir_index+=1

for i in range(x_test.shape[0]):
    print(i)
    x = x_test[i].reshape((heigh*width)).astype(np.uint8)
    x = zlib.compress(x.tobytes())
    ID = 'test'+str(i)
    prefix = dataset_name + str(dir_index) + '/'
    if not os.path.exists(prefix):
        os.makedirs(prefix)
    with open(prefix+ID+'.tar.gz','wb') as fd:
        fd.write(x)
    Y_test[prefix[9:]+ID]=y_test[i]
    counter+=1
    if counter>=10000:
        counter=0
        dir_index+=1

with open('dataset1/labels.p','wb') as fd:
    pk.dump([Y_train, Y_test], fd)

