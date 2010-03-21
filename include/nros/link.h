#pragma once

struct link {
  struct link* prev;
  struct link* next;
};

#define LINK_INIT(link) {&link, &link}

static inline void link_init(struct link* l)
{
  l->next = l;
  l->prev = l;
}

static inline void list_insert_after(struct link* newlist, struct link* list)
{
  struct link* next;
  // list -> newlist -> next
  next = list->next;
  next->prev = newlist;
  list->next = newlist;
  newlist->prev = list;
  newlist->next = next;
}

static inline void list_insert_before(struct link* newlist, struct link* list)
{
  struct link* prev;
  // prev -> newlist -> list
  prev = list->prev;
  prev->next = newlist;
  newlist->prev = prev;
  newlist->next = list;
  list->prev = newlist;
}

static inline void list_remove(struct link* l)
{
  l->prev->next = l->next;
  l->next->prev = l->prev;
  l->next = l;
  l->next = l;
}

#define container_of(linkptr, STRUCT, member) \
  ((STRUCT*)((char*)&(linkptr)->next - __builtin_offsetof(STRUCT, member.next)))

#define list_for_each(list, head) \
  for(list = (head)->next; list != (head); list = list->next)
  

#define list_for_each_entry(entry, head, member) \
  for(entry = container_of((head)->next,typeof(*entry),member);	\
      &entry->member != (head);						\
      entry = container_of(entry->member.next, typeof(*entry), member))
