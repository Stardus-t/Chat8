#pragma once
#include "const.h"
#include <map>
#include <iostream>
#include <unordered_map>

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
	friend class LogicSystem;
public:
	HttpConnection(tcp::socket socket);
	void Start();
	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;

private:
	void CheckDeadline();
	void WriteResponse();
	void HandleReq();
	void PreParseGetParam();
	tcp::socket  _socket;
	// The buffer for performing reads.
	beast::flat_buffer  _buffer{ 8192 };

	// The request message.
	http::request<http::dynamic_body> _request;

	// The response message.
	http::response<http::dynamic_body> _response;

	// The timer for putting a deadline on connection proc+essing.
	net::steady_timer deadline_{
		_socket.get_executor(), std::chrono::seconds(60) };
};

