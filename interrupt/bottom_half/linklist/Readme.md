# Link List

## Introduction
1. Definition
Linklist is a structure that consists of sequence of nodes. Each node is compose 2 fields:
- Data field: contains the data
- Pointer field: contains the address of the next node in the sequence
2. Types of linklist
- Singly linked list: each node contains a data and a pointer to the next node
- Doubly linked list: each node contains a data, a pointer to the next node and a pointer to the previous node
- Circular linked list: last node points to the first node

## Linklist in Linux Kernel
1. Definition 
It's define in `<linux/list.h>. Normally, we use to declare a link list as below:
```c
struct my_node
{
    int data;
    struct my_node* next;
    struct my_node* prev;
}
```
But in linux kernel, we declare a link list as below:
```c
struct my_node
{
    int data;
    struct list_head list;
}
```
where struct `list_head` is defined in `<linux/list.h>` as:
```c
struct list_head 
{
	struct list_head *next;
    struct list_head *prev;
};
```
2. Initialize a linklist
Before create a node in linklist, we should create a linked list head node first:
```c
LIST_HEAD(my_linklist);
```

This macro defines and initializes a `struct list_head` variable named `my_linklist` to be the head of a new empty linked list. <br>
```c
#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)
struct list_head 
{
    struct list_head *next;
    struct list_head *prev;
};
```

-> So after using `LIST_HEAD(my_linklist);`, the `my_linklist` will be initialized as:

```c
struct list_head my_linklist = { &my_linklist, &my_linklist };
```
It indicates that the list is empty because both the `next` and `prev` pointers point to the head itself. <br>

3. Create node in link list
You have to create linklist node static/dynamic. Your linklist node must contain `struct list_head` member.
```c
INIT_LIST_HEAD(struct list_head *list);
``
Ex:
```c
struct my_list
{
    struct list_head list;
    int data;
};
struct my_list new_node;
INIT_LIST_HEAD(&new_node.list);
``` 
where `INIT_LIST_HEAD` macro is defined as:
```c
static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}
```

### Function to manipulate linklist
1. Add node to linklist
```c
void list_add(struct list_head* new, struct list_head* head);
```
- where 
    - `new` is the new node to be added
    - `head` is the head of the linklist.

This function adds the new node right after the head of the list. <br>
Ex:
```c
list_add(&new_node.list, &my_linklist);
```

2. Add before node to linklist
This function adds the new node right before the head of the list (i.e., at the end of the list). <br>
    **head <-> node1 <-> node2 <-> ... <-> newnode <-> head**
```c
list_add_tail(struct list_head* new, struct list_head* head);
```
- where
    - `new` is the new node to be added
    - `head` is the head of the linklist.
Ex:
```c
list_add_tail(&new_node.list, &my_linklist);
```
3. Delete node from linklist
This function removes next and prev pointers of the node from the linklist. But it does not free the memory of the node.
```c
void list_del(struct list_head *entry);
```
- where
    - `entry` is the node to be deleted.
Ex:
```c
list_del(&new_node.list);
```

This function removes next and prev pointers of the node from the linklist and initializes the node as an empty list. But it does not free the memory of the node.
```c
void list_del_init(struct list_head *entry);
```
- where
    - `entry` is the node to be deleted.
Ex:
```c
list_del_init(&new_node.list);
```
4. Replace node in linklist
This function is used to replace an old node with the new node in the linklist.
```c
void list_replace(struct list_head *old, struct list_head *new);
```
- where
    - `old` is the old node to be replaced.
    - `new` is the new node to be added.
Ex:
```c
list_replace(&old_node.list, &new_node.list);
```
This funtion is used to replace an old node with the new node in the linklist and initializes the old node as an empty list.
```c
void list_replace_init(struct list_head *old, struct list_head *new);
```
- where
    - `old` is the old node to be replaced.
    - `new` is the new node to be added.
Ex:
```c
list_replace_init(&old_node.list, &new_node.list);
```
5. Moving node in linklist
This function is used to delete a node from list and add to it after the head node.
```c
list_move(struct list_head* list, struct list_head* head);
```
- where
    - `list` is the node to be moved.
    - `head` is the head of the linklist.
Ex:
```c
list_move(&node_to_move.list, &my_linklist);
```
This function is used to delete a node from list and add to it before the head node.
```c
list_move_tail(struct list_head* list, struct list_head* head);
```
- where
    - `list` is the node to be moved.
    - `head` is the head of the linklist.
Ex:
```c
list_move_tail(&node_to_move.list, &my_linklist);
```
6. check empty linklist
This function checks whether the linklist is empty or not.
```c
int list_empty(const struct list_head *head);
```
- where
    - `head` is the head of the linklist.
Ex:
```c
//return 1 if empty, 0 if not empty
if (list_empty(&my_linklist))   
{
    // linklist is empty
}
else
{
    // linklist is not empty
}
```
7. Splice two linklists
This function is used to join two linklists.
```c
void list_splice(const struct list_head *list, struct list_head *head);
```
- where
    - `list` is the linklist to be added.
    - `head` is the head of the linklist to which the other list is added
Ex:
```c
list_splice(&linklist_to_add, &my_linklist);
```
8. Splice linklists into two parts
This function is used to cut a linklist into two parts and join the first part to another linklist.
```c
void list_cut_position(struct list_head *list,
                      struct list_head *head,
                      struct list_head *entry);
```
- where
    - `list` a new linklist to add all remove entries.
    - `head` is the head of the linklist to be cut.
    - `entry` is the node where the linklist is cut.
Ex:
```c
list_cut_position(&new_linklist, &my_linklist, &cut_node.list);
```

9. Get the struct for this entry
This macro is used to get the struct for this entry.
```c
list_entry(ptr, type, member);
```
- where
    - `ptr`: the struct list_head pointer.
    - `type`: the type of the struct this is embedded in.
    - `member`: the name of the list_head within the struct.
Ex:
```c
struct my_list* node;
struct list_head* ptr;
node = list_entry(ptr, struct my_list, list);
```
10. Iterate over linklist
This macro is used to iterate over the linklist.
```c
list_for_each(pos, head);
```
- where
    - `pos`: the struct list_head pointer used as a loop cursor.
    - `head`: the head of the linklist.
Ex:
```c
struct list_head* pos;
list_for_each(pos, &my_linklist)
{
    struct my_list* node = list_entry(pos, struct my_list, list);
    // do something with node
}
```
Iterates over each node in 1 macro call. <br>
**list_for_each_entry**
```c 
list_for_each_entry(pos, head, member);
```
- where
    - `pos`: the type * pointer to use as a loop cursor.
    - `head`: the head of the linklist.
    - `member`: the name of the list_head within the struct.
Ex:
```c
struct my_list* pos;
list_for_each_entry(pos, &my_linklist, list)
{
    // do something with pos
}
```
Iterates over each node in 1 macro call, and get the struct pointer directly. <br>

Another variant that is safe against removal of list entry during the iteration:
```c
list_for_each_entry_safe(pos, n, head, member);
```
- where
    - `pos`: the type * pointer to use as a loop cursor.
    - `n`: another type * pointer to use as temporary storage.
    - `head`: the head of the linklist.
    - `member`: the name of the list_head within the struct.
Ex:
```c
struct my_list *pos, *n;
list_for_each_entry_safe(pos, n, &my_linklist, list)
{
    // do something with pos
}
```
Iterates over each node in 1 macro call, and is safe against removal of list entry
during the iteration. <br>

And many more......