#This is the easiest way to build the example:

1. Build before the libraries. This ensure the complete compatibility between libraries and application.

PATH_TO_CLIENT_ROOT=`pwd`/cpp-client

        git clone https://github.com/infinispan/cpp-client.git
        cd cpp-client && INFINISPAN_VERSION=9.0.0.Alpha2 ./build.sh DEBUG
        cd build && cpack -G RPM

2. Now build the example against the libraries (this is done by the HOTROD_CLIENT_PATH variable)

        cd ../.. && git clone https://github.com/rigazilla/cpp-client-examples.git
        cd cpp-client-examples && mkdir build
        cd build && cmake -DHOTROD_CLIENT_PATH=${PATH_TO_CLIENT_ROOT}/build/_CPack_Packages/DEB-x86_64/RPM/infinispan-hotrod-cpp-8.0.0-SNAPSHOT-DEB-x86_64/ ..
        cmake --build .

If you want to run the example

3. Run the Infinispan 9.x Server

        ${PATH_TO_CLIENT_ROOT}/infinispan-server-9.0.0.Alpha2/bin/standalone.sh > console.log 2>&1 &

4. Run the example

        ./remoteExec
