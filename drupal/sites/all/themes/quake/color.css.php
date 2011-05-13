<?php
$nodetype=array(
	'page'=>'#6E9ECE',
	'meeting'=>'green',
	'news'=>'blue',
	'event'=>'red',
);

if(!array_key_exists($_REQUEST['node'], $nodetype)){
	exit;
} else {
	echo "
	
h2 {
	color: {$nodetype[$_REQUEST['node']]};
}

";
}

?>


