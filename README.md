# gcc-syscall-extractor
GCC Plugin to extract system calls, mainly for the Glibc library.

0. Install the GCC plugin development headers `sudo apt install gcc-X-plugin-dev`, where `X` is the version of GCC on your system. This plugin has been tested to work (not crash) on GCC 5 - 9.

1. Compile the plugin by running `make`

2. Configure the glibc compilation to invoke the plugin by using the `-fplugin` option. For example,
`../glibc-2.23/configure --prefix=$INSTALL --host=x86_64-linux-gnu --build=x86_64-linux-gnu CFLAGS="-fplugin=/morespace/gcc-python-plugin2/syscall-extractor/syscall_extractor.so -O2" CC=$CC`
where `$CC` is path to GCC, and `$INSTALL` is install directory.

3. During compilation, the plugin creates files with an extension `.confine` in the Glibc **source** directory. By concating them together you can get the CFG.
`find . -name "*.confine" | xargs cat > ../glibc-analysis/glibc-callgraph/egypt/egypt-1.10/initial.cfg`


Known issues:
1. System calls made in assembly are not yet handled. 
