# Compiler ve linker ayarları
CXX = g++
CXXFLAGS = -std=c++11 -Wall -g
LDFLAGS = -lglfw -lGLEW -lGL -lGLU -lglut

# Kaynak dosyaları ve çıkış dosyası
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = flappy_bird

# Derleme komutu
$(EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $(EXEC) $(LDFLAGS)

# .cpp -> .o dosyası derleme
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

# Temizleme komutu
clean:
	rm -f $(OBJS) $(EXEC)
