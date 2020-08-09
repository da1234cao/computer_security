<?php
    $name = $_POST['name'];
    $password = $_POST['password'];

    function getDB() {
        $dbhost = 'localhost';
        $dbuser = 'dacao';
        $dbpassword = '1735505';
        $dbname = 'crashcourse';

        $conn = new mysqli($dbhost,$dbuser,$dbpassword,$dbname);
        if($conn->connect_error){
            die("connect failed ".$conn->connect_error." \n");
        }
        return $conn;
    }

?>

<!DOCTYPE html>
<html>
    <head>
        <meta charset="utf-8">
    </head>
    <body>
        <?php
            // $name = "Alice'#";
            // $password = "anything";
            // echo "name:".$name."password:".$password."\n";

            $conn = getDB();
            $name = $conn->real_escape_string($name);
            $password = $conn->real_escape_string($password);
            $sql = "SELECT EID,Name,Salary
                    FROM credential
                    WHERE Name='$name' AND Password=sha1('$password')";

            // $result = $conn->multi_query($sql);
            $result = $conn->query($sql);
            if($result){
                while($row = $result->fetch_assoc()){
                    printf("EID: %s -- Name: %s -- Salary: %s<br>",
                            $row['EID'],$row['Name'],$row['Salary']);
                }
                $result->free();
            }else{
                echo "<h1>SQL语句执行失败</h1>";
            }
            $conn->close();
        ?>
    </body>
</html>