LLVM_PREFIX = /usr

%.so: %.cpp
	$(CXX) -fPIC -fno-rtti -fno-exceptions -shared -o $@ $< `$(LLVM_PREFIX)/bin/llvm-config --cxxflags` -Wl,-headerpad_max_install_names  -Wl,-dead_strip  -Wl,-flat_namespace -Wl,-undefined -Wl,suppress
