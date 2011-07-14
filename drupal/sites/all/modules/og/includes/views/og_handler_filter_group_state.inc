<?php
// $Id: og_handler_filter_group_state.inc,v 1.1.2.1 2010/12/15 09:14:11 amitaibu Exp $
/**
 * Filter by group state.
 */
class og_handler_filter_group_state extends views_handler_filter_in_operator {
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Group state');
      $this->value_options = og_group_states();
    }
  }
}
