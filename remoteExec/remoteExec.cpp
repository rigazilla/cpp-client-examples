#include "infinispan/hotrod/ConfigurationBuilder.h"
#include "infinispan/hotrod/RemoteCacheManager.h"
#include "infinispan/hotrod/RemoteCache.h"
#include "infinispan/hotrod/Version.h"

#include "infinispan/hotrod/JBasicMarshaller.h"


using namespace infinispan::hotrod;

int main(int argc, char** argv) {

        ConfigurationBuilder builder;
        builder.addServer().host(argc > 1 ? argv[1] : "127.0.0.1").port(argc > 2 ? atoi(argv[2]) : 11222).protocolVersion(Configuration::PROTOCOL_VERSION_24);
        builder.balancingStrategyProducer(nullptr);
        RemoteCacheManager cacheManager(builder.build(), false);
        auto *km = new JBasicMarshaller<std::string>();
        auto *vm = new JBasicMarshaller<std::string>();
        RemoteCache<std::string, std::string> cache = cacheManager.getCache<std::string, std::string>(km,
                &Marshaller<std::string>::destroy,
                vm,
                &Marshaller<std::string>::destroy, std::string("namedCache"));
        cacheManager.start();

        try
        {
            std::map<std::string,std::string> s;
            cache.put("privateValue","Counted Access Value");
            std::string getValueScript(
            			   "// mode=local,language=javascript\n "
            			   "var cache = cacheManager.getCache(\"namedCache\");\n "
            			   "var ct = cache.get(\"accessCounter\");\n "
            			   "var c = ct==null ? 0 : parseInt(ct);\n "
            			   "cache.put(\"accessCounter\",(++c).toString());\n "
            			   "cache.get(\"privateValue\") ");
            std::string getValueScriptName("getValue.js");

            std::string getAccessScript(
			               "// mode=local,language=javascript\n "
                           "var cache = cacheManager.getCache(\"namedCache\");\n "
                           "cache.get(\"accessCounter\")");
            std::string getAccessScriptName("getAccessCounter.js");

            RemoteCache<std::string, std::string> scriptCache=cacheManager.getCache<std::string,std::string>("___script_cache",false);


            std::string pGetValueScriptName=JBasicMarshaller<std::string>::addPreamble(getValueScriptName);
            std::string pGetValueScript=JBasicMarshaller<std::string>::addPreamble(getValueScript);
            scriptCache.put(pGetValueScriptName, pGetValueScript);

            std::string pGetAccessScriptName=JBasicMarshaller<std::string>::addPreamble(getAccessScriptName);
            std::string pGetAccessScript=JBasicMarshaller<std::string>::addPreamble(getAccessScript);
            scriptCache.put(pGetAccessScriptName, pGetAccessScript);


            std::vector<unsigned char> execValueResult = cache.execute(getValueScriptName,s);
            std::vector<unsigned char> execAccessResult = cache.execute(getAccessScriptName,s);

            std::string value(JBasicMarshallerHelper::unmarshall<std::string>((char*)execValueResult.data()));
            std::string access(JBasicMarshallerHelper::unmarshall<std::string>((char*)execAccessResult.data()));

            std::cout << "Returned value is '"<<value << "' and has been accessed: "<< access << " times."<< std::endl;

        } catch (const Exception& e) {
            std::cout << "is: " << typeid(e).name() << '\n';
            std::cerr << "fail unexpected exception: " << e.what() << std::endl;
            return 1;
        }

        cacheManager.stop();
    return 0;
}

