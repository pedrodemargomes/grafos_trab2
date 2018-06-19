#include <stdio.h>
#include <stdlib.h>
#include <graphviz/cgraph.h> 
#include "grafo.h"

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

typedef struct mynode_s {
	Agrec_t header;
	int *rotulos;
	int tamRotulos;
	int cor;
} mynode_t;


struct node {
	Agnode_t *elem;
	struct node *prox;
	struct node *ant;
};

struct vertice {
	Agnode_t *vertice;
};

#define ROTULOS(NODE) (((mynode_t *)(AGDATA(NODE)))->rotulos)
#define COR(NODE) (((mynode_t *)(AGDATA(NODE)))->cor)
#define TAM_ROTULOS(NODE) (((mynode_t *)(AGDATA(NODE)))->tamRotulos)

// ----------------------------------------------------------------------------
// Insere rotulo

void insereRotulo(Agnode_t *node,int n) {
	ROTULOS(node)[TAM_ROTULOS(node)] = n;
	TAM_ROTULOS(node)++;
}

int comparaRotulos(grafo g,int *rA, int *rB) {
	int i;
	for(i = 0; i < g->numNodes; i++) {
		if(rA[i] < rB[i])
			return 1;
		else if(rA[i] > rB[i])
			return 0;
	}
	return 0;
}

// ----------------------------------------------------------------------------
// Implementa fila

struct node *head;
int tamF;

void criaF() {
	tamF = 0;
	head = NULL;
}


void insereF(grafo g,Agnode_t *elem) {	 
	struct node *novo = malloc(sizeof(struct node));
	novo->elem = elem;
	
	if(head == NULL) {
		novo->ant = novo->prox = NULL;
		head = novo;
	} else {
		novo->ant = NULL;
		novo->prox = head;
		head->ant = novo;
		head = novo;
	}
	tamF++;
}

void impF(struct node *p) {
	if(p != NULL) {
		int j = 0;
		printf("vertice %s: ",agnameof(p->elem));
		while( ROTULOS(p->elem)[j] ) {
			printf("%d ", ROTULOS(p->elem)[j]);
			j++;
		}
		printf("\n");
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

Agnode_t *removeF(grafo g) {
	if(head == NULL) {
		return NULL;
	} else {		// Lista com mais de um elemento
		struct node *p = head;
		struct node *ret = p;
		while(p != NULL) {
			//printf("aaaaaaaaaaaaaa\n");
			if( comparaRotulos(g, ROTULOS(ret->elem), ROTULOS(p->elem) ) ) {
				//printf("%s %s\n", agnameof(ret->elem), agnameof(p->elem));
				ret = p;
			}
			p = p->prox;
		}
		if(ret == head) {
			if(head->prox != NULL)
				head->prox->ant = NULL;
			head = ret->prox;
		} else {
			if(ret->prox != NULL)
				ret->prox->ant = ret->ant;
			if(ret->ant != NULL)
				ret->ant->prox = ret->prox;
		}

		tamF--;
		Agnode_t *retNode = ret->elem;
		free(ret);
		return retNode;
	}
	
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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


int n_vertices(grafo g) {
	return g->numNodes;
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
	Agnode_t *n;
	agattr(g->grafo,AGNODE,"style","filled");
	agattr(g->grafo,AGNODE,"color","white");
	char str[8];
	for (n = agfstnode(g->grafo); n; n = agnxtnode(g->grafo,n)) {
		//printf("vertice %s = %d\n",agnameof(n),COR(n));
		sprintf(str,"#%06x", (unsigned int)COR(n) );
		agset(n,"color",str);
	}

	agwrite(g->grafo,output);
	return g;
}
//------------------------------------------------------------------------------
// devolve um número entre 0 e o número de vertices de g

unsigned int cor(vertice v, grafo g) {
	return COR(v->vertice);
}

//------------------------------------------------------------------------------
// aloca e devolve um vetor com os vértices de g ordenados de acordo com 
// uma busca em largura lexicográfica a partir de r

vertice *busca_lexicografica(grafo g, vertice *v) {
	struct vertice *ret = (struct vertice *)v;

	Agedge_t *e;
	Agnode_t *n;
	int i;
	for (n = agfstnode(g->grafo); n; n = agnxtnode(g->grafo,n)) {	
		ROTULOS(n) = calloc((g->numNodes),sizeof(int)); // Inicia com zero
		TAM_ROTULOS(n) = 0;
	}
	
	Agnode_t *verticeInicial = agfstnode(g->grafo);
	insereRotulo(verticeInicial,g->numNodes+1);

	criaF();
	for (n = agfstnode(g->grafo); n; n = agnxtnode(g->grafo,n)) {	
		insereF(g, n);
	}

	int j;
	i = 0;
	while(ehVazia() == 0) {
		// printf("------------------\n");
		// imprimeF();
		// printf("+++++++++++++++++++\n");
		n = removeF(g);
		
		//printf("%s\n", agnameof(n) );
		// j = 0;
		// while( ROTULOS(n)[j] ) {
		// 	printf("%d ", ROTULOS(n)[j]);
		// 	j++;
		// }
		// printf("\n");

		ret[i].vertice = n;
		for (e = agfstedge(g->grafo,n); e; e = agnxtedge(g->grafo,e,n)) {
			if(aghead(e) != n)
				insereRotulo(aghead(e), tamF+1);
			else
				insereRotulo(agtail(e), tamF+1);
		}
		i++;
	}

	/*for(i = 0; i < g->numNodes;i++) {
		printf("vertice %s: ", agnameof(ret[i].vertice) );
		j = 0;
		while( ROTULOS(ret[i].vertice)[j] ) {
			printf("%d ", ROTULOS(ret[i].vertice)[j]);
			j++;
		}
		printf("\n");
	}*/

	for (n = agfstnode(g->grafo); n; n = agnxtnode(g->grafo,n))	
		free(ROTULOS(n));

	return (struct vertice **)ret;
}

//------------------------------------------------------------------------------
// colore os vértices de g de maneira "gulosa" segundo a ordem dos
// vértices em v e devolve o número de cores utilizado
//
// ao final da execução,
//     1. cor(v,g) > 0 para todo vértice de g
//     2. cor(u,g) != cor(v,g), para toda aresta {u,v} de g

unsigned int colore(grafo g, vertice *v) {
	Agnode_t *n;	
	for (n = agfstnode(g->grafo); n; n = agnxtnode(g->grafo,n)) {	
		COR(n) = 0;
	}

	struct vertice *vPtr = (struct vertice *)v;
	Agedge_t *e;
	int i,j;
	int *disp = malloc( (g->numNodes+1)*sizeof(int));
	for(i = g->numNodes-1; i >=0;i--) {
		if( COR(vPtr[i].vertice) != 0 ) // Se o vertice tiver alguma cor
			continue;
		for(j=0;j<g->numNodes+1;j++)
			disp[j] = 1;
		//printf("vertice = %s\n",agnameof(vPtr[i].vertice));
		for (e = agfstedge(g->grafo,vPtr[i].vertice); e; e = agnxtedge(g->grafo,e,vPtr[i].vertice)) {
			if(aghead(e) != vPtr[i].vertice) {
				//printf("vertice viz = %s\n", agnameof(aghead(e)) );
				if( COR(aghead(e)) != 0 ) {
					disp[COR(aghead(e))] = 0;
					//printf("+++ cor = %d\n",COR(aghead(e)));
				}
			} else {
				//printf("vertice viz = %s\n", agnameof(agtail(e)) );
				if( COR(agtail(e)) != 0 ) {
					disp[COR(agtail(e))] = 0;
					//printf("+++ cor = %d\n",COR(agtail(e)));
				}
			}
		}
		for(j=1;j<g->numNodes+1;j++) {
			if(disp[j] == 1)
				break;
		}
		//printf("disp = %d %d %d\n",disp[0],disp[1],disp[2]);
		COR(vPtr[i].vertice) = j;
		//printf("cor = %d\n",j);
	}

	int count = 1;
	for (n = agfstnode(g->grafo); n; n = agnxtnode(g->grafo,n)) {	
		if(COR(n) > count)
			count++;
	}

	free(disp);
	return count;
}

//------------------------------------------------------------------------------
