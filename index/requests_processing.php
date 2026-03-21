<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

define('MSG_FILE_NAME', 'message.json');

function sendPostToSelf($data) {
    $url = "http://" . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'];
    
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_POST, true);
    curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($data));

    $response = curl_exec($ch);
    
    return $response;
}

if ($_SERVER['REQUEST_METHOD'] === 'POST'){
    $mode = $_POST['mode'];

    if ($mode == 'post_message'){
        $password = $_POST['password'];
        $message = $_POST['message'];
        
        $password = hash('sha256', $password);
        // $message = htmlentities($message);

        if (true){
            $message_data = [
                'is_readed' => false,
                'message' => $message
            ];

            $json_string = json_encode($message_data, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
            file_put_contents(MSG_FILE_NAME, $json_string);
        }
    } elseif ($mode == 'post_message_status') {
        $password = $_POST['password'];
        $password = hash('sha256', $password);

        $message_data = json_decode(file_get_contents(MSG_FILE_NAME), true);

        if (true){
            $message_data['is_readed'] = true;

            $json_string = json_encode($message_data, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
            file_put_contents(MSG_FILE_NAME, $json_string);
        }
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

    $post_data = [
        'mode' => 'post_message_status',
        'password' => ''
    ];

    sendPostToSelf($post_data);

    echo json_encode($response);
    exit;
}