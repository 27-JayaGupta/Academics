CC = gcc
CFLAGS = -g

LIB=lib
BIN=bin
OBJ=obj
SRC=src
LIB_SRC=common
INC=inc
LIB_OBJ=lib


HDRS=$(shell ls $(INC)/*.h)

#all: crdir generic dma mmio mmap multiprocess
all: crdir dma mmio mmap multiprocess

crdir:
	cp ../include/crypter.h ./inc/crypter.h
	mkdir -p lib
	cp ../lib/libcrypter.so ./lib/libcrypter.so
	#HDRS=$(shell ls $(INC)/*.h)
	mkdir -p bin/Generic
	mkdir -p bin/MMIO/IRQ
	mkdir -p bin//MMIO/withoutIRQ
	mkdir -p bin/DMA/IRQ
	mkdir -p bin//DMA/withoutIRQ
	mkdir -p bin/MMAP/IRQ
	mkdir -p bin//MMAP/withoutIRQ
	mkdir -p bin/MultiProcess
	mkdir -p obj/Generic
	mkdir -p obj/MMIO/IRQ
	mkdir -p obj//MMIO/withoutIRQ
	mkdir -p obj/DMA/IRQ
	mkdir -p obj//DMA/withoutIRQ
	mkdir -p obj/MMAP/IRQ
	mkdir -p obj//MMAP/withoutIRQ
	mkdir -p obj/MultiProcess

#generic: $(BIN)/Generic/test1 $(BIN)/Generic/test2 $(BIN)/Generic/test3 $(BIN)/Generic/test4

multiprocess: $(BIN)/MultiProcess/test1 $(BIN)/MultiProcess/test2 $(BIN)/MultiProcess/test3 $(BIN)/MultiProcess/test4 $(BIN)/MultiProcess/test5 $(BIN)/MultiProcess/test6 $(BIN)/MultiProcess/test7 $(BIN)/MultiProcess/test8 $(BIN)/MultiProcess/test9 $(BIN)/MultiProcess/test10

dma: dma_irq dma_wirq

dma_irq: $(BIN)/DMA/IRQ/test1 $(BIN)/DMA/IRQ/test2 $(BIN)/DMA/IRQ/test3 $(BIN)/DMA/IRQ/test4 $(BIN)/DMA/IRQ/test5

dma_wirq: $(BIN)/DMA/withoutIRQ/test1 $(BIN)/DMA/withoutIRQ/test2 $(BIN)/DMA/withoutIRQ/test3 $(BIN)/DMA/withoutIRQ/test4 $(BIN)/DMA/withoutIRQ/test5

mmap: mmap_irq mmap_wirq

mmap_irq: $(BIN)/MMAP/IRQ/test1 $(BIN)/MMAP/IRQ/test2 $(BIN)/MMAP/IRQ/test3 $(BIN)/MMAP/IRQ/test4

mmap_wirq: $(BIN)/MMAP/withoutIRQ/test1 $(BIN)/MMAP/withoutIRQ/test2 $(BIN)/MMAP/withoutIRQ/test3 $(BIN)/MMAP/withoutIRQ/test4


mmio: mmio_irq mmio_wirq

mmio_irq: $(BIN)/MMIO/IRQ/test1 $(BIN)/MMIO/IRQ/test2 $(BIN)/MMIO/IRQ/test3 $(BIN)/MMIO/IRQ/test4 $(BIN)/MMIO/IRQ/test5

mmio_wirq: $(BIN)/MMIO/withoutIRQ/test1 $(BIN)/MMIO/withoutIRQ/test2 $(BIN)/MMIO/withoutIRQ/test3 $(BIN)/MMIO/withoutIRQ/test4 $(BIN)/MMIO/withoutIRQ/test5


$(BIN)/%: $(OBJ)/%.o $(LIB_OBJ)/gen.o
	$(CC) -o $@ -I$(INC) $^ -L$(LIB) -lcrypter -lpthread

#$(OBJ)/%.o: $(SRC)/%.c $(HDRS)
$(OBJ)/%.o: $(SRC)/%.c inc/crypter.h inc/gen.h
	$(CC) -c -I$(INC) $< -o $@

# library: $(LIB)/libcommon.so
#
# $(LIB)/libcommon.so: $(LIB_OBJ)/gen.o
# 	gcc -shared -o $@ $^

#$(LIB_OBJ)/%.o: $(LIB_SRC)/%.c $(HDRS)

$(LIB_OBJ)/%.o: $(LIB_SRC)/%.c inc/crypter.h inc/gen.h 
	mkdir -p lib
	$(CC) -c -I$(INC) $< -o $@

%-pa-cs730.tar.gz:	clean
	tar cf - `find . -type f | grep -v '^\.*$$' | grep -v '/CVS/' | grep -v '/\.svn/' | grep -v '/\.git/' | grep -v '[0-9].*\.tar\.gz' | grep -v '/submit.token$$'` | gzip > $@

.PHONY: prepare-submit
prepare-submit: $(RNO)-pa-cs730.tar.gz

.PHONY: clean
clean:
	rm -rf  $(OBJ) $(BIN) $(LIB) $(INC)/crypter.h
