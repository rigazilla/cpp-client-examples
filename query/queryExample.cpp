#include "addressbook.pb.h"
#include "bank.pb.h"
#include <infinispan/hotrod/BasicTypesProtoStreamMarshaller.h>
#include <infinispan/hotrod/ProtoStreamMarshaller.h>
#include "infinispan/hotrod/ConfigurationBuilder.h"
#include "infinispan/hotrod/RemoteCacheManager.h"
#include "infinispan/hotrod/RemoteCache.h"
#include "infinispan/hotrod/Version.h"
#include "infinispan/hotrod/query.pb.h"
#include "infinispan/hotrod/QueryUtils.h"
#include <vector>
#include <tuple>

#define PROTOBUF_METADATA_CACHE_NAME "___protobuf_metadata"
#define ERRORS_KEY_SUFFIX  ".errors"

using namespace infinispan::hotrod;
using namespace org::infinispan::query::remote::client;

std::string read(std::string file)
{
	std::ifstream t(file);
	std::stringstream buffer;
	buffer << t.rdbuf();
	return buffer.str();
}

int main(int argc, char** argv) {
	std::cout << "Tests for Query" << std::endl;
    ConfigurationBuilder builder;
    builder.addServer().host(argc > 1 ? argv[1] : "127.0.0.1").port(argc > 2 ? atoi(argv[2]) : 11222).protocolVersion(Configuration::PROTOCOL_VERSION_24);
    builder.balancingStrategyProducer(nullptr);
    RemoteCacheManager cacheManager(builder.build(), false);
    cacheManager.start();
    //Create the Protobuf Metadata cache peer with a Protobuf marshaller
    auto *km = new BasicTypesProtoStreamMarshaller<std::string>();
    auto *vm = new BasicTypesProtoStreamMarshaller<std::string>();
    auto metadataCache = cacheManager.getCache<std::string, std::string>(
    		km, &Marshaller<std::string>::destroy, vm, &Marshaller<std::string>::destroy,PROTOBUF_METADATA_CACHE_NAME, false);
    // Install the data model into the Protobuf metadata cache
    metadataCache.put("sample_bank_account/bank.proto", read("proto/bank.proto"));
    if (metadataCache.containsKey(ERRORS_KEY_SUFFIX))
	{
      std::cerr << "fail: error in registering .proto model" << std::endl;
      return -1;
	}
    //Create the application cache peer with a Protobuf marshaller
    auto *testkm = new BasicTypesProtoStreamMarshaller<int>();
    auto *testvm = new ProtoStreamMarshaller<sample_bank_account::User>();
    auto testCache = cacheManager.getCache<int, sample_bank_account::User>(
    		testkm, &Marshaller<int>::destroy, testvm, &Marshaller<sample_bank_account::User>::destroy, "namedCache", false);

    //Fill the cache with the application data: two users Tom and Jerry
    testCache.clear();
    sample_bank_account::User_Address a;
    sample_bank_account::User user1;
    user1.set_id(3);
    user1.set_name("Tom");
    user1.set_surname("Cat");
    user1.set_gender(sample_bank_account::User_Gender_MALE);
    sample_bank_account::User_Address * addr= user1.add_addresses();
    addr->set_street("Via Roma");
    addr->set_number(3);
    addr->set_postcode("202020");
    testCache.put(3, user1);
    user1.set_id(4);
    user1.set_name("Jerry");
    user1.set_surname("Mouse");
    addr->set_street("Via Milano");
    user1.set_gender(sample_bank_account::User_Gender_MALE);
    testCache.put(4, user1);
    // Simple query to get User objects
	{
		QueryRequest qr;
		std::cout << "Query: from sample_bank_account.User" << std::endl;
		qr.set_jpqlstring("from sample_bank_account.User");
		QueryResponse resp = testCache.query(qr);
		std::vector<sample_bank_account::User> res;
		unwrapResults(resp, res);
		for (auto i = 0; i < res.size(); i++) {
			std::cout << "User(id=" << res[i].id() << ",name=" << res[i].name()
					<< ",surname=" << res[i].surname() << ")" << std::endl;
		}
	}
    // Simple query to get User objects with where condition
	{
		QueryRequest qr;
		std::cout << "from sample_bank_account.User u where u.addresses.street=\"Via Milano\"" << std::endl;
		qr.set_jpqlstring("from sample_bank_account.User u where u.addresses.street=\"Via Milano\"");
		QueryResponse resp = testCache.query(qr);
		std::vector<sample_bank_account::User> res;
		unwrapResults(resp, res);
		for (auto i = 0; i < res.size(); i++) {
			std::cout << "User(id=" << res[i].id() << ",name=" << res[i].name()
					<< ",surname=" << res[i].surname() << ")" << std::endl;
		}
	}
	// Simple query to get projection (name, surname)
	{
		QueryRequest qr;
		std::cout << "Query: select u.name, u.surname from sample_bank_account.User u" << std::endl;
		qr.set_jpqlstring(
				"select u.name, u.surname from sample_bank_account.User u");
		QueryResponse resp = testCache.query(qr);
        //Typed resultset
		std::vector<std::tuple<std::string, std::string> > prjRes;

		unwrapProjection(resp, prjRes);
		for (auto i = 0; i < prjRes.size(); i++) {
			std::cout << "Name: " << std::get<0> (prjRes[i])
							  << " Surname: " << std::get<1> (prjRes[i]) << std::endl;
		}
	}
	{
		QueryRequest qr;
		qr.set_jpqlstring("select count(u.id) from sample_bank_account.User u");
		QueryResponse resp = testCache.query(qr);
		int i = unwrapSingleResult<int>(resp);
		std::cout << "Result is: " << i << std::endl;
	}
	cacheManager.stop();
	return 0;
}

