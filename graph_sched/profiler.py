import os
import pargraph as par
import xml.etree.cElementTree as ET
import numpy 
import re




def profileCreator(cycle, executable):
	pragma_times = {}
	function_times = {}
	j = 0

	for i in range(cycle):
		print "profiling iteration: " + str((j + 1))
		os.system("./" + executable + ">/dev/null")
		os.system("mv log_file.xml " + "./logfile%s.xml" % j)
		root = ET.ElementTree(file = "./logfile%s.xml" % j).getroot()

		for pragma in root.iter('Pragma'):
			key = pragma.attrib['fid'] + pragma.attrib['pid']
			if (key not in pragma_times):
				pragma_times[key] = par.Time_Node(int(pragma.attrib['fid']), int(pragma.attrib['pid']))
			if ('callerid' in pragma.attrib):
				if pragma.attrib['callerid'] not in pragma_times[key].caller_list:
					pragma_times[key].caller_list.append(pragma.attrib['callerid'])
			if ('loops' in pragma.attrib):
				pragma_times[key].loops.append(int(pragma.attrib['loops']))
			if ('time' in pragma.attrib):
				pragma_times[key].time = pragma.attrib['time']
			if ('childrenTime' in pragma.attrib):
				pragma_times[key].children_time.append(float(pragma.attrib['childrenTime']))
			pragma_times[key].times.append(float(pragma.attrib['elapsedTime']))

				
		for func in root.iter('Function'):
			key = func.attrib['fid']
			if (key in function_times):
				function_times[key].times.append(float(func.attrib['elapsedTime']))
			else:
				function_times[key] = par.Time_Node(int(func.attrib['fid']), 0)
				function_times[key].times.append(float(func.attrib['elapsedTime']))
			if ('callerid' in func.attrib):
				if int(func.attrib['callerid']) not in function_times[key].caller_list:
					function_times[key].caller_list.append(int(func.attrib['callerid']))
			if ('time' in func.attrib):
				function_times[key].time = func.attrib['time']
			if ('childrenTime' in func.attrib):
				function_times[key].children_time.append(float(func.attrib['childrenTime']))

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
		if (len(function_times[key].children_time) != 0):
			children_time = ET.SubElement(s, 'ChildrenTime')
			children_time.text = str(numpy.mean(function_times[key].children_time))
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
		if (len(pragma_times[key].children_time) != 0):
			children_time = ET.SubElement(s, 'ChildrenTime')
			children_time.text = str(numpy.mean(pragma_times[key].children_time))
		time.text = str(numpy.mean(pragma_times[key].times))
		f_line.text = str(pragma_times[key].func_line)
		p_line.text = str(pragma_times[key].pragma_line)
		var.text = str(numpy.std(pragma_times[key].times))

	tree = ET.ElementTree(root)
	par.indent(tree.getroot())
	tree.write(executable + "_profile.xml")

	return executable + "_profile.xml"

def add_profile_xml(profile_xml, xml_tree):
	functions = getProfilesMap(profile_xml)
	tree = ET.ElementTree(file = xml_tree) 
	root = tree.getroot()
	type_ = ET.SubElement(root, 'GraphType')
	type_.text = 'Code'

	for func in root.findall('Function'):
		key = func.find('Line').text
		func_time =  ET.SubElement(func, 'Time')
		func_time.text = str(functions[key].time)
		func_variance = ET.SubElement(func, 'Variance')
		func_variance.text = str(functions[key].variance)
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

	par.indent(tree.getroot())			
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

def getProfilesMap(profile_xml):
	profile_graph_root = ET.ElementTree(file = profile_xml).getroot()

	functions = {}
	l = []

	for func in profile_graph_root.findall('Function'):
		f = par.Function(func.find('Time').text, func.find('Variance').text, func.find('ChildrenTime').text)
		f.callerid = []
		if (func.find('CallerId') != None):
			l = re.findall(r'\d+',func.find('CallerId').text.replace("[","").replace("]",""))
			for id_ in l:
				f.callerid.append(id_)
		functions[func.find('FunctionLine').text] = f

	for pragma in profile_graph_root.findall('Pragma'):
		if pragma.find('CallerId') != None:
			callerid = pragma.find('CallerId').text.replace("[\'","").replace("\']","")
		else:
			callerid = None
		if (pragma.find('Loops') != None):
			loops = pragma.find('Loops').text
		else :
			loops = 0
		functions[pragma.find('FunctionLine').text].add_pragma( (pragma.find('PragmaLine').text, pragma.find('Time').text, pragma.find('Variance').text, loops, callerid, pragma.find('ChildrenTime').text ))
	
	return functions