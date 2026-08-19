#include <stdio.h>
#include <stdlib.h>

typedef struct ListNode {
    int val;
    struct ListNode *next;
} ListNode;

/* 删除倒数第 k 个节点（返回新头） */
ListNode* removeNthFromEnd(ListNode* head, int k) {
    ListNode dummy = {0, head};
    ListNode *first = &dummy, *second = &dummy;
    // 先移动 first k+1 步，使 second 指向待删除节点的前驱
    for (int i = 0; i < k + 1; ++i) {
        if (first == NULL) return head; // k 可能不合法
        first = first->next;
    }
    while (first) { //这个循环开始，快慢指针一起开始工作，直到快指针first到达链表末尾NULL，慢指针second就会到达倒数第k个节点的前一个节点
        first = first->next;
        second = second->next;
    }
    // second->next 即为待删除节点
    ListNode *to_delete = second->next;
    if (to_delete) {
        second->next = to_delete->next;
        free(to_delete); // 释放内存，将所需删除的节点赋到to_delete指针上，然后释放内存
    }
    return dummy.next; //返回释放后的链表头节点，dummy为无意义的哨兵节点，因此返回dummy.next即可
}

/* 检测环并返回环入口（无环返回 NULL） */
ListNode* detectCycle(ListNode* head) {
    ListNode *slow = head, *fast = head;
    while (fast && fast->next) {
        slow = slow->next;       //单步
        fast = fast->next->next; //双步
        if (slow == fast) { // 相遇，说明有环
            ListNode *p = head;// 从头开始，慢指针从相遇点开始，两个指针每次走一步，相遇点即为环入口
            while (p != slow) {
                p = p->next;
                slow = slow->next;
            }
            return p; // 环入口
        }
    }
