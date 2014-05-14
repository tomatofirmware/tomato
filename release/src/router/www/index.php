<html> 
<title>PHP Check File</title> 
<body> 
<?php 
print "<h1>Congratulations, Tomato NGINX server can process PHP!</h1>"; 
print "<h3>Here you can view the PHP configuration on your server</h3>"; 
phpinfo( ); 
exit(); 
?> 
<h1>Sorry, PHP is not enabled on this server.</h1><br /><br /> 
<h3>Check your firmware provider documentation or contact 
your firmware developper to ask them how to enable PHP.</h3> 
</body> 
</html>

