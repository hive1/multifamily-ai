#include <stdio.h>
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

//holds starting positon for map
double startingLat = -75.860;
double startingLon = 42.180;
double latMeterScale = 111139.0;
double lonMeterScale = 111320.0;

#define getMeters_Lon(a) (((double)(a))*lonMeterScale)
#define getMeters_Lat(a) (((double)(a))*latMeterScale)

#define getGridX_Lon(a) ((getMeters_Lon(a)-startingLon*lonMeterScale)/squareSize)
#define getGridY_Lat(a) ((getMeters_Lat(a)-startingLat*latMeterScale)/squareSize)

#define getGridX_Meters(a) (((double)(a)-startingLon*lonMeterScale)/squareSize)
#define getGridY_Meters(a) (((double)(a)-startingLat*latMeterScale)/squareSize)

#define getMeters_GridX(a) ((a)*squareSize + startingLon*lonMeterScale)
#define getMeters_GridY(a) ((a)*squareSize + startingLat*latMeterScale)

#define getLat_GridX(a) ((a)*squareSize/lonMeterScale + startingLon)
#define getLon_GridY(a) ((a)*squareSize/latMeterScale + startingLat)


//map struct holds an array of map nodes
int xSquares = 419;
int ySquares = 361;
//size of square in meter;
double squareSize = 25;
struct map{
    struct mapNode * nodes;
};



#define NODE_AT(x,y)  (((int)(y)*xSquares+(int)(x)))

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
        printf("Error opening the file: %s \n",name);
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
        printf("Error opening the file: %s \n",name);
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
    int totalNodes = xSquares * ySquares;
    int numberOfNormalizedParams = 5;

    for (int k = 0; k < numberOfNormalizedParams; k++) {
        // Pointer to the target parameter of node 0
        double *firstVal = ((double *)&(myMap->nodes[0].utilityAccessFactor)) + k;
        double maxFactor = *firstVal;
        double minFactor = *firstVal;

        // 1. Find min and max across all nodes (0 to totalNodes - 1)
        for (int i = 0; i < totalNodes; i++) {
            double *valPtr = ((double *)&(myMap->nodes[i].utilityAccessFactor)) + k;
            double value = *valPtr;
            if (value > maxFactor) maxFactor = value;
            if (value < minFactor) minFactor = value;
        }

        // 2. Normalize all nodes (prevent division by zero if all values are equal)
        double range = maxFactor - minFactor;
        for (int i = 0; i < totalNodes; i++) {
            double *valPtr = ((double *)&(myMap->nodes[i].utilityAccessFactor)) + k;
            if (range > 0.000001) {
                *valPtr = (*valPtr - minFactor) / range;
            } else {
                *valPtr = 0.0;
            }
        }
    }
}

//represents the data from a single census track
struct censusTrack{
    long double centerX;
    long double centerY;
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
void findNearestKCensusTracks(int * retList, int k, int x, int y){

    double * nearestDistances = (double * )malloc(sizeof(double)*k);
    int * nearestDistancesIndex = (int * )malloc(sizeof(int)*k);
    double largestDistance;
    int largestIndex;

    double maxPossibleDistance = (xSquares+ySquares)*squareSize;

    for(int i = 0; i < k; i++){
        nearestDistancesIndex[i] = 0;
        nearestDistances[i] = maxPossibleDistance;
    }

    double xPos = getMeters_GridX(x);
    double yPos = getMeters_GridY(y);

    for(int i = 0; i < censusTrackLength; i++){
        struct censusTrack * track = &(censusTrackArray[i]);
        double disX = track->centerX - xPos;
        double disY = track->centerY - yPos;
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


double getEmploymentFactor( struct map* myMap, int x, int y);


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
void populateMapNodeWithRawKNearestCensusData(struct map* myMap, int k, int u, int x, int y){
    int * censusTrackIndex = (int *)malloc(sizeof(int)*k);
    double * popFactors = (double *)malloc(sizeof(double)*(k+1));
    double * jobFactors = (double *)malloc(sizeof(double)*(k+1));
    double * rentFactors = (double *)malloc(sizeof(double)*(k+1));
    double * utilityFactors = (double *)malloc(sizeof(double)*(k+1));
    double * employmentFactors = (double *)malloc(sizeof(double)*(k+1));
    double * dis = (double *)malloc(sizeof(double)*(k));
    findNearestKCensusTracks(censusTrackIndex,k,x,y);
    long double xPos = getMeters_GridX(x);
    long double yPos = getMeters_GridY(y);


    for(int i = 0; i < k; i++){
        struct censusTrack * censusT = &(censusTrackArray[censusTrackIndex[i]]);
        popFactors[i] = getRawPopulationFactor(censusT->numOfPeople);
        jobFactors[i] = getJobFactor(censusT->numOfEmployedPeople);
        rentFactors[i] = getRawRentFactor(censusT->medianRent);
        utilityFactors[i] = getUtilityAccessFactor(censusT->utilityCoverage);
        printf("median rent %f\n",censusT->medianRent);
        dis[i] = sqrt((xPos-censusT->centerX)*(xPos-censusT->centerX)+(yPos-censusT->centerY)*(yPos-censusT->centerY));
        if(dis[i] < squareSize/2) dis[i] = squareSize/2;
        printf("distance is %f for point %Lf, %Lf - from point %Lf, %Lf\n",dis[i],xPos,yPos,censusT->centerX,censusT->centerY);
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
    printf("total distance is %f for point %Lf, %Lf\n",total,xPos,yPos);


    for(int i = 0; i < k; i++){
        dis[i] = dis[i]/total;
        //printf("normalized distance is %f , for %f, %f\n",dis[i],x,y);
    }


    for(int i = 0; i < k; i++){
        double factor = dis[i];
        //printf("factors - pop : %f  - job : %f  - rent : %f  - utility : %f \n",popFactors[i],jobFactors[i],rentFactors[i],utilityFactors[i]);
        //printf("normalized factor is %f , for %f, %f\n",factor,x,y);
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


void fillBusStopWeightedDistances();

//****************IMPLEMENT EMPLOYMENT ACESS METRIC */
//does final processing of map nodes
// this means averaging population factors
//also we
void processRawMapData(struct map * myMap){
    printf("Proces Raw Map Data\n");
    fillBusStopWeightedDistances();
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
                int index = NODE_AT(xPos,yPos);
                struct mapNode * otherNode = &(myMap->nodes[index]);
                numSquares ++;
                populationAverage += otherNode->populationFactor;
            }

        }
        populationAverage /= numSquares;
        newPopFactors[i] = populationAverage;


        //calculate employment access for this square
        node->employmentAccess = getEmploymentFactor(myMap, xPos0, yPos0);
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
            populateMapNodeWithRawKNearestCensusData(myMap,2,1,x,y);
        }
    }

    printMap(myMap);

    processRawMapData(myMap);

    printMap(myMap);

    normalizeMapNodes(myMap);

    printMap(myMap);
}


//***************************IMPLEMENT CENSUS LOADING */
//loads census track data from csv info
void loadCensusData(struct map* myMap, char * name){
    /*
    int numOfPeople;
    int numOfEmployedPeople;
    double utilityCoverage;
    double medianRent;
    */
    FILE * fptr = fopen(name, "r");

    if (fptr == NULL) {
        printf("Error opening the file!!!!\n");
        return; 
    }

    char buffer[256];
    int lineIndex = 0;

    while (fgets(buffer, sizeof(buffer), fptr) != NULL) {
        // Skip header row
        if (lineIndex == 0) {
            lineIndex++;
            continue;
        }

        long long id;
        int totalPop;
        int employed;
        int rent;
        int rBurden;
        double xPos;
        double yPos;

        /*
         * Format breakdown:
         * 1400000US%lld -> matches prefix and parses ID into long long
         * ,%*[^,],      -> skips the name field between commas (e.g. Census Tract 1; Broome...)
         * %d            -> parses Total Population (e.g. 2859)
         * ,%*d,         -> skips Rent Burdened column (e.g. 118)
         * %d            -> parses Pop Employed (e.g. 1757)
         * ,%d           -> parses MedianRent (e.g. 834)
         */
        int matched = sscanf(buffer, "1400000US%lld,%*[^,],%d,%d,%d,%d,%lf,%lf\n",
                             &id, &totalPop, &rBurden, &employed, &rent,&yPos, &xPos );

        //xPos = getGridX_Lon(xPos);
        //yPos = getGridY_Lat(yPos);

        if (matched == 7) {
            struct censusTrack * track = (struct censusTrack *)malloc(sizeof(struct censusTrack));
            //track->id = id;
            track->centerX = getMeters_Lon(xPos);
            track->centerY = getMeters_Lat(yPos);
            track->numOfPeople = totalPop;
            track->numOfEmployedPeople = employed;
            track->medianRent = rent;

            addCensusTrack(track);

            printf("Parsed -> [%f,%f]=(%Lf,%Lf), Pop: %d | Employed: %d | Rent: %f\n",
                   xPos,yPos,track->centerX,track->centerY,track->numOfPeople, track->numOfEmployedPeople, track->medianRent);

            // TODO: Link `track` into your myMap structure or array here

        } else {
            printf("Failed to parse line %d: %s", lineIndex + 1, buffer);
        }

        lineIndex++;
    }

    fclose(fptr);
    return;

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
void loadZoning(struct map* myMap, char * name) {
    FILE * fptr = fopen(name, "r");

    if (fptr == NULL) {
        printf("Error opening the file: %s\n", name);
        return; 
    }

    char buffer[256];
    int lineIndex = 0;

    while (fgets(buffer, sizeof(buffer), fptr) != NULL) {
        if (lineIndex == 0) {
            int width;
            if (sscanf(buffer, "%d", &width) == 1) {
                // If you need to store this in xSquares or myMap, do it here:
                // xSquares = width;
                printf("Read width: %d\n", width);
            }
        } 
        else if (lineIndex == 1) {
            int height;
            if (sscanf(buffer, "%d", &height) == 1) {
                // If you need to store this in ySquares or myMap, do it here:
                // ySquares = height;
                printf("Read height: %d\n", height);
            }
        } 
        else {
            int nodeIndex = lineIndex - 2;

            if (nodeIndex >= xSquares * ySquares) {
                break;
            }

            int zoneType;
            // %*d and %*lf skip the first two ints and two doubles
            if (sscanf(buffer, "%*d,%*d,%*lf,%*lf,%d", &zoneType) == 1) {
                myMap->nodes[nodeIndex].zoneType = zoneType;
            } else {
                printf("Failed to parse zone type from line %d: %s", lineIndex + 1, buffer);
            }
        }

        lineIndex++;
    }

    fclose(fptr);
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
    double * distanceFields;
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
    stop->distanceFields = (double *)malloc(sizeof(double)*xSquares*ySquares);
}

//delete a bus stop structure
void deleteBusStop(struct busStop * stop){
    if(stop->distanceFields!=0)
    free(stop->distanceFields);
    stop->distanceFields = 0;
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


    //printf("connecting %f, %f to %f, %f with min distance %f: \n", x1, y1, x2, y2, minPathDistance);

    return minPathDistance;
}


//recursive function used to traverse the bus paths
int maxRecursions = 2;
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

//fills best weighted paths for all bus stops
void fillBusStopWeightedDistances(){

    double maxDistance = (xSquares+ySquares)*squareSize;

    for(int i = 0; i < busStopLength; i++){
        struct busStop * thisStop = busStops[i];
        for(int x = 0; x < xSquares; x++){
            for(int y = 0; y < ySquares; y++){
                int index = NODE_AT(x,y);
                thisStop->distanceFields[index] = maxDistance;
            }
        }
    }

    for(int i = 0; i < busStopLength; i++){
        struct busStop * thisStop = busStops[i];
        double thisX = thisStop->xPos;
        double thisY = thisStop->yPos;
        for(int x = 0; x < xSquares; x++){

            double otherX = getMeters_GridX(x);

            for(int y = 0; y < ySquares; y++){

                double otherY = getMeters_GridY(y);

                int index = NODE_AT(x,y);

                thisStop->distanceFields[index] = findWeightedPathBetweenPoints((i%xSquares)*squareSize,(i/xSquares)*squareSize,otherX,otherY);
            }
        }
    }
}

//calculates exployment factor for square
double getEmploymentFactor( struct map * myMap, int x, int y){

    int xPos = getMeters_GridX(x);
    int yPos = getMeters_GridY(y);

    double total = 0;
    for(int i = 0; i < xSquares*ySquares;i++){

        //printf("getting employment factor for node %i\n",i);

        if(NODE_AT(x,y)==i) continue;
        struct mapNode * node = &(myMap->nodes[i]);

        //check if commercial or industrially zoned
        if(((node->zoneType == 0)||(node->zoneType == 1)))continue;

        long double otherX = getMeters_GridX(i%xSquares);
        long double otherY = getMeters_GridY(i/xSquares);

        //now get straightline distance
        double straightDisX = otherX - xPos;
        double straightDisY = otherY - yPos;
        straightDisX *= straightDisX;
        straightDisY *= straightDisY;
        double straightDisSq = straightDisX + straightDisY;

        double minDistance = straightDisSq;
        //now we loop through the busStops and decide if this is a faster route
        for(int k = 0; k < busStopLength; k++){

            struct busStop * stop = busStops[k];

            long double stopX = stop->xPos;
            long double stopY = stop->yPos;

            //now get straightline distance
            double stopStraightDisX = stopX - xPos;
            double stopStraightDisY = stopY - yPos;
            stopStraightDisX *= stopStraightDisX;
            stopStraightDisY *= stopStraightDisY;
            double stopStraightDisSq = stopStraightDisX + stopStraightDisY;
            stopStraightDisSq += stop->distanceFields[i];
            if(minDistance > stopStraightDisSq){
                minDistance = stopStraightDisSq;
            }
        }

        //printf("straight distance : %f, fastest distance : %f\n",straightDisSq,minDistance);

        double dis = minDistance;
        if(dis < squareSize/40) dis = squareSize/40;
        total += node->jobQuantity/dis;
    }

    printf("connecting %d, %d has total employment distance %f: \n", x, y, total);
    return total;
}

//****IMPLEMENT CORRECT LOADING
//load utility info
void loadUtility(struct map* myMap, char *name) {
    FILE *fptr = fopen(name, "r");

    if (fptr == NULL) {
        printf("Error opening the file: %s\n", name);
        return;
    }

    char buffer[256];
    int lineIndex = 0;

    // Read the file line by line
    while (fgets(buffer, sizeof(buffer), fptr) != NULL) {
        // Skip the header row (GridID,PowerDistM,...)
        if (lineIndex == 0) {
            lineIndex++;
            continue;
        }

        int gridID;
        double powerDist, sewerDist, waterDist;
        double invPower, invSewer, invWater;
        double utilityScore;

        // Parse all 8 comma-separated values from the line
        int parsed = sscanf(buffer, "%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                            &gridID,
                            &powerDist,
                            &sewerDist,
                            &waterDist,
                            &invPower,
                            &invSewer,
                            &invWater,
                            &utilityScore);

        if (parsed == 8) {
            int nodeIndex = gridID - 1;

            // Bounds check
            if (nodeIndex >= 0 && nodeIndex < xSquares * ySquares) {
                struct mapNode *node = &(myMap->nodes[nodeIndex]);
                
                // Store the utility score into the node
                // (Adjust the field name to match your struct definition, e.g., utility or utilityScore)
                node->utilityAccessFactor = utilityScore;

                //printf("GridID: %d, Utility: %f\n", gridID, utilityScore);
            }
        } else {
            printf("Failed to parse line %d: %s", lineIndex + 1, buffer);
        }

        lineIndex++;
    }

    fclose(fptr);
}




//final Q value calculation
double maxBeta [5] ={0.2,0.5,0.1,0.4,0.4};
double beta [5]  = {0.1,0.1,0.05,0.05,0.05};
double alpha [5] = {1,1,1,1,1};

void calcQualityFactor(struct mapNode * node){
    double q1 = pow((maxBeta[0] + node->utilityAccessFactor),alpha[0]);
    double q2 = pow((maxBeta[1] + node->populationFactor),alpha[1]);
    double q3 = pow((maxBeta[2] + node->employmentAccess),alpha[2]);
    double q4 = pow((maxBeta[3] + node->jobQuantity),alpha[3]);
    double q5 = pow((maxBeta[4] + node->rentFactor),alpha[4]);

    double rawOut = q1*q2*q3*q4*q5;
    node->quality = 0.5 + tanh(rawOut)/2;
}

//fill q values for table
void fillQValues(struct map *myMap){
    for(int i = 0; i < xSquares*ySquares;i++){
        struct mapNode * node = &(myMap->nodes[i]);
        calcQualityFactor(node);
    }
}

//save weights function
void saveWeights(char * name){
    FILE * fptr;

    fptr = fopen(name,"w");

    if (fptr == NULL) {
        printf("Error opening the file: %s \n",name);
        return; 
    }

    for(int i = 0; i <5; i++){
        fwrite(&(beta[i]), 1, sizeof(double), fptr);
    }

    for(int i = 0; i <5; i++){
        fwrite(&(alpha[i]), 1, sizeof(double), fptr);
    }

    fclose(fptr);
    return;
}

//load weights function
void loadWeights( char * name){
    FILE * fptr;

    fptr = fopen(name,"r");

    if (fptr == NULL) {
        printf("Error opening the file: %s \n",name);
        return; 
    }

    for(int i = 0; i <5; i++){
        fread(&(beta[i]), 1, sizeof(double), fptr);
    }

    for(int i = 0; i <5; i++){
        fread(&(alpha[i]), 1, sizeof(double), fptr);
    }


    fclose(fptr);
    return;
}

//print weights function
void printWeights(){

    printf("Weights:\n");
    printf("Constants - \n");
    for(int i = 0; i <5; i++){
        printf(" %f \n",beta[i]);
    }

    printf("exponents - \n");
    for(int i = 0; i <5; i++){
        printf(" %f \n",alpha[i]);
    }

    return;
}

double tempStore [15];
struct mapNode * lastDerivativeNode = 0;

//calculates derivatives of weight, index is 0-5, type, is 0: alpha, 1: beta
double calculateDerivative(double expected, struct mapNode * node, int index, int type){

    if(lastDerivativeNode !=node){
         tempStore[0] = (maxBeta[0] + node->utilityAccessFactor);
         tempStore[1] = (maxBeta[1] + node->populationFactor);
         tempStore[2] = (maxBeta[2] + node->employmentAccess);
         tempStore[3] = (maxBeta[3] + node->jobQuantity);
         tempStore[4] = (maxBeta[4] + node->rentFactor);

         tempStore[5] = pow(tempStore[0],alpha[0]);
         tempStore[6] = pow(tempStore[1],alpha[1]);
         tempStore[7] = pow(tempStore[2],alpha[2]);
         tempStore[8] = pow(tempStore[3],alpha[3]);
         tempStore[9] = pow(tempStore[4],alpha[4]);

         tempStore[10] = pow(tempStore[0],alpha[0]-1);
         tempStore[11] = pow(tempStore[1],alpha[1]-1);
         tempStore[12] = pow(tempStore[2],alpha[2]-1);
         tempStore[13] = pow(tempStore[3],alpha[3]-1);
         tempStore[14] = pow(tempStore[4],alpha[4]-1);
    }

    lastDerivativeNode = node;
    
    double rawOut = tempStore[10]*tempStore[11]*tempStore[12]*tempStore[13]*tempStore[14];
    double actual = 0.5 + tanh(rawOut)/2;
    double dif = expected-actual;
    double firstTerm = 1;
    for(int i = 0; i < 5; i++){
        if(i==index) continue;
        firstTerm *= tempStore[4+i];
    }
    if(type==0){//exponential
        firstTerm *= tempStore[4+index]*log(tempStore[index]);
    }else{//constant
        firstTerm *= tempStore[9+index]*(beta[index]-1);
    }
    double secTerm = 1/cosh(firstTerm);

    return dif*secTerm*secTerm;
}

//
double constLearningRate = 0.001;
double expLearningRate = 0.004;
void trainSquare(double expected, struct mapNode * node, double scalarEffect){
    double changeVal = 0;
    for(int i = 0; i < 5; i++){
        changeVal = scalarEffect*expLearningRate*calculateDerivative(expected,node,i,0);
        alpha[i] -= changeVal;
        if(alpha[i] < 0)alpha[i] = 0;
    }

    for(int i = 0; i < 5; i++){
        changeVal = scalarEffect*constLearningRate*calculateDerivative(expected,node,i,1);
        beta[i] -= changeVal;
        if(beta[i] > maxBeta[i])beta[i] = maxBeta[i];
        if(beta[i] < 0)beta[i] = 0;
    }
}

//trains on building a new building in spot
void trainOnNewBuilding(struct map* myMap, double buildingX, double buildingY){
    int squareX = buildingX/squareSize;
    int squareY = buildingY/squareSize;
    int squareIndex = squareY*xSquares + squareX;

    double negativeValue = 1/(1000*(xSquares*ySquares - 1));

    for(int i = 0; i < xSquares*ySquares;i++){
        struct mapNode * node = &(myMap->nodes[i]);
        if(i==squareIndex) trainSquare(1.0,node,1.0);
        else trainSquare(0.0,node,negativeValue);
        fillQValues(myMap);
    }
}

//get error against building at a location
double getBuildingError(struct map* myMap, double buildingX, double buildingY){
    int squareX = buildingX/squareSize;
    int squareY = buildingY/squareSize;
    int squareIndex = squareY*xSquares + squareX;

    double negativeValue = 1/(xSquares*ySquares - 1);

    double totalError = 0;

    for(int i = 0; i < xSquares*ySquares;i++){
        struct mapNode * node = &(myMap->nodes[i]);
        double expected;
        double scalar = 1;
        if(i==squareIndex){
            expected = 1.0;
        } 
        else {
            expected = 0.0;
            scalar = negativeValue;
        }

        double error = expected - node->quality;
        error *= error;
        error *= scalar;
        totalError += error;
    }
    return totalError;
}

//load busStops
void loadBusStops(struct map* myMap, char *name) {
    FILE *fptr = fopen(name, "r");

    if (fptr == NULL) {
        printf("Error opening the file: %s\n", name);
        return;
    }

    char buffer[256];
    int lineIndex = 0;

    // Read the file line by line
    while (fgets(buffer, sizeof(buffer), fptr) != NULL) {
        // Skip the header row (GridID,PowerDistM,...)
        if (lineIndex == 0) {
            lineIndex++;
            continue;
        }

        int gridID;
        double powerDist, sewerDist, waterDist;
        double invPower, invSewer, invWater;
        double utilityScore;

        // Parse all 8 comma-separated values from the line
        int parsed = sscanf(buffer, "%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                            &gridID,
                            &powerDist,
                            &sewerDist,
                            &waterDist,
                            &invPower,
                            &invSewer,
                            &invWater,
                            &utilityScore);

        if (parsed == 8) {
            int nodeIndex = gridID - 1;

            // Bounds check
            if (nodeIndex >= 0 && nodeIndex < xSquares * ySquares) {
                struct mapNode *node = &(myMap->nodes[nodeIndex]);
                
                // Store the utility score into the node
                // (Adjust the field name to match your struct definition, e.g., utility or utilityScore)
                node->utilityAccessFactor = utilityScore;

                //printf("GridID: %d, Utility: %f\n", gridID, utilityScore);
            }
        } else {
            printf("Failed to parse line %d: %s", lineIndex + 1, buffer);
        }

        lineIndex++;
    }

    fclose(fptr);
}


int main(){

    censusTrackArray = (struct censusTrack *)malloc(sizeof(struct censusTrack));

    struct map myMap;

    initMap(&myMap);

    printMap(&myMap);

    loadCensusData(&myMap,"census.csv");
    loadZoning(&myMap,"jack.txt");
    loadUtility(&myMap,"utility.csv");



    printMap(&myMap);

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


    printMap(&myMap);

    fillQValues(&myMap);


    printMap(&myMap);

    saveMap(&myMap,"mapOutput.txt");

    loadMap(&myMap,"mapOutput.txt");

    printMap(&myMap);

    saveWeights("weightsFile.txt");

    loadWeights("weightsFile.txt");

    printWeights();

    double buildingXs [7];
    double buildingYs [7];

    for(int i = 0; i < 7; i++){
        buildingXs[i] = RANDOM*squareSize*xSquares;
        buildingYs[i] = RANDOM*squareSize*ySquares;
    }

    for(int z = 0; z < 2000; z++){
        double avgError = 0;
        for(int i = 0; i < 7; i++){
            avgError = 0;
            for(int k = 0; k < 7; k++){
                avgError += getBuildingError(&myMap,buildingXs[k],buildingYs[k]);
            }
            printf("Average Error: %f\n",avgError);
            printWeights();
            trainOnNewBuilding(&myMap,buildingXs[i],buildingYs[i]);
            printWeights();
        }

        avgError = 0;
        for(int k = 0; k < 7; k++){
            avgError += getBuildingError(&myMap,buildingXs[k],buildingYs[k]);
        }
        printf("Average Error: %f\n",avgError);
    }



    saveMap(&myMap,"mapOutputFinal.txt");
    saveWeights("weightsFinal.txt");


    return 0;
}


