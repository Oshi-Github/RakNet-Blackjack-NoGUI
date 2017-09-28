#include <iostream>
#include <string>
#include <time.h>

#include "Player.h"
#include "Card.h"
#include "Deck.h"
#include <vector>

// includes raknet header files
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

// adds the library files for RakNet
#pragma comment(lib, "LibStatic.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Defines the maximum number of clients
const unsigned int MAX_PLAYERS = 8;

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
	ID_PLAYER_LEFT_GAME,
	ID_SEND_CURRENT_PLAYERS,
	ID_SEND_CARDS_STRING,
	ID_SEND_SCORE_STATS,
	ID_SEND_DEALER_FIRST,
	ID_SEND_DEALER_ALL,
	ID_SEND_ONE_PLAYER_CARDS,
	ID_SEND_HIT_STAND,
	ID_SEND_STRING,
	ID_SEND_HIT_STAND_TO_SERVER,
	ID_SEND_TURN_COUNT
};

RakNet::RakPeerInterface *peerInterface;
RakNet::SystemAddress remoteIP;

size_t decksSize = 5;
vector<Deck> decks(decksSize);
int activeDeck = 0;

Player dealer = Player();
bool showCards = false;

const int betScore = 10;

unsigned int numPlayersReadyToStart = 0;

vector<Player> players;
int playerCount = 0;

bool hitStandRecieved = false;
char hitStandResponse;

int lastPlayerID = 0;
int turn = 0;

bool GameInProgress = false;

int currentPlayer;
int turnCount;
bool DoneEnd = false;

bool validInput;

bool readyToStart = false;

bool turnTaken = false;

void SetupGame();
void StartGame();
void GameLoop();
void CheckWins();
void ClearHands();
void DrawForAll();
void DrawPlayer(Player &player);
bool CheckBust(Player player);
int ValidInput_int(int min, int max);
string ValidInput_string();
char ValidInput_char(char valid[]);


void displayHostIPAddresses()
{
	cout << "IP addresses assigned to this host are: " << endl;

	for (unsigned int count = 0; count < peerInterface->GetNumberOfAddresses(); count++)
	{
		cout << peerInterface->GetLocalIP(count) << endl;
	}
}

void initialisePeerInterface()
{
	peerInterface = RakNet::RakPeerInterface::GetInstance();

	if (peerInterface == nullptr)
	{
		cerr << "Failed to start peer interface, ending program" << endl;
		exit(-1);
	}
}

unsigned int getPortNum()
{
	cout << "Enter port number to listen for connections on> ";

	unsigned int PortNum;

	cin >> PortNum;

	return PortNum;
}

void listenForConnections(unsigned int portNum)
{
	RakNet::SocketDescriptor socketDescriptor(portNum, 0);
	peerInterface->Startup(MAX_PLAYERS, &socketDescriptor, 1);

	peerInterface->SetMaximumIncomingConnections(MAX_PLAYERS);

	cout << "Waiting for connections..." << endl;
	cout << "\n" << endl;
}

void SendPlayerID(int ID, RakNet::SystemAddress IPAddr)
{
	RakNet::BitStream bsPlayerID;

	bsPlayerID.Write((RakNet::MessageID) ID_PLAYER_ID);
	bsPlayerID.Write(ID);

	peerInterface->Send(&bsPlayerID, HIGH_PRIORITY, RELIABLE_ORDERED, 0, IPAddr, false);
}

void CreateNewPlayer(RakNet::SystemAddress &clientIP)
{
	Player newPlayer;

	newPlayer.clientIP = clientIP;

	newPlayer.playerID = lastPlayerID;

	lastPlayerID++;

	players.push_back(newPlayer);

	SendPlayerID(newPlayer.playerID, clientIP);

	cout << "New player created ID = " << newPlayer.playerID << endl;
	cout << "\n" << endl;
}

int findPlayerIdxForIPAddr(RakNet::SystemAddress &addr)
{
	vector<Player>::size_type idx = 0;
	while (idx < players.size())
	{
		if (players[idx].clientIP == addr)
		{
			break;
		}
		idx++;
	}
	return idx;
}

void SendPlayerNamesToClients()
{
	RakNet::BitStream bsPlayers;

	bsPlayers.Write((RakNet::MessageID) ID_PLAYER_JOINED_GAME);

	bsPlayers.Write(players.size());

	for (vector<Player>::size_type count = 0; count < players.size(); count++)
	{
		bsPlayers.Write(players[count].GetName());
		bsPlayers.Write(players[count].GetScore());
	}

	for (vector<Player>::size_type idx = 0; idx < players.size(); idx++)
	{
		peerInterface->Send(&bsPlayers, HIGH_PRIORITY, RELIABLE_ORDERED, 0, players[idx].clientIP, false);
	}
}

void UpdatePlayerName(RakNet::Packet *packet)
{
	vector<Player>::size_type playerIdx = findPlayerIdxForIPAddr(packet->systemAddress);

	if (playerIdx < players.size())
	{
		RakNet::BitStream bsPlayerName(packet->data, packet->length, false);
		RakNet::RakString playerName;

		bsPlayerName.IgnoreBytes(1);

		bsPlayerName.Read(playerName);

		players[playerIdx].SetName(playerName);

		cout << "Player ID " << players[playerIdx].playerID << " is called " << players[playerIdx].GetName() << endl;

		cout << "\n" << endl;

		SendPlayerNamesToClients();
	}
}

void SendCurrentPlayers(RakNet::Packet *packet)
{
	RakNet::BitStream bsPlayers;

	bsPlayers.Write((RakNet::MessageID) ID_SEND_CURRENT_PLAYERS);

	bsPlayers.Write(players.size());

	for (vector<Player>::size_type count = 0; count < players.size(); count++)
	{
		bsPlayers.Write(players[count].GetName());
		bsPlayers.Write(players[count].GetScore());
	}

	
	peerInterface->Send(&bsPlayers, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
	
}


void SendStartGameMessage()
{
	for (vector<Player>::size_type playerIdx = 0; playerIdx < players.size(); playerIdx++)
	{
		RakNet::BitStream bsStartMsg;

		bsStartMsg.Write((RakNet::MessageID) ID_GAME_STARTED);

		peerInterface->Send(&bsStartMsg, HIGH_PRIORITY, RELIABLE_ORDERED, 0, players[playerIdx].clientIP, false);
	}
}

void SendTurnToPlayers()
{
	for (vector<Player>::size_type playerIdx = 0; playerIdx < players.size(); playerIdx++)
	{
		RakNet::BitStream bsPlayerTurn;

		bsPlayerTurn.Write((RakNet::MessageID) ID_PLAYER_TURN);
		bsPlayerTurn.Write(turn);

		peerInterface->Send(&bsPlayerTurn, HIGH_PRIORITY, RELIABLE_ORDERED, 0, players[playerIdx].clientIP, false);
	}
}

void SendAllPlayersCardsString()
{
	RakNet::BitStream bsCards;

	bsCards.Write((RakNet::MessageID) ID_SEND_CARDS_STRING);

	for (vector<Player>::size_type count = 0; count < players.size(); count++)
	{

		bsCards.Write(players[count].GetName());
		bsCards.Write("'s final hand: ");
		bsCards.Write(players[count].hand.PrintHand());

		
	}
	for (vector<Player>::size_type count = 0; count < players.size(); count++)
	{
		peerInterface->Send(&bsCards, HIGH_PRIORITY, RELIABLE_ORDERED, 0, players[count].clientIP, false);
	}
}

void SendOnePlayerCardsString(vector<Player>::size_type count)
{
	RakNet::BitStream bsCards;

	bsCards.Write((RakNet::MessageID) ID_SEND_ONE_PLAYER_CARDS);

	bsCards.Write(players[count].GetName());
	bsCards.Write("'s hand: ");
	bsCards.Write(players[count].hand.PrintHand());

	for (vector<Player>::size_type count2 = 0; count2 < players.size(); count2++)
	{
		peerInterface->Send(&bsCards, HIGH_PRIORITY, RELIABLE_ORDERED, 0, players[count2].clientIP, false);
	}
}

void SendHitStand(vector<Player>::size_type count) 
{
	RakNet::BitStream bsHitStand;

	bsHitStand.Write((RakNet::MessageID) ID_SEND_HIT_STAND);

	bsHitStand.Write("Would you like to (h)it, (s)tand?");

	peerInterface->Send(&bsHitStand, HIGH_PRIORITY, RELIABLE_ORDERED, 0, players[count].clientIP, false);
}

void SendScoreStats()
{
	for (vector<Player>::size_type count = 0; count < players.size(); count++) 
	{
		RakNet::BitStream bsScoreStats;

		bsScoreStats.Write((RakNet::MessageID) ID_SEND_SCORE_STATS);
		bsScoreStats.Write(players[count].GetName() + "'s current score is " + to_string(players[count].GetScore()).c_str() + ". (" + to_string(players[count].wins).c_str() + "W/" + to_string(players[count].draws).c_str() + "D/" + to_string(players[count].losses).c_str() + "L)");

		peerInterface->Send(&bsScoreStats, HIGH_PRIORITY, RELIABLE_ORDERED, 0, players[count].clientIP, false);
	}
}

void SendDealerFirst(vector<Player>::size_type count)
{
	RakNet::BitStream bsDealerFirst;

	bsDealerFirst.Write((RakNet::MessageID) ID_SEND_DEALER_FIRST);
	bsDealerFirst.Write("Dealers hand: ");
	bsDealerFirst.Write(dealer.hand.PrintFirst());
	bsDealerFirst.Write(", ??");

	peerInterface->Send(&bsDealerFirst, HIGH_PRIORITY, RELIABLE_ORDERED, 0, players[count].clientIP, false);
	
}

void SendDealerFull()
{
	RakNet::BitStream bsDealerAll;

	bsDealerAll.Write((RakNet::MessageID)	ID_SEND_DEALER_ALL);
	bsDealerAll.Write("Dealers full hand: ");
	bsDealerAll.Write(dealer.hand.PrintHand());

	for (vector<Player>::size_type count = 0; count < players.size(); count++)
	{
		peerInterface->Send(&bsDealerAll, HIGH_PRIORITY, RELIABLE_ORDERED, 0, players[count].clientIP, false);
	}
}

void SendStringToAll(const char* inputString)
{
	RakNet::BitStream bsString;

	bsString.Write((RakNet::MessageID) ID_SEND_STRING);
	bsString.Write(inputString);

	for (vector<Player>::size_type count = 0; count < players.size(); count++)
	{
		peerInterface->Send(&bsString, HIGH_PRIORITY, RELIABLE_ORDERED, 0, players[count].clientIP, false);
	}
}

void SendStringToOne(const char* inputString, vector<Player>::size_type count)
{
	RakNet::BitStream bsString;
	bsString.Write((RakNet::MessageID) ID_SEND_STRING);

	bsString.Write(inputString);

	peerInterface->Send(&bsString, HIGH_PRIORITY, RELIABLE_ORDERED, 0, players[count].clientIP, false);
}

void DoEnd()
{
	while (dealer.hand.GetValue() < 16) {
		DrawPlayer(dealer);
	}

	// print everyone's hands
	SendDealerFull();
	SendAllPlayersCardsString();

	cout << endl;

	// perform win check
	CheckWins();

	DoneEnd = true;

	numPlayersReadyToStart = 0;

	GameInProgress = false;
	readyToStart = false;

	cout << endl << " waiting for possible restart" << endl;
	cin.ignore();
}

void SendTurnCount()
{
	RakNet::BitStream bsTurnCount;
	bsTurnCount.Write((RakNet::MessageID) ID_SEND_TURN_COUNT);
	bsTurnCount.Write(currentPlayer);
	for (vector<Player>::size_type count = 0; count < players.size(); count++)
	{
		peerInterface->Send(&bsTurnCount, HIGH_PRIORITY, RELIABLE_ORDERED, 0, players[count].clientIP, false);
	}

}

void SetHitStandRecieved(RakNet::Packet *packet)
{
	RakNet::BitStream bsHitStandRecieved(packet->data, packet->length, false);

	bsHitStandRecieved.IgnoreBytes(1);

	bsHitStandRecieved.Read(hitStandResponse);

	hitStandRecieved = true;

	vector<Player>::size_type playerIdx = findPlayerIdxForIPAddr(packet->systemAddress);

	switch (hitStandResponse)
	{
	case 'h':
		DrawPlayer(players[playerIdx]);
		SendOnePlayerCardsString(playerIdx);
		SendHitStand(playerIdx);
		break;

	case 's':
		//cout << hitStandResponse << endl;
		if (currentPlayer == players.size() - 1)
		{
			DoEnd();
		}
		else
		{
			currentPlayer++;
			GameLoop();
		}
		break;

	default:
		break;
	}
}



void RunGame()
{
	

	while (true)
	{
		for (RakNet::Packet *packet = peerInterface->Receive(); packet; peerInterface->DeallocatePacket(packet), packet = peerInterface->Receive())
		{
			switch (packet->data[0])
			{
			case ID_NEW_INCOMING_CONNECTION:
				cout << packet->systemAddress.ToString() << " has connected" << endl;
				cout << "\n" << endl;
				CreateNewPlayer(packet->systemAddress);
				SendCurrentPlayers(packet);
				break;
				
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				cout << "Server is full" << endl;
				cout << "\n" << endl;
				break;

			case ID_DISCONNECTION_NOTIFICATION:
				cout << packet->systemAddress.ToString() << " disconnected" << endl;
				break;

			case ID_CONNECTION_LOST:
				cout << packet->systemAddress.ToString() << " disconnected abnormally" << endl;
				break;

			case ID_PLAYERNAME:
				UpdatePlayerName(packet);
				break;

			case ID_PLAYER_READY_TO_START:
				numPlayersReadyToStart++;
				if (DoneEnd == true)
				{
					cout << "Size of Player Vector: " << players.size() << endl;


				}
				if ((int)players.size() >= 2 && numPlayersReadyToStart >= players.size() && !GameInProgress)
				{
					readyToStart = true;
					cout << "Ready to start new game" << endl;

				}
				if (readyToStart == true && GameInProgress == false)
				{
					GameInProgress = true;
					cout << "Starting new game:" << endl;
					cout << "Ready to start = " << readyToStart << endl;
					cout << "Game in progress = " << GameInProgress << endl;
					StartGame();
				}
				break;

			case ID_PLAYER_DRAW_CARD:
				break;

			case ID_SEND_HIT_STAND_TO_SERVER:
				SetHitStandRecieved(packet);
				break;

			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
#ifdef _WIN32
			Sleep(30);
#else
			usleep(30 * 1000);
#endif // _WIN32

		}


	}
}

int main(){
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(hConsole, 14);

	srand(time(NULL)); // setup random seed

	initialisePeerInterface();

	displayHostIPAddresses();

	unsigned int portNum = getPortNum();

	listenForConnections(portNum);

	RunGame();


	cout << "press any key to end" << endl;
	cin.get();

	return 0;

	//SetupGame();
}



void StartGame(){
	// clear hands and reset stats
	dealer.hand.ClearHand();
	for (vector<Player>::size_type count = 0; count < players.size(); count++) {
		players[count].hand.ClearHand();
		players[count].currentBet = 0;
		players[count].turnCount = 1;
		players[count].insurance = false;
	}

	// draw everyone 2 cards
	DrawForAll();
	DrawForAll();

	// print everyone's score and stats
	SendScoreStats();

	currentPlayer = players[0].playerID;
	turnCount = 0;
	DoneEnd = false;

	// begin the game loop
	GameLoop();
}

// function for checking each players wins
void CheckWins(){
	bool dealerbust = CheckBust(dealer);
	bool dealerblackjack = false;

	if(dealerbust)
	{ 
		SendStringToAll("Dealer is Bust!");
	}

	if(dealer.hand.GetValue() == 21)
	{ 
		SendStringToAll("Dealer got blackjack!");
		dealerblackjack = true; 
	} // dealer blackjack

	Sleep(30);

	for (vector<Player>::size_type count = 0; count < players.size(); count++) 
	{
		if (CheckBust(players[count])) 
		{ 
			SendStringToAll(players[count].GetName() + " went bust, no points!");
			players[count].losses++; 
		}

		else {
			// player blackjack
			if (players[count].hand.GetValue() == 21) {
				players[count].AddScore(players[count].currentBet * 2.5);
				players[count].wins++;
				SendStringToAll(players[count].GetName() + " has blackjack and has been given " + to_string((players[count].currentBet * 2.5)).c_str());
				continue;
			}

			if (dealerbust) {
				// dealer bust
				players[count].AddScore(players[count].currentBet * 2);
				players[count].wins++;
				SendStringToAll(players[count].GetName() + " won and has been given " + to_string((players[count].currentBet * 2)).c_str());
				continue;
			}

			// player win
			if (players[count].hand.GetValue() > dealer.hand.GetValue()) {
				players[count].AddScore(players[count].currentBet * 2);
				players[count].wins++;
				SendStringToAll(players[count].GetName() + " won and has been given " + to_string((players[count].currentBet * 2)).c_str());
			}
			else {
				// player loss
				players[count].losses++;
				SendStringToAll(players[count].GetName() + "'s hand is worse than the dealers! and loses 10 points");
			}
		}
		cout << "Player size check wins: " << players.size() << endl;
		if (count == players.size() - 1)
		{
			SendStringToAll("end");
		}
		Sleep(30);
	}
	
}

// clear players hands
void ClearHands(){
	for(vector<Player>::size_type count = 0; count < players.size(); count++){
		players[count].hand.ClearHand();
	}
}

// draw a card for everyone
void DrawForAll(){
	DrawPlayer(dealer);

	for(vector<Player>::size_type count = 0; count < players.size(); count++){
		DrawPlayer(players[count]);
	}
}

// draw a card for a specific player
void DrawPlayer(Player &player){
	player.hand.AddCard(decks[activeDeck].NextCard());
}

// check if a player has gone bust
bool CheckBust(Player player){
	if(player.hand.GetValue() > 21) { return true; }
	return false;
}

void GameLoop(){
	bool playing = true;
	RakNet::RakString hit = "hit";
	RakNet::RakString stand = "stand";

	//for(vector<Player>::size_type count = 0; count < players.size(); count++)
	//{

	SendTurnCount();

		cout << endl << "---------------------"<< endl << "NEW PLAYER" << endl << "---------------------" << endl;
		if(players[currentPlayer].GetScore() < 0){ cout << players[currentPlayer].GetScore() << " has ran out of score and therefore can't play!" << endl; }
		SendDealerFirst(currentPlayer);

		players[currentPlayer].AddScore(-betScore);


		SendOnePlayerCardsString(currentPlayer);
		if (CheckBust(players[currentPlayer])) { cout << players[currentPlayer].GetName() << " has gone bust!" << endl; playing = false; }
		else {
			SendHitStand(currentPlayer);
		}
		// main play loop
		/*playing = true;
		while(playing){
			SendOnePlayerCardsString(count);
			if(CheckBust(players[count])){ cout << players[count].GetName() << " has gone bust!" << endl; playing = false; }
			else{
				SendHitStand(count);
				
				switch (hitStandResponse)
				{
				case 'h':
					DrawPlayer(players[count]);
					cout << hitStandResponse << endl;
					break;

				case 's':
					playing = false;
					cout << hitStandResponse << endl;
					break;

				default:
					break;
				}
				Sleep(30);
			}
			players[count].turnCount++;
		}*/
//	}
		// draw cards until players hand is worth more than 16
	/*
		while (dealer.hand.GetValue() < 16) {
			DrawPlayer(dealer);
		}

		// print everyone's hands
		SendDealerFull();
		SendAllPlayersCardsString();

		cout << endl;

		// perform win check
		CheckWins();

		// ask if another game should be played
		cout << endl << "Would you like to play again, (Y)es or (N)o?" << endl;
		char valid[] = {'y', 'n'};
		char action = ValidInput_char(valid);
		switch(action){
		case 'y':
			StartGame();
			break;
		case 'n':

		cout << endl << " Press any key to close" << endl;
		cin.ignore();
		//break;
		*/
	//}
}

// wait for valid int input between min and max
int ValidInput_int(int min, int max){
	int input;
	for(;;){
		if(cin >> input)
			if(input >= min && input <= max){ break; }
			cin.clear();
			cin.ignore(INT_MAX,'\n');
	}
	return input;
}

// ensure valid string input is given
string ValidInput_string(){
	string input;
	for(;;){
		if(cin >> input)
			break;
		cin.clear();
		cin.ignore(65565,'\n');
	}
	return input;
}

// ensure valid char input
char ValidInput_char(char valid[]){
	char input;
	for(;;){
		if(cin >> input){
			for(int i = 0; i < sizeof(valid); i++){
				if(tolower(input) == tolower(valid[i])){ return input; }
			}
		}
		cin.clear();
		cin.ignore(CHAR_MAX,'\n');
	}
	return input;
}