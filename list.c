/* List implementation by Iván García and Daniel Quintillán.
 * Logins: ivan.garcia.fernandez and daniel.quintillan
 * As we changed groups, we were allowed to keep the old implementation
 * of the list.
 */

#include "list.h"
#include <string.h>//y tho
#include <stdlib.h>
#include <stdio.h>

struct node {
	void *d;
	struct node *next;
};

typedef struct node *list;
typedef struct node *pos;

void create_list(list *l) {
	(*l) = NULL;
}

void insert(list *l, void *x) {
	list elem = malloc(sizeof(*elem));					
	list aux;
	if (elem == NULL)
		exit(0); // im out of mem
	elem->d = x;
	elem->next = NULL;
	aux = *l;
	if (aux == NULL)
	*l = elem;
	else {
		while (aux->next != NULL)
		aux = aux->next;
		aux->next = elem;
	}
}

int is_empty_list(list l) {
	return (l == NULL);
}

pos find (list l, void *e) {
	pos p = l;
	if (p == NULL)
		return NULL;
	while(p != NULL && p->d != e){
		p = p->next;
	}
	if (p != NULL && (p->d) == e)
		return p;
	else
		return NULL;
}

pos previous (list l, pos e) {
	pos p = l;
	while(p != NULL && p->next != NULL && p->next != e) {
		p = p->next;
	}
	if (p->next == e) {
		return p;
	} else {
		return NULL;
	}
}

void delete_elem (list *l, pos p) {
	if (*l == p) {
		*l = (*l)->next;
		free(p);
	} else {
		pos temp = previous(*l,p);
		if (temp->next != NULL)
		temp->next = temp->next->next;
		free(p);
	}
}

pos first (list l){
	return (l);
}

pos next(pos p){
	return (p->next);
}

int at_end(list l, pos p){
	return (p == NULL);
}

void * data (list l, pos p){
	return (p->d);
}
