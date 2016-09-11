import numpy as np
import matplotlib.pyplot as plt

d1 = np.loadtxt('data/face_lipcorner_low.dat')
d2 = np.loadtxt('data/face_lipcorner_high.dat')
for d in [d1, d2]:
    e = d[48:55]
    e[:,1] *= -1
    meanx, meany = e.mean(axis=0)
    maxx, maxy = e.max(axis=0)
    minx, miny = e.min(axis=0)

    scale = maxx - minx
    enorm = (e - [meanx, meany]) / scale
    x = enorm[:,0]
    y = enorm[:,1]

    A = np.vstack([x**4,x**3,x**2,x**1,np.ones(len(x))]).T
    print(A)
    res = np.linalg.lstsq(A, y)[0]
    print(res)
    plt.plot(x, y)
    fx = np.linspace(minx / scale, maxx / scale)
    plt.plot(fx, [res[0]*z**4+res[1]*z**3+res[2]*z**2+res[3]*z+res[4] for z in fx])
    if (d == d1).all():
        np.savetxt('data/lipcorner_low_norm.dat', enorm)
        np.savetxt('data/lipcorner_low_coeff.dat', res)
    else:
        np.savetxt('data/lipcorner_high_norm.dat', enorm)
        np.savetxt('data/lipcorner_high_coeff.dat', res)
plt.show()
