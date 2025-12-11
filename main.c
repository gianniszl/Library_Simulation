/************************************************************
 *
 *   main.c – Library Management System
 *   Phase 1 + Phase 2
 *
 *   Περιέχει:
 *   - Υλοποίηση Φάσης 1 (Genres, Books, Members, Loans, D/P commands)
 *   - Υλοποίηση Φάσης 2 (AVL BookIndex, RecommendationHeap,
 *     MemberActivity και εντολές AM, TOP, BF, X)
 *
 *************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Library.h"

/* ============================================================
   GLOBAL LIBRARY INSTANCE
   Περιέχει ΟΛΑ τα genres, members, books, indexes, heaps, lists.
   ============================================================ */
library_t library;

/* ============================================================
   ΣΥΝΑΡΤΗΣΕΙΣ ΦΑΣΗΣ 1
   - add_genre()
   - add_book()
   - add_member()
   - loan_book()
   - return_book()
   - remove_book()
   - print_display()
   - print_genre()
   ============================================================ */

/* ============================================================
   ΣΥΝΑΡΤΗΣΕΙΣ ΦΑΣΗΣ 2 (AVL, Heap, MemberActivity)
   - AVL insert/search/delete
   - Heap insert/update/remove
   - Activity tracking
   - Stats (X)
   - Free memory (BF)
   ============================================================ */

/* ============================================================
   set_slots()
   Ρυθμίζει τον αριθμό των διαθεσίμων θέσεων προβολής (SLOTS)
   ΦΑΣΗ 1
   ============================================================ */
void set_slots(int slots) {
    if (slots > 0) {
        SLOTS = slots;
        printf("DONE\n");
    } else {
        printf("IGNORED\n");
    }
}

/* ============================================================
   add_genre()
   Προσθέτει νέα κατηγορία στη sorted list κατά gid.
   ΦΑΣΗ 1
   ============================================================ */
void add_genre(int gid, char *name) {

    /* Έλεγχος για διπλό gid */
    genre_t *g = library.genres;
    while (g != NULL) {
        if (g->gid == gid) {
            printf("IGNORED\n");
            return;
        }
        g = g->next;
    }

    /* Δέσμευση μνήμης */
    genre_t *newg = malloc(sizeof(genre_t));
    if (!newg) {
        printf("IGNORED\n");
        return;
    }

    /* Αρχικοποίηση */
    newg->gid = gid;
    strncpy(newg->name, name, NAME_MAX);
    newg->books = NULL;
    newg->lost_count = 0;
    newg->invalid_count = 0;
    newg->slots = 0;
    newg->display = NULL;
    newg->next = NULL;

    /* Εισαγωγή στη sorted list των genres */
    if (library.genres == NULL || gid < library.genres->gid) {
        newg->next = library.genres;
        library.genres = newg;
    } else {
        genre_t *prev = library.genres;
        while (prev->next != NULL && prev->next->gid < gid)
            prev = prev->next;

        newg->next = prev->next;
        prev->next = newg;
    }

    printf("DONE\n");
}

/* ============================================================
   add_book()
   Προσθέτει βιβλίο στο genre:
   - Εισαγωγή στη διπλά συνδεδεμένη λίστα ταξινομημένη κατά avg desc
   - Χωρίς βαθμολογίες αρχικά (avg = 0)
   - ΦΑΣΗ 1 + ΦΑΣΗ 2 (heap_pos reset)
   ============================================================ */
void add_book(int bid, int gid, char *title) {

    /* Εύρεση genre */
    genre_t *g = library.genres;
    while (g != NULL && g->gid != gid)
        g = g->next;

    if (!g) {
        printf("IGNORED\n");
        return;
    }

    /* Έλεγχος αν υπάρχει ήδη το bid */
    genre_t *t = library.genres;
    while (t != NULL) {
        book_t *bb = t->books;
        while (bb != NULL) {
            if (bb->bid == bid) {
                printf("IGNORED\n");
                return;
            }
            bb = bb->next;
        }
        t = t->next;
    }

    /* Δέσμευση νέου βιβλίου */
    book_t *nb = malloc(sizeof(book_t));
    if (!nb) {
        printf("IGNORED\n");
        return;
    }

    /* Αρχικοποίηση */
    nb->bid = bid;
    nb->gid = gid;
    strncpy(nb->title, title, TITLE_MAX);

    nb->sum_scores = 0;
    nb->n_reviews = 0;
    nb->avg = 0;
    nb->lost_flag = 0;

    nb->prev = NULL;
    nb->next = NULL;

    nb->heap_pos = -1;   /* PHASE 2 */

    /* Εισαγωγή στο τέλος (avg = 0 → πάντα στο τέλος) */
    if (g->books == NULL) {
        g->books = nb;
    } else {
        book_t *end = g->books;
        while (end->next != NULL)
            end = end->next;

        end->next = nb;
        nb->prev = end;
    }

    /* PHASE 2 → εισαγωγή στο AVL BookIndex */
    library.book_index = avl_insert(library.book_index, nb);

    printf("DONE\n");
}

/* ============================================================
   add_member()
   Προσθέτει μέλος:
   - Sorted list κατά sid
   - Δημιουργία tail-sentinel λίστας δανεισμών
   - PHASE 2: δημιουργεί Activity node
   ============================================================ */
void add_member(int sid, char *name) {

    /* Έλεγχος για διπλό sid */
    member_t *m = library.members;
    while (m != NULL) {
        if (m->sid == sid) {
            printf("IGNORED\n");
            return;
        }
        m = m->next;
    }

    /* Δέσμευση νέου μέλους */
    member_t *nm = malloc(sizeof(member_t));
    if (!nm) {
        printf("IGNORED\n");
        return;
    }

    nm->sid = sid;
    strncpy(nm->name, name, NAME_MAX);
    nm->next = NULL;

    /* PHASE 1 — Tail-sentinel list για loans */
    loan_t *sent = malloc(sizeof(loan_t));
    sent->sid = sid;
    sent->bid = -1;
    sent->next = NULL;
    nm->loans = sent;

    /* PHASE 2 — Activity node */
    MemberActivity *act = malloc(sizeof(MemberActivity));
    act->sid = sid;
    act->loans_count = 0;
    act->reviews_count = 0;
    act->score_sum = 0;

    act->next = library.activity;   /* push front */
    library.activity = act;

    nm->activity = act;

    /* Εισαγωγή στη sorted list κατά sid */
    if (library.members == NULL || sid < library.members->sid) {
        nm->next = library.members;
        library.members = nm;
    } else {
        member_t *p = library.members;
        while (p->next != NULL && p->next->sid < sid)
            p = p->next;

        nm->next = p->next;
        p->next = nm;
    }

    printf("DONE\n");
}

/* ============================================================
   loan_book() 
   Ο δανεισμός καταγράφεται στη loans list του μέλους.
   ============================================================ */
void loan_book(int sid, int bid) {

    /* --- Βρες το μέλος --- */
    member_t *m = library.members;
    while (m && m->sid != sid)
        m = m->next;

    if (!m) {
        printf("IGNORED\n");
        return;
    }

    /* --- Βρες το βιβλίο σε ΟΛΑ τα genres --- */
    genre_t *g = library.genres;
    book_t *b = NULL;

    while (g && !b) {
        for (book_t *tmp = g->books; tmp != NULL; tmp = tmp->next) {
            if (tmp->bid == bid) {
                b = tmp;
                break;
            }
        }
        g = g->next;
    }

    if (!b || b->lost_flag == 1) {
        printf("IGNORED\n");
        return;
    }

    /* --- ΠΡΟΣΘΗΚΗ ΔΑΝΕΙΣΜΟΥ --- */
    loan_t *newloan = malloc(sizeof(loan_t));
    newloan->sid  = sid;
    newloan->bid  = bid;
    newloan->next = m->loans; /* πριν το sentinel */
    m->loans = newloan;

    /* PHASE 2 activity update */
    m->activity->loans_count++;

    printf("DONE\n");
}

/* ============================================================
   return_book()
   Επιστροφή βιβλίου από μέλος.
   Υποστηρίζει:
     - status = "ok"   → επιστροφή με βαθμολογία
     - status = "lost" → δήλωση απώλειας
   Χειρισμός Phase 2:
     - ενημέρωση activity
     - ενημέρωση heap
     - ενημέρωση/διαγραφή AVL
   Προσοχή:
     - loan list έχει sentinel στο ΤΕΛΟΣ
     - ΠΟΤΕ δεν αγγίζουμε τον sentinel
   ============================================================ */
void return_book(int sid, int bid, char *status, int score) {

    /* === 1. Βρες μέλος === */
    member_t *m = library.members;
    while (m && m->sid != sid)
        m = m->next;

    if (!m) {
        printf("IGNORED\n");
        return;
    }

    /* === 2. Βρες sentinel του loan list === */
    loan_t *sentinel = m->loans;
    while (sentinel && sentinel->next != NULL)
        sentinel = sentinel->next;

    /* sentinel->bid = -1 (όπως ορίζεται κατά τη δημιουργία) */

    /* === 3. Βρες τον loan κόμβο ΠΡΙΝ το sentinel === */
    loan_t *prev = NULL;
    loan_t *ln   = m->loans;

    while (ln != sentinel) {
        if (ln->bid == bid)
            break;
        prev = ln;
        ln   = ln->next;
    }

    /* Αν ln == sentinel, το loan δεν υπάρχει */
    if (ln == sentinel) {
        printf("IGNORED\n");
        return;
    }

    /* === 4. Βρες το βιβλίο και το genre του === */
    genre_t *g = library.genres;
    book_t  *b = NULL;

    while (g && !b) {
        for (book_t *tmp = g->books; tmp != NULL; tmp = tmp->next) {
            if (tmp->bid == bid) {
                b = tmp;
                break;
            }
        }
        if (!b)
            g = g->next;
    }

    if (!b) {
        /* Αν δεν βρέθηκε, κάτι πάει πολύ λάθος */
        printf("IGNORED\n");
        return;
    }

    /* === 5. Αφαίρεση loan node === */
    if (prev == NULL) {
        /* Το πρώτο στοιχείο */
        m->loans = ln->next;
    } else {
        prev->next = ln->next;
    }
    free(ln);

    /* -------------------------------------------------------
       CASE A: STATUS = "lost"
       ------------------------------------------------------- */
    if (strcmp(status, "lost") == 0) {

        b->lost_flag = 1;
        g->lost_count++;

        /* Phase 2: Αφαίρεση από heap */
        if (b->heap_pos != -1)
            heap_remove(library.recommendations, b->heap_pos);

        /* Phase 2: Αφαίρεση από AVL */
        library.book_index = avl_delete(library.book_index, b->title);

        printf("DONE\n");
        return;
    }

    /* -------------------------------------------------------
       CASE B: STATUS = "ok"
       ------------------------------------------------------- */

    /* === 6. Ενημέρωση score/avg (Phase 1) === */
    b->sum_scores += score;
    b->n_reviews  += 1;
    b->avg         = b->sum_scores / b->n_reviews;

    /* === 7. Ενημέρωση activity (Phase 2) === */
    m->activity->reviews_count++;
    m->activity->score_sum += score;

    /* === 8. Ενημέρωση heap (Phase 2) === */
    heap_update(library.recommendations, b);

    /* === 9. Επανατοποθέτηση βιβλίου στη genre list ταξινομημένο === */

    /* Αφαίρεση από τρέχουσα θέση */
    if (b->prev) b->prev->next = b->next;
    else          g->books = b->next;

    if (b->next) b->next->prev = b->prev;

    /* Εύρεση νέας θέσης κατά avg DESC */
    book_t *curr = g->books;
    while (curr && curr->avg > b->avg)
        curr = curr->next;

    /* Εισαγωγή στην αρχή */
    if (curr == g->books) {
        b->prev = NULL;
        b->next = g->books;
        if (g->books)
            g->books->prev = b;
        g->books = b;
    }
    /* Εισαγωγή πριν από curr (μεσαία θέση) */
    else if (curr) {
        b->prev = curr->prev;
        b->next = curr;
        curr->prev->next = b;
        curr->prev = b;
    }
    /* Εισαγωγή στο τέλος */
    else {
        book_t *tail = g->books;
        while (tail && tail->next)
            tail = tail->next;

        if (tail) {
            tail->next = b;
            b->prev = tail;
            b->next = NULL;
        } else {
            /* Η λίστα ήταν άδεια */
            g->books = b;
            b->prev = b->next = NULL;
        }
    }

    printf("DONE\n");
}

/* ============================================================
   display_books()
   Εντολή: D

   Δημιουργεί τη display προβολή για όλα τα genres.
   Βήματα:
     1) Αν SLOTS <= 0 → καθαρίζουμε όλες τις προβολές.
     2) Υπολογίζουμε points ανά genre.
     3) Αν όλα τα points = 0 → καμία προβολή.
     4) Υπολογισμός quota = total_points / SLOTS.
     5) Αρχικός υπολογισμός seats = points / quota.
     6) Διανομή υπολοίπων θέσεων (greedy).
     7) Δημιουργία genre->display[] για κάθε genre.
   ============================================================ */
void display_books() {

    if (SLOTS <= 0) {
        for (genre_t *g = library.genres; g; g = g->next) {
            g->slots = 0;
            if (g->display) {
                free(g->display);
                g->display = NULL;
            }
        }
        printf("DONE\n");
        return;
    }

    /* Count genres */
    int genre_count = 0;
    for (genre_t *g = library.genres; g; g = g->next)
        genre_count++;

    if (genre_count == 0) {
        printf("DONE\n");
        return;
    }

    /* Local arrays */
    int *points   = calloc(genre_count, sizeof(int));
    int *seats    = calloc(genre_count, sizeof(int));
    double *extra = calloc(genre_count, sizeof(double));

    /* 1) compute points */
    int idx = 0;
    int total_points = 0;

    for (genre_t *g = library.genres; g; g = g->next, idx++) {
        int p = 0;
        for (book_t *b = g->books; b; b = b->next) {
            if (!b->lost_flag && b->n_reviews > 0)
                p += b->sum_scores;
        }
        points[idx] = p;
        total_points += p;
    }

    /* If no points → empty display */
    if (total_points == 0) {
        idx = 0;
        for (genre_t *g = library.genres; g; g = g->next, idx++) {
            g->slots = 0;
            if (g->display) free(g->display);
            g->display = NULL;
        }

        free(points);
        free(seats);
        free(extra);
        
        printf("DONE\n");
        return;
    }

    /* 2) quota allocation */
    double quota = (double) total_points / SLOTS;
    if (quota <= 0) quota = 1;

    idx = 0;
    int allocated = 0;

    for (genre_t *g = library.genres; g; g = g->next, idx++) {
        if (points[idx] == 0) {
            seats[idx] = 0;
            extra[idx] = 0;
            continue;
        }

        seats[idx] = (int)(points[idx] / quota);
        extra[idx] = points[idx] / quota - seats[idx];
        allocated += seats[idx];
    }

    /* 3) distribute leftover seats */
    int remaining = SLOTS - allocated;

    while (remaining > 0) {
        int best = -1;
        for (int i = 0; i < genre_count; i++) {
            if (points[i] == 0) continue;
            if (best == -1 || extra[i] > extra[best])
                best = i;
        }
        if (best == -1) break;
        seats[best]++;
        extra[best] = 0;
        remaining--;
    }

    /* 4) Apply seats to each genre and build display lists */
    idx = 0;
    for (genre_t *g = library.genres; g; g = g->next, idx++) {

        int s = seats[idx];

        if (g->display) {
            free(g->display);
            g->display = NULL;
        }

        g->slots = s;

        if (s <= 0) continue;

        g->display = malloc(sizeof(book_t*) * s);

        int count = 0;
        for (book_t *b = g->books; b && count < s; b = b->next) {
            if (!b->lost_flag)
                g->display[count++] = b;
        }

        while (count < s)
            g->display[count++] = NULL;
    }

    free(points);
    free(seats);
    free(extra);

    printf("DONE\n");
}

/* ============================================================
   print_genre()
   Εντολή: PG <gid>

   Εμφανίζει όλα τα βιβλία ενός genre:
     - bid και avg
   (Στη Φάση 1 δεν εμφανίζει τίτλους / lost flags.)
   ============================================================ */
void print_genre(int gid) {

    genre_t *g = library.genres;
    while (g && g->gid != gid)
        g = g->next;

    if (!g) {
        printf("IGNORED\n");
        return;
    }

    printf("Genre %d:\n", gid);

    if (!g->books) {
        printf("(empty)\n");
        return;
    }

    for (book_t *b = g->books; b; b = b->next) {
        printf("%d \"%s\" %d\n",
               b->bid,
               b->title,
               b->avg);
    }
}

/* ============================================================
   print_member_loans()
   Εντολή: PM sid

   Εμφανίζει όλα τα δανεισμένα βιβλία του μέλους.
   Παραλείπει τον sentinel κόμβο (l->next != NULL).
   ============================================================ */
void print_member_loans(int sid) {

    member_t *m = library.members;

    while (m != NULL && m->sid != sid)
        m = m->next;

    printf("Loans:\n");

    if (m == NULL || m->loans == NULL)
        return;

    /* Παράλειψη sentinel: εκτυπώνουμε μόνο l->next != NULL */
    for (loan_t *l = m->loans; l->next != NULL; l = l->next)
        printf("%d\n", l->bid);
}

/* ============================================================
   print_display()
   Εντολή: PD

   Εμφανίζει την προβολή που δημιούργησε η display_books().
   Μορφή (Φάση 1):
       Display:
       gid:
         bid, avg
   ============================================================ */
void print_display() {

    printf("Display:\n");

    for (genre_t *g = library.genres; g; g = g->next) {

        printf("%d:\n", g->gid);

        if (!g->display || g->slots == 0) {
            printf("(empty)\n");
            continue;
        }

        for (int i = 0; i < g->slots; i++) {
            book_t *b = g->display[i];
            if (b)
                printf("%d, %d\n", b->bid, b->avg);
        }
    }
}

/* ============================================================
   print_stats()
   Εντολή: PS

   Εμφανίζει:
      - SLOTS
      - points ανά genre
   ============================================================ */
void print_stats() {

    printf("SLOTS=%d\n", SLOTS);

    for (genre_t *g = library.genres; g != NULL; g = g->next) {

        int points = 0;

        for (book_t *b = g->books; b != NULL; b = b->next) {
            if (b->n_reviews > 0 && b->lost_flag == 0)
                points += b->sum_scores;
        }

        printf("%d: points=%d\n", g->gid, points);
    }
}



/************************************************************
 *  AVL TREE – BOOK INDEX (PHASE 2)
 *
 *  Χρησιμοποιείται για:
 *   - Αναζήτηση βιβλίου κατά τίτλο (εντολή F)
 *   - Διαγραφή από το index όταν βιβλίο γίνεται LOST
 *   - Ενημέρωση όταν αλλάζει τίτλος (εντολή U)
 *
 *  Key: title (char[ ])
 *  Value: pointer σε book_t
 ************************************************************/

/* Επιστρέφει το ύψος ενός κόμβου ή 0 αν είναι NULL */
int avl_height(BookNode *n) {
    return n ? n->height : 0;
}

/* Ενημερώνει το ύψος ενός κόμβου με βάση τα παιδιά του */
void avl_update_height(BookNode *n) {
    int hl = avl_height(n->lc);
    int hr = avl_height(n->rc);
    n->height = (hl > hr ? hl : hr) + 1;
}

/* Συντελεστής ισορροπίας = height(left) - height(right) */
int avl_balance(BookNode *n) {
    return n ? avl_height(n->lc) - avl_height(n->rc) : 0;
}

/* ----------------- Περιστροφές ----------------- */
/* Δεξιά περιστροφή γύρω από y */
BookNode* avl_rotate_right(BookNode *y) {
    BookNode *x  = y->lc;
    BookNode *T2 = x->rc;

    /* Περιστροφή */
    x->rc = y;
    y->lc = T2;

    /* Ενημέρωση υψών */
    avl_update_height(y);
    avl_update_height(x);

    return x;   /* νέα ρίζα του υποδέντρου */
}

/* Αριστερή περιστροφή γύρω από x */
BookNode* avl_rotate_left(BookNode *x) {
    BookNode *y  = x->rc;
    BookNode *T2 = y->lc;

    /* Περιστροφή */
    y->lc = x;
    x->rc = T2;

    /* Ενημέρωση υψών */
    avl_update_height(x);
    avl_update_height(y);

    return y;   /* νέα ρίζα του υποδέντρου */
}

/* ----------------- Εισαγωγή ----------------- */
/* Εισάγει ή αγνοεί βιβλίο αν υπάρχει ήδη τίτλος.
 * Επιστρέφει νέα ρίζα του υποδέντρου.
 */
BookNode* avl_insert(BookNode *node, book_t *book) {
    if (node == NULL) {
        BookNode *newnode = malloc(sizeof(BookNode));
        if (!newnode) return NULL;  /* αν αποτύχει malloc, απλά δεν μπαίνει */

        strncpy(newnode->title, book->title, TITLE_MAX);
        newnode->title[TITLE_MAX - 1] = '\0';

        newnode->book   = book;
        newnode->lc     = NULL;
        newnode->rc     = NULL;
        newnode->height = 1;

        return newnode;
    }

    int cmp = strcmp(book->title, node->title);

    if (cmp == 0) {
        /* Τίτλος υπάρχει ήδη → δεν επιτρέπουμε duplicate */
        return node;
    } else if (cmp < 0) {
        node->lc = avl_insert(node->lc, book);
    } else {
        node->rc = avl_insert(node->rc, book);
    }

    /* Ενημέρωση ύψους & ισορροπίας */
    avl_update_height(node);
    int balance = avl_balance(node);

    /* LL case */
    if (balance > 1 && strcmp(book->title, node->lc->title) < 0)
        return avl_rotate_right(node);

    /* RR case */
    if (balance < -1 && strcmp(book->title, node->rc->title) > 0)
        return avl_rotate_left(node);

    /* LR case */
    if (balance > 1 && strcmp(book->title, node->lc->title) > 0) {
        node->lc = avl_rotate_left(node->lc);
        return avl_rotate_right(node);
    }

    /* RL case */
    if (balance < -1 && strcmp(book->title, node->rc->title) < 0) {
        node->rc = avl_rotate_right(node->rc);
        return avl_rotate_left(node);
    }

    return node;
}

/* ----------------- Αναζήτηση ----------------- */
/* Βρίσκει κόμβο με δοθέντα τίτλο, αλλιώς NULL */
BookNode* avl_search(BookNode *root, const char *title) {
    while (root != NULL) {
        int cmp = strcmp(title, root->title);
        if (cmp == 0)
            return root;
        else if (cmp < 0)
            root = root->lc;
        else
            root = root->rc;
    }
    return NULL;
}

/* ----------------- Διαγραφή ----------------- */
/* Βρίσκει το αριστερότερο (ελάχιστο) στη δεξιά υποδένδρο */
BookNode* avl_find_min(BookNode *n) {
    while (n && n->lc != NULL)
        n = n->lc;
    return n;
}

/* Διαγράφει κόμβο με συγκεκριμένο title από το AVL.
 * Επιστρέφει νέα ρίζα του υποδέντρου.
 */
BookNode* avl_delete(BookNode *root, const char *title) {
    if (root == NULL)
        return NULL;

    int cmp = strcmp(title, root->title);

    if (cmp < 0) {
        root->lc = avl_delete(root->lc, title);
    } else if (cmp > 0) {
        root->rc = avl_delete(root->rc, title);
    } else {
        /* Βρέθηκε ο κόμβος προς διαγραφή */

        /* 0 ή 1 παιδί */
        if (root->lc == NULL || root->rc == NULL) {
            BookNode *tmp = root->lc ? root->lc : root->rc;
            free(root);
            return tmp;  /* μπορεί να είναι NULL */
        }

        /* 2 παιδιά → πάρε inorder successor από δεξιά υποδένδρο */
        BookNode *minRight = avl_find_min(root->rc);

        strncpy(root->title, minRight->title, TITLE_MAX);
        root->title[TITLE_MAX - 1] = '\0';
        root->book = minRight->book;

        root->rc = avl_delete(root->rc, minRight->title);
    }

    /* Αν μετά τη διαγραφή είναι NULL → επέστρεψέ το */
    if (!root) return NULL;

    /* Ενημέρωση ύψους & επαναϊσορρόπηση */
    avl_update_height(root);
    int balance = avl_balance(root);

    /* LL case */
    if (balance > 1 && avl_balance(root->lc) >= 0)
        return avl_rotate_right(root);

    /* LR case */
    if (balance > 1 && avl_balance(root->lc) < 0) {
        root->lc = avl_rotate_left(root->lc);
        return avl_rotate_right(root);
    }

    /* RR case */
    if (balance < -1 && avl_balance(root->rc) <= 0)
        return avl_rotate_left(root);

    /* RL case */
    if (balance < -1 && avl_balance(root->rc) > 0) {
        root->rc = avl_rotate_right(root->rc);
        return avl_rotate_left(root);
    }

    return root;
}



/************************************************************
 *  RECOMMENDATION HEAP (PHASE 2)
 *
 *  Max-Heap of books by:
 *     1) avg  (DESC)
 *     2) bid  (ASC)  tie-break
 *
 *  Κάθε βιβλίο κρατάει b->heap_pos για O(1) updates.
 ************************************************************/

/* ----------------------------------------------------------
   heap_create()
   Δημιουργεί νέο heap με αρχική capacity 4.
   ---------------------------------------------------------- */
RecHeap* heap_create() {
    RecHeap *h = malloc(sizeof(RecHeap));
    if (!h) return NULL;

    h->capacity = 4;
    h->size = 0;
    h->heap = malloc(sizeof(book_t*) * h->capacity);

    return h;
}

/* ----------------------------------------------------------
   heap_swap()
   Ανταλλάσσει δύο στοιχεία του heap και ενημερώνει heap_pos.
   ---------------------------------------------------------- */
void heap_swap(RecHeap *h, int i, int j) {
    book_t *tmp = h->heap[i];
    h->heap[i] = h->heap[j];
    h->heap[j] = tmp;

    h->heap[i]->heap_pos = i;
    h->heap[j]->heap_pos = j;
}

/* ----------------------------------------------------------
   heap_expand()
   Διπλασιάζει το capacity όταν χρειάζεται.
   ---------------------------------------------------------- */
void heap_expand(RecHeap *h) {
    h->capacity *= 2;
    h->heap = realloc(h->heap, sizeof(book_t*) * h->capacity);
}

/* ----------------------------------------------------------
   cmp_books_for_heap()
   Επιστρέφει 1 αν b1 πρέπει να είναι ΠΑΝΩ από b2.

   Δηλαδή:
     - Αν avg μεγαλύτερο
     - Αν avg ίσο → μικρότερο bid
   ---------------------------------------------------------- */
int cmp_books_for_heap(book_t *b1, book_t *b2) {
    if (b1->avg > b2->avg) return 1;
    if (b1->avg < b2->avg) return 0;

    /* tie-break by bid ascending */
    return (b1->bid < b2->bid);
}

/* ----------------------------------------------------------
   heap_sift_up()
   Ανασηκώνει στοιχείο προς τα πάνω αν πρέπει.
   ---------------------------------------------------------- */
void heap_sift_up(RecHeap *h, int pos) {
    while (pos > 0) {
        int parent = (pos - 1) / 2;
        if (!cmp_books_for_heap(h->heap[pos], h->heap[parent]))
            break;
        heap_swap(h, pos, parent);
        pos = parent;
    }
}

/* ----------------------------------------------------------
   heap_sift_down()
   Κατεβάζει στοιχείο προς τα κάτω μετά από remove/update.
   ---------------------------------------------------------- */
void heap_sift_down(RecHeap *h, int pos) {
    while (1) {
        int left = pos * 2 + 1;
        int right = pos * 2 + 2;
        int best = pos;

        if (left < h->size &&
            cmp_books_for_heap(h->heap[left], h->heap[best]))
            best = left;

        if (right < h->size &&
            cmp_books_for_heap(h->heap[right], h->heap[best]))
            best = right;

        if (best == pos)
            break;

        heap_swap(h, pos, best);
        pos = best;
    }
}

/* ----------------------------------------------------------
   heap_insert()
   Εισάγει βιβλίο στο heap αν avg > 0
   (αν avg == 0, απλά αγνοείται).
   ---------------------------------------------------------- */
void heap_insert(RecHeap *h, book_t *b) {
    if (b->avg <= 0) return;

    if (h->size == h->capacity)
        heap_expand(h);

    int pos = h->size++;
    h->heap[pos] = b;
    b->heap_pos = pos;

    heap_sift_up(h, pos);
}

/* ----------------------------------------------------------
   heap_remove()
   Αφαιρεί βιβλίο από το heap στη θέση pos.
   ---------------------------------------------------------- */
void heap_remove(RecHeap *h, int pos) {
    if (pos < 0 || pos >= h->size)
        return;

    /* Βάλε το τελευταίο στοιχείο στη θέση pos */
    int last = h->size - 1;
    h->heap[pos] = h->heap[last];
    h->heap[pos]->heap_pos = pos;

    h->size--;

    if (h->size == 0)
        return;

    /* Sift both ways — whichever applies */
    heap_sift_up(h, pos);
    heap_sift_down(h, pos);
}

/* ----------------------------------------------------------
   heap_update()
   Καλείται κάθε φορά που αλλάζει το avg του βιβλίου.
   ---------------------------------------------------------- */
void heap_update(RecHeap *h, book_t *b) {

    /* lost → remove */
    if (b->lost_flag == 1) {
        if (b->heap_pos != -1)
            heap_remove(h, b->heap_pos);
        return;
    }

    /* avg == 0 → remove */
    if (b->avg <= 0) {
        if (b->heap_pos != -1)
            heap_remove(h, b->heap_pos);
        return;
    }

    /* Αν δεν είναι στο heap → εισαγωγή */
    if (b->heap_pos == -1) {
        heap_insert(h, b);
        return;
    }

    /* Ήδη στο heap → adjust position */
    int pos = b->heap_pos;
    heap_sift_up(h, pos);
    heap_sift_down(h, pos);
}



/************************************************************
 *  MEMBER ACTIVITY (PHASE 2)
 *
 *  Για κάθε μέλος κρατάμε:
 *    - loans_count   : πόσους δανεισμούς έκανε
 *    - reviews_count : πόσες επιστροφές με βαθμολογία έκανε
 *    - score_sum     : συνολικό άθροισμα βαθμών
 *
 *  Συνδέεται στη δομή member_t ως:
 *      member->activity = <pointer>
 *
 *  Η λίστα library.activity συνδέει όλα τα Activity nodes.
 ************************************************************/

/* ----------------------------------------------------------
   create_activity()
   Δημιουργεί νέο Activity node για μέλος (sid).
   Καλείται μέσα στην add_member().
   ---------------------------------------------------------- */
MemberActivity* create_activity(int sid) {
    MemberActivity *act = malloc(sizeof(MemberActivity));
    if (!act) return NULL;

    act->sid = sid;
    act->loans_count   = 0;
    act->reviews_count = 0;
    act->score_sum     = 0;

    act->next = NULL;
    return act;
}

/* ----------------------------------------------------------
   notify_loan()
   Καλείται μέσα στη loan_book().
   Αυξάνει τον μετρητή loans_count.
   ---------------------------------------------------------- */
void notify_loan(member_t *m) {
    if (m && m->activity)
        m->activity->loans_count++;
}

/* ----------------------------------------------------------
   notify_review()
   Καλείται μέσα στη return_book() όταν status = "ok".
   Αυξάνει reviews_count και προσθέτει το score.
   ---------------------------------------------------------- */
void notify_review(member_t *m, int score) {
    if (m && m->activity) {
        m->activity->reviews_count++;
        m->activity->score_sum += score;
    }
}

/* ----------------------------------------------------------
   print_active_members()
   Εντολή: AM

   Εκτυπώνει τα μέλη με activity με τη μορφή:
      sid, loans_count, reviews_count, score_sum

   Τα γράφει με σειρά που βρίσκονται στη activity list.
   ---------------------------------------------------------- */
void print_active_members() {

    MemberActivity *act = library.activity;

    if (!act) {
        printf("(empty)\n");
        return;
    }

    while (act) {
        printf("%d, %d, %d, %d\n",
               act->sid,
               act->loans_count,
               act->reviews_count,
               act->score_sum);
        act = act->next;
    }
}



/************************************************************
 *  FREE MEMORY + SYSTEM STATS
 *
 *  Περιλαμβάνει:
 *    - Καθαρισμό των genres και βιβλίων
 *    - Καθαρισμό των members και loans
 *    - Καθαρισμό του activity list
 *    - Καθαρισμό AVL tree
 *    - Καθαρισμό Heap
 *    - Εκτύπωση στατιστικών (εντολή X)
 ************************************************************/

/* ----------------------------------------------------------
   free_avl()
   Διαγράφει αναδρομικά όλα τα nodes από το AVL tree.
   ---------------------------------------------------------- */
void free_avl(BookNode *root) {
    if (!root) return;
    free_avl(root->lc);
    free_avl(root->rc);
    free(root);
}

/* ----------------------------------------------------------
   free_heap()
   Καθαρίζει το recommendation heap.
   ΔΕΝ ελευθερώνει βιβλία — μόνο τον πίνακα.
   ---------------------------------------------------------- */
void free_heap(RecHeap *h) {
    if (!h) return;
    if (h->heap) free(h->heap);
    free(h);
}

/* ----------------------------------------------------------
   free_activity()
   Καθαρίζει ολόκληρη τη λίστα MemberActivity.
   ---------------------------------------------------------- */
void free_activity(MemberActivity *act) {
    while (act) {
        MemberActivity *next = act->next;
        free(act);
        act = next;
    }
}

/* ----------------------------------------------------------
   free_members()
   Καθαρίζει:
     - loan list με sentinel
     - member struct
   ---------------------------------------------------------- */
void free_members(member_t *m) {

    while (m) {
        member_t *nextm = m->next;

        /* καθάρισε loan list */
        loan_t *ln = m->loans;
        while (ln) {
            loan_t *nextln = ln->next;
            free(ln);
            ln = nextln;
        }

        /* activity ΔΕΝ καθαρίζεται εδώ — γίνεται κεντρικά */
        free(m);
        m = nextm;
    }
}

/* ----------------------------------------------------------
   free_genres()
   Καθαρίζει:
     - books (διπλά συνδεδεμένη λίστα)
     - display arrays
     - genre structs
   ---------------------------------------------------------- */
void free_genres(genre_t *g) {

    while (g) {
        genre_t *nextg = g->next;

        /* free διπλή λίστα βιβλίων */
        book_t *b = g->books;
        while (b) {
            book_t *nextb = b->next;
            free(b);
            b = nextb;
        }

        /* free display array */
        if (g->display)
            free(g->display);

        free(g);
        g = nextg;
    }
}

/* ----------------------------------------------------------
   free_library()
   Κεντρικός καθαρισμός όλων των δομών του συστήματος.
   ---------------------------------------------------------- */
void free_library() {

    /* genres + books + display */
    free_genres(library.genres);
    library.genres = NULL;

    /* members + loans */
    free_members(library.members);
    library.members = NULL;

    /* activity list */
    free_activity(library.activity);
    library.activity = NULL;

    /* heap */
    free_heap(library.recommendations);
    library.recommendations = heap_create();  // reset

    /* avl */
    free_avl(library.book_index);
    library.book_index = NULL;

    /* reset SLOTS */
    SLOTS = 0;
}

/* ------------------------------------------------------------
   FIND_BOOK
   Αναζητά ένα βιβλίο μέσω του AVL tree με βάση τον τίτλο.
   Αν βρεθεί, εμφανίζει όλα τα απαραίτητα στοιχεία του.
   Αν δεν υπάρχει, τυπώνει NOTFOUND.
   ------------------------------------------------------------ */
void find_book(const char *title) {

    /* Αναζήτηση τίτλου μέσω του AVL tree */
    BookNode *node = avl_search(library.book_index, title);

    if (node == NULL) {
        printf("NOTFOUND\n");
        return;
    }

    book_t *b = node->book;

    /* Εκτύπωση αποτελέσματος */
    printf("FOUND %d \"%s\" %d %d %d\n",
           b->bid,
           b->title,
           b->sum_scores,
           b->n_reviews,
           b->avg);
}

/* ------------------------------------------------------------
   UPDATE_TITLE
   Ενημερώνει τον τίτλο ενός βιβλίου.
   1. Εντοπίζει το βιβλίο μέσω των λιστών των genres.
   2. Το αφαιρεί από το AVL tree με τον παλιό τίτλο.
   3. Αλλάζει τον τίτλο.
   4. Εισάγει ξανά το βιβλίο στο AVL tree.
   ------------------------------------------------------------ */
void update_title(int bid, const char *newtitle) {

    /* Βρες το βιβλίο μέσω των συνδεδεμένων λιστών */
    book_t *b = NULL;

    for (genre_t *g = library.genres; g != NULL; g = g->next) {
        for (book_t *bk = g->books; bk != NULL; bk = bk->next) {
            if (bk->bid == bid) {
                b = bk;
            }
        }
    }

    /* Αν το βιβλίο δεν υπάρχει */
    if (b == NULL) {
        printf("IGNORED\n");
        return;
    }

    /* Αφαίρεση από AVL με τον παλιό τίτλο */
    library.book_index = avl_delete(library.book_index, b->title);

    /* Αλλαγή του τίτλου */
    strcpy(b->title, newtitle);

    /* Εισαγωγή του βιβλίου ξανά με τον νέο τίτλο */
    library.book_index = avl_insert(library.book_index, b);

    printf("DONE\n");
}

/* ------------------------------------------------------------
   TOP_K_BOOKS
   Εμφανίζει τα k κορυφαία βιβλία από το heap συστάσεων.
   - Αν το k είναι 0 ή το heap άδειο → NONE.
   - Αν το k > πλήθος, περιορίζεται στο size του heap.
   Τα βιβλία αποθηκεύονται στο heap ως book_t*.
   ------------------------------------------------------------ */
void top_k_books(int k) {

    RecHeap *h = library.recommendations;

    if (!h || h->size == 0 || k <= 0) {
        printf("NONE\n");
        return;
    }

    if (k > h->size)
        k = h->size;

    for (int i = 0; i < k; i++) {
        book_t *b = h->heap[i];
        printf("%d \"%s\" %d\n",
               b->bid,
               b->title,
               b->avg);
    }
}



/* ============================================================
   print_system_stats()
   Εντολή: X

   Εκτυπώνει:
      - Σύνολο Genres
      - Σύνολο Books
      - Σύνολο Members
      - Σύνολο Loans
      - Lost books
      - Reviews (total σε όλα τα βιβλία)
   ============================================================ */

void print_system_stats() {

    int genres  = 0;
    int books   = 0;
    int members = 0;
    int loans   = 0;
    int lost    = 0;
    int reviews = 0;

    /* -------------------------------------------------------
       Count genres + books + lost + reviews
       ------------------------------------------------------- */
    for (genre_t *g = library.genres; g != NULL; g = g->next) {

        genres++;

        for (book_t *b = g->books; b != NULL; b = b->next) {

            books++;

            if (b->lost_flag)
                lost++;

            /* Sum of all reviews in all books */
            reviews += b->n_reviews;
        }
    }

    /* -------------------------------------------------------
       Count members + loan nodes (including sentinel)
       ------------------------------------------------------- */
    for (member_t *m = library.members; m != NULL; m = m->next) {

        members++;

        for (loan_t *ln = m->loans; ln != NULL; ln = ln->next)
            loans++;
    }

    /* -------------------------------------------------------
       Αφαίρεση των sentinel loan nodes
       (1 sentinel ανά μέλος)
       ------------------------------------------------------- */
    loans -= members;

    /* -------------------------------------------------------
       Output
       ------------------------------------------------------- */
    printf("Genres: %d\n", genres);
    printf("Books: %d\n", books);
    printf("Members: %d\n", members);
    printf("Loans: %d\n", loans);
    printf("Lost: %d\n", lost);
    printf("Reviews: %d\n", reviews);
}


/* ============================================================
   Διαβάζει τίτλο που βρίσκεται μέσα σε εισαγωγικά.
   Παράδειγμα input:   "The Hobbit"
   Αποτέλεσμα dest =   The Hobbit
   ============================================================ */
void read_quoted(char *dest, const char *src)
{
    const char *start = strchr(src, '"');
    if (!start) {
        dest[0] = '\0';
        return;
    }
    start++;  // skip first "
    const char *end = strchr(start, '"');
    if (!end) {
        dest[0] = '\0';
        return;
    }
    size_t len = end - start;
    strncpy(dest, start, len);
    dest[len] = '\0';
}

int SLOTS = 0; // Παγκόσμια μεταβλητή για τις θέσεις της display

int main(int argc, char *argv[]) {

    /* ------------------------------------------------------
       Επιλογή πηγής εισόδου (stdin ή αρχείο)
       ------------------------------------------------------ */
    FILE *input = stdin;
    

    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
            return 1;
        }
    }

    /* ------------------------------------------------------
       Αρχικοποίηση όλων των δομών της βιβλιοθήκης
       ------------------------------------------------------ */
    library.genres          = NULL;
    library.members         = NULL;
    library.activity        = NULL;
    library.book_index      = NULL;
    library.recommendations = heap_create();
    SLOTS = 0;

    char line[LINE_MAX];

    /* ------------------------------------------------------
       Κύριο loop εισόδου
       ------------------------------------------------------ */
    while (fgets(line, LINE_MAX, input)) {

        if (line[0] == '\n' || line[0] == '\0')
            continue;

        char cmd;
        int offset = 0;
        sscanf(line, "%c%n", &cmd, &offset);
        char *args = line + offset;

        switch (cmd) {


/* ============================================================
   CASE G  → ADD GENRE
   ============================================================ */
        case 'G': {
            int gid;
            char name[NAME_MAX];
            if (sscanf(args, "%d \"%[^\"]\"", &gid, name) == 2)
                add_genre(gid, name);
            else
                printf("IGNORED\n");
            break;
        }


/* ============================================================
   CASE B → ADD BOOK ή BF (FREE LIBRARY)
   ============================================================ */
    case 'B': {

        // Πάρε το πρώτο token μέσα στα args (ώστε να δούμε αν είναι "F" ή αριθμός)
        char cmd2[8];
        if (sscanf(args, "%s", cmd2) != 1) {
            printf("IGNORED\n");
            break;
        }

        /* ---------------------------------- */
        /* BF → Free Library                  */
        /* ---------------------------------- */
        if (strcmp(cmd2, "F") == 0) {
            free_library();
            printf("DONE\n");
            break;
        }

        /* ---------------------------------- */
        /* B bid gid "title" → Add Book       */
        /* ---------------------------------- */
        int bid, gid;
        char title[TITLE_MAX];

        if (sscanf(args, "%d %d \"%[^\"]\"", &bid, &gid, title) == 3) {
            add_book(bid, gid, title);
        } else {
            printf("IGNORED\n");
        }

        break;
    }


/* ============================================================
   CASE P → (PG, PD, PM, PS)
   ============================================================ */
        case 'P': {
            char sub;
            if (sscanf(args, "%c", &sub) != 1) {
                printf("IGNORED\n");
                break;
            }

            switch (sub) {

                /* PG gid */
                case 'G': {
                    int gid;
                    if (sscanf(args + 1, "%d", &gid) == 1)
                        print_genre(gid);
                    else
                        printf("IGNORED\n");
                    break;
                }

                /* PD */
                case 'D':
                    print_display();
                    break;

                /* PM sid */
                case 'M': {
                    int sid;
                    if (sscanf(args + 1, "%d", &sid) == 1)
                        print_member_loans(sid);
                    else
                        printf("IGNORED\n");
                    break;
                }

                /* PS */
                case 'S':
                    print_stats();
                    break;

                default:
                    printf("IGNORED\n");
            }
            break;
        }


/* ============================================================
   MEMBERS
   ============================================================ */
        case 'M': {
            int sid;
            char name[NAME_MAX];
            if (sscanf(args, "%d \"%[^\"]\"", &sid, name) == 2)
                add_member(sid, name);
            else
                printf("IGNORED\n");
            break;
        }


/* ============================================================
   LOAN / RETURN
   ============================================================ */
        case 'L': {
            int sid, bid;
            if (sscanf(args, "%d %d", &sid, &bid) == 2)
                loan_book(sid, bid);
            else
                printf("IGNORED\n");
            break;
        }

        case 'R': {
            int sid, bid, score;
            char status[16];
            if (sscanf(args, "%d %d %s %d", &sid, &bid, status, &score) == 4)
                return_book(sid, bid, status, score);
            else
                printf("IGNORED\n");
            break;
        }


/* ============================================================
   DISPLAY (S, D)
   ============================================================ */
        case 'S': {
            int slots;
            if (sscanf(args, "%d", &slots) == 1)
                set_slots(slots);
            else
                printf("IGNORED\n");
            break;
        }

        case 'D':
            display_books();
            break;


/* ============================================================
   PHASE 2 — FIND BOOK (ΔΙΟΡΘΩΜΕΝΟ)
   ============================================================ */
    case 'F': {
        char title[TITLE_MAX];
        read_quoted(title, args);  // σωστή ανάγνωση τίτλου με κενά
        if (title[0] == '\0') {
            printf("IGNORED\n");
            break;
        }
        find_book(title);
        break;
    }


/* ============================================================
   PHASE 2 — UPDATE TITLE (ΔΙΟΡΘΩΜΕΝΟ)
   ============================================================ */
    case 'U': {
        int bid;
        if (sscanf(args, "%d", &bid) != 1) {
            printf("IGNORED\n");
            break;
        }

        char newtitle[TITLE_MAX];
        read_quoted(newtitle, args);
        if (newtitle[0] == '\0') {
            printf("IGNORED\n");
            break;
        }

        update_title(bid, newtitle);
        break;
    }


/* ============================================================
   PHASE 2 — TOP K BOOKS
   ============================================================ */
        case 'T': {
            int k;
            if (sscanf(args, "%d", &k) == 1)
                top_k_books(k);
            else
                printf("IGNORED\n");
            break;
        }


/* ============================================================
   PHASE 2 — ACTIVE MEMBERS
   ============================================================ */
        case 'A': {
            char sub;
            if (sscanf(args, "%c", &sub) == 1 && sub == 'M')
                print_active_members();
            else
                printf("IGNORED\n");
            break;
        }


/* ============================================================
   PHASE 2 — SYSTEM STATS
   ============================================================ */
        case 'X':
            print_system_stats();
            break;


/* ============================================================
   INVALID COMMAND
   ============================================================ */
        default:
            printf("IGNORED\n");
        }
    }

    /* ------------------------------------------------------
       EOF — καθαρισμός
       ------------------------------------------------------ */
    free_library();

    if (input != stdin)
        fclose(input);

    return 0;
}
