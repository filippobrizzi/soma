import sys
import pargraph as par
import copy
import schedule as sched
import profiler as pro
import time
import multiprocessing
import itertools
import random
import threading




""" Usage: call with <filename> <pragma_xml_file> <executable_name> <profiling_interations> True/False (for output)
"""

if __name__ == "__main__":
	
	pragma_xml = sys.argv[1]
	executable = sys.argv[2]
	count = int(sys.argv[3])
	output = sys.argv[4]
	execution_time = float(sys.argv[5])
	deadline = float(sys.argv[6])
	multi = sys.argv[7]

	#runs count time the executable and aggregates the informations in executable_profile.xml. The single profile outputs are saved as profile+iter.xml
	profile_xml = pro.profileCreator(count, executable)

	#return the nested dot graphs in code style (one for each function)
	visual_nested_graphs = par.getNesGraph(pragma_xml, profile_xml)

	#returns the graphs to be visualized and the object graphs in flow style (one for each function)
	(visual_flow_graphs, flow_graphs) = par.getParalGraph(pragma_xml, profile_xml) 

	i = 0
	for g in visual_nested_graphs:
		g.write_pdf('graphs/%s_code.pdf'%flow_graphs[i].type)
		g.write_dot('graphs/%s_code.dot'%flow_graphs[i].type)
		i += 1

	i = 0
	for g in visual_flow_graphs:
		g.write_pdf('graphs/%s_flow.pdf'%flow_graphs[i].type)
		g.write_dot('graphs/%s_flow.dot'%flow_graphs[i].type)
		i += 1

	#creates the flow type graph --> flow.xml
	par.dump_graphs(flow_graphs)
	#adding to the original xml the profiling informations --> code.xml
	pro.add_profile_xml(profile_xml, pragma_xml)
	#creating the total graph with the call-tree
	func_graph = par.create_complete_graph(visual_flow_graphs, profile_xml)
	#creating the graphs with the function calls
	func_graph.write_pdf('graphs/function_graphs.pdf')
	func_graph.write_dot('graphs/function_graphs.dot')

	

	#creating the expanded graph where the functions are inserted in the flow graph
	exp_flows = copy.deepcopy(flow_graphs)
	par.explode_graph(exp_flows)
	main_flow = sched.get_main(exp_flows)
	
	#creating a generator for the expanded graph
	gen = sched.generate_task(main_flow)
	
	#creating a new generator for the expanded graph
	sched.make_white(main_flow)

	#getting the number of physical cores of the machine profiled
	max_flows = sched.get_core_num(profile_xml)

	#getting cores of the actual machine
	cores = multiprocessing.cpu_count() / 2
	
	#initializing all the lists for the parallel scheduling algorithm
	tasks_list = []
	task_list = []
	flows_list = []
	optimal_flow_list = []
	p_list = []
	queue_list = []
	results = []
	num_tasks = 0

	#getting the number of tasks in the expanded graph and creating a list of task
	for task in gen:
		task_list.append(task)
		num_tasks += 1
	
	#use just one core for a small number of tasks
	if len(task_list) < 10:
		cores = 1


	if multi == 'parallel':
		for core in range(cores):
			tmp = []
			optimal_flow_list.append(tmp)
			tmp_2 = []
			flows_list.append(tmp_2)
			random.shuffle(task_list)
			tasks_list.append(copy.deepcopy(task_list))
			q = sched.Queue()
			queue_list.append(q)
			p_list.append(multiprocessing.Process(target = sched.get_optimal_flow, args = (flows_list[core], tasks_list[core], 0, optimal_flow_list[core], num_tasks, max_flows, execution_time, queue_list[core],  )))
			print "starting core: ",core
			p_list[core].start()
		#getting the results from the processes
		for queue in queue_list:
			t = queue.q.get()
			results.append(t)
		#joining all the processes
		i = 0
		for p in p_list:
			p.join()
			print "core ", i, " joined"
			i += 1
		#getting the best result
		optimal_flow = results[0]
		best = 0
		for i in range(len(results)):
			for flow in results[i]:
				if sched.get_cost(results[i]) < sched.get_cost(optimal_flow):
					best = i
		optimal_flow = results[best]
	else:
			optimal_flow = []
			flow_list = []
			execution_time += time.clock()
			print "searching best schedule"
			sched.get_optimal_flow_single(flow_list, task_list, 0, optimal_flow, num_tasks, max_flows, execution_time )
			#p_list.append(threading.Thread(target = sched.get_optimal_flow, args = (flows_list[core], tasks_list[core], 0, optimal_flow_list[core], num_tasks, max_flows, start_time, execution_time, )))

	

	#printing the best result	
	print "solution:"
	for flow in optimal_flow:
		flow.dump("\t")
		print "\ttime:",flow.time

	#substitutes for tasks with splitted versions if present in the optimal flows
	par.add_new_tasks(optimal_flow, main_flow)
	sched.make_white(main_flow)
	gen_ = sched.generate_task(main_flow)

	t_list = []
	for t in gen_:
		t_list.append(t)

	par.add_flow_id(optimal_flow, t_list)

	#sets arrival times and deadlines using a modified version of the chetto algorithm
	sched.chetto(main_flow, deadline, optimal_flow)
	
	sched.make_white(main_flow)
	sched.print_schedule(main_flow)
	
	if sched.check_schedule(main_flow):
		sched.create_schedule(main_flow, len(optimal_flow))
	else:
		print "tasks not schedulable, try with more search time"

	if output == 'True':
		sched.make_white(main_flow)
		par.scanGraph(main_flow)

	
	
	



