#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <strings.h>//to use "bzero"
#include <time.h>//to estimate the runing time
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include "partition.h"
#include "struct.h"

#define NLINKS 100000000 //maximum number of edges of the input graph: used for memory allocation, will increase if needed. //NOT USED IN THE CURRENT VERSION
#define NNODES 10000000 //maximum number of nodes in the input graph: used for memory allocation, will increase if needed
#define HMAX 100 //maximum depth of the tree: used for memory allocation, will increase if needed

//compute the maximum of three unsigned long
inline unsigned long max3(unsigned long a,unsigned long b,unsigned long c){
  a = (a > b) ? a : b;
  return (a > c) ? a : c;
}

//reading the edgelist from file
//NOT USED IN THE CURRENT VERSION
edgelist* readedgelist(char* input){
  unsigned long long e1=NLINKS;
  edgelist *el=malloc(sizeof(edgelist));
  FILE *file;

  el->n=0;
  el->e=0;
  file=fopen(input,"r");
  el->edges=malloc(e1*sizeof(edge));
  while (fscanf(file,"%lu %lu", &(el->edges[el->e].s), &(el->edges[el->e].t))==2) {
    el->n=max3(el->n,el->edges[el->e].s,el->edges[el->e].t);
    if (++(el->e)==e1) {
      e1+=NLINKS;
      el->edges=realloc(el->edges,e1*sizeof(edge));
    }
  }
  fclose(file);
  el->n++;

  el->edges=realloc(el->edges,el->e*sizeof(edge));

  return el;
}

//freeing memory
//NOT USED IN THE CURRENT VERSION
void free_edgelist(edgelist *el){
  free(el->edges);
  free(el);
}

//building the adjacency matrix
//NOT USED IN THE CURRENT VERSION
adjlist* mkadjlist(edgelist* el){
  unsigned long long i;
  unsigned long u,v;
  unsigned long *d=calloc(el->n,sizeof(unsigned long));
  adjlist *g=malloc(sizeof(adjlist));

  g->n=el->n;
  g->e=el->e;

  for (i=0;i<el->e;i++) {
    d[el->edges[i].s]++;
    d[el->edges[i].t]++;
  }

  g->cd=malloc((g->n+1)*sizeof(unsigned long long));
  g->cd[0]=0;
  for (i=1;i<g->n+1;i++) {
    g->cd[i]=g->cd[i-1]+d[i-1];
    d[i-1]=0;
  }

  g->adj=malloc(2*g->e*sizeof(unsigned long));
  for (i=0;i<g->e;i++) {
    u=el->edges[i].s;
    v=el->edges[i].t;
    g->adj[ g->cd[u] + d[u]++ ]=v;
    g->adj[ g->cd[v] + d[v]++ ]=u;
  }

  g->weights = NULL;
  g->totalWeight = 2*g->e;
  g->map=NULL;

  free(d);
  free_edgelist(el);

  return g;
}

//reading the list of edges and building the adjacency array
adjlist* readadjlist(char* input){
  unsigned long n1=NNODES,n2,u,v,i;
  unsigned long *d=calloc(n1,sizeof(unsigned long));
  adjlist *g=malloc(sizeof(adjlist));
  FILE *file;

  g->n=0;
  g->e=0;
  file=fopen(input,"r");//first reading to compute the degrees
  while (fscanf(file,"%lu %lu", &u, &v)==2) {
    g->e++;
    g->n=max3(g->n,u,v);
    if (g->n+1>=n1) {
      n2=g->n+NNODES;
      d=realloc(d,n2*sizeof(unsigned long));
      bzero(d+n1,(n2-n1)*sizeof(unsigned long));
      n1=n2;
    }
    d[u]++;
    d[v]++;
  }
  fclose(file);

  g->n++;
  d=realloc(d,g->n*sizeof(unsigned long));

  g->cd=malloc((g->n+1)*sizeof(unsigned long long));
  g->cd[0]=0;
  for (i=1;i<g->n+1;i++) {
    g->cd[i]=g->cd[i-1]+d[i-1];
    d[i-1]=0;
  }

  g->adj=malloc(2*g->e*sizeof(unsigned long));

  file=fopen(input,"r");//secong reading to fill the adjlist
  while (fscanf(file,"%lu %lu", &u, &v)==2) {
    g->adj[ g->cd[u] + d[u]++ ]=v;
    g->adj[ g->cd[v] + d[v]++ ]=u;
  }
  fclose(file);

  g->weights = NULL;
  g->totalWeight = 2*g->e;
  g->map=NULL;

  free(d);

  return g;
}

//reading the list of edges and building the adjacency array
//NOT USED IN THE CURRENT VERSION
#define BUFFER_SIZE (16 * 1024)

int
read_two_integers(int fd, unsigned long *u, unsigned long *v) {
  static char buf[BUFFER_SIZE];
  static ssize_t how_many = 0;
  static int pos = 0;
  unsigned long node_number=0;
  int readu = 0;

  while (1) {
    while(pos < how_many) {
      if (buf[pos] == ' ') {
	*u = node_number;
	readu=1;
	node_number = 0;
	pos++;
      } else if (buf[pos] == '\n') {
	*v = node_number;
	node_number = 0;
	readu=0;
	pos++;
	return 2;
      } else {
	node_number = node_number * 10 + buf[pos] - '0';
	pos++;
      }
    }

    how_many = read (fd, buf, BUFFER_SIZE);
    pos = 0;
    if (how_many == 0) {
      if(readu==1) {
	*v = node_number;
	return 2;
      }	    
      return 0;;
    }
  } 
}

adjlist *
readadjlist_v2(char* input_filename){
  unsigned long n1 = NNODES, n2, u, v, i;
  unsigned long *d = calloc(n1,sizeof(unsigned long));
  adjlist *g = malloc(sizeof(adjlist));

  g->n = 0;
  g->e = 0;

  // first read of the file to compute degree of each node
  int fd = open(input_filename, O_RDONLY);
  posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
  while (read_two_integers(fd, &u, &v) == 2) {
    g->e++;
    g->n = max3(g->n, u, v);
    if (g->n + 1 >= n1) {
      n2 = g->n+NNODES;
      d = realloc(d, n2 * sizeof(unsigned long));
      bzero(d + n1, (n2 - n1) * sizeof(unsigned long));
      n1 = n2;
    }
    d[u]++;
    d[v]++;
  }
  close(fd);

  g->n++;
  d = realloc(d, g->n * sizeof(unsigned long));

  // computation of cumulative degrees
  g->cd = malloc((g->n + 1) * sizeof(unsigned long long));
  g->cd[0] = 0;
  for (i = 1; i < g->n + 1; i++) {
    g->cd[i] = g->cd[i - 1] + d[i - 1];
    d[i - 1] = 0;
  }

  g->adj = malloc(2 * g->e * sizeof(unsigned long));

  // second read to create adjlist
  fd = open(input_filename, O_RDONLY);
  posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
  while (read_two_integers(fd, &u, &v) == 2) {
    g->adj[g->cd[u] + d[u]] = v;
    d[u]++;
    g->adj[g->cd[v] + d[v]] = u;
    d[v]++;
  }
  close(fd);

  g->weights = NULL;
  g->totalWeight = 2*g->e;
  g->map = NULL;

  free(d);

  return g;
}

//freeing memory
void free_adjlist(adjlist *g){
  free(g->cd);
  free(g->adj);
  free(g->weights);
  free(g->map);
  free(g);
}


//main function
int main(int argc,char** argv){
  adjlist* g;
  unsigned long *part;
  unsigned long i;

  time_t t0 = time(NULL), t1, t2, t3;
  srand(time(NULL));

  printf("Reading edgelist from file %s and building adjacency array\n", argv[1]);
  g = readadjlist(argv[1]);
  printf("Number of nodes: %lu\n", g->n);
  printf("Number of edges: %llu\n", g->e);

  /*
  //using more memory but reading the input text file only once
  edgelist* el;
  printf("Reading edgelist from file %s\n", argv[1]);
  el=readedgelist(argv[1]);
  printf("Number of nodes: %lu\n", el->n);
  printf("Number of edges: %llu\n", el->e);
  t1 = time(NULL);
  printf("- Time = %ldh%ldm%lds\n", (t1-t0)/3600, ((t1-t0)%3600)/60, ((t1-t0)%60));
  printf("Building adjacency array\n");
  g=mkadjlist(el);
  */

  t1 = time(NULL);
  printf("- Time to load the graph = %ldh%ldm%lds\n", (t1-t0)/3600, ((t1-t0)%3600)/60, ((t1-t0)%60));

  printf("Starting louvain\n");
  part = malloc(g->n * sizeof(unsigned long));
  louvainComplete(g, part);
  t2 = time(NULL);
  printf("- Time to compute communities = %ldh%ldm%lds\n", (t2-t1)/3600, ((t2-t1)%3600)/60, ((t2-t1)%60));
  
  printf("Prints result in file %s\n", argv[2]);
  FILE* out = fopen(argv[2], "w");
  for(i = 0; i < g->n; i++){
    fprintf(out, "%lu %lu\n", i, part[i]);
  }
  fclose(out);
  t3 = time(NULL);
  printf("- Time to export communities = %ldh%ldm%lds\n", (t3-t2)/3600, ((t3-t2)%3600)/60, ((t3-t2)%60));

  printf("- Overall time = %ldh%ldm%lds\n", (t3-t0)/3600, ((t3-t0)%3600)/60, ((t3-t0)%60));

  return 0;
}
