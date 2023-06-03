<!DOCTYPE html>
<html>
<head>
    <title>이미지 업로드 및 내려받기</title>
</head>
<body>
    <h1>이미지 업로드 및 내려받기</h1>
        <form action="/upload" method="post" enctype="multipart/form-data">
            <input type="file" name="file">
        <br><br>
        <input type="submit" value="업로드">
        </form>

    <h1>GET 요청 버튼</h1>
    <?php
    $documentRoot = $_SERVER['DOCUMENT_ROOT'];
    ?>
    <a href="<?php echo $documentRoot ?>/tmp/upload" target="_blank">GET 요청 보내기</a>
    </body>
</html>
