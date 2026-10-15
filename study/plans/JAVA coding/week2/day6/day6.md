# Week2 Day1 学习记录 — 内部类收尾与图形面积计算实践

日期：2026-10-15

目标：
- 巩固 Java 内部类（成员内部类、静态内部类、局部内部类、匿名内部类）的概念与用法。
- 用“图形面积计算器”实现 3–4 种内部类变体并写简单测试与复盘。


一、核心概念速览（必须掌握）
- 静态内部类（static nested class）：不依赖外部实例，适合逻辑归属但无需访问外实列成员的场景；不会持有外部 this，内存安全性更好。
- 成员内部类（non-static inner class）：隐式持有外部实例引用（Outer.this），可以访问外部私有成员；实例化需通过外部实例：outer.new Inner(...)
- 局部内部类（local inner class）：在方法/块中定义，作用域仅限方法体；可访问方法局部变量，但要求这些局部变量为 effectively final。
- 匿名内部类（anonymous inner class）：无名、声明即实例化，常用于一次性回调或实现接口；当目标是函数式接口时，优先用 lambda。
- this 与 Outer.this：this 指向当前类实例；在内部类中引用外部实例要写 OuterClassName.this。
- 内存泄漏风险：非静态内部类会持有外部引用，若把内部类实例放到静态容器中会阻止外部实例回收。

二、练习要点与实现思路
- 先写接口：

```java
public interface Shape {
    double area();
    String name();
}
```

- 基础实现（独立类）：Circle/Rectangle/Triangle，负责字段校验与 area 实现。
- 静态内部类：例如在 ShapeManager 中定义 public static class RectangleImpl implements Shape，不依赖外部实例。
- 成员内部类：在 ShapeManager 中定义 public class CircleImpl implements Shape，使用外部类状态（例如默认单位或 id）。
- 局部内部类：在 parseShape(String) 方法中定义局部类 RightTriangle 并返回其实例，观察 effectively final 行为。
- 匿名内部类：快速构造一次性 Shape，或作为回调参数传入方法。

三、示例代码片段（演示四种内部类）

```java name=ShapeAreaCalculator.java
public class ShapeAreaCalculator {
    public interface Shape { //定义接口
        double area(); //计算面积公式
        String name(); //内部类名称
    }

    // 静态内部类（不依赖外部实例）
    // 长乘宽面积实现
    public static class Rectangle implements Shape {
        private final double w, h; //私有化最终变量
        public Rectangle(double w, double h) {
            this.w = w;
            this.h = h; }
        @Override public double area() {
            return w * h;}
        @Override public String name() {
            return "Rectangle"; }
    }

    // 成员内部类（依赖外部实例）
    // 圆的面积实现
    public class Circle implements Shape {
        private final double r; //私有化最终变量
        public Circle(double r) {
            this.r = r; }
        @Override public double area() {
            return Math.PI * r * r; }
        @Override public String name() {
            return "Circle"; }
    }

    // 在方法内定义局部内部类
    // 三角形面积实现
    public Shape makeRightTriangle(double base, double height) { //这里的Shape是方法里的返回值，可直接让局部内部类返回Shape类型的值，若加上static修饰符则无法返回局部内部类
        class RightTriangle implements Shape { //局部内部类(作用仅限于该方法)
            @Override public double area() {
                return base * height / 2.0; }
            @Override public String name() {
                return "RightTriangle"; }
        }
        return new RightTriangle();
    }

    // 汇总示例
    public static void main(String[] args) {
        ShapeAreaCalculator mgr = new ShapeAreaCalculator();

        Shape r = new Rectangle(3, 4);                // 静态内部类直接 new
        Shape c = mgr.new Circle(2.0);                   // 成员内部类需要外部实例
        Shape t = mgr.makeRightTriangle(3, 4); // 局部内部类

        // 匿名内部类（一次性实现）
        Shape anon = new Shape() {
            @Override public double area() {
                return 42.0; }
            @Override public String name() {
                return "Anonymous"; }
        };

        Shape[] shapes = { r, c, t, anon };
        double total = 0;
        for (Shape s : shapes) {
            System.out.printf("%s -> area=%.3f%n", s.name(), s.area());
            total += s.area();
        }
        System.out.printf("Total = %.3f%n", total);
    }
}
```

四、今日复盘
- 区分静态内部类与成员内部类的最大不同：是否依赖外部实例，以及由此产生的内存/访问差异。
- 匿名内部类适合一次性实现；当接口为函数式接口时优先使用 lambda。
- 局部内部类能访问方法局部变量但受 effectively final 限制。

---
