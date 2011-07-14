<?php
// $Id: og_handler_argument_group_audience_gid.inc,v 1.1.2.1 2010/12/15 09:14:11 amitaibu Exp $
/**
 * @file
 * Contains the group argument for field group audience.
 */

/**
 * Field group audience - gid argument handler.
 */
class og_handler_argument_group_audience_gid extends views_handler_argument_numeric {

  /**
   * Override title(); Get the label of the entity type.
   */
  function title() {
    $title = '';
    if (!empty($this->argument) && $group = og_get_group('group', $this->argument)) {
      $title = og_label($group->gid);
    }

    return $title;
  }

}
