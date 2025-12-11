

# Library Simulation — C Data Structures Project

A command–line simulation of a library system written in C, using advanced data structures such as **AVL trees**, **max-heaps**, linked lists and dynamic structures.
The program supports book management, member activity tracking, recommendations, fast searching, and system statistics.


## Features

###  Fast Book Search (AVL Tree)

Books are indexed in an **AVL balanced binary search tree**, allowing efficient lookup by title in **O(log n)** time.

###  Book Recommendation System (Max-Heap)

A max-heap stores the most popular books based on their average user rating.
Supports:

* automatic updates after reviews
* retrieving the **top-k** highest-rated books

###  Member Activity Tracking

Tracks:

* number of loans
* number of reviews
* total rating activity

Allows listing the most active members.

###  Genre-based Book Lists

Each genre maintains a doubly–linked list of books and supports multiple view and print operations.

###  System Statistics

The program can print aggregated library statistics such as:

* number of books
* number of members
* active loans
* average rating

###  Memory Cleanup

All data structures (AVL tree, heap, lists) are safely deallocated on exit.


##  Compilation

Use the provided **Makefile**:

```bash
make
```

This produces an executable named:

```
library
```

---

##  Running the Program

Run the program by passing an input file that contains commands:

```bash
./library input.txt
```

Example commands inside an input file:

```
M 1 "Alice"
B 101 1 "The Hobbit"
L 1 101
R 1 101 ok 9
F "The Hobbit"
TOP 3
AM
X
```

##  Data Structures Used

* **AVL Tree**
  For fast title-based search and updates.

* **Max-Heap**
  Stores pointers to the highest-rated books for recommendation queries.

* **Doubly Linked Lists**
  Used for the books inside each genre.

* **Singly Linked Lists**
  For member activities and loans.

* **Dynamic Arrays**
  Used for handling book displays and sorted collections.


## Author

Giannis Zlatanos
Library Simulation in C — Systems & Data Structures Project
