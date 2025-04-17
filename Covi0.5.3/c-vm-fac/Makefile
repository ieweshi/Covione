CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SRC = src/main.c src/vm.c
OBJ = $(SRC:.c=.o)
TARGET = ../env/covim

COVIO_SO = ../CoviSys/NativeLib/libCovio.so
COVIO_SRC = ../CoviSys/NativeLink/Covio.c

COVICC = ../env/covicc
COVICC_SRC = ../covicc.cpp

LIBTEST_SO = ../CoviSys/NativeLib/libtest.so
LIBTEST_SRC = ../CoviSys/NativeLink/ntivetest.c

CNILINK_SO = ../CoviSys/NativeLib/cnilink.so
CNILINK_SRC = ../CoviSys/NativeLink/cnilink.c

all: $(TARGET) $(COVIO_SO) $(COVICC) $(LIBTEST_SO) $(CNILINK_SO)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -ldl -o $(TARGET)

$(COVIO_SO): $(COVIO_SRC)
	mkdir -p ../CoviSys/NativeLib
	$(CC) -shared -fPIC -o $(COVIO_SO) $(COVIO_SRC)

$(COVICC): $(COVICC_SRC)
	mkdir -p ../env
	g++ -std=c++11 -o $(COVICC) $(COVICC_SRC)

$(LIBTEST_SO): $(LIBTEST_SRC)
	mkdir -p ../CoviSys/NativeLib
	$(CC) -shared -fPIC -o $(LIBTEST_SO) $(LIBTEST_SRC)

$(CNILINK_SO): $(CNILINK_SRC)
	mkdir -p ../CoviSys/NativeLib
	$(CC) -shared -fPIC -o $(CNILINK_SO) $(CNILINK_SRC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) $(COVICC) $(COVIO_SO) $(LIBTEST_SO) $(CNILINK_SO)