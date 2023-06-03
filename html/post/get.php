<?php
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    $name = $_GET['Name'];
    $age = $_GET['Age'];
    echo "Name: " . $name . "<br>";
    echo "Age: " . $age . "<br>";
} else {
    echo "GET request not received";
}
?>
