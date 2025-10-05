#!/usr/bin/php-cgi

<?php
// enable error reporting
ini_set('display_errors', 1);
ini_set('error_log', __DIR__ . 'logs/php_errors.log');
error_reporting(E_ALL);

// define variables and set to empty values
$input = $data = $response = $name = $email = $message = $file = $timestamp = "";

// determine the request method
$request_method = $_SERVER['REQUEST_METHOD'];
$uploadDir = $_SERVER['UPLOAD_PATH'];

if ($request_method === 'GET')
{
    if (!isset($_GET['name']) || !isset($_GET['email']) || !isset($_GET['message']))
    {
        header('Content-Type: application/json');
        header('Status: 400 Bad Request');
        echo json_encode(['success' => false, 'error' => 'All fields are required.']);
        exit;
    }

    // parse data from the query
    $name = test_input($_GET['name'] ?? '');
    $email = test_input($_GET['email'] ?? '');
    $message = test_input($_GET['message'] ?? '');
    $timestamp = date('Y-m-d H:i:s');
}
else if ($request_method === 'POST') {
    // parse data from the request body (JSON)

    // read input data from POST
    $input = file_get_contents('php://input');
    file_put_contents("logs/debug.log", "Data:\n" . $input, FILE_APPEND);
    $data = json_decode($input, true);
    if ($data === null) {
        header('Content-Type: application/json');
        header('Status: 400 Bad Request');
        echo json_encode(['success' => false, 'error' => 'Invalid input.']);
        exit;
    }

    // Prepare email content
    // use htmlspecialchars to prevent XSS attacks when outputting user-provided data
    $name = test_input($data['name'] ?? '');
    $email = test_input($data['email'] ?? '');
    $message = test_input($data['message'] ?? '');
    $timestamp = date('Y-m-d H:i:s');
}
else {
    // Unsupported request method
    header('Content-Type: application/json');
    header('Status: 405 Method Not Allowed');
    echo json_encode(['success' => false, 'error' => 'Method not allowed.']);
    exit;
}

if (empty($name) || empty($email) || empty($message) || !filter_var($email, FILTER_VALIDATE_EMAIL)) {
    header('Content-Type: application/json');
    header('Status: 400 Bad Request');
    echo json_encode(['success' => false, 'error' => 'All fields are required.']);
    exit;
}

$email_content = "New Contact Form Submission:\n\n";
$email_content .= "Name: $name\n";
$email_content .= "Email: $email\n";
$email_content .= "Message: $message\n";
$email_content .= "Submitted on: $timestamp\n";

// save to a file
$file = __DIR__ . '/logs' . '/messages.log';
file_put_contents($file, $email_content . "\n\n", FILE_APPEND);

// respond with success
$response =  "<!DOCTYPE html>
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
    <div class=\"home_bnt\">
    <a href=\"/index.html\">H O M E</a><br />
    <h1>Hello, $name!</h1>
    <p>Thank you for your message.</p>
</body>
</html>";

// return a JSON response
header('Status: 200 OK');
echo $response;

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}

?>
