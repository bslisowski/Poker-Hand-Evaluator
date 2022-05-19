
typedef enum {
    TWO = 2,
    THREE = 3,
    FOUR = 4,
    FIVE = 5,
    SIX = 6,
    SEVEN = 7,
    EIGHT = 8,
    NINE = 9,
    TEN = 10,
    JACK = 11,
    QUEEN = 12,
    KING = 13,
    ACE = 14
}rank;

typedef enum { 
    CLUBS = 'C', 
    DIAMONDS = 'D', 
    HEARTS = 'H', 
    SPADES = 'S' 
}suit;

typedef enum {
    HIGH = 0,
    ONE_PAIR,
    TWO_PAIRS,
    THREE_OF_A_KIND,
    STRAIGHT,
    FLUSH,
    FULL_HOUSE,
    FOUR_OF_A_KIND,
    STRAIGHT_FLUSH,
    ROYAL_FLUSH
}hand;

typedef struct {
    rank rank;
    suit suit;
}card;


card *dealCards(char *, short);
hand evalHand(card *);
int checkPairs(card *);
int checkStraight(card *);
int checkFlush(card *);
int checkRoyal(card *);
int highestRank(card *);
int highestPair(card *);
int highestTriple(card *);
void printCards(card *);
char *rankToString(rank);
char *handToString(hand);