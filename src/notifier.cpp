#include <asio/io_context.hpp>
#include <string>
#include <via/http/request.hpp>
#include <via/http/request_method.hpp>

#define ASIO_STANDALONE
#include "via/comms/tcp_adaptor.hpp"
#include "via/http_client.hpp"

/// Define an HTTP client using std::string to store message bodies
typedef via::http_client<via::comms::tcp_adaptor, std::string> http_client_type;
typedef http_client_type::http_response http_response;
typedef http_client_type::chunk_type http_chunk_type;

#include "config.hpp"

namespace notifier
{
	asio::io_context context;
	http_client_type::shared_pointer http_client = nullptr;

	/// The handler for incoming HTTP requests.
	/// Prints the response.
	void response_handler(http_response const& response,
							std::string const& body)
	{
		std::cout << "Rx response: " << response.to_string()
				<< response.headers().to_string();
		std::cout << "Rx body: "     << body << std::endl;

		if (!response.is_chunked())
		http_client->disconnect();
	}

	/// The handler for incoming HTTP chunks.
	/// Prints the chunk header and data to std::cout.
	void chunk_handler(http_chunk_type const& chunk, std::string const& data)
	{
		if (chunk.is_last())
		{
		std::cout << "Rx chunk is last, extension: " << chunk.extension()
					<< " trailers: " << chunk.trailers().to_string() << std::endl;
		http_client->disconnect();
		}
		else
		std::cout << "Rx chunk, size: " << chunk.size()
					<< " data: " << data << std::endl;
	}

	void init()
	{
		auto& c = config::get();

		if (!c.enable_notifier) return;

		http_client = http_client_type::create(context, response_handler, chunk_handler);
		if (!http_client->connect("localhost", c.notifier_host.data()))
		{
			return;
		}

		context.run();
		http_client.reset();
		http_client->connection()->set_keep_alive(true);
		http_client->connection()->set_timeout(c.notifier_connection_timeout_msec * 1000);
	}

	void notify_response(const std::string& data)
	{
		if (http_client == nullptr) return;

		via::http::tx_request req(via::http::request_method::POST, "/notify/response", "Content-Type: application/x-msgpack");
		auto res = http_client->send(req, data.data());

		if (res)
		{
			if (http_client->response().status() != 200)
			{
				std::cout << "Unexpected response from listener: " << http_client->response().status() << "\n";
			}
		}
		else
		{
			if (config::get().notifier_print_error)
			{
				std::cout << "Failed to notify listener\n";
			}
		}
	}
}
