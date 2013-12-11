import pargraph as par
import xml.etree.cElementTree as ET
import math
import copy
import time
import multiprocessing


class Queue():
	def __init__(self):
		self.q = multiprocessing.Queue()
		self.set = False



#returns the optimal flows 
def get_optimal_flow(flow_list, task_list, level, optimal_flow, NUM_TASKS, MAX_FLOWS, start_time, execution_time, q):
	cur_time = time.clock() - start_time
	if cur_time < execution_time:
		curopt = get_cost(optimal_flow)
		cur = get_cost(flow_list)
		if len(flow_list) < MAX_FLOWS and len(task_list) != level and cur <= curopt:
			task_i = task_list[level]
			# test integrating the single task in each
			for flow in flow_list :
				flow.add_task(task_i)
				get_optimal_flow(flow_list, task_list, level + 1, optimal_flow, NUM_TASKS, MAX_FLOWS, start_time, execution_time, q)
				flow.remove_task(task_i)
			new_flow = par.Flow()		
			new_flow.add_task(task_i)
			flow_list.append(new_flow)
			get_optimal_flow(flow_list, task_list, level + 1, optimal_flow, NUM_TASKS, MAX_FLOWS, start_time, execution_time, q)
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
					get_optimal_flow(flow_list, task_list, level + 1, optimal_flow, NUM_TASKS + i - 1, MAX_FLOWS, start_time, execution_time, q)
					for tmp_task in tmp_task_list:
						task_list.remove(tmp_task)
				
		else:
			if len(task_list) == level and len(flow_list) <= MAX_FLOWS and cur < curopt:
				#print "acutal cost: ", get_cost(flow_list), "optimal cost: ", get_cost(optimal_flow)
				del optimal_flow[:]
				id = 0
				#print "newflowset:"
				for flow in flow_list:
					for task in flow.tasks:
						task.id = id
					id += 1
					optimal_flow.append(copy.deepcopy(flow))
	elif q.set == False:
		q.q.put(optimal_flow)
		q.set = True



				#flow.dump("\t")
				#print "\ttime:",flow.time
			#print "\tcost ", get_cost(optimal_flow),"with flows",len(flow_list)
				
#generator for the tasks of the graph
def generate_task(node):
	if node.color == 'white':
		node.color = 'black'
		yield node
		for n in node.children:
			for node in generate_task(n):
				yield node

def generate_list(l, node):
	if node.color == 'white':
		node.color = 'black'
		l.append(node)
		for n in node.children:
			generate_list(l, n)

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
			chetto_arrival(child, optimal_flow)


def get_max(node, optimal_flow):
	maximum_a = 0
	maximum_d = 0
	for p in node.parent:
		if p.arrival > maximum_a and p.id == node.id:
			maximum_a = p.arrival
		if p.d > maximum_d and p.id != node.id:
			maximum_d = p.d
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

def print_schedule(node):
	if node.color == 'white':
		node.color = 'black'
		print node.type," @ ", node.start_line
		print "\t start: ", node.arrival
		print "\t deadline: ", node.d
		print "\t flow: ", node.id
	for n in node.children:
		print_schedule(n)

def create_schedule(graph, num_cores):
	mapped = []
	schedule = ET.Element('Schedule')
	cores = ET.SubElement(schedule, 'Cores')
	cores.text = str(num_cores)
	task_list = generate_task(graph)
	tree = ET.ElementTree(schedule)
	for task in task_list:
		if 'splitted' in task.type:
			serialize_splitted(task, schedule, mapped)
		elif 'BARRIER' not in task.type:
			pragma = ET.SubElement(schedule, 'Pragma')
			id = ET.SubElement(pragma, 'id')
			id.text = str(task.start_line)
			pragma_type = ET.SubElement(pragma, 'Type')
			pragma_type.text = str(task.type)
			threads = ET.SubElement(pragma, 'Threads')
			thread = ET.SubElement(threads, 'Thread')
			thread.text = str(task.id)
			start = ET.SubElement(pragma, 'Start_time')
			start.text = str(task.arrival)
			end = ET.SubElement(pragma, 'Deadline')
			end.text = str(task.d)
		if len(task.children) > 1:
			l = []
			barrier = ET.SubElement(pragma, 'Barrier')
			first = ET.SubElement(barrier, 'id')
			first.text = str(task.start_line)
			for c in task.children:
				if c.start_line not in l:
					tmp_id = ET.SubElement(barrier, 'id')
					tmp_id.text = str(c.start_line)
					l.append(c.start_line)

	par.indent(tree.getroot())			
	tree.write('schedule.xml')

def serialize_splitted(task, schedule, mapped):
	if task.start_line not in mapped:
		pragma = ET.SubElement(schedule, 'Pragma')
		id = ET.SubElement(pragma, 'id')
		id.text = str(task.start_line)
		pragma_type = ET.SubElement(pragma, 'Type')
		pragma_type.text = 'OMPForDirective'
		threads = ET.SubElement(pragma, 'Threads')
		thread = ET.SubElement(threads, 'Thread')
		thread.text = str(task.id)
		start = ET.SubElement(pragma, 'Start_time')
		start.text = str(task.arrival)
		end = ET.SubElement(pragma, 'Deadline')
		end.text = str(task.d)
		mapped.append(task.start_line)
	else:
		for p in schedule.findall("Pragma"):
			if p.find('id').text == task.start_line:
				threads_ = p.find('Threads')
				thread = ET.SubElement(threads_, 'Thread')
				thread.text = str(task.id)
			









