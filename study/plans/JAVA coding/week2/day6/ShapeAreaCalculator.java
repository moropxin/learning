package study.plans.JAVAcoding.week2.day6;

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
