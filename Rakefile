task default: :build

desc 'Build Natalie (same as build_debug)'
task build: :build_debug

desc 'Build Natalie with no optimization and all warnings (default)'
task build_debug: [:set_build_debug, :libnatalie]

desc 'Build Natalie with release optimizations enabled and warnings off'
task build_release: [:set_build_release, :libnatalie]

desc 'Remove temporary files created during build'
task :clean do
  rm_rf 'build/build.log'
  rm_rf 'build/generated'
  rm_rf 'build/libnatalie_base.a'
  rm_rf Rake::FileList['build/*.o']
end

desc 'Remove all generated files'
task :clobber do
  rm_rf 'build'
end

desc 'Run the test suite'
task test: :build do
  sh 'bundle exec ruby test/all.rb'
end

desc 'Build the self-hosted version of Natalie at ./nat'
task bootstrap: [:build, :nat]

desc 'Show line counts for the project'
task :cloc do
  sh 'cloc include lib src test'
end

desc 'Generate tags file for development'
task :ctags do
  sh 'ctags -R --exclude=.cquery_cache --exclude=ext --exclude=build --append=no .'
end
task tags: :ctags

desc 'Format C++ code with clang-format'
task :format do
  sh "find include -type f -name '*.hpp' -exec clang-format -i --style=file {} +"
  sh "find src -type f -name '*.cpp' -exec clang-format -i --style=file {} +"
end

desc 'Show TODO and FIXME comments in the project'
task :todo do
  sh "egrep -r 'FIXME|TODO' src include lib"
end


# # # # Docker Tasks (used for CI) # # # #

DOCKER_FLAGS = !ENV['CI'] && STDOUT.isatty ? '-i -t' : ''

task :docker_build do
  sh "docker build #{ENV['DOCKER_BUILD_FLAGS']} -t natalie ."
end

task docker_bash: :docker_build do
  sh 'docker run -it --rm --entrypoint bash natalie'
end

task :docker_build_clang do
  sh 'docker build -t natalie_clang --build-arg CC=clang --build-arg CXX=clang++ .'
end

task :docker_build_ruby3 do
  sh 'docker build -t natalie_ruby3 --build-arg IMAGE="ruby:3.0" .'
end

task docker_test: [:docker_test_gcc, :docker_test_clang, :docker_test_release, :docker_test_ruby3]

task docker_test_gcc: :docker_build do
  sh "docker run #{DOCKER_FLAGS} --rm --entrypoint rake natalie test"
end

task docker_test_clang: :docker_build_clang do
  sh "docker run #{DOCKER_FLAGS} --rm --entrypoint rake natalie_clang test"
end

task docker_test_release: :docker_build do
  sh "docker run #{DOCKER_FLAGS} --rm --entrypoint rake natalie clean build_release test"
end

# NOTE: this tests that Natalie can be hosted by MRI 3.0 -- not Natalie under Ruby 3 specs
task docker_test_ruby3: :docker_build_ruby3 do
  sh "docker run #{DOCKER_FLAGS} --rm --entrypoint rake natalie_ruby3 test"
end


# # # # Build Compile Database # # # #

if system('which compiledb 2>&1 >/dev/null')
  $compiledb_out = []

  def $stderr.puts(str)
    write(str + "\n")
    $compiledb_out << str
  end

  task :write_compile_database do
    if $compiledb_out.any?
      File.write('build/build.log', $compiledb_out.join("\n"))
      sh 'compiledb < build/build.log'
    end
  end
else
  task :write_compile_database do
    # noop
  end
end


# # # # Internal Tasks and Rules # # # #

STANDARD = 'c++17'
HEADERS = Rake::FileList['include/**/{*.h,*.hpp}']
PRIMARY_SOURCES = Rake::FileList['src/**/*.{c,cpp}'].exclude('src/main.cpp', 'src/parser_c_ext/*')
RUBY_SOURCES = Rake::FileList['src/**/*.rb'].exclude('**/extconf.rb')
SPECIAL_SOURCES = Rake::FileList['build/generated/platform.cpp', 'build/generated/bindings.cpp']
OBJECT_FILES = PRIMARY_SOURCES.pathmap('build/%f.o') +
               RUBY_SOURCES.pathmap('build/generated/%f.o') +
               SPECIAL_SOURCES.pathmap('%p.o')

task(:set_build_debug) { ENV['BUILD'] = 'debug'; File.write('.build', 'debug') }
task(:set_build_release) { ENV['BUILD'] = 'release'; File.write('.build', 'release') }

task libnatalie: [
  :build_dir,
  'build/onigmo/lib/libonigmo.a',
  :primary_sources,
  :ruby_sources,
  :special_sources,
  'build/libnatalie.a',
  :write_compile_database,
]

task :build_dir do
  mkdir_p 'build/generated' unless File.exist?('build/generated')
end

multitask primary_sources: PRIMARY_SOURCES.pathmap('build/%f.o')
multitask ruby_sources: RUBY_SOURCES.pathmap('build/generated/%f.o')
multitask special_sources: SPECIAL_SOURCES.pathmap('build/generated/%f.o')

file 'build/libnatalie.a' => ['build/libnatalie_base.a', 'build/onigmo/lib/libonigmo.a'] do |t|
  if RUBY_PLATFORM =~ /darwin/
    sh "libtool -static -o #{t.name} #{t.sources.join(' ')}"
  else
    ar_script = ["create #{t.name}"]
    t.sources.each do |source|
      ar_script << "addlib #{source}"
    end
    ar_script << 'save'
    ENV['AR_SCRIPT'] = ar_script.join("\n")
    sh 'echo "$AR_SCRIPT" | ar -M'
  end
end

file 'build/libnatalie_base.a' => OBJECT_FILES + HEADERS do |t|
  sh "ar rcs #{t.name} #{OBJECT_FILES}"
end

file 'build/onigmo/lib/libonigmo.a' do
  build_dir = File.expand_path('build/onigmo', __dir__)
  rm_rf build_dir
  cp_r 'ext/onigmo', build_dir
  arch_conf = "--host #{ENV['ARCH']}" if ENV['ARCH']
  sh <<-SH
    cd #{build_dir} && \
    sh autogen.sh && \
    ./configure --with-pic #{arch_conf} --prefix #{build_dir} && \
    make -j 4 && \
    make install
  SH
end

file 'build/generated/platform.cpp.o' do |t|
  File.write(t.name.pathmap('%d/%n'), <<~END)
    #include "natalie.hpp"
    const char *Natalie::ruby_platform = #{RUBY_PLATFORM.inspect};
  END
  sh "#{cxx} #{cxx_flags.join(' ')} -std=#{STANDARD} -c -o #{t.name} #{t.name.pathmap('%d/%n')}"
end

file 'build/generated/bindings.cpp.o' => ['lib/natalie/compiler/binding_gen.rb'] + HEADERS do |t|
  sh "ruby lib/natalie/compiler/binding_gen.rb > #{t.name.pathmap('%d/%n')}"
  sh "#{cxx} #{cxx_flags.join(' ')} -std=#{STANDARD} -c -o #{t.name} #{t.name.pathmap('%d/%n')}"
end

file 'nat' => OBJECT_FILES do
  sh 'bin/natalie -c nat bin/natalie'
end

rule '.c.o' => 'src/%n' do |t|
  sh "#{cc} -g -fPIC -c -o #{t.name} #{t.source}"
end

rule '.cpp.o' => ['src/%n'] + HEADERS do |t|
  sh "#{cxx} #{cxx_flags.join(' ')} -std=#{STANDARD} -c -o #{t.name} #{t.source}"
end

rule '.rb.o' => ['.rb.cpp'] + HEADERS do |t|
  sh "#{cxx} #{cxx_flags.join(' ')} -std=#{STANDARD} -c -o #{t.name} #{t.source}"
end

rule '.rb.cpp' => 'src/%n' do |t|
  sh "bin/natalie --write-obj #{t.name} #{t.source}"
end

file 'build/parser_c_ext.so' => :libnatalie do
  build_dir = File.expand_path('build/parser_c_ext', __dir__)
  rm_rf build_dir
  cp_r 'src/parser_c_ext', build_dir
  sh <<-SH
    cd #{build_dir} && \
    ruby extconf.rb && \
    make && \
    cp parser_c_ext.so ..
  SH
end

def cc
  @cc ||= if ENV['CC']
    ENV['CC']
  elsif system('which ccache 2>&1 > /dev/null')
    'ccache cc'
  else
    'cc'
  end
end

def cxx
  @cxx ||= if ENV['CXX']
    ENV['CXX']
  elsif system('which ccache 2>&1 > /dev/null')
    'ccache c++'
  else
    'c++'
  end
end

def cxx_flags
  base_flags = case ENV['BUILD']
  when 'release'
    [
      '-pthread',
      '-fPIC',
      '-g',
      '-O2',
    ]
  else
    [
      '-pthread',
      '-fPIC',
      '-g',
      '-Wall',
      '-Wextra',
      '-Werror',
      '-Wno-unused-parameter',
      '-Wno-unused-variable',
      '-Wno-unused-but-set-variable',
      '-Wno-unknown-warning-option',
    ]
  end
  base_flags + include_paths.map { |path| "-I #{path}" }
end

def include_paths
  [
    File.expand_path('include', __dir__),
    File.expand_path('build/onigmo/include', __dir__),
  ]
end
