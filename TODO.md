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
