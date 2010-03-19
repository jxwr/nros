#pragma once

struct link_s {
  struct link_s* next;
  struct link_s* prev;
};

typedef struct link_s link_t;

static inline void link_init(link_t* l)
{
  l->next = l;
  l->prev = l;
}

#define LINK_INIT(link)	{ &(link), &(link) }

static inline void list_insert_after(link_t* new, link_t* list)
{
  link_t* next = list->next;
  
  /*
   * The new link goes between the
   * current and next links on the list e.g.
   * list -> new -> next
   */
  new->next = next;
  next->prev = new;
  list->next = new;
  new->prev = list;
}

static inline void list_insert_before(link_t* new, link_t* list)
{
  link_t* prev = list->prev;
  
  /*
   * The new link goes between the
   * current and prev links  on the list, e.g.
   * prev -> new -> list
   */
  new->next = list;
  list->prev = new;
  new->prev = prev;
  prev->next = new;
}

static inline void list_remove(link_t* link)
{
  link_t* prev = link->prev;
  link_t* next = link->next;

  prev->next = next;
  next->prev = prev;
  
  link->next = (link_t* )0;
  link->prev = (link_t* )0;
}

static inline void list_remove_init(link_t* link)
{
  link_t* prev = link->prev;
  link_t* next = link->next;
  
  prev->next = next;
  next->prev = prev;

  link->next = link;
  link->prev = link;
}

/* Cuts the whole list from head and returns it */
static inline link_t* list_detach(link_t* head)
{
  link_t* next = head->next;
  
  /* Detach head from rest of the list */
  list_remove_init(head);
  
  /* Return detached list */
  return next;
}

static inline int list_empty(link_t* list)
{
  return list->prev == list && list->next == list;
}


#define link_to_struct(link, struct_type, link_field)	\
  container_of(link, struct_type, link_field)

#define list_foreach_struct(struct_ptr, link_start, link_field)		\
  for (struct_ptr = link_to_struct((link_start)->next, typeof(*struct_ptr), link_field); \
       &struct_ptr->link_field != (link_start);				\
       struct_ptr = link_to_struct(struct_ptr->link_field.next, typeof(*struct_ptr), link_field))

#define list_foreach_removable_struct(struct_ptr, temp_ptr, link_start, link_field) \
  for (struct_ptr = link_to_struct((link_start)->next, typeof(*struct_ptr), link_field), \
	 temp_ptr = link_to_struct((struct_ptr)->link_field.next, typeof(*struct_ptr), link_field); \
       &struct_ptr->link_field != (link_start);				\
       struct_ptr = temp_ptr, temp_ptr = link_to_struct(temp_ptr->link_field.next, typeof(*temp_ptr), link_field))
