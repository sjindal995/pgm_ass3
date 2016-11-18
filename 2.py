from bif_parser import *
from gibbs import *
import pickle

max_samples=100000

bif_filepath = input("Enter bif file:");
print bif_filepath
(variables,dist) = parse(bif_filepath)

def learn():
	samples_filepath = input("Enter samples filepath:")
	# print sample_vars
	for key in dist:
		sample_file = open(samples_filepath,'r')
		sample_vars = sample_file.readline()[:-1].split(" ")
		numer=0
		denom=0
		query_var = key[0]
		print "key0: "+key[0]
		print "key1: "+key[1]
		if(key[1]==""):
			given_vars=[]
		else:
			given_vars = key[1].split(", ")
		for i in range(0,max_samples):
			var_assign = sample_file.readline()[:-1].split(" ")
			# print var_assign
			given_assign = []
			for given_var in given_vars:
				# print given_var
				try:
					given_assign.append(var_assign[sample_vars.index(given_var)])
				except (IndexError, ValueError) as e:
					print e
					print given_var
					print query_var
					print sample_vars
					print sample_vars.index(query_var)
					print i
					print var_assign
					return
			query_assign = var_assign[sample_vars.index(query_var)];
			# print dist[key][tuple(given_assign)][1]
			dist[key][tuple(given_assign)][1] += 1;
			dist[key][tuple(given_assign)][0][query_assign] += 1;
		for given_assign in dist[key]:
			for q_assign in dist[key][given_assign][0]:
				if(dist[key][given_assign][1] > 0):
					dist[key][given_assign][0][q_assign] /= float(dist[key][given_assign][1])
		sample_file.close()

def infer():
	gibbs(variables,dist)

# learn()
# with open("variables.txt",'wb') as v:
# 	pickle.dump(variables,v)

# with open("dist.txt",'wb') as d:
# 	pickle.dump(dist,d)

with open("variables.txt",'rb') as v:
	variables = pickle.load(v)

with open("dist.txt",'rb') as d:
	dist = pickle.load(d)


infer()
