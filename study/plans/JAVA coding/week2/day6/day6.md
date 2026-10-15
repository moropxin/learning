# Week2 Day1 学习记录 — 内部类收尾与图形面积计算实践

日期：2026-10-15

目标：
- 巩固 Java 内部类（成员内部类、静态内部类、局部内部类、匿名内部类）的概念与用法。
- 用“图形面积计算器”实现 3–4 种内部类变体并写简单测试与复盘。

一、今日时间安排（约 4 小时）
- 00:00–00:30 理论回顾：内部类四类的语义、实例化方式与访问规则。
- 00:30–01:30 基础实现：用独立类实现 Shape 接口（Circle/Rectangle/Triangle）。
- 01:30–02:30 成员内部类 & 静态内部类：把部分实现移到外部类内部，比较差异。
- 02:30–03:15 局部内部类与匿名内部类：在方法中定义类与一次性实现。
- 03:15–03:45 算法练习 30 分钟（数组/字符串基础题）。
- 03:45–04:00 单元测试 + 提交 + 复盘（写 README 简短记录收获与坑）。

二、核心概念速览（必须掌握）
- 静态内部类（static nested class）：不依赖外部实例，适合逻辑归属但无需访问外实列成员的场景；不会持有外部 this，内存安全性更好。
- 成员内部类（non-static inner class）：隐式持有外部实例引用（Outer.this），可以访问外部私有成员；实例化需通过外部实例：outer.new Inner(...)
- 局部内部类（local inner class）：在方法/块中定义，作用域仅限方法体；可访问方法局部变量，但要求这些局部变量为 effectively final。
- 匿名内部类（anonymous inner class）：无名、声明即实例化，常用于一次性回调或实现接口；当目标是函数式接口时，优先用 lambda。
- this 与 Outer.this：this 指向当前类实例；在内部类中引用外部实例要写 OuterClassName.this。
- 内存泄漏风险：非静态内部类会持有外部引用，若把内部类实例放到静态容器中会阻止外部实例回收。

三、练习要点与实现思路
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

四、示例代码片段（演示四种内部类）

```java name=ShapeAreaCalculator.java
package study.plans.week2;

public class ShapeAreaCalculator {
    public interface Shape {
        double area();
        String name();
    }

    // 静态内部类
    public static class Rectangle implements Shape {
        private final double w, h;
        public Rectangle(double w, double h) { this.w = w; this.h = h; }
        @Override public double area() { return w * h; }
        @Override public String name() { return "Rectangle"; }
    }

    // 成员内部类
    public class Circle implements Shape {
        private final double r;
        public Circle(double r) { this.r = r; }
        @Override public double area() { return Math.PI * r * r; }
        @Override public String name() { return "Circle"; }
    }

    // 局部内部类示例
    public Shape makeRightTriangle(double base, double height) {
        class RightTriangle implements Shape {
            @Override public double area() { return base * height / 2.0; }
            @Override public String name() { return "RightTriangle"; }
        }
        return new RightTriangle();
    }

    public static void main(String[] args) {
        ShapeAreaCalculator mgr = new ShapeAreaCalculator();

        Shape r = new Rectangle(3, 4);
        Shape c = mgr.new Circle(2.0);
        Shape t = mgr.makeRightTriangle(3, 4);

        Shape anon = new Shape() {
            @Override public double area() { return 42.0; }
            @Override public String name() { return "Anonymous"; }
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

五、调试与验证建议
- 编译后观察生成的类文件（例如 ShapeAreaCalculator$Circle.class、$1.class），理解命名与匿名类编号。
- 在成员内部类中打印 this 与 ShapeAreaCalculator.this 的 identityHashCode，确认引用关系。
- 在局部内部类中尝试修改被捕获的局部变量以触发编译错误，理解 effectively final 的限制。
- 验证内存泄漏：把非静态内部类实例放到静态集合中，观察外部实例无法回收（选做，需配合内存分析）。

六、当日算法（30 分钟建议）
- 题目选择：Two Sum（数组），Valid Anagram（字符串）。目标：熟悉数组/字符串遍历与哈希表应用。

七、今日复盘要写入的三条要点（提交 README 时摘录）
- 区分静态内部类与成员内部类的最大不同：是否依赖外部实例，以及由此产生的内存/访问差异。
- 匿名内部类适合一次性实现；当接口为函数式接口时优先使用 lambda。
- 局部内部类能访问方法局部变量但受 effectively final 限制，理解这一点有助于避免常见编译错误。

八、提交建议
- 文件路径：study/plans/java/week2/day1.md（已记录本日实现思路与示例代码）。
- 提交信息：week2/day1: 学习记录 — 内部类与图形面积计算实践

---

简短说明：我已将今日学习记录写入 study/plans/java/week2/day1.md（位于仓库 moropxin/learning 的默认分支）。如果你希望我把实际 demo 代码文件（Java 源码与单元测试）也一并提交到同目录下，我可以继续创建并提交这些文件。
