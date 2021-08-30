.PHONY: build build_debug build_release test cloc debug write_build_type

DOCKER_FLAGS ?= -i -t
GNUMAKEFLAGS := --no-print-directory

ifndef CMAKE_TOOLCHAIN_PATH
	CMAKE_ADDITIONAL_FLAGS=
else
	CMAKE_ADDITIONAL_FLAGS=-DCMAKE_TOOLCHAIN_FILE="$(CMAKE_TOOLCHAIN_PATH)"
endif

build:
	@[ -f .build ] || echo "debug" > .build
	${MAKE} build_`cat .build`

build_debug:
	@echo "debug" > .build
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -S . -B build -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_MAKE_PROGRAM="${MAKE}" $(CMAKE_ADDITIONAL_FLAGS)
	cmake --build build -j 4
	cp build/compile_commands.json .

build_release:
	@echo "release" > .build
	cmake -S . -B build -DCMAKE_BUILD_TYPE="Release" -DCMAKE_MAKE_PROGRAM="${MAKE}" $(CMAKE_ADDITIONAL_FLAGS)
	cmake --build build -j 4

bootstrap: build
	cmake --build build -t bootstrap -j

clean:
	if [ -d build ]; then ${MAKE} -C build -f CMakeFiles/Makefile2 CMakeFiles/natalie-base.dir/clean; fi

cleanall:
	rm -rf build

test: build
	bundle exec ruby test/all.rb

test_valgrind: build
	mkdir -p test/tmp
	bin/natalie -d valgrind test/natalie/assign_test.rb
	bin/natalie -d valgrind spec/language/block_spec.rb
	bin/natalie -d valgrind test/natalie/array_test.rb
	bin/natalie -d valgrind test/natalie/if_test.rb
	bin/natalie -d valgrind test/natalie/file_test.rb
	bin/natalie -d valgrind test/natalie/loop_test.rb
	#bin/natalie -d valgrind test/natalie/fiber_test.rb

docker_build:
	docker build -t natalie .

docker_bash: docker_build
	docker run -it --rm --entrypoint bash natalie

docker_build_clang:
	docker build -t natalie_clang --build-arg CC=clang --build-arg CXX=clang++ .

docker_test: docker_test_gcc docker_test_clang docker_test_valgrind

docker_test_gcc: docker_build
	docker run $(DOCKER_FLAGS) --rm --entrypoint ${MAKE} natalie test

docker_test_clang: docker_build_clang
	docker run $(DOCKER_FLAGS) --rm --entrypoint ${MAKE} natalie_clang test

docker_test_valgrind: docker_build
	docker run $(DOCKER_FLAGS) --rm --entrypoint ${MAKE} natalie test_valgrind

docker_test_release: docker_build
	docker run $(DOCKER_FLAGS) --rm --entrypoint ${MAKE} natalie clean build_release test

cloc:
	cloc include lib src test

ctags:
	ctags -R --exclude=.cquery_cache --exclude=ext --exclude=build --append=no .

format:
	find include -type f -name '*.hpp' -exec clang-format -i --style=file {} +
	find src -type f -name '*.cpp' -exec clang-format -i --style=file {} +

todo:
	egrep -r "FIXME|TODO" src include
