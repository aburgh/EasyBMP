EXECUTABLE := BmpInfo
EASYBMP := ../../
BMPSUIT := bmpsuit

all: compile run

compile:
	ccache g++ --std=c++17 -g -O0 -I. -I$(EASYBMP) $(EASYBMP)/EasyBMP.cpp main.cpp -o $(EXECUTABLE)

run: $(BMPSUIT)
	./BmpInfo -h
	./BmpInfo -i $(BMPSUITE)/g/pal8.bmp

$(BMPSUIT):
	@git clone https://github.com/jsummers/bmpsuite.git