#include <stdlib.h>
#include <stdio.h>
#include "grafo.h"

int main(void) {

	grafo g = le_grafo(stdin);

 	if ( !g )
 		return 1;
	
	vertice *r = malloc(n_vertices(g)*sizeof(vertice));
	busca_lexicografica( vertice_de_nome("A",g), g, r);
	printf("++ cores: %u\n", colore(g, r));
	free(r);

	escreve_grafo(stdout, g);
	destroi_grafo(g);

	return 0;
}
