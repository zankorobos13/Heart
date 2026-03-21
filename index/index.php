<?php 
    
?>
<div class = >
    <h1>Form</h1>

    <form action = "requests_processing.php" method = "post">
        <input type ="hidden" name = "mode" value = "post_message"/>
        <input type= "password" name = "password" placeholder = "password"/><br>
        <textarea name = "message" placeholder = "message"></textarea><br>
        
        <input type="submit" value = "Enter" class = "btn btn-success"/>
    </form>
</div>