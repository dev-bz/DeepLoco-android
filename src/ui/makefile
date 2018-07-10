ifndef OUTPUT
OUTPUT:=/data/user/0/org.c.ide/files/tmpdir/a.out
endif
RMObjects:= $(wildcard *.o)
ifdef MORE
	MoreBaseName:=$(sort $(basename $(MORE)))
endif
oreObjects:=$(wildcard $(MoreBaseName:%=%.o) $(OUTPUT))
ifdef MoreObjects
RMObjects+=$(MoreObjects)
endif
ifdef RMObjects
RMObjects:=@rm $(RMObjects)
endif

Sources:=$(wildcard *.c *.cc *.cxx *.cpp net/*.cpp)  $(MORE)
BaseName:=$(basename $(Sources))
CPPFLAGS+= -I. -I/sdcard/_ws/jsoncpp/thrid/bullet3
ifdef BaseName
all.suffix:= $(suffix $(Sources))
ifeq ($(words $(BaseName)), $(words $(all.suffix)))
cxx.suffix:= $(filter .cc .cpp .cxx, $(all.suffix))
XCC:= $(if $(cxx.suffix), $(CXX),$(CC))
Objects:=$(BaseName:%=%.o)
all: $(OUTPUT)
#	@echo $(Objects) $(CPPFLAGS)
$(OUTPUT):$(Objects)
	$(XCC) $^ $(LDFLAGS) -o $@ -landroid -lbt -lEGL -lGLESv3 -L/sdcard/_ws/jsoncpp/thrid/lib64
	@echo $(XCC) $^ $(LDFLAGS) -o $@ -lEGL -lGLESv3
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
	$(RMObjects)
%.o: %.c %.cc %.cxx %.cpp
	@echo $<