# Compiler settings
CXX = g++
# -O3 is CRITICAL for the benchmark performance leaderboard
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -O3 -I./include

# ------------------ SERVER ------------------
SERVER_SRCS = \
    src/main.cpp \
    src/network/server.cpp \
    src/parser/tokenizer.cpp \
    src/parser/parser.cpp \
    src/query/executor.cpp \
    src/storage/database.cpp \
    src/storage/table.cpp \
    src/storage/row.cpp \
    src/storage/schema.cpp \
    src/index/index.cpp \
    src/cache/lru_cache.cpp

SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
SERVER_EXEC = flexql_server

# ------------------ REPL CLIENT ------------------
CLIENT_SRCS = \
    src/main2.cpp \
    src/network/client.cpp \
    src/network/flexql.cpp

CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
CLIENT_EXEC = flexql_client

# ------------------ BENCHMARK ------------------
BENCHMARK_SRCS = \
    tests/benchmark.cpp \
    src/network/flexql.cpp

BENCHMARK_OBJS = $(BENCHMARK_SRCS:.cpp=.o)
BENCHMARK_EXEC = run_benchmark

# ------------------ TARGETS ------------------
# "make all" builds everything
all: $(SERVER_EXEC) $(CLIENT_EXEC) $(BENCHMARK_EXEC)

$(SERVER_EXEC): $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BENCHMARK_EXEC): $(BENCHMARK_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Generic rule to compile .cpp into .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up compiled files
clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) $(BENCHMARK_OBJS) $(SERVER_EXEC) $(CLIENT_EXEC) $(BENCHMARK_EXEC)

.PHONY: all clean