CC = icc
CXX = icpc
CFLAGS = -qopenmp -g
NATIVEFLAG = -mmic
NO_OFFLOAD = -qno-offload
DEBUG = -qopt-report-phase=offload

# set env for OFFLOAD_REPORT
#export OFFLOAD_REPORT = 2

# the build target executables:
TARGET = fpgrowth

$(TARGET):	$(TARGET).cpp
	$(CXX)	$(CFLAGS)	$(DEBUG)	$(TARGET).c fptree.h tract.h -o	$(TARGET)
# clean
clean:
	$(RM) $(TARGET)
	# $(RM) $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) $(TARGET6) $(TARGET7)
