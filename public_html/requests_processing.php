<?php
// ini_set('display_errors', 1);
// ini_set('display_startup_errors', 1);
// error_reporting(E_ALL);

define('MSG_FILE_NAME', 'message.json');
define('PASSWORD_HASH', '043a718774c572bd8a25adbeb1bfcd5c0256ae11cecf9f9c3f925d0e52beaf89');
define('TO_MORZE', [
    'а' => '.-',    'б' => '-...',  'в' => '.--',   'г' => '--.',
    'д' => '-..',   'е' => '.',     'ё' => '.',     'ж' => '...-',
    'з' => '--..',  'и' => '..',    'й' => '.---',  'к' => '-.-',
    'л' => '.-..',  'м' => '--',    'н' => '-.',    'о' => '---',
    'п' => '.--.',  'р' => '.-.',   'с' => '...',   'т' => '-',
    'у' => '..-',   'ф' => '..-.',  'х' => '....',  'ц' => '-.-.',
    'ч' => '---.',  'ш' => '----',  'щ' => '--.-',  'ъ' => '.--.-.',
    'ы' => '-.--',  'ь' => '-..-',  'э' => '..-..', 'ю' => '..--',
    'я' => '.-.-'
]);

function str_to_morze($text) {
    $morze = '';
    $chars = preg_split('//u', mb_strtolower($text, 'UTF-8'), -1, PREG_SPLIT_NO_EMPTY);
    
    foreach ($chars as $char) {
        if ($char === ' ') {
            $morze .= '   ';
        } elseif (isset(TO_MORZE[$char])) {
            $morze .= TO_MORZE[$char] . ' ';
        }
    }
    
    return $morze;
}

function morze_to_bin($text){
    $bin = '';
    $chars = str_split($text);

    foreach ($chars as $char) {
        if ($char === '.') {
            $bin .= '1';
        } elseif ($char === '-'){
            $bin .= '111';
        } elseif ($char === ' ') {
            $bin .= '00';
        }

        $bin .= '0';
    }
    
    return $bin;
}

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
    // Только кириллица
    return preg_match('/^[А-Яа-яЁё\s+]+$/u', $message);
}

if ($_SERVER['REQUEST_METHOD'] === 'POST'){
    if (array_key_exists('mode', $_POST) && !validatePassword($_POST['mode'])){
        echo "<script>
                alert('Недопустимые символы в поле \"Mode\". Разрешены только латинские буквы, цифры, и спец символы');
                window.location.href = 'index.html';
            </script>";
        exit;
    }
    if (array_key_exists('password', $_POST) && !validatePassword($_POST['password'])){
        echo "<script>
                alert('Недопустимые символы в поле \"Password\". Разрешены только латинские буквы, цифры, и спец символы');
                window.location.href = 'index.html';
            </script>";
        exit;
    }
    if (array_key_exists('message', $_POST) && !validateMessage($_POST['message'])){
        echo "<script>
                alert('Недопустимые символы в поле \"Message\". Разрешены только буквы кириллицы');
                window.location.href = 'index.html';
            </script>";
        exit;
    }
    $mode = htmlspecialchars($_POST['mode'], ENT_QUOTES, 'UTF-8');
    $password = htmlspecialchars($_POST['password'], ENT_QUOTES, 'UTF-8');
    $password_hash = hash('sha256', $password);
    $msg_file_path = __DIR__ . "/../config" . "/" . MSG_FILE_NAME;

    if ($mode === 'post_message'){
        $message = htmlspecialchars(preg_replace('/[\t\n\r\f\v]+/u', '', mb_strtolower($_POST['message'], 'UTF-8')), ENT_QUOTES, 'UTF-8');
        $message = str_to_morze($message);
        $message = morze_to_bin($message);

        if ($password_hash === PASSWORD_HASH){
            $message_data = [
                'is_readed' => false,
                'message' => $message
            ];

            $json_string = json_encode($message_data, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
            file_put_contents($msg_file_path, $json_string);

            header('Location: index.html');
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

            $response = [
                'status' => 'success',
                'data' => [
                    'message' => $message
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