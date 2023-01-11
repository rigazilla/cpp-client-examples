#include "infinispan/hotrod/ConfigurationBuilder.h"
#include "infinispan/hotrod/RemoteCacheManager.h"
#include "infinispan/hotrod/RemoteCacheManagerAdmin.h"
#include "infinispan/hotrod/RemoteCache.h"
#include "infinispan/hotrod/Version.h"
#include "infinispan/hotrod/JBasicMarshaller.h"
using namespace infinispan::hotrod;
std::string cacheConf = " <infinispan><cache-container>     <local-cache name=\"namedCache\">\
         <encoding>\
		 <key media-type=\"text/plain\"/>\
		 <value media-type=\"text/plain\"/>\
         </encoding>\
      </local-cache></cache-container></infinispan>";
int main(int argc, char **argv)
{
	ConfigurationBuilder builder;
	builder.addServer().host("127.0.0.1").port(11222).protocolVersion(Configuration::PROTOCOL_VERSION_28);
	RemoteCacheManager cacheManager(builder.build(), false);
	try
	{
		cacheManager.start();
		// Create the cache
		// and wrap it with data format text/plain
		RemoteCache<std::string, std::string> cache0 = cacheManager.administration()->getOrCreateCacheWithXml<std::string, std::string>("namedCache",cacheConf);
		DataFormat<std::string, std::string> df;
		df.keyMediaType.typeSubtype = std::string("text/plain");
		df.valueMediaType.typeSubtype = std::string("text/plain");
		RemoteCache<std::string, std::string> cache = cache0.withDataFormat(&df);

		// Get the ___script_cache
		// and wrap it with data format text/plain
		RemoteCache<std::string, std::string> scriptCache0 = cacheManager.getCache<
			std::string, std::string>(std::string("___script_cache"));
		DataFormat<std::string, std::string> dfs;
		dfs.keyMediaType.typeSubtype = std::string("text/plain");
		dfs.valueMediaType.typeSubtype = std::string("text/plain");
		RemoteCache<std::string, std::string> scriptCache = scriptCache0.withDataFormat(&dfs);
		// Install on the server the getValue script
		std::string getValueScript(
			"// mode=local,language=javascript\n "
			"var cache = cacheManager.getCache(\"namedCache\");\n "
			"var ct = cache.get(\"accessCounter\");\n "
			"var c = ct==null ? 0 : parseInt(ct);\n "
			"cache.put(\"accessCounter\",(++c).toString());\n "
			"cache.get(\"privateValue\")");
		std::string getValueScriptName("getValue.js");
		scriptCache.put(getValueScriptName, getValueScript);
		// Install on the server the get access counter script
		std::string getAccessScript(
			"// mode=local,language=javascript\n "
			"var cache = cacheManager.getCache(\"namedCache\");\n "
			"cache.get(\"accessCounter\")");
		std::string getAccessScriptName("getAccessCounter.js");
		scriptCache.put(getAccessScriptName, getAccessScript);
		// Set a value for the privateValue entry
		cache.put("privateValue", "Counted Access Value");
		// Execute JS
		std::map<std::string, std::string> s;
		std::vector<unsigned char> execValueResult = cache.execute(
			getValueScriptName, s);
		std::vector<unsigned char> execAccessResult = cache.execute(
			getAccessScriptName, s);
		// Log the results
		std::string s1(reinterpret_cast<char *>(&execValueResult[0]), execValueResult.size());
		std::string s2(reinterpret_cast<char *>(&execAccessResult[0]), execAccessResult.size());
		std::cout << "Returned value is '" << s1
				  << "' and has been accessed: " << s2 << " times."
				  << std::endl;
	}
	catch (const Exception &e)
	{
		std::cout << "is: " << typeid(e).name() << '\n';
		std::cerr << "fail unexpected exception: " << e.what() << std::endl;
		return 1;
	}
	cacheManager.stop();
	return 0;
}
