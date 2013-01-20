<?php

/**
 * @file
 *
 *
 * @see themekey.module
 *
 * @author Markus Kalkbrenner | bio.logis GmbH
 *   @see http://drupal.org/user/124705
 *
 * @author profix898
 *   @see http://drupal.org/user/35192
 */


/**
 * Implements hook_themekey_properties().
 *
 * Provides additional properties for the ThemeKey module:
 *   themekey_ui:node_triggers_theme
 *   themekey_ui:node_author_triggers_theme
 *
 * @return
 *   array of themekey properties
 */
function themekey_blog_themekey_properties() {

  // Attributes for properties
  $attributes = array();

  $attributes['blog:uid'] = array(
    'description' => t('Blog: User ID - The id of the user (uid) who owns the blog.'),
    'validator' => 'themekey_validator_ctype_digit',
    'page cache' => THEMEKEY_PAGECACHE_SUPPORTED,
  );

  if (module_exists('themekey_ui')) {
    $attributes['themekey_ui:blog_author_triggers_theme'] = array(
      'description' => t("The property, themekey_ui:blog_author_triggers_theme, could not be selected from the property drop down. You get this static property by activating !link. Afterwards, you can move the property to any position in the rule chain. When done, it triggers the switch to the theme the author selected for her nodes in her user profile. This property is an addition to themekey_ui:node_author_triggers_theme to cover the automatically generated blog pages as well.",
        array('!link' => l(t('Let the user select a theme for her blog pages'), 'admin/config/user-interface/themekey/settings/ui'))),
      'page cache' => THEMEKEY_PAGECACHE_SUPPORTED,
      'static' => TRUE,
    );
  }

  // Mapping functions
  $maps = array();

  $maps[] = array(
    'src' => 'node:nid',
    'dst' => 'blog:uid',
    'callback' => 'themekey_blog_nid2uid',
  );

  $maps[] = array(
    'src' => 'blog:uid',
    'dst' => 'themekey_ui:blog_author_triggers_theme',
    'callback' => 'themekey_blog_author2theme',
  );

  return array('attributes' => $attributes, 'maps' => $maps);
}


/**
* Implements hook_themekey_paths().
*/
function themekey_blog_themekey_paths() {
  $paths = array();

  $paths[] = array('path' => 'blog/#blog:uid');

  return $paths;
}


/**
 * ThemeKey mapping function to set a
 * ThemeKey property's value (destination)
 * with the aid of another ThemeKey property (source).
 *
 * src: node:nid
 * dst: blog:uid
 *
 * @param $nid
 *   a node id
 *
 * @return
 *   int
 *   or NULL if no value could be mapped
 */
function themekey_blog_nid2uid($nid) {
  if ('blog' == themekey_node_get_simple_node_property($nid, 'type')) {
    return themekey_node_get_simple_node_property($nid, 'uid');
  }

  return NULL;
}


/**
 * This function implements the interface of a ThemeKey
 * mapping function but doesn't set a ThemeKey property's
 * value. It sets the Drupal static themekey_custom_theme
 * which will cause ThemeKey to use this theme.
 *
 * @param $uid
 *   a uid, current value of ThemeKey property blog:uid
 *
 * @return
 *   string "static" if global custom theme has been set
 *   or NULL if no theme has been assigned to the node
 */
function themekey_blog_author2theme($uid) {
  $custom_theme = &drupal_static('themekey_custom_theme', '');

  $query = db_select('themekey_ui_author_theme', 'tuat')
    ->fields('tuat', array('theme'))
    ->condition('tuat.uid', $uid);

  if ($theme = $query->execute()->fetchField()) {
    if ('default' != $theme && themekey_check_theme_enabled($theme)) {
      $custom_theme = $theme;
      return 'static';
    }
  }

  return NULL;
}

