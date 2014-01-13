import pydot as p
import profiler as pro
import xml.etree.cElementTree as ET
from random import randrange
import copy
import schedule as sched
import re
import math

colors = (	"beige",  "bisque3",	"bisque4",	"blanchedalmond",	   "blue",  
"blue1",	"blue2",	"blue3",	"blue4",	"blueviolet",
"brown",	"brown1",	"brown2",   "brown3", 	"brown4",
"burlywood",	"burlywood1",	"burlywood2",	"burlywood3",	"burlywood4",
"cadetblue",	"cadetblue1",	"cadetblue2",	"cadetblue3",	"cadetblue4",
"chartreuse",	"chartreuse1",	"chartreuse2",	"chartreuse3",	"chartreuse4",
"chocolate",	"chocolate1",	"chocolate2",	"chocolate3",	"chocolate4",
"coral",	"coral1",	"coral2",	"coral3",	"coral4",
"cornflowerblue",	"crimson",	   "cyan",   	"cyan1",	"cyan2",
"cyan3",	"cyan4",	"darkgoldenrod",	"darkgoldenrod1",	"darkgoldenrod2",
"darkgoldenrod3",	"darkgoldenrod4",	"darkgreen",	"darkkhaki",	"darkolivegreen",
"darkolivegreen1",	"darkolivegreen2",	"darkolivegreen3",	"darkolivegreen4",	"darkorange",
"darkorange1",	"darkorange2",	"darkorange3",	"darkorange4",	"darkorchid",
"darkorchid1",	"darkorchid2",	"darkorchid3",	"darkorchid4",	"darksalmon",
"darkseagreen",	"darkseagreen1",	"darkseagreen2",	"darkseagreen3",	"darkseagreen4",
"darkslateblue",	"darkslategray",	"darkslategray1",	"darkslategray2",	"darkslategray3",
"darkslategray4",	"darkslategrey",	"darkturquoise",	"darkviolet",	"deeppink",
"deeppink1",	"deeppink2",	"deeppink3",	"deeppink4",	"deepskyblue",
"deepskyblue1",	"deepskyblue2",	"deepskyblue3",	"deepskyblue4",	"dimgray",
"dimgrey",	"dodgerblue",	"dodgerblue1",	"dodgerblue2",	"dodgerblue3",
"dodgerblue4",	"firebrick",	"firebrick1",	"firebrick2",	"firebrick3",
"firebrick4",	"forestgreen", "gold",   	"gold1",	"gold2",	
"gold3",	"gold4",    "goldenrod",	"goldenrod1",	"goldenrod2",	"goldenrod3",	"goldenrod4")

class Node(object):
    def __init__(self, Ptype, s_line, time, variance):
        self.type = Ptype
        self.start_line = s_line
        self.children = []
        self.parent = []
        self.options = []
        self.time = float(time)
        self.variance = variance
        self.end_line = 0
        self.callerid = []
        self.deadline = None
        self.arrival = None
        self.d = None
        self.children_time = 0
        self.in_time = 0
        self.color = 'white'
        self.id = None
    def add(self, x):	
    	x.parent.append(self)
    	self.children.append(x)
    def myself(self):
		if self.type != 'BARRIER':
			print "pragma node: ", self.type, "\n    start_line: ", self.start_line, "\n    endl_line", self.end_line
			if self.type.find("_end") == -1:
				if self.time != 0:
					print "    time: ", self.time
					print "    variance: ", self.variance
					print "    children time: ", self.children_time
					print "    self time: ", self.in_time
				else:
					print "    not executed"
				if(len(self.options) != 0):
					print "    Options:"
					for i in self.options:
						print "        ",i[0]," ",i[1]
			print "     chetto deadline :", self.d
			print "     chetto arrival :", self.arrival
		else:
			print "pragma node: ", self.type, "\n    start_line: ", self.start_line
		print

class For_Node(Node): 
    def __init__(self, Ptype, s_line, init_type, init_var, init_value, init_cond, init_cond_value, init_increment, init_increment_value, time, variance, mean_loops):
        Node.__init__(self, Ptype, s_line, time, variance)
        self.init_type = init_type
        self.init_var = init_var
        self.init_value = init_value
        self.init_cond = init_cond
        self.init_cond_value = init_cond_value
        self.init_increment = init_increment
        self.init_increment_value = init_increment_value
        self.mean_loops = mean_loops
    def myself(self):
		print "for node: ", self.type, "\n    start_line: ", self.start_line, "\n    endl_line: ", self.end_line, "\n    init_type:", self.init_type, "\n    init_var: ", self.init_var, "\n    init_value: ", self.init_value,"\n    init_condition: ", self.init_cond, "\n    init_condition_value: ", self.init_cond_value, "\n    init_increment_type: ", self.init_increment,"\n    init_increment: ", self.init_increment_value, "\n    mean_loops:", self.mean_loops
		print "     chetto deadline :", self.d
		print "     chetto arrival :", self.arrival
		if(len(self.options) != 0):
			print "    Options:"
			for i in self.options:
				print "        ", i[0], " ", i[1]
		if self.time != 0:
			print "    time: ", self.time
			print "    variance: ", self.variance
			print "    children time: ", self.children_time,"\n"
			print "    self time: ", self.in_time,"\n"
		else:
			print "    not executed\n"

class Fx_Node(Node):
	def __init__(self, Ptype, line, returnType, time, variance, file_name):
		Node.__init__(self, Ptype, line, time, variance)
		self.arguments = []
		self.returnType = returnType
		self.time = float(time)
		self.file_name = file_name
	def add_arg(self, type_):
		self.arguments.append(type_)
	def myself(self):
		print "function node: ", self.type, "() {\n    line: ", self.start_line, "\n    return type: ", self.returnType
		print "     chetto deadline :", self.d
		print "     chetto arrival :", self.arrival
		if(len(self.arguments) != 0):
			print "    Parameters: "
			i = 0
			for par in self.arguments:
				print "   ", i, ") ", par[0], " ", par[1]
				i = i + 1
		else:
			print "    No input parameters"
		if self.time != 0:
			print "    time: ", self.in_time
			print "    variance: ", self.variance
			print "    children time: ", self.children_time,"\n}\n"
		else:
			print "    not executed\n}\n"

class Function():
	def __init__(self, time, variance, children_time):
		self.time = float(time)
		self.variance = variance
		self.pragmas = {}
		self.children_time = float(children_time)
		self.in_time = float(self.time) - float(self.children_time)
	def add_pragma(self, pragma):
		self.pragmas[pragma[0]] = (pragma[1], pragma[2], pragma[3], pragma[4], pragma[5])

class Architecture():
	def __init__(self, num_cores, tot_memory):
		self.num_cores = num_cores
		self.tot_memory = tot_memory

class Time_Node():
	def __init__(self, func_line, pragma_line ):
		self.times = []
		self.func_line = func_line
		self.pragma_line = pragma_line
		self.variance = 0
		self.loops = []
		self.caller_list = []
		self.children_time = []

class Flow():
	def __init__(self):
		self.tasks = []
		self.bandwidth = 0
		self.time = 0
	def add_task(self, task):
		self.tasks.append(task)
		self.update(task)
	def update(self, task):
		self.time += task.in_time #float(task.time) - float(task.children_time)
	def dump(self,prefix=""):
		print prefix,"flow:"
		for task in self.tasks:
			print prefix, "\t", task.type, " ", task.start_line, " ", task.in_time, " id ", task.id
	def remove_task(self, task):
		self.tasks.remove(task)
		self.time -= task.in_time #float(task.time) - float(task.children_time)

class Task():
	def __init__(self, count, id):
		self.count = count
		self.id = []
		self.id.append(id)


def scanGraph(node):
	#print pre, node.type
	if node.color != 'black':
		node.color = 'black'
		node.myself()
		print "     has children:"
		for c in node.children:
			print "          ",c.type,"@",c.start_line
		print "     has parent:"
		for p in node.parent :
			print "          ",p.type,"@",p.start_line
		for n in node.children:
			scanGraph(n)

def indent(elem, level=0):
    i = "\n" + level * "  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            indent(elem, level + 1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i

def getParalGraph(pragma_xml, profile_xml):
	pragma_graph_root = ET.ElementTree(file = pragma_xml).getroot()
	profile_graph_root = ET.ElementTree(file = profile_xml).getroot()

	functions = pro.getProfilesMap(profile_xml)
	objGraph = []
	graphs = []
	count = 0
	arch = Architecture(profile_graph_root.find('Hardware/NumberofCores').text, profile_graph_root.find('Hardware/MemorySize').text)
	
	file_name = pragma_graph_root.find('Name').text
	
	for n in pragma_graph_root.findall('Function'):
		graphs.append(p.Dot(graph_type = 'digraph'))
		name = n.find('Name').text
		time = float(functions[n.find('Line').text].time)
		callerid = functions[n.find('Line').text].callerid
		children_time = float(functions[n.find('Line').text].children_time)
		root = n.find('Line').text
		if (time == 0):
			pragma_graph_root = p.Node(n.find('Line').text, label = name + "()\nnot executed", root = root)
		else:
			pragma_graph_root = p.Node(n.find('Line').text, label = name + "()\nexecution time %g" % time, root = root)
		pragma_graph_root.callerid = callerid
		graphs[count].add_node(pragma_graph_root)
		Objroot = Fx_Node(name, n.find('Line').text,n.find('ReturnType').text, float(functions[n.find('Line').text].time), functions[n.find('Line').text].variance, file_name)
		for par in n.findall('Parameters/Parameter'):
			Objroot.add_arg( ( par.find('Type').text,par.find('Name').text ) )
		Objroot.children_time = children_time
		Objroot.in_time = Objroot.time - children_time
		for caller in functions[n.find('Line').text].callerid:
			Objroot.callerid.append(caller)
		objGraph.append(Objroot)
		scan(n, graphs[count], pragma_graph_root, objGraph[count], functions[n.find('Line').text].pragmas, root)
		count = count + 1
	return (graphs, objGraph)

def scan(xml_tree, pragma_graph, node, treeNode, func_pragmas, root):
	for d in xml_tree.find('Pragmas').findall('Pragma'):
		end_line = d.find('Position/EndLine').text
		key = d.find('Position/StartLine').text

		if key not in func_pragmas:
			time = 0
			variance = None
			loops = 0
			callerid = None
			children_time = 0
		else:
			time = float(func_pragmas[key][0])
			variance = func_pragmas[key][1]
			loops = func_pragmas[key][2]
			callerid = func_pragmas[key][3]
			children_time =  float(func_pragmas[key][4])

		tmp_name = d.find('Name').text.replace("::", " ")
		visual_name = tmp_name+"@%s"%key

		if ("For" in tmp_name ):
			if (d.find('For/Declaration/InitValue') != None):
				init_value = d.find('For/Declaration/InitValue').text
			else:
				init_value = d.find('For/Declaration/InitVariable').text
			if (d.find('For/Condition/ConditionValue') != None):
				init_var = d.find('For/Condition/ConditionValue').text
			else:
				init_var = d.find('For/Condition/ConditionVariable').text
			if(d.find('For/Increment/IncrementValue') != None):
				inc = d.find('For/Increment/IncrementValue').text
			else:
				inc = ""
			Objchild = For_Node(tmp_name, d.find('Position/StartLine').text, d.find('For/Declaration/Type').text, d.find('For/Declaration/LoopVariable').text, init_value, d.find('For/Condition/Op').text, init_var, d.find('For/Increment/Op').text, inc, time, variance, loops )
			visual_name = visual_name + "\nfor( " + Objchild.init_var + " = " + Objchild.init_value + "; " + Objchild.init_var + " " + Objchild.init_cond + " " + Objchild.init_cond_value + "; " + Objchild.init_var + " " + Objchild.init_increment + " " + Objchild.init_increment_value + ")"
		else:
			Objchild = Node(tmp_name, key, time, variance )

		deadline = None
		if(d.find('Options')):
			for op in d.findall('Options/Option'):
				Objchild.options.append( (op.find('Name').text,[get_parameter(i) for i in op.findall('Parameter')]) )
				if op.find('Name').text == 'deadline':
					deadline = op.find('Parameter').text
		Objchild.end_line = end_line
		Objchild.callerid.append(callerid)
		Objchild.deadline = deadline
		Objchild.children_time = children_time
		Objchild.in_time = Objchild.time - children_time
		if (time == 0):
			child = p.Node(key, label = visual_name + "\nnot executed", root = root)
		else:
			child = p.Node(key, label = visual_name + "\nexecution time: " + str(time) + "\nvariance: " + str(variance), root = root)
		pragma_graph.add_node(node)
		pragma_graph.add_node(child)
		pragma_graph.add_edge(p.Edge(node, child))
		treeNode.add(Objchild)
		#print Objchild.type,"@",Objchild.start_line," is attached to ",treeNode.type,"@",treeNode.start_line

		if(d.find('Children')):
			node_ = create_diamond(d.find('Children'), pragma_graph, child, Objchild, func_pragmas, root)
			tmp_name = (node_.start_line)
			if tmp_name not in func_pragmas:
				time = 0
			else:
				time = func_pragmas[tmp_name][0]
			#treeNode = Node('BARRIER_end', tmp_name, 0, 0)
			#Objchild.add(treeNode)
			treeNode = node_
			node = p.Node(tmp_name + "_end", label = "BARRIER", root = root)
		else:
			node = child
			treeNode = Objchild

def create_diamond(tree, graph, node, treeNode, func_pragmas, root):
	special_node = p.Node(node.get_name().replace("\"", "") + "_end", label = 'BARRIER', root = root)
	Objspecial_node = Node( 'BARRIER_end' , node.get_name() , 0, 0 )
	color = colors[randrange(len(colors) - 1)]
	for d in tree.find('Pragmas').findall('Pragma'):

		end_line = d.find('Position/EndLine').text
		key = d.find('Position/StartLine').text

		if key not in func_pragmas:
			time = 0
			variance = None
			loops = 0
			callerid = None
			children_time = 0
		else:
			time = float(func_pragmas[key][0])
			variance = func_pragmas[key][1]
			loops = func_pragmas[key][2]
			callerid = func_pragmas[key][3]
			children_time = float(func_pragmas[key][4])

		tmp_name = d.find('Name').text.replace("::", " ")
		visual_name = tmp_name + "@%s" % key

		if ("For" in tmp_name ):
			loops = func_pragmas[key][2]
			if (d.find('For/Declaration/InitValue') != None):
				init_value = d.find('For/Declaration/InitValue').text
			else:
				init_value = d.find('For/Declaration/InitVariable').text
			if (d.find('For/Condition/ConditionValue') != None):
				init_var = d.find('For/Condition/ConditionValue').text
			else:
				init_var = d.find('For/Condition/ConditionVariable').text
			if(d.find('For/Increment/IncrementValue') != None):
				inc = d.find('For/Increment/IncrementValue').text
			else:
				inc = ""
			Objchild = For_Node(tmp_name, key, d.find('For/Declaration/Type').text, d.find('For/Declaration/LoopVariable').text, init_value, d.find('For/Condition/Op').text, init_var, d.find('For/Increment/Op').text, inc , time, variance, loops)
			visual_name = visual_name + "\nfor( " + Objchild.init_var + " = " + Objchild.init_value + "; "+Objchild.init_var + " " + Objchild.init_cond + " " + Objchild.init_cond_value + "; " + Objchild.init_var + " " + Objchild.init_increment + " " + Objchild.init_increment_value + ")"
		else:
			Objchild = Node(tmp_name, key, time, variance)

		deadline = None
		if(d.find('Options')):
			for op in d.find('Options').findall('Option'):
				Objchild.options.append( (op.find('Name').text,[get_parameter(i) for i in op.findall('Parameter')]) )
				if op.find('Name').text == 'deadline':
					deadline = op.find('Parameter').text

		Objchild.end_line = end_line
		Objchild.callerid.append(callerid)
		Objchild.deadline = deadline
		Objchild.children_time = children_time
		Objchild.in_time = Objchild.time - children_time

		child = p.Node(key, label = visual_name + "\nexecution time: " + str(time) + "\nvariance: " + str(variance), root = root)
		graph.add_node(node)
		graph.add_node(child)
		graph.add_edge(p.Edge(node, child, color = color))
		treeNode.add(Objchild)

		if(d.find('Children')):
			#get the real returned label as name
			tmp_node = create_diamond(d.find('Children'), graph, child, Objchild, func_pragmas, root)
			g_node = p.Node(tmp_node.start_line+ "_end", label = 'BARRIER', root = root)
			graph.add_node(g_node)
			graph.add_node(special_node)
			graph.add_edge(p.Edge(g_node, special_node, color = color))
			#tmp_name = tmp.get_name().replace("\"", "") 
			#ObjTmp = Node(tmp_name, tmp_name, 0, 0)
			tmp_node.add(Objspecial_node)
		else:
			graph.add_node(child)
			graph.add_node(special_node)
			graph.add_edge(p.Edge(child, special_node, color = color))
			Objchild.add(Objspecial_node)
	return Objspecial_node

def find_nesting(tree, graph, node, func_pragmas, pre = ""):
	color = colors[randrange(len(colors) - 1)]
	for d in tree.find('Pragmas').findall('Pragma'):
		key = d.find('Position/StartLine').text
		if(key in func_pragmas):
			time = "\n execution time: " + str(func_pragmas[key][0])
			variance = "\nvariance: " + str(func_pragmas[key][1])
		else:
			time = "\nnot executed"
			variance = ""
		name = d.find('Name').text.replace("::"," ") + "@%s" % key
		child = p.Node(name, label = name + time + variance)
		graph.add_node(node)
		graph.add_node(child)
		graph.add_edge(p.Edge(node, child, color = color ))
		#print pre+name
		if(d.find('Children')):
			find_nesting(d.find('Children'), graph, child, func_pragmas, pre + " ")

def getNesGraph(xml, profile_xml):
	tree = ET.ElementTree(file = xml) 
	profile_graph_root = ET.ElementTree(file = profile_xml).getroot()
	functions = pro.getProfilesMap(profile_xml)

	root = tree.getroot()
	graphs = []
	count = 0

	for n in root.iter('Function'):
		key = n.find('Line').text
		time = float(functions[key].time)
		variance = functions[key].variance
		graphs.append(p.Dot(graph_type = 'digraph'))
		name = n.find('Name').text
		if (time == 0):
			root = p.Node(name, label = name + "()" + "\nnot executed")
		else:
			root = p.Node(name, label = name + "()" + "\n execution time: %f" % time + "\nvariance: " + str(variance))
		graphs[count].add_node(root)
		find_nesting(n, graphs[count], root, functions[key].pragmas)
		count += 1

	return graphs

def create_complete_graph(visual_flow_graphs, profile_xml):
	func_graph = p.Dot(graph_type = 'digraph', compound = 'true')
	clusters = []

	i = 0

	for func in visual_flow_graphs:
		clusters.append(p.Cluster(str(i)))
		for node in func.get_nodes():
			clusters[i].add_node(node)
		for edge in func.get_edge_list():
			clusters[i].add_edge(edge)
		func_graph.add_subgraph(clusters[i])
		i +=  1

	functions_callers = pro.get_table(profile_xml)
	
	for func in visual_flow_graphs:
		root = func.get_nodes()[0].obj_dict['attributes']['root']
		if len(functions_callers[root]) > 0 :
			for caller in functions_callers[root]:
				func_graph.add_edge(p.Edge(caller, root))

	return func_graph

def dump_graphs(flow_graphs):
	root = ET.Element('File')
	name = ET.SubElement(root, 'Name')
	name.text = flow_graphs[0].file_name
	graph_type = ET.SubElement(root, 'GraphType')
	graph_type.text = "flow"
	for func in flow_graphs:
		function = ET.SubElement(root, 'Function')
		function.attrib['id'] = str(func.start_line) + str(func.end_line)
		func_name = ET.SubElement(function, 'Name')
		func_name.text = func.type
		returnType = ET.SubElement(function, 'ReturnType')
		returnType.text = func.returnType
		if len(func.arguments) != 0:
			parameters = ET.SubElement(function, 'Parameters')
			for par in func.arguments:
				parameter = ET.SubElement( parameters, 'Parameter')
				type_ = ET.SubElement( parameter, 'Type')
				type_.text = par[0]
				name_ = ET.SubElement( parameter, 'Name')
				name_.text = par[1]
		line = ET.SubElement(function, 'Line')
		line.text = func.start_line 
		time = ET.SubElement(function, 'Time')
		time.text = str(func.time)
		variance = ET.SubElement(function, 'Variance')
		variance.text = str(func.variance)
		func.xml_parent = None
		if ( func.callerid != None ):
			callerids = ET.SubElement(function, 'Callerids')
			for id_ in func.callerid:
				callerid = ET.SubElement(callerids, 'Callerid')
				callerid.text = id_
		if len(func.children) != 0:
			pragma_list = []
			edge_list = []
			pragmas = ET.SubElement(function, 'Nodes')
			dump_pragmas(func, pragmas, pragma_list)
			edges = ET.SubElement(function, 'Edges')
			dump_edges(func, edges, edge_list)

	tree = ET.ElementTree(root)
	indent(tree.getroot())
	tree.write('flow.xml')

def dump_pragmas(pragma_node, pragmas_element, pragma_list):
	for pragma in pragma_node.children:
		if str(pragma.start_line) + str(pragma.end_line) not in pragma_list:
			pragma_list.append(str(pragma.start_line) + str(pragma.end_line))
			pragma_ = ET.SubElement(pragmas_element, 'Pragma')
			pragma_.attrib['id'] = str(pragma.start_line) + str(pragma.end_line)
			name = ET.SubElement(pragma_, 'Name')
			if not "_end" in pragma.type:
				name.text = pragma.type
			else:
				name.text = "BARRIER"
			if(len(pragma.options) != 0):
				options = ET.SubElement(pragma_, 'Options')
				for op in pragma.options:
					option = ET.SubElement(options, 'Option')
					op_name = ET.SubElement(option, 'Name')
					op_name.text = op[0]
					for par in op[1]:
						op_parameter = ET.SubElement(option, 'Parameter')
						op_var = ET.SubElement(op_parameter, 'Var')
						op_var.text = par[1]
						op_type = ET.SubElement(op_parameter, 'Type')
						op_type.text = par[0]
			position = ET.SubElement(pragma_, 'Position')
			start = ET.SubElement(position, 'StartLine')
			start.text = pragma.start_line
			if(name.text != "BARRIER"):
				end = ET.SubElement(position, 'EndLine')
				end.text = pragma.end_line
			if (pragma.callerid != None ):
				callerids = ET.SubElement(pragma_, 'Callerids')
				for id_ in pragma.callerid:
					callerid = ET.SubElement(callerids, 'Callerid')
					callerid.text = id_
			if(pragma.time != 0):
				time = ET.SubElement(pragma_, 'Time')
				time.text = str(pragma.time)
			if(pragma.variance != None):
				variance = ET.SubElement(pragma_, 'Variance')
				variance.text = str(pragma.variance)

		dump_pragmas(pragma, pragmas_element, pragma_list)

def dump_edges(pragma_node, edges_element, pragma_list):
	for pragma in pragma_node.children:
		if pragma_node.start_line + pragma.start_line not in pragma_list:
			pragma_list.append(pragma_node.start_line+pragma.start_line)
			edge = ET.SubElement(edges_element, 'Edge')
			source = ET.SubElement(edge, 'Source')
			source.text = str(pragma_node.start_line) + str(pragma_node.end_line)
			dest = ET.SubElement(edge, 'Dest')
			dest.text = str(pragma.start_line) + str(pragma.end_line)
		dump_edges(pragma,edges_element, pragma_list)

def find_node(node, flow_graphs):
	for function in flow_graphs:
		tmp_node = find_sub_node(node, function) 
		if tmp_node != None :
			return tmp_node

def find_node2(key_start, key_parent, flow_graphs):
	tmp_node = find_sub_node2(key_start, key_parent, flow_graphs) 
	if tmp_node != None :
		return tmp_node

def find_sub_node2(key_start, key_parent, function):
	if (function.start_line) == key_start and ('BARRIER' not in function.type):
			return function
	for child in function.children:
		if (child.start_line) == key_start and ('BARRIER' not in child.type) and child.parent[0].start_line == key_parent:
			return child
		else:
			tmp_node = find_sub_node2(key_start, key_parent, child)
		if tmp_node != None:
			return tmp_node
	return None

def find_sub_node(node, function):
	if (function.start_line) == node and ('BARRIER' not in function.type):
			return function
	for child in function.children:
		if (child.start_line) == node and ('BARRIER' not in child.type):
			return child
		else:
			tmp_node = find_sub_node(node, child)
		if tmp_node != None:
			return tmp_node
	return None

class Caller():
	def __init__(self, original_caller, used_caller):
		self.original_caller = original_caller
		self.used_caller = used_caller
		self.old_children = []

#adding to the main graph all the function which are called taking care of multiple connections between pragma and caller
def explode_graph(flow_graphs):
	setted_callers = {}
	for function in flow_graphs:
		count = 0
		caller_list = function.callerid
		if caller_list != None:
			for caller in caller_list:
				function_copy = copy.deepcopy(function)
				count += 1
				caller_node = find_node(caller, flow_graphs)
				if caller_node.start_line not in setted_callers:
					setted_callers[caller_node.start_line] = Caller(copy.copy(caller_node), caller_node)
					function_copy.parent.append(caller_node)
					children_list = []
					for child in caller_node.children:
						children_list.append(child)
						child.parent.remove(caller_node)
						setted_callers[caller_node.start_line].old_children.append(child)
					caller_node.children = []
					caller_node.children.append(function_copy)
					last_node = sched.get_last(function_copy)
					last_node.children = children_list
					for child in children_list:
						child.parent.append(last_node)
				else:
					children_list = []
					for child in setted_callers[caller_node.start_line].old_children:
						children_list.append(child)
					function_copy.parent.append(setted_callers[caller_node.start_line].used_caller)
					setted_callers[caller_node.start_line].used_caller.children.append(function_copy)
					last_node = sched.get_last(function_copy)
					last_node.children = children_list
					for child in children_list:
						child.parent.append(last_node)

					

def get_parameter(parameter):
	if parameter.find('Type') != None:
		type_ = parameter.find('Type').text
	else:
		type_ = 'None'
	return (type_, parameter.find('Var').text)

def create_map(optimal_flow):
	for_map = {}
	for flow in optimal_flow:
		for task in flow.tasks:
			if "splitted" in task.type:
				l = re.findall(r'\d+',task.type)
				id = str(l[0]) + "_" + str(l[2])
				if id in for_map:
					for_map[id].count += 1
					for_map[id].id.append(task.id)
				else:
					for_map[id] = Task(1, task.id)
	return for_map

def add_new_tasks(optimal_flow, main_flow):
	for_map = create_map(optimal_flow)
	for key in for_map:
		l = re.findall(r'\d+',key)
		node_to_replace = find_node2(l[0], l[1], main_flow)
		nodes_to_add = []

		for i in range(for_map[key].count):
			nodes_to_add.append(For_Node("splitted_" + node_to_replace.start_line + "." + str(i), node_to_replace.start_line, node_to_replace.init_type, node_to_replace.init_var, node_to_replace.init_value, node_to_replace.init_cond, node_to_replace.init_cond_value, node_to_replace.init_increment, node_to_replace.init_increment_value, node_to_replace.time, node_to_replace.variance, math.floor(float(node_to_replace.mean_loops) / (i + 1))))
		
		for parent in node_to_replace.parent:
			parent.children.remove(node_to_replace)
			for n in nodes_to_add:
				parent.add(n)
				n.id = for_map[key].id.pop(0)
				n.color = 'white'
				n.from_type = node_to_replace.type

		for child in node_to_replace.children:
			child.parent.remove(node_to_replace)
			for n in nodes_to_add:
				n.add(child) 


def add_flow_id(optimal_flow, task_list):
	id_map = {}
	for flow in optimal_flow:
		for task in flow.tasks:
			if "splitted" not in task.type:
				if task.start_line not in id_map:
					id_map[task.start_line] = task.id
				else:
					id_map[task.start_line + str(1)] = task.id
	for task in task_list:
		if "splitted" not in task.type:
			if task.start_line in id_map:
				task.id = id_map[task.start_line]
				id_map.pop(task.start_line, None)
			else:
				task.id = id_map[task.start_line + str(1)]




















	







	
	













		




	



















	



