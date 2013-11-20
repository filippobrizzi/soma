import pargraph as par
import copy
import xml.etree.cElementTree as ET
import math

#returns the optimal flows 
def get_optimal_flow(flow_list, task_list, level, optimal_flow, NUM_TASKS, MAX_FLOWS):
	if len(flow_list) < MAX_FLOWS - 1 and level < NUM_TASKS:
		task_i = task_list[level]
		new_flow = par.Flow()
		
		for flow in flow_list :
			flow.add_task(task_i)
			get_optimal_flow(flow_list, task_list, level + 1, optimal_flow, NUM_TASKS, MAX_FLOWS)
			flow.tasks.remove(task_i)
		new_flow.add_task(task_i)
		flow_list.append(new_flow)
		get_optimal_flow(flow_list, task_list, level + 1, optimal_flow, NUM_TASKS, MAX_FLOWS)
		flow_list.remove(new_flow)
		if 'For' in task_i.type :
			#checks the possible splittings of the for node
			for i in range(1, MAX_FLOWS):
				tmp_task_list = []
				#splits the for node in j nodes
				for j in range(1, i + 1):
					#fix time
					task = par.For_Node("splitted_" + task_i.start_line + "_" + str(j), task_i.start_line, task_i.init_type, task_i.init_var, task_i.init_value, task_i.init_cond, task_i.init_cond_value, task_i.init_increment, task_i.init_increment_value, task_i.time, task_i.variance, math.floor(float(task_i.mean_loops) / i))
					task_list.append(task)
					tmp_task_list.append(task)
				get_optimal_flow(flow_list, task_list, level + 1, optimal_flow, NUM_TASKS + i, MAX_FLOWS)
				for tmp_task in tmp_task_list:
					task_list.remove(tmp_task)
	else:
		if get_cost(flow_list) < get_cost(optimal_flow):
			print "better sol:"
			del optimal_flow[:]
			for flow in flow_list:
				optimal_flow.append(flow)



		"""
		if 'For' in task_i.type :
			#checks the possible splittings of the for node
			for i in range(1, MAX_FLOWS):
				tmp_task_list = []
				#splits the for node in j nodes
				for j in range(1, i + 1):
					#fix time
					task = par.For_Node("splitted_" + task_i.start_line + "_" + str(j), task_i.start_line, task_i.init_type, task_i.init_var, task_i.init_value, task_i.init_cond, task_i.init_cond_value, task_i.init_increment, task_i.init_increment_value, task_i.time, task_i.variance, math.floor(float(task_i.mean_loops) / i))
					task_list.append(task)
					tmp_task_list.append(task)
				get_optimal_flow(copy.deepcopy(flow_list), task_list, level + 1, optimal_flow, NUM_TASKS + i, MAX_FLOWS)
				for tmp_task in tmp_task_list:
					task_list.remove(tmp_task)
		"""

#generator for the tasks of the graph
def generate_task(node):
	if node.color != 'black':
		node.color = 'black'
		yield node
		for n in node.children:
			for node in generate_task(n):
				yield node

#returns the number or physical cores
def get_core_num(profile):
	root = ET.ElementTree(file = profile).getroot()
	return int(root.find('Hardware/NumberofCores').text) / 2

#sets the color of each node to white
def make_white(node):
	if node.color == 'black':
		node.color = 'white'
		for child in node.children:
			make_white(child)

#returns the graph which contains the 'main' function
def get_main(exp_flows):
	for i in range(len(exp_flows)):
		if exp_flows[i].type == 'main':
			return exp_flows[i]

#returns the last node of the input graph
def get_last(node):
	if not node.children:
		return node	
	else:
		 return get_last(node.children[0])

#returns the children with the least deadline - computation_time 
def get_min(node):
	minimum = float("inf")
	found = False
	for child in node.children:
		if child.d == None:
			found = True
	if found == False:
		#print "setting: ",child.type,"@",child.start_line
		for child in node.children:
			min_tmp = child.d - (float(child.time) - float(child.children_time))
			if min_tmp < minimum:
				minimum = min_tmp
		return minimum


#sets the deadline for each task
def chetto_deadlines(node):
	if node.parent :
		for p in node.parent:
			p.d = get_min(p)
		for p in node.parent:
			chetto_deadlines(p)

#applys the chetto algorithm to obtain the deadline and arrival time for each task
def chetto(flow_graph, deadline):
	node = get_last(flow_graph)
	node.d = deadline
	chetto_deadlines(node)

#gets the cost of the worst flow
def get_cost(flow_list):
	max_cost = 0
	if len(flow_list) == 0:
		return float("inf")
	else:
		for flow in flow_list:
			if flow.time > max_cost:
				max_cost = flow.time
		return max_cost






