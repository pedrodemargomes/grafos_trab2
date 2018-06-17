#include <stdlib.h>
#include <stdio.h>
#include "grafo.h"

int main(void) {

	grafo g = le_grafo(stdin);

 	if ( !g )
 		return 1;
	
	vertice *t =  busca_lexicografica(g, 0);
	colore( g ,  t);

	escreve_grafo(stdout, g);
	return !destroi_grafo(g);
}
