#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "pokereval.h"

int player1wins = 0, player2wins = 0, ties = 0;
long playerhands[10] = {0};
FILE *input = NULL;

int main() {
    
    if (!(input = fopen("poker.txt", "r"))){
        printf("Failed to open file.\n");
        
        return 1;
    } 
    char * line = (char *)malloc(30 * sizeof(char));

    while (fgets(line, 31, input) != NULL){
        card *player1cards = dealCards(line, 1), *player2cards = dealCards(line, 2);
        hand p1hand = evalHand(player1cards), p2hand = evalHand(player2cards);
        int p1high = 0, p2high = 0;

        if (p1hand > p2hand){
            player1wins++;
        } 
        else if (p1hand < p2hand){
            player2wins++;
        } 
        else {

            do {
                if (p1hand == HIGH || p1hand == STRAIGHT || p1hand == FLUSH || p1hand == STRAIGHT_FLUSH){
                    p1high = highestRank(player1cards);
                    p2high = highestRank(player2cards);
                }   
                else if (p1hand == FULL_HOUSE || p1hand == THREE_OF_A_KIND){
                    p1high = highestTriple(player1cards);
                    p2high = highestTriple(player2cards);
                }
                else {
                    // FOUR_OF_A_KIND || ONE_PAIR || TWO_PAIRS
                    p1high = highestPair(player1cards);
                    p2high = highestPair(player2cards);
                } 
            } while (p1high == p2high && p1high != 0 && p2high != 0);
            
            if (p1high > p2high){
                player1wins++;
            } 
            else if (p1high < p2high){
                player2wins++;
            }
            else ties++;
        } 
        free(player1cards);
        free(player2cards);   
    } 
    printf("\nPlayer 1 wins %d times.\nPlayer 2 wins %d times.\nPlayers tied %d times.\n", player1wins, player2wins, ties);
    free(line);
    fclose(input);
    
    return 0;
}


/*
    Parses the input line into each players' hands.
*/
card *dealCards(char *cards, short player){ 
    card *buffer = (card *)malloc(5 * sizeof(card));
    short index = (player - 1) * 15;
    
    for (int i = 0; i < 5; i++){
        card temp; 
        switch(cards[index]){
            case 'T':
                temp.rank = TEN;
                break;
            case 'J':
                temp.rank = JACK;
                break;
            case 'Q':
                temp.rank = QUEEN;
                break;
            case 'K':
                temp.rank = KING;
                break;
            case 'A':
                temp.rank = ACE;
                break;
            default:
                temp.rank = cards[index] - 48;
        }
        temp.suit = cards[index+1];
        index += 3;
        buffer[i] = temp;
    }
    return buffer;
}

/*
    Input: array of Cards
    Output: Hand type (High, One_Pair, Two_Pair, Three_of_Kind, Straight,
                        Flush, Full_House, Four_of_Kind, Straight_Flush, 
                        or Royal_Flush)
    
    First checks it if it is a "Pair hand", i.e., a hand with duplicate face ranks.
    If it is not a "Pair hand", then it is a High and checks for a Straight or a Flush
    and then a Straight-Flush or Royal Flush accordingly.
*/
hand evalHand(card *cards){
    hand returnhand = checkPairs(cards);
    int straight = -1;
    int flush = -1;

    if (returnhand == HIGH){
        if ((straight = checkStraight(cards)) == 1){
            returnhand = STRAIGHT;
        }
        if ((flush = checkFlush(cards)) == 1){
            returnhand = FLUSH;
        }
        if (straight == 1 && flush == 1){
            if (checkRoyal(cards) == 1){
                returnhand = ROYAL_FLUSH;
            } else returnhand = STRAIGHT_FLUSH;
        }
    }
    playerhands[returnhand]++;
    return returnhand;
}

/*
    Checks for High, One_Pair, Two_Pair, Three_of_Kind, Full_House, and Four_of_Kind.
    Uses an unsigned long as a bitfield representing each card, where every 1 represents
    a card in the hand. So the hand     
                                       KD 8S 9S 4C KS
                                            =
            0000 0000 0000 0011 0000 0000 0000 0001 0001 0000 0000 0000 0001 0000 0000
              X    X    A    K    Q    J   10    9    8    7    6    5    4    3    2  .

    This bitfield modulo 15 results in a different ranks depending on the hand!
            Four of a Kind = 1,       High Card = 5,         One Pair = 6, 
            Two Pairs = 7,          Three of a Kind = 9,     Full House = 10 

    This works because it essentially adds up the ranks of the 4 bits (1111b = 15, hence 
    modulo 15) corresponding to each face. In the above case we have,
                            0011 + 0001 + 0001 + 0001 = 6 === 6 (mod 15).
                              K      9      8      4
    
    If we had Four of a kind then we would have,
                                1111 + 0001 = 16 === 1 (mod 15).

*/
int checkPairs(card *cards){
    unsigned long b = 0;
    int offset[13] = { 0 };
    
    for (int i = 0; i < 5; i++){
        int r = cards[i].rank;
        b = b | (unsigned long)1 << (4 * r + offset[r-2]);
        offset[r-2]++;
    }
    
    switch (b%15){
        case 1:
            return FOUR_OF_A_KIND;
        case 5:
            return HIGH;
        case 6:
            return ONE_PAIR;
        case 7:
            return TWO_PAIRS;
        case 9:
            return THREE_OF_A_KIND;
        case 10:
            return FULL_HOUSE;
        default:
            return HIGH;
    }
}

/*
    Similar to checkPairs(), this function also uses a bitfield to mark the occurance of 
    each rank. So the given the hand 
                                    KD 8S 9S 4C KS
                                          =
                    0  1  0  0   0  1  1  0  0  0  1  0  0  0  0      
                    A  K  Q  J  10  9  8  7  6  5  4  3  2  X  X .
    
    The number produced by 
                                        b & -b
    gives us the rank of the least significant bit. Dividing the original number by
    this shifts the bits to the right so the least significant bit is in the 1 position.
    So if the hand is a straight, the resulting numer will be 
                                       0b11111 = 31.
    If the bitfield = 16444, then the hand has an Ace low straight (A2345).
*/
int checkStraight(card *cards){
    int b = 0;
    b = b | 1 << cards[0].rank 
            | 1 << cards[1].rank 
            | 1 << cards[2].rank 
            | 1 << cards[3].rank 
            | 1 << cards[4].rank;
    if ((b/(b & -b) == 31) || b == 16444){
        
        return 1;
    } else return -1;
}

int checkFlush(card *cards){
    int s = cards[0].suit;
    if ((s == cards[1].suit) && (s == cards[2].suit) && (s == cards[3].suit) && (s == cards[4].suit)){
        
        return 1;
    } else return -1;
}

/*
    Sums the face ranks to check for a Royal Flush.
*/
int checkRoyal(card *cards){
    int val = cards[0].rank + cards[1].rank + cards[2].rank + cards[3].rank + cards[4].rank;
    if (val == 60){
        
        return 1;
    } else return -1;
}


/*
    This funtion simply returns the card with the highest face rank.
    
    The rank of that card is then set to 0 in case the opposing hand has the same 
    high card and the next highest card needs to be returned. (This should probably be 
    changed).
*/
int highestRank(card *cards){
    int high = cards[0].rank;
    for (int i = 1; i < 5; i++){
        if (high < cards[i].rank){
            high = cards[i].rank;
        }
    }
    for (int i = 0; i < 5; i++){
        if (cards[i].rank == high){
            cards[i].rank = 0;
        }
    }
    return high;
}

/*
    This function finds the highest face rank with a duplicate. Since there can only 
    be at most two duplicate face ranks, 
*/
int highestPair(card *cards){
    
    int pairs[2] = {-1, -1};
    int ret = -1;
    for (int i = 0; i < 4; i++){
        for (int j = i+1; j < 5; j++){
            if (cards[i].rank > 0 && cards[i].rank == cards[j].rank){
                if (pairs[0] == -1){
                    pairs[0] = cards[i].rank;
                }
                else if (pairs[0] != cards[i].rank){
                    pairs[1] = cards[i].rank;
                }
            }
        }
    }
    
    int bye;
    if (pairs[0] == -1 && pairs[1] == -1){
        bye = highestRank(cards);
    }
    else if (pairs[0] > pairs[1]){
        bye = pairs[0];
    } else bye = pairs[1];

    for (int i = 0; i < 5; i++){
        if (cards[i].rank == bye){
            cards[i].rank = 0;
        }
    }
    return bye;
}

/*
    This should probably be changed.
*/
int highestTriple(card *cards){
    for (int i = 0; i < 4; i++){
        int count = 1;
        for (int j = i+1; j < 5; j++){
            if (cards[i].rank > 0 && cards[i].rank == cards[j].rank){
                cards[j].rank = 0;
                if ((++count) == 3){
                    int temp = cards[i].rank;
                    cards[i].rank = 0;
                    return temp;
                }
            }
        }
    }
    return highestPair(cards);
}


/*
        Debug Helper Functions
*/

/*
                                        C   A   N       R   E   M   O   V   E
    Takes an unsigned long and prints it to stdout in binary 
*/
void ulongtobin(unsigned long l){
    int arr[64] = { 0 };
    for (int i = 0; l > 0; i++){
        arr[i] = l%2;
        l /= 2;
    }  
    for (int i = 63; i >= 0; i--){
        printf("%d", arr[i]);
        if (i%4 == 0){
            printf(" ");
        }
    }
    printf("\n");
}


// returns a string representation of the Hand enum
char *handToString(hand type){
    switch(type){
        case HIGH:
            return "High";
        case ONE_PAIR:
            return "One Pair";
        case TWO_PAIRS:
            return "Two Pair";
        case THREE_OF_A_KIND:
            return "Three of a Kind";
        case STRAIGHT:
            return "Straight";
        case FLUSH:
            return "Flush";
        case FULL_HOUSE:
            return "Full House";
        case FOUR_OF_A_KIND:
            return "Four of a Kind";
        case STRAIGHT_FLUSH:
            return "Straight Flush";
        case ROYAL_FLUSH:
            return "Royal Flush";
        default:
            return "Error";
    }
    return "";
}


// converts rank enum to string
char *rankToString(rank r){
    switch(r){
        case TWO:
            return "2";
        case THREE:
            return "3";
        case FOUR:
            return "4";
        case FIVE:
            return "5";
        case SIX:
            return "6";
        case SEVEN:
            return "7";
        case EIGHT:
            return "8";
        case NINE:
            return "9";
        case TEN:
            return "T";
        case JACK:
            return "J";
        case QUEEN:
            return "Q";
        case KING:
            return "K";
        case ACE:
            return "A";
        default:
            return "Error";
    }
}


void printCards(card *cards){
    for (int i = 0; i < 5; i++){
        printf("%s%c ", rankToString(cards[i].rank), cards[i].suit);
    }
    printf("\n");
}