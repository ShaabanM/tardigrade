CC = g++

CXXFLAGS = -g -Wall

# The only dependencies for this script are the DL api and fits managment 
LIBS = -lcfitsio -ldlapi 

TARGET = capture

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CXXFLAGS) -o $(TARGET) $(TARGET).cpp $(LIBS)

clean:
	$(RM) $(TARGET)
