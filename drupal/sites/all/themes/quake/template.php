<?php
function quake_preprocess_page($variables){
    if(!isset($variables['node']->type)){
		return;
	}
	$node_type=$variables['node']->type;
	$stylesheet=drupal_get_path('theme', 'quake').'/color.'.$node_type.'.css';
	drupal_add_css($stylesheet);
}
?>