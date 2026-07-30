#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool isMatching(char open, char close) {
    return (open == '(' && close == ')') ||
           (open == '[' && close == ']') ||
           (open == '{' && close == '}');
}

bool isValid(const char *s) {
    int n = strlen(s);
    char *stack = (char*)malloc(n);
    int top = -1;
    for (int i = 0; s[i]; ++i) {
        char c = s[i];
        if (c == '(' || c == '[' || c == '{') {
            stack[++top] = c;
        } else if (c == ')' || c == ']' || c == '}') {
            if (top < 0) { free(stack); return false; }
            if (!isMatching(stack[top--], c)) { free(stack); return false; }
        }
    }
    bool res = (top == -1);
    free(stack);
    return res;
}

int main() {
    const char *tests[] = {"()", "()[]{}", "(]", "{[]}", "", "([{}])", "(((()))", "([)]"};
    int t = sizeof(tests)/sizeof(tests[0]);
    for (int i=0;i<t;i++) {
        printf("%s -> %s\n", tests[i], isValid(tests[i]) ? "true":"false");
    }
    return 0;
}
