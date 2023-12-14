#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

#define MAX_POINTS 4096
#define MAX_CLUSTERS 32

typedef struct point
{
    float x; // The x-coordinate of the point
    float y; // The y-coordinate of the point
    int cluster; // The cluster that the point belongs to
} point;

int	N;		// number of entries in the data
int k;      // number of centroids
point data[MAX_POINTS];		// Data coordinates
point cluster[MAX_CLUSTERS]; // The coordinates of each cluster center (also called centroid)

void read_data(char *data_file, int numk)
{
    N = 1797;
    k = numk;
    FILE* fp = fopen(data_file, "r");
    if (fp == NULL) {
        perror("Cannot open the file");
        exit(EXIT_FAILURE);
    }
   
    // Initialize points from the data file
    float temp;
    for (int i = 0; i < N; i++)
    {
        fscanf(fp, "%f %f", &data[i].x, &data[i].y);
        data[i].cluster = -1; // Initialize the cluster number to -1
    }
    // printf("Read the problem data!\n");
    // Initialize centroids randomly
    srand(0); // Setting 0 as the random number generation seed
    for (int i = 0; i < k; i++)
    {
        int r = rand() % N;
        cluster[i].x = data[r].x;
        cluster[i].y = data[r].y;
    }
    fclose(fp);
}

int get_closest_centroid(int i, int k)
{
    /* find the nearest centroid */
    int nearest_cluster = -1;
    double xdist, ydist, dist, min_dist;
    min_dist = dist = INT_MAX;
    for (int c = 0; c < k; c++)
    { // For each centroid
        // Calculate the square of the Euclidean distance between that centroid and the point
        xdist = data[i].x - cluster[c].x;
        ydist = data[i].y - cluster[c].y;
        dist = xdist * xdist + ydist * ydist; // The square of Euclidean distance
        //printf("%.2lf \n", dist);
        if (dist <= min_dist)
        {
            min_dist = dist;
            nearest_cluster = c;
        }
    }
    //printf("-----------\n");
    return nearest_cluster;
}

bool assign_clusters_to_points()
{
    bool something_changed = false;
    int old_cluster = -1, new_cluster = -1;
    for (int i = 0; i < N; i++)
    { // For each data point
        old_cluster = data[i].cluster;
        new_cluster = get_closest_centroid(i, k);
        data[i].cluster = new_cluster; // Assign a cluster to the point i
        if (old_cluster != new_cluster)
        {
            something_changed = true;
        }
    }
    return something_changed;
}

void update_cluster_centers()
{
    /* Update the cluster centers */
    int c;
    int count[MAX_CLUSTERS] = { 0 }; // Array to keep track of the number of points in each cluster
    point temp[MAX_CLUSTERS] = { 0.0 };

    for (int i = 0; i < N; i++)
    {
        c = data[i].cluster;
        count[c]++;
        temp[c].x += data[i].x;
        temp[c].y += data[i].y;
    }
    for (int i = 0; i < k; i++)
    {
        cluster[i].x = temp[i].x / count[i];
        cluster[i].y = temp[i].y / count[i];
    }
}

int kmeans(int k)
{
    bool somechange;
    int iter = 0;
    do {
        iter++; // Keep track of number of iterations
        somechange = assign_clusters_to_points();
        update_cluster_centers();
    } while (somechange);
    // printf("Number of iterations taken = %d\n", iter);
    // printf("Computed cluster numbers successfully!\n");
    // printf("Number of clusters: %d \n", k);
}

void write_results()
{
    for (int i = 0; i < N; i++)
    {
        printf("%.2f %.2f %d\n", data[i].x, data[i].y, data[i].cluster);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 5) {
        printf("Usage: %s -f <data_file> -k <number_of_clusters>\n", argv[0]);
        return 1;
    }

    char *data_file = NULL;
    k = 0;  // number of clusters

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            data_file = argv[++i];
        } else if (strcmp(argv[i], "-k") == 0) {
            k = atoi(argv[++i]);
        }
    }

    if (!data_file || k <= 0) {
        printf("Invalid arguments.\n");
        return 1;
    }

    read_data(data_file, k); 
    kmeans(k);
    write_results();
}