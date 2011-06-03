<?php
$nodetype=array(
	'home'=>'#a4ac73',
	'page'=>'#eac5d3',
	'news'=>'#e0e3e7',
	'event'=>'#f2cfae',
);


if(!array_key_exists($_REQUEST['node'], $nodetype)){
	exit;
} else {
	echo "
	h4 {
	color: {$nodetype[$_REQUEST['node']]};
	}
	
	#bannerQuote { 
		background-color: {$nodetype[$_REQUEST['node']]};
	}

	#leftCol ul li {
	background-image: dash-{$nodetype[$_REQUEST['node']]}.png;
	}
";
}

?>

