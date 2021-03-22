#!make
CURRENT_PATH	=	.
BUILD_HOME		=	$(CURRENT_PATH)/build
LIBS_HOME		=	$(BUILD_HOME)/libs
BIN_HOME		=	$(BUILD_HOME)/bin
OBJS_HOME		=	$(BUILD_HOME)/objs
SRC_HOME		=	$(CURRENT_PATH)/src
SYSTEM_ARCH		=	x64

CXX	=	g++
CC	=	gcc
LD	=	ld
AR  =   ar
RM	=	rm -f
LIBCOBJS 	=			
LIBSOBJS	=
LIBCXXOBJS 	=	$(OBJS_HOME)/tiny_string.o						\
				$(OBJS_HOME)/tiny_md5.o							\
				$(OBJS_HOME)/tiny_sha1.o						\
				$(OBJS_HOME)/tiny_base64.o						\
				$(OBJS_HOME)/tiny_location.o					\
				$(OBJS_HOME)/tinyjson.o							\
				$(OBJS_HOME)/tinyxml2.o							\
				$(OBJS_HOME)/tiny_time.o						\
				$(OBJS_HOME)/tiny_assert.o						\
				$(OBJS_HOME)/tiny_parser.o						\
				$(OBJS_HOME)/tiny_socket.o						\
				$(OBJS_HOME)/tiny_thread.o						\
				$(OBJS_HOME)/tiny_logger.o						\
				$(OBJS_HOME)/tiny_event_center.o				\
				$(OBJS_HOME)/tiny_sql_helper.o					\
				$(OBJS_HOME)/tiny_sqlite3_helper.o				\
				$(OBJS_HOME)/tiny_file.o
				

# Compile and link options

INCPATH 	= -I. 
INCPATH		+=	-I$(CURRENT_PATH)/include
INCPATH		+=	-I$(CURRENT_PATH)/3rd/sqlite/linux/include
INCPATH		+=	-I$(CURRENT_PATH)/3rd/tinyutils/include
INCPATH		+=	-I$(CURRENT_PATH)/3rd/sqlite/linux/include


CXXFLAGS =  -std=c++11 $(INCPATH) \
     -DUNI_POSIX -Wall -Os -fPIC -static

CCFLAGS =  -std=c99 $(INCPATH) \
     -DUNI_POSIX -D_GLIBCXX_USE_C99 -Wall -Os -fPIC -static

CFLAGS +=  	-mno-unaligned-access -fno-aggressive-loop-optimizations
CXXFlAGS += -mno-unaligned-access -fno-aggressive-loop-optimizations


CLIBS = -lrt -ldl -lpthread 
	
LFLAGS	=	-Wl	


SQLITE3_STATIC	=	$(CURRENT_PATH)/3rd/sqlite/linux/lib/libsqlite3.a
####################################################################
LIBTARGET = $(LIBS_HOME)/libtinyutils.a
BINTARGET = $(BIN_HOME)/tinyutils_test
####################################################################
# make all
# client:all


####################################################################

all: checkoutdir $(BINTARGET)

lib: $(LIBTARGET)

$(BINTARGET): $(LIBTARGET) $(OBJS_HOME)/tinyutils_main.o
	$(CXX) -o $(BINTARGET) $(OBJS_HOME)/tinyutils_main.o $(LIBTARGET) $(LIBEVENT_STATIC) $(SQLITE3_STATIC) $(CURL_STATIC) $(LIBSSL_STATIC) -DUNI_POSIX  $(CLIBS)

$(LIBTARGET): $(LIBCOBJS) $(LIBSOBJS) $(LIBCXXOBJS)
	$(AR) rsv $(LIBTARGET) $(LIBCOBJS) $(LIBSOBJS) $(LIBCXXOBJS)
	
		
install: checkinstdir $(TARGET)
	install -m 755 $(TARGET) $(P_INST_BINDIR)

checkoutdir: 
	@if test ! -d $(OBJS_HOME) ; \
	then \
		mkdir -p $(OBJS_HOME) ; \
	fi
	@if test ! -d $(LIBS_HOME) ; \
	then \
		mkdir -p $(LIBS_HOME) ; \
	fi
	@if test ! -d $(BIN_HOME) ; \
	then \
		mkdir -p $(BIN_HOME) ; \
	fi


####################################################################
# clean
cleancli:clean
clean:
	$(RM) $(LIBSOBJS) $(LIBCOBJS) $(TARGET) $(LIBTARGET) $(LIBCXXOBJS)
	$(RM) $(OBJS_HOME)/tinyutils_main.o
	$(RM) -rf $(BUILD_HOME)

####### Compile
$(OBJS_HOME)/tinyutils_main.o: $(CURRENT_PATH)/tinyutils_main.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tinyutils_main.o $(CURRENT_PATH)/tinyutils_main.cpp
	
###LIBCOBJS
$(OBJS_HOME)/tinyjson.o: $(SRC_HOME)/tinyjson.c $(SRC_HOME)/tinyjson.h
	$(CC) -c $(CCFLAGS) $(INCPATH) -o $(OBJS_HOME)/tinyjson.o $(SRC_HOME)/tinyjson.c
	

###### LIBCXXOBJS
$(OBJS_HOME)/tiny_string.o: $(SRC_HOME)/tiny_string.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_string.o $(SRC_HOME)/tiny_string.cpp
	
$(OBJS_HOME)/tiny_md5.o: $(SRC_HOME)/tiny_md5.cpp $(CURRENT_PATH)/include/tiny_byte_order.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_md5.o $(SRC_HOME)/tiny_md5.cpp
	
$(OBJS_HOME)/tiny_sha1.o: $(SRC_HOME)/tiny_sha1.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_sha1.o $(SRC_HOME)/tiny_sha1.cpp
	
$(OBJS_HOME)/tiny_base64.o: $(SRC_HOME)/tiny_base64.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_base64.o $(SRC_HOME)/tiny_base64.cpp
	
$(OBJS_HOME)/tiny_location.o: $(SRC_HOME)/tiny_location.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_location.o $(SRC_HOME)/tiny_location.cpp
		
$(OBJS_HOME)/tinyxml2.o: $(SRC_HOME)/tinyxml2.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tinyxml2.o $(SRC_HOME)/tinyxml2.cpp
		
$(OBJS_HOME)/tiny_time.o: $(SRC_HOME)/tiny_time.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_time.o $(SRC_HOME)/tiny_time.cpp
		
$(OBJS_HOME)/tiny_assert.o: $(SRC_HOME)/tiny_assert.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_assert.o $(SRC_HOME)/tiny_assert.cpp
		
$(OBJS_HOME)/tiny_parser.o: $(SRC_HOME)/tiny_parser.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_parser.o $(SRC_HOME)/tiny_parser.cpp
		
$(OBJS_HOME)/tiny_socket.o: $(SRC_HOME)/tiny_socket.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_socket.o $(SRC_HOME)/tiny_socket.cpp
		
$(OBJS_HOME)/tiny_logger.o: $(SRC_HOME)/tiny_logger.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_logger.o $(SRC_HOME)/tiny_logger.cpp
		
$(OBJS_HOME)/tiny_thread.o: $(SRC_HOME)/tiny_thread.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_thread.o $(SRC_HOME)/tiny_thread.cpp
		
$(OBJS_HOME)/tiny_event_center.o: $(SRC_HOME)/tiny_event_center.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_event_center.o $(SRC_HOME)/tiny_event_center.cpp
		
$(OBJS_HOME)/tiny_sql_helper.o: $(SRC_HOME)/tiny_sql_helper.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_sql_helper.o $(SRC_HOME)/tiny_sql_helper.cpp
		
$(OBJS_HOME)/tiny_sqlite3_helper.o: $(SRC_HOME)/tiny_sqlite3_helper.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_sqlite3_helper.o $(SRC_HOME)/tiny_sqlite3_helper.cpp
		
$(OBJS_HOME)/tiny_file.o: $(SRC_HOME)/tiny_file.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $(OBJS_HOME)/tiny_file.o $(SRC_HOME)/tiny_file.cpp

