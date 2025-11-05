#ifndef LIB_LIST_H_
#define LIB_LIST_H_
/*
 * \file list.h
 * \brief Doppelt verkettete List Datenstruktur
 *
 * Diese Datei ist eine Hilfestellung für Aufgabe 3.
 * Sie erspart euch den Aufwand, selber eine doppelt verkettete Liste zu implementieren.
 */

// List Node struct
typedef struct list_node {
	struct list_node *next;
	struct list_node *prev;
} list_node;

// Makro zum initialisieren einer Liste
#define list_create(N)                                                \
	static list_node  head__##N = { &(head__##N), &(head__##N) }; \
	static list_node *N	    = &(head__##N)

//checks if list is empty
[[nodiscard]] static inline bool list_is_empty(list_node *head)
{
	return head == head->next;
}

// Gibt das erste Elemnt der Liste zurück
[[nodiscard, maybe_unused]] static inline list_node *list_get_first(list_node *head)
{
	if (list_is_empty(head)) {
		return nullptr;
	}
	return head->next;
}

// Gibt das letzte Element der Liste zurück
[[nodiscard, maybe_unused]] static inline list_node *list_get_last(list_node *head)
{
	if (list_is_empty(head)) {
		return nullptr;
	}
	return head->prev;
}

// Füge Elemente hinzu (Hilfsfunktion)
static inline void list_add_(list_node *node, list_node *prev)
{
	node->prev = prev;
	node->next = prev->next;

	prev->next->prev = node;
	prev->next	 = node;
}

// Füge Element am Anfang der Liste hinzu
[[maybe_unused]] static inline void list_add_first(list_node *head, list_node *new)
{
	list_add_(new, head);
}

// Füge Element am Ende der Liste hinzu
[[maybe_unused]] static inline void list_add_last(list_node *head, list_node *new)
{
	list_add_(new, head->prev);
}

// Entferne Element (Hilfsfunktion)
static inline void list_remove_(list_node *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

// Entfernt das erste Element aus der Liste und gibt es zurück
[[maybe_unused]] static inline list_node *list_remove_first(list_node *head)
{
	if (list_is_empty(head)) {
		return nullptr;
	}
	list_node *del = head->next;
	list_remove_(del);
	return del;
}

// Entferne das letzte Element aus der Liste und gibt es zurück
[[maybe_unused]] static inline list_node *list_remove_last(list_node *head)
{
	if (list_is_empty(head)) {
		return nullptr;
	}
	list_node *del = head->prev;
	list_remove_(del);
	return del;
}

// Entfernt das spezielle Element aus der Liste
[[maybe_unused]] static inline list_node *list_remove(list_node *head, list_node *rem)
{
	list_node *curr;
	for (curr = head->next; curr != head; curr = curr->next) {
		if (curr == rem) {
			list_remove_(curr);
			return curr;
		}
	}
	return nullptr;
}

#endif // LIB_LIST_H_
