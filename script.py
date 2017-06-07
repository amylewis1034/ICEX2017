import subprocess

numToGenerate = 31; # Must be one more than number of paths to generate
subprocess.call(['make', '-j4'])

# thirds
bestPath = -1
bestWeight = 1.0
secondBestPath = -1
secondBestWeight = 1.0

for pathNum in range(0, numToGenerate):
	proc = subprocess.Popen(['./ProjF','thirds'],stdout=subprocess.PIPE)

	for line in iter(proc.stdout.readline,''):
		weight = float(line)
		if weight < bestWeight:
			bestWeight = weight
			bestPath = pathNum
		elif weight < secondBestWeight and weight > bestWeight:
			secondBestWeight = weight
			secondBestPath = pathNum
	subprocess.call(['mv', 'resources/path.txt', 'resources/paths/thirds/path'+str(pathNum)+'.txt'])

print 'Thirds'
print "bestPath: " + str(bestPath) + ", pathWeight: " + str(bestWeight)
print './ProjF paths /paths/thirds/path'+str(bestPath)+'.txt'
print "second bestPath: " + str(secondBestPath) + ", pathWeight: " + str(secondBestWeight)
print './ProjF paths /paths/thirds/path'+str(secondBestPath)+'.txt\n'


# normals
bestPath = -1;
bestWeight = 0.0;
secondBestPath = -1;
secondBestWeight = 0.0;

for pathNum in range(0, numToGenerate):
	proc = subprocess.Popen(['./ProjF','norms'],stdout=subprocess.PIPE)

	for line in iter(proc.stdout.readline,''):
		weight = float(line)
		if weight > bestWeight:
			bestWeight = weight
			bestPath = pathNum
		elif weight > secondBestWeight and weight < bestWeight:
			secondBestWeight = weight
			secondBestPath = pathNum
	subprocess.call(['mv', 'resources/path.txt', 'resources/paths/norms/path'+str(pathNum)+'.txt'])

print 'Normals'
print "bestPath: " + str(bestPath) + ", pathWeight: " + str(bestWeight)
print './ProjF paths /paths/norms/path'+str(bestPath)+'.txt'
print "second bestPath: " + str(secondBestPath) + ", pathWeight: " + str(secondBestWeight)
print './ProjF paths /paths/norms/path'+str(secondBestPath)+'.txt\n'


#combo
bestPath = -1;
bestWeight = 0.0;
secondBestPath = -1;
secondBestWeight = 0.0;

for pathNum in range(0, numToGenerate):
	proc = subprocess.Popen(['./ProjF','combo'],stdout=subprocess.PIPE)

	for line in iter(proc.stdout.readline,''):
		weight = float(line)
		if weight > bestWeight:
			bestWeight = weight
			bestPath = pathNum
		elif weight > secondBestWeight and weight < bestWeight:
			secondBestWeight = weight
			secondBestPath = pathNum
	subprocess.call(['mv', 'resources/path.txt', 'resources/paths/combo/path'+str(pathNum)+'.txt'])

print 'Combo'
print "bestPath: " + str(bestPath) + ", pathWeight: " + str(bestWeight)
print './ProjF paths /paths/combo/path'+str(bestPath)+'.txt'
print "second bestPath: " + str(secondBestPath) + ", pathWeight: " + str(secondBestWeight)
print './ProjF paths /paths/combo/path'+str(secondBestPath)+'.txt\n'