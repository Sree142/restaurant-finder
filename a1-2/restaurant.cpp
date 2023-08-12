#include "restaurant.h"
// #include <cmath>
// #include <algorithm>

/*
	Sets *ptr to the i'th restaurant. If this restaurant is already in the cache,
	it just copies it directly from the cache to *ptr. Otherwise, it fetches
	the block containing the i'th restaurant and stores it in the cache before
	setting *ptr to it.
*/
void getRestaurant(restaurant* ptr, int i, Sd2Card* card, RestCache* cache) {
	// calculate the block with the i'th restaurant
	uint32_t block = REST_START_BLOCK + i/8;

	// if this is not the cached block, read the block from the card
	if (block != cache->cachedBlock) {
		while (!card->readBlock(block, (uint8_t*) cache->block)) {
			Serial.print("readblock failed, try again");
		}
		cache->cachedBlock = block;
	}

	// either way, we have the correct block so just get the restaurant
	*ptr = cache->block[i%8];
}

// Swaps the two restaurants (which is why they are pass by reference).
void swap(RestDist& r1, RestDist& r2) {
	RestDist tmp = r1;
	r1 = r2;
	r2 = tmp;
}

// Insertion sort to sort the restaurants.
void insertionSort(RestDist restaurants[], int k) {
	// Invariant: at the start of iteration i, the
	// array restaurants[0 .. i-1] is sorted.
	for (int i = 1; i < k+1; ++i) {
		// Swap restaurant[i] back through the sorted list restaurants[0 .. i-1]
		// until it finds its place.
		for (int j = i; j > 0 && restaurants[j].dist < restaurants[j-1].dist; --j) {
			swap(restaurants[j-1], restaurants[j]);
		}
	}
}

int pivot(RestDist restaurants[], int n, int pi) {
	int low = 0;
	int high = n-2;
	swap(restaurants[pi],restaurants[n-1]);

	while (low <= high) {
		if (restaurants[low].dist <= restaurants[n-1].dist){
			low++;
		}
		else if (restaurants[high].dist > restaurants[n-1].dist) {
			high--;
		}
		else {
			swap(restaurants[low],restaurants[high]);
		}
	}
	swap(restaurants[low],restaurants[n-1]);
	return low;
}

void qsort(RestDist restaurants[], int n) {
	if (n > 1) {
		int pi = n/2;
		int new_pi = pivot(restaurants,n,pi);
		qsort(restaurants, new_pi);
		qsort(restaurants + new_pi + 1, n- 1-new_pi);
	}
}

// Computes the manhattan distance between two points (x1, y1) and (x2, y2).
int16_t manhattan(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
	return abs(x1-x2) + abs(y1-y2);
}

/*
	Fetches all restaurants from the card, saves their RestDist information
	in restaurants[], and then sorts them based on their distance to the
	point on the map represented by the MapView.
*/

int createRestaurants(const MapView& mv, RestDist restaurants[], int* currentRating, Sd2Card* card, RestCache* cache){
	restaurant r;
	int j = 0;
	// Serial.println(*currentRating);
	// First get all the restaurants and store their corresponding RestDist information.
	for (int i = 0; i < NUM_RESTAURANTS; ++i) {
		getRestaurant(&r, i, card, cache);
		int rate = max(floor((r.rating + 1)/2), 1);
		// Serial.println(rate);
		if(rate >= *currentRating){
			// Serial.println("ha");
			restaurants[j].index = i;
			restaurants[j].dist = manhattan(lat_to_y(r.lat), lon_to_x(r.lon),
									mv.mapY + mv.cursorY, mv.mapX + mv.cursorX);
			j++;
		}
	}
	// Serial.println(j);
	return j;
}

int getAndSortRestaurants(const MapView& mv, RestDist restaurants[], int* currentRating, int* mode, Sd2Card* card, RestCache* cache) {
	
	uint16_t time;
	int numRests;

	// int j = createRestaurants(mv, restaurants, currentRating, card, cache);
	if(*mode == 0 || *mode == 2){
		numRests = createRestaurants(mv, restaurants, currentRating, card, cache);
		time = millis();
		qsort(restaurants, numRests);
		// Serial.print("QSORT");
		time = millis() - time;
		Serial.print("Qsort running time: ");
		Serial.print(time);
		Serial.println(" ms");
	}

	if(*mode == 1 || *mode == 2){
		numRests = createRestaurants(mv, restaurants, currentRating, card, cache);
		time = millis();
		insertionSort(restaurants, numRests);
		time = millis() - time;
		Serial.print("Insertion sort running time: ");
		Serial.print(time);
		Serial.println(" ms");
	}

	return numRests;
}
