# Copyright 2019 Uber Technologies, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================

import tensorflow as tf
import horovod.tensorflow.keras as hvd
import socket
import os
from   data_generator import DataGenerator
import pickle as pk
import argparse


'''
* Default configuration
'''

# manually specify the GPUs to use
os.environ["CUDA_DEVICE_ORDER"]    = "PCI_BUS_ID"
os.environ["CUDA_VISIBLE_DEVICES"] = "0,1"

# default path
default_path  = '/mnt/local-storage/daloflow/dataset32x32'
channels      = 1
batch_size    = 32
shuffle       = True


'''
* Command line arguments
'''

parser = argparse.ArgumentParser(description='Build dataset.')
parser.add_argument('--height',  type=int, default=32,           nargs=1, required=False, help='an integer for the height')
parser.add_argument('--width',   type=int, default=32,           nargs=1, required=False, help='an integer for the width')
parser.add_argument('--path',    type=str, default=default_path, nargs=1, required=False, help='dataset path')
args = parser.parse_args()


#
# Configuration (by command line switches)
#

height           = int(args.height[0])
width            = int(args.width[0])
images_path      = args.path[0]


'''
* train and validation params
'''

TRAIN_PARAMS = {'height':height,
                'width':width,
                'channels':channels,
                'batch_size':32,
                'images_path':images_path,
                'shuffle':shuffle}

with open(images_path+'/labels.p', 'rb') as fd:
    labels_train, labels_test = pk.load(fd)

nevents=len(list(labels_train.keys()))
partition = {'train' : list(labels_train.keys()), 'validation' : list(labels_test.keys())}

'''
* GENERATORS
'''

training_generator   = DataGenerator(**TRAIN_PARAMS).generate(labels_train, partition['train'],      True)
validation_generator = DataGenerator(**TRAIN_PARAMS).generate(labels_test,  partition['validation'], True)


'''
* Main
'''

# Horovod: initialize Horovod.
hvd.init()

hostname = socket.gethostname()
local_ip = socket.gethostbyname(hostname)
print('%s, %d' % (local_ip, hvd.local_rank()))

# Horovod: pin GPU to be used to process local rank (one GPU per process)
gpus = tf.config.experimental.list_physical_devices('GPU')
for gpu in gpus:
    tf.config.experimental.set_memory_growth(gpu, True)
if gpus:
    tf.config.experimental.set_visible_devices(gpus[hvd.local_rank()], 'GPU')


input_shape = [height,width,channels]
img_input = tf.keras.layers.Input(shape=input_shape, name='input')
x = tf.keras.layers.Conv2D(32, [3, 3], activation='relu')(img_input)
for i in range(10):
    x = tf.keras.layers.Conv2D(64, [3, 3], activation='relu')(x)
    #x = tf.keras.layers.MaxPooling2D(pool_size=(2, 2))(x)
    x = tf.keras.layers.Dropout(0.25)(x)
x = tf.keras.layers.Flatten()(x)
x = tf.keras.layers.Dense(128, activation='relu')(x)
x = tf.keras.layers.Dropout(0.5)(x)
x = tf.keras.layers.Dense(10, activation='softmax')(x)
mnist_model = tf.keras.models.Model(inputs=img_input, outputs=x, name='my_model')
mnist_model.summary()


# Horovod: adjust learning rate based on number of GPUs.
opt = tf.optimizers.Adam(0.001 * hvd.size())

# Horovod: add Horovod DistributedOptimizer.
opt = hvd.DistributedOptimizer(opt)

# Horovod: Specify `experimental_run_tf_function=False` to ensure TensorFlow
# uses hvd.DistributedOptimizer() to compute gradients.
mnist_model.compile(loss=tf.losses.SparseCategoricalCrossentropy(),
                    optimizer=opt,
                    metrics=['accuracy'],
                    experimental_run_tf_function=False)

callbacks = [
    # Horovod: broadcast initial variable states from rank 0 to all other processes.
    # This is necessary to ensure consistent initialization of all workers when
    # training is started with random weights or restored from a checkpoint.
    hvd.callbacks.BroadcastGlobalVariablesCallback(0),

    # Horovod: average metrics among workers at the end of every epoch.
    #
    # Note: This callback must be in the list before the ReduceLROnPlateau,
    # TensorBoard or other metrics-based callbacks.
    hvd.callbacks.MetricAverageCallback(),

    # Horovod: using `lr = 1.0 * hvd.size()` from the very beginning leads to worse final
    # accuracy. Scale the learning rate `lr = 1.0` ---> `lr = 1.0 * hvd.size()` during
    # the first three epochs. See https://arxiv.org/abs/1706.02677 for details.
    hvd.callbacks.LearningRateWarmupCallback(warmup_epochs=3, verbose=1),
]

# Horovod: save checkpoints only on worker 0 to prevent other workers from corrupting them.
if hvd.rank() == 0:
    callbacks.append(tf.keras.callbacks.ModelCheckpoint('./checkpoint-{epoch}.h5'))

# Horovod: write logs on worker 0.
verbose = 1 if hvd.rank() == 0 else 0

# Train the model.
# Horovod: adjust number of steps based on number of GPUs.
#mnist_model.fit(dataset, steps_per_epoch=10 // hvd.size(), callbacks=callbacks, epochs=24, verbose=verbose)
steps_per_epoch=nevents//batch_size
mnist_model.fit(x=training_generator, steps_per_epoch=steps_per_epoch // hvd.size(), callbacks=callbacks, epochs=24, verbose=verbose)

