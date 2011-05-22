<?php
$nodetype=array(
	'home'=>'#a4ac73',
	'page'=>'pink',
	'news'=>'blue',
	'event'=>'orange',
);


if(!array_key_exists($_REQUEST['node'], $nodetype)){
	exit;
} else {
	echo "
	
h4 {
	color: {$nodetype[$_REQUEST['node']]};
}

#leftCol ul li {
	background-image: {$nodetype[$_REQUEST['node']]};
}

#bannerQuote { 
	background-color: {$nodetype[$_REQUEST['node']]}}

";
}

?>

