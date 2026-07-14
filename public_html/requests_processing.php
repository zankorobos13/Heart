<?php
// ini_set('display_errors', 1);
// ini_set('display_startup_errors', 1);
// error_reporting(E_ALL);

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

function validatePassword($password) {
    // Только латиница, цифры, и спец символы
    return preg_match('/^[a-zA-Z0-9!@#$%^&*()_+\-=\[\]{};:,.?\/\\|`~]+$/', $password);
}

function validateMessage($message) {
    // Только латиница, цифры, и спец символы
    return preg_match('/^[a-zA-Z0-9!@#$%^&*()_+\-=\[\]{};:,.?\/\\|`~]+$/', $message);
}

if ($_SERVER['REQUEST_METHOD'] === 'POST'){
    if (array_key_exists('mode', $_POST) && !validatePassword($_POST['mode'])){
        echo "<script>
                alert('Недопустимые символы в поле \"Mode\". Разрешены только латинские буквы, цифры, и спец символы');
                window.location.href = '".$_POST['from']."';
            </script>";
        exit;
    }
    if (array_key_exists('password', $_POST) && !validatePassword($_POST['password'])){
        echo "<script>
                alert('Недопустимые символы в поле \"Пароль\". Разрешены только латинские буквы, цифры, и спецсимволы');
                window.location.href = '".$_POST['from']."';
            </script>";
        exit;
    }
    if (array_key_exists('message', $_POST) && !validateMessage($_POST['message'])){
        echo "<script>
                alert('Недопустимые символы в поле \"Сообщение\". Разрешены только латинские буквы, цифры, и спецсимволы');
                window.location.href = '".$_POST['from']."';
            </script>";
        exit;
    }

    $mode = htmlspecialchars($_POST['mode'], ENT_QUOTES, 'UTF-8');
    $password = htmlspecialchars($_POST['password'], ENT_QUOTES, 'UTF-8');
    $password_hash = hash('sha256', $password);
    $msg_file_path = __DIR__ . "/../config" . "/" . MSG_FILE_NAME;

    if ($mode === 'post_message'){
        $message = $_POST['message'];
       
        if ($password_hash === PASSWORD_HASH){
            $message_data = [
                'is_readed' => false,
                'is_image' => $_POST['isImage'],
                'message' => $message,
                'timestamp' => time()
            ];

            $json_string = json_encode($message_data, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
            file_put_contents($msg_file_path, $json_string);

            header('Location: '.$_POST['from']);
            exit;
        } else {
            echo "<script>
                alert('Неверный пароль!');
                window.location.href = 'index.html';
            </script>";
            exit;
        }
    } elseif ($mode === 'get_message') {
        if ($password_hash === PASSWORD_HASH){
            $message_data = json_decode(file_get_contents($msg_file_path), true);
            $message = $message_data['message'];
            $timestamp = $message_data['timestamp'];
            $isImage = $message_data['is_image'];

            $response = [
                'status' => 'success',
                'data' => [
                    'message' => $message,
                    'is_image' => $isImage,
                    'timestamp' => $timestamp
                ]
            ];

            $message_data['is_readed'] = true;
            $json_string = json_encode($message_data, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
            file_put_contents($msg_file_path, $json_string);
            
            echo json_encode($response);
        } else {
            $response = [
                'status' => 'invalid password'
            ];
            echo json_encode($response);
        }
    }
    else{
        header('Location: index.html');
        exit;
    }
}