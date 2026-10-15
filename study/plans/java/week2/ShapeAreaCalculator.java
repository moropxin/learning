package study.plans.java.week2;

/**
 * Demo: ShapeAreaCalculator
 * Shows examples of static nested class, member inner class, local inner class, and anonymous inner class.
 */
public class ShapeAreaCalculator {
    public interface Shape {
        double area();
        String name();
    }

    // 静态内部类（static nested class）——不依赖外部实例
    public static class Rectangle implements Shape {
        private final double w, h;
        public Rectangle(double w, double h) {
            this.w = w;
            this.h = h;
        }
        @Override public double area() { return w * h; }
        @Override public String name() { return "Rectangle"; }
    }

    // 成员内部类（non-static inner class）——隐式持有外部实例引用
    public class Circle implements Shape {
        private final double r;
        public Circle(double r) { this.r = r; }
        @Override public double area() { return Math.PI * r * r; }
        @Override public String name() { return "Circle"; }

        // 辅助方法用于演示 this 与 Outer.this 的区别
        public void printIdentity() {
            System.out.println("Circle.this = " + System.identityHashCode(this));
            System.out.println("ShapeAreaCalculator.this = " + System.identityHashCode(ShapeAreaCalculator.this));
        }
    }

    // 在方法中定义局部内部类（local inner class）示例
    public Shape makeRightTriangle(final double base, final double height) {
        // base 和 height 在局部内部类中被捕获，必须是 effectively final（或显式 final）
        class RightTriangle implements Shape {
            @Override public double area() { return base * height / 2.0; }
            @Override public String name() { return "RightTriangle"; }
        }
        return new RightTriangle();
    }

    // 工具方法：制作一个匿名内部类实例（一次性实现）
    public Shape makeAnonymousShape() {
        return new Shape() {
            @Override public double area() { return 42.0; }
            @Override public String name() { return "Anonymous"; }
        };
    }

    public static void main(String[] args) {
        ShapeAreaCalculator mgr = new ShapeAreaCalculator();

        // 静态内部类直接通过外部类类型 new
        Shape r = new Rectangle(3, 4);
        // 成员内部类需要外部实例来 new
        Shape c = mgr.new Circle(2.0);
        // 局部内部类由工厂方法创建
        Shape t = mgr.makeRightTriangle(3, 4);
        // 匿名内部类
        Shape anon = mgr.makeAnonymousShape();

        Shape[] shapes = { r, c, t, anon };
        double total = 0;
        for (Shape s : shapes) {
            System.out.printf("%s -> area=%.3f%n", s.name(), s.area());
            total += s.area();
        }
        System.out.printf("Total = %.3f%n", total);

        // 演示成员内部类持有外部实例引用
        if (c instanceof ShapeAreaCalculator.Circle) {
            ((ShapeAreaCalculator.Circle) c).printIdentity();
        }
    }
}
