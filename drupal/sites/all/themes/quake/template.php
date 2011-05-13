<?php
function quake_preprocess_page($variables){
    if(!isset($variables['node']->type)){
		return;
	}
	$node_type=$variables['node']->type;
	$stylesheet=drupal_get_path('theme', 'quake').'/color.css.php?node='.$node_type;
	drupal_add_css($stylesheet);
}
?>