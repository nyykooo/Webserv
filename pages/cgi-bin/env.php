#!/usr/bin/php-cgi

<!DOCTYPE html>
<html lang='en'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Message Received</title>
    <style>
        h1, p {  width: 50%; margin: 20px auto;}
    </style>
</head>
<body>
<?php
    echo "<pre>";
    foreach ($_SERVER as $key => $value) {
        echo "$key: $value\n";
    }
    echo "</pre>";
?>
</body>
</html>
