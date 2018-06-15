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
	int rotuloMax;
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


#define ROTULO_MAX(NODE) (((mynode_t *)(AGDATA(NODE)))->rotuloMax)
#define COR(NODE) (((mynode_t *)(AGDATA(NODE)))->cor)

// ----------------------------------------------------------------------------
// Implementa fila

struct node *head;
int tamF = 0;

void criaF() {
	head = NULL;
}

void insereF(Agnode_t *elem) {	 
	struct node *novo = malloc(sizeof(struct node));
	novo->elem = elem;
	
	if(head == NULL) {
		novo->ant = novo->prox = NULL;
		head = novo;
	} else {
		struct node *p = head;
		while( p->prox != NULL && ROTULO_MAX(novo->elem) < ROTULO_MAX(p->elem))
			p = p->prox;

		if(p->prox == NULL) {
			if( ROTULO_MAX(novo->elem) < ROTULO_MAX(p->elem) ) {
				p->prox = novo;
				novo->ant = p;
				novo ->prox = NULL;
			} else {
				novo->ant = p->ant;
				novo->prox = p;	
				if(p != head)
					p->ant->prox = novo;
				p->ant = novo;
			}
		} else {
			novo->ant = p->ant;
			novo->prox = p;	
			if(p != head)
				p->ant->prox = novo;
			p->ant = novo;
		}
	}
	tamF++;
}

void impF(struct node *p) {
	if(p != NULL) {
		printf("%d ",ROTULO_MAX(p->elem));
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

Agnode_t *removeF() {
	if(head == NULL) {
		return NULL;
	} else if(head->prox != NULL) {		// Lista com mais de um elemento
		struct node *p = head;
		head = head->prox;
		head->ant = NULL;
		tamF--;
		return p->elem;
	} else {
		struct node *ret = head;
		head = NULL;
		tamF--;
		return ret->elem;
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
	char str[8];
	for (n = agfstnode(g->grafo); n; n = agnxtnode(g->grafo,n)) {
		sprintf(str,"#%06x", (unsigned int)(16777215.0/(COR(n)+1)) );
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

vertice *busca_lexicografica(grafo g, vertice r) {
	struct vertice *ret = malloc(g->numNodes*sizeof(struct vertice));

	Agedge_t *e;
	Agnode_t *n;
	for (n = agfstnode(g->grafo); n; n = agnxtnode(g->grafo,n)) {	
		ROTULO_MAX(n) = 0;
		COR(n) = -1;
	}
	
	Agnode_t *verticeInicial = agnode(g->grafo,"a",FALSE);//r->vertice;
	ROTULO_MAX(verticeInicial) = g->numNodes;

	criaF();
	for (n = agfstnode(g->grafo); n; n = agnxtnode(g->grafo,n)) {	
		insereF(n);
	}

	int i = 0;
	while(ehVazia() == 0) {
		n = removeF();
		ret[i].vertice = n;
		for (e = agfstout(g->grafo,n); e; e = agnxtout(g->grafo,e)) {
			if( ROTULO_MAX(aghead(e)) < tamF )
				ROTULO_MAX(aghead(e)) = tamF;
		}
		i++;
	}

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
	struct vertice *vPtr = (struct vertice *)v;
	Agedge_t *e;
	int i,j;
	int *disp = malloc(g->numNodes*sizeof(int));
	for(i = g->numNodes-1; i >=0;i--) {
		if( COR(vPtr[i].vertice) != -1 ) // Se o vertice tiver alguma cor
			continue;
		for(j=0;j<g->numNodes;j++)
			disp[j] = 1;
		for (e = agfstout(g->grafo,vPtr[i].vertice); e; e = agnxtout(g->grafo,e)) {
			if( COR(aghead(e)) != -1 )	
				disp[COR(aghead(e))] = 0;
		}
		for(j=0;j<g->numNodes;j++) {
			if(disp[j] == 1)
				break;
		}
		COR(vPtr[i].vertice) = j;
	}

	free(disp);
	return 0;
}

//------------------------------------------------------------------------------
