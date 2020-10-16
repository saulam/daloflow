import mnist
import numpy as np
import pickle as pk
import zlib
import os

dataset_name = 'dataset32x32/'
heigh = 32
width = 32
n_train = 1000000
n_test = 1000

'''
mnist.init()

heigh = 28
width = 28
x_train, t_train, x_test, t_test = mnist.load()
y_train=t_train.reshape((60000*1)).astype(np.uint8)
y_test=t_test.reshape((10000*1)).astype(np.uint8)
'''

y_train= np.random.randint(2, size=n_train)
y_test= np.random.randint(2, size=n_test)

Y_train = {}
Y_test  = {}

counter = 0
dir_index = 0

print('Init...')
for i in range(n_train):
    print(i)
    x = np.random.randn(heigh,width).astype(np.uint8)
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

for i in range(n_test):
    print(i)
    x = np.random.randn(heigh,width).astype(np.uint8)
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

with open(dataset_name+'labels.p','wb') as fd:
    pk.dump([Y_train, Y_test], fd)

