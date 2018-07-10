ifndef OUTPUT
OUTPUT:=/data/user/0/org.c.ide/files/tmpdir/a.out
endif
DirName:=util sim anim scenarios learning caffe_mods  ui
FindName:=$(DirName:%=%/*.cpp) $(DirName:%=%/*.c)
RMObjects:= $(wildcard *.o $(DirName:%=%/*.o))
ifdef MORE
	MoreBaseName:=$(sort $(basename $(MORE)))
endif
MoreObjects:=$(wildcard $(MoreBaseName:%=%.o) $(OUTPUT))
ifdef MoreObjects
RMObjects+=$(MoreObjects)
endif
ifdef RMObjects
RMObjects:=@rm $(RMObjects)
endif

Sources:=$(wildcard *.c *.cc *.cxx *.cpp $(FindName) src/lib_json/*.cpp)  $(MORE)
BaseName:=$(basename $(Sources))
DEPEND_OPTIONS = -MMD -MP -MF "$*.d" \
         -MT "$*.o" -MT "$*.d"
DEPEND_MOVEFILE = then $(MV) -f "$(ObjDir)/$*.d.tmp" "$(ObjDir)/$*.d"; \
                  else $(RM) "$(ObjDir)/$*.d.tmp"; exit 1; fi

CPPFLAGS+= -DDEMO -DCPU_ONLY -I. -I../thrid/bullet3 -I../thrid/include
# $(DEPEND_OPTIONS)

ifdef USE_DEP
DependSourceFiles := $(basename $(filter %.cpp %.c %.cc %.m %.mm, $(Sources)))
DependFiles := $(DependSourceFiles:%=%.d)
-include $(DependFiles) ""
endif

ifdef BaseName
all.suffix:= $(suffix $(Sources))
ifeq ($(words $(BaseName)), $(words $(all.suffix)))
cxx.suffix:= $(filter .cc .cpp .cxx, $(all.suffix))
XCC:= $(if $(cxx.suffix), $(CXX),$(CC))
Objects:=$(BaseName:%=%.o)

all: $(OUTPUT)
#	@echo  hello $(DEPEND_OPTIONS)
#	@echo $(Objects)
$(OUTPUT):a.out
	strip -s $<
	cp $< $@
	chmod 700 $@
a.out:$(Objects)
	$(XCC) $^ $(LDFLAGS) -o $@ -O3 -L../thrid/lib64 -Wl,--whole-archive -lcaffe -Wl,--no-whole-archive -lcaffeproto -lglog -lgflags -lopenblas -lprotobuf -lbt -lEGL -lGLESv3
else
all:
	clear
	@echo 某些文件名含有空格,终止编译
endif
else
all:
	clear
	@echo 项目中没有找到源代码文件
endif
clean:
	@echo Clean $(RMObjects)
	$(RMObjects)
%.o: %.c %.cc %.cxx %.cpp
	@echo $<
	
#a.out:makefile
#	touch a.out