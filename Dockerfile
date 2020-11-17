FROM debian:buster-backports AS runtimeenv
# Yes there are -dev packages being installed in the runtime environment, but
# it's easier to do that than it is to install whatever version the -dev pkg
# gives into buildenv and then try to figure out the corresponding non-dev pkg
# version to install in the runtimeenv.
RUN apt-get update && apt-get -t buster-backports install --yes \
    gdb \
    libboost-date-time-dev \
    libboost-filesystem-dev \
    libboost-locale-dev \
    libboost-program-options-dev \
    libboost-regex-dev \
    libboost-system-dev \
    libdbd-pgsql \
    libglib2.0-dev

FROM runtimeenv AS commonbuildenv
RUN apt-get -t buster-backports install --yes cmake g++

FROM commonbuildenv AS gnucashbuildenv
RUN apt-get -t buster-backports install --yes \
    gettext \
    git \
    googletest \
    guile-2.2-dev \
    libaqbanking-dev \
    libdbd-pgsql \
    libdbi-dev \
    libgwengui-gtk3-dev \
    libgwenhywfar-core-dev \
    libofx-dev \
    libsecret-1-dev \
    libwebkit2gtk-4.0-dev \
    libxml2-dev \
    libxslt1-dev \
    pkg-config \
    swig \
    xsltproc
RUN mkdir -p /home/root
WORKDIR /home/root
RUN git clone https://github.com/Gnucash/gnucash.git

RUN apt-get -t buster-backports install --yes git-restore-mtime
# git-restore-mtime is broken (fixed in repo but not in dpkg). patch to fix it:
RUN sed -i '287s/file\[1:-1\].decode("string-escape")/file[1:-1].encode("latin1").decode("unicode-escape").encode("latin1").decode("utf-8")/' /usr/lib/git-core/git-restore-mtime

WORKDIR gnucash
RUN git restore-mtime
WORKDIR ..

RUN mkdir build
WORKDIR build
RUN cmake -DCMAKE_INSTALL_PREFIX=/opt/gnucash -DCMAKE_BUILD_TYPE=debug ../gnucash && make && make install
COPY ./gnucash /home/root/gnucash_modded
RUN cp -uR /home/root/gnucash_modded/* /home/root/gnucash
RUN make && make install

# work around what seems to be a bug in gnucash:
# following replacement needs to happen for external clients of this header, but cannot happen before building gnucash, or you'll see:
#     In file included from /opt/gnucash/src/libgnucash/engine/qoflog.h:95,
#                      from /opt/gnucash/src/libgnucash/engine/qof.h:73,
#                      from /opt/gnucash/src/libgnucash/engine/kvp-value.hpp:30,
#                      from /opt/gnucash/src/libgnucash/engine/kvp-value.cpp:24:
#     /opt/gnucash/src/libgnucash/engine/qofutil.h:59:5: error: #error "No scanf format string is known for LLD. Fix your ./configure so that the correct one is detected!"
#      #   error "No scanf format string is known for LLD. Fix your ./configure so that the correct one is detected!"
#          ^~~~~
#     make[2]: *** [libgnucash/engine/CMakeFiles/gnc-engine.dir/build.make:756: libgnucash/engine/CMakeFiles/gnc-engine.dir/kvp-value.cpp.o] Error 1
#     make[1]: *** [CMakeFiles/Makefile2:5471: libgnucash/engine/CMakeFiles/gnc-engine.dir/all] Error 2
#     make: *** [Makefile:163: all] Error 2
RUN sed -ie 's/#include <config.h>//' /opt/gnucash/include/gnucash/kvp-value.hpp

FROM commonbuildenv AS buildenv
COPY --from=gnucashbuildenv /opt/gnucash /opt/gnucash
RUN apt-get install -t buster-backports --yes gdb
RUN mkdir src src/build
WORKDIR src
# create one docker layer for changes to CMakeLists.txt, and a different layer
# for changes to source code, so that changes to source code don't induce
# re-runs of cmake.  do this by creating 0-length source files, where cmake
# expects them, and then, after the cmake layer is built, actually COPY the
# sources into place.
RUN touch a-gnucash-integration.cpp
COPY CMakeLists.txt .
WORKDIR build
RUN cmake ..
COPY a-gnucash-integration.cpp ..
RUN make

FROM runtimeenv
COPY --from=gnucashbuildenv /opt/gnucash /opt/gnucash
COPY --from=gnucashbuildenv /home/root/gnucash /home/root/gnucash
COPY --from=buildenv /src/build/a-gnucash-integration /usr/local/bin/
ENTRYPOINT ["a-gnucash-integration"]
