import sys
import math


# exponential fitting Y = A*exp(B*X)

X = []
Y = []
Z = []  # Z = log Y
sX = 0
sZ = 0
sXZ = 0
sXX = 0
n = 0

vals = input()
vals = vals.split()
for i in range(len(vals)):
    two = vals[i].split('-')
    X.append(float(two[0]))
    Y.append(float(two[1]))
    Z.append(math.log(Y[-1]))
    sX += X[-1]
    sZ += Z[-1]
    sXZ += X[-1]*Z[-1]
    sXX += X[-1]*X[-1]
    n += 1

n = float(n)
B = (n*sXZ - sX*sZ) / (n*sXX - sX*sX)
C = (sZ - B*sX) / n

A = math.exp(C)

# print the exponential factor
print('Y = %s * exp(%s * X)' % (A, B))
