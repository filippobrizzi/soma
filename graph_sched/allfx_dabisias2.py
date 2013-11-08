# REQUIRED: libclang.so or libclang.dylib in the LD_LIBRARY_PATH
# Minimum LLVM 3.1 or 3.2

#!/usr/bin/env python
""" Usage: call with <filename> <typename>
"""

import sys
import clang.cindex

from clang.cindex import CursorKind
from clang.cindex import Index
from clang.cindex import TypeKind

class Node(object):
    def __init__(self, Ptype, line):
        self.type = Ptype
        self.line = line
        self.children = []
        self.parent = None
    def add(self,x):
        x.parent = self
        self.children.append(x)

class For_Node(Node): 
    def __init__(self,Ptype,line,init_var,init_value,init_cond,init_cond_value,init_increment):
        Node.__init__(self,Ptype,line)

class Fx_Node(Node):
	def __init__(self, Ptype, line):
		Node.__init__(self, Ptype, line)
		self.arguments = []
	def arg_add(self,type_):
		self.arguments.append(type_)

      

def scanTree(node, level, pre=""):
    print pre,node.type,'at level',level
    for n in node.children:
        scanTree(n,level+1,pre+"  ")

cachedfiles={}
def cachedfile(name):                                                  #cosa fa
    if not name in cachedfiles:
        x = [y.strip() for y in open(name,"r").read().split("\n")]
        cachedfiles[name] = x
        return x
    else:
        return cachedfiles[name]

compoundkinds = set([ CursorKind.COMPOUND_STMT,CursorKind.IF_STMT,CursorKind.SWITCH_STMT,CursorKind.WHILE_STMT,
                      CursorKind.DO_STMT,CursorKind.FOR_STMT,CursorKind.CXX_TRY_STMT,CursorKind.CXX_CATCH_STMT,
                      CursorKind.CXX_FOR_RANGE_STMT])

BASE_TYPE = set([TypeKind.VOID, TypeKind.BOOL, TypeKind.SHORT, TypeKind.INT,TypeKind.LONG ,TypeKind.FLOAT ,TypeKind.DOUBLE, TypeKind.POINTER])

def dump(d, pre=""):
    """Recursive over functions"""
    print pre,d.displayname,d.kind.name
    if d.kind == CursorKind.DECL_REF_EXPR:
        # extract the class name of this call
        pass
    pre = pre + " "
    for c in d.get_children():
        dump(c,pre)
        
def scanforpragma(d, pre=""):
    """Scans a single function or statement"""
    if d.kind.is_statement():
        line0 = d.location.line
        f = cachedfile(d.location.file.name)
        line = f[line0].strip() 
        while not line:
            line0=line0+1
            line=f[line0].strip()
        # pragma lookup
        xpragma = ""
            # TODO: possibly line in comment
        if line.startswith("#pragma"):
            xpragma = line
            return True;

    # scan children of function
    for cd in d.get_children():
        if scanforpragma(cd,pre+" "):
                return True
    return False

def tokens2text(x):
    x = "".join([y.spelling for y in x.get_tokens()])
    #if x[-1] == ";" or x[-1] == ",":
        #x = x[0:-1]
    return x

def extractforvariable(cd, node, pre=""):

    c = list(cd.get_children())
    

    if len(c) == 0:
        return None # empty for()

    if c[0].kind == CursorKind.DECL_STMT:       
        t = list(c[0].get_children())
        t1 = list(c[1].get_children())
        t2 = list(c[2].get_children())

        k = list(t[0].get_children())

        init_value_raw = tokens2text(k[0])

        init_value = init_value_raw[0:len(init_value_raw)-1]

        init_var_raw = tokens2text(t1[0])

        #counts the number of charachters which compose the comparison operator to subtract them 
        i=0
        for c in init_var_raw:
        	if c=='=' or c=='<' or c=='>' or c=='!':
        		i=i+1

        init_var = init_var_raw[0:len(init_var_raw)-i]

        init_increment_raw = tokens2text(t2[0])

        init_increment = init_increment_raw.replace(init_var,"")
        
        init_condition_value_raw = tokens2text(t1[1])
        init_condition_value = init_condition_value_raw[0:len(init_condition_value_raw)-1]

        init_condition_operator_raw = tokens2text(t1[0])
        init_condition_operator = init_condition_operator_raw.replace(init_var,"")
 

        print pre,"for"
        print pre," initialization variable ",init_var," initialization value ",init_value
        print pre," looping condition value ",init_condition_value," with operator ", init_condition_operator
        print pre," increment type ", init_increment 
        print pre,"endfor"

    elif c[0].kind == CursorKind.BINARY_OPERATOR:
        t = list(c[0].get_children())
        t1 = list(c[1].get_children())
        t2 = list(c[2].get_children())

        k = list(t[0].get_children())

        init_var_raw = tokens2text(t[0])
       
      	i = 0
      	c = init_var_raw[0]

      	while c != "=":
      			i=i+1
      			c=init_var_raw[i]
      	
      	init_var = init_var_raw[0:i]
      	
      	init_value_raw = tokens2text(t[1])
      	init_value = init_value_raw[0:len(init_value_raw)-1]
      	
      	init_condition_value_raw = tokens2text(t1[1])
      	init_condition_value = init_condition_value_raw[0:len(init_condition_value_raw)-1]
      	
      	init_condition_operator_raw = tokens2text(t1[0])
      	init_condition_operator = init_condition_operator_raw.replace(init_var,"")

      	init_increment_raw = tokens2text(t2[0])
      	init_increment = init_increment_raw.replace(init_var,"")

    	print pre,"for"
        print pre," initialization variable ",init_var," initialization value ",init_value
        print pre," looping condition value ",init_condition_value," with operator ", init_condition_operator
        print pre," increment type ", init_increment 
        print pre,"endfor"
        Node.__init__(self,Ptype,line,init_var,init_value,init_cond,init_cond_value,init_increment)

    for_node=For_Node("For Node",cd.location.line,init_var,init_value,init_condition_value,init_condition_operator,init_increment)
    node.add(for_node)

def scandef(d, for_found, nested, node, prev_line, pre=""):
    for cd in d.get_children():   

        line0 = cd.location.line
        f = cachedfile(cd.location.file.name)
        xpragma = ""

        line = f[line0].strip()
        while not line :
            line0=line0+1
            line=f[line0].strip()

        child = None

        if cd.kind.is_statement():
            print pre,cd.kind.name,"@",cd.location.line

        if line.startswith("#pragma omp parallel for") and line0 != prev_line:
            xpragma = line
            print pre,"***found pragma omp parallel for @",line0+1   
            child = Node(xpragma,line0+1)
            node.add(child)
            for_found = True
            nested = False

        elif line.startswith("#pragma omp for") and line0 != prev_line:
            xpragma = line
            print pre,"*** found pragma omp for @",line0+1   
            child = Node(xpragma,line0+1)
            node.add(child)
            for_found = True
            nested = False

        elif line.startswith("#pragma omp parallel") and line0 != prev_line:
            xpragma = line
            print pre,"*** found pragma omp parallel @",line0+1   
            child = Node(xpragma,line0+1)
            node.add(child)
            for_found = False
            nested = True

        elif line.startswith("#pragma omp single") and line0 != prev_line:
            xpragma = line
            print pre,"*** found pragma omp single @",line0+1   
            child = Node(xpragma,line0+1)
            node.add(child)
            for_found = False
            nested = False


        elif line.startswith("#pragma omp task") and line0 != prev_line:
            xpragma = line
            print pre,"*** found pragma omp task @",line0+1   
            child = Node(xpragma,line0+1)
            node.add(child)
            for_found = False
            nested = True
            nested = False

        elif line.startswith("#pragma omp master") and line0 != prev_line:
            xpragma = line
            print pre,"*** found pragma omp master @",line0+1   
            child = Node(xpragma,line0+1)
            node.add(child)
            for_found = False
            nested = False

        elif line.startswith("#pragma omp critical") and line0 != prev_line:
            xpragma = line
            print pre,"*** found pragma omp critical @",line0+1   
            child = Node(xpragma,line0+1)
            node.add(child)
            for_found = False
            nested = False

        elif line.startswith("#pragma omp barrier") and line0 != prev_line:
            xpragma = line
            print pre,"*** found pragma omp barrier @",line0+1   
            child = Node(xpragma,line0+1)
            node.add(child)
            for_found = False
            nested = False

        elif line.startswith("#pragma omp taskwait") and line0 != prev_line:
            xpragma = line
            print pre,"*** found pragma omp taskwait @",line0+1   
            child = Node(xpragma,line0+1)
            node.add(child)
            for_found = False
            nested = False

        elif line.startswith("#pragma omp atomic") and line0 != prev_line:
            xpragma = line
            print pre,"*** found pragma omp atomic @",line0+1   
            child = Node(xpragma,line0+1)
            node.add(child)
            for_found = False
            nested = False

        if cd.kind == CursorKind.CALL_EXPR:
            dump(cd,pre+" -")

        if cd.kind == CursorKind.FOR_STMT and for_found:
            varname = extractforvariable(cd, node, pre+" ")

        if cd.kind in compoundkinds:      
            if child is not None:
                c = child
            else:
                c = node
            scandef(cd, for_found, nested, c, line0, pre+" ")
            if nested:
                node = c.parent
        




def fxdefs(tu, mainfile, node):
    skippedfiles = set()
    """Scans all functions of the file"""
    for cursor in tu.cursor.get_children():										
        # Ignore AST elements not from the main source file (e.g.
        # from included files).
        if not cursor.location.file or cursor.location.file.name != mainfile:
            if cursor.location.file:
                name = cursor.location.file.name
                if name not in skippedfiles:
                    skippedfiles.add(name)
                    print "skip file",name
            continue


        # Ignore AST elements not a function declaration.
        if cursor.kind == CursorKind.FUNCTION_DECL or cursor.kind == CursorKind.CXX_METHOD:
        # Obtain the return Type for this function.
            result_type = cursor.type.get_result()

            d = cursor.get_definition()
            haspragma = scanforpragma(d)

            if not haspragma:
                print "skip function",cursor.spelling
            else:

                if cursor.kind == CursorKind.CXX_METHOD:
            	    print "USR: ",cursor.get_usr()
                
                k = list(  cursor.type.argument_types())
                arg = list(cursor.get_children())

                fxnode = Fx_Node("fx "+cursor.spelling,cursor.extent.start.line)
                fxnode.name = cursor.spelling


                for l in k :
                	kind = l.get_canonical().kind
                	if kind == TypeKind.VOID:
               			name = tokens2text(arg[0]).replace("void","").replace(",","")
               			print "  void ",name
               			t=("void",name)
               			fxnode.arg_add(t)
               		elif kind == TypeKind.BOOL:
               			name = tokens2text(arg[0]).replace("bool","").replace(",","")
               			print "  bool ",name 
               			t=("bool",name)
               			fxnode.arg_add(t)
               		elif kind == TypeKind.SHORT:
               			name = tokens2text(arg[0]).replace("short","").replace(",","")
               			print "  short ",name
               			t=("short",name)
               			fxnode.arg_add(t)
               		elif kind == TypeKind.INT:
               			name = tokens2text(arg[0]).replace("int","").replace(",","")
               			print "  int ",name
               			t=("int",name)
               			fxnode.arg_add(t)
               		elif kind == TypeKind.LONG:
               			name = tokens2text(arg[0]).replace("long","").replace(",","")
               			print "  long",name
               			t=("long",name)
               			fxnode.arg_add(t)
               		elif kind == TypeKind.FLOAT:
               			name = tokens2text(arg[0]).replace("float","").replace(",","")
               			t=("float",name)
               			print "  float ",name
               			fxnode.arg_add(t)
               		elif kind == TypeKind.DOUBLE:
               			name = tokens2text(arg[0]).replace("double","").replace(",","")
               			t=("double",name)
               			print "  name ",name
               			fxnode.arg_add(t)
               		elif kind == TypeKind.POINTER:
               			name = tokens2text(arg[1]).replace(")","").partition("*")
               			t=("pointer",name[2])
               			print " ",name[0]," * ", name[2]
               			fxnode.arg_add(t)

               	print 'Function with pragma: ', cursor.spelling
                print '  Extent: %s-%s' % (cursor.extent.start.line,cursor.extent.end.line)
                print '  Return type: ',result_type.kind.name#.spelling
                print '  Arguments:',fxnode.arguments
                		
                node.add(fxnode)
                scandef(d, False, False, fxnode,0,"   ")

if __name__ == "__main__":
            #get_definition
    print
    index = clang.cindex.Index.create()
    tu = index.parse(sys.argv[1])
    print 'Translation unit:', tu.spelling
    root = Node("root",0)
    fxdefs(tu,sys.argv[1],root)
    print
    print
    scanTree(root,0)

