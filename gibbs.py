import sys
import random
burn_in_thresh=1000
max_iter=10000

def getProb(conf, index, query_var, test_vars_all, dist):
	ans=1
	for key in dist:
		if(key[0] == query_var):
			try:
				given_assign = []
				if(key[1]!=""):
					for given_var in key[1].split(", "):
						given_assign.append(conf[test_vars_all.index(given_var)])
						ans *= getProb(conf,test_vars_all.index(given_var),given_var, test_vars_all, dist)
				return (ans*(dist[key][tuple(given_assign)][0][conf[index]]));
			except (KeyError,ValueError) as e:
				print e
				print dist[key][tuple(given_assign)][0][conf[index]]
				print key[0]
				print "key[1]:"
				print key[1]
				print "given_assign:"
				print given_assign
				print "conf :"+str(conf[index])
				sys.exit()

def gibbs(variables,dist):
	test_filepath = input("Enter test file: ");
	test_file = open(test_filepath,'r')
	output_filepath = input("Enter output file: ")
	out_file = open(output_filepath,'w')
	t=0
	burn_in_over=False
	test_vars_all = test_file.readline()[:-1].split(" ")
	while True:
		cur_test = test_file.readline()
		if not cur_test:
			break
		else:
			cur_test = cur_test[:-1]
		cur_test_l = cur_test.split(" ")
		tmp = cur_test_l[:]
		n_nodes=0
		for i in range(0,len(tmp)):
			if (tmp[i]=='?'):
				n_nodes+=1
				try:
					tmp[i] = random.choice(variables[test_vars_all[i]])
				except (KeyError) as e:
					print variables
					print test_vars_all
					print i
					print test_vars_all[i]
					return
		while((not burn_in_over) or n_nodes*t<=max_iter):
			t+=1
			for i in range(0,len(tmp)):
				if(cur_test_l[i]=='?'):
					tmp2_conf = tmp[:]
					max_prob=-1.0
					best_conf=[]
					for cur_var_assign in variables[test_vars_all[i]]:
						tmp2_conf[i]=cur_var_assign
						cur_prob = getProb(tmp2_conf, i, test_vars_all[i], test_vars_all, dist)
						if(cur_prob > max_prob):
							max_prob=cur_prob
							best_conf=tmp2_conf[:]
					if(burn_in_over):
						tmp[i]=best_conf[i]
			if(not burn_in_over):
				if(n_nodes*t >= burn_in_thresh):
					burn_in_over=True
					t=1
		out_file.write(" ".join(tmp) + "\n");

	out_file.close()

	test_file.close()