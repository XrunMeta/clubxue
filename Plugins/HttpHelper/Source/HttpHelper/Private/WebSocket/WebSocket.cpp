

#include "WebSocket/WebSocket.h"

UWebSocket* UWebSocket::AsyncWebSocket(UObject* InWorldContextObject, const FWebSocketOptions& Options, UWebSocketHandler* & Result)
{
	UWebSocket* Node = NewObject<UWebSocket>();
	Node->Options = Options;
	Node->WebSocket = UWebSocketHandler::CreateWebSocket(InWorldContextObject);
	Result = Node->WebSocket;
	return Node;
}

void UWebSocket::Activate()
{
	if (Active)
	{
		FFrame::KismetExecutionMessage(TEXT("WebSocket is already running, close websocket and restart to update options"), ELogVerbosity::Warning);
		return;
	}

	WebSocket->OnConnected.AddUniqueDynamic(this, &UWebSocket::OnWebSocketConnected);

	WebSocket->OnClosed.AddUniqueDynamic(this, &UWebSocket::OnWebSocketClosed);

	WebSocket->OnConnectionError.AddUniqueDynamic(this, &UWebSocket::OnWebSocketConnectionError);

	WebSocket->OnTextMessage.AddUniqueDynamic(this, &UWebSocket::OnWebSocketTextMessage);

	WebSocket->OnBinaryMessage.AddUniqueDynamic(this, &UWebSocket::OnWebSocketBinaryMessage);

	WebSocket->OnConnectionRetry.AddUniqueDynamic(this, &UWebSocket::OnWebSocketRetry);

	Active = WebSocket->Open(Options);
}

void UWebSocket::OnWebSocketConnected()
{
	if (OnConnected.IsBound())
	{
		FWebSocketResult Result;
		OnConnected.Broadcast(Result);
	}

	AddToRoot();
}

void UWebSocket::OnWebSocketClosed(const int32& Code, const FString& Reason, bool ClosedByPeer)
{
	Active = false;
	if (OnClosed.IsBound())
	{
		FWebSocketResult Result;
		Result.Code = Code;
		Result.ClosedReason = Reason;
		Result.ClosedByPeer = ClosedByPeer;
		OnClosed.Broadcast(Result);
	}

	RemoveFromRoot();
}

void UWebSocket::OnWebSocketConnectionError(const FString& Reason)
{
	if (Options.ReconnectAmount <= 0 || Options.ReconnectTimeout <= 0)
	{
		Active = false;
		RemoveFromRoot();
	}

	if (OnError.IsBound())
	{
		FWebSocketResult Result;
		Result.Error = Reason;
		Result.ErrorReason = EWebSocketError::Connection;
		OnError.Broadcast(Result);
	}
}

void UWebSocket::OnWebSocketTextMessage(const FString& Message)
{
	if (OnTextMessage.IsBound())
	{
		FWebSocketResult Result;
		Result.TextMessage = Message;
		OnTextMessage.Broadcast(Result);
	}
}

void UWebSocket::OnWebSocketBinaryMessage(const TArray<uint8>& Message)
{
	if (OnBinaryMessage.IsBound())
	{
		FWebSocketResult Result;
		Result.BytesMessage = Message;
		OnBinaryMessage.Broadcast(Result);
	}
}

void UWebSocket::OnWebSocketRetry(const int32& RetryCount)
{
	if (RetryCount >= Options.ReconnectAmount)
	{
		Active = false;
		RemoveFromRoot();
	}

	if (OnRetry.IsBound())
	{
		FWebSocketResult Result;
		OnRetry.Broadcast(Result);
	}
}