<?php
function quake_preprocess_page($variables){
	if($variables['is_front']){
		$sheet='front';
	}elseif(isset($variables['node']->type)){
 		$sheet=$variables['node']->type;
	}else{
		$pathBasedCSS = array(
			'events'=>'event',
			'news'=>'news',
			'map'=>'meeting'
			'meeting'=>'meeting'
		);
		$firstPageArgument = arg(0);
		$sheet = $pathBasedCSS[$firstPageArgument];
	}
	if(!isset($sheet)){
		return;
	}
	$stylesheet=drupal_get_path('theme', 'quake').'/color.'.$sheet.'.css';
	drupal_add_css($stylesheet);
}
?>