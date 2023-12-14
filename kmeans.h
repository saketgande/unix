#ifndef KMEANS_H
#define KMEANS_H

typedef struct {
    float x;
    float y;
    // ... include other dimensions/attributes as needed
} Point;

typedef struct {
    Point centroid;
    Point* points;
    int num_points;
} Cluster;

/**
 * Perform k-means clustering on the given dataset.
 * 
 * @param data The dataset of points to be clustered.
 * @param num_points The number of points in the dataset.
 * @param k The number of clusters.
 * @param clusters The output clusters. The caller is responsible for allocating memory for this.
 * @param max_iterations The maximum number of iterations to run the k-means algorithm.
 * 
 * @return 0 for success and non-zero for any failures.
 */
int kmeans_cluster(Point* data, int num_points, int k, Cluster* clusters, int max_iterations);

#endif // KMEANS_H