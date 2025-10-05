#!/usr/bin/php-cgi

<?php
// enable error reporting
ini_set('display_errors', 1);
ini_set('error_log', __DIR__ . '/logs/php_errors.log');
error_reporting(E_ALL);

// Check if the form was submitted
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Get the file name and body from the form
    $input = json_decode(file_get_contents("php://input"), true);
    $fileName = $input['file_name'] ?? '';
    $fileBody = $input['file_body'] ?? '';

    if (empty($fileName) || empty($fileBody)) {
        header ('Status: 400 Bad request');
        echo json_encode(['success' => false, 'message' => 'File name and body are required.']);
        exit;
    }
    $uploadDir = $_SERVER['UPLOAD_PATH']; // the path from server config
    
    // the full relative file path
    if (strrpos($uploadDir, '/') !== strlen($uploadDir) - 1)
        $uploadDir .= '/';
    $uploadDir = dirname(__DIR__, 2) . '/' . $uploadDir;
    $filePath = $uploadDir . $fileName;

    if (!is_dir($uploadDir)) {
        if (!mkdir($uploadDir, 0755, true)) { //create the directory if it does not exists
            header ('Status: 400 Bad request');
            echo json_encode(['success' => false, 'message' => 'Failed to create directory.', 'path: ' => $uploadDir]);
            exit;
        }
    }
    // Save the file
    if (file_put_contents($filePath, $fileBody) !== false) {
        header ('Status: 201 Created');
        echo json_encode(['success' => true, 'message' => 'File ' . htmlspecialchars($fileName) . ' saved successfully!', 'upload_path: ' => $uploadDir]);
    } else {
        header ('Status: 400 Bad request');
        echo json_encode(['success' => false, 'message' => 'Failed to save file.', 'upload_path: ' => $uploadDir]);
    }
} else {
    // If the request method is not POST, return an error
    header('Status: 405 Method not allowed');
    echo json_encode(['success' => false, 'message' => 'Invalid request method.']);
}
echo("");
ob_end_flush(); // End output buffering
?>
