import sys
import pargraph as par
import copy
import schedule as sched
import profiler as pro
import threading
import time


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
	gen = sched.generate_task(main_flow)
	task_list = []
	for task in gen:
		task_list.append(task)

	#getting the number of physical cores of the machine
	max_flows = sched.get_core_num(profile_xml)
	flow_list = []
	optimal_flow = []
	start_time = time.clock()
	sched.get_optimal_flow(flow_list, task_list, 0, optimal_flow, num_tasks, max_flows, start_time, execution_time)

	par.add_new_tasks(optimal_flow, exp_flows)

	#sched.chetto(main_flow, 12 optimal_flow)

	print "best solution:"
	for flow in optimal_flow:
		flow.dump("\t")
		print "\ttime:",flow.time

	#prints the flow graphs
	if(output == "True"):
		for g in exp_flows:
			sched.make_white(g)
			par.scanGraph(g)

	



