MAKER = "make"
all: wellcome
ifeq ("$(shell uname -s)","FreeBSD")
	MAKER = "gmake"
endif
	@if [ ! -f "src/Makefile" ]; then \
		echo "Makefile not found, run configure script before compile the code."; \
		exit; \
		else \
		cd gc; \
		./autogen.sh; \
		./configure --enable-cplusplus --enable-threads=posix --enable-thread-local-alloc --enable-parallel-mark; \
		make -j; \
		make check; \
		make -f Makefile.direct c++; \
		cd ..; \
		cd boost-sources/tools/build; \
		./bootstrap.sh; \
		./b2 install --prefix=../../compiled; \
		cd ../../; \
		cp compiled/bin/b2 ./; \
		cp compiled/bin/bjam ./; \
		./bootstrap.sh --prefix=../boost-compiled --with-libraries=system,thread,locale; \
		./b2 cxxstd=14 install --prefix=../boost-compiled; \
		cd ../src; $(MAKER); \
		cd ..; \
	fi


wellcome:
	@echo "****************************************"
	@echo "****************************************"
	@echo "************************ ,--.,---.,----."
	@echo "**** ,---. {code}        |  |  ¬_|  ,--."
	@echo "****    / .---.,   .,---.|  |  __,  |   "
	@echo "****   /  |---'    | ---.|  |  |\  \'--'"
	@echo "**** '---'^---'^---''---'^--^--^ ^--^--'"
	@echo "****************************************"
	@echo "*** { Innovating, Making The World } ***"
	@echo "****************************************"
	@echo "****************************************"
clean:
	@if [ ! -f "src/Makefile" ]; then \
		echo "Makefile not found, run configure script before compile the code."; \
		exit; \
		else \
		cd src; make clean; \
		cd ..; \
	fi
