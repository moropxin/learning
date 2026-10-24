import java.sql.*;

public class JDBC {
    public static void main(String[] args) throws Exception {
        // ① 拨号：接线员 DriverManager 接通数据库，返回"电话线"conn
        Connection conn = DriverManager.getConnection(
                "jdbc:mysql://localhost:3306/dbname",   // 网址：mysql协议://主机:端口/数据库名
                "root", "482477665a");                  // 账号、密码

        // ② 下单：在电话线上写点菜单，? 是空位
        String sql = "select * from student where id = ?";
        PreparedStatement ps = conn.prepareStatement(sql);


        // ③ 填单：把第 1 个 ? 填成 1（防止 SQL 注入就靠这一步）
        ps.setInt(1, 1);

        // ④ 点"查询菜"：executeQuery 返回结果集 rs
        ResultSet rs = ps.executeQuery();

        // ⑤ 吃菜：next() 翻一行，再取这一行的列值
        while (rs.next()) {
            System.out.println(rs.getString("name") + " " + rs.getInt("age"));
        }

        // ⑥ 挂电话：先开后关（rs → ps → conn）；下面三行可用 try-with-resources 省略
        rs.close();
        ps.close();
        conn.close();
    }
}
