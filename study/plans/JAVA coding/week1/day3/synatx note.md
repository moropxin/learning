# 方法重载与参数传递

## 一、方法重载

### 定义 同名称同定义类型的方法，但形参不同

### 构成条件
    public static int sum(int a,int b){ return a+b;}
    public static double sum(double a,double b){ return a+b;}


## 二、Java 参数传递

### 基本类型 
    >方法只拿到a的副本，并不会修改a的值，会在栈内存中新建一个空间存放a的副本
    int a = 10;
    change(a); //change方法改变a=100
    System.out.println(a); // 10

    
### 数组和对象
    >方法拿到数组nums的地址，可通过内存地址直接修改数组值
     int[] nums = {1, 2, 3};
     change(nums); //change方法拿到数组nums的地址，修改nums[0]为100
     System.out.println(nums[0]); // 100

