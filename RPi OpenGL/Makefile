NAME=rpi-simple-paramplot
CXXFLAGS=-Wall -std=c++0x -g
INCLUDES=-I/opt/vc/include \
		 -I/opt/vc/include/interface/vcos/pthreads \
		 -I/opt/vc/include/interface/vmcs_host/linux \
		 `pkg-config --cflags sdl`
LDFLAGS=-L/opt/vc/lib -lGLESv2 -lalut -lEGL -lbcm_host `pkg-config --libs sdl`
SRCS=main.cpp graphics.cpp evaluator.cpp shader.cpp audioin.cpp model.cpp primitives.cpp
ALLSRC=$(SRC) util.hpp audioin.hpp graphics.hpp evaluator.hpp shader.hpp exceptions.hpp Makefile cmds.txt mover.hpp
OBJS=$(SRCS:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

debug: $(NAME)
	$(CXX) -g -o $@ $(OBJS) $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

main.o: util.hpp graphics.hpp evaluator.hpp exceptions.hpp shader.hpp model.hpp primitives.hpp
graphics.o: graphics.hpp exceptions.hpp shader.hpp model.cpp 
evaluator.o: evaluator.hpp
audioin.o: audioin.hpp
model.o: model.hpp 
#primitives.o: complex_model.hpp simple_model.hpp
commit:
	git add $(ALLSRC)
	git commit

clean:
	rm -f $(OBJS)
	rm -f $(NAME)
