#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/SecureServerSocket.h>
#include <Poco/Net/SecureStreamSocket.h>

#include <string>
#include <iostream>



class EchoConnection: public Poco::Net::TCPServerConnection
{
public:
	EchoConnection(const Poco::Net::StreamSocket& s): Poco::Net::TCPServerConnection(s){}

	void run()
	{
		Poco::Net::StreamSocket& ss = socket();
		Poco::Net::SecureStreamSocket securedSocket = (Poco::Net::SecureStreamSocket)dynamic_cast<const Poco::Net::StreamSocket&>(ss);
		// securedSocket.completeHandshake();

		std::cout << "connection from client: " << ss.address() << std::endl;
		try
		{
			char buffer[1024];
			memset(buffer, '\0', 1024);
			int n = securedSocket.receiveBytes(buffer, sizeof(buffer));

			securedSocket.sendBytes(buffer, n);
		}
		catch (Poco::Exception& exc)
		{
			std::cerr << "--------------- EchoConnection: " << exc.displayText() << std::endl;
		}
	}
};

void serverClientTest()
{
	try {
		Poco::Net::initializeSSL();
		// Socket server
		Poco::Net::Context::Ptr ptrContext =
				new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE,
						"keys/server_strict/key.key",
						"keys/server_strict/cert.crt",
						"keys/root/rootCA.pem",
						Poco::Net::Context::VERIFY_STRICT,
						9,
						false,
						"ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
		Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> ptrCert = new Poco::Net::AcceptCertificateHandler(true);

		Poco::SharedPtr<Poco::Net::PrivateKeyPassphraseHandler> ptrPrivateKeyPassphraseHandler;
		ptrPrivateKeyPassphraseHandler = new Poco::Net::KeyConsoleHandler(true);

		Poco::Net::SSLManager::instance().initializeServer(ptrPrivateKeyPassphraseHandler, ptrCert, ptrContext);

		Poco::Net::SocketAddress serverAddress("0.0.0.0", 8085);
		Poco::Net::SecureServerSocket serverSecureSocket(serverAddress);
		Poco::Net::TCPServer srv(new Poco::Net::TCPServerConnectionFactoryImpl<EchoConnection>(), serverSecureSocket);
		srv.start();
	
		Poco::Net::Context::Ptr ptrContext2 =
				new Poco::Net::Context(Poco::Net::Context::TLS_CLIENT_USE,
						"keys/client_strict/key.key",
						"keys/client_strict/cert.crt",
						"keys/root/rootCA.pem",
						Poco::Net::Context::VERIFY_STRICT,
						9,
						false,
						"ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");

		Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> ptrCert2 = new Poco::Net::AcceptCertificateHandler(true);

		Poco::SharedPtr<Poco::Net::PrivateKeyPassphraseHandler>ptrPrivateKeyPassphraseHandler2(new Poco::Net::KeyConsoleHandler(false));

		Poco::Net::SSLManager::instance().initializeClient(ptrPrivateKeyPassphraseHandler2, ptrCert2, ptrContext2);

		Poco::Net::SocketAddress sa("127.0.0.1", 8085);
		Poco::Net::SecureStreamSocket ss1(sa);
		std::string data("TEST  TEST");
		int retSend = ss1.sendBytes(data.data(), (int) data.size());
		if (retSend>0)
		{
			std::cout << "buffer -> : " << data.data() << std::endl;
			char buffer[1024];
			memset(buffer, '\0', 1024);
			int retRecv = ss1.receiveBytes(buffer, sizeof(buffer));
			if (retRecv > 0)
			{
				std::cout << "buffer <- : " << buffer << std::endl;
			}
			else
			{
				std::cout << "ERROR: recv " << retRecv << std::endl;
			}
		}
		ss1.close();
	}
	catch (Poco::Exception& ex)
	{
		std::cout << "!! EXCEPTION " << ex.displayText() << std::endl;
	}
}

int main()
{
	serverClientTest();

	return 0;
}