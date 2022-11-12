#include <stdio.h>
#include <stdlib.h>
#define PIPS 7
#define MAX 15
#define TBZ 20000

typedef struct block{
    int left, right;
    struct block *left_block, *right_block;
} block;

typedef struct {
    block* head;
    block* tail;
} snake;

typedef struct game {
    snake snake;
    int hands[PIPS][PIPS]; // hands[i][j] means who owns the domino [i|j]. 0 for first player, 1 for second player, -1 for the table, -2 is undecided.
    int hand_sizes[2];
    int turn;
    int pass_counter;
    block *block_set[PIPS][PIPS];
} game;

typedef struct move {
    int left;
    int right;
    int head;
} move;

// tree structure for the game tree
typedef struct node {
    move move;
    int score;
    struct node *children[MAX];
    int num_children;
} node;

// print a tree recursively in a way like how the "tree" command does
void print_tree(node *n, int depth, FILE *f) {
    for(int i = 0; i < depth; i++) {
        fprintf(f, ".");
    }
    fprintf(f, "%c[%d|%d] = %d\n", n->move.head ? 'r' : 'l', n->move.left, n->move.right, n->score);
    for(int i = 0; i < n->num_children; i++) {
        print_tree(n->children[i], depth + 1, f);
    }

}

void print_wrapper(node *n, char *filename) {
    FILE *f = fopen(filename, "w");
    char BUFFER[TBZ];
    setvbuf(f, BUFFER, _IOFBF, TBZ);
    print_tree(n, 0, f);
    fclose(f);
}

// initialize a game
void init_game(game *g) {
    int i, j;
    for (i = 0; i < PIPS; i++) {
        for (j = 0; j < PIPS; j++) {
            g->block_set[i][j] = (block*) malloc(sizeof(block));
            g->block_set[i][j]->left = i;
            g->block_set[i][j]->right = j;
            g->block_set[i][j]->left_block = NULL;
            g->block_set[i][j]->right_block = NULL;
        }
    }
    g->snake.head = NULL;
    g->snake.tail = NULL;
    g->pass_counter = 0;
    for(int i = 0; i < 2; i++) {
        g->hand_sizes[i] = 0;
    }
}

node *new_node(move move, int score) {
    node *n = malloc(sizeof(node));
    n->move = move;
    n->score = score;
    n->num_children = 0;
    return n;
}

int possible_move(snake* s, move move) {
    if(s->head == NULL) {
        return 1;
    }
    if(move.head) {
        if(s->head->right == move.left) {
            return 1;
        }
    } else {
        if(s->tail->left == move.right) {
            return 1;
        }
    }
}

int playable_domino(snake *s, int left, int right) {
    int l = s->head->left;
    int r = s->tail->right;
    if (left == l || left == r || right == l || right == r) {
        return 1;
    }
    return 0;
}

void print_snake(game *g) {
    block *b = g->snake.tail;
    printf("Snake: ");
    while(b!=NULL){
        printf("[%d|%d]", b->left, b->right);
        b = b->right_block;
    }
    printf("\n");
}

void print_hand(game *g, int i) {
    printf("Player %d: ", i);
    for(int j = 0; j < PIPS; j++) {
        for(int k = 0; k <= j; k++) {
            if(g->hands[k][j] == i) {
                printf("[%d|%d] ", k, j);
            }
        }
    }
    printf("\n");
}

void print_game(game *g){
    printf("--------------------\n");
    // print the hands then the snake.
    for(int i = 0; i < 2; i++) {
        print_hand(g, i);
    }
    print_snake(g);
    // print whose turn it is and the pass counter. also print possible_move moves for the current player.
    printf("Turn: %d\n", g->turn);
    printf("Pass Counter: %d\n", g->pass_counter);
}

void print_moves(move moves[], int n){
    printf("Possible moves: ");
    for(int i = 0; i < n; i++) {
        printf("[%d|%d] ", moves[i].left, moves[i].right);
    }
    printf("\n");
}

// print the hands[PIPS][PIPS] matrix for debugging purposes.
void print_hands(int hands[PIPS][PIPS]) {
    printf("Hands matrix:\n");
    for(int i = 0; i < PIPS; i++) {
        for(int j = 0; j < PIPS; j++) {
            printf("%d ", hands[i][j]);
        }
        printf("\n");
    }
}

void get_moves(game *g, move moves[PIPS+1], int *n){
    *n = 0;
    int left = g->snake.tail->left;
    int right = g->snake.head->right;
    for(int i = 0; i < PIPS; i++){
        if(g->hands[i][left] == g->turn && possible_move(&g->snake, (move){i, left, 0})){
            moves[*n].left = i;
            moves[*n].right = left;
            moves[*n].head = 0;
            (*n)++;
        }
    }
    if(left != right){
        for(int i = 0; i < PIPS; i++){
            if(g->hands[right][i] == g->turn && possible_move(&g->snake, (move){right, i, 1})){
                moves[*n].left = right;
                moves[*n].right = i;
                moves[*n].head = 1;
                (*n)++;
            }
        }
    }
}

void sort_moves(move moves[], int n){
    return;
}

void add_block(snake* s, block *block, int head) {
    if(s->head == NULL){
        s->head = block;
        s->tail = block;
    } else if (head) {
        block->left_block = s->head;
        block->right_block = NULL;
        s->head->right_block = block;
        s->head = block;
    } else {
        block->right_block = s->tail;
        block->left_block = NULL;
        s->tail->left_block = block;
        s->tail = block;
    }
}

// remove the block on the head or tail of the snake depending on the head parameter. isolate the block by setting it's own pointers to NULL. don't forget about the case when there is only one block.
void remove_block(snake* s, int head) {
    if(s->head == s->tail){
        s->head = NULL;
        s->tail = NULL;
    } else if (head) {
        s->head = s->head->left_block;
        s->head->right_block->left_block = NULL;
        s->head->right_block->right_block = NULL;
        s->head->right_block = NULL;
    } else {
        s->tail = s->tail->right_block;
        s->tail->left_block->right_block = NULL;
        s->tail->left_block->left_block = NULL;
        s->tail->left_block = NULL;
    }
}

int max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

int min(int a, int b) {
    if (a < b) {
        return a;
    }
    return b;
}

void domove(game *g, move move) {
    add_block(&g->snake, g->block_set[move.left][move.right], move.head);
    g->hands[move.left][move.right] = -1;
    g->hands[move.right][move.left] = -1;
    g->hand_sizes[g->turn]--;
    g->turn = !g->turn;
    g->pass_counter = 0;
}

void unmove(game *g, move move) {
    remove_block(&g->snake, move.head);
    g->turn = !g->turn;
    g->hands[move.left][move.right] = g->turn;
    g->hands[move.right][move.left] = g->turn;
    g->hand_sizes[g->turn]++;
}

void pass(game *g) {
    g->turn = !g->turn;
    g->pass_counter = g->pass_counter + 1;
}

void unpass(game *g){
    g->turn = !g->turn;
    g->pass_counter = g->pass_counter - 1;
}

int over(game *g) {
    return g->pass_counter == 2 || g->hand_sizes[0] == 0 || g->hand_sizes[1] == 0;
}

int endgame_evaluation(game *g){
    int pips[2] = {0, 0};
    for(int i = 0; i < PIPS; i++) {
        for(int j = 0; j <= i; j++) {
            switch(g->hands[i][j]) {
                case 0:
                    pips[0] += i + j;
                    break;
                case 1:
                    pips[1] += i + j;
            }
        }
    }
    return pips[0] * (pips[0] > pips[1]) - pips[1] * (pips[1] > pips[0]);
}

int heuristic_evaluation(game *g){
    int pips = 0;
    for(int i = 0; i < PIPS; i++) {
        for(int j = 0; j <= i; j++) {
            switch(g->hands[i][j]) {
                case 0:
                    pips += i + j;
                    break;
                case 1:
                    pips -= i + j;
            }
        }
    }
    return pips;
}

node *minmax_tree(game *g, int depth, int alpha, int beta, int maximizing_player, move move){
    if(over(g))
        return new_node(move, endgame_evaluation(g) * 1000);
    if(depth == 0)
        return new_node(move, heuristic_evaluation(g));
    struct move moves[MAX];
    int n = 0;
    get_moves(g, moves, &n);
    sort_moves(moves, n);
    node *move_node = new_node(move, maximizing_player ? INT_MIN : INT_MAX);
    switch(n){
    case 0:
        pass(g);
        move_node->children[0] = minmax_tree(g, depth, alpha, beta, !maximizing_player, (struct move){-1, -1, -1});
        move_node->score = move_node->children[0]->score;
        move_node->num_children = 1;
        unpass(g);
        return move_node;
    case 1:
        domove(g, moves[0]);
        move_node->children[0] = minmax_tree(g, depth-1, alpha, beta, !maximizing_player, moves[0]);
        move_node->score = move_node->children[0]->score;
        move_node->num_children = 1;
        unmove(g, moves[0]);
        return move_node;
    default:
        if(maximizing_player){
            move_node->num_children = n;
            for(int i = 0; i < n; i++){
                domove(g, moves[i]);
                move_node->children[i] = minmax_tree(g, depth-1, alpha, beta, 0, moves[i]);
                unmove(g, moves[i]);
                move_node->score = max(move_node->score, move_node->children[i]->score);
                alpha = max(alpha, move_node->score);
                if(beta <= alpha){
                    move_node->num_children = i+1;
                    break;
                }
            }
        } else {
            move_node->num_children = n;
            for(int i = 0; i < n; i++){
                domove(g, moves[i]);
                move_node->children[i] = minmax_tree(g, depth-1, alpha, beta, 1, moves[i]);
                unmove(g, moves[i]);
                move_node->score = min(move_node->score, move_node->children[i]->score);
                beta = min(beta, move_node->score);
                if(beta <= alpha){
                    move_node->num_children = i+1;
                    break;
                }
            }
        }
        return move_node;   
    }
}

// implement the same function as above, but only return the score of the given move instead of the move tree.
int minmax(game *g, int depth, int alpha, int beta, int maximizing_player){
    if(over(g))
        return endgame_evaluation(g) * 1000;
    if(depth == 0)
        return heuristic_evaluation(g);
    struct move moves[MAX];
    int n = 0, score;
    get_moves(g, moves, &n);
    sort_moves(moves, n);
    switch(n){
    case 0:
        pass(g);
        score = minmax(g, depth, alpha, beta, !maximizing_player);
        unpass(g);
        return score;
    case 1:
        domove(g, moves[0]);
        score = minmax(g, depth-1, alpha, beta, !maximizing_player);
        unmove(g, moves[0]);
        return score;
    default:
        if(maximizing_player){
            for(int i = 0; i < n; i++){
                domove(g, moves[i]);
                score = minmax(g, depth-1, alpha, beta, 0);
                unmove(g, moves[i]);
                alpha = max(alpha, score);
                if(beta <= alpha)
                    break;
            }
            return alpha;
        } else {
            for(int i = 0; i < n; i++){
                domove(g, moves[i]);
                score = minmax(g, depth-1, alpha, beta, 1);
                unmove(g, moves[i]);
                beta = min(beta, score);
                if(beta <= alpha)
                    break;
            }
            return beta;
        }
    }
}


void get_hands(int hands[PIPS][PIPS], int hand_sizes[2]) {
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            printf("[%d|%d]: ", i, j);
            scanf("%d", &hands[i][j]);
            hands[j][i] = hands[i][j];
            hand_sizes[hands[i][j]]++;
        }
    }
}

move best_move(game *g, int depth){
    struct move moves[MAX], best_move;
    int n = 0, best_score;
    get_moves(g, moves, &n);
    sort_moves(moves, n);
    best_score = g->turn ? INT_MIN : INT_MAX;
    for(int i = 0; i < n; i++){
        domove(g, moves[i]);
        int score = minmax(g, depth-1, INT_MIN, INT_MAX, g->turn);
        unmove(g, moves[i]);
        if((g->turn && score > best_score) || (!g->turn && score < best_score)){
            best_score = score;
            best_move = moves[i];
        }
    }
    printf("best score: %d, best move: %d %d %d\n", best_score, best_move.left, best_move.right, best_move.head);
    return best_move;
}

node *game_theoretic_tree(int depth, game *g, move *first, int write){
    if(g == NULL){
        g = malloc(sizeof(game));
        init_game(g);
        get_hands(g->hands, g->hand_sizes);
        first = malloc(sizeof(move));
        *first = (struct move){PIPS-1, PIPS-1, 0};
        g->turn = g->hands[PIPS-1][PIPS-1];
        domove(g, *first);
    }
    node *root = minmax_tree(g, depth, INT_MIN, INT_MAX, g->turn, *first);
    if(write) print_wrapper(root, "C:\\Users\\PC\\Desktop\\FL\\tree.txt");
    return root;
}

void play(){
    game *g = malloc(sizeof(game));
    init_game(g);
    get_hands(g->hands, g->hand_sizes);
    // the first move has to be the biggest double, which is just [PIPS-1|PIPS-1]. so we initialize the turn with who has it and play it right away.
    g->turn = g->hands[PIPS-1][PIPS-1];
    domove(g, (struct move){PIPS-1, PIPS-1, 0});
    int n;
    move moves[MAX];
    do {
        print_game(g);
        get_moves(g, moves, &n);
        print_moves(moves, n);
        printf("%d moves\n", n);
        switch(n){
        case 0:
            pass(g);
            break;
        case 1:
            domove(g, moves[0]);
            break;
        default:
            move move;
            while(1){
                scanf("%d %d %d", &move.left, &move.right, &move.head);
                if(move.left == -1){
                    if(move.head == -1){
                        move = best_move(g, /*move.right*/20); // using move.right as depth when move.left is -1.
                        continue;
                    }
                    move = best_move(g, /*move.right*/20); // this isn't the best way to do this, but it works.
                }
                domove(g, move);
                break;
            }
        }
    } while(!over(g));
    print_game(g);
    printf("game over, score: %d\n", endgame_evaluation(g));
}

int main(){
    play();
    //game_theoretic_tree(13, NULL, NULL, 1);
    return 0;
}