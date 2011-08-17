<?php
function quake_preprocess_page($variables){
	$sheet='front';
	if(isset($variables['node']->type)){
 		$sheet=$variables['node']->type;
	}else{
		$firstPageArgument = arg(0);
		$pathBasedCSS = array(
			'events'=>'event',
			'news'=>'news',
			'map'=>'meeting',
			'meeting'=>'meeting'
		);
		if(array_key_exists($firstPageArgument, $pathBasedCSS)){
			$sheet = $pathBasedCSS[$firstPageArgument];			
		}
	}
	$stylesheet=drupal_get_path('theme', 'quake').'/color.'.$sheet.'.css';
	drupal_add_css($stylesheet);
}
?>