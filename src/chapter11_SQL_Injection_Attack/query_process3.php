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
            // $name = $conn->real_escape_string($name);
            // $password = $conn->real_escape_string($password);
            $sql = "SELECT EID,Name,Salary
                    FROM credential
                    WHERE Name=? AND Password=sha1(?)";

            // $result = $conn->multi_query($sql);
            // $result = $conn->query($sql);

            if($stmt = $conn->prepare($sql)){
                $stmt->bind_param("ss",$name,$password);
                $stmt->execute();
            }

            $stmt->bind_result($Eid,$Name,$Salary);
            while($stmt->fetch()){
                echo "$Eid -- $Name -- $Salary <br>";
            }
            $conn->close();
        ?>
    </body>
</html>