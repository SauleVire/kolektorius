<?php

mysql_connect("localhost","vardas","slaptazodis");

mysql_select_db("duomenu-baze");

if($_SERVER['HTTP_USER_AGENT'])
	$user_agent = $_SERVER['HTTP_USER_AGENT'];
else
	$user_agent = "NO USER AGENT";
$myFile = "testams.txt";
$fh = fopen($myFile, 'a') or die("FAILED");
	if(isset($_GET["p1"]) && isset($_GET["p2"]) && isset($_GET["p3"]) && isset($_GET["p4"])){
		$stringData = $_GET["p1"] . "\t" . $_GET["p2"] . "\t" . $_GET["p3"] . "\t".  $_GET["p4"] . "\t". time() . " " . date("Y-m-d H:i:s") . "\t" . $user_agent . "\t" . $_SERVER['REMOTE_ADDR'] ."\n";
		mysql_query("INSERT INTO kolektoriaus_duomenys (temp1, temp2, temp3, temp4, ip, timestamp) VALUES ('".$_GET['p1']."', '".$_GET['p2']."', '".$_GET['p3']."', '".$_GET['p4']."', '".$_SERVER['REMOTE_ADDR']."','".time()."')");
	}
	
fwrite($fh, $stringData);
fclose($fh);
print "OK";

?>

