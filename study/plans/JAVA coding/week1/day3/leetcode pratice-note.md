## LEETCODE 27 移除元素
### 题目思路
  运用快慢指针实现，快指针用于索引元素，慢指针用于存放处理后数组
### 代码
  class Solution {
    public int removeElement(int[] nums, int val) {
        int slow=0;
        int fast=0;
        for(fast=0;fast<nums.length;fast++){
            if(nums[fast]!=val){
                nums[slow]=nums[fast];
                slow++;
            }
        }
        return slow;
    }
}
### 时间复杂度
    O(1)
### 空间复杂度
    O(N)
### 易错点
    快慢指针更新逻辑错误


## LEETCODE 283 移动零

### 题目思路
    双指针索引更新数组
### 代码
  class Solution {
    public void moveZeroes(int[] nums) {
        int slow=0;
        for(int fast=0;fast<nums.length;fast++){
            if(nums[fast]!=0){
                nums[slow]=nums[fast];
                slow++;
            }
        }
        while(slow<nums.length){
            nums[slow]=0;
            slow++;
        }
        
    }
}
### 时间复杂度
  O(N)
### 空间复杂度
  O(1)


## LEETCODE 88 合并两个有序数组

### 题目思路
    方法一：双指针索引更新数组
    方法二：merge-sort算法运算
### 代码
  方法一
class Solution {
    public void merge(int[] nums1, int m, int[] nums2, int n) {
        int p1 = m - 1; // nums1有效末尾
        int p2 = n - 1; // nums2末尾
        int cur = m + n - 1; // nums1总末尾
        while(p1 >=0 && p2 >=0){
            if(nums1[p1] > nums2[p2]){
                nums1[cur--] = nums1[p1--];
            }else{
                nums1[cur--] = nums2[p2--];
            }
        }
        // 剩下nums2元素继续填
        while(p2 >=0){
            nums1[cur--] = nums2[p2--];
        }
    }
}


  方法二
class Solution {
    public void merge(int[] nums1, int m, int[] nums2, int n) {
        int j = 0;
        // 修正循环条件
        for(int i = m; i < m + n; i++){
            nums1[i] = nums2[j];
            j++;
        }
        mergesort(nums1, 0, m + n);
    }

    public static void mergesort(int[] nums,int start,int end){
        if(end - start <= 1) return; // 递归终止条件！缺少这个会栈溢出
        int mid = start+(end-start)/2;
        mergesort(nums,start,mid);
        mergesort(nums,mid,end);
        // 归并合并
        mergeArray(nums, start, mid, end);
    }

    private static void mergeArray(int[] nums, int start, int mid, int end){
        int[] temp = new int[end - start];
        int i = start, k = mid, idx = 0;
        while(i < mid && k < end){
            if(nums[i] <= nums[k]) temp[idx++] = nums[i++];
            else temp[idx++] = nums[k++];
        }
        while(i < mid) temp[idx++] = nums[i++];
        while(k < end) temp[idx++] = nums[k++];
        // 拷贝回原数组
        System.arraycopy(temp, 0, nums, start, temp.length);
    }
}

### 时间复杂度
  O(N)
### 空间复杂度
  O(m+n)

