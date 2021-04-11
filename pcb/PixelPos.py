CAD = [
    [0, 10.5966],
    [-4.2579, 14.7652],
    [-9.7407, 16.8550],
    [-15.4258, 15.4034],
    [-19.2355, 10.9408],
    [-19.7774, 5.0983],
    [-16.8597, 0.0047],
    [-12.6448, -4.2102],
    [-8.4298, -8.4251],
    [-4.2149, -12.6401],
    [0, -16.8550]
]

Width = 700
Ratio = Width / 50
Offset = 750 / 2

CVS1 = [[round(Offset + i[0] * Ratio,2), round(Offset - i[1] * Ratio,2)] for i in CAD]

CVS2 = [[round(Offset - CAD[i][0] * Ratio,2), round(Offset - CAD[i][1] * Ratio,2)] for i in range(-2, -11, -1)]

CVS = CVS1+CVS2
for i in CVS:
    print("[{:.2f}, {:.2f}],".format(i[0],i[1]))