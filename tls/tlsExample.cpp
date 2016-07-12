#include "infinispan/hotrod/ConfigurationBuilder.h"
#include "infinispan/hotrod/RemoteCacheManager.h"
#include "infinispan/hotrod/RemoteCache.h"
#include "infinispan/hotrod/Version.h"

#include "infinispan/hotrod/JBasicMarshaller.h"
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <typeinfo>


using namespace infinispan::hotrod;

int main(int argc, char** argv) {
    std::cout << "TLS Test" << std::endl;
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " server_ca_file [client_ca_file]" << std::endl;
        return 1;
    }
    {
      ConfigurationBuilder builder;
      builder.addServer().host("127.0.0.1").port(11222).protocolVersion(Configuration::PROTOCOL_VERSION_24);
      // Enable the TLS layer and install the server public key
      // this ensure that the server is authenticated
      builder.ssl().enable().serverCAFile(argv[1]);
      if (argc > 2) {
          // Send a client certificate for authentication (optional)
          // without this the socket will only be encrypted
          std::cout << "Using supplied client certificate for authentication against the server" << std::endl;
          builder.ssl().clientCertificateFile(argv[2]);
      }
      // That's all. Now do business as usual
      RemoteCacheManager cacheManager(builder.build(), false);
      BasicMarshaller<std::string> *km = new BasicMarshaller<std::string>();
      BasicMarshaller<std::string> *vm = new BasicMarshaller<std::string>();
      RemoteCache<std::string, std::string> cache = cacheManager.getCache<std::string, std::string>(km,
          &Marshaller<std::string>::destroy, vm, &Marshaller<std::string>::destroy);
      cacheManager.start();
      cache.clear();
      std::string k1("key13");
      std::string v1("boron");

      cache.put(k1, v1);
      std::unique_ptr<std::string> rv(cache.get(k1));
      if (rv->compare(v1)) {
          std::cerr << "get/put fail for " << k1 << " got " << *rv << " expected " << v1 << std::endl;
          return 1;
      }
      cacheManager.stop();
    }
    return 0;
}

