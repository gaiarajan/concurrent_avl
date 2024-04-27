#include <iostream>
#include <pthread.h>
#include <limits>

using namespace std;

class Node_L {
	public:
		int key;
		Node_L *left;
		Node_L *right;
		Node_L *parent;

		bool removed;
		int left_height, right_height;
		Node_L *succ, *pred;
		pthread_mutex_t tree_lock, succ_lock;
};

Node_L* root;

void tree_lock(Node_L *N)
{
	pthread_mutex_lock(&N->tree_lock);
	//printf("tree locked node %d \n", N->key);
}

void tree_unlock(Node_L *N)
{
    //printf("tree unlocked node %d \n", N->key);
	pthread_mutex_unlock(&N->tree_lock);
}

void succ_lock(Node_L *N)
{
    pthread_mutex_lock(&N->succ_lock);
    //printf("succ locked node %d \n", N->key);
}

void succ_unlock(Node_L *N)
{
    //printf("succ unlocked node %d \n", N->key);
    pthread_mutex_unlock(&N->succ_lock);
}

Node_L* search(int k)
{
	if (root == NULL) return NULL;
	Node_L *curr = root;
	Node_L *curr_child = root;
	int curr_key = root->key;
	while (true)
	{
		curr_key = curr->key;
		if (curr_key == k) return curr;
		curr_child = curr_key < k ? curr->right : curr->left;
		if (curr_child == NULL) return curr;
		curr = curr_child;
	}
	return NULL; //absurd
}

bool contains(int k)
{
	Node_L *res = search(k);
	while (res->key > k)
	{
		res = res->pred;
	}
	while (res->key < k)
    {
        res = res->succ;
    }
	bool ans = (res->key == k && !res->removed);
	return ans;
}

int get_max (int x, int y) {
	return (x > y) ? x : y;
}

int get_bfactor(Node_L *N) {
	if (N == NULL) return 0;
	return N->left_height - N->right_height;
}

bool fix_height(Node_L *N) {
	tree_lock(N);
	int old_left = N->left_height;
	int old_right = N->right_height;
	if (N->left)
		N->left_height = get_max(N->left->left_height, N->left->right_height) + 1;
	else N->left_height = 0;
	if (N->right)
		N->right_height = get_max(N->right->left_height, N->right->right_height) + 1;
	else N->right_height = 0;
	bool ans = (old_left != N->left_height) || (old_right != N->right_height);
	tree_unlock(N);
	return ans;
	//N->height = get_max(N->left_height, N->right_height) + 1;
}

void fix_height_no_lock(Node_L *N) {
    if (N->left)
        N->left_height = get_max(N->left->left_height, N->left->right_height) + 1;
    else N->left_height = 0;
    if (N->right)
        N->right_height = get_max(N->right->left_height, N->right->right_height) + 1;
    else N->right_height = 0;
}

void replace(Node_L *P, Node_L *O, Node_L *C)
{
	//if (C) printf("Node %d's original child Node %d is now replaced by Node %d \n", P->key, O->key, C->key);
    if (P->left == O) P->left = C;
    else P->right = C;
    if (C) C->parent = P;
}

Node_L* create_node(int k) {
	Node_L *N = new Node_L();
	
	N->key = k;
	N->left = NULL;
	N->right = NULL;
	N->parent = NULL;
	N->removed = false;
	pthread_mutex_init(&N->succ_lock, NULL);
	pthread_mutex_init(&N->tree_lock, NULL);
	N->left_height = 0;
	N->right_height = 0;

	return N;
}

void init_tree()
{
	root = create_node(numeric_limits<int>::max());
	root->pred = create_node(numeric_limits<int>::min());
	(root->pred)->succ = root;
}

Node_L* rotate_right(Node_L *N) {
	if(!N || !N->left) return NULL;

	tree_lock(N);
	tree_lock(N->parent);
	if (N->left) tree_lock(N->left);
	if (N->left->right) tree_lock(N->left->right);	

	Node_L *L = N->left;
	N->left = L->right;
	if (L->right) L->right->parent = N;
	L->right = N;

	Node_L *old_parent = N->parent;
	replace(N->parent, N, L);
    N->parent = L;
	N->left_height = L->right_height;
	L->right_height = get_max(N->left_height, N->right_height) + 1;

		
	if (L) tree_unlock(L);
    if (N->left) tree_unlock(N->left);
	tree_unlock(old_parent);
	tree_unlock(N);

	return L;
}

Node_L* rotate_left(Node_L *N) {
	if(!N || !N->right) return NULL;
	
	tree_lock(N);
	tree_lock(N->parent);
	if (N->right) tree_lock(N->right);
	if (N->right->left) tree_lock(N->right->left);

	Node_L *R = N->right;
	N->right = R->left;
	if (R->left) R->left->parent = N;
	R->left = N;

	N->right_height = R->left_height;
    R->left_height = get_max(N->left_height, N->right_height) + 1;

	Node_L *old_parent = N->parent;
	replace(N->parent, N, R);	
	N->parent = R;

	if (R) tree_unlock(R);
	if (N->right) tree_unlock(N->right);
	tree_unlock(old_parent);
	tree_unlock(N);

	return R;
}


void balance(Node_L *N) {
	if (N == NULL) return;
	
	if(get_bfactor(N) >= 2)
	{
		//printf("right rotation on key %d \n", N->key);
		
		if(get_bfactor(N->left) < 0)
			N->left = rotate_left(N->left);
		fix_height(N);		
		N = rotate_right(N);
	}

	if(get_bfactor(N) <= -2)
	{
		//printf("left rotation on key %d \n", N->key);
		
		if(get_bfactor(N->right) > 0)
			N->right = rotate_right(N->right);
		fix_height(N);
		N = rotate_left(N);
	}
}

Node_L* find_parent(Node_L *P, Node_L *S, Node_L *K)
{
	Node_L* curr = (K == S) ? K : P;
	while (true)
	{
		tree_lock(curr);
		if (curr == P)
		{
            if (curr->right == NULL) return curr;
            tree_unlock(curr);
			curr = S;
		}
		else
		{
            if (curr->left == NULL) return curr;
            tree_unlock(curr);
			curr = P;
		}
	}
	return NULL; //absurd
}


bool insert(int k)
{
	//printf("inserting node with key %d \n", k);
	while (true) 
	{
		Node_L *N = search(k);
		Node_L *L = (N->key > k && N->pred) ? N->pred : N;
		succ_lock(L);
		Node_L *R = L->succ; //L and R are the lower and upper bounds for k
		
		if (L->key < k <= R->key && !L->removed)
		{
			if (R->key == k)
			{
				succ_unlock(L);
				return false;
			}
			Node_L *K = create_node(k);
			
			Node_L *P = find_parent(L, R, N);
			tree_unlock(P); //just in case it returned before unlock-- don't want to unlock before in case of change
			
			K->succ = R;
			K->pred = L;
			R->pred = K;
			L->succ = K;
			
			succ_unlock(L);	
			
			//begin reparenting
			K->parent = P;
			if (k < P->key)
			{
				P->left = K;
				P->left_height = 1;
			}
			else if (k > P->key)
			{
				P->right = K;
				P->right_height = 1;
			}
		
			//end reparenting
			balance(K);
			while (P != root) 
			{	
				Node_L *PP = P->parent;
        		if (P->parent == PP && !PP->removed) 
				{
					fix_height(P); 
					fix_height(PP);
					balance(P);
					
					P = PP;
					tree_unlock(PP);
				}
			}
			//balance(K);	
			//fix_height(root);
			return true;
		}
		succ_unlock(L);
	}

	return false; //absurd
}

void clone(Node_L *original, Node_L *copy)
{
	copy->key = original->key;
	copy->left = original->left;
	copy->right = original->right;
	copy->left_height = original->left_height;
	copy->right_height = original->right_height;
}

bool remove_tree(Node_L *N)
{
	Node_L *parent;
	Node_L *original_parent = N->parent;
	tree_lock(N);
	tree_lock(N->parent);
	
	if ((!N->left || N->left->removed) || (!N->right || N->right->removed)) 
	{
		Node_L *original_left = N->left;
		Node_L *original_right = N->right;
		if (original_left) tree_lock(original_left);
    	if (original_right) tree_lock(original_right);

		parent = N->parent;
		if (N->left) N->left->parent = parent;
		if (N->right) N->right->parent = parent;
		Node_L* child = N->left ? N->left : N->right;
		replace(parent, N, child);
		
		if (original_left) tree_unlock(original_left);
        if (original_right) tree_unlock(original_right);
	}
	else
	{
		Node_L *succ = N->succ;
		Node_L *temp = NULL;
		if (succ->parent != N) tree_lock(succ->parent);
		tree_lock(succ);
		if (succ->right) 
		{
			tree_lock(succ->right);
			temp = succ->right;
		}

		Node_L *child = succ->right;
		parent = succ->parent;
		replace(parent, succ, child);
		succ->left = N->left;
		succ->right = N->right;
		succ->left_height = N->left_height;
		succ->right_height= N->right_height;
		N->left->parent = succ;
		if (N->right) N->right->parent = succ;
		replace(N->parent, N, succ);
		
		if (parent == N) parent = succ; 
		else tree_unlock(succ); 
		tree_unlock(N->parent);
		tree_unlock(parent);
		if (temp) tree_unlock(temp);
	}
	tree_unlock(N);
	tree_unlock(original_parent);
	balance(parent);
	return true;
}

bool remove (int k)
{
	//printf("removing key %d \n", k);
	
	Node_L *N = search(k);
	while (N->key > k) N = N->pred;
	while (N->key < k) N = N->succ;
	if (N->key != k) return false;
	
	Node_L *P = N->pred;
	succ_lock(P);
	Node_L *S = P->succ;
	succ_lock(S);

	S->removed = true;
	Node_L *S_succ = S->succ;
	S_succ->pred = P;
	P->succ = S_succ;
	if (!remove_tree(S)) return false;
	succ_unlock(S);
	succ_unlock(P);

	return true;
}

