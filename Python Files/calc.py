import numpy as np
import matplotlib.pylab as plt

samprate = 32
#frec =  12280
frec =  11289.6 #master clk
fract = np.arange(0, 256, 1) 
divide = -1+(100000*(fract+1))/(frec)
err = 1

for i in range(0, len(divide)):
    if (err > (divide[i]-np.floor(divide[i]))):
        err = divide[i]-np.floor(divide[i]) 
        divide_ = np.floor(divide[i])
        fract_ = i
        #plt.plot(i, err, 'x')
    if (err > (np.ceil(divide[i])-divide[i])):
        err = np.ceil(divide[i])-divide[i]
        divide_ = np.ceil(divide[i])
        fract_ = i
        #plt.plot(i, err, 'x')
    #plt.plot(i, err, 'o')  

print(fract_)
print(divide_, err)

div = np.arange(0, 256, 1)
frec_ = samprate*16*2*(div+1)*2
err_ = np.inf
for i in range(0, len(div)):
    if (err_ > np.abs(frec - frec_[i])):
        err_ = np.abs(frec - frec_[i])
        div_ = i
        #plt.plot(i, err_, 'x')
    #plt.plot(i, err_, 'o')

print(div_, err_)
plt.show()
