#include <stdlib.h>
#include  <stdio.h>
#include <stdlib.h>
#include <math.h>

//defines zoning types
enum zoningType {
    NONE,
    RESIDENTIAL,
    COMMERICIAL,
    INDUSTRIAL,
    COMMERCIAL_AND_RESIDENTIAL,
    INDUSTRIAL_AND_RESIDENTIAL,
    COMMERCIAL_AND_INDUSTRIAL
};


//basic map node
//each map node represents a square and is characterized by the following vector:
//  X1 : Is Occupied, Is Commercially Zoned, Is Industrially zoned
//  X2 : Is transit Access
//  X3 : Population Factor
//  X4 : Utility Factor
//  X5 : Rent Factor
//  X6 : Employment Access
struct mapNode{
    enum zoningType zoneType;
    int isTransitAccess;
    double utilityAccessFactor;
    double populationFactor;
    double employmentAccess;
    double jobQuantity;
    double rentFactor;
    double quality;
};



//map struct holds an array of map nodes
int xSquares = 25;
int ySquares = 25;
//size of square in miles;
double squareSize = 1;
struct map{
    struct mapNode * nodes;
};



#define NODE_AT(x,y)  (((int)(y/squareSize)*xSquares+(int)(x/squareSize)))

#define RANDOM (((double)rand())/RAND_MAX)

//Map node initialization function 
void initMapNode( struct mapNode * node){

    node->zoneType = NONE;
    node->isTransitAccess = 1;
    node->utilityAccessFactor = 0;
    node->populationFactor = 0;
    node->employmentAccess = 0;
    node->jobQuantity = 0;
    node->rentFactor = 0;
    node->quality = 0;

}



void normalizeMapNodes(struct map * myMap);



//map initialization
void initMap(struct map * myMap){
    myMap->nodes = (struct mapNode * )malloc(sizeof(struct mapNode)*xSquares*ySquares);
    for(int a = 0; a < xSquares*ySquares; a++){
        struct mapNode * nodeAt = &(myMap->nodes[a]);
        initMapNode(nodeAt);
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
    printf("%i",node->isTransitAccess);
    printf(",");
    printf("%f",node->utilityAccessFactor);
    printf(",");
    printf("%f",node->populationFactor);
    printf(",");
    printf("%f",node->employmentAccess);
    printf(",");
    printf("%f",node->jobQuantity);
    printf(",");
    printf("%f",node->rentFactor);
    printf(",");
    printf("%f",node->quality);
    printf("]\n");
}



//prints all map squares
void printMap(struct map * myMap){
    //myMap->nodes = (struct mapNode * )malloc(sizeof(struct mapNode)*xSquares*ySquares);
    for(int a = 0; a < xSquares*ySquares; a++){
        struct mapNode * nodeAt = &(myMap->nodes[a]);
        printNode(nodeAt);
    }
}




void saveMap(struct map* myMap, char * name){
    FILE * fptr;

    fptr = fopen(name,"w");

    if (fptr == NULL) {
        printf("Error opening the file!\n");
        return; 
    }
    fwrite(&(xSquares), 1, sizeof(int), fptr);
    fwrite(&(ySquares), 1, sizeof(int), fptr);
    fwrite(&(squareSize), 1, sizeof(double), fptr);
    for(int i = 0; i < xSquares*ySquares; i++){
        printf("Now writing squares\n");
        struct mapNode * node = &(myMap->nodes[i]);
        fwrite(&(node->zoneType), 1, sizeof(int), fptr);
        fwrite(&(node->utilityAccessFactor), 1, sizeof(double), fptr);
        fwrite(&(node->populationFactor), 1, sizeof(double), fptr);
        fwrite(&(node->employmentAccess), 1, sizeof(double), fptr);
        fwrite(&(node->jobQuantity), 1, sizeof(double), fptr);
        fwrite(&(node->rentFactor), 1, sizeof(double), fptr);
        fwrite(&(node->quality), 1, sizeof(double), fptr);
        printNode(node);
    }


    fclose(fptr);
    return;
}



void loadMap(struct map* myMap, char * name){
    FILE * fptr;

    fptr = fopen(name,"r");

    if (fptr == NULL) {
        printf("Error opening the file!\n");
        return; 
    }
    fread(&(xSquares),sizeof(int),1,fptr);
    fread(&(ySquares),sizeof(int),1,fptr);
    fread(&(squareSize),sizeof(double),1,fptr);
    for(int i = 0; i < xSquares*ySquares; i++){
        printf("Now reading squares\n");
        struct mapNode * node = &(myMap->nodes[i]);
        fread(&(node->zoneType),sizeof(int),1,fptr);
        fread(&(node->utilityAccessFactor),sizeof(double),1,fptr);
        fread(&(node->populationFactor),sizeof(double),1,fptr);
        fread(&(node->employmentAccess),sizeof(double),1,fptr);
        fread(&(node->jobQuantity),sizeof(double),1,fptr);
        fread(&(node->rentFactor),sizeof(double),1,fptr);
        fread(&(node->quality),sizeof(double),1,fptr);
        printNode(node);
    }


    fclose(fptr);
    return;
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
//      for rent burden
//          fill each square with rent burden
//          normalize values;
//fourth we calculate employment access
//  for each square:
//      N/p, with p being the total path length, and N being the number of commerical/industrial squares
//              total path lenght is the sum of indiviudal path lengths
//                  each path length is calculated as min(walking distance/ public transit data)
//
//




//this normalizes the map values using min/max
void normalizeMapNodes(struct map * myMap){

    int adj = 0;
    double maxFactor = 0;
    double minFactor = 0;

    int numberOfNormalizedParams = 5;

    struct mapNode * startingPosition = &(myMap->nodes[0]);
    struct mapNode * currentPosition = &(myMap->nodes[0]);
    double * valuePointer;

    for(int k = 0; k < numberOfNormalizedParams; k++){

        //printf("About to start new params\n");

        currentPosition = startingPosition;
        maxFactor = *(((double*)&(myMap->nodes[0]))+k);
        minFactor = 0;

        //printf("Starting new param\n");

        for(int i = 0; i < xSquares*ySquares; i++){

            /*printf("At Square %i %i ",i/xSquares, i%xSquares);
            printf(" - index %i \n",i);
            printf(" - param number %i \n",k+1);*/
            
            currentPosition += 1;   
            valuePointer = (((double *)&(currentPosition->utilityAccessFactor)) + k);


            double value = *(valuePointer);
            

            //printf(" - Value %f \n",value);
            

            if(maxFactor < value){
                maxFactor = value;
            }
            
            
            if(minFactor > value){
                minFactor = value;
            }

            //printf("Finished counting values\n");
        }

        currentPosition = startingPosition;
        for(int i = 0; i < xSquares*ySquares; i++){
            /*printf("Finished counting values\n");
            printf("At Square %i %i ",i/xSquares, i%xSquares);
            printf(" - index %i \n",i);
            printf(" - param number %i \n",k+1);*/

            currentPosition += 1;
            valuePointer = (((double *)&(currentPosition->utilityAccessFactor)) + k);

            double value = *(valuePointer);
            *valuePointer  = (value - minFactor)/(maxFactor - minFactor);

            //printf("Finished counting values\n");
        }
    }
}


//represents the data from a single census track
struct censusTrack{
    double centerX;
    double centerY;
    int numOfPeople;
    int numOfEmployedPeople;
    double utilityCoverage;
    double medianRent;
    long int geoID;
};

void initCensusTrack(struct censusTrack * track){
    track->geoID = 36007000100;
    track->centerX = 0;
    track->centerY = 0;
    track->numOfPeople = 0;
    track->medianRent = 0;
    track->numOfEmployedPeople = 0;
    track->utilityCoverage = 0;
}


struct censusTrack * censusTrackArray;
int censusTrackLength = 0;

//add census track info to census track array
void addCensusTrack(struct censusTrack * newCensusTrack){

    struct censusTrack * newLs = (struct censusTrack * )(malloc(sizeof(struct censusTrack)*(censusTrackLength+1)));

    for(int i = 0; i < censusTrackLength; i++){
        newLs[i] = censusTrackArray[i];
    }
    newLs[censusTrackLength] = *(newCensusTrack);

    if( censusTrackLength > 0 ){
        free(censusTrackArray);
    }

    censusTrackArray = newLs;

    censusTrackLength++;
}

//find nearest k census tracks to point, fills retList with K integers, each is an index of the track in the censusTrackArray
void findNearestKCensusTracks(int * retList, int k, double x, double y){

    double * nearestDistances = (double * )malloc(sizeof(double)*k);
    int * nearestDistancesIndex = (int * )malloc(sizeof(int)*k);
    double largestDistance;
    int largestIndex;

    double maxPossibleDistance = (xSquares+ySquares)*squareSize;

    for(int i = 0; i < k; i++){
        nearestDistancesIndex[i] = 0;
        nearestDistances[i] = maxPossibleDistance;
    }

    for(int i = 0; i < censusTrackLength; i++){
        struct censusTrack * track = &(censusTrackArray[i]);
        double disX = track->centerX - x;
        double disY = track->centerY - y;
        disX *= disX;
        disY *= disY;
        double totalDis = sqrt(disX+disY);
        largestDistance = nearestDistances[0];
        largestIndex = 0;
        for(int j = 0; j < k; j++){
            if(nearestDistances[j] > largestDistance){
                largestIndex = j;
                largestDistance = nearestDistances[j];
            }
        }
        if(largestDistance > totalDis){
            nearestDistancesIndex[largestIndex] = i;
            nearestDistances[largestIndex] = totalDis;
            //printf("new nearest distance of %f at index %i to point %f, %f\n",nearestDistances[i],retList[i],x,y);
        }
    }

    for(int i = 0; i < k; i++){
        retList[i] = nearestDistancesIndex[i];
        //printf("nearest distance of %f at index %i to point %f, %f\n",nearestDistances[i],retList[i],x,y);
    }

    free(nearestDistances);
    free(nearestDistancesIndex);

    printf("nearest tracks ");
    for(int i = 0; i < k-1; i++){
        printf("%i ",retList[i]);
    }
    //printf("and %i to point %f, %f\n",retList[k-1],x,y);

}


double getEmploymentFactor( struct map* myMap, double x, double y);


//factor calculations
double getRawPopulationFactor( int squarePopulation ){
    return squarePopulation/1000.0;
}
double getJobFactor( int numberEmployed ){
    return numberEmployed/1000.0;
}
double getRawRentFactor( double medianRent ){
    return medianRent/1000.0;
}
double getUtilityAccessFactor( double utilityAccessFactor ){
    return utilityAccessFactor;
}


//fills in square census data using weighted average from nearest K census tracks
//parameter u should be 1 for normal operation
//parameter u allows control of distance metric:
//              d = abs(x1-x2)^u
//
//this is raw data, expects a final call to 
//
void populateMapNodeWithRawKNearestCensusData(struct map* myMap, int k, int u, double x, double y){
    int * censusTrackIndex = (int *)malloc(sizeof(int)*k);
    double * popFactors = (double *)malloc(sizeof(double)*(k+1));
    double * jobFactors = (double *)malloc(sizeof(double)*(k+1));
    double * rentFactors = (double *)malloc(sizeof(double)*(k+1));
    double * utilityFactors = (double *)malloc(sizeof(double)*(k+1));
    double * employmentFactors = (double *)malloc(sizeof(double)*(k+1));
    double * dis = (double *)malloc(sizeof(double)*(k));
    findNearestKCensusTracks(censusTrackIndex,k,x,y);


    for(int i = 0; i < k; i++){
        struct censusTrack * censusT = &(censusTrackArray[censusTrackIndex[i]]);
        popFactors[i] = getRawPopulationFactor(censusT->numOfPeople);
        jobFactors[i] = getJobFactor(censusT->numOfEmployedPeople);
        rentFactors[i] = getRawRentFactor(censusT->medianRent);
        utilityFactors[i] = getUtilityAccessFactor(censusT->utilityCoverage);
        printf("utility coverage %f\n",censusT->utilityCoverage);
        dis[i] = sqrt((x-censusT->centerX)*(x-censusT->centerX)+(y-censusT->centerY)*(y-censusT->centerY));
        if(dis[i] < squareSize/2) dis[i] = squareSize/2;
        printf("distance is %f for point %f, %f\n",dis[i],x,y);
    }

    
    popFactors[k] = 0;
    jobFactors[k] = 0;
    rentFactors[k] = 0;
    utilityFactors[k] = 0;


    double total = 0;
    for(int i = 0; i < k; i++){
        dis[i] = 1/pow(dis[i],u);
    }
    for(int i = 0; i < k; i++){
        double factor = dis[i];
        total += factor;
    }
    //total = sqrt(total);
    printf("total distance is %f for point %f, %f\n",total,x,y);


    for(int i = 0; i < k; i++){
        dis[i] = dis[i]/total;
        printf("normalized distance is %f , for %f, %f\n",dis[i],x,y);
    }


    for(int i = 0; i < k; i++){
        double factor = dis[i];
        printf("factors - pop : %f  - job : %f  - rent : %f  - utility : %f \n",popFactors[i],jobFactors[i],rentFactors[i],utilityFactors[i]);
        printf("normalized factor is %f , for %f, %f\n",factor,x,y);
        popFactors[k] += popFactors[i]*factor;
        jobFactors[k] += jobFactors[i]*factor;
        rentFactors[k] += rentFactors[i]*factor;
        utilityFactors[k] += utilityFactors[i]*factor;
    }


    struct mapNode * node = &(myMap->nodes[NODE_AT(x,y)]);
    //node->utilityAccessFactor = utilityFactors[k];
    node->populationFactor = popFactors[k];
    node->jobQuantity = jobFactors[k];
    node->rentFactor = rentFactors[k];


    free(censusTrackIndex);
    free(popFactors);
    free(jobFactors);
    free(rentFactors);
    free(utilityFactors);
    free(employmentFactors);
    free(dis);
}

//****************IMPLEMENT EMPLOYMENT ACESS METRIC */
//does final processing of map nodes
// this means averaging population factors
//also we
void processRawMapData(struct map * myMap){
    printf("Proces Raw Map Data\n");
    double * newPopFactors = (double *)malloc(sizeof(double)*xSquares*ySquares);
    for(int i = 0; i < xSquares*ySquares; i++){
        struct mapNode * node = &(myMap->nodes[i]);
        int xPos0 = i%xSquares;
        int yPos0 = i/xSquares;
        //perform population averaging
        double populationAverage = 0;
        int numSquares = 0;
        for(int kx = -1; kx < 2; kx++){
            int xPos = xPos0 + kx;
            if((xPos < 0 )||(xPos >= xSquares)) continue;
            for(int ky = -1; ky < 2; ky++){
                int yPos = yPos0 + ky;
                if((yPos < 0 )||(yPos >= ySquares)) continue;
                int index = NODE_AT(kx*squareSize,ky*squareSize);
                struct mapNode * otherNode = &(myMap->nodes[i]);
                numSquares ++;
                populationAverage += otherNode->populationFactor;
            }

        }
        populationAverage /= numSquares;
        newPopFactors[i] = populationAverage;


        //calculate employment access for this square
        node->employmentAccess = getEmploymentFactor(myMap, xPos0*squareSize, yPos0*squareSize);
    }

    for(int i = 0; i < xSquares*ySquares; i++){
        struct mapNode * node = &(myMap->nodes[i]);
        node->populationFactor = newPopFactors[i];
    }
    

    free(newPopFactors);
}


//populates map
void populateMap(struct map *myMap){

    for(int x = 0; x < xSquares; x++){
        for(int y = 0; y < ySquares; y++){
            populateMapNodeWithRawKNearestCensusData(myMap,2,1,x*squareSize,y*squareSize);
        }
    }

    processRawMapData(myMap);

    normalizeMapNodes(myMap);
}


//***************************IMPLEMENT CENSUS LOADING */
//loads census track data from csv info
void loadCensusTrack(struct map* myMap, char * name){
    /*FILE * fptr;

    fptr = fopen(name,"r");

    if (fptr == NULL) {
        printf("Error opening the file!\n");
        return; 
    }
    fread(&(xSquares),sizeof(int),1,fptr);
    fread(&(ySquares),sizeof(int),1,fptr);
    fread(&(squareSize),sizeof(int),1,fptr);
    for(int i = 0; i < xSquares*ySquares; i++){
        printf("Now reading squares\n");
        struct mapNode * node = &(myMap->nodes[i]);
        fread(&(node->zoneType),sizeof(int),1,fptr);
        fread(&(node->utilityAccessFactor),sizeof(double),1,fptr);
        fread(&(node->populationFactor),sizeof(double),1,fptr);
        fread(&(node->employmentAccess),sizeof(double),1,fptr);
        fread(&(node->jobQuantity),sizeof(double),1,fptr);
        fread(&(node->rentFactor),sizeof(double),1,fptr);
        printNode(node);
    }


    fclose(fptr);
    return;*/

    /*
    double centerX;
    double centerY;
    int numOfPeople;
    int numOfEmployedPeople;
    double utilityCoverage;
    double medianRent;
    */

    struct censusTrack * trackA = (struct censusTrack *)malloc(sizeof(struct censusTrack));
    initCensusTrack(trackA);
    trackA->centerX = 5;
    trackA->centerY = 5;
    trackA->numOfPeople = 500;
    trackA->numOfEmployedPeople = 500;
    trackA->utilityCoverage = 0.75;
    trackA->medianRent = 525;
    addCensusTrack(trackA);


    struct censusTrack * trackB = (struct censusTrack *)malloc(sizeof(struct censusTrack));
    initCensusTrack(trackB);
    trackB->centerX = 25;
    trackB->centerY = 15;
    trackB->numOfPeople = 10000;
    trackB->numOfEmployedPeople = 5000;
    trackB->utilityCoverage = 0.95;
    trackB->medianRent = 925;
    addCensusTrack(trackB);



    struct censusTrack * trackC = (struct censusTrack *)malloc(sizeof(struct censusTrack));
    initCensusTrack(trackC);
    trackC->centerX = 75;
    trackC->centerY = 75;
    trackC->numOfPeople = 100;
    trackC->numOfEmployedPeople = 70;
    trackC->utilityCoverage = 0.45;
    trackC->medianRent = 425;
    addCensusTrack(trackC);
}

void printBusStop(int index);


//***IMPLEMENT CORRECT LOADING */
//load zoning info
void loadZoning(struct map* myMap, char * name){
    FILE * fptr;

    fptr = fopen(name,"r");

    if (fptr == NULL) {
        printf("Error opening the file!\n");
        return; 
    }
    int xSq;
    int ySq;
    double sqSize;
    fread(&(xSq),sizeof(int),1,fptr);
    fread(&(ySq),sizeof(int),1,fptr);
    fread(&(sqSize),sizeof(int),1,fptr);
    for(int i = 0; i < xSq*ySq; i++){
        printf("Now reading square zoning\n");
        struct mapNode * node = &(myMap->nodes[i]);
        int zoningType = i%7;
        node->zoneType = zoningType;
        printNode(node);
    }
    fclose(fptr);
    return;
}


//represents a bus stop
// populate connectedStops array  with connectd stops
//essentially an N-connected linked list
struct busStop {
    double xPos;
    double yPos;
    int stopNumber;
    struct busStop * * connectedStops;
    int numPossibleNextStops;
};

struct busStop * * busStops;
int busStopLength = 0;

void addBusStop( struct busStop * stop ){

    int squareX = stop->xPos/squareSize+0.5;
    int squareY = stop->yPos/squareSize+0.5;
    //check busStop is not inside existing square with busStop
    for(int i = 0; i < busStopLength; i++){
        int squareXOther = busStops[i]->xPos/squareSize+0.5;
        int squareYOther = busStops[i]->yPos/squareSize+0.5;
        if((squareX==squareXOther)&&(squareY==squareYOther)){
            return;
        }
    }

    struct busStop * * newLsA = (struct busStop * *)(malloc(sizeof(struct busStop *)*(busStopLength+1)));

    for(int i = 0; i < busStopLength; i++){
        newLsA[i] = busStops[i];
    }
    newLsA[busStopLength] = (stop);

    stop->stopNumber = busStopLength;



    if(busStopLength>0)free(busStops);
    busStopLength++;


    busStops = newLsA;


    //printBusStop(busStopLength-1);
}

//initialize a bus stop structure
void initializeBusStop(struct busStop * stop){
    stop->xPos = 0;
    stop->yPos = 0;
    stop->stopNumber = 0;
    stop->numPossibleNextStops = 0;
}


//connects two bus stops
void connectBusStops(struct busStop * stopA,struct busStop * stopB){

    //check they're not already connected
    for(int i = 0; i < stopA->numPossibleNextStops; i++){
        if( stopA->connectedStops[i] == stopB) return;
    }

    struct busStop * * newLsA = (struct busStop * *)(malloc(sizeof(struct busStop *)*(stopA->numPossibleNextStops+1)));
    struct busStop * * newLsB = (struct busStop * *)(malloc(sizeof(struct busStop * )*(stopB->numPossibleNextStops+1)));

    for(int i = 0; i < stopA->numPossibleNextStops; i++){
        newLsA[i] = stopA->connectedStops[i];
    }
    newLsA[stopA->numPossibleNextStops] = (stopB);

    for(int i = 0; i < stopB->numPossibleNextStops; i++){
        newLsB[i] = stopB->connectedStops[i];
    }
    newLsB[stopB->numPossibleNextStops] = (stopA);

    if( stopA->numPossibleNextStops > 0 ){
        free(stopA->connectedStops);
    }

    if( stopB->numPossibleNextStops > 0 ){
        free(stopB->connectedStops);
    }

    stopA->connectedStops = newLsA;
    stopB->connectedStops = newLsB;

    stopA->numPossibleNextStops++;
    stopB->numPossibleNextStops++;
}

//find nearest k census tracks to point, fills retList with K integers, each is an index of the track in the censusTrackArray
void fillNearestKBusStops(int busIndex, int k){

    struct busStop * stop = busStops[busIndex];
    double x = stop->xPos;
    double y = stop->yPos;

    double * nearestDistances = (double * )malloc(sizeof(double)*k);
    int * nearestDistancesIndex = (int * )malloc(sizeof(int)*k);
    double largestDistance;
    int largestIndex;

    double maxPossibleDistance = (xSquares+ySquares)*squareSize;

    for(int i = 0; i < k; i++){
        nearestDistancesIndex[i] = -1;
        nearestDistances[i] = maxPossibleDistance;
    }

    for(int i = 0; i < busStopLength; i++){
        struct busStop * stopB = busStops[i];
        if(stop==stopB) continue;

        double disX = stopB->xPos - x;
        double disY = stopB->yPos - y;
        disX *= disX;
        disY *= disY;
        double totalDis = sqrt(disX+disY);
        largestDistance = nearestDistances[0];
        largestIndex = 0;
        for(int j = 0; j < k; j++){
            if(nearestDistances[j] > largestDistance){
                largestIndex = j;
                largestDistance = nearestDistances[j];
            }
        }
        if(largestDistance > totalDis){
            nearestDistancesIndex[largestIndex] = i;
            nearestDistances[largestIndex] = totalDis;
            printf("new nearest distance of %f at index %i to point %f, %f\n",nearestDistances[largestIndex],nearestDistancesIndex[largestIndex],x,y);
        }
    }

    for(int i = 0; i < k; i++){
        if(nearestDistancesIndex[i]==-1) continue;
        connectBusStops(busStops[nearestDistancesIndex[i]],stop);
    }

}

//bus stop print out
void printBusStop(int index){
    
    struct busStop * stop = busStops[index];
    printf("Bus Stop %i at %f , %f -> ",stop->stopNumber, stop->xPos,stop->yPos);
    for(int j = 0; j < stop->numPossibleNextStops; j++){
        printf("%i + ",stop->connectedStops[j]->stopNumber);
    }
    printf("\n");
}

//generates bus stop adjacency lists
void generateBusStopKAdjacency(int k){
    for(int i = 0; i < busStopLength; i++){
        fillNearestKBusStops(i,k);
        printf("did % i bus stop nearest neigbors: \n",i);
    }
}



//finds weighted path betwene any two points
//busStopScalar adjusts the relative bonus for using bus routes, 0.5 means bus distances cost half of driving distances, (ie 2x bonus)
//
double rec_findBestPath(int currentStopIndex, double currentLength, int recursionNum, double targX, double targY, double lastDistance);
double busStopScalar = 0.5;
//
double findWeightedPathBetweenPoints(double x1, double y1, double x2, double y2){
    double maxDis = (xSquares+ySquares)*squareSize;
    maxDis *= maxDis;
    double minPathDistance = maxDis;

    double disX = (x2 - x1);
    double disY = (y2 - y1);
    double dis = (disX*disX + disY*disY)*busStopScalar;
    double straightDis = dis/busStopScalar;
    minPathDistance = dis;
    

    /*
    struct busStop * * busStopss;
    int busStopLength = 0;
    */
    for(int i = 0; i < busStopLength; i++){
        struct busStop * stop = busStops[i];
        disX = (stop->xPos - x1);
        disY = (stop->yPos - y1);
        double disCompLast = (disX*disX + disY*disY);
        if(disCompLast > straightDis) continue;
        dis = disCompLast*busStopScalar;


        dis = rec_findBestPath(i,dis,0,x2,y2,maxDis);
        if(minPathDistance > dis) minPathDistance = dis;

        //printf("analyzed starting at stop % i with distance %f \n",i,dis);
    }


    printf("connecting %f, %f to %f, %f with min distance %f: \n", x1, y1, x2, y2, minPathDistance);

    return minPathDistance;
}


//recursive function used to traverse the bus paths
int maxRecursions = 10;
double rec_findBestPath(int currentStopIndex, double currentLength, int recursionNum, double targX, double targY, double lastDistance){

    double bestPath = (xSquares+ySquares)*squareSize;
    struct busStop * stop = busStops[currentStopIndex];
    
    double disX = (stop->xPos - targX);
    double disY = (stop->yPos - targY);
    double dis = (disX*disX + disY*disY);

    bestPath = currentLength + dis;

    if(recursionNum < maxRecursions){
        for(int i = 0; i < stop->numPossibleNextStops; i++){

            
            double disCompLastX = (stop->connectedStops[i]->xPos - targX);
            double disCompLastY = (stop->connectedStops[i]->yPos - targY);
            double disCompLast = (disCompLastX*disCompLastX + disCompLastY*disCompLastY);
            if(disCompLast > lastDistance) continue;

            disX = (stop->xPos - stop->connectedStops[i]->xPos);
            disY = (stop->yPos - stop->connectedStops[i]->yPos);
            dis = (disX*disX + disY*disY)*busStopScalar;
            dis = rec_findBestPath(stop->connectedStops[i]->stopNumber,currentLength+dis,recursionNum+1,targX,targY,disCompLast);
            if(dis < bestPath){
                bestPath = dis;
            }
        }
    }

    return bestPath;
}

//calulates exployment factor for square
double getEmploymentFactor( struct map * myMap, double x, double y){
    double total = 0;
    for(int i = 0; i < xSquares*ySquares;i++){
        if((((int)(x/squareSize)) + ((int)(y/squareSize))*xSquares)==i) continue;
        struct mapNode * node = &(myMap->nodes[i]);

        //check if commercial or industrially zoned
        if((node->zoneType == 0)||(node->zoneType == 2)||(node->zoneType ==3))continue;

        double dis = findWeightedPathBetweenPoints((i%xSquares)*squareSize,(i/xSquares)*squareSize,x,y);
        if(dis < squareSize/40) dis = squareSize/40;
        total += node->jobQuantity/dis;
    }
    return total;
}

//****IMPLEMENT CORRECT LOADING
//load utility info
void loadUtility(struct map* myMap, char * name){
    FILE * fptr;

    fptr = fopen(name,"r");

    if (fptr == NULL) {
        printf("Error opening the file!\n");
        return; 
    }
    int xSq;
    int ySq;
    double sqSize;
    fread(&(xSq),sizeof(int),1,fptr);
    fread(&(ySq),sizeof(int),1,fptr);
    fread(&(sqSize),sizeof(int),1,fptr);
    for(int i = 0; i < xSq*ySq; i++){
        printf("Now reading square utility\n");
        struct mapNode * node = &(myMap->nodes[i]);
        double utilityCoverage = RANDOM;
        node->utilityAccessFactor = utilityCoverage;
        printNode(node);
    }
    fclose(fptr);
    return;
}


int main(){

    censusTrackArray = (struct censusTrack *)malloc(sizeof(struct censusTrack));

    struct map myMap;

    initMap(&myMap);

    printMap(&myMap);

    loadCensusTrack(&myMap,"noFile.txt");
    loadZoning(&myMap,"mapOutput.txt");
    loadUtility(&myMap,"mapOutput.txt");

    for(int i = 0; i < 30; i++){
        struct busStop * stop = (struct busStop * )malloc(sizeof(struct busStop));
        initializeBusStop(stop);
        stop->xPos = (RANDOM*squareSize*xSquares);
        stop->yPos = (RANDOM*squareSize*ySquares);
        addBusStop(stop);
    }
    generateBusStopKAdjacency(4);
    for(int i = 0; i < busStopLength; i++)printBusStop(i);


    
    populateMap(&myMap);

    saveMap(&myMap,"mapOutput.txt");

    loadMap(&myMap,"mapOutput.txt");

    printMap(&myMap);

    return 0;
}


