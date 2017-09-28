// Blackjack_RakNet_Client.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <string>
#include <time.h>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// includes raknet header files
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

// adds library files for RakNet
#pragma comment(lib, "LibStatic.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;

class Player
{
public:
	Player();
	RakNet::RakString name;
	int score;
private:

};

Player::Player()
{
}

class Card
{
private:
	int value; // card value for calculations
	char type; // card type 'A' for Ace, '2' for 2, etc, 'J' - Jack, 'Q' - Queen, 'K' - King
	char suit; // card suit 'D' - Diamonds, 'H' - Hearts, 'S' - Spades, 'C' - Clubs
	RakNet::RakString name; // cards name for printing
public:
	Card();
	Card(char t, char s);

	void SetValue(char t); // set the cards value using its type
	int GetValue() { return value; } // return cards value

	void SetName(char t, char s); // set the cards name using its type and suit from GetName method
	RakNet::RakString GetName() { return name; } // return cards name
	RakNet::RakString GetNameString(char t, char s); // get name string for given type and suit
};

// default constructor
Card::Card() {
	type = 'A';
	suit = 'D';
	SetValue('A');
	SetName('A', 'D');
}


// constructor
Card::Card(char t, char s) {
	type = t;
	suit = s;
	SetValue(t);
	SetName(t, s);
}

/*Card::Card(RakNet::RakString &card)
{
type = (card.SubStr(0, 1)).C_String;
suit = (card.SubStr(1, 1)).C_String;
SetValue(type);
SetName(type, suit);
}*/

// set the cards value using its type
void Card::SetValue(char t) {
	switch (t) {
	case 'A': value = 11; break;
	case '2': value = 2; break;
	case '3': value = 3; break;
	case '4': value = 4; break;
	case '5': value = 5; break;
	case '6': value = 6; break;
	case '7': value = 7; break;
	case '8': value = 8; break;
	case '9': value = 9; break;
	case 'T': value = 10; break;
	case 'J': value = 10; break;
	case 'Q': value = 10; break;
	case 'K': value = 10; break;
	}
}

// set the cards name using its type and suit from GetName method
void Card::SetName(char t, char s) {
	name = GetNameString(t, s);
}

// get name string for given type and suit
RakNet::RakString Card::GetNameString(char t, char s) {
	RakNet::RakString nme = ""; // local name variable, no a to prevent masking the original name

	switch (t) {
	case 'A': nme = "Ace of "; break;
	case '2': nme = "2 of "; break;
	case '3': nme = "3 of "; break;
	case '4': nme = "4 of "; break;
	case '5': nme = "5 of "; break;
	case '6': nme = "6 of "; break;
	case '7': nme = "7 of "; break;
	case '8': nme = "8 of "; break;
	case '9': nme = "9 of "; break;
	case 'T': nme = "10 of "; break;
	case 'J': nme = "Jack of "; break;
	case 'Q': nme = "Queen of "; break;
	case 'K': nme = "King of "; break;
	}

	switch (s) {
	case 'D': nme += "Diamonds"; break;
	case 'S': nme += "Spades"; break;
	case 'H': nme += "Hearts"; break;
	case 'C': nme += "Clubs"; break;
	}

	return nme;
}

class Hand {
public:
	// constructor
	Hand();

	void ClearHand(); // clear the hand
	void AddCard(Card adding); // add card to the hand
	int GetValue(); // return hand value
	RakNet::RakString PrintFirst(); // return first card in hand for printing
	RakNet::RakString PrintHand(); // return hand string for printing
private:
	Card cards[52];
	int cardCount;
};

Hand::Hand() {
	cardCount = 0;
}

// empty hand
void Hand::ClearHand() {
	cardCount = 0;
}

// add a card to the hand
void Hand::AddCard(Card adding) {
	Hand::cardCount++;
	Hand::cards[cardCount - 1] = adding;
}

// get the value of the hand by adding each cards value
int Hand::GetValue() {
	int lowvalue = 0; // value for low ace (1)
	int highvalue = 0; // value for high ace (11)

	for (int i = 0; i < Hand::cardCount; i++) {
		if (Hand::cards[i].GetValue() == 11) {
			// if the card is ace, apply to values correctly
			lowvalue += 1;
			highvalue += 11;
		}
		else {
			lowvalue += Hand::cards[i].GetValue();
			highvalue += Hand::cards[i].GetValue();
		}
	}

	// if high ace takes the hand over 21, use low ace value instead
	if (highvalue > 21) { return lowvalue; }
	else { return highvalue; }
}

// print first card in hand
RakNet::RakString Hand::PrintFirst() {
	return Hand::cards[0].GetName();
}

// print whole hand
RakNet::RakString Hand::PrintHand() {
	RakNet::RakString returning = "";

	// loop through hand adding each card name to the string
	for (int i = 0; i < Hand::cardCount; i++) {
		returning += Hand::cards[i].GetName();
		if (i < (cardCount - 1)) { returning += ", "; }
	}
	return returning;
}

class Deck {
private:
	Card cards[52];
	int currentCard;
public:
	// constructor
	Deck();

	void GenerateSuit(int startindex, char suit); // generate a set of cards for a suit
	void Shuffle(int count); // shuffle the deck count times
	Card NextCard(); // return next card
};

// constructor
Deck::Deck() {
	currentCard = -1;
	GenerateSuit(0, 'H');
	GenerateSuit(13, 'D');
	GenerateSuit(26, 'C');
	GenerateSuit(39, 'S');
	Shuffle(150);
}

// generate a set of cards for a suit
void Deck::GenerateSuit(int startindex, char suit) {
	Deck::cards[startindex] = Card('A', suit);
	Deck::cards[startindex + 1] = Card('2', suit);
	Deck::cards[startindex + 2] = Card('3', suit);
	Deck::cards[startindex + 3] = Card('4', suit);
	Deck::cards[startindex + 4] = Card('5', suit);
	Deck::cards[startindex + 5] = Card('6', suit);
	Deck::cards[startindex + 6] = Card('7', suit);
	Deck::cards[startindex + 7] = Card('8', suit);
	Deck::cards[startindex + 8] = Card('9', suit);
	Deck::cards[startindex + 9] = Card('T', suit);
	Deck::cards[startindex + 10] = Card('J', suit);
	Deck::cards[startindex + 11] = Card('Q', suit);
	Deck::cards[startindex + 12] = Card('K', suit);
}

// shuffle the deck count times
// swap 2 cards around [count] times, effective shuffle decently performant depending on how high count is
void Deck::Shuffle(int count) {
	srand(time(NULL)); // setup random seed
	while (count > 0) {
		Card temp;
		int swap = rand() % 52;
		int swap2 = rand() % 52;
		temp = Deck::cards[swap];
		Deck::cards[swap] = Deck::cards[swap2];
		Deck::cards[swap2] = temp;

		count--;
	}
}

Card Deck::NextCard() {
	Deck::currentCard++;
	if (Deck::currentCard > 51) { Deck::currentCard = 0; Deck::Shuffle(150); }
	return Deck::cards[currentCard];
}

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

char valid[] = { 'h', 's' };
char action;

bool gameInProgress = false;
bool readyToStart = false;
bool myTurn = false;
bool preReadyToStart = false;
bool readyForInput = false;

int input;

int numPlayers;

int ValidInput_int(int min, int max);
string ValidInput_string();
char ValidInput_char(char valid[]);

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
	cout << "\n" << endl;
	for (unsigned int count = 0; count < peerInterface->GetNumberOfAddresses(); count++)
	{
		cout << peerInterface->GetLocalIP(count) << endl;
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
	cout << "\n" << endl;
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
	cout << "Sending Ready to Server" << endl;
}

void UpdatePlayers(RakNet::Packet *packet)
{
	RakNet::BitStream bsPlayers(packet->data, packet->length, false);
	bsPlayers.IgnoreBytes(1);
	bsPlayers.Read(numPlayers);
	Players.clear();
	for (int count = 0; count < numPlayers; count++)
	{
		Player newPlayer;
		bsPlayers.Read(newPlayer.name);
		bsPlayers.Read(newPlayer.score);
		Players.push_back(newPlayer);
		if (count == numPlayers - 1)
		{
			cout << newPlayer.name << " has joined the game" << endl;
			cout << "\n" << endl;
		}
		
		if (count == numPlayers - 1) 
		{
			if (newPlayer.name == playerName) 
			{
				cout << "input (e) to indicate ready to start game" << endl;
				cin >> input;
				if (input = 'e')
				{
					preReadyToStart = true;
				}
			}
		}
		Sleep(30);
	}
}

void Restartinput()
{
	cout << "input (e) to indicate ready to start game" << endl;
	cin >> input;
	if (input = 'e')
	{
		preReadyToStart = true;
	}
}

void PrintCurrentPlayers(RakNet::Packet *packet)
{
	RakNet::BitStream bsPlayers(packet->data, packet->length, false);
	bsPlayers.IgnoreBytes(1);
	bsPlayers.Read(numPlayers);
	Players.clear();
	cout << "Current Players: " << endl;
	for (int count = 0; count < numPlayers; count++)
	{
		Player newPlayer;
		bsPlayers.Read(newPlayer.name);
		bsPlayers.Read(newPlayer.score);
		Players.push_back(newPlayer);
		cout << newPlayer.name << endl;
	}

}

void PrintScoreStats(RakNet::Packet *packet)
{
	RakNet::BitStream bsScoreStats(packet->data, packet->length, false);
	bsScoreStats.IgnoreBytes(1);

	RakNet::RakString statsFull;
	bsScoreStats.Read(statsFull);

	cout << statsFull << endl;
	cout << "\n" << endl;
}

void PrintDealerFirst(RakNet::Packet *packet)
{
	RakNet::BitStream bsDealerFirst(packet->data, packet->length, false);
	bsDealerFirst.IgnoreBytes(1);
	RakNet::RakString dealerFirst;
	RakNet::RakString dealerFirstFull;
	bsDealerFirst.Read(dealerFirst);
	dealerFirstFull = dealerFirst;
	bsDealerFirst.Read(dealerFirst);
	dealerFirstFull += dealerFirst;
	bsDealerFirst.Read(dealerFirst);
	dealerFirstFull += dealerFirst;

	cout << dealerFirstFull << endl;
	cout << "\n" << endl;
}

void PrintDealerFull(RakNet::Packet *packet)
{
	RakNet::BitStream bsDealerFull(packet->data, packet->length, false);
	bsDealerFull.IgnoreBytes(1);
	RakNet::RakString dealer;
	RakNet::RakString dealerFull;
	bsDealerFull.Read(dealer);
	dealerFull = dealer;
	bsDealerFull.Read(dealer);
	dealerFull += dealer;
	cout << dealerFull << endl;
	cout << "\n" << endl;
}

void PrintPlayerCards(RakNet::Packet *packet)
{
	RakNet::BitStream bsPlayerCards(packet->data, packet->length, false);
	bsPlayerCards.IgnoreBytes(1);
	RakNet::RakString cards;
	RakNet::RakString cardsFull;
	bsPlayerCards.Read(cards);
	cardsFull = cards;
	bsPlayerCards.Read(cards);
	cardsFull += cards;
	bsPlayerCards.Read(cards);
	cardsFull += cards;

	cout << cardsFull << endl;
	cout << "\n" << endl;
}



void TestTest()
{
	RakNet::BitStream bsHS;


	switch (action)
	{
	case 'h':
		bsHS.Write((RakNet::MessageID) ID_SEND_HIT_STAND_TO_SERVER);
		bsHS.Write(action);
		peerInterface->Send(&bsHS, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, 0);
		break;

	case 's':
		bsHS.Write((RakNet::MessageID) ID_SEND_HIT_STAND_TO_SERVER);
		bsHS.Write(action);
		peerInterface->Send(&bsHS, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, 0);

		if (playerID < numPlayers - 1)
		{
			cout << "Passing turn to next player" << endl;
			cout << "\n" << endl;
		}
		break;

	default:
		break;
	}
}

void Restart()
{
	cout << "Would you like to restart" << endl;
	char response;
	cin >> response;
	switch (response)
	{
	case 'y':
		gameInProgress = false;
		readyToStart = false;
		Restartinput();
		break;

	case 'n':
		exit(0);
		break;

	default:
		break;
	}
}

void HitorStand(RakNet::Packet *packet)
{
	RakNet::BitStream bsHitorStand(packet->data, packet->length, false);
	bsHitorStand.IgnoreBytes(1);
	RakNet::RakString hitorstand;
	bsHitorStand.Read(hitorstand);
	cout << hitorstand << endl;


	action = ValidInput_char(valid);

	Sleep(30);

	TestTest();
}

void RecieveAllCards(RakNet::Packet *packet)
{
	RakNet::BitStream bsAllCards(packet->data, packet->length, false);
	bsAllCards.IgnoreBytes(1);
	RakNet::RakString allCards;
	RakNet::RakString allCardsAll;

	for (int i = 0; i < numPlayers; i++)
	{
		bsAllCards.Read(allCards);
		allCardsAll = allCards;
		bsAllCards.Read(allCards);
		allCardsAll += allCards;
		bsAllCards.Read(allCards);
		allCardsAll += allCards;
		cout << allCardsAll << endl;
	}


}

void RecieveString(RakNet::Packet *packet)
{
	RakNet::BitStream bsRecieveString(packet->data, packet->length, false);
	bsRecieveString.IgnoreBytes(1);
	RakNet::RakString recievedString;
	bsRecieveString.Read(recievedString);
	cout << recievedString << endl;
	cout << "\n" << endl;

	if (recievedString == "end")
	{
		Restart();
	}
}

void RecieveCurrentPlayers(RakNet::Packet *packet)
{
	RakNet::BitStream bsRecieveCurrentPlayers(packet->data, packet->length, false);
	bsRecieveCurrentPlayers.IgnoreBytes(1);
	int recievedCurrentPlayers;
	bsRecieveCurrentPlayers.Read(recievedCurrentPlayers);
	if (recievedCurrentPlayers < playerID)
	{
		cout << "Waiting for your turn" << endl;
		cout << "\n" << endl;
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
				cout << "\n" << endl;
				break;

				// Notification other system has disconnected.  That is if this is the client then server disconnected
			case ID_DISCONNECTION_NOTIFICATION:
				cout << packet->systemAddress.ToString() << " disconnected" << endl;
				cout << "\n" << endl;
				break;

				// Same as above but abnormal disconnection
			case ID_CONNECTION_LOST:
				cout << packet->systemAddress.ToString() << " disconnected abnormally" << endl;
				cout << "\n" << endl;
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

			case ID_SEND_CURRENT_PLAYERS:
				PrintCurrentPlayers(packet);
				break;

			case ID_SEND_CARDS_STRING:
				RecieveAllCards(packet);
				break;

			case ID_SEND_SCORE_STATS:
				PrintScoreStats(packet);
				break;

			case ID_SEND_DEALER_FIRST:	
				PrintDealerFirst(packet);
				break;

			case ID_SEND_DEALER_ALL:
				PrintDealerFull(packet);
				break;

			case ID_SEND_ONE_PLAYER_CARDS:
				PrintPlayerCards(packet);
				break;

			case ID_SEND_HIT_STAND:
				HitorStand(packet);
				break;

			case ID_SEND_STRING:
				RecieveString(packet);
				break;
			case ID_SEND_TURN_COUNT:
				RecieveCurrentPlayers(packet);
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

		// If a key has been pressed send ready message
		if (preReadyToStart == true && readyToStart == false) {
			// Get line to clear input buffer of ENTER
			//char temp[32];

			//cin.getline(temp, 32);

			//cin.ignore(INT_MAX, '\n');

			SendReady();
			readyToStart = true;
			cout << "Waiting for other players to start..." << endl;
			cout << "\n" << endl;
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

// wait for valid int input between min and max
int ValidInput_int(int min, int max) {
	int input;
	for (;;) {
		if (cin >> input)
			if (input >= min && input <= max) { break; }
		cin.clear();
		cin.ignore(INT_MAX, '\n');
	}
	return input;
}

// ensure valid string input is given
string ValidInput_string() {
	string input;
	for (;;) {
		if (cin >> input)
			break;
		cin.clear();
		cin.ignore(65565, '\n');
	}
	return input;
}

// ensure valid char input
char ValidInput_char(char valid[]) {
	char input;
	for (;;) {
		if (cin >> input) {
			for (int i = 0; i < sizeof(valid); i++) {
				if (tolower(input) == tolower(valid[i])) { return input; }
			}
		}
		cin.clear();
		cin.ignore(CHAR_MAX, '\n');
	}
	return input;
}

