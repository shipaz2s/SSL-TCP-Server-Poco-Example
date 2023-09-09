#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/SSLManager.h>

#include <string>
#include <iostream>



void serverClientTest()
{
    try {
        Poco::Net::initializeSSL();
        // Socket server
        Poco::Net::Context::Ptr ptrContext =
                new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE,
                        "./cert4/myKey.pem",
                        "./cert4/myCert.pem",
                        "./cert4/myCert.pem",
                        Poco::Net::Context::VERIFY_ONCE);
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
                        "./cert4/myKey.pem",
                        "./cert4/myCert.pem",
                        "./cert4/myCert.pem",
                        Poco::Net::Context::VERIFY_ONCE);

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
        std::cout << "!!  EXCEPTION "<< ex.displayText() << std::endl;
    }
}


class EchoConnection: public Poco::Net::TCPServerConnection
{
public:
   EchoConnection(const Poco::Net::StreamSocket& s): Poco::Net::TCPServerConnection(s){}

   void run()
   {
       Poco::Net::StreamSocket& ss = socket();
       std::cout << "connection from client: " << ss.address() << std::endl;
       try
       {
            // ...
       }
       catch (Poco::Exception& exc)
       {
           std::cerr << "--------------- EchoConnection: " << exc.displayText() << std::endl;
       }
   }
};


int main()
{
	std::cout << "start\n";
	Poco::Net::TCPServer srv(new Poco::Net::TCPServerConnectionFactoryImpl<EchoConnection>());
	srv.start();

	Poco::Net::SocketAddress sa("localhost", srv.socket().address().port());
	Poco::Net::StreamSocket ss(sa);
	std::string data("hello, world");
	ss.sendBytes(data.data(), data.size());
	char buffer[256] = {0};
	int n = ss.receiveBytes(buffer, sizeof(buffer));
	std::cout << std::string(buffer, n) << std::endl;

	data = "hello, world 2";
	ss.sendBytes(data.data(), data.size());
	n = ss.receiveBytes(buffer, sizeof(buffer));
	std::cout << std::string(buffer, n) << std::endl;

	srv.stop();

	return 0;
}