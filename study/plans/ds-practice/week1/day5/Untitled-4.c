#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct TreeNode {
    int val; // 树节点的值
    struct TreeNode *left; //左子树
    struct TreeNode *right;//右子树
} TreeNode;

// 生成新节点
TreeNode* new_node(int v) {
    TreeNode *n = malloc(sizeof(TreeNode));
    n->val = v; n->left = n->right = NULL;
    return n;
}

/* 从层序数组构建二叉树：用 -1 表示 null
   arr: 数值数组, n: 长度. 返回根指针
   构建适合测试的小树 */
TreeNode** node_array_create(int n) {
    TreeNode **arr = malloc(sizeof(TreeNode*) * n);
    for (int i = 0; i < n; ++i) arr[i] = NULL;
    return arr;
}

// 建立树结构，将数组vals[]转换为treenode
TreeNode* build_from_level_order(int *vals, int n) {
    if (n == 0) return NULL; // 如果无此点，返回NULL
    TreeNode **nodes = node_array_create(n);
    for (int i = 0; i < n; ++i) {
        if (vals[i] != -1) nodes[i] = new_node(vals[i]);
    }
    for (int i = 0; i < n; ++i) {
        if (!nodes[i]) continue;
        int l = 2*i + 1;
        int r = 2*i + 2;
        if (l < n) nodes[i]->left = nodes[l];
        if (r < n) nodes[i]->right = nodes[r];
    }
    TreeNode *root = nodes[0]; //根节点
    free(nodes);
    return root;
}

/* 递归遍历 */
// 先序遍历，中值->左子树->右子树
void preorder_rec(TreeNode* r) {
    if (!r) return;
    printf("%d ", r->val);
    preorder_rec(r->left);
    preorder_rec(r->right);
}
//中序遍历，左子树->中值->右子树
void inorder_rec(TreeNode* r) {
    if (!r) return;
    inorder_rec(r->left);
    printf("%d ", r->val);
    inorder_rec(r->right);
}
//后序遍历，右子树->中值->左子树
void postorder_rec(TreeNode* r) {
    if (!r) return;
    postorder_rec(r->left);
    postorder_rec(r->right);
    printf("%d ", r->val);
}

/* 简单栈（用于迭代遍历） */
//DFS
//如果用的是栈指针stack *s 访问成员要用->
//如果用的是栈变量stack s 访问成员要用 .
typedef struct {
    TreeNode **data;
    int top;
    int cap;
} Stack;
Stack* st_create(int cap) {
    Stack *s = malloc(sizeof(Stack));
    s->data = malloc(sizeof(TreeNode*) * cap);
    s->top = -1; s->cap = cap;
    return s;
}
void st_push(Stack *s, TreeNode *t) {
    if (s->top + 1 >= s->cap) {
        s->cap *= 2;
        s->data = realloc(s->data, sizeof(TreeNode*) * s->cap);
    }   //realloc 函数用于重新分配内存块的大小。它接受一个指向已分配内存的指针
        //和新的大小，并返回指向新内存块的指针，如果重新分配失败，则返回 NULL。
    s->data[++s->top] = t;
}
TreeNode* st_pop(Stack *s) {
    if (s->top < 0) return NULL;
    return s->data[s->top--];
}
TreeNode* st_peek(Stack *s) { return s->top < 0 ? NULL : s->data[s->top]; }
bool st_empty(Stack *s) { return s->top < 0; }
void st_free(Stack *s) { free(s->data); free(s); }

/* 迭代前序 */
void preorder_iter(TreeNode* root) {
    if (!root) return;
    Stack *s = st_create(32);
    st_push(s, root);
    while (!st_empty(s)) {
        TreeNode *t = st_pop(s);
        printf("%d ", t->val);
        if (t->right) st_push(s, t->right);
        if (t->left) st_push(s, t->left);
    }
    st_free(s);
}

/* 迭代中序 */
void inorder_iter(TreeNode* root) {
    Stack *s = st_create(32);
    TreeNode *cur = root;
    while (cur || !st_empty(s)) {
        while (cur) {
            st_push(s, cur);
            cur = cur->left;
        }
        cur = st_pop(s);
        printf("%d ", cur->val);
        cur = cur->right;
    }
    st_free(s);
}

/* 迭代后序（单栈法：记录上次访问） */
void postorder_iter(TreeNode* root) {
    Stack *s = st_create(32);
    TreeNode *cur = root;
    TreeNode *last = NULL;
    while (cur || !st_empty(s)) {
        while (cur) {
            st_push(s, cur);
            cur = cur->left;
        }
        TreeNode *peek = st_peek(s);
        if (peek->right && last != peek->right) {
            cur = peek->right;
        } else {
            printf("%d ", peek->val);
            last = st_pop(s);
        }
    }
    st_free(s);
}

/* 层序遍历（队列） */
//BFS
typedef struct {
    TreeNode **data;
    int head, tail, cap; //head为队首，tail为队尾，cap为容量
} Queue;
Queue* q_create(int cap) { //创建队列
    Queue *q = malloc(sizeof(Queue));
    q->data = malloc(sizeof(TreeNode*) * cap);
    q->head = q->tail = 0; q->cap = cap;
    return q;
}
void q_enqueue(Queue *q, TreeNode *t) { //将树节点入队
    q->data[q->tail++] = t;
    if (q->tail == q->cap) q->tail = 0; //队尾到达容量上限，循环回到队首
    /* assume enough capacity for simplicity in this test code */
}
TreeNode* q_dequeue(Queue *q) { //将队首元素出队
    TreeNode *t = q->data[q->head++];
    if (q->head == q->cap) q->head = 0;
    return t;
}
bool q_empty(Queue *q) { return q->head == q->tail; }  //判空条件
void q_free(Queue *q) { free(q->data); free(q); }

void level_order(TreeNode* root) {
    if (!root) return;
    Queue *q = q_create(128);
    q_enqueue(q, root);
    // 依次出队，打印值，并将左右子树入队，再依次打印左右子树的值，将其出队
    while (!q_empty(q)) {
        TreeNode *t = q_dequeue(q); // 这两步为依
        if (!t) continue;           // 次出队操作
        printf("%d ", t->val);
        if (t->left) q_enqueue(q, t->left);
        if (t->right) q_enqueue(q, t->right);
    }
    q_free(q); //；释放指针，防止内存泄漏
}

/* 小工具：释放整棵树（后序释放） */
void free_tree(TreeNode* r) {
    if (!r) return;
    free_tree(r->left);
    free_tree(r->right);
    free(r);
}

int main() {
    /* 层序数组表示 (使用 -1 作为 NULL)
         1
        / \
       2   3
      / \   \
     4  5    6
       数组（层序）: [1,2,3,4,5,-1,6]
    */
    int vals[] = {1,2,3,4,5,-1,6};
    int n = sizeof(vals)/sizeof(vals[0]);
    TreeNode *root = build_from_level_order(vals, n);

    printf("pre rec: "); preorder_rec(root); printf("\n");
    printf("in  rec: "); inorder_rec(root); printf("\n");
    printf("post rec: "); postorder_rec(root); printf("\n");
    //DFS迭代遍历
    printf("pre iter: "); preorder_iter(root); printf("\n");
    printf("in  iter: "); inorder_iter(root); printf("\n");
    printf("post iter: "); postorder_iter(root); printf("\n");

    //BFS层序遍历
    printf("level:    "); level_order(root); printf("\n");

    free_tree(root);
    return 0;
}
