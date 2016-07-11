#C++ classes generation from the proto files

Just run those commands:

1. protoc --cpp_out=. addressbook.proto
2. protoc --cpp_out=. bank.proto

the C++ model is generated in the current directory.

This is the basic step, feel free to integrate this in you build process. 
