#include <stdlib.h>
#include  <stdio.h>

//defines zoning types
enum zoningType {
    NONE,
    RESIDENTIAL,
    COMMERICIAL,
    INDUSTRIAL
};


//basic map node
//each map node represents a square and is characterized by the following vector:
//  X1 : Is Occupied
//  X2 : Is transit Access
//  X3 : Is residentially zoned
//  X4 : Is Commercially Zoned
//  X5 : Is Industrially zoned
//  X6 : Population Factor
//  X7 : Utility Factor
//  X8 : Rent Factor
//  X9 : Employment Access
struct mapNode{
    enum zoningType zoneType;
    double utilityAccessFactor;
    int isTransitAccess;
    double populationFactor;
    double employmentAccess;
    double jobQuanity;
    double rentFactor;
};


//map struct holds an array of map nodes
int xSquares = 100;
int ySquares = 100;
struct map{
    struct mapNode * nodes;
};


#define NODE_AT(x,y)  (y*xSquares+x)

//Map node initialization function 
void initMapNode( struct mapNode * node){
    node->zoneType = NONE;
    node->utilityAccessFactor = 0;
    node->isTransitAccess = 0;
    node->populationFactor = 0;
    node->employmentAccess = 0;
    node->jobQuanity = 0;
    node->rentFactor = 0;
}


//map initialization
void initMap(struct map * myMap){
    myMap->nodes = (struct mapNode * )malloc(sizeof(struct mapNode)*xSquares*ySquares);
    for(int a = 0; a < xSquares*ySquares; a++){
        struct mapNode * nodeAt = &(myMap->nodes[a]);
    }
}

//this funciton is used to remove the map when done, frees memmory
void freeMap(struct map* myMap){
    free(myMap->nodes);
    free(myMap);
}

//prints a nodes state, (prints the vector representing a square of the map)
void printNode(struct mapNode * node){
    printf("[");
    printf("%i",node->zoneType);
    printf(",");
    printf("%f",node->utilityAccessFactor);
    printf(",");
    printf("%f",node->populationFactor);
    printf(",");
    printf("%f",node->employmentAccess);
    printf(",");
    printf("%f",node->jobQuanity);
    printf(",");
    printf("%f",node->rentFactor);
    printf("]\n");
}


//prints all map squares
void printMap(struct map * myMap){
    myMap->nodes = (struct mapNode * )malloc(sizeof(struct mapNode)*xSquares*ySquares);
    for(int a = 0; a < xSquares*ySquares; a++){
        struct mapNode * nodeAt = &(myMap->nodes[a]);
        printNode(nodeAt);
    }
}

//map fill algorithmn
//first lets fill int he zoning information
//next we append public transit information to denote squares as public transit access points
//third use census data:
//      for population factor:
//          count total population
//          assign each squarte with percentage of population
//          normalize values to the population factor sits between 0 and 1
//      for rent data
//          find average rent across map
//          rent factor for square = rent at square / average rent
//      for jobs data
//          fill each square with number of available jobs
//          normalize so values sit betwen 0 and 1
//fourth we calculate employment access
//  for each square:
//      N/p, with p being the total path length, and N being the number of commerical/industrial squares
//              total path lenght is the sum of indiviudal path lengths
//                  each path length is calculated as min(walking distance/ public transit data)
//



int main(){

    struct map myMap;

    initMap(&myMap);

    printMap(&myMap);

    return 0;
}