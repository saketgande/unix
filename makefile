CC = gcc
CFLAGS = -Wall -O2
PTHREAD = -lpthread

# Executable names
SERVER_EXE = ./mathserver/src/server
CLIENT_EXE = ./Client1/client
MATRIX_INV_EXE = ./mathserver/src/matinvpar
KMEANS_EXE = ./mathserver/src/kmeanspar

# Source file paths
SERVER_SRC = mathserver/src/server.c
CLIENT_SRC = Client1/client.c
MATRIX_INV_SRC = mathserver/src/matrix_inverse.c
KMEANS_SRC = mathserver/src/kmeans.c

# Default target
all: $(SERVER_EXE) $(CLIENT_EXE) $(MATRIX_INV_EXE) $(KMEANS_EXE)

# Server build rule
$(SERVER_EXE): 
	$(CC) $(CFLAGS) -o $(SERVER_EXE) $(SERVER_SRC) $(PTHREAD)

# Client build rule
$(CLIENT_EXE): 
	$(CC) $(CFLAGS) -o $(CLIENT_EXE) $(CLIENT_SRC)

# Matrix Inversion build rule
$(MATRIX_INV_EXE): 
	$(CC) $(CFLAGS) -o $(MATRIX_INV_EXE) $(MATRIX_INV_SRC) $(PTHREAD)

# KMeans build rule
$(KMEANS_EXE): 
	$(CC) $(CFLAGS) -o $(KMEANS_EXE) $(KMEANS_SRC) $(PTHREAD)

# Clean up
clean:
	rm -f $(SERVER_EXE) $(MATRIX_INV_EXE) $(KMEANS_EXE)
	rm -f mathserver/computed_results/*.txt
	rm -f Client1/results/*.txt
	rm -f Client1/client
	rm -f mathserver/src/*.txt

.PHONY: all clean