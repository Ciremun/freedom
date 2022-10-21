CXX=cl
CSC=csc

INJECTOR = freedom.exe
INJECTOR_SRC = injector.cpp
INJECTOR_OBJ = injector.obj
INJECTOR_CXXFLAGS = -DNDEBUG -DUNICODE -std:c++17 -O2 -EHsc -nologo
INJECTOR_LINKER_FLAGS = -link -MACHINE:x86

FREEDOM = freedom.dll
FREEDOM_SRC := $(wildcard freedom/*.cpp) $(wildcard imgui/*.cpp) $(wildcard imgui/backends/*.cpp)
FREEDOM_OBJ = $(addsuffix .obj, $(basename $(notdir $(FREEDOM_SRC))))
FREEDOM_CXXFLAGS = -DWIN32_LEAN_AND_MEAN -DUNICODE -std:c++17 -DIMGUI_USE_STB_SPRINTF -O2 -EHsc -nologo -Iinclude -Iimgui -Iimgui/backends
FREEDOM_LINKER_FLAGS = -link -DLL -MACHINE:x86 -OUT:freedom.dll

PREJIT = prejit.dll
PREJIT_SRC = freedom/prejit.cs
PREJIT_CSCFLAGS = -nologo -target:library -out:$(PREJIT)

all: $(INJECTOR) $(FREEDOM) $(PREJIT)

$(INJECTOR_OBJ): $(INJECTOR_SRC)
	$(CXX) $(INJECTOR_CXXFLAGS) -c $<

%.obj: freedom/%.cpp
	$(CXX) $(FREEDOM_CXXFLAGS) -c $<

%.obj: imgui/%.cpp
	$(CXX) $(FREEDOM_CXXFLAGS) -c $<

%.obj: imgui/backends/%.cpp
	$(CXX) $(FREEDOM_CXXFLAGS) -c $<

$(INJECTOR): $(INJECTOR_OBJ)
	$(CXX) $(INJECTOR_CXXFLAGS) -Fe:$(INJECTOR) $^ $(INJECTOR_LINKER_FLAGS)

$(FREEDOM): $(FREEDOM_OBJ)
	$(CXX) $(FREEDOM_CXXFLAGS) $^ $(FREEDOM_LINKER_FLAGS)

$(PREJIT): $(FREEDOM) $(PREJIT_SRC)
	$(CSC) $(PREJIT_CSCFLAGS) $(PREJIT_SRC)
