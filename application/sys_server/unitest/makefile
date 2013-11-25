

#---------------------------------------------------------------------
WORKSPACE := .

#---------------------------------------------------------------------
CC := g++
AR := ar

#---------------------------------------------------------------------
INCS := 
CFLAGS := -O2 $(INCS) -Wall 
#---------------------------------------------------------------------
FILES :=$(WORKSPACE)/client_test.cpp\
	

		 
OBJS :=$(WORKSPACE)/client_test.o\
	

#---------------------------------------------------------------------
TARGETS         := client_test
LDFLAGS         := -L ../../../../osal/library -L ../../../../module/gmi/command_pipeline/library -L ../../../../module/gmi/rudp/library
SYS_LIBS        := -lcommand_pipeline_d -lrudp -losal_d -lpthread 

all: $(TARGETS)

$(TARGETS):$(OBJS)
	$(CC) -o $@ $(DEFS) $(CFLAGS) $(LDFLAGS) $(OBJS) $(SYS_LIBS)
  
.PHONY:clean

clean:
	rm -f $(TARGETS) ./*/*.o ./*/*.c~ ./*/*.h~ *.o	
