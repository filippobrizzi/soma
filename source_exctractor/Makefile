CXX := clang++
LLVMCOMPONENTS := #cppbackend
RTTIFLAG := #-fno-rtti
LLVMCONFIG := llvm-config

INCLUDEPATH := -I./include/
CXXFLAGS := -I$(shell $(LLVMCONFIG) --src-root)/tools/clang/include -I$(shell $(LLVMCONFIG) \
			--obj-root)/tools/clang/include $(shell $(LLVMCONFIG) --cxxflags) $(INCLUDEPATH)
LLVMLDFLAGS := $(shell $(LLVMCONFIG) --ldflags )
#--libs $(LLVMCOMPONENTS))

SOURCES = src/main.cpp src/driver/compiler.cpp src/driver/program.cpp src/utils/source_locations.cpp
#SOURCES = $(wildcard src/*.cpp) $(wildcard src/driver/*.cpp) $(wildcard src/utils/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
#@echo $(OBJECTS)
EXES = $(OBJECTS:.o=)
#@echo $(EXES)


CLANGLIBS = \
				-lclangTooling\
				-lclangFrontendTool\
				-lclangFrontend\
				-lclangDriver\
				-lclangSerialization\
				-lclangCodeGen\
				-lclangParse\
				-lclangSema\
				-lclangStaticAnalyzerFrontend\
				-lclangStaticAnalyzerCheckers\
				-lclangStaticAnalyzerCore\
				-lclangAnalysis\
				-lclangARCMigrate\
				-lclangRewriteFrontend\
				-lclangRewriteCore\
				-lclangEdit\
				-lclangAST\
				-lclangLex\
				-lclangBasic\
				$(shell $(LLVMCONFIG) --libs)\
               # -lcurses
all: $(OBJECTS) 
	$(CXX)  $(LLVMLDFLAGS) $(CLANGLIBS)$(CXXFLAGS) -o pragma_exctractor.exec

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@



#main.exe: $(OBJ_FILES)
#   g++ $(LD_FLAGS) -o $@ $^

#obj/%.o: src/%.cpp
#   g++ $(CC_FLAGS) -c -o $@ $<


clean:
	-rm -f $(EXES) $(OBJECTS) *~
