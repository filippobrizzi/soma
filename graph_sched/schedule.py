import pargraph as par
import copy
import xml.etree.cElementTree as ET

#returns the optimal flows 
def get_flow(flow_list, task_list, level, optimal_flow, NUM_TASKS, MAX_FLOWS):
	if len(flow_list) <= MAX_FLOWS and level < NUM_TASKS:
		task_i = task_list[level]	
		new_flow = par.Flow()
		flow_list_2 = copy.deepcopy(flow_list)
		for flow in flow_list_2 :
			flow.add_task(task_i)
			if flow.bandwidth <= 1:
				if level == NUM_TASKS - 1:
					print
					print "solution:"
					for flow in flow_list_2:
						flow.dump()
					#if get_tot_bandwidth(flow_list_2) < get_tot_bandwidth(optimal_flow):
						#optimal_flow = deepcopy(flow_list_2)
				else:
					get_flow(copy.deepcopy(flow_list_2), task_list, level + 1, optimal_flow, NUM_TASKS, MAX_FLOWS)
			else:
				flow.tasks.remove(task_i)
		new_flow.add_task(task_i)
		flow_list.append(new_flow)
		if new_flow.bandwidth <= 1:
				if level == NUM_TASKS - 1:
					print
					print "solution:"
					for flow in flow_list:
						flow.dump()
					#if get_tot_bandwidth(flow_list) < get_tot_bandwidth(optimal_flow):
						#optimal_flow = deepcopy(flow_list)
				else:
					get_flow(flow_list, task_list, level + 1, optimal_flow, NUM_TASKS, MAX_FLOWS)
		else:
			new_flow.tasks.remove(task_i)

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
