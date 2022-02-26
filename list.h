/* List headers by Iván García and Daniel Quintillán.
 * Logins: ivan.garcia.fernandez and daniel.quintillan
 */

typedef struct node *list;
typedef struct node *pos;

void create_list(list *l); // Initializes the list for usage
void insert(list *l, void *x); // Inserts x on the list
int is_empty_list(list l); // Returns 1 if the list is empty, 0 otherwise
pos find(list l, void *e); // Returns the pos of the first entry matching e
pos previous(list l, pos e); // Returns the previous pos in the list
pos next(pos p); // Returns the next pos in the list
void delete_elem(list *l, pos p); // Deletes the element at pos p
pos first(list l); // Returns the pos of the first element
int at_end(list l, pos p); // Checks if an element is the last of the list
void * data (list l, pos p); // Accesses the data (pointer)in a given pos
