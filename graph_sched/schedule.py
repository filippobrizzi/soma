import pargraph as par
import xml.etree.cElementTree as ET
import math
import copy
from threading import Thread
import time


#returns the optimal flows 
def get_optimal_flow(flow_list, task_list, level, optimal_flow, NUM_TASKS, MAX_FLOWS, start_time, execution_time):
	curopt = get_cost(optimal_flow)
	cur = get_cost(flow_list)
	cur_time = time.clock() - start_time
	if len(flow_list) < MAX_FLOWS and len(task_list) != level and cur <= curopt and cur_time < execution_time:
		task_i = task_list[level]
		# test integrating the single task in each
		for flow in flow_list :
			flow.add_task(task_i)
			get_optimal_flow(flow_list, task_list, level + 1, optimal_flow, NUM_TASKS, MAX_FLOWS, start_time, execution_time)
			flow.remove_task(task_i)
		new_flow = par.Flow()		
		new_flow.add_task(task_i)
		flow_list.append(new_flow)
		get_optimal_flow(flow_list, task_list, level + 1, optimal_flow, NUM_TASKS, MAX_FLOWS, start_time, execution_time)
		flow_list.remove(new_flow)
		
		if 'For' in task_i.type :
			#checks the possible splittings of the for node
			for i in range(2, MAX_FLOWS + 1):
				tmp_task_list = []
				#splits the for node in j nodes
				for j in range(0, i):
					task = par.For_Node("splitted_" + task_i.start_line + "." + str(j), task_i.start_line, task_i.init_type, task_i.init_var, task_i.init_value, task_i.init_cond, task_i.init_cond_value, task_i.init_increment, task_i.init_increment_value, task_i.time, task_i.variance, math.floor(float(task_i.mean_loops) / i))
					task.in_time = float(task_i.time) / i
					task_list.append(task)
					tmp_task_list.append(task)
				get_optimal_flow(flow_list, task_list, level + 1, optimal_flow, NUM_TASKS + i - 1, MAX_FLOWS, start_time, execution_time)
				for tmp_task in tmp_task_list:
					task_list.remove(tmp_task)
			
	else:
		if len(task_list) == level and len(flow_list) <= MAX_FLOWS and cur < curopt:
			#print "acutal cost: ", get_cost(flow_list), "optimal cost: ", get_cost(optimal_flow)
			del optimal_flow[:]
			id = 0
			#print "newflowset:"
			for flow in flow_list:
				flow.id = id
				id += 1
				optimal_flow.append(copy.deepcopy(flow))
				#flow.dump("\t")
				#print "\ttime:",flow.time
			#print "\tcost ", get_cost(optimal_flow),"with flows",len(flow_list)

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
			min_tmp = child.d - float(child.in_time)
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
def chetto(flow_graph, deadline, optimal_flow):
	node = get_last(flow_graph)
	node.d = deadline
	chetto_deadlines(node)
	flow_graph.arrival = 0
	chetto_arrival(flow_graph, optimal_flow)

#gets the cost of the worst flow
def get_cost(flow_list):
	if len(flow_list) == 0:
		return float("inf")
	else:
		return max([flow.time for flow in flow_list])

def chetto_arrival(node, optimal_flow):
	if node.children :
		for child in node.children:
			if child.arrival == None and all_set(child) == True:
				(a, d) = get_max(child, optimal_flow)
				child.arrival = max(a, d)
			chetto_arrival(child)


def get_max(node, optimal_flow):
	maximum_a = 0
	maximum_d = 0
	for p in node.parent:
		if p.arrival > maximum_a and get_id(p, optimal_flow) == get_id(node, optimal_flow):
			maximum_a = p.arrival
		if p.deadline > maximum_d and get_id(p, optimal_flow) != get_id(node, optimal_flow):
			maximum_d = p.deadline
	return (maximum_a, maximum_d)

#checks if all the parent nodes have the arrival times set
def all_set(node):
	found = True
	for p in node.parent:
		if p.arrival == None:
			found = False
	return found

def get_id(node, optimal_flow):
	for flow in optimal_flow:
		for task in flow.tasks:
			if node.type == task.type:
				return flow.id












