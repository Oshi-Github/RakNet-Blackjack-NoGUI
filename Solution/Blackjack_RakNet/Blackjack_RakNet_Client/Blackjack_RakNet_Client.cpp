// Blackjack_RakNet_Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <string>
#include <time.h>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Card.h"
#include "Deck.h"
#include "Hand.h"
#include "Player.h"

// includes raknet header files
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

// adds library files for RakNet
#pragma comment(lib, "LibStatic.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;

enum Program_Messages
{
	ID_PLAYERNAME = ID_USER_PACKET_ENUM + 1,
	ID_PLAYER_ID,
	ID_PLAYER_READY_TO_START,
	ID_GAME_STARTED,
	ID_PLAYER_CARDS,
	ID_PLAYER_TURN,
	ID_PLAYER_DRAW_CARD,
	ID_CARD_DRAWN,
	ID_PLAYER_SWAP_CARD,
	ID_PLAYER_SWAPPED_CARD,
	ID_PLAYER_CARD_DISCARD,
	ID_PLAYER_DISCARDED_CARD,
	ID_NEW_TOP_CARD,
	ID_PLAYER_TURN_FINISHED,
	ID_PLAYER_KNOCKED,
	ID_SHOWDOWN_RESULT,
	ID_UPDATE_PLAYER,
	ID_PLAYER_JOINED_GAME,
	ID_PLAYER_LEFT_GAME
};

RakNet::RakPeerInterface *peerInterface;
RakNet::SystemAddress serverAddress;

RakNet::RakString playerName;
int playerID;
int playerScore;
int playerBet;
int turnCount;
int wins, losses, draws;
bool insurance;
Hand hand = Hand();
int handScore;
vector<Player> Players;

bool gameInProgress = false;
bool readyToStart = false;
bool myTurn = false;

void getPlayerName()
{
	cout << endl << "Please enter your name (16 characters)> ";
	char name[16];
	cin.getline(name, sizeof(name));
	playerName = name;
	cout << endl;
}

void displayHostAddresses()
{
	cout << "IP Addresses assigned to this host are: " << endl;
	for (unsigned int count = 0; count < peerInterface->GetNumberOfAddresses(); count++)
	{
		cout << "/t" << peerInterface->GetLocalIP(count) << endl;
	}
}

void initiliasepeerInterface()
{
	peerInterface = RakNet::RakPeerInterface::GetInstance();
	if (peerInterface == nullptr)
	{
		cerr << "Failed to initliase peer interface" << endl;
		exit(-1);
	}
}

void connectToServer()
{
	RakNet::SocketDescriptor socketDescriptor;
	peerInterface->Startup(1, &socketDescriptor, 1);
	cout << endl << "Enter IP Adress of host computer to connect to> ";
	char ipAddress[32];
	ZeroMemory(ipAddress, sizeof(ipAddress));
	cin.getline(ipAddress, 32);
	unsigned short serverPort;
	cout << endl << "Enter the port number to connect to> ";
	cin >> serverPort;
	cout << endl << "Joining game on " << ipAddress << ":" << serverPort << endl;
	RakNet::RakString rakStringServerAddress = ipAddress;
	rakStringServerAddress += "¦";
	rakStringServerAddress += serverPort;
	serverAddress.FromString(ipAddress);
	peerInterface->Connect(ipAddress, serverPort, 0, 0);
}

void sendPlayerName()
{
	RakNet::BitStream bitStreamPlayerName;
	bitStreamPlayerName.Write((RakNet::MessageID) ID_PLAYERNAME);
	bitStreamPlayerName.Write(playerName);
	peerInterface->Send(&bitStreamPlayerName, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
}

void SetServerAddr(RakNet::SystemAddress &addr)
{
	serverAddress = addr;
}

/*void ExtractPlayerCards(RakNet::Packet *packet)
{
	RakNet::BitStream bsCards(packet->data, packet->length, false);

	bsCards.IgnoreBytes(1);

	int numCards;

	bsCards.Read(numCards);

	hand.ClearHand();

	for (int cardIdx = 0; cardIdx < numCards; cardIdx++)
	{
		RakNet::RakString card;
		bsCards.Read(card);
		char type = (card.SubStr(0, 1)).C_String;
		char suit = (card.SubStr(1, 1)).C_String;

		hand.AddCard(Card(type, suit));
	}
	handScore = hand.GetValue();
	bsCards.Read(handScore);
}*/

void SetPlayerID(RakNet::Packet *packet)
{
	RakNet::BitStream bsPlayerID(packet->data, packet->length, false);
	bsPlayerID.IgnoreBytes(1);
	bsPlayerID.Read(playerID);
}

void SendReady()
{
	RakNet::BitStream bsReady;
	bsReady.Write((RakNet::MessageID) ID_PLAYER_READY_TO_START);
	bsReady.Write(playerName);
	peerInterface->Send(&bsReady, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
}

void UpdatePlayers(RakNet::Packet *packet)
{
	RakNet::BitStream bsPlayers(packet->data, packet->length, false);
	bsPlayers.IgnoreBytes(1);
	int numPlayers;
	bsPlayers.Read(numPlayers);
	Players.clear();
	for (int count = 0; count < numPlayers; count++)
	{
		Player newPlayer;
		RakNet::RakString name;
		int points;
		bsPlayers.Read(name);
		bsPlayers.Read(points);
		newPlayer.SetName(name);
		newPlayer.SetScore(points);
		Players.push_back(newPlayer);
		cout << newPlayer.GetName() << " has joined the game" << endl;
	}
}


void PlayGame() 
{
	while (true) {
		// Define main receiving loop for RakNet.  Note that more than data is processed by this loop
		for (RakNet::Packet *packet = peerInterface->Receive(); packet; peerInterface->DeallocatePacket(packet), packet = peerInterface->Receive())
		{
			// Determine which message has been received
			// by inspecting the first byte of the packet
			switch (packet->data[0])
			{
				// Notification that (this) client has connected
			case ID_CONNECTION_REQUEST_ACCEPTED:
				SetServerAddr(packet->systemAddress);
				sendPlayerName();
				break;

				// Notification reached maximum number of connections
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				cout << "Server is full" << endl;
				break;

				// Notification other system has disconnected.  That is if this is the client then server disconnected
			case ID_DISCONNECTION_NOTIFICATION:
				cout << packet->systemAddress.ToString() << " disconnected" << endl;
				break;

				// Same as above but abnormal disconnection
			case ID_CONNECTION_LOST:
				cout << packet->systemAddress.ToString() << " disconnected abnormally" << endl;
				break;

				// Receive message from server that a player id has been assigned
				// to this client
			case ID_PLAYER_ID:
				SetPlayerID(packet);
				break;

				// Receive message from server with player's initial or new cards
			case ID_PLAYER_CARDS:
				//ExtractPlayerCards(packet);
				break;

				// Receive message from server that game has been started
				// all players have indicated they are ready to start
			case ID_GAME_STARTED:
				gameInProgress = true;
				break;

				//[1] Receive this message from server every time turn changes
			case ID_PLAYER_TURN:
				//DisplayCards();
				//CheckIsMyTurn(packet);
				break;

				// Receive this message when a card has been drawn
				// TODO: Display card and ask if player wants to swap or discard it
			case ID_CARD_DRAWN:
				break;

				//[7] Receive this message when the player swapped the card with the
				// one on the top of the deck
			case ID_PLAYER_SWAPPED_CARD:
				//ExtractPlayerCards(packet);
				break;

				// Receive this message from server indicating the card drawn
				// was discarded.
			case ID_PLAYER_DISCARDED_CARD:
				break;

				// Receive from server when the top card has been swapped or drawn
			case ID_NEW_TOP_CARD:
				
				break;

				// Show the result of the showdown
			case ID_SHOWDOWN_RESULT:
				break;

				// Update player.  For future expansion
			case ID_UPDATE_PLAYER:
				break;

				// Player joined message.
			case ID_PLAYER_JOINED_GAME:
				UpdatePlayers(packet);
				break;

				// Display any other received message
			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}

			// Add the following standard code to let the main thread yield
			// and let RakNet threads to do some work
#ifdef _WIN32
			Sleep(30);
#else
			usleep(30 * 1000);
#endif
		}

		// Display prompt to press ENTER to indicate ready to play
		if (!gameInProgress && !readyToStart) {
			cout << "Press ENTER to indicate ready to start game" << '\r';
		}

		// If a key has been pressed send ready message
		if (_kbhit() && !readyToStart) {
			// Get line to clear input buffer of ENTER
			char temp[32];

			cin.getline(temp, 32);

			cin.ignore(INT_MAX, '\n');

			SendReady();
			readyToStart = true;
			cout << "Waiting for other players to start..." << endl;
		}

		// Is it my turn?
		if (myTurn) {
			//h
		}
	}
}

int main()
{

	initiliasepeerInterface();

	displayHostAddresses();

	getPlayerName();

	connectToServer();

	PlayGame();

	cout << "Press any key to exit" << endl;

	cin.get();

#pragma region old
	/*// sets up the RakPeerInterface object for managing communications
	RakNet::RakPeerInterface *peer = RakNet::RakPeerInterface::GetInstance();

	// sets up a variable to recieve packet objects
	RakNet::Packet *packet;

	// sets up a system address to hold the server IP
	RakNet::SystemAddress serverIP;

	// setup connection to connect to host
	RakNet::SocketDescriptor socketDescriptor;
	peer->Startup(1, &socketDescriptor, 1);

	// Prompt user to input the ip address to connect to
	cout << "Enter IP Address of host computer to connect to> " << endl;

	char ipAddress[32];
	ZeroMemory(ipAddress, sizeof(ipAddress));

	cin.getline(ipAddress, 32);

	cout << "Connecting to " << ipAddress << endl;

	RakNet::RakString rsipAddress;

	rsipAddress += ipAddress;
	rsipAddress += "¦";
	rsipAddress += PORT_NUM;

	// sets the server IP
	serverIP.FromString(rsipAddress.C_String());

	peer->Connect(ipAddress, PORT_NUM, 0, 0);

	char message[512];

	if (_kbhit())
	{
		RakNet::RakString rs;
		cin.getline(message, 512);
		rs += message;
		RakNet::BitStream bsOut;
		bsOut.Write((RakNet::MessageID) ID_MESSAGE);
		bsOut.Write(rs);
		peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverIP, true);
	}

	cin.ignore();
	*/
#pragma endregion old


    return 0;
}

