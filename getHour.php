<?php

//look up dateTime php
//telnet courtney-mitchell.com 80
//GET mtt2/foo.php HTTP1.0
$today = getDate(); //get date from server
$hour = $today[hours] + 3; //add 3 hours to convert from PST to EST
//echo "<$today>";
//print_r($today);
echo "<$hour>"; //echo out hour

?>
