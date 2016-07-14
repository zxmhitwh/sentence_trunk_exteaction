#coding:utf-8
'''
infile1 = open("newtrainset").readlines()
infile2 = open("newtestset").readlines()
#infile3 = open("newtestset2").readlines()
infile4 = open("newresult").readlines()
outfile = open("corpus","w")
a = []
b = []
for line in infile1:
	a.append(line.split("-----")[0])
for line in infile2:
	a.append(line.split("-----")[0])
for line in infile4:
	a.append(line.split("-----")[0])
#for line in infile4:
#	if line.split("-----")[0] not in a:
#		b.append(line)
#b = set(b)
#for line in b:
#	outfile.write(line)
for line in set(a):
	outfile.write(line+"\n")
'''


#num = 0

from train import *
if __name__ == '__main__':
	outfile = open("corpus_result2","w")
	datadic = get_corpus("corpus_result1")
	num = 0
	for key in datadic.keys():
		for i in range(len(datadic[key])):
			if  i == int(datadic[key][i].split()[0].split('_')[-1].split('/')[0]):
				pass
			else:
				num += 1
				word = datadic[key][i].split()[0].split('_')[0]
				pos = datadic[key][i].split()[0].split('_')[-1].split('/')[1]
				#yulai1 = datadic[key][i].split()[0]
				#replace1 = word + "_" + str(i) + "/" + pos
				yulai2 = word + "_" + datadic[key][i].split()[0].split('_')[-1].split('/')[0]
				replace2 = word + "_" + str(i)
				print yulai2,replace2
				for i in range(len(datadic[key])):
					datadic[key][i] = datadic[key][i].replace(yulai2,replace2)
					#datadic[key][i] = datadic[key][i].replace(yulai1,replace1)
		for line in datadic[key]:
			outfile.write(line)
		outfile.write(key+"\n")
	print num
