
# El primer argumento se toma como número
NUM = $(arg)


#CC      = gcc
#CFLAGS  =  -W"switch" -W"no-deprecated-declarations" -W"empty-body" -W"conversion" -W"return-type" -W"parentheses" -W"no-format" -W"uninitialized" -W"unreachable-code" -W"unused-function" -W"unused-value" -W"unused-variable" -Wsign-compare -fpermissive -frtti -fno-omit-frame-pointer -std=c11


#CPLEX=/opt/ibm/ILOG/CPLEX_Studio128
#CXX          = g++
#LDFLAGS      = -O3 -DIL_STD -fthreadsafe-statics -flto -Wswitch -Wno-deprecated-declarations -Wempty-body -Wconversion -Wreturn-type -Wparentheses -Wno-pointer-sign -Wno-format -Wuninitialized -Wunreachable-code -Wunused-function -Wunused-value -Wunused-variable -Wsign-compare -std=c++11 -funroll-all-loops -Wall -fno-strict-aliasing -I $CPLEX/concert/include -I $CPLEX/cplex/include  -g2 -gdwarf-2 
#LIBS 	    = -fPIC -lilocplex -lconcert -lcplex -lm -lpthread -ldl -fopenmp -L $CPLEX/concert/lib/x86-64_linux/static_pic -L $CPLEX/cplex/lib/x86-64_linux/static_pic 


CC      = gcc
CFLAGS  =  -W"switch" -W"no-deprecated-declarations" -W"empty-body" -W"conversion" -W"return-type" -W"parentheses" -W"no-format" -W"uninitialized" -W"unreachable-code" -W"unused-function" -W"unused-value" -W"unused-variable" -Wsign-compare -fpermissive -frtti -fno-omit-frame-pointer -fcommon -std=c11



CXX          = g++
LDFLAGS      = -O3 -DIL_STD -fthreadsafe-statics -flto -Wswitch -Wno-deprecated-declarations -Wempty-body -Wconversion -Wreturn-type -Wparentheses -Wno-pointer-sign -Wno-format -Wuninitialized -Wunreachable-code -Wunused-function -Wunused-value -Wunused-variable -Wsign-compare -std=c++11 -funroll-all-loops -Wall -fno-strict-aliasing -I /opt/ibm/ILOG/CPLEX_Studio2211/concert/include -I /opt/ibm/ILOG/CPLEX_Studio2211/cplex/include -g2 -gdwarf-2 
LIBS 	    = -fPIC -lilocplex -lconcert -lcplex -lm -lpthread -ldl -fopenmp -L /opt/ibm/ILOG/CPLEX_Studio2211/concert/lib/x86-64_linux/static_pic -L /opt/ibm/ILOG/CPLEX_Studio2211/cplex/lib/x86-64_linux/static_pic




$(OUT): $(OBJFILES)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

OBJSC =   modelo.o item.o kosaraju.o leer_datos.o parameters.o Problema.o Subproblema.o truck.o utilidades.o bouknap.o relaxfix.o BusquedaLocal.o
OBJSF  = 
OBJS= $(OBJSC) $(OBJSF)

default: Roadef2022

all: $(CPROGS)

clean:	
	rm -f core  
	rm -f salida* 
	rm -f sal* 
	rm -f *.o  
	rm -f *_$(NUM).exe  

Roadef2022 :   renault.o $(OBJS)
	$(CXX) $(LDFLAGS) -o ../TeamS46_$(NUM).exe renault.o $(OBJS) $(LIBS)

.f.o:
	$(F77) $(FFLAGS) -c $<

.cpp.o:
	$(CXX) -c $(LDFLAGS) -c $(LIBS) $<

.c.o:
	$(CC) -c $(CFLAGS) -c $(LIBS) $<

