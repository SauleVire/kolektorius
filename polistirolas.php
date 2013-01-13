<?php

mysql_connect("localhost","vardas","slaptazodis");

mysql_select_db("duomenu-baze");

if($_SERVER['HTTP_USER_AGENT'])
	$user_agent = $_SERVER['HTTP_USER_AGENT'];
else
	$user_agent = "NO USER AGENT";
$myFile = "textinis-failas-testavimui.txt";
$fh = fopen($myFile, 'a') or die("FAILED");
	if(isset($_GET["p5"]) && isset($_GET["p6"]) && isset($_GET["p7"]) && isset($_GET["p8"]) && isset($_GET["p9"]) && isset($_GET["p10"])){
		$stringData = $_GET["p5"] . "\t" . $_GET["p6"] . "\t" . $_GET["p7"] . "\t".  $_GET["p8"] . "\t".  $_GET["p9"] . "\t".  $_GET["p10"] . "\t". time() . " " . date("Y-m-d H:i:s") . "\t" . $user_agent . "\t" . $_SERVER['REMOTE_ADDR'] ."\n";
		mysql_query("INSERT INTO polistirolo_duomenys (polistirolas_5cm, polistirolas_10cm, neoporas_5cm, neoporas_10cm, lauko_t, kambario_t, ip, timestamp) VALUES ('".$_GET['p5']."', '".$_GET['p6']."', '".$_GET['p7']."', '".$_GET['p8']."', '".$_GET['p9']."', '".$_GET['p10']."', '".$_SERVER['REMOTE_ADDR']."','".time()."')");
	}
	
fwrite($fh, $stringData);
fclose($fh);
print "OK";

?>

