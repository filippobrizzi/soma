import pargraph
import xml.etree.cElementTree as ET

#returns the optimal flows 
def get_flow(flow_list, task_list, level, optimal_flow, NUM_TASKS, MAX_FLOWS):
	if len(flow_list) <= MAX_FLOWS and level < NUM_TASKS:
		task_i = task_list[level]	
		new_flow = Flow()
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
