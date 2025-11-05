// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "RelayClient.h"
#include "CavrnusConnectorModule.h"

#include <Sockets.h>
#include <SocketSubsystem.h>

namespace Cavrnus
{
	static const int BYTE_STREAM_MAGIC_NUMBER = 0x50392354;

	RelayClient::RelayClient(const FString& ServerAddressIn, int ServerPortIn)
		: ServerAddress(ServerAddressIn)
		, ServerPort(ServerPortIn)
		, ListenSocket(nullptr)
		, ClientSocket(nullptr)
	{
	}

	RelayClient::~RelayClient()
	{
	}

	bool RelayClient::CreateListenSocket()
	{
		// Test for valid port
		ISocketSubsystem* SocketSubSys = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

		TSharedPtr<FInternetAddr> InternetAddr = SocketSubSys->GetAddressFromString(ServerAddress);

		for (int CurrPort = ServerPort; CurrPort < ServerPort + 32; CurrPort++)
		{
			// Build does a Bind and Listen
			ListenSocket = SocketSubSys->CreateSocket(NAME_Stream, TEXT("RelayClient Listen Socket"));

			if (ListenSocket)
			{
				InternetAddr->SetPort(CurrPort);

				if (!ListenSocket->Bind(*InternetAddr))
				{
					ESocketErrors ErrorCode = SocketSubSys->GetLastErrorCode();
					UE_LOG(LogCavrnusConnector, Log, TEXT("[RelayClient::CreateListenSocket] Bind failed on Port %d. Error: %s"), CurrPort, SocketSubSys->GetSocketError(ErrorCode));
					continue;
				}

				if (!ListenSocket->Listen(16))
				{
					ESocketErrors ErrorCode = SocketSubSys->GetLastErrorCode();
					UE_LOG(LogCavrnusConnector, Log, TEXT("[RelayClient::CreateListenSocket] Listen failed on Port %d. Error: %s"), CurrPort, SocketSubSys->GetSocketError(ErrorCode));
					continue;
				}

				UE_LOG(LogCavrnusConnector, Log, TEXT("[RelayClient::CreateListenSocket] Socket listening at %s:%d"), *ServerAddress, CurrPort);

				ServerPort = CurrPort;
				return true;
			}
			else
			{
				ESocketErrors ErrorCode = SocketSubSys->GetLastErrorCode();
				UE_LOG(LogCavrnusConnector, Log, TEXT("[RelayClient::CreateListenSocket] CreateSocket failed on Port %d. Error: %s"), CurrPort, SocketSubSys->GetSocketError(ErrorCode));
			}
		}

		UE_LOG(LogCavrnusConnector, Error, TEXT("[RelayClient::Connect] No usable ports found."));
		return false;
	}

	bool RelayClient::Connect()
	{
		if (!ListenSocket)
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("[RelayClient::CreateClientSocket] ListenSocket needs to be created and listening,"));
			return false;
		}

		ISocketSubsystem* SocketSubSys = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

		ClientSocket = ListenSocket->Accept(TEXT("RelayClient Client Socket"));

		if (!ClientSocket)
		{
			ESocketErrors ErrorCode = SocketSubSys->GetLastErrorCode();
			UE_LOG(LogCavrnusConnector, Error, TEXT("[RelayClient::CreateListenSocket] Client Socket failed. Error: %s"), SocketSubSys->GetSocketError(ErrorCode));

			return false;
		}

		return true;
	}

	bool RelayClient::SendMessage(const ServerData::RelayClientMessageBatch& message)
	{
		if (!ClientSocket)
		{
			return false;
		}

		int MessageLength = message.ByteSizeLong();

		// first send the magic number
		const int magic_number = BYTE_STREAM_MAGIC_NUMBER;
		int bytesSent = 0;

		bool bSendResult = ClientSocket->Send((uint8*)&magic_number, sizeof(magic_number), bytesSent);
		if (!bSendResult || bytesSent != sizeof(magic_number))
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("[RelayClient::SendMessage()] Failed to send magic number"));
			return false;
		}

		// then send the size of the message
		bSendResult = ClientSocket->Send((uint8*)&MessageLength, sizeof(MessageLength), bytesSent);
		if (!bSendResult || bytesSent != sizeof(MessageLength))
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("[RelayClient::SendMessage()] Failed to send message size"));
			HandleSocketError();
			return false;
		}

		// now send the actual message over the socket
		std::vector<uint8> MessageBuffer(MessageLength);
		bool bSerializeResult = message.SerializeToArray(MessageBuffer.data(), MessageLength);

		bSendResult = ClientSocket->Send(MessageBuffer.data(), MessageLength, bytesSent);
		if (!bSendResult || bytesSent != MessageLength)
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("[RelayClient::SendMessage()] Failed to send message content"));
			HandleSocketError();
			return false;
		}

		return true;
	}

	bool RelayClient::ReceiveMessage(ServerData::RelayRemoteMessageBatch& message)
	{
		if (!ClientSocket)
		{
			return false;
		}

		int MagicNumber = 0;
		int MagicNumberBytesReceived = 0;

		bool bRecvResult = ClientSocket->Recv((uint8*)&MagicNumber, sizeof(MagicNumber), MagicNumberBytesReceived);
		if (!bRecvResult)
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("[RelayClient::ReceiveMessage] Error receiving MagicNumber data from socket."));
			return false;
		}
		else if (MagicNumberBytesReceived <= 0)
		{
			return false;
		}

		if (MagicNumber != BYTE_STREAM_MAGIC_NUMBER)
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("[RelayClient::ReceiveMessage] Error decoding message"));
			return false;
		}

		int MessageLength = 0;
		int MessageLengthBytesReceived = 0;

		if (!ClientSocket)
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("[RelayClient::ReceiveMessage] Error, ClientSocket is null."));
			return false;
		}
		bRecvResult = ClientSocket->Recv((uint8*)&MessageLength, sizeof(MessageLength), MessageLengthBytesReceived);
		if (!bRecvResult)
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("[RelayClient::ReceiveMessage] Error receiving MessageLength data from socket."));
			return false;
		}

		std::vector<uint8> MessageBuffer(MessageLength);

		int BytesReceived = 0;
		while (BytesReceived < MessageLength)
		{
			int CurrentBytesReceived = 0;
			if (!ClientSocket)
			{
				UE_LOG(LogCavrnusConnector, Error, TEXT("[RelayClient::ReceiveMessage] Error, ClientSocket is null."));
				return false;
			}
			bRecvResult = ClientSocket->Recv(MessageBuffer.data() + BytesReceived, MessageLength - BytesReceived, CurrentBytesReceived);
			if (!bRecvResult)
			{
				UE_LOG(LogCavrnusConnector, Error, TEXT("[RelayClient::ReceiveMessage] Error receiving message data from socket."));
				return false;
			}

			BytesReceived += CurrentBytesReceived;
		}

		// Parse the received message
		if (!message.ParseFromArray(MessageBuffer.data(), BytesReceived))
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("Failed to parse received data in RelayClient::ReceiveMessage()"));
			return false;
		}

		return true;
	}

	void RelayClient::Disconnect()
	{
		if (ClientSocket)
		{
			ClientSocket->Shutdown(ESocketShutdownMode::ReadWrite);
			ClientSocket->Close();
			ClientSocket = nullptr;
		}

		if (ListenSocket)
		{
			ListenSocket->Shutdown(ESocketShutdownMode::ReadWrite);
			ListenSocket->Close();
			ListenSocket = nullptr;
		}
	}

	void RelayClient::HandleSocketError()
	{
		ISocketSubsystem* SocketSubSys = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		ESocketErrors ErrorCode = SocketSubSys->GetLastErrorCode();
		UE_LOG(LogCavrnusConnector, Error, TEXT("[RelayClient::HandleSocketError] Client socket error, disconnecting. Error: %s"), SocketSubSys->GetSocketError(ErrorCode));

		Disconnect();
	}

	int RelayClient::GetServerPort() const
	{
		return ServerPort;
	}
} // namespace Cavrnus
