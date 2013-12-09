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
	execution_time = int(sys.argv[5])

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

	#getting the number of tasks in the expanded graph
	num_tasks = 0
	for node in gen:
		num_tasks += 1

	#creating a new generator for the expanded graph
	sched.make_white(main_flow)

	#getting the number of physical cores of the machine profiled
	max_flows = sched.get_core_num(profile_xml)

	#getting cores of the actual machine
	cores = multiprocessing.cpu_count() / 2
	
	tasks_list = []
	task_list = []
	flows_list = []
	optimal_flow_list = []
	p_list = []
	queue_list = []
	gen = sched.generate_task(main_flow)

	for task in gen:
		task_list.append(task)
	
	for core in range(cores):
		tmp = []
		optimal_flow_list.append(tmp)
		tmp_2 = []
		flows_list.append(tmp_2)
		random.shuffle(task_list)
		tasks_list.append(copy.deepcopy(copy.deepcopy(task_list)))
		q = sched.Queue()
		queue_list.append(q)



	start_time = time.clock()
	

	for core in range(cores):
		#p_list.append(threading.Thread(target = sched.get_optimal_flow, args = (flows_list[core], tasks_list[core], 0, optimal_flow_list[core], num_tasks, max_flows, start_time, execution_time, )))
		p_list.append(multiprocessing.Process(target = sched.get_optimal_flow, args = (flows_list[core], tasks_list[core], 0, optimal_flow_list[core], num_tasks, max_flows, start_time, execution_time, queue_list[core],  )))
		print "starting core: ",core
		p_list[core].start()

	results = []
	for queue in queue_list:
		t = queue.q.get()
		results.append(t)

	i = 0
	for p in p_list:
		p.join()
		print "core ", i, " joined"
		i += 1

	optimal_flow = results[0]
	best = 0
	for i in range(len(results)):
		if sched.get_cost(results[i]) < sched.get_cost(optimal_flow):
			best = i

			
	print "solution:"
	for flow in results[best]:
		flow.dump("\t")
		print "\ttime:",flow.time

	"""
	optimal_flow = optimal_flow_list[0]

	for flow in optimal_flow_list:
		if sched.get_cost(flow) < optimal_flow:
			optimal_flow = flow



	par.add_new_tasks(optimal_flow, main_flow)

	sched.chetto(main_flow, 12, optimal_flow)


	print "best solution:"
	for flow in optimal_flow:
		flow.dump("\t")
		print "\ttime:",flow.time
	
	sched.make_white(main_flow)
	#sched.print_schedule(main_flow)
	
	sched.create_schedule(main_flow, len(optimal_flow))


	if output == 'True':
		sched.make_white(main_flow)
		par.scanGraph(main_flow)
	"""
	
	



