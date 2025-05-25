# 编译器
CXX = g++
CC = gcc

SRC_DIR = src
OBJ_DIR = obj

# 编译选项
CXXFLAGS = -g -std=c++11
CFLAGS = -g

# 源文件（根据你tasks.json中列出的）
SRCS_CPP = main.cpp algorithm_process.cpp pic_process.cpp ground_comm.cpp
SRCS = $(addprefix $(SRC_DIR)/, $(SRCS_CPP))
SRCS_C = udp_socket.c trdp_pd.c
SRCSC = $(addprefix $(SRC_DIR)/, $(SRCS_C))

# 对应的对象文件
#OBJS = $(SRCS:.cpp=.o) $(SRCSC:.c=.o)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS)) $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCSC))
# 头文件路径
INCLUDES = -I./inc -I./trdp/api -I./trdp/vos/api -I./zmqlib_old/include

# 库路径
LIBS = -L./zmqlib_old/lib -lzmq -lpthread -lm -L./trdp -ltrdp -luuid

# 创建目标目录
$(shell mkdir -p $(OBJ_DIR))
# 输出目标
TARGET = control_soft

# 默认目标
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)
