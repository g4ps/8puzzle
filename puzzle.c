#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>


//Implementation for a bsearch tree for holding keys
//Need this stuff to hold passed states
struct
t_node
{
  struct t_node *left, *right, *parent;
  long key;
};

struct
b_tree
{
  struct t_node *root;  
};


//Somewhat of a hash function for a puzzle
long
getkey(const void *p, size_t size)
{
  long ret = 0;
  const int *t = p;
  for (int i = 0; i < (size / sizeof(int)); i++) {
    ret *= 16;
    ret += t[i];
  }
  return ret;
}

int
insert(struct b_tree* t, long key)
{
  struct t_node *curr = t -> root;
  struct t_node *prev = NULL;
  while (curr) {
    prev = curr;
    if (key < curr -> key)
      curr = curr -> left;
    else
      curr = curr -> right;
  }
  struct t_node *temp = malloc(sizeof(struct t_node));
  if (prev == NULL) {
    t -> root = temp;
  }
  temp -> key = key;
  temp -> left = temp -> right = NULL;
  temp -> parent = prev;
  if (prev) {
    if (key > temp -> parent -> key) {
      temp -> parent -> right = temp;      
    }
    else {
      temp -> parent -> left = temp;
    }
  }
}

int
is_set(struct b_tree* t, long key)
{
  struct t_node *curr = t -> root;
  while (curr) {
    if (curr -> key == key)
      return 1;
    else if (key < curr -> key)
      curr = curr -> left;
    else
      curr = curr -> right;
  }
  return 0;
}


//Guess what does this thing do
enum
dir {LEFT, RIGHT, UP, DOWN};


//Only global variable in the code
//Needed for visual output grade i.e. how mush stuff
//does program print
int v_state = 0;

//Predicat for checking if the puzzle is solved
int
is_solved(const int *p)
{
  static int *buf = NULL;
  if (buf == NULL) {
    buf = calloc(9, sizeof(int));
    for (int i = 0; i < 9; i++) {
      buf[i] = i;
    }
  }
  int ret = memcmp(p, buf, sizeof(int) * 9);
  if (ret == 0)
    return 1;
  else
    return 0;
}


//Couple of hueristics
long
not_in_place(const int *p)
{
  long total = 0;
  for (int i = 0; i < 9; i++) {
    if (i != p[i] && i != 0) {
      total++;
    }
  }
  return total;
}

long
l_abs(long i) //local absolute function
{
  if (i < 0)
    return -i;
  return i;
}

long
manhattan(const int *p)
{
  long total = 0;
  for (int i = 0; i < 9; i++) {
    if (p[i] == 0)
      continue;
    total += l_abs(p[i] % 3 - i % 3) + l_abs(p[i] / 3 - i / 3);
  }
  return total;
}


//Guess what does this function do
void
print_puzzle(const int *p)
{
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (p[i * 3 + j] != 0)
	printf("%d ", p[i * 3 + j]);
      else
	printf("  ");
    }
    printf("\n");
  }
  //printf("----------------------\n");
}


//In the puzzle *p check, if it legal
//To perform a move (i.e not going into wall)
int
is_legal(int *p, int move)
{
  int pos = 0;
  while (p[pos] != 0)
    pos++;
  if (move == UP) {
    if (pos < 2)
      return 0;
    else
      return 1;
  }
  else if (move == DOWN) {
    if (pos > 5)
      return 0;
    else
      return 1;
  }
  else if (move == LEFT) {
    if (pos % 3 == 0)
      return 0;
    else
      return 1;    
  }
  else if (move == RIGHT) {
    if (pos % 3 == 2)
      return 0;
    else
      return 1;
  }
}


//Move puzzle into direction dir
void
move(int *p, int dir)
{
  switch(dir){
  case UP:
    for (int i = 0; i < 9; i++) {
      if (p[i] == 0 && i > 2) {
	p[i] = p[i - 3];
	p[i - 3] = 0;
	return;
      }
    }
    break;
  case DOWN:
    for (int i = 0; i < 9; i++) {
      if (p[i] == 0 && i < 6) {
	p[i] = p[i + 3];
	p[i + 3] = 0;
	return;
      }
    }
    break;  
  case LEFT:
    for (int i = 0; i < 9; i++) {
      if (p[i] == 0 && ((i % 3) != 0)) {
	p[i] = p[i - 1];
	p[i - 1] = 0;
	return;
      }
    }
    break;
  case RIGHT:
    for (int i = 0; i < 9; i++) {
      if (p[i] == 0 && i % 3 != 2) {
	p[i] = p[i + 1];
	p[i + 1] = 0;
	return;
      }
    }
    break;
  default:
    return;
  }      
}


//list of moves
//making it a deq was a mistake
struct
l_moves
{
  struct l_moves* prev;
  struct move_seq* seq;
  struct l_moves* next;
  int points;
};




//list of lists of moves
struct 
move_seq
{
  int move;
  struct move_seq* next;
  struct move_seq* prev;
};

void
print_move_seq(struct move_seq *m)
{
  struct move_seq *s = m;
  while(m) {
    char *dir;
    switch(m -> move) {
    case 0:
      dir = "LEFT";
      break;
    case 1:
      dir = "RIGHT";
      break;
    case 2:
      dir = "UP";
      break;
    case 3:
      dir = "DOWN";
      break;
    default:
      dir = "UNKNOWN";
      break;
    }
    printf("%s->", dir);
    m = m -> next;
  }
  printf("done\t(%p)\n", s);
}

void
print_l_moves(struct l_moves *m)
{
  int i = 0;
  while (m){    
    printf("%d: (points %d) ", i, m -> points);
    i++;    
    print_move_seq(m -> seq);
    m = m -> next;
  }
  printf("---------------------------------------");
  printf("\n");
}

void
print_frontier(struct l_moves *m, int depth)  //prints first "depth" entries of lmoves
{
  int i = 0;
  int l = 0;
  while (m) {
    if (l < depth) {
      printf("%d (%d points) ", i, m -> points);
      i++;
      print_move_seq(m -> seq);
    }
    l++;
    m = m -> next;
  }
  if (l > depth) {
    printf("... %d more\n", l - depth);
  }
}

//Same as before, but now we supply a puzzle and make it pretty
void
print_sol(const int *p, const struct move_seq *m)
{
  printf("Solution:\n");
  int arr[9];
  memcpy(arr, p, sizeof(int) * 9);
  print_puzzle(p);
  printf(" |\n v\n\n");
  while (m) {
    move(arr, m -> move);
    print_puzzle(arr);
    printf(" |\n v\n\n");
    m = m -> next;
  }
  printf("-------------\n");
}


//execute a list of moves on the puzzle
void
execute(int *p, const struct move_seq *mov)
{
  while (mov) {
    move(p, mov -> move);
    mov = mov -> next;
  }
}


//Utter trash, that doesnt work
//Dont bother yourself w/ making it normal
void
del_move_seq(struct move_seq* m)
{
  while (m -> next) {
    struct move_seq *temp = m;
    m = m -> next;
    free(temp);
  }
  free(m);
}


void
del_l_moves_el(struct l_moves *m)
{
  del_move_seq(m -> seq);
  free(m);
}






//Support function for BFS search
void  //advance move for BFS search
adv_mov(struct move_seq *m)
{
  while(m -> next) {
    m = m -> next;
  }
  (m -> move)++;
  while (m -> move > DOWN) { //process overflow
    if (m -> prev == NULL) { //if root
      while(m -> next) {
	m -> move = 0;
	m = m -> next;
      }
      m -> next = malloc(sizeof(struct move_seq));
      m -> next -> prev = m;
      m -> next -> next = NULL;
      m -> move = -1;
      break;
    }
    else {
      m -> move = 0;
      m = m -> prev;
      (m -> move)++;
    }
  }
}

//return structure of a search, supplies
//depth of solution, number of created nodes
//and the solution
struct
puzzle_info
{
  long depth;
  long nodes;
  struct move_seq *seq;
};

//create copy of list of moves
struct move_seq*
create_copy(const struct move_seq *m)
{
  if (m == NULL)
    return NULL;
  struct move_seq *ret = NULL, *temp, *prev;
  temp = ret;
  prev = NULL;
  while (m) { 
    temp = malloc(sizeof(struct move_seq));
    if (ret == NULL) {
      ret = temp;
    }
    temp -> prev = prev;
    if (prev)
      prev -> next = temp;
    temp -> move = m -> move;
    prev = temp;
    m = m -> next;
  }
  return ret;
}

//
struct l_moves*
expand_frontier(const struct move_seq *m, const int *p, struct b_tree *t)
{
  int *buf = calloc(9, sizeof(int));
  memcpy(buf, p, sizeof(int) * 9);
  execute(buf, m);
  struct l_moves *f = NULL, *s = NULL;
  for(int i = 0; i < 4; i++) {
    if (is_legal(buf, i) == 1) {
      int *temp = calloc(9, sizeof(int));
      memcpy(temp, p, sizeof(int) * 9);
      execute(temp, m);
      move(temp, i);
      long key = getkey(temp, sizeof(int) * 9);
      free(temp);
      if (is_set(t, key) == 0) {
	insert(t, key);
	struct l_moves *l = malloc(sizeof(struct l_moves));
	l -> points = -1;
	if (!s)
	  s = l;
	l -> prev = f;
	if (f) {
	  f -> next = l;
	}
	f = l;
	l -> next = NULL;
	if (m == NULL) {      //if first
	  l -> seq = malloc(sizeof(struct move_seq));
	  l -> seq -> next = l -> seq -> prev = NULL;
	  l -> seq -> move = i;
	}
	else {
	  l -> seq = create_copy(m);
	  struct move_seq *m_st = l -> seq;
	  while (l -> seq -> next) {
	    l -> seq = l -> seq -> next;
	  }
	  l -> seq -> next = malloc(sizeof(struct move_seq));
	  l -> seq -> next -> prev = l -> seq;
	  l -> seq -> next -> next = NULL;
	  l -> seq -> next -> move = i;
	  l -> seq = m_st;
	}
      }
    }
  }
  free(buf);
  return s;
}

int
depth(const struct move_seq *m)
{
  int k = 0;
  while (m) {
    k++;
    m = m -> next;
  }
  return k;
}

struct move_seq*
lowest(const int *p, struct l_moves* front,
       long (*eval)(const int*))
{
  struct move_seq *ret = front -> seq;
  int temp[9];
  long low = eval(temp);
  while (front) {
    memcpy(temp, p, sizeof(int) * 9);
    int score = front -> points;
    if (v_state) {
      printf("evaluating ");
      print_move_seq(front -> seq);
    }
    if (score < 0) {
      execute(temp, front -> seq);
      score = eval(temp) + depth(front -> seq);
      front -> points = score;
    }
    if (low >= score) {
      low = score;
      ret = front -> seq;
    }
    if (v_state)
      printf("res is %d, h(x) is %d\n", score, eval(temp));
    front = front -> next;
  }
  return ret;
}

struct l_moves* //adding mov_seq add with pts points into front
add_seq_to_front(struct l_moves *front, struct move_seq *add, int pts)
{  
  if (front -> seq  == NULL) {
    front -> next = front -> prev = NULL;
    front -> seq = add;
    front -> points = pts;
    return front;
  }
  while (front) {
    if (front -> points >= pts) {
      struct l_moves *new = malloc(sizeof(struct l_moves));
      new -> prev = front -> prev;
      if (front -> prev)
	front -> prev -> next = new;
      new -> next = front;
      front -> prev = new;
      new -> seq = add;
      new -> points = pts;
      while (front -> prev) {
	front = front -> prev;
      }
      return(front);
    }
    else if (front -> next == NULL) {
      front -> next = malloc(sizeof(struct l_moves));
      front -> next -> prev = front;
      front -> next -> next = NULL;
      front -> next -> seq = add;
      front -> next -> points = pts;
      while (front -> prev) {
	front = front -> prev;
      }
      return(front);
    }
    front = front -> next;    
  }
  return NULL;
}

struct l_moves*  
add_l_moves(struct l_moves *dest, struct l_moves *src)
{
  for (; src; src = src -> next) {
    dest = add_seq_to_front(dest, src -> seq, src -> points);
  }
  while (dest -> prev) {
    dest = dest -> prev;
  }
  return dest;
}

void
eval_moves(struct l_moves *src, const int *p, long (*eval)(const int *p))
{
  int *copy = malloc(sizeof(int) * 9);
  for (; src; src = src -> next) {
    memcpy(copy, p, sizeof(int) * 9);
    execute(copy, src -> seq);
    src -> points = eval(copy) + depth(src -> seq);
  }
}


struct puzzle_info
A_star_search(const int *p, long(*heur)(const int*))
{
  struct b_tree states;
  memset(&states, 0, sizeof(struct b_tree));
  long nodes = 0;
  long (*eval)(const int*) = heur;
  long tries = 1;
  int copy[9];
  memcpy(copy, p, sizeof(int) * 9);
  struct l_moves *frontier = malloc(sizeof(struct l_moves));;
  insert(&states, getkey(p, sizeof(int) * 9));
  struct l_moves* toadd = expand_frontier(NULL, p, &states);
  nodes += 4;
  eval_moves(toadd, p, eval);
  frontier = add_l_moves(frontier, toadd);
  if (v_state) {
    printf("Init frontier:\n");
    print_l_moves(frontier);
  }
  struct move_seq *todo = NULL;
  while (is_solved(copy) == 0) {
    memcpy(copy, p, sizeof(int) * 9);
    tries++;
    todo = frontier -> seq;
    long f_p = frontier -> points;
    frontier = frontier -> next;
    free(frontier -> prev);
    frontier -> prev = NULL;
    struct l_moves *t = expand_frontier(todo, p, &states), *temp;
    nodes += 4;
    eval_moves(t, p, eval);
    if (v_state > 1) {
      printf("Adding new stuff to frontier\n");      
    }
    if (v_state == 2) {
      printf("New addition to frontier:\n");
      print_l_moves(t);
    }
    frontier = add_l_moves(frontier, t);
    memcpy(copy, p, sizeof(int) * 9);
    execute(copy, todo);
    /* printf("curr sol is:\n"); */
    /* print_sol(p, todo); */
    if (v_state == 1) {
      printf("front at %d:\n", tries - 1);
      print_frontier(frontier, 7);
    }
    if (v_state == 2) {
      printf("front at %d:\n", tries - 1);
      print_l_moves(frontier);
    }
    if (v_state) {
      printf("Lowest (%d points)  is: \n", f_p);
      print_move_seq(todo);
      printf("\n");
    }
    //print_sol(start, todo);
    //sleep(1);
  }
  struct puzzle_info ret;
  ret.seq = todo;
  long j = depth(todo);
  ret . depth = j;
  ret . nodes = nodes;
  return ret;
}

struct puzzle_info
BFS_search(const int *copy, long (*heur)(const int*))
{
  int p[9];
  for (int i = 0; i < 9; i++) {
    p[i] = copy[i];
  }
  static struct puzzle_info ret = {0, 0};  
  int *start = calloc(9, sizeof(int));
  memcpy(start, p, sizeof(int) * 9);
  long k = 1;
  int level = 1;
  static struct move_seq *m = NULL;
  if (!m)
    m = malloc(sizeof(struct move_seq));
  m -> next = NULL;
  m -> move = -1;
  m -> prev = NULL;  
  //Implementing BFS
  while (!is_solved(p)) {
    adv_mov(m);
    memcpy(p, start, sizeof(int) * 9);
    execute(p, m);
    k++;
  }
  int j = 0;
  ret . seq = m;
  while(m -> next) {
    m = m -> next;
    j++;
  }
  ret . nodes = k;
  ret . depth = depth(ret.seq);
  return ret;
}

void
fill_puzzle(int *p, int shuffle)
{
  static int is_set = 0;
  if (is_set == 0) {
    srand(time(NULL));
    is_set = 1;
  }
  for (int i = 0; i < 9; i++)
    *(p + i) = i;
  for (int i = 0; i < shuffle; i++) {
    int a = rand() % 4;
    move(p, a);
  }
}



int
is_valid_puzzle(int *p)
{
  int *arr = calloc(9, sizeof(int));
  memcpy(arr, p, sizeof(int) * 9);
  for (int i = 0; i < 9; i++)
    arr[i] = i;
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      if (p[i] == arr[j]) {
	arr[j] = -1;	
	break;
      }
    }
  }
  for (int i = 0; i < 9; i++) {
    if (arr[i] != -1) {
      free(arr);
      return 0;
    }
  }
  free(arr);
  return 1;
}

int *
get_puzzle (FILE *fs, int* state)
{
  (*state) = 0;
  int *arr = calloc(9, sizeof(int));
  char c;
  if (fscanf(fs, "%c", &c) == 1) {
    if (c == '#') {
      while ((c = fgetc(fs)) != '\n');
      ungetc(c, fs);
      (*state) = 1;
      return (int*) 1;      
    }
    else
      ungetc(c, fs);
  }
  for (int i = 0; i < 9; i++) {
    if (fscanf(fs, "%d", arr + i) != 1)
	 return NULL;
  }  
  while (getc(fs) != '\n') ;
  return arr;
}

int**
parse_file(FILE *fs)
{
  int **puzzles = calloc(BUFSIZ, sizeof(int*));
  int *arg;
  int i;
  int state;
  for (i = 0; ((arg = get_puzzle(fs, &state)) != NULL); i++)  {
    if (state == 1) {
      i--;
      continue;
    }
    if (i >= BUFSIZ) {
      fprintf(stderr, "File is too big :( (there should be less than %d puzzles)\n", BUFSIZ);
      exit(1);
    }
    puzzles[i] = arg;
  }
  for (int j = 0; j < i; j++) {
    if (!is_valid_puzzle(puzzles[j])) {
      fprintf(stderr, "Invalid puzzle at line %d\n", j + 1);
      exit(1);
    }
  }
  return puzzles;
}

void
debug()
{
  long key = 112233;
  struct b_tree s;
  printf("\n");
}

int
main(int argc, char **argv)
{
  int shuffle = 10;
  int tries = 1;
  int p[9];
  int c = 0;
  int solution = 0;
  char *filename = NULL;
  long (*heur)(const int*) = manhattan;
  struct puzzle_info (*search)(const int*, long (*f)(const int*)) = A_star_search;
  while ((c = getopt(argc, argv, "l:e:s:t:hv:f:go")) != EOF) {
    switch(c) {
    case 'o':
      solution = 1;
      break;
    case 'l':
      if (strcmp(optarg, "nip") == 0)
	heur = not_in_place;
      else if (strcmp(optarg, "man") == 0)
	heur = manhattan;
      else {
	fprintf(stderr, "Unknown search heuristic\n");
	exit(1);
      }
      break;
    case 'g':
      debug();
      return 0;
      break;
    case 'e':
      if (strcmp("BFS", optarg) == 0)
	search=BFS_search;
      else if (strcmp("Astar", optarg) == 0)
	search = A_star_search;
      else {
	fprintf(stderr, "Unknown search\n");
	exit(1);
      }
      break;
    case 's':
      shuffle = atoi(optarg);
      break;
    case 't':
      tries = atoi(optarg);
      break;
    case 'v':
      v_state = atoi(optarg);
      break;
    case 'f':
      filename = optarg;
      break;
    case 'h':
      printf("Usage: %s [-s shuffle_times (def=10)] "
	      "[-t tries_num (def=1)] [-v visual_num (def=0)] [-e search_type (def Astar)] [-l heuristic function (default manhattan)]\n"
	     "[-o show solution] [-g debug]\n",
	      argv[0]);
      exit(0);      
    default:
      fprintf(stderr, "Usage: %s [-s shuffle_times (def=10)] "
	      "[-t tries_num (def=1)] [-v visual_num (def=0)]\n",
	      argv[0]);
      exit(1);
    }
  }

  if (filename) {
    FILE *fs = fopen(filename, "r");
    if (fs == NULL) {
      perror("filename");
      exit(1);
    }
    int **puzzles = parse_file(fs);
    for (int j = 0; puzzles [j]; j++) {
      if (v_state) {
	printf("\n\n");
	print_puzzle(puzzles[j]);
      }
      struct puzzle_info ret = search(puzzles[j], heur);
      if (v_state)
	print_sol(puzzles[j], ret . seq);
      printf("shuffle: %d\tdepth: %d\tnodes: %d\n", shuffle, ret . depth, ret.nodes);
      fflush(stdout);
    }
    return 0;
  }
  for (int i = 0; i < tries; i++) {
    fill_puzzle(p, shuffle);
    int s[9];
    memcpy(s, p, sizeof(int) * 9);
    if (v_state) {
      printf("\n\n");
      print_puzzle(s);
    }
    struct puzzle_info ret = search(p, heur);
    if (v_state)
      print_sol(s, ret . seq);
    printf("shuffle: %d\tdepth: %d\tnodes: %d", shuffle, ret . depth, ret.nodes);
    if (solution || v_state != 0) {
      printf(" s: ");
      for (int i = 0; i < 9; i++) {
	printf("%d ", s[i]);
      }
      printf("\n");
    }
    else{
      printf("\n");
    }
    fflush(stdout);
  }
  return 0;
}
