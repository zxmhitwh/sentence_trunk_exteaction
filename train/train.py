#!/usr/bin/python
#-*- coding:UTF-8 -*-
#########################################################################
# File Name: 1.py
# Author: MT_NLP_Team
# Mail: tne.nlp@meituan.com
# Created Time: 09:56:22 2016-07-08
########################################################################
import sys
from sklearn.linear_model.logistic import logistic_regression_path
from sklearn import linear_model

def get_Totalwords_Cluster():
	infile = open("sample_label").readlines()
	clusterdic = {}
	total_clusternumber = []
	#total_words = []
	for line in infile:
		items = line.split()
		total_clusternumber.append(int(items[1]))
		#total_words.append(items[0])
		clusterdic[items[0]] = int(items[1])
	total_clusternumber = list(set(total_clusternumber))
	total_clusternumber = sorted(total_clusternumber)
	return clusterdic,total_clusternumber

def get_corpus(corpus):
	infile = open(corpus).readlines()
	datadic = {}
	ins = []
	for line in infile:
		if len(line.split()) == 3 and '_' in line.split()[0]:
			ins.append(line)
		else:
			datadic[line.strip('\n')] = ins
			ins = []
	return datadic

def get_trainset(trainset):
	infile = open(trainset).readlines()
	traindic = {}
	for line in infile:
		items = line.split("-----")
		traindic[items[0]] = []
		positems = items[1].split()
		for positem in positems:
			inverse = positem[::-1]
			nPos = inverse.find(':')
			n = len(positem) - 1 - nPos
			traindic[items[0]].append([positem[0:n],positem[n+1::]])
	return traindic

def get_childedge(num,tagresult):
	childedges = []
	for tag in tagresult:
		if tag.split()[1] == 'Root':
			continue
		elif tag.split()[1].split('_')[-1]==num:
			childedges.append(tag.split()[-1])
	childedges = set(childedges)
	return childedges

def get_parentedge(num,tagresult):
	return tagresult[int(num)].split()[-1]

def get_parentnode(num,tagresult):
	if tagresult[int(num)].split()[1] == "Root":
		return "Root"
	else:
		return tagresult[int(num)].split()[1].split("_")[-1]

def word_extraction(datadic):
	totalwords = []
	for key in datadic.keys():
		for item in datadic[key]:
			item1 = item.split()[0]
			inverse = item1[::-1]
			nPos = inverse.find('_')
			n = len(item1) - 1 - nPos
			totalwords.append(item1[0:n])
	totalwords = list(set(totalwords))
	return totalwords

def get_feature(traindic,datadic,threshold):#设置考虑位置前后两个词还是一个词
	clusterdic,total_clusternumber = get_Totalwords_Cluster()
	total_words = word_extraction(datadic)
	labels = []
	total_vector = []
	target = ""
	total_pos = ['nh', 'ni', 'nl', 'nd', 'nz', 'ns', 'nt', 'ws', 'wp', 'a', 'c', 'b', 'e', 'd', 'i', 'k', 'j', 'm', 'o', 'n', 'q', 'p', 'r', 'u', 't', 'v', 'z']
	total_dp = ['ADV', 'RAD', 'SBV', 'DBL', 'VOB', 'LAD', 'HED', 'ATT', 'FOB', 'WP', 'POB', 'IOB', 'COO', 'CMP']
	for sentence in traindic.keys():
		#print sentence
		feature_vector = []
		tagresult = datadic[sentence]
		for m in traindic[sentence]:
			if m[1] == '' or m[1] == '0':
				labels.append(0)
			elif m[1] == '1':
				labels.append(1)
			else:
				labels.append(0)
				print sentence,"error!!!!!!!!!!!!"
		#print labels
		feature_list = [] #记录这个句子中的每个特征词
		for tag in tagresult:
			vectorlist = []
			word = tag.split()[0].split('_')[0]
			word_num = tag.split()[0].split('_')[-1].split('/')[0]
			pos = tag.split()[0].split('_')[-1].split('/')[1]
			parentedge = get_parentedge(word_num,tagresult)
			childedges = get_childedge(word_num,tagresult)
			word_vector = [0 for x in total_words]
			pos_vector = [0 for x in total_pos]
			clusternumber_vector = [0 for x in total_clusternumber]
			parent_vector = [0 for x in total_dp]
			child_vector = [0 for x in total_dp]
			for i in range(len(total_words)):
				if total_words[i] == word:
					word_vector[i] = 1
					break
			for i in range(len(total_pos)):
				if total_pos[i] == pos:
					pos_vector[i] = 1
					break
			if word in clusterdic.keys():
				clusternumber_vector[clusterdic[word]] = 1
			for i in range(len(total_dp)):
				if total_dp[i] == parentedge:
					parent_vector[i] = 1
					break
			for i in range(len(total_dp)):
				if total_dp[i] in childedges:
					child_vector[i] = 1
			parentnode = get_parentnode(word_num,tagresult)
			feature_list.append([word_vector+pos_vector+clusternumber_vector,parent_vector+child_vector,parentnode])

		parentnode_vector = []
		for i in range(len(feature_list)):
			each_feature_vector = []
			word_pos_cls,parent_child,parentnode = feature_list[i]
			if threshold == 1:
	 			if i == 0 and i == len(feature_list)-1:
	 				prevectorlist = [0 for x in word_pos_cls]
	 				nextvectorlist = [0 for x in word_pos_cls]
	 			elif i == 0:
	 				prevectorlist = [0 for x in word_pos_cls]
	 				nextvectorlist = feature_list[i+1][0]
	 			elif i == len(feature_list)-1:
	 				prevectorlist = feature_list[i-1][0]
	 				nextvectorlist = [0 for x in word_pos_cls]
	 			else:
	 				prevectorlist = feature_list[i-1][0]
	 				nextvectorlist = feature_list[i+1][0]
	 			each_feature_vector = word_pos_cls+prevectorlist+nextvectorlist
	 		elif threshold == 2:
	 			prevectorlist_2 = [0 for x in word_pos_cls]
	 			prevectorlist_1 = [0 for x in word_pos_cls]
	 			nextvectorlist_1 = [0 for x in word_pos_cls]
	 			nextvectorlist_2 = [0 for x in word_pos_cls]
	 			if i ==0 and i ==len(feature_list)-1:
	 				pass
	 			elif i==0 and len(feature_list)==2:
	 				nextvectorlist_1 = feature_list[i+1][0]
	 			elif i==1 and len(feature_list)==2:
	 				prevectorlist_1 = feature_list[i-1][0]
	 			elif i==0 and len(feature_list)==3:
	 				nextvectorlist_1 = feature_list[i+1][0]
	 				nextvectorlist_2 = feature_list[i+2][0]
	 			elif i==1 and len(feature_list)==3:
	 				prevectorlist_1 = feature_list[i-1][0]
	 				nextvectorlist_1 = feature_list[i+1][0]
	 			elif i==2 and len(feature_list)==3:
	 				prevectorlist_2 = feature_list[i-2][0]
	 				prevectorlist_1 = feature_list[i-1][0]
	 			elif i==0:
	 				nextvectorlist_1 = feature_list[i+1][0]
	 				nextvectorlist_2 = feature_list[i+2][0]
	 			elif i==1:
	 				prevectorlist_1 = feature_list[i-1][0]
	 				nextvectorlist_1 = feature_list[i+1][0]
	 				nextvectorlist_2 = feature_list[i+2][0]
	 			elif i==len(feature_list)-2:
	 				prevectorlist_2 = feature_list[i-2][0]
	 				prevectorlist_1 = feature_list[i-1][0]
	 				nextvectorlist_1 = feature_list[i+1][0]
	 			elif i==len(feature_list)-1:
	 				prevectorlist_2 = feature_list[i-2][0]
	 				prevectorlist_1 = feature_list[i-1][0]
	 			else:
	 				prevectorlist_2 = feature_list[i-2][0]
	 				prevectorlist_1 = feature_list[i-1][0]
	 				nextvectorlist_1 = feature_list[i+1][0]
	 				nextvectorlist_2 = feature_list[i+2][0]
	 			each_feature_vector = word_pos_cls+prevectorlist_2+prevectorlist_1+nextvectorlist_1+nextvectorlist_2
	 		else:
	 			print "input error!"
	 		feature_vector.append(each_feature_vector)
	 		if parentnode == "Root":
	 			parentnode_vector.append([0 for x in word_pos_cls]+[0 for x in parent_child])
			else:
	 			parentnode_vector.append(feature_list[int(parentnode)][1]+feature_list[int(parentnode)][0])
	 	for i in range(len(parentnode_vector)):
			if i == 0 and i == len(parentnode_vector)-1:
				preparentvector = [0 for x in parentnode_vector[i]]
				nextparentvector = [0 for x in parentnode_vector[i]]
			elif i == 0:
				preparentvector = [0 for x in parentnode_vector[i]]
				nextparentvector = parentnode_vector[i+1]
			elif i == len(parentnode_vector)-1:
				preparentvector = parentnode_vector[i-1]
				nextparentvector = [0 for x in parentnode_vector[i]]
			else:
				preparentvector = parentnode_vector[i-1]
				nextparentvector = parentnode_vector[i+1]
			feature_vector[i].extend(parentnode_vector[i]+preparentvector+nextparentvector)
			if len(feature_vector)!=len(traindic[sentence]):
				target = sentence
			#print len(feature_vector[i])
		total_vector.extend(feature_vector)
	print len(total_vector)
	print len(labels)
	print target
	return total_vector,labels

def get_weight(datadic,feature_vector,labels,threshold):
	clf = linear_model.LogisticRegression(C=0.3,n_jobs=-1,max_iter=100,tol=0.1,penalty='l2')
	print clf.fit(feature_vector,labels)
	file2 = open("weight1.txt","w")
	num = 0
	for value in clf.coef_[0]:
		if value != 0:
			num += 1
	print num
	total_words = word_extraction(datadic)
	total_pos = ['nh', 'ni', 'nl', 'nd', 'nz', 'ns', 'nt', 'ws', 'wp', 'a', 'c', 'b', 'e', 'd', 'i', 'k', 'j', 'm', 'o', 'n', 'q', 'p', 'r', 'u', 't', 'v', 'z']
	total_dp = ['ADV', 'RAD', 'SBV', 'DBL', 'VOB', 'LAD', 'HED', 'ATT', 'FOB', 'WP', 'POB', 'IOB', 'COO', 'CMP']
	clusterdic,total_clusternumber = get_Totalwords_Cluster()
	keywords = []
	for word in total_words:
		keywords.append(word+"_word_0")
	for pos in total_pos:
		keywords.append(pos+"_pos_0")
	for number in total_clusternumber:
		keywords.append(str(number)+"_cluster_0")
	if threshold==2:
		for word in total_words:
			keywords.append(word+"_word_-2")
		for pos in total_pos:
			keywords.append(pos+"_pos_-2")
		for number in total_clusternumber:
			keywords.append(str(number)+"_cluster_-2")
	for word in total_words:
		keywords.append(word+"_word_-1")
	for pos in total_pos:
		keywords.append(pos+"_pos_-1")
	for number in total_clusternumber:
		keywords.append(str(number)+"_cluster_-1")
	for word in total_words:
		keywords.append(word+"_word_1")
	for pos in total_pos:
		keywords.append(pos+"_pos_1")
	for number in total_clusternumber:
		keywords.append(str(number)+"_cluster_1")
	if threshold==2:
		for word in total_words:
			keywords.append(word+"_word_2")
		for pos in total_pos:
			keywords.append(pos+"_pos_2")
		for number in total_clusternumber:
			keywords.append(str(number)+"_cluster_2")
	for dp in total_dp:
		keywords.append(dp+"_parent_node_edge_0")
	for dp in total_dp:
		keywords.append(dp+"_child_node_edge_0")
	for word in total_words:
		keywords.append(word+"_word_parent_node_0")
	for pos in total_pos:
		keywords.append(pos+"_pos_parent_node_0")
	for number in total_clusternumber:
		keywords.append(str(number)+"_cluster_parent_node_0")
	for dp in total_dp:
		keywords.append(dp+"_parent_node_edge_-1")
	for dp in total_dp:
		keywords.append(dp+"_child_node_edge_-1")
	for word in total_words:
		keywords.append(word+"_word_parent_node_-1")
	for pos in total_pos:
		keywords.append(pos+"_pos_parent_node_-1")
	for number in total_clusternumber:
		keywords.append(str(number)+"_cluster_parent_node_-1")
	for dp in total_dp:
		keywords.append(dp+"_parent_node_edge_1")
	for dp in total_dp:
		keywords.append(dp+"_child_node_edge_1")
	for word in total_words:
		keywords.append(word+"_word_parent_node_1")
	for pos in total_pos:
		keywords.append(pos+"_pos_parent_node_1")
	for number in total_clusternumber:
		keywords.append(str(number)+"_cluster_parent_node_1")
	for i in range(len(clf.coef_[0])):
		file2.write(keywords[i]+"\t"+str(clf.coef_[0][i])+"\n")

def accuracy_test(train_feature,train_labels,test_feature,test_labels):
	clf = linear_model.LogisticRegression()
	print clf.fit(train_feature,train_labels)
	predict_labels = clf.predict(test_feature)
	TP = 0
	FN = 0
	FP = 0
	TN = 0
	for i in range(len(test_labels)):
		if test_labels[i] == 1 and predict_labels[i] == 1:
			TP += 1
		elif test_labels[i] == 1 and predict_labels[i] == 0:
			FN += 1
		elif test_labels[i] == 0 and predict_labels[i] == 1:
			FP += 1
		else:
			TN += 1
	print TP,FP
	precision = float(TP)/(TP+FP)
	recall = float(TP)/(TP+FN)
	f1 = 2*precision*recall/(precision+recall)
	print precision,recall,f1

def paramenter_adjust(train_feature,train_labels,test_feature,test_labels):
	c_op = 0
	tol_op = 0
	tol_list = [0.1,0.01,0.001,0.0001,0.00001,0.000001]
	maxiter_op = 0
	score_op = 0
	precision_op = 0
	recall_op = 0	
	for i in range(1,21):
		c = float(i)/10
		for tol in tol_list:
			for maxiter in range(100,1100,100):
				clf = linear_model.LogisticRegression(C=c,n_jobs=-1,max_iter=maxiter,tol=tol,penalty='l2')
				print clf.fit(train_feature,train_labels)
				predict_labels = clf.predict(test_feature)
				TP = 0
				FN = 0
				FP = 0
				TN = 0
				for i in range(len(test_labels)):
					if test_labels[i] == 1 and predict_labels[i] == 1:
						TP += 1
					elif test_labels[i] == 1 and predict_labels[i] == 0:
						FN += 1
					elif test_labels[i] == 0 and predict_labels[i] == 1:
						FP += 1
					else:
						TN += 1
				precision = float(TP)/(TP+FP)
				recall = float(TP)/(TP+FN)
				f1 = 2*precision*recall/(precision+recall)
				print precision,recall
				print f1
				if f1>score_op:
					c_op = c
					tol_op = tol
					maxiter_op = maxiter
					score_op = f1
					precision_op = precision
					recall_op = recall
	print c_op,tol_op,maxiter_op
	print precision_op,recall_op,score_op


def test():
	traindic = get_trainset("newtrainset")
	datadic = get_corpus("corpus_result2")
	train_feature,train_labels = get_feature(traindic,datadic,2)
	testdic = get_trainset("newtest")
	test_feature,test_labels = get_feature(testdic,datadic,2)
	#accuracy_test(train_feature,train_labels,test_feature,test_labels)
	paramenter_adjust(train_feature,train_labels,test_feature,test_labels)

if __name__ == '__main__':
	traindic = get_trainset("newtrainset")
	datadic = get_corpus("corpus_result2")
	total_vector,labels = get_feature(traindic,datadic,2)
	get_weight(datadic,total_vector,labels,2)
	
