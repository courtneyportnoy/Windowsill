<?php

if(isset($_FILES)) {
    $fileName = $_FILES['file']['name'];
    $fileTempName = $_FILES['file']['tmp_name'];
    $fileType = $_FILES['file']['type'];
    $fileSize = $_FILES['file']['size'];
    $fileError = $_FILES['file']['error'];
    
    //if the file is a JPEG, proceed:
    if(($fileType = "image/jpeg")) {
        //if there's a file error, print it:
        if($fileError > 0) {
            echo "Return Code: " . $fileError . "<br/>";
        }
        //if there's no file error, print some HTML about the file
        else {
            echo "Upload: " . $fileName . "<br/>";
            echo "Type: " . $fileType . "<br/>";
            echo "Size: " . ($fileSize / 1024) . "KB<br/>";
            echo "Temp file: " . $fileTempName . "<br/>";
            
            //if the file already exists, delete previous file
            if(file_exists($fileName)) {
                unlink($fileName);
            }
            //move the file from temp location to this directory
            move_uploaded_file($fileTempName, $fileName);
            echo "Uploaded file stored as: ".$fileName . "<br/>";
        }
    }
    
    //if the file's not a JPEG say so:
    else {
        echo "File is not a JPEG";
    }
}

    include_once('class.phpmailer.php');
    
    $mail             = new PHPMailer(); // defaults to using php "mail()"
    
    $body             = $mail->getFile($fileName);
    $body             = eregi_replace("[\]",'',$body);
    
    $mail->From       = "courtney.mitchell@live.com";
    $mail->FromName   = "Courtney Mitchell";
    
    $mail->Subject    = "A new photo via mail()";
    
    //$mail->AltBody    = "To view the message, please use an HTML compatible email viewer!"; // optional, comment out and test
    
    $mail->MsgHTML($body);
    
    $mail->AddAddress("windowsill@kodakpulse.com", "Picture Frame");
    
    $mail->AddAttachment($fileName);             // attachment
    
    if(!$mail->Send()) {
      echo "Mailer Error: " . $mail->ErrorInfo;
    } else {
      echo "Message sent!";
    }

    
?>

<html>
    <body>
        <form action="fileupload.php" method="post" enctype="multipart/form-data">
            <label for="file">Filename:</label>
            <input type="file" name="file" id="file" />
            <br/>
            <input type="submit" name="submit" value="Upload" />
        </form>
    </body>
</html>