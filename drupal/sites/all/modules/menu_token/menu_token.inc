<?php

/*
 * @file
 *
 * Helper functions of menu_token.
 *
 */


/*
 * Load menu token configuration for menu item from database.
 *
 * @param $mlid
 *   Menu item id.
 *
 * @return array
 *   Options array.
 *
 */
function menu_token_get($mlid) {
  global $menu_token_conf;

  if (!$mlid) {
    return NULL;
  }

  if (!isset($menu_token_conf[$mlid])) {
    $item_conf = db_select('menu_token', 't')
      ->fields('t')
      ->condition('t.mlid', $mlid)
      ->execute()->fetchAssoc();
    $menu_token_conf[$mlid] = empty($item_conf) ? NULL : $item_conf;
  }
  
  return $menu_token_conf[$mlid];
}


/*
 * Insert/update menu token configuration for menu item to database.
 *
 * @param $mlid
 *   Menu item id.
 *
 * @param $options
 *   Options array (mlid, link_path).
 *
 */
function menu_token_set($mlid, $options) {
  global $menu_token_conf;

  db_merge('menu_token')
    ->key(array('mlid' => $mlid))
    ->fields($options)
    ->execute();
  
  unset ($menu_token_conf[$mlid]);
}


/*
 * Delete menu token configuration for menu item from database.
 *
 * @param $mlid
 *   Menu item id.
 *
 */
function menu_token_unset($mlid) {
  global $menu_token_conf;

  db_delete('menu_token')->condition('mlid', $mlid)->execute();

  unset ($menu_token_conf[$mlid]);
}


/**
 * Returns true if menu item is being generated by menu administration modules.
 *
 * @see menu_overview_form().
 */
function _menu_token_menu_admin($item) {
  global $menu_admin;
  return $menu_admin;
}