package Day4;
import java.io.BufferedReader; //适合大量数据录入，效率比scanner高
import java.io.IOException;    //用于在main方法后面加上throws IOException（我也不知道干什么的）
import java.io.InputStreamReader;//创建控制台对象
import java.util.Arrays;
import java.util.Scanner;
    /*小蓝有若干张数字卡片，每种数字 0～9 都有 N 张。

    他想按照以下顺序拼出正整数：
    1, 2, 3, 4, 5, ...
    拼出一个数字时，需要消耗这个数字中对应的所有数字卡片。
    例如，拼出数字 12 需要消耗：
        1 张数字 1
        1 张数字 2
    当无法继续拼出下一个数字时，停止。
    请问小蓝最多能拼出多少个连续的正整数？*/

public class demon1 {
    public static void main(String[] args) throws IOException {
        Scanner sc = new Scanner(new BufferedReader(new InputStreamReader(System.in)));
        int n = sc.nextInt();
        int[] arr = new int[10];
        for (int i = 0; i < 10; i++) arr[i] = n;
        int x=1;
        int[] temp = arr.clone(); //克隆arr数组给temp

        while (true) {
            String s = String.valueOf(x);
            boolean flag = true;
            for (int i = 0; i < s.length(); i++) {
                int digit = s.charAt(i) - '0';
                if(temp[digit]==0){
                    flag = false;
                    break;
                }
                temp[digit]--;
            }
            if(!flag){
                break;
            }
            x++;
            arr = temp;
        }
        System.out.println(x-1);
    }
}
