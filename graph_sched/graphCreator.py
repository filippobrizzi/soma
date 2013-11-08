import sys
import pargraph as par
import copy

""" Usage: call with <filename> <pragma_xml_file> <executable_name> <profiling_interations> True/False (for output)
"""

if __name__ == "__main__":
	

	pragma_xml = sys.argv[1]
	executable = sys.argv[2]
	count = int(sys.argv[3])
	output = sys.argv[4]

	#runs count time the executable and aggregates the informations in executable_profile.xml. The single profile outputs are saved as profile+iter.xml
	profile_xml = par.profileCreator(count, executable)

	#return the nested dot graphs in code style (one for each function)
	visual_nested_graphs = par.getNesGraph(pragma_xml, profile_xml)

	#returns the graphs to be visualized and the object graphs in flow style (one for each function)
	(visual_flow_graphs, flow_graphs) = par.getParalGraph(pragma_xml, profile_xml) 

	i=0

	for g in visual_nested_graphs:
		g.write_png('graphs/%s_code.png'%flow_graphs[i].type)
		g.write_dot('graphs/%s_code.dot'%flow_graphs[i].type)
		i = i + 1
	i = 0
	for g in visual_flow_graphs:
		g.write_png('graphs/%s_flow.png'%flow_graphs[i].type)
		g.write_dot('graphs/%s_flow.dot'%flow_graphs[i].type)
		i  = i + 1

	#creates the flow type graph --> flow.xml
	par.dump_graphs(flow_graphs)

	#adds to the original xml the profiling informations --> code.xml
	par.add_profile_xml(profile_xml, pragma_xml)

	#creates the total graph with the call-tree
	func_graph = par.create_complete_graph(visual_flow_graphs, profile_xml)

	func_graph.write_png('graphs/function_graphs.png')
	func_graph.write_dot('graphs/function_graphs.dot')

	#for flow_graph in flow_graphs:
	#par.chetto(flow_graphs[1],10)
	exp_flows = copy.deepcopy(flow_graphs)
	par.explode_graph(exp_flows)
	node_list = []
	#par.scanGraph(exp_flows[1], node_list)

	#prints the object flow graphs
	l = []
	if(output == "True"):
		for g in flow_graphs:
			par.scanGraph(g, l)

	



