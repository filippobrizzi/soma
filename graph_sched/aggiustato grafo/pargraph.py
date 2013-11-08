import pydot as p
import numpy 
import xml.etree.cElementTree as ET
from random import randrange
import os
import re

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
        self.parent = None
        self.options = []
        self.time = time
        self.variance = variance
        self.end_line = 0
        self.callerid = None
        self.deadline = None
        self.d = None
    def add(self, x):	
    	x.parent = self
    	self.children.append(x)
    def myself(self):
		if self.type != 'BARRIER':
			print "pragma node: ", self.type, "\n    start_line: ", self.start_line, "\n    endl_line", self.end_line
			if self.type.find("_end") == -1:
				if self.time != 0:
					print "    time: ", self.time
					print "    variance: ", self.variance
				else:
					print "    not executed"
				if(len(self.options) != 0):
					print "    Options:"
					for i in self.options:
						print "        ",i[0]," ",i[1]
			print "     chetto deadline :",self.d
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
		if(len(self.options) != 0):
			print "    Options:"
			for i in self.options:
				print "        ", i[0], " ", i[1]
		if self.time != 0:
			print "    time: ", self.time
			print "    variance: ", self.variance,"\n"
		else:
			print "    not executed\n"

class Fx_Node(Node):
	def __init__(self, Ptype, line, returnType, time, variance, file_name):
		Node.__init__(self, Ptype, line, time, variance)
		self.arguments = []
		self.returnType = returnType
		self.time = time
		self.file_name = file_name
	def add_arg(self, type_):
		self.arguments.append(type_)
	def myself(self):
		print "function node: ", self.type, "() {\n    line: ", self.start_line, "\n    return type: ", self.returnType
		if(len(self.arguments) != 0):
			print "    Parameters: "
			i = 0
			for par in self.arguments:
				print "   ", i, ") ", par[0], " ", par[1]
				i = i + 1
		else:
			print "    No input parameters"
		if self.time != 0:
			print "    time: ", self.time
			print "    variance: ", self.variance,"\n}\n"
		else:
			print "    not executed\n}\n"

class Function():
	def __init__(self, time, variance):
		self.time = time
		self.variance = variance
		self.pragmas = {}
	def add_pragma(self, pragma):
		self.pragmas[pragma[0]] = (pragma[1], pragma[2], pragma[3], pragma[4])

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

def scanGraph(node, node_list, pre = ""):
	#print pre, node.type
	if (node.type+node.start_line) not in node_list:
		node_list.append(node.type+node.start_line)
		node.myself()
		print "     has children:"
		for c in node.children:
			print "          ",c.type,"@",c.start_line
		if node.parent != None:
			print "     has parent:"
			print "          ",node.parent.type,"@",node.parent.start_line
		for n in node.children:
			scanGraph(n, node_list, pre + "  ")

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

def create_diamond(tree, graph, node, treeNode, func_pragmas, root):
	special_node = p.Node(node.get_name().replace("\"", "") + "_end", label = 'BARRIER', root = root)
	Objspecial_node = Node( 'BARRIER_end' , node.get_name() , 0, 0 )
	color = colors[randrange(len(colors) - 1)]
	for d in tree.find('Pragmas').findall('Pragma'):

		end_line = d.find('Position/EndLine').text
		key = d.find('Position/StartLine').text

		if key not in func_pragmas:
			time = 0
			variance = 0
			loops = 0
			callerid = None
		else:
			time = func_pragmas[key][0]
			variance = func_pragmas[key][1]
			loops = func_pragmas[key][2]
			callerid = func_pragmas[key][3]

		tmp_name = d.find('Name').text.replace("::", " ")
		visual_name = tmp_name + "@%s" % key

		if (tmp_name == 'omp parallel for' or tmp_name == 'omp for'):
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
				Objchild.options.append( (op.find('Name').text,[i.text for i in op.findall('Parameter')]) )
				if op.find('Name').text == 'deadline':
					deadline = op.find('Parameter').text

		Objchild.end_line = end_line
		Objchild.callerid = callerid
		Objchild.deadline = deadline

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

def scan(xml_tree, pragma_graph, node, treeNode, func_pragmas, root):
	for d in xml_tree.find('Pragmas').findall('Pragma'):
		end_line = d.find('Position/EndLine').text
		key = d.find('Position/StartLine').text

		if key not in func_pragmas:
			time = 0
			variance = 0
			loops = 0
			callerid = None
		else:
			time = func_pragmas[key][0]
			variance = func_pragmas[key][1]
			loops = func_pragmas[key][2]
			callerid = func_pragmas[key][3]

		tmp_name = d.find('Name').text.replace("::", " ")
		visual_name = tmp_name+"@%s"%key

		if (tmp_name == 'omp parallel for' or tmp_name == 'omp for'):
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
				Objchild.options.append( (op.find('Name').text,[i.text for i in op.findall('Parameter')]) )
				if op.find('Name').text == 'deadline':
					deadline = op.find('Parameter').text
		Objchild.end_line = end_line
		Objchild.callerid = callerid
		Objchild.deadline = deadline
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


def getNesGraph(xml, profile_xml):
	tree = ET.ElementTree(file = xml) 
	profile_graph_root = ET.ElementTree(file = profile_xml).getroot()
	functions = getProfilesMap(profile_xml)

	root = tree.getroot()
	graphs = []
	count = 0

	for n in root.iter('Function'):
		key = n.find('Line').text
		time = functions[key].time
		variance = functions[key].variance
		graphs.append(p.Dot(graph_type = 'digraph'))
		name = n.find('Name').text
		if (time == 0):
			root = p.Node(name, label = name + "()" + "\nnot executed")
		else:
			root = p.Node(name, label = name + "()" + "\n execution time: " + str(time) + "\nvariance: " + str(variance))
		graphs[count].add_node(root)
		find_nesting(n, graphs[count], root, functions[key].pragmas)
		count += 1

	return graphs


def getProfilesMap(profile_xml):
	profile_graph_root = ET.ElementTree(file = profile_xml).getroot()

	functions = {}

	for func in profile_graph_root.findall('Function'):
		f = Function(func.find('Time').text, func.find('Variance').text)
		f.callerid = []
		if (func.find('CallerId') != None):
			f.callerid.append(func.find('CallerId').text.replace("[","").replace("]",""))
		functions[func.find('FunctionLine').text] = f

	for pragma in profile_graph_root.findall('Pragma'):
		if pragma.find('CallerId') != None:
			callerid = pragma.find('CallerId').text
		else:
			callerid = None
		if (pragma.find('Loops') != None):
			functions[pragma.find('FunctionLine').text].add_pragma( (pragma.find('PragmaLine').text, pragma.find('Time').text, pragma.find('Variance').text, pragma.find('Loops').text, callerid ))
		else:
			functions[pragma.find('FunctionLine').text].add_pragma( (pragma.find('PragmaLine').text, pragma.find('Time').text, pragma.find('Variance').text, 0, callerid ))

	return functions

def getParalGraph(pragma_xml, profile_xml):
	pragma_graph_root = ET.ElementTree(file = pragma_xml).getroot()
	profile_graph_root = ET.ElementTree(file = profile_xml).getroot()

	functions = getProfilesMap(profile_xml)
	objGraph = []
	graphs = []
	count = 0
	arch = Architecture(profile_graph_root.find('Hardware/NumberofCores').text, profile_graph_root.find('Hardware/MemorySize').text)
	
	file_name = pragma_graph_root.find('Name').text
	
	for n in pragma_graph_root.findall('Function'):
		graphs.append(p.Dot(graph_type = 'digraph'))
		name = n.find('Name').text
		time = functions[n.find('Line').text].time
		callerid = functions[n.find('Line').text].callerid
		root = n.find('Line').text
		if (time == 0):
			pragma_graph_root = p.Node(n.find('Line').text, label = name + "()\nnot executed", root = root)
		else:
			pragma_graph_root = p.Node(n.find('Line').text, label = name + "()\nexecution time " + time, root = root)
		pragma_graph_root.callerid = callerid
		graphs[count].add_node(pragma_graph_root)
		Objroot = Fx_Node(name, n.find('Line').text,n.find('ReturnType').text, functions[n.find('Line').text].time, functions[n.find('Line').text].variance, file_name)
		for par in n.findall('Parameters/Parameter'):
			Objroot.add_arg( ( par.find('Type').text,par.find('Name').text ) )
		objGraph.append(Objroot)
		scan(n, graphs[count], pragma_graph_root, objGraph[count], functions[n.find('Line').text].pragmas, root)
		count = count + 1
	return (graphs, objGraph)


def profileCreator(cycle, executable):
	pragma_times = {}
	function_times = {}
	j = 0

	for i in range(cycle):
		print "profiling iteration: " + str((j + 1))
		os.system("./" + executable + ">/dev/null")
		os.system("mv log_file.xml " + "logfile%s.xml" % j)
		root = ET.ElementTree(file = "logfile%s.xml" % j).getroot()

		for pragma in root.iter('Pragma'):
			key = pragma.attrib['fid'] + pragma.attrib['pid']
			if (key not in pragma_times):
				pragma_times[key] = Time_Node(int(pragma.attrib['fid']), int(pragma.attrib['pid']))
			if ('callerid' in pragma.attrib):
				if pragma.attrib['callerid'] not in pragma_times[key].caller_list:
					pragma_times[key].caller_list.append(pragma.attrib['callerid'])
			if ('loops' in pragma.attrib):
				pragma_times[key].loops.append(int(pragma.attrib['loops']))
			if ('time' in pragma.attrib):
				pragma_times[key].time= pragma.attrib['time']
			pragma_times[key].times.append(int(pragma.attrib['elapsedTime']))

				
		for func in root.iter('Function'):
			key = func.attrib['fid']
			if (key in function_times):
				function_times[key].times.append(int(func.attrib['elapsedTime']))
			else:
				function_times[key] = Time_Node(int(func.attrib['fid']), 0)
				function_times[key].times.append(int(func.attrib['elapsedTime']))
			if ('callerid' in func.attrib):
				function_times[key].caller_list.append(int(func.attrib['callerid']))
			if ('time' in func.attrib):
				function_times[key].time = func.attrib['time']

		j += 1

	num_cores = ET.ElementTree(file = "logfile0.xml").getroot().find('Hardware').attrib['NumberofCores']
	tot_memory = ET.ElementTree(file = "logfile0.xml").getroot().find('Hardware').attrib['MemorySize']

	root = ET.Element('Log_file')
	h = ET.SubElement(root, 'Hardware')
	h1 = ET.SubElement(h, 'NumberofCores')
	h2 = ET.SubElement(h, 'MemorySize')
	h1.text = num_cores
	h2.text = tot_memory

	for key in function_times:
		s = ET.SubElement(root, 'Function')
		line = ET.SubElement(s, 'FunctionLine')
		time = ET.SubElement(s, 'Time')
		var = ET.SubElement(s, 'Variance')
		if (len(function_times[key].caller_list) != 0 ):
			callerid = ET.SubElement(s, 'CallerId')
			callerid.text = str(function_times[key].caller_list)
		time.text = str(numpy.mean(function_times[key].times))
		line.text = str(function_times[key].func_line)
		var.text = str(numpy.std(function_times[key].times))

	for key in pragma_times:
		s = ET.SubElement(root, 'Pragma')
		f_line = ET.SubElement(s, 'FunctionLine')
		p_line = ET.SubElement(s, 'PragmaLine')
		time = ET.SubElement(s, 'Time')
		var = ET.SubElement(s, 'Variance')
		if (len(pragma_times[key].loops) != 0):
			loops = ET.SubElement(s, 'Loops')
			loops.text = str(numpy.mean(pragma_times[key].loops))
		if (len(pragma_times[key].caller_list) != 0 ):
			callerid = ET.SubElement(s, 'CallerId')
			callerid.text = str(pragma_times[key].caller_list)
		time.text = str(numpy.mean(pragma_times[key].times))
		f_line.text = str(pragma_times[key].func_line)
		p_line.text = str(pragma_times[key].pragma_line)
		var.text = str(numpy.std(pragma_times[key].times))

	tree = ET.ElementTree(root)
	indent(tree.getroot())
	tree.write(executable + "_profile.xml")

	return executable + "_profile.xml"


def dump_flow_xml (par_graphs):
	root = ET.Element('File')
	name = ET.SubElement(root, 'Name')
	name.text = par_graphs[0].file_name
	graph_type = ET.SubElement(root, 'GraphType')
	graph_type.text = "flow"
	for func in par_graphs:
		function = ET.SubElement(root, 'Function')
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
		time.text = func.time
		variance = ET.SubElement(function, 'Variance')
		variance.text = func.variance
		func.parent = None
		if ( func.callerid != None ):
			l = re.findall(r'\d+',pragma.callerid)
			callerids = ET.SubElement(function, 'Callerids')
			for id_ in func.callerid:
				callerid = ET.SubElement(callerids, 'Callerid')
				callerid.text = id_
		if len(func.children) != 0:
			pragmas = ET.SubElement(function, 'Pragmas')
			func.element = pragmas
			dump_flow_pragmas (func, pragmas)

	tree = ET.ElementTree(root)
	indent(tree.getroot())
	tree.write('flow.xml')


def dump_flow_pragmas(pragma_node, pragmas_element):
	length = len(pragma_node.children)
	count = 0
	if  "_end" in pragma_node.type :
		pragmas_element = pragma_node.parent.parent.element
	for pragma in pragma_node.children:
		pragma.parent = pragma_node
		pragma.element = pragmas_element
		count = count + 1
		pragma_ = ET.SubElement(pragmas_element, 'Pragma')
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
					op_parameter.text = par
		position = ET.SubElement(pragma_, 'Position')
		start = ET.SubElement(position, 'StartLine')
		start.text = pragma.start_line
		if(name.text != "BARRIER"):
			end = ET.SubElement(position, 'EndLine')
			end.text = pragma.end_line
		if (pragma.callerid != None ):
			l = re.findall(r'\d+',pragma.callerid)
			callerids = ET.SubElement(pragma_, 'Callerids')
			for id_ in l:
				callerid = ET.SubElement(callerids, 'Callerid')
				callerid.text = id_
		if len(pragma.children) > 1 :
			if not "_end" in pragma.children[0].type :
				children = ET.SubElement(pragma_, 'Children')
				pragmas_ = ET.SubElement(children, 'Pragmas')
				dump_flow_pragmas (pragma, pragmas_)
			else:
				if count == length:
					dump_flow_pragmas (pragma, pragmas_element)
		elif len(pragma.children) == 1:
			if not "_end" in pragma.children[0].type :
				dump_flow_pragmas (pragma, pragma.parent.element)
			else:
				if count == length:
					dump_flow_pragmas (pragma, pragmas_element)

def add_profile_xml(profile_xml, xml_tree):
	functions = getProfilesMap(profile_xml)
	tree = ET.ElementTree(file = xml_tree) 
	root = tree.getroot()
	type_ = ET.SubElement(root, 'GraphType')
	type_.text = 'Code'

	for func in root.findall('Function'):
		key = func.find('Line').text
		func_time =  ET.SubElement(func, 'Time')
		func_time.text = functions[key].time
		func_variance = ET.SubElement(func, 'Variance')
		func_variance.text = functions[key].variance
		if len(functions[key].callerid) > 0:
			func_caller_ids = ET.SubElement(func, 'Callerids')
			tmp_list = set(functions[key].callerid)
			for id in tmp_list:
				func_caller_id = ET.SubElement(func_caller_ids,'Callerid')
				func_caller_id.text = id
		for pragma in func.iter('Pragma'):
			pragma_key = pragma.find('Position/StartLine').text
			if pragma_key in functions[key].pragmas:
				pragma_time = ET.SubElement(pragma, 'Time')
				pragma_time.text = functions[key].pragmas[pragma_key][0]
				pragma_variance = ET.SubElement(pragma, 'Variance')
				pragma_variance.text = functions[key].pragmas[pragma_key][1]
				if (functions[key].pragmas[pragma_key][2] != 0):
					pragma_loops = ET.SubElement(pragma, 'Loops')
					pragma_loops.text = functions[key].pragmas[pragma_key][2]	
				if 	(functions[key].pragmas[pragma_key][3] != None):
					pragma_callerid = ET.SubElement(pragma, 'Callerid')
					pragma_callerid.text = functions[key].pragmas[pragma_key][3].replace('[','').replace(']','').replace('\'','')

	indent(tree.getroot())			
	tree.write('code.xml')

def get_table(profile_xml):
	tree = ET.ElementTree(file = profile_xml) 
	root = tree.getroot()
	table = {}

	for func in root.iter('Function'):
		table[func.find('FunctionLine').text] = []
		if func.find('CallerId') != None:
			l = re.findall(r'\d+',func.find('CallerId').text)
			for j in l:
				table[func.find('FunctionLine').text].append(j)

	return table

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

	functions_callers = get_table(profile_xml)
	

	for func in visual_flow_graphs:
		root = func.get_nodes()[0].obj_dict['attributes']['root']
		if len(functions_callers[root]) > 0 :
			for caller in functions_callers[root]:
				func_graph.add_edge(p.Edge(caller, root))

	return func_graph


def get_last(node):
	if not node.children:
		return node	
	else:
		 return get_last(node.children[0])

def get_min(node):
	minimum = float("inf")
	for child in node.children:
		if child.time != None:
			min_tmp = child.d - float(child.time)
		else:
			min_tmp = child.d
		if min_tmp < minimum:
			minimum = min_tmp
	return minimum

def chetto_deadlines(node):
	if not node.parent :
		for p in node.parent:
			p.d = get_min(p)
			print "set ",p.type,"@",p.start_line
		for p in node.parent:
			chetto_deadlines(p)

def chetto (flow_graph, deadline):
	node = get_last(flow_graph)
	node.d = deadline

	chetto_deadlines(node)





	



















	



