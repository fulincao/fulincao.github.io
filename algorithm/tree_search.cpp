#include<stdio.h>
#include<stack>
#include<stdlib.h>
#include<queue>
using namespace std;

struct node
{
    struct node* left = NULL;
    struct node* right = NULL; 
    int val;
};

node* root;

void mid_order() {
    stack<node*> st;
    node* now = root;

    while( !st.empty() || now ) {
        while( now ) {
            // printf("%d\n", now->val); // 前序
            st.push(now);
            now = now->left;
        }
        now = st.top();
        st.pop();
        printf("%d\n", now->val);  // 中序
        now = now->right;
    }
}


void at_order() {
    stack<node*> st;
    node *now = root, *prev = NULL;
    while( !st.empty() || now ) {
    
        while( now ) {
            st.push(now);
            now = now->left;
        }
        now = st.top();
        if( now->right == NULL || now->right == prev) {
            printf("%d\n", now->val);
            prev = now;
            st.pop();
        }else{
            now = now->right;
        }
    }
}

void le_order() {
    queue<node*> q;
    q.push(root);
    node* now = NULL;
    while( !q.empty()) {
        now = q.front();
        q.pop();
        printf("%d\n", now->val);
        if( now->left ) q.push(now->left);
        if( now->right) q.push(now->right);

    }
}

int main(int argc, char argv[]) {


    return 0;
}
