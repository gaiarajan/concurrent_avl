#include "fine_grained_avl.cpp" 
#include <random>
#include <vector>
#include <algorithm>
#include <assert.h>
using namespace std;

int s = 107;

void print_tree(Node_L* node, int level = 0) {
    if (node == NULL)
        return;
    // Print current node and its parent
    printf("Node: %d, Left height: %d, Right height: %d, Parent: %d\n", node->key, node->left_height, node->right_height, (node->parent != NULL) ? node->parent->key : -1);

    // Print left child
    if (node->left != NULL)
        printf("  Left Child: %d\n", node->left->key);

    // Print right child
    if (node->right != NULL)
        printf("  Right Child: %d\n", node->right->key);

    // Recursively print left and right subtrees
    print_tree(node->left);
    print_tree(node->right);
}

void test_insert_sequence()
{
	vector<int> numbers;
    for (int i = 1; i <= s; ++i) {
        numbers.push_back(i);
    }
	random_shuffle(numbers.begin(), numbers.end());
	for (int i = 0; i < s; i++) {
		insert(numbers[i]);
		//print_tree(root);
	}
	for (int i = 1; i <= s; ++i) {
        assert(contains(i));
    	//printf("contained %d \n", i);
	}
	//printf("Sequential insert passed! \n");
}

void test_delete_sequence()
{
	vector<int> numbers;
    for (int i = 1; i <= s; ++i) {
        numbers.push_back(i);
    }
    random_shuffle(numbers.begin(), numbers.end());
    for (int i = 0; i < s; i++) {
		remove(numbers[i]);
		//print_tree(root);
	}
    for (int i = 1; i <= s; ++i) {
		//printf("now asserting delete for  %d \n", i);
		assert(!contains(i));
		//printf("passed for %d \n", i);
    }
	//printf("Sequential delete passed! \n");
}

bool test_height(Node_L *node)
{
	//pthread_mutex_lock(&root->tree_lock);
	if (node == NULL || node == root) return true;
	int correct_left = node->left ? max(node->left->left_height, node->left->right_height) + 1 : 0;
	int correct_right = node->right ? max(node->right->left_height, node->right->right_height) + 1 : 0;
	//printf("correct left height: %d for key %d \n", correct_left, node->key);
	bool left_correct = (!node->left) || (node->left_height == correct_left);
	bool right_correct = (!node->right) || (node->right_height = correct_right);
	bool left_subtree_correct = test_height(node->left);
    bool right_subtree_correct = test_height(node->right);
	assert(left_correct); 
	assert(right_correct);
	assert(left_subtree_correct && right_subtree_correct);
	//pthread_mutex_unlock(&root->tree_lock);
	return true;
}

bool test_parent_child(Node_L *node)
{
	if (node == NULL) return true;
	bool correct_left = node->left? node->left->parent == node : true;
    bool correct_right = node->right? node->right->parent == node : true;	
	bool correct_left_subtree = test_parent_child(node->left);
	bool correct_right_subtree = test_parent_child(node->right);
	assert(correct_left&&correct_right&&correct_left_subtree&&correct_right_subtree);
	return true;
}

int test_basic_balance(Node_L *node)
{
	if (node == NULL) return 0;
	int bfactor = std::abs(get_bfactor(node)) + test_basic_balance(node->right) + test_basic_balance(node->left);
	return bfactor;
}

int main()
{
	init_tree();
	
	test_insert_sequence();
	printf("Sequential insert passed! \n");
	test_height(root);
	printf("Height check passed!\n");
	test_parent_child(root);
	printf("Relation check passed! \n");
	float avg_bfactor = test_basic_balance(root);
	printf("Average bfactor = %f \n", avg_bfactor/s);
	test_delete_sequence();
	printf("Sequential delete passed! \n");
	test_insert_sequence();
    printf("Sequential insert after delete passed! \n");
	test_height(root);
    printf("Height check passed!\n");
    test_parent_child(root);
    printf("Relation check passed! \n");
    avg_bfactor = test_basic_balance(root);
    printf("Average bfactor = %f \n", avg_bfactor/s);
	test_delete_sequence();
    printf("Sequential delete 2 passed! \n");
	/*
	insert(7);
	print_tree(root);
	insert(1);
	print_tree(root);
    insert(4);
    print_tree(root);
	insert(6);
	print_tree(root);
	insert(8);
	print_tree(root);
	insert(9);
	print_tree(root);
	insert(5);
	print_tree(root);
		insert(5);
	print_tree(root);
	insert(4);
	print_tree(root);
	insert(1);
	print_tree(root);
	insert(3);
	print_tree(root);
	insert(2);
print_tree(root);
*/	/*insert(7);
	print_tree(root);
    insert(11);
    print_tree(root);
	insert(12);
	print_tree(root);
    insert(15);
    print_tree(root);
	Node_L *temp = search(7);
	rotate_left(temp);
	print_tree(root);
	*/
	//test_basic_balance();
	//print_tree(root);
}
