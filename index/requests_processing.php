<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

define('MSG_FILE_NAME', 'message.json');
define('PASSWORD_HASH', '043a718774c572bd8a25adbeb1bfcd5c0256ae11cecf9f9c3f925d0e52beaf89');

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
    $password = $_POST['password'];
    $password_hash = hash('sha256', $password);

    if ($mode === 'post_message'){
        $message = $_POST['message'];
        
        if ($password_hash === PASSWORD_HASH){
            $message_data = [
                'is_readed' => false,
                'message' => $message
            ];

            $json_string = json_encode($message_data, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
            file_put_contents(MSG_FILE_NAME, $json_string);

            header('Location: index.php');
            exit;
        } else {
            echo "<script>
                alert('Неверный пароль!');
                window.location.href = 'index.php';
            </script>";
            exit;
        }
    } elseif ($mode === 'get_message') {
        if ($password_hash === PASSWORD_HASH){
            $message_data = json_decode(file_get_contents(MSG_FILE_NAME), true);
            $message = $message_data['message'];

            $response = [
                'status' => 'success',
                'data' => [
                    'message' => $message
                ]
            ];

            $message_data['is_readed'] = true;
            $json_string = json_encode($message_data, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
            file_put_contents(MSG_FILE_NAME, $json_string);
            
            echo json_encode($response);
        } else {
            $response = [
                'status' => 'invalid password'
            ];
            echo json_encode($response);
        }
    }
    else{
        header('Location: index.php');
        exit;
    }
    
}