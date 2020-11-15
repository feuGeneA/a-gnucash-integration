FROM debian:buster-backports AS runtimeenv
RUN apt-get update && apt-get -t buster-backports install --yes gnucash

FROM runtimeenv AS buildenv
RUN apt-get update && apt-get -t buster-backports install --yes g++ cmake libglib2.0-dev
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
COPY --from=buildenv /src/build /usr/local/bin
ENTRYPOINT ["a-gnucash-integration"]
