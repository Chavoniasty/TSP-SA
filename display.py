import matplotlib.pyplot as plt

f = open("result.txt", "r")
cities = [[], []]

for line in f:
    line = line.split()
    cities[0].append(float(line[0]))
    cities[1].append(float(line[1]))
f.close()

plt.plot(cities[0], cities[1], 'ro')
plt.plot(cities[0], cities[1], 'b-')
plt.plot([cities[0][0], cities[0][-1]], [cities[1][0], cities[1][-1]], 'b-')

plt.show()
