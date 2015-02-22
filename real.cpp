#ifdef CPU
#include "Cpu_miner.hpp"
#else
#include "Gpu_miner.hpp"
#endif

#include "sha256.hpp"

#include <boost/network.hpp>

extern "C"
{
#include <libblkmaker-0.1/blkmaker.h>
#include <libblkmaker-0.1/blkmaker_jansson.h>
}

namespace network = boost::network;
namespace http = network::http;

const char host[] = "http://127.0.0.1:8332/";
const char username[] = "aaa";
const char password[] = "aaa";

bool sha256_impl(void *hash, const void *msg, size_t len)
{
	SHA256 sha;
	sha.init();
	sha.update(reinterpret_cast<const unsigned char *>(msg), len);
	sha.final(reinterpret_cast<unsigned char *>(hash));
	return true;
}

http::client::response send2(json_t *request)
{
	char *body = json_dumps(request, 0);
	std::cout << "Sending request " << body << std::endl;

	http::client::request request2(host);
	request2 << network::header("Connecton", "close");
	request2 << network::body(body);

	http::client client;
	return client.post(request2);
}

void send(json_t *request)
{
	send2(request);
}

json_t *sendrec(json_t *request)
{
	http::client::response response = send2(request);
	std::string resp_body = http::body(response);
	std::cout << "Got response " << resp_body << std::endl;

	return json_loads(resp_body.c_str(), 0, nullptr);
}

int main()
{
	blkmk_sha256_impl = sha256_impl;

#ifdef CPU
	Cpu_miner miner;
#else
	Gpu_miner miner;
#endif

	while(true)
	{
		blktemplate_t *blktemplate = blktmpl_create();
		json_t *request = blktmpl_request_jansson(blktmpl_addcaps(blktemplate), nullptr);
		json_t *response;

		response = sendrec(request);

		json_decref(request);

		blktmpl_add_jansson(blktemplate, response, time(nullptr));
		json_decref(response);

		int nonce = 0;

		while(blkmk_time_left(blktemplate, time(nullptr)) && blkmk_work_left(blktemplate))
		{
			char data[80];
			unsigned int id;
			unsigned int size = blkmk_get_data(blktemplate, data, sizeof(data), time(nullptr), nullptr, &id);

			nonce = miner.mine(data, nonce, nonce + 100000000, 32);
			*reinterpret_cast<int *>(data + 76) = nonce;

			json_t *request = blkmk_submit_jansson(blktemplate, reinterpret_cast<const unsigned char *>(data), id, nonce);

			send(request);

			json_decref(request);
		}

		blktmpl_free(blktemplate);
	}
	return 0;
}
