#include <stdio.h>
#include <stdlib.h>
#include <graphviz/cgraph.h> 
#include "grafo.h"

// ----------------------------------------------------------------------------
// Implementa fila

struct node {
	Agnode_t *valor;
	struct node *prox;
};

struct node *head;

void criaF() {
	head = NULL;
}

void insereF(int valor) {	 
	struct node *novo = malloc(sizeof(struct node));
	novo->valor = valor;

	novo->prox = head;
	head = novo;
}

void impF(struct node *p) {
	if(p != NULL) {
		printf("%d ",p->valor);
		impF(p->prox);
	}

}

void imprimeF() {
	impF(head);
}

int ehVazia() {
	if(head == NULL) return 1;
	return 0;
}

void removeF() {
	if(head == NULL) {
		return NULL;
	} else if(head->prox != NULL) {		// Lista com mais de um elemento
		struct node *p = head;
		struct node *maior,*antMaior;
		int maiorVal = 0;
		while(p->prox != NULL) {
			if( p->numRotulos > 0 && p->rotulos[p->numRotulos-1] > maiorVal ) {
				maiorVal = p->rotulos[p->numRotulos-1];
				maior = p;
			}
			p = p->prox;
		}
	} else {
		struct node *ret = head;
		head = NULL;
		return ret;
	}
	
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct mynode_s {
	Agrec_t header;
	int *rotulos;
	int numRotulos;
} mynode_t;

//------------------------------------------------------------------------------
// (apontador para) estrutura de dados para representar um grafo
// 
// o grafo pode ser direcionado ou não
// 
// o grafo tem um nome, que é uma "string"

struct grafo {
	Agraph_t *grafo;
	int numNodes;
};


// ++++++++++++++++++++++++++++++++

int numVizinhosComuns(grafo g, Agnode_t *n, Agnode_t *m){
	Agedge_t *e;
	Agnode_t *a[1000],*b[1000];
	
	// Vizinhos do a
	int i = 0;
	for (e = agfstedge(g->grafo,n); e; e = agnxtedge(g->grafo,e,n)) {
		a[i] = aghead(e);
		i++;
	}

	int j = 0;
	// Vizinhos do b
	for (e = agfstedge(g->grafo,m); e; e = agnxtedge(g->grafo,e,m)) {	
		b[j] = aghead(e);
		j++;
	}

	int k,x;
	
	/*printf("a = ");
	for(k = 0;k < i;k++) {
		printf("%s ",agnameof(a[k]));
	}
	printf("\nb = ");
	for(x = 0;x < j;x++) {
		printf("%s",agnameof(b[x]));
	}
	printf("\n");*/

	int numViz = 0;
	for(k = 0;k < i;k++) {
		for(x = 0;x < j;x++) {
			if(a[k] == b[x])
				numViz++;
		}
	}

	return numViz;
}

void sub(grafo g, Agnode_t *m,Agnode_t *n,Agnode_t **r) {
	Agedge_t *e;
	Agnode_t **a = malloc(g->numNodes * sizeof(Agnode_t *));
	
	// Vizinhos do a
	int i = 0;
	for (e = agfstedge(g->grafo,n); e; e = agnxtedge(g->grafo,e,n)) {
		a[i] = aghead(e);
		i++;
	}

	int j,b,k;
	k = 0;
	for (e = agfstedge(g->grafo,m); e; e = agnxtedge(g->grafo,e,m)) {	
		b = 1;
		for(j=0;j<i;j++) {
			if( a[j] == aghead(e) ) {
				b=0;
			}
		}
		if(b == 1) {
			r[k] = aghead(e);
			k++;
		}
	}
	r[k] = NULL;
	free(a);
}

// ++++++++++++++++++++++++++++++++


//------------------------------------------------------------------------------
// desaloca toda a memória usada em *g
// 
// devolve 1 em caso de sucesso,
//         ou 
//         0, caso contrário

int destroi_grafo(grafo g) {
	if(g == NULL)
		return 0;
  	agclose(g->grafo);
  	free(g);
	return 1;
}
//------------------------------------------------------------------------------
// lê um grafo no formato dot de input
// 
// devolve o grafo lido,
//         ou 
//         NULL, em caso de erro 

grafo le_grafo(FILE *input) {
  	grafo g = malloc(sizeof(struct grafo) );
  	g->grafo = agread(input,NULL);
	Agnode_t *n;

	aginit(g->grafo, AGNODE, "mynode_t", sizeof(mynode_t), TRUE);
	
  	int numNodes = 0;
	for (n = agfstnode(g->grafo); n; n = agnxtnode(g->grafo,n)) {	
		numNodes++;
	}
	g->numNodes = numNodes;
	return g;
}
//------------------------------------------------------------------------------
// escreve o grafo g em output usando o formato dot.
//
// devolve o grafo escrito,
//         ou 
//         NULL, em caso de erro 

grafo escreve_grafo(FILE *output, grafo g) {
	agwrite(g->grafo,output);
	return g;
}
//------------------------------------------------------------------------------
// devolve o grafo de recomendações de g
//
// cada aresta {c,p} de H representa uma recomendação do produto p
// para o consumidor c, e tem um atributo "weight" que é um inteiro
// representando a intensidade da recomendação do produto p para o
// consumidor c.

grafo recomendacoes(grafo g){
	
	grafo gReco = malloc(sizeof(struct grafo));
	gReco->grafo = agopen("Grafo recomendacoes",Agundirected,NULL);
	agattr(gReco->grafo,AGEDGE,"weight","0");
	agattr(gReco->grafo,AGEDGE,"label","0");
	agattr(gReco->grafo,AGNODE,"tipo","p");
	agattr(gReco->grafo,AGNODE,"xlabel","p");
	
	Agnode_t *n,*m;
	Agnode_t **r = calloc(g->numNodes,sizeof(Agnode_t *));
	
	Agnode_t *cReco;
	Agnode_t *pReco; 
	Agedge_t *novaAresta;

	int numViz,k,peso;
	char str[1000];
	for (n = agfstnode(g->grafo); n; n = agnxtnode(g->grafo,n)) {
		if( agget(n,"tipo")[0] == 'c' ) {
			for (m = agfstnode(g->grafo); m; m = agnxtnode(g->grafo,m)) {
				if( m != n && agget(m,"tipo")[0] == 'c' ) {
					//printf("Testa %s e %s\n",agnameof(n),agnameof(m));
					numViz = numVizinhosComuns(g,n,m);	
					//printf("numViz = %d\n",numViz);
					cReco = agnode(gReco->grafo,agnameof(m),TRUE); // c
					agset(cReco,"tipo","c");
					agset(cReco,"xlabel","c");	
					if( numViz  >= (agdegree(g->grafo,n,TRUE,TRUE)-numViz) ) {
						// Recomenda para m os produtos de n
						//printf("Recomenda os produtos de %s para o %s\n",agnameof(n),agnameof(m));
						
						sub(g,n,m,r);
						k = 0;
						while(r[k] != NULL) {
							//printf("%s ",agnameof( r[k]) );
							pReco = agnode(gReco->grafo,agnameof(r[k]),TRUE); // p	
							novaAresta = agedge(gReco->grafo,cReco,pReco,"",TRUE);
							strcpy(str, agget(novaAresta,"weight")); 
							peso = atoi(str) + 1;
							sprintf(str,"%d",peso);
							agset(novaAresta,"weight",str);	
							agset(novaAresta,"label",str);
							k++;
						}
						//printf("\n");

					}			
				}
			}
		} else {
			// p
			agnode(gReco->grafo,agnameof(n),TRUE);
		}
	}

	free(r);

	gReco->numNodes = g->numNodes;
	return gReco;
}

//------------------------------------------------------------------------------
// devolve um número entre 0 e o número de vertices de g

unsigned int cor(vertice v, grafo g) {
	return ;
}

//------------------------------------------------------------------------------
// aloca e devolve um vetor com os vértices de g ordenados de acordo com 
// uma busca em largura lexicográfica a partir de r

vertice *busca_lexicografica(grafo g, vertice r) {
	Agnode_t *n;
	for (n = agfstnode(g->grafo); n; n = agnxtnode(g->grafo,n)) {	
		((mynode_t *)(AGDATA(n)))->numRotulos = 0;
		((mynode_t *)(AGDATA(n)))->rotulos = malloc(g->numNodes*sizeof(int));
	}
	
	Agnode_t *verticeInicial = r;
	verticeInicial->numRotulos = 1;
	verticeInicial->rotulos[0] = g->numNodes;


	for (n = agfstnode(g->grafo); n; n = agnxtnode(g->grafo,n)) {	
		insereF();
	}

	while(listaVerticesTam != 0) {
		
	}
	
	
	return ;
}

//------------------------------------------------------------------------------
// colore os vértices de g de maneira "gulosa" segundo a ordem dos
// vértices em v e devolve o número de cores utilizado
//
// ao final da execução,
//     1. cor(v,g) > 0 para todo vértice de g
//     2. cor(u,g) != cor(v,g), para toda aresta {u,v} de g

unsigned int colore(grafo g, vertice *v) {



	return ;
}

//------------------------------------------------------------------------------
