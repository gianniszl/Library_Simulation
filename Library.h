#ifndef LIBRARY_H
#define LIBRARY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
   CONSTANTS
   ============================================================ */
#define NAME_MAX   100
#define TITLE_MAX  200
#define LINE_MAX   1024

/* ============================================================
   FORWARD DECLARATIONS (απαραίτητα γιατί οι δομές δείχνουν
   η μία στην άλλη)
   ============================================================ */

struct genre;
struct member;
struct book;

/* ============================================================
   LOAN NODE
   ============================================================ */
typedef struct loan {
    int sid;
    int bid;
    struct loan *next;
} loan_t;

/* ============================================================
   MEMBER ACTIVITY (PHASE 2)
   ============================================================ */
typedef struct MemberActivity {
    int sid;
    int loans_count;
    int reviews_count;
    int score_sum;
    struct MemberActivity *next;
} MemberActivity;

/* ============================================================
   MEMBER
   ============================================================ */
typedef struct member {
    int sid;
    char name[NAME_MAX];
    loan_t *loans;              /* tail sentinel list */
    MemberActivity *activity;   /* Phase 2 */
    struct member *next;
} member_t;

/* ============================================================
   BOOK (doubly linked list node)
   ============================================================ */
typedef struct book {
    int bid;
    int gid;
    char title[TITLE_MAX];

    int sum_scores;
    int n_reviews;
    int avg;
    int lost_flag;

    int heap_pos; /* position in RecHeap */

    struct book *prev;
    struct book *next;
} book_t;

/* ============================================================
   GENRE
   ============================================================ */
typedef struct genre {
    int gid;
    char name[NAME_MAX];

    book_t *books;       /* doubly linked list */
    int slots;           /* display seats */
    int lost_count;      
    int invalid_count;   

    book_t **display;    /* dynamic array for PD */
    struct genre *next;
} genre_t;

/* ============================================================
   AVL NODE (Book Index)
   ============================================================ */
typedef struct BookNode {
    char title[TITLE_MAX];
    book_t *book;

    struct BookNode *lc;
    struct BookNode *rc;

    int height;
} BookNode;

/* ============================================================
   RECOMMENDATION HEAP
   ============================================================ */
typedef struct RecHeap {
    book_t **heap;
    int size;
    int capacity;
} RecHeap;

/* ============================================================
   GLOBAL LIBRARY STRUCT
   ============================================================ */
typedef struct {
    genre_t *genres;
    member_t *members;
    MemberActivity *activity;
    BookNode *book_index;
    RecHeap *recommendations;
} library_t;

/* Δηλώνεται στο main.c */
extern library_t library;

/* Το SLOTS είναι global (ΑΛΛΑ υπάρχει local στη main — δεν το πειράζουμε).
   Το Library.h χρειάζεται global declaration. */
extern int SLOTS;

/* ============================================================
   FUNCTION PROTOTYPES (ΦΑΣΗ 1 + ΦΑΣΗ 2)
   ============================================================ */

/* ---------- PHASE 1 ---------- */
void set_slots(int slots);

void add_genre(int gid, char *name);
void add_book(int bid, int gid, char *title);
void add_member(int sid, char *name);

void loan_book(int sid, int bid);
void return_book(int sid, int bid, char *status, int score);

void display_books(void);
void print_display(void);
void print_genre(int gid);
void print_member_loans(int sid);
void print_stats(void);

/* ---------- PHASE 2: FIND + UPDATE (AVL) ---------- */
void find_book(const char *title);
void update_title(int bid, const char *newtitle);

/* ---------- PHASE 2: AVL TREE ---------- */
BookNode* avl_insert(BookNode *node, book_t *book);
BookNode* avl_search(BookNode *root, const char *title);
BookNode* avl_delete(BookNode *root, const char *title);
void free_avl(BookNode *root);

/* ---------- PHASE 2: HEAP ---------- */
RecHeap* heap_create(void);
void heap_insert(RecHeap *h, book_t *b);
void heap_update(RecHeap *h, book_t *b);
void heap_remove(RecHeap *h, int pos);
void free_heap(RecHeap *h);
void top_k_books(int k);

/* ---------- PHASE 2: ACTIVITY ---------- */
MemberActivity* create_activity(int sid);
void notify_loan(member_t *m);
void notify_review(member_t *m, int score);
void print_active_members(void);
void free_activity(MemberActivity *act);

/* ---------- SYSTEM STATS ---------- */
void print_system_stats(void);

/* ---------- FREE EVERYTHING ---------- */
void free_members(member_t *m);
void free_genres(genre_t *g);
void free_library(void);

#endif
