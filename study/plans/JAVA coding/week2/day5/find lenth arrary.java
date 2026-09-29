package Day4;
/*题目：和为 S 的最长连续子数组（正整数版）

描述：给定一个只包含正整数的数组和一个正整数 S，求和恰好等于 S 的最长连续子数组的长度。如果不存在，输出 0。
输入格式（示例风格）：
第一行：两个正整数 n S  （1 ≤ n ≤ 100000，S ≤ 10^18）
第二行：n 个正整数，数组元素 ≤ 10^9
输出：一个整数，表示满足和为 S 的最长连续子数组长度（不存在则输出 0）。
样例 输入: 5 9
    第二行：1 2 3 4 5
    输出: 3 （解释：子数组 [2,3,4] 的和为 9，长度 3，是最长的）*/
import java.util.Scanner;

public class demon2  {
    public static void main(String[] args) {
        System.out.println("请输入n和S");
        Scanner s = new Scanner(System.in);

        long nums = s.nextInt(); //题目要求s为大数据
        int target = s.nextInt();
        int[] arr = new int[(int)nums];

        s.nextLine();
        System.out.println("请输入数组");

        for (int i = 0; i < nums; i++) {
            arr[i] = s.nextInt();
        }

        int left = 0;
        long sum = 0;
        int maxlen = 0;

        for (int right = 0; right < nums; right++) {
            sum += arr[right];

            //更新数组状态
            while (sum > target && left <= right) {
                sum -= arr[left];
                left++;
            }

            if (sum == target) {
                maxlen = Math.max(maxlen, right - left + 1);
            }
        }
        System.out.println(maxlen);
    }
}
