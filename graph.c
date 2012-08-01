#include "graph.h"
#include <stdbool.h>

void print(graph_info g)
{
	for (int i = 0; i < g.n; i++)
	{
		for (int j = 0; j < g.n; j++)
			printf("%d\t", g.distances[g.n*i + j]);
		printf("\n");
	}

	for (int i = 0; i < g.n; i++)
		printf("%d ", g.k[i]);
	printf("\n");
	printf("K: %d, D: %d, S: %d, M: %d\n", g.max_k, g.diameter, g.sum_of_distances, g.m);
}


void floyd_warshall(graph_info g) {
	for (int k = 0; k < g.n; k++) {
		for (int i = 0; i < g.n; i++) {
			for (int j = 0; j < g.n; j++) {
				int dist = g.distances[g.n*i + k] + g.distances[g.n*k + j];
				if(dist < g.distances[g.n*i + j]) {
					g.distances[g.n*i + j] = dist;
				}
			}
		}
	}
}

void fill_dist_matrix(graph_info g)
{
	//Figure out distance from new node to each other node
	for(int i = 0; i < g.n-1; i++)
	{
		if(g.distances[g.n*i + g.n-1] == GRAPH_INFINITY)
		{
			int min_dist = GRAPH_INFINITY;
			for(int j = 0; j < g.n-1; j++)
				if(g.distances[g.n*(g.n-1) + j] == 1 
				   && g.distances[g.n*j + i] + 1 < min_dist)
					 min_dist = g.distances[g.n*j + i] + 1;
			g.distances[g.n*(g.n-1) + i] = g.distances[g.n*i + g.n-1] = min_dist;
		}
	}

	//One iteration of Floyd-Warshall with k = g.n - 1
	for (int i = 0; i < g.n-1; i++) {
		for (int j = i+1; j < g.n-1; j++) {
			int dist = g.distances[g.n*i + g.n-1] + g.distances[g.n*(g.n-1) + j];
			if(dist < g.distances[g.n*i + j]) {
				g.distances[g.n*i + j] = g.distances[g.n*j+i] = dist;
			}
		}
	}

}

void test_fill_dist_matrix(void)
{
	graph_info g;
	int distances[9] = {
		GRAPH_INFINITY, 1, 1,
		1, GRAPH_INFINITY, GRAPH_INFINITY,
		1, GRAPH_INFINITY, GRAPH_INFINITY
	};
	g.distances = distances;
	g.n = 3;
	
	fill_dist_matrix(g);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
			printf("%d, ", g.distances[i*3 + j]);
		printf("\n");
	}
}

int calc_sum(graph_info g)
{
	int sum = 0;
	for(int i = 0; i < g.n; i++)
		for(int j = i+1; j < g.n; j++)
		sum += g.distances[g.n*i+j];
	return sum;
}

int calc_diameter(graph_info g)
{
	int diameter = 0;
	for(int i = 0; i < g.n; i++)
		for(int j = i+1; j < g.n; j++)
			if(diameter < g.distances[g.n*i + j])
				diameter = g.distances[g.n*i + j];
	return diameter;
}


static void init_extended(graph_info input, graph_info *extended)
{
	extended->n = (input.n+1);

	extended->distances = malloc(extended->n*extended->n*sizeof(*extended->distances));
	for(int i = 0; i < input.n; i++)
		for(int j = 0; j < input.n; j++)
			extended->distances[(extended->n)*i + j] = input.distances[(input.n)*i + j];
	for(int i = 0; i < extended->n; i++)
		extended->distances[(extended->n)*i+extended->n-1] =
		extended->distances[(extended->n)*(extended->n-1)+i] = GRAPH_INFINITY;

	extended->k = (int*) malloc(extended->n*sizeof(int));
	for(int i = 0; i < input.n; i++)
		extended->k[i] = input.k[i];
	extended->k[input.n] = 0;

	extended->m = input.m;
	extended->max_k = input.max_k;
}

static void add_edges(graph_info *g, unsigned start)
{
	//setup m and k[n] for the children
	//note that these values will not change b/w each child
	//of this node in the search tree
	g->m++;
	g->k[g->n - 1]++;
	unsigned old_max_k = g->max_k;
	if(g->k[g->n - 1] > g->max_k)
		g->max_k = g->k[g->n - 1];
	
	//if the child has a node of degree greater than MAX_K,
	//don't search it
	if(g->k[g->n - 1] <= MAX_K)
	{
		for(unsigned i = start; i < g->n - 1; i++)
		{
			g->k[i]++;
			
			//same as comment above
			if(g->k[i] <= MAX_K)
			{
				unsigned old_max_k = g->max_k;
				if(g->k[i] > g->max_k)
					g->max_k = g->k[i];
				
				g->distances[g->n*i + (g->n-1)] = g->distances[g->n*(g->n-1) + i] = 1;
				
				add_edges(g, i + 1);
				
				g->distances[g->n*i + (g->n-1)] = g->distances[g->n*(g->n-1) + i] = GRAPH_INFINITY;
				g->max_k = old_max_k;
			}
			g->k[i]--;
		}
	}
	
	//tear down values we created in the beginning
	g->max_k = old_max_k;
	g->m--;
	g->k[g->n - 1]--;

	if(g->k[g->n - 1] > 0)
		print(*g);
}

void add_edges_and_transfer_to_queue(graph_info input)
{
	graph_info extended;
	init_extended(input, &extended);
	
	add_edges(&extended, 0);
}

void test_add_edges(void)
{
	graph_info g;
	int distances [25] = {
		GRAPH_INFINITY, 1, 1, 2, 2,
		1, GRAPH_INFINITY, 2, 1, 3,
		1, 2, GRAPH_INFINITY, 3, 1,
		2, 1, 3, GRAPH_INFINITY, 4,
		2, 3, 1, 4, GRAPH_INFINITY,
	};
	g.distances = distances;
	g.n = 5;
	int g_k[5] = {2, 2, 2, 1 ,1};
	g.k = g_k;
	g.m = 4;
	g.max_k = 2;

	add_edges_and_transfer_to_queue(g);
}
