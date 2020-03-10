## Reference:

Fast unfolding of communities in large networks. Vincent D. Blondel, Jean-Loup Guillaume, Renaud Lambiotte, Etienne Lefebvre. J. Stat. Mech. P10008, 2008.

## To compile:

type "make"

## To execute:

"./louvain infile outfile"
- "infile" should contain an undirected unweighted graph: one edge on each line (two unsigned long (ID of the 2 nodes) separated by a space).
- "outfile": will contain the resulting partition: one node and its community id on each line.

## Initial contributors:  

Jean-Loup Guillaume
March 2020
jean-loup.guillaume@univ-lr.fr
