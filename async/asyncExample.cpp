#include "infinispan/hotrod/ConfigurationBuilder.h"
#include "infinispan/hotrod/RemoteCacheManager.h"
#include "infinispan/hotrod/RemoteCache.h"
#include "infinispan/hotrod/Version.h"

#include "infinispan/hotrod/JBasicMarshaller.h"
#include <iostream>
#include <thread>
#include <future>

using namespace infinispan::hotrod;

int main(int argc, char** argv) {
  ConfigurationBuilder builder;
  builder.addServer().host(argc > 1 ? argv[1] : "127.0.0.1").port(argc > 2 ? atoi(argv[2]) : 11222).protocolVersion(Configuration::PROTOCOL_VERSION_24);
  builder.balancingStrategyProducer(nullptr);
  RemoteCacheManager cacheManager(builder.build(), false);
  auto *km = new BasicMarshaller<std::string>();
  auto *vm = new BasicMarshaller<std::string>();
  auto cache = cacheManager.getCache<std::string, std::string>(km,&Marshaller<std::string>::destroy,
                                                               vm, &Marshaller<std::string>::destroy);
  cacheManager.start();
  std::string ak1("asyncK1");
  std::string av1("asyncV1");
  std::string ak2("asyncK2");
  std::string av2("asyncV2");
  cache.clear();

  // Put ak1,av1 in async thread
  std::future<std::string*> future_put= cache.putAsync(ak1,av1);
  // Get the value in this thread
  std::string* arv1= cache.get(ak1);
    
  // Now wait for put completion
  future_put.wait();

  // All is synch now
  std::string* arv11= cache.get(ak1);
  if (!arv11 || arv11->compare(av1))
  {
    std::cout << "fail: expected " << av1 << "got " << (arv11 ? *arv11 : "null") << std::endl;
    return 1;
  }

  // Read ak1 again, but in async way and test that the result is the same
  std::future<std::string*> future_ga= cache.getAsync(ak1);
  std::string* arv2= future_ga.get();
  if (!arv2 || arv2->compare(av1))
  {
    std::cerr << "fail: expected " << av1 << " got " << (arv2 ? *arv2 : "null") << std::endl;
    return 1;
  }

  // Now user pass a simple lambda func that set a flag to true when the put completes
  bool flag=false;
  std::future<std::string*> future_put1= cache.putAsync(ak2,av2,0,0,[&] (std::string *v){flag=true; return v;});
  // The put is not completed here so flag must be false
  if (flag)
  {
    std::cerr << "fail: expected false got true" << std::endl;
    return 1;
  }
  // Now wait for put completion
  future_put1.wait();
  // The user lambda must be executed so flag must be true
  if (!flag)
  {
    std::cerr << "fail: expected true got false" << std::endl;
    return 1;
  }

  // Same test for get
  flag=false;
  // Now user pass a simple lambda func that set a flag to true when the put completes
  std::future<std::string*> future_get1= cache.getAsync(ak2,[&] (std::string *v){flag=true; return v;});
  // The get is not completed here so flag must be false
  if (flag)
  {
    std::cerr << "fail: expected false got true" << std::endl;
    return 1;
  }
  // Now wait for get completion
  future_get1.wait();
  if (!flag)
  {
    std::cerr << "fail: expected true got false" << std::endl;
    return 1;
  }
  std::string* arv3= future_get1.get();
  if (!arv3 || arv3->compare(av2))
  {
    std::cerr << "fail: expected " << av2 << " got " << (arv3 ? *arv3 : "null") << std::endl;
    return 1;
  }
  cacheManager.stop();
}

