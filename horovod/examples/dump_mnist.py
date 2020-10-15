import mnist
import numpy as np
import pickle as pk
import zlib
mnist.init()

heigh = 28
width = 28
x_train, t_train, x_test, t_test = mnist.load()
y_train=t_train.reshape((60000*1)).astype(np.uint8)
y_test=t_test.reshape((10000*1)).astype(np.uint8)

Y_train = {}
Y_test  = {}
for i in range(x_train.shape[0]):
    x = x_train[i].reshape((heigh*width)).astype(np.uint8)
    x = zlib.compress(x.tobytes())
    ID = 'train'+str(i)
    with open('mnist/'+ID+'.tar.gz','wb') as fd:
        fd.write(x)
    Y_train[ID]=y_train[i]
for i in range(x_test.shape[0]):
    x = x_test[i].reshape((heigh*width)).astype(np.uint8)
    x = zlib.compress(x.tobytes())
    ID = 'test'+str(i)
    with open('mnist/'+ID+'.tar.gz','wb') as fd:
        fd.write(x)
    Y_test[ID]=y_test[i]

with open('mnist/labels.p','wb') as fd:
    pk.dump([Y_train, Y_test], fd)

