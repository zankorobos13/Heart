<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

define('MSG_FILE_NAME', 'message.txt');

if ($_SERVER['REQUEST_METHOD'] === 'POST'){
    $password = $_POST['password'];
    $message = $_POST['message'];
    
    $password = hash('sha256', $password);
    $message = htmlentities($message);

    if (true){
        $message_data = [
            'is_readed' => false,
            'message' => $message
        ];
        $fd = fopen(MSG_FILE_NAME, 'w');
        fwrite($fd, json_encode($message_data));
        fclose($fd);
    }

    header('Location: index.php');
    exit;
}

if (count($_GET) != 0){
    header('Content-Type: application/json');

    $message_data = json_decode(file_get_contents(MSG_FILE_NAME), true);
    $message = $message_data['message'];

    $response = [
        'status' => 'success',
        'data' => [
            'message' => $message
        ]
    ];

    echo json_encode($response);
    exit;
}