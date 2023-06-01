OBJS	= main.o ProcessConfig.o Dispatcher.o Producer.o Queue.o FreeResource.o manager.o CoEditor.o initThreads.o initStructsObjects.o
SOURCE	= main.c ProcessConfig.c Dispatcher.c Producer.c Queue.c FreeResource.c manager.c CoEditor.c initThreads.c initStructsObjects.c
HEADER	= Producer.h ProcessConfig.h Queue.h Dispatcher.h CoEditor.h manager.h Structs.h FreeResource.h initThreads.h initStructsObjects.h
OUT	= ex3.out
CC	 = gcc
FLAGS	 = -g -c -Wall -pthread -lrt
LFLAGS	 = -lpthread

all: $(OBJS)
	@$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

main.o: main.c
	@$(CC) $(FLAGS) main.c -std=c11

ProcessConfig.o: ProcessConfig.c
	@$(CC) $(FLAGS) ProcessConfig.c -std=c11

Dispatcher.o: Dispatcher.c
	@$(CC) $(FLAGS) Dispatcher.c -std=c11

Producer.o: Producer.c
	@$(CC) $(FLAGS) Producer.c -std=c11

Queue.o: Queue.c
	@$(CC) $(FLAGS) Queue.c -std=c11

FreeResource.o: FreeResource.c
	@$(CC) $(FLAGS) FreeResource.c -std=c11

manager.o: manager.c
	@$(CC) $(FLAGS) manager.c -std=c11

CoEditor.o: CoEditor.c
	@$(CC) $(FLAGS) CoEditor.c -std=c11 2>/dev/null

initThreads.o: initThreads.c
	@$(CC) $(FLAGS) initThreads.c -std=c11

initStructsObjects.o: initStructsObjects.c
	@$(CC) $(FLAGS) initStructsObjects.c -std=c11


clean:
	@rm -f $(OBJS) $(OUT)
