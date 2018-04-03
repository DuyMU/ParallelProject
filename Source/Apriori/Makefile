CC = icc
CXX = icpc
CFLAGS = -qopenmp -g
NATIVEFLAG = -mmic
NO_OFFLOAD = -qno-offload
DEBUG = -qopt-report-phase=offload

# set env for OFFLOAD_REPORT
#export OFFLOAD_REPORT = 2

# the build target executables:
TARGET = apriori_omp_xeonphi

$(TARGET):	$(TARGET).cpp
	$(CXX)	$(CFLAGS)	$(DEBUG)	$(TARGET).cpp -o	$(TARGET)

clean:
	$(RM) $(TARGET)